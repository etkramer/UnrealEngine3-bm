/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"

/*-----------------------------------------------------------------------------
	WxBitmap.
-----------------------------------------------------------------------------*/

WxBitmap::WxBitmap()
{
}

WxBitmap::WxBitmap(const wxImage& img, int depth)
	: wxBitmap( img, depth )
{
}

WxBitmap::WxBitmap(int width, int height, int depth)
	: wxBitmap( width, height, depth )
{
}

WxBitmap::WxBitmap(const FString& InFilename)
{
	Load( InFilename );
}

UBOOL WxBitmap::Load(const FString& InFilename)
{
	// @todo DB : insert OS-specific loading code here.
	return LoadFile( *FString::Printf( TEXT("%swxres\\%s.bmp"), appBaseDir(), *InFilename ), wxBITMAP_TYPE_BMP );
}

UBOOL WxBitmap::LoadLiteral(const FString& InFilename)
{
	// @todo DB : insert OS-specific loading code here.
	return LoadFile( *FString::Printf( TEXT("%s%s.bmp"), appBaseDir(), *InFilename ), wxBITMAP_TYPE_BMP );
}

/*-----------------------------------------------------------------------------
	WxMaskedBitmap.
-----------------------------------------------------------------------------*/

WxMaskedBitmap::WxMaskedBitmap()
{
}

WxMaskedBitmap::WxMaskedBitmap(const FString& InFilename)
	: WxBitmap( InFilename )
{
}

UBOOL WxMaskedBitmap::Load(const FString& InFilename)
{
	const UBOOL bRet = WxBitmap::Load( InFilename );

	SetMask( new wxMask( *this, wxColor(192,192,192) ) );

	return bRet;
}

/*-----------------------------------------------------------------------------
	WxAlphaBitmap.
-----------------------------------------------------------------------------*/

WxAlphaBitmap::WxAlphaBitmap()
{
}

WxAlphaBitmap::WxAlphaBitmap(const FString& InFilename, UBOOL InBorderBackground)
	: WxBitmap( LoadAlpha( InFilename, InBorderBackground ) )
{
}

wxImage WxAlphaBitmap::LoadAlpha(const FString& InFilename, UBOOL InBorderBackground)
{
	// Import the TGA file into a temporary texture
	const FString FullFilename = FString::Printf( TEXT("wxres\\%s.tga"), *InFilename );
	UTexture2D* Tex = ImportObject<UTexture2D>( GEngine, NAME_None, RF_Public|RF_Standalone, *FullFilename, NULL, NULL, TEXT("CREATEMIPMAPS=0 NOCOMPRESSION=1") );

	// If we can't load the file from the disk, create a small empty image as a placeholder and return that instead.
	if( !Tex )
	{
		GWarn->Logf( NAME_Warning, TEXT("WxAlphaBitmap::LoadAlpha : Couldn't load '%s'"), *FullFilename );
		return wxImage( 16, 16 );
	}

	// Get the background color we need to blend with.
	wxColour clr = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);

	if( InBorderBackground )
	{
		clr.Set( 140,190,190 );
	}

	// Create a wxImage of the alpha'd image data combined with the background color.

	wxImage img( Tex->SizeX, Tex->SizeY );

	BYTE* MipData = (BYTE*) Tex->Mips(0).Data.Lock(LOCK_READ_ONLY);
	INT Toggle = 0;
	for( INT x = 0 ; x < Tex->SizeX ; ++x )
	{
		for( INT y = 0 ; y < Tex->SizeY ; ++y )
		{
			const INT idx = ((y * Tex->SizeX) + x) * 4;
			FLOAT B = MipData[idx];
			FLOAT G = MipData[idx+1];
			FLOAT R = MipData[idx+2];
			FLOAT SrcA = MipData[idx+3] / 255.f;

			if( InBorderBackground )
			{
				if( x == 0 || x == 1 || x == Tex->SizeX-1 || x == Tex->SizeX-2 || y == 0 || y == 1 || y == Tex->SizeY-1 || y == Tex->SizeY-2 )
				{
					R = 170;
					G = 220;
					B = 220;
					SrcA = 1.f;
				}
			}

			if( InBorderBackground )
			{
				Toggle++;
				if( Toggle%3 )
				{
					clr.Set( 140,190,190 );
				}
				else
				{
					clr = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
				}
			}

			const FLOAT DstA = 1.f - SrcA;
			const BYTE NewR = ((R*SrcA) + (clr.Red()*DstA));
			const BYTE NewG = ((G*SrcA) + (clr.Green()*DstA));
			const BYTE NewB = ((B*SrcA) + (clr.Blue()*DstA));

			img.SetRGB( x, y, NewR, NewG, NewB );
		}
	}
	Tex->Mips(0).Data.Unlock();

	return img;
}
