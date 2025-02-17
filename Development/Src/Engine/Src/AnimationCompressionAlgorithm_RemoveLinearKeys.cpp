/*=============================================================================
	AnimationCompressionAlgorithm_RemoveLinearKeys.cpp: Keyframe reduction algorithm that simply removes every second key.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"
#include "AnimationUtils.h"
#include "AnimationEncodingFormat.h"

IMPLEMENT_CLASS(UAnimationCompressionAlgorithm_RemoveLinearKeys);


/**
 * Helper function to enforce that the delta between two Quaternions represents
 * the shortest possible rotation angle
 */
static FQuat EnforceShortestArc(const FQuat& A, const FQuat& B)
{
	const FLOAT DotResult = (A | B);
	const FLOAT Bias = appFloatSelect(DotResult, 1.0f, -1.0f);
	return B*Bias;
}

/**
 * Helper template function to interpolate between two data types.
 * Used in the FilterLinearKeysTemplate function below
 */
template <typename T>
T Interpolate(const T& A, const T& B, FLOAT Alpha)
{
	// only the custom instantiations below are valid
	check(0);
	return 0;
}

/** custom instantiation of Interpolate for FVectors */
template <> FVector Interpolate<FVector>(const FVector& A, const FVector& B, FLOAT Alpha)
{
	return Lerp(A,B,Alpha);
}

/** custom instantiation of Interpolate for FQuats */
template <> FQuat Interpolate<FQuat>(const FQuat& A, const FQuat& B, FLOAT Alpha)
{
	FQuat result = LerpQuat(A,B,Alpha);
	result.Normalize();

	return result;
}

/**
 * Helper template function to calculate the delta between two data types.
 * Used in the FilterLinearKeysTemplate function below
 */
template <typename T>
FLOAT CalcDelta(const T& A, const T& B)
{
	// only the custom instantiations below are valid
	check(0);
	return 0;
}

/** custom instantiation of CalcDelta for FVectors */
template <> FLOAT CalcDelta<FVector>(const FVector& A, const FVector& B)
{
	return (A - B).Size();
}

/** custom instantiation of CalcDelta for FQuat */
template <> FLOAT CalcDelta<FQuat>(const FQuat& A, const FQuat& B)
{
	return FQuatError(A, EnforceShortestArc(A, B));
}

/**
 * Helper function to replace a specific component in the FBoneAtom structure
 */
template <typename T>
FBoneAtom UpdateBoneAtom(INT BoneIndex, const FBoneAtom& Atom, const T& Component)
{
	// only the custom instantiations below are valid
	check(0);
	return FBoneAtom();
}

/** custom instantiation of UpdateBoneAtom for FVectors */
template <> FBoneAtom UpdateBoneAtom<FVector>(INT BoneIndex, const FBoneAtom& Atom, const FVector& Component)
{
	return FBoneAtom(Atom.Rotation, Component, 1.0f);
}

/** custom instantiation of UpdateBoneAtom for FQuat */
template <> FBoneAtom UpdateBoneAtom<FQuat>(INT BoneIndex, const FBoneAtom& Atom, const FQuat& Component)
{
	FQuat IncommingQuat = Component;
	if (BoneIndex > 0)
	{
		IncommingQuat.W *= -1.0f;
	}
	return FBoneAtom(IncommingQuat, Atom.Translation, 1.0f);
}


/**
 * Template function to reduce the keys of a given data type.
 * Used to reduce both Translation and Rotation keys using the cooresponding
 * data types FVector and FQuat
 */
template <typename T>
void FilterLinearKeysTemplate(
	TArray<T>& Keys,
    TArray<FLOAT>& Times,
	TArray<FBoneAtom>& BoneAtoms,
    const TArray<FLOAT>* ParentTimes,
	const TArray<FMatrix>& RawWorldBones,
	const TArray<FMatrix>& NewWorldBones,
    const TArray<INT>& TargetBoneIndices,
	INT NumFrames,
	INT BoneIndex,
	INT ParentBoneIndex,
	FLOAT ParentScale,
	FLOAT MaxDelta,
	FLOAT MaxTargetDelta)
{
	const INT KeyCount = Keys.Num();
	check( Keys.Num() == Times.Num() );
	check( KeyCount == 1 || KeyCount == NumFrames ); // must begin with one key per frame
	
	// generate new arrays we will fill with the final keys
	TArray<T> NewKeys;
	TArray<FLOAT> NewTimes;
	NewKeys.Empty(KeyCount);
	NewTimes.Empty(KeyCount);

	// Only bother doing anything if we have some keys!
	if(KeyCount > 0)
	{
		INT LowKey = 0;
		INT HighKey = KeyCount-1;
		INT PrevKey = 0;
		
		// copy the low key (this one is a given)
		NewTimes.AddItem(Times(0));
		NewKeys.AddItem(Keys(0));

		// We will test within a sliding window between LowKey and HighKey.
		// Therefore, we are done when the LowKey exceeds the range
		while (LowKey < KeyCount-1)
		{
			// high key always starts at the top of the range
			HighKey = KeyCount-1;

			// keep testing until the window is closed
			while (HighKey > LowKey+1)
			{
				// get the parameters of the window we are testing
				const FLOAT LowTime = Times(LowKey);
				const FLOAT HighTime = Times(HighKey);
				const T LowValue = Keys(LowKey);
				const T HighValue = Keys(HighKey);
				const FLOAT Range = HighTime - LowTime;
				const FLOAT InvRange = 1.0f/Range;

				// iterate through all interpolated members of the window to
				// compute the error when compared to the original raw values
				FLOAT MaxLerpError = 0.0f;
				FLOAT MaxTargetError = 0.0f;
				for (INT TestKey = LowKey+1; TestKey< HighKey; ++TestKey)
				{
					// get the parameters of the member being tested
					FLOAT TestTime = Times(TestKey);
					T TestValue = Keys(TestKey);

					// compute the proposed, interpolated value for the key
					const FLOAT Alpha = (TestTime - LowTime) * InvRange;
					const T LerpValue = Interpolate(LowValue, HighValue, Alpha);

					// compute the error between our interpolated value and the desired value
					FLOAT LerpError = CalcDelta(TestValue, LerpValue);

					// if the local-space lerp error is within our tolerances, we will also check the
					// effect this interpolated key will have on our target end effectors
					FLOAT TargetError = -1.0f;
					if (LerpError <= MaxDelta)
					{
						// get the raw world transform for this bone (the original world-space position)
						const INT FrameIndex = TestKey;
						const FMatrix& RawBase = RawWorldBones((BoneIndex*NumFrames) + FrameIndex);
						const FMatrix InvRawBase = RawBase.Inverse();
						
						// generate the proposed local bone atom and transform (local space)
						FBoneAtom ProposedAtom = UpdateBoneAtom(BoneIndex, BoneAtoms(FrameIndex), LerpValue);
						FMatrix ProposedTM;
						ProposedAtom.ToTransform(ProposedTM);

						// convert the proposed local transform to world space using this bone's parent transform
						const FMatrix& CurrentParent = ParentBoneIndex != INDEX_NONE ? NewWorldBones((ParentBoneIndex*NumFrames) + FrameIndex) : FMatrix::Identity;
						FMatrix ProposedBase = ProposedTM * CurrentParent;
						
						// for each target end effector, compute the error we would introduce with our proposed key
						for (INT TargetIndex=0; TargetIndex<TargetBoneIndices.Num(); ++TargetIndex)
						{
							// find the offset transform from the raw base to the end effector
							const INT TargetBoneIndex = TargetBoneIndices(TargetIndex);
							const FMatrix& RawTarget = RawWorldBones((TargetBoneIndex*NumFrames) + FrameIndex);
							const FMatrix RelTM = RawTarget * InvRawBase; 

							// forcast where the new end effector would be using our proposed key
							FMatrix ProposedTarget = RelTM * ProposedBase;

							// determine the extend of error at the target end effector
							FLOAT ThisError = (ProposedTarget.GetOrigin() - RawTarget.GetOrigin()).Size();
							TargetError = Max(TargetError, ThisError); 

							// exit early when we encounter a large delta
							if (TargetError > MaxTargetDelta)
							{
								break;
							}
						}
					}

					// If the parent has a key at this time, we'll scale our error values as requested.
					// This increases the odds that we will choose keys on the same frames as our parent bone,
					// making the skeleton more uniform in key distribution.
					if (ParentTimes)
					{
						if (ParentTimes->FindItemIndex(TestTime) != INDEX_NONE)
						{
							// our parent has a key at this time, 
							// inflate our percieved error to increase our sensitivity
							// for also retaining a key at this time
							LerpError *= ParentScale;
							TargetError *= ParentScale;
						}
					}
					
					// keep track of the worst errors encountered for both 
					// the local-space 'lerp' error and the end effector drift we will cause
					MaxLerpError = Max(MaxLerpError, LerpError);
					MaxTargetError = Max(MaxTargetError, TargetError);

					// exit early if we have failed in this span
					if (MaxLerpError > MaxDelta ||
						MaxTargetError > MaxTargetDelta)
					{
						break;
					}
				}

				// determine if the span succeeded. That is, the worst errors found are within tolerances
				if (MaxLerpError <= MaxDelta &&
					MaxTargetError <= MaxTargetDelta)
				{
					// save the high end of the test span as our next key
					NewTimes.AddItem(Times(HighKey));
					NewKeys.AddItem(Keys(HighKey));

					// start testing a new span
					LowKey = HighKey;
					HighKey =  KeyCount-1;
				}
				else
				{
					// we failed, shrink the test span window and repeat
					--HighKey;
				}
			}

			// if the test window is still valid, accept the high key
			if (HighKey > LowKey)
			{
				NewTimes.AddItem(Times(HighKey));
				NewKeys.AddItem(Keys(HighKey));
			}
			LowKey= HighKey;
		}

		// The process has eneded, but we must make sure the last key is accounted for
		if (NewTimes.Last() != Times.Last() &&
			CalcDelta(Keys.Last(), NewKeys.Last()) >= MaxDelta )
		{
			NewTimes.AddItem(Times.Last());
			NewKeys.AddItem(Keys.Last());
		}

		// remove any padding from the key set we have built
		NewTimes.Shrink();
		NewKeys.Shrink();

		// return the new key set to the caller
		Times= NewTimes;
		Keys= NewKeys;
	}
}

/**
 * To guide the key removal process, we need to maintain a table of world transforms
 * for the bones we are investigating. This helper function fills a row of the 
 * table for a specified bone.
 */
static void UpdateWorldBoneTransformTable(
	UAnimSequence* AnimSeq, 
	USkeletalMesh* SkelMesh, 
	const TArray<FBoneData>& BoneData, 
	const FAnimSetMeshLinkup& AnimLinkup,
	const TArray<FMeshBone>& RefSkel,
	INT BoneIndex,
	UBOOL UseRaw,
	TArray<FMatrix>& OutputWorldBones)
{
	const FBoneData& Bone		= BoneData(BoneIndex);
	const INT NumFrames			= AnimSeq->NumFrames;
	const FLOAT SequenceLength	= AnimSeq->SequenceLength;
	const INT FrameStart		= (BoneIndex*NumFrames);
	const INT TrackIndex		= AnimLinkup.BoneToTrackTable(BoneIndex);
	
	check(OutputWorldBones.Num() >= (FrameStart+NumFrames));

	const FLOAT TimePerFrame = SequenceLength / (FLOAT)(NumFrames-1);

	if (TrackIndex != INDEX_NONE)
	{
		// get the local-space bone transforms using the animation solver
		for ( INT FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex )
		{
			FLOAT Time = (FLOAT)FrameIndex * TimePerFrame;
			FBoneAtom LocalAtom;
			AnimSeq->GetBoneAtom(LocalAtom, TrackIndex, Time, FALSE, UseRaw);

			if (BoneIndex > 0)
			{
				LocalAtom.Rotation.W *= -1.0f;
			}

			LocalAtom.Rotation= EnforceShortestArc(FQuat::Identity, LocalAtom.Rotation);
			LocalAtom.ToTransform(OutputWorldBones((BoneIndex*NumFrames) + FrameIndex));
		}

	}
	else
	{
		// get the default rotation and translation from the reference skeleton
		FMatrix DefaultTransform;
		FBoneAtom LocalAtom= FBoneAtom( RefSkel(BoneIndex).BonePos.Orientation, RefSkel(BoneIndex).BonePos.Position, 1.f );
		LocalAtom.Rotation= EnforceShortestArc(FQuat::Identity, LocalAtom.Rotation);
		LocalAtom.ToTransform(DefaultTransform);

		// copy the default transformation into the world bone table
		for ( INT FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex )
		{
			OutputWorldBones((BoneIndex*NumFrames) + FrameIndex) = DefaultTransform;
		}
	}

	// apply parent transforms to bake into world space. We assume the parent transforms were previously set using this function
	const INT ParentIndex = Bone.GetParent();
	if (ParentIndex != INDEX_NONE)
	{
		check (ParentIndex < BoneIndex);
		for ( INT FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex )
		{
			OutputWorldBones((BoneIndex*NumFrames) + FrameIndex) = OutputWorldBones((BoneIndex*NumFrames) + FrameIndex) * OutputWorldBones((ParentIndex*NumFrames) + FrameIndex);
		}
	}
}


static void UpdateWorldBoneTransformRange(
	UAnimSequence* AnimSeq, 
	USkeletalMesh* SkelMesh, 
	const TArray<FBoneData>& BoneData, 
	const FAnimSetMeshLinkup& AnimLinkup,
	const TArray<FMeshBone>& RefSkel,
	const TArray<FTranslationTrack>& PositionTracks,
	const TArray<FRotationTrack>& RotationTracks,
	AnimationCompressionFormat TranslationCompressionFormat,
	AnimationCompressionFormat RotationCompressionFormat,
	INT StartingBoneIndex,
	INT EndingBoneIndex,
	UBOOL UseRaw,
	TArray<FMatrix>& OutputWorldBones)
{
	// bitwise compress the tracks into the anim sequence buffers
	// to make sure the data we've compressed so far is ready for solving
	UAnimationCompressionAlgorithm::BitwiseCompressAnimationTracks(
		AnimSeq,
		TranslationCompressionFormat,
		RotationCompressionFormat,
		PositionTracks,
		RotationTracks,
		TRUE);

	// build all world-space transforms from this bone to the target end effetor we are monitoring
	// all parent transforms have been built already
	for ( INT Index = StartingBoneIndex; Index <= EndingBoneIndex; ++Index )
	{
		UpdateWorldBoneTransformTable(
			AnimSeq, 
			SkelMesh, 
			BoneData, 
			AnimLinkup,
			RefSkel,
			Index,
			UseRaw,
			OutputWorldBones);
	}
}

static void UpdateBoneAtomList(
	UAnimSequence* AnimSeq, 
	INT BoneIndex,
	INT TrackIndex,
	INT NumFrames,
	FLOAT TimePerFrame,
	TArray<FBoneAtom>& BoneAtoms)
{
	BoneAtoms.Empty(NumFrames);
	for ( INT FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex )
	{
		FLOAT Time = (FLOAT)FrameIndex * TimePerFrame;
		FBoneAtom LocalAtom;
		AnimSeq->GetBoneAtom(LocalAtom, TrackIndex, Time, FALSE, FALSE);

		if (BoneIndex > 0)
		{
			LocalAtom.Rotation.W *= -1.0f;
		}

		LocalAtom.Rotation= EnforceShortestArc(FQuat::Identity, LocalAtom.Rotation);
		BoneAtoms.AddItem(LocalAtom);
	}
}

/**
 * Locates spans of keys within the position and rotation tracks provided which can be estimated
 * through linear interpolation of the surrounding keys. The remaining key values are bit packed into
 * the animation sequence provided
 *
 * @param	AnimSeq		The animation sequence being compressed
 * @param	SkelMesh	The skeletal mesh to use to guide the compressor
 * @param	BoneData	BoneData array describing the hierarchy of the animated skeleton
 * @param	TranslationCompressionFormat	The format to use when encoding translation keys.
 * @param	RotationCompressionFormat		The format to use when encoding rotation keys.
 * @param	TranslationData		Translation Tracks to compress and bit-pack into the Animation Sequence.
 * @param	RotationData		Rotation Tracks to compress and bit-pack into the Animation Sequence.
 * @return				None.
 */
void UAnimationCompressionAlgorithm_RemoveLinearKeys::ProcessAnimationTracks(
	UAnimSequence* AnimSeq, 
	USkeletalMesh* SkelMesh, 
	const TArray<FBoneData>& BoneData, 
	AnimationCompressionFormat TranslationCompressionFormat,
	AnimationCompressionFormat RotationCompressionFormat,
	TArray<FTranslationTrack>& PositionTracks,
	TArray<FRotationTrack>& RotationTracks)
{
	// extract all the data we'll need about the skeleton and animation sequence
	const INT NumBones			= BoneData.Num();
	const INT NumFrames			= AnimSeq->NumFrames;
	const FLOAT SequenceLength	= AnimSeq->SequenceLength;
	const INT LastFrame = NumFrames-1;
	const FLOAT FrameRate = (FLOAT)(LastFrame) / SequenceLength;
	const FLOAT TimePerFrame = SequenceLength / (FLOAT)(LastFrame);

	UAnimSet* AnimSet		= AnimSeq->GetAnimSet();
	const INT AnimLinkupIndex	= AnimSet->GetMeshLinkupIndex( SkelMesh );
	check( AnimLinkupIndex != INDEX_NONE );
	check( AnimLinkupIndex < AnimSet->LinkupCache.Num() );

	const FAnimSetMeshLinkup& AnimLinkup	= AnimSet->LinkupCache( AnimLinkupIndex );
	const TArray<FMeshBone>& RefSkel		= SkelMesh->RefSkeleton;
	check( AnimLinkup.BoneToTrackTable.Num() == RefSkel.Num() );

	// make sure the parent key scale is properly bound to 1.0 or more
	ParentKeyScale = Max(ParentKeyScale, 1.0f);

	// generate the raw and compressed skeleton in world-space
	TArray<FMatrix> RawWorldBones;
	TArray<FMatrix> NewWorldBones;
	RawWorldBones.Empty(NumBones * NumFrames);
	NewWorldBones.Empty(NumBones * NumFrames);
	RawWorldBones.AddZeroed(NumBones * NumFrames);
	NewWorldBones.AddZeroed(NumBones * NumFrames);

	// generate an array to hold the indices of our end effectors
	TArray<INT> EndEffectors;
	EndEffectors.Empty(NumBones);

	// Create an array of FBoneAtom to use as a workspace
	TArray<FBoneAtom> BoneAtoms;

	// setup the raw bone transformation and find all end effectors
	for ( INT BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex )
	{
		// get the raw world-atoms for this bone
		UpdateWorldBoneTransformTable(
			AnimSeq, 
			SkelMesh, 
			BoneData, 
			AnimLinkup,
			RefSkel,
			BoneIndex,
			TRUE,
			RawWorldBones);

		// also record all end-effectors we find
		const FBoneData& Bone = BoneData(BoneIndex);
		if (Bone.IsEndEffector())
		{
			EndEffectors.AddItem(BoneIndex);
		}
	}

	// for each bone...
	for ( INT BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex )
	{
		const FBoneData& Bone = BoneData(BoneIndex);
		const INT ParentBoneIndex= Bone.GetParent();

		const INT TrackIndex = AnimLinkup.BoneToTrackTable(BoneIndex);
		if (TrackIndex != INDEX_NONE)
		{
			// get the tracks we will be editing for this bone
			FRotationTrack& RotTrack = RotationTracks(TrackIndex);
			FTranslationTrack& TransTrack = PositionTracks(TrackIndex);
			const INT NumRotKeys = RotTrack.RotKeys.Num();
			const INT NumPosKeys = TransTrack.PosKeys.Num();

			check( NumPosKeys == 1 || NumPosKeys == NumFrames );
			check( NumRotKeys == 1 || NumRotKeys == NumFrames );

			// build an array of end effectors we need to monitor
			TArray<INT> TargetBoneIndices;
			TargetBoneIndices.Empty(NumBones);
			INT HighestTargetBoneIndex= BoneIndex;
			INT FurthestTargetBoneIndex= BoneIndex;
			INT ShortestChain = 0;
			FLOAT OffsetLength= -1.0f;
			for (long EffectorIndex=0; EffectorIndex < EndEffectors.Num(); ++EffectorIndex)
			{
				const INT EffectorBoneIndex = EndEffectors(EffectorIndex);
				const FBoneData& EffectorBoneData = BoneData(EffectorBoneIndex);

				INT RootIndex = EffectorBoneData.BonesToRoot.FindItemIndex(BoneIndex);
				if (RootIndex != INDEX_NONE)
				{
					if (ShortestChain == 0 || (RootIndex+1) < ShortestChain)
					{
						ShortestChain = (RootIndex+1);
					}
					TargetBoneIndices.AddItem(EffectorBoneIndex);
					HighestTargetBoneIndex = Max(HighestTargetBoneIndex, EffectorBoneIndex);
					FLOAT ChainLength= 0.0f;
					for (long FamilyIndex=0; FamilyIndex < RootIndex; ++FamilyIndex)
					{
						const INT NextParentBoneIndex= EffectorBoneData.BonesToRoot(FamilyIndex);
						ChainLength += RefSkel(NextParentBoneIndex).BonePos.Position.Size();
					}

					if (ChainLength > OffsetLength)
					{
						FurthestTargetBoneIndex = EffectorBoneIndex;
						OffsetLength = ChainLength;
					}

				}
			}
			
			// if requested, retarget the FBoneAtoms towards the target end effectors
			if (bRetarget)
			{
				if (NumRotKeys > 0)
				{
					if (HighestTargetBoneIndex == BoneIndex)
					{
						for ( INT KeyIndex = 0; KeyIndex < NumRotKeys; ++KeyIndex )
						{
							FQuat& Key= RotTrack.RotKeys(KeyIndex);

							const INT FrameIndex= Clamp(KeyIndex, 0, LastFrame);
							const FMatrix& NewWorldParent= NewWorldBones((ParentBoneIndex*NumFrames) + FrameIndex);
							const FMatrix& RawWorldChild= RawWorldBones((BoneIndex*NumFrames) + FrameIndex);
							const FMatrix InvNewWorldParent = NewWorldParent.Inverse();
							const FMatrix RelTM= RawWorldChild * InvNewWorldParent; 
							FBoneAtom Delta = FBoneAtom(RelTM);

							// rotations we apply need to be in the inverted key space
							if (BoneIndex > 0)
							{
								Delta.Rotation.W *= -1.0f;
							}

							FQuat AlignedKey= EnforceShortestArc(Key, Delta.Rotation);
							Key= AlignedKey;
						}
					}
					else
					{
						// update our bone table from the current bone through the last end effector we need to test
						UpdateWorldBoneTransformRange(
							AnimSeq, 
							SkelMesh, 
							BoneData, 
							AnimLinkup,
							RefSkel,
							PositionTracks,
							RotationTracks,
							TranslationCompressionFormat,
							RotationCompressionFormat,
							BoneIndex,
							HighestTargetBoneIndex,
							FALSE,
							NewWorldBones);

						// adjust all rotation keys towards the end effector target
						for ( INT KeyIndex = 0; KeyIndex < NumRotKeys; ++KeyIndex )
						{
							FQuat& Key= RotTrack.RotKeys(KeyIndex);

							const INT FrameIndex= Clamp(KeyIndex, 0, LastFrame);

							const FMatrix& NewWorldTransform= NewWorldBones((BoneIndex*NumFrames) + FrameIndex);
							FMatrix DesiredChildTransform= RawWorldBones((FurthestTargetBoneIndex*NumFrames) + FrameIndex);
							FMatrix CurrentChildTransform= NewWorldBones((FurthestTargetBoneIndex*NumFrames) + FrameIndex);
							
							const FMatrix InvSpace = NewWorldTransform.Inverse();
							DesiredChildTransform *= InvSpace;
							CurrentChildTransform *= InvSpace;

							// find the two vectors which represent the angular error we are trying to correct
							FVector CurrentHeading= CurrentChildTransform.GetOrigin();
							FVector DesiredHeading= DesiredChildTransform.GetOrigin();

							// if these are valid, we can continue
							if (!CurrentHeading.IsNearlyZero() && !DesiredHeading.IsNearlyZero())
							{
								CurrentHeading.Normalize();
								DesiredHeading.Normalize();
								const FLOAT DotResult= CurrentHeading | DesiredHeading;

								// limit the range we will retarget to something reasonable (~60 degrees)
								if (DotResult < 1.0f && DotResult > 0.5f)
								{
									FQuat Adjustment= FQuatFindBetween(CurrentHeading, DesiredHeading); 
									Adjustment.Normalize();
									Adjustment= EnforceShortestArc(FQuat::Identity, Adjustment);

									const FVector Test = Adjustment.RotateVector(CurrentHeading);
									const FLOAT Delta = (Test - DesiredHeading).Size();
									if (Delta < 0.001f)
									{
										// rotations we apply need to be in the inverted key space
										if (BoneIndex > 0)
										{
											Adjustment.W *= -1.0f;
										}

										FQuat NewKey = Adjustment * Key;
										NewKey.Normalize();

										FQuat AlignedKey= EnforceShortestArc(Key, NewKey);
										Key= AlignedKey;
									}
								}
							}
						}
					}
				}

				if (NumPosKeys > 0 && ParentBoneIndex != INDEX_NONE)
				{
					// update our bone table from the current bone through the last end effector we need to test
					UpdateWorldBoneTransformRange(
						AnimSeq, 
						SkelMesh, 
						BoneData, 
						AnimLinkup,
						RefSkel,
						PositionTracks,
						RotationTracks,
						TranslationCompressionFormat,
						RotationCompressionFormat,
						BoneIndex,
						HighestTargetBoneIndex,
						FALSE,
						NewWorldBones);

					// adjust all translation keys to align better with the destination
					for ( INT KeyIndex = 0; KeyIndex < NumPosKeys; ++KeyIndex )
					{
						FVector& Key= TransTrack.PosKeys(KeyIndex);

						const INT FrameIndex= Clamp(KeyIndex, 0, LastFrame);
						const FMatrix& NewWorldParent= NewWorldBones((ParentBoneIndex*NumFrames) + FrameIndex);
						const FMatrix& RawWorldChild= RawWorldBones((BoneIndex*NumFrames) + FrameIndex);
						const FMatrix InvNewWorldParent = NewWorldParent.Inverse();
						const FMatrix RelTM= RawWorldChild * InvNewWorldParent; 
						const FBoneAtom Delta = FBoneAtom(RelTM);

						Key= Delta.Translation;
					}
				}

			}

			// look for a parent track to reference as a guide
			INT GuideTrackIndex = INDEX_NONE;
			if (ParentKeyScale > 1.0f)
			{
				for (long FamilyIndex=0; (FamilyIndex < Bone.BonesToRoot.Num()) && (GuideTrackIndex == INDEX_NONE); ++FamilyIndex)
				{
					const INT NextParentBoneIndex= Bone.BonesToRoot(FamilyIndex);
					GuideTrackIndex = AnimLinkup.BoneToTrackTable(NextParentBoneIndex);
				}
			}

			// update our bone table from the current bone through the last end effector we need to test
			UpdateWorldBoneTransformRange(
				AnimSeq, 
				SkelMesh, 
				BoneData, 
				AnimLinkup,
				RefSkel,
				PositionTracks,
				RotationTracks,
				TranslationCompressionFormat,
				RotationCompressionFormat,
				BoneIndex,
				HighestTargetBoneIndex,
				FALSE,
				NewWorldBones);

			// rebuild the BoneAtoms table using the current set of keys
			UpdateBoneAtomList(AnimSeq, BoneIndex, TrackIndex, NumFrames, TimePerFrame, BoneAtoms); 

			// determine the EndEffectorTolerance. 
			// We use the Maximum value by default, and the Minimum value
			// as we apprach the end effectors
			FLOAT EndEffectorTolerance = MaxEffectorDiff;
			if (ShortestChain <= 1)
			{
				EndEffectorTolerance = MinEffectorDiff;
			}

			// Determine if a guidance track should be used to aid in choosing keys to retain
			TArray<FLOAT>* GuidanceTrack = NULL;
			FLOAT GuidanceScale = 1.0f;
			if (GuideTrackIndex != INDEX_NONE)
			{
				FRotationTrack& GuideRotTrack = RotationTracks(GuideTrackIndex);
				FTranslationTrack& GuideTransTrack = PositionTracks(GuideTrackIndex);
				GuidanceTrack = &GuideTransTrack.Times;
				GuidanceScale = ParentKeyScale;
			}
			
			// if the TargetBoneIndices array is empty, then this bone is an end effector.
			// so we add it to the list to maintain our tolerance checks
			if (TargetBoneIndices.Num() == 0)
			{
				TargetBoneIndices.AddItem(BoneIndex);
			}

			// filter out translations we can approximate through interpolation
			FilterLinearKeysTemplate<FVector>(
				TransTrack.PosKeys, 
				TransTrack.Times, 
				BoneAtoms,
				GuidanceTrack, 
				RawWorldBones,
				NewWorldBones,
				TargetBoneIndices,
				NumFrames,
				BoneIndex,
				ParentBoneIndex,
				GuidanceScale, 
				MaxPosDiff, 
				EndEffectorTolerance);

			// update our bone table from the current bone through the last end effector we need to test
			UpdateWorldBoneTransformRange(
				AnimSeq, 
				SkelMesh, 
				BoneData, 
				AnimLinkup,
				RefSkel,
				PositionTracks,
				RotationTracks,
				TranslationCompressionFormat,
				RotationCompressionFormat,
				BoneIndex,
				HighestTargetBoneIndex,
				FALSE,
				NewWorldBones);

			// rebuild the BoneAtoms table using the current set of keys
			UpdateBoneAtomList(AnimSeq, BoneIndex, TrackIndex, NumFrames, TimePerFrame, BoneAtoms); 

			// filter out rotations we can approximate through interpolation
			FilterLinearKeysTemplate<FQuat>(
				RotTrack.RotKeys, 
				RotTrack.Times, 
				BoneAtoms,
				GuidanceTrack, 
				RawWorldBones,
				NewWorldBones,
				TargetBoneIndices,
				NumFrames,
				BoneIndex,
				ParentBoneIndex,
				GuidanceScale, 
				MaxAngleDiff, 
				EndEffectorTolerance);
		}

		// make sure the final compressed keys are repesented in our NewWorldBones table
		UpdateWorldBoneTransformRange(
			AnimSeq, 
			SkelMesh, 
			BoneData, 
			AnimLinkup,
			RefSkel,
			PositionTracks,
			RotationTracks,
			TranslationCompressionFormat,
			RotationCompressionFormat,
			BoneIndex,
			BoneIndex,
			FALSE,
			NewWorldBones);
	}
};

/**
 * Keyframe reduction algorithm that simply removes every second key.
 *
 * @return		TRUE if the keyframe reduction was successful.
 */
void UAnimationCompressionAlgorithm_RemoveLinearKeys::DoReduction(UAnimSequence* AnimSeq, USkeletalMesh* SkelMesh, const TArray<FBoneData>& BoneData)
{
	if( GIsEditor )
	{
		GWarn->BeginSlowTask( TEXT("Compressing animation with RemoveLinearKeys scheme."), FALSE);
	}

	// if this is an additive animation, temporarily convert it out of relative-space
	UBOOL bAdditiveAnimation = AnimSeq->bIsAdditive;
	if (bAdditiveAnimation)
	{
		// let the sequence believe it is no longer additive
		AnimSeq->bIsAdditive = FALSE;

		// convert the raw tracks out of additive-space
		const INT NumTracks = AnimSeq->RawAnimData.Num();
		for ( INT TrackIndex = 0; TrackIndex < NumTracks; ++TrackIndex )
		{
			const FBoneAtom& RefBoneAtom = AnimSeq->AdditiveRefPose(TrackIndex);
			FRawAnimSequenceTrack& RawTrack	= AnimSeq->RawAnimData(TrackIndex);

			// transform position keys.
			for ( INT PosIndex = 0; PosIndex < RawTrack.PosKeys.Num(); ++PosIndex )
			{
				RawTrack.PosKeys(PosIndex) += RefBoneAtom.Translation;
			}

			// transform rotation keys.
			for ( INT RotIndex = 0; RotIndex < RawTrack.RotKeys.Num(); ++RotIndex )
			{
				RawTrack.RotKeys(RotIndex) = RawTrack.RotKeys(RotIndex) * RefBoneAtom.Rotation;
			}
		}
	}

	// split the filtered data into tracks
	TArray<FTranslationTrack> TranslationData;
	TArray<FRotationTrack> RotationData;
	SeparateRawDataIntoTracks( AnimSeq->RawAnimData, AnimSeq->SequenceLength, TranslationData, RotationData );
	
	// remove obviously redundant keys from the source data
	FilterTrivialKeys(TranslationData, RotationData, 0.0001f, 0.0003f);

	// compress this animation without any key-reduction to prime the codec
	BitwiseCompressAnimationTracks(
		AnimSeq,
		static_cast<AnimationCompressionFormat>(TranslationCompressionFormat),
		static_cast<AnimationCompressionFormat>(RotationCompressionFormat),
		TranslationData,
		RotationData,
		TRUE);

	// record the proper runtime decompressor to use
	AnimSeq->KeyEncodingFormat = AKF_VariableKeyLerp;
#if USE_ANIMATION_CODEC_INTERFACE
	// setup the Codec interfaces
	AnimationFormat_SetInterfaceLinks(*AnimSeq);
#endif
	AnimSeq->CompressionScheme = static_cast<UAnimationCompressionAlgorithm*>( StaticDuplicateObject( this, this, AnimSeq, TEXT("None"), ~RF_RootSet ) );
	AnimSeq->MarkPackageDirty();

	// now remove the keys which can be approximated with linear interpolation
	ProcessAnimationTracks(
		AnimSeq, 
		SkelMesh, 
		BoneData, 
		static_cast<AnimationCompressionFormat>(TranslationCompressionFormat),
		static_cast<AnimationCompressionFormat>(RotationCompressionFormat),
		TranslationData,
		RotationData);


	// if previously additive, convert back to relative-space
	if (bAdditiveAnimation)
	{
		// restore the additive flag in the sequence
		AnimSeq->bIsAdditive = TRUE;

		// convert the raw tracks back to additive-space
		const INT NumTracks = AnimSeq->RawAnimData.Num();
		for ( INT TrackIndex = 0; TrackIndex < NumTracks; ++TrackIndex )
		{
			const FBoneAtom& RefBoneAtom = AnimSeq->AdditiveRefPose(TrackIndex);
			const FBoneAtom InvRefBoneAtom(-RefBoneAtom.Rotation, -RefBoneAtom.Translation, 1.0f);
			FRawAnimSequenceTrack& RawTrack	= AnimSeq->RawAnimData(TrackIndex);

			// transform position keys.
			for ( INT PosIndex = 0; PosIndex < RawTrack.PosKeys.Num(); ++PosIndex )
			{
				RawTrack.PosKeys(PosIndex) += InvRefBoneAtom.Translation;
			}

			// transform rotation keys.
			for ( INT RotIndex = 0; RotIndex < RawTrack.RotKeys.Num(); ++RotIndex )
			{
				RawTrack.RotKeys(RotIndex) = RawTrack.RotKeys(RotIndex) * InvRefBoneAtom.Rotation;
			}

			// convert the new translation tracks to additive space
			FTranslationTrack& TranslationTrack = TranslationData(TrackIndex);
			for (INT KeyIndex = 0; KeyIndex < TranslationTrack.PosKeys.Num(); ++KeyIndex)
			{
				TranslationTrack.PosKeys(KeyIndex) += InvRefBoneAtom.Translation;
			}

			// convert the new rotation tracks to additive space
			FRotationTrack& RotationTrack = RotationData(TrackIndex);
			for (INT KeyIndex = 0; KeyIndex < RotationTrack.RotKeys.Num(); ++KeyIndex)
			{
				RotationTrack.RotKeys(KeyIndex) = RotationTrack.RotKeys(KeyIndex) * InvRefBoneAtom.Rotation;
			}
		}
	}

	// bitwise compress the final tracks into the anim sequence buffers
	BitwiseCompressAnimationTracks(
		AnimSeq,
		static_cast<AnimationCompressionFormat>(TranslationCompressionFormat),
		static_cast<AnimationCompressionFormat>(RotationCompressionFormat),
		TranslationData,
		RotationData,
		TRUE);

	if( GIsEditor )
	{
		GWarn->EndSlowTask();
	}
};

