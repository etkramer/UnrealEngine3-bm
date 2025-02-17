/*=============================================================================
	AnimationEncodingFormat.cpp: Skeletal mesh animation functions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "AnimationCompression.h"
#include "AnimationEncodingFormat.h"
#include "AnimationUtils.h"

// known codecs
#include "AnimationEncodingFormat_ConstantKeyLerp.h"
#include "AnimationEncodingFormat_VariableKeyLerp.h"

//IMPLEMENT_CLASS(AnimationEncodingFormat);

#if USE_ANIMATION_CODEC_INTERFACE

/** Each CompresedTranslationData track's ByteStream will be byte swapped in chunks of this size. */
const INT CompressedTranslationStrides[ACF_MAX] =
{
	sizeof(FLOAT),	// ACF_None					(FVectors are serialized per element hence sizeof(FLOAT) rather than sizeof(FVector).
	sizeof(FLOAT),	// ACF_Float96NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	sizeof(FLOAT),	// ACF_Fixed48NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	sizeof(FLOAT),	// ACF_IntervalFixed32NoW	(Translation data currently uncompressed, hence same size as ACF_None).
	sizeof(FLOAT),	// ACF_Fixed32NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	sizeof(FLOAT),	// ACF_Float32NoW			(Translation data currently uncompressed, hence same size as ACF_None).
};

/** Number of swapped chunks per element. */
const INT CompressedTranslationNum[ACF_MAX] =
{
	3,	// ACF_None					(FVectors are serialized per element hence sizeof(FLOAT) rather than sizeof(FVector).
	3,	// ACF_Float96NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	3,	// ACF_Fixed48NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	3,	// ACF_IntervalFixed32NoW	(Translation data currently uncompressed, hence same size as ACF_None).
	3,	// ACF_Fixed32NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	3,	// ACF_Float32NoW			(Translation data currently uncompressed, hence same size as ACF_None).
};

/** Each CompresedRotationData track's ByteStream will be byte swapped in chunks of this size. */
const INT CompressedRotationStrides[ACF_MAX] =
{
	sizeof(FLOAT),						// ACF_None					(FQuats are serialized per element hence sizeof(FLOAT) rather than sizeof(FQuat).
	sizeof(FLOAT),						// ACF_Float96NoW			(FQuats with one component dropped and the remaining three uncompressed at 32bit floating point each
	sizeof(WORD),						// ACF_Fixed48NoW			(FQuats with one component dropped and the remaining three compressed to 16-16-16 fixed point.
	sizeof(FQuatIntervalFixed32NoW),	// ACF_IntervalFixed32NoW	(FQuats with one component dropped and the remaining three compressed to 11-11-10 per-component interval fixed point.
	sizeof(FQuatFixed32NoW),			// ACF_Fixed32NoW			(FQuats with one component dropped and the remaining three compressed to 11-11-10 fixed point.
	sizeof(FQuatFloat32NoW),			// ACF_Float32NoW			(FQuats with one component dropped and the remaining three compressed to 11-11-10 floating point.
};

/** Number of swapped chunks per element. */
const INT CompressedRotationNum[ACF_MAX] =
{
	4,	// ACF_None					(FQuats are serialized per element hence sizeof(FLOAT) rather than sizeof(FQuat).
	3,	// ACF_Float96NoW			(FQuats with one component dropped and the remaining three uncompressed at 32bit floating point each
	3,	// ACF_Fixed48NoW			(FQuats with one component dropped and the remaining three compressed to 16-16-16 fixed point.
	1,	// ACF_IntervalFixed32NoW	(FQuats with one component dropped and the remaining three compressed to 11-11-10 per-component interval fixed point.
	1,	// ACF_Fixed32NoW			(FQuats with one component dropped and the remaining three compressed to 11-11-10 fixed point.
	1,  // ACF_Float32NoW			(FQuats with one component dropped and the remaining three compressed to 11-11-10 floating point.
};

/**
 * Compressed translation data will be byte swapped in chunks of this size.
 */
inline INT GetCompressedTranslationStride(AnimationCompressionFormat TranslationCompressionFormat)
{
	return CompressedTranslationStrides[TranslationCompressionFormat];
}

/**
 * Compressed rotation data will be byte swapped in chunks of this size.
 */
inline INT GetCompressedRotationStride(AnimationCompressionFormat RotationCompressionFormat)
{
	return CompressedRotationStrides[RotationCompressionFormat];
}


/**
 * Compressed translation data will be byte swapped in chunks of this size.
 */
inline INT GetCompressedTranslationStride(const UAnimSequence* Seq)
{
	return CompressedTranslationStrides[static_cast<AnimationCompressionFormat>(Seq->TranslationCompressionFormat)];
}

/**
 * Compressed rotation data will be byte swapped in chunks of this size.
 */
inline INT GetCompressedRotationStride(const UAnimSequence* Seq)
{
	return CompressedRotationStrides[static_cast<AnimationCompressionFormat>(Seq->RotationCompressionFormat)];
}

/**
 * Pads a specified number of bytes to the memory writer to maintain alignment
 */
void PadMemoryWriter(FMemoryWriter* MemoryWriter, BYTE*& TrackData, const INT Alignment)
{
	const PTRINT ByteStreamLoc = (PTRINT) TrackData;
	const INT Pad = static_cast<INT>( Align( ByteStreamLoc, Alignment ) - ByteStreamLoc );
	const BYTE PadSentinel = 85; // (1<<1)+(1<<3)+(1<<5)+(1<<7)
	
	for ( INT PadByteIndex = 0; PadByteIndex < Pad; ++PadByteIndex )
	{
		MemoryWriter->Serialize( (void*)&PadSentinel, sizeof(BYTE) );
	}
	TrackData += Pad;
}

/**
 * Skips a specified number of bytes in the memory reader to maintain alignment
 */
void PadMemoryReader(FMemoryReader* MemoryReader, BYTE*& TrackData, const INT Alignment)
{
	const PTRINT ByteStreamLoc = (PTRINT) TrackData;
	const INT Pad = static_cast<INT>( Align( ByteStreamLoc, Alignment ) - ByteStreamLoc );
	MemoryReader->Serialize( TrackData, Pad );
	TrackData += Pad;
}

/**
 * Extracts a single BoneAtom from an Animation Sequence.
 *
 * @param	OutAtom			The BoneAtom to fill with the extracted result.
 * @param	Seq				An Animation Sequence to extract the BoneAtom from.
 * @param	TrackIndex		The index of the track desired in the Animation Sequence.
 * @param	Time			The time (in seconds) to calculate the BoneAtom for.
 * @param	bLooping		TRUE if the animation should be played in a cyclic manner.
 */
void AnimationFormat_GetBoneAtom(	
	FBoneAtom& OutAtom,
	const UAnimSequence& Seq,
	INT TrackIndex,
	FLOAT Time,
	UBOOL bLooping)
{
	// scale is not animated
	OutAtom.Scale = 1.f;

	// Use the CompressedTrackOffsets stream to find the data addresses
	const INT* RESTRICT TrackData= Seq.CompressedTrackOffsets.GetTypedData() + (TrackIndex*4);
	INT TransKeysOffset = *(TrackData+0);
	INT NumTransKeys	= *(TrackData+1);
	INT RotKeysOffset	= *(TrackData+2);
	INT NumRotKeys		= *(TrackData+3);
	const BYTE* RESTRICT TransStream	= Seq.CompressedByteStream.GetTypedData()+TransKeysOffset;
	const BYTE* RESTRICT RotStream		= Seq.CompressedByteStream.GetTypedData()+RotKeysOffset;

	const FLOAT RelativePos = Time / (FLOAT)Seq.SequenceLength;

	// decompress the translation component using the proper method
	if (Seq.TranslationCodec != NULL)
	{
		((AnimationEncodingFormat*)Seq.TranslationCodec)->GetBoneAtomTranslation(OutAtom, Seq, TransStream, NumTransKeys, Time, RelativePos, bLooping);
	}
	else
	{
		appErrorf( TEXT("%i: unknown or unsupported animation format"), (INT)Seq.KeyEncodingFormat );
		// Silence compilers warning about a value potentially not being assigned.
		OutAtom.Translation = FVector(0.f,0.f,0.f);
	};

	// decompress the rotation component using the proper method
	if (Seq.RotationCodec != NULL)
	{
		((AnimationEncodingFormat*)Seq.RotationCodec)->GetBoneAtomRotation(OutAtom, Seq, RotStream, NumRotKeys, Time, RelativePos, bLooping);
	}
	else
	{
		appErrorf( TEXT("%i: unknown or unsupported animation format"), (INT)Seq.KeyEncodingFormat );
		// Silence compilers warning about a value potentially not being assigned.
		OutAtom.Rotation= FQuat::Identity;
	};

}

#if USE_ANIMATION_CODEC_BATCH_SOLVER

/**
 * Extracts an array of BoneAtoms from an Animation Sequence representing an entire pose of the skeleton.
 *
 * @param	Atoms				The BoneAtoms to fill with the extracted result.
 * @param	RotationTracks		A BoneTrackArray element for each bone requesting rotation data. 
 * @param	TranslationTracks	A BoneTrackArray element for each bone requesting translation data. 
 * @param	Seq					An Animation Sequence to extract the BoneAtom from.
 * @param	Time				The time (in seconds) to calculate the BoneAtom for.
 * @param	bLooping			TRUE if the animation should be played in a cyclic manner.
 */
void AnimationFormat_GetAnimationPose(	
	FBoneAtomArray& Atoms, 
	const BoneTrackArray& RotationPairs,
	const BoneTrackArray& TranslationPairs,
	const UAnimSequence& Seq,
	FLOAT Time,
	UBOOL bLooping)
{
	// decompress the translation component using the proper method
	if (Seq.TranslationCodec != NULL)
	{
		((AnimationEncodingFormat*)Seq.TranslationCodec)->GetPoseTranslations(Atoms, TranslationPairs, Seq, Time, bLooping);
	}
	else
	{
		appErrorf( TEXT("%i: unknown or unsupported animation format"), (INT)Seq.KeyEncodingFormat );
	};

	// decompress the rotation component using the proper method
	if (Seq.RotationCodec != NULL)
	{
		((AnimationEncodingFormat*)Seq.RotationCodec)->GetPoseRotations(Atoms, RotationPairs, Seq, Time, bLooping);
	}
	else
	{
		appErrorf( TEXT("%i: unknown or unsupported animation format"), (INT)Seq.KeyEncodingFormat );
	};
}
#endif


/**
 * Handles Byte-swapping incomming animation data from a MemoryReader
 *
 * @param	Seq					An Animation Sequence to contain the read data.
 * @param	MemoryReader		The MemoryReader object to read from.
 */
void AnimationFormat_ByteSwapIn(
	UAnimSequence& Seq, 
	FMemoryReader& MemoryReader)
{
	BYTE* StreamBase		= Seq.CompressedByteStream.GetTypedData();
	const INT NumTracks		= Seq.CompressedTrackOffsets.Num()/4;

	for ( INT TrackIndex = 0; TrackIndex < NumTracks; ++TrackIndex )
	{
		const INT OffsetTrans	= Seq.CompressedTrackOffsets(TrackIndex*4);
		const INT NumKeysTrans	= Seq.CompressedTrackOffsets(TrackIndex*4+1);
		const INT OffsetRot		= Seq.CompressedTrackOffsets(TrackIndex*4+2);
		const INT NumKeysRot	= Seq.CompressedTrackOffsets(TrackIndex*4+3);

		// Translation data.
		checkSlow( (OffsetTrans % 4) == 0 && "CompressedByteStream not aligned to four bytes" );
		BYTE* TransTrackData = StreamBase + OffsetTrans;
		if (Seq.TranslationCodec != NULL)
		{
			((AnimationEncodingFormat*)Seq.TranslationCodec)->ByteSwapTranslationIn(Seq, MemoryReader, TransTrackData, NumKeysTrans);
		}
		else
		{
			appErrorf( TEXT("%i: unknown or unsupported animation format"), (INT)Seq.KeyEncodingFormat );
		};

		// Like the compressed byte stream, pad the serialization stream to four bytes.
		// As a sanity check, each pad byte can be checked to be the PadSentinel.
		PadMemoryReader(&MemoryReader, TransTrackData, 4); 

		// Rotation data.
		checkSlow( (OffsetRot % 4) == 0 && "CompressedByteStream not aligned to four bytes" );
		BYTE* RotTrackData = StreamBase + OffsetRot;
		if (Seq.RotationCodec != NULL)
		{
			((AnimationEncodingFormat*)Seq.RotationCodec)->ByteSwapRotationIn(Seq, MemoryReader, RotTrackData, NumKeysRot);
		}
		else
		{
			appErrorf( TEXT("%i: unknown or unsupported animation format"), (INT)Seq.KeyEncodingFormat );
		};

		// Like the compressed byte stream, pad the serialization stream to four bytes.
		// As a sanity check, each pad byte can be checked to be the PadSentinel.
		PadMemoryReader(&MemoryReader, RotTrackData, 4); 
	}
}

/**
 * Handles Byte-swapping outgoing animation data to an array of BYTEs
 *
 * @param	Seq					An Animation Sequence to write.
 * @param	SerializedData		The output buffer.
 * @param	ForceByteSwapping	TRUE is byte swapping is not optional.
 */
void AnimationFormat_ByteSwapOut(
	UAnimSequence& Seq, TArray<BYTE>& SerializedData, 
	UBOOL ForceByteSwapping)
{
	FMemoryWriter MemoryWriter( SerializedData, TRUE );
	MemoryWriter.SetByteSwapping( ForceByteSwapping );
	
	BYTE* StreamBase		= Seq.CompressedByteStream.GetTypedData();
	const INT NumTracks		= Seq.CompressedTrackOffsets.Num()/4;

	for ( INT TrackIndex = 0; TrackIndex < NumTracks; ++TrackIndex )
	{
		const INT OffsetTrans	= Seq.CompressedTrackOffsets(TrackIndex*4);
		const INT NumKeysTrans	= Seq.CompressedTrackOffsets(TrackIndex*4+1);
		const INT OffsetRot		= Seq.CompressedTrackOffsets(TrackIndex*4+2);
		const INT NumKeysRot	= Seq.CompressedTrackOffsets(TrackIndex*4+3);

		// Translation data.
		checkSlow( (OffsetTrans % 4) == 0 && "CompressedByteStream not aligned to four bytes" );
		BYTE* TransTrackData = StreamBase + OffsetTrans;
		if (Seq.TranslationCodec != NULL)
		{
			((AnimationEncodingFormat*)Seq.TranslationCodec)->ByteSwapTranslationOut(Seq, MemoryWriter, TransTrackData, NumKeysTrans);
		}
		else
		{
			appErrorf( TEXT("%i: unknown or unsupported animation format"), (INT)Seq.KeyEncodingFormat );
		};

		// Like the compressed byte stream, pad the serialization stream to four bytes.
		PadMemoryWriter(&MemoryWriter, TransTrackData, 4);

		// Rotation data.
		checkSlow( (OffsetRot % 4) == 0 && "CompressedByteStream not aligned to four bytes" );
		BYTE* RotTrackData = StreamBase + OffsetRot;
		if (Seq.RotationCodec != NULL)
		{
			((AnimationEncodingFormat*)Seq.RotationCodec)->ByteSwapRotationOut(Seq, MemoryWriter, RotTrackData, NumKeysRot);
		}
		else
		{
			appErrorf( TEXT("%i: unknown or unsupported animation format"), (INT)Seq.KeyEncodingFormat );
		};

		// Like the compressed byte stream, pad the serialization stream to four bytes.
		PadMemoryWriter(&MemoryWriter, RotTrackData, 4);
	}
}


/**
 * Extracts statistics about a given Animation Sequence
 *
 * @param	Seq					An Animation Sequence.
 * @param	NumTransTracks		The total number of Translation Tracks found.
 * @param	NumRotTracks		The total number of Rotation Tracks found.
 * @param	TotalNumTransKeys	The total number of Translation Keys found.
 * @param	TotalNumRotKeys		The total number of Rotation Keys found.
 * @param	TranslationKeySize	The size (in BYTES) of a single Translation Key.
 * @param	RotationKeySize		The size (in BYTES) of a single Rotation Key.
 * @param	NumTransTracksWithOneKey	The total number of Translation Tracks found containing a single key.
 * @param	NumRotTracksWithOneKey		The total number of Rotation Tracks found containing a single key.
*/
void AnimationFormat_GetStats(	
	const UAnimSequence* Seq, 
	INT& NumTransTracks,
	INT& NumRotTracks,
	INT& TotalNumTransKeys,
	INT& TotalNumRotKeys,
	INT& TranslationKeySize,
	INT& RotationKeySize,
	INT& NumTransTracksWithOneKey,
	INT& NumRotTracksWithOneKey)
{
	if (Seq)
	{
		const INT TransStride	= GetCompressedTranslationStride(Seq);
		const INT RotStride		= GetCompressedRotationStride(Seq);
		const INT TransNum		= CompressedTranslationNum[Seq->TranslationCompressionFormat];
		const INT RotNum		= CompressedRotationNum[Seq->RotationCompressionFormat];

		TranslationKeySize= TransStride * TransNum;
		RotationKeySize= RotStride * RotNum;

		// Track number of tracks.
		NumTransTracks	= Seq->CompressedTrackOffsets.Num()/4;
		NumRotTracks	= Seq->CompressedTrackOffsets.Num()/4;

		// Track total number of keys.
		TotalNumTransKeys = 0;
		TotalNumRotKeys = 0;

		// Track number of tracks with a single key.
		NumTransTracksWithOneKey = 0;
		NumRotTracksWithOneKey = 0;

		// Translation.
		for ( INT TrackIndex = 0; TrackIndex < NumTransTracks; ++TrackIndex )
		{
			const INT NumTransKeys = Seq->CompressedTrackOffsets(TrackIndex*4+1);
			TotalNumTransKeys += NumTransKeys;
			if ( NumTransKeys == 1 )
			{
				++NumTransTracksWithOneKey;
			}
		}

		// Rotation.
		for ( INT TrackIndex = 0; TrackIndex < NumRotTracks; ++TrackIndex )
		{
			const INT NumRotKeys = Seq->CompressedTrackOffsets(TrackIndex*4+3);
			TotalNumRotKeys += NumRotKeys;
			if ( NumRotKeys == 1 )
			{
				++NumRotTracksWithOneKey;
			}
		}
	}
}

/**
 * Sets the internal Animation Codec Interface Links within an Animation Sequence
 *
 * @param	Seq					An Animation Sequence to setup links within.
*/
void AnimationFormat_SetInterfaceLinks(UAnimSequence& Seq)
{
	Seq.TranslationCodec = NULL;
	Seq.RotationCodec = NULL;

	if (Seq.KeyEncodingFormat == AKF_ConstantKeyLerp)
	{
		static AEFConstantKeyLerp<ACF_None>					AEFConstantKeyLerp_None;
		static AEFConstantKeyLerp<ACF_Float96NoW>			AEFConstantKeyLerp_Float96NoW;
		static AEFConstantKeyLerp<ACF_Fixed48NoW>			AEFConstantKeyLerp_Fixed48NoW;
		static AEFConstantKeyLerp<ACF_IntervalFixed32NoW>	AEFConstantKeyLerp_IntervalFixed32NoW;
		static AEFConstantKeyLerp<ACF_Fixed32NoW>			AEFConstantKeyLerp_Fixed32NoW;
		static AEFConstantKeyLerp<ACF_Float32NoW>			AEFConstantKeyLerp_Float32NoW;

		// setup translation codec
		switch(Seq.TranslationCompressionFormat)
		{
			case ACF_None:
				Seq.TranslationCodec = &AEFConstantKeyLerp_None;
				break;

			default:
				appErrorf( TEXT("%i: unknown or unsupported translation compression"), (INT)Seq.RotationCompressionFormat );
		};

		// setup rotation codec
		switch(Seq.RotationCompressionFormat)
		{
			case ACF_None:
				Seq.RotationCodec = &AEFConstantKeyLerp_None;
				break;
			case ACF_Float96NoW:
				Seq.RotationCodec = &AEFConstantKeyLerp_Float96NoW;
				break;
			case ACF_Fixed48NoW:
				Seq.RotationCodec = &AEFConstantKeyLerp_Fixed48NoW;
				break;
			case ACF_IntervalFixed32NoW:
				Seq.RotationCodec = &AEFConstantKeyLerp_IntervalFixed32NoW;
				break;
			case ACF_Fixed32NoW:
				Seq.RotationCodec = &AEFConstantKeyLerp_Fixed32NoW;
				break;
			case ACF_Float32NoW:
				Seq.RotationCodec = &AEFConstantKeyLerp_Float32NoW;
				break;

			default:
				appErrorf( TEXT("%i: unknown or unsupported rotation compression"), (INT)Seq.RotationCompressionFormat );
		};
	}
	else if (Seq.KeyEncodingFormat == AKF_VariableKeyLerp)
	{
		static AEFVariableKeyLerp<ACF_None>					AEFVariableKeyLerp_None;
		static AEFVariableKeyLerp<ACF_Float96NoW>			AEFVariableKeyLerp_Float96NoW;
		static AEFVariableKeyLerp<ACF_Fixed48NoW>			AEFVariableKeyLerp_Fixed48NoW;
		static AEFVariableKeyLerp<ACF_IntervalFixed32NoW>	AEFVariableKeyLerp_IntervalFixed32NoW;
		static AEFVariableKeyLerp<ACF_Fixed32NoW>			AEFVariableKeyLerp_Fixed32NoW;
		static AEFVariableKeyLerp<ACF_Float32NoW>			AEFVariableKeyLerp_Float32NoW;

		// setup translation codec
		switch(Seq.TranslationCompressionFormat)
		{
			case ACF_None:
				Seq.TranslationCodec = &AEFVariableKeyLerp_None;
				break;

			default:
				appErrorf( TEXT("%i: unknown or unsupported translation compression"), (INT)Seq.RotationCompressionFormat );
		};

		// setup rotation codec
		switch(Seq.RotationCompressionFormat)
		{
			case ACF_None:
				Seq.RotationCodec = &AEFVariableKeyLerp_None;
				break;
			case ACF_Float96NoW:
				Seq.RotationCodec = &AEFVariableKeyLerp_Float96NoW;
				break;
			case ACF_Fixed48NoW:
				Seq.RotationCodec = &AEFVariableKeyLerp_Fixed48NoW;
				break;
			case ACF_IntervalFixed32NoW:
				Seq.RotationCodec = &AEFVariableKeyLerp_IntervalFixed32NoW;
				break;
			case ACF_Fixed32NoW:
				Seq.RotationCodec = &AEFVariableKeyLerp_Fixed32NoW;
				break;
			case ACF_Float32NoW:
				Seq.RotationCodec = &AEFVariableKeyLerp_Float32NoW;
				break;

			default:
				appErrorf( TEXT("%i: unknown or unsupported rotation compression"), (INT)Seq.RotationCompressionFormat );
		};
	}
	else
	{
		appErrorf( TEXT("%i: unknown or unsupported animation format"), (INT)Seq.KeyEncodingFormat );
	}
}

#endif //#if USE_ANIMATION_CODEC_INTERFACE
