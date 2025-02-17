/*=============================================================================
	AnimationCompressionAlgorithm_SeparateToTracks.cpp: Reverts any animation compression, restoring the animation to the raw data.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"
#include "AnimationUtils.h"

IMPLEMENT_CLASS(UAnimationCompressionAlgorithm_RevertToRaw);

/**
 * Reverts any animation compression, restoring the animation to the raw data.
 */
void UAnimationCompressionAlgorithm_RevertToRaw::DoReduction(UAnimSequence* AnimSeq, USkeletalMesh* SkelMesh, const TArray<FBoneData>& BoneData)
{
	AnimSeq->TranslationData.Empty();
	AnimSeq->RotationData.Empty();
	AnimSeq->CompressedTrackOffsets.Empty();
	AnimSeq->CompressedByteStream.Empty();

	// Separate to raw data to tracks so that the bitwise compressor will have a data to operate on.
	// AnimSeq->SeparateRawDataToTracks( AnimSeq->RawAnimData, AnimSeq->SequenceLength, AnimSeq->TranslationData, AnimSeq->RotationData );

	// record the proper runtime decompressor to use
	AnimSeq->KeyEncodingFormat = AKF_ConstantKeyLerp;
}
