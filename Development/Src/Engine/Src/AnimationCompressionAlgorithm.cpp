/*=============================================================================
	AnimationCompressionAlgorithm.cpp: Skeletal mesh animation compression.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"
#include "AnimationCompression.h"
#include "AnimationUtils.h"
#include "FloatPacker.h"
#include "AnimationEncodingFormat.h"

IMPLEMENT_CLASS(UAnimationCompressionAlgorithm);

// Writes the specified data to Seq->CompresedByteStream with four-byte alignment.
#define AC_UnalignedWriteToStream( Src, Len )										\
	{																				\
		const INT Ofs = Seq->CompressedByteStream.Add( Len );						\
		appMemcpy( Seq->CompressedByteStream.GetTypedData()+Ofs, (Src), (Len) );	\
	}

static const BYTE PadSentinel = 85; //(1<<1)+(1<<3)+(1<<5)+(1<<7)

static void PackQuaternionToStream(
	UAnimSequence* Seq, 
	AnimationCompressionFormat TargetRotationFormat, 
	const FQuat& Quat,
	const FLOAT* Mins,
	const FLOAT* Ranges)
{
	if ( TargetRotationFormat == ACF_None )
	{
		AC_UnalignedWriteToStream( &Quat, sizeof(FQuat) );
	}
	else if ( TargetRotationFormat == ACF_Float96NoW )
	{
		const FQuatFloat96NoW QuatFloat96NoW( Quat );
		AC_UnalignedWriteToStream( &QuatFloat96NoW, sizeof(FQuatFloat96NoW) );
	}
	else if ( TargetRotationFormat == ACF_Fixed32NoW )
	{
		const FQuatFixed32NoW QuatFixed32NoW( Quat );
		AC_UnalignedWriteToStream( &QuatFixed32NoW, sizeof(FQuatFixed32NoW) );
	}
	else if ( TargetRotationFormat == ACF_Fixed48NoW )
	{
		const FQuatFixed48NoW QuatFixed48NoW( Quat );
		AC_UnalignedWriteToStream( &QuatFixed48NoW, sizeof(FQuatFixed48NoW) );
	}
	else if ( TargetRotationFormat == ACF_IntervalFixed32NoW )
	{
		const FQuatIntervalFixed32NoW QuatIntervalFixed32NoW( Quat, Mins, Ranges );
		AC_UnalignedWriteToStream( &QuatIntervalFixed32NoW, sizeof(FQuatIntervalFixed32NoW) );
	}
	else if ( TargetRotationFormat == ACF_Float32NoW )
	{
		const FQuatFloat32NoW QuatFloat32NoW( Quat );
		AC_UnalignedWriteToStream( &QuatFloat32NoW, sizeof(FQuatFloat32NoW) );
	}
	else if ( TargetRotationFormat == ACF_Fixed48Max )
	{
		const FQuatFixed48Max QuatFixed48Max( Quat );
		AC_UnalignedWriteToStream( &QuatFixed48Max, sizeof(FQuatFixed48Max) );
	}
}

/**
 * Common compression utility to walk an array of rotation tracks and enforce
 * that all adjacent rotation keys are represented by shortest-arc quaternion pairs.
 *
 * @param	RotationData	Array of rotation track elements to reduce.
 */
void UAnimationCompressionAlgorithm::PrecalculateShortestQuaternionRoutes(
	TArray<struct FRotationTrack>& RotationData)
{
	const INT NumTracks = RotationData.Num();
	for ( INT TrackIndex = 0 ; TrackIndex < NumTracks ; ++TrackIndex )
	{
		FRotationTrack& SrcRot	= RotationData(TrackIndex);
		for ( INT KeyIndex = 1 ; KeyIndex < SrcRot.RotKeys.Num() ; ++KeyIndex )
		{
			const FQuat& R0 = SrcRot.RotKeys(KeyIndex-1);
			FQuat& R1 = SrcRot.RotKeys(KeyIndex);
			
			if( (R0 | R1) < 0.f )
			{
				// invert R1 so that R0|R1 will always be >=0.f
				// making the delta between them the shortest possible route
				R1 = (R1 * -1);
			}
		}
	}
}

void PadByteStream(TArrayNoInit<BYTE>& CompressedByteStream, const INT Alignment, BYTE sentinel)
{
	INT Pad = Align( CompressedByteStream.Num(), 4 ) - CompressedByteStream.Num();
	for ( INT i = 0 ; i < Pad ; ++i )
	{
		CompressedByteStream.AddItem(sentinel);
	}
}

/**
 * Encodes individual key arrays into an AnimSequence using the desired bit packing formats.
 *
 * @param	Seq							Pointer to an Animation Sequence which will contain the bit-packed data .
 * @param	TargetTranslationFormat		The format to use when encoding translation keys.
 * @param	TargetRotationFormat		The format to use when encoding rotation keys.
 * @param	TranslationData				Translation Tracks to bit-pack into the Animation Sequence.
 * @param	RotationData				Rotation Tracks to bit-pack into the Animation Sequence.
 * @param	IncludeKeyTable				TRUE if the compressed data should also contain a table of frame indices for each key. (required by some codecs)
 */
void UAnimationCompressionAlgorithm::BitwiseCompressAnimationTracks(
	UAnimSequence* Seq, 
	AnimationCompressionFormat TargetTranslationFormat, 
	AnimationCompressionFormat TargetRotationFormat,
	const TArray<FTranslationTrack>& TranslationData,
	const TArray<FRotationTrack>& RotationData,
	UBOOL IncludeKeyTable)
{
	// Ensure supported compression formats.
	UBOOL bInvalidCompressionFormat = FALSE;
	if( !(TargetTranslationFormat == ACF_None) )
	{
		appMsgf( AMT_OK, TEXT("Unknown or unsupported translation compression format (%i)"), (INT)TargetTranslationFormat );
		bInvalidCompressionFormat = TRUE;
	}
	if ( !(TargetRotationFormat >= ACF_None && TargetRotationFormat < ACF_MAX) )
	{
		appMsgf( AMT_OK, TEXT("Unknown or unsupported rotation compression format (%i)"), (INT)TargetRotationFormat );
		bInvalidCompressionFormat = TRUE;
	}
	if ( bInvalidCompressionFormat )
	{
		Seq->TranslationCompressionFormat	= ACF_None;
		Seq->RotationCompressionFormat		= ACF_None;
		Seq->CompressedTrackOffsets.Empty();
		Seq->CompressedByteStream.Empty();
	}
	else
	{
		Seq->RotationCompressionFormat		= TargetRotationFormat;
		Seq->TranslationCompressionFormat	= TargetTranslationFormat;

		check( TranslationData.Num() == RotationData.Num() );
		const INT NumTracks = RotationData.Num();

		if ( NumTracks == 0 )
		{
			debugf( NAME_Warning, TEXT("When compressing %s: no key-reduced data"), *Seq->SequenceName.ToString() );
		}

		Seq->CompressedTrackOffsets.Empty( NumTracks*4 );
		Seq->CompressedTrackOffsets.Add( NumTracks*4 );
		Seq->CompressedByteStream.Empty();

		for ( INT TrackIndex = 0 ; TrackIndex < NumTracks ; ++TrackIndex )
		{
			// Translation data.
			const FTranslationTrack& SrcTrans	= TranslationData(TrackIndex);

			const INT OffsetTrans				= Seq->CompressedByteStream.Num();
			const INT NumKeysTrans				= SrcTrans.PosKeys.Num();

			// Warn on empty data.
			if ( NumKeysTrans == 0 )
			{
				debugf( NAME_Warning, TEXT("When compressing %s track %i: no translation keys"), *Seq->SequenceName.ToString(), TrackIndex );
			}

			checkMsg( (OffsetTrans % 4) == 0, "CompressedByteStream not aligned to four bytes" );
			Seq->CompressedTrackOffsets(TrackIndex*4) = OffsetTrans;
			Seq->CompressedTrackOffsets(TrackIndex*4+1) = NumKeysTrans;


			for ( INT KeyIndex = 0 ; KeyIndex < NumKeysTrans ; ++KeyIndex )
			{
				// A translation track of n keys is packed as n uncompressed float[3].
				AC_UnalignedWriteToStream( &(SrcTrans.PosKeys(KeyIndex)), sizeof(FVector) );
			}

			if (NumKeysTrans > 1)
			{
				if (IncludeKeyTable)
				{
					// Align to four bytes.
					PadByteStream(Seq->CompressedByteStream, 4, PadSentinel );

					// write the key table
					const INT NumFrames = Seq->NumFrames;
					const INT LastFrame = Seq->NumFrames-1;
					const size_t FrameSize = Seq->NumFrames > 0xff ? sizeof(WORD) : sizeof(BYTE);
					const FLOAT FrameRate = NumFrames / Seq->SequenceLength;

					const INT TableSize= NumKeysTrans*FrameSize;
					const INT TableDwords= (TableSize+3)>>2;
					const INT StartingOffset = Seq->CompressedByteStream.Num();

					for ( INT KeyIndex = 0 ; KeyIndex < NumKeysTrans ; ++KeyIndex )
					{
						// write the frame values for each key
						FLOAT KeyTime= SrcTrans.Times(KeyIndex);
						FLOAT FrameTime = KeyTime * FrameRate;
						INT FrameIndex= Clamp(appTrunc(FrameTime), 0, LastFrame);
						AC_UnalignedWriteToStream( &FrameIndex, FrameSize );
					}

					// Align to four bytes. Padding with 0's to round out the key table
					PadByteStream(Seq->CompressedByteStream, 4, 0 );

					const INT EndingOffset = Seq->CompressedByteStream.Num();
					check((EndingOffset - StartingOffset) == (TableDwords*4));

				}
			}

			// Align to four bytes.
			PadByteStream(Seq->CompressedByteStream, 4, PadSentinel );

			// Compress rotation data.
			const FRotationTrack& SrcRot	= RotationData(TrackIndex);
			const INT OffsetRot				= Seq->CompressedByteStream.Num();
			const INT NumKeysRot			= SrcRot.RotKeys.Num();

			checkMsg( (OffsetRot % 4) == 0, "CompressedByteStream not aligned to four bytes" );
			Seq->CompressedTrackOffsets(TrackIndex*4+2) = OffsetRot;
			Seq->CompressedTrackOffsets(TrackIndex*4+3) = NumKeysRot;

			if ( NumKeysRot > 1 )
			{

				// For a rotation track of n>1 keys, the first 24 bytes are reserved for compression info
				// (eg Fixed32 stores float Mins[3]; float Ranges[3]), followed by n elements of the compressed type.
				// Compute Mins and Ranges for rotation X,Y,Zs.
				FLOAT MinX = 1.f;
				FLOAT MinY = 1.f;
				FLOAT MinZ = 1.f;
				FLOAT MaxX = -1.f;
				FLOAT MaxY = -1.f;
				FLOAT MaxZ = -1.f;
				for ( INT KeyIndex = 0 ; KeyIndex < SrcRot.RotKeys.Num() ; ++KeyIndex )
				{
					FQuat Quat( SrcRot.RotKeys(KeyIndex) );
					if ( Quat.W < 0.f )
					{
						Quat.X = -Quat.X;
						Quat.Y = -Quat.Y;
						Quat.Z = -Quat.Z;
						Quat.W = -Quat.W;
					}
					Quat.Normalize();

					MinX = ::Min( MinX, Quat.X );
					MaxX = ::Max( MaxX, Quat.X );
					MinY = ::Min( MinY, Quat.Y );
					MaxY = ::Max( MaxY, Quat.Y );
					MinZ = ::Min( MinZ, Quat.Z );
					MaxZ = ::Max( MaxZ, Quat.Z );
				}
				const FLOAT Mins[3]	= { MinX,		MinY,		MinZ };
				FLOAT Ranges[3]		= { MaxX-MinX,	MaxY-MinY,	MaxZ-MinZ };
				if ( Ranges[0] == 0.f ) { Ranges[0] = 1.f; }
				if ( Ranges[1] == 0.f ) { Ranges[1] = 1.f; }
				if ( Ranges[2] == 0.f ) { Ranges[2] = 1.f; }

				AC_UnalignedWriteToStream( Mins, sizeof(FLOAT)*3 );
				AC_UnalignedWriteToStream( Ranges, sizeof(FLOAT)*3 );

				// n elements of the compressed type.
				for ( INT KeyIndex = 0 ; KeyIndex < SrcRot.RotKeys.Num() ; ++KeyIndex )
				{
					const FQuat& Quat = SrcRot.RotKeys(KeyIndex);
					PackQuaternionToStream(Seq, TargetRotationFormat, Quat, Mins, Ranges);
				}

				// n elements of frame indices
				if (IncludeKeyTable)
				{
					// Align to four bytes.
					PadByteStream(Seq->CompressedByteStream, 4, PadSentinel );

					// write the key table
					const INT NumFrames = Seq->NumFrames;
					const INT LastFrame= Seq->NumFrames-1;
					const size_t FrameSize= Seq->NumFrames > 0xff ? sizeof(WORD) : sizeof(BYTE);
					const FLOAT FrameRate = NumFrames / Seq->SequenceLength;

					const INT TableSize= NumKeysRot*FrameSize;
					const INT TableDwords= (TableSize+3)>>2;
					const INT StartingOffset = Seq->CompressedByteStream.Num();

					for ( INT KeyIndex = 0 ; KeyIndex < NumKeysRot ; ++KeyIndex )
					{
						// write the frame values for each key
						FLOAT KeyTime= SrcRot.Times(KeyIndex);
						FLOAT FrameTime = KeyTime * FrameRate;
						INT FrameIndex= Clamp(appTrunc(FrameTime), 0, LastFrame);
						AC_UnalignedWriteToStream( &FrameIndex, FrameSize );
					}

					// Align to four bytes. Padding with 0's to round out the key table
					PadByteStream(Seq->CompressedByteStream, 4, 0 );

					const INT EndingOffset = Seq->CompressedByteStream.Num();
					check((EndingOffset - StartingOffset) == (TableDwords*4));

				}
			}
			else if ( NumKeysRot == 1 )
			{
				// For a rotation track of n=1 keys, the single key is packed as an FQuatFloat96NoW.
				const FQuat& Quat = SrcRot.RotKeys(0);
				const FQuatFloat96NoW QuatFloat96NoW( Quat );
				AC_UnalignedWriteToStream( &QuatFloat96NoW, sizeof(FQuatFloat96NoW) );
			}
			else
			{
				debugf( NAME_Warning, TEXT("When compressing %s track %i: no rotation keys"), *Seq->SequenceName.ToString(), TrackIndex );
			}


			// Align to four bytes.
			PadByteStream(Seq->CompressedByteStream, 4, PadSentinel );
		}

		// Trim unused memory.
		Seq->CompressedByteStream.Shrink();
	}

	// We may not have used the key data arrays resident in this sequence,
	// but we should make sure they are empty at this point.
	Seq->TranslationData.Empty();
	Seq->RotationData.Empty();
}

/**
 * Tracks
 */
class FCompressionMemorySummary
{
public:
	FCompressionMemorySummary(UBOOL bInEnabled)
		:	bEnabled( bInEnabled )
		,	TotalRaw( 0 )
		,	TotalBeforeCompressed( 0 )
		,	TotalAfterCompressed( 0 )
		,	ErrorTotal( 0 )
		,	ErrorCount( 0 )
		,	AverageError( 0 )
		,	MaxError( 0 )
		,	MaxErrorTime( 0 )
		,	MaxErrorBone( 0 )
		,	MaxErrorBoneName( NAME_None )
		,	MaxErrorAnimName( NAME_None )
	{
		if ( bEnabled )
		{
			GWarn->BeginSlowTask( TEXT("Compressing animations"), TRUE );
		}
	}

	void GatherPreCompressionStats(UAnimSequence* Seq, INT ProgressNumerator, INT ProgressDenominator)
	{
		if ( bEnabled )
		{
			GWarn->StatusUpdatef( ProgressNumerator,
									ProgressDenominator,
									*FString::Printf( TEXT("Compressing %s (%i/%i)"), *Seq->SequenceName.ToString(), ProgressNumerator, ProgressDenominator ) );

			TotalRaw += Seq->GetApproxRawSize();
			TotalBeforeCompressed += Seq->GetApproxCompressedSize();
		}
	}

	void GatherPostCompressionStats(UAnimSequence* Seq, USkeletalMesh* SkelMesh, TArray<FBoneData>& BoneData)
	{
		if ( bEnabled )
		{
			TotalAfterCompressed += Seq->GetApproxCompressedSize();

			if( SkelMesh != NULL )
			{
				// determine the error added by the compression
				AnimationErrorStats ErrorStats;
				FAnimationUtils::ComputeCompressionError(Seq, SkelMesh, BoneData, ErrorStats);

				ErrorTotal += ErrorStats.AverageError;
				ErrorCount += 1.0f;
				AverageError = ErrorTotal / ErrorCount;

				if (ErrorStats.MaxError > MaxError)
				{
					MaxError = ErrorStats.MaxError;
					MaxErrorTime = ErrorStats.MaxErrorTime;
					MaxErrorBone = ErrorStats.MaxErrorBone;
					MaxErrorAnimName = Seq->SequenceName;
					MaxErrorBoneName = BoneData(ErrorStats.MaxErrorBone).Name;
				}
			}
		}
	}

	~FCompressionMemorySummary()
	{
		if ( bEnabled )
		{
			GWarn->EndSlowTask();

			const FLOAT TotalRawKB					= static_cast<FLOAT>(TotalRaw)/1024.f;
			const FLOAT TotalBeforeCompressedKB		= static_cast<FLOAT>(TotalBeforeCompressed)/1024.f;
			const FLOAT TotalAfterCompressedKB		= static_cast<FLOAT>(TotalAfterCompressed)/1024.f;
			const FLOAT TotalBeforeSavingKB			= TotalRawKB - TotalBeforeCompressedKB;
			const FLOAT TotalAfterSavingKB			= TotalRawKB - TotalAfterCompressedKB;
			const FLOAT OldCompressionRatio			= (TotalBeforeCompressedKB > 0.f) ? (TotalRawKB / TotalBeforeCompressedKB) : 0.f;
			const FLOAT NewCompressionRatio			= (TotalAfterCompressedKB > 0.f) ? (TotalRawKB / TotalAfterCompressedKB) : 0.f;

			appMsgf( AMT_OK,
				TEXT("Raw: %7.2fKB - Compressed: %7.2fKB\nSaving: %7.2fKB (%.2f)\n")
				TEXT("Raw: %7.2fKB - Compressed: %7.2fKB\nSaving: %7.2fKB (%.2f)\n")
				TEXT("\nEnd Effector Translation Added By Compression:\n")
				TEXT("%7.2f avg, %7.2f max\n")
				TEXT("Max occurred in %s, Bone %s(#%i), at Time %7.2f\n"),
				TotalRawKB, TotalBeforeCompressedKB, TotalBeforeSavingKB, OldCompressionRatio,
				TotalRawKB, TotalAfterCompressedKB, TotalAfterSavingKB, NewCompressionRatio,
				AverageError, MaxError,
				*MaxErrorAnimName.ToString(), *MaxErrorBoneName.ToString(), MaxErrorBone, MaxErrorTime);
		}
	}

private:
	UBOOL bEnabled;
	INT TotalRaw;
	INT TotalBeforeCompressed;
	INT TotalAfterCompressed;

	FLOAT ErrorTotal;
	FLOAT ErrorCount;
	FLOAT AverageError;
	FLOAT MaxError;
	FLOAT MaxErrorTime;
	INT MaxErrorBone;
	FName MaxErrorBoneName;
	FName MaxErrorAnimName;
};

/** @return		TRUE if the specified object is in package which has not been cooked. */
static UBOOL InUncookedPackage(const UObject* Obj)
{
	const UPackage* Package = Obj->GetOutermost();
	return ( Package->PackageFlags & PKG_Cooked ) ? FALSE : TRUE;
}

/**
 * Reduce the number of keyframes and bitwise compress the specified sequence.
 *
 * @param	AnimSet		The animset to compress.
 * @param	SkelMesh	The skeletal mesh against which to compress the animation.  Not needed by all compression schemes.
 * @param	bOutput		If FALSE don't generate output or compute memory savings.
 * @return				FALSE if a skeleton was needed by the algorithm but not provided.
 */
UBOOL UAnimationCompressionAlgorithm::Reduce(UAnimSequence* AnimSeq, USkeletalMesh* SkelMesh, UBOOL bOutput)
{
	UBOOL bResult = FALSE;

	const UBOOL bSekeltonExistsIfNeeded = ( SkelMesh || !bNeedsSkeleton );
	if ( bSekeltonExistsIfNeeded && InUncookedPackage(AnimSeq) )
	{
		// Build skeleton metadata to use during the key reduction.
		TArray<FBoneData> BoneData;
		if ( SkelMesh )
		{
			FAnimationUtils::BuildSekeltonMetaData( SkelMesh->RefSkeleton, BoneData );
		}

//		UAnimationCompressionAlgorithm* TrivialKeyRemover =
//			ConstructObject<UAnimationCompressionAlgorithm>( UAnimationCompressionAlgorithm_RemoveTrivialKeys::StaticClass() );

		FCompressionMemorySummary CompressionMemorySummary( bOutput );
		CompressionMemorySummary.GatherPreCompressionStats( AnimSeq, 0, 1 );

		// Remove trivial keys.
//		TrivialKeyRemover->DoReduction( AnimSeq, SkelMesh, BoneData );

		// assume we are using the default decompression method (child classes may override this in DoReduction)
		AnimSeq->KeyEncodingFormat= AKF_ConstantKeyLerp;

		// General key reduction.
		DoReduction( AnimSeq, SkelMesh, BoneData );

#if USE_ANIMATION_CODEC_INTERFACE
		// setup the Codec interfaces
		AnimationFormat_SetInterfaceLinks(*AnimSeq);
#endif

		// Bitwise compression.
//		BitwiseCompress( AnimSeq,
//						static_cast<AnimationCompressionFormat>(TranslationCompressionFormat),
//						static_cast<AnimationCompressionFormat>(RotationCompressionFormat) );

		AnimSeq->CompressionScheme = static_cast<UAnimationCompressionAlgorithm*>( StaticDuplicateObject( this, this, AnimSeq, TEXT("None"), ~RF_RootSet ) );
		AnimSeq->EncodingPkgVersion = CURRENT_ANIMATION_ENCODING_PACKAGE_VERSION;

		AnimSeq->MarkPackageDirty();

		// determine the error added by the compression
		CompressionMemorySummary.GatherPostCompressionStats( AnimSeq, SkelMesh, BoneData );

		bResult = TRUE;
	}

	return bResult;
}

/**
 * Reduce the number of keyframes and bitwise compress all sequences in the specified animation set.
 *
 * @param	AnimSet		The animset to compress.
 * @param	SkelMesh	The skeletal mesh against which to compress the animation.  Not needed by all compression schemes.
 * @param	bOutput		If FALSE don't generate output or compute memory savings.
 * @return				FALSE if a skeleton was needed by the algorithm but not provided.
 */
UBOOL UAnimationCompressionAlgorithm::Reduce(UAnimSet* AnimSet, USkeletalMesh* SkelMesh, UBOOL bOutput)
{
	UBOOL bResult = FALSE;

	const UBOOL bSekeltonExistsIfNeeded = ( SkelMesh || !bNeedsSkeleton );
	if ( bSekeltonExistsIfNeeded && InUncookedPackage(AnimSet) )
	{
		// Build skeleton metadata to use during the key reduction.
		TArray<FBoneData> BoneData;
		if ( SkelMesh )
		{
			FAnimationUtils::BuildSekeltonMetaData( SkelMesh->RefSkeleton, BoneData );
		}

//		UAnimationCompressionAlgorithm* TrivialKeyRemover =
//			ConstructObject<UAnimationCompressionAlgorithm>( UAnimationCompressionAlgorithm_RemoveTrivialKeys::StaticClass() );

		FCompressionMemorySummary CompressionMemorySummary( bOutput );

		for( INT SeqIndex = 0 ; SeqIndex < AnimSet->Sequences.Num() ; ++SeqIndex )
		{
			UAnimSequence* AnimSeq = AnimSet->Sequences(SeqIndex);
			CompressionMemorySummary.GatherPreCompressionStats( AnimSeq, SeqIndex, AnimSet->Sequences.Num() );

			// Remove trivial keys.
//			TrivialKeyRemover->DoReduction( AnimSeq, SkelMesh, BoneData );

			// assume we are using the default decompression method (child classes may override this in DoReduction)
			AnimSeq->KeyEncodingFormat= AKF_ConstantKeyLerp;

			// General key reduction.
			DoReduction( AnimSeq, SkelMesh, BoneData );

#if USE_ANIMATION_CODEC_INTERFACE
			// setup the Codec interfaces
			AnimationFormat_SetInterfaceLinks(*AnimSeq);
#endif

			// Bitwise compression.
//			BitwiseCompress( AnimSeq,
//							static_cast<AnimationCompressionFormat>(TranslationCompressionFormat),
//							static_cast<AnimationCompressionFormat>(RotationCompressionFormat) );

			AnimSeq->CompressionScheme = static_cast<UAnimationCompressionAlgorithm*>( StaticDuplicateObject( this, this, AnimSeq, TEXT("None") ) );
			AnimSeq->EncodingPkgVersion = CURRENT_ANIMATION_ENCODING_PACKAGE_VERSION;


			CompressionMemorySummary.GatherPostCompressionStats( AnimSeq, SkelMesh, BoneData );
		}

		AnimSet->MarkPackageDirty();
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Common compression utility to remove 'redundant' position keys based on the provided delta threshold
 *
 * @param	InputTracks		Array of position track elements to reduce
 * @param	MaxPosDelta		Maximum local-space threshold for stationary motion
 */
void UAnimationCompressionAlgorithm::FilterTrivialPositionKeys(
	TArray<FTranslationTrack>& InputTracks, 
	FLOAT MaxPosDelta)
{
	const INT NumTracks = InputTracks.Num();
	for( INT TrackIndex = 0; TrackIndex < NumTracks; ++TrackIndex )
	{
		FTranslationTrack& Track = InputTracks(TrackIndex);
		const INT KeyCount = Track.Times.Num();
		check( Track.PosKeys.Num() == Track.Times.Num() );

		// Only bother doing anything if we have some keys!
		if(KeyCount > 0)
		{
			const FVector& FirstPos = Track.PosKeys(0);

			UBOOL bFramesIdentical = TRUE;
			for(INT KeyIndex=1; KeyIndex < KeyCount; ++KeyIndex)
			{
				const FVector& ThisPos = Track.PosKeys(KeyIndex);
				if((FirstPos - ThisPos).Size() > MaxPosDelta )
				{
					bFramesIdentical = FALSE;
					break;
				}
			}

			// If all keys are the same, remove all but first frame
			if(bFramesIdentical)
			{
				Track.PosKeys.Remove(1, Track.PosKeys.Num()- 1);
				Track.PosKeys.Shrink();
				Track.Times.Remove(1, Track.Times.Num()- 1);
				Track.Times.Shrink();
				Track.Times(0) = 0.0f;
			}
		}
	}
}

/**
 * Common compression utility to remove 'redundant' rotation keys based on the provided delta threshold
 *
 * @param	InputTracks		Array of rotation track elements to reduce
 * @param	MaxRotDelta		Maximum angle threshold to consider stationary motion
 */
void UAnimationCompressionAlgorithm::FilterTrivialRotationKeys(
	TArray<FRotationTrack>& InputTracks, 
	FLOAT MaxRotDelta)
{
	const INT NumTracks = InputTracks.Num();
	for( INT TrackIndex = 0 ; TrackIndex < NumTracks ; ++TrackIndex )
	{
		FRotationTrack& Track = InputTracks(TrackIndex);
		const INT KeyCount = Track.Times.Num();
		check( Track.RotKeys.Num() == Track.Times.Num() );

		// Only bother doing anything if we have some keys!
		if(KeyCount > 0)
		{
			const FQuat& FirstRot = Track.RotKeys(0);
			UBOOL bFramesIdentical = TRUE;
			for(INT KeyIndex=1; KeyIndex<KeyCount; ++KeyIndex)
			{
				if( FQuatError(FirstRot, Track.RotKeys(KeyIndex)) > MaxRotDelta )
				{
					bFramesIdentical = FALSE;
					break;
				}
			}

			if(bFramesIdentical)
			{
				Track.RotKeys.Remove(1, Track.RotKeys.Num()- 1);
				Track.RotKeys.Shrink();
				Track.Times.Remove(1, Track.Times.Num()- 1);
				Track.Times.Shrink();
				Track.Times(0) = 0.0f;
			}			
		}
	}
}

/**
 * Common compression utility to remove 'redundant' keys based on the provided delta thresholds
 *
 * @param	PositionTracks	Array of position track elements to reduce
 * @param	RotationTracks	Array of rotation track elements to reduce
 * @param	MaxPosDelta		Maximum local-space threshold for stationary motion
 * @param	MaxRotDelta		Maximum angle threshold to consider stationary motion
 */
void UAnimationCompressionAlgorithm::FilterTrivialKeys(
	TArray<FTranslationTrack>& PositionTracks,
	TArray<FRotationTrack>& RotationTracks, 
	FLOAT MaxPosDelta,
	FLOAT MaxRotDelta)
{
	FilterTrivialRotationKeys(RotationTracks, MaxRotDelta);
	FilterTrivialPositionKeys(PositionTracks, MaxPosDelta);
}

/**
 * Common compression utility to retain only intermittent position keys. For example,
 * calling with an Interval of 3 would keep every third key in the set and discard the rest
 *
 * @param	PositionTracks	Array of position track elements to reduce
 * @param	StartIndex		Index at which to begin reduction
 * @param	Interval		Interval of keys to retain
 */
void UAnimationCompressionAlgorithm::FilterIntermittentPositionKeys(
	TArray<FTranslationTrack>& PositionTracks, 
	INT StartIndex,
	INT Interval)
{
	const INT NumPosTracks = PositionTracks.Num();

	// copy intermittent position keys
	for( INT TrackIndex = 0; TrackIndex < NumPosTracks; ++TrackIndex )
	{
		FTranslationTrack& OldTrack	= PositionTracks(TrackIndex);
		const INT KeyCount = OldTrack.Times.Num();
		const INT FinalIndex = KeyCount - 1;
		StartIndex = ::Min(StartIndex, FinalIndex);

		check(OldTrack.Times.Num() == OldTrack.PosKeys.Num());

		TArray<FVector> NewPosKeys;
		TArray<FLOAT> NewTimes;

		NewTimes.Empty(KeyCount);
		NewPosKeys.Empty(KeyCount);

		// step through and retain the desired interval
		for (INT KeyIndex = StartIndex; KeyIndex < KeyCount; KeyIndex += Interval )
		{
			NewTimes.AddItem( OldTrack.Times(KeyIndex) );
			NewPosKeys.AddItem( OldTrack.PosKeys(KeyIndex) );
		}

		NewTimes.Shrink();
		NewPosKeys.Shrink();

		OldTrack.Times = NewTimes;
		OldTrack.PosKeys = NewPosKeys;
	}
}

/**
 * Common compression utility to retain only intermittent rotation keys. For example,
 * calling with an Interval of 3 would keep every third key in the set and discard the rest
 *
 * @param	RotationTracks	Array of rotation track elements to reduce
 * @param	StartIndex		Index at which to begin reduction
 * @param	Interval		Interval of keys to retain
 */
void UAnimationCompressionAlgorithm::FilterIntermittentRotationKeys(
	TArray<FRotationTrack>& RotationTracks, 
	INT StartIndex,
	INT Interval)
{
	const INT NumRotTracks = RotationTracks.Num();

	// copy intermittent position keys
	for( INT TrackIndex = 0; TrackIndex < NumRotTracks; ++TrackIndex )
	{
		FRotationTrack& OldTrack = RotationTracks(TrackIndex);
		const INT KeyCount = OldTrack.Times.Num();
		const INT FinalIndex = KeyCount-1;
		StartIndex = ::Min(StartIndex, FinalIndex);

		check(OldTrack.Times.Num() == OldTrack.RotKeys.Num());

		TArray<FQuat> NewRotKeys;
		TArray<FLOAT> NewTimes;

		NewTimes.Empty(KeyCount);
		NewRotKeys.Empty(KeyCount);

		// step through and retain the desired interval
		for (INT KeyIndex = StartIndex; KeyIndex < KeyCount; KeyIndex += Interval )
		{
			NewTimes.AddItem( OldTrack.Times(KeyIndex) );
			NewRotKeys.AddItem( OldTrack.RotKeys(KeyIndex) );
		}

		NewTimes.Shrink();
		NewRotKeys.Shrink();
		OldTrack.Times = NewTimes;
		OldTrack.RotKeys = NewRotKeys;
	}
}

/**
 * Common compression utility to retain only intermittent animation keys. For example,
 * calling with an Interval of 3 would keep every third key in the set and discard the rest
 *
 * @param	PositionTracks	Array of position track elements to reduce
 * @param	RotationTracks	Array of rotation track elements to reduce
 * @param	StartIndex		Index at which to begin reduction
 * @param	Interval		Interval of keys to retain
 */
void UAnimationCompressionAlgorithm::FilterIntermittentKeys(
	TArray<FTranslationTrack>& PositionTracks, 
	TArray<FRotationTrack>& RotationTracks, 
	INT StartIndex,
	INT Interval)
{
	FilterIntermittentPositionKeys(PositionTracks, StartIndex, Interval);
	FilterIntermittentRotationKeys(RotationTracks, StartIndex, Interval);
}

/**
 * Common compression utility to populate individual rotation and translation track
 * arrays from a set of raw animation tracks. Used as a precurser to animation compression.
 *
 * @param	RawAnimData			Array of raw animation tracks
 * @param	SequenceLength		The duration of the animation in seconds
 * @param	OutTranslationData	Translation tracks to fill
 * @param	OutRotationData		Rotation tracks to fill
 */
void UAnimationCompressionAlgorithm::SeparateRawDataIntoTracks(
	const TArray<FRawAnimSequenceTrack>& RawAnimData,
	FLOAT SequenceLength,
	TArray<FTranslationTrack>& OutTranslationData,
	TArray<FRotationTrack>& OutRotationData)
{
	const INT NumTracks = RawAnimData.Num();

	OutTranslationData.Empty( NumTracks );
	OutRotationData.Empty( NumTracks );
	OutTranslationData.AddZeroed( NumTracks );
	OutRotationData.AddZeroed( NumTracks );

	for ( INT TrackIndex = 0; TrackIndex < NumTracks; ++TrackIndex )
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
		for ( INT PosIndex = 0; PosIndex < RawTrack.PosKeys.Num(); ++PosIndex )
		{
			TranslationTrack.PosKeys.AddItem( RawTrack.PosKeys(PosIndex) );
		}

		// Copy over rotation keys.
		for ( INT RotIndex = 0; RotIndex < RawTrack.RotKeys.Num(); ++RotIndex )
		{
			RotationTrack.RotKeys.AddItem( RawTrack.RotKeys(RotIndex) );
		}

		// Set times for the translation track.
		if ( TranslationTrack.PosKeys.Num() > 1 )
		{
			const FLOAT PosFrameInterval = SequenceLength / static_cast<FLOAT>(TranslationTrack.PosKeys.Num()-1);
			for ( INT PosIndex = 0; PosIndex < TranslationTrack.PosKeys.Num(); ++PosIndex )
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
			for ( INT RotIndex = 0; RotIndex < RotationTrack.RotKeys.Num(); ++RotIndex )
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
}

