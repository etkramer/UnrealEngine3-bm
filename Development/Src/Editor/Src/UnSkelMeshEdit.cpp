/*=============================================================================
	UnSkelMeshEdit.cpp: Unreal editor skeletal mesh/anim support
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "EngineAnimClasses.h"
#include "SkelImport.h"
#include "UnColladaImporter.h"
#include "AnimationUtils.h"

/**
 * Open a PSA file with the given name, and import each sequence into the supplied AnimSet.
 * This is only possible if each track expected in the AnimSet is found in the target file. If not the case, a warning is given and import is aborted.
 * If AnimSet is empty (ie. TrackBoneNames is empty) then we use this PSA file to form the track names array.
 */
void UEditorEngine::ImportPSAIntoAnimSet( UAnimSet* AnimSet, const TCHAR* Filename, class USkeletalMesh* FillInMesh )
{
	// Open skeletal animation key file and read header.
	FArchive* AnimFile = GFileManager->CreateFileReader( Filename, 0, GLog );
	if( !AnimFile )
	{
		appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_ErrorOpeningAnimFile"), Filename) );
		return;
	}

	// Read main header
	VChunkHeader ChunkHeader;
	AnimFile->Serialize( &ChunkHeader, sizeof(VChunkHeader) );
	if( AnimFile->IsError() )
	{
		appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_ErrorReadingAnimFile"), Filename) );
		delete AnimFile;
		return;
	}

	/////// Read the bone names - convert into array of FNames.
	AnimFile->Serialize( &ChunkHeader, sizeof(VChunkHeader) );

	INT NumPSATracks = ChunkHeader.DataCount; // Number of tracks of animation. One per bone.

	TArray<FNamedBoneBinary> RawBoneNamesBin;
	RawBoneNamesBin.Add(NumPSATracks);
	AnimFile->Serialize( &RawBoneNamesBin(0), sizeof(FNamedBoneBinary) * ChunkHeader.DataCount);

	TArray<FName> RawBoneNames;
	RawBoneNames.Add(NumPSATracks);
	for(INT i=0; i<NumPSATracks; i++)
	{
		FNamedBoneBinary& Bone = RawBoneNamesBin( i );
		FString BoneName = FSkeletalMeshBinaryImport::FixupBoneName( Bone.Name );
		RawBoneNames( i ) = FName( *BoneName );
	}

	/////// Read the animation sequence infos
	AnimFile->Serialize( &ChunkHeader, sizeof(VChunkHeader) );

	INT NumPSASequences = ChunkHeader.DataCount;

	TArray<AnimInfoBinary> RawAnimSeqInfo; // Array containing per-sequence information (sequence name, key range etc)
	RawAnimSeqInfo.Add(NumPSASequences);
	AnimFile->Serialize( &RawAnimSeqInfo(0), sizeof(AnimInfoBinary) * ChunkHeader.DataCount);


	/////// Read the raw keyframe data (1 key per
	AnimFile->Serialize( &ChunkHeader, sizeof(VChunkHeader) );

	TArray<VQuatAnimKey> RawAnimKeys;
	RawAnimKeys.Add(ChunkHeader.DataCount);

	AnimFile->Serialize( &RawAnimKeys(0), sizeof(VQuatAnimKey) * ChunkHeader.DataCount);	

	// Have now read all information from file - can release handle.
	delete AnimFile;
	AnimFile = NULL;

	// Y-flip quaternions and translations from Max/Maya/etc space into Unreal space.
	for( INT i=0; i<RawAnimKeys.Num(); i++)
	{
		FQuat Bone = RawAnimKeys(i).Orientation;
		Bone.W = - Bone.W;
		Bone.Y = - Bone.Y;
		RawAnimKeys(i).Orientation = Bone;

		FVector Pos = RawAnimKeys(i).Position;
		Pos.Y = - Pos.Y;
		RawAnimKeys(i).Position = Pos;
	}


	// Calculate track mapping from target tracks in AnimSet to the source amoung those we are loading.
	TArray<INT> TrackMap;

	if(AnimSet->TrackBoneNames.Num() == 0)
	{
		AnimSet->TrackBoneNames.Add( NumPSATracks );
		TrackMap.Add( AnimSet->TrackBoneNames.Num() );

		for(INT i=0; i<AnimSet->TrackBoneNames.Num(); i++)
		{
			// If an empty AnimSet, we create the AnimSet track name table from this PSA file.
			AnimSet->TrackBoneNames(i) = RawBoneNames(i);

			TrackMap(i) = i;
		}
	}
	else
	{
		// Otherwise, ensure right track goes to right place.
		// If we are missing a track, we give a warning and refuse to import into this set.

		TrackMap.Add( AnimSet->TrackBoneNames.Num() );

		// For each track in the AnimSet, find its index in the imported data
		for(INT i=0; i<AnimSet->TrackBoneNames.Num(); i++)
		{
			FName TrackName = AnimSet->TrackBoneNames(i);

			TrackMap(i) = INDEX_NONE;

			for(INT j=0; j<NumPSATracks; j++)
			{
				FName TestName = RawBoneNames(j);
				if(TestName == TrackName)
				{	
					if( TrackMap(i) != INDEX_NONE )
					{
						debugf( TEXT(" DUPLICATE TRACK IN INCOMING DATA: %s"), *TrackName.ToString() );
					}
					TrackMap(i) = j;
				}
			}

			// If we failed to find a track we need in the imported data - see if we want to patch it using the skeletal mesh ref pose.
			if(TrackMap(i) == INDEX_NONE)
			{
				UBOOL bDoPatching = appMsgf(AMT_YesNo, LocalizeSecure(LocalizeUnrealEd("Error_CouldNotFindTrack"), *TrackName.ToString()));
				if(bDoPatching)
				{
					// Check the selected skel mesh has a bone called that. If we can't find it - fail.
					INT PatchBoneIndex = FillInMesh->MatchRefBone(TrackName);
					if(PatchBoneIndex == INDEX_NONE)
					{
						appMsgf(AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_CouldNotFindPatchBone"), *TrackName.ToString()));
						return;
					}
				}
				else
				{
					return;
				}
			}
		}
	}

	// Flag to indicate that the AnimSet has had its TrackBoneNames array changed, and therefore its LinkupCache needs to be flushed.
	UBOOL bAnimSetTrackChanged = FALSE;

	// Now we see if there are any tracks in the incoming data which do not have a use in the AnimSet. 
	// These will be thrown away unless we extend the AnimSet name table.
	TArray<FName> AnimSetMissingNames;
	TArray<UAnimSequence*> SequencesRequiringRecompression;
	for(INT i=0; i<NumPSATracks; i++)
	{
		if(!TrackMap.ContainsItem(i))
		{
			FName ExtraTrackName = RawBoneNames(i);
			UBOOL bDoExtension = appMsgf(AMT_YesNo, LocalizeSecure(LocalizeUnrealEd("Error_ExtraInfoInPSA"), *ExtraTrackName.ToString()));
			if(bDoExtension)
			{
				INT PatchBoneIndex = FillInMesh->MatchRefBone(ExtraTrackName);

				// If we can't find the extra track in the SkelMesh to create an animation from, warn and fail.
				if(PatchBoneIndex == INDEX_NONE)
				{
					appMsgf(AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_CouldNotFindPatchBone"), *ExtraTrackName.ToString()));
					return;
				}
				// If we could, add to all animations in the AnimSet, and add to track table.
				else
				{
					AnimSet->TrackBoneNames.AddItem(ExtraTrackName);
					bAnimSetTrackChanged = TRUE;

					// Iterate over all existing sequences in this set and add an extra track to the end.
					for(INT SetAnimIndex=0; SetAnimIndex<AnimSet->Sequences.Num(); SetAnimIndex++)
					{
						UAnimSequence* ExtendSeq = AnimSet->Sequences(SetAnimIndex);

						// Remove any compression on the sequence so that it will be recomputed with the new track.
						if ( ExtendSeq->CompressedTrackOffsets.Num() > 0 )
						{
							ExtendSeq->CompressedTrackOffsets.Empty();
							// Mark the sequence as requiring recompression.
							SequencesRequiringRecompression.AddUniqueItem( ExtendSeq );
						}

						// Add an extra track to the end, based on the ref skeleton.
						ExtendSeq->RawAnimData.AddZeroed();
						FRawAnimSequenceTrack& RawTrack = ExtendSeq->RawAnimData( ExtendSeq->RawAnimData.Num()-1 );

						// Create 1-frame animation from the reference pose of the skeletal mesh.
						// This is basically what the compression does, so should be fine.
						if( ExtendSeq->bIsAdditive )
						{
							RawTrack.PosKeys.AddItem(FVector(0.f));

							FQuat RefOrientation = FQuat::Identity;
							// To emulate ActorX-exported animation quat-flipping, we do it here.
							if( PatchBoneIndex > 0 )
							{
								RefOrientation.W *= -1.f;
							}
							RawTrack.RotKeys.AddItem(RefOrientation);
							RawTrack.KeyTimes.AddItem( 0.f );

							// Extend AdditiveRefPose
							const FMeshBone& RefSkelBone = FillInMesh->RefSkeleton(PatchBoneIndex);

							FBoneAtom RefBoneAtom = FBoneAtom::Identity;
							RefBoneAtom.Translation = RefSkelBone.BonePos.Position;
							RefBoneAtom.Rotation    = RefSkelBone.BonePos.Orientation;
							if( PatchBoneIndex > 0)
							{
								RefBoneAtom.Rotation.W *= -1.f; // As above - flip if necessary
							}

							// Save off RefPose into destination AnimSequence
							ExtendSeq->AdditiveRefPose.AddItem(RefBoneAtom);
						}
						else
						{
							const FVector RefPosition = FillInMesh->RefSkeleton(PatchBoneIndex).BonePos.Position;
							RawTrack.PosKeys.AddItem(RefPosition);

							FQuat RefOrientation = FillInMesh->RefSkeleton(PatchBoneIndex).BonePos.Orientation;
							// To emulate ActorX-exported animation quat-flipping, we do it here.
							if( PatchBoneIndex > 0 )
							{
								RefOrientation.W *= -1.f;
							}
							RawTrack.RotKeys.AddItem(RefOrientation);
							RawTrack.KeyTimes.AddItem( 0.f );
						}
					}

					// So now the new incoming track 'i' maps to the last track in the AnimSet.
					TrackMap.AddItem(i);
				}
			}
		}
	}

	// For each sequence in the PSA file, find the UAnimSequence we want to import into.
	for(INT SeqIdx=0; SeqIdx<NumPSASequences; SeqIdx++)
	{
		AnimInfoBinary& PSASeqInfo = RawAnimSeqInfo(SeqIdx);

		// Get sequence name as an FName
		appTrimSpaces( &PSASeqInfo.Name[0] );
		FName SeqName = FName( ANSI_TO_TCHAR(&PSASeqInfo.Name[0]) );

		// See if this sequence already exists.
		UAnimSequence* DestSeq = AnimSet->FindAnimSequence(SeqName);

		// If not, create new one now.
		if(!DestSeq)
		{
			DestSeq = ConstructObject<UAnimSequence>( UAnimSequence::StaticClass(), AnimSet );
			AnimSet->Sequences.AddItem( DestSeq );
		}
		else
		{
			UBOOL bDoReplace = appMsgf( AMT_YesNo, LocalizeSecure(LocalizeUnrealEd("Prompt_25"), *SeqName.ToString()) );
			if(!bDoReplace)
			{
				continue; // Move on to next sequence...
			}

			DestSeq->RecycleAnimSequence();
		}

		DestSeq->SequenceName = SeqName;
		DestSeq->SequenceLength = PSASeqInfo.TrackTime / PSASeqInfo.AnimRate; // Time of animation if playback speed was 1.0
		DestSeq->NumFrames = PSASeqInfo.NumRawFrames;

		// Make sure data is zeroes
		DestSeq->RawAnimData.AddZeroed( AnimSet->TrackBoneNames.Num() );

		// Structure of data is this:
		// RawAnimKeys contains all keys. 
		// Sequence info FirstRawFrame and NumRawFrames indicate full-skel frames (NumPSATracks raw keys). Ie number of elements we need to copy from RawAnimKeys is NumRawFrames * NumPSATracks.

		// Import each track.
		for(INT TrackIdx = 0; TrackIdx < AnimSet->TrackBoneNames.Num(); TrackIdx++)
		{
			check( AnimSet->TrackBoneNames.Num() == DestSeq->RawAnimData.Num() );
			FRawAnimSequenceTrack& RawTrack = DestSeq->RawAnimData(TrackIdx);

			// Find the source track for this one in the AnimSet
			INT SourceTrackIdx = TrackMap( TrackIdx );

			// If bone was not found in incoming data, use SkeletalMesh to create the track.
			if(SourceTrackIdx == INDEX_NONE)
			{
				FName TrackName = AnimSet->TrackBoneNames(TrackIdx);
				INT PatchBoneIndex = FillInMesh->MatchRefBone(TrackName);
				check(PatchBoneIndex != INDEX_NONE); // Should have checked for this case above!

				// Create 1-frame animation from the reference pose of the skeletal mesh.
				// This is basically what the compression does, so should be fine.

				FVector RefPosition = FillInMesh->RefSkeleton(PatchBoneIndex).BonePos.Position;
				RawTrack.PosKeys.AddItem(RefPosition);

				FQuat RefOrientation = FillInMesh->RefSkeleton(PatchBoneIndex).BonePos.Orientation;
				// To emulate ActorX-exported animation quat-flipping, we do it here.
				if(PatchBoneIndex > 0)
				{
					RefOrientation.W *= -1.f;
				}
				RawTrack.RotKeys.AddItem(RefOrientation);

				RawTrack.KeyTimes.AddItem( 0.f );
			}
			else
			{
				check(SourceTrackIdx >= 0 && SourceTrackIdx < NumPSATracks);

				RawTrack.PosKeys.Add(DestSeq->NumFrames);
				RawTrack.RotKeys.Add(DestSeq->NumFrames);
				RawTrack.KeyTimes.Add(DestSeq->NumFrames);

				for(INT KeyIdx = 0; KeyIdx < DestSeq->NumFrames; KeyIdx++)
				{
					INT SrcKeyIdx = ((PSASeqInfo.FirstRawFrame + KeyIdx) * NumPSATracks) + SourceTrackIdx;
					check( RawAnimKeys.IsValidIndex(SrcKeyIdx) );

					VQuatAnimKey& RawSrcKey = RawAnimKeys(SrcKeyIdx);

					RawTrack.PosKeys(KeyIdx) = RawSrcKey.Position;
					RawTrack.RotKeys(KeyIdx) = RawSrcKey.Orientation;
					RawTrack.KeyTimes(KeyIdx) = ((FLOAT)KeyIdx/(FLOAT)DestSeq->NumFrames) * DestSeq->SequenceLength;
				}
			}
		}

		// Apply compression
		FAnimationUtils::CompressAnimSequence(DestSeq, NULL, FALSE, FALSE);
	}

	// If we need to, flush the LinkupCache.
	if( bAnimSetTrackChanged )
	{
		AnimSet->LinkupCache.Empty();

		// We need to re-init any skeletal mesh components now, because they might still have references to linkups in this set.
		for(TObjectIterator<USkeletalMeshComponent> It;It;++It)
		{
			USkeletalMeshComponent* SkelComp = *It;
			if(!SkelComp->IsPendingKill() && !SkelComp->IsTemplate())
			{
				SkelComp->InitAnimTree();
			}
		}

		// Recompress any sequences that need it.
		for( INT SequenceIndex = 0 ; SequenceIndex < SequencesRequiringRecompression.Num() ; ++SequenceIndex )
		{
			UAnimSequence* AnimSeq = SequencesRequiringRecompression( SequenceIndex );
			FAnimationUtils::CompressAnimSequence(AnimSeq, NULL, FALSE, FALSE);
		}
	}
}

void UEditorEngine::ImportCOLLADAANIMIntoAnimSet(UAnimSet* AnimSet, const TCHAR* InFilename, USkeletalMesh* FillInMesh)
{
	GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("ImportingCOLLADAAnimations")), TRUE );

	UnCollada::CImporter* ColladaImporter = UnCollada::CImporter::GetInstance();
	if ( !ColladaImporter->ImportFromFile( InFilename ) )
	{
		// Log the error message and fail the import.
		GWarn->Log( NAME_Error, ColladaImporter->GetErrorMessage() );
	}
	else
	{
		// Simply log the import message and import the animation
		GWarn->Log( ColladaImporter->GetErrorMessage() );

		const FFilename Filename( InFilename );
		ColladaImporter->ImportAnimSet( AnimSet, FillInMesh, *Filename.GetBaseFilename() );
	}

	ColladaImporter->CloseDocument();
	GWarn->EndSlowTask();
}
