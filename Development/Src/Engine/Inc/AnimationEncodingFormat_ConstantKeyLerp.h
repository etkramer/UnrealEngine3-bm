/*=============================================================================
	AnimationEncodingFormat.h: Skeletal mesh animation compression.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 


#ifndef __ANIMATIONENCODINGFORMAT_CONSTANTKEYLERP_H__
#define __ANIMATIONENCODINGFORMAT_CONSTANTKEYLERP_H__

#include "AnimationEncodingFormat.h"

#if USE_ANIMATION_CODEC_INTERFACE

/**
 * Base class for all Animation Encoding Formats using consistantly-spaced key interpolation.
 */
class AEFConstantKeyLerpShared : public AnimationEncodingFormat
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
	 * @param	RelativePos		The relative position to solve in the range [0,1] inclusive.
	 * @param	bLooping		TRUE if the animation should be consider cyclic (last frame interpolates back to the start)
	 * @param	NumKeys			The number of keys present in the track being solved.
	 * @param	PosIndex0Out	Output value for the closest key index before the RelativePos specified.
	 * @param	PosIndex1Out	Output value for the closest key index after the RelativePos specified.
	 * @return	The rate at which to interpolate the two keys returned to obtain the final result.
	 */
	FLOAT TimeToIndex(
		const UAnimSequence& Seq,
		FLOAT RelativePos,
		UBOOL bLooping,
		INT NumKeys,
		INT &PosIndex0Out,
		INT &PosIndex1Out);
	
};

template<INT FORMAT>
class AEFConstantKeyLerp : public AEFConstantKeyLerpShared
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
 * Utility function to determine the two key indices to interpolate given a relative position in the animation
 *
 * @param	Seq				The UAnimSequence container.
 * @param	RelativePos		The relative position to solve in the range [0,1] inclusive.
 * @param	bLooping		TRUE if the animation should be consider cyclic (last frame interpolates back to the start)
 * @param	NumKeys			The number of keys present in the track being solved.
 * @param	PosIndex0Out	Output value for the closest key index before the RelativePos specified.
 * @param	PosIndex1Out	Output value for the closest key index after the RelativePos specified.
 * @return	The rate at which to interpolate the two keys returned to obtain the final result.
 */
#ifdef _DEBUG
inline
#else
FORCEINLINE 
#endif
FLOAT AEFConstantKeyLerpShared::TimeToIndex(
	const UAnimSequence& Seq,
	FLOAT RelativePos,
	UBOOL bLooping,
	INT NumKeys,
	INT &PosIndex0Out,
	INT &PosIndex1Out)
{
	static INT		NumKeysCache = 0; // this value is guaranteed to not be used for valid data
	static FLOAT	TimeCache;
	static FLOAT	SequenceLengthCache;
	static UBOOL	LoopingCache;
	static INT		PosIndex0CacheOut; 
	static INT		PosIndex1CacheOut; 
	static FLOAT	AlphaCacheOut;

	const FLOAT SequenceLength= Seq.SequenceLength;

	if (NumKeys < 2)
	{
		checkSlow(NumKeys == 1); // check if data is empty for some reason.
		PosIndex0Out = 0;
		PosIndex1Out = 0;
		return 0.0f;
	}
	if (
		NumKeysCache		!= NumKeys ||
		LoopingCache		!= bLooping ||
		SequenceLengthCache != SequenceLength ||
		TimeCache			!= RelativePos
		)
	{
		NumKeysCache		= NumKeys;
		LoopingCache		= bLooping;
		SequenceLengthCache = SequenceLength;
		TimeCache			= RelativePos;
		// Check for before-first-frame case.
		if( RelativePos <= 0.f )
		{
			PosIndex0CacheOut = 0;
			PosIndex1CacheOut = 0;
			AlphaCacheOut = 0.0f;
		}
		else
		{
			if (!bLooping)
			{
				NumKeys -= 1; // never used without the minus one in this case
				// Check for after-last-frame case.
				if( RelativePos >= 1.0f )
				{
					// If we're not looping, key n-1 is the final key.
					PosIndex0CacheOut = NumKeys;
					PosIndex1CacheOut = NumKeys;
					AlphaCacheOut = 0.0f;
				}
				else
				{
					// For non-looping animation, the last frame is the ending frame, and has no duration.
					const FLOAT KeyPos = RelativePos * FLOAT(NumKeys);
					checkSlow(KeyPos >= 0.0f);
					const FLOAT KeyPosFloor = floorf(KeyPos);
					PosIndex0CacheOut = Min( appTrunc(KeyPosFloor), NumKeys );
					AlphaCacheOut = KeyPos - KeyPosFloor;
					PosIndex1CacheOut = Min( PosIndex0CacheOut + 1, NumKeys );
				}
			}
			else // we are looping
			{
				// Check for after-last-frame case.
				if( RelativePos >= 1.0f )
				{
					// If we're looping, key 0 is the final key.
					PosIndex0CacheOut = 0;
					PosIndex1CacheOut = 0;
					AlphaCacheOut = 0.0f;
				}
				else
				{
					// For looping animation, the last frame has duration, and interpolates back to the first one.
					const FLOAT KeyPos = RelativePos * FLOAT(NumKeys);
					checkSlow(KeyPos >= 0.0f);
					const FLOAT KeyPosFloor = floorf(KeyPos);
					PosIndex0CacheOut = Min( appTrunc(KeyPosFloor), NumKeys - 1 );
					AlphaCacheOut = KeyPos - KeyPosFloor;
					PosIndex1CacheOut = PosIndex0CacheOut + 1;
					if ( PosIndex1CacheOut == NumKeys )
					{
						PosIndex1CacheOut = 0;
					}
				}
			}
		}
	}
	PosIndex0Out = PosIndex0CacheOut;
	PosIndex1Out = PosIndex1CacheOut;
	return AlphaCacheOut;
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
FORCEINLINE void AEFConstantKeyLerp<FORMAT>::GetBoneAtomRotation(	
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
		INT Index0;
		INT Index1;
		FLOAT Alpha = TimeToIndex(Seq,RelativePos,bLooping,NumRotKeys,Index0,Index1);

		const INT RotationStreamOffset= (sizeof(FLOAT)*6); // offset past Min and Range data

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
FORCEINLINE void AEFConstantKeyLerp<FORMAT>::GetBoneAtomTranslation(	
	FBoneAtom& OutAtom,
	const UAnimSequence& Seq,
	const BYTE* RESTRICT TransStream,
	INT NumTransKeys,
	FLOAT Time,
	FLOAT RelativePos,
	UBOOL bLooping)
{
	INT Index0;
	INT Index1;
	FLOAT Alpha = TimeToIndex(Seq,RelativePos,bLooping,NumTransKeys,Index0,Index1);

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
inline void AEFConstantKeyLerp<FORMAT>::GetPoseRotations(	
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
		AEFConstantKeyLerp<FORMAT>::GetBoneAtomRotation(BoneAtom, Seq, RotStream, NumRotKeys, Time, RelativePos, bLooping);

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
inline void AEFConstantKeyLerp<FORMAT>::GetPoseTranslations(	
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
		AEFConstantKeyLerp<FORMAT>::GetBoneAtomTranslation(BoneAtom, Seq, TransStream, NumTransKeys, Time, RelativePos, bLooping);
	}
}
#endif

#endif

#endif // __ANIMATIONENCODINGFORMAT_H__
