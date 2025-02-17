/*=============================================================================
	TTFontImport.cpp: True-type Font Importing
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "Factories.h"

IMPLEMENT_CLASS(UTrueTypeFontFactory);
IMPLEMENT_CLASS(UTrueTypeMultiFontFactory);


INT FromHex( TCHAR Ch )
{
	if( Ch>='0' && Ch<='9' )
		return Ch-'0';
	else if( Ch>='a' && Ch<='f' )
		return 10+Ch-'a';
	else if( Ch>='A' && Ch<='F' )
		return 10+Ch-'A';
	appErrorf(LocalizeSecure(LocalizeUnrealEd("Error_ExpectingDigitGotCharacter"),Ch));
	return 0;
}




void UTrueTypeFontFactory::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);

	// Font import options property (UFontImportOptions)
	new( GetClass(), TEXT( "ImportOptions" ), RF_Public ) UObjectProperty( CPP_PROPERTY( ImportOptions ), TEXT( "" ), CPF_Edit | CPF_EditInline | CPF_NoClear , UFontImportOptions::StaticClass() );
	{
		// Tell the garbage collector about our UObject reference so that it won't be GC'd!
		UClass* TheClass = GetClass();
		TheClass->EmitObjectReference( STRUCT_OFFSET( UTrueTypeFontFactory, ImportOptions ) );
	}
}


#if !__WIN32__  // !!! FIXME
#define FW_NORMAL 400
#endif



/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UTrueTypeFontFactory::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UFont::StaticClass();
	bCreateNew = TRUE;
	bEditAfterNew = TRUE;
	AutoPriority = -1;
	Description = TEXT("Font Imported From TrueType");
}



UTrueTypeFontFactory::UTrueTypeFontFactory()
{
}



void UTrueTypeFontFactory::SetupFontImportOptions()
{
	// Allocate our import options object if it hasn't been created already!
	if( ImportOptions == NULL )
	{
		ImportOptions = ConstructObject< UFontImportOptions >( UFontImportOptions::StaticClass(), this, NAME_None );
	}
}


UObject* UTrueTypeFontFactory::FactoryCreateNew(
	UClass* Class,
	UObject* InParent,
	FName Name,
	EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext*	Warn )
{
#if !__WIN32__
	STUBBED("Win32 TTF code");
	return NULL;
#else
	check(Class==UFont::StaticClass());

	// Create font and its texture.
	UFont* Font = new( InParent, Name, Flags )UFont;

	// Copy the import settings into the font for later reference
	Font->ImportOptions = ImportOptions->Data;


	// For a single-resolution font, we'll create a one-element array and pass that along to our import function
	TArray< FLOAT > ResHeights;
	ResHeights.AddItem( ImportOptions->Data.Height );

	GWarn->BeginSlowTask( *LocalizeUnrealEd( TEXT( "FontFactory_ImportingTrueTypeFont" ) ), TRUE );

	// Import the font
	const UBOOL bSuccess = ImportTrueTypeFont( Font, Warn, ResHeights.Num(), ResHeights );

	GWarn->EndSlowTask();

	return bSuccess ? Font : NULL;
#endif
}


#if __WIN32__
/**
 * Win32 Platform Only: Creates a single font texture using the Windows GDI
 *
 * @param Font (In/Out) The font we're operating with
 * @param dc The device context configured to paint this font
 * @param RowHeight Height of a font row in pixels
 * @param TextureNum The current texture index
 *
 * @return Returns the newly created texture, if successful, otherwise NULL
 */
UTexture2D* UTrueTypeFontFactory::CreateTextureFromDC( UFont* Font, DWORD Indc, INT Height, INT TextureNum )
{
	HDC dc = (HDC)Indc;

	FString TextureString = FString::Printf( TEXT("%s_Page"), *Font->GetName() );
	if( TextureNum < 26 )
	{
		TextureString = TextureString + FString::Printf(TEXT("%c"), 'A'+TextureNum);
	}
	else
	{
		TextureString = TextureString + FString::Printf(TEXT("%c%c"), 'A'+TextureNum/26, 'A'+TextureNum%26 );
	}

 	if( StaticFindObject( NULL, Font, *TextureString ) )
	{
		warnf( TEXT("A texture named %s already exists!"), *TextureString );
	}

	// Create texture for page.
	UTexture2D* Texture = new(Font, *TextureString)UTexture2D;

	// note RF_Public because font textures can be referenced directly by material expressions
	Texture->SetFlags(RF_Public);
	Texture->Init( ImportOptions->Data.TexturePageWidth, appRoundUpToPowerOfTwo(Height), PF_A8R8G8B8 );

	// Copy the LODGroup from the font factory to the new texture
	// By default, this should be TEXTUREGROUP_UI for fonts!
	Texture->LODGroup = LODGroup;

	// Also, we never want to stream in font textures since that always looks awful
	Texture->NeverStream = TRUE;

	// Copy bitmap data to texture page.
	FColor FontColor8Bit( ImportOptions->Data.ForegroundColor );

	BYTE* MipData = (BYTE*) Texture->Mips(0).Data.Lock(LOCK_READ_WRITE);
	if( !ImportOptions->Data.bEnableAntialiasing )
	{
		for( INT i=0; i<(INT)Texture->SizeX; i++ )
		{
			for( INT j=0; j<(INT)Texture->SizeY; j++ )
			{
				INT CharAlpha = GetRValue( GetPixel( dc, i, j ) );
				INT DropShadowAlpha;

				if( ImportOptions->Data.bEnableDropShadow && i > 0 && j >> 0 )
				{
					DropShadowAlpha = GetRValue( GetPixel( dc, i - 1, j - 1 ) );
				}
				else
				{
					DropShadowAlpha = 0;
				}

				if( CharAlpha )
				{
					MipData[4 * (i + j * Texture->SizeX) + 0] = FontColor8Bit.B;
					MipData[4 * (i + j * Texture->SizeX) + 1] = FontColor8Bit.G;
					MipData[4 * (i + j * Texture->SizeX) + 2] = FontColor8Bit.R;
					MipData[4 * (i + j * Texture->SizeX) + 3] = 0xFF;
				}
				else if( DropShadowAlpha )
				{
					MipData[4 * (i + j * Texture->SizeX) + 0] = 0x00;
					MipData[4 * (i + j * Texture->SizeX) + 1] = 0x00;
					MipData[4 * (i + j * Texture->SizeX) + 2] = 0x00;
					MipData[4 * (i + j * Texture->SizeX) + 3] = 0xFF;
				}
				else
				{
					MipData[4 * (i + j * Texture->SizeX) + 0] = FontColor8Bit.B;
					MipData[4 * (i + j * Texture->SizeX) + 1] = FontColor8Bit.G;
					MipData[4 * (i + j * Texture->SizeX) + 2] = FontColor8Bit.R;
					MipData[4 * (i + j * Texture->SizeX) + 3] = 0x00;
				}
			}
		}
	}
	else
	{
		for( INT i=0; i<(INT)Texture->SizeX; i++ )
		{
			for( INT j=0; j<(INT)Texture->SizeY; j++ )
			{
				INT CharAlpha = GetRValue( GetPixel( dc, i, j ) );
				FLOAT fCharAlpha = FLOAT( CharAlpha ) / 255.0f;

				INT DropShadowAlpha = 0;
				if( ImportOptions->Data.bEnableDropShadow && i > 0 && j > 0 )
				{
					// Character opacity takes precedence over drop shadow opacity
					DropShadowAlpha =
						( BYTE )( ( 1.0f - fCharAlpha ) * ( FLOAT )GetRValue( GetPixel( dc, i - 1, j - 1 ) ) );
				}
				FLOAT fDropShadowAlpha = FLOAT( DropShadowAlpha ) / 255.0f;

				// Color channel = Font color, except for drop shadow pixels
				MipData[4 * (i + j * Texture->SizeX) + 0] = ( BYTE )( FontColor8Bit.B * ( 1.0f - fDropShadowAlpha ) );
				MipData[4 * (i + j * Texture->SizeX) + 1] = ( BYTE )( FontColor8Bit.G * ( 1.0f - fDropShadowAlpha ) );
				MipData[4 * (i + j * Texture->SizeX) + 2] = ( BYTE )( FontColor8Bit.R * ( 1.0f - fDropShadowAlpha ) );
				MipData[4 * (i + j * Texture->SizeX) + 3] = CharAlpha + DropShadowAlpha;
			}
		}
	}
	Texture->Mips(0).Data.Unlock();
	MipData = NULL;

	// PNG Compress.
	FPNGHelper PNG;
	PNG.InitRaw( Texture->Mips(0).Data.Lock(LOCK_READ_ONLY), Texture->Mips(0).Data.GetBulkDataSize(), Texture->SizeX, Texture->SizeY );
	TArray<BYTE> CompressedData = PNG.GetCompressedData();
	check( CompressedData.Num() );
	Texture->Mips(0).Data.Unlock();

	// Store source art.
	Texture->SourceArt.Lock(LOCK_READ_WRITE);
	void* SourceArtPointer = Texture->SourceArt.Realloc( CompressedData.Num() );
	appMemcpy( SourceArtPointer, CompressedData.GetData(), CompressedData.Num() );
	Texture->SourceArt.Unlock();

	Texture->CompressionNoMipmaps = 1;
	Texture->Compress();

	return Texture;
}
#endif



#if __WIN32__
/**
 * Win32 Platform Only: Imports a TrueType font
 *
 * @param Font (In/Out) The font object that we're importing into
 * @param Warn Feedback context for displaying warnings and errors
 * @param NumResolutions Number of resolution pages we should generate 
 * @param ResHeights Font height for each resolution (one array entry per resolution)
 *
 * @return TRUE if successful
 */
UBOOL UTrueTypeFontFactory::ImportTrueTypeFont(
	UFont* Font,
	FFeedbackContext* Warn,
	const INT NumResolutions,
	const TArray< FLOAT >& ResHeights )
{
	Font->Kerning = ImportOptions->Data.Kerning;
	Font->IsRemapped = 0;

	// Zero out the Texture Index
	INT CurrentTexture = 0;

	TMap<TCHAR,TCHAR> InverseMap;

	const UBOOL UseFiles = ImportOptions->Data.CharsFileWildcard != TEXT("") && ImportOptions->Data.CharsFilePath != TEXT("");
	const UBOOL UseRange = ImportOptions->Data.UnicodeRange != TEXT("");

	INT CharsPerPage = 0;
	if( UseFiles || UseRange )
	{
		Font->IsRemapped = 1;

		// Only include ASCII characters if we were asked to
		INT MinRangeCharacter = 0;
		if( ImportOptions->Data.bIncludeASCIIRange )
		{
			// Map (ASCII)
			for( TCHAR c=0;c<256;c++ )
			{
				Font->CharRemap.Set( c, c );
				InverseMap.Set( c, c );
			}

			MinRangeCharacter = 256;
		}

		TArray<BYTE> Chars;
		Chars.AddZeroed(65536);

		if( UseFiles )
		{
			// find all characters in specified path/wildcard
			TArray<FString> Files;
			GFileManager->FindFiles( Files, *(ImportOptions->Data.CharsFilePath*ImportOptions->Data.CharsFileWildcard),1,0 );
			for( TArray<FString>::TIterator it(Files); it; ++it )
			{
				FString S;
				verify(appLoadFileToString(S,*(ImportOptions->Data.CharsFilePath * *it)));
				for( INT i=0; i<S.Len(); i++ )
				{
					Chars((*S)[i]) = 1;
				}
			}
			warnf(TEXT("Checked %d files"), Files.Num() );
		}

		if( UseRange )
		{
			Warn->Logf(TEXT("UnicodeRange <%s>:"), *ImportOptions->Data.UnicodeRange);
			INT From = 0;
			INT To = 0;
			UBOOL HadDash = 0;
			for( const TCHAR* C=*ImportOptions->Data.UnicodeRange; *C; C++ )
			{
				if( (*C>='A' && *C<='F') || (*C>='a' && *C<='f') || (*C>='0' && *C<='9') )
				{
					if( HadDash )
					{
						To = 16*To + FromHex(*C);
					}
					else
					{
						From = 16*From + FromHex(*C);
					}
				}
				else if( *C=='-' )
				{
					HadDash = 1;
				}
				else if( *C==',' )
				{
					warnf(TEXT("Adding unicode character range %x-%x (%d-%d)"),From,To,From,To);
					for( INT i=From;i<=To&&i>=0&&i<65536;i++ )
					{
						Chars(i) = 1;
					}
					HadDash=0;
					From=0;
					To=0;
				}
			}
			warnf(TEXT("Adding unicode character range %x-%x (%d-%d)"),From,To,From,To);
			for( INT i=From; i<=To && i>=0 && i<65536; i++ )
			{
				Chars(i) = 1;
			}

		}

		INT j=MinRangeCharacter;
		INT Min=65536, Max=0;
		for( INT i=MinRangeCharacter; i<65536; i++ )
		{
			if( Chars(i) )
			{
				if( i < Min )
				{
					Min = i;
				}
				if( i > Max )
				{
					Max = i;
				}

				Font->CharRemap.Set( i, j );
				InverseMap.Set( j++, i );
			}
		}

		warnf(TEXT("Importing %d characters (unicode range %04x-%04x)"), j, Min, Max);

		CharsPerPage = j;
	}
	else
	{
		// No range specified, so default to the ASCII range
		CharsPerPage = 256;
	}

	// Add space for characters.
	Font->Characters.AddZeroed(CharsPerPage * NumResolutions );
    
	// If all upper case chars have lower case char counterparts no mapping is required.   
	if( !Font->IsRemapped )
	{
		bool NeedToRemap = false;
        
		for( const TCHAR* p = *ImportOptions->Data.Chars; *p; p++ )
		{
			TCHAR c;
            
			if( !appIsAlpha( *p ) )
			{
				continue;
			}
            
			if( appIsUpper( *p ) )
			{
				c = appToLower( *p );
			}
			else
			{
				c = appToUpper( *p );
			}

			if( appStrchr(*ImportOptions->Data.Chars, c) )
			{
				continue;
			}
            
			NeedToRemap = true;
			break;
		}
        
		if( NeedToRemap )
		{
			Font->IsRemapped = 1;

			for( const TCHAR* p = *ImportOptions->Data.Chars; *p; p++ )
			{
				TCHAR c;

				if( !appIsAlpha( *p ) )
				{
					Font->CharRemap.Set( *p, *p );
					InverseMap.Set( *p, *p );
					continue;
				}
                
				if( appIsUpper( *p ) )
				{
					c = appToLower( *p );
				}
				else
				{
					c = appToUpper( *p );
				}

				Font->CharRemap.Set( *p, *p );
				InverseMap.Set( *p, *p );

				if( !appStrchr(*ImportOptions->Data.Chars, c) )
				{
					Font->CharRemap.Set( c, *p );
				}
			}
		}
	}


	// Get the Logical Pixels Per Inch to be used when calculating the height later
	HDC tempDC = CreateCompatibleDC( NULL );
	FLOAT LogicalPPIY = (FLOAT)GetDeviceCaps(tempDC, LOGPIXELSY) / 72.0;
	
	DeleteDC( tempDC );

	const INT TotalProgress = NumResolutions * CharsPerPage;

	DWORD ImportCharSet = DEFAULT_CHARSET;
	switch ( ImportOptions->Data.CharacterSet )
	{
		case FontICS_Ansi:
			ImportCharSet = ANSI_CHARSET;
			break;

		case FontICS_Default:
			ImportCharSet = DEFAULT_CHARSET;
			break;

		case FontICS_Symbol:
			ImportCharSet = SYMBOL_CHARSET;
			break;
	}

	for( INT Page = 0; Page < NumResolutions; ++Page )
	{
		INT nHeight = -appRound( ResHeights(Page) * LogicalPPIY );

		// Create the Windows font
		HFONT FontHandle =
			CreateFont(
				nHeight,
				0,
				0,
				0,
				ImportOptions->Data.bEnableBold ? FW_BOLD : FW_NORMAL,
				ImportOptions->Data.bEnableItalic,
				ImportOptions->Data.bEnableUnderline,
				0,
				ImportCharSet,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				ImportOptions->Data.bEnableAntialiasing ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY,
				VARIABLE_PITCH,
				*ImportOptions->Data.FontName );

		if( FontHandle == NULL ) 
		{
			Warn->Logf( NAME_Error, TEXT("CreateFont failed: %s"), appGetSystemErrorMessage() );
			return FALSE;
		}

		// Create DC
		HDC DeviceDCHandle = GetDC( NULL );
		if( DeviceDCHandle == NULL )
		{
			Warn->Logf( NAME_Error, TEXT("GetDC failed: %s"), appGetSystemErrorMessage() );
			return FALSE;
		}

		HDC DCHandle = CreateCompatibleDC( DeviceDCHandle );
		if( !DCHandle )
		{
			Warn->Logf( NAME_Error, TEXT("CreateDC failed: %s"), appGetSystemErrorMessage() );
			return FALSE;
		}

		// Create bitmap
		BITMAPINFO WinBitmapInfo;
		appMemzero( &WinBitmapInfo, sizeof( WinBitmapInfo ) );
		HBITMAP BitmapHandle;
		void* BitmapDataPtr = NULL;
		if( ImportOptions->Data.bEnableAntialiasing )
		{
			WinBitmapInfo.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
			WinBitmapInfo.bmiHeader.biWidth         = ImportOptions->Data.TexturePageWidth;
			WinBitmapInfo.bmiHeader.biHeight        = ImportOptions->Data.TexturePageMaxHeight;
			WinBitmapInfo.bmiHeader.biPlanes        = 1;      //  Must be 1
			WinBitmapInfo.bmiHeader.biBitCount      = 32;
			WinBitmapInfo.bmiHeader.biCompression   = BI_RGB; 
			WinBitmapInfo.bmiHeader.biSizeImage     = 0;      
			WinBitmapInfo.bmiHeader.biXPelsPerMeter = 0;      
			WinBitmapInfo.bmiHeader.biYPelsPerMeter = 0;      
			WinBitmapInfo.bmiHeader.biClrUsed       = 0;      
			WinBitmapInfo.bmiHeader.biClrImportant  = 0;      

			BitmapHandle = CreateDIBSection(
				(HDC)NULL, 
				&WinBitmapInfo,
				DIB_RGB_COLORS,
				&BitmapDataPtr,
				NULL,
				0);  
		}
		else
		{
			BitmapHandle = CreateBitmap( ImportOptions->Data.TexturePageWidth, ImportOptions->Data.TexturePageMaxHeight, 1, 1, NULL);
		}

		if( BitmapHandle == NULL )
		{
			Warn->Logf( NAME_Error, TEXT("CreateBitmap failed: %s"), appGetSystemErrorMessage() );
			return FALSE;
		}

		SelectObject( DCHandle, FontHandle );

		// Grab size information for this font
		TEXTMETRIC WinTextMetrics;
		GetTextMetrics( DCHandle, &WinTextMetrics );

		HBITMAP LastBitmapHandle = ( HBITMAP )SelectObject( DCHandle, BitmapHandle );
		SetTextColor( DCHandle, 0x00ffffff );
		SetBkColor( DCHandle, 0x00000000 );

		// clear the bitmap
		HBRUSH Black = CreateSolidBrush(0x00000000);
		RECT r = {0, 0, ImportOptions->Data.TexturePageWidth, ImportOptions->Data.TexturePageMaxHeight};
		FillRect( DCHandle, &r, Black );

		INT X=0, Y=0, RowHeight=0;
		for( INT CurCharIndex = 0; CurCharIndex < CharsPerPage; ++CurCharIndex )
		{
  			GWarn->UpdateProgress( Page * CharsPerPage + CurCharIndex, TotalProgress );

			// Remap the character if we need to
			TCHAR Char = ( TCHAR )CurCharIndex;
			if( Font->IsRemapped )
			{
				TCHAR* FoundRemappedChar = InverseMap.Find( Char );
				if( FoundRemappedChar != NULL )
				{
					Char = *FoundRemappedChar;
				}
				else
				{
					// Skip missing remapped character
					continue;
				}
			}


			// Skip ASCII character if it isn't in the list of characters to import.
			if( Char < 256 && ImportOptions->Data.Chars != TEXT("") && (!Char || !appStrchr(*ImportOptions->Data.Chars, Char)) )
			{
				continue;
			}

			// Skip if the user has requested that only printable characters be
			// imported and the character isn't printable
			if( ImportOptions->Data.bCreatePrintableOnly == TRUE && iswprint(Char) == FALSE )
			{
				continue;
			}

			
			// Compute the size of the character
			INT CharWidth = 0;
			INT CharHeight = 0;
			{
				TCHAR Tmp[2];
				Tmp[0] = Char;
				Tmp[1] = 0;

				SIZE Size;
				GetTextExtentPoint32( DCHandle, Tmp, 1, &Size );

				CharWidth = Size.cx;
				CharHeight = Size.cy;
			}
			
			
			// OK, now try to grab glyph data using the GetGlyphOutline API.  This is only supported for vector-based fonts
			// like TrueType and OpenType; it won't work for raster fonts!
			UBOOL bUsingGlyphOutlines = FALSE;
			GLYPHMETRICS WinGlyphMetrics;
			const MAT2 WinIdentityMatrix2x2 = { 0,1, 0,0, 0,0, 0,1 };
			INT VerticalOffset = 0;
			UINT GGODataSize = 0;
			if( !ImportOptions->Data.bEnableLegacyMode && ImportOptions->Data.bEnableAntialiasing )    // We only bother using GetGlyphOutline for AntiAliased fonts!
			{
				GGODataSize =
					GetGlyphOutlineW(
						DCHandle,                         // Device context
						Char,	                            // Character
						GGO_GRAY8_BITMAP,                 // Format
						&WinGlyphMetrics,                 // Out: Metrics
						0,																// Output buffer size
						NULL,															// Glyph data buffer or NULL
						&WinIdentityMatrix2x2 );          // Transform

				if( GGODataSize != GDI_ERROR && GGODataSize != 0)
				{
					CharWidth = WinGlyphMetrics.gmBlackBoxX;
					CharHeight = WinGlyphMetrics.gmBlackBoxY;

					// If the character is bigger than our texture size, then this isn't going to work!  The user
					// will need to specify a larger texture resolution
					if( CharWidth > ImportOptions->Data.TexturePageWidth ||
						CharHeight > ImportOptions->Data.TexturePageMaxHeight )
					{
						warnf( TEXT( "At the specified font size, at least one font glyph would be larger than the maximum texture size you specified.") );
						DeleteDC( DCHandle );
						DeleteObject( BitmapHandle );
						return FALSE;
					}
					
					VerticalOffset = WinTextMetrics.tmAscent - WinGlyphMetrics.gmptGlyphOrigin.y;

					// Extend the width of the character by 1 (empty) pixel for spacing purposes.  Note that with the legacy
					// font import, we got this "for free" from TextOut
					// @todo frick: Properly support glyph origins and cell advancement!  The only reason we even want to
					//    to continue to do this is to prevent texture bleeding across glyph cell UV rects
					++CharWidth;

					bUsingGlyphOutlines = TRUE;
				}
				else
				{
					// GetGlyphOutline failed; it's probably a raster font.  Oh well, no big deal.
				}
			}

			// Adjust character dimensions to accommodate a drop shadow
			if( ImportOptions->Data.bEnableDropShadow )
			{
				CharWidth += 1;
				CharHeight += 1;
			}

			// If it doesn't fit right here, advance to next line.
			if( CharWidth + X + 2 > ImportOptions->Data.TexturePageWidth)
			{
				X = 0;
				Y = Y + RowHeight + ImportOptions->Data.YPadding;
				RowHeight = 0;
			}
			INT OldRowHeight = RowHeight;
			if( CharHeight > RowHeight )
			{
				RowHeight = CharHeight;
			}

			// new page
			if( Y+RowHeight > ImportOptions->Data.TexturePageMaxHeight )
			{
				Font->Textures.AddItem( CreateTextureFromDC( Font, (DWORD)DCHandle, Y+OldRowHeight, CurrentTexture ) );
				CurrentTexture++;

				// blank out DC
				HBRUSH Black = CreateSolidBrush(0x00000000);
				RECT r = {0, 0, ImportOptions->Data.TexturePageWidth, ImportOptions->Data.TexturePageMaxHeight};
				FillRect( DCHandle, &r, Black );

				X = 0;
				Y = 0;
				RowHeight = 0;
			}

			// NOTE: This extra offset is for backwards compatibility with legacy TT/raster fonts.  With the new method
			//   of importing fonts, this offset is not needed since the glyphs already have the correct vertical size
			const INT ExtraVertOffset = bUsingGlyphOutlines ? 0 : 1;

			// Set font character information.
			FFontCharacter& NewCharacterRef = Font->Characters( CurCharIndex + (CharsPerPage * Page));
			NewCharacterRef.StartU =
				Clamp<INT>( X - ImportOptions->Data.ExtendBoxLeft,
							0, ImportOptions->Data.TexturePageWidth - 1 );
			NewCharacterRef.StartV =
				Clamp<INT>( Y + ExtraVertOffset-ImportOptions->Data.ExtendBoxTop,
							0, ImportOptions->Data.TexturePageMaxHeight - 1 );
			NewCharacterRef.USize =
				Clamp<INT>( CharWidth + ImportOptions->Data.ExtendBoxLeft + ImportOptions->Data.ExtendBoxRight,
							0, ImportOptions->Data.TexturePageWidth - NewCharacterRef.StartU );
			NewCharacterRef.VSize =
				Clamp<INT>( CharHeight + ImportOptions->Data.ExtendBoxTop + ImportOptions->Data.ExtendBoxBottom,
							0, ImportOptions->Data.TexturePageMaxHeight - NewCharacterRef.StartV );
			NewCharacterRef.TextureIndex = CurrentTexture;
			NewCharacterRef.VerticalOffset = VerticalOffset;



			// Draw character into font and advance.
			if( bUsingGlyphOutlines )
			{
				// GetGlyphOutline requires at least a DWORD aligned address
				BYTE* AlignedGlyphData = ( BYTE* )appMalloc( GGODataSize, DEFAULT_ALIGNMENT );
				check( AlignedGlyphData != NULL );

				// Grab the actual glyph bitmap data
				GetGlyphOutlineW(
					DCHandle,                         // Device context
					Char,	                            // Character
					GGO_GRAY8_BITMAP,                 // Format
					&WinGlyphMetrics,                 // Out: Metrics
					GGODataSize,                      // Data size
					AlignedGlyphData,                 // Out: Glyph data (aligned)
					&WinIdentityMatrix2x2 );          // Transform

				// Make sure source pitch is DWORD aligned
				INT SourceDataPitch = WinGlyphMetrics.gmBlackBoxX;
				if( SourceDataPitch % 4 != 0 )
				{
					SourceDataPitch += 4 - SourceDataPitch % 4;
				}
				BYTE* SourceDataPtr = AlignedGlyphData;

				const INT DestDataPitch = WinBitmapInfo.bmiHeader.biWidth * WinBitmapInfo.bmiHeader.biBitCount / 8;
				BYTE* DestDataPtr = ( BYTE* )BitmapDataPtr;
				check( DestDataPtr != NULL );

				// We're going to write directly to the bitmap, so we'll unbind it from the GDI first
				SelectObject( DCHandle, LastBitmapHandle );


				// Copy the glyph data to our bitmap!
				for( UINT SourceY = 0; SourceY < WinGlyphMetrics.gmBlackBoxY; ++SourceY )
				{
					for( UINT SourceX = 0; SourceX < WinGlyphMetrics.gmBlackBoxX; ++SourceX )
					{
						// Values are between 0 and 64 inclusive (64 possible shades, including black)
						BYTE Opacity = ( BYTE )( ( ( INT )SourceDataPtr[ SourceY * SourceDataPitch + SourceX ] * 255 ) / 64 );

						// Copy the opacity value into the RGB components of the bitmap, since that's where we'll be looking for them
						// NOTE: Alpha channel is set to zero
						const UINT DestX = X + SourceX;
						const UINT DestY = WinBitmapInfo.bmiHeader.biHeight - ( Y + SourceY ) - 1;  // Image is upside down!
						*( UINT* )&DestDataPtr[ DestY * DestDataPitch + DestX * sizeof( UINT ) ] =
							( Opacity ) |            // B
							( Opacity << 8 ) |       // G
							( Opacity << 16 );       // R
					}
				}

				// OK, we can rebind it now!
				SelectObject( DCHandle, BitmapHandle );

				appFree( AlignedGlyphData );
			}
			else
			{
				TCHAR Tmp[2];
				Tmp[0] = Char;
				Tmp[1] = 0;

				TextOut( DCHandle, X, Y, Tmp, 1 );
			}
			X = X + CharWidth + ImportOptions->Data.XPadding;
		}
		// save final page
		Font->Textures.AddItem( CreateTextureFromDC( Font, (DWORD)DCHandle, Y+RowHeight, CurrentTexture ) );
		CurrentTexture++;

		DeleteDC( DCHandle );
		DeleteObject( BitmapHandle );
	}

	// Store character count
	Font->CacheCharacterCountAndMaxCharHeight();

	GWarn->UpdateProgress( TotalProgress, TotalProgress );

	return TRUE;
}
#endif



void UTrueTypeMultiFontFactory::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);

	UArrayProperty*	RTProp = new(GetClass(),TEXT("ResTests"),	  RF_Public)UArrayProperty(CPP_PROPERTY(ResTests  ), TEXT(""), CPF_Edit );
	check(RTProp);
	RTProp->Inner = new(RTProp,TEXT("FloatProperty0"),RF_Public) UFloatProperty(EC_CppProperty,0,TEXT(""),CPF_Edit);

	UArrayProperty* RHProp = new(GetClass(),TEXT("ResHeights"), RF_Public)UArrayProperty(CPP_PROPERTY(ResHeights), TEXT(""), CPF_Edit );
	check(RHProp);
	RHProp->Inner = new(RHProp, TEXT("FloatProperty0"), RF_Public) UFloatProperty(EC_CppProperty,0,TEXT(""),CPF_Edit);

	UArrayProperty* RFProp = new(GetClass(),TEXT("ResFonts"), RF_Public)UArrayProperty(CPP_PROPERTY(ResFonts), TEXT(""), CPF_Edit );
	check(RFProp);
	RFProp->Inner = new(RFProp, TEXT("ObjectProperty0"), RF_Public) UObjectProperty( EC_CppProperty,0,TEXT(""),CPF_Edit, UFont::StaticClass() );
}



/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UTrueTypeMultiFontFactory::InitializeIntrinsicPropertyValues()
{
	Super::InitializeIntrinsicPropertyValues();
	SupportedClass = UMultiFont::StaticClass();
	Description = TEXT("MultiFont Imported From TrueType");
}



UTrueTypeMultiFontFactory::UTrueTypeMultiFontFactory()
{
}



UObject* UTrueTypeMultiFontFactory::FactoryCreateNew(
	UClass* Class,
	UObject* InParent,
	FName Name,
	EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext*	Warn )
{
#if !__WIN32__
	STUBBED("Win32 TTF code");
	return NULL;
#else
	check(Class==UMultiFont::StaticClass());

	// Make sure we are properly setup
	if (ResTests.Num() == 0)
	{
		Warn->Logf(NAME_Error, TEXT("Could not create fonts: At least 1 resolution test is required"));
		return NULL;
	}
	if (ResFonts.Num() > 0)
	{
		if ( ResTests.Num() != ResFonts.Num() )
		{
			Warn->Logf( NAME_Error, TEXT("Could not combine fonts: Resolution Tests must equal Heights & Fonts"));
			return NULL;
		}
	}
	else if ( ResTests.Num() != ResHeights.Num() )
	{
		Warn->Logf( NAME_Error, TEXT("Could not create fonts: Resolution Tests must equal Heights/Fonts"));
		return NULL;
	}

	// Create font and its texture.
	UMultiFont* Font = new( InParent, Name, Flags )UMultiFont;

	// Copy the Resolution Tests
	Font->ResolutionTestTable = ResTests;



	// Check to see if we are converting fonts, or creating a new font
	if ( ResFonts.Num() > 0 )		// <--- Converting
	{
		// Zero out the Texture Index
		INT CurrentTexture = 0;

		// Copy the font information
		Font->Kerning = ResFonts(0)->Kerning;
		Font->IsRemapped = ResFonts(0)->IsRemapped;
		Font->CharRemap = ResFonts(0)->CharRemap;

		INT CharIndex = 0;

		// Process each font
		for ( INT Fnt = 0; Fnt < ResFonts.Num() ; Fnt++ )
		{
			if (!Cast<UMultiFont>( ResFonts(Fnt) ))
			{
				// Make duplicates of the Textures	
				for (INT i=0;i <ResFonts(Fnt)->Textures.Num();i++)
				{
					FString TextureString = FString::Printf( TEXT("%s_Page"), *Font->GetName() );

					if( CurrentTexture < 26 )
					{
						TextureString = TextureString + FString::Printf(TEXT("%c"), 'A'+CurrentTexture);
					}
					else
					{
						TextureString = TextureString + FString::Printf(TEXT("%c%c"), 'A'+CurrentTexture/26, 'A'+CurrentTexture%26 );
					}

					UTexture2D* FontTex = Cast<UTexture2D>( StaticDuplicateObject(ResFonts(Fnt)->Textures(i),ResFonts(Fnt)->Textures(i), Font, *TextureString));
					Font->Textures.AddItem(FontTex);
				}

				// Now duplicate the characters and fix up their references
				Font->Characters.AddZeroed( ResFonts(Fnt)->Characters.Num() );
				for (INT i=0;i<ResFonts(Fnt)->Characters.Num();i++)
				{
					Font->Characters(CharIndex) = ResFonts(Fnt)->Characters(i);
					Font->Characters(CharIndex).TextureIndex += CurrentTexture;
					CharIndex++;
				}

				CurrentTexture += ResFonts(Fnt)->Textures.Num();
			}
			else
			{
				Warn->Logf( NAME_Error, TEXT("Could not process %s because it's already a multifont.. Skipping!"),*ResFonts(Fnt)->GetFullName() );
			}
		}
	}
	else
	{
		// OK, we're creating a new font!

		GWarn->BeginSlowTask( *LocalizeUnrealEd( TEXT( "FontFactory_ImportingTrueTypeFont" ) ), TRUE );

		// For multi-resolution fonts, we'll pass in our per-resolution height array
		if( !ImportTrueTypeFont( Font, Warn, ResTests.Num(), ResHeights ) )
		{
			// Error importing font
			Font = NULL;
		}			

		GWarn->EndSlowTask();
	}

	return Font;
#endif
}
