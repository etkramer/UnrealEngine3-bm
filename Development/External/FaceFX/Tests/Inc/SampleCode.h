//------------------------------------------------------------------------------
// The sample code for the FaceFX Documentation.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef SampleCode_H__
#define SampleCode_H__

using namespace OC3Ent::Face;

#define GetAppTime() 1.0f


struct FictionalAudioSource
{
	FxReal GetOffsetInSound() { return 1.0f; }
};

struct FictionalAudioSystem
{
	FictionalAudioSource GetSource(const FxName& name) { return audioSource; }

	FictionalAudioSource audioSource;
};

FictionalAudioSystem fictionalAudioSystem;

UNITTEST( Sample, Code )
{
	FxActor* pActor = NULL;
	FxActorInstance* pActorInstance = NULL;

	{
	// Loads an actor from the file using the fast method (reading the entire
	// file into memory and serializing once the entire actor is in memory.)
	pActor = new FxActor();
	FxBool loaded = FxLoadActorFromFile(*pActor, "../Studio/Samples/Slade-Maya.fxa", FxTrue);
	CHECK(loaded);
	}

	FxSize numBytes = 0;
	FxByte* actorMem = FxFile::ReadBinaryFile("C:/Program Files/OC3 Entertainment/FaceFX 1.71/Samples/Slade-Maya.fxa", numBytes);
	FxActor* test = new FxActor();
	FxBool check = FxLoadActorFromMemory(*test, actorMem, numBytes);
	delete test; test = 0;
	FxFree(actorMem, numBytes); actorMem = 0;

	// Ensure the actor was loaded.
	CHECK(pActor);
	CHECK(pActor->GetNumAnimGroups() > 0);

	delete pActor;

	{
	// Loads an actor from a chunk in memory.
	FxBool loaded = FxFalse;
	FxFile actorFile("../Studio/Samples/Slade-Maya.fxa", FxFile::FM_Read|FxFile::FM_Binary);
	if( actorFile.IsValid() )
	{
		// Read the actor file into memory manually.
		FxSize numBytesInFile = actorFile.Length();
		FxByte* pActorMemory = new FxByte[numBytesInFile];
		FxSize numBytesToRead = actorFile.Read(pActorMemory, numBytesInFile);
		FxAssert(numBytesInFile == numBytesToRead);

		// Now load the actor from the memory chunk.
		pActor = new FxActor();
		loaded = FxLoadActorFromMemory(*pActor, pActorMemory, numBytesToRead);

		delete[] pActorMemory;
		pActorMemory = NULL;
	}
	CHECK(loaded);
	}

	pActorInstance = new FxActorInstance(pActor);

	// Sets reference bone 0's client index to 25.
	pActor->GetMasterBoneList().SetRefBoneClientIndex(0, 25);

	// Returns reference bone 0's client index (25 in this case).
	FxInt32 clientIndex = pActorInstance->GetBoneClientIndex(0);

	CHECK(clientIndex == 25);

	{
	// Evaluating the Face Graph.
	// Create 10 instances of the actor.
	FxSize i = 0;
	FxArray<FxActorInstance*> actorInstances;
	for( i = 0; i < 10; ++i ) 
	{
		actorInstances.PushBack(new FxActorInstance(pActor));
	}

	FxName happy("Happy");
	FxSize numInstances = actorInstances.Length();
	for( i = 0; i < numInstances; ++i )
	{
		// Do work only if the actor instance is playing a facial animation.
		if( actorInstances[i]->IsPlayingAnim() )
		{
			// Set any appropriate register values.  For this example, we'll make
			// all the actor instances happy.
			actorInstances[i]->SetRegister(happy, 1.0f, RO_Add);

			// Get the time to pass to Tick(), both in seconds.
			FxReal applicationTime = GetAppTime();
			FxReal audioTime = -1.0f;
			if( actorInstances[i]->IsPlayingAnim() )
			{
				// Audio playback is not part of the SDK: you would use your own system here.
				audioTime = fictionalAudioSystem.GetSource(actorInstances[i]->GetCurrentAnim()->GetName()).GetOffsetInSound();
			}

			// Set the correct animation values into the face graph.
			actorInstances[i]->Tick(applicationTime, audioTime);

			// Begin the frame for this actor instance by updating any pending register operations.
			actorInstances[i]->BeginFrame();

			// Update the bones in the skeleton
			FxSize numBones = actorInstances[i]->GetNumBones();
			for( FxSize boneIndex = 0; boneIndex < numBones; ++boneIndex )
			{
				FxVec3 bonePos;
				FxQuat boneRot;
				FxVec3 boneScale;
				FxReal boneWeight = 0.f;
				actorInstances[i]->GetBone(boneIndex, bonePos, boneRot, boneScale, boneWeight);

				// If you would like to use our client index system to link up bones in your skeleton
				// to bones in the FaceFX actor, make sure you read the bone blending system documentation.
				// If you set that up, you can set transforms into your skeleton directly without searching
				// for the bone by name, like so:
				FxInt32 clientIndex = actorInstances[i]->GetBoneClientIndex(boneIndex);
				if( FX_INT32_MAX != clientIndex )
				{
					// Use clientIndex as an index into your skeletal structure or some sort of bone identifier
					// and set the bone transformation in your skeleton here.
				}
			}

			// Update the register values for later inspection, and end the frame for this actor instance.
			actorInstances[i]->EndFrame();
		}
	} // End Face Graph eval example.

	i = 0;
	// Do work only if the actor instance is playing a facial animation or non anim ticks are allowed.
	if( actorInstances[i]->IsPlayingAnim() || actorInstances[i]->GetAllowNonAnimTick() )
	{
		// ... the rest of the example is unchanged.
	}

	for( FxSize i = 0; i < 10; ++i )
	{
		delete actorInstances[i];
	}
	}

	{
	// Making a character happy.
	FxName registerName("Happy");
	pActorInstance->SetRegister(registerName, 1.0f, RO_Add);
	// The actor instance now has "Happy" at 1, and will continue to across
	// frames until the register is reset.
	// ...
	// "Unset" the register.
	pActorInstance->SetRegister(registerName, 0.0f, RO_None);
	}

	{
	// Making a character happy, smoothly.
	// Tell the register to blend from the current register value to 1 over
	// 0.75 seconds.
	pActorInstance->SetRegister("Happy", 1.0f, RO_Add, 0.75f);
	// The actor instance will interpolate over the next 3/4 of a second to
	// its new value.  Any frames rendered during this time will show the 
	// character becoming happier and happier, until its target value of 1 is
	// reached, and that value will be sustained until it is set to a new value
	// or reset.
	// ... frames pass ...
	// Remove the register's influence.  
	pActorInstance->SetRegister("Happy", 0.0f, RO_Add, 1.0f);
	}

	{
	// Removing an effect from a canned animation.
	// Using the Godfather example from Slade, where the node's value is 
	// stuck at 1.0 for the duration of the animation.  Let's say we want to
	// remove the effect from the animation.
	pActorInstance->SetRegister("Godfather", 0.0f, RO_Multiply);
	}

	{
	// Triggering a dynamic blink.
	// In one call, tell the register to go to 1 over 0.125 seconds, and once it
	// reaches 1, to go immediately back to 0 over 0.083 seconds.
	pActorInstance->SetRegisterEx("Blink", RO_Add, 1.0f, 0.125f, RO_Add, 0.0f, 0.083f);
	}

	{
	// Mounting an anim set.
	FxAnimSet animSet;
	FxName animGroupName;
	if( FxLoadAnimSetFromFile(animSet, "../Studio/Samples/ExternalAnimSet_noText.fxe", FxTrue) )
	{
		// Cache anim group name for next example.
		animGroupName = animSet.GetAnimGroup().GetName();
	    pActor->MountAnimSet(animSet);
	}

	CHECK(pActor->FindAnimGroup(animGroupName) != FxInvalidIndex);

	// Assumes the group name was cached when it was mounted.
	pActor->UnmountAnimSet(animGroupName);

	CHECK(pActor->FindAnimGroup(animGroupName) == FxInvalidIndex);
	delete pActorInstance;
	delete pActor;
	}
}

struct GameXCharacterType
{
	const FxName& GetName() { return FxName::NullName; }
	FxString GetFaceFXActorPath() { return "../Studio/Samples/Slade-Maya.fxa"; }
	FxActorInstance& GetFaceFXActorInstance() { return actorInstance; }

	FxActorInstance actorInstance;
};

// Loads FaceFX actors for GameX.
void GameX_LoadFaceFXActor( GameXCharacterType& gameXCharacter )
{
	// Find the actor in the FaceFX actor management system.
	FxActor* pActor = FxActor::FindActor(gameXCharacter.GetName());
	if( !pActor )
	{
		// Do something to figure out where the FaceFX actor file is located.
		FxString actorPath = gameXCharacter.GetFaceFXActorPath();
		// Load the FaceFX Actor.
		pActor = new FxActor();
		if( FxLoadActorFromFile(*pActor, actorPath.GetData(), FxTrue) )
		{
			// Add the actor to the actor list.
			FxActor::AddActor(pActor);
		}
		else
		{
			// The actor couldn't be loaded, so free the memory.
			delete pActor;
			pActor = NULL;
		}
	}

	if( pActor )
	{
		// Set the actor pointer in the actor instance.
		gameXCharacter.GetFaceFXActorInstance().SetActor(pActor);
	}
} // GameX_LoadFaceFXActor

#endif
