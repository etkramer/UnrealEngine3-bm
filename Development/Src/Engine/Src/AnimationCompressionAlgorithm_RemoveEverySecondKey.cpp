/*=============================================================================
	AnimationCompressionAlgorithm_RemoveEverySecondKey.cpp: Keyframe reduction algorithm that simply removes every second key.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"
#include "AnimationUtils.h"

IMPLEMENT_CLASS(UAnimationCompressionAlgorithm_RemoveEverySecondKey);

/**
 * Keyframe reduction algorithm that simply removes every second key.
 *
 * @return		TRUE if the keyframe reduction was successful.
 */
void UAnimationCompressionAlgorithm_RemoveEverySecondKey::DoReduction(UAnimSequence* AnimSeq, USkeletalMesh* SkelMesh, const TArray<FBoneData>& BoneData)
{
	const INT StartIndex = bStartAtSecondKey ? 1 : 0;
	const INT Interval = 2;

	// split the filtered data into tracks
	TArray<FTranslationTrack> TranslationData;
	TArray<FRotationTrack> RotationData;
	SeparateRawDataIntoTracks( AnimSeq->RawAnimData, AnimSeq->SequenceLength, TranslationData, RotationData );

	// remove obviously redundant keys from the source data
	FilterTrivialKeys(TranslationData, RotationData, 0.0001f, 0.0003f);

	// remove intermittent keys from the source data
	FilterIntermittentKeys(TranslationData, RotationData, StartIndex, Interval);

	// bitwise compress the tracks into the anim sequence buffers
	BitwiseCompressAnimationTracks(
		AnimSeq,
		static_cast<AnimationCompressionFormat>(TranslationCompressionFormat),
		static_cast<AnimationCompressionFormat>(RotationCompressionFormat),
		TranslationData,
		RotationData);

	// record the proper runtime decompressor to use
	AnimSeq->KeyEncodingFormat = AKF_ConstantKeyLerp;
};

