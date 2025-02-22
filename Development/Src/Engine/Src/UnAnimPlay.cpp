/*=============================================================================
	UnAnimPlay.cpp: Skeletal mesh animation  - UAnimNodeSequence implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"
#include "AnimationEncodingFormat.h"

IMPLEMENT_CLASS(UAnimNodeSequence);
IMPLEMENT_CLASS(UAnimNodeSequenceBlendBase);
IMPLEMENT_CLASS(UAnimNodeSequenceBlendByAim);


UBOOL UAnimNodeSequence::GetCachedResults(FBoneAtomArray& OutAtoms, FBoneAtom& OutRootMotionDelta, INT& bOutHasRootMotion)
{
	check(SkelComponent);

	// See if cached array is the same size as the target array.
	// Don't check for CachedAtomsTag as if the animation is paused, we don't want to recache it.
	if( CachedBoneAtoms.Num() == OutAtoms.Num() )
	{
		OutAtoms = CachedBoneAtoms;

		// Only use cached root motion if animation has been cached this frame.
		// We don't want a paused animation to send our mesh drifting away.
		if( NodeCachedAtomsTag == SkelComponent->CachedAtomsTag )
		{
			OutRootMotionDelta = CachedRootMotionDelta;
			bOutHasRootMotion = bCachedHasRootMotion;
		}
		else
		{
			OutRootMotionDelta = FBoneAtom::Identity;
			bOutHasRootMotion = 0;
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/** 
 * Cache results always if bCacheAnimSequenceNodes == TRUE. (Optimization if animation is not frequently updated).
 * or if node has more than a single parent.
 */
UBOOL UAnimNodeSequence::ShouldSaveCachedResults()
{
	return (SkelComponent->bCacheAnimSequenceNodes || (ParentNodes.Num() > 1) || (AnimSeq && AnimSeq->NumFrames == 1) || ShouldSkipWhenMeshNotRendered() );
}

UBOOL UAnimNodeSequence::ShouldSkipWhenMeshNotRendered()
{
	if( !SkelComponent->bRecentlyRendered && bSkipTickWhenZeroWeight && !GIsEditor )
	{
		// Are we doing root motion for this node?
		const UBOOL bDoRootTranslation	= (RootBoneOption[0] != RBA_Default) || (RootBoneOption[1] != RBA_Default) || (RootBoneOption[2] != RBA_Default);
		const UBOOL bDoRootRotation		= (RootRotationOption[0] != RRO_Default) || (RootRotationOption[1] != RRO_Default) || (RootRotationOption[2] != RRO_Default);
		return !bDoRootTranslation && !bDoRootRotation;
	}

	return FALSE;
}

void UAnimNodeSequence::ConditionalClearCachedData()
{
	// Clear out the cached data.
	if( CachedBoneAtoms.Num() > 0 && !ShouldSkipWhenMeshNotRendered() )
	{
		CachedBoneAtoms.Reset();
	}
}

FLOAT UAnimNodeSequence::GetSliderPosition(INT SliderIndex, INT ValueIndex)
{
	check(0 == SliderIndex && 0 == ValueIndex);

	if( AnimSeq && AnimSeq->SequenceLength > 0.f )
	{
		return (CurrentTime / AnimSeq->SequenceLength);
	}
	return 0.f;
}

void UAnimNodeSequence::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(0 == SliderIndex && 0 == ValueIndex);

	if( !AnimSeq || AnimSeq->SequenceLength == 0.f )
	{
		return;
	}

	const FLOAT NewTime = NewSliderValue * AnimSeq->SequenceLength;
	SetPosition(NewTime, FALSE);
}

FString UAnimNodeSequence::GetSliderDrawValue(INT SliderIndex)
{
	check(0 == SliderIndex);

	if( !AnimSeq || AnimSeq->SequenceLength == 0.f )
	{
		return FString::Printf(TEXT("N/A"));
	}

	return FString::Printf(TEXT("Pos: %3.2f%%, Time: %3.2fs"), (CurrentTime/AnimSeq->SequenceLength)*100.f, CurrentTime);
}


void UAnimNodeSequence::PostEditChange(UProperty* PropertyThatChanged)
{
	SetAnim( AnimSeqName );

	if(SkelComponent && SkelComponent->IsAttached())
	{
		SkelComponent->UpdateSkelPose();
		SkelComponent->ConditionalUpdateTransform();
	}

	Super::PostEditChange(PropertyThatChanged);
}

void UAnimNodeSequence::BeginDestroy()
{
	// make sure we're not still trying to play a camera anim
	if (ActiveCameraAnimInstance)
	{
		StopCameraAnim();
	}

	Super::BeginDestroy();
}


void UAnimNodeSequence::InitAnim( USkeletalMeshComponent* meshComp, UAnimNodeBlendBase* Parent )
{
	Super::InitAnim( meshComp, Parent );

	SetAnim( AnimSeqName );

	if (ActiveCameraAnimInstance != NULL)
	{
		StopCameraAnim();
	}
}


/** AnimSets have been updated, update all animations */
void UAnimNodeSequence::AnimSetsUpdated()
{
	SetAnim( AnimSeqName );
}

void UAnimNodeSequence::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	// If this node is part of a group, animation is going to be advanced in a second pass, once all weights have been calculated in the tree.
	if( SynchGroupName == NAME_None )
	{
		// Keep track of PreviousTime before any update. This is used by Root Motion.
		PreviousTime = CurrentTime;

		// Can only do something if we are currently playing a valid AnimSequence
		if( bPlaying && AnimSeq )
		{
			// Time to move forwards by - DeltaSeconds scaled by various factors.
			const FLOAT MoveDelta = Rate * AnimSeq->RateScale * SkelComponent->GlobalAnimRateScale * DeltaSeconds;
			AdvanceBy(MoveDelta, DeltaSeconds, (SkelComponent->bUseRawData) ? FALSE : TRUE);
		}
	}

	if (ActiveCameraAnimInstance != NULL)
	{
		// check and see if we still have a camera associated with our skeletal mesh
		AAnimatedCamera* const Cam = GetPlayerCamera();
		if ( Cam && bPlaying && bRelevant )
		{
			// associated camera anims are weighted the same as this node
			ActiveCameraAnimInstance->ApplyTransientScaling(NodeTotalWeight);
		}
		else
		{
			StopCameraAnim();
		}
	}
	else if ( CameraAnim && bPlaying && bLoopCameraAnim && bRelevant )
	{
		// looping camera anims should always be playing, so make sure it's on whenever 
		// note: nonlooping camera anims will play when the real anim starts
		StartCameraAnim();
	}
}

/** Advance animation time. Take care of issuing notifies, looping and so on */
void UAnimNodeSequence::AdvanceBy(FLOAT MoveDelta, FLOAT DeltaSeconds, UBOOL bFireNotifies)
{
	if( !AnimSeq || MoveDelta == 0.f || DeltaSeconds == 0.f )
	{
		return;
	}

	// If animation has only 1 frame, then no need to clear it.
	if( AnimSeq->NumFrames > 1 )
	{
		// Animation updated, clear cached data.
		ConditionalClearCachedData();
	}

	// This node should try to fire notifies
	if( bFireNotifies && MoveDelta > 0.f )
	{
		// Can fire notifies if part of a synchronization group and node is relevant.
		// then bFireNotifies tells us if we should actually fire notifies or not.
		const UBOOL bCanFireNotifyGroup		= SynchGroupName != NAME_None && bRelevant;
	
		// If not part of a group then we check for the weight threshold.
		const UBOOL	bCanFireNotifyNoGroup	= (NodeTotalWeight > NotifyWeightThreshold);

		// Before we actually advance the time, issue any notifies (if desired).
		if( !bNoNotifies && (bCanFireNotifyGroup || bCanFireNotifyNoGroup) )
		{
			IssueNotifies(MoveDelta);

			// If a notification cleared the animation, stop here, don't crash.
			if( !AnimSeq )
			{
				return;
			}
		}
	}

	// Then update internal time.
	CurrentTime	+= MoveDelta;

	// See if we passed the end of the animation.
	if( CurrentTime > AnimSeq->SequenceLength )
	{
		// If we are looping, wrap over.
		if( bLooping )
		{
			CurrentTime	= appFmod(CurrentTime, AnimSeq->SequenceLength);
		}
		// If not, snap time to end of sequence and stop playing.
		else 
		{
			// Find Rate of this movement.
			const FLOAT MoveRate = MoveDelta / DeltaSeconds;

			// figure out by how much we've reached beyond end of animation.
			// This is useful for transitions. It is made independent from play rate
			// So we can transition properly between animations of different play rates
			const FLOAT ExcessTime = (CurrentTime - AnimSeq->SequenceLength) / MoveRate;
			CurrentTime = AnimSeq->SequenceLength;
			StopAnim();

			// Notify that animation finished playing
			OnAnimEnd(DeltaSeconds - ExcessTime, ExcessTime);
		}
	}
	// See if we passed before the beginning of the animation
	else if( CurrentTime < 0.f )
	{
		// If we are looping, wrap over.
		if( bLooping )
		{
			CurrentTime	= appFmod(CurrentTime, AnimSeq->SequenceLength);
			if( CurrentTime < 0.f )
			{
				CurrentTime += AnimSeq->SequenceLength;
			}
		}
		// If not, snap time to beginning of sequence and stop playing.
		else 
		{
			// Find Rate of this movement.
			const FLOAT MoveRate = MoveDelta / DeltaSeconds;

			// figure out by how much we've reached beyond beginning of animation.
			// This is useful for transitions.
			const FLOAT ExcessTime = CurrentTime / Abs(MoveRate);
			CurrentTime = 0.f;
			StopAnim();

			// Notify that animation finished playing
			OnAnimEnd(DeltaSeconds + ExcessTime, ExcessTime);
		}
	}
}


/** 
 * notification that current animation finished playing. 
 * @param	PlayedTime	Time in seconds of animation played. (play rate independent).
 * @param	ExcessTime	Time in seconds beyond end of animation. (play rate independent).
 */
void UAnimNodeSequence::OnAnimEnd(FLOAT PlayedTime, FLOAT ExcessTime)
{
	// When we hit the end and stop, issue notifies to parent AnimNodeBlendBase
	for(INT i=0; i<ParentNodes.Num(); i++)
	{
		ParentNodes(i)->OnChildAnimEnd(this, PlayedTime, ExcessTime); 
	}

	if( bForceRefposeWhenNotPlaying && !SkelComponent->bForceRefpose)
	{
		SkelComponent->SetForceRefPose(TRUE);
	}

	if( bCauseActorAnimEnd && SkelComponent->GetOwner() )
	{
		SkelComponent->GetOwner()->eventOnAnimEnd(this, PlayedTime, ExcessTime);
	}
}

void UAnimNodeSequence::GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion)
{
	START_GETBONEATOM_TIMER

	if( GetCachedResults(Atoms, RootMotionDelta, bHasRootMotion) )
	{
		//debugf(TEXT("%2.3f: %s returning cached atoms"),GWorld->GetTimeSeconds(),*GetPathName());
		return;
	}

	GetAnimationPose(AnimSeq, AnimLinkupIndex, Atoms, DesiredBones, RootMotionDelta, bHasRootMotion);
	SaveCachedResults(Atoms, RootMotionDelta, bHasRootMotion);
}

void UAnimNodeSequence::GetAnimationPose(UAnimSequence* InAnimSeq, INT& InAnimLinkupIndex, FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion)
{
	SCOPE_CYCLE_COUNTER(STAT_GetAnimationPose);

	check(SkelComponent);
	check(SkelComponent->SkeletalMesh);

	// Set root motion delta to identity, so it's always initialized, even when not extracted.
	RootMotionDelta = FBoneAtom::Identity;
	bHasRootMotion	= 0;

	if( !InAnimSeq || InAnimLinkupIndex == INDEX_NONE )
	{
#if 0
		debugf(TEXT("UAnimNodeSequence::GetAnimationPose - %s - No animation data!"), *GetFName());
#endif
		FillWithRefPose(Atoms, DesiredBones, SkelComponent->SkeletalMesh->RefSkeleton);
		return;
	}

	// Get the reference skeleton
	TArray<FMeshBone>& RefSkel = SkelComponent->SkeletalMesh->RefSkeleton;
	const INT NumBones = RefSkel.Num();
	check(NumBones == Atoms.Num());

	UAnimSet* AnimSet = InAnimSeq->GetAnimSet();
	check(InAnimLinkupIndex < AnimSet->LinkupCache.Num());

	FAnimSetMeshLinkup* AnimLinkup = &AnimSet->LinkupCache(InAnimLinkupIndex);

	// @remove me, trying to figure out why this is failing
	if( AnimLinkup->BoneToTrackTable.Num() != NumBones )
	{
		debugf(TEXT("AnimLinkup->BoneToTrackTable.Num() != NumBones, BoneToTrackTable.Num(): %d, NumBones: %d, AnimName: %s, Owner: %s, Mesh: %s"), AnimLinkup->BoneToTrackTable.Num(), NumBones, *InAnimSeq->SequenceName.ToString(), *SkelComponent->GetOwner()->GetName(), *SkelComponent->SkeletalMesh->GetName());
	}
	check(AnimLinkup->BoneToTrackTable.Num() == NumBones);

	// Are we doing root motion for this node?
	const UBOOL bDoRootTranslation	= (RootBoneOption[0] != RBA_Default) || (RootBoneOption[1] != RBA_Default) || (RootBoneOption[2] != RBA_Default);
	const UBOOL bDoRootRotation		= (RootRotationOption[0] != RRO_Default) || (RootRotationOption[1] != RRO_Default) || (RootRotationOption[2] != RRO_Default);
	const UBOOL	bDoingRootMotion	= bDoRootTranslation || bDoRootRotation;

	// Is the skeletal mesh component requesting that raw animation data be used?
	const UBOOL bUseRawData = SkelComponent->bUseRawData;

	// Handle Last Frame to First Frame interpolation when looping.
	// It is however disabled for the Root Bone.
	// And the 'bNoLoopingInterpolation' flag on the animation can also disable that behavior.
	const UBOOL bLoopingInterpolation = bLooping && !InAnimSeq->bNoLoopingInterpolation;

	// Never process bone indices higher than our static array size to avoid crashes for malformed assets
	const INT DesiredBoneCount = Min( MAX_BONES, DesiredBones.Num() );

#if (USE_ANIMATION_CODEC_BATCH_SOLVER)
	if (!bUseRawData && DesiredBones.Num()>0 && InAnimSeq->CompressedTrackOffsets.Num() > 0)
	{
		// Note: These are static to avoid allocation. This makes it non-threadsafe.
		static BoneTrackArray RotationPairs;
		static BoneTrackArray TranslationPairs;
		check( IsInGameThread() );

		// build a list of desired bones
		RotationPairs.Empty();
		TranslationPairs.Empty();

		BOOL NonRootEncountered = FALSE;
		for( INT DesiredIndex=0; DesiredIndex < DesiredBoneCount; DesiredIndex++ )
		{
			// Find which track in the sequence we look in for this bones data
			const INT BoneIndex = DesiredBones(DesiredIndex);
			checkSlow( BoneIndex < MAX_BONES );

			const INT	TrackIndex = AnimLinkup->BoneToTrackTable(BoneIndex);

			Atoms(BoneIndex).Scale = 1.0f;

			if( TrackIndex == INDEX_NONE )
			{
				// use the default rotation and translation for this bone
				if( InAnimSeq->bIsAdditive )
				{
					Atoms(BoneIndex) = FBoneAtom::Identity;
				}
				else
				{
					Atoms(BoneIndex) = FBoneAtom(RefSkel(BoneIndex).BonePos.Orientation, RefSkel(BoneIndex).BonePos.Position, 1.f);				
				}
			}
			else if (BoneIndex > 0)
			{
				NonRootEncountered = TRUE;
				RotationPairs.AddItem(BoneTrackPair(BoneIndex, TrackIndex));

				// If doing 'rotation only' case, use ref pose for all non-root bones that are not in the BoneUseAnimTranslation array.
				if(	!InAnimSeq->bIsAdditive && ((AnimSet->bAnimRotationOnly && !AnimLinkup->BoneUseAnimTranslation(BoneIndex)) || AnimLinkup->ForceUseMeshTranslation(BoneIndex)) )
				{
					// use the default translation for this bone
					Atoms(BoneIndex).Translation = RefSkel(BoneIndex).BonePos.Position;
				}
				else
				{
					TranslationPairs.AddItem(BoneTrackPair(BoneIndex, TrackIndex));
				}
			}
			else
			{
				// Otherwise read it from the sequence.
				FBoneAtom& RootAtom = Atoms(0);

				AnimationFormat_GetBoneAtom(	
					RootAtom,
					*InAnimSeq,
					TrackIndex,
					CurrentTime,
					bLoopingInterpolation && !bDoingRootMotion);

				// If doing root motion for this animation, extract it!
				if( bDoingRootMotion )
				{
					ExtractRootMotion(InAnimSeq, TrackIndex, RootAtom, RootMotionDelta, bHasRootMotion);
				}

				// If desired, zero out Root Bone rotation.
				if( bZeroRootRotation )
				{
					RootAtom.Rotation = FQuat::Identity;
				}

				// If desired, zero out Root Bone translation.
				if( bZeroRootTranslation )
				{
					RootAtom.Translation = FVector(0.f);
				}
			}
		}

		// get the remaining bone atoms
		if (NonRootEncountered)
		{
			AnimationFormat_GetAnimationPose(	
				Atoms, 
				RotationPairs,
				TranslationPairs,
				*InAnimSeq,
				CurrentTime,
				bLoopingInterpolation);
		}
	}
	else
	{
#endif
	// For each desired bone...
	for( INT i=0; i < DesiredBoneCount; i++ )
	{
		const INT BoneIndex = DesiredBones(i);
		checkSlow( BoneIndex < MAX_BONES );

		// Find which track in the sequence we look in for this bones data
		const INT	TrackIndex = AnimLinkup->BoneToTrackTable(BoneIndex);

		// If there is no track for this bone, we just use the reference pose.
		if( TrackIndex == INDEX_NONE )
		{
			if( InAnimSeq->bIsAdditive )
			{
				Atoms(BoneIndex) = FBoneAtom::Identity;
			}
			else
			{
				Atoms(BoneIndex) = FBoneAtom(RefSkel(BoneIndex).BonePos.Orientation, RefSkel(BoneIndex).BonePos.Position, 1.f);				
			}
		}
		else 
		{
			// Non Root Bone
			if( BoneIndex > 0 )
			{
				// Otherwise read it from the sequence.
				InAnimSeq->GetBoneAtom(Atoms(BoneIndex), TrackIndex, CurrentTime, bLoopingInterpolation, bUseRawData);

				// If doing 'rotation only' case, use ref pose for all non-root bones that are not in the BoneUseAnimTranslation array.
				if(	!InAnimSeq->bIsAdditive && ((AnimSet->bAnimRotationOnly && !AnimLinkup->BoneUseAnimTranslation(BoneIndex)) || AnimLinkup->ForceUseMeshTranslation(BoneIndex)) )
				{
					Atoms(BoneIndex).Translation = RefSkel(BoneIndex).BonePos.Position;
				}

				// Apply quaternion fix for ActorX-exported quaternions.
				Atoms(BoneIndex).Rotation.W *= -1.0f;
			}
			// Root Bone
			else
			{
				// Otherwise read it from the sequence.
				InAnimSeq->GetBoneAtom(Atoms(BoneIndex), TrackIndex, CurrentTime, bLoopingInterpolation && !bDoingRootMotion, bUseRawData);

				// If doing root motion for this animation, extract it!
				if( bDoingRootMotion )
				{
					ExtractRootMotion(InAnimSeq, TrackIndex, Atoms(0), RootMotionDelta, bHasRootMotion);
				}

				// If desired, zero out Root Bone rotation.
				if( bZeroRootRotation )
				{
					Atoms(0).Rotation = FQuat::Identity;
				}

				// If desired, zero out Root Bone translation.
				if( bZeroRootTranslation )
				{
					Atoms(0).Translation = FVector(0.f);
				}
			}
		}
	}
#if (USE_ANIMATION_CODEC_BATCH_SOLVER)
	}
#endif

	// Check that all bone atoms coming from animation are normalized
#if !FINAL_RELEASE && !CONSOLE
	for( INT i=0; i<DesiredBoneCount; i++ )
	{
		const INT	BoneIndex = DesiredBones(i);
		check( Atoms(BoneIndex).Rotation.IsNormalized() );
	}
#endif

// In Editor, show additive animation added to its ref pose
// Made into a separate branch, so we don't add a branch test for each bone when we only care about this in the editor.
#if 1 && !CONSOLE
	if( bEditorOnlyAddRefPoseToAdditiveAnimation && InAnimSeq->bIsAdditive )
	{
		for( INT i=0; i<DesiredBoneCount; i++ )
		{
			const INT	BoneIndex = DesiredBones(i);
			// Find which track in the sequence we look in for this bones data
			const INT	TrackIndex = AnimLinkup->BoneToTrackTable(BoneIndex);

			if( TrackIndex == INDEX_NONE )
			{
				Atoms(BoneIndex) = FBoneAtom(RefSkel(BoneIndex).BonePos.Orientation, RefSkel(BoneIndex).BonePos.Position, 1.f);				
			}
			else
			{
				FBoneAtom&	RefBoneAtom = InAnimSeq->AdditiveRefPose(TrackIndex);

				// If doing 'rotation only' case, use ref pose for all non-root bones that are not in the BoneUseAnimTranslation array.
				if(	BoneIndex > 0 && ((AnimSet->bAnimRotationOnly && !AnimLinkup->BoneUseAnimTranslation(BoneIndex)) || AnimLinkup->ForceUseMeshTranslation(BoneIndex)) )
				{
					Atoms(BoneIndex).Translation += RefSkel(BoneIndex).BonePos.Position;
				}
				else
				{
					Atoms(BoneIndex).Translation += RefBoneAtom.Translation;
				}

				// Apply quaternion fix for ActorX-exported quaternions.
				FQuat RefRot = RefBoneAtom.Rotation;
				if( BoneIndex > 0 )
				{
					RefRot.W *= -1.0f;
				}

				// Add ref pose relative animation to base animation, only if rotation is significant.
				if( Square(Atoms(BoneIndex).Rotation.W) < 1.f - DELTA * DELTA )
				{
					Atoms(BoneIndex).Rotation = Atoms(BoneIndex).Rotation * RefRot;
				}
				else
				{
					Atoms(BoneIndex).Rotation = RefRot;
				}
			}
		}
	}
#endif

}


/**
 *  Extract Root Motion for the current Animation Pose.
 */
void UAnimNodeSequence::ExtractRootMotion(UAnimSequence* InAnimSeq, const INT &TrackIndex, FBoneAtom& CurrentFrameAtom, FBoneAtom& DeltaMotionAtom, INT& bHasRootMotion)
{
	// SkeletalMesh has a transformation that is applied between the component and the actor, 
	// instead of being between mesh and component. 
	// So we have to take that into account when doing operations happening in component space (such as per Axis masking/locking).
	const FMatrix MeshToCompTM = FRotationMatrix(SkelComponent->SkeletalMesh->RotOrigin);
	// Inverse transform, from component space to mesh space.
	const FMatrix CompToMeshTM = MeshToCompTM.Inverse();

	// Get the exact translation of the root bone on the first frame of the animation
	FBoneAtom FirstFrameAtom;
	InAnimSeq->GetBoneAtom(FirstFrameAtom, TrackIndex, 0.f, FALSE, SkelComponent->bUseRawData);

	// Do we need to extract root motion?
	const UBOOL bExtractRootTranslation	= (RootBoneOption[0] == RBA_Translate) || (RootBoneOption[1] == RBA_Translate) || (RootBoneOption[2] == RBA_Translate);
	const UBOOL bExtractRootRotation	= (RootRotationOption[0] == RRO_Extract) || (RootRotationOption[1] == RRO_Extract) || (RootRotationOption[2] == RRO_Extract);
	const UBOOL	bExtractRootMotion		= bExtractRootTranslation || bExtractRootRotation;

	// Calculate bone motion
	if( bExtractRootMotion )
	{
		// We are extracting root motion, so set the flag to TRUE
		bHasRootMotion	= 1;
		FLOAT StartTime	= PreviousTime;
		FLOAT EndTime	= CurrentTime;

		/** 
		 * If FromTime == ToTime, then we can't give a root delta for this frame.
		 * To avoid delaying it to next frame, because physics may need it right away,
		 * see if we can make up one by simulating forward.
		 */
		if( StartTime == EndTime )
		{
			// Only try to make up movement if animation is allowed to play
			if( bPlaying && InAnimSeq )
			{
				// See how much time would have passed on this frame
				const FLOAT DeltaTime = Rate * InAnimSeq->RateScale * SkelComponent->GlobalAnimRateScale * GWorld->GetDeltaSeconds();

				// If we can push back FromTime, then do so.
				if( StartTime > DeltaTime )
				{
					StartTime -= DeltaTime;
				}
				else
				{
					// otherwise, advance in time, to predict the movement
					EndTime += DeltaTime;

					// See if we passed the end of the animation.
					if( EndTime > InAnimSeq->SequenceLength )
					{
						// If we are looping, wrap over. If not, snap time to end of sequence.
						EndTime = bLooping ? appFmod(EndTime, InAnimSeq->SequenceLength) : InAnimSeq->SequenceLength;
					}
				}
			}
			else
			{
				// If animation is done playing we're not extracting root motion anymore.
				bHasRootMotion = 0;
			}
		}

		// If time has passed, compute root delta movement
		if( StartTime != EndTime )
		{
			// Get Root Bone Position of start of movement
			FBoneAtom StartAtom;
			if( StartTime != CurrentTime )
			{
				InAnimSeq->GetBoneAtom(StartAtom, TrackIndex, StartTime, FALSE, SkelComponent->bUseRawData);
			}
			else
			{
				StartAtom = CurrentFrameAtom;
			}

			// Get Root Bone Position of end of movement
			FBoneAtom EndAtom;
			if( EndTime != CurrentTime )
			{
				InAnimSeq->GetBoneAtom(EndAtom, TrackIndex, EndTime, FALSE, SkelComponent->bUseRawData);
			}
			else
			{
				EndAtom = CurrentFrameAtom;
			}

			// Get position on last frame if we extract translation and/or rotation
			FBoneAtom LastFrameAtom;
			if( StartTime > EndTime && (bExtractRootTranslation || bExtractRootRotation) )
			{
				// Get the exact root position of the root bone on the last frame of the animation
				InAnimSeq->GetBoneAtom(LastFrameAtom, TrackIndex, InAnimSeq->SequenceLength, FALSE, SkelComponent->bUseRawData);
			}

			// We don't support scale
			DeltaMotionAtom.Scale = 0.f;

			// If extracting root translation, filter out any axis
			if( bExtractRootTranslation )
			{
				// Handle case if animation looped
				if( StartTime > EndTime )
				{
					// Handle proper translation wrapping. We don't want the mesh to translate back to origin. So split that in 2 moves.
					DeltaMotionAtom.Translation = (LastFrameAtom.Translation - StartAtom.Translation) + (EndAtom.Translation - FirstFrameAtom.Translation);
				}
				else
				{
					// Delta motion of the root bone in mesh space
					DeltaMotionAtom.Translation = EndAtom.Translation - StartAtom.Translation;
				}

				// Only do that if an axis needs to be filtered out.
				if( RootBoneOption[0] != RBA_Translate || RootBoneOption[1] != RBA_Translate || RootBoneOption[2] != RBA_Translate )
				{
					// Convert Delta translation from mesh space to component space
					// We do this for axis filtering
					FVector CompDeltaTranslation = MeshToCompTM.TransformNormal(DeltaMotionAtom.Translation);

					// Filter out any of the X, Y, Z channels in mesh space
					if( RootBoneOption[0] != RBA_Translate )
					{
						CompDeltaTranslation.X = 0.f;
					}
					if( RootBoneOption[1] != RBA_Translate )
					{
						CompDeltaTranslation.Y = 0.f;
					}
					if( RootBoneOption[2] != RBA_Translate )
					{
						CompDeltaTranslation.Z = 0.f;
					}

					// Convert back to mesh space.
					DeltaMotionAtom.Translation = MeshToCompTM.InverseTransformNormal(CompDeltaTranslation);
				}
#if 0
				debugf(TEXT("%3.2f [%s] [%s] Extracted Root Motion Trans: %3.3f, Vect: %s, StartTime: %3.3f, EndTime: %3.3f"), GWorld->GetTimeSeconds(), *SkelComponent->GetOwner()->GetName(), *AnimSeqName.ToString(), DeltaMotionAtom.Translation.Size(), *DeltaMotionAtom.Translation.ToString(), StartTime, EndTime);
#endif
			}
			else
			{
				// Otherwise, don't extract any translation
				DeltaMotionAtom.Translation = FVector(0.f);
			}

			// If extracting root translation, filter out any axis
			if( bExtractRootRotation )
			{
				// Delta rotation
				// Handle case if animation looped
				if( StartTime > EndTime )
				{
					// Handle proper Rotation wrapping. We don't want the mesh to rotate back to origin. So split that in 2 turns.
					DeltaMotionAtom.Rotation = (LastFrameAtom.Rotation * (-StartAtom.Rotation)) * (EndAtom.Rotation * (-FirstFrameAtom.Rotation));
				}
				else
				{
					// Delta motion of the root bone in mesh space
					DeltaMotionAtom.Rotation = EndAtom.Rotation * (-StartAtom.Rotation);
				}
				DeltaMotionAtom.Rotation.Normalize();

#if 0 // DEBUG ROOT ROTATION
				debugf(TEXT("%3.2f Root Rotation StartAtom: %s, EndAtom: %s, DeltaMotionAtom: %s"), GWorld->GetTimeSeconds(), 
					*(FQuatRotationTranslationMatrix(StartAtom.Rotation, FVector(0.f)).Rotator()).ToString(), 
					*(FQuatRotationTranslationMatrix(EndAtom.Rotation, FVector(0.f)).Rotator()).ToString(), 
					*(FQuatRotationTranslationMatrix(DeltaMotionAtom.Rotation, FVector(0.f)).Rotator()).ToString());
#endif

				// Only do that if an axis needs to be filtered out.
				if( RootRotationOption[0] != RRO_Extract || RootRotationOption[1] != RRO_Extract || RootRotationOption[2] != RRO_Extract )
				{
					FQuat	MeshToCompQuat(MeshToCompTM);

					// Turn delta rotation from mesh space to component space
					FQuat	CompDeltaQuat = MeshToCompQuat * DeltaMotionAtom.Rotation * (-MeshToCompQuat);
					CompDeltaQuat.Normalize();

#if 0 // DEBUG ROOT ROTATION
					debugf(TEXT("%3.2f Mesh To Comp Delta: %s"), GWorld->GetTimeSeconds(), *(FQuatRotationTranslationMatrix(CompDeltaQuat, FVector(0.f)).Rotator()).ToString());
#endif

					// Turn component space delta rotation to FRotator
					// @note going through rotators introduces some errors. See if this can be done using quaterions instead.
					FRotator CompDeltaRot = FQuatRotationTranslationMatrix(CompDeltaQuat, FVector(0.f)).Rotator();

					// Filter out any of the Roll (X), Pitch (Y), Yaw (Z) channels in mesh space
					if( RootRotationOption[0] != RRO_Extract )
					{
						CompDeltaRot.Roll	= 0;
					}
					if( RootRotationOption[1] != RRO_Extract )
					{
						CompDeltaRot.Pitch	= 0;
					}
					if( RootRotationOption[2] != RRO_Extract )
					{
						CompDeltaRot.Yaw	= 0;
					}

					// Turn back filtered component space delta rotator to quaterion
					FQuat CompNewDeltaQuat	= CompDeltaRot.Quaternion();

					// Turn component space delta to mesh space.
					DeltaMotionAtom.Rotation = (-MeshToCompQuat) * CompNewDeltaQuat * MeshToCompQuat;
					DeltaMotionAtom.Rotation.Normalize();

#if 0 // DEBUG ROOT ROTATION
					debugf(TEXT("%3.2f Post Comp Filter. CompDelta: %s, MeshDelta: %s"), GWorld->GetTimeSeconds(), 
						*(FQuatRotationTranslationMatrix(CompNewDeltaQuat, FVector(0.f)).Rotator()).ToString(), 
						*(FQuatRotationTranslationMatrix(DeltaMotionAtom.Rotation, FVector(0.f)).Rotator()).ToString());
#endif

				}

#if 0 // DEBUG ROOT ROTATION
				FQuat	MeshToCompQuat(MeshToCompTM);

				// Transform mesh space delta rotation to component space.
				FQuat	CompDeltaQuat	= MeshToCompQuat * DeltaMotionAtom.Rotation * (-MeshToCompQuat);
				CompDeltaQuat.Normalize();

				debugf(TEXT("%3.2f Extracted Root Rotation: %s"), GWorld->GetTimeSeconds(), *(FQuatRotationTranslationMatrix(CompDeltaQuat, FVector(0.f)).Rotator()).ToString());
#endif

				// Transform delta translation by this delta rotation.
				// This is to compensate the fact that the rotation will rotate the actor, and affect the translation.
				// This assumes that root rotation won't be weighted down the tree, and that Actor will actually use it...
				// Also we don't filter rotation per axis here.. what is done for delta root rotation, should be done here as well.
				if( !DeltaMotionAtom.Translation.IsZero() )
				{
					// Delta rotation since first frame
					// Remove delta we just extracted, because translation is going to be applied with current rotation, not new one.
					FQuat	MeshDeltaRotQuat = (CurrentFrameAtom.Rotation * (-FirstFrameAtom.Rotation)) * (-DeltaMotionAtom.Rotation);
					MeshDeltaRotQuat.Normalize();

					// Only do that if an axis needs to be filtered out.
					if( RootRotationOption[0] != RRO_Extract || RootRotationOption[1] != RRO_Extract || RootRotationOption[2] != RRO_Extract )
					{
						FQuat	MeshToCompQuat(MeshToCompTM);

						// Turn delta rotation from mesh space to component space
						FQuat	CompDeltaQuat = MeshToCompQuat * MeshDeltaRotQuat * (-MeshToCompQuat);
						CompDeltaQuat.Normalize();

						// Turn component space delta rotation to FRotator
						// @note going through rotators introduces some errors. See if this can be done using quaterions instead.
						FRotator CompDeltaRot = FQuatRotationTranslationMatrix(CompDeltaQuat, FVector(0.f)).Rotator();

						// Filter out any of the Roll (X), Pitch (Y), Yaw (Z) channels in mesh space
						if( RootRotationOption[0] != RRO_Extract )
						{
							CompDeltaRot.Roll	= 0;
						}
						if( RootRotationOption[1] != RRO_Extract )
						{
							CompDeltaRot.Pitch	= 0;
						}
						if( RootRotationOption[2] != RRO_Extract )
						{
							CompDeltaRot.Yaw	= 0;
						}

						// Turn back filtered component space delta rotator to quaterion
						FQuat CompNewDeltaQuat = CompDeltaRot.Quaternion();

						// Turn component space delta to mesh space.
						MeshDeltaRotQuat = (-MeshToCompQuat) * CompNewDeltaQuat * MeshToCompQuat;
						MeshDeltaRotQuat.Normalize();
					}

					FMatrix	MeshDeltaRotTM		= FQuatRotationTranslationMatrix(MeshDeltaRotQuat, FVector(0.f));
					DeltaMotionAtom.Translation = MeshDeltaRotTM.InverseTransformNormal( DeltaMotionAtom.Translation );
				}
			}
			else
			{			
				// If we're not extracting rotation, then set to identity
				DeltaMotionAtom.Rotation = FQuat::Identity;
			}
		}
		else // if( StartTime != EndTime )
		{
			// Root Motion cannot be extracted.
			DeltaMotionAtom = FBoneAtom::Identity;
		}
	}

	// Apply bone locking, with axis filtering (in component space)
	// Bone is locked to its position on the first frame of animation.
	{
		// Lock full rotation to first frame.
		if( RootRotationOption[0] != RRO_Default && RootRotationOption[1] != RRO_Default && RootRotationOption[2] != RRO_Default)
		{
			CurrentFrameAtom.Rotation = FirstFrameAtom.Rotation;
		}
		// Do we need to lock at least one axis of the bone's rotation to the first frame's value?
		else if( RootRotationOption[0] != RRO_Default || RootRotationOption[1] != RRO_Default || RootRotationOption[2] != RRO_Default )
		{
			FQuat	MeshToCompQuat(MeshToCompTM);

			// Find delta between current frame and first frame
			FQuat	CompFirstQuat	= MeshToCompQuat * FirstFrameAtom.Rotation;
			FQuat	CompCurrentQuat	= MeshToCompQuat * CurrentFrameAtom.Rotation;
			FQuat	CompDeltaQuat	= CompCurrentQuat * (-CompFirstQuat);
			CompDeltaQuat.Normalize();

			FRotator CompDeltaRot = FQuatRotationTranslationMatrix(CompDeltaQuat, FVector(0.f)).Rotator();

			// Filter out any of the Roll (X), Pitch (Y), Yaw (Z) channels in mesh space
			if( RootRotationOption[0] != RRO_Default )
			{
				CompDeltaRot.Roll	= 0;
			}
			if( RootRotationOption[1] != RRO_Default )
			{
				CompDeltaRot.Pitch	= 0;
			}
			if( RootRotationOption[2] != RRO_Default )
			{
				CompDeltaRot.Yaw	= 0;
			}

			// Use new delta and first frame to find out new current rotation.
			FQuat	CompNewDeltaQuat	= CompDeltaRot.Quaternion();
			FQuat	CompNewCurrentQuat	= CompNewDeltaQuat * CompFirstQuat;
			CompNewCurrentQuat.Normalize();

			CurrentFrameAtom.Rotation	= (-MeshToCompQuat) * CompNewCurrentQuat;
			CurrentFrameAtom.Rotation.Normalize();
		}

		// Lock full bone translation to first frame
		if( RootBoneOption[0] != RBA_Default && RootBoneOption[1] != RBA_Default && RootBoneOption[2] != RBA_Default )
		{
			CurrentFrameAtom.Translation = FirstFrameAtom.Translation;

#if 0
			debugf(TEXT("%3.2f Lock Root Bone to first frame translation: %s"), GWorld->GetTimeSeconds(), *FirstFrameAtom.Translation.ToString());
#endif
		}
		// Do we need to lock at least one axis of the bone's translation to the first frame's value?
		else if( RootBoneOption[0] != RBA_Default || RootBoneOption[1] != RBA_Default || RootBoneOption[2] != RBA_Default )
		{
			FVector CompCurrentFrameTranslation			= MeshToCompTM.TransformNormal(CurrentFrameAtom.Translation);
			const	FVector	CompFirstFrameTranslation	= MeshToCompTM.TransformNormal(FirstFrameAtom.Translation);

			// Lock back to first frame position any of the X, Y, Z axis
			if( RootBoneOption[0] != RBA_Default  )
			{
				CompCurrentFrameTranslation.X = CompFirstFrameTranslation.X;
			}
			if( RootBoneOption[1] != RBA_Default  )
			{
				CompCurrentFrameTranslation.Y = CompFirstFrameTranslation.Y;
			}
			if( RootBoneOption[2] != RBA_Default  )
			{
				CompCurrentFrameTranslation.Z = CompFirstFrameTranslation.Z;
			}

			// convert back to mesh space
			CurrentFrameAtom.Translation = MeshToCompTM.InverseTransformNormal(CompCurrentFrameTranslation);
		}
	}				
}

void UAnimNodeSequence::IssueNotifies(FLOAT DeltaTime)
{
	// If no sequence - do nothing!
	if(!AnimSeq)
	{
		return;
	}

	// Do nothing if there are no notifies to play.
	const INT NumNotifies = AnimSeq->Notifies.Num();
	if(NumNotifies == 0)
	{
		return;
	}


	// Total interval we're about to proceed CurrentTime forward  (right after this IssueNotifies routine)
	FLOAT TimeToGo = DeltaTime; 

	// First, find the next notify we are going to hit.
	INT NextNotifyIndex = INDEX_NONE;
	FLOAT TimeToNextNotify = BIG_NUMBER;
	FLOAT WorkTime = BIG_NUMBER;

	// Note - if there are 2 notifies with the same time, it starts with the lower index one.
	for(INT i=0; i<NumNotifies; i++)
	{
		FLOAT NotifyEventTime = AnimSeq->Notifies(i).Time;
		FLOAT TryTimeToNotify = NotifyEventTime - CurrentTime;
		if(TryTimeToNotify < 0.0f)
		{
			if(!bLooping)
			{
				// Not interested in notifies before current time if not looping.
				continue; 
			}
			else
			{
				// Correct TimeToNextNotify as it lies before WorkTime.
				TryTimeToNotify += AnimSeq->SequenceLength; 
			}
		}

		// Check to find soonest one.
		if(TryTimeToNotify < TimeToNextNotify)
		{
			TimeToNextNotify = TryTimeToNotify;
			NextNotifyIndex = i;
			WorkTime = NotifyEventTime;
		}
	}

	// If there is no potential next notify, do nothing.
	// This can only happen if there are no notifies (and we would have returned at start) or the anim is not looping.
	if(NextNotifyIndex == INDEX_NONE)
	{
		check(!bLooping);
		return;
	}

	// Wind current time to first notify.
	TimeToGo -= TimeToNextNotify;

	// Set flag to show we are firing notifies.
	bIsIssuingNotifies = TRUE;
	// Backup SeqNode, in case it gets changed during a notify, so we can process them all.
	UAnimSequence* AnimSeqNotify = AnimSeq;

	// Then keep walking forwards until we run out of time.
	while( TimeToGo > 0.0f )
	{
		//debugf( TEXT("NOTIFY: %d %s %f"), NextNotifyIndex, *(AnimSeqNotify->Notifies(NextNotifyIndex).Comment.ToString()), TimeToGo );

		// Execute this notify. NextNotifyIndex will be the soonest notify inside the current TimeToGo interval.
		UAnimNotify* AnimNotify = AnimSeqNotify->Notifies(NextNotifyIndex).Notify;
		if( AnimNotify )
		{
			// Call Notify function
			AnimNotify->Notify( this->SkelComponent );
		}
		
		// Then find the next one.
		NextNotifyIndex = (NextNotifyIndex + 1) % NumNotifies; // Assumes notifies are ordered.
		TimeToNextNotify = AnimSeqNotify->Notifies(NextNotifyIndex).Time - WorkTime;

		// Wrap if necessary.
		if( NextNotifyIndex == 0 )
		{
			if( !bLooping )
			{
				// If not looping, nothing more to do if notify is before current working time.
				bIsIssuingNotifies = FALSE;
				return;
			}
			else
			{
				// Correct TimeToNextNotify as it lies before WorkTime.
				TimeToNextNotify += AnimSeqNotify->SequenceLength; 
			}
		}

		// Wind on to next notify.
		TimeToGo -= TimeToNextNotify;
		WorkTime = AnimSeqNotify->Notifies(NextNotifyIndex).Time;
	}

	bIsIssuingNotifies = FALSE;
}

/** 
 * Set a new animation by name.
 * This will find the UAnimationSequence by name, from the list of AnimSets specified in the SkeletalMeshComponent and cache it
 * Will also store a pointer to the anim track <-> 
 */
void UAnimNodeSequence::SetAnim(FName InSequenceName)
{
	if (0)
	{
		debugf(TEXT("** SetAnim %s, on %s"), *InSequenceName.ToString(), *GetFName().ToString());
	}

	// Abort if we are in the process of firing notifies, as this can cause a crash.
	//
	//	Unless the animation is the same. This can happen if a new skeletal mesh is set, then it forces all
	// animations to be recached. If the animation is the same, then it's safe to update it.
	// Note that it can be set to NAME_None if the AnimSet has been removed as well.
	if( bIsIssuingNotifies && AnimSeqName != InSequenceName )
	{
		debugf( TEXT("UAnimNodeSequence::SetAnim : Not safe to call SetAnim from inside a Notify. AnimName: %s, Owner: %s"), *InSequenceName.ToString(), *SkelComponent->GetOwner()->GetName() );
		return;
	}

	AnimSeqName		= InSequenceName;
	AnimSeq			= NULL;
	AnimLinkupIndex = INDEX_NONE;

	// Animation updated, clear cached data
	ConditionalClearCachedData();

	if( InSequenceName == NAME_None || !SkelComponent || !SkelComponent->SkeletalMesh )
	{
		return;
	}

	AnimSeq = SkelComponent->FindAnimSequence(AnimSeqName);
	if( AnimSeq != NULL )
	{
		UAnimSet* AnimSet = AnimSeq->GetAnimSet();
		AnimLinkupIndex = AnimSet->GetMeshLinkupIndex( SkelComponent->SkeletalMesh );
		
		check(AnimLinkupIndex != INDEX_NONE);
		check(AnimLinkupIndex < AnimSet->LinkupCache.Num());

		FAnimSetMeshLinkup* AnimLinkup = &AnimSet->LinkupCache(AnimLinkupIndex);

		check( AnimLinkup->SkelMeshLinkupGUID == SkelComponent->SkeletalMesh->SkelMeshGUID );
		check( AnimLinkup->BoneToTrackTable.Num() == SkelComponent->SkeletalMesh->RefSkeleton.Num() );
		check( AnimLinkup->BoneUseAnimTranslation.Num() == SkelComponent->SkeletalMesh->RefSkeleton.Num() );
	}
	else if( !bDisableWarningWhenAnimNotFound && !SkelComponent->bDisableWarningWhenAnimNotFound )
	{
		debugf( NAME_DevAnim, TEXT("%s - Failed to find animsequence '%s' on SkeletalMeshComponent: %s whose owner is: %s using mesh: %s" ),
			   *GetName(),
			   *InSequenceName.ToString(),
			   *SkelComponent->GetName(),
			   *SkelComponent->GetOwner()->GetName(),
			   *SkelComponent->SkeletalMesh->GetPathName()
			   );
	}
}

/** Returns the camera associated with the skelmesh's owner, if any. */
AAnimatedCamera* UAnimNodeSequence::GetPlayerCamera() const
{
	if (SkelComponent != NULL)
	{
		AActor* const Owner = SkelComponent->GetOwner();

		if (Owner != NULL)
		{
			APawn* const Pawn = Owner->GetAPawn();
			if ( (Pawn != NULL) && (Pawn->Controller != NULL) )
			{
				APlayerController* const PC = Pawn->Controller->GetAPlayerController();

				if (PC != NULL)
				{
					return Cast<AAnimatedCamera>(PC->PlayerCamera);
				}
			}
		}
	}

	return NULL;
}


/** Start the current animation playing. This just sets the bPlaying flag to true, so that TickAnim will move CurrentTime forwards. */
void UAnimNodeSequence::PlayAnim(UBOOL bInLoop, FLOAT InRate, FLOAT StartTime)
{
	CurrentTime		= StartTime;
	PreviousTime	= StartTime;
	Rate			= InRate;
	bLooping		= bInLoop;
	bPlaying		= TRUE;

	if( bForceRefposeWhenNotPlaying && SkelComponent->bForceRefpose)
	{
		SkelComponent->SetForceRefPose(FALSE);
	}

	if( bCauseActorAnimPlay && SkelComponent->GetOwner() )
	{
		SkelComponent->GetOwner()->eventOnAnimPlay(this);
	}

	// start any nonlooping camera anim  looping camera anims will start in Tick().
	if (CameraAnim && !bLooping)
	{
		StartCameraAnim();
	}
}

/** Stop playing current animation. Will set bPlaying to false. */
void UAnimNodeSequence::StopAnim()
{
	bPlaying = false;

	if (ActiveCameraAnimInstance != NULL)
	{
		StopCameraAnim();
	}
}

/** Starts playing any camera anim we want to play in conjunction with this anim. */
void UAnimNodeSequence::StartCameraAnim()
{
	if (CameraAnim != NULL)
	{
		AAnimatedCamera* Cam = GetPlayerCamera();
		if (Cam)
		{
			// let's be sure we aren't somehow stomping our reference to a playing anim
			if (ActiveCameraAnimInstance != NULL)
			{
				StopCameraAnim();
			}

			// start our desired anim
			ActiveCameraAnimInstance = Cam->PlayCameraAnim(CameraAnim, CameraAnimPlayRate, CameraAnimScale, 0.f, 0.f, bLoopCameraAnim, bRandomizeCameraAnimLoopStartTime);
			if (ActiveCameraAnimInstance != NULL)
			{
				// we don't want this deallocated behind our backs.  necessary since we're pooling the instances.
				ActiveCameraAnimInstance->bAutoReleaseWhenFinished = FALSE;
				ActiveCameraAnimInstance->RegisterAnimNode(this);
			}
		}
	}
}

/** Stops playing any active camera anim playing in conjunction with this anim. */
void UAnimNodeSequence::StopCameraAnim()
{
	if (ActiveCameraAnimInstance != NULL)
	{
		ActiveCameraAnimInstance->Stop(TRUE);
		ActiveCameraAnimInstance->bAutoReleaseWhenFinished = TRUE;
		ActiveCameraAnimInstance = NULL;
	}
}


/** 
 *	Set the CurrentTime to the supplied position. 
 *	If bFireNotifies is true, this will fire any notifies between current and new time.
 */
void UAnimNodeSequence::SetPosition(FLOAT NewTime, UBOOL bFireNotifies)
{
	// Ensure NewTime lies within sequence length.
	const FLOAT AnimLength = AnimSeq ? AnimSeq->SequenceLength : 0.f;
	NewTime = ::Clamp<FLOAT>(NewTime, 0.f, AnimLength+KINDA_SMALL_NUMBER);

	// Find the amount we are moving.
	const FLOAT DeltaTime = NewTime - CurrentTime;

	// If moving forwards, and this node generates notifies, and is sufficiently 'in the mix', fire notifies now.
	if( bFireNotifies && 
		DeltaTime > 0.f && 
		!bNoNotifies && 
		(NodeTotalWeight >= NotifyWeightThreshold) )
	{
		IssueNotifies(DeltaTime);		
	}

	// Then update the time.
	CurrentTime = NewTime;

	// If we don't fire notifies, it means we jump to that new position instantly, 
	// so reset previous time
	if( !bFireNotifies )
	{
		PreviousTime = CurrentTime;
	}

	// If animation has only one frame then no need to clear it.
	if( !AnimSeq || AnimSeq->NumFrames > 1 )
	{
		// Animation updated, clear cached data
		ConditionalClearCachedData();
	}
}


/** 
 * Get normalized position, from 0.f to 1.f.
 */
FLOAT UAnimNodeSequence::GetNormalizedPosition()
{
	if( AnimSeq && AnimSeq->SequenceLength > 0.f )
	{
		return Clamp<FLOAT>(CurrentTime / AnimSeq->SequenceLength, 0.f, 1.f);
	}

	return 0.f;
}

/** 
 * Finds out normalized position of a synchronized node given a relative position of a group. 
 * Takes into account node's relative SynchPosOffset.
 */
FLOAT UAnimNodeSequence::FindNormalizedPositionFromGroupRelativePosition(FLOAT GroupRelativePosition)
{
	FLOAT NormalizedPosition = appFmod(GroupRelativePosition + SynchPosOffset, 1.f);
	if( NormalizedPosition < 0.f )
	{
		NormalizedPosition += 1.f;
	}

	return NormalizedPosition;
}


/** Returns the global play rate of this animation. Taking into account all Rate Scales */
FLOAT UAnimNodeSequence::GetGlobalPlayRate()
{
	// AnimNodeSequence play rate
	FLOAT GlobalRate = Rate * SkelComponent->GlobalAnimRateScale;

	// AnimSequence play rate
	if( AnimSeq )
	{
		GlobalRate *= AnimSeq->RateScale;
	}

	// AnimGroup play rate
	if( SynchGroupName != NAME_None && SkelComponent )
	{
		UAnimTree* RootNode = Cast<UAnimTree>(SkelComponent->Animations);
		if( RootNode )
		{
			const INT GroupIndex = RootNode->GetGroupIndex(SynchGroupName);
			if( GroupIndex != INDEX_NONE )
			{
				GlobalRate *= RootNode->AnimGroups(GroupIndex).RateScale;
			}
		}
	}

	return GlobalRate;
}


FLOAT UAnimNodeSequence::GetAnimPlaybackLength()
{
	if( AnimSeq )
	{
		const FLOAT GlobalPlayRate = GetGlobalPlayRate();
		if( GlobalPlayRate != 0.f )
		{
			return AnimSeq->SequenceLength / GlobalPlayRate;
		}
	}

	return 0.f;
}

/** 
 * Returns in seconds the time left until the animation is done playing. 
 * This is assuming the play rate is not going to change.
 */
FLOAT UAnimNodeSequence::GetTimeLeft()
{
	if( AnimSeq )
	{
		const FLOAT GlobalPlayRate = GetGlobalPlayRate();
		if( GlobalPlayRate > 0.f )
		{
			return Max(AnimSeq->SequenceLength - CurrentTime, 0.f) / GlobalPlayRate;
		}
	}

	return 0.f;
}

////////////////////////////////
// Actor latent function

void AActor::FinishAnim(class UAnimNodeSequence* SeqNode)
{
	GetStateFrame()->LatentAction = EPOLL_FinishAnim;
	LatentSeqNode  = SeqNode;
}

void AActor::execPollFinishAnim( FFrame& Stack, RESULT_DECL )
{
	// Block as long as LatentSeqNode is present and playing.
	if( !LatentSeqNode || !LatentSeqNode->bPlaying )
	{
		GetStateFrame()->LatentAction = 0;
		LatentSeqNode = NULL;
	}
}
IMPLEMENT_FUNCTION( AActor, EPOLL_FinishAnim, execPollFinishAnim );


/************************************************************************************
 * UAnimNodeSequenceBlendBase
 ***********************************************************************************/

void UAnimNodeSequenceBlendBase::CheckAnimsUpToDate()
{
	// Make sure animations are up to date
	const	INT		NumAnims		= Anims.Num();
			bool	bUpdatedSeqNode = FALSE;

	for(INT i=0; i<NumAnims; i++)
	{
		SetAnimInfo(Anims(i).AnimName, Anims(i).AnimInfo);

		if( !bUpdatedSeqNode && Anims(i).AnimInfo.AnimSeq != NULL )
		{
			// Ensure AnimNodeSequence playback compatibility by setting one animation
			// The node will use this animation for timing and notifies.
			SetAnim(Anims(i).AnimName);
			bUpdatedSeqNode = (AnimSeq != NULL);
		}
	}
}


/**
 * Init anim tree.
 * Make sure animation references are up to date.
 */
void UAnimNodeSequenceBlendBase::InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent)
{
	// Call Super version first, because that's where SkeletalMeshComponent reference is set (see UAnimNode::InitAnim()).
	Super::InitAnim(MeshComp, Parent);

#if 0
	debugf(TEXT("* InitAnim on %s"), *GetFName());
#endif

	// Make sure animations are up to date
	CheckAnimsUpToDate();
}


/** AnimSets have been updated, update all animations */
void UAnimNodeSequenceBlendBase::AnimSetsUpdated()
{
	Super::AnimSetsUpdated();

	// Make sure animations are up to date
	CheckAnimsUpToDate();
}


/**
 * A property has been changed from the editor
 * Make sure animation references are up to date.
 */
void UAnimNodeSequenceBlendBase::PostEditChange(UProperty* PropertyThatChanged)
{
#if 0
	debugf(TEXT("* PostEditChange on %s"), *GetFName());
#endif

	// Make sure animations are up to date
	CheckAnimsUpToDate();

	Super::PostEditChange(PropertyThatChanged);
}


/** 
 * Set a new animation by name.
 * This will find the UAnimationSequence by name, from the list of AnimSets specified in the SkeletalMeshComponent and cache it.
 */
void UAnimNodeSequenceBlendBase::SetAnimInfo(FName InSequenceName, FAnimInfo& InAnimInfo)
{
	UBOOL bFailed = FALSE;

#if 0
	debugf(TEXT("** SetAnimInfo %s on %s"), *InSequenceName, *GetFName());
#endif

	// if not valid, reset
	if( InSequenceName == NAME_None || !SkelComponent || !SkelComponent->SkeletalMesh )
	{
		bFailed = TRUE;
	}

	if( !bFailed )
	{
		InAnimInfo.AnimSeq = SkelComponent->FindAnimSequence(InSequenceName);
		if( InAnimInfo.AnimSeq != NULL )
		{
			InAnimInfo.AnimSeqName = InSequenceName;
			
			UAnimSet* AnimSet = InAnimInfo.AnimSeq->GetAnimSet();

			InAnimInfo.AnimLinkupIndex = AnimSet->GetMeshLinkupIndex(SkelComponent->SkeletalMesh);

			check(InAnimInfo.AnimLinkupIndex != INDEX_NONE);
			check(InAnimInfo.AnimLinkupIndex < AnimSet->LinkupCache.Num());

			FAnimSetMeshLinkup* AnimLinkup = &AnimSet->LinkupCache(InAnimInfo.AnimLinkupIndex);

			check(AnimLinkup->SkelMeshLinkupGUID == SkelComponent->SkeletalMesh->SkelMeshGUID);
			check(AnimLinkup->BoneToTrackTable.Num() == SkelComponent->SkeletalMesh->RefSkeleton.Num());
			check(AnimLinkup->BoneUseAnimTranslation.Num() == SkelComponent->SkeletalMesh->RefSkeleton.Num());
		}
		else
		{
			bFailed = TRUE;
		}
	}

	if( bFailed )
	{
		// See if we should bypass the error message
		const UBOOL bDisableWarning = bDisableWarningWhenAnimNotFound || (SkelComponent && SkelComponent->bDisableWarningWhenAnimNotFound);

		// If InSequenceName == NAME_None, it's not really error... it was intentional! :)
		if( InSequenceName != NAME_None && !bDisableWarning )
		{
			AActor* Owner = SkelComponent ? SkelComponent->GetOwner() : NULL;

			debugf(NAME_Warning, TEXT("%s - Failed, with animsequence '%s' on SkeletalMeshComponent: '%s' whose owner is: '%s' and is of type '%s'"), 
				*GetName(), 
				*InSequenceName.ToString(), 
				SkelComponent ? *SkelComponent->GetName() : TEXT("None"),
				Owner ? *Owner->GetName() : TEXT("None"),
				SkelComponent ? *SkelComponent->TemplateName.ToString() : TEXT("None")
				);
		}

		InAnimInfo.AnimSeqName		= NAME_None;
		InAnimInfo.AnimSeq			= NULL;
		InAnimInfo.AnimLinkupIndex	= INDEX_NONE;
	}
}


/**
 * Blends together the animations of this node based on the Weight in each element of the Anims array.
 *
 * @param	Atoms			Output array of relative bone transforms.
 * @param	DesiredBones	Indices of bones that we want to return. Note that bones not in this array will not be modified, so are not safe to access! 
 *							This array must be in strictly increasing order.
 */
void UAnimNodeSequenceBlendBase::GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion)
{
	START_GETBONEATOM_TIMER

	// See if results are cached.
	if( GetCachedResults(Atoms, RootMotionDelta, bHasRootMotion) )
	{
		return;
	}

	if( ShouldSkipWhenMeshNotRendered() )
	{
		FillWithRefPose(Atoms, DesiredBones, SkelComponent->SkeletalMesh->RefSkeleton);
		// No root motion here, move along, nothing to see...
		RootMotionDelta = FBoneAtom::Identity;
		bHasRootMotion	= 0;
		return;
	}

	const INT NumAnims = Anims.Num();

#if 1
	if( NumAnims == 0 )
	{
		debugf(TEXT("UAnimNodeSequenceBlendBase::GetBoneAtoms - %s - Anims array is empty!"), *GetName());
		RootMotionDelta = FBoneAtom::Identity;
		bHasRootMotion	= 0;
		FillWithRefPose(Atoms, DesiredBones, SkelComponent->SkeletalMesh->RefSkeleton);
		return;
	}
#endif

#if 1
	FLOAT TotalWeight = 0.f;

	for(INT i=0; i<NumAnims; i++)
	{
		TotalWeight += Anims(i).Weight;
	}

	// Check all children weights sum to 1.0
	if( Abs(TotalWeight - 1.f) > ZERO_ANIMWEIGHT_THRESH )
	{
		debugf(TEXT("WARNING: UAnimNodeSequenceBlendBase (%s) has Children weights which do not sum to 1.0."), *GetName());

		for(INT i=0; i<NumAnims; i++)
		{
			debugf(TEXT("Pose: %d Weight: %f"), i, Anims(i).Weight);
		}

		debugf(TEXT("Total Weight: %f"), TotalWeight);
		//@todo - adjust first node weight to 

		RootMotionDelta = FBoneAtom::Identity;
		bHasRootMotion	= 0;
		FillWithRefPose(Atoms, DesiredBones, SkelComponent->SkeletalMesh->RefSkeleton);
		return;
	}
#endif

	// Find index of the last child with a non-zero weight.
	INT LastChildIndex = INDEX_NONE;
	for(INT i=0; i<NumAnims; i++)
	{
		if( Anims(i).Weight > ZERO_ANIMWEIGHT_THRESH )
		{
			// If this is the only child with any weight, pass Atoms array into it directly.
			if( Anims(i).Weight >= (1.f - ZERO_ANIMWEIGHT_THRESH) )
			{
				GetAnimationPose(Anims(i).AnimInfo.AnimSeq, Anims(i).AnimInfo.AnimLinkupIndex, Atoms, DesiredBones, RootMotionDelta, bHasRootMotion);
				SaveCachedResults(Atoms, RootMotionDelta, bHasRootMotion);
				return;
			}
			LastChildIndex = i;
		}
	}
	check(LastChildIndex != INDEX_NONE);

	// We don't allocate this array until we need it.
	FBoneAtomArray ChildAtoms;
	UBOOL bNoChildrenYet = TRUE;

	// Root Motion
	FBoneAtom ExtractedRootMotion;

	// Iterate over each child getting its atoms, scaling them and adding them to output (Atoms array)
	for(INT i=0; i<=LastChildIndex; i++)
	{
		// If this child has non-zero weight, blend it into accumulator.
		if( Anims(i).Weight > ZERO_ANIMWEIGHT_THRESH )
		{
			// Do need to request atoms, so allocate array here.
			if( ChildAtoms.Num() == 0 )
			{
				const INT NumAtoms = SkelComponent->SkeletalMesh->RefSkeleton.Num();
				check(NumAtoms == Atoms.Num());
				ChildAtoms.Add(NumAtoms);
			}

			check(ChildAtoms.Num() == Atoms.Num());

			// Get Animation pose
			GetAnimationPose(Anims(i).AnimInfo.AnimSeq, Anims(i).AnimInfo.AnimLinkupIndex, ChildAtoms, DesiredBones, ExtractedRootMotion, bHasRootMotion);

			if( bHasRootMotion )
			{
				// Accumulate weighted Root Motion
				if( bNoChildrenYet )
				{
					RootMotionDelta = ExtractedRootMotion * Anims(i).Weight;
				}
				else
				{
					// To ensure the 'shortest route', we make sure the dot product between the accumulator and the incoming child atom is positive.
					if( (RootMotionDelta.Rotation | ExtractedRootMotion.Rotation) < 0.f )
					{
						ExtractedRootMotion.Rotation = ExtractedRootMotion.Rotation * -1.f;
					}

					RootMotionDelta += ExtractedRootMotion * Anims(i).Weight;
				}

				// If Last Child, normalize rotation quaternion
				if( i == LastChildIndex )
				{
					RootMotionDelta.Rotation.Normalize();
				}
		
#if 0 // Debug Root Motion
				if( !RMD.Translation.IsZero() )
				{
					const FVector RDTW = RMD.Translation * Anims(i).Weight;
					debugf(TEXT("%3.2f %s: Added weighted root motion translation: %3.2f, vect: %s"), GWorld->GetTimeSeconds(), *GetFName(), RDTW.Size(), *RDTW.ToString());
				}
#endif
			}

			for(INT j=0; j<DesiredBones.Num(); j++)
			{
				const INT BoneIndex = DesiredBones(j);

				// We just write the first childrens' atoms into the output array. Avoids zero-ing it out.
				if( bNoChildrenYet )
				{
					Atoms(BoneIndex) = ChildAtoms(BoneIndex) * Anims(i).Weight;
				}
				else
				{
					// To ensure the 'shortest route', we make sure the dot product between the accumulator and the incoming child atom is positive.
					if( (Atoms(BoneIndex).Rotation | ChildAtoms(BoneIndex).Rotation) < 0.f )
					{
						ChildAtoms(BoneIndex).Rotation = ChildAtoms(BoneIndex).Rotation * -1.f;
					}

					Atoms(BoneIndex) += ChildAtoms(BoneIndex) * Anims(i).Weight;
				}

				// If last child - normalize the rotation quaternion now.
				if( i == LastChildIndex )
				{
					Atoms(BoneIndex).Rotation.Normalize();
				}
			}

			bNoChildrenYet = FALSE;
		}
	}

	SaveCachedResults(Atoms, RootMotionDelta, bHasRootMotion);
}


/************************************************************************************
 * UAnimNodeSequenceBlendByAim
 ***********************************************************************************/

void UAnimNodeSequenceBlendByAim::CheckAnimsUpToDate()
{
	// Make sure animations are properly set
	Anims(0).AnimName = AnimName_LU;
	Anims(1).AnimName = AnimName_LC;
	Anims(2).AnimName = AnimName_LD;
	Anims(3).AnimName = AnimName_CU;
	Anims(4).AnimName = AnimName_CC;
	Anims(5).AnimName = AnimName_CD;
	Anims(6).AnimName = AnimName_RU;
	Anims(7).AnimName = AnimName_RC;
	Anims(8).AnimName = AnimName_RD;

	Super::CheckAnimsUpToDate();
}

/** Override this function in a subclass, and return normalized Aim from Pawn. */
FVector2D UAnimNodeSequenceBlendByAim::GetAim() 
{ 
	return Aim;
}

void UAnimNodeSequenceBlendByAim::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	// if not relevant to the final blend, no need to update weights
	if( !bRelevant )
	{
		Super::TickAnim(DeltaSeconds, TotalWeight);
		return;
	}

	// Get Normalized Aim.
	FVector2D SafeAim = GetAim();

	const UBOOL bAimChanged = (SafeAim != PreviousAim);
	if( bAimChanged )
	{
		// Reset cached bone atoms to force an update of the node.
		ConditionalClearCachedData();
		PreviousAim = SafeAim;
	}

	// Update weights to blend animations together
	if( bAimChanged || bJustBecameRelevant )
	{
		// Add in rotation offset, but not in the editor
		if( !GIsEditor || GIsGame )
		{
			if( AngleOffset.X != 0.f )
			{
				SafeAim.X = UnWindNormalizedAimAngleGE(SafeAim.X - AngleOffset.X);
			}
			if( AngleOffset.Y != 0.f )
			{
				SafeAim.Y = UnWindNormalizedAimAngleGE(SafeAim.Y - AngleOffset.Y);
			}
		}

		// Scale by range
		if( SafeAim.X < 0.f )
		{
			if( HorizontalRange.X != 0.f )
			{
				SafeAim.X = SafeAim.X / Abs(HorizontalRange.X);
			}
			else
			{
				SafeAim.X = 0.f;
			}
		}
		else
		{
			if( HorizontalRange.Y != 0.f )
			{
				SafeAim.X = SafeAim.X / HorizontalRange.Y;
			}
			else
			{
				SafeAim.X = 0.f;
			}
		}

		if( SafeAim.Y < 0.f )
		{
			if( VerticalRange.X != 0.f )
			{
				SafeAim.Y = SafeAim.Y / Abs(VerticalRange.X);
			}
			else
			{
				SafeAim.Y = 0.f;
			}
		}
		else
		{
			if( VerticalRange.Y != 0.f )
			{
				SafeAim.Y = SafeAim.Y / VerticalRange.Y;
			}
			else
			{
				SafeAim.Y = 0.f;
			}
		}

		// Make sure we're using values within legal range.
		SafeAim.X = Clamp<FLOAT>(SafeAim.X, -1.f, +1.f);
		SafeAim.Y = Clamp<FLOAT>(SafeAim.Y, -1.f, +1.f);

		if( SafeAim.X >= 0.f && SafeAim.Y >= 0.f ) // Up Right
		{
			// Calculate weight of each relevant animation
			Anims(4).Weight = BiLerp(1.f, 0.f, 0.f, 0.f, SafeAim.X, SafeAim.Y);
			Anims(7).Weight = BiLerp(0.f, 1.f, 0.f, 0.f, SafeAim.X, SafeAim.Y);
			Anims(3).Weight = BiLerp(0.f, 0.f, 1.f, 0.f, SafeAim.X, SafeAim.Y);
			Anims(6).Weight = BiLerp(0.f, 0.f, 0.f, 1.f, SafeAim.X, SafeAim.Y);

			// The rest is set to zero
			Anims(0).Weight = 0.f;
			Anims(1).Weight = 0.f;
			Anims(2).Weight = 0.f;
			Anims(5).Weight = 0.f;
			Anims(8).Weight = 0.f;
		}
		else if( SafeAim.X >= 0.f && SafeAim.Y < 0.f ) // Bottom Right
		{
			SafeAim.Y += 1.f;

			// Calculate weight of each relevant animation
			Anims(5).Weight = BiLerp(1.f, 0.f, 0.f, 0.f, SafeAim.X, SafeAim.Y);
			Anims(8).Weight = BiLerp(0.f, 1.f, 0.f, 0.f, SafeAim.X, SafeAim.Y);
			Anims(4).Weight = BiLerp(0.f, 0.f, 1.f, 0.f, SafeAim.X, SafeAim.Y);
			Anims(7).Weight = BiLerp(0.f, 0.f, 0.f, 1.f, SafeAim.X, SafeAim.Y);

			// The rest is set to zero
			Anims(0).Weight = 0.f;
			Anims(1).Weight = 0.f;
			Anims(2).Weight = 0.f;
			Anims(3).Weight = 0.f;
			Anims(6).Weight = 0.f;
		}
		else if( SafeAim.X < 0.f && SafeAim.Y >= 0.f ) // Up Left
		{
			SafeAim.X += 1.f;

			// Calculate weight of each relevant animation
			Anims(1).Weight = BiLerp(1.f, 0.f, 0.f, 0.f, SafeAim.X, SafeAim.Y);
			Anims(4).Weight = BiLerp(0.f, 1.f, 0.f, 0.f, SafeAim.X, SafeAim.Y);
			Anims(0).Weight = BiLerp(0.f, 0.f, 1.f, 0.f, SafeAim.X, SafeAim.Y);
			Anims(3).Weight = BiLerp(0.f, 0.f, 0.f, 1.f, SafeAim.X, SafeAim.Y);

			// The rest is set to zero
			Anims(2).Weight = 0.f;
			Anims(5).Weight = 0.f;
			Anims(6).Weight = 0.f;
			Anims(7).Weight = 0.f;
			Anims(8).Weight = 0.f;
		}
		else if( SafeAim.X < 0.f && SafeAim.Y < 0.f ) // Bottom Left
		{
			SafeAim.X += 1.f;
			SafeAim.Y += 1.f;

			// Calculate weight of each relevant animation
			Anims(2).Weight = BiLerp(1.f, 0.f, 0.f, 0.f, SafeAim.X, SafeAim.Y);
			Anims(5).Weight = BiLerp(0.f, 1.f, 0.f, 0.f, SafeAim.X, SafeAim.Y);
			Anims(1).Weight = BiLerp(0.f, 0.f, 1.f, 0.f, SafeAim.X, SafeAim.Y);
			Anims(4).Weight = BiLerp(0.f, 0.f, 0.f, 1.f, SafeAim.X, SafeAim.Y);

			// The rest is set to zero
			Anims(0).Weight = 0.f;
			Anims(3).Weight = 0.f;
			Anims(6).Weight = 0.f;
			Anims(7).Weight = 0.f;
			Anims(8).Weight = 0.f;
		}
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}


FLOAT UAnimNodeSequenceBlendByAim::GetSliderPosition(INT SliderIndex, INT ValueIndex)
{
	check(SliderIndex == 0);
	check(ValueIndex == 0 || ValueIndex == 1);

	if( ValueIndex == 0 )
	{
		return (0.5f * Aim.X) + 0.5f;
	}
	else
	{
		return ((-0.5f * Aim.Y) + 0.5f);
	}
}

void UAnimNodeSequenceBlendByAim::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(SliderIndex == 0);
	check(ValueIndex == 0 || ValueIndex == 1);

	if( ValueIndex == 0 )
	{
		Aim.X = (NewSliderValue - 0.5f) * 2.f;
	}
	else
	{
		Aim.Y = (NewSliderValue - 0.5f) * -2.f;
	}
}

FString UAnimNodeSequenceBlendByAim::GetSliderDrawValue(INT SliderIndex)
{
	check(SliderIndex == 0);
	return FString::Printf(TEXT("%0.2f,%0.2f"), Aim.X, Aim.Y);
}
