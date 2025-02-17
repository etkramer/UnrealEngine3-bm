/*=============================================================================
	UnAudioCompress.h: Unreal audio compression - ogg vorbis
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNAUDIOCOMPRESS_H__
#define __UNAUDIOCOMPRESS_H__

#include "UnConsoleTools.h"

/**
 * PC (ogg vorbis) sound cooker interface.
 */
class FPCSoundCooker : public FConsoleSoundCooker
{
public:
	/**
	 * Constructor
	 */
	FPCSoundCooker( void ) {}

	/**
	 * Virtual destructor
	 */
	~FPCSoundCooker( void ) {}

	/**
	 * Cooks the source data for the platform and stores the cooked data internally.
	 *
	 * @param	SrcBuffer		Pointer to source buffer
	 * @param	SrcBufferSize	Size in bytes of source buffer
	 * @param	WaveFormat		Pointer to platform specific wave format description
	 * @param	Compression		Quality value ranging from 1 [poor] to 100 [very good]
	 *
	 * @return	TRUE if succeeded, FALSE otherwise
	 */
	virtual bool Cook( short* SrcBuffer, DWORD SrcBufferSize, void* WaveFormat, INT Quality, const TCHAR* Name );

	/**
	 * Cooks upto 8 mono files into a multistream file (eg. 5.1). The front left channel is required, the rest are optional.
	 *
	 * @param	SrcBuffers		Pointers to source buffers
	 * @param	SrcBufferSize	Size in bytes of source buffer
	 * @param	WaveFormat		Pointer to platform specific wave format description
	 * @param	Compression		Quality value ranging from 1 [poor] to 100 [very good]
	 *
	 * @return	TRUE if succeeded, FALSE otherwise
	 */
	virtual bool CookSurround( short* SrcBuffers[8], DWORD SrcBufferSize, void* WaveFormat, INT Quality, const TCHAR* Name );

	/**
	 * Returns the size of the cooked data in bytes.
	 *
	 * @return The size in bytes of the cooked data including any potential header information.
	 */
	virtual UINT GetCookedDataSize( void );

	/**
	 * Copies the cooked data into the passed in buffer of at least size GetCookedDataSize()
 	 *
	 * @param CookedData	Buffer of at least GetCookedDataSize() bytes to copy data to.
	 */
	virtual void GetCookedData( BYTE* CookedData );

	/** 
	 * Decompresses the platform dependent format to raw PCM. Used for quality previewing.
	 *
	 * @param	CompressedData			ogg vorbis data
	 * @param	CompressedDataSize		Size of ogg vorbis data
	 * @param	AdditionalInfo			Any additonal info required to decompress the sound
	 */
	virtual bool Decompress( const BYTE* CompressedData, DWORD CompressedDataSize, void* AdditionalInfo );

	/**
	 * Returns the size of the decompressed data in bytes.
	 *
	 * @return The size in bytes of the raw PCM data
	 */
	virtual DWORD GetRawDataSize( void );

	/**
	 * Copies the raw data into the passed in buffer of at least size GetRawDataSize()
	 *
	 * @param DstBuffer	Buffer of at least GetRawDataSize() bytes to copy data to.
	 */
	virtual void GetRawData( BYTE * DstBuffer );

private:
	TArray<BYTE>		CompressedDataStore;
	DWORD				BufferOffset;

	TArray<BYTE>		PCMData;
};

/**
 * Singleton to return the cooking class for PC sounds
 */
FPCSoundCooker* GetPCSoundCooker( void );

// end
#endif
