/*=============================================================================
	AnimationEncodingFormat_ConstantKeyLerp.cpp: Skeletal mesh animation functions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "AnimationCompression.h"
#include "AnimationEncodingFormat_ConstantKeyLerp.h"
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
void AEFConstantKeyLerpShared::ByteSwapRotationIn(
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
void AEFConstantKeyLerpShared::ByteSwapTranslationIn(
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
}

/**
 * Handles the ByteSwap of compressed rotation data on export
 *
 * @param	Seq				The UAnimSequence container.
 * @param	MemoryWriter	The FMemoryWriter to write to.
 * @param	RotTrackData	The compressed rotation data stream.
 * @param	NumKeysRot		The number of keys to write to the stream.
 */
void AEFConstantKeyLerpShared::ByteSwapRotationOut(
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
void AEFConstantKeyLerpShared::ByteSwapTranslationOut(
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
}


#endif //#if USE_ANIMATION_CODEC_INTERFACE
