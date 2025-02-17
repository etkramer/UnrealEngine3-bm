/*=============================================================================
ImageUtils.h: Image utility functions.
Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __IMAGEUTILS_H__
#define __IMAGEUTILS_H__

/**
 * Class of static image utility functions.
 */
class FImageUtils
{
public:
	/**
	 * Resizes the given image using a simple average filter and stores it in the destination array.  This version constrains aspect ratio.
	 *
	 * @param SrcWidth	Source image width.
	 * @param SrcHeight	Source image height.
	 * @param SrcData	Source image data.
	 * @param DstWidth	Destination image width.
	 * @param DstHeight Destination image height.
	 * @param DstData	Destination image data.
	 */
	static void ImageResize(INT SrcWidth, INT SrcHeight, const TArray<FColor> &SrcData,  INT DstWidth, INT DstHeight, TArray<FColor> &DstData );

	/**
	 * Creates a 2D texture from a array of raw color data.
	 *
	 * @param SrcWidth		Source image width.
	 * @param SrcHeight		Source image height.
	 * @param SrcData		Source image data.
	 * @param Outer			Outer for the texture object.
	 * @param Name			Name for the texture object.
	 * @param Flags			Object flags for the texture object.
	 * @return				Returns a pointer to the constructed 2D texture object.
	 *
	 */
	static UTexture2D* ConstructTexture2D(INT SrcWidth, INT SrcHeight, const TArray<FColor> &SrcData, UObject* Outer, const FString& Name, const EObjectFlags &Flags);
};

#endif

