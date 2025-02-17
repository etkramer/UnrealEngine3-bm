/*=============================================================================
	UnInterpolation.cpp: Code for supporting interpolation of properties in-game.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineSequenceClasses.h"
#include "EngineInterpolationClasses.h"
#include "EngineAnimClasses.h"
#include "EngineParticleClasses.h"
#include "EngineMaterialClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "EngineSoundClasses.h"
#include "UnLinkedObjDrawUtils.h"
#include "UnInterpolationHitProxy.h"

// Priority with which to display sounds triggered by Matinee sound tracks.
#define SUBTITLE_PRIORITY_MATINEE	10000

IMPLEMENT_CLASS(AInterpActor);
IMPLEMENT_CLASS(AMatineeActor);

IMPLEMENT_CLASS(USeqAct_Interp);

IMPLEMENT_CLASS(UInterpData);

IMPLEMENT_CLASS(UInterpGroup);
IMPLEMENT_CLASS(UInterpGroupInst);

IMPLEMENT_CLASS(UInterpGroupDirector);
IMPLEMENT_CLASS(UInterpGroupInstDirector);

IMPLEMENT_CLASS(UInterpTrack);
IMPLEMENT_CLASS(UInterpTrackInst);

IMPLEMENT_CLASS(UInterpTrackInstProperty);

IMPLEMENT_CLASS(UInterpTrackMove);
IMPLEMENT_CLASS(UInterpTrackInstMove);

IMPLEMENT_CLASS(UInterpTrackToggle);
IMPLEMENT_CLASS(UInterpTrackInstToggle);

IMPLEMENT_CLASS(UInterpTrackFloatBase);
IMPLEMENT_CLASS(UInterpTrackVectorBase);

IMPLEMENT_CLASS(UInterpTrackFloatProp);
IMPLEMENT_CLASS(UInterpTrackInstFloatProp);

IMPLEMENT_CLASS(UInterpTrackVectorProp);
IMPLEMENT_CLASS(UInterpTrackInstVectorProp);

IMPLEMENT_CLASS(UInterpTrackColorProp);
IMPLEMENT_CLASS(UInterpTrackInstColorProp);

IMPLEMENT_CLASS(UInterpTrackEvent);
IMPLEMENT_CLASS(UInterpTrackInstEvent);

IMPLEMENT_CLASS(UInterpTrackDirector);
IMPLEMENT_CLASS(UInterpTrackInstDirector);

IMPLEMENT_CLASS(UInterpTrackFade);
IMPLEMENT_CLASS(UInterpTrackInstFade);

IMPLEMENT_CLASS(UInterpTrackSlomo);
IMPLEMENT_CLASS(UInterpTrackInstSlomo);

IMPLEMENT_CLASS(UInterpTrackAnimControl);
IMPLEMENT_CLASS(UInterpTrackInstAnimControl);

IMPLEMENT_CLASS(UInterpTrackSound);
IMPLEMENT_CLASS(UInterpTrackInstSound);

IMPLEMENT_CLASS(UInterpTrackFloatParticleParam);
IMPLEMENT_CLASS(UInterpTrackInstFloatParticleParam);

IMPLEMENT_CLASS(UInterpTrackFloatMaterialParam);
IMPLEMENT_CLASS(UInterpTrackInstFloatMaterialParam);

IMPLEMENT_CLASS(UInterpTrackVectorMaterialParam);
IMPLEMENT_CLASS(UInterpTrackInstVectorMaterialParam);

IMPLEMENT_CLASS(UInterpTrackColorScale);
IMPLEMENT_CLASS(UInterpTrackInstColorScale);

IMPLEMENT_CLASS(UInterpTrackFaceFX);
IMPLEMENT_CLASS(UInterpTrackInstFaceFX);

IMPLEMENT_CLASS(UInterpTrackMorphWeight);
IMPLEMENT_CLASS(UInterpTrackInstMorphWeight);

IMPLEMENT_CLASS(UInterpTrackSkelControlScale);
IMPLEMENT_CLASS(UInterpTrackInstSkelControlScale);

IMPLEMENT_CLASS(UInterpTrackAudioMaster);
IMPLEMENT_CLASS(UInterpTrackInstAudioMaster);

IMPLEMENT_CLASS(UInterpTrackVisibility);
IMPLEMENT_CLASS(UInterpTrackInstVisibility);

IMPLEMENT_CLASS(UInterpTrackParticleReplay);
IMPLEMENT_CLASS(UInterpTrackInstParticleReplay);


/*-----------------------------------------------------------------------------
	Macros for making arrays-of-structs type tracks easier
-----------------------------------------------------------------------------*/

#define STRUCTTRACK_GETNUMKEYFRAMES( TrackClass, KeyArray ) \
INT TrackClass::GetNumKeyframes() \
{ \
	return KeyArray.Num(); \
}

#define STRUCTTRACK_GETTIMERANGE( TrackClass, KeyArray, TimeVar ) \
void TrackClass::GetTimeRange(FLOAT& StartTime, FLOAT& EndTime) \
{ \
	if(KeyArray.Num() == 0) \
	{ \
		StartTime = 0.f; \
		EndTime = 0.f; \
	} \
	else \
	{ \
		StartTime = KeyArray(0).TimeVar; \
		EndTime = KeyArray( KeyArray.Num()-1 ).TimeVar; \
	} \
}

#define STRUCTTRACK_GETKEYFRAMETIME( TrackClass, KeyArray, TimeVar ) \
FLOAT TrackClass::GetKeyframeTime(INT KeyIndex) \
{ \
	if( KeyIndex < 0 || KeyIndex >= KeyArray.Num() ) \
	{ \
		return 0.f; \
	} \
 	return KeyArray(KeyIndex).TimeVar; \
}

#define STRUCTTRACK_SETKEYFRAMETIME( TrackClass, KeyArray, TimeVar, KeyType ) \
INT TrackClass::SetKeyframeTime(INT KeyIndex, FLOAT NewKeyTime, UBOOL bUpdateOrder) \
{ \
	if( KeyIndex < 0 || KeyIndex >= KeyArray.Num() ) \
	{ \
		return KeyIndex; \
	} \
	if(bUpdateOrder) \
	{ \
		/* First, remove cut from track */ \
		KeyType MoveKey = KeyArray(KeyIndex); \
		KeyArray.Remove(KeyIndex); \
		/* Set its time to the new one. */ \
		MoveKey.TimeVar = NewKeyTime; \
		/* Find correct new position and insert. */ \
		INT i=0; \
		for( i=0; i<KeyArray.Num() && KeyArray(i).TimeVar < NewKeyTime; i++); \
		KeyArray.InsertZeroed(i); \
		KeyArray(i) = MoveKey; \
		return i; \
	} \
	else \
	{ \
		KeyArray(KeyIndex).TimeVar = NewKeyTime; \
		return KeyIndex; \
	} \
}

#define STRUCTTRACK_REMOVEKEYFRAME( TrackClass, KeyArray ) \
void TrackClass::RemoveKeyframe(INT KeyIndex) \
{ \
	if( KeyIndex < 0 || KeyIndex >= KeyArray.Num() ) \
	{ \
		return; \
	} \
	KeyArray.Remove(KeyIndex); \
}

#define STRUCTTRACK_DUPLICATEKEYFRAME( TrackClass, KeyArray, TimeVar, KeyType ) \
INT TrackClass::DuplicateKeyframe(INT KeyIndex, FLOAT NewKeyTime) \
{ \
	if( KeyIndex < 0 || KeyIndex >= KeyArray.Num() ) \
	{ \
		return INDEX_NONE; \
	} \
	KeyType NewKey = KeyArray(KeyIndex); \
	NewKey.TimeVar = NewKeyTime; \
	/* Find the correct index to insert this key. */ \
	INT i=0; for( i=0; i<KeyArray.Num() && KeyArray(i).TimeVar < NewKeyTime; i++); \
	KeyArray.InsertZeroed(i); \
	KeyArray(i) = NewKey; \
	return i; \
}

#define STRUCTTRACK_GETCLOSESTSNAPPOSITION( TrackClass, KeyArray, TimeVar ) \
UBOOL TrackClass::GetClosestSnapPosition(FLOAT InPosition, TArray<INT> &IgnoreKeys, FLOAT& OutPosition) \
{ \
	if(KeyArray.Num() == 0) \
	{ \
		return false; \
	} \
	UBOOL bFoundSnap = false; \
	FLOAT ClosestSnap = 0.f; \
	FLOAT ClosestDist = BIG_NUMBER; \
	for(INT i=0; i<KeyArray.Num(); i++) \
	{ \
		if(!IgnoreKeys.ContainsItem(i)) \
		{ \
			FLOAT Dist = Abs( KeyArray(i).TimeVar - InPosition ); \
			if(Dist < ClosestDist) \
			{ \
				ClosestSnap = KeyArray(i).TimeVar; \
				ClosestDist = Dist; \
				bFoundSnap = true; \
			} \
		} \
	} \
	OutPosition = ClosestSnap; \
	return bFoundSnap; \
}

/*-----------------------------------------------------------------------------
	AActor
-----------------------------------------------------------------------------*/

UBOOL AActor::FindInterpMoveTrack(UInterpTrackMove** OutMoveTrack, UInterpTrackInstMove** OutMoveTrackInst, USeqAct_Interp** OutSeq)
{
	for(INT i=0; i<LatentActions.Num(); i++)
	{
		USeqAct_Interp* InterpAct = Cast<USeqAct_Interp>(LatentActions(i) );
		if(InterpAct)
		{
			UInterpGroupInst* GrInst = InterpAct->FindGroupInst(this);
			check(GrInst); // Should have an instance of some group for this actor.
			check(GrInst->Group); // Should be based on an InterpGroup
			check(GrInst->TrackInst.Num() == GrInst->Group->InterpTracks.Num()); // Check number of tracks match up.

			for(INT j=0; j<GrInst->Group->InterpTracks.Num(); j++)
			{
				UInterpTrackMove* MoveTrack = Cast<UInterpTrackMove>( GrInst->Group->InterpTracks(j) );
				if(MoveTrack)
				{
					*OutMoveTrack = MoveTrack;
					*OutMoveTrackInst = CastChecked<UInterpTrackInstMove>( GrInst->TrackInst(j) );
					*OutSeq = InterpAct;
					return true;
				}
			}
		}
	}

	*OutMoveTrack = NULL;
	*OutMoveTrackInst = NULL;
	*OutSeq = NULL;
	return false;
}

void AMatineeActor::TickSpecial(FLOAT DeltaTime)
{
	Super::TickSpecial(DeltaTime);

	if (Role < ROLE_Authority && bIsPlaying && InterpAction != NULL)
	{
		InterpAction->StepInterp(DeltaTime, false);
	}
}

void AInterpActor::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);

	// handle AI notifications
	if (bMonitorMover)
	{
		if (Velocity.IsZero())
		{
			bMonitorMover = false;
			for (AController* C = GWorld->GetWorldInfo()->ControllerList; C != NULL; C = C->NextController)
			{
				if (C->PendingMover == this)
				{
					bMonitorMover = !C->eventMoverFinished() || bMonitorMover;
				}
			}
			MaxZVelocity = 0.f;
		}
		else
		{
			MaxZVelocity = ::Max(MaxZVelocity, Velocity.Z);
			if (bMonitorZVelocity && Velocity.Z > 0.f && MaxZVelocity > 2.f * Velocity.Z)
			{
				bMonitorMover = false;
				for (AController* C = GWorld->GetWorldInfo()->ControllerList; C != NULL; C = C->NextController)
				{
					if (C->PendingMover == this)
					{
						bMonitorMover = !C->eventMoverFinished() || bMonitorMover;
					}
				}
				MaxZVelocity = 0.f;
				bMonitorZVelocity = bMonitorMover;
			}
		}
	}
	else
	{
		MaxZVelocity = 0.f;
	}
}

/** called when the level that contains this sequence object is being removed/unloaded */
void USeqAct_Interp::CleanUp()
{
	Super::CleanUp();

	TermInterp();
	if (ReplicatedActor != NULL)
	{
		GWorld->DestroyActor(ReplicatedActor);
	}
	// clear from all actors LatentActions list, so they don't try to reference us again
	//@todo: maybe all latent actions should do this?
	for (INT i = 0; i < LatentActors.Num(); i++)
	{
		if (LatentActors(i) != NULL)
		{
			LatentActors(i)->LatentActions.RemoveItem(this);
		}
	}
}

void USeqAct_Interp::InitGroupActorForGroup(class UInterpGroup* InGroup, class AActor* GroupActor)
{
	// Create a new variable connector on all Matinee's using this data.
	USequence* RootSeq = Cast<USequence>(GetOuter());
	if (RootSeq == NULL)
	{
		RootSeq = ParentSequence;
	}
	check(RootSeq);
	RootSeq->UpdateInterpActionConnectors();

	// Find the newly created connector on this SeqAct_Interp. Should always have one now!
	const INT NewLinkIndex = FindConnectorIndex(InGroup->GroupName.ToString(), LOC_VARIABLE );
	check(NewLinkIndex != INDEX_NONE);
	FSeqVarLink* NewLink = &(VariableLinks(NewLinkIndex));

	if(GroupActor)
	{
		USeqVar_Object* NewObjVar = ConstructObject<USeqVar_Object>( USeqVar_Object::StaticClass(), RootSeq, NAME_None, RF_Transactional );

		NewObjVar->ObjPosX = ObjPosX + 50 * VariableLinks.Num();
		NewObjVar->ObjPosY = ObjPosY + DrawHeight;
		NewObjVar->ObjValue = GroupActor;
		NewObjVar->OnCreated();

		RootSeq->SequenceObjects.AddItem(NewObjVar);

		// Set the new variable connector to point to the new variable.
		NewLink->LinkedVariables.AddItem(NewObjVar);
	}
}

/*-----------------------------------------------------------------------------
  UInterpData
-----------------------------------------------------------------------------*/

FString UInterpData::GetValueStr()
{
	return FString::Printf( TEXT("Matinee Data (%3.1fs)"), InterpLength );
}

/** Search through all InterpGroups in this InterpData to find a group whose GroupName matches the given name. Returns NULL if not group found. */
INT UInterpData::FindGroupByName(FName InGroupName)
{
	if(InGroupName != NAME_None)
	{
		for(INT i=0; i<InterpGroups.Num(); i++)
		{
			const FName& GroupName = InterpGroups(i)->GroupName;
			if( GroupName == InGroupName )
			{
				return i;
			}
		}
	}

	return INDEX_NONE;
}

/** Search through all InterpGroups in this InterpData to find a group whose GroupName matches the given name. Returns NULL if not group found. */
INT UInterpData::FindGroupByName(const FString& InGroupName)
{
	for(INT i=0; i<InterpGroups.Num(); i++)
	{
		const FName& GroupName = InterpGroups(i)->GroupName;
		if( GroupName.ToString() == InGroupName )
		{
			return i;
		}
	}

	return INDEX_NONE;
}

/** Search through all groups to find all tracks of the given class. */
void UInterpData::FindTracksByClass(UClass* TrackClass, TArray<UInterpTrack*>& OutputTracks)
{
	for(INT i=0; i<InterpGroups.Num(); i++)
	{
		UInterpGroup* Group = InterpGroups(i);
		Group->FindTracksByClass(TrackClass, OutputTracks);
	}
}

/** Find a DirectorGroup in the data. Should only ever be 0 or 1 of these! */
UInterpGroupDirector* UInterpData::FindDirectorGroup()
{
	UInterpGroupDirector* DirGroup = NULL;

	for(INT i=0; i<InterpGroups.Num(); i++)
	{
		UInterpGroupDirector* TestDirGroup = Cast<UInterpGroupDirector>( InterpGroups(i) );
		if(TestDirGroup)
		{
			check(!DirGroup); // Should only have 1 DirectorGroup at most!
			DirGroup = TestDirGroup;
		}
	}

	return DirGroup;
}

/** Get all the names of events in EventTracks generated by this InterpData. */
void UInterpData::GetAllEventNames(TArray<FName>& OutEventNames)
{
	TArray<UInterpTrack*> Tracks;
	FindTracksByClass(UInterpTrackEvent::StaticClass(), Tracks);

	for(INT i=0; i<Tracks.Num(); i++)
	{
		UInterpTrackEvent* EventTrack = CastChecked<UInterpTrackEvent>( Tracks(i) );

		for(INT j=0; j<EventTrack->EventTrack.Num(); j++)
		{
			OutEventNames.AddUniqueItem( EventTrack->EventTrack(j).EventName );
		}
	}
}


/*-----------------------------------------------------------------------------
  USeqAct_Interp
-----------------------------------------------------------------------------*/

void USeqAct_Interp::UpdateObject()
{
	Modify();

	// only update input links for matinee as everything else is generated based on the matinee data
	USequenceOp *DefaultOp = GetArchetype<USequenceOp>();
	checkSlow(DefaultOp);

	// check for changes to input links
	{
		if (InputLinks.Num() < DefaultOp->InputLinks.Num())
		{
			// add room for new entries
			InputLinks.AddZeroed(DefaultOp->InputLinks.Num()-InputLinks.Num());
		}
		else
		if (InputLinks.Num() > DefaultOp->InputLinks.Num())
		{
			if (DefaultOp->InputLinks.Num() == 0)
			{
				InputLinks.Empty();
			}
			else
			{
				// remove the extra entries
				InputLinks.Remove(DefaultOp->InputLinks.Num()-1,InputLinks.Num()-DefaultOp->InputLinks.Num());
			}
		}
		// match up the properties of all the input links
		for (INT Idx = 0; Idx < InputLinks.Num(); Idx++)
		{
			InputLinks(Idx).LinkDesc = DefaultOp->InputLinks(Idx).LinkDesc;
		}
	}
	USequenceObject::UpdateObject();
}

/** When you create a SeqAct_Interp (aka Matinee action) automatically create an InterpData as well and connect it. */
void USeqAct_Interp::OnCreated()
{
	Super::OnCreated();

	// Declared in default properties
	check(VariableLinks.Num() >= 1);
	check(VariableLinks(0).ExpectedType == UInterpData::StaticClass());
	check(VariableLinks(0).LinkedVariables.Num() == 0);

	// Create new InterpData to go along with the new Matinee
	USequence* ParentSeq = Cast<USequence>(GetOuter());;
	if (!ParentSeq)
	{
		ParentSeq = ParentSequence;
	}
	UInterpData* NewData = ConstructObject<UInterpData>( UInterpData::StaticClass(), GetOuter(), NAME_None, RF_Transactional);

	check(ParentSeq);

	// Add to sequence (this sets the ParentSequence pointer)
	if( ParentSeq->AddSequenceObject(NewData) )
	{
		NewData->ObjPosX = ObjPosX;
		NewData->ObjPosY = ObjPosY + 200;
		NewData->OnCreated();
		NewData->Modify();

		// Add to links of Matinee action.
		VariableLinks(0).LinkedVariables.AddItem(NewData);
	}
}

static USequenceVariable* GetFirstVar( const FSeqVarLink& VarLink )
{
	for (INT idx = 0; idx < VarLink.LinkedVariables.Num(); idx++)
	{
		if(VarLink.LinkedVariables(idx) != NULL)
		{
			return VarLink.LinkedVariables(idx);
		}
	}

	return NULL;
}

/** 
 *	Return the InterpData currently connected to this Matinee action. Returns NULL if none connected. 
 *	Should never allow more than 1 InterpData connected.
 */
UInterpData* USeqAct_Interp::FindInterpDataFromVariable()
{
	USequence* RootSeq = GetRootSequence();
	// First variable connector should always be the InterpData.
	if (RootSeq != NULL &&
		VariableLinks.Num() > 0 &&
		VariableLinks(0).ExpectedType == UInterpData::StaticClass() &&
		VariableLinks(0).LinkedVariables.Num() > 0)
	{
		// We need to handle the case where the InterpData is connected via an External or Named variable.
		// Here we keep traversing these until we either find an InterpData, or fail (returning NULL).
		USequenceVariable* Var = VariableLinks(0).LinkedVariables(0);
		while(Var)
		{
			UInterpData* Data = Cast<UInterpData>(Var);
			if( Data )
			{
				return Data;
			}

			USeqVar_External* ExtVar = Cast<USeqVar_External>(Var);
			USeqVar_Named* NamedVar = Cast<USeqVar_Named>(Var);
			Var = NULL;

			if(ExtVar)
			{
				USequence* ParentSeq = Cast<USequence>(ExtVar->GetOuter());
				if (ParentSeq != NULL)
				{
					for (INT varIdx = 0; varIdx < ParentSeq->VariableLinks.Num(); varIdx++)
					{
						if (ParentSeq->VariableLinks(varIdx).LinkVar == ExtVar->GetFName())
						{
							Var = GetFirstVar( ParentSeq->VariableLinks(varIdx) );					
						}
					}
				}
			}
			else if(NamedVar)
			{
				TArray<USequenceVariable*> Vars;
				RootSeq->FindNamedVariables(NamedVar->FindVarName, false, Vars);

				if( Vars.Num() == 1 )
				{
					Data = Cast<UInterpData>( Vars(0) );
					if(Data)
					{
						return Data;
					}
				}
			}
		}
	}

	return NULL;
}

/** Synchronize the variable connectors with the currently attached InterpData. */
void USeqAct_Interp::UpdateConnectorsFromData()
{
	UInterpData* const Data = FindInterpDataFromVariable();

	USeqAct_Interp* const ClassDefaultObject = (USeqAct_Interp*) GetClass()->GetDefaultObject();
	INT const NumDefaultVariableLinks = ClassDefaultObject ? ClassDefaultObject->VariableLinks.Num() : 0;
	INT const NumDefaultOutputLinks = ClassDefaultObject ? ClassDefaultObject->OutputLinks.Num() : 0;

	if (Data)
	{
		// Remove any connectors for which there is no Group (or group is a Director Group or Folder). Note, we don't
		// check the MatineeData connector!
		for(INT i=VariableLinks.Num()-1; i>=NumDefaultVariableLinks; i--)
		{
			// ignore exposed variable links
			if (VariableLinks(i).PropertyName != NAME_None)
			{
				continue;
			}
			const TCHAR* LinkDescription = *VariableLinks(i).LinkDesc;
			const FName LinkGroupName( LinkDescription );
			const INT GroupIndex = Data->FindGroupByName( LinkGroupName );
			if( GroupIndex == INDEX_NONE || Data->InterpGroups(GroupIndex)->IsA(UInterpGroupDirector::StaticClass()) || Data->InterpGroups(GroupIndex)->bIsFolder )
			{
				VariableLinks.Remove(i);
			}
		}

		// Ensure there is a connector for each InterpGroup.
		for(INT i=0; i<Data->InterpGroups.Num(); i++)
		{
			// Ignore director groups and folders
			if( !Data->InterpGroups(i)->IsA(UInterpGroupDirector::StaticClass()) && !Data->InterpGroups(i)->bIsFolder )
			{
				FName GroupName = Data->InterpGroups(i)->GroupName;
				if(FindConnectorIndex( GroupName.ToString(), LOC_VARIABLE ) == INDEX_NONE)
				{
					FSeqVarLink NewLink;
					appMemset(&NewLink, 0, sizeof(FSeqVarLink));
					NewLink.MinVars = 0;
					NewLink.MaxVars = 255;
					NewLink.ExpectedType = USeqVar_Object::StaticClass();
					NewLink.LinkDesc = Data->InterpGroups(i)->GroupName.ToString();

					VariableLinks.AddItem(NewLink);
				}
			}
		}

		// Take care of output connectors in a similar way.
		TArray<FName> EventNames;
		Data->GetAllEventNames(EventNames);

		for(INT i=OutputLinks.Num()-1; i>=NumDefaultOutputLinks; i--)
		{		
			const TCHAR* LinkDescription = *OutputLinks(i).LinkDesc;
			const FName LinkName( LinkDescription );
			if( !EventNames.ContainsItem(LinkName) )
			{
				OutputLinks.Remove(i);
			}
		}

		for(INT i=0; i<EventNames.Num(); i++)
		{
			const FString EventString = EventNames(i).ToString();
			const INT OutputIndex = FindConnectorIndex( EventString, LOC_OUTPUT );
			if(OutputIndex == INDEX_NONE)
			{
				const INT NewOutIndex = OutputLinks.AddZeroed();

				FSeqOpOutputLink NewOut;
				appMemset( &NewOut, 0, sizeof(FSeqOpOutputLink) );
				NewOut.LinkDesc = EventString;

				OutputLinks(NewOutIndex) = NewOut;
			}
		}
	}
	else
	{
		// First N variable links are always there  - for MatineeData
		if(VariableLinks.Num() > NumDefaultVariableLinks)
		{
			VariableLinks.Remove(NumDefaultVariableLinks, VariableLinks.Num() - NumDefaultVariableLinks);
		}

		// First 2 output links are always there - for Completed and Aborted outputs
		if(OutputLinks.Num() > NumDefaultOutputLinks)
		{
			OutputLinks.Remove(NumDefaultOutputLinks, OutputLinks.Num() - NumDefaultOutputLinks);
		}
	}
}

void USeqAct_Interp::Activated()
{
	USequenceOp::Activated();
	if( bIsPlaying )
	{
		// Don't think we should ever get here...
		return;
	}

	// See if the 'Play' or 'Reverse' inputs have got an impulse. If so, start up action and get it ticking.
	// do not start up if we're client side only and on a dedicated server
	if ((!bClientSideOnly || GWorld->GetNetMode() != NM_DedicatedServer) && (InputLinks(0).bHasImpulse || InputLinks(1).bHasImpulse || InputLinks(4).bHasImpulse))
	{
		InitInterp();

		if( InputLinks(0).bHasImpulse )
		{
			Play();
		}
		else if( InputLinks(1).bHasImpulse )
		{
			Reverse();
		}
		else if (InputLinks(4).bHasImpulse)
		{
			ChangeDirection();
		}

		// For each Actor being interpolated- add us to its LatentActions list, and add it to our LatentActors list.
		// Similar to USequenceAction::Activated - but we don't call a handler function on the Object. Should we?
		TArray<UObject**> objectVars;
		GetObjectVars(objectVars);

		for(INT i=0; i<objectVars.Num(); i++)
		{
			if( objectVars( i ) != NULL )
			{
				AActor* Actor = Cast<AActor>( *(objectVars(i)) );
				if( Actor )
				{
					// ignore Actor if we failed to initialize a group for it. This can happen if the tracks in the matinee data are changed, but the connections are not.
					// This most often seems to happen if a prefab changes but the map instance hasn't been updated.
					if (FindGroupInst(Actor) != NULL)
					{
						PreActorHandle(Actor);
						// if actor has already been ticked, reupdate physics with updated position from track.
						// Fixes Matinee viewing through a camera at old position for 1 frame.
						Actor->performPhysics(1.f);
						Actor->eventInterpolationStarted(this);
					}
					else
					{
						debugf(NAME_Warning, TEXT("%s has no groups that reference connected Actor '%s'"), *GetName(), *Actor->GetName());
					}
				}
			}
		}
		// if we are a server, create/update matinee actor for replication to clients
		if (!bClientSideOnly && GWorld->GetNetMode() != NM_Client)
		{
			if ((ReplicatedActor == NULL || ReplicatedActor->bDeleteMe) && ReplicatedActorClass != NULL)
			{
				ReplicatedActor = (AMatineeActor*)GWorld->SpawnActor(ReplicatedActorClass);
				check(ReplicatedActor != NULL);
				ReplicatedActor->InterpAction = this;
			}
			if (ReplicatedActor != NULL)
			{
				ReplicatedActor->eventUpdate();
			}
		}
	}
}

void USeqAct_Interp::NotifyActorsOfChange()
{
	for (INT i = 0; i < LatentActors.Num(); i++)
	{
		AActor* Actor = LatentActors(i);
		if (Actor != NULL && !Actor->IsPendingKill())
		{
			Actor->eventInterpolationChanged(this);
		}
	}
	if (ReplicatedActor != NULL)
	{
		ReplicatedActor->eventUpdate();
	}
}

// Returning true from here indicated we are done.
UBOOL USeqAct_Interp::UpdateOp(FLOAT deltaTime)
{
	// First check inputs, to see if there is an input that might change direction etc.

	// NOTE: We check for Pause events first, so that a playing sequence can be paused in the same
	//		 stack frame that it started playing in.
	if( bIsPlaying && InputLinks(3).bHasImpulse )
	{
		Pause();
		NotifyActorsOfChange();
	}
	else if( InputLinks(0).bHasImpulse )
	{
		Play();
		NotifyActorsOfChange();
	}
	else if( InputLinks(1).bHasImpulse )
	{
		Reverse();
		NotifyActorsOfChange();
	}
	else if( InputLinks(2).bHasImpulse )
	{
		Stop();
	}
	else if (InputLinks(4).bHasImpulse)
	{
		ChangeDirection();
		NotifyActorsOfChange();
	}
	else if( !bIsPlaying )
	{
		// If we have stopped playing - return 'true' to indicate so.
		return true;
	}


	// Clear all the inputs now.
	InputLinks(0).bHasImpulse = 0;
	InputLinks(1).bHasImpulse = 0;
	InputLinks(2).bHasImpulse = 0;
	InputLinks(3).bHasImpulse = 0;
	InputLinks(4).bHasImpulse = 0;

	// Update the current position and do any interpolation work.
	StepInterp(deltaTime, false);

	// This is a latent function. To indicate we are still doing something - return false.
	return false;

}

void USeqAct_Interp::DeActivated()
{
	// Never fire any outputs if no Matinee Data attached.
	if(InterpData)
	{
		// If we are at the start, fire off the 'Aborted' output.
		if(Position < KINDA_SMALL_NUMBER)
		{
			if( !OutputLinks(1).bDisabled && !(OutputLinks(1).bDisabled && GIsEditor))
			{
				OutputLinks(1).bHasImpulse = true;
			}
		}
		// If we reached the end, fire off the 'Complete' output.
		else if(Position > (InterpData->InterpLength - KINDA_SMALL_NUMBER))
		{
			if( !OutputLinks(0).bDisabled && !(OutputLinks(0).bDisabled && GIsEditor))
			{
				OutputLinks(0).bHasImpulse = true;
			}
		}
		// If we are in the middle (ie. stopped because the 'Stop' input was hit). Don't fire any output.
	}

	// Remove this latent action from all actors it was working on, and empty our list of actors.
	for(INT i=0; i<LatentActors.Num(); i++)
	{
		AActor* Actor = LatentActors(i);
		if(Actor && !Actor->IsPendingKill())
		{
			Actor->LatentActions.RemoveItem(this);
			Actor->eventInterpolationFinished(this);
		}
	}
	if (ReplicatedActor != NULL)
	{
		ReplicatedActor->eventUpdate();
	}

	LatentActors.Empty();

	// Do any interpolation sequence cleanup-  destroy track/group instances etc.
	TermInterp();
}

void USeqAct_Interp::Play()
{
	// Jump to specific location if desired.
	if(bForceStartPos && !bIsPlaying)
	{
		UpdateInterp(ForceStartPosition, FALSE, TRUE);
	}
	// See if we should rewind to beginning...
	else if(bRewindOnPlay && (!bIsPlaying || bRewindIfAlreadyPlaying))
	{
		if(bNoResetOnRewind)
		{
			ResetMovementInitialTransforms();
		}

		// 'Jump' interpolation to the start (ie. will not fire events between current position and start).
		UpdateInterp(0.f, FALSE, TRUE);
	}

	bReversePlayback = false;
	bIsPlaying = true;
	bPaused = false;
}

void USeqAct_Interp::Stop()
{
	bIsPlaying = false;
	bPaused = false;
}

void USeqAct_Interp::Reverse()
{
	bReversePlayback = true;
	bIsPlaying = true;
	bPaused = false;
}

void USeqAct_Interp::Pause()
{
	// If we are playing - toggle the paused state.
	if(bIsPlaying)
	{
		bPaused = !bPaused;
	}
}

void USeqAct_Interp::ChangeDirection()
{
	bReversePlayback = !bReversePlayback;
	bIsPlaying = true;
	bPaused = false;
}

/**
 *	Advance current position in sequence and call UpdateInterp to update each group/track and the actor it is working on.
 *
 *	@param	DeltaSeconds	Amount to step sequence on by.
 *	@param	bPreview		If we are previewing sequence (ie. viewing in editor without gameplay running)
 */
void USeqAct_Interp::StepInterp(FLOAT DeltaSeconds, UBOOL bPreview)
{
	// Do nothing if not playing.
	if(!bIsPlaying || bPaused || !InterpData)
		return;

	// do nothing if client side only and no affected Actors are recently visible
	UBOOL bSkipUpdate = FALSE;
	if (bClientSideOnly && bSkipUpdateIfNotVisible)
	{
		bSkipUpdate = TRUE;
		TArray<UObject**> ObjectVars;
		GetObjectVars(ObjectVars);
		for (INT i = 0; i < ObjectVars.Num() && bSkipUpdate; i++)
		{
			if( ObjectVars( i ) != NULL )
			{
				AActor* Actor = Cast<AActor>(*(ObjectVars(i)));
				if (Actor != NULL && Actor->LastRenderTime > Actor->WorldInfo->TimeSeconds - 1.f)
				{
					bSkipUpdate = FALSE;
				}
			}
		}
	}

	if (!bSkipUpdate)
	{
		FLOAT NewPosition;
		UBOOL bLooped = 0;

		// Playing forwards
		if(!bReversePlayback)
		{
			NewPosition = Position + (DeltaSeconds * PlayRate);

			if(NewPosition > InterpData->InterpLength)
			{
				// If looping, play to end, jump to start, and set target to somewhere near the beginning.
				if(bLooping)
				{
					UpdateInterp(InterpData->InterpLength, bPreview);

					if(bNoResetOnRewind)
					{
						ResetMovementInitialTransforms();
					}

					UpdateInterp(0.f, bPreview, true);

					while(NewPosition > InterpData->InterpLength)
					{
						NewPosition -= InterpData->InterpLength;
					}

					bLooped = true;
				}
				// If not looping, snap to end and stop playing.
				else
				{
					NewPosition = InterpData->InterpLength;
					bIsPlaying = false;
				}
			}
		}
		// Playing backwards.
		else
		{
			NewPosition = Position - (DeltaSeconds * PlayRate);

			if(NewPosition < 0.f)
			{
				// If looping, play to start, jump to end, and set target to somewhere near the end.
				if(bLooping)
				{
					UpdateInterp(0.f, bPreview);
					UpdateInterp(InterpData->InterpLength, bPreview, true);

					while(NewPosition < 0.f)
					{
						NewPosition += InterpData->InterpLength;
					}

					bLooped = true;
				}
				// If not looping, snap to start and stop playing.
				else
				{
					NewPosition = 0.f;
					bIsPlaying = false;
				}
			}
		}

		UpdateInterp(NewPosition, bPreview);

		UpdateStreamingForCameraCuts(NewPosition, bPreview);

		
		if (ReplicatedActor != NULL)
		{
			// if we looped back to the start, notify the replicated actor so it can refresh any clients
			if (bLooped)
			{
				ReplicatedActor->eventUpdate();
			}
			else
			{
				// otherwise, just update position without notifying it
				// so that clients that join the game during movement will get the correct updated position
				// but nothing will get replicated to other clients that should be simulating the movement
				ReplicatedActor->Position = NewPosition;
			}
		}
	}
}

/** Number of seconds to look ahead for camera cuts (for notifying the streaming system). */
FLOAT GCameraCutLookAhead = 10.0f;

/**
 *	Updates the streaming system with the camera locations for the upcoming camera cuts, so
 *	that it can start streaming in textures for those locations now.
 *
 *	@param	CurrentTime		Current time within the matinee, in seconds
 *	@param	bPreview		If we are previewing sequence (ie. viewing in editor without gameplay running)
 */
void USeqAct_Interp::UpdateStreamingForCameraCuts(FLOAT CurrentTime, UBOOL bPreview/*=FALSE*/)
{
	// Only supports forward-playing non-looping matinees.
	if ( GIsGame && bIsPlaying && !bReversePlayback && !bLooping )
	{
		for ( INT CameraCutIndex=0; CameraCutIndex < CameraCuts.Num(); CameraCutIndex++ )
		{
			FCameraCutInfo& CutInfo = CameraCuts(CameraCutIndex);
			FLOAT TimeDifference = CutInfo.TimeStamp - CurrentTime;
			if ( TimeDifference > 0.0f && TimeDifference < GCameraCutLookAhead )
			{
				GStreamingManager->AddViewSlaveLocation( CutInfo.Location );
			}
			else if ( TimeDifference >= GCameraCutLookAhead )
			{
				break;
			}
		}
	}
}

/**
 * Checks to see if this Matinee should be associated with the specified player.  This is a relatively
 * quick test to perform.
 *
 * @param InPC The player controller to check
 *
 * @return TRUE if this Matinee sequence is compatible with the specified player
 */
UBOOL USeqAct_Interp::IsMatineeCompatibleWithPlayer( APlayerController* InPC ) const
{
	UBOOL bBindPlayerToMatinee = TRUE;

	// If the 'preferred split screen' value is non-zero, we'll only bind this Matinee to
	// player controllers that are associated with the specified game player index
	if( PreferredSplitScreenNum != 0 )
	{
		bBindPlayerToMatinee = FALSE;
		ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>( InPC->Player );
		if( LocalPlayer != NULL )
		{
			const INT GamePlayerIndex = GEngine->GamePlayers.FindItemIndex( LocalPlayer );
			if( ( GamePlayerIndex + 1 ) == PreferredSplitScreenNum )
			{
				bBindPlayerToMatinee = TRUE;
			}
		}
	}

	return bBindPlayerToMatinee;
}


/** Finds and returns the Director group, or NULL if not found. */
UInterpGroupDirector* USeqAct_Interp::FindDirectorGroup()
{
	if(InterpData)
	{
		for(INT i=0; i<InterpData->InterpGroups.Num(); i++)
		{
			UInterpGroup* Group = InterpData->InterpGroups(i);
			UInterpGroupDirector* DirGroup = Cast<UInterpGroupDirector>(Group);
			if (DirGroup)
			{
				return DirGroup;
			}
		}
	}
	return NULL;
}



// For each Actor connected to a particular group, create an instance.
void USeqAct_Interp::InitInterp()
{
	// Get the InterpData that this SeqAct_Interp is linked to.
	InterpData = FindInterpDataFromVariable();

	// Only continue if we successfully found some data.
	if(InterpData)
	{
		// Cache whether or not we want to enable extreme content within this sequence
		bShouldShowGore = TRUE;
		if( GWorld != NULL && GWorld->GetWorldInfo() != NULL )
		{
			AGameReplicationInfo* GRI = GWorld->GetWorldInfo()->GRI;
			if( GRI != NULL )
			{
				bShouldShowGore = GRI->eventShouldShowGore();
			}
		}

		for(INT i=0; i<InterpData->InterpGroups.Num(); i++)
		{
			UInterpGroup* Group = InterpData->InterpGroups(i);

			// If this is a DirectorGroup, we find a player controller and pass it in instead of looking to a variable.
			UInterpGroupDirector* DirGroup = Cast<UInterpGroupDirector>(Group);
			if(DirGroup)
			{
				// Need to do a game specific check here because there are no player controllers in the editor and matinee expects a group instance to be initialized.
				if(GIsGame)
				{
					UBOOL bCreatedGroup = FALSE;

					// iterate through the controller list
					for (AController *Controller = GWorld->GetFirstController(); Controller != NULL; Controller = Controller->NextController)
					{
						APlayerController *PC = Cast<APlayerController>(Controller);
						
						// If it's a player and this sequence is compatible with the player...
						if (PC != NULL && IsMatineeCompatibleWithPlayer( PC ) )
						{
							// create a new instance with this player
							UInterpGroupInstDirector* NewGroupInstDir = ConstructObject<UInterpGroupInstDirector>(UInterpGroupInstDirector::StaticClass(), this, NAME_None, RF_Transactional);
							GroupInst.AddItem(NewGroupInstDir);

							// and initialize the instance
							NewGroupInstDir->InitGroupInst(DirGroup, PC);
							bCreatedGroup = TRUE;
						}
					}

					// Sanity check to make sure that there is an instance for all groups.
					if(bCreatedGroup == FALSE)
					{
						UInterpGroupInstDirector* NewGroupInstDir = ConstructObject<UInterpGroupInstDirector>(UInterpGroupInstDirector::StaticClass(), this, NAME_None, RF_Transactional);
						GroupInst.AddItem(NewGroupInstDir);
						// and initialize the instance
						NewGroupInstDir->InitGroupInst(DirGroup, NULL);
					}
				}
				else
				{
					// In the editor always create a director group instance with a NULL group actor since there are no player controllers.
					UInterpGroupInstDirector* NewGroupInstDir = ConstructObject<UInterpGroupInstDirector>(UInterpGroupInstDirector::StaticClass(), this, NAME_None, RF_Transactional);
					GroupInst.AddItem(NewGroupInstDir);

					// and initialize the instance
					NewGroupInstDir->InitGroupInst(DirGroup, NULL);
				}
			}
			else
			{
				// Folder groups don't get variables
				if( !Group->bIsFolder )
				{
					TArray<UObject**> ObjectVars;
					GetObjectVars(ObjectVars, *Group->GroupName.ToString());

					// We create at least one instance for every group - even if no Actor is attached.
					// Allows for tracks that dont require an Actor (eg. Event tracks).

					for(INT j=0; j<ObjectVars.Num() || j==0; j++)
					{
						AActor* Actor = NULL;
						if( j < ObjectVars.Num() )
						{
							if( ObjectVars( j ) != NULL )
							{
								UObject *Obj = *(ObjectVars(j));
								Actor = Cast<AActor>(Obj);

								// See if there is already a group working on this actor.
								UInterpGroupInst* TestInst = FindGroupInst(Actor);
								if(TestInst)
								{
									debugf( NAME_Warning, TEXT("Skipping instancing group - an Actor may only be connected to one Group! [%s]"), *Actor->GetName()  );
									//Actor = NULL;
								}
							}
						}

						UInterpGroupInst* NewGroupInst = ConstructObject<UInterpGroupInst>(UInterpGroupInst::StaticClass(), this, NAME_None, RF_Transactional);
						GroupInst.AddItem(NewGroupInst);

						NewGroupInst->InitGroupInst(Group, Actor);
					}
				}
			}
		}
	}

	/// Scan the matinee for camera cuts and sets up the CameraCut array.
	SetupCameraCuts();
}

/** Scans the matinee for camera cuts and sets up the CameraCut array. */
void USeqAct_Interp::SetupCameraCuts()
{
	UInterpGroupDirector* DirGroup = FindDirectorGroup();
	UInterpTrackDirector* DirTrack = DirGroup ? DirGroup->GetDirectorTrack() : NULL;
	if ( InterpData && DirTrack && DirTrack->CutTrack.Num() > 0 )
	{
		CameraCuts.Reserve( DirTrack->CutTrack.Num() );

		// Find the starting camera location for each cut.
		for( INT KeyFrameIndex=0; KeyFrameIndex < DirTrack->CutTrack.Num(); KeyFrameIndex++)
		{
			const FDirectorTrackCut& Cut = DirTrack->CutTrack(KeyFrameIndex);
			INT GroupIndex = InterpData->FindGroupByName( Cut.TargetCamGroup );
			if ( GroupIndex != INDEX_NONE )
			{
				// Find a valid move track for this cut.
				UInterpGroup* Group = InterpData->InterpGroups(GroupIndex);
				for(INT TrackIndex=0; TrackIndex < Group->InterpTracks.Num(); TrackIndex++)
				{
					UInterpTrackMove* MoveTrack = Cast<UInterpTrackMove>(Group->InterpTracks(TrackIndex));
					if ( MoveTrack )
					{
						FCameraCutInfo CameraCut;
						CameraCut.Location = MoveTrack->EvalPositionAtTime( NULL, Cut.Time );

						// The first keyframe could be (0,0,0), try again slightly past it in time.
						if ( CameraCut.Location.IsNearlyZero() == TRUE )
						{
							CameraCut.Location = MoveTrack->EvalPositionAtTime( NULL, Cut.Time+0.01f );
						}

						// Only add locations that aren't (0,0,0)
						if ( CameraCut.Location.IsNearlyZero() == FALSE )
						{
							CameraCut.TimeStamp = Cut.Time;
							CameraCuts.AddItem( CameraCut );
						}
						break;
					}
				}
			}
		}
	}
}

void USeqAct_Interp::TermInterp()
{
	// Destroy each group instance.
	for(INT i=0; i<GroupInst.Num(); i++)
	{
		GroupInst(i)->TermGroupInst(true);
	}
	GroupInst.Empty();

	// Drop reference to interpolation data.
	InterpData = NULL;

	// remember last time this happened in game
	if (GIsGame && GWorld != NULL)
	{
		TerminationTime = GWorld->GetWorldInfo()->TimeSeconds;
	}
}

/** adds the passed in PlayerController to all running Director tracks so that its camera is controlled
 * all PCs that are available at playback start time are hooked up automatically, but this needs to be called to hook up
 * any that are created during playback (player joining a network game during a cinematic, for example)
 * @param PC the PlayerController to add
 */
void USeqAct_Interp::AddPlayerToDirectorTracks(APlayerController* PC)
{
	// if we aren't initialized (i.e. not currently running) then do nothing
	if (PC != NULL && InterpData != NULL && GroupInst.Num() > 0 && GIsGame)
	{
		for (INT i = 0; i < InterpData->InterpGroups.Num(); i++)
		{
			UInterpGroupDirector* DirGroup = Cast<UInterpGroupDirector>(InterpData->InterpGroups(i));
			if (DirGroup != NULL)
			{
				UBOOL bAlreadyHasGroup = FALSE;
				for (INT j = 0; j < GroupInst.Num(); j++)
				{
					if (GroupInst(j)->Group == DirGroup && GroupInst(j)->GroupActor == PC)
					{
						bAlreadyHasGroup = TRUE;
						break;
					}
				}
				if (!bAlreadyHasGroup)
				{
					// Make sure this sequence is compatible with the player
					if( IsMatineeCompatibleWithPlayer( PC ) )
					{
						// create a new instance with this player
						UInterpGroupInstDirector* NewGroupInstDir = ConstructObject<UInterpGroupInstDirector>(UInterpGroupInstDirector::StaticClass(), this, NAME_None, RF_Transactional);
						GroupInst.AddItem(NewGroupInstDir);

						// and initialize the instance
						NewGroupInstDir->InitGroupInst(DirGroup, PC);
					}
				}
			}
		}
	}
}

void USeqAct_Interp::UpdateInterp(FLOAT NewPosition, UBOOL bPreview, UBOOL bJump)
{
	if(!InterpData)
	{
		return;
	}

	NewPosition = Clamp(NewPosition, 0.f, InterpData->InterpLength);

	for(INT i=0; i<GroupInst.Num(); i++)
	{
		UInterpGroupInst* GrInst = GroupInst(i);

		check(GrInst->Group);
		GrInst->Group->UpdateGroup(NewPosition, GrInst, bPreview, bJump);
	}

	// check for any attached cover links that should be updated
	if (bInterpForPathBuilding &&
		Position <= InterpData->PathBuildTime &&
		NewPosition > InterpData->PathBuildTime)
	{
		for (INT Idx = 0; Idx < LinkedCover.Num(); Idx++)
		{
			ACoverLink *Link = LinkedCover(Idx);
			if (Link->IsEnabled())
			{
				for (INT SlotIdx = 0; SlotIdx < Link->Slots.Num(); SlotIdx++)
				{
					if (Link->Slots(SlotIdx).bEnabled)
					{
						Link->AutoAdjustSlot(SlotIdx,TRUE);
					}
				}
			}
		}
	}

	Position = NewPosition;
}

/** Reset any movement tracks so their 'initial position' is the current position. */
void USeqAct_Interp::ResetMovementInitialTransforms()
{
	if(!InterpData)
	{
		return;
	}

	for(INT i=0; i<GroupInst.Num(); i++)
	{
		UInterpGroupInst* GrInst = GroupInst(i);

		check(GrInst->Group);
		check(GrInst->Group->InterpTracks.Num() == GrInst->TrackInst.Num());

		for(INT j=0; j<GrInst->TrackInst.Num(); j++)
		{
			UInterpTrackInst* TrInst = GrInst->TrackInst(j);
			UInterpTrackInstMove* MoveInst = Cast<UInterpTrackInstMove>(TrInst);
			if(MoveInst)
			{
				MoveInst->CalcInitialTransform( GrInst->Group->InterpTracks(j), true );
			}
		}
	}
}

UInterpGroupInst* USeqAct_Interp::FindGroupInst(AActor* Actor)
{
	if(!Actor || Actor->bDeleteMe)
	{
		return NULL;
	}

	for(INT i=0; i<GroupInst.Num(); i++)
	{
		if( GroupInst(i)->GetGroupActor() == Actor )
		{
			return GroupInst(i);
		}
	}

	return NULL;
}

UInterpGroupInst* USeqAct_Interp::FindFirstGroupInst(UInterpGroup* InGroup)
{
	if(!InGroup)
	{
		return NULL;
	}

	for(INT i=0; i<GroupInst.Num(); i++)
	{
		if( GroupInst(i)->Group == InGroup )
		{
			return GroupInst(i);
		}
	}

	return NULL;
}

UInterpGroupInst* USeqAct_Interp::FindFirstGroupInstByName(FName InGroupName)
{
	if(InGroupName == NAME_None)
	{
		return NULL;
	}

	for(INT i=0; i<GroupInst.Num(); i++)
	{
		if( GroupInst(i)->Group->GroupName == InGroupName )
		{
			return GroupInst(i);
		}
	}

	return NULL;
}

/** Find the first group instance based on the InterpGroup with the given name. */
UInterpGroupInst* USeqAct_Interp::FindFirstGroupInstByName( const FString& InGroupName )
{
	for(INT i=0; i<GroupInst.Num(); i++)
	{
		if( GroupInst(i)->Group->GroupName.ToString() == InGroupName )
		{
			return GroupInst(i);
		}
	}

	return NULL;
}


/** If we have a DirectorGroup, use it to find the viewed group at the give time, then the first instance of that group, and the Actor it is bound to. */
AActor* USeqAct_Interp::FindViewedActor()
{
	UInterpGroupDirector* DirGroup = InterpData->FindDirectorGroup();
	if(DirGroup)
	{
		UInterpTrackDirector* DirTrack = DirGroup->GetDirectorTrack();
		if(DirTrack)
		{
			FLOAT CutTime, CutTransitionTime;
			FName ViewGroupName = DirTrack->GetViewedGroupName(Position, CutTime, CutTransitionTime);
			UInterpGroupInst* ViewGroupInst = FindFirstGroupInstByName(ViewGroupName);
			if(ViewGroupInst)
			{
				return ViewGroupInst->GetGroupActor();
			}
		}
	}

	return NULL;
}

/** 
 *	Utility for getting all Actors currently being worked on by this Matinee action. 
 *	If bMovementTrackOnly is set, Actors must have a Movement track in their group to be included in the results.
 */
void USeqAct_Interp::GetAffectedActors(TArray<AActor*>& OutActors, UBOOL bMovementTrackOnly)
{
	for(INT i=0; i<GroupInst.Num(); i++)
	{
		if(GroupInst(i)->GroupActor)
		{
			UInterpGroup* Group = GroupInst(i)->Group;
			TArray<UInterpTrack*> MovementTracks;
			Group->FindTracksByClass(UInterpTrackMove::StaticClass(), MovementTracks);

			// If we either dont just want movement tracks, or we do and we have a movement track, add to array.
			if(!bMovementTrackOnly || MovementTracks.Num() > 0)
			{	
				OutActors.AddUniqueItem( GroupInst(i)->GroupActor );
			}
		}
	}
}

void USeqAct_Interp::SetPosition(FLOAT NewPosition, UBOOL bJump)
{
	// if we aren't currently active, temporarily activate to change the position
	UBOOL bTempActivate = !bActive;
	if (bTempActivate)
	{
		InitInterp();
	}

	UpdateInterp(NewPosition, FALSE, bJump);
	// update interpolating actors for the new position
	TArray<UObject**> ObjectVars;
	GetObjectVars(ObjectVars);
	for (INT i = 0; i < ObjectVars.Num(); i++)
	{
		if( ObjectVars( i ) != NULL )
		{
			AActor* Actor = Cast<AActor>(*(ObjectVars(i)));
			if (Actor != NULL && !Actor->bDeleteMe && Actor->Physics == PHYS_Interpolating)
			{
				// temporarily add ourselves to the Actor's LatentActions list so it can find us
				const INT Index = Actor->LatentActions.AddItem(this);
				Actor->physInterpolating(Actor->WorldInfo->DeltaSeconds);
				Actor->LatentActions.Remove(Index);
			}
		}
	}

	if (bTempActivate)
	{
		TermInterp();
	}

	if (ReplicatedActor != NULL)
	{
		ReplicatedActor->eventUpdate();
	}
}



/**
 * Conditionally saves state for the specified actor and its children
 */
void USeqAct_Interp::ConditionallySaveActorState( UInterpGroupInst* GroupInst, AActor* Actor )
{
	UBOOL bShouldCaptureTransforms = FALSE;
	UBOOL bShouldCaptureChildTransforms = FALSE;
	UBOOL bShouldCaptureVisibility = FALSE;

	// Iterate over all of this group's tracks
	for( INT TrackIdx = 0; TrackIdx < GroupInst->Group->InterpTracks.Num(); ++TrackIdx )
	{
		const UInterpTrack* CurTrack = GroupInst->Group->InterpTracks( TrackIdx );

		// Is this is a 'movement' track?  If so, then we'll consider it worthy of our test
		if( CurTrack->IsA( UInterpTrackMove::StaticClass() ) )
		{
			bShouldCaptureTransforms = TRUE;
			bShouldCaptureChildTransforms = TRUE;
		}

		// Is this an 'anim control' track?  If so, we'll need to capture the object's transforms along
		// with all of it's attached objects.  As the object animates, any attached objects will wander
		// so we'll need to make sure to use this to restore their transform later.
		if( CurTrack->IsA( UInterpTrackAnimControl::StaticClass() ) )
		{
			// @todo: Consider not capturing parent actor transforms for anim control tracks
			bShouldCaptureTransforms = TRUE;
			bShouldCaptureChildTransforms = TRUE;
		}

		// Is this a 'visibility' track?  If so, we'll save the actor's original bHidden state
		if( CurTrack->IsA( UInterpTrackVisibility::StaticClass() ) )
		{
			bShouldCaptureVisibility = TRUE;
		}
	}

	if( bShouldCaptureTransforms || bShouldCaptureChildTransforms )
	{
		// Save off the transforms of the actor and anything directly or indirectly attached to it.
		const UBOOL bOnlyCaptureChildren = !bShouldCaptureTransforms;
		SaveActorTransforms( Actor, bOnlyCaptureChildren );
	}

	if( bShouldCaptureVisibility )
	{
		// Save visibility state
		SaveActorVisibility( Actor );
	}
}



/**
 * Adds the specified actor and any actors attached to it to the list
 * of saved actor transforms.  Does nothing if an actor has already
 * been saved.
 */
void USeqAct_Interp::SaveActorTransforms( AActor* Actor, UBOOL bOnlyChildren )
{
	check( GIsEditor );

	if( Actor != NULL )
	{
		if( bOnlyChildren )
		{
			// Only save transforms for child actors
			for( INT AttachedActorIndex = 0 ; AttachedActorIndex < Actor->Attached.Num() ; ++AttachedActorIndex )
			{
				AActor* Other = Actor->Attached(AttachedActorIndex);
				SaveActorTransforms( Other, TRUE );
			}
		}
		else if ( !Actor->bDeleteMe )
		{
			// Save transforms for the parent actor and all of its children
			const FSavedTransform* SavedTransform = SavedActorTransforms.Find( Actor );
			if ( !SavedTransform )
			{
				FSavedTransform NewSavedTransform;
				NewSavedTransform.Location = Actor->Location;
				NewSavedTransform.Rotation = Actor->Rotation;

				SavedActorTransforms.Set( Actor, NewSavedTransform );

				for( INT AttachedActorIndex = 0 ; AttachedActorIndex < Actor->Attached.Num() ; ++AttachedActorIndex )
				{
					AActor* Other = Actor->Attached(AttachedActorIndex);
					SaveActorTransforms( Other, FALSE );
				}
			}
		}
	}
}

/**
 * Applies the saved locations and rotations to all saved actors.
 */
void USeqAct_Interp::RestoreActorTransforms()
{
	check( GIsEditor );

	for( TMap<AActor*, FSavedTransform>::TIterator It(SavedActorTransforms) ; It ; ++It )
	{
		AActor* SavedActor = It.Key();
		FSavedTransform& SavedTransform = It.Value();

		// only update actor position/rotation if the track changed its position/rotation
		if( !SavedActor->Location.Equals(SavedTransform.Location) || 
			!(SavedActor->Rotation == SavedTransform.Rotation) )
		{
			SavedActor->Location = SavedTransform.Location;
			SavedActor->Rotation = SavedTransform.Rotation;
			SavedActor->ConditionalForceUpdateComponents();
			SavedActor->PostEditMove( TRUE );
		}
	}
	SavedActorTransforms.Empty();
}


/** Saves whether or not this actor is hidden so we can restore it later */
void USeqAct_Interp::SaveActorVisibility( AActor* Actor )
{
	check( GIsEditor );

	if( Actor != NULL )
	{
		if ( !Actor->bDeleteMe )
		{
			const BYTE* SavedVisibility = SavedActorVisibilities.Find( Actor );
			if ( !SavedVisibility )
			{
				SavedActorVisibilities.Set( Actor, ( BYTE )Actor->bHidden );
			}
		}
	}
}



/** Applies the saved visibility state for all saved actors */
void USeqAct_Interp::RestoreActorVisibilities()
{
	check( GIsEditor );

	for( TMap<AActor*, BYTE>::TIterator It(SavedActorVisibilities) ; It ; ++It )
	{
		AActor* SavedActor = It.Key();
		UBOOL bSavedHidden = ( UBOOL )It.Value();

		// only update actor if something has actually changed
		if( SavedActor->bHidden != bSavedHidden )
		{
			SavedActor->SetHidden( bSavedHidden );
			SavedActor->ConditionalForceUpdateComponents();
			SavedActor->PostEditMove( TRUE );
		}
	}
	SavedActorVisibilities.Empty();
}


/**
 * Stores the current scrub position, restores all saved actor transforms,
 * then saves off the transforms for actors referenced (directly or indirectly)
 * by group instances, and finally restores the scrub position.
 */
void USeqAct_Interp::RecaptureActorState()
{
	check( GIsEditor );

	// We now need to remove from the saved actor transformation state any actors
	// that belonged to the removed group instances, along with actors rooted to
	// those group actors.  However, another group could be affecting an actor which
	// is an ancestor of the removed actor(S).  So, we store the current scrub position,
	// restore all actor transforms (including the ones assigned to the groups that were
	// removed), then save off the transforms for actors referenced (directly or indirectly)
	// by group instances, then restore the scrub position.

	const FLOAT SavedScrubPosition = Position;

	RestoreActorVisibilities();
	RestoreActorTransforms();
	for(INT i=0; i<GroupInst.Num(); i++)
	{
		UInterpGroupInst* GrInst = GroupInst(i);
		AActor* GroupActor = GrInst->GetGroupActor();
		if( GroupActor )
		{
			ConditionallySaveActorState( GrInst, GroupActor );
		}
	}
	UpdateInterp( SavedScrubPosition, TRUE );
}

/**
 * Serialize function.
 *
 * @param	Ar		The archive to serialize with.
 */
void USeqAct_Interp::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	// @fixme: Why are we serializing these our to disk?
	Ar << SavedActorTransforms;

	if( Ar.IsObjectReferenceCollector() )
	{
		Ar << SavedActorVisibilities;
	}
}

/**
 * Activates the output for the named event.
 */
void USeqAct_Interp::NotifyEventTriggered(class UInterpTrackEvent const* EventTrack, INT EventIdx)
{
	if ( EventTrack && (EventIdx >= 0) && (EventIdx < EventTrack->EventTrack.Num()) )
	{
		FName const EventName = EventTrack->EventTrack(EventIdx).EventName;

		// Find output with give name 
		INT OutputIndex = FindConnectorIndex( EventName.ToString(), LOC_OUTPUT );
		if (OutputIndex != INDEX_NONE)
		{
			if( !OutputLinks(OutputIndex).bDisabled &&
				!(OutputLinks(OutputIndex).bDisabledPIE && GIsEditor) )
			{
				ActivateOutputLink(OutputIndex);
			}
		}
	}
}

/*-----------------------------------------------------------------------------
 UInterpGroup
-----------------------------------------------------------------------------*/

void UInterpGroup::PostLoad()
{
	Super::PostLoad();

	// Remove any NULLs in the InterpTracks array.
	INT TrackIndex = 0;
	while(TrackIndex < InterpTracks.Num())
	{
		if(InterpTracks(TrackIndex))
		{
			TrackIndex++;
		}
		else
		{
			InterpTracks.Remove(TrackIndex);
		}
	}

	// Now we have moved the AnimSets array into InterpGroup, we need to fix up old content.
	for(INT i=0; i<InterpTracks.Num(); i++)
	{
		UInterpTrackAnimControl* AnimTrack = Cast<UInterpTrackAnimControl>(InterpTracks(i));
		if(AnimTrack)
		{
			// Copy contents from that AnimTrack into GroupAnimSets..
			for(INT j=0; j<AnimTrack->AnimSets.Num(); j++)
			{
				GroupAnimSets.AddUniqueItem( AnimTrack->AnimSets(j) );
			}

			// ..and empty it
			AnimTrack->AnimSets.Empty();
		}
	}
}

/** Iterate over all InterpTracks in this InterpGroup, doing any actions to bring the state to the specified time. */
void UInterpGroup::UpdateGroup(FLOAT NewPosition, UInterpGroupInst* GrInst, UBOOL bPreview, UBOOL bJump)
{
	check( InterpTracks.Num() == GrInst->TrackInst.Num() );

	// We'll make sure to always update FaceFX tracks last, if we have any, since those will modify the skeletal
	// pose of the actor which can easily be blown away by VectorProperty tracks (DrawScale3D, etc) which force
	// an update of the actor's entire component set.
	for( INT CurUpdatePass = 1; CurUpdatePass <= 2; ++CurUpdatePass )
	{
		UBOOL bAnyFaceFXTracks = FALSE;
		for(INT i=0; i<InterpTracks.Num(); i++)
		{
			UInterpTrack* Track = InterpTracks(i);
			UInterpTrackInst* TrInst = GrInst->TrackInst(i);

			// Check to see if this is a FaceFX track; we'll always process those later.
			UInterpTrackFaceFX* FaceFXTrack = Cast< UInterpTrackFaceFX >( Track );
			if( FaceFXTrack != NULL )
			{
				bAnyFaceFXTracks = TRUE;
			}

			// Process FaceFX tracks on the second pass, all other tracks on the first pass
			if( ( CurUpdatePass == 2 ) == ( FaceFXTrack != NULL ) )
			{
				if(bPreview)
				{
					Track->ConditionalPreviewUpdateTrack(NewPosition, TrInst);
				}
				else
				{
					Track->ConditionalUpdateTrack(NewPosition, TrInst, bJump);
				}
			}
		}

		if( !bAnyFaceFXTracks )
		{
			// No FaceFX tracks found during first pass, so no need for more iteration!
			break;
		}
	}

	// Update animation state of Actor.
	UpdateAnimWeights(NewPosition, GrInst, bPreview, bJump);

	// Update stuff attached to bones (if group has anim control).
	if( bPreview && HasAnimControlTrack() )
	{
		GrInst->UpdateAttachedActors();
	}
}

/** Utility function for adding a weight entry to a slot with the given name. Creates a new entry in the array if there is not already one present. */
static void AddSlotInfo(TArray<FAnimSlotInfo>& SlotInfos, FName SlotName, FLOAT InChannelWeight)
{
	// Look for an existing entry with this name.
	for(INT i=0; i<SlotInfos.Num(); i++)
	{
		// If we find one, add weight to array and we are done.
		if(SlotInfos(i).SlotName == SlotName)
		{
			SlotInfos(i).ChannelWeights.AddItem(InChannelWeight);
			return;
		}
	}

	// If we didn't find one, add a new entry to the array now.
	const INT NewIndex = SlotInfos.AddZeroed();
	SlotInfos(NewIndex).SlotName = SlotName;
	SlotInfos(NewIndex).ChannelWeights.AddItem(InChannelWeight);
}

/** Returns whether this Group contains at least one AnimControl track. */
UBOOL UInterpGroup::HasAnimControlTrack() const
{
	UBOOL bHasAnimTrack = FALSE;
	for(INT i=0; i<InterpTracks.Num(); i++)
	{
		if( InterpTracks(i)->bIsAnimControlTrack )
		{
			bHasAnimTrack = TRUE;
		}
	}

	return bHasAnimTrack;
}

/** Returns whether this Group contains a movement track. */
UBOOL UInterpGroup::HasMoveTrack() const
{
	UBOOL bHasMoveTrack = FALSE;
	for(INT i=0; i<InterpTracks.Num(); i++)
	{
		if( InterpTracks(i)->IsA(UInterpTrackMove::StaticClass()) )
		{
			bHasMoveTrack = TRUE;
			break;
		}
	}

	return bHasMoveTrack;
}

/**
 * We keep this around here as otherwise we are constantly allocating/deallocating the FAnimSlotInfos that Matinee is using
 * NOTE:  We probably need to clear this out every N calls
 **/
namespace
{
	TArray<FAnimSlotInfo> UpdateAnimWeightsSlotInfos;
}

/** Iterate over AnimControl tracks in this Group, build the anim blend info structures, and pass to the Actor via (Preview)SetAnimWeights. */
void UInterpGroup::UpdateAnimWeights(FLOAT NewPosition, class UInterpGroupInst* GrInst, UBOOL bPreview, UBOOL bJump)
{
	// Get the Actor this group is working on.
	AActor* Actor = GrInst->GetGroupActor();
	if(!Actor)
	{
		return;
	}

	// Now iterate over tracks looking for AnimControl ones.
	for(INT i=0; i<InterpTracks.Num(); i++)
	{
		UInterpTrack* Track = InterpTracks(i);
		check(Track);

		UInterpTrackAnimControl* AnimTrack = Cast<UInterpTrackAnimControl>(Track);
		if(AnimTrack)
		{
			// Add entry for this track to the SlotInfos array.
			const FLOAT TrackWeight = AnimTrack->GetWeightForTime(NewPosition);
			AddSlotInfo(UpdateAnimWeightsSlotInfos, AnimTrack->SlotName, TrackWeight);
		}
	}

	// Finally, pass the array to the Actor. Does different things depending on whether we are in Matinee or not.
	if(bPreview)
	{
		Actor->PreviewSetAnimWeights(UpdateAnimWeightsSlotInfos);
	}
	else
	{
		Actor->SetAnimWeights(UpdateAnimWeightsSlotInfos);
	}

	UpdateAnimWeightsSlotInfos.Reset();
}

/** Ensure this group name is unique within this InterpData (its Outer). */
void UInterpGroup::EnsureUniqueName()
{
	UInterpData* IData = CastChecked<UInterpData>( GetOuter() );

	FName NameBase = GroupName;
	INT Suffix = 0;

	// Test all other groups apart from this one to see if name is already in use
	UBOOL bNameInUse = false;
	for(INT i=0; i<IData->InterpGroups.Num(); i++)
	{
		if( (IData->InterpGroups(i) != this) && (IData->InterpGroups(i)->GroupName == GroupName) )
		{
			bNameInUse = true;
		}
	}

	// If so - keep appending numbers until we find a name that isn't!
	while( bNameInUse )
	{
		FString GroupNameString = FString::Printf(TEXT("%s%d"), *NameBase.ToString(), Suffix);
		GroupName = FName( *GroupNameString );
		Suffix++;

		bNameInUse = false;
		for(INT i=0; i<IData->InterpGroups.Num(); i++)
		{
			if( (IData->InterpGroups(i) != this) && (IData->InterpGroups(i)->GroupName == GroupName) )
			{
				bNameInUse = true;
			}
		}
	}
}

/** 
 *	Find all the tracks in this group of a specific class.
 *	Tracks are in the output array in the order they appear in the group.
 */
void UInterpGroup::FindTracksByClass(UClass* TrackClass, TArray<UInterpTrack*>& OutputTracks)
{
	for(INT i=0; i<InterpTracks.Num(); i++)
	{
		UInterpTrack* Track = InterpTracks(i);
		if( Track->IsA(TrackClass) )
		{
			OutputTracks.AddItem(Track);
		}
	}
}

/** Util for determining how many AnimControl tracks within this group are using the Slot with the supplied name. */
INT UInterpGroup::GetAnimTracksUsingSlot(FName InSlotName)
{
	INT NumTracks = 0;
	for(INT i=0; i<InterpTracks.Num(); i++)
	{
		UInterpTrackAnimControl* AnimTrack = Cast<UInterpTrackAnimControl>( InterpTracks(i) );
		if(AnimTrack && AnimTrack->SlotName == InSlotName)
		{
			NumTracks++;
		}
	}
	return NumTracks;
}


/*-----------------------------------------------------------------------------
	UInterpFilter
-----------------------------------------------------------------------------*/
IMPLEMENT_CLASS(UInterpFilter);

/** 
 * Given a interpdata object, updates visibility of groups and tracks based on the filter settings
 *
 * @param InData			Data to filter.
 */
void UInterpFilter::FilterData(class USeqAct_Interp* InData)
{
	// Mark our custom filtered groups as visible
	for( INT GroupIdx = 0; GroupIdx < InData->InterpData->InterpGroups.Num(); GroupIdx++)
	{
		UInterpGroup* CurGroup = InData->InterpData->InterpGroups( GroupIdx );
		CurGroup->bVisible = TRUE;

		for( INT CurTrackIndex = 0; CurTrackIndex < CurGroup->InterpTracks.Num(); ++CurTrackIndex )
		{
			UInterpTrack* CurTrack = CurGroup->InterpTracks( CurTrackIndex );
			CurTrack->bVisible = TRUE;
		}
	}
}

/*-----------------------------------------------------------------------------
	UInterpFilter_Classes
-----------------------------------------------------------------------------*/
IMPLEMENT_CLASS(UInterpFilter_Classes);

/** 
 * Given a interpdata object, updates visibility of groups and tracks based on the filter settings
 *
 * @param InData			Data to filter.
 */
void UInterpFilter_Classes::FilterData(USeqAct_Interp* InData)
{
	for(INT GroupIdx=0; GroupIdx<InData->InterpData->InterpGroups.Num(); GroupIdx++)
	{
		UInterpGroup* Group = InData->InterpData->InterpGroups(GroupIdx);
		UInterpGroupInst* GroupInst = InData->FindFirstGroupInst(Group);

		UBOOL bIncludeThisGroup = TRUE;

		// Folder groups may not have a group instance
		if( GroupInst != NULL )
		{
			// We avoid filtering out director groups (unless all of the group's tracks are filtered out below)
			if( !Group->IsA( UInterpGroupDirector::StaticClass() ) )
			{
				// If we were set to filter specific classes, then do that! (Otherwise, the group will always
				// be included)
				if( ClassToFilterBy != NULL )
				{
					AActor* Actor = GroupInst->GetGroupActor();
					if( Actor != NULL )
					{
						if( !Actor->IsA( ClassToFilterBy ) )
						{
							bIncludeThisGroup = FALSE;
						}
					}
					else
					{
						// No actor bound but we have an actor filter set, so don't include it
						bIncludeThisGroup = FALSE;
					}
				}
			}

			// If we were set to only include the group if it contains specific types of
			// tracks, then do that now
			if( TrackClasses.Num() > 0 )
			{
				UBOOL bHasAppropriateTrack = FALSE;

				for( INT CurTrackIndex = 0; CurTrackIndex < Group->InterpTracks.Num(); ++CurTrackIndex )
				{
					UInterpTrack* CurTrack = Group->InterpTracks( CurTrackIndex );
					if( CurTrack != NULL )
					{
						for( INT TrackClassIndex = 0; TrackClassIndex < TrackClasses.Num(); ++TrackClassIndex )
						{
							if( CurTrack->IsA( TrackClasses( TrackClassIndex ) ) )
							{
								// We found a track that matches the filter!
								bHasAppropriateTrack = TRUE;
								break;
							}
						}
					}
				}

				if( !bHasAppropriateTrack )
				{
					// Group doesn't contain any tracks that matches the desired filter
					bIncludeThisGroup = FALSE;
				}
			}
		}
		else
		{
			// No group inst (probably a folder), so don't include it
			bIncludeThisGroup = FALSE;
		}

		if( bIncludeThisGroup )
		{
			// Mark the group as visible!
			Group->bVisible = TRUE;

			for( INT CurTrackIndex = 0; CurTrackIndex < Group->InterpTracks.Num(); ++CurTrackIndex )
			{
				UInterpTrack* CurTrack = Group->InterpTracks( CurTrackIndex );
				if( CurTrack != NULL )
				{
					// If we need to, go through and constrain which track types are visible using our
					// list of filtered track classes
					if( TrackClasses.Num() > 0 )
					{
						for( INT TrackClassIndex = 0; TrackClassIndex < TrackClasses.Num(); ++TrackClassIndex )
						{
							if( CurTrack->IsA( TrackClasses( TrackClassIndex ) ) )
							{
								// We found a track that matches the filter!
								CurTrack->bVisible = TRUE;
							}
						}
					}
					else
					{
						// No track filter set, so make sure they're all visible
						CurTrack->bVisible = TRUE;
					}
				}
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	UInterpFilter_Custom
-----------------------------------------------------------------------------*/
IMPLEMENT_CLASS(UInterpFilter_Custom);

/** 
 * Given a interpdata object, updates visibility of groups and tracks based on the filter settings
 *
 * @param InData			Data to filter.
 */
void UInterpFilter_Custom::FilterData(USeqAct_Interp* InData)
{
	// Mark our custom filtered groups as visible
	for(INT GroupIdx=0; GroupIdx<GroupsToInclude.Num(); GroupIdx++)
	{
		UInterpGroup* CurGroup = GroupsToInclude( GroupIdx );
		CurGroup->bVisible = TRUE;

		for( INT CurTrackIndex = 0; CurTrackIndex < CurGroup->InterpTracks.Num(); ++CurTrackIndex )
		{
			UInterpTrack* CurTrack = CurGroup->InterpTracks( CurTrackIndex );
			CurTrack->bVisible = TRUE;
		}
	}
}


/*-----------------------------------------------------------------------------
 UInterpGroupInst
-----------------------------------------------------------------------------*/

/** 
 *	Returns the Actor that this GroupInstance is working on. 
 *	Should use this instead of just referencing GroupActor, as it check bDeleteMe for you.
 */
AActor* UInterpGroupInst::GetGroupActor()
{
	if(!GroupActor || GroupActor->bDeleteMe)
	{
		return NULL;
	}
	else
	{
		return GroupActor;
	}
}

/** Called before Interp editing to save original state of Actor. @see UInterpTrackInst::SaveActorState */
void UInterpGroupInst::SaveGroupActorState()
{
	check(Group);
	for(INT i=0; i<TrackInst.Num(); i++)
	{
		TrackInst(i)->SaveActorState( Group->InterpTracks(i) );
	}
}

/** Called after Interp editing to put object back to its original state. @see UInterpTrackInst::RestoreActorState */
void UInterpGroupInst::RestoreGroupActorState()
{
	check(Group);
	for(INT i=0; i<TrackInst.Num(); i++)
	{
		TrackInst(i)->RestoreActorState( Group->InterpTracks(i) );
	}
}

/** 
*	Initialize this Group instance. Called from USeqAct_Interp::InitInterp before doing any interpolation.
*	Save the Actor for the group and creates any needed InterpTrackInsts
*/
void UInterpGroupInst::InitGroupInst(UInterpGroup* InGroup, AActor* InGroupActor)
{
	check(InGroup);
	Group = InGroup;
	GroupActor = InGroupActor;

	for(INT i=0; i<InGroup->InterpTracks.Num(); i++)
	{
		// Construct Track instance object
		UInterpTrackInst* TrInst = ConstructObject<UInterpTrackInst>( InGroup->InterpTracks(i)->TrackInstClass, this, NAME_None, RF_Transactional );
		TrackInst.AddItem(TrInst);

		TrInst->InitTrackInst( InGroup->InterpTracks(i) );
	}

	// If we have an anim control track, do startup for that.
	UBOOL bHasAnimTrack = Group->HasAnimControlTrack();
	if(bHasAnimTrack && GroupActor)
	{
		// If in the editor and we haven't started playing, this should be Matinee! Bit yuck...
		if(GIsEditor && !GWorld->HasBegunPlay())
		{
			// Then set the ones specified by this Group.
			GroupActor->PreviewBeginAnimControl( Group->GroupAnimSets );
		}
		else
		{
			// If in game - call script function that notifies us to that.
			GroupActor->eventBeginAnimControl(Group->GroupAnimSets);
		}
	}
}

/** 
 *	Called when done with interpolation sequence. Cleans up InterpTrackInsts etc. 
 *	Do not do anything further with the Interpolation after this.
 */
void UInterpGroupInst::TermGroupInst(UBOOL bDeleteTrackInst)
{
	for(INT i=0; i<TrackInst.Num(); i++)
	{
		// Do any track cleanup
		UInterpTrack* Track = Group->InterpTracks(i);
		TrackInst(i)->TermTrackInst( Track );
	}
	TrackInst.Empty();

	// If we have an anim control track, do startup for that.
	UBOOL bHasAnimTrack = Group->HasAnimControlTrack();
	if (GroupActor != NULL && !GroupActor->IsPendingKill())
	{
		// If in the editor and we haven't started playing, this should be Matinee!
		// We always call PreviewFinishAnimControl, even if we don't have an AnimTrack now, because we may have done at some point during editing in Matinee.
		if(GIsEditor && !GWorld->HasBegunPlay())
		{
			// Restore the AnimSets that was set on this actor when we entered Matinee.
			GroupActor->PreviewFinishAnimControl();

			// DB: no longer needed, as WxInterpEd stores exact actor transformations.
			// Update any attachments to skeletal actors, so they are in the right place after the skeletal mesh has been reset.
			//UpdateAttachedActors();			
		}
		// Only call eventFinishAnimControl in the game if we have an anim track.
		else if(bHasAnimTrack)
		{
			// If in game - call script function to say we've finish with the anim control.
			GroupActor->eventFinishAnimControl();
		}
	}
}

/** Force any actors attached to this group's actor to update their position using their relative location/rotation. */
void UInterpGroupInst::UpdateAttachedActors()
{
	if(!GroupActor)
	{
		return;
	}

	// We don't want to update any Actors that this Matinee is acting on, in case we mess up where the sequence has put it.
	USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GetOuter() );

	// Get a list of all Actors that this SeqAct_Interp is working on.
	TArray<AActor*> AffectedActors;	
	Seq->GetAffectedActors(AffectedActors, true);

	// Update all actors attached to Actor, expect those we specify.
	GroupActor->EditorUpdateAttachedActors(AffectedActors);
}

/*-----------------------------------------------------------------------------
 UInterpGroupDirector
-----------------------------------------------------------------------------*/

/** Iterate over all InterpTracks in this InterpGroup, doing any actions to bring the state to the specified time. */
void UInterpGroupDirector::UpdateGroup(FLOAT NewPosition, UInterpGroupInst* GrInst, UBOOL bPreview, UBOOL bJump)
{
	Super::UpdateGroup(NewPosition, GrInst, bPreview, bJump);
}

/** Returns the director track inside this Director group - if present. */
UInterpTrackDirector* UInterpGroupDirector::GetDirectorTrack()
{
	for(INT i=0; i<InterpTracks.Num(); i++)
	{
		UInterpTrackDirector* DirTrack = Cast<UInterpTrackDirector>( InterpTracks(i) );
		if( DirTrack )
		{
			return DirTrack;
		}
	}

	return NULL;
}

/** Returns the fade track inside this Director group - if present. */
UInterpTrackFade* UInterpGroupDirector::GetFadeTrack()
{
	for(INT i=0; i<InterpTracks.Num(); i++)
	{
		UInterpTrackFade* FadeTrack = Cast<UInterpTrackFade>( InterpTracks(i) );
		if( FadeTrack )
		{
			return FadeTrack;
		}
	}

	return NULL;
}

/** Returns the slomo track inside this Director group - if present. */
UInterpTrackSlomo* UInterpGroupDirector::GetSlomoTrack()
{
	for(INT i=0; i<InterpTracks.Num(); i++)
	{
		UInterpTrackSlomo* SlomoTrack = Cast<UInterpTrackSlomo>( InterpTracks(i) );
		if( SlomoTrack )
		{
			return SlomoTrack;
		}
	}

	return NULL;
}

UInterpTrackColorScale* UInterpGroupDirector::GetColorScaleTrack()
{
	for(INT i=0; i<InterpTracks.Num(); i++)
	{
		UInterpTrackColorScale* ColorTrack = Cast<UInterpTrackColorScale>( InterpTracks(i) );
		if( ColorTrack )
		{
			return ColorTrack;
		}
	}

	return NULL;
}


/** Returns this director group's Audio Master track, if it has one */
UInterpTrackAudioMaster* UInterpGroupDirector::GetAudioMasterTrack()
{
	for(INT i=0; i<InterpTracks.Num(); i++)
	{
		UInterpTrackAudioMaster* AudioMasterTrack = Cast<UInterpTrackAudioMaster>( InterpTracks(i) );
		if( AudioMasterTrack )
		{
			return AudioMasterTrack;
		}
	}

	return NULL;
}



/*-----------------------------------------------------------------------------
 UInterpTrack
-----------------------------------------------------------------------------*/


/** 
 *	Conditionally calls PreviewUpdateTrack depending on whether or not the track is enabled.
 */
void UInterpTrack::ConditionalPreviewUpdateTrack(FLOAT NewPosition, class UInterpTrackInst* TrInst)
{
	// Is the track enabled?
	UBOOL bIsTrackEnabled = !bDisableTrack;
	UInterpGroupInst* GrInst = Cast<UInterpGroupInst>( TrInst->GetOuter() );
	if( GrInst != NULL )
	{
		USeqAct_Interp* Seq = Cast<USeqAct_Interp>( GrInst->GetOuter() );
		if( Seq != NULL )
		{
			if( ActiveCondition == ETAC_GoreEnabled && !Seq->bShouldShowGore ||
				ActiveCondition == ETAC_GoreDisabled && Seq->bShouldShowGore )
			{
				bIsTrackEnabled = FALSE;
			}
		}
	}

	if(bIsTrackEnabled)
	{
		PreviewUpdateTrack( NewPosition, TrInst );
	}
	else
	{
		TrInst->RestoreActorState(this);
	}
}

/** 
 *	Conditionally calls UpdateTrack depending on whether or not the track is enabled.
 */
void UInterpTrack::ConditionalUpdateTrack(FLOAT NewPosition, class UInterpTrackInst* TrInst, UBOOL bJump)
{
	// Is the track enabled?
	UBOOL bIsTrackEnabled = !bDisableTrack;
	UInterpGroupInst* GrInst = Cast<UInterpGroupInst>( TrInst->GetOuter() );
	if( GrInst != NULL )
	{
		USeqAct_Interp* Seq = Cast<USeqAct_Interp>( GrInst->GetOuter() );
		if( Seq != NULL )
		{
			if( ActiveCondition == ETAC_GoreEnabled && !Seq->bShouldShowGore ||
				ActiveCondition == ETAC_GoreDisabled && Seq->bShouldShowGore )
			{
				bIsTrackEnabled = FALSE;
			}
		}
	}

	if( bIsTrackEnabled )
	{
		UpdateTrack(NewPosition, TrInst, bJump);
	}
	else
	{
		TrInst->RestoreActorState(this);
	}
}


/** Get the name of the class used to help out when adding tracks, keys, etc. in UnrealEd.
* @return	String name of the helper class.*/
const FString	UInterpTrack::GetEdHelperClassName() const
{
	return FString( TEXT("UnrealEd.InterpTrackHelper") );
}


/*-----------------------------------------------------------------------------
 UInterpTrackInst
-----------------------------------------------------------------------------*/

/** 
 *	Return the Actor associated with this instance of a Group. 
 *	Note that all Groups have at least 1 instance, even if no Actor variable is attached, so this may return NULL. 
 */
AActor* UInterpTrackInst::GetGroupActor()
{
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( GetOuter() );
	return GrInst->GetGroupActor();
}

/*-----------------------------------------------------------------------------
	UInterpTrackInstProperty
-----------------------------------------------------------------------------*/

/** 
 *	This utility finds a property reference by name within an actor.
 *
 * @param InActor				Actor to search within
 * @param InPropName			Name of the property to search for
 * @param OutOuterObjectInst	Out variable: Outer object instance that holds the property.
 */
static UProperty* FindPropertyByName(AActor* InActor, FName InPropName, UObject*& OutOuterObjectInst)
{
	FString CompString, PropString;

	if(InPropName.ToString().Split(TEXT("."), &CompString, &PropString))
	{
		// STRUCT
		// First look for a struct with first part of name.
		UStructProperty* StructProp = FindField<UStructProperty>( InActor->GetClass(), *CompString );
		if(StructProp)
		{
			// Look 
			UProperty* Prop = FindField<UProperty>( StructProp->Struct, *PropString );
			OutOuterObjectInst = NULL;
			return Prop;
		}

		// COMPONENT
		// If no struct property with that name, search for a matching component
		FName CompName(*CompString);
		FName PropName(*PropString);
		UObject* OutObject = NULL;

		// First try old method - ie. component name.
		for (INT compIdx = 0; compIdx < InActor->Components.Num() && OutObject == InActor; compIdx++)
		{
			if (InActor->Components(compIdx) != NULL &&
				InActor->Components(compIdx)->GetFName() == CompName)
			{
				OutObject = InActor->Components(compIdx);
			}
		}

		// If that fails, try new method  - using name->component mapping table.
		if(!OutObject)
		{
			TArray<UComponent*> Components;
			InActor->CollectComponents(Components,FALSE);

			for ( INT ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++ )
			{
				UComponent* Component = Components(ComponentIndex);
				if ( Component->GetInstanceMapName() == CompName )
				{
					OutObject = Component;
					break;
				}
			}
		}

		// If we found a component - look for the named property within it.
		if(OutObject)
		{
			UProperty* Prop = FindField<UProperty>( OutObject->GetClass(), *PropName.ToString() );
			OutOuterObjectInst = OutObject;
			return Prop;
		}
		// No component with that name found - return NULL;
		else
		{
			return NULL;
		}
	}
	// No dot in name - just look for property in this actor.
	else
	{
		UProperty* Prop = FindField<UProperty>( InActor->GetClass(), *InPropName.ToString() );
		OutOuterObjectInst = InActor;
		return Prop;
	}
}

/**
 * Retrieves the update callback from the interp property's metadata and stores it.
 *
 * @param InActor			Actor we are operating on.
 * @param TrackProperty		Property we are interpolating.
 */
void UInterpTrackInstProperty::SetupPropertyUpdateCallback(AActor* InActor, const FName& TrackPropertyName)
{
	// Try to find a custom callback to use when updating the property.  This callback will be called instead of UpdateComponents.
	UObject* PropertyOuterObject = NULL;
	UProperty* InterpProperty = FindPropertyByName( InActor, TrackPropertyName, PropertyOuterObject );
	if(InterpProperty != NULL && PropertyOuterObject != NULL)
	{
		FString UpdateCallbackName = FString(TEXT("OnUpdateProperty")) + InterpProperty->GetName();
		PropertyUpdateCallback = PropertyOuterObject->FindFunction(*UpdateCallbackName);

		if(PropertyUpdateCallback!=NULL)
		{
			PropertyOuterObjectInst=PropertyOuterObject;
		}
	}
}

/** 
 * Tries to call the property update callback.
 *
 * @return TRUE if the callback existed and was called, FALSE otherwise.
 */
UBOOL UInterpTrackInstProperty::CallPropertyUpdateCallback()
{
	UBOOL bResult = FALSE;

	// if we have a UFunction and object instance resolved, then call the update callback on it.  Otherwise, return FALSE so
	// the interp track can handle the updating. We only do this in the game as it relies on calling script code.
	if(GIsGame && PropertyUpdateCallback != NULL && PropertyOuterObjectInst != NULL)
	{
		void *FuncParms = appAlloca(PropertyUpdateCallback->ParmsSize);
		appMemzero(FuncParms, PropertyUpdateCallback->ParmsSize);
		PropertyOuterObjectInst->ProcessEvent(PropertyUpdateCallback, FuncParms);
		appFree(FuncParms);

		bResult = TRUE;
	}

	return bResult;
}

/** Called when interpolation is done. Should not do anything else with this TrackInst after this. */
void UInterpTrackInstProperty::TermTrackInst(UInterpTrack* Track)
{
	// Clear references
	PropertyUpdateCallback=NULL;
	PropertyOuterObjectInst=NULL;

	Super::TermTrackInst(Track);
}

/*-----------------------------------------------------------------------------
 UInterpTrackMove
-----------------------------------------------------------------------------*/

/** Set this track to sensible default values. Called when track is first created. */
void UInterpTrackMove::SetTrackToSensibleDefault()
{
	// Use 'relative to initial' as the default.
	MoveFrame = IMF_RelativeToInitial;
}


/** Total number of keyframes in this track. */
INT UInterpTrackMove::GetNumKeyframes()
{
	check( (PosTrack.Points.Num() == EulerTrack.Points.Num()) && (PosTrack.Points.Num() == LookupTrack.Points.Num()) );
	return PosTrack.Points.Num();
}

/** Get first and last time of keyframes in this track. */
void UInterpTrackMove::GetTimeRange(FLOAT& StartTime, FLOAT& EndTime)
{
	check( (PosTrack.Points.Num() == EulerTrack.Points.Num()) && (PosTrack.Points.Num() == LookupTrack.Points.Num()) );

	if(PosTrack.Points.Num() == 0)
	{
		StartTime = 0.f;
		EndTime = 0.f;
	}
	else
	{
		// PosTrack and EulerTrack should have the same number of keys at the same times.
		check( (PosTrack.Points(0).InVal - EulerTrack.Points(0).InVal) < KINDA_SMALL_NUMBER );
		check( (PosTrack.Points(PosTrack.Points.Num()-1).InVal - EulerTrack.Points(EulerTrack.Points.Num()-1).InVal) < KINDA_SMALL_NUMBER );

		StartTime = PosTrack.Points(0).InVal;
		EndTime = PosTrack.Points( PosTrack.Points.Num()-1 ).InVal;
	}
}

/** Get the time of the keyframe with the given index. */
FLOAT UInterpTrackMove::GetKeyframeTime(INT KeyIndex)
{
	check( (PosTrack.Points.Num() == EulerTrack.Points.Num()) && (PosTrack.Points.Num() == LookupTrack.Points.Num()) );
	if( KeyIndex < 0 || KeyIndex >= PosTrack.Points.Num() )
		return 0.f;

	check( (PosTrack.Points(KeyIndex).InVal - EulerTrack.Points(KeyIndex).InVal) < KINDA_SMALL_NUMBER );

	return PosTrack.Points(KeyIndex).InVal;
}

/** Add a new keyframe at the speicifed time. Returns index of new keyframe. */
INT UInterpTrackMove::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	check( (PosTrack.Points.Num() == EulerTrack.Points.Num()) && (PosTrack.Points.Num() == LookupTrack.Points.Num()) );

	AActor* Actor = TrInst->GetGroupActor();
	if(!Actor)
	{
		return INDEX_NONE;
	}

	INT NewKeyIndex = PosTrack.AddPoint( Time, FVector(0,0,0) );
	PosTrack.Points(NewKeyIndex).InterpMode = InitInterpMode;

	INT NewRotKeyIndex = EulerTrack.AddPoint( Time, FVector(0.f) );
	EulerTrack.Points(NewRotKeyIndex).InterpMode = InitInterpMode;

	FName DefaultName(NAME_None);
	INT NewLookupKeyIndex = LookupTrack.AddPoint(Time, DefaultName);

	check((NewKeyIndex == NewRotKeyIndex) && (NewKeyIndex == NewLookupKeyIndex));

	// First key of a 'relative to initial' track must always be zero
	if(MoveFrame == IMF_World || NewKeyIndex != 0)
	{
		UpdateKeyframe(NewKeyIndex, TrInst);
	}

	PosTrack.AutoSetTangents(LinCurveTension);
	EulerTrack.AutoSetTangents(AngCurveTension);

	return NewKeyIndex;
}

FVector WindNumToEuler(const FVector& WindNum)
{
	FVector OutEuler;
	OutEuler.X = appRound(WindNum.X) * 360.f;
	OutEuler.Y = appRound(WindNum.Y) * 360.f;
	OutEuler.Z = appRound(WindNum.Z) * 360.f;
	return OutEuler;
}

static FMatrix GetBaseMatrix(AActor* Actor)
{
	check(Actor);

	FMatrix BaseTM = FMatrix::Identity;
	if(Actor->Base)
	{
		// Look at bone we are attached to if attached to a skeletal mesh component
		if(Actor->BaseSkelComponent)
		{
			INT BoneIndex = Actor->BaseSkelComponent->MatchRefBone(Actor->BaseBoneName);
			// If we can't find the bone, just use actor ref frame.
			if(BoneIndex != INDEX_NONE)
			{
				BaseTM = Actor->BaseSkelComponent->GetBoneMatrix(BoneIndex);
			}
			else
			{
				BaseTM = FRotationTranslationMatrix(Actor->Base->Rotation, Actor->Base->Location);
			}
		}
		// Not skeletal case - just use actor transform.
		else
		{
			BaseTM = FRotationTranslationMatrix(Actor->Base->Rotation, Actor->Base->Location);
		}
	}
	else
	{
		BaseTM = FMatrix::Identity;
	}

	BaseTM.RemoveScaling();
	return BaseTM;
}

/** Clamps values between -DELTA and DELTA to 0 */
static FLOAT ClampValueNearZero(FLOAT Value)
{
	FLOAT Result = Value;

	if(Abs<FLOAT>(Value) < DELTA)
	{
		Result = 0;
	}

	return Result;
}

/** Creates a rotator given a source matrix, it checks for very small floating point errors being passed to atan2 and clamps them accordingly. */
static FRotator GetCleanedUpRotator(const FMatrix &Mat)
{
	const FVector		XAxis	= Mat.GetAxis( 0 );
	const FVector		YAxis	= Mat.GetAxis( 1 );
	const FVector		ZAxis	= Mat.GetAxis( 2 );

	FRotator	Rotator	= FRotator( 
		appRound(appAtan2( ClampValueNearZero(XAxis.Z), ClampValueNearZero(appSqrt(Square(XAxis.X)+Square(XAxis.Y))) ) * 32768.f / PI), 
		appRound(appAtan2( ClampValueNearZero(XAxis.Y), ClampValueNearZero(XAxis.X) ) * 32768.f / PI), 
		0 
		);

	const FVector		SYAxis	= FRotationMatrix( Rotator ).GetAxis(1);
	Rotator.Roll		= appRound(appAtan2( ClampValueNearZero(ZAxis | SYAxis), ClampValueNearZero(YAxis | SYAxis) ) * 32768.f / PI);
	return Rotator;
}

/** Change the value of an existing keyframe. */
void UInterpTrackMove::UpdateKeyframe(INT KeyIndex, UInterpTrackInst* TrInst)
{
	check( (PosTrack.Points.Num() == EulerTrack.Points.Num()) && (PosTrack.Points.Num() == LookupTrack.Points.Num()) );
	if( KeyIndex < 0 || KeyIndex >= EulerTrack.Points.Num() )
	{
		return;
	}

	AActor* Actor = TrInst->GetGroupActor();
	if(!Actor)
	{
		return;
	}

	// You can't move the first frame - must always be 0,0,0
	if(MoveFrame == IMF_RelativeToInitial && KeyIndex == 0)
	{
		return;
	}


	// Don't want to record keyframes if track disabled.
	if(bDisableMovement)
	{
		return;
	}

	UInterpTrackInstMove* MoveTrackInst = CastChecked<UInterpTrackInstMove>(TrInst);


	AActor* BaseActor = Actor->GetBase();
	if( BaseActor == NULL && MoveFrame == IMF_World )
	{
		// World space coordinates, and no base actor.  This will be easy!
		PosTrack.Points(KeyIndex).OutVal = Actor->Location;
		EulerTrack.Points(KeyIndex).OutVal = Actor->Rotation.Euler();
	}
	else if( MoveFrame == IMF_World || MoveFrame == IMF_RelativeToInitial )
	{
		// First, figure out our reference transformation
		FMatrix RelativeToWorldTransform;
		if( MoveFrame == IMF_World )
		{
			// Our reference transform is simply our base actor's transform
			check( BaseActor != NULL );
			RelativeToWorldTransform = GetBaseMatrix( Actor );
		}
		else if( MoveFrame == IMF_RelativeToInitial )
		{
			if( BaseActor != NULL )
			{
				// Our reference transform is our initial transform combined with the base actor's transform
				RelativeToWorldTransform = MoveTrackInst->InitialTM * GetBaseMatrix( Actor );
			}
			else
			{
				// No base actor, so our reference transform is simply our initial transform
				RelativeToWorldTransform = MoveTrackInst->InitialTM;
			}
		}

		// Compute matrix that transforms from world space back into reference space
		FMatrix WorldToRelativeTransform = RelativeToWorldTransform.Inverse();


		// Take the Actor Rotator and turn it into Winding and Remainder parts
		FRotator WorldSpaceWindRot, WorldSpaceUnwoundRot;
		Actor->Rotation.GetWindingAndRemainder( WorldSpaceWindRot, WorldSpaceUnwoundRot );

		// Compute matrix that transforms the actor into the reference coordinate system
		FRotationTranslationMatrix ActorToWorldUnwoundTransform( WorldSpaceUnwoundRot, Actor->Location );

		// @todo: We are losing possible over-winding data (from rotation + unwound relative rotation) by
		//        using matrices to concatenate transforms here.  This can cause winding direction errors!
		FMatrix ActorToRelativeUnwoundTransform = ActorToWorldUnwoundTransform * WorldToRelativeTransform;
	
		// Position key
		{
			PosTrack.Points( KeyIndex ).OutVal = ActorToRelativeUnwoundTransform.GetOrigin();
		}

		// Rotation key
		{
			// Compute any extra rotation incurred by an over-wound world space rotation
			FVector WorldSpaceWindEulerNum = WorldSpaceWindRot.Euler() / 360.0f;
			FVector RelativeSpaceWindEulerNum = WorldToRelativeTransform.TransformNormal( WorldSpaceWindEulerNum );
			FVector RelativeSpaceWindEuler = WindNumToEuler( RelativeSpaceWindEulerNum );

			// Compute the rotation amount, relative to the initial transform
			FRotator RelativeSpaceUnwoundRot = GetCleanedUpRotator( ActorToRelativeUnwoundTransform );
			FVector RelativeSpaceUnwoundEuler = RelativeSpaceUnwoundRot.Euler();

			// Combine the relative space unwound euler with the transformed winding amount
			FVector RelativeSpaceEuler = RelativeSpaceUnwoundEuler + RelativeSpaceWindEuler;

			// To mitigate winding errors caused by using matrices for concatenation on actors that
			// relative to an initial transform or base actor (or both), we peek at an adjacent key
			// frame to attempt to keep rotation continuous
			if( EulerTrack.Points.Num() > 1 )
			{
				INT AdjacentKeyIndex = ( KeyIndex > 0 ) ? ( KeyIndex - 1 ) : ( KeyIndex + 1 );
				const FVector AdjacentEuler = EulerTrack.Points( AdjacentKeyIndex ).OutVal;

				// Try to minimize differences in curves
				const FVector EulerDiff = RelativeSpaceEuler - AdjacentEuler;
				if( EulerDiff.X > 180.0f )
				{
					RelativeSpaceEuler.X -= 360.0f;
				}
				else if( EulerDiff.X < -180.0f )
				{
					RelativeSpaceEuler.X += 360.0f;
				}
				if( EulerDiff.Y > 180.0f )
				{
					RelativeSpaceEuler.Y -= 360.0f;
				}
				else if( EulerDiff.Y < -180.0f )
				{
					RelativeSpaceEuler.Y += 360.0f;
				}
				if( EulerDiff.Z > 180.0f )
				{
					RelativeSpaceEuler.Z -= 360.0f;
				}
				else if( EulerDiff.Z < -180.0f )
				{
					RelativeSpaceEuler.Z += 360.0f;
				}
			}

			// Store the final rotation amount
			EulerTrack.Points( KeyIndex ).OutVal = RelativeSpaceEuler;
		}
	}
	else 
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_UnknownInterpolationType") );
		PosTrack.Points(KeyIndex).OutVal = FVector(0.f);
		EulerTrack.Points(KeyIndex).OutVal = FVector(0.f);
	}

	// Update the tangent vectors for the changed point, and its neighbours.
	PosTrack.AutoSetTangents(LinCurveTension);
	EulerTrack.AutoSetTangents(AngCurveTension);
}

/** Change the time position of an existing keyframe. This can change the index of the keyframe - the new index is returned. */
INT UInterpTrackMove::SetKeyframeTime(INT KeyIndex, FLOAT NewKeyTime, UBOOL bUpdateOrder)
{
	check( (PosTrack.Points.Num() == EulerTrack.Points.Num()) && (PosTrack.Points.Num() == LookupTrack.Points.Num()) );
	if( KeyIndex < 0 || KeyIndex >= PosTrack.Points.Num() )
		return KeyIndex;

	INT NewKeyIndex = KeyIndex;
	if(bUpdateOrder)
	{
		NewKeyIndex = PosTrack.MovePoint(KeyIndex, NewKeyTime);
		INT NewEulerKeyIndex = EulerTrack.MovePoint(KeyIndex, NewKeyTime);
		INT NewLookupKeyIndex = LookupTrack.MovePoint(KeyIndex, NewKeyTime);
		check( (NewKeyIndex == NewEulerKeyIndex) && (NewKeyIndex == NewLookupKeyIndex) );
	}
	else
	{
		PosTrack.Points(KeyIndex).InVal = NewKeyTime;
		EulerTrack.Points(KeyIndex).InVal = NewKeyTime;
		LookupTrack.Points(KeyIndex).Time = NewKeyTime;
	}

	PosTrack.AutoSetTangents(LinCurveTension);
	EulerTrack.AutoSetTangents(AngCurveTension);

	return NewKeyIndex;
}

/** Remove the keyframe with the given index. */
void UInterpTrackMove::RemoveKeyframe(INT KeyIndex)
{
	check( (PosTrack.Points.Num() == EulerTrack.Points.Num()) && (PosTrack.Points.Num() == LookupTrack.Points.Num()) );
	if( KeyIndex < 0 || KeyIndex >= PosTrack.Points.Num() )
		return;

	if(MoveFrame == IMF_RelativeToInitial && KeyIndex == 0)
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd(TEXT("CannotRemoveFirstMoveKey")) );
		return;
	}

	PosTrack.Points.Remove(KeyIndex);
	EulerTrack.Points.Remove(KeyIndex);
	LookupTrack.Points.Remove(KeyIndex);

	PosTrack.AutoSetTangents(LinCurveTension);
	EulerTrack.AutoSetTangents(AngCurveTension);
}

/** 
 *	Duplicate the keyframe with the given index to the specified time. 
 *	Returns the index of the newly created key.
 */
INT UInterpTrackMove::DuplicateKeyframe(INT KeyIndex, FLOAT NewKeyTime)
{
	check( (PosTrack.Points.Num() == EulerTrack.Points.Num()) && (PosTrack.Points.Num() == LookupTrack.Points.Num()) );
	if( KeyIndex < 0 || KeyIndex >= PosTrack.Points.Num() )
		return INDEX_NONE;

	FInterpCurvePoint<FVector> PosPoint = PosTrack.Points(KeyIndex);
	INT NewPosIndex = PosTrack.AddPoint(NewKeyTime, FVector(0.f));
	PosTrack.Points(NewPosIndex) = PosPoint; // Copy properties from source key.
	PosTrack.Points(NewPosIndex).InVal = NewKeyTime;

	FInterpCurvePoint<FVector> EulerPoint = EulerTrack.Points(KeyIndex);
	INT NewEulerIndex = EulerTrack.AddPoint(NewKeyTime, FVector(0.f));
	EulerTrack.Points(NewEulerIndex) = EulerPoint;
	EulerTrack.Points(NewEulerIndex).InVal = NewKeyTime;

	FName OldName = LookupTrack.Points(KeyIndex).GroupName;
	INT NewLookupKeyIndex = LookupTrack.AddPoint(NewKeyTime, OldName);

	check((NewPosIndex == NewEulerIndex) && (NewPosIndex == NewLookupKeyIndex));

	PosTrack.AutoSetTangents(LinCurveTension);
	EulerTrack.AutoSetTangents(AngCurveTension);

	return NewPosIndex;
}

/** Return the closest time to the time passed in that we might want to snap to. */
UBOOL UInterpTrackMove::GetClosestSnapPosition(FLOAT InPosition, TArray<INT> &IgnoreKeys, FLOAT& OutPosition)
{
	check( (PosTrack.Points.Num() == EulerTrack.Points.Num()) && (PosTrack.Points.Num() == LookupTrack.Points.Num()) );

	if(PosTrack.Points.Num() == 0)
	{
		return false;
	}

	UBOOL bFoundSnap = false;
	FLOAT ClosestSnap = 0.f;
	FLOAT ClosestDist = BIG_NUMBER;
	for(INT i=0; i<PosTrack.Points.Num(); i++)
	{
		if(!IgnoreKeys.ContainsItem(i))
		{
			FLOAT Dist = Abs( PosTrack.Points(i).InVal - InPosition );
			if(Dist < ClosestDist)
			{
				ClosestSnap = PosTrack.Points(i).InVal;
				ClosestDist = Dist;
				bFoundSnap = true;
			}
		}
	}

	OutPosition = ClosestSnap;
	return bFoundSnap;
}

/** 
 *	Conditionally calls PreviewUpdateTrack depending on whether or not the track is enabled.
 */
void UInterpTrackMove::ConditionalPreviewUpdateTrack(FLOAT NewPosition, class UInterpTrackInst* TrInst)
{
	// Is the track enabled?
	UBOOL bIsTrackEnabled = !bDisableTrack;
	UInterpGroupInst* GrInst = Cast<UInterpGroupInst>( TrInst->GetOuter() );
	if( GrInst != NULL )
	{
		USeqAct_Interp* Seq = Cast<USeqAct_Interp>( GrInst->GetOuter() );
		if( Seq != NULL )
		{
			if( ActiveCondition == ETAC_GoreEnabled && !Seq->bShouldShowGore ||
				ActiveCondition == ETAC_GoreDisabled && Seq->bShouldShowGore )
			{
				bIsTrackEnabled = FALSE;
			}
		}
	}

	FLOAT CurTime = NewPosition;

	if( !bIsTrackEnabled )
	{
		CurTime = 0.0f;
	}

	PreviewUpdateTrack(CurTime, TrInst);
}

/** 
 *	Function which actually updates things based on the new position in the track. 
 *  This is called in the editor, when scrubbing/previewing etc.
 */
void UInterpTrackMove::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
	AActor* Actor = TrInst->GetGroupActor();
	if(!Actor)
	{
		return;
	}

	if(bDisableMovement)
	{
		NewPosition = 0.0f;
	}

	FVector NewPos = Actor->Location;
	FRotator NewRot = Actor->Rotation;

	GetLocationAtTime(TrInst, NewPosition, NewPos, NewRot);

	Actor->Location = NewPos;
	Actor->Rotation = NewRot;

	// Don't force update all components unless we're in the editor.
	Actor->ConditionalForceUpdateComponents();

	// Update actors Based on this one once it moves.
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( TrInst->GetOuter() );
	GrInst->UpdateAttachedActors();
}

void UInterpTrackMove::PostLoad()
{
	Super::PostLoad();
}

void UInterpTrackMove::PostEditChange(UProperty* PropertyThatChanged)
{
	check( (PosTrack.Points.Num() == EulerTrack.Points.Num()) && (PosTrack.Points.Num() == LookupTrack.Points.Num()) );

	PosTrack.AutoSetTangents(LinCurveTension);
	EulerTrack.AutoSetTangents(AngCurveTension);
	Super::PostEditChange(PropertyThatChanged);
}

/** Called after copying/pasting this track.  Makes sure that the number of keys in the 3 keyframe arrays matchup properly. */
void UInterpTrackMove::PostEditImport()
{
	Super::PostEditImport();

	// Make sure that our array sizes match up.  If they don't, it is due to default struct keys not being exported. (Only happens for keys at Time=0).
	// @todo: This is a hack and can be removed once the struct properties are exported to text correctly for arrays of structs.
	if(PosTrack.Points.Num() > LookupTrack.Points.Num())	// Lookup track elements weren't imported.
	{
		INT Count = PosTrack.Points.Num()-LookupTrack.Points.Num();
		FName DefaultName(NAME_None);
		for(INT PointIdx=0; PointIdx<Count; PointIdx++)
		{
			LookupTrack.AddPoint(PosTrack.Points(PointIdx).InVal, DefaultName);
		}

		for(INT PointIdx=Count; PointIdx<PosTrack.Points.Num(); PointIdx++)
		{
			LookupTrack.Points(PointIdx).Time = PosTrack.Points(PointIdx).InVal;
		}
	}
	else if(PosTrack.Points.Num()==EulerTrack.Points.Num() && PosTrack.Points.Num() < LookupTrack.Points.Num())	// Pos/euler track elements weren't imported.
	{
		INT Count = LookupTrack.Points.Num()-PosTrack.Points.Num();

		for(INT PointIdx=0; PointIdx<Count; PointIdx++)
		{
			PosTrack.AddPoint( LookupTrack.Points(PointIdx).Time, FVector(0,0,0) );
			EulerTrack.AddPoint( LookupTrack.Points(PointIdx).Time, FVector(0.f) );
		}

		for(INT PointIdx=Count; PointIdx<LookupTrack.Points.Num(); PointIdx++)
		{
			PosTrack.Points(PointIdx).InVal = LookupTrack.Points(PointIdx).Time;
			EulerTrack.Points(PointIdx).InVal = LookupTrack.Points(PointIdx).Time;
		}

		PosTrack.AutoSetTangents(LinCurveTension);
		EulerTrack.AutoSetTangents(AngCurveTension);
	}

	check((PosTrack.Points.Num() == EulerTrack.Points.Num()) && (PosTrack.Points.Num() == LookupTrack.Points.Num()));
}

/**
 * @param KeyIndex	Index of the key to retrieve the lookup group name for.
 *
 * @return Returns the groupname for the keyindex specified.
 */
FName UInterpTrackMove::GetLookupKeyGroupName(INT KeyIndex)
{
	check( (PosTrack.Points.Num() == EulerTrack.Points.Num()) && (PosTrack.Points.Num() == LookupTrack.Points.Num()) );
	check( KeyIndex < LookupTrack.Points.Num() );

	return LookupTrack.Points(KeyIndex).GroupName;
}

/**
 * Sets the lookup group name for a movement track keyframe.
 *
 * @param KeyIndex			Index of the key to modify.
 * @param NewGroupName		Group name to set the keyframe's lookup group to.
 */
void UInterpTrackMove::SetLookupKeyGroupName(INT KeyIndex, const FName &NewGroupName)
{
	check( (PosTrack.Points.Num() == EulerTrack.Points.Num()) && (PosTrack.Points.Num() == LookupTrack.Points.Num()) );
	check( KeyIndex < LookupTrack.Points.Num() );

	LookupTrack.Points(KeyIndex).GroupName = NewGroupName;
}

/**
 * Clears the lookup group name for a movement track keyframe.
 *
 * @param KeyIndex			Index of the key to modify.
 */
void UInterpTrackMove::ClearLookupKeyGroupName(INT KeyIndex)
{
	FName DefaultName(NAME_None);
	SetLookupKeyGroupName(KeyIndex, DefaultName);
}

/**
 * Gets the position of a keyframe given its key index.  Also optionally retrieves the Arrive and Leave tangents for the key.
 * This function respects the LookupTrack.
 *
 * @param TrInst			TrackInst to use for lookup track positions.
 * @param KeyIndex			Index of the keyframe to get the position of.
 * @param OutTime			Final time of the keyframe.
 * @param OutPos			Final position of the keyframe.
 * @param OutArriveTangent	Pointer to a vector to store the arrive tangent in, can be NULL.
 * @param OutLeaveTangent	Pointer to a vector to store the leave tangent in, can be NULL.
 */
void UInterpTrackMove::GetKeyframePosition(UInterpTrackInst* TrInst, INT KeyIndex, FLOAT& OutTime, FVector &OutPos, FVector *OutArriveTangent, FVector *OutLeaveTangent)
{
	UBOOL bUsePosTrack = TRUE;

	check( (PosTrack.Points.Num() == EulerTrack.Points.Num()) && (PosTrack.Points.Num() == LookupTrack.Points.Num()) );
	check( KeyIndex < LookupTrack.Points.Num() );

	// See if this key is trying to get its position from another group.
	FName GroupName = LookupTrack.Points(KeyIndex).GroupName;
	if(GroupName != NAME_None && TrInst)
	{
		// Lookup position from the lookup track.
		AActor* Actor = TrInst->GetGroupActor();
		UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( TrInst->GetOuter() );
		USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );
		UInterpGroupInst* LookupGroupInst = Seq->FindFirstGroupInstByName(GroupName);

		if(Actor && LookupGroupInst && LookupGroupInst->GetGroupActor())
		{
			AActor* LookupActor = LookupGroupInst->GetGroupActor();

			// Slight hack here so that if we are trying to look at a Player variable, it looks at their Pawn.
			APlayerController* PC = Cast<APlayerController>(LookupActor);
			if(PC && PC->Pawn)
			{
				LookupActor = PC->Pawn;
			}

			// Find position
			OutPos = LookupActor->Location;
			OutTime = LookupTrack.Points( KeyIndex ).Time;

			// Find arrive and leave tangents.
			if(OutLeaveTangent != NULL || OutArriveTangent != NULL)
			{
				if(KeyIndex==0 || KeyIndex==(LookupTrack.Points.Num()-1))	// if we are an endpoint, set tangents to 0.
				{
					if(OutArriveTangent!=NULL)
					{
						appMemset( OutArriveTangent, 0, sizeof(FVector) );
					}

					if(OutLeaveTangent != NULL)
					{
						appMemset( OutLeaveTangent, 0, sizeof(FVector) );
					}
				}
				else
				{
					FVector PrevPos, NextPos;
					FLOAT PrevTime, NextTime;
					FVector AutoTangent;

					// Get previous and next positions for the tangents.
					GetKeyframePosition(TrInst, KeyIndex-1, PrevTime, PrevPos, NULL, NULL);
					GetKeyframePosition(TrInst, KeyIndex+1, NextTime, NextPos, NULL, NULL);

					if( PosTrack.InterpMethod == IMT_UseFixedTangentEvalAndNewAutoTangents )
					{
						// @todo: Should this setting be exposed in some way to the Lookup track?
						const UBOOL bWantClamping = FALSE;

						ComputeCurveTangent(
							PrevTime, PrevPos,
							OutTime, OutPos,
							NextTime, NextPos,
							LinCurveTension,				// Tension
							bWantClamping,
							AutoTangent );					// Out
					}
					else
					{
						LegacyAutoCalcTangent( PrevPos, OutPos, NextPos, LinCurveTension, AutoTangent );
					}

					if(OutArriveTangent!=NULL)
					{
						*OutArriveTangent = AutoTangent;
					}

					if(OutLeaveTangent != NULL)
					{
						*OutLeaveTangent = AutoTangent;
					}
				}
			}

			bUsePosTrack = FALSE;
		}
	}

	// We couldn't lookup a position from another group, so use the value stored in the pos track.
	if(bUsePosTrack)
	{
		OutTime = PosTrack.Points(KeyIndex).InVal;
		OutPos = PosTrack.Points(KeyIndex).OutVal;

		if(OutArriveTangent != NULL)
		{
			*OutArriveTangent = PosTrack.Points(KeyIndex).ArriveTangent;
		}

		if(OutLeaveTangent != NULL)
		{
			*OutLeaveTangent = PosTrack.Points(KeyIndex).LeaveTangent;
		}
	}
}


/**
 * Gets the rotation of a keyframe given its key index.  Also optionally retrieves the Arrive and Leave tangents for the key.
 * This function respects the LookupTrack.
 *
 * @param TrInst			TrackInst to use for lookup track rotations.
 * @param KeyIndex			Index of the keyframe to get the rotation of.
 * @param OutTime			Final time of the keyframe.
 * @param OutRot			Final rotation of the keyframe.
 * @param OutArriveTangent	Pointer to a vector to store the arrive tangent in, can be NULL.
 * @param OutLeaveTangent	Pointer to a vector to store the leave tangent in, can be NULL.
 */
void UInterpTrackMove::GetKeyframeRotation(UInterpTrackInst* TrInst, INT KeyIndex, FLOAT& OutTime, FVector &OutRot, FVector *OutArriveTangent, FVector *OutLeaveTangent)
{
	UBOOL bUseRotTrack = TRUE;

	check( (PosTrack.Points.Num() == EulerTrack.Points.Num()) && (PosTrack.Points.Num() == LookupTrack.Points.Num()) );
	check( KeyIndex < LookupTrack.Points.Num() );

	// See if this key is trying to get its rotation from another group.
	FName GroupName = LookupTrack.Points(KeyIndex).GroupName;
	if(GroupName != NAME_None && TrInst)
	{
		// Lookup rotation from the lookup track.
		AActor* Actor = TrInst->GetGroupActor();
		UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( TrInst->GetOuter() );
		USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );
		UInterpGroupInst* LookupGroupInst = Seq->FindFirstGroupInstByName(GroupName);

		if(Actor && LookupGroupInst && LookupGroupInst->GetGroupActor())
		{
			AActor* LookupActor = LookupGroupInst->GetGroupActor();

			// Slight hack here so that if we are trying to look at a Player variable, it looks at their Pawn.
			APlayerController* PC = Cast<APlayerController>(LookupActor);
			if(PC && PC->Pawn)
			{
				LookupActor = PC->Pawn;
			}

			// Find rotation
			OutRot = LookupActor->Rotation.Euler();
			OutTime = LookupTrack.Points( KeyIndex ).Time;

			// Find arrive and leave tangents.
			if(OutLeaveTangent != NULL || OutArriveTangent != NULL)
			{
				if(KeyIndex==0 || KeyIndex==(LookupTrack.Points.Num()-1))	// if we are an endpoint, set tangents to 0.
				{
					if(OutArriveTangent!=NULL)
					{
						appMemset( OutArriveTangent, 0, sizeof(FVector) );
					}

					if(OutLeaveTangent != NULL)
					{
						appMemset( OutLeaveTangent, 0, sizeof(FVector) );
					}
				}
				else
				{
					FVector PrevRot, NextRot;
					FLOAT PrevTime, NextTime;
					FVector AutoTangent;

					// Get previous and next positions for the tangents.
					GetKeyframeRotation(TrInst, KeyIndex-1, PrevTime, PrevRot, NULL, NULL);
					GetKeyframeRotation(TrInst, KeyIndex+1, NextTime, NextRot, NULL, NULL);

					if( EulerTrack.InterpMethod == IMT_UseFixedTangentEvalAndNewAutoTangents )
					{
						// @todo: Should this setting be exposed in some way to the Lookup track?
						const UBOOL bWantClamping = FALSE;

						ComputeCurveTangent(
							PrevTime, PrevRot,
							OutTime, OutRot,
							NextTime, NextRot,
							LinCurveTension,				// Tension
							bWantClamping,
							AutoTangent );					// Out
					}
					else
					{
						LegacyAutoCalcTangent( PrevRot, OutRot, NextRot, LinCurveTension, AutoTangent );
					}

					if(OutArriveTangent!=NULL)
					{
						*OutArriveTangent = AutoTangent;
					}

					if(OutLeaveTangent != NULL)
					{
						*OutLeaveTangent = AutoTangent;
					}
				}
			}

			bUseRotTrack = FALSE;
		}
	}

	// We couldn't lookup a position from another group, so use the value stored in the pos track.
	if(bUseRotTrack)
	{
		OutTime = EulerTrack.Points(KeyIndex).InVal;
		OutRot = EulerTrack.Points(KeyIndex).OutVal;

		if(OutArriveTangent != NULL)
		{
			*OutArriveTangent = EulerTrack.Points(KeyIndex).ArriveTangent;
		}

		if(OutLeaveTangent != NULL)
		{
			*OutLeaveTangent = EulerTrack.Points(KeyIndex).LeaveTangent;
		}
	}
}


/**
 * Replacement for the PosTrack eval function that uses GetKeyframePosition.  This is so we can replace keyframes that get their information from other tracks.
 *
 * @param TrInst	TrackInst to use for looking up groups.
 * @param Time		Time to evaluate position at.
 * @return			Final position at the specified time.
 */
FVector UInterpTrackMove::EvalPositionAtTime(UInterpTrackInst* TrInst, FLOAT Time)
{
	FVector OutPos;
	FLOAT KeyTime;	// Not used here
	const INT NumPoints = PosTrack.Points.Num();

	// If no point in curve, return the Default value we passed in.
	if( NumPoints == 0 )
	{
		return FVector(0.f);
	}

	// If only one point, or before the first point in the curve, return the first points value.
	if( NumPoints < 2 || (Time <= PosTrack.Points(0).InVal) )
	{
		GetKeyframePosition(TrInst, 0, KeyTime, OutPos, NULL, NULL);
		return OutPos;
	}

	// If beyond the last point in the curve, return its value.
	if( Time >= PosTrack.Points(NumPoints-1).InVal )
	{
		GetKeyframePosition(TrInst, NumPoints - 1, KeyTime, OutPos, NULL, NULL);
		return OutPos;
	}

	// Somewhere with curve range - linear search to find value.
	for( INT i=1; i<NumPoints; i++ )
	{	
		if( Time < PosTrack.Points(i).InVal )
		{
			const FLOAT Diff = PosTrack.Points(i).InVal - PosTrack.Points(i-1).InVal;

			if( Diff > 0.f && PosTrack.Points(i-1).InterpMode != CIM_Constant )
			{
				const FLOAT Alpha = (Time - PosTrack.Points(i-1).InVal) / Diff;

				if( PosTrack.Points(i-1).InterpMode == CIM_Linear )	// Linear interpolation
				{
					FVector PrevPos, CurrentPos;
					GetKeyframePosition(TrInst, i-1, KeyTime, PrevPos, NULL, NULL);
					GetKeyframePosition(TrInst, i, KeyTime, CurrentPos, NULL, NULL);

					OutPos =  Lerp( PrevPos, CurrentPos, Alpha );
					return OutPos;
				}
				else	//Cubic Interpolation
				{
					// Get keyframe positions and tangents.
					FVector CurrentPos, CurrentArriveTangent, PrevPos, PrevLeaveTangent;
					GetKeyframePosition(TrInst, i-1, KeyTime, PrevPos, NULL, &PrevLeaveTangent);
					GetKeyframePosition(TrInst, i, KeyTime, CurrentPos, &CurrentArriveTangent, NULL);

					if(PosTrack.InterpMethod == IMT_UseBrokenTangentEval)
					{
						OutPos = CubicInterp( PrevPos, PrevLeaveTangent, CurrentPos, CurrentArriveTangent, Alpha );
						return OutPos;
					}
					else
					{
						OutPos = CubicInterp( PrevPos, PrevLeaveTangent * Diff, CurrentPos, CurrentArriveTangent * Diff, Alpha );
						return OutPos;
					}
				}
			}
			else	// Constant Interpolation
			{
				GetKeyframePosition(TrInst, i-1, KeyTime, OutPos, NULL, NULL);
				return OutPos;
			}
		}
	}

	// Shouldn't really reach here.
	GetKeyframePosition(TrInst, NumPoints-1, KeyTime, OutPos, NULL, NULL);
	return OutPos;
}


/**
 * Replacement for the RotTrack eval function that uses GetKeyframeRotation.  This is so we can replace keyframes that get their information from other tracks.
 *
 * @param TrInst	TrackInst to use for looking up groups.
 * @param Time		Time to evaluate rotation at.
 * @return			Final rotation at the specified time.
 */
FVector UInterpTrackMove::EvalRotationAtTime(UInterpTrackInst* TrInst, FLOAT Time)
{
	FVector OutRot;
	FLOAT KeyTime;	// Not used here
	const INT NumPoints = EulerTrack.Points.Num();

	// If no point in curve, return the Default value we passed in.
	if( NumPoints == 0 )
	{
		return FVector(0.f);
	}

	// If only one point, or before the first point in the curve, return the first points value.
	if( NumPoints < 2 || (Time <= EulerTrack.Points(0).InVal) )
	{
		GetKeyframeRotation(TrInst, 0, KeyTime, OutRot, NULL, NULL);
		return OutRot;
	}

	// If beyond the last point in the curve, return its value.
	if( Time >= EulerTrack.Points(NumPoints-1).InVal )
	{
		GetKeyframeRotation(TrInst, NumPoints - 1, KeyTime, OutRot, NULL, NULL);
		return OutRot;
	}

	// Somewhere with curve range - linear search to find value.
	for( INT i=1; i<NumPoints; i++ )
	{	
		if( Time < EulerTrack.Points(i).InVal )
		{
			const FLOAT Diff = EulerTrack.Points(i).InVal - EulerTrack.Points(i-1).InVal;

			if( Diff > 0.f && EulerTrack.Points(i-1).InterpMode != CIM_Constant )
			{
				const FLOAT Alpha = (Time - EulerTrack.Points(i-1).InVal) / Diff;

				if( EulerTrack.Points(i-1).InterpMode == CIM_Linear )	// Linear interpolation
				{
					FVector PrevRot, CurrentRot;
					GetKeyframeRotation(TrInst, i-1, KeyTime, PrevRot, NULL, NULL);
					GetKeyframeRotation(TrInst, i, KeyTime, CurrentRot, NULL, NULL);

					OutRot =  Lerp( PrevRot, CurrentRot, Alpha );
					return OutRot;
				}
				else	//Cubic Interpolation
				{
					// Get keyframe rotations and tangents.
					FVector CurrentRot, CurrentArriveTangent, PrevRot, PrevLeaveTangent;
					GetKeyframeRotation(TrInst, i-1, KeyTime, PrevRot, NULL, &PrevLeaveTangent);
					GetKeyframeRotation(TrInst, i, KeyTime, CurrentRot, &CurrentArriveTangent, NULL);

					if(EulerTrack.InterpMethod == IMT_UseBrokenTangentEval)
					{
						OutRot = CubicInterp( PrevRot, PrevLeaveTangent, CurrentRot, CurrentArriveTangent, Alpha );
						return OutRot;
					}
					else
					{
						OutRot = CubicInterp( PrevRot, PrevLeaveTangent * Diff, CurrentRot, CurrentArriveTangent * Diff, Alpha );
						return OutRot;
					}
				}
			}
			else	// Constant Interpolation
			{
				GetKeyframeRotation(TrInst, i-1, KeyTime, OutRot, NULL, NULL);
				return OutRot;
			}
		}
	}

	// Shouldn't really reach here.
	GetKeyframeRotation(TrInst, NumPoints-1, KeyTime, OutRot, NULL, NULL);
	return OutRot;
}


void UInterpTrackMove::GetKeyTransformAtTime(UInterpTrackInst* TrInst, FLOAT Time, FVector& OutPos, FRotator& OutRot)
{
	FQuat KeyQuat;
	FLOAT KeyTime;	// Not used here
	if(bUseQuatInterpolation)
	{
		INT NumPoints = EulerTrack.Points.Num();

		// If no point in curve, return the Default value we passed in.
		if( NumPoints == 0 )
		{
			KeyQuat = FQuat::Identity;
		}
		// If only one point, or before the first point in the curve, return the first points value.
		else if( NumPoints < 2 || (Time <= EulerTrack.Points(0).InVal) )
		{
			FVector OutRot;
			GetKeyframeRotation(TrInst, 0, KeyTime, OutRot, NULL, NULL);
			KeyQuat = FQuat::MakeFromEuler(OutRot);
		}
		// If beyond the last point in the curve, return its value.
		else if( Time >= EulerTrack.Points(NumPoints-1).InVal )
		{
			FVector OutRot;
			GetKeyframeRotation(TrInst, NumPoints-1, KeyTime, OutRot, NULL, NULL);
			KeyQuat = FQuat::MakeFromEuler(OutRot);
		}
		// Somewhere with curve range - linear search to find value.
		else
		{			
			UBOOL bFoundPos = false;
			for( INT KeyIdx=1; KeyIdx<NumPoints && !bFoundPos; KeyIdx++ )
			{	
				if( Time < EulerTrack.Points(KeyIdx).InVal )
				{
					FLOAT Delta = EulerTrack.Points(KeyIdx).InVal - EulerTrack.Points(KeyIdx-1).InVal;
					FLOAT Alpha = Clamp( (Time - EulerTrack.Points(KeyIdx-1).InVal) / Delta, 0.f, 1.f );
					FVector CurrentRot, PrevRot;

					GetKeyframeRotation(TrInst, KeyIdx-1, KeyTime, PrevRot, NULL, NULL);
					GetKeyframeRotation(TrInst, KeyIdx, KeyTime, CurrentRot, NULL, NULL);

					FQuat Key1Quat = FQuat::MakeFromEuler(PrevRot);
					FQuat Key2Quat = FQuat::MakeFromEuler(CurrentRot);

					KeyQuat = SlerpQuat( Key1Quat, Key2Quat, Alpha );

					bFoundPos = true;
				}
			}
		}

		OutRot = FRotator(KeyQuat);
	}
	else
	{
		OutRot = FRotator::MakeFromEuler( EvalRotationAtTime(TrInst, Time) );
	}

	// Evaluate position
	OutPos = EvalPositionAtTime(TrInst, Time);
}



/**
 * Computes the world space coordinates for a key; handles keys that use IMF_RelativeToInitial, basing, etc.
 *
 * @param MoveTrackInst		An instance of this movement track
 * @param RelativeSpacePos	Key position value from curve
 * @param RelativeSpaceRot	Key rotation value from curve
 * @param OutPos			Output world space position
 * @param OutRot			Output world space rotation
 */
void UInterpTrackMove::ComputeWorldSpaceKeyTransform( UInterpTrackInstMove* MoveTrackInst,
													  const FVector& RelativeSpacePos,
													  const FRotator& RelativeSpaceRot,
													  FVector& OutPos,
													  FRotator& OutRot )
{
	// Break into rotation and winding part.
	FRotator RelativeSpaceWindRot, RelativeSpaceUnwoundRot;
	RelativeSpaceRot.GetWindingAndRemainder( RelativeSpaceWindRot, RelativeSpaceUnwoundRot );

	// Find the reference frame the key is considered in.
	FMatrix RelativeToWorldTransform = GetMoveRefFrame( MoveTrackInst );

	// Use rotation part to form transformation matrix.
	FRotationTranslationMatrix ActorToRelativeUnwoundTransform( RelativeSpaceUnwoundRot, RelativeSpacePos );

	// Compute the rotation amount in world space
	// @todo: Doesn't take into account overwind introduced by transforming relative euler angles to world space
	FMatrix ActorToWorldUnwoundTransform = ActorToRelativeUnwoundTransform * RelativeToWorldTransform;


	// Position
	{
		// Apply keyframed position to base to find desired position in world frame.
		OutPos = ActorToWorldUnwoundTransform.GetOrigin();
	}

	// Rotation
	{
		// Transform winding into correct ref frame.
		FVector RelativeSpaceWindEulerNum = RelativeSpaceWindRot.Euler() / 360.f;
		FVector WorldSpaceWindEulerNum = RelativeToWorldTransform.TransformNormal( RelativeSpaceWindEulerNum );
		FVector WorldSpaceWindEuler = WindNumToEuler( WorldSpaceWindEulerNum );
	
		// Combine the world space unwound euler with the transformed winding amount
		OutRot = GetCleanedUpRotator( ActorToWorldUnwoundTransform ) + FRotator::MakeFromEuler( WorldSpaceWindEuler );
	}
}



// The inputs here are treated as the 'default' output ie. if there is no track data.
void UInterpTrackMove::GetLocationAtTime(UInterpTrackInst* TrInst, FLOAT Time, FVector& OutPos, FRotator& OutRot)
{
	UInterpTrackInstMove* MoveTrackInst = CastChecked<UInterpTrackInstMove>(TrInst);

	check((EulerTrack.Points.Num() == PosTrack.Points.Num()) && (EulerTrack.Points.Num() == LookupTrack.Points.Num()));

	// Do nothing if no data on this track.
	if(EulerTrack.Points.Num() == 0)
	{
		return;
	}

	// Find the transform for the given time.
	FVector RelativeSpacePos;
	FRotator RelativeSpaceRot;
	GetKeyTransformAtTime( TrInst, Time, RelativeSpacePos, RelativeSpaceRot );

	// Compute world space key transform
	ComputeWorldSpaceKeyTransform( MoveTrackInst, RelativeSpacePos, RelativeSpaceRot, OutPos, OutRot );
	

	// Replace rotation if using a special rotation mode.
	if(RotMode == IMR_LookAtGroup)
	{		
		if(LookAtGroupName != NAME_None)
		{
			AActor* Actor = TrInst->GetGroupActor();

			UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( TrInst->GetOuter() );
			USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );
			UInterpGroupInst* LookAtGroupInst = Seq->FindFirstGroupInstByName(LookAtGroupName);

			if(Actor && LookAtGroupInst && LookAtGroupInst->GetGroupActor())
			{
				AActor* LookAtActor = LookAtGroupInst->GetGroupActor();

				// Slight hack here so that if we are trying to look at a Player variable, it looks at their Pawn.
				APlayerController* PC = Cast<APlayerController>(LookAtActor);
				if(PC && PC->Pawn)
				{
					LookAtActor = PC->Pawn;
				}

				// Find Rotator that points at LookAtActor
				FVector LookDir = (LookAtActor->Location - Actor->Location).SafeNormal();
				OutRot = LookDir.Rotation();
			}
		}
	}
}

/** 
 *	Return the reference frame that the animation is currently working within.
 *	Looks at the current MoveFrame setting and whether the Actor is based on something.
 */
FMatrix UInterpTrackMove::GetMoveRefFrame(UInterpTrackInstMove* MoveTrackInst)
{
	AActor* Actor = MoveTrackInst->GetGroupActor();
	FMatrix BaseTM = FMatrix::Identity;

	AActor* BaseActor = NULL;
	if(Actor)
	{
		BaseActor = Actor->GetBase();
		BaseTM = GetBaseMatrix(Actor);
	}

	FMatrix RefTM = FMatrix::Identity;
	if( MoveFrame == IMF_World )
	{
		RefTM = BaseTM;
	}
	else if( MoveFrame == IMF_RelativeToInitial )
	{
		RefTM = MoveTrackInst->InitialTM * BaseTM;
		RefTM.RemoveScaling();
	}

	return RefTM;
}

/*-----------------------------------------------------------------------------
 UInterpTrackInstMove
-----------------------------------------------------------------------------*/

/** 
 *	Calculate and store the InitialTM and InitialQuat values, that are used as the basis for RelativeToInitial movements. 
 *	If the bZeroFromHere parameter is true, 
 */
void UInterpTrackInstMove::CalcInitialTransform(UInterpTrack* Track, UBOOL bZeroFromHere)
{
	UInterpTrackMove* MoveTrack = CastChecked<UInterpTrackMove>( Track );
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( GetOuter() );
	USeqAct_Interp* Seq = Cast<USeqAct_Interp>( GrInst->GetOuter() );			// ok if this is NULL

	AActor* Actor = GetGroupActor();
	if(!Actor)
	{
		return;
	}

	// The following is only used in the case of IMF_RelativeToInitial

	FMatrix ActorTM = FRotationTranslationMatrix( Actor->Rotation, Actor->Location );

	// If this Actor has a base, transform its current position into the frame of its Base.
	AActor* BaseActor = Actor->GetBase();
	if(BaseActor)
	{
		FMatrix BaseTM = GetBaseMatrix(Actor);
		FMatrix InvBaseTM = BaseTM.Inverse();
		ActorTM = ActorTM * InvBaseTM;
	}

	FVector RelPos;
	FRotator RelRot;

	// If flag is set, use this as the 'base' transform regardless.
	if(bZeroFromHere)
	{
		MoveTrack->GetKeyTransformAtTime(this, 0.f, RelPos, RelRot);
	}
	else
	{
		// Find the current relative position according to the interpolation.
		FLOAT const Pos = Seq ? Seq->Position : 0.f;
		MoveTrack->GetKeyTransformAtTime(this, Pos, RelPos, RelRot);
	}

	FRotationTranslationMatrix RelTM(RelRot,RelPos);

	// Initial position is the current position of the Actor, minus the transform from the beginning of the track.
	// This is so that we can stop a 'relative to initial' interpolation and restart it again etc.
	InitialTM = RelTM.Inverse() * ActorTM;
	InitialTM.RemoveScaling();

	InitialQuat = FQuat(InitialTM);
}

/** Initialise this Track instance. Called in-game before doing any interpolation. */
void UInterpTrackInstMove::InitTrackInst(UInterpTrack* Track)
{
	// Initialise the 'initial' position for this track.
	CalcInitialTransform(Track, FALSE);
}


/*-----------------------------------------------------------------------------
  UInterpTrackFloatBase
-----------------------------------------------------------------------------*/

/** Total number of keyframes in this track. */
INT UInterpTrackFloatBase::GetNumKeyframes()
{
	return FloatTrack.Points.Num();
}

/** Get first and last time of keyframes in this track. */
void UInterpTrackFloatBase::GetTimeRange(FLOAT& StartTime, FLOAT& EndTime)
{
	if(FloatTrack.Points.Num() == 0)
	{
		StartTime = 0.f;
		EndTime = 0.f;
	}
	else
	{
		StartTime = FloatTrack.Points(0).InVal;
		EndTime = FloatTrack.Points( FloatTrack.Points.Num()-1 ).InVal;
	}
}

/** Get the time of the keyframe with the given index. */
FLOAT UInterpTrackFloatBase::GetKeyframeTime(INT KeyIndex)
{
	if( KeyIndex < 0 || KeyIndex >= FloatTrack.Points.Num() )
		return 0.f;

	return FloatTrack.Points(KeyIndex).InVal;
}

/** Change the time position of an existing keyframe. This can change the index of the keyframe - the new index is returned. */
INT UInterpTrackFloatBase::SetKeyframeTime(INT KeyIndex, FLOAT NewKeyTime, UBOOL bUpdateOrder)
{
	if( KeyIndex < 0 || KeyIndex >= FloatTrack.Points.Num() )
		return KeyIndex;

	INT NewKeyIndex = KeyIndex;
	if(bUpdateOrder)
	{
		NewKeyIndex = FloatTrack.MovePoint(KeyIndex, NewKeyTime);
	}
	else
	{
		FloatTrack.Points(KeyIndex).InVal = NewKeyTime;
	}

	FloatTrack.AutoSetTangents(CurveTension);

	return NewKeyIndex;
}

/** Remove the keyframe with the given index. */
void UInterpTrackFloatBase::RemoveKeyframe(INT KeyIndex)
{
	if( KeyIndex < 0 || KeyIndex >= FloatTrack.Points.Num() )
		return;

	FloatTrack.Points.Remove(KeyIndex);

	FloatTrack.AutoSetTangents(CurveTension);
}


/** 
 *	Duplicate the keyframe with the given index to the specified time. 
 *	Returns the index of the newly created key.
 */
INT UInterpTrackFloatBase::DuplicateKeyframe(INT KeyIndex, FLOAT NewKeyTime)
{
	if( KeyIndex < 0 || KeyIndex >= FloatTrack.Points.Num() )
		return INDEX_NONE;

	FInterpCurvePoint<FLOAT> FloatPoint = FloatTrack.Points(KeyIndex);
	INT NewKeyIndex = FloatTrack.AddPoint(NewKeyTime, 0.f);
	FloatTrack.Points(NewKeyIndex) = FloatPoint; // Copy properties from source key.
	FloatTrack.Points(NewKeyIndex).InVal = NewKeyTime;

	FloatTrack.AutoSetTangents(CurveTension);

	return NewKeyIndex;
}

/** Return the closest time to the time passed in that we might want to snap to. */
UBOOL UInterpTrackFloatBase::GetClosestSnapPosition(FLOAT InPosition, TArray<INT> &IgnoreKeys, FLOAT& OutPosition)
{
	if(FloatTrack.Points.Num() == 0)
	{
		return false;
	}

	UBOOL bFoundSnap = false;
	FLOAT ClosestSnap = 0.f;
	FLOAT ClosestDist = BIG_NUMBER;
	for(INT i=0; i<FloatTrack.Points.Num(); i++)
	{
		if(!IgnoreKeys.ContainsItem(i))
		{
			FLOAT Dist = Abs( FloatTrack.Points(i).InVal - InPosition );
			if(Dist < ClosestDist)
			{
				ClosestSnap = FloatTrack.Points(i).InVal;
				ClosestDist = Dist;
				bFoundSnap = true;
			}
		}
	}

	OutPosition = ClosestSnap;
	return bFoundSnap;
}

void UInterpTrackFloatBase::PostEditChange(UProperty* PropertyThatChanged)
{
	FloatTrack.AutoSetTangents(CurveTension);
	Super::PostEditChange(PropertyThatChanged);
}

/*-----------------------------------------------------------------------------
	UInterpTrackToggle
-----------------------------------------------------------------------------*/

STRUCTTRACK_GETNUMKEYFRAMES(UInterpTrackToggle, ToggleTrack)
STRUCTTRACK_GETTIMERANGE(UInterpTrackToggle, ToggleTrack, Time)
STRUCTTRACK_GETKEYFRAMETIME(UInterpTrackToggle, ToggleTrack, Time)
STRUCTTRACK_SETKEYFRAMETIME(UInterpTrackToggle, ToggleTrack, Time, FToggleTrackKey)
STRUCTTRACK_REMOVEKEYFRAME(UInterpTrackToggle, ToggleTrack)
STRUCTTRACK_DUPLICATEKEYFRAME(UInterpTrackToggle, ToggleTrack, Time, FToggleTrackKey)
STRUCTTRACK_GETCLOSESTSNAPPOSITION(UInterpTrackToggle, ToggleTrack, Time)

// InterpTrack interface
INT UInterpTrackToggle::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	UInterpTrackInstToggle* ToggleInst = CastChecked<UInterpTrackInstToggle>(TrInst);

	INT i = 0;
	for (i = 0; i < ToggleTrack.Num() && ToggleTrack(i).Time < Time; i++);
	ToggleTrack.Insert(i);
	ToggleTrack(i).Time = Time;
	ToggleTrack(i).ToggleAction = ToggleInst->Action;

	return i;
}

void UInterpTrackToggle::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	AActor* Actor = TrInst->GetGroupActor();
	if (Actor == NULL)
	{
		return;
	}

	UInterpTrackInstToggle* ToggleInst = CastChecked<UInterpTrackInstToggle>(TrInst);
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( ToggleInst->GetOuter() );
	USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );
	UInterpGroup* Group = CastChecked<UInterpGroup>( GetOuter() );
	UInterpData* IData = CastChecked<UInterpData>( Group->GetOuter() );



	AEmitter* EmitterActor = Cast<AEmitter>(Actor);
	if( EmitterActor && bActivateSystemEachUpdate )
	{
		// @todo: Deprecate this legacy particle track behavior!  It doesn't support playing skipped events,
		//        and it doesn't support network synchronization!
		if ((NewPosition > ToggleInst->LastUpdatePosition)  && !bJump)
		{
			for (INT KeyIndex = ToggleTrack.Num() - 1; KeyIndex >= 0; KeyIndex--)
			{
				FToggleTrackKey& ToggleKey = ToggleTrack(KeyIndex);
				// We have found the key to the left of the position
				if (ToggleKey.ToggleAction == ETTA_On)
				{
					EmitterActor->ParticleSystemComponent->ActivateSystem();
				}
				else
				if (ToggleKey.ToggleAction == ETTA_Trigger)
				{
					if (ToggleKey.Time >= ToggleInst->LastUpdatePosition)
					{
						EmitterActor->ParticleSystemComponent->SetActive(TRUE);
					}
				}
				else
				{
					EmitterActor->ParticleSystemComponent->DeactivateSystem();
				}
				break;
			}
		}
	}
	else
	{
		// This is the normal pathway for toggle tracks.  It supports firing toggle events
		// even when jummping forward in time (skipping a cutscene.)

		// NOTE: We don't fire events when jumping forwards in Matinee preview since that would
		//       fire off particles while scrubbing, which we currently don't want.
		const UBOOL bShouldActuallyFireEventsWhenJumpingForwards =
			bFireEventsWhenJumpingForwards && !( GIsEditor && !GWorld->HasBegunPlay() );

		// @todo: Make this configurable?
		const UBOOL bInvertBoolLogicWhenPlayingBackwards = TRUE;

		// Only allow triggers to play when jumping when scrubbing in editor's Matinee preview.  We
		// never want to allow this in game, since this could cause many particles to fire off
		// when a cinematic is skipped (as we "jump" to the end point)
		const UBOOL bPlayTriggersWhenJumping = GIsEditor && !GWorld->HasBegunPlay();


		// We'll consider playing events in reverse if we're either actively playing in reverse or if
		// we're in a paused state but forcing an update to an older position (scrubbing backwards in editor.)
		UBOOL bIsPlayingBackwards =
			( Seq->bIsPlaying && Seq->bReversePlayback ) ||
			( bJump && !Seq->bIsPlaying && NewPosition < ToggleInst->LastUpdatePosition );


		// Find the interval between last update and this to check events with.
		UBOOL bFireEvents = TRUE;


		if( bJump )
		{
			// If we are playing forwards, and the flag is set, fire events even if we are 'jumping'.
			if( bShouldActuallyFireEventsWhenJumpingForwards && !bIsPlayingBackwards )
			{
				bFireEvents = TRUE;
			}
			else
			{
				bFireEvents = FALSE;
			}
		}

		// If playing sequence forwards.
		FLOAT MinTime, MaxTime;
		if( !bIsPlayingBackwards )
		{
			MinTime = ToggleInst->LastUpdatePosition;
			MaxTime = NewPosition;

			// Slight hack here.. if playing forwards and reaching the end of the sequence, force it over a little to ensure we fire events actually on the end of the sequence.
			if( MaxTime == IData->InterpLength )
			{
				MaxTime += (FLOAT)KINDA_SMALL_NUMBER;
			}

			if( !bFireEventsWhenForwards )
			{
				bFireEvents = FALSE;
			}
		}
		// If playing sequence backwards.
		else
		{
			MinTime = NewPosition;
			MaxTime = ToggleInst->LastUpdatePosition;

			// Same small hack as above for backwards case.
			if( MinTime == 0.0f )
			{
				MinTime -= (FLOAT)KINDA_SMALL_NUMBER;
			}

			if( !bFireEventsWhenBackwards )
			{
				bFireEvents = FALSE;
			}
		}


		// If we should be firing events for this track...
		if( bFireEvents )
		{
			// See which events fall into traversed region.
			INT KeyIndexToPlay = INDEX_NONE;
			for(INT CurKeyIndex = 0; CurKeyIndex < ToggleTrack.Num(); ++CurKeyIndex )
			{
				FToggleTrackKey& ToggleKey = ToggleTrack( CurKeyIndex );

				FLOAT EventTime = ToggleKey.Time;

				// Need to be slightly careful here and make behavior for firing events symmetric when playing forwards of backwards.
				UBOOL bFireThisEvent = FALSE;
				if( !bIsPlayingBackwards )
				{
					if( EventTime >= MinTime && EventTime < MaxTime )
					{
						bFireThisEvent = TRUE;
					}
				}
				else
				{
					if( EventTime > MinTime && EventTime <= MaxTime )
					{
						bFireThisEvent = TRUE;
					}
				}

				if( bFireThisEvent )
				{
					// Check for "fire and forget" events that must always be played
					if (ToggleKey.ToggleAction == ETTA_Trigger)
					{
						// Don't play triggers when jumping forward unless we're configured to do that
						if( bPlayTriggersWhenJumping || !bJump )
						{
							if( EmitterActor )
							{
								// Use ActivateSystem as multiple triggers should fire it multiple times.
								EmitterActor->ParticleSystemComponent->ActivateSystem();
								// don't set bCurrentlyActive (assume it's a one shot effect which the client will perform through its own matinee simulation)
							}
						}
					}
					else
					{
						// The idea here is that there's no point in playing multiple bool-style events in a
						// single frame, so we skip over events to find the most relevant.
						if( KeyIndexToPlay == INDEX_NONE ||
							( !bIsPlayingBackwards && CurKeyIndex > KeyIndexToPlay ) ||
							( bIsPlayingBackwards && CurKeyIndex < KeyIndexToPlay ) )
						{
							// Found the key we want to play!  
							KeyIndexToPlay = CurKeyIndex;
						}
					}
				}
			}

			if( KeyIndexToPlay != INDEX_NONE )
			{
				FToggleTrackKey& ToggleKey = ToggleTrack( KeyIndexToPlay );

				// Trigger keys should have been handled earlier!
				check( ToggleKey.ToggleAction != ETTA_Trigger );

				ALensFlareSource* LensFlareActor = Cast<ALensFlareSource>(Actor);
				ALight* LightActor = Cast<ALight>(Actor);

				if( EmitterActor )
				{
					UBOOL bShouldActivate = (ToggleKey.ToggleAction == ETTA_On);
					if( bInvertBoolLogicWhenPlayingBackwards && bIsPlayingBackwards )
					{
						// Playing in reverse, so invert bool logic
						bShouldActivate = !bShouldActivate;
					}

					EmitterActor->ParticleSystemComponent->SetActive( bShouldActivate );
					EmitterActor->bCurrentlyActive = bShouldActivate;
					EmitterActor->bNetDirty = TRUE;
					EmitterActor->eventForceNetRelevant();
				}
				else if( LensFlareActor && LensFlareActor->LensFlareComp )
				{
					UBOOL bShouldActivate = (ToggleKey.ToggleAction == ETTA_On);
					if( bInvertBoolLogicWhenPlayingBackwards && bIsPlayingBackwards )
					{
						// Playing in reverse, so invert bool logic
						bShouldActivate = !bShouldActivate;
					}

					LensFlareActor->LensFlareComp->SetIsActive( bShouldActivate );
				}
				else if( LightActor != NULL )
				{
					// We'll only allow *toggleable* lights to be toggled like this!  Static lights are ignored.
					if( LightActor->IsToggleable() )
					{
						UBOOL bShouldActivate = (ToggleKey.ToggleAction == ETTA_On);
						if( bInvertBoolLogicWhenPlayingBackwards && bIsPlayingBackwards )
						{
							// Playing in reverse, so invert bool logic
							bShouldActivate = !bShouldActivate;
						}

						LightActor->LightComponent->SetEnabled( bShouldActivate );
					}
				}
			}
		}
	}

	ToggleInst->LastUpdatePosition = NewPosition;
}

/** 
 *	Function which actually updates things based on the new position in the track. 
 *  This is called in the editor, when scrubbing/previewing etc.
 */
void UInterpTrackToggle::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( TrInst->GetOuter() );
	USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );

	// Dont play sounds unless we are preview playback (ie not scrubbing).
	UBOOL bJump = !(Seq->bIsPlaying);
	UpdateTrack(NewPosition, TrInst, bJump);
}

/** 
 *	Get the name of the class used to help out when adding tracks, keys, etc. in UnrealEd.
 *
 *	@return		String name of the helper class.
 */
const FString UInterpTrackToggle::GetEdHelperClassName() const
{
	return FString( TEXT("UnrealEd.InterpTrackToggleHelper") );
}

/*-----------------------------------------------------------------------------
	UInterpTrackVectorBase
-----------------------------------------------------------------------------*/

/** Total number of keyframes in this track. */
INT UInterpTrackVectorBase::GetNumKeyframes()
{
	return VectorTrack.Points.Num();
}

/** Get first and last time of keyframes in this track. */
void UInterpTrackVectorBase::GetTimeRange(FLOAT& StartTime, FLOAT& EndTime)
{
	if(VectorTrack.Points.Num() == 0)
	{
		StartTime = 0.f;
		EndTime = 0.f;
	}
	else
	{
		StartTime = VectorTrack.Points(0).InVal;
		EndTime = VectorTrack.Points( VectorTrack.Points.Num()-1 ).InVal;
	}
}

/** Get the time of the keyframe with the given index. */
FLOAT UInterpTrackVectorBase::GetKeyframeTime(INT KeyIndex)
{
	if( KeyIndex < 0 || KeyIndex >= VectorTrack.Points.Num() )
		return 0.f;

	return VectorTrack.Points(KeyIndex).InVal;
}

/** Change the time position of an existing keyframe. This can change the index of the keyframe - the new index is returned. */
INT UInterpTrackVectorBase::SetKeyframeTime(INT KeyIndex, FLOAT NewKeyTime, UBOOL bUpdateOrder)
{
	if( KeyIndex < 0 || KeyIndex >= VectorTrack.Points.Num() )
		return KeyIndex;

	INT NewKeyIndex = KeyIndex;
	if(bUpdateOrder)
	{
		NewKeyIndex = VectorTrack.MovePoint(KeyIndex, NewKeyTime);
	}
	else
	{
		VectorTrack.Points(KeyIndex).InVal = NewKeyTime;
	}

	VectorTrack.AutoSetTangents(CurveTension);

	return NewKeyIndex;
}

/** Remove the keyframe with the given index. */
void UInterpTrackVectorBase::RemoveKeyframe(INT KeyIndex)
{
	if( KeyIndex < 0 || KeyIndex >= VectorTrack.Points.Num() )
		return;

	VectorTrack.Points.Remove(KeyIndex);

	VectorTrack.AutoSetTangents(CurveTension);
}


/** 
 *	Duplicate the keyframe with the given index to the specified time. 
 *	Returns the index of the newly created key.
 */
INT UInterpTrackVectorBase::DuplicateKeyframe(INT KeyIndex, FLOAT NewKeyTime)
{
	if( KeyIndex < 0 || KeyIndex >= VectorTrack.Points.Num() )
		return INDEX_NONE;

	FInterpCurvePoint<FVector> VectorPoint = VectorTrack.Points(KeyIndex);
	INT NewKeyIndex = VectorTrack.AddPoint(NewKeyTime, FVector(0.f));
	VectorTrack.Points(NewKeyIndex) = VectorPoint; // Copy properties from source key.
	VectorTrack.Points(NewKeyIndex).InVal = NewKeyTime;

	VectorTrack.AutoSetTangents(CurveTension);

	return NewKeyIndex;
}

/** Return the closest time to the time passed in that we might want to snap to. */
UBOOL UInterpTrackVectorBase::GetClosestSnapPosition(FLOAT InPosition, TArray<INT> &IgnoreKeys, FLOAT& OutPosition)
{
	if(VectorTrack.Points.Num() == 0)
	{
		return false;
	}

	UBOOL bFoundSnap = false;
	FLOAT ClosestSnap = 0.f;
	FLOAT ClosestDist = BIG_NUMBER;
	for(INT i=0; i<VectorTrack.Points.Num(); i++)
	{
		if(!IgnoreKeys.ContainsItem(i))
		{
			FLOAT Dist = Abs( VectorTrack.Points(i).InVal - InPosition );
			if(Dist < ClosestDist)
			{
				ClosestSnap = VectorTrack.Points(i).InVal;
				ClosestDist = Dist;
				bFoundSnap = true;
			}
		}
	}

	OutPosition = ClosestSnap;
	return bFoundSnap;
}

void UInterpTrackVectorBase::PostEditChange(UProperty* PropertyThatChanged)
{
	VectorTrack.AutoSetTangents(CurveTension);
	Super::PostEditChange(PropertyThatChanged);
}

/*-----------------------------------------------------------------------------
	UInterpTrackFloatProp
-----------------------------------------------------------------------------*/

/** Add a new keyframe at the speicifed time. Returns index of new keyframe. */
INT UInterpTrackFloatProp::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	UInterpTrackInstFloatProp* PropInst = CastChecked<UInterpTrackInstFloatProp>(TrInst);
	if( !PropInst->FloatProp )
		return INDEX_NONE;

	INT NewKeyIndex = FloatTrack.AddPoint( Time, 0.f );
	FloatTrack.Points(NewKeyIndex).InterpMode = InitInterpMode;

	UpdateKeyframe(NewKeyIndex, TrInst);

	FloatTrack.AutoSetTangents(CurveTension);

	return NewKeyIndex;
}

/** Change the value of an existing keyframe. */
void UInterpTrackFloatProp::UpdateKeyframe(INT KeyIndex, UInterpTrackInst* TrInst)
{
	UInterpTrackInstFloatProp* PropInst = CastChecked<UInterpTrackInstFloatProp>(TrInst);
	if( !PropInst->FloatProp )
		return;

	if( KeyIndex < 0 || KeyIndex >= FloatTrack.Points.Num() )
		return;

	FloatTrack.Points(KeyIndex).OutVal = *((FLOAT*)(PropInst->FloatProp));

	FloatTrack.AutoSetTangents(CurveTension);
}


/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the editor, when scrubbing/previewing etc.
*/
void UInterpTrackFloatProp::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
	UpdateTrack(NewPosition, TrInst, FALSE);
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the game, when USeqAct_Interp is ticked
*  @param bJump	Indicates if this is a sudden jump instead of a smooth move to the new position.
*/
void UInterpTrackFloatProp::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	AActor* Actor = TrInst->GetGroupActor();
	if(!Actor)
	{
		return;
	}

	UInterpTrackInstFloatProp* PropInst = CastChecked<UInterpTrackInstFloatProp>(TrInst);
	if(!PropInst->FloatProp)
		return;

	FLOAT NewFloatValue = FloatTrack.Eval( NewPosition, *((FLOAT*)(PropInst->FloatProp)) );
	*((FLOAT*)(PropInst->FloatProp)) = NewFloatValue;

	// If we have a custom callback for this property, call that, otherwise just force update components on the actor.
	if(PropInst->CallPropertyUpdateCallback()==FALSE)
	{
		// We update components, so things like draw scale take effect.
		Actor->ForceUpdateComponents(FALSE,FALSE);
	}
}

/** Get the name of the class used to help out when adding tracks, keys, etc. in UnrealEd.
* @return	String name of the helper class.*/
const FString	UInterpTrackFloatProp::GetEdHelperClassName() const
{
	return FString( TEXT("UnrealEd.InterpTrackFloatPropHelper") );
}


/*-----------------------------------------------------------------------------
  UInterpTrackInstFloatProp
-----------------------------------------------------------------------------*/

/** Called before Interp editing to put object back to its original state. */
void UInterpTrackInstFloatProp::SaveActorState(UInterpTrack* Track)
{
	AActor* Actor = GetGroupActor();
	if(!Actor)
	{
		return;
	}

	if(!FloatProp)
		return;

	// Remember current value of property for when we quite Matinee
	ResetFloat = *((FLOAT*)(FloatProp));
}

/** Restore the saved state of this Actor. */
void UInterpTrackInstFloatProp::RestoreActorState(UInterpTrack* Track)
{
	AActor* Actor = GetGroupActor();
	if(!Actor)
	{
		return;
	}

	if(!FloatProp)
		return;

	// Restore original value of property
	*((FLOAT*)(FloatProp)) = ResetFloat;

	// We update components, so things like draw scale take effect.
	// Don't force update all components unless we're in the editor.
	Actor->ConditionalForceUpdateComponents(FALSE,FALSE);
}

/** Initialise this Track instance. Called in-game before doing any interpolation. */
void UInterpTrackInstFloatProp::InitTrackInst(UInterpTrack* Track)
{
	AActor* Actor = GetGroupActor();
	if(!Actor)
	{
		return;
	}

	// Store a pointer to the float data for the property we will be interpolating.
	UInterpTrackFloatProp* TrackProp = Cast<UInterpTrackFloatProp>(Track);
	FloatProp = Actor->GetInterpFloatPropertyRef(TrackProp->PropertyName);

	SetupPropertyUpdateCallback(Actor, TrackProp->PropertyName);
}

/*-----------------------------------------------------------------------------
	UInterpTrackVectorProp
-----------------------------------------------------------------------------*/

/** Add a new keyframe at the speicifed time. Returns index of new keyframe. */
INT UInterpTrackVectorProp::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	UInterpTrackInstVectorProp* PropInst = CastChecked<UInterpTrackInstVectorProp>(TrInst);
	if( !PropInst->VectorProp )
		return INDEX_NONE;

	INT NewKeyIndex = VectorTrack.AddPoint( Time, FVector(0.f));
	VectorTrack.Points(NewKeyIndex).InterpMode = InitInterpMode;

	UpdateKeyframe(NewKeyIndex, TrInst);

	VectorTrack.AutoSetTangents(CurveTension);

	return NewKeyIndex;
}

/** Change the value of an existing keyframe. */
void UInterpTrackVectorProp::UpdateKeyframe(INT KeyIndex, UInterpTrackInst* TrInst)
{
	UInterpTrackInstVectorProp* PropInst = CastChecked<UInterpTrackInstVectorProp>(TrInst);
	if( !PropInst->VectorProp )
		return;

	if( KeyIndex < 0 || KeyIndex >= VectorTrack.Points.Num() )
		return;

	VectorTrack.Points(KeyIndex).OutVal = *((FVector*)(PropInst->VectorProp));

	VectorTrack.AutoSetTangents(CurveTension);
}


/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the editor, when scrubbing/previewing etc.
*/
void UInterpTrackVectorProp::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
	UpdateTrack(NewPosition, TrInst, FALSE);
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the game, when USeqAct_Interp is ticked
*  @param bJump	Indicates if this is a sudden jump instead of a smooth move to the new position.
*/
void UInterpTrackVectorProp::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	AActor* Actor = TrInst->GetGroupActor();
	if(!Actor)
	{
		return;
	}

	UInterpTrackInstVectorProp* PropInst = CastChecked<UInterpTrackInstVectorProp>(TrInst);
	if(!PropInst->VectorProp)
		return;

	FVector NewVectorValue = VectorTrack.Eval( NewPosition, *((FVector*)(PropInst->VectorProp)) );
	*((FVector*)(PropInst->VectorProp)) = NewVectorValue;

	// If we have a custom callback for this property, call that, otherwise just force update components on the actor.
	if(PropInst->CallPropertyUpdateCallback()==FALSE)
	{
		// We update components, so things like draw scale take effect.
		Actor->ForceUpdateComponents(FALSE,FALSE);
	}
}

/** Get the name of the class used to help out when adding tracks, keys, etc. in UnrealEd.
* @return	String name of the helper class.*/
const FString	UInterpTrackVectorProp::GetEdHelperClassName() const
{
	return FString( TEXT("UnrealEd.InterpTrackVectorPropHelper") );
}


/*-----------------------------------------------------------------------------
	UInterpTrackInstVectorProp
-----------------------------------------------------------------------------*/

/** Called before Interp editing to put object back to its original state. */
void UInterpTrackInstVectorProp::SaveActorState(UInterpTrack* Track)
{
	AActor* Actor = GetGroupActor();
	if(!Actor)
	{
		return;
	}

	if(!VectorProp)
		return;

	// Remember current value of property for when we quite Matinee
	ResetVector = *((FVector*)(VectorProp));
}

/** Restore the saved state of this Actor. */
void UInterpTrackInstVectorProp::RestoreActorState(UInterpTrack* Track)
{
	AActor* Actor = GetGroupActor();
	if(!Actor)
	{
		return;
	}

	if(!VectorProp)
		return;

	// Restore original value of property
	*((FVector*)(VectorProp)) = ResetVector;

	// We update components, so things like draw scale take effect.
	// Don't force update all components unless we're in the editor.
	Actor->ConditionalForceUpdateComponents(FALSE,FALSE);
}

/** Initialise this Track instance. Called in-game before doing any interpolation. */
void UInterpTrackInstVectorProp::InitTrackInst(UInterpTrack* Track)
{
	AActor* Actor = GetGroupActor();
	if(!Actor)
	{
		return;
	}

	UInterpTrackVectorProp* TrackProp = Cast<UInterpTrackVectorProp>(Track);

	VectorProp = Actor->GetInterpVectorPropertyRef(TrackProp->PropertyName);

	SetupPropertyUpdateCallback(Actor, TrackProp->PropertyName);
}



/*-----------------------------------------------------------------------------
	UInterpTrackColorProp
-----------------------------------------------------------------------------*/

/** Add a new keyframe at the speicifed time. Returns index of new keyframe. */
INT UInterpTrackColorProp::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	UInterpTrackInstColorProp* PropInst = CastChecked<UInterpTrackInstColorProp>(TrInst);
	if( !PropInst->ColorProp )
		return INDEX_NONE;

	INT NewKeyIndex = VectorTrack.AddPoint( Time, FVector(0.f));
	VectorTrack.Points(NewKeyIndex).InterpMode = InitInterpMode;

	UpdateKeyframe(NewKeyIndex, TrInst);

	VectorTrack.AutoSetTangents(CurveTension);

	return NewKeyIndex;
}

/** Change the value of an existing keyframe. */
void UInterpTrackColorProp::UpdateKeyframe(INT KeyIndex, UInterpTrackInst* TrInst)
{
	UInterpTrackInstColorProp* PropInst = CastChecked<UInterpTrackInstColorProp>(TrInst);
	if( !PropInst->ColorProp )
	{
		return;
	}

	if( KeyIndex < 0 || KeyIndex >= VectorTrack.Points.Num() )
	{
		return;
	}

	FColor ColorValue = *((FColor*)(PropInst->ColorProp));
	FLinearColor LinearValue(ColorValue);
	VectorTrack.Points(KeyIndex).OutVal = FVector(LinearValue.R, LinearValue.G, LinearValue.B);

	VectorTrack.AutoSetTangents(CurveTension);
}


/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the editor, when scrubbing/previewing etc.
*/
void UInterpTrackColorProp::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
	UpdateTrack(NewPosition, TrInst, FALSE);
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the game, when USeqAct_Interp is ticked
*  @param bJump	Indicates if this is a sudden jump instead of a smooth move to the new position.
*/
void UInterpTrackColorProp::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	AActor* Actor = TrInst->GetGroupActor();
	if(!Actor)
	{
		return;
	}

	UInterpTrackInstColorProp* PropInst = CastChecked<UInterpTrackInstColorProp>(TrInst);
	if(!PropInst->ColorProp)
	{
		return;
	}

	FVector NewVectorValue = VectorTrack.Eval( NewPosition, *((FVector*)(PropInst->ColorProp)) );
	FColor NewColorValue = FLinearColor(NewVectorValue.X, NewVectorValue.Y, NewVectorValue.Z);
	*((FColor*)(PropInst->ColorProp)) = NewColorValue;

	// If we have a custom callback for this property, call that, otherwise just force update components on the actor.
	if(PropInst->CallPropertyUpdateCallback()==FALSE)
	{
		// We update components, so things like draw scale take effect.
		Actor->ForceUpdateComponents(FALSE,FALSE);
	}
}

/** Get the name of the class used to help out when adding tracks, keys, etc. in UnrealEd.
* @return	String name of the helper class.*/
const FString	UInterpTrackColorProp::GetEdHelperClassName() const
{
	return FString( TEXT("UnrealEd.InterpTrackColorPropHelper") );
}



/*-----------------------------------------------------------------------------
UInterpTrackInstColorProp
-----------------------------------------------------------------------------*/

/** Called before Interp editing to put object back to its original state. */
void UInterpTrackInstColorProp::SaveActorState(UInterpTrack* Track)
{
	AActor* Actor = GetGroupActor();
	if(!Actor)
	{
		return;
	}

	if(!ColorProp)
		return;

	// Remember current value of property for when we quite Matinee
	ResetColor = *((FColor*)(ColorProp));
}

/** Restore the saved state of this Actor. */
void UInterpTrackInstColorProp::RestoreActorState(UInterpTrack* Track)
{
	AActor* Actor = GetGroupActor();
	if(!Actor)
	{
		return;
	}

	if(!ColorProp)
		return;

	// Restore original value of property
	*((FColor*)(ColorProp)) = ResetColor;

	// We update components, so things like draw scale take effect.
	// Don't force update all components unless we're in the editor.
	Actor->ConditionalForceUpdateComponents(FALSE,FALSE);
}

/** Initialize this Track instance. Called in-game before doing any interpolation. */
void UInterpTrackInstColorProp::InitTrackInst(UInterpTrack* Track)
{
	AActor* Actor = GetGroupActor();
	if(!Actor)
	{
		return;
	}

	UInterpTrackColorProp* TrackProp = Cast<UInterpTrackColorProp>(Track);
	ColorProp = Actor->GetInterpColorPropertyRef(TrackProp->PropertyName);

	SetupPropertyUpdateCallback(Actor, TrackProp->PropertyName);
}

/*-----------------------------------------------------------------------------
	UInterpTrackEvent
-----------------------------------------------------------------------------*/

STRUCTTRACK_GETNUMKEYFRAMES(UInterpTrackEvent, EventTrack)
STRUCTTRACK_GETTIMERANGE(UInterpTrackEvent, EventTrack, Time)
STRUCTTRACK_GETKEYFRAMETIME(UInterpTrackEvent, EventTrack, Time)
STRUCTTRACK_SETKEYFRAMETIME(UInterpTrackEvent, EventTrack, Time, FEventTrackKey )
STRUCTTRACK_REMOVEKEYFRAME(UInterpTrackEvent, EventTrack )
STRUCTTRACK_DUPLICATEKEYFRAME(UInterpTrackEvent, EventTrack, Time, FEventTrackKey )
STRUCTTRACK_GETCLOSESTSNAPPOSITION(UInterpTrackEvent, EventTrack, Time)

/** Add a new keyframe at the speicifed time. Returns index of new keyframe. */
INT UInterpTrackEvent::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	FEventTrackKey NewEventKey;
	NewEventKey.EventName = NAME_None;
	NewEventKey.Time = Time;

	// Find the correct index to insert this key.
	INT i=0; for( i=0; i<EventTrack.Num() && EventTrack(i).Time < Time; i++);
	EventTrack.Insert(i);
	EventTrack(i) = NewEventKey;

	return i;
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the game, when USeqAct_Interp is ticked
*  @param bJump	Indicates if this is a sudden jump instead of a smooth move to the new position.
*/
void UInterpTrackEvent::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	UInterpTrackInstEvent* EventInst = CastChecked<UInterpTrackInstEvent>(TrInst);
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( EventInst->GetOuter() );
	USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );
	UInterpGroup* Group = CastChecked<UInterpGroup>( GetOuter() );
	UInterpData* IData = CastChecked<UInterpData>( Group->GetOuter() );

	// We'll consider playing events in reverse if we're either actively playing in reverse or if
	// we're in a paused state but forcing an update to an older position (scrubbing backwards in editor.)
	UBOOL bIsPlayingBackwards =
		( Seq->bIsPlaying && Seq->bReversePlayback ) ||
		( bJump && !Seq->bIsPlaying && NewPosition < EventInst->LastUpdatePosition );

	// Find the interval between last update and this to check events with.
	UBOOL bFireEvents = true;

	if(bJump)
	{
		// If we are playing forwards, and the flag is set, fire events even if we are 'jumping'.
		if(bFireEventsWhenJumpingForwards && !bIsPlayingBackwards)
		{
			bFireEvents = true;
		}
		else
		{
			bFireEvents = false;
		}
	}

	// If playing sequence forwards.
	FLOAT MinTime, MaxTime;
	if(!bIsPlayingBackwards)
	{
		MinTime = EventInst->LastUpdatePosition;
		MaxTime = NewPosition;

		// Slight hack here.. if playing forwards and reaching the end of the sequence, force it over a little to ensure we fire events actually on the end of the sequence.
		if(MaxTime == IData->InterpLength)
		{
			MaxTime += (FLOAT)KINDA_SMALL_NUMBER;
		}

		if(!bFireEventsWhenForwards)
		{
			bFireEvents = false;
		}
	}
	// If playing sequence backwards.
	else
	{
		MinTime = NewPosition;
		MaxTime = EventInst->LastUpdatePosition;

		// Same small hack as above for backwards case.
		if(MinTime == 0.f)
		{
			MinTime -= (FLOAT)KINDA_SMALL_NUMBER;
		}

		if(!bFireEventsWhenBackwards)
		{
			bFireEvents = false;
		}
	}

	// If we should be firing events for this track...
	if(bFireEvents)
	{
		// See which events fall into traversed region.
		for(INT i=0; i<EventTrack.Num(); i++)
		{
			FLOAT EventTime = EventTrack(i).Time;

			// Need to be slightly careful here and make behavior for firing events symmetric when playing forwards of backwards.
			UBOOL bFireThisEvent = false;
			if(!bIsPlayingBackwards)
			{
				if( EventTime >= MinTime && EventTime < MaxTime )
				{
					bFireThisEvent = true;
				}
			}
			else
			{
				if( EventTime > MinTime && EventTime <= MaxTime )
				{
					bFireThisEvent = true;
				}
			}

			if( bFireThisEvent )
			{
				Seq->NotifyEventTriggered(this, i);
			}
		}
	}

	// Update LastUpdatePosition.
	EventInst->LastUpdatePosition = NewPosition;
}

/** Get the name of the class used to help out when adding tracks, keys, etc. in UnrealEd.
* @return	String name of the helper class.*/
const FString	UInterpTrackEvent::GetEdHelperClassName() const
{
	return FString( TEXT("UnrealEd.InterpTrackEventHelper") );
}

/*-----------------------------------------------------------------------------
	UInterpTrackInstEvent
-----------------------------------------------------------------------------*/

/** Initialise this Track instance. Called in-game before doing any interpolation. */
void UInterpTrackInstEvent::InitTrackInst(UInterpTrack* Track)
{
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( GetOuter() );
	USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );

	LastUpdatePosition = Seq->Position;
}


/*-----------------------------------------------------------------------------
	UInterpTrackDirector
-----------------------------------------------------------------------------*/

STRUCTTRACK_GETNUMKEYFRAMES(UInterpTrackDirector, CutTrack)
STRUCTTRACK_GETTIMERANGE(UInterpTrackDirector, CutTrack, Time)
STRUCTTRACK_GETKEYFRAMETIME(UInterpTrackDirector, CutTrack, Time)
STRUCTTRACK_SETKEYFRAMETIME(UInterpTrackDirector, CutTrack, Time, FDirectorTrackCut )
STRUCTTRACK_REMOVEKEYFRAME(UInterpTrackDirector, CutTrack )
STRUCTTRACK_DUPLICATEKEYFRAME(UInterpTrackDirector, CutTrack, Time, FDirectorTrackCut )
STRUCTTRACK_GETCLOSESTSNAPPOSITION(UInterpTrackDirector, CutTrack, Time)

/** Add a new keyframe at the speicifed time. Returns index of new keyframe. */
INT UInterpTrackDirector::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	FDirectorTrackCut NewCut;
	NewCut.TargetCamGroup = NAME_None;
	NewCut.TransitionTime = 0.f;
	NewCut.Time = Time;

	// Find the correct index to insert this cut.
	INT i=0; for( i=0; i<CutTrack.Num() && CutTrack(i).Time < Time; i++);
	CutTrack.Insert(i);
	CutTrack(i) = NewCut;

	return i;
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the game, when USeqAct_Interp is ticked
*  @param bJump	Indicates if this is a sudden jump instead of a smooth move to the new position.
*/
void UInterpTrackDirector::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	UInterpTrackInstDirector* DirInst = CastChecked<UInterpTrackInstDirector>(TrInst);
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( TrInst->GetOuter() );

	// Actor for a Director group should be a PlayerController.
	APlayerController* PC = Cast<APlayerController>(GrInst->GetGroupActor());
	if( PC )
	{
		USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );
		// server is authoritative on viewtarget changes
		if (PC->Role == ROLE_Authority || Seq->bClientSideOnly || bSimulateCameraCutsOnClients)
		{
			FLOAT CutTime, CutTransitionTime;
			FName ViewGroupName = GetViewedGroupName(NewPosition, CutTime, CutTransitionTime);
			// if our group name was specified, make sure we use ourselves instead of any other instances with that name (there might be multiple in the multiplayer case)
			UInterpGroupInst* ViewGroupInst = (ViewGroupName == GrInst->Group->GroupName) ? GrInst : Seq->FindFirstGroupInstByName(ViewGroupName);
			
			AActor* ViewTarget = PC->GetViewTarget();
			if( ViewGroupInst && ViewGroupInst->GetGroupActor() && (ViewGroupInst->GetGroupActor() != PC) )
			{
				// If our desired view target is different from our current one...
				if( ViewTarget != ViewGroupInst->GroupActor )
				{
					// If we don't have a backed up ViewTarget, back up this one.
					if( !DirInst->OldViewTarget )
					{
						// If the actor's current view target is a director track camera, then we want to store 
						// the director track's 'old view target' in case the current Matinee sequence finishes
						// before our's does.
						UInterpTrackInstDirector* PreviousDirInst = PC->GetControllingDirector();
						if( PreviousDirInst != NULL && PreviousDirInst->OldViewTarget != NULL )
						{
							// Store the underlying director track's old view target so we can restore this later
							DirInst->OldViewTarget = PreviousDirInst->OldViewTarget;
						}
						else
						{
							DirInst->OldViewTarget = ViewTarget;
						}

						PC->bClientSimulatingViewTarget = bSimulateCameraCutsOnClients;
						PC->SetControllingDirector( DirInst );
						PC->eventNotifyDirectorControl(TRUE);
					}

					//debugf(TEXT("UInterpTrackDirector::UpdateTrack SetViewTarget ViewGroupInst->GroupActor Time:%f Name: %s"), GWorld->GetTimeSeconds(), *ViewGroupInst->GroupActor->GetFName());
					// Change view to desired view target.
					FViewTargetTransitionParams TransitionParams(EC_EventParm);
					TransitionParams.BlendTime = CutTransitionTime;

					// a bit ugly here, but we don't want this particular SetViewTarget to bash OldViewTarget
					AActor* const BackupViewTarget = DirInst->OldViewTarget;
					PC->SetViewTarget( ViewGroupInst->GroupActor, TransitionParams);
					DirInst->OldViewTarget = BackupViewTarget;
				}
			}
			// If assigning to nothing or the PlayerController, restore any backed up viewtarget.
			else if (DirInst->OldViewTarget != NULL)
			{
				//debugf(TEXT("UInterpTrackDirector::UpdateTrack SetViewTarget DirInst->OldViewTarget Time:%f Name: %s"), GWorld->GetTimeSeconds(), *DirInst->OldViewTarget->GetFName());
				if (!DirInst->OldViewTarget->IsPendingKill())
				{
					FViewTargetTransitionParams TransitionParams(EC_EventParm);
					TransitionParams.BlendTime = CutTransitionTime;
					PC->SetViewTarget( DirInst->OldViewTarget, TransitionParams );
				}
				PC->eventNotifyDirectorControl(FALSE);
				PC->SetControllingDirector( NULL );
				PC->bClientSimulatingViewTarget = FALSE;
				DirInst->OldViewTarget = NULL;
			}
		}
	}
}

/** For the supplied time, find which group name we should be viewing from. */
FName UInterpTrackDirector::GetViewedGroupName(FLOAT CurrentTime, FLOAT& CutTime, FLOAT& CutTransitionTime)
{
	INT PrevKeyIndex = INDEX_NONE; // Index of key before current time.

	if(CutTrack.Num() > 0 && CutTrack(0).Time < CurrentTime)
	{
		for( INT i=0; i < CutTrack.Num() && CutTrack(i).Time <= CurrentTime; i++)
		{
			PrevKeyIndex = i;
		}
	}

	// If no index found - we are before first frame (or no frames present), so use the director group name.
	if(PrevKeyIndex == INDEX_NONE)
	{
		CutTime = 0.f;
		CutTransitionTime = 0.f;

		UInterpGroup* Group = CastChecked<UInterpGroup>( GetOuter() );
		return Group->GroupName;
	}
	else
	{
		CutTime = CutTrack(PrevKeyIndex).Time;
		CutTransitionTime = CutTrack(PrevKeyIndex).TransitionTime;

		return CutTrack(PrevKeyIndex).TargetCamGroup;
	}
}

/** Get the name of the class used to help out when adding tracks, keys, etc. in UnrealEd.
* @return	String name of the helper class.*/
const FString	UInterpTrackDirector::GetEdHelperClassName() const
{
	return FString( TEXT("UnrealEd.InterpTrackDirectorHelper") );
}

/*-----------------------------------------------------------------------------
	UInterpTrackInstDirector
-----------------------------------------------------------------------------*/

void UInterpTrackInstDirector::InitTrackInst(UInterpTrack* Track)
{
	Super::InitTrackInst(Track);
}

/** Use this to ensure we always cut back to the players last view-target. Need this in case there was no explicit cut back to the Dir track. */
void UInterpTrackInstDirector::TermTrackInst(UInterpTrack* Track)
{
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>(GetOuter());
	APlayerController* PC = Cast<APlayerController>(GrInst->GetGroupActor());
	if (PC != NULL)
	{
		if (OldViewTarget != NULL && !OldViewTarget->IsPendingKill())
		{
			// if we haven't already, restore original view target.
			AActor* ViewTarget = PC->GetViewTarget();
			if (ViewTarget != OldViewTarget)
			{
				PC->SetViewTarget(OldViewTarget);
			}
		}
		// this may be a duplicate call if it was already called in UpdateTrack(), but that's better than not at all and leaving code thinking we stayed in matinee forever
		PC->eventNotifyDirectorControl(FALSE);
		PC->SetControllingDirector( NULL );
		PC->bClientSimulatingViewTarget = FALSE;
	}
	
	OldViewTarget = NULL;
}


/*-----------------------------------------------------------------------------
	UInterpTrackFade
-----------------------------------------------------------------------------*/

/** Add a new keyframe at the speicifed time. Returns index of new keyframe. */
INT UInterpTrackFade::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	INT NewKeyIndex = FloatTrack.AddPoint( Time, 0.f );
	FloatTrack.Points(NewKeyIndex).InterpMode = InitInterpMode;

	FloatTrack.AutoSetTangents(CurveTension);

	return NewKeyIndex;
}

/** Change the value of an existing keyframe. */
void UInterpTrackFade::UpdateKeyframe(INT KeyIndex, UInterpTrackInst* TrInst)
{
	// Do nothing here - fading is all set up through curve editor.
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the editor, when scrubbing/previewing etc.
*/
void UInterpTrackFade::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
	// Do nothing - in the editor Matinee itself handles updating the editor viewports.
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the game, when USeqAct_Interp is ticked
*  @param bJump	Indicates if this is a sudden jump instead of a smooth move to the new position.
*/
void UInterpTrackFade::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	// when doing a skip in game, don't update fading - we only want it applied when actually running
	if (!bJump || !GIsGame)
	{
		UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( TrInst->GetOuter() );

		// Actor for a Director group should be a PlayerController.
		APlayerController* PC = Cast<APlayerController>(GrInst->GetGroupActor());
		if(PC && PC->PlayerCamera && !PC->PlayerCamera->bDeleteMe)
		{
			PC->PlayerCamera->bEnableFading = true;
			PC->PlayerCamera->FadeAmount = GetFadeAmountAtTime(NewPosition);
			// disable the Kismet fade control so that we don't thrash
			PC->PlayerCamera->FadeTimeRemaining = 0.0f;
		}
	}
}

/** Return the amount of fading we want at the given time. */
FLOAT UInterpTrackFade::GetFadeAmountAtTime(FLOAT Time)
{
	FLOAT Fade = FloatTrack.Eval(Time, 0.f);
	Fade = Clamp(Fade, 0.f, 1.f);
	return Fade;
}

/*-----------------------------------------------------------------------------
	UInterpTrackInstFade
-----------------------------------------------------------------------------*/

/** Use this to turn off any fading that was applied by this track to this player controller. */
void UInterpTrackInstFade::TermTrackInst(UInterpTrack* Track)
{
	UInterpTrackFade *FadeTrack = Cast<UInterpTrackFade>(Track);
	if (FadeTrack == NULL || !FadeTrack->bPersistFade)
	{
		UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( GetOuter() );
		APlayerController* PC = Cast<APlayerController>(GrInst->GroupActor);
		if(PC && PC->PlayerCamera && !PC->PlayerCamera->bDeleteMe)
		{
			PC->PlayerCamera->bEnableFading = false;
			PC->PlayerCamera->FadeAmount = 0.f;
			// if the player is remote, ensure they got it
			// this handles cases where the LDs stream out this level immediately afterwards,
			// which can mean the client never gets the matinee replication if it was temporarily unresponsive
			if (!PC->LocalPlayerController())
			{
				PC->eventClientSetCameraFade(FALSE);
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	UInterpTrackSlomo
-----------------------------------------------------------------------------*/

/** Add a new keyframe at the speicifed time. Returns index of new keyframe. */
INT UInterpTrackSlomo::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	INT NewKeyIndex = FloatTrack.AddPoint( Time, 1.0f );
	FloatTrack.Points(NewKeyIndex).InterpMode = InitInterpMode;

	FloatTrack.AutoSetTangents(CurveTension);

	return NewKeyIndex;
}

/** Change the value of an existing keyframe. */
void UInterpTrackSlomo::UpdateKeyframe(INT KeyIndex, UInterpTrackInst* TrInst)
{
	// Do nothing here - slomo is all set up through curve editor.
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the editor, when scrubbing/previewing etc.
*/
void UInterpTrackSlomo::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
	// Do nothing - in the editor Matinee itself handles updating the editor viewports.
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the game, when USeqAct_Interp is ticked
*  @param bJump	Indicates if this is a sudden jump instead of a smooth move to the new position.
*/
void UInterpTrackSlomo::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	// do nothing if we're the client, as the server will replicate TimeDilation
	if (CastChecked<UInterpTrackInstSlomo>(TrInst)->ShouldBeApplied())
	{
		AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
		WorldInfo->TimeDilation = GetSlomoFactorAtTime(NewPosition);
		WorldInfo->bNetDirty = TRUE;
		WorldInfo->bForceNetUpdate = TRUE;
	}
}

/** Return the slomo factor we want at the given time. */
FLOAT UInterpTrackSlomo::GetSlomoFactorAtTime(FLOAT Time)
{
	FLOAT Slomo = FloatTrack.Eval(Time, 0.f);
	Slomo = ::Max(Slomo, 0.1f);
	return Slomo;
}

/** Set the slomo track to a default of one key at time zero and a slomo factor of 1.0 (ie no slomo) */
void UInterpTrackSlomo::SetTrackToSensibleDefault()
{
	FloatTrack.Points.Empty();
	FloatTrack.AddPoint(0.f, 1.f);
}	

/*-----------------------------------------------------------------------------
	UInterpTrackInstSlomo
-----------------------------------------------------------------------------*/

/** In editor, backup the LevelInfo->TimeDilation when opening Matinee. */
void UInterpTrackInstSlomo::SaveActorState(UInterpTrack* Track)
{
	OldTimeDilation = GWorld->GetWorldInfo()->TimeDilation;
}

/** In the editor, when we exit Matinee, restore levels TimeDilation to the backed-up value. */
void UInterpTrackInstSlomo::RestoreActorState(UInterpTrack* Track)
{
	GWorld->GetWorldInfo()->TimeDilation = OldTimeDilation;
}

UBOOL UInterpTrackInstSlomo::ShouldBeApplied()
{
	if (GIsEditor)
	{
		return TRUE;
	}
	else if (GWorld->GetWorldInfo()->NetMode == NM_Client)
	{
		return FALSE;
	}
	else
	{
		// if GroupActor is NULL, then this is the instance created on a dedicated server when no players were around
		// otherwise, check that GroupActor is the first player
		AActor* GroupActor = GetGroupActor();
		return (GroupActor == NULL || (GEngine != NULL && GEngine->GamePlayers.Num() > 0 && GEngine->GamePlayers(0) != NULL && GEngine->GamePlayers(0)->Actor == GroupActor));
	}
}

/** Remember the slomo factor applied when interpolation begins. */
void UInterpTrackInstSlomo::InitTrackInst(UInterpTrack* Track)
{
	// do nothing if we're the client, as the server will replicate TimeDilation
	if (ShouldBeApplied())
	{
		OldTimeDilation = GWorld->GetWorldInfo()->TimeDilation;
	}
}

/** Ensure the slomo factor is restored to what it was when interpolation begins. */
void UInterpTrackInstSlomo::TermTrackInst(UInterpTrack* Track)
{
	// do nothing if we're the client, as the server will replicate TimeDilation
	if (ShouldBeApplied())
	{
		AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
		if(OldTimeDilation <= 0.f)
		{
			warnf(TEXT("WARNING! OldTimeDilation was not initialized in %s!  Setting to 1.0f"),*GetPathName());
			OldTimeDilation = 1.0f;
		}
		WorldInfo->TimeDilation = OldTimeDilation;
		WorldInfo->bNetDirty = TRUE;
		WorldInfo->bForceNetUpdate = TRUE;
	}
}

/*-----------------------------------------------------------------------------
	UInterpTrackAnimControl
-----------------------------------------------------------------------------*/

void UInterpTrackAnimControl::PostLoad()
{
	Super::PostLoad();

	// Fix any anims with zero play rate.
	for(INT i=0; i<AnimSeqs.Num(); i++)
	{
		if(AnimSeqs(i).AnimPlayRate < 0.001f)
		{
			AnimSeqs(i).AnimPlayRate = 1.f;
		}
	}
}

STRUCTTRACK_GETNUMKEYFRAMES(UInterpTrackAnimControl, AnimSeqs)
STRUCTTRACK_GETTIMERANGE(UInterpTrackAnimControl, AnimSeqs, StartTime)
STRUCTTRACK_GETKEYFRAMETIME(UInterpTrackAnimControl, AnimSeqs, StartTime)
STRUCTTRACK_SETKEYFRAMETIME(UInterpTrackAnimControl, AnimSeqs, StartTime, FAnimControlTrackKey )
STRUCTTRACK_REMOVEKEYFRAME( UInterpTrackAnimControl, AnimSeqs )
STRUCTTRACK_DUPLICATEKEYFRAME( UInterpTrackAnimControl, AnimSeqs, StartTime, FAnimControlTrackKey )

/** Add a new keyframe at the speicifed time. Returns index of new keyframe. */
INT UInterpTrackAnimControl::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	FAnimControlTrackKey NewSeq;
	NewSeq.AnimSeqName = NAME_None;
	NewSeq.bLooping = false;
	NewSeq.AnimStartOffset = 0.f;
	NewSeq.AnimEndOffset = 0.f;
	NewSeq.AnimPlayRate = 1.f;
	NewSeq.StartTime = Time;
	NewSeq.bReverse = FALSE;

	// Find the correct index to insert this cut.
	INT i=0; for( i=0; i<AnimSeqs.Num() && AnimSeqs(i).StartTime < Time; i++);
	AnimSeqs.Insert(i);
	AnimSeqs(i) = NewSeq;

	return i;
}

/** Return the closest time to the time passed in that we might want to snap to. */
UBOOL UInterpTrackAnimControl::GetClosestSnapPosition(FLOAT InPosition, TArray<INT> &IgnoreKeys, FLOAT& OutPosition)
{
	if(AnimSeqs.Num() == 0)
	{
		return false;
	}

	UBOOL bFoundSnap = false;
	FLOAT ClosestSnap = 0.f;
	FLOAT ClosestDist = BIG_NUMBER;
	for(INT i=0; i<AnimSeqs.Num(); i++)
	{
		if(!IgnoreKeys.ContainsItem(i))
		{
			FLOAT SeqStartTime = AnimSeqs(i).StartTime;
			FLOAT SeqEndTime = SeqStartTime;

			FLOAT SeqLength = 0.f;
			UAnimSequence* Seq = FindAnimSequenceFromName(AnimSeqs(i).AnimSeqName);
			if(Seq)
			{
				SeqLength = ::Max((Seq->SequenceLength - (AnimSeqs(i).AnimStartOffset + AnimSeqs(i).AnimEndOffset)) / AnimSeqs(i).AnimPlayRate, 0.01f);
				SeqEndTime += SeqLength;
			}

			// If there is a sequence following this one - we stop drawing this block where the next one begins.
			if((i < AnimSeqs.Num()-1) && !IgnoreKeys.ContainsItem(i+1))
			{
				SeqEndTime = ::Min( AnimSeqs(i+1).StartTime, SeqEndTime );
			}

			FLOAT Dist = Abs( SeqStartTime - InPosition );
			if(Dist < ClosestDist)
			{
				ClosestSnap = SeqStartTime;
				ClosestDist = Dist;
				bFoundSnap = true;
			}

			Dist = Abs( SeqEndTime - InPosition );
			if(Dist < ClosestDist)
			{
				ClosestSnap = SeqEndTime;
				ClosestDist = Dist;
				bFoundSnap = true;
			}
		}
	}

	OutPosition = ClosestSnap;
	return bFoundSnap;
}

/** Return color to draw each keyframe in Matinee. */
FColor UInterpTrackAnimControl::GetKeyframeColor(INT KeyIndex)
{
	return FColor(0,0,0);
}

/** Find the AnimSequence with the given name in the set of AnimSets defined in this AnimControl track. */
UAnimSequence* UInterpTrackAnimControl::FindAnimSequenceFromName(FName InName)
{
	if(InName == NAME_None)
	{
		return NULL;
	}

	UInterpGroup* Group = CastChecked<UInterpGroup>(GetOuter());

	// Work from last element in list backwards, so you can replace a specific sequence by adding a set later in the array.
	for(INT i=Group->GroupAnimSets.Num()-1; i>=0; i--)
	{
		if( Group->GroupAnimSets(i) )
		{
			UAnimSequence* FoundSeq = Group->GroupAnimSets(i)->FindAnimSequence(InName);
			if(FoundSeq)
			{
				return FoundSeq;
			}
		}
	}

	return NULL;	
}

/** Find the animation name and position for the given point in the track timeline. */
void UInterpTrackAnimControl::GetAnimForTime(FLOAT InTime, FName& OutAnimSeqName, FLOAT& OutPosition, UBOOL& bOutLooping)
{
	if(AnimSeqs.Num() == 0)
	{
		OutAnimSeqName = NAME_None;
		OutPosition = 0.f;
	}
	else
	{
		if(InTime < AnimSeqs(0).StartTime)
		{
			OutAnimSeqName = AnimSeqs(0).AnimSeqName;
			OutPosition = AnimSeqs(0).AnimStartOffset;

			// Reverse position if the key is set to be reversed.
			if(AnimSeqs(0).bReverse)
			{
				UAnimSequence *Seq = FindAnimSequenceFromName(AnimSeqs(0).AnimSeqName);

				if(Seq)
				{
					OutPosition = ConditionallyReversePosition(AnimSeqs(0), Seq, OutPosition);
				}

				bOutLooping = AnimSeqs(0).bLooping;
			}
		}
		else
		{
			INT i=0; for( i=0; i<AnimSeqs.Num()-1 && AnimSeqs(i+1).StartTime <= InTime; i++);

			OutAnimSeqName = AnimSeqs(i).AnimSeqName;
			OutPosition = ((InTime - AnimSeqs(i).StartTime) * AnimSeqs(i).AnimPlayRate);

			UAnimSequence *Seq = FindAnimSequenceFromName(AnimSeqs(i).AnimSeqName);
			if(Seq)
			{
				FLOAT SeqLength = ::Max(Seq->SequenceLength - (AnimSeqs(i).AnimStartOffset + AnimSeqs(i).AnimEndOffset), 0.01f);

				if(AnimSeqs(i).bLooping)
				{
					OutPosition = appFmod(OutPosition, SeqLength);
					OutPosition += AnimSeqs(i).AnimStartOffset;
				}
				else
				{
					OutPosition = ::Clamp(OutPosition + AnimSeqs(i).AnimStartOffset, 0.f, (Seq->SequenceLength - AnimSeqs(i).AnimEndOffset) + (FLOAT)KINDA_SMALL_NUMBER);
				}

				// Reverse position if the key is set to be reversed.
				if(AnimSeqs(i).bReverse)
				{
					OutPosition = ConditionallyReversePosition(AnimSeqs(i), Seq, OutPosition);
				}

				bOutLooping = AnimSeqs(i).bLooping;
			}
		}
	}
}

/** Get the strength that the animation from this track should be blended in with at the give time. */
FLOAT UInterpTrackAnimControl::GetWeightForTime(FLOAT InTime)
{
	return FloatTrack.Eval(InTime, 0.f);
}


/** 
 * Calculates the reversed time for a sequence key.
 *
 * @param SeqKey		Key that is reveresed and we are trying to find a position for.
 * @param Seq			Anim sequence the key represents.
 * @param InPosition	Timeline position that we are currently at.
 *
 * @return Returns the position in the specified seq key. 
 */
FLOAT UInterpTrackAnimControl::ConditionallyReversePosition(FAnimControlTrackKey &SeqKey, UAnimSequence* Seq, FLOAT InPosition)
{
	FLOAT Result = InPosition;

	// Reverse position if the key is set to be reversed.
	if(SeqKey.bReverse)
	{
		if(Seq == NULL)
		{
			Seq = FindAnimSequenceFromName(SeqKey.AnimSeqName);
		}

		// Reverse the clip.
		if(Seq)
		{
			const FLOAT RealLength = Seq->SequenceLength - (SeqKey.AnimStartOffset+SeqKey.AnimEndOffset);
			Result = (RealLength - (InPosition-SeqKey.AnimStartOffset))+SeqKey.AnimStartOffset;	// Mirror the cropped clip.
		}
	}

	return Result;
}

/** Update the Actors animation state, in the editor. */
void UInterpTrackAnimControl::PreviewUpdateTrack(FLOAT NewPosition, class UInterpTrackInst* TrInst)
{
	AActor* Actor = TrInst->GetGroupActor();
	if(!Actor)
	{
		return;
	}

	// Calculate this channels index within the named slot.
	INT ChannelIndex = CalcChannelIndex();


	FName NewAnimSeqName;
	FLOAT NewAnimPosition;
	UBOOL bNewLooping;
	GetAnimForTime(NewPosition, NewAnimSeqName, NewAnimPosition, bNewLooping);

	if(NewAnimSeqName != NAME_None)
	{
		Actor->PreviewSetAnimPosition(SlotName, ChannelIndex, NewAnimSeqName, NewAnimPosition, bNewLooping);
	}
}

/** Update the Actors animation state, in the game. */
void UInterpTrackAnimControl::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	AActor* Actor = TrInst->GetGroupActor();
	if(!Actor)
	{
		return;
	}

	//debugf(TEXT("UInterpTrackAnimControl::UpdateTrack, NewPosition: %3.2f"), NewPosition);

	UInterpTrackInstAnimControl* AnimInst = CastChecked<UInterpTrackInstAnimControl>(TrInst);

	// Calculate this channels index within the named slot.
	INT ChannelIndex = CalcChannelIndex();

	// Don't do complicated stuff for notifies if playing backwards, or not moving at all.
	if(AnimSeqs.Num() == 0 || NewPosition <= AnimInst->LastUpdatePosition || bJump)
	{
		FName NewAnimSeqName;
		FLOAT NewAnimPosition;
		UBOOL bNewLooping;
		GetAnimForTime(NewPosition, NewAnimSeqName, NewAnimPosition, bNewLooping);

		if( NewAnimSeqName != NAME_None )
		{
			Actor->eventSetAnimPosition(SlotName, ChannelIndex, NewAnimSeqName, NewAnimPosition, FALSE, bNewLooping);
		}
	}
	// Playing forwards - need to do painful notify stuff.
	else
	{
		// Find which anim we are starting in. -1 Means before first anim.
		INT StartSeqIndex = -1; 
		for( StartSeqIndex = -1; StartSeqIndex<AnimSeqs.Num()-1 && AnimSeqs(StartSeqIndex+1).StartTime <= AnimInst->LastUpdatePosition; StartSeqIndex++);

		// Find which anim we are ending in. -1 Means before first anim.
		INT EndSeqIndex = -1; 
		for( EndSeqIndex = -1; EndSeqIndex<AnimSeqs.Num()-1 && AnimSeqs(EndSeqIndex+1).StartTime <= NewPosition; EndSeqIndex++);

		// Now start walking from the first block.
		INT CurrentSeqIndex = StartSeqIndex;
		while(CurrentSeqIndex <= EndSeqIndex)
		{
			// If we are before the first anim - do nothing but set ourselves to the beginning of the first anim.
			if(CurrentSeqIndex == -1)
			{
				FAnimControlTrackKey &SeqKey = AnimSeqs(0);
				FLOAT Position = SeqKey.AnimStartOffset;

				// Reverse position if the key is set to be reversed.
				if( SeqKey.bReverse )
				{
					Position = ConditionallyReversePosition(SeqKey, NULL, Position);
				}

				if( SeqKey.AnimSeqName != NAME_None )
				{
					Actor->eventSetAnimPosition(SlotName, ChannelIndex, SeqKey.AnimSeqName, Position, FALSE, SeqKey.bLooping);
				}
			}
			// If we are within an anim.
			else
			{
				// Find the name and starting time
				FAnimControlTrackKey &AnimSeq = AnimSeqs(CurrentSeqIndex);
				FName CurrentAnimName = AnimSeq.AnimSeqName;
				FLOAT CurrentSeqStart = AnimSeq.StartTime;
				FLOAT CurrentStartOffset = AnimSeq.AnimStartOffset;
				FLOAT CurrentEndOffset = AnimSeq.AnimEndOffset;
				FLOAT CurrentRate = AnimSeq.AnimPlayRate;

				// Find the time we are currently at.
				// If this is the first start anim - its the 'current' position of the Matinee.
				FLOAT FromTime = (CurrentSeqIndex == StartSeqIndex) ? AnimInst->LastUpdatePosition : CurrentSeqStart;

				// Find the time we want to move to.
				// If this is the last anim - its the 'new' position of the Matinee. Otherwise, its the start of the next anim.
				// Safe to address AnimSeqs at CurrentSeqIndex+1 in the second case, as it must be <EndSeqIndex and EndSeqIndex<AnimSeqs.Num().
				FLOAT ToTime = (CurrentSeqIndex == EndSeqIndex) ? NewPosition : AnimSeqs(CurrentSeqIndex+1).StartTime; 
				
				// If looping, we need to play through the sequence multiple times, to ensure notifies are execute correctly.
				if( AnimSeq.bLooping )
				{
					UAnimSequence* Seq = FindAnimSequenceFromName(CurrentAnimName);
					if(Seq)
					{
						// Find position we should not play beyond in this sequence.
						FLOAT SeqEnd = (Seq->SequenceLength - CurrentEndOffset);

						// Find time this sequence will take to play
						FLOAT SeqLength = ::Max(Seq->SequenceLength - (CurrentStartOffset + CurrentEndOffset), 0.01f);

						// Find the number of loops we make. 
						// @todo: This will need to be updated if we decide to support notifies in reverse.
						if(AnimSeq.bReverse == FALSE)
						{
							INT FromLoopNum = appFloor( (((FromTime - CurrentSeqStart) * CurrentRate) + CurrentStartOffset)/SeqLength );
							INT ToLoopNum = appFloor( (((ToTime - CurrentSeqStart) * CurrentRate) + CurrentStartOffset)/SeqLength );
							INT NumLoopsToJump = ToLoopNum - FromLoopNum;

							for(INT i=0; i<NumLoopsToJump; i++)
							{
								Actor->eventSetAnimPosition(SlotName, ChannelIndex, CurrentAnimName, SeqEnd + KINDA_SMALL_NUMBER, TRUE, TRUE);
								Actor->eventSetAnimPosition(SlotName, ChannelIndex, CurrentAnimName, CurrentStartOffset, FALSE, TRUE);
							}
						}

						FLOAT AnimPos = appFmod((ToTime - CurrentSeqStart) * CurrentRate, SeqLength) + CurrentStartOffset;

						// Reverse position if the key is set to be reversed.
						if( AnimSeq.bReverse )
						{
							AnimPos = ConditionallyReversePosition(AnimSeq, Seq, AnimPos);
						}

						if( CurrentAnimName != NAME_None )
						{
							Actor->eventSetAnimPosition(SlotName, ChannelIndex, CurrentAnimName, AnimPos, TRUE, TRUE);
						}
					}
					// If we failed to find the sequence, just use simpler method.
					else
					{
						if( CurrentAnimName != NAME_None )
						{
							Actor->eventSetAnimPosition(SlotName, ChannelIndex, CurrentAnimName, ((ToTime - CurrentSeqStart) * CurrentRate) + CurrentStartOffset, TRUE, TRUE);
						}
					}
				}
				// No looping or reversed - its easy - wind to desired time.
				else
				{
					FLOAT AnimPos = ((ToTime - CurrentSeqStart) * CurrentRate) + CurrentStartOffset;

					UAnimSequence* Seq = FindAnimSequenceFromName(CurrentAnimName);
					if( Seq )
					{
						FLOAT SeqEnd = (Seq->SequenceLength - CurrentEndOffset);
						AnimPos = ::Clamp( AnimPos, 0.f, SeqEnd + (FLOAT)KINDA_SMALL_NUMBER );
					}

					// Conditionally reverse the position.
					AnimPos = ConditionallyReversePosition(AnimSeq, Seq, AnimPos);

					if( CurrentAnimName != NAME_None )
					{
						Actor->eventSetAnimPosition(SlotName, ChannelIndex, CurrentAnimName, AnimPos, TRUE, FALSE);
					}
				}

				// If we are not yet at target anim, set position at start of next anim.
				if( CurrentSeqIndex < EndSeqIndex )
				{
					FAnimControlTrackKey &SeqKey = AnimSeqs(CurrentSeqIndex+1);
					FLOAT Position = SeqKey.AnimStartOffset;

					// Conditionally reverse the position.
					if( SeqKey.bReverse )
					{
						Position = ConditionallyReversePosition(SeqKey, NULL, Position);
					}

					if( SeqKey.AnimSeqName != NAME_None )
					{
						Actor->eventSetAnimPosition(SlotName, ChannelIndex, SeqKey.AnimSeqName, Position, FALSE, SeqKey.bLooping);
					}
				}
			}

			// Move on the CurrentSeqIndex counter.
			CurrentSeqIndex++;
		}
	}

	// Now remember the location we are at this frame, to use as the 'From' time next frame.
	AnimInst->LastUpdatePosition = NewPosition;
}

/** 
 *	Utility to split the animation we are currently over into two pieces at the current position. 
 *	InPosition is position in the entire Matinee sequence.
 *	Returns the index of the newly created key.
 */
INT UInterpTrackAnimControl::SplitKeyAtPosition(FLOAT InPosition)
{
	// Check we are over a valid animation
	INT SplitSeqIndex = -1; 
	for( SplitSeqIndex = -1; SplitSeqIndex < AnimSeqs.Num()-1 && AnimSeqs(SplitSeqIndex+1).StartTime <= InPosition; SplitSeqIndex++ );
	if(SplitSeqIndex == -1)
	{
		return INDEX_NONE;
	}

	// Check the sequence is valid.
	FAnimControlTrackKey& SplitKey = AnimSeqs(SplitSeqIndex);
	UAnimSequence* Seq = FindAnimSequenceFromName(SplitKey.AnimSeqName);
	if(!Seq)
	{
		return INDEX_NONE;
	}

	// Check we are over an actual chunk of sequence.
	FLOAT SplitAnimPos = ((InPosition - SplitKey.StartTime) * SplitKey.AnimPlayRate) + SplitKey.AnimStartOffset;
	if(SplitAnimPos <= SplitKey.AnimStartOffset || SplitAnimPos >= (Seq->SequenceLength - SplitKey.AnimEndOffset))
	{
		return INDEX_NONE;
	}

	// Create new Key.
	FAnimControlTrackKey NewKey;
	NewKey.AnimPlayRate = SplitKey.AnimPlayRate;
	NewKey.AnimSeqName = SplitKey.AnimSeqName;
	NewKey.StartTime = InPosition;
	NewKey.bLooping = SplitKey.bLooping;
	NewKey.AnimStartOffset = SplitAnimPos; // Start position in the new animation wants to be the place we are currently at.
	NewKey.AnimEndOffset = SplitKey.AnimEndOffset; // End place is the same as the one we are splitting.

	SplitKey.AnimEndOffset = Seq->SequenceLength - SplitAnimPos; // New end position is where we are.
	SplitKey.bLooping = false; // Disable looping for section before the cut.

	// Add new key to track.
	AnimSeqs.InsertZeroed(SplitSeqIndex+1);
	AnimSeqs(SplitSeqIndex+1) = NewKey;

	return SplitSeqIndex+1;
}

/**
 * Crops the key at the position specified, by deleting the area of the key before or after the position.
 *
 * @param InPosition				Position to use as a crop point.
 * @param bCutAreaBeforePosition	Whether we should delete the area of the key before the position specified or after.
 *
 * @return Returns the index of the key that was cropped.
 */
INT UInterpTrackAnimControl::CropKeyAtPosition(FLOAT InPosition, UBOOL bCutAreaBeforePosition)
{
	// Check we are over a valid animation
	INT SplitSeqIndex = -1; 
	for( SplitSeqIndex = -1; SplitSeqIndex < AnimSeqs.Num()-1 && AnimSeqs(SplitSeqIndex+1).StartTime <= InPosition; SplitSeqIndex++ );
	if(SplitSeqIndex == -1)
	{
		return INDEX_NONE;
	}

	// Check the sequence is valid.
	FAnimControlTrackKey& SplitKey = AnimSeqs(SplitSeqIndex);
	UAnimSequence* Seq = FindAnimSequenceFromName(SplitKey.AnimSeqName);
	if(!Seq)
	{
		return INDEX_NONE;
	}

	// Check we are over an actual chunk of sequence.
	FLOAT SplitAnimPos = ((InPosition - SplitKey.StartTime) * SplitKey.AnimPlayRate) + SplitKey.AnimStartOffset;
	if(SplitAnimPos <= SplitKey.AnimStartOffset || SplitAnimPos >= (Seq->SequenceLength - SplitKey.AnimEndOffset))
	{
		return INDEX_NONE;
	}

	// Crop either left or right depending on which way the user wants to crop.
	if(bCutAreaBeforePosition)
	{
		SplitKey.StartTime = InPosition;
		SplitKey.AnimStartOffset = SplitAnimPos; // New end position is where we are.
	}
	else
	{
		SplitKey.AnimEndOffset = Seq->SequenceLength - SplitAnimPos; // New end position is where we are.
	}

	return SplitSeqIndex;
}

/** Calculate the index of this Track within its Slot (for when multiple tracks are using same slot). */
INT UInterpTrackAnimControl::CalcChannelIndex()
{
	UInterpGroup* Group = CastChecked<UInterpGroup>(GetOuter());

	// Count number of tracks with same slot name before we reach this one
	INT ChannelIndex = 0;
	for(INT i=0; i<Group->InterpTracks.Num(); i++)
	{
		UInterpTrackAnimControl* AnimTrack = Cast<UInterpTrackAnimControl>(Group->InterpTracks(i));

		// If we have reached this track, return current ChannelIndex
		if(AnimTrack == this)
		{
			return ChannelIndex;
		}

		// If not this track, but has same slot name, increment ChannelIndex
		if(AnimTrack && AnimTrack->SlotName == SlotName)
		{
			ChannelIndex++;
		}
	}

	check(FALSE && "AnimControl Track Not Found In It's Group!"); // Should not reach here!
	return 0;
}

/** Get the name of the class used to help out when adding tracks, keys, etc. in UnrealEd.
* @return	String name of the helper class.*/
const FString UInterpTrackAnimControl::GetEdHelperClassName() const
{
	return FString( TEXT("UnrealEd.InterpTrackAnimControlHelper") );
}

/*-----------------------------------------------------------------------------
	UInterpTrackInstAnimControl
-----------------------------------------------------------------------------*/

/** Initialise this Track instance. Called in-game before doing any interpolation. */
void UInterpTrackInstAnimControl::InitTrackInst(UInterpTrack* Track)
{
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( GetOuter() );
	USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );
	LastUpdatePosition = Seq->Position;
}

/*-----------------------------------------------------------------------------
	UInterpTrackSound
-----------------------------------------------------------------------------*/

STRUCTTRACK_GETNUMKEYFRAMES(UInterpTrackSound, Sounds)
STRUCTTRACK_GETTIMERANGE(UInterpTrackSound, Sounds, Time)
STRUCTTRACK_GETKEYFRAMETIME(UInterpTrackSound, Sounds, Time)
STRUCTTRACK_SETKEYFRAMETIME(UInterpTrackSound, Sounds, Time, FSoundTrackKey )
STRUCTTRACK_REMOVEKEYFRAME(UInterpTrackSound, Sounds )
STRUCTTRACK_DUPLICATEKEYFRAME(UInterpTrackSound, Sounds, Time, FSoundTrackKey )

/** Set this track to sensible default values. Called when track is first created. */
void UInterpTrackSound::SetTrackToSensibleDefault()
{
	VectorTrack.Points.Empty();

	const FLOAT DefaultSoundKeyVolume = 1.0f;
	const FLOAT DefaultSoundKeyPitch = 1.0f;

	VectorTrack.AddPoint( 0.0f, FVector( DefaultSoundKeyVolume, DefaultSoundKeyPitch, 1.f ) );
}

/** Add a new keyframe at the speicifed time. Returns index of new keyframe. */
INT UInterpTrackSound::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	FSoundTrackKey NewSound;
	NewSound.Sound = NULL;
	NewSound.Time = Time;
	NewSound.Volume = 1.0f;
	NewSound.Pitch = 1.0f;

	// Find the correct index to insert this cut.
	INT i=0; for( i=0; i<Sounds.Num() && Sounds(i).Time < Time; i++);
	Sounds.Insert(i);
	Sounds(i) = NewSound;

	return i;
}

/** Return the closest time to the time passed in that we might want to snap to. */
UBOOL UInterpTrackSound::GetClosestSnapPosition(FLOAT InPosition, TArray<INT> &IgnoreKeys, FLOAT& OutPosition)
{
	if(Sounds.Num() == 0)
	{
		return false;
	}

	UBOOL bFoundSnap = false;
	FLOAT ClosestSnap = 0.f;
	FLOAT ClosestDist = BIG_NUMBER;
	for(INT i=0; i<Sounds.Num(); i++)
	{
		if(!IgnoreKeys.ContainsItem(i))
		{
			FLOAT SoundStartTime = Sounds(i).Time;
			FLOAT SoundEndTime = SoundStartTime;

			// Make block as long as the SoundCue is.
			USoundCue* Cue = Sounds(i).Sound;
			if(Cue)
			{
				SoundEndTime += Cue->GetCueDuration();
			}

			// Truncate sound cue at next sound in the track.
			if((i < Sounds.Num()-1) && !IgnoreKeys.ContainsItem(i+1))
			{
				SoundEndTime = ::Min( Sounds(i+1).Time, SoundEndTime );
			}

			FLOAT Dist = Abs( SoundStartTime - InPosition );
			if(Dist < ClosestDist)
			{
				ClosestSnap = SoundStartTime;
				ClosestDist = Dist;
				bFoundSnap = true;
			}

			Dist = Abs( SoundEndTime - InPosition );
			if(Dist < ClosestDist)
			{
				ClosestSnap = SoundEndTime;
				ClosestDist = Dist;
				bFoundSnap = true;
			}
		}
	}

	OutPosition = ClosestSnap;
	return bFoundSnap;
}

void UInterpTrackSound::PostLoad()
{
	Super::PostLoad();
	if (VectorTrack.Points.Num() <= 0)
	{
		SetTrackToSensibleDefault();
	}
}

/**
 * Returns the key at the specified position in the track.
 */
FSoundTrackKey& UInterpTrackSound::GetSoundTrackKeyAtPosition(FLOAT InPosition)
{
	INT SoundIndex; 
	for( SoundIndex = -1; SoundIndex<Sounds.Num()-1 && Sounds(SoundIndex+1).Time < InPosition; SoundIndex++);
	if (SoundIndex == -1)
	{
		SoundIndex = 0;
	}
	return Sounds(SoundIndex);
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the game, when USeqAct_Interp is ticked
*  @param bJump	Indicates if this is a sudden jump instead of a smooth move to the new position.
*/
void UInterpTrackSound::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	if (Sounds.Num() <= 0)
	{
		//debugf(NAME_Warning,TEXT("No sounds for sound track %s"),*GetName());
		return;
	}

	UInterpGroup* Group = CastChecked<UInterpGroup>(GetOuter());
	UInterpTrackInstSound* SoundInst = CastChecked<UInterpTrackInstSound>(TrInst);
	AActor* Actor = TrInst->GetGroupActor();


	// If this is a director group and the associated actor is a player controller, we need to make sure that it's
	// the local client's player controller and not some other client's player.  In the case where we're a host
	// with other connected players, we don't want the audio to be played for each of the players -- once is fine!
	UBOOL bIsOkayToPlaySound = TRUE;
	if( Group->IsA( UInterpGroupDirector::StaticClass() ) && Actor != NULL )
	{
		APlayerController* PC = Cast< APlayerController >( Actor );
		if( PC != NULL )
		{
			if( !PC->IsLocalPlayerController() )
			{
				// The director track is trying to play audio for a non-local client's player.  This is probably not
				// what was intended, so we don't allow it!  This will be played only by the local player's
				// audio track instance.
				bIsOkayToPlaySound = FALSE;
			}
		}
	}

    
	// Only play sounds if we are playing Matinee forwards, we're not hopping around in time, and we're allowed to
	// play the sound.
	FVector VolumePitchValue( 1.0f, 1.0f, 1.0f );
	if( NewPosition > SoundInst->LastUpdatePosition && !bJump && bIsOkayToPlaySound )
	{
		// Find which sound we are starting in. -1 Means before first sound.
		INT StartSoundIndex = -1; 
		for( StartSoundIndex = -1; StartSoundIndex<Sounds.Num()-1 && Sounds(StartSoundIndex+1).Time < SoundInst->LastUpdatePosition; StartSoundIndex++);

		// Find which sound we are ending in. -1 Means before first sound.
		INT EndSoundIndex = -1; 
		for( EndSoundIndex = -1; EndSoundIndex<Sounds.Num()-1 && Sounds(EndSoundIndex+1).Time < NewPosition; EndSoundIndex++);

		//////
		FSoundTrackKey& SoundTrackKey = GetSoundTrackKeyAtPosition(NewPosition);
		VolumePitchValue *= FVector( SoundTrackKey.Volume, SoundTrackKey.Pitch, 1.0f );
		if (VectorTrack.Points.Num() > 0)
		{
			VolumePitchValue *= VectorTrack.Eval(NewPosition,VolumePitchValue);
		}

		// If we have moved into a new sound, we should start playing it now.
		if(StartSoundIndex != EndSoundIndex)
		{
			USoundCue* NewCue = SoundTrackKey.Sound;

			// If we have a sound playing already (ie. an AudioComponent exists) stop it now.
			if(SoundInst->PlayAudioComp)
			{
				SoundInst->PlayAudioComp->Stop();
				SoundInst->PlayAudioComp->SetSoundCue(NewCue);
				SoundInst->PlayAudioComp->VolumeMultiplier = VolumePitchValue.X;
				SoundInst->PlayAudioComp->PitchMultiplier = VolumePitchValue.Y;
				SoundInst->PlayAudioComp->Play();
			}
			else
			{
				// If there is no AudioComponent - create one now.
				SoundInst->PlayAudioComp = UAudioDevice::CreateComponent(NewCue, GWorld->Scene, Actor, false, false);
				if(SoundInst->PlayAudioComp)
				{
					// If we have no actor to attach sound to - its location is meaningless, so we turn off spacialisation.
					// Also if we are playing on a director group, disable spatialisation.
					if(!Actor || Group->IsA(UInterpGroupDirector::StaticClass()))
					{
						SoundInst->PlayAudioComp->bAllowSpatialization = false;
					}

					// Start the sound playing.
					SoundInst->PlayAudioComp->Play();				
				}
			}
		}
	}


	// Apply master volume and pitch scale
	{
		UInterpData* IData = CastChecked<UInterpData>( Group->GetOuter() );
		UInterpGroupDirector* DirGroup = IData->FindDirectorGroup();
		if( DirGroup != NULL )
		{
			UInterpTrackAudioMaster* AudioMasterTrack = DirGroup->GetAudioMasterTrack();
			if( AudioMasterTrack != NULL )
			{
				VolumePitchValue.X *= AudioMasterTrack->GetVolumeScaleForTime( NewPosition );
				VolumePitchValue.Y *= AudioMasterTrack->GetPitchScaleForTime( NewPosition );
			}
		}
	}


	// Update the sound if its playing
	if (SoundInst->PlayAudioComp)
	{
		SoundInst->PlayAudioComp->VolumeMultiplier = VolumePitchValue.X;
		SoundInst->PlayAudioComp->PitchMultiplier = VolumePitchValue.Y;
		SoundInst->PlayAudioComp->SubtitlePriority = bSuppressSubtitles ? 0.f : SUBTITLE_PRIORITY_MATINEE;
	}

	// Finally update the current position as the last one.
	SoundInst->LastUpdatePosition = NewPosition;
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the editor, when scrubbing/previewing etc.
*/
void UInterpTrackSound::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( TrInst->GetOuter() );
	USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );

	// Dont play sounds unless we are preview playback (ie not scrubbing).
	UBOOL bJump = !(Seq->bIsPlaying);
	UpdateTrack(NewPosition, TrInst, bJump);
}

/** Get the name of the class used to help out when adding tracks, keys, etc. in UnrealEd.
* @return	String name of the helper class.*/
const FString	UInterpTrackSound::GetEdHelperClassName() const
{
	return FString( TEXT("UnrealEd.InterpTrackSoundHelper") );
}

/** Stop sound playing when you press stop in Matinee. */
void UInterpTrackSound::PreviewStopPlayback(class UInterpTrackInst* TrInst)
{
	UInterpTrackInstSound* SoundTrInst = CastChecked<UInterpTrackInstSound>(TrInst);
	if(SoundTrInst->PlayAudioComp)
	{
		SoundTrInst->PlayAudioComp->Stop();
	}
}

/*-----------------------------------------------------------------------------
	UInterpTrackInstSound
-----------------------------------------------------------------------------*/

/** Initialise this Track instance. Called in-game before doing any interpolation. */
void UInterpTrackInstSound::InitTrackInst(UInterpTrack* Track)
{
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( GetOuter() );
	USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );
	LastUpdatePosition = Seq->Position;
}

/** Called when interpolation is done. Should not do anything else with this TrackInst after this. */
void UInterpTrackInstSound::TermTrackInst(UInterpTrack* Track)
{
	UInterpTrackSound* SoundTrack = CastChecked<UInterpTrackSound>(Track);

	// If we still have an audio component - deal with it.
	if(PlayAudioComp)
	{
		UBOOL bCompIsPlaying = (PlayAudioComp->bWasPlaying && !PlayAudioComp->bFinished);

		// If we are currently playing, and want to keep the sound playing, 
		// just flag it as 'auto destroy', and it will destroy itself when it finishes.
		if(bCompIsPlaying && SoundTrack->bContinueSoundOnMatineeEnd)
		{
			PlayAudioComp->bAutoDestroy = true;
			PlayAudioComp = NULL;
		}
		else
		{
			PlayAudioComp->Stop();
			PlayAudioComp->DetachFromAny();
			PlayAudioComp = NULL;
		}
	}
}

/*-----------------------------------------------------------------------------
	UInterpTrackFloatParticleParam
-----------------------------------------------------------------------------*/

/** Add a new keyframe at the speicifed time. Returns index of new keyframe. */
INT UInterpTrackFloatParticleParam::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	INT NewKeyIndex = FloatTrack.AddPoint( Time, 0.f );
	FloatTrack.Points(NewKeyIndex).InterpMode = InitInterpMode;

	FloatTrack.AutoSetTangents(CurveTension);

	return NewKeyIndex;
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the editor, when scrubbing/previewing etc.
*/
void UInterpTrackFloatParticleParam::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
	UpdateTrack(NewPosition, TrInst, FALSE);
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the game, when USeqAct_Interp is ticked
*  @param bJump	Indicates if this is a sudden jump instead of a smooth move to the new position.
*/
void UInterpTrackFloatParticleParam::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	AActor* Actor = TrInst->GetGroupActor();
	AEmitter* Emitter = Cast<AEmitter>(Actor);
	if(!Emitter)
	{
		return;
	}

	FLOAT NewFloatValue = FloatTrack.Eval(NewPosition, 0.f);
	Emitter->ParticleSystemComponent->SetFloatParameter( ParamName, NewFloatValue );
}

/*-----------------------------------------------------------------------------
	UInterpTrackInstFloatParticleParam
-----------------------------------------------------------------------------*/

/** Called before Interp editing to put object back to its original state. */
void UInterpTrackInstFloatParticleParam::SaveActorState(UInterpTrack* Track)
{
	UInterpTrackFloatParticleParam* ParamTrack = CastChecked<UInterpTrackFloatParticleParam>(Track);
	AActor* Actor = GetGroupActor();
	AEmitter* Emitter = Cast<AEmitter>(Actor);
	if(!Emitter)
	{
		return;
	}

	UBOOL bFoundParam = Emitter->ParticleSystemComponent->GetFloatParameter( ParamTrack->ParamName, ResetFloat );
	if(!bFoundParam)
	{
		ResetFloat = 0.f;
	}
}

/** Restore the saved state of this Actor. */
void UInterpTrackInstFloatParticleParam::RestoreActorState(UInterpTrack* Track)
{
	UInterpTrackFloatParticleParam* ParamTrack = CastChecked<UInterpTrackFloatParticleParam>(Track);
	AActor* Actor = GetGroupActor();
	AEmitter* Emitter = Cast<AEmitter>(Actor);
	if(!Emitter)
	{
		return;
	}

	Emitter->ParticleSystemComponent->SetFloatParameter( ParamTrack->ParamName, ResetFloat );
}

/*-----------------------------------------------------------------------------
	UInterpTrackFloatMaterialParam
-----------------------------------------------------------------------------*/

/** Add a new keyframe at the speicifed time. Returns index of new keyframe. */
INT UInterpTrackFloatMaterialParam::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	INT NewKeyIndex = FloatTrack.AddPoint( Time, 0.f );
	FloatTrack.Points(NewKeyIndex).InterpMode = InitInterpMode;

	FloatTrack.AutoSetTangents(CurveTension);

	return NewKeyIndex;
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the editor, when scrubbing/previewing etc.
*/
void UInterpTrackFloatMaterialParam::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
	UpdateTrack(NewPosition, TrInst, FALSE);
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the game, when USeqAct_Interp is ticked
*  @param bJump	Indicates if this is a sudden jump instead of a smooth move to the new position.
*/
void UInterpTrackFloatMaterialParam::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	AActor* Actor = TrInst->GetGroupActor();
	AMaterialInstanceActor* MatInstActor = Cast<AMaterialInstanceActor>(Actor);
	if(!MatInstActor || !MatInstActor->MatInst)
	{
		return;
	}

	FLOAT NewFloatValue = FloatTrack.Eval(NewPosition, 0.f);
	MatInstActor->MatInst->SetScalarParameterValue( ParamName, NewFloatValue );
}

/*-----------------------------------------------------------------------------
	UInterpTrackInstFloatMaterialParam
-----------------------------------------------------------------------------*/

/** Called before Interp editing to put object back to its original state. */
void UInterpTrackInstFloatMaterialParam::SaveActorState(UInterpTrack* Track)
{
	UInterpTrackFloatMaterialParam* ParamTrack = CastChecked<UInterpTrackFloatMaterialParam>(Track);
	AActor* Actor = GetGroupActor();
	AMaterialInstanceActor* MatInstActor = Cast<AMaterialInstanceActor>(Actor);
	if(!MatInstActor || !MatInstActor->MatInst)
	{
		return;
	}

	UBOOL bFoundParam = MatInstActor->MatInst->GetScalarParameterValue( ParamTrack->ParamName, ResetFloat );
	if(!bFoundParam)
	{
		ResetFloat = 0.f;
	}
}

/** Restore the saved state of this Actor. */
void UInterpTrackInstFloatMaterialParam::RestoreActorState(UInterpTrack* Track)
{
	UInterpTrackFloatMaterialParam* ParamTrack = CastChecked<UInterpTrackFloatMaterialParam>(Track);
	AActor* Actor = GetGroupActor();
	AMaterialInstanceActor* MatInstActor = Cast<AMaterialInstanceActor>(Actor);
	if(!MatInstActor || !MatInstActor->MatInst)
	{
		return;
	}

	MatInstActor->MatInst->SetScalarParameterValue( ParamTrack->ParamName, ResetFloat );
}

/*-----------------------------------------------------------------------------
	UInterpTrackVectorMaterialParam
-----------------------------------------------------------------------------*/

/** Add a new keyframe at the speicifed time. Returns index of new keyframe. */
INT UInterpTrackVectorMaterialParam::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	INT NewKeyIndex = VectorTrack.AddPoint( Time, FVector(0.f) );
	VectorTrack.Points(NewKeyIndex).InterpMode = InitInterpMode;

	VectorTrack.AutoSetTangents(CurveTension);

	return NewKeyIndex;
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the editor, when scrubbing/previewing etc.
*/
void UInterpTrackVectorMaterialParam::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
	UpdateTrack(NewPosition, TrInst, FALSE);
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the game, when USeqAct_Interp is ticked
*  @param bJump	Indicates if this is a sudden jump instead of a smooth move to the new position.
*/
void UInterpTrackVectorMaterialParam::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	AActor* Actor = TrInst->GetGroupActor();
	AMaterialInstanceActor* MatInstActor = Cast<AMaterialInstanceActor>(Actor);
	if(!MatInstActor || !MatInstActor->MatInst)
	{
		return;
	}

	FVector NewFloatValue = VectorTrack.Eval(NewPosition, FVector(0.f));
	FLinearColor NewLinearColor(NewFloatValue.X, NewFloatValue.Y, NewFloatValue.Z);

	MatInstActor->MatInst->SetVectorParameterValue( ParamName, NewLinearColor );
}



/*-----------------------------------------------------------------------------
	UInterpTrackInstToggle
-----------------------------------------------------------------------------*/


/** Initialise this Track instance. Called in-game before doing any interpolation. */
void UInterpTrackInstToggle::InitTrackInst(UInterpTrack* Track)
{

}



/** Called before Interp editing to put object back to its original state. */
void UInterpTrackInstToggle::SaveActorState(UInterpTrack* Track)
{
	AActor* Actor = GetGroupActor();

	AEmitter* EmitterActor = Cast<AEmitter>(Actor);
	ALensFlareSource* LensFlareActor = Cast<ALensFlareSource>(Actor);
	ALight* LightActor = Cast<ALight>(Actor);

	bSavedActiveState = FALSE;

	if( EmitterActor )
	{
		bSavedActiveState = EmitterActor->bCurrentlyActive;
	}
	else if( LensFlareActor && LensFlareActor->LensFlareComp )
	{
		bSavedActiveState = LensFlareActor->LensFlareComp->bIsActive;
	}
	else if( LightActor != NULL )
	{
		bSavedActiveState = LightActor->LightComponent->bEnabled;
	}
}



/** Restore the saved state of this Actor. */
void UInterpTrackInstToggle::RestoreActorState(UInterpTrack* Track)
{
	AActor* Actor = GetGroupActor();

	AEmitter* EmitterActor = Cast<AEmitter>(Actor);
	ALensFlareSource* LensFlareActor = Cast<ALensFlareSource>(Actor);
	ALight* LightActor = Cast<ALight>(Actor);

	if( EmitterActor )
	{
		if( bSavedActiveState )
		{
			// Use SetActive to only activate a non-active system...
			EmitterActor->ParticleSystemComponent->SetActive(TRUE);
			EmitterActor->bCurrentlyActive = TRUE;
			EmitterActor->bNetDirty = TRUE;
			EmitterActor->eventForceNetRelevant();
		}
		else
		{
			EmitterActor->ParticleSystemComponent->SetActive(FALSE);
			EmitterActor->bCurrentlyActive = FALSE;
			EmitterActor->bNetDirty = TRUE;
			EmitterActor->eventForceNetRelevant();
		}
	}
	else if( LensFlareActor && LensFlareActor->LensFlareComp )
	{
		LensFlareActor->LensFlareComp->SetIsActive( bSavedActiveState );
	}
	else if( LightActor != NULL )
	{
		// We'll only allow *toggleable* lights to be toggled like this!  Static lights are ignored.
		if( LightActor->IsToggleable() )
		{
			LightActor->LightComponent->SetEnabled( bSavedActiveState );
		}
	}
}



/*-----------------------------------------------------------------------------
	UInterpTrackInstVectorMaterialParam
-----------------------------------------------------------------------------*/

/** Called before Interp editing to put object back to its original state. */
void UInterpTrackInstVectorMaterialParam::SaveActorState(UInterpTrack* Track)
{
	UInterpTrackVectorMaterialParam* ParamTrack = CastChecked<UInterpTrackVectorMaterialParam>(Track);
	AActor* Actor = GetGroupActor();
	AMaterialInstanceActor* MatInstActor = Cast<AMaterialInstanceActor>(Actor);
	if(!MatInstActor || !MatInstActor->MatInst)
	{
		return;
	}

	FLinearColor ResetLinearColor;
	UBOOL bFoundParam = MatInstActor->MatInst->GetVectorParameterValue( ParamTrack->ParamName, ResetLinearColor );
	if(bFoundParam)
	{
		ResetVector = FVector(ResetLinearColor.R, ResetLinearColor.G, ResetLinearColor.B);
	}
	else
	{
		ResetVector = FVector(0.f);
	}
}

/** Restore the saved state of this Actor. */
void UInterpTrackInstVectorMaterialParam::RestoreActorState(UInterpTrack* Track)
{
	UInterpTrackVectorMaterialParam* ParamTrack = CastChecked<UInterpTrackVectorMaterialParam>(Track);
	AActor* Actor = GetGroupActor();
	AMaterialInstanceActor* MatInstActor = Cast<AMaterialInstanceActor>(Actor);
	if(!MatInstActor || !MatInstActor->MatInst)
	{
		return;
	}

	FLinearColor ResetLinearColor(ResetVector.X, ResetVector.Y, ResetVector.Z);
	MatInstActor->MatInst->SetVectorParameterValue( ParamTrack->ParamName, ResetLinearColor );
}

/*-----------------------------------------------------------------------------
	UInterpTrackColorScale
-----------------------------------------------------------------------------*/

/** Add a new keyframe at the speicifed time. Returns index of new keyframe. */
INT UInterpTrackColorScale::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	INT NewKeyIndex = VectorTrack.AddPoint( Time, FVector(1.f,1.f,1.f) );
	VectorTrack.Points(NewKeyIndex).InterpMode = InitInterpMode;

	VectorTrack.AutoSetTangents(CurveTension);

	return NewKeyIndex;
}

/** Change the value of an existing keyframe. */
void UInterpTrackColorScale::UpdateKeyframe(INT KeyIndex, UInterpTrackInst* TrInst)
{
	
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the editor, when scrubbing/previewing etc.
*/
void UInterpTrackColorScale::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
	
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the game, when USeqAct_Interp is ticked
*  @param bJump	Indicates if this is a sudden jump instead of a smooth move to the new position.
*/
void UInterpTrackColorScale::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( TrInst->GetOuter() );

	// Actor for a Director group should be a PlayerController.
	APlayerController* PC = Cast<APlayerController>(GrInst->GetGroupActor());
	if(PC && PC->PlayerCamera && !PC->PlayerCamera->bDeleteMe)
	{
		PC->PlayerCamera->bEnableColorScaling = TRUE;
		PC->PlayerCamera->ColorScale = GetColorScaleAtTime(NewPosition);

		// Disable the camera's "color scale interpolation" features since that would blow away our changes
		// when the camera's UpdateCamera function is called.  For the moment, we'll be the authority over color scale!
		PC->PlayerCamera->bEnableColorScaleInterp = FALSE;
	}
}

/** Return the blur alpha we want at the given time. */
FVector UInterpTrackColorScale::GetColorScaleAtTime(FLOAT Time)
{
	FVector ColorScale = VectorTrack.Eval(Time, FVector(1.f,1.f,1.f));
	return ColorScale;
}


/** Set the track to a default of one key at time zero with default blur settings. */
void UInterpTrackColorScale::SetTrackToSensibleDefault()
{
	VectorTrack.Points.Empty();
	VectorTrack.AddPoint(0.f, FVector(1.f,1.f,1.f));
}	

/*-----------------------------------------------------------------------------
	UInterpTrackInstColorScale
-----------------------------------------------------------------------------*/

/** Use this to turn off any blur changes that were applied by this track to this player controller. */
void UInterpTrackInstColorScale::TermTrackInst(UInterpTrack* Track)
{
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( GetOuter() );
	APlayerController* PC = Cast<APlayerController>(GrInst->GroupActor);
	if(PC && PC->PlayerCamera && !PC->PlayerCamera->bDeleteMe)
	{
		PC->PlayerCamera->bEnableColorScaling = false;
		PC->PlayerCamera->ColorScale = FVector(1.f,1.f,1.f);
	}
}

/*-----------------------------------------------------------------------------
	UInterpTrackFaceFX
-----------------------------------------------------------------------------*/

STRUCTTRACK_GETNUMKEYFRAMES(UInterpTrackFaceFX, FaceFXSeqs)
STRUCTTRACK_GETTIMERANGE(UInterpTrackFaceFX, FaceFXSeqs, StartTime)
STRUCTTRACK_GETKEYFRAMETIME(UInterpTrackFaceFX, FaceFXSeqs, StartTime)
STRUCTTRACK_SETKEYFRAMETIME(UInterpTrackFaceFX, FaceFXSeqs, StartTime, FFaceFXTrackKey )
STRUCTTRACK_REMOVEKEYFRAME( UInterpTrackFaceFX, FaceFXSeqs )
STRUCTTRACK_DUPLICATEKEYFRAME( UInterpTrackFaceFX, FaceFXSeqs, StartTime, FFaceFXTrackKey )

/** Add a new keyframe at the speicifed time. Returns index of new keyframe. */
INT UInterpTrackFaceFX::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	FFaceFXTrackKey NewSeq;
	appMemzero(&NewSeq, sizeof(FFaceFXTrackKey));
	NewSeq.FaceFXGroupName = FString(TEXT(""));
	NewSeq.FaceFXSeqName = FString(TEXT(""));
	NewSeq.StartTime = Time;

	// Find the correct index to insert this sequence.
	INT i=0; for( i=0; i<FaceFXSeqs.Num() && FaceFXSeqs(i).StartTime < Time; i++);
	FaceFXSeqs.InsertZeroed(i);
	FaceFXSeqs(i) = NewSeq;

	return i;
}

void UInterpTrackFaceFX::GetSeqInfoForTime( FLOAT InTime, FString& OutGroupName, FString& OutSeqName, FLOAT& OutPosition, FLOAT& OutSeqStart, USoundCue*& OutSoundCue)
{
	// If no keys, or before first key, return no sequence.
	if(FaceFXSeqs.Num() == 0 || InTime < FaceFXSeqs(0).StartTime)
	{
		OutGroupName = FString(TEXT(""));
		OutSeqName = FString(TEXT(""));
		OutPosition = 0.f;
		OutSeqStart = 0.f;
		OutSoundCue = NULL;
	}
	else
	{
		// Find index of sequence we are 'within' at the current time.
		INT i=0; for( i=0; i<FaceFXSeqs.Num()-1 && FaceFXSeqs(i+1).StartTime <= InTime; i++);

		OutGroupName = FaceFXSeqs(i).FaceFXGroupName;
		OutSeqName = FaceFXSeqs(i).FaceFXSeqName;
		OutSeqStart = FaceFXSeqs(i).StartTime;
		OutPosition = InTime - FaceFXSeqs(i).StartTime;

		// Grab the sound cue reference from the track instance key array
		OutSoundCue = NULL;
		if( FaceFXSoundCueKeys.IsValidIndex( i ) )
		{
			OutSoundCue = FaceFXSoundCueKeys( i ).FaceFXSoundCue;
		}
	}
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the editor, when scrubbing/previewing etc.
*/
void UInterpTrackFaceFX::PreviewUpdateTrack(FLOAT NewPosition, class UInterpTrackInst* TrInst)
{
	AActor* Actor = TrInst->GetGroupActor();
	if(!Actor)
	{
		return;
	}

	// If we are in proper playback, we use FaceFX to drive animation like in the game
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( TrInst->GetOuter() );
	USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );
	if(Seq->bIsPlaying)
	{
		UpdateTrack(NewPosition, TrInst, FALSE);
		Actor->PreviewUpdateFaceFX(FALSE, FString(TEXT("")), FString(TEXT("")), 0.f);
	}
	// If scrubbing - force to a particular frame
	else
	{
		FString GroupName, SeqName;
		FLOAT Position, StartPos;
		USoundCue* SoundCue = NULL;
		GetSeqInfoForTime( NewPosition, GroupName, SeqName, Position, StartPos, SoundCue );
		Actor->PreviewUpdateFaceFX(TRUE, GroupName, SeqName, Position);
	}
}

/** 
*	Function which actually updates things based on the new position in the track. 
*  This is called in the game, when USeqAct_Interp is ticked
*  @param bJump	Indicates if this is a sudden jump instead of a smooth move to the new position.
*/
void UInterpTrackFaceFX::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	AActor* Actor = TrInst->GetGroupActor();
	if(!Actor)
	{
		return;
	}

	UInterpTrackInstFaceFX* FaceFXTrInst = CastChecked<UInterpTrackInstFaceFX>(TrInst);

	// Only play facefx if we are playing Matinee forwards, and not jumping.
	if(NewPosition > FaceFXTrInst->LastUpdatePosition && !bJump)
	{
		FLOAT DummyPos, OldPos;
		FString OldGroupName, OldSeqName;
		USoundCue* OldSoundCue = NULL;
		GetSeqInfoForTime( FaceFXTrInst->LastUpdatePosition, OldGroupName, OldSeqName, DummyPos, OldPos, OldSoundCue );

		FLOAT NewPos;
		FString NewGroupName, NewSeqName;
		USoundCue* NewSoundCue = NULL;
		GetSeqInfoForTime( NewPosition, NewGroupName, NewSeqName, DummyPos, NewPos, NewSoundCue );

		// If this is the first update and we have a valid sequence name, or the sequence has changed, tell the FaceFX animation to play
		if((FaceFXTrInst->bFirstUpdate && NewSeqName != TEXT("") ) ||
			NewGroupName != OldGroupName ||
			NewSeqName != OldSeqName ||
			NewPos != OldPos ||
			NewSoundCue != OldSoundCue)
		{
			// Becuase we can't call script events from Matinee, if we are in matinee, use C++ version of function
			if(GIsEditor && !GWorld->HasBegunPlay())
			{
				Actor->PreviewActorPlayFaceFX(NewGroupName, NewSeqName, NewSoundCue);
			}
			else
			{
				Actor->eventPlayActorFaceFXAnim(NULL, NewGroupName, NewSeqName, NewSoundCue);
			}

			FaceFXTrInst->bFirstUpdate = FALSE;
		}
	}

	FaceFXTrInst->LastUpdatePosition = NewPosition;
}

/** Stop playing the FaceFX animation on the Actor when we hit stop in Matinee. */
void UInterpTrackFaceFX::PreviewStopPlayback(class UInterpTrackInst* TrInst)
{
	AActor* Actor = TrInst->GetGroupActor();
	if(Actor)
	{
		Actor->PreviewActorStopFaceFX();
	}
}

const FString UInterpTrackFaceFX::GetEdHelperClassName() const
{
	return FString( TEXT("UnrealEd.InterpTrackFaceFXHelper") );
}

/** MountFaceFXAnimSets as soon as the user adds them to the array. */
void UInterpTrackFaceFX::PostEditChange(UProperty* PropertyThatChanged)
{
	if(CachedActorFXAsset)
	{
		for(INT i=0; i<FaceFXAnimSets.Num(); i++)
		{
			UFaceFXAnimSet* Set = FaceFXAnimSets(i);
			if(Set)
			{
				CachedActorFXAsset->MountFaceFXAnimSet(Set);
			}
		}
	}
}


/** Updates references to sound cues for all of this track's FaceFX animation keys.  Should be called at
    load time in the editor as well as whenever the track's data is changed. */
void UInterpTrackFaceFX::UpdateFaceFXSoundCueReferences( UFaceFXAsset* FaceFXAsset )
{
#if WITH_FACEFX
	using namespace OC3Ent;
	using namespace Face;

	if( FaceFXAsset != NULL )
	{
		// First, make sure that our track instance's key array is the same size as the track's key array
		if( FaceFXSoundCueKeys.Num() != FaceFXSeqs.Num() )
		{
			FaceFXSoundCueKeys.Empty( FaceFXSeqs.Num() );
			FaceFXSoundCueKeys.AddZeroed( FaceFXSeqs.Num() );

			// Make sure our changes are saved to disk
			MarkPackageDirty();
		}

		for( INT CurKeyIndex = 0; CurKeyIndex < FaceFXSeqs.Num(); ++CurKeyIndex )
		{
			FFaceFXTrackKey& CurKey = FaceFXSeqs( CurKeyIndex );
			FFaceFXSoundCueKey& SoundCueKey = FaceFXSoundCueKeys( CurKeyIndex );

			USoundCue* NewSoundCue = NULL;

			// Get the FxActor
			FxActor* fActor = FaceFXAsset->GetFxActor();

			// Find the Group by name
			FxSize GroupIndex = fActor->FindAnimGroup(TCHAR_TO_ANSI(*(CurKey.FaceFXGroupName)));
			if(FxInvalidIndex != GroupIndex)
			{
				FxAnimGroup& fGroup = fActor->GetAnimGroup(GroupIndex);

				// Find the animation by name
				FxSize SeqIndex = fGroup.FindAnim(TCHAR_TO_ANSI(*(CurKey.FaceFXSeqName)));
				if(FxInvalidIndex != SeqIndex)
				{
					const FxAnim& fAnim = fGroup.GetAnim(SeqIndex);

					FxString SoundCuePath = fAnim.GetSoundCuePath();
					if( SoundCuePath.Length() > 0 )
					{
						NewSoundCue = LoadObject<USoundCue>(NULL, ANSI_TO_TCHAR(SoundCuePath.GetData()), NULL, LOAD_NoWarn, NULL);
					}
				}
			}

			// Has anything changed?
			if( SoundCueKey.FaceFXSoundCue != NewSoundCue )
			{
				SoundCueKey.FaceFXSoundCue = NewSoundCue;
				
				// Mark the package dirty so that the new sound cue reference will be resaved!
				MarkPackageDirty();
			}
		}
	}
#endif
}




/*-----------------------------------------------------------------------------
	UInterpTrackInstFaceFX
-----------------------------------------------------------------------------*/

/** Initialise this Track instance. Called in-game before doing any interpolation. */
void UInterpTrackInstFaceFX::InitTrackInst(UInterpTrack* Track)
{
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( GetOuter() );
	USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );
	UInterpTrackFaceFX* FaceFXTrack = CastChecked<UInterpTrackFaceFX>(Track);

	// Make sure all the FaceFXAnimSets that are wanted are mounted.
	AActor* Actor = GetGroupActor();
	if(Actor)
	{
		UFaceFXAsset* Asset = NULL;

		// Because we can't call script events from Matinee, if we are in matinee, use C++ version of function
		if(GIsEditor && !GWorld->HasBegunPlay())
		{
			Asset = Actor->PreviewGetActorFaceFXAsset();
		}
		else
		{
			Asset = Actor->eventGetActorFaceFXAsset();
		}

		// If we got a FaceFXAsset, mount AnimSets to it.
		if(Asset)
		{
			for(INT i=0; i<FaceFXTrack->FaceFXAnimSets.Num(); i++)
			{
				UFaceFXAnimSet* Set = FaceFXTrack->FaceFXAnimSets(i);
				if(Set)
				{
					Asset->MountFaceFXAnimSet(Set);
				}

				if( GIsEditor && !GWorld->HasBegunPlay() )
				{
					// Now that we've mounted our anim sets, we'll make sure sound cue references are up to date
					FaceFXTrack->UpdateFaceFXSoundCueReferences( Asset );
				}
			}
		}
	}

	// Init position, and flag so we catch first update.
	LastUpdatePosition = Seq->Position;
	bFirstUpdate = TRUE;
}

/** Stop playing FaceFX animations when exiting a cinematic. */
void UInterpTrackInstFaceFX::TermTrackInst(UInterpTrack* Track)
{
	AActor* Actor = GetGroupActor();
	if(Actor)
	{
		// Because we can't call script events from Matinee, if we are in matinee, use C++ version of function
		if(GIsEditor && !GWorld->HasBegunPlay())
		{
			Actor->PreviewActorStopFaceFX();
		}
		else
		{
			Actor->eventStopActorFaceFXAnim();
		}
	}
}

/** Get a ref to the FaceFXAsset that is being used by the Actor. */
void UInterpTrackInstFaceFX::SaveActorState(UInterpTrack* Track)
{
	UInterpTrackFaceFX* FaceFXTrack = CastChecked<UInterpTrackFaceFX>(Track);

	AActor* Actor = GetGroupActor();
	if(Actor)
	{
		FaceFXTrack->CachedActorFXAsset = Actor->PreviewGetActorFaceFXAsset();
	}
}

/** On closing Matinee, clear any FaceFX animation on the Actor. */
void UInterpTrackInstFaceFX::RestoreActorState(UInterpTrack* Track)
{
	// Make sure sound cue references are up to date before exiting Matinee
	UInterpTrackFaceFX* FaceFXTrack = CastChecked<UInterpTrackFaceFX>(Track);
	FaceFXTrack->UpdateFaceFXSoundCueReferences( FaceFXTrack->CachedActorFXAsset );


	AActor* Actor = GetGroupActor();
	if(!Actor)
	{
		return;
	}

	Actor->PreviewUpdateFaceFX(TRUE, TEXT(""), TEXT(""), 0.f);
}


/*-----------------------------------------------------------------------------
	UInterpTrackMorphWeight
-----------------------------------------------------------------------------*/

INT UInterpTrackMorphWeight::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	INT NewKeyIndex = FloatTrack.AddPoint( Time, 0.f );
	FloatTrack.Points(NewKeyIndex).InterpMode = InitInterpMode;

	FloatTrack.AutoSetTangents(CurveTension);

	return NewKeyIndex;
}

void UInterpTrackMorphWeight::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
	AActor* Actor = TrInst->GetGroupActor();
	if(Actor)
	{
		Actor->PreviewSetMorphWeight(MorphNodeName, FloatTrack.Eval(NewPosition, 0.f));
	}
}

void UInterpTrackMorphWeight::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	AActor* Actor = TrInst->GetGroupActor();
	if(Actor)
	{
		Actor->eventSetMorphWeight(MorphNodeName, FloatTrack.Eval(NewPosition, 0.f));
	}
}


/*-----------------------------------------------------------------------------
	UInterpTrackInstMorphWeight
-----------------------------------------------------------------------------*/


void UInterpTrackInstMorphWeight::RestoreActorState(UInterpTrack* Track)
{
	UInterpTrackMorphWeight* MorphTrack = CastChecked<UInterpTrackMorphWeight>(Track);
	AActor* Actor = GetGroupActor();
	if(Actor)
	{
		Actor->PreviewSetMorphWeight(MorphTrack->MorphNodeName, 0.f);
	}
}


/*-----------------------------------------------------------------------------
	UInterpTrackSkelControlScale
-----------------------------------------------------------------------------*/

INT UInterpTrackSkelControlScale::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	INT NewKeyIndex = FloatTrack.AddPoint( Time, 0.f );
	FloatTrack.Points(NewKeyIndex).InterpMode = InitInterpMode;

	FloatTrack.AutoSetTangents(CurveTension);

	return NewKeyIndex;
}

void UInterpTrackSkelControlScale::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
	AActor* Actor = TrInst->GetGroupActor();
	if(Actor)
	{
		Actor->PreviewSetSkelControlScale(SkelControlName, FloatTrack.Eval(NewPosition, 0.f));
	}
}


void UInterpTrackSkelControlScale::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	AActor* Actor = TrInst->GetGroupActor();
	if(Actor)
	{
		Actor->eventSetSkelControlScale(SkelControlName, FloatTrack.Eval(NewPosition, 0.f));
	}
}

/*-----------------------------------------------------------------------------
	UInterpTrackInstSkelControlScale
-----------------------------------------------------------------------------*/


void UInterpTrackInstSkelControlScale::RestoreActorState(UInterpTrack* Track)
{
	UInterpTrackSkelControlScale* ScaleTrack = CastChecked<UInterpTrackSkelControlScale>(Track);
	AActor* Actor = GetGroupActor();
	if(Actor)
	{
		Actor->PreviewSetSkelControlScale(ScaleTrack->SkelControlName, 1.f);
	}
}



/*-----------------------------------------------------------------------------
	UInterpTrackAudioMaster
-----------------------------------------------------------------------------*/

/** Add a new keyframe at the specified time. Returns index of new keyframe. */
INT UInterpTrackAudioMaster::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	const FLOAT DefaultVolume = 1.0f;
	const FLOAT DefaultPitch = 1.0f;

	INT NewKeyIndex = VectorTrack.AddPoint( Time, FVector( DefaultVolume, DefaultPitch, 0.0f ) );
	VectorTrack.Points( NewKeyIndex ).InterpMode = InitInterpMode;

	VectorTrack.AutoSetTangents( CurveTension );

	return NewKeyIndex;
}


/** Change the value of an existing keyframe. */
void UInterpTrackAudioMaster::UpdateKeyframe(INT KeyIndex, UInterpTrackInst* TrInst)
{
}


/** This is called in the editor, when scrubbing/previewing etc. */
void UInterpTrackAudioMaster::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
}


/** This is called in the game, when USeqAct_Interp is ticked */
void UInterpTrackAudioMaster::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
}


/** Set the AudioMaster track to defaults */
void UInterpTrackAudioMaster::SetTrackToSensibleDefault()
{
	const FLOAT DefaultVolume = 1.0f;
	const FLOAT DefaultPitch = 1.0f;

	VectorTrack.Points.Empty();
	VectorTrack.AddPoint( 0.0f, FVector( DefaultVolume, DefaultPitch, 0.0f ) );
}	


/** Return the sound volume scale for the specified time */
FLOAT UInterpTrackAudioMaster::GetVolumeScaleForTime( FLOAT Time ) const
{
	const FLOAT DefaultVolume = 1.0f;
	const FLOAT DefaultPitch = 1.0f;
	FVector DefaultVolumePitch( DefaultVolume, DefaultPitch, 0.0f );

	return VectorTrack.Eval( Time, DefaultVolumePitch ).X;		// X = Volume
}


/** Return the sound pitch scale for the specified time */
FLOAT UInterpTrackAudioMaster::GetPitchScaleForTime( FLOAT Time ) const
{
	const FLOAT DefaultVolume = 1.0f;
	const FLOAT DefaultPitch = 1.0f;
	FVector DefaultVolumePitch( DefaultVolume, DefaultPitch, 0.0f );

	return VectorTrack.Eval( Time, DefaultVolumePitch ).Y;		// Y = Pitch
}


/*-----------------------------------------------------------------------------
	UInterpTrackInstAudioMaster
-----------------------------------------------------------------------------*/

void UInterpTrackInstAudioMaster::InitTrackInst(UInterpTrack* Track)
{
}

void UInterpTrackInstAudioMaster::TermTrackInst(UInterpTrack* Track)
{
}




/*-----------------------------------------------------------------------------
	UInterpTrackVisibility
-----------------------------------------------------------------------------*/

STRUCTTRACK_GETNUMKEYFRAMES(UInterpTrackVisibility, VisibilityTrack)
STRUCTTRACK_GETTIMERANGE(UInterpTrackVisibility, VisibilityTrack, Time)
STRUCTTRACK_GETKEYFRAMETIME(UInterpTrackVisibility, VisibilityTrack, Time)
STRUCTTRACK_SETKEYFRAMETIME(UInterpTrackVisibility, VisibilityTrack, Time, FVisibilityTrackKey)
STRUCTTRACK_REMOVEKEYFRAME(UInterpTrackVisibility, VisibilityTrack)
STRUCTTRACK_DUPLICATEKEYFRAME(UInterpTrackVisibility, VisibilityTrack, Time, FVisibilityTrackKey)
STRUCTTRACK_GETCLOSESTSNAPPOSITION(UInterpTrackVisibility, VisibilityTrack, Time)

// InterpTrack interface
INT UInterpTrackVisibility::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	UInterpTrackInstVisibility* VisibilityInst = CastChecked<UInterpTrackInstVisibility>(TrInst);

	INT i = 0;
	for (i = 0; i < VisibilityTrack.Num() && VisibilityTrack(i).Time < Time; i++);
	VisibilityTrack.Insert(i);
	VisibilityTrack(i).Time = Time;
	VisibilityTrack(i).Action = VisibilityInst->Action;

	return i;
}

void UInterpTrackVisibility::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	AActor* Actor = TrInst->GetGroupActor();
	if (Actor == NULL)
	{
		return;
	}

	UInterpTrackInstVisibility* VisibilityInst = CastChecked<UInterpTrackInstVisibility>(TrInst);
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( TrInst->GetOuter() );
	USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );
	UInterpGroup* Group = CastChecked<UInterpGroup>( GetOuter() );
	UInterpData* IData = CastChecked<UInterpData>( Group->GetOuter() );


	// NOTE: We don't fire events when jumping forwards in Matinee preview since that would
	//       fire off particles while scrubbing, which we currently don't want.
	const UBOOL bShouldActuallyFireEventsWhenJumpingForwards =
		bFireEventsWhenJumpingForwards && !( GIsEditor && !GWorld->HasBegunPlay() );

	// @todo: Make this configurable?
	const UBOOL bInvertBoolLogicWhenPlayingBackwards = TRUE;


	// We'll consider playing events in reverse if we're either actively playing in reverse or if
	// we're in a paused state but forcing an update to an older position (scrubbing backwards in editor.)
	UBOOL bIsPlayingBackwards =
		( Seq->bIsPlaying && Seq->bReversePlayback ) ||
		( bJump && !Seq->bIsPlaying && NewPosition < VisibilityInst->LastUpdatePosition );


	// Find the interval between last update and this to check events with.
	UBOOL bFireEvents = TRUE;


	if( bJump )
	{
		// If we are playing forwards, and the flag is set, fire events even if we are 'jumping'.
		if( bShouldActuallyFireEventsWhenJumpingForwards && !bIsPlayingBackwards )
		{
			bFireEvents = TRUE;
		}
		else
		{
			bFireEvents = FALSE;
		}
	}

	// If playing sequence forwards.
	FLOAT MinTime, MaxTime;
	if( !bIsPlayingBackwards )
	{
		MinTime = VisibilityInst->LastUpdatePosition;
		MaxTime = NewPosition;

		// Slight hack here.. if playing forwards and reaching the end of the sequence, force it over a little to ensure we fire events actually on the end of the sequence.
		if( MaxTime == IData->InterpLength )
		{
			MaxTime += (FLOAT)KINDA_SMALL_NUMBER;
		}

		if( !bFireEventsWhenForwards )
		{
			bFireEvents = FALSE;
		}
	}
	// If playing sequence backwards.
	else
	{
		MinTime = NewPosition;
		MaxTime = VisibilityInst->LastUpdatePosition;

		// Same small hack as above for backwards case.
		if( MinTime == 0.0f )
		{
			MinTime -= (FLOAT)KINDA_SMALL_NUMBER;
		}

		if( !bFireEventsWhenBackwards )
		{
			bFireEvents = FALSE;
		}
	}


	// If we should be firing events for this track...
	if( bFireEvents )
	{
		// See which events fall into traversed region.
		for(INT CurKeyIndex = 0; CurKeyIndex < VisibilityTrack.Num(); ++CurKeyIndex )
		{
			// Iterate backwards if we're playing in reverse so that toggles are applied in the correct order
			INT ActualKeyIndex = CurKeyIndex;
			if( bIsPlayingBackwards )
			{
				ActualKeyIndex = ( VisibilityTrack.Num() - 1 ) - CurKeyIndex;
			}

			FVisibilityTrackKey& VisibilityKey = VisibilityTrack( ActualKeyIndex );

			FLOAT EventTime = VisibilityKey.Time;

			// Need to be slightly careful here and make behavior for firing events symmetric when playing forwards of backwards.
			UBOOL bFireThisEvent = FALSE;
			if( !bIsPlayingBackwards )
			{
				if( EventTime >= MinTime && EventTime < MaxTime )
				{
					bFireThisEvent = TRUE;
				}
			}
			else
			{
				if( EventTime > MinTime && EventTime <= MaxTime )
				{
					bFireThisEvent = TRUE;
				}
			}

			if( bFireThisEvent )
			{
				// NOTE: Because of how Toggle keys work, we need to run every event in the range, not
				//       just the last event.

				if( Actor != NULL )
				{
					// Make sure the key's condition is satisfied
					if( !( VisibilityKey.ActiveCondition == EVTC_GoreEnabled && !Seq->bShouldShowGore ) &&
						!( VisibilityKey.ActiveCondition == EVTC_GoreDisabled && Seq->bShouldShowGore ) )
					{
						if( VisibilityKey.Action == EVTA_Show )
						{
							UBOOL bShouldHide = FALSE;
							if( bInvertBoolLogicWhenPlayingBackwards && bIsPlayingBackwards )
							{
								// Playing in reverse, so invert bool logic (Show -> Hide)
								bShouldHide = TRUE;
							}

							// Show the actor
							Actor->SetHidden( bShouldHide );
						}
						else if( VisibilityKey.Action == EVTA_Hide )
						{
							UBOOL bShouldHide = TRUE;
							if( bInvertBoolLogicWhenPlayingBackwards && bIsPlayingBackwards )
							{
								// Playing in reverse, so invert bool logic (Hide -> Show)
								bShouldHide = FALSE;
							}

							// Hide the actor
							Actor->SetHidden( bShouldHide );
						}
						else if( VisibilityKey.Action == EVTA_Toggle )
						{
							// Toggle the actor's visibility
							Actor->SetHidden( !Actor->bHidden );
						}
						if (VisibilityKey.ActiveCondition == EVTC_Always)
						{
							Actor->bNetDirty = TRUE;
							Actor->eventForceNetRelevant();
						}
					}
				}
			}	
		}
	}

	VisibilityInst->LastUpdatePosition = NewPosition;
}

/** 
 *	Function which actually updates things based on the new position in the track. 
 *  This is called in the editor, when scrubbing/previewing etc.
 */
void UInterpTrackVisibility::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( TrInst->GetOuter() );
	USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );

	// Dont play sounds unless we are preview playback (ie not scrubbing).
	UBOOL bJump = !(Seq->bIsPlaying);
	UpdateTrack(NewPosition, TrInst, bJump);
}

/** 
 *	Get the name of the class used to help out when adding tracks, keys, etc. in UnrealEd.
 *
 *	@return		String name of the helper class.
 */
const FString UInterpTrackVisibility::GetEdHelperClassName() const
{
	return FString( TEXT("UnrealEd.InterpTrackVisibilityHelper") );
}


/*-----------------------------------------------------------------------------
	UInterpTrackInstVisibility
-----------------------------------------------------------------------------*/
/** Initialise this Track instance. Called in-game before doing any interpolation. */
void UInterpTrackInstVisibility::InitTrackInst(UInterpTrack* Track)
{

}





/*-----------------------------------------------------------------------------
	UInterpTrackParticleReplay
-----------------------------------------------------------------------------*/

STRUCTTRACK_GETNUMKEYFRAMES(UInterpTrackParticleReplay, TrackKeys)
STRUCTTRACK_GETTIMERANGE(UInterpTrackParticleReplay, TrackKeys, Time)
STRUCTTRACK_GETKEYFRAMETIME(UInterpTrackParticleReplay, TrackKeys, Time)
STRUCTTRACK_SETKEYFRAMETIME(UInterpTrackParticleReplay, TrackKeys, Time, FParticleReplayTrackKey)
STRUCTTRACK_REMOVEKEYFRAME(UInterpTrackParticleReplay, TrackKeys)
STRUCTTRACK_DUPLICATEKEYFRAME(UInterpTrackParticleReplay, TrackKeys, Time, FParticleReplayTrackKey)
STRUCTTRACK_GETCLOSESTSNAPPOSITION(UInterpTrackParticleReplay, TrackKeys, Time)

// InterpTrack interface
INT UInterpTrackParticleReplay::AddKeyframe( FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode )
{
	UInterpTrackInstParticleReplay* ParticleReplayInst = CastChecked<UInterpTrackInstParticleReplay>(TrInst);

	// Figure out which key we should insert before by testing key time values
	INT InsertBeforeIndex = 0;
	while( InsertBeforeIndex < TrackKeys.Num() && TrackKeys( InsertBeforeIndex ).Time < Time )
	{
		++InsertBeforeIndex;
	}

	// Create new key frame
	FParticleReplayTrackKey NewKey;
	NewKey.Time = Time;
	NewKey.ClipIDNumber = 1;	// Default clip ID number
	NewKey.Duration = 1.0f;		// Default duration

	// Insert the new key
	TrackKeys.InsertItem( NewKey, InsertBeforeIndex );

	return InsertBeforeIndex;
}



void UInterpTrackParticleReplay::UpdateTrack( FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump )
{
	AActor* Actor = TrInst->GetGroupActor();
	if (Actor == NULL)
	{
		return;
	}

	UInterpTrackInstParticleReplay* ParticleReplayInst = CastChecked<UInterpTrackInstParticleReplay>(TrInst);

	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( TrInst->GetOuter() );
	USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );

	// Particle replay tracks are expecting to be dealing with emitter actors
	AEmitter* EmitterActor = Cast< AEmitter >( Actor );
	if( EmitterActor != NULL && EmitterActor->ParticleSystemComponent != NULL )
	{
		if( ( NewPosition > ParticleReplayInst->LastUpdatePosition ) && !bJump )
		{
			for (INT KeyIndex = 0; KeyIndex < TrackKeys.Num(); KeyIndex++)
			{
				FParticleReplayTrackKey& ParticleReplayKey = TrackKeys(KeyIndex);

				// Check to see if we hit this key's start time
				if( ( ParticleReplayKey.Time < NewPosition ) && ( ParticleReplayKey.Time >= ParticleReplayInst->LastUpdatePosition ) )
				{
					if( bIsCapturingReplay )
					{
						// Do we already have data for this clip?
						UParticleSystemReplay* ExistingClipReplay =
							EmitterActor->ParticleSystemComponent->FindReplayClipForIDNumber( ParticleReplayKey.ClipIDNumber );
						if( ExistingClipReplay != NULL )
						{
							// Clear the existing clip's frame data.  We're re-recording the clip now!
							ExistingClipReplay->Frames.Empty();
						}

						// Start capturing!
						EmitterActor->ParticleSystemComponent->ReplayState = PRS_Capturing;
						EmitterActor->ParticleSystemComponent->ReplayClipIDNumber = ParticleReplayKey.ClipIDNumber;
						EmitterActor->ParticleSystemComponent->ReplayFrameIndex = 0;

						// Make sure we're alive and kicking
						EmitterActor->ParticleSystemComponent->SetActive( TRUE );
					}
					else
					{
						// Start playback!
						EmitterActor->ParticleSystemComponent->ReplayState = PRS_Replaying;
						EmitterActor->ParticleSystemComponent->ReplayClipIDNumber = ParticleReplayKey.ClipIDNumber;
						EmitterActor->ParticleSystemComponent->ReplayFrameIndex = 0;

						// Make sure we're alive and kicking
						EmitterActor->ParticleSystemComponent->SetActive( TRUE );
					}
				}

				// Check to see if we hit this key's end time
				const FLOAT KeyEndTime = ParticleReplayKey.Time + ParticleReplayKey.Duration;
				if( ( KeyEndTime < NewPosition ) && ( KeyEndTime >= ParticleReplayInst->LastUpdatePosition ) )
				{
					if( !bIsCapturingReplay )
					{
						// Done playing back replay sequence, so turn off the particle system
						EmitterActor->ParticleSystemComponent->SetActive( FALSE );

						// Stop playback/capture!  We'll still keep the particle system in 'replay mode' while
						// the replay track is bound so that the system doesn't start simulating/rendering
						EmitterActor->ParticleSystemComponent->ReplayState = PRS_Replaying;
						EmitterActor->ParticleSystemComponent->ReplayClipIDNumber = INDEX_NONE;
						EmitterActor->ParticleSystemComponent->ReplayFrameIndex = INDEX_NONE;
					}
				}
			}
		}


		// Are we 'jumping in time'? (scrubbing)
		if( bJump )
		{
			if( bIsCapturingReplay )
			{
				// Scrubbing while capturing will stop the capture
				EmitterActor->ParticleSystemComponent->ReplayState = PRS_Disabled;
			}
			else
			{
				// Scrubbing while replaying with render the specific frame of the particle system

				// Find the time that the last replay was started
				UBOOL bHaveReplayStartKey = FALSE;
				FParticleReplayTrackKey CurrentReplayStartKey;
				for( INT KeyIndex = TrackKeys.Num() - 1; KeyIndex >= 0; --KeyIndex )
				{
					FParticleReplayTrackKey& ParticleReplayKey = TrackKeys( KeyIndex );

					// Check to see if we hit this key's start time
					if( ParticleReplayKey.Time < NewPosition )
					{
						CurrentReplayStartKey = ParticleReplayKey;
						bHaveReplayStartKey = TRUE;
						break;
					}
				}

				UBOOL bIsReplayingSingleFrame = FALSE;
				if( bHaveReplayStartKey )
				{
					const FLOAT TimeWithinReplay = NewPosition - CurrentReplayStartKey.Time;
					const INT ReplayFrameIndex = appTrunc( TimeWithinReplay / Max( ( FLOAT )KINDA_SMALL_NUMBER, FixedTimeStep ) );


					// Check to see if we have a clip
					UParticleSystemReplay* ParticleSystemReplay =
						EmitterActor->ParticleSystemComponent->FindReplayClipForIDNumber( CurrentReplayStartKey.ClipIDNumber );
					if( ParticleSystemReplay != NULL )
					{
						if( ReplayFrameIndex < ParticleSystemReplay->Frames.Num() )
						{
							// Playback specific frame!
							bIsReplayingSingleFrame = TRUE;

							// Make sure replay mode is turned on
							EmitterActor->ParticleSystemComponent->ReplayState = PRS_Replaying;
							EmitterActor->ParticleSystemComponent->ReplayClipIDNumber = CurrentReplayStartKey.ClipIDNumber;
							EmitterActor->ParticleSystemComponent->ReplayFrameIndex = ReplayFrameIndex;

							// Make sure we're alive and kicking
							EmitterActor->ParticleSystemComponent->SetActive( TRUE );
						}
					}
				}

				if( !bIsReplayingSingleFrame )
				{
					// Stop playback!  We'll still keep the particle system in 'replay mode' while
					// the replay track is bound so that the system doesn't start simulating/rendering
					EmitterActor->ParticleSystemComponent->ReplayState = PRS_Replaying;
					EmitterActor->ParticleSystemComponent->ReplayClipIDNumber = INDEX_NONE;
					EmitterActor->ParticleSystemComponent->ReplayFrameIndex = INDEX_NONE;

					// We're not currently capturing and we're not in the middle of a replay frame,
					// so turn off the particle system
					EmitterActor->ParticleSystemComponent->SetActive( FALSE );
				}
			}
		}
		else
		{
			// Okay, we're not scrubbing, but are we replaying a particle system?
			if( EmitterActor->ParticleSystemComponent->ReplayState == PRS_Replaying )
			{
				// Advance to next frame (or reverse to the previous frame)
				if( Seq->bReversePlayback )
				{
					--EmitterActor->ParticleSystemComponent->ReplayFrameIndex;
				}
				else
				{
					++EmitterActor->ParticleSystemComponent->ReplayFrameIndex;
				}
			}
		}
	}


	ParticleReplayInst->LastUpdatePosition = NewPosition;
}


/** 
 *	Function which actually updates things based on the new position in the track. 
 *  This is called in the editor, when scrubbing/previewing etc.
 */
void UInterpTrackParticleReplay::PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst)
{
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( TrInst->GetOuter() );
	USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );

	UBOOL bJump = !(Seq->bIsPlaying);
	UpdateTrack(NewPosition, TrInst, bJump);
}



/** 
 *	Get the name of the class used to help out when adding tracks, keys, etc. in UnrealEd.
 *
 *	@return		String name of the helper class.
 */
const FString UInterpTrackParticleReplay::GetEdHelperClassName() const
{
	return FString( TEXT("UnrealEd.InterpTrackParticleReplayHelper") );
}


/*-----------------------------------------------------------------------------
	UInterpTrackInstParticleReplay
-----------------------------------------------------------------------------*/

/** Initialise this Track instance. Called in-game before doing any interpolation. */
void UInterpTrackInstParticleReplay::InitTrackInst(UInterpTrack* Track)
{

}


/** Restore the saved state of this Actor. */
void UInterpTrackInstParticleReplay::RestoreActorState(UInterpTrack* Track)
{
	AActor* Actor = GetGroupActor();
	if( Actor != NULL )
	{
		// Particle replay tracks are expecting to be dealing with emitter actors
		AEmitter* EmitterActor = Cast< AEmitter >( Actor );
		if( EmitterActor != NULL && EmitterActor->ParticleSystemComponent != NULL )
		{
			// Make sure we don't leave the particle system in 'capture mode'
			
			// Stop playback/capture!  We'll still keep the particle system in 'replay mode' while
			// the replay track is bound so that the system doesn't start simulating/rendering
			EmitterActor->ParticleSystemComponent->ReplayState = PRS_Disabled;
			EmitterActor->ParticleSystemComponent->ReplayClipIDNumber = 0;
			EmitterActor->ParticleSystemComponent->ReplayFrameIndex = 0;
		}
	}
}

