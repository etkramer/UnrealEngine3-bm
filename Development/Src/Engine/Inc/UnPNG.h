/*=============================================================================
	UnPNG.h: Unreal PNG support.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*------------------------------------------------------------------------------
	FPNGHelper definition.
------------------------------------------------------------------------------*/

// don't include this on consoles or in gcc
#if !CONSOLE && defined(_MSC_VER)

#pragma pack (push,8)
#include "..\..\..\External\libpng\png.h"
#pragma pack (pop)

class FPNGHelper
{
public:
	void InitCompressed( void* InCompressedData, INT InCompressedSize, INT InWidth, INT InHeight );
	void InitRaw( void* InRawData, INT InRawSize, INT InWidth, INT InHeight );

	TArray<BYTE> GetRawData();
	TArray<BYTE> GetCompressedData();

protected:
	void Uncompress();
	void Compress();

	static void user_read_data( png_structp png_ptr, png_bytep data, png_size_t length );
	static void user_write_data( png_structp png_ptr, png_bytep data, png_size_t length );
	static void user_flush_data( png_structp png_ptr );

	static void user_error_fn( png_structp png_ptr, png_const_charp error_msg );
	static void user_warning_fn( png_structp png_ptr, png_const_charp warning_msg );

	// Variables.
	TArray<BYTE>	RawData;
	TArray<BYTE>	CompressedData;

	INT				ReadOffset,
					Width,
					Height;
};

#endif // !CONSOLE && defined(_MSC_VER)

