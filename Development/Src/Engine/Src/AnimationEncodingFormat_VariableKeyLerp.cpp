/*=============================================================================
	AnimationEncodingFormat_VariableKeyLerp.cpp: Skeletal mesh animation functions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "AnimationCompression.h"
#include "AnimationEncodingFormat_VariableKeyLerp.h"
#include "AnimationUtils.h"

#if USE_ANIMATION_CODEC_INTERFACE

/**
 * Handles the ByteSwap of compressed rotation data on import
 *
 * @param	Seq				The UAnimSequence container.
 * @param	MemoryReader	The FMemoryReader to read from.
 * @param	RotTrackData	The compressed rotation data stream.
 * @param	NumKeysRot		The number of keys present in the stream.
 */
void AEFVariableKeyLerpShared::ByteSwapRotationIn(
	UAnimSequence& Seq, 
	FMemoryReader& MemoryReader,
	BYTE*& RotTrackData,
	INT NumKeysRot)
{
	const INT RotStride		= CompressedRotationStrides[Seq.RotationCompressionFormat];
	const INT RotNum		= CompressedRotationNum[Seq.RotationCompressionFormat];

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

		for ( INT KeyIndex = 0; KeyIndex < NumKeysRot; ++KeyIndex )
		{
			for ( INT i = 0; i < RotNum; ++i )
			{
				AC_UnalignedSwap( MemoryReader, RotTrackData, RotStride );
			}
		}

		// Like the compressed byte stream, pad the serialization stream to four bytes.
		// As a sanity check, each pad byte can be checked to be the PadSentinel.
		PadMemoryReader(&MemoryReader, RotTrackData, 4); 

		// swap the track table
		const size_t EntryStride = (Seq.NumFrames > 0xff) ? sizeof(WORD) : sizeof(BYTE);
		for ( INT KeyIndex = 0; KeyIndex < NumKeysRot; ++KeyIndex )
		{
			AC_UnalignedSwap( MemoryReader, RotTrackData, EntryStride);
		}
	}
	else if ( NumKeysRot == 1 )
	{
		// For a rotation track of n=1 keys, the single key is packed as an FQuatFloat96NoW.
		AC_UnalignedSwap( MemoryReader, RotTrackData, sizeof(FLOAT) );
		AC_UnalignedSwap( MemoryReader, RotTrackData, sizeof(FLOAT) );
		AC_UnalignedSwap( MemoryReader, RotTrackData, sizeof(FLOAT) );
	}
}

/**
 * Handles the ByteSwap of compressed translation data on import
 *
 * @param	Seq				The UAnimSequence container.
 * @param	MemoryReader	The FMemoryReader to read from.
 * @param	TransTrackData	The compressed translation data stream.
 * @param	NumKeysTrans	The number of keys present in the stream.
 */
void AEFVariableKeyLerpShared::ByteSwapTranslationIn(
	UAnimSequence& Seq, 
	FMemoryReader& MemoryReader,
	BYTE*& TransTrackData,
	INT NumKeysTrans)
{
	const INT TransStride	= CompressedTranslationStrides[Seq.TranslationCompressionFormat];
	for ( INT KeyIndex = 0; KeyIndex < NumKeysTrans; ++KeyIndex )
	{
		// A translation track of n keys is packed as n uncompressed float[3].
		AC_UnalignedSwap( MemoryReader, TransTrackData, sizeof(FLOAT) );
		AC_UnalignedSwap( MemoryReader, TransTrackData, sizeof(FLOAT) );
		AC_UnalignedSwap( MemoryReader, TransTrackData, sizeof(FLOAT) );
	}

	if (NumKeysTrans > 1)
	{
		// Like the compressed byte stream, pad the serialization stream to four bytes.
		// As a sanity check, each pad byte can be checked to be the PadSentinel.
		PadMemoryReader(&MemoryReader, TransTrackData, 4); 

		// swap the track table
		const size_t EntryStride = (Seq.NumFrames > 0xff) ? sizeof(WORD) : sizeof(BYTE);
		for ( INT KeyIndex = 0; KeyIndex < NumKeysTrans; ++KeyIndex )
		{
			AC_UnalignedSwap( MemoryReader, TransTrackData, EntryStride);
		}
	}
}

/**
 * Handles the ByteSwap of compressed rotation data on export
 *
 * @param	Seq				The UAnimSequence container.
 * @param	MemoryWriter	The FMemoryWriter to write to.
 * @param	RotTrackData	The compressed rotation data stream.
 * @param	NumKeysRot		The number of keys to write to the stream.
 */
void AEFVariableKeyLerpShared::ByteSwapRotationOut(
	UAnimSequence& Seq, 
	FMemoryWriter& MemoryWriter,
	BYTE*& RotTrackData,
	INT NumKeysRot)
{
	const INT RotStride		= CompressedRotationStrides[Seq.RotationCompressionFormat];
	const INT RotNum		= CompressedRotationNum[Seq.RotationCompressionFormat];

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

		for ( INT KeyIndex = 0; KeyIndex < NumKeysRot; ++KeyIndex )
		{
			for ( INT i = 0; i < RotNum; ++i )
			{
				AC_UnalignedSwap( MemoryWriter, RotTrackData, RotStride );
			}
		}

		// Like the compressed byte stream, pad the serialization stream to four bytes.
		// As a sanity check, each pad byte can be checked to be the PadSentinel.
		PadMemoryWriter(&MemoryWriter, RotTrackData, 4);

		// swap the track table
		const size_t EntryStride = (Seq.NumFrames > 0xff) ? sizeof(WORD) : sizeof(BYTE);
		for ( INT KeyIndex = 0; KeyIndex < NumKeysRot; ++KeyIndex )
		{
			AC_UnalignedSwap( MemoryWriter, RotTrackData, EntryStride );
		}

	}
	else if ( NumKeysRot == 1 )
	{
		// For a rotation track of n=1 keys, the single key is packed as an FQuatFloat96NoW.
		AC_UnalignedSwap( MemoryWriter, RotTrackData, sizeof(FLOAT) );
		AC_UnalignedSwap( MemoryWriter, RotTrackData, sizeof(FLOAT) );
		AC_UnalignedSwap( MemoryWriter, RotTrackData, sizeof(FLOAT) );
	}
}

/**
 * Handles the ByteSwap of compressed translation data on export
 *
 * @param	Seq				The UAnimSequence container.
 * @param	MemoryWriter	The FMemoryWriter to write to.
 * @param	TransTrackData	The compressed translation data stream.
 * @param	NumKeysTrans	The number of keys to write to the stream.
 */
void AEFVariableKeyLerpShared::ByteSwapTranslationOut(
	UAnimSequence& Seq, 
	FMemoryWriter& MemoryWriter,
	BYTE*& TransTrackData,
	INT NumKeysTrans)
{
	const INT TransStride	= CompressedTranslationStrides[Seq.TranslationCompressionFormat];

	for ( INT KeyIndex = 0; KeyIndex < NumKeysTrans; ++KeyIndex )
	{
		// A translation track of n keys is packed as n uncompressed float[3].
		AC_UnalignedSwap( MemoryWriter, TransTrackData, sizeof(FLOAT) );
		AC_UnalignedSwap( MemoryWriter, TransTrackData, sizeof(FLOAT) );
		AC_UnalignedSwap( MemoryWriter, TransTrackData, sizeof(FLOAT) );
	}

	if (NumKeysTrans > 1)
	{
		// Like the compressed byte stream, pad the serialization stream to four bytes.
		// As a sanity check, each pad byte can be checked to be the PadSentinel.
		PadMemoryWriter(&MemoryWriter, TransTrackData, 4);

		// swap the track table
		const size_t EntryStride = (Seq.NumFrames > 0xff) ? sizeof(WORD) : sizeof(BYTE);
		for ( INT KeyIndex = 0; KeyIndex < NumKeysTrans; ++KeyIndex )
		{
			AC_UnalignedSwap( MemoryWriter, TransTrackData, EntryStride );
		}

	}
}

#endif //#if USE_ANIMATION_CODEC_INTERFACE
