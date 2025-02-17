/*=============================================================================
ImageUtils.cpp: Image utility functions.
Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ImageUtils.h"

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
void FImageUtils::ImageResize(INT SrcWidth, INT SrcHeight, const TArray<FColor> &SrcData, INT DstWidth, INT DstHeight, TArray<FColor> &DstData )
{
	DstData.Empty();
	DstData.AddZeroed(DstWidth*DstHeight);

	// Do a nearest resize to downsample the image to thumbnail size.
	const UBOOL bWidthMajor = SrcWidth > SrcHeight;

	FLOAT SrcX = 0;
	FLOAT SrcY = 0;
	FLOAT StepSize;
	INT LoopWidth;
	INT LoopHeight;

	// Find the step size for the resize operation.  Since we constrain proportion, the step size is the same for both X and Y.
	// We also need to find the major axis and scale down the other axis proportional to the original image size.
	if(bWidthMajor == TRUE)
	{
		StepSize = SrcWidth / (FLOAT)DstWidth;
		LoopWidth = DstWidth;
		LoopHeight = appCeil(DstWidth * SrcHeight / (FLOAT)SrcWidth);
	}
	else
	{
		StepSize = SrcHeight / (FLOAT)DstHeight;
		LoopHeight = DstHeight;
		LoopWidth = appCeil(DstHeight * SrcWidth / (FLOAT)SrcHeight);
	}


	for(INT Y=0; Y<LoopHeight;Y++)
	{
		INT PixelPos = Y * DstWidth;
		SrcX = 0.0f;	
	
		for(INT X=0; X<LoopWidth; X++)
		{
			INT PixelCount = 0;
			FLinearColor StepColor(0.0f,0.0f,0.0f,0.0f);
			FLOAT EndX = SrcX + StepSize;
			FLOAT EndY = SrcY + StepSize;
			
			// Generate a rectangular region of pixels and then find the average color of the region.
			INT PosY = appTrunc(SrcY+0.5f);
			PosY = Clamp<INT>(PosY, 0, (SrcHeight - 1));

			INT PosX = appTrunc(SrcX+0.5f);
			PosX = Clamp<INT>(PosX, 0, (SrcWidth - 1));

			INT EndPosY = appTrunc(EndY+0.5f);
			EndPosY = Clamp<INT>(EndPosY, 0, (SrcHeight - 1));

			INT EndPosX = appTrunc(EndX+0.5f);
			EndPosX = Clamp<INT>(EndPosX, 0, (SrcWidth - 1));

			for(INT PixelX = PosX; PixelX <= EndPosX; PixelX++)
			{
				for(INT PixelY = PosY; PixelY <= EndPosY; PixelY++)
				{
					INT StartPixel =  PixelX + PixelY * SrcWidth;
					
					StepColor += SrcData(StartPixel);
					PixelCount++;
				}
			}

			StepColor /= (FLOAT)PixelCount;

			// Store the final averaged pixel color value.
			FColor FinalColor(StepColor);
			FinalColor.A = 255;
			DstData(PixelPos) = FinalColor;

			SrcX = EndX;	
			PixelPos++;
		}

		SrcY += StepSize;
	}
}

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
UTexture2D* FImageUtils::ConstructTexture2D(INT SrcWidth, INT SrcHeight, const TArray<FColor> &SrcData, UObject* Outer, const FString& Name, const EObjectFlags &Flags)
{
#if !CONSOLE && defined(_MSC_VER)

	UTexture2D* Tex2D;

	Tex2D = CastChecked<UTexture2D>( UObject::StaticConstructObject(UTexture2D::StaticClass(), Outer, FName(*Name), Flags) );
	Tex2D->Init(SrcWidth, SrcHeight, PF_A8R8G8B8);
	
	// Create base mip for the texture we created.
	BYTE* MipData = (BYTE*) Tex2D->Mips(0).Data.Lock(LOCK_READ_WRITE);
	for( INT y=0; y<SrcHeight; y++ )
	{
		BYTE* DestPtr = &MipData[(SrcHeight - 1 - y) * SrcWidth * sizeof(FColor)];
		FColor* SrcPtr = const_cast<FColor*>(&SrcData((SrcHeight - 1 - y) * SrcWidth));
		for( INT x=0; x<SrcWidth; x++ )
		{
			*DestPtr++ = SrcPtr->B;
			*DestPtr++ = SrcPtr->G;
			*DestPtr++ = SrcPtr->R;
			*DestPtr++ = 0xFF;
			SrcPtr++;
		}
	}
	Tex2D->Mips(0).Data.Unlock();

	// PNG Compress.
	FPNGHelper PNG;
	PNG.InitRaw( Tex2D->Mips(0).Data.Lock(LOCK_READ_ONLY), Tex2D->Mips(0).Data.GetBulkDataSize(), Tex2D->SizeX, Tex2D->SizeY );
	TArray<BYTE> CompressedData = PNG.GetCompressedData();
	Tex2D->Mips(0).Data.Unlock();

	// Store source art.
	Tex2D->SourceArt.Lock(LOCK_READ_WRITE);
	void* SourceArtPointer = Tex2D->SourceArt.Realloc( CompressedData.Num() );
	appMemcpy( SourceArtPointer, CompressedData.GetData(), CompressedData.Num() );
	Tex2D->SourceArt.Unlock();

	// Set compression options.
	Tex2D->CompressionSettings	= TC_Default;
	Tex2D->CompressionNoAlpha	= TRUE;
	Tex2D->DeferCompression		= FALSE;

	// This will trigger compressions.
	Tex2D->PostEditChange(NULL);

	return Tex2D;
#else
	appErrorf(TEXT("ConstructTexture2D not supported on console."));
	return NULL;
#endif

}

