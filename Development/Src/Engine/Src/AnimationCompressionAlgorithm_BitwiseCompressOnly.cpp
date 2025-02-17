/*=============================================================================
	AnimationCompressionAlgorithm_BitwiseCompressionOnly.cpp: Bitwise animation compression only; performs no key reduction.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"
#include "AnimationUtils.h"

IMPLEMENT_CLASS(UAnimationCompressionAlgorithm_BitwiseCompressOnly);

/**
 * Bitwise animation compression only; performs no key reduction.
 */
void UAnimationCompressionAlgorithm_BitwiseCompressOnly::DoReduction(UAnimSequence* AnimSeq, USkeletalMesh* SkelMesh, const TArray<FBoneData>& BoneData)
{
	// split the raw data into tracks
	TArray<FTranslationTrack> TranslationData;
	TArray<FRotationTrack> RotationData;
	SeparateRawDataIntoTracks( AnimSeq->RawAnimData, AnimSeq->SequenceLength, TranslationData, RotationData );

	// remove obviously redundant keys from the source data
	FilterTrivialKeys(TranslationData, RotationData, 0.0001f, 0.0003f);

	// bitwise compress the tracks into the anim sequence buffers
	BitwiseCompressAnimationTracks(
		AnimSeq,
		static_cast<AnimationCompressionFormat>(TranslationCompressionFormat),
		static_cast<AnimationCompressionFormat>(RotationCompressionFormat),
		TranslationData,
		RotationData);

	// record the proper runtime decompressor to use
	AnimSeq->KeyEncodingFormat = AKF_ConstantKeyLerp;
}
