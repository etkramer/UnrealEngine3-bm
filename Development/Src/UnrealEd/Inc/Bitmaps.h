/*=============================================================================
	Bitmaps.h: Classes based on standard bitmaps
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __BITMAPS_H__
#define __BITMAPS_H__

/**
 * A bitmap that has OS-specific loading hooks.  All input filenames in this interface
 * are the base name of the bitmap; in other words, "yourPath\\New" represents
 * "<appBaseDir>\\wxres\\yourPath\\New.bmp" to Load and "<appBaseDir>\\yourPath\\New.bmp" to LoadLiteral.
 */
class WxBitmap : public wxBitmap
{
public:
	WxBitmap();
	WxBitmap(const wxImage& img, int depth = -1);
	WxBitmap(int width, int height, int depth = -1);
	WxBitmap(const FString& InFilename);

	/**
	 * Tries to load the specified bitmap from the <appBaseDir>\\wxres directory..
	 *
	 * @param		InFilename		The base filename; eg "yourPath\\New" represents "<appBaseDir>\\wxres\\yourPath\\New.bmp".
	 * @return						TRUE on success.
	 */
	virtual UBOOL Load(const FString& InFilename);

	/**
	 * Loads the bitmap from a literal path rather than assuming it's in the wxres directory.
	 * Prepends appBaseDir() to the specified path.
	 *
	 * @param		InFilename		The path and base filename; "yourPath\\New" represents "<appBaseDir>\\wxres\\yourPath\\New.bmp"
	 * @return						TRUE on success.
	 */
	virtual UBOOL LoadLiteral(const FString& InFilename);
};

/**
 * A bitmap that automatically generates a wxMask for itself.
 */
class WxMaskedBitmap : public WxBitmap
{
public:
	WxMaskedBitmap();
	WxMaskedBitmap(const FString& InFilename);

	virtual UBOOL Load(const FString& InFilename);
};

/**
 * A bitmap that automatically generates a wxBitmap for itself based on the
 * alpha channel of the source TGA file.
 */
class WxAlphaBitmap : public WxBitmap
{
public:
	WxAlphaBitmap();
	WxAlphaBitmap(const FString& InFilename, UBOOL InBorderBackground);

private:
	wxImage LoadAlpha(const FString& InFilename, UBOOL InBorderBackground);
};

#endif // __BITMAPS_H__
