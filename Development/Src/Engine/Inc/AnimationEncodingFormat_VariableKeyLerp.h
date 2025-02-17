/*=============================================================================
	AnimationEncodingFormat.h: Skeletal mesh animation compression.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 


#ifndef __ANIMATIONENCODINGFORMAT_VARIABLEKEYLERP_H__
#define __ANIMATIONENCODINGFORMAT_VARIABLEKEYLERP_H__

#include "AnimationEncodingFormat.h"

#if USE_ANIMATION_CODEC_INTERFACE

#ifdef _DEBUG
#define INLINE_CODE inline
#else
#define INLINE_CODE FORCEINLINE
#endif

/**
 * Base class for all Animation Encoding Formats using variably-spaced key interpolation.
 */
class AEFVariableKeyLerpShared : public AnimationEncodingFormat
{
public:

	/**
	 * Handles the ByteSwap of compressed rotation data on import
	 *
	 * @param	Seq				The UAnimSequence container.
	 * @param	MemoryReader	The FMemoryReader to read from.
	 * @param	RotTrackData	The compressed rotation data stream.
	 * @param	NumKeysRot		The number of keys present in the stream.
	 */
	void ByteSwapRotationIn(
		UAnimSequence& Seq, 
		FMemoryReader& MemoryReader,
		BYTE*& RotTrackData,
		INT NumKeysRot);

	/**
	 * Handles the ByteSwap of compressed translation data on import
	 *
	 * @param	Seq				The UAnimSequence container.
	 * @param	MemoryReader	The FMemoryReader to read from.
	 * @param	TransTrackData	The compressed translation data stream.
	 * @param	NumKeysTrans	The number of keys present in the stream.
	 */
	void ByteSwapTranslationIn(
		UAnimSequence& Seq, 
		FMemoryReader& MemoryReader,
		BYTE*& TransTrackData,
		INT NumKeysTrans);

	/**
	 * Handles the ByteSwap of compressed rotation data on export
	 *
	 * @param	Seq				The UAnimSequence container.
	 * @param	MemoryWriter	The FMemoryWriter to write to.
	 * @param	RotTrackData	The compressed rotation data stream.
	 * @param	NumKeysRot		The number of keys to write to the stream.
	 */
	void ByteSwapRotationOut(
		UAnimSequence& Seq, 
		FMemoryWriter& MemoryWriter,
		BYTE*& RotTrackData,
		INT NumKeysRot);

	/**
	 * Handles the ByteSwap of compressed translation data on export
	 *
	 * @param	Seq				The UAnimSequence container.
	 * @param	MemoryWriter	The FMemoryWriter to write to.
	 * @param	TransTrackData	The compressed translation data stream.
	 * @param	NumKeysTrans	The number of keys to write to the stream.
	 */
	void ByteSwapTranslationOut(
		UAnimSequence& Seq, 
		FMemoryWriter& MemoryWriter,
		BYTE*& TransTrackData,
		INT NumKeysTrans);

protected:

	/**
	 * Utility function to determine the two key indices to interpolate given a relative position in the animation
	 *
	 * @param	Seq				The UAnimSequence container.
	 * @param	FrameTable		The frame table containing a frame index for each key.
	 * @param	RelativePos		The relative position to solve in the range [0,1] inclusive.
	 * @param	bLooping		TRUE if the animation should be consider cyclic (last frame interpolates back to the start)
	 * @param	NumKeys			The number of keys present in the track being solved.
	 * @param	PosIndex0Out	Output value for the closest key index before the RelativePos specified.
	 * @param	PosIndex1Out	Output value for the closest key index after the RelativePos specified.
	 * @return	The rate at which to interpolate the two keys returned to obtain the final result.
	 */
	FLOAT TimeToIndex(
		const UAnimSequence& Seq,
		const BYTE* FrameTable,
		FLOAT RelativePos,
		UBOOL bLooping,
		INT NumKeys,
		INT &PosIndex0Out,
		INT &PosIndex1Out);
	
};

template<INT FORMAT>
class AEFVariableKeyLerp : public AEFVariableKeyLerpShared
{
public:
	/**
	 * Decompress the Rotation component of a BoneAtom
	 *
	 * @param	OutAtom			The FBoneAtom to fill in.
	 * @param	Stream			The compressed animation data.
	 * @param	NumKeys			The number of keys present in Stream.
	 * @param	Time			Current time to solve for.
	 * @param	RelativePos		Current position within the animation to solve for in the range [0.0,1.0].
	 * @param	bLooping		True when looping the stream in intended.
	 * @return					None. 
	 */
	void GetBoneAtomRotation(	
		FBoneAtom& OutAtom,
		const UAnimSequence& Seq,
		const BYTE* RESTRICT Stream,
		INT NumKeys,
		FLOAT Time,
		FLOAT RelativePos,
		UBOOL bLooping);

	/**
	 * Decompress the Translation component of a BoneAtom
	 *
	 * @param	OutAtom			The FBoneAtom to fill in.
	 * @param	Stream			The compressed animation data.
	 * @param	NumKeys			The number of keys present in Stream.
	 * @param	Time			Current time to solve for.
	 * @param	RelativePos		Current position within the animation to solve for in the range [0.0,1.0].
	 * @param	bLooping		True when looping the stream in intended.
	 * @return					None. 
	 */
	void GetBoneAtomTranslation(	
		FBoneAtom& OutAtom,
		const UAnimSequence& Seq,
		const BYTE* RESTRICT Stream,
		INT NumKeys,
		FLOAT Time,
		FLOAT RelativePos,
		UBOOL bLooping);

#if USE_ANIMATION_CODEC_BATCH_SOLVER

	/**
	 * Decompress all requested rotation components from an Animation Sequence
	 *
	 * @param	Atoms			The FBoneAtom array to fill in.
	 * @param	DesiredPairs	Array of requested bone information
	 * @param	Seq				The animation sequence to use.
	 * @param	Time			Current time to solve for.
	 * @param	bLooping		True when looping the stream in intended.
	 * @return					None. 
	 */
	void GetPoseRotations(	
		FBoneAtomArray& Atoms, 
		const BoneTrackArray& DesiredPairs,
		const UAnimSequence& Seq,
		FLOAT RelativePos,
		UBOOL bLooping);

	/**
	 * Decompress all requested translation components from an Animation Sequence
	 *
	 * @param	Atoms			The FBoneAtom array to fill in.
	 * @param	DesiredPairs	Array of requested bone information
	 * @param	Seq				The animation sequence to use.
	 * @param	Time			Current time to solve for.
	 * @param	bLooping		True when looping the stream in intended.
	 * @return					None. 
	 */
	void GetPoseTranslations(	
		FBoneAtomArray& Atoms,
		const BoneTrackArray& DesiredPairs,
		const UAnimSequence& Seq,
		FLOAT RelativePos,
		UBOOL bLooping);
#endif

};

/**
 * Utility function to find the key before the specified search value.
 * Templated for each possible type of frame table encoding size.
 *
 * @param	FrameTable		The frame table, containing on frame index value per key.
 * @param	NumKeys			The total number of keys in the table.
 * @param	SearchFrame		The Frame we are attemptng to find.
 * @param	KeyEstimate		An estimate of the best location to search from in the KeyTable.
 * @return	The index of the first key immediatly below the specified search frame.
 */
template <typename TABLE_TYPE>
INLINE_CODE INT FindLowKeyIndex(
	const TABLE_TYPE* FrameTable, 
	INT NumKeys, 
	INT SearchFrame, 
	INT KeyEstimate)
{
	const INT LastKeyIndex = NumKeys-1;
	INT LowKeyIndex= KeyEstimate;

	if (FrameTable[KeyEstimate] <= SearchFrame)
	{
		// unless we find something better, we'll default to the last key
		LowKeyIndex = LastKeyIndex;

		// search forward from the estimate for the first value greater than our search parameter
		// if found, this is the high key and we want the one just prior to it
		for (INT i=KeyEstimate+1; i<=LastKeyIndex; ++i)
		{
			if (FrameTable[i] > SearchFrame)
			{
				LowKeyIndex= i-1;
				break;
			}
		}
	}
	else
	{
		// unless we find something better, we'll default to the first key
		LowKeyIndex = 0;

		// search backward from the estimate for the first value less than or equal to the search parameter
		// if found, this is the low key we are searching for
		for (INT i=KeyEstimate-1; i>0; --i)
		{
			if (FrameTable[i] <= SearchFrame)
			{
				LowKeyIndex= i;
				break;
			}
		}
	}

	return LowKeyIndex;
}

/**
 * Utility function to determine the two key indices to interpolate given a relative position in the animation
 *
 * @param	Seq				The UAnimSequence container.
 * @param	FrameTable		The frame table containing a frame index for each key.
 * @param	RelativePos		The relative position to solve in the range [0,1] inclusive.
 * @param	bLooping		TRUE if the animation should be consider cyclic (last frame interpolates back to the start)
 * @param	NumKeys			The number of keys present in the track being solved.
 * @param	PosIndex0Out	Output value for the closest key index before the RelativePos specified.
 * @param	PosIndex1Out	Output value for the closest key index after the RelativePos specified.
 * @return	The rate at which to interpolate the two keys returned to obtain the final result.
 */
INLINE_CODE FLOAT AEFVariableKeyLerpShared::TimeToIndex(
	const UAnimSequence& Seq,
	const BYTE* FrameTable,
	FLOAT RelativePos,
	UBOOL bLooping,
	INT NumKeys,
	INT &PosIndex0Out,
	INT &PosIndex1Out)
{
	const FLOAT SequenceLength = Seq.SequenceLength;
	FLOAT Alpha = 0.0f;

	check(NumKeys != 0);
	
	const INT LastKey= NumKeys-1;
	
	INT TotalFrames = Seq.NumFrames-1;
	INT EndingKey = LastKey;
	if (bLooping)
	{
		TotalFrames = Seq.NumFrames;
		EndingKey = 0;
	}

	if (NumKeys < 2 || RelativePos <= 0.f)
	{
		// return the first key
		PosIndex0Out = 0;
		PosIndex1Out = 0;
		Alpha = 0.0f;
	}
	else if( RelativePos >= 1.0f )
	{
		// return the ending key
		PosIndex0Out = EndingKey;
		PosIndex1Out = EndingKey;
		Alpha = 0.0f;
	}
	else
	{
		// find the proper key range to return
		const INT LastFrame= TotalFrames-1;
		const FLOAT KeyPos = RelativePos * (FLOAT)LastKey;
		const FLOAT FramePos = RelativePos * (FLOAT)TotalFrames;
		const INT FramePosFloor = Clamp(appTrunc(FramePos), 0, LastFrame);
		const INT KeyEstimate = Clamp(appTrunc(KeyPos), 0, LastKey);


		INT LowFrame= 0;
		INT HighFrame= 0;
		
		// find the pair of keys which surround our target frame index
		if (Seq.NumFrames > 0xff)
		{
			const WORD* Frames= (WORD*)FrameTable;
			PosIndex0Out = FindLowKeyIndex<WORD>(Frames, NumKeys, FramePosFloor, KeyEstimate);
			LowFrame = Frames[PosIndex0Out];

			PosIndex1Out = PosIndex0Out + 1;
			if (PosIndex1Out > LastKey)
			{
				PosIndex1Out= EndingKey;
			}
			HighFrame= Frames[PosIndex1Out];
		}
		else
		{
			const BYTE* Frames= (BYTE*)FrameTable;
			PosIndex0Out = FindLowKeyIndex<BYTE>(Frames, NumKeys, FramePosFloor, KeyEstimate);
			LowFrame = Frames[PosIndex0Out];

			PosIndex1Out = PosIndex0Out + 1;
			if (PosIndex1Out > LastKey)
			{
				PosIndex1Out= EndingKey;
			}
			HighFrame= Frames[PosIndex1Out];
		}

		// compute the blend parameters for the keys we have found
		INT Delta= Max(HighFrame - LowFrame, 1);
		const FLOAT Remainder = (FramePos - (FLOAT)LowFrame);
		Alpha = Remainder / (FLOAT)Delta;
	}
	
	return Alpha;
}


/**
 * Decompress the Rotation component of a BoneAtom
 *
 * @param	OutAtom			The FBoneAtom to fill in.
 * @param	Stream			The compressed animation data.
 * @param	NumKeys			The number of keys present in Stream.
 * @param	Time			Current time to solve for.
 * @param	RelativePos		Current position within the animation to solve for in the range [0.0,1.0].
 * @param	bLooping		True when looping the stream in intended.
 * @return					None. 
 */
template<INT FORMAT>
INLINE_CODE void AEFVariableKeyLerp<FORMAT>::GetBoneAtomRotation(	
	FBoneAtom& OutAtom,
	const UAnimSequence& Seq,
	const BYTE* RESTRICT RotStream,
	INT NumRotKeys,
	FLOAT Time,
	FLOAT RelativePos,
	UBOOL bLooping)
{
	if (NumRotKeys == 1)
	{
		// For a rotation track of n=1 keys, the single key is packed as an FQuatFloat96NoW.
		DecompressRotation<ACF_Float96NoW>( OutAtom.Rotation, RotStream, RotStream );
	}
	else
	{
		const INT RotationStreamOffset= (sizeof(FLOAT)*6); // offset past Min and Range data
		const BYTE* RESTRICT FrameTable= RotStream + RotationStreamOffset +(NumRotKeys*CompressedRotationStrides[FORMAT]*CompressedRotationNum[FORMAT]);
		FrameTable= Align(FrameTable, 4);

		INT Index0;
		INT Index1;
		FLOAT Alpha = TimeToIndex(Seq,FrameTable,RelativePos,bLooping,NumRotKeys,Index0,Index1);


		if (Index0 != Index1)
		{
			// unpack and lerp between the two nearest keys
			FQuat R0;
			FQuat R1;
			const BYTE* RESTRICT KeyData0= RotStream + RotationStreamOffset +(Index0*CompressedRotationStrides[FORMAT]*CompressedRotationNum[FORMAT]);
			const BYTE* RESTRICT KeyData1= RotStream + RotationStreamOffset +(Index1*CompressedRotationStrides[FORMAT]*CompressedRotationNum[FORMAT]);
			DecompressRotation<FORMAT>( R0, RotStream, KeyData0 );
			DecompressRotation<FORMAT>( R1, RotStream, KeyData1 );

			// Fast linear quaternion interpolation.
			// To ensure the 'shortest route', we make sure the dot product between the two keys is positive.
			const FLOAT DotResult = (R0 | R1);
			const FLOAT Bias = appFloatSelect(DotResult, 1.0f, -1.0f);
			OutAtom.Rotation = (R0 * (1.f-Alpha)) + (R1 * (Alpha * Bias));
			OutAtom.Rotation.Normalize();
		}
		else // (Index0 == Index1)
		{
			// unpack a single key
			const BYTE* RESTRICT KeyData= RotStream + RotationStreamOffset +(Index0*CompressedRotationStrides[FORMAT]*CompressedRotationNum[FORMAT]);
			DecompressRotation<FORMAT>( OutAtom.Rotation, RotStream, KeyData );
		}
	}
}

/**
 * Decompress the Translation component of a BoneAtom
 *
 * @param	OutAtom			The FBoneAtom to fill in.
 * @param	Stream			The compressed animation data.
 * @param	NumKeys			The number of keys present in Stream.
 * @param	Time			Current time to solve for.
 * @param	RelativePos		Current position within the animation to solve for in the range [0.0,1.0].
 * @param	bLooping		True when looping the stream in intended.
 * @return					None. 
 */
template<INT FORMAT>
INLINE_CODE void AEFVariableKeyLerp<FORMAT>::GetBoneAtomTranslation(	
	FBoneAtom& OutAtom,
	const UAnimSequence& Seq,
	const BYTE* RESTRICT TransStream,
	INT NumTransKeys,
	FLOAT Time,
	FLOAT RelativePos,
	UBOOL bLooping)
{
	const BYTE* RESTRICT FrameTable= TransStream +(NumTransKeys*CompressedTranslationStrides[FORMAT]*CompressedTranslationNum[FORMAT]);
	FrameTable= Align(FrameTable, 4);

	INT Index0;
	INT Index1;
	FLOAT Alpha = TimeToIndex(Seq,FrameTable,RelativePos,bLooping,NumTransKeys,Index0,Index1);

	if (Index0 != Index1)
	{
		FVector P0;
		FVector P1;
		const BYTE* RESTRICT KeyData0 = TransStream + Index0*CompressedTranslationStrides[FORMAT]*CompressedTranslationNum[FORMAT];
		const BYTE* RESTRICT KeyData1 = TransStream + Index1*CompressedTranslationStrides[FORMAT]*CompressedTranslationNum[FORMAT];
		DecompressTranslation<FORMAT>( P0, TransStream, KeyData0 );
		DecompressTranslation<FORMAT>( P1, TransStream, KeyData1 );
		OutAtom.Translation = Lerp( P0, P1, Alpha );
	}
	else // (Index0 == Index1)
	{
		// unpack a single key
		const BYTE* RESTRICT KeyData = TransStream + Index0*CompressedTranslationStrides[FORMAT]*CompressedTranslationNum[FORMAT];
		DecompressTranslation<FORMAT>( OutAtom.Translation, TransStream, KeyData);
	}
}

#if USE_ANIMATION_CODEC_BATCH_SOLVER

/**
 * Decompress all requested rotation components from an Animation Sequence
 *
 * @param	Atoms			The FBoneAtom array to fill in.
 * @param	DesiredPairs	Array of requested bone information
 * @param	Seq				The animation sequence to use.
 * @param	Time			Current time to solve for.
 * @param	bLooping		True when looping the stream in intended.
 * @return					None. 
 */
template<INT FORMAT>
INLINE_CODE void AEFVariableKeyLerp<FORMAT>::GetPoseRotations(	
	FBoneAtomArray& Atoms, 
	const BoneTrackArray& DesiredPairs,
	const UAnimSequence& Seq,
	FLOAT Time,
	UBOOL bLooping)
{
	const INT PairCount = DesiredPairs.Num();
	const FLOAT RelativePos = Time / (FLOAT)Seq.SequenceLength;

	for (INT PairIndex=0; PairIndex<PairCount; ++PairIndex)
	{
		const BoneTrackPair& Pair = DesiredPairs(PairIndex);
		const INT TrackIndex = Pair.TrackIndex;
		const INT AtomIndex = Pair.AtomIndex;
		FBoneAtom& BoneAtom = Atoms(AtomIndex);

		const INT* RESTRICT TrackData = Seq.CompressedTrackOffsets.GetTypedData() + (TrackIndex*4);
		const INT RotKeysOffset	= *(TrackData+2);
		const INT NumRotKeys	= *(TrackData+3);
		const BYTE* RESTRICT RotStream		= Seq.CompressedByteStream.GetTypedData()+RotKeysOffset;

		// call the decoder directly (not through the vtable)
		AEFVariableKeyLerp<FORMAT>::GetBoneAtomRotation(BoneAtom, Seq, RotStream, NumRotKeys, Time, RelativePos, bLooping);

		// Apply quaternion fix for ActorX-exported quaternions.
		BoneAtom.Rotation.W *= -1.0f;
	}
}

/**
 * Decompress all requested translation components from an Animation Sequence
 *
 * @param	Atoms			The FBoneAtom array to fill in.
 * @param	DesiredPairs	Array of requested bone information
 * @param	Seq				The animation sequence to use.
 * @param	Time			Current time to solve for.
 * @param	bLooping		True when looping the stream in intended.
 * @return					None. 
 */
template<INT FORMAT>
INLINE_CODE void AEFVariableKeyLerp<FORMAT>::GetPoseTranslations(	
	FBoneAtomArray& Atoms, 
	const BoneTrackArray& DesiredPairs,
	const UAnimSequence& Seq,
	FLOAT Time,
	UBOOL bLooping)
{
	const INT PairCount= DesiredPairs.Num();
	const FLOAT RelativePos = Time / (FLOAT)Seq.SequenceLength;

	for (INT PairIndex=0; PairIndex<PairCount; ++PairIndex)
	{
		const BoneTrackPair& Pair = DesiredPairs(PairIndex);
		const INT TrackIndex = Pair.TrackIndex;
		const INT AtomIndex = Pair.AtomIndex;
		FBoneAtom& BoneAtom = Atoms(AtomIndex);

		const INT* RESTRICT TrackData = Seq.CompressedTrackOffsets.GetTypedData() + (TrackIndex*4);
		const INT TransKeysOffset	= *(TrackData+0);
		const INT NumTransKeys		= *(TrackData+1);
		const BYTE* RESTRICT TransStream = Seq.CompressedByteStream.GetTypedData()+TransKeysOffset;

		// call the decoder directly (not through the vtable)
		AEFVariableKeyLerp<FORMAT>::GetBoneAtomTranslation(BoneAtom, Seq, TransStream, NumTransKeys, Time, RelativePos, bLooping);
	}
}
#endif

#endif

#endif // __ANIMATIONENCODINGFORMAT_H__
