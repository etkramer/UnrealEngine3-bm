/*=============================================================================
	UnSkeletalAnim.cpp: Skeletal mesh animation functions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "AnimationCompression.h"
#include "AnimationEncodingFormat.h"
#include "AnimationUtils.h"

// Priority with which to display sounds triggered by sound notifies.
#define SUBTITLE_PRIORITY_ANIMNOTIFY	10000

IMPLEMENT_CLASS(UAnimSequence);
IMPLEMENT_CLASS(UAnimSet);
IMPLEMENT_CLASS(UAnimNotify);

#define USE_SLERP 0

#if !USE_ANIMATION_CODEC_INTERFACE

/** Each CompresedTranslationData track's ByteStream will be byte swapped in chunks of this size. */
static const INT CompressedTranslationStrides[ACF_MAX] =
{
	sizeof(FLOAT),	// ACF_None					(FVectors are serialized per element hence sizeof(FLOAT) rather than sizeof(FVector).
	sizeof(FLOAT),	// ACF_Float96NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	sizeof(FLOAT),	// ACF_Fixed48NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	sizeof(FLOAT),	// ACF_IntervalFixed32NoW	(Translation data currently uncompressed, hence same size as ACF_None).
	sizeof(FLOAT),	// ACF_Fixed32NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	sizeof(FLOAT),	// ACF_Float32NoW			(Translation data currently uncompressed, hence same size as ACF_None).
};

/** Number of swapped chunks per element. */
static const INT CompressedTranslationNum[ACF_MAX] =
{
	3,	// ACF_None					(FVectors are serialized per element hence sizeof(FLOAT) rather than sizeof(FVector).
	3,	// ACF_Float96NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	3,	// ACF_Fixed48NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	3,	// ACF_IntervalFixed32NoW	(Translation data currently uncompressed, hence same size as ACF_None).
	3,	// ACF_Fixed32NoW			(Translation data currently uncompressed, hence same size as ACF_None).
	3,	// ACF_Float32NoW			(Translation data currently uncompressed, hence same size as ACF_None).
};

/** Each CompresedRotationData track's ByteStream will be byte swapped in chunks of this size. */
static const INT CompressedRotationStrides[ACF_MAX] =
{
	sizeof(FLOAT),						// ACF_None					(FQuats are serialized per element hence sizeof(FLOAT) rather than sizeof(FQuat).
	sizeof(FLOAT),						// ACF_Float96NoW			(FQuats with one component dropped and the remaining three uncompressed at 32bit floating point each
	sizeof(WORD),						// ACF_Fixed48NoW			(FQuats with one component dropped and the remaining three compressed to 16-16-16 fixed point.
	sizeof(FQuatIntervalFixed32NoW),	// ACF_IntervalFixed32NoW	(FQuats with one component dropped and the remaining three compressed to 11-11-10 per-component interval fixed point.
	sizeof(FQuatFixed32NoW),			// ACF_Fixed32NoW			(FQuats with one component dropped and the remaining three compressed to 11-11-10 fixed point.
	sizeof(FQuatFloat32NoW),			// ACF_Float32NoW			(FQuats with one component dropped and the remaining three compressed to 11-11-10 floating point.
};

/** Number of swapped chunks per element. */
static const INT CompressedRotationNum[ACF_MAX] =
{
	4,	// ACF_None					(FQuats are serialized per element hence sizeof(FLOAT) rather than sizeof(FQuat).
	3,	// ACF_Float96NoW			(FQuats with one component dropped and the remaining three uncompressed at 32bit floating point each
	3,	// ACF_Fixed48NoW			(FQuats with one component dropped and the remaining three compressed to 16-16-16 fixed point.
	1,	// ACF_IntervalFixed32NoW	(FQuats with one component dropped and the remaining three compressed to 11-11-10 per-component interval fixed point.
	1,	// ACF_Fixed32NoW			(FQuats with one component dropped and the remaining three compressed to 11-11-10 fixed point.
	1,  // ACF_Float32NoW			(FQuats with one component dropped and the remaining three compressed to 11-11-10 floating point.
};

/*-----------------------------------------------------------------------------
	UAnimSequence
-----------------------------------------------------------------------------*/

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

#endif //#if !USE_ANIMATION_CODEC_INTERFACE

//@deprecated with VER_REPLACED_LAZY_ARRAY_WITH_UNTYPED_BULK_DATA
struct FRawAnimSequenceTrackNativeDeprecated
{
    TArray<FVector> PosKeys;
    TArray<FQuat>	RotKeys;
    TArray<FLOAT>	KeyTimes;
	friend FArchive& operator<<(FArchive& Ar, FRawAnimSequenceTrackNativeDeprecated& T)
	{
		return	Ar << T.PosKeys << T.RotKeys << T.KeyTimes;
	}
};

/**
 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
 *
 * @return size of resource as to be displayed to artists/ LDs in the Editor.
 */
INT UAnimSequence::GetResourceSize()
{
	const INT ResourceSize = CompressedTrackOffsets.Num() == 0 ? GetApproxRawSize() : GetApproxCompressedSize();
	return ResourceSize;
}

/**
 * @return		The approximate size of raw animation data.
 */
INT UAnimSequence::GetApproxRawSize() const
{
	INT Total = sizeof(FRawAnimSequenceTrack) * RawAnimData.Num();
	for (INT i=0;i<RawAnimData.Num();++i)
	{
		const FRawAnimSequenceTrack& RawTrack = RawAnimData(i);
		Total +=
			sizeof( FVector ) * RawTrack.PosKeys.Num() +
			sizeof( FQuat ) * RawTrack.RotKeys.Num() +
			sizeof( FLOAT ) * RawTrack.KeyTimes.Num();
	}
	return Total;
}

/**
 * @return		The approximate size of key-reduced animation data.
 */
INT UAnimSequence::GetApproxReducedSize() const
{
	INT Total =
		sizeof(FTranslationTrack) * TranslationData.Num() +
		sizeof(FRotationTrack) * RotationData.Num();

	for (INT i=0;i<TranslationData.Num();++i)
	{
		const FTranslationTrack& TranslationTrack = TranslationData(i);
		Total +=
			sizeof( FVector ) * TranslationTrack.PosKeys.Num() +
			sizeof( FLOAT ) * TranslationTrack.Times.Num();
	}

	for (INT i=0;i<RotationData.Num();++i)
	{
		const FRotationTrack& RotationTrack = RotationData(i);
		Total +=
			sizeof( FQuat ) * RotationTrack.RotKeys.Num() +
			sizeof( FLOAT ) * RotationTrack.Times.Num();
	}
	return Total;
}


/**
 * @return		The approximate size of compressed animation data.
 */
INT UAnimSequence::GetApproxCompressedSize() const
{
	const INT Total = sizeof(INT)*CompressedTrackOffsets.Num() + CompressedByteStream.Num();
	return Total;
}

/**
 * Deserializes old compressed track formats from the specified archive.
 */
static void LoadOldCompressedTrack(FArchive& Ar, FCompressedTrack& Dst, INT ByteStreamStride)
{
	// Serialize from the archive to a buffer.
	INT NumBytes = 0;
	Ar << NumBytes;

	TArray<BYTE> SerializedData;
	SerializedData.Empty( NumBytes );
	SerializedData.Add( NumBytes );
	Ar.Serialize( SerializedData.GetData(), NumBytes );

	// Serialize the key times.
	Ar << Dst.Times;

	// Serialize mins and ranges.
	Ar << Dst.Mins[0] << Dst.Mins[1] << Dst.Mins[2];
	Ar << Dst.Ranges[0] << Dst.Ranges[1] << Dst.Ranges[2];
}

#if !USE_ANIMATION_CODEC_INTERFACE
#if !CONSOLE || PS3
#define AC_UnalignedSwap( MemoryArchive, Data, Len )		\
	MemoryArchive.ByteOrderSerialize( (Data), (Len) );		\
	(Data) += (Len);
#else
	// No need to swap on Xenon, as the cooker will have ordered bytes for the target platform.
#define AC_UnalignedSwap( MemoryArchive, Data, Len )		\
	MemoryArchive.Serialize( (Data), (Len) );		\
	(Data) += (Len);
#endif // !CONSOLE
#endif
		
void UAnimSequence::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if ( Ar.IsLoading() )
	{
		// Serialize the compressed byte stream from the archive to the buffer.
		INT NumBytes;
		Ar << NumBytes;

		TArray<BYTE> SerializedData;
		SerializedData.Empty( NumBytes );
		SerializedData.Add( NumBytes );
		Ar.Serialize( SerializedData.GetData(), NumBytes );

		// Swap the buffer into the byte stream.
		FMemoryReader MemoryReader( SerializedData, TRUE );
		MemoryReader.SetByteSwapping( Ar.ForceByteSwapping() );

		CompressedByteStream.Empty( NumBytes );
		CompressedByteStream.Add( NumBytes );

#if USE_ANIMATION_CODEC_INTERFACE

		// we must know the proper codecs to use
		AnimationFormat_SetInterfaceLinks(*this);

		// and then use the codecs to byte swap
		AnimationFormat_ByteSwapIn(*this, MemoryReader);
#else
		const INT TransStride	= GetCompressedTranslationStride(this);
		const INT RotStride		= GetCompressedRotationStride(this);
		const INT RotNum		= CompressedRotationNum[RotationCompressionFormat];

		BYTE* StreamBase		= CompressedByteStream.GetTypedData();
		const INT NumTracks		= CompressedTrackOffsets.Num()/4;

		for ( INT TrackIndex = 0 ; TrackIndex < NumTracks ; ++TrackIndex )
		{
			const INT OffsetTrans	= CompressedTrackOffsets(TrackIndex*4);
			const INT NumKeysTrans	= CompressedTrackOffsets(TrackIndex*4+1);
			const INT OffsetRot		= CompressedTrackOffsets(TrackIndex*4+2);
			const INT NumKeysRot	= CompressedTrackOffsets(TrackIndex*4+3);

			// Translation data.
			checkSlow( (OffsetTrans % 4) == 0 && "CompressedByteStream not aligned to four bytes" );
			BYTE* TransTrackData = StreamBase + OffsetTrans;
			for ( INT KeyIndex = 0 ; KeyIndex < NumKeysTrans ; ++KeyIndex )
			{
				// A translation track of n keys is packed as n uncompressed float[3].
				AC_UnalignedSwap( MemoryReader, TransTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryReader, TransTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryReader, TransTrackData, sizeof(FLOAT) );
			}

			// Rotation data.
			checkSlow( (OffsetRot % 4) == 0 && "CompressedByteStream not aligned to four bytes" );
			BYTE* RotTrackData = StreamBase + OffsetRot;
			if ( NumKeysRot > 1 )
			{
				// For a rotation track of n>1 keys, the first 24 bytes are reserved for compression info
				// (eg Fixed32 stores float Mins[3]; float Ranges[3]), followed by n elements of the compressed type.
				AC_UnalignedSwap( MemoryReader, RotTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryReader, RotTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryReader, RotTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryReader, RotTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryReader, RotTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryReader, RotTrackData, sizeof(FLOAT) );

				for ( INT KeyIndex = 0 ; KeyIndex < NumKeysRot ; ++KeyIndex )
				{
					for ( int i = 0 ; i < RotNum ; ++i )
					{
						AC_UnalignedSwap( MemoryReader, RotTrackData, RotStride );
					}
				}
			}
			else if ( NumKeysRot == 1 )
			{
				// For a rotation track of n=1 keys, the single key is packed as an FQuatFloat96NoW.
				AC_UnalignedSwap( MemoryReader, RotTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryReader, RotTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryReader, RotTrackData, sizeof(FLOAT) );
			}

			// Like the compressed byte stream, pad the serialization stream to four bytes.
			// As a sanity check, each pad byte can be checked to be the PadSentinel.
			const PTRINT ByteStreamLoc = (PTRINT) RotTrackData;
			const INT Pad = static_cast<INT>( Align( ByteStreamLoc, 4 ) - ByteStreamLoc );
			MemoryReader.Serialize( RotTrackData, Pad );
			RotTrackData += Pad;
		}
#endif
	}
	else if( Ar.IsSaving() || Ar.IsCountingMemory() )
	{
		// Swap the byte stream into a buffer.
		TArray<BYTE> SerializedData;

#if USE_ANIMATION_CODEC_INTERFACE
		// we must know the proper codecs to use
		AnimationFormat_SetInterfaceLinks(*this);

		// and then use the codecs to byte swap
		AnimationFormat_ByteSwapOut(*this, SerializedData, Ar.ForceByteSwapping());
#else
		const INT TransStride	= GetCompressedTranslationStride(this);
		const INT RotStride		= GetCompressedRotationStride(this);
		const INT RotNum		= CompressedRotationNum[RotationCompressionFormat];

		FMemoryWriter MemoryWriter( SerializedData, TRUE );
		MemoryWriter.SetByteSwapping( Ar.ForceByteSwapping() );
		BYTE* StreamBase		= CompressedByteStream.GetTypedData();
		const INT NumTracks		= CompressedTrackOffsets.Num()/4;

		for ( INT TrackIndex = 0 ; TrackIndex < NumTracks ; ++TrackIndex )
		{
			const INT OffsetTrans	= CompressedTrackOffsets(TrackIndex*4);
			const INT NumKeysTrans	= CompressedTrackOffsets(TrackIndex*4+1);
			const INT OffsetRot		= CompressedTrackOffsets(TrackIndex*4+2);
			const INT NumKeysRot	= CompressedTrackOffsets(TrackIndex*4+3);

			// Translation data.
			checkSlow( (OffsetTrans % 4) == 0 && "CompressedByteStream not aligned to four bytes" );
			BYTE* TransTrackData = StreamBase + OffsetTrans;
			for ( INT KeyIndex = 0 ; KeyIndex < NumKeysTrans ; ++KeyIndex )
			{
				// A translation track of n keys is packed as n uncompressed float[3].
				AC_UnalignedSwap( MemoryWriter, TransTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryWriter, TransTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryWriter, TransTrackData, sizeof(FLOAT) );
			}

			// Rotation data.
			checkSlow( (OffsetRot % 4) == 0 && "CompressedByteStream not aligned to four bytes" );
			BYTE* RotTrackData = StreamBase + OffsetRot;
			if ( NumKeysRot > 1 )
			{
				// For a rotation track of n>1 keys, the first 24 bytes are reserved for compression info
				// (eg Fixed32 stores float Mins[3]; float Ranges[3]), followed by n elements of the compressed type.
				AC_UnalignedSwap( MemoryWriter, RotTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryWriter, RotTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryWriter, RotTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryWriter, RotTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryWriter, RotTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryWriter, RotTrackData, sizeof(FLOAT) );

				for ( INT KeyIndex = 0 ; KeyIndex < NumKeysRot ; ++KeyIndex )
				{
					for ( int i = 0 ; i < RotNum ; ++i )
					{
						AC_UnalignedSwap( MemoryWriter, RotTrackData, RotStride );
					}
				}
			}
			else if ( NumKeysRot == 1 )
			{
				// For a rotation track of n=1 keys, the single key is packed as an FQuatFloat96NoW.
				AC_UnalignedSwap( MemoryWriter, RotTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryWriter, RotTrackData, sizeof(FLOAT) );
				AC_UnalignedSwap( MemoryWriter, RotTrackData, sizeof(FLOAT) );
			}

			// Like the compressed byte stream, pad the serialization stream to four bytes.
			const INT Pad = Align( SerializedData.Num(), 4 ) - SerializedData.Num();
			for ( INT PadByteIndex = 0 ; PadByteIndex < Pad ; ++PadByteIndex )
			{
				BYTE PadSentinel = 85; // (1<<1)+(1<<3)+(1<<5)+(1<<7)
				MemoryWriter.Serialize( &PadSentinel, sizeof(BYTE) );
			}
		}
#endif
		// Make sure the entire byte stream was serialized.
		check( CompressedByteStream.Num() == SerializedData.Num() );

		// Serialize the buffer to archive.
		INT Num = SerializedData.Num();
		Ar << Num;
		Ar.Serialize( SerializedData.GetData(), SerializedData.Num() );

		// Count compressed data.
		Ar.CountBytes( SerializedData.Num(), SerializedData.Num() );
	}
}

/**
 * Used by various commandlets to purge Editor only data from the object.
 * 
 * @param TargetPlatform Platform the object will be saved for (ie PC vs console cooking, etc)
 */
void UAnimSequence::StripData(UE3::EPlatformType TargetPlatform)
{
	Super::StripData(TargetPlatform);

	// Remove raw animation data.
	for ( INT TrackIndex = 0 ; TrackIndex < RawAnimData.Num() ; ++TrackIndex )
	{
		FRawAnimSequenceTrack& RawTrack = RawAnimData(TrackIndex);
		RawTrack.PosKeys.Empty();
		RawTrack.RotKeys.Empty();
		RawTrack.KeyTimes.Empty();
	}
	RawAnimData.Empty();

	// Remove any key-reduced data.
	TranslationData.Empty();
	RotationData.Empty();

	// Remove ref pose for additive animation
	AdditiveRefPose.Empty();
}

void UAnimSequence::PreSave()
{
	// UAnimSequence::CompressionScheme is editoronly, but animations could be compressed during cook
	// via PostLoad; so, clear the reference before saving in the cooker,
	if ( GIsCooking )
	{
		CompressionScheme = NULL;
	}
}

void UAnimSequence::PostLoad()
{
	Super::PostLoad();

	// Fix up 'Bake and Prune' animations where their num frames doesn't match NumKeys.
#if !CONSOLE && !FINAL_RELEASE
	for(INT i=0; i<RawAnimData.Num(); i++)
	{
		const INT NumPosKeys = RawAnimData(i).PosKeys.Num();
		const INT NumRotKeys = RawAnimData(i).RotKeys.Num();

		if( NumPosKeys > 1 || NumRotKeys > 1 )
		{
			if( (NumPosKeys != 1 && NumPosKeys != NumFrames) || (NumRotKeys != 1 && NumRotKeys != NumFrames) )
			{
				UPackage* AnimSeqPackage = GetOutermost();
				warnf(TEXT("Found animation (%s, %s, %s) where NumFrames doesn't match NumKeys!! NumFrames: %i, NumPosKeys: %i, NumRotKeys: %i"), 
					*SequenceName.ToString(), *GetAnimSet()->GetFName().ToString(), *AnimSeqPackage->GetFName().ToString(), NumFrames, NumPosKeys, NumRotKeys);

				// If saved into a level, then we know it was baked and pruned, so we just fix up NumFrames in that case
				if( AnimSeqPackage->ContainsMap() )
				{
					NumFrames = (NumPosKeys >= NumRotKeys) ? NumPosKeys : NumRotKeys;
					warnf(TEXT("Was detected as a Bake and Prune error, fixing up! NumFrames: %d"), NumFrames);
					MarkPackageDirty();
				}
			}
			break;
		}
	}
#endif

	// Ensure notifies are sorted.
	SortNotifies();

	// Compress animation data if no compression currently exists.
	if( CompressedTrackOffsets.Num() == 0 )
	{
#if CONSOLE
		// Never compress on consoles.
		appErrorf( TEXT("No animation compression exists for sequence %s (%s)"), *SequenceName.ToString(), (GetOuter() ? *GetOuter()->GetFullName() : *GetFullName()) );
#else
		warnf( TEXT("No animation compression exists for sequence %s (%s)"), *SequenceName.ToString(), (GetOuter() ? *GetOuter()->GetFullName() : *GetFullName()) );
		FAnimationUtils::CompressAnimSequence(this, NULL, TRUE, FALSE);
		// Mark package dirty for resave
		if( GIsEditor )
		{
			MarkPackageDirty();
		}
#endif // CONSOLE
	}

#if USE_ANIMATION_CODEC_INTERFACE
	static UBOOL ForcedRecompressionSetting = FAnimationUtils::GetForcedRecompressionSetting();

	// Recompress the animation if it was encoded with an old package set
	// or we are being forced to do so
	if (EncodingPkgVersion != CURRENT_ANIMATION_ENCODING_PACKAGE_VERSION ||
		ForcedRecompressionSetting)
	{
#if CONSOLE
		if (EncodingPkgVersion != CURRENT_ANIMATION_ENCODING_PACKAGE_VERSION)
		{
			// Never compress on consoles.
			appErrorf( TEXT("Animation compression method out of date for sequence %s"), *SequenceName.ToString() );
			CompressedTrackOffsets.Empty(0);
			CompressedByteStream.Empty(0);
		}
#else
		FAnimationUtils::CompressAnimSequence(this, NULL, TRUE, FALSE);
#endif // CONSOLE
	}

#if !VARIABLE_KEY_CODEC_ENABLED
	// when the variable-key codec has been disabled, recompress all animations using it
	if (KeyEncodingFormat == AKF_VariableKeyLerp)
	{
#if CONSOLE
		// Never compress on consoles.
		appErrorf( TEXT("Animation compression method deprecated for sequence %s"), *SequenceName.ToString() );
		CompressedTrackOffsets.Empty(0);
		CompressedByteStream.Empty(0);
#else
		FAnimationUtils::CompressAnimSequence(this, NULL, FALSE, FALSE);
#endif // CONSOLE
	}
#endif // !VARIABLE_KEY_CODEC_ENABLED

#endif // USE_ANIMATION_CODEC_INTERFACE

	// If we're in the game and compressed animation data exists, whack the raw data.
	if( GIsGame && !GIsEditor )
	{
		if( RawAnimData.Num() > 0  && CompressedTrackOffsets.Num() > 0 )
		{
#if CONSOLE
			// Don't do this on consoles; raw animation data should have been stripped during cook!
			appErrorf( TEXT("Cooker did not strip raw animation from sequence %s"), *SequenceName.ToString() );
#else
			// Remove raw animation data.
			for ( INT TrackIndex = 0 ; TrackIndex < RawAnimData.Num() ; ++TrackIndex )
			{
				FRawAnimSequenceTrack& RawTrack = RawAnimData(TrackIndex);
				RawTrack.PosKeys.Empty();
				RawTrack.RotKeys.Empty();
				RawTrack.KeyTimes.Empty();
			}
			
			RawAnimData.Empty();
#endif // CONSOLE
		}

		// Remove additive animation ref pose.
		AdditiveRefPose.Empty();
	}

#if USE_ANIMATION_CODEC_INTERFACE
	// setup the Codec interfaces
	AnimationFormat_SetInterfaceLinks(*this);
#endif

	if( GIsGame && !GIsEditor )
	{
		// this probably will not show newly created animations in PIE but will show them in the game once they have been saved off
		INC_DWORD_STAT_BY( STAT_AnimationMemory, GetResourceSize() );
	}
}

void UAnimSequence::BeginDestroy()
{
	Super::BeginDestroy();

	// clear any active codec links
	RotationCodec = NULL;
	TranslationCodec = NULL;

	if( GIsGame && !GIsEditor )
	{
		DEC_DWORD_STAT_BY( STAT_AnimationMemory, GetResourceSize() );
	}
}

void UAnimSequence::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	if(!IsTemplate())
	{
		// Make sure package is marked dirty when doing stuff like adding/removing notifies
		MarkPackageDirty();
	}
}

// @todo DB: Optimize!
template<typename TimeArray>
static INT FindKeyIndex(FLOAT Time, const TimeArray& Times)
{
	INT FoundIndex = 0;
	for ( INT Index = 0 ; Index < Times.Num() ; ++Index )
	{
		const FLOAT KeyTime = Times(Index);
		if ( Time >= KeyTime )
		{
			FoundIndex = Index;
		}
		else
		{
			break;
		}
	}
	return FoundIndex;
}



/**
 * Populates the key reduced arrays from raw animation data.
 */
void UAnimSequence::SeparateRawDataToTracks(const TArray<FRawAnimSequenceTrack>& RawAnimData,
											FLOAT SequenceLength,
											TArray<FTranslationTrack>& OutTranslationData,
											TArray<FRotationTrack>& OutRotationData)
{
	// functionality moved to UAnimationCompressionAlgorithm for general use by Codecs
#if !USE_ANIMATION_CODEC_INTERFACE
	const INT NumTracks = RawAnimData.Num();

	OutTranslationData.Empty( NumTracks );
	OutRotationData.Empty( NumTracks );
	OutTranslationData.AddZeroed( NumTracks );
	OutRotationData.AddZeroed( NumTracks );

	for ( INT TrackIndex = 0 ; TrackIndex < NumTracks ; ++TrackIndex )
	{
		const FRawAnimSequenceTrack& RawTrack	= RawAnimData(TrackIndex);
		FTranslationTrack&	TranslationTrack	= OutTranslationData(TrackIndex);
		FRotationTrack&		RotationTrack		= OutRotationData(TrackIndex);

		const INT PrevNumPosKeys = RawTrack.PosKeys.Num();
		const INT PrevNumRotKeys = RawTrack.RotKeys.Num();

		// Do nothing if the data for this track is empty.
		if( PrevNumPosKeys == 0 || PrevNumRotKeys == 0 )
		{
			continue;
		}

		// Copy over position keys.
		for ( INT PosIndex = 0 ; PosIndex < RawTrack.PosKeys.Num() ; ++PosIndex )
		{
			TranslationTrack.PosKeys.AddItem( RawTrack.PosKeys(PosIndex) );
		}

		// Copy over rotation keys.
		for ( INT RotIndex = 0 ; RotIndex < RawTrack.RotKeys.Num() ; ++RotIndex )
		{
			RotationTrack.RotKeys.AddItem( RawTrack.RotKeys(RotIndex) );
		}

		// Set times for the translation track.
		if ( TranslationTrack.PosKeys.Num() > 1 )
		{
			const FLOAT PosFrameInterval = SequenceLength / static_cast<FLOAT>(TranslationTrack.PosKeys.Num()-1);
			for ( INT PosIndex = 0 ; PosIndex < TranslationTrack.PosKeys.Num() ; ++PosIndex )
			{
				TranslationTrack.Times.AddItem( PosIndex * PosFrameInterval );
			}
		}
		else
		{
			TranslationTrack.Times.AddItem( 0.f );
		}

		// Set times for the rotation track.
		if ( RotationTrack.RotKeys.Num() > 1 )
		{
			const FLOAT RotFrameInterval = SequenceLength / static_cast<FLOAT>(RotationTrack.RotKeys.Num()-1);
			for ( INT RotIndex = 0 ; RotIndex < RotationTrack.RotKeys.Num() ; ++RotIndex )
			{
				RotationTrack.Times.AddItem( RotIndex * RotFrameInterval );
			}
		}
		else
		{
			RotationTrack.Times.AddItem( 0.f );
		}

		// Trim unused memory.
		TranslationTrack.PosKeys.Shrink();
		TranslationTrack.Times.Shrink();
		RotationTrack.RotKeys.Shrink();
		RotationTrack.Times.Shrink();
	}
#endif
}

#if !USE_ANIMATION_CODEC_INTERFACE

/**
 * Reconstructs a bone atom from key-reduced tracks.
 */
void UAnimSequence::ReconstructBoneAtom(FBoneAtom& OutAtom,
										const FTranslationTrack& TranslationTrack,
										const FRotationTrack& RotationTrack,
										FLOAT SequenceLength,
										FLOAT Time,
										UBOOL bLooping)
{
	OutAtom.Scale = 1.f;

	// Bail out (with rather wacky data) if data is empty for some reason.
	if( TranslationTrack.PosKeys.Num() == 0 || TranslationTrack.Times.Num() == 0 || 
		RotationTrack.RotKeys.Num() == 0 || RotationTrack.Times.Num() == 0 )
	{
		debugf( TEXT("UAnimSequence::ReconstructBoneAtom(reduced) : No anim data in AnimSequence!") );
		OutAtom.Rotation = FQuat::Identity;
		OutAtom.Translation = FVector(0.f, 0.f, 0.f);
		return;
	}

	// Check for before-first-frame case.
	if( Time <= 0.f )
	{
		OutAtom.Translation = TranslationTrack.PosKeys(0);
		OutAtom.Rotation	= RotationTrack.RotKeys(0);
		return;
	}

	// Check for after-last-frame case.
	const INT LastPosIndex	= TranslationTrack.PosKeys.Num() - 1;
	const INT LastRotIndex	= RotationTrack.RotKeys.Num() - 1;
	if( Time >= SequenceLength )
	{
		OutAtom.Translation = TranslationTrack.PosKeys(LastPosIndex);
		OutAtom.Rotation	= RotationTrack.RotKeys(LastRotIndex);
		return;
	}

	// Find the "starting" key indices.
	const INT PosIndex0 = FindKeyIndex( Time, TranslationTrack.Times );
	const INT RotIndex0 = FindKeyIndex( Time, RotationTrack.Times );

	///////////////////////
	// Translation.

	INT PosIndex1;
	FLOAT PosAlpha;

	// If we have gone over the end, do different things in case of looping.
	if ( PosIndex0 == LastPosIndex )
	{
		// If looping, interpolate between last and first frame.
		if( bLooping )
		{
			PosIndex1 = 0;
			// @todo DB: handle looping with variable-length keys.
			PosAlpha = 0.5f;
		}
		// If not looping - hold the last frame.
		else
		{
			PosIndex1 = PosIndex0;
			PosAlpha = 1.f;
		}
	}
	else
	{
		// Find the "ending" key index and alpha.
		PosIndex1 = PosIndex0+1;
		const FLOAT DeltaTime = TranslationTrack.Times(PosIndex1) - TranslationTrack.Times(PosIndex0);
		PosAlpha = (Time - TranslationTrack.Times(PosIndex0))/DeltaTime;
	}

	OutAtom.Translation = Lerp( TranslationTrack.PosKeys(PosIndex0), TranslationTrack.PosKeys(PosIndex1), PosAlpha );

	///////////////////////
	// Rotation.

	INT RotIndex1;
	FLOAT RotAlpha;

	// If we have gone over the end, do different things in case of looping.
	if ( RotIndex0 == LastRotIndex )
	{
		// If looping, interpolate between last and first frame.
		if( bLooping )
		{
			RotIndex1 = 0;
			 // @todo DB: handle looping with variable-length keys.
			RotAlpha = 0.5f;
		}
		// If not looping - hold the last frame.
		else
		{
			RotIndex1 = RotIndex0;
			RotAlpha = 1.f;
		}
	}
	else
	{
		// Find the "ending" key index and alpha.
		RotIndex1 = RotIndex0+1;
		const FLOAT DeltaTime = RotationTrack.Times(RotIndex1) - RotationTrack.Times(RotIndex0);
		RotAlpha = (Time - RotationTrack.Times(RotIndex0))/DeltaTime;
	}

#if !USE_SLERP
	// Fast linear quaternion interpolation.
	// To ensure the 'shortest route', we make sure the dot product between the two keys is positive.
	if( (RotationTrack.RotKeys(RotIndex0) | RotationTrack.RotKeys(RotIndex1)) < 0.f )
	{
		// To clarify the code here: a slight optimization of inverting the parametric variable as opposed to the quaternion.
		OutAtom.Rotation = (RotationTrack.RotKeys(RotIndex0) * (1.f-RotAlpha)) + (RotationTrack.RotKeys(RotIndex1) * -RotAlpha);
	}
	else
	{
		OutAtom.Rotation = (RotationTrack.RotKeys(RotIndex0) * (1.f-RotAlpha)) + (RotationTrack.RotKeys(RotIndex1) * RotAlpha);
	}
#else
	OutAtom.Rotation = SlerpQuat( RotationTrack.RotKeys(RotIndex0), RotationTrack.RotKeys(RotIndex1), RotAlpha );
#endif

	OutAtom.Rotation.Normalize();
}

/**
 * Decompresses a translation key from the specified compressed translation track.
 */
inline void UAnimSequence::ReconstructTranslation(FVector& Out, const BYTE* Stream, INT KeyIndex, AnimationCompressionFormat TranslationCompressionFormat)
{
	const BYTE* KeyData = Stream + KeyIndex*CompressedTranslationStrides[TranslationCompressionFormat]*CompressedTranslationNum[TranslationCompressionFormat];
	if ( TranslationCompressionFormat == ACF_None )
	{
		Out = *((FVector*)KeyData);
	}
	else
	{
		appErrorf( TEXT("%i: unknown or unsupported animation compression format"), (INT)TranslationCompressionFormat );
		// Silence compilers warning about a value potentially not being assigned.
		Out = FVector(0.f,0.f,0.f);
	}
}

/**
 * Decompresses a rotation key from the specified compressed rotation track.
 */
inline void UAnimSequence::ReconstructRotation(FQuat& Out,
											   const BYTE* Stream,
											   INT KeyIndex,
											   AnimationCompressionFormat RotationCompressionFormat,
											   const FLOAT* Mins,
											   const FLOAT* Ranges)
{
	const BYTE* KeyData = Stream + KeyIndex*CompressedRotationStrides[RotationCompressionFormat]*CompressedRotationNum[RotationCompressionFormat];

	if ( RotationCompressionFormat == ACF_None )
	{
		Out = *((FQuat*)KeyData);
	}
	else if ( RotationCompressionFormat == ACF_Fixed32NoW )
	{
		( *((FQuatFixed32NoW*)KeyData) ).ToQuat( Out );
	}
	else if ( RotationCompressionFormat == ACF_Fixed48NoW )
	{
		( *((FQuatFixed48NoW*)KeyData) ).ToQuat( Out );
	}
	else if ( RotationCompressionFormat == ACF_Float96NoW )
	{
		( *((FQuatFloat96NoW*)KeyData) ).ToQuat( Out );
	}
	else if ( RotationCompressionFormat == ACF_IntervalFixed32NoW )
	{
		( *((FQuatIntervalFixed32NoW*)KeyData) ).ToQuat( Out, Mins, Ranges );
	}
	else if ( RotationCompressionFormat == ACF_Float32NoW )
	{
		( *((FQuatFloat32NoW*)KeyData) ).ToQuat( Out );
	}
	else
	{
		appErrorf( TEXT("%i: unknown or unsupported animation compression format"), (INT)RotationCompressionFormat );
		// Silence compilers warning about a value potentially not being assigned.
		Out = FQuat::Identity;
	}
}

/**
 * Reconstructs a bone atom from compressed tracks.
 */
void UAnimSequence::ReconstructBoneAtom(FBoneAtom& OutAtom,
										const FCompressedTrack& TranslationTrack,
										const FCompressedTrack& RotationTrack,
										AnimationCompressionFormat TranslationCompressionFormat,
										AnimationCompressionFormat RotationCompressionFormat,
										FLOAT SequenceLength,
										FLOAT Time,
										UBOOL bLooping)
{
	OutAtom.Scale = 1.f;

	// Bail out (with rather wacky data) if data is empty for some reason.
	if( TranslationTrack.ByteStream.Num() == 0 ||// TranslationTrack.Times.Num() == 0 || 
		RotationTrack.ByteStream.Num() == 0/* || RotationTrack.Times.Num() == 0*/ )
	{
		debugf( TEXT("UAnimSequence::ReconstructBoneAtom(compressed) : No anim data in AnimSequence!") );
		OutAtom.Rotation = FQuat::Identity;
		OutAtom.Translation = FVector(0.f, 0.f, 0.f);
		return;
	}

	// Check for before-first-frame case.
	const BYTE* TransStream		= TranslationTrack.ByteStream.GetTypedData();
	const BYTE* RotStream		= RotationTrack.ByteStream.GetTypedData();
	if( Time <= 0.f )
	{
		ReconstructTranslation( OutAtom.Translation, TransStream, 0, TranslationCompressionFormat );
		ReconstructRotation( OutAtom.Rotation, RotStream, 0, RotationCompressionFormat, RotationTrack.Mins, RotationTrack.Ranges );
		return;
	}

	// Check for after-last-frame case.
	const INT TransStride	= CompressedTranslationStrides[TranslationCompressionFormat];
	const INT RotStride		= CompressedRotationStrides[RotationCompressionFormat];

	const INT TransNum = CompressedTranslationNum[TranslationCompressionFormat];
	const INT RotNum = CompressedRotationNum[RotationCompressionFormat];

	const INT NumTransKeys	= TranslationTrack.ByteStream.Num() / (TransStride*TransNum);
	const INT NumRotKeys	= RotationTrack.ByteStream.Num() / (RotStride*RotNum);

	const INT LastPosIndex	= NumTransKeys - 1;
	const INT LastRotIndex	= NumRotKeys - 1;
	if( Time >= SequenceLength )
	{
		ReconstructTranslation( OutAtom.Translation, TransStream, LastPosIndex, TranslationCompressionFormat );
		ReconstructRotation( OutAtom.Rotation, RotStream, LastRotIndex, RotationCompressionFormat, RotationTrack.Mins, RotationTrack.Ranges );
		return;
	}

	const UBOOL bEquallySpacedKeys = (TranslationTrack.Times.Num() == 0) && (RotationTrack.Times.Num() == 0);

	INT PosIndex0;
	INT PosIndex1;
	INT RotIndex0;
	INT RotIndex1;
	FLOAT PosAlpha;
	FLOAT RotAlpha;

	///////////////////////
	// Translation.

	if ( bEquallySpacedKeys )
	{
		if ( NumTransKeys == 1 )
		{
			PosIndex0 = 0;
			PosIndex1 = 0;
			PosAlpha = 0.f;
		}
		else
		{
			// For looping animation, the last frame has duration, and interpolates back to the first one.
			// For non-looping animation, the last frame is the ending frame, and has no duration.
			const FLOAT TransFrameInterval = SequenceLength / (bLooping ? NumTransKeys : NumTransKeys-1);

			const FLOAT KeyPos = Time/TransFrameInterval;
			PosIndex0 = Clamp<INT>( appFloor(KeyPos), 0, NumTransKeys-1 ); // @todo should be changed to appTrunc
			PosAlpha = KeyPos - (FLOAT)PosIndex0;

			PosIndex1 = PosIndex0+1;
			if ( PosIndex1 == NumTransKeys )
			{
				PosIndex1 = ( bLooping ? 0 : PosIndex0 );
			}
		}
	}
	else
	{
		PosIndex0 = FindKeyIndex( Time, TranslationTrack.Times );

		// If we have gone over the end, do different things in case of looping.
		if ( PosIndex0 == LastPosIndex )
		{
			// If looping, interpolate between last and first frame.
			if( bLooping )
			{
				PosIndex1 = 0;
				// @todo DB: handle looping with variable-length keys.
				PosAlpha = 0.5f;
			}
			// If not looping - hold the last frame.
			else
			{
				PosIndex1 = PosIndex0;
				PosAlpha = 0.f;
			}
		}
		else
		{
			// Find the "ending" key index and alpha.
			PosIndex1 = PosIndex0+1;
			const FLOAT DeltaTime = TranslationTrack.Times(PosIndex1) - TranslationTrack.Times(PosIndex0);
			PosAlpha = (Time - TranslationTrack.Times(PosIndex0))/DeltaTime;
		}
	}

	FVector P0;
	FVector P1;
	ReconstructTranslation( P0, TransStream, PosIndex0, TranslationCompressionFormat );
	ReconstructTranslation( P1, TransStream, PosIndex1, TranslationCompressionFormat );
	OutAtom.Translation = Lerp( P0, P1, PosAlpha );

	///////////////////////
	// Rotation.
	if( bEquallySpacedKeys )
	{
		if ( NumRotKeys == 1 )
		{
			RotIndex0 = 0;
			RotIndex1 = 0;
			RotAlpha = 0.f;
		}
		else
		{
			// For looping animation, the last frame has duration, and interpolates back to the first one.
			// For non-looping animation, the last frame is the ending frame, and has no duration.
			const FLOAT RotFrameInterval = SequenceLength / (bLooping ? NumRotKeys : NumRotKeys-1);

			const FLOAT KeyPos = Time/RotFrameInterval;
			RotIndex0 = Clamp<INT>( appFloor(KeyPos), 0, NumRotKeys-1 ); // @todo should be changed to appTrunc
			RotAlpha = KeyPos - (FLOAT)RotIndex0;

			RotIndex1 = RotIndex0+1;
			if ( RotIndex1 == NumRotKeys )
			{
				RotIndex1 = ( bLooping ? 0 : RotIndex0 );
			}
		}
	}
	else
	{
		RotIndex0 = FindKeyIndex( Time, RotationTrack.Times );

		// If we have gone over the end, do different things in case of looping.
		if ( RotIndex0 == LastRotIndex )
		{
			// If looping, interpolate between last and first frame.
			if( bLooping )
			{
				RotIndex1 = 0;
				// @todo DB: handle looping with variable-length keys.
				RotAlpha = 0.5f;
			}
			// If not looping - hold the last frame.
			else
			{
				RotIndex1 = RotIndex0;
				RotAlpha = 0.f;
			}
		}
		else
		{
			// Find the "ending" key index and alpha.
			RotIndex1 = RotIndex0+1;
			const FLOAT DeltaTime = RotationTrack.Times(RotIndex1) - RotationTrack.Times(RotIndex0);
			RotAlpha = (Time - RotationTrack.Times(RotIndex0))/DeltaTime;
		}
	}

	FQuat R0;
	FQuat R1;
	ReconstructRotation( R0, RotStream, RotIndex0, RotationCompressionFormat, RotationTrack.Mins, RotationTrack.Ranges );
	ReconstructRotation( R1, RotStream, RotIndex1, RotationCompressionFormat, RotationTrack.Mins, RotationTrack.Ranges );

#if !USE_SLERP
	// Fast linear quaternion interpolation.
	// To ensure the 'shortest route', we make sure the dot product between the two keys is positive.
	if( (R0 | R1) < 0.f )
	{
		// To clarify the code here: a slight optimization of inverting the parametric variable as opposed to the quaternion.
		OutAtom.Rotation = (R0 * (1.f-RotAlpha)) + (R1 * -RotAlpha);
	}
	else
	{
		OutAtom.Rotation = (R0 * (1.f-RotAlpha)) + (R1 * RotAlpha);
	}
#else
	OutAtom.Rotation = SlerpQuat( R0, R1, RotAlpha );
#endif

	OutAtom.Rotation.Normalize();
}

/**
 * Decompresses a translation key from the specified compressed translation track.
 */
void UAnimSequence::ReconstructTranslation(FVector& Out, const BYTE* Stream, INT KeyIndex)
{
	// A translation track of n keys is packed as n uncompressed float[3].
	Out = *( (FVector*)(Stream + KeyIndex*sizeof(FVector)) );
}

/**
 * Decompresses a rotation key from the specified compressed rotation track.
 */
void UAnimSequence::ReconstructRotation(FQuat& Out, const BYTE* Stream, INT KeyIndex, UBOOL bTrackHasCompressionInfo, AnimationCompressionFormat RotationCompressionFormat)
{
	if ( bTrackHasCompressionInfo )
	{
		const BYTE* KeyData = Stream+sizeof(FLOAT)*6+KeyIndex*CompressedRotationStrides[RotationCompressionFormat]*CompressedRotationNum[RotationCompressionFormat];
		if ( RotationCompressionFormat == ACF_None )
		{
			Out = *((FQuat*)KeyData);
		}
		else if ( RotationCompressionFormat == ACF_Float96NoW )
		{
			((FQuatFloat96NoW*)KeyData)->ToQuat( Out );
		}
		else if ( RotationCompressionFormat == ACF_Fixed32NoW )
		{
			((FQuatFixed32NoW*)KeyData)->ToQuat( Out );
		}
		else if ( RotationCompressionFormat == ACF_Fixed48NoW )
		{
			((FQuatFixed48NoW*)KeyData)->ToQuat( Out );
		}
		else if ( RotationCompressionFormat == ACF_IntervalFixed32NoW )
		{
			const FLOAT* Mins = (FLOAT*)Stream;
			const FLOAT* Ranges = (FLOAT*)(Stream+sizeof(FLOAT)*3);
			((FQuatIntervalFixed32NoW*)KeyData)->ToQuat( Out, Mins, Ranges );
		}
		else if ( RotationCompressionFormat == ACF_Float32NoW )
		{
			((FQuatFloat32NoW*)KeyData)->ToQuat( Out );
		}
	}
	else
	{
		// For a rotation track of n=1 keys, the single key is packed as an FQuatFloat96NoW.
		check( KeyIndex == 0 );
		((FQuatFloat96NoW*)Stream)->ToQuat( Out );
	}
}

/** Computes indices into the stream for the keyframes bracketing the specified time. */
static FORCEINLINE FLOAT TimeToIndex(FLOAT SequenceLength,FLOAT Time,UBOOL bLooping,INT NumKeys,INT &PosIndex0Out,INT &PosIndex1Out)
{
	static INT		NumKeysCache = 0; // this value is guaranteed to not be used for valid data
	static FLOAT	TimeCache;
	static FLOAT	SequenceLengthCache;
	static UBOOL	LoopingCache;
	static INT		PosIndex0CacheOut; 
	static INT		PosIndex1CacheOut; 
	static FLOAT	AlphaCacheOut;

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
		TimeCache			!= Time
		)
	{
		NumKeysCache		= NumKeys;
		LoopingCache		= bLooping;
		SequenceLengthCache = SequenceLength;
		TimeCache			= Time;
		// Check for before-first-frame case.
		if( Time <= 0.f )
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
				if( Time >= SequenceLength )
				{
					// If we're not looping, key n-1 is the final key.
					PosIndex0CacheOut = NumKeys;
					PosIndex1CacheOut = NumKeys;
					AlphaCacheOut = 0.0f;
				}
				else
				{
					// For non-looping animation, the last frame is the ending frame, and has no duration.
					const FLOAT TransFrameIntervalInv = FLOAT(NumKeys) / SequenceLength;
					const FLOAT KeyPos = Time * TransFrameIntervalInv;
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
				if( Time >= SequenceLength )
				{
					// If we're looping, key 0 is the final key.
					PosIndex0CacheOut = 0;
					PosIndex1CacheOut = 0;
					AlphaCacheOut = 0.0f;
				}
				else
				{
					// For looping animation, the last frame has duration, and interpolates back to the first one.
					const FLOAT TransFrameIntervalInv = FLOAT(NumKeys) / SequenceLength;
					const FLOAT KeyPos = Time * TransFrameIntervalInv;
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


void UAnimSequence::ReconstructBoneAtom(FBoneAtom& OutAtom,
										const BYTE* TransStream,
										INT NumTransKeys,
										const BYTE* RotStream,
										INT NumRotKeys,
										AnimationCompressionFormat TranslationCompressionFormat,
										AnimationCompressionFormat RotationCompressionFormat,
										FLOAT SequenceLength,
										FLOAT Time,
										UBOOL bLooping)

{
	OutAtom.Scale = 1.f;

	INT Index0;
	INT Index1;
	FLOAT Alpha = TimeToIndex(SequenceLength,Time,bLooping,NumTransKeys,Index0,Index1);

	if (Index0 == Index1)
	{
		ReconstructTranslation( OutAtom.Translation, TransStream, Index0);
	}
	else
	{
		FVector P0;
		FVector P1;
		ReconstructTranslation( P0, TransStream, Index0 );
		ReconstructTranslation( P1, TransStream, Index1 );
		OutAtom.Translation = Lerp( P0, P1, Alpha );
	}

	if (NumRotKeys != NumTransKeys)
	{
		Alpha = TimeToIndex(SequenceLength,Time,bLooping,NumRotKeys,Index0,Index1);
	}

	if (Index0 == Index1)
	{
		ReconstructRotation( OutAtom.Rotation, RotStream, Index0, NumRotKeys > 1, RotationCompressionFormat );
	}
	else
	{
		FQuat R0;
		FQuat R1;
		ReconstructRotation( R0, RotStream, Index0, TRUE, RotationCompressionFormat );
		ReconstructRotation( R1, RotStream, Index1, TRUE, RotationCompressionFormat );

#if !USE_SLERP
		// Fast linear quaternion interpolation.
		// To ensure the 'shortest route', we make sure the dot product between the two keys is positive.
		if( (R0 | R1) < 0.f )
		{
			// To clarify the code here: a slight optimization of inverting the parametric variable as opposed to the quaternion.
			OutAtom.Rotation = (R0 * (1.f-Alpha)) + (R1 * -Alpha);
		}
		else
		{
			OutAtom.Rotation = (R0 * (1.f-Alpha)) + (R1 * Alpha);
		}
#else
		OutAtom.Rotation = SlerpQuat( R0, R1, Alpha );
#endif
		OutAtom.Rotation.Normalize();
	}
}

#endif //#if !USE_ANIMATION_CODEC_INTERFACE

#if 0
class FScopedTimer
{
public:
	const DOUBLE StartTime;
	const TCHAR* String;
	FScopedTimer(const TCHAR* InStr)
		:	StartTime( appSeconds() )
		,	String( InStr )
	{}
	~FScopedTimer()
	{
		debugf(TEXT("%s %f ms"), String, (appSeconds()-StartTime) * 1000.0 );
	}
};
#endif

/**
 * Interpolate keyframes in this sequence to find the bone transform (relative to parent).
 * 
 * @param	OutAtom			[out] Output bone transform.
 * @param	TrackIndex		Index of track to interpolate.
 * @param	Time			Time on track to interpolate to.
 * @param	bLooping		TRUE if the animation is looping.
 * @param	bUseRawData		If TRUE, use raw animation data instead of compressed data.
 */
void UAnimSequence::GetBoneAtom(FBoneAtom& OutAtom, INT TrackIndex, FLOAT Time, UBOOL bLooping, UBOOL bUseRawData) const
{
	// If the caller didn't request that raw animation data be used . . .
	if ( !bUseRawData )
	{
#if 0
		if( CompressedTranslationData.Num() > 0 )
		{
			//const FScopedTimer Timer( TEXT("Compressed") );
			// Build the pose using bitwise compressed data.
			ReconstructBoneAtom( OutAtom,
									CompressedTranslationData(TrackIndex),
									CompressedRotationData(TrackIndex),
									static_cast<AnimationCompressionFormat>(TranslationCompressionFormat),
									static_cast<AnimationCompressionFormat>(RotationCompressionFormat),
									SequenceLength,
									Time,
									bLooping );
			return;
		}
#else
		if ( CompressedTrackOffsets.Num() > 0 )
		{

#if USE_ANIMATION_CODEC_INTERFACE

		AnimationFormat_GetBoneAtom(	OutAtom,
									*this,
									TrackIndex,
									Time,
									bLooping);

#else
			ReconstructBoneAtom( OutAtom,
									CompressedByteStream.GetTypedData()+CompressedTrackOffsets(TrackIndex*4),
									CompressedTrackOffsets(TrackIndex*4+1),
									CompressedByteStream.GetTypedData()+CompressedTrackOffsets(TrackIndex*4+2),
									CompressedTrackOffsets(TrackIndex*4+3),
									static_cast<AnimationCompressionFormat>(TranslationCompressionFormat),
									static_cast<AnimationCompressionFormat>(RotationCompressionFormat),
									SequenceLength,
									Time,
									bLooping );
#endif
			return;
		}

#endif
		/*
		else if ( TranslationData.Num() > 0 )
		{
			// Build the pose using the key reduced data.
			ReconstructBoneAtom( OutAtom, TranslationData(TrackIndex), RotationData(TrackIndex), SequenceLength, Time, bLooping );
			return;
		}
		*/
	}

	//const FScopedTimer Timer( TEXT("Raw") );
	OutAtom.Scale = 1.f;

	// Bail out if the animation data doesn't exists (e.g. was stripped by the cooker).
	if ( RawAnimData.Num() == 0 )
	{
		debugf( TEXT("UAnimSequence::GetBoneAtom : No anim data in AnimSequence!") );
		OutAtom.Rotation = FQuat::Identity;
		OutAtom.Translation = FVector(0.f, 0.f, 0.f);
		return;
	}

	const FRawAnimSequenceTrack& RawTrack = RawAnimData(TrackIndex);

	// Bail out (with rather wacky data) if data is empty for some reason.
	if( RawTrack.KeyTimes.Num() == 0 || 
		RawTrack.PosKeys.Num() == 0 ||
		RawTrack.RotKeys.Num() == 0 )
	{
		debugf( TEXT("UAnimSequence::GetBoneAtom : No anim data in AnimSequence!") );
		OutAtom.Rotation = FQuat::Identity;
		OutAtom.Translation = FVector(0.f, 0.f, 0.f);
		return;
	}

   	// Check for 1-frame, before-first-frame and after-last-frame cases.
	if( Time <= 0.f || RawTrack.KeyTimes.Num() == 1 )
	{
		OutAtom.Translation = RawTrack.PosKeys(0);
		OutAtom.Rotation	= RawTrack.RotKeys(0);
		return;
	}

	const INT LastIndex		= RawTrack.KeyTimes.Num() - 1;
	const INT LastPosIndex	= ::Min(LastIndex, RawTrack.PosKeys.Num()-1);
	const INT LastRotIndex	= ::Min(LastIndex, RawTrack.RotKeys.Num()-1);
	if( Time >= SequenceLength )
	{
		// If we're not looping, key n-1 is the final key.
		// If we're looping, key 0 is the final key.
		OutAtom.Translation = RawTrack.PosKeys( bLooping ? 0 : LastPosIndex );
		OutAtom.Rotation	= RawTrack.RotKeys( bLooping ? 0 : LastRotIndex );
		return;
	}

	// This assumes that all keys are equally spaced (ie. won't work if we have dropped unimportant frames etc).	
	const FLOAT FrameInterval = bLooping ?
	(
		// by default, for looping animation, the last frame has a duration, and interpolates back to the first one.
		//RawTrack.KeyTimes(1) - RawTrack.KeyTimes(0);
		SequenceLength / (FLOAT)RawTrack.KeyTimes.Num()
	)
		:
	(
		// For non looping animation, the last frame is the ending frame, and has no duration.
		SequenceLength / ((FLOAT)RawTrack.KeyTimes.Num()-1)
	);

	// Keyframe we want is somewhere in the actual data. 

	// Find key position as a float.
	const FLOAT KeyPos = Time/FrameInterval;

	// Find the integer part (ensuring within range) and that gives us the 'starting' key index.
	const INT KeyIndex1 = Clamp<INT>( appFloor(KeyPos), 0, RawTrack.KeyTimes.Num()-1 );  // @todo should be changed to appTrunc

	// The alpha (fractional part) is then just the remainder.
	const FLOAT Alpha = KeyPos - (FLOAT)KeyIndex1;

	INT KeyIndex2 = KeyIndex1 + 1;

	// If we have gone over the end, do different things in case of looping
	if( KeyIndex2 == RawTrack.KeyTimes.Num() )
	{
		// If looping, interpolate between last and first frame
		if( bLooping )
		{
			KeyIndex2 = 0;
		}
		// If not looping - hold the last frame.
		else
		{
			KeyIndex2 = KeyIndex1;
		}
	}

	const INT PosKeyIndex1 = ::Min(KeyIndex1, RawTrack.PosKeys.Num()-1);
	const INT RotKeyIndex1 = ::Min(KeyIndex1, RawTrack.RotKeys.Num()-1);
	const INT PosKeyIndex2 = ::Min(KeyIndex2, RawTrack.PosKeys.Num()-1);
	const INT RotKeyIndex2 = ::Min(KeyIndex2, RawTrack.RotKeys.Num()-1);

	OutAtom.Translation = Lerp(RawTrack.PosKeys(PosKeyIndex1), RawTrack.PosKeys(PosKeyIndex2), Alpha);

#if !USE_SLERP
	// Fast linear quaternion interpolation.
	// To ensure the 'shortest route', we make sure the dot product between the two keys is positive.
	if( (RawTrack.RotKeys(RotKeyIndex1) | RawTrack.RotKeys(RotKeyIndex2)) < 0.f )
	{
		// To clarify the code here: a slight optimization of inverting the parametric variable as opposed to the quaternion.
		OutAtom.Rotation = (RawTrack.RotKeys(RotKeyIndex1) * (1.f-Alpha)) + (RawTrack.RotKeys(RotKeyIndex2) * -Alpha);
	}
	else
	{
		OutAtom.Rotation = (RawTrack.RotKeys(RotKeyIndex1) * (1.f-Alpha)) + (RawTrack.RotKeys(RotKeyIndex2) * Alpha);
		OutAtom.Rotation = (RawTrack.RotKeys(RotKeyIndex1) * (1.f-Alpha)) + (RawTrack.RotKeys(RotKeyIndex2) * Alpha);
	}
#else
	OutAtom.Rotation = SlerpQuat( RawTrack.RotKeys(RotKeyIndex1), RawTrack.RotKeys(RotKeyIndex2), Alpha );
#endif
	OutAtom.Rotation.Normalize();
}

IMPLEMENT_COMPARE_CONSTREF( FAnimNotifyEvent, UnSkeletalAnim, 
{
	if		(A.Time > B.Time)	return 1;
	else if	(A.Time < B.Time)	return -1;
	else						return 0;
} 
)

/**
 * Sort the Notifies array by time, earliest first.
 */
void UAnimSequence::SortNotifies()
{
	Sort<USE_COMPARE_CONSTREF(FAnimNotifyEvent,UnSkeletalAnim)>(&Notifies(0),Notifies.Num());
}

/**
 * @return		A reference to the AnimSet this sequence belongs to.
 */
UAnimSet* UAnimSequence::GetAnimSet() const
{
	return CastChecked<UAnimSet>( GetOuter() );
}

/**
 * Crops the raw anim data either from Start to CurrentTime or CurrentTime to End depending on 
 * value of bFromStart.  Can't be called against cooked data.
 *
 * @param	CurrentTime		marker for cropping (either beginning or end)
 * @param	bFromStart		whether marker is begin or end marker
 * @return					TRUE if the operation was successful.
 */
UBOOL UAnimSequence::CropRawAnimData( FLOAT CurrentTime, UBOOL bFromStart )
{
	// Can't crop cooked animations.
	const UPackage* Package = GetOutermost();
	if( Package->PackageFlags & PKG_Cooked )
	{
		return FALSE;
	}

	// Calculate new sequence length
	FLOAT NewLength = bFromStart ? (SequenceLength - CurrentTime) : CurrentTime;

	// Iterate over tracks removing keys from each one.
	for(INT i=0; i<RawAnimData.Num(); i++)
	{
		FRawAnimSequenceTrack& RawTrack = RawAnimData(i);
		INT NumKeys = Max3( RawTrack.KeyTimes.Num(), RawTrack.PosKeys.Num(), RawTrack.RotKeys.Num() );

		check(RawTrack.KeyTimes.Num() == NumKeys);
		check(RawTrack.PosKeys.Num() == 1 || RawTrack.PosKeys.Num() == NumKeys);
		check(RawTrack.RotKeys.Num() == 1 || RawTrack.RotKeys.Num() == NumKeys);

		if(NumKeys > 1)
		{
			// Find the right key to cut at.
			// This assumes that all keys are equally spaced (ie. won't work if we have dropped unimportant frames etc).
			FLOAT FrameInterval = RawTrack.KeyTimes(1) - RawTrack.KeyTimes(0);
			INT KeyIndex = appCeil( CurrentTime/FrameInterval );
			KeyIndex = Clamp<INT>(KeyIndex, 1, RawTrack.KeyTimes.Num()-1); // Ensure KeyIndex is in range.

			// KeyIndex should now be the very next frame after the desired Time.
			
			if(bFromStart)
			{
				if(RawTrack.PosKeys.Num() > 1)
				{
					RawTrack.PosKeys.Remove(0, KeyIndex);
				}
				if(RawTrack.RotKeys.Num() > 1)
				{
					RawTrack.RotKeys.Remove(0, KeyIndex);
				}
				RawTrack.KeyTimes.Remove(0, KeyIndex);
			}
			else
			{
				if(RawTrack.PosKeys.Num() > 1)
				{
					check(RawTrack.PosKeys.Num() == NumKeys);
					RawTrack.PosKeys.Remove(KeyIndex, NumKeys - KeyIndex);
				}
				if(RawTrack.RotKeys.Num() > 1)
				{
					check(RawTrack.RotKeys.Num() == NumKeys);
					RawTrack.RotKeys.Remove(KeyIndex, NumKeys - KeyIndex);
				}
				RawTrack.KeyTimes.Remove(KeyIndex, NumKeys - KeyIndex);
			}

			// Recalculate key times
			for(INT KeyIdx=0; KeyIdx < RawTrack.KeyTimes.Num(); KeyIdx++)
			{
				RawTrack.KeyTimes(KeyIdx) = ((FLOAT)KeyIdx/(FLOAT)RawTrack.KeyTimes.Num()) * NewLength;
			}

			// Update NumFrames below to reflect actual number of keys.
			const INT NumKeys = RawTrack.KeyTimes.Num();
			if( NumKeys > 1 )
			{
				NumFrames = NumKeys;
			}
		}
	}

	SequenceLength = NewLength;

	MarkPackageDirty();
	return TRUE;
}

/** Clears any data in the AnimSequence, so it can be recycled when importing a new animation with same name over it. */
void UAnimSequence::RecycleAnimSequence()
{
	// Clear RawAnimData
	RawAnimData.Empty();

	// Clear additive animation information
	bIsAdditive = FALSE;
	AdditiveRefPose.Empty();
}

/*-----------------------------------------------------------------------------
	UAnimSet
-----------------------------------------------------------------------------*/

void FAnimSetMeshLinkup::BuildLinkup(USkeletalMesh* InSkelMesh, UAnimSet* InAnimSet)
{
	INT NumBones = InSkelMesh->RefSkeleton.Num();

	BoneToTrackTable.Empty();
	BoneToTrackTable.Add(NumBones);

	BoneUseAnimTranslation.Empty();
	BoneUseAnimTranslation.Add(NumBones);

	ForceUseMeshTranslation.Empty();
	ForceUseMeshTranslation.Add(NumBones);

	// For each bone in skeletal mesh, find which track to pull from in the AnimSet.
	for(INT i=0; i<NumBones; i++)
	{
		FName BoneName = InSkelMesh->RefSkeleton(i).Name;

		// FindTrackWithName will return INDEX_NONE if no track exists.
		BoneToTrackTable(i) = InAnimSet->FindTrackWithName(BoneName);

		// Cache whether to use the translation from this bone or from ref pose.
		BoneUseAnimTranslation(i) = InAnimSet->UseTranslationBoneNames.ContainsItem(BoneName);
		ForceUseMeshTranslation(i) = InAnimSet->ForceMeshTranslationBoneNames.ContainsItem(BoneName);
	}

	// Remember GUID of skeletal mesh.
	check(InSkelMesh->SkelMeshGUID.IsValid());
	SkelMeshLinkupGUID = InSkelMesh->SkelMeshGUID;

#if !FINAL_RELEASE
	TArray<UBOOL> TrackUsed;
	TrackUsed.AddZeroed(InAnimSet->TrackBoneNames.Num());
	const INT AnimLinkupIndex = InAnimSet->GetMeshLinkupIndex( InSkelMesh );
	const FAnimSetMeshLinkup& AnimLinkup = InAnimSet->LinkupCache( AnimLinkupIndex );
	for(INT BoneIndex=0; BoneIndex<NumBones; BoneIndex++)
	{
		const INT TrackIndex = AnimLinkup.BoneToTrackTable(BoneIndex);

		if( TrackIndex == INDEX_NONE )
		{
			continue;
		}

		if( TrackUsed(TrackIndex) )
		{
			warnf(TEXT("%s has multiple bones sharing the same track index!!!"), *InAnimSet->GetFullName());	
			for(INT DupeBoneIndex=0; DupeBoneIndex<NumBones; DupeBoneIndex++)
			{
				const INT DupeTrackIndex = AnimLinkup.BoneToTrackTable(DupeBoneIndex);
				if( DupeTrackIndex == TrackIndex )
				{
					warnf(TEXT(" BoneIndex: %i, BoneName: %s, TrackIndex: %i, TrackBoneName: %s"), DupeBoneIndex, *InSkelMesh->RefSkeleton(DupeBoneIndex).Name.ToString(), DupeTrackIndex, *InAnimSet->TrackBoneNames(DupeTrackIndex).ToString());	
				}
			}
		}

		TrackUsed(TrackIndex) = TRUE;
	}
#endif
}

/**
 * Serialize code for calculating memory usage
 */
void UAnimSet::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );

	if( Ar.IsCountingMemory() )
	{
		//TODO: Account for the following member when calculating memory footprint
		// Ar << LinkupCache;
	}
}

void UAnimSet::PostLoad()
{
	Super::PostLoad();

	// Make sure that AnimSets (and sequences) within level packages are not marked as standalone.
	if(GetOutermost()->ContainsMap() && HasAnyFlags(RF_Standalone))
	{
		ClearFlags(RF_Standalone);

		for(INT i=0; i<Sequences.Num(); i++)
		{
			UAnimSequence* Seq = Sequences(i);
			if(Seq)
			{
				Seq->ClearFlags(RF_Standalone);
			}
		}
	}
}	


/**
 * See if we can play sequences from this AnimSet on the provided SkeletalMesh.
 * Returns true if there is a bone in SkelMesh for every track in the AnimSet,
 * or there is a track of animation for every bone of the SkelMesh.
 * 
 * @param	SkelMesh	SkeletalMesh to compare the AnimSet against.
 * @return				TRUE if animation set can play on supplied SkeletalMesh, FALSE if not.
 */
UBOOL UAnimSet::CanPlayOnSkeletalMesh(USkeletalMesh* SkelMesh) const
{
	// Temporarily allow any animation to play on any AnimSet. 
	// We need a looser metric for matching animation to skeletons. Some 'overlap bone count'?
#if 0
	// This is broken and needs to be looked into.
	// we require at least 10% of tracks matched by skeletal mesh.
	return GetSkeletalMeshMatchRatio(SkelMesh) > 0.1f;
#else
	return TRUE;
#endif
}

FLOAT UAnimSet::GetSkeletalMeshMatchRatio(USkeletalMesh* SkelMesh) const
{
	// First see if there is a bone for all tracks
	INT TracksMatched = 0;
	for(INT i=0; i<TrackBoneNames.Num() ; i++)
	{
		const INT BoneIndex = SkelMesh->MatchRefBone( TrackBoneNames(i) );
		if( BoneIndex != INDEX_NONE )
		{
			++TracksMatched;
		}
	}

	INT BonesMatched = 0;
	for(INT i=0; i<SkelMesh->RefSkeleton.Num() ; i++)
	{
		const INT TrackIndex = FindTrackWithName( SkelMesh->RefSkeleton(i).Name );
		if( TrackIndex != INDEX_NONE )
		{
			++BonesMatched;
		}
	}

	// If we can't match any bones, then this is definitely not compatible.
	if( BonesMatched == 0 || TracksMatched == 0 )
	{
		return 0.f;
	}

	// return how many of the animation tracks were matched by that mesh
	return (FLOAT)TracksMatched / FLOAT(TrackBoneNames.Num());
}

/**
 * Returns the AnimSequence with the specified name in this set.
 * 
 * @param		SequenceName	Name of sequence to find.
 * @return						Pointer to AnimSequence with desired name, or NULL if sequence was not found.
 */
UAnimSequence* UAnimSet::FindAnimSequence(FName SequenceName) const
{
	if( SequenceName != NAME_None )
	{
		for(INT i=0; i<Sequences.Num(); i++)
		{
			if( Sequences(i)->SequenceName == SequenceName )
			{
				return Sequences(i);
			}
		}
	}

	return NULL;
}

/**
 * Find a mesh linkup table (mapping of sequence tracks to bone indices) for a particular SkeletalMesh
 * If one does not already exist, create it now.
 *
 * @param InSkelMesh SkeletalMesh to look for linkup with.
 *
 * @return Index of Linkup between mesh and animation set.
 */
INT UAnimSet::GetMeshLinkupIndex(USkeletalMesh* InSkelMesh)
{
	// First, see if we have a cached link-up between this animation and the given skeletal mesh.
	check(InSkelMesh);
	check(InSkelMesh->SkelMeshGUID.IsValid());
	for(INT i=0; i<LinkupCache.Num(); i++)
	{
		if( LinkupCache(i).SkelMeshLinkupGUID == InSkelMesh->SkelMeshGUID )
		{
			return i;
		}
	}

	// No linkup found - so create one here and add to cache.
	const INT NewLinkupIndex = LinkupCache.AddZeroed();
	FAnimSetMeshLinkup* NewLinkup = &LinkupCache(NewLinkupIndex);
	NewLinkup->BuildLinkup(InSkelMesh, this);

	return NewLinkupIndex;
}

/**
 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
 *
 * @return size of resource as to be displayed to artists/ LDs in the Editor.
 */
INT UAnimSet::GetResourceSize()
{
	FArchiveCountMem CountBytesSize( this );
	INT ResourceSize = CountBytesSize.GetNum();
	for( INT i=0; i<Sequences.Num(); i++ )
	{
		ResourceSize += Sequences(i)->GetResourceSize();
	}
	return ResourceSize;
}

/**
 * Clears all sequences and resets the TrackBoneNames table.
 */
void UAnimSet::ResetAnimSet()
{
	Sequences.Empty();
	TrackBoneNames.Empty();
	LinkupCache.Empty();

	// We need to re-init any skeleltal mesh components now, because they might still have references to linkups in this set.
	for(TObjectIterator<USkeletalMeshComponent> It;It;++It)
	{
		USkeletalMeshComponent* SkelComp = *It;
		if(!SkelComp->IsPendingKill() && !SkelComp->IsTemplate())
		{
			SkelComp->InitAnimTree();
		}
	}
}

/** Util that find all AnimSets and flushes their LinkupCache, then calls InitAnimTree on all SkeletalMeshComponents. */
void UAnimSet::ClearAllAnimSetLinkupCaches()
{
	DOUBLE Start = appSeconds();

	TArray<UAnimSet*> AnimSets;
	TArray<USkeletalMeshComponent*> SkelComps;
	// Find all AnimSets and SkeletalMeshComponents (just do one iterator)
	for(TObjectIterator<UObject> It;It;++It)
	{
		UAnimSet* AnimSet = Cast<UAnimSet>(*It);
		if(AnimSet && !AnimSet->IsPendingKill() && !AnimSet->IsTemplate())
		{
			AnimSets.AddItem(AnimSet);
		}

		USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(*It);
		if(SkelComp && !SkelComp->IsPendingKill() && !SkelComp->IsTemplate())
		{
			SkelComps.AddItem(SkelComp);
		}
	}

	// For all AnimSets, empty their linkup cache
	for(INT i=0; i<AnimSets.Num(); i++)
	{
		AnimSets(i)->LinkupCache.Empty();
	}

	// For all SkeletalMeshComponents, force anims to be re-bound
	for(INT i=0; i<SkelComps.Num(); i++)
	{
		SkelComps(i)->UpdateAnimations();
	}

	debugf(TEXT("ClearAllAnimSetLinkupCaches - Took %3.2fms"), (appSeconds() - Start)*1000.f);
}

/*-----------------------------------------------------------------------------
	AnimNotify subclasses
-----------------------------------------------------------------------------*/

//
// UAnimNotify_Sound
//
void UAnimNotify_Sound::Notify( UAnimNodeSequence* NodeSeq )
{
	USkeletalMeshComponent* SkelComp = NodeSeq->SkelComponent;
	check( SkelComp );

	AActor* Owner = SkelComp->GetOwner();
	const UBOOL bIsOwnerHidden = Owner != NULL && Owner->bHidden;

	if( !bIgnoreIfActorHidden || !bIsOwnerHidden )
	{
		UAudioComponent* AudioComponent = UAudioDevice::CreateComponent( SoundCue, SkelComp->GetScene(), Owner, 0 );
		if( AudioComponent )
		{
			if( BoneName != NAME_None )
			{
				AudioComponent->bUseOwnerLocation	= 0;
				AudioComponent->Location			= SkelComp->GetBoneLocation( BoneName );
			}
			else if( !(bFollowActor && Owner) )
			{	
				AudioComponent->bUseOwnerLocation	= 0;
				AudioComponent->Location			= SkelComp->LocalToWorld.GetOrigin();
			}

			AudioComponent->bAllowSpatialization	&= GIsGame;
			AudioComponent->bIsUISound				= !GIsGame;
			AudioComponent->bAutoDestroy			= 1;
			AudioComponent->SubtitlePriority		= SUBTITLE_PRIORITY_ANIMNOTIFY;
			AudioComponent->Play();
		}
	}
}
IMPLEMENT_CLASS(UAnimNotify_Sound);

//
// UAnimNotify_Script
//
void UAnimNotify_Script::Notify( UAnimNodeSequence* NodeSeq )
{
	AActor* Owner = NodeSeq->SkelComponent->GetOwner();
	if( Owner && NotifyName != NAME_None )
	{
		if( !GWorld->HasBegunPlay() )
		{
			debugf( NAME_Log, TEXT("Editor: skipping AnimNotify_Script %s"), *NotifyName.ToString() );
		}
		else
		{
			UFunction* Function = Owner->FindFunction( NotifyName );
			if( Function )
			{
				if( Function->FunctionFlags & FUNC_Delegate )
				{
					UDelegateProperty* DelegateProperty = FindField<UDelegateProperty>( Owner->GetClass(), *FString::Printf(TEXT("__%s__Delegate"),*NotifyName.ToString()) );
					FScriptDelegate* ScriptDelegate = (FScriptDelegate*)((BYTE*)Owner + DelegateProperty->Offset);
                    Owner->ProcessDelegate( NotifyName, ScriptDelegate, NULL );
				}
				else
				{
					Owner->ProcessEvent( Function, NULL );								
				}
			}
		}
	}

}
IMPLEMENT_CLASS(UAnimNotify_Script);

//
// UAnimNotify_Scripted
//
void UAnimNotify_Scripted::Notify( UAnimNodeSequence* NodeSeq )
{
	AActor* Owner = NodeSeq->SkelComponent->GetOwner();
	if( Owner )
	{
		if( !GWorld->HasBegunPlay() )
		{
			debugf( NAME_Log, TEXT("Editor: skipping AnimNotify_Scripted %s"), *GetName() );
		}
		else
		{
			eventNotify( Owner, NodeSeq );
		}
	}
}
IMPLEMENT_CLASS(UAnimNotify_Scripted);

//
// UAnimNotify_Footstep
//
void UAnimNotify_Footstep::Notify( UAnimNodeSequence* NodeSeq )
{
	AActor* Owner = (NodeSeq && NodeSeq->SkelComponent) ? NodeSeq->SkelComponent->GetOwner() : NULL;

	if( !Owner )
	{
		// This should not be the case in the game, so generate a warning.
		if( GWorld->HasBegunPlay() )
		{
			debugf(TEXT("FOOTSTEP no owner"));
		}
	}
	else
	{
		//debugf(TEXT("FOOTSTEP for %s"),*Owner->GetName());

		// Act on footstep...  FootDown signifies which paw hits earth 0=left, 1=right, 2=left-hindleg etc.
		if( Owner->GetAPawn() )
		{
			Owner->GetAPawn()->eventPlayFootStepSound(FootDown);
		}
	}
}
IMPLEMENT_CLASS(UAnimNotify_Footstep);


//
// AnimNotify_CameraEffect
//
void UAnimNotify_CameraEffect::Notify( UAnimNodeSequence* NodeSeq )
{
	AActor* Owner = NodeSeq->SkelComponent->GetOwner();
	if( Owner )
	{
		if( !GWorld->HasBegunPlay() )
		{
			debugf( NAME_Log, TEXT("Editor: skipping AnimNotify_CameraEffect %s"), *GetName() );
		}
		else
		{
			if( ( Owner->GetAPawn() != NULL )
				&& ( Cast<APlayerController>(Owner->GetAPawn()->Controller) != NULL )
				)
			{
				Cast<APlayerController>(Owner->GetAPawn()->Controller)->eventClientSpawnCameraLensEffect( CameraLensEffect );
			}
		}
	}
}
IMPLEMENT_CLASS(UAnimNotify_CameraEffect);


//
// AnimNotify_PlayParticleEffect
//
void UAnimNotify_PlayParticleEffect::Notify( UAnimNodeSequence* NodeSeq )
{
	AActor* Owner = NodeSeq->SkelComponent->GetOwner();

	// try owner first
	if( Owner )
	{
		if( !GWorld->HasBegunPlay() )
		{
			debugf( NAME_Log, TEXT("Editor: skipping AnimNotify_PlayParticleEffect %s"), *GetName() );
		}
		else
		{
			Owner->eventPlayParticleEffect( this );
		}
	}
	
    // go ahead and send to the mesh now also
	if( NodeSeq->SkelComponent != NULL )
	{
		if( !GWorld->HasBegunPlay() )
		{
			debugf( NAME_Log, TEXT("Editor: skipping AnimNotify_PlayParticleEffect %s"), *GetName() );
		}
		else
		{
			NodeSeq->SkelComponent->eventPlayParticleEffect( this );
		}
	}
}
IMPLEMENT_CLASS(UAnimNotify_PlayParticleEffect);


//
// AnimNotify_Rumble
//
void UAnimNotify_Rumble::Notify( UAnimNodeSequence* NodeSeq )
{
	AActor* Owner = NodeSeq->SkelComponent->GetOwner();
	if( Owner )
	{
		if( !GWorld->HasBegunPlay() )
		{
			debugf( NAME_Log, TEXT("Editor: skipping AnimNotify_Rumble %s"), *GetName() );
		}
		else
		{
			if( ( Owner->GetAPawn() != NULL )
				&& ( Cast<APlayerController>(Owner->GetAPawn()->Controller) != NULL )
				)
			{
				Cast<APlayerController>(Owner->GetAPawn()->Controller)->eventPlayRumble( this );
			}
		}
	}
}
IMPLEMENT_CLASS(UAnimNotify_Rumble);
IMPLEMENT_CLASS(UWaveFormBase);


////////////

void GatherAnimSequenceStats(FOutputDevice& Ar)
{
	INT AnimationKeyFormatNum[AKF_MAX];
	INT TranslationCompressionFormatNum[ACF_MAX];
	INT RotationCompressionFormatNum[ACF_MAX];
	appMemzero( AnimationKeyFormatNum, AKF_MAX * sizeof(INT) );
	appMemzero( TranslationCompressionFormatNum, ACF_MAX * sizeof(INT) );
	appMemzero( RotationCompressionFormatNum, ACF_MAX * sizeof(INT) );

	Ar.Logf( TEXT(" %60s %s %s"), TEXT("Sequence Name"), TEXT("NTT/NRT NT1/NR1"), TEXT("TotTrnKys TotRotKys") );
	INT GlobalNumTransTracks = 0;
	INT GlobalNumRotTracks = 0;
	INT GlobalNumTransTracksWithOneKey = 0;
	INT GlobalNumRotTracksWithOneKey = 0;
	INT GlobalApproxCompressedSize = 0;
	INT GlobalApproxKeyDataSize = 0;
	for( TObjectIterator<UAnimSequence> It; It; ++It )
	{
		UAnimSequence* Seq		= *It;

#if USE_ANIMATION_CODEC_INTERFACE

		INT NumTransTracks = 0;
		INT NumRotTracks = 0;
		INT TotalNumTransKeys = 0;
		INT TotalNumRotKeys = 0;
		INT TranslationKeySize = 0;
		INT RotationKeySize = 0;
		INT NumTransTracksWithOneKey = 0;
		INT NumRotTracksWithOneKey = 0;

		AnimationFormat_GetStats(	Seq, 
						  			NumTransTracks,
									NumRotTracks,
						  			TotalNumTransKeys,
									TotalNumRotKeys,
									TranslationKeySize,
									RotationKeySize,
									NumTransTracksWithOneKey,
									NumRotTracksWithOneKey);

		GlobalNumTransTracks += NumTransTracks;
		GlobalNumRotTracks += NumRotTracks;
		GlobalNumTransTracksWithOneKey += NumTransTracksWithOneKey;
		GlobalNumRotTracksWithOneKey += NumRotTracksWithOneKey;

		GlobalApproxCompressedSize += Seq->GetApproxCompressedSize();
		GlobalApproxKeyDataSize += (TotalNumTransKeys * TranslationKeySize) + (TotalNumRotKeys * RotationKeySize);
#else
		const INT TransStride	= GetCompressedTranslationStride(Seq);
		const INT RotStride		= GetCompressedRotationStride(Seq);
		const INT TransNum		= CompressedTranslationNum[Seq->TranslationCompressionFormat];
		const INT RotNum		= CompressedRotationNum[Seq->RotationCompressionFormat];

		// Track compression format usage.
		++TranslationCompressionFormatNum[Seq->TranslationCompressionFormat];
		++RotationCompressionFormatNum[Seq->RotationCompressionFormat];

		// Track number of tracks.
		const INT NumTransTracks	= Seq->CompressedTrackOffsets.Num()/4;
		const INT NumRotTracks		= Seq->CompressedTrackOffsets.Num()/4;
		GlobalNumTransTracks		+= NumTransTracks;
		GlobalNumRotTracks			+= NumRotTracks;

		// Track total number of keys.
		INT TotalNumTransKeys = 0;
		INT TotalNumRotKeys = 0;

		// Track number of tracks with a single key.
		INT NumTransTracksWithOneKey = 0;
		INT NumRotTracksWithOneKey = 0;

		// Translation.
		for ( INT TrackIndex = 0 ; TrackIndex < NumTransTracks ; ++TrackIndex )
		{
			const INT NumTransKeys = Seq->CompressedTrackOffsets(TrackIndex*4+1);
			TotalNumTransKeys += NumTransKeys;
			if ( NumTransKeys == 1 )
			{
				++NumTransTracksWithOneKey;
			}
		}

		// Rotation.
		for ( INT TrackIndex = 0 ; TrackIndex < NumRotTracks ; ++TrackIndex )
		{
			const INT NumRotKeys = Seq->CompressedTrackOffsets(TrackIndex*4+3);
			TotalNumRotKeys += NumRotKeys;
			if ( NumRotKeys == 1 )
			{
				++NumRotTracksWithOneKey;
			}
		}

		GlobalNumTransTracksWithOneKey += NumTransTracksWithOneKey;
		GlobalNumRotTracksWithOneKey += NumRotTracksWithOneKey;
#endif
		Ar.Logf(TEXT(" %60s % 3i/%3i %3i/%3i % 10i %10i"),
			*Seq->SequenceName.ToString(), NumTransTracks, NumRotTracks, NumTransTracksWithOneKey, NumRotTracksWithOneKey, TotalNumTransKeys, TotalNumRotKeys );
	}
	Ar.Logf( TEXT("======================================================================") );
	Ar.Logf( TEXT("Total Num Tracks: %i trans, %i rot, %i trans1, %i rot1"), GlobalNumTransTracks, GlobalNumRotTracks, GlobalNumTransTracksWithOneKey, GlobalNumRotTracksWithOneKey );
	Ar.Logf( TEXT("Approx Compressed Memory: %i bytes"), GlobalApproxCompressedSize);
	Ar.Logf( TEXT("Approx Key Data Memory: %i bytes"), GlobalApproxKeyDataSize);
}
