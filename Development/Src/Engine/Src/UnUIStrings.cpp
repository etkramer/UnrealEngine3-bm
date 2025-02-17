/*=============================================================================
	UnUIStrings.cpp: UI structs, utility, and helper class implementations for rendering and formatting strings
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// engine classes
#include "EnginePrivate.h"

// widgets and supporting UI classes
#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"
#include "UnUIMarkupResolver.h"
#include "Localization.h"

/**
 * Allows access to UObject::GetLanguage() from modules which aren't linked against Core
 */
const TCHAR* appGetCurrentLanguage()
{
	return UObject::GetLanguage();
}

IMPLEMENT_CLASS(UUIString);
	IMPLEMENT_CLASS(UUIEditboxString);
	IMPLEMENT_CLASS(UUIListString);

DECLARE_CYCLE_STAT(TEXT("ParseString Time"),STAT_UIParseString,STATGROUP_UI);

/* ==========================================================================================================
	UUIString
========================================================================================================== */
/**
 * Find the data store that has the specified tag.
 *
 * @param	DataStoreTag	A name corresponding to the 'Tag' property of a data store
 *
 * @return	a pointer to the data store that has a Tag corresponding to DataStoreTag, or NULL if no data
 *			were found with that tag.
 */
UUIDataStore* UUIString::ResolveDataStore( FName DataStoreTag ) const
{
	UUIDataStore* Result = NULL;

	UUIScreenObject* OwnerWidget = GetOuterUUIScreenObject();
	if ( !OwnerWidget->HasAnyFlags(RF_ClassDefaultObject) )
	{
		UUIScene* OwnerScene = OwnerWidget->GetScene();
		if ( OwnerScene != NULL )
		{
			// Try to use the player owner for the widget if it has one.
			ULocalPlayer* PlayerOwner = NULL;
			TArray<INT> SupportedPlayers;
			OwnerWidget->GetInputMaskPlayerIndexes(SupportedPlayers);

			if(SupportedPlayers.Num() == 1)
			{
				PlayerOwner = OwnerWidget->GetPlayerOwner(SupportedPlayers(0));
			}

			// Resolve the datastore using the scene.
			Result = OwnerScene->ResolveDataStore(DataStoreTag, PlayerOwner);
		}
	}

	return Result;
}

/**
 * Calculates the width and height of a typical character in the specified font.
 *
 * @param	DrawFont			the font to use for calculating the character size
 * @param	DefaultCharWidth	[out] will be set to the width of the typical character
 * @param	DefaultCharHeight	[out] will be set to the height of the typical character
 * @param	pDefaultChar		if specified, pointer to a single character to use for calculating the default character size
 */
static void GetDefaultCharSize( UFont* DrawFont, FLOAT& DefaultCharWidth, FLOAT& DefaultCharHeight, FLOAT ViewportHeight, const TCHAR* pDefaultChar=NULL )
{
	// Get the scaling and resolution information from the font.
	const INT PageIndex = DrawFont->GetResolutionPageIndex(ViewportHeight);

	TCHAR DefaultChar = pDefaultChar != NULL ? *pDefaultChar : TEXT('0');
	DrawFont->GetCharSize(DefaultChar, DefaultCharWidth, DefaultCharHeight, PageIndex);
	if ( DefaultCharWidth == 0 )
	{
		// this font doesn't contain '0', try 'A'
		DrawFont->GetCharSize(TEXT('A'), DefaultCharWidth, DefaultCharHeight, PageIndex);
	}
}

/**
 * Calculates the size of the specified string.
 *
 * @param	Parameters	Used for various purposes
 *							DrawXL:		[out] will be set to the width of the string
 *							DrawYL:		[out] will be set to the height of the string
 *							DrawFont:	specifies the font to use for retrieving the size of the characters in the string
 *							Scale:		specifies the amount of scaling to apply to the string
 * @param	pText		the string to calculate the size for
 * @param	EOL			a pointer to a single character that is used as the end-of-line marker in this string
 * @param	bStripTrailingCharSpace
 *						whether the inter-character spacing following the last character should be included in the calculated width of the result string
 */
void UUIString::StringSize( FRenderParameters& Parameters, const TCHAR* pText, const TCHAR* EOL/*=NULL*/, UBOOL bStripTrailingCharSpace/*=TRUE*/ )
{
	Parameters.DrawXL = 0.f;
	Parameters.DrawYL = 0.f;

	if( Parameters.DrawFont )
	{

		const TCHAR* pCurrentPos = pText;

		// Get the scaling and resolution information from the font.
		const FLOAT FontResolutionTest = GEngine != NULL && GEngine->GameViewport && GEngine->GameViewport->Viewport != NULL 
			? GEngine->GameViewport->Viewport->GetSizeY() : 768.f;
		const INT PageIndex = Parameters.DrawFont->GetResolutionPageIndex(FontResolutionTest);
		const FLOAT FontScale = Parameters.DrawFont->GetScalingFactor(FontResolutionTest);

		// get a default character width and height to be used for non-renderable characters
		FLOAT DefaultCharWidth, DefaultCharHeight;
		GetDefaultCharSize( Parameters.DrawFont, DefaultCharWidth, DefaultCharHeight, FontResolutionTest );

		// we'll need to use scaling in multiple places, so create a variable to hold it so that if we modify
		// how the scale is calculated we only have to update these two lines
		const FLOAT ScaleX = Parameters.Scaling.X * FontScale;
		const FLOAT ScaleY = Parameters.Scaling.Y * FontScale;


		const FLOAT CharIncrement = ( (FLOAT)Parameters.DrawFont->Kerning + Parameters.SpacingAdjust.X ) * ScaleX;
		const FLOAT DefaultScaledHeight = DefaultCharHeight * ScaleY + Parameters.SpacingAdjust.Y * ScaleY;
		while ( *pCurrentPos )
		{
			FLOAT CharWidth, CharHeight;

			// if an EOL character was specified, skip over that character in the stream
			if ( EOL != NULL )
			{
				while ( *pCurrentPos == *EOL )
				{
					Parameters.DrawYL = Max(Parameters.DrawYL, DefaultScaledHeight);
					pCurrentPos++;
				}

				if ( *pCurrentPos == 0 )
				{
					break;
				}
			}

			TCHAR Ch = *pCurrentPos++;
			Parameters.DrawFont->GetCharSize(Ch, CharWidth, CharHeight,PageIndex);
			if ( CharHeight == 0 && Ch == TEXT('\n') )
			{
				CharHeight = DefaultCharHeight;
			}
			CharWidth *= ScaleX;
			CharHeight *= ScaleY;

			// never add character spacing if the next character is whitespace
			if ( !appIsWhitespace(*pCurrentPos) )
			{
				// if we have another character or desire trailing char spacing to be included, append the character spacing
				if ( *pCurrentPos || !bStripTrailingCharSpace )
				{
					CharWidth += CharIncrement;
				}
			}

			const FLOAT ScaledVertSpacing = Parameters.SpacingAdjust.Y * ScaleY;

			Parameters.DrawXL += CharWidth;
			Parameters.DrawYL = Max<FLOAT>(Parameters.DrawYL, CharHeight + ScaledVertSpacing );
		}
	}
}


/**
 * Clips text to the bounding region specified.
 *
 * @param	Parameters			Various:
 *									DrawX:		[in] specifies the pixel location of the start of the bounding region that should be used for clipping
 *									DrawXL:		[in] specifies where absolute pixel value of the right side of the bounding region, in pixels
 *												[out] set to the width of out_ResultString
 *									DrawY:		unused
 *									DrawYL:		[out] set to the height of the string
 *									Scaling:	specifies the amount of scaling to apply to the string
 * @param	pText				the text that should be clipped
 * @param	out_ResultString	[out] a string containing all characters from the source string that fit into the bounding region
 * @param	ClipAlignment		controls which part of the input string is preserved (remains after clipping).  Only UIALIGN_Left and UIALIGN_Right are supported.
 * @param	bStripTrailingCharSpace
 *								whether the inter-character spacing following the last character should be included in the calculated width of the result string
 * @param	bClipToNearestEdge	indicates whether the last character should be included in the result string if its midpoint is inside the bounding region.
 */
void UUIString::ClipString( FRenderParameters& Parameters, const TCHAR* pText, FString& out_ResultString, EUIAlignment ClipAlignment/*=UIALIGN_Left*/, UBOOL bStripTrailingCharSpace/*=TRUE*/, UBOOL bClipToNearestEdge/*=FALSE*/ )
{
	check(pText != NULL);
	check(Parameters.DrawFont != NULL);

	if ( *pText == 0 )
		return;

	if ( ClipAlignment == UIALIGN_Center )
	{
		debugf(NAME_Warning, TEXT("UUIString::ClipString: Only left and right alignment is supported.  Changing ClipAlignment from UIALIGN_Center to UIALIGN_Left."));
		ClipAlignment = UIALIGN_Left;
	}

	// Get the scaling and resolution information from the font.
	const FLOAT ViewportHeight = Parameters.ViewportHeight > 0 
		? Parameters.ViewportHeight
		: (GEngine != NULL && GEngine->GameViewport && GEngine->GameViewport->Viewport != NULL
			? GEngine->GameViewport->Viewport->GetSizeY() : 768.f);

	const INT PageIndex = Parameters.DrawFont->GetResolutionPageIndex(ViewportHeight);
	const FLOAT FontScale = Parameters.DrawFont->GetScalingFactor(ViewportHeight);

	// we'll need to use scaling in multiple places, so create a variable to hold it so that if we modify
	// how the scale is calculated we only have to update these two lines
	const FLOAT ScaleX = Parameters.Scaling.X * FontScale;
	const FLOAT ScaleY = Parameters.Scaling.Y * FontScale;

	// add any additional spacing between characters here
	const FLOAT CharSpacing = ( (FLOAT)Parameters.DrawFont->Kerning + Parameters.SpacingAdjust.X ) * ScaleX;

	// get a default character width and height to be used for non-renderable characters
	FLOAT DefaultCharWidth, DefaultCharHeight;
	GetDefaultCharSize( Parameters.DrawFont, DefaultCharWidth, DefaultCharHeight, ViewportHeight );

	DefaultCharWidth *= ScaleX;
	DefaultCharHeight *= ScaleY;

	const INT SourceStringLength = appStrlen(pText);

	// store the value of the bounding region
	const FLOAT OrigX = Parameters.DrawX;
	const FLOAT ClipX = Parameters.DrawXL - OrigX;

	// initialize the output variables to empty values, in case the string is empty
	Parameters.DrawXL = 0.f;
	Parameters.DrawYL = 0.f;
	out_ResultString.Empty( SourceStringLength );

	const FLOAT ScaledVertSpacing = Parameters.SpacingAdjust.Y * ScaleY;

	// tracks the current width/height of the output text...once this overflows ClipX, the function returns
	FLOAT LineXL = 0.f;
	FLOAT LineYL = DefaultCharHeight + ScaledVertSpacing;

	/** the character to start processing on */
	const TCHAR* FirstCharacter = pText;

	/** the last character to process */
	const TCHAR* LastCharacter = &pText[SourceStringLength];

	/** the direction to traverse the string */
	const INT CharIncrement = ClipAlignment == UIALIGN_Left ? 1 : -1;
	
	/** this represents the character we're currently processing */
	const TCHAR* pChar = (ClipAlignment == UIALIGN_Left || SourceStringLength == 0)
		? FirstCharacter
		: LastCharacter - 1;

	// Process each character until the total width is equal to the bounding region
	while ( true )
	{
		if ( pChar < FirstCharacter )
		{
			checkSlow(ClipAlignment==UIALIGN_Right);

			// we've reached the beginning of the string
			break;
		}

		const TCHAR CurrentCharacter = *pChar;

		// move the character pointer to the next character in the string
		pChar += CharIncrement;
		if ( pChar > LastCharacter )
		{
			// we've reached the end of the string
			break;
		}

		// get the size for the next character in the string
		FLOAT CharWidth, CharHeight;
		Parameters.DrawFont->GetCharSize(CurrentCharacter, CharWidth, CharHeight, PageIndex);

		if ( CharHeight == 0 && CurrentCharacter == TEXT('\n') )
		{
			CharHeight = DefaultCharHeight;
		}

		// apply the scaling
		CharWidth *= ScaleX;
		CharHeight *= ScaleY;

		// check whether appending this character would overflow the bounding region.  we must perform this check prior
		// to appending the character spacing, since the spacing wouldn't appear in the output text anyway, and in some
		// cases it can be the character spacing itself that causes this character to overflow the bounding region.
		const UBOOL bCharOverflowsBounds = LineXL + CharWidth > ClipX;
		UBOOL bClippingToNearestEdge=FALSE;
		if ( bCharOverflowsBounds )
		{
			// this will be the last character - now we need to check whether we should include this character in the clipped
			// output string or not.  if bClipToNearestEdge is TRUE, figure out whether the mid-point of the character would
			// fall inside the bounding region.
			if ( bClipToNearestEdge )
			{
				const FLOAT CharMidpoint = CharWidth * 0.5f;
				if ( LineXL + CharMidpoint < ClipX + KINDA_SMALL_NUMBER )
				{
					// the midpoint falls inside the bounding region - include this character
					bClippingToNearestEdge = TRUE;
				}
			}
		}

		if ( bCharOverflowsBounds && !bClippingToNearestEdge )
		{
			// we've reached the end of the bounding region - stop here
			break;
		}

		// if we have another character (or we want to have the trailing character spacing included, add any optional character spacing
		const UBOOL bIsWhitespace = (CharIncrement > 0 ? appIsWhitespace(*pChar) : appIsWhitespace(CurrentCharacter));
		if ( !bIsWhitespace )
		{
			if (!bStripTrailingCharSpace 
			|| (pChar > FirstCharacter && pChar < LastCharacter && *pChar))
			{
				CharWidth += CharSpacing;
			}
		}

		// increment our tracking variables
		LineXL += CharWidth;
		LineYL = Max<FLOAT>(LineYL, CharHeight + ScaledVertSpacing );

		// add this character to the output string
		out_ResultString += CurrentCharacter;
		if ( bCharOverflowsBounds )
		{
			break;
		}
	}

	if ( ClipAlignment == UIALIGN_Right )
	{
		// if we're right aligned, the output string's value is backwards, so flip it
		out_ResultString.ReverseString();
	}

	// set the length and height of the clipped line
	Parameters.DrawXL = LineXL;
	Parameters.DrawYL = LineYL;
}

/**
 * Parses a single string into an array of strings that will fit inside the specified bounding region.
 *
 * @param	Parameters		Used for various purposes:
 *							DrawX:		[in] specifies the pixel location of the start of the horizontal bounding region that should be used for wrapping.
 *							DrawY:		[in] specifies the Y origin of the bounding region.  This should normally be set to 0, as this will be
 *										     used as the base value for DrawYL.
 *										[out] Will be set to the Y position (+YL) of the last line, i.e. the total height of all wrapped lines relative to the start of the bounding region
 *							DrawXL:		[in] specifies the pixel location of the end of the horizontal bounding region that should be used for wrapping
 *							DrawYL:		[in] specifies the height of the bounding region, in pixels.  A input value of 0 indicates that
 *										     the bounding region height should not be considered.  Once the total height of lines reaches this
 *										     value, the function returns and no further processing occurs.
 *							DrawFont:	[in] specifies the font to use for retrieving the size of the characters in the string
 *							Scaling:	[in] specifies the amount of scaling to apply to the string
 * @param	CurX			specifies the pixel location to begin the wrapping; usually equal to the X pos of the bounding region, unless wrapping is initiated
 * @param	pText			the text that should be wrapped
 * @param	out_Lines		[out] will contain an array of strings which fit inside the bounding region specified.  Does
 *							not clear the array first.
 * @param	EOL				a pointer to a single character that is used as the end-of-line marker in this string, for manual line breaks
 * @param	MaxLines		the maximum number of lines that can be created.
 */
void UUIString::WrapString( FRenderParameters& Parameters, FLOAT CurX, const TCHAR* pText, TArray<FWrappedStringElement>& out_Lines, const TCHAR* EOL/*=NULL*/, INT MaxLines/*=MAXINT*/ )
{
	check(pText != NULL);
	check(Parameters.DrawFont != NULL);

	if ( *pText == 0 )
		return;

	const FLOAT OrigX = Parameters.DrawX;
	const FLOAT OrigY = Parameters.DrawY;
	const FLOAT ClipX = Parameters.DrawXL;
	const FLOAT ClipY = Parameters.DrawYL == 0 ? MAX_FLT : (OrigY + Parameters.DrawYL);

	// Get the scaling and resolution information from the font.
	const FLOAT ViewportHeight = Parameters.ViewportHeight > 0 
		? Parameters.ViewportHeight
		: (GEngine != NULL && GEngine->GameViewport && GEngine->GameViewport->Viewport != NULL
			? GEngine->GameViewport->Viewport->GetSizeY() : 768.f);

	const INT PageIndex = Parameters.DrawFont->GetResolutionPageIndex(ViewportHeight);
	const FLOAT FontScale = Parameters.DrawFont->GetScalingFactor(ViewportHeight);

	// we'll need to use scaling in multiple places, so create a variable to hold it so that if we modify
	// how the scale is calculated we only have to update these two lines
	const FLOAT ScaleX = Parameters.Scaling.X * FontScale;
	const FLOAT ScaleY = Parameters.Scaling.Y * FontScale;

	// calculate the width of the default character for this font; this will be used as the size for non-printable characters
	FLOAT DefaultCharWidth, DefaultCharHeight;
	GetDefaultCharSize(Parameters.DrawFont, DefaultCharWidth, DefaultCharHeight, ViewportHeight, EOL);
	DefaultCharWidth *= ScaleX;
	DefaultCharHeight *= ScaleY;

	// used to determine what to do if we run out of space before the first word has been processed...
	// if the starting position indicates that this line is indented, we'll assume this means that destination region had
	// existing text on the starting line - in this case, we'll just wrap the entire word to the next line
	UBOOL bIndented = CurX != OrigX;

	// this represents the total height of the wrapped lines, relative to the Y position of the bounding region
	FLOAT& LineExtentY = Parameters.DrawY;

	// how much spacing to insert between each character
	//@note: add any additional spacing between characters here
	FLOAT CharIncrement = ( (FLOAT)Parameters.DrawFont->Kerning + Parameters.SpacingAdjust.X ) * ScaleX;

	const FLOAT ScaledVertSpacing = Parameters.SpacingAdjust.Y * ScaleY;

	INT LineStartPosition, WordStartPosition;
	LineStartPosition = WordStartPosition = 0;//FLocalizedWordWrapHelper::GetStartingPosition(pText);
	INT WordEndPosition = FLocalizedWordWrapHelper::GetNextBreakPosition(pText, WordStartPosition);

	const TCHAR* pCurrent = pText;
	while ( *pCurrent != 0 && out_Lines.Num() < MaxLines )
	{
 		// for each leading line break, add an empty line to the output array
		while ( FLocalizedWordWrapHelper::IsLineBreak(pText, pCurrent - pText, EOL) && out_Lines.Num() < MaxLines )
		{
			// if the next line would fall outside the clipping region, stop here
			if ( LineExtentY + DefaultCharHeight + ScaledVertSpacing > ClipY + 0.25f )
				return;

			new(out_Lines) FWrappedStringElement(TEXT(""),DefaultCharWidth,DefaultCharHeight + ScaledVertSpacing);
			LineExtentY += DefaultCharHeight + ScaledVertSpacing;

			// no longer indented
			bIndented = FALSE;

			// reset the starting position to the left edge of the bounding region for calculating the next line
			CurX  = OrigX;

			// since we've processed this character, advance all tracking variables forward by one
			pCurrent++;
			WordStartPosition++;
			LineStartPosition++;

		}

		// the running width of the characters evaluated in this line so far
		FLOAT TestXL = CurX;
		// the maximum height of the characters evaluated in this line so far
		FLOAT TestYL = 0;
		// the combined width of the characters from the beginning of the line to the wrap position
		FLOAT LineXL = 0;
		// the maximum height of the characters from the beginning of the line to the wrap position
		FLOAT LineYL = 0;
		// the maximum height of the characters from the beginning of the line to the last character processed before the line overflowed
		FLOAT LastYL = 0;
		// the width and height of the last character that was processed
		FLOAT ChW=0, ChH=0;

		UBOOL bProcessedWord = FALSE;
		INT ProcessedCharacters = 0;

		// Process each word until the current line overflows.
		for ( ; WordEndPosition != INDEX_NONE;
			WordStartPosition = WordEndPosition, WordEndPosition = FLocalizedWordWrapHelper::GetNextBreakPosition(pText, WordStartPosition) )
		{
			UBOOL bWordOverflowed = FALSE;
			UBOOL bManualLineBreak = FALSE;

			// calculate the widths of the characters in this word - if this word fits within the bounding region,
			// append the word width to the total line width; otherwise, create an entry in the output array containing the text
			// to the left of the current word, then reset all line tracking variables and start over from the beginning of the current word.
			for ( INT CharIndex = WordStartPosition; CharIndex < WordEndPosition; CharIndex++ )
			{
				if ( FLocalizedWordWrapHelper::IsLineBreak(pText, CharIndex, EOL) )
				{
					WordStartPosition = CharIndex;
					bManualLineBreak = TRUE;
					break;
				}

				Parameters.DrawFont->GetCharSize(pText[CharIndex], ChW, ChH, PageIndex);
				ChW *= ScaleX;
				ChH *= ScaleY;

				// if we have at least one more character in the string, add the inter-character spacing here
				if ( pText[CharIndex+1] && !appIsWhitespace(pText[CharIndex+1]) )
				{
					ChW += CharIncrement;
				}

				TestXL += ChW;
				TestYL = Max(TestYL, ChH + ScaledVertSpacing);

				if ( TestXL > ClipX && !appIsWhitespace(pText[CharIndex]) )
				{
					bWordOverflowed = TRUE;
					break;
				}

				LastYL = TestYL;
				ProcessedCharacters++;
			}

			if ( bWordOverflowed == TRUE )
			{
				break;
			}

			bProcessedWord = TRUE;
			LineXL = TestXL - CurX;
			LineYL = TestYL;

			if ( bManualLineBreak == TRUE )
			{
				if ( LineYL == 0 )
				{
					LineYL = DefaultCharHeight + ScaledVertSpacing;
				}
				break;
			}
		}

		// if the next line would fall outside the clipping region, stop here
		if ( LineExtentY + LineYL + ScaledVertSpacing > ClipY + 0.25f )
		{
			break;
		}

		// CreateNewLine
		
		// we've reached the end of the current line - add a new element to the output array which contains the character we've processed so far
		// WordStartPosition is now the position of the first character that should be on the next line, so the current line is everything from
		// the start of the current line up to WordStartPosition
		INT LineLength = WordStartPosition - LineStartPosition;

		FWrappedStringElement* CurrentLine = NULL;
		if ( bProcessedWord == FALSE && bIndented == TRUE )
		{
			// we didn't have enough space for the first word.  If indented, we shouldn't break the line in
			// the middle of the word; rather, create a blank line and wrap the entire word to the next line
			CurrentLine = new(out_Lines) FWrappedStringElement(TEXT(""),0,DefaultCharHeight + ScaledVertSpacing);
			LineExtentY += DefaultCharHeight + ScaledVertSpacing;

			// we're going to advance the pCurrent pointer by LineLength below, but since all the characters we processed are going on the next line,
			// we don't want to advance the pCurrent pointer at all
			LineLength = 0;
		}
		else
		{
			if ( bProcessedWord == FALSE && ProcessedCharacters == 0 )
			{
				// there wasn't enough room in the bounding region for the first character
				if ( *pCurrent != 0 )
				{
					CurrentLine = new(out_Lines) FWrappedStringElement(*FString::Printf(TEXT("%c"), *pCurrent++), TestXL, TestYL);
					LineExtentY += TestYL;
					WordStartPosition++;
				}
				else
				{
					// we've reached the end of the string
					break;
				}
			}
			else
			{
				if ( LineLength == 0 )
				{
					// not enough room to process the first word - we'll have to break up the word

					// set the line length to the number of characters that will fit into the bounding region
					LineLength = ProcessedCharacters;

					// advance the position to start the next line by the number of characters processed
					WordStartPosition += ProcessedCharacters;

					// now calculate the length and height of the processed characters - TestXL and TestYL include the dimensions of the character that
					// overflowed the bounds (which means it won't be part of this line), so subtract the last character's width from the running width
					// and use the height calculated after processing the previous character
					LineXL = TestXL - ChW;
					LineYL = LastYL;
				}

				// we've run out of space - copy the characters up to the beginning of the current word into the line text
				FString LineString;
				TArray<TCHAR>& LineStringChars = LineString.GetCharArray();
				if ( pCurrent )
				{
					LineStringChars.Add(LineLength + 1);
					appStrncpy(&LineStringChars(0), pCurrent, LineLength+1);

					CurrentLine = new(out_Lines) FWrappedStringElement(*LineString, LineXL, LineYL);
				}
				LineExtentY += TestYL;
			}
		}

		// reset the starting position to the left edge of the bounding region for calculating the next line
		CurX  = OrigX;

		// advance the character pointer beyond the characters that we just added to the wrapped line
		pCurrent += LineLength;

		// no longer indented
		bIndented = FALSE;

		if ( *pCurrent && FLocalizedWordWrapHelper::IsLineBreak(pText, pCurrent - pText, EOL) )
		{
			// if this line was forced by a manual line break, advance past the line break
			// so that we don't create a blank line at the top of this loop
			pCurrent++;
			WordStartPosition++;
		}

		// reset the line start marker to point to the beginning of the current word
		LineStartPosition = WordStartPosition;
	}
}

FLOAT UUIString::GetDefaultLineHeight( FLOAT ViewportHeight ) const
{
	FLOAT LineHeight = 0.f;

	if ( StringStyleData.IsInitialized() && StringStyleData.DrawFont != NULL )
	{
		FLOAT DefaultCharWidth = 0.f;

		for ( INT NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++ )
		{
			FUIStringNode* Node = Nodes(NodeIndex);
			if ( Node->IsTextNode() )
			{
				UFont* CellFont = StringStyleData.DrawFont;

				FUIStringNode_Text* TextNode = static_cast<FUIStringNode_Text*>(Node);
				if ( TextNode->IsModifierNode() && TextNode->GetNodeStyleData().DrawFont != NULL )
				{
					CellFont = TextNode->GetNodeStyleData().DrawFont;
				}

				// Get the scaling and resolution information from the font.
				const FLOAT FontScale = CellFont->GetScalingFactor(ViewportHeight);
				FLOAT CellHeight=0;
				GetDefaultCharSize(CellFont, DefaultCharWidth, CellHeight, ViewportHeight);
				CellHeight *= FontScale;

				LineHeight = Max(CellHeight, LineHeight);
			}
		}

		if ( LineHeight == 0 )
		{
			// Get the scaling and resolution information from the font.
			const FLOAT FontScale = StringStyleData.DrawFont->GetScalingFactor(ViewportHeight);
			GetDefaultCharSize(StringStyleData.DrawFont, DefaultCharWidth, LineHeight, ViewportHeight);
			LineHeight *= FontScale;
		}
	}

	return LineHeight;
}

/**
 * Changes the style data for this UIString.
 *
 * @return	TRUE if the string needs to be reformatted, indicating that the new style data was successfully applied
 *			to the string.  FALSE if the new style data was identical to the current style data or the new style data
 *			was invalid.
 */
UBOOL UUIString::SetStringStyle( const FUICombinedStyleData& NewStringStyle )
{
	UBOOL bResult = FALSE;
	
	if ( NewStringStyle.IsInitialized() )
	{
		if ( NewStringStyle != StringStyleData )
		{
			// validate that the new style data has a valid font
			checkf(NewStringStyle.DrawFont, TEXT("Attempting to apply string style which has an invalid NULL font to the string's StringStyleData '%s'"), *GetPathName());
			StringStyleData = NewStringStyle;

			RefreshNodeStyles();
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Changes the complete style for this UIString.
 *
 * @return	TRUE if the string needs to be reformatted, indicating that the new style data was successfully applied
 *			to the string.  FALSE if the new style data was identical to the current style data or the new style data
 *			was invalid.
 */
UBOOL UUIString::SetStringStyle( UUIStyle_Combo* NewStringStyle )
{
	UBOOL bResult = FALSE;

	if ( NewStringStyle != NULL )
	{
		FUICombinedStyleData NewStyleData = StringStyleData;
		NewStyleData.InitializeStyleDataContainer(NewStringStyle,FALSE);

		bResult = SetStringStyle(NewStyleData);
	}

	return bResult;
}

/**
 * Changes the text style for this UIString.
 *
 * @param	NewTextStyle	the new text style data to use
 *
 * @return	TRUE if the string needs to be reformatted, indicating that the new style data was successfully applied
 *			to the string.  FALSE if the new style data was identical to the current style data or the new style data
 *			was invalid.
 */
UBOOL UUIString::SetStringTextStyle( const FStyleDataReference& NewTextStyle )
{
	// we need to convert this FStyleDataReference into a FUICombinedStyleData so we can compare it to the string's current value, so
	// first grab the style object from the style reference
	UUIStyle_Text* TextStyleData = Cast<UUIStyle_Text>(NewTextStyle.GetStyleData());

	// next, validate that the new style has valid text style data
	checkf(TextStyleData, TEXT("Attempting to apply invalid text style data to the StringStyleData for '%s'"), *GetPathName());

	// validate that the new style has a valid font
	checkf(TextStyleData->StyleFont, TEXT("Attempting to apply a NULL font to the StringStyleData for '%s': %s"),
		*GetPathName(), *TextStyleData->GetPathName());

	// create a temporary combined style data based on the existing style data (this is so we can easily keep the image portion of the existing value)
	FUICombinedStyleData NewStyleData = StringStyleData;

	// now copy the new text style data into the temp copy, leaving the image portion alone
	NewStyleData.InitializeStyleDataContainer(TextStyleData, FALSE);

	// now apply the style data (if it's different)
	return SetStringStyle(NewStyleData);
}

/**
 * Changes the text style for this UIString.
 *
 * @param	NewSourceStyle	the UIStyle object to retrieve the new text style data from
 * @param	NewSourceState	the menu state corresponding to the style data to apply to the string
 *
 * @return	TRUE if the string needs to be reformatted, indicating that the new style data was successfully applied
 *			to the string.  FALSE if the new style data was identical to the current style data or the new style data
 *			was invalid.
 */
UBOOL UUIString::SetStringTextStyle( UUIStyle* NewSourceStyle, UUIState* NewSourceState )
{
	FStyleDataReference NewStyleReference(NewSourceStyle, NewSourceState);
	return SetStringTextStyle(NewStyleReference);
}

/**
 * Changes the image style for this UIString.
 *
 * @param	NewTextStyle	the new image style data to use
 *
 * @return	TRUE if the string needs to be reformatted, indicating that the new style data was successfully applied
 *			to the string.  FALSE if the new style data was identical to the current style data or the new style data
 *			was invalid.
 */
UBOOL UUIString::SetStringImageStyle( const FStyleDataReference& NewImageStyle )
{
	// we need to convert this FStyleDataReference into a FUICombinedStyleData so we can compare it to the string's current value.
	// first grab the style object from the style reference
	UUIStyle_Image* ImageStyleData = Cast<UUIStyle_Image>(NewImageStyle.GetStyleData());

	// next, validate that the new style has valid image style data
	checkf(ImageStyleData, TEXT("Attempting to apply invalid image style data to the StringStyleData for '%s'"), *GetPathName());

	// create a temporary combined style data based on the existing style data (this is so we can easily keep the text portion of the existing value)
	FUICombinedStyleData NewStyleData = StringStyleData;

	// now copy the new image style data into the temp copy, leaving the image portion alone
	NewStyleData.InitializeStyleDataContainer(ImageStyleData, FALSE);

	// now apply the style data (if it's different)
	return SetStringStyle(NewStyleData);
}

/**
 * Changes the image style for this UIString.
 *
 * @param	NewSourceStyle	the UIStyle object to retrieve the new image style data from
 * @param	NewSourceState	the menu state corresponding to the style data to apply to the string
 *
 * @return	TRUE if the string needs to be reformatted, indicating that the new style data was successfully applied
 *			to the string.  FALSE if the new style data was identical to the current style data or the new style data
 *			was invalid.
 */
UBOOL UUIString::SetStringImageStyle( UUIStyle* NewSourceStyle, UUIState* NewSourceState )
{
	FStyleDataReference NewStyleReference(NewSourceStyle, NewSourceState);
	return SetStringImageStyle(NewStyleReference);
}

/**
 * Retrieves the UIState that should be used for applying style data.
 */
UUIState* UUIString::GetCurrentMenuState() const
{
	return GetOuterUUIScreenObject()->GetCurrentState();
}

/**
 * Propagates the string's text and image styles to all existing string nodes.
 */
void UUIString::RefreshNodeStyles()
{
	FUIStringNodeModifier StyleModifier(StringStyleData, GetCurrentMenuState());
	for ( INT NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++ )
	{
		FUIStringNode* Node = Nodes(NodeIndex);

		// only text nodes can possibly contain modifier markup
		if ( Node->IsTextNode() )
		{
			FUIStringNode_Text* TextNode = (FUIStringNode_Text*)Node;
			if ( TextNode->IsModifierNode() )
			{
				// FMarkupTextChunk already has all of the methods we'll need to parse out the data store reference
				FMarkupTextChunk MarkupProcessor;

				const TCHAR* SourceChar = *TextNode->SourceText;

				// ProcessChunk will separate the data store tag from the value and remove the brackets
				verify(MarkupProcessor.ProcessChunk(SourceChar, FALSE));

				FString DataStoreTag, DataStoreValue;
				if ( MarkupProcessor.GetDataStoreMarkup(DataStoreTag, DataStoreValue) )
				{
					UUIDataStore* DataStore = ResolveDataStore(*DataStoreTag);

					// supposedly, this node has already been resolved into a data store reference correctly, so assert on this assumption
					check(DataStore);
					if ( !DataStore->ParseStringModifier(DataStoreValue, StyleModifier) )
					{
						debugf(TEXT("Failed to parse modifier data store reference - Source '%s' DataStoreTag '%s' DataStoreValue '%s'"), *TextNode->SourceText, *DataStoreTag, *DataStoreValue);
					}
				}
			}
		}

		Node->InitializeStyle(StyleModifier.GetCustomStyleData());
	}
}

/**
 * Removes all slave nodes which were created as a result of wrapping or other string formatting, appending their RenderedText
 * to the parent node.
 */
void UUIString::UnrollWrappedNodes()
{
	// iterate through all nodes in this string
	for ( INT NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++ )
	{
		FUIStringNode* Node = Nodes(NodeIndex);
		Node->bForceWrap = FALSE;
		
		// if this is a FormattedNodeParent
		if ( Node->IsFormattingParent() )
		{
			FUIStringNode_FormattedNodeParent* ParentNode = (FUIStringNode_FormattedNodeParent*)Node;
			const INT SlaveNodeIndex = NodeIndex + 1;

			// remove all nodes that were created to contain the post-wrapped text for this node
			while ( SlaveNodeIndex < Nodes.Num() && Nodes(SlaveNodeIndex)->IsSlaveNode(ParentNode) )
			{
				FUIStringNode* SlaveNode = Nodes(SlaveNodeIndex);

				// remove this node from the list
				Nodes.Remove(SlaveNodeIndex);

				// then cleanup the memory for this slave node
				delete SlaveNode;
			}

			// now replace the WrappedNodeParent with a normal text node so that it will calculate its extent properly
			FUIStringNode_Text* TextNode = new FUIStringNode_Text(*ParentNode);
			Nodes(NodeIndex) = TextNode;

			delete ParentNode;
		}
	}
}

/**
 * Retrieves the configured auto-scale percentage.
 *
 * @param	BoundingRegionSize		the bounding region to use for determining autoscale factor (only relevant for certain
 *									auto-scale modes).
 * @param	StringSize				the size of the string, unwrapped and non-scaled; (only relevant for certain
 *									auto-scale modes).
 * @param	out_AutoScalePercent	receives the autoscale percent value.
 */
void UUIString::GetAutoScaleValue( FVector2D BoundingRegionSize, FVector2D StringSize, FVector2D& out_AutoScalePercent ) const
{
	const ETextAutoScaleMode AutoScaleMode = static_cast<ETextAutoScaleMode>(StringStyleData.TextAutoScaling.AutoScaleMode);
	if ( AutoScaleMode == UIAUTOSCALE_Normal || AutoScaleMode == UIAUTOSCALE_Justified )
	{
		//@fixme ronp scalemode - Parameters.DrawY/DrawYL isn't set when we're autosizing the string vertically.
		out_AutoScalePercent.X = Max(BoundingRegionSize.X / StringSize.X, StringStyleData.TextAutoScaling.MinScale);
		out_AutoScalePercent.Y = Max(BoundingRegionSize.Y / StringSize.Y, StringStyleData.TextAutoScaling.MinScale);
		if ( AutoScaleMode == UIAUTOSCALE_Justified )
		{
			if ( StringSize.Y * out_AutoScalePercent.X > BoundingRegionSize.Y )
			{
				out_AutoScalePercent.X = out_AutoScalePercent.Y;
			}
			else
			{
				out_AutoScalePercent.Y = out_AutoScalePercent.X;
			}
			
			out_AutoScalePercent.X = Clamp<FLOAT>(out_AutoScalePercent.X,0.0f,1.0f);
			out_AutoScalePercent.Y = Clamp<FLOAT>(out_AutoScalePercent.Y,0.0f,1.0f);
		}
	}
	else if ( AutoScaleMode == UIAUTOSCALE_ResolutionBased )
	{
		out_AutoScalePercent.X = out_AutoScalePercent.Y = GetOuterUUIScreenObject()->GetAspectRatioAutoScaleFactor(StringStyleData.DrawFont);
	}
}

/**
 * This struct contains data used for formatting individual nodes within a UIString, during the execution of UUIString::ApplyFormatting()
 */
struct FNodeFormattingData
{
	/** Render parameters that are used for the node currently being processed */
	FRenderParameters&		Parameters;

	/** the current X position of the pen.  Cumulative. */
	FLOAT					CurX;

	/** the current Y position of the pen.  Cumulative. */
	FLOAT					CurY;

	/** the maximum value allowed for CurX; represents the right edge of the bounding region */
	FLOAT					ClipX;

	/** the maximum value allowed for CurY; represents the bottom edge of the bounding region */
	FLOAT					ClipY;

	/** the height of the last line from this string that was processed */
	FLOAT					LastYL;

	/** the original value for CurX, represents the left side of the string's bounding region */
	const FLOAT				OrigX;

	/** the original value for CurY, represents the top of the string's bounding region */
	const FLOAT				OrigY;

	/**
	 * the height of a standard character in the current font.  Used by FUIStringNode_Images
	 * to determine how to scale images, in some cases.
	 */
	const FLOAT				DefaultLineHeight;

	/**
	 * the amount of scaling to apply for strings which are configured to be auto-scaled
	 */
	FVector2D				AutoScale;

	/** Constructor */
	FNodeFormattingData( FRenderParameters& RenderParameters, FLOAT DefaultStringHeight )
	: Parameters(RenderParameters)
	, CurX(RenderParameters.DrawX)
	, CurY(RenderParameters.DrawY)
	, ClipX(RenderParameters.DrawX + RenderParameters.DrawXL)
	, ClipY(RenderParameters.DrawYL)
	, LastYL(0.f)
	, OrigX(RenderParameters.DrawX)
	, OrigY(RenderParameters.DrawY)
	, DefaultLineHeight(DefaultStringHeight)
	, AutoScale(1.f, 1.f)
	{
	}
};

#define ELLIPSIS_TEXT TEXT("...")

/**
 * Reformats this UIString's nodes to fit within the bounding region specified.
 *
 * @param	Parameters		Used for various purposes:
 *							DrawX:		[in] specifies the X position of the bounding region, in pixels
 *										[out] Will be set to the X position of the end of the last node in the string.
 *							DrawY:		[out] Will be set to the Y position of the last node in the string
 *							DrawXL:		[in] specifies the width of the bounding region, in pixels.
 *							DrawYL:		[in] specifies the bottom of the bounding region, in pixels.
 *							DrawFont:	unused
 *							Scale:		[in] specifies the scaling to apply to the string
 * @param	bIgnoreMarkup	if TRUE, does not attempt to process any markup and only one UITextNode is created
 */
void UUIString::ApplyFormatting( FRenderParameters& Parameters, UBOOL bIgnoreMarkup )
{
	FNodeFormattingData FormatData(Parameters, GetDefaultLineHeight(Parameters.ViewportHeight));

	FLOAT& CurX		= FormatData.CurX;
	FLOAT& CurY		= FormatData.CurY;
	FLOAT& ClipX	= FormatData.ClipX;
	FLOAT& ClipY	= FormatData.ClipY;

	// we'll need to use scaling in multiple places, so create a variable to hold it so that if we modify
	// how the scale is calculated we only have to update these two lines
	const FLOAT ScaleX = FormatData.Parameters.Scaling.X;
	const FLOAT ScaleY = FormatData.Parameters.Scaling.Y;

	// LastYL is the height of the last line that was encountered
	FLOAT& LastYL	= FormatData.LastYL;

	const FLOAT& OrigX				= FormatData.OrigX;
	const FLOAT& OrigY				= FormatData.OrigY;

	// We're going to recalculate the string's overall extent, so clear the current values
	StringExtent.X = StringExtent.Y = 0.f;
	// we also won't need to know the value of Parameters.YL (since we've moved it into ClipY) and the editbox uses this to determine
	// whether it contains any visible characters (AdjustNodeExtents)
	Parameters.DrawYL = 0.f;

	// unroll any previously wrapped nodes
	UnrollWrappedNodes();

	// first, calculate the raw extents for all nodes
	INT NodeIndex;

	// Store off the full extent for later
	FVector2D RawExtents(0,0);

	FLOAT ForcedScaleX = 1.f;
	FLOAT ForcedScaleY = 1.f;

	const ETextAutoScaleMode AutoScaleMode = static_cast<ETextAutoScaleMode>(StringStyleData.TextAutoScaling.AutoScaleMode);
	if ( AutoScaleMode == UIAUTOSCALE_ResolutionBased )
	{
		GetAutoScaleValue(FVector2D(Parameters.DrawXL, ClipY - OrigY), RawExtents, FormatData.AutoScale);
	}

	for ( NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++ )
	{
		FUIStringNode* StringNode = Nodes(NodeIndex);

		//@fixme ronp scalemode - we should always use the configured scaling.  If two labels have the same text
		// and one has a scaling of e.g. 0.75, it should be smaller than the other.
		if (AutoScaleMode == UIAUTOSCALE_Normal ||	AutoScaleMode == UIAUTOSCALE_Justified)
		{
			StringNode->Scaling.X = 1.f;
			StringNode->Scaling.Y = 1.f;
		}
		else if ( StringNode->IsTextNode() )
		{
			StringNode->Scaling.X = ScaleX;
			StringNode->Scaling.Y = ScaleY;
		}
		if ( AutoScaleMode == UIAUTOSCALE_ResolutionBased )
		{
			StringNode->Scaling.X *= FormatData.AutoScale.X;
			StringNode->Scaling.Y *= FormatData.AutoScale.Y;
		}

		StringNode->CalculateExtent(FormatData.DefaultLineHeight, Parameters.ViewportHeight);
		
		RawExtents.X += StringNode->Extent.X;
		RawExtents.Y = Max(RawExtents.Y, StringNode->Extent.Y);
	}

	if ( AutoScaleMode != UIAUTOSCALE_ResolutionBased )
	{
		GetAutoScaleValue(FVector2D(Parameters.DrawXL, ClipY - OrigY), RawExtents, FormatData.AutoScale);
		ForcedScaleX = FormatData.AutoScale.X;
		ForcedScaleY = FormatData.AutoScale.Y;
	}

	// allow any child classes to override the extents calculated for each node, replace nodes with more specialized nodes, initialize nodes, etc.
	const UBOOL bForceLeftClipAlignment = AdjustNodeExtents(FormatData);

	for ( NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++ )
	{
		FUIStringNode* StringNode = Nodes(NodeIndex);
		if ( !StringNode->Extent.IsZero() )
		{
			if ( StringNode->IsTextNode() )
			{
				FUIStringNode_Text* TextNode = static_cast<FUIStringNode_Text*>(StringNode);
				FUICombinedStyleData& NodeStyleData = TextNode->GetNodeStyleData();

				// determine if this node needs further processing

				// does this node overflow the bounding region?
				const UBOOL bOverflowsBounds = CurX + (StringNode->Extent.X * ForcedScaleX) > ClipX + KINDA_SMALL_NUMBER;

				// should this node be wrapped?
				const UBOOL bShouldWrapNode = TextNode->bForceWrap || TextNode->RenderedText.InStr(TEXT("\n")) != INDEX_NONE;

				if ( bOverflowsBounds || bShouldWrapNode )
				{
					// initialize the parameters for wrapping this node
					Parameters.DrawX = OrigX;
					Parameters.DrawXL = ClipX;
					Parameters.DrawY = 0;
					Parameters.DrawYL = 0;
					Parameters.Scaling = TextNode->Scaling *= FVector2D(ForcedScaleX, ForcedScaleY);
					Parameters.DrawFont = NodeStyleData.DrawFont;
					Parameters.SpacingAdjust = NodeStyleData.TextSpacingAdjust;

					if ( StringStyleData.TextClipMode == CLIP_Wrap )
					{
						// if bForceWrap is true, this node has already been wrapped - we don't need to wrap it again
						if ( TextNode->bForceWrap )
						{
							// move the cursor back to the left side of the bounding region, then advance it past this text node
							CurX = OrigX + StringNode->Extent.X * ForcedScaleX;

							// move the cursor to the next line
							CurY += LastYL;

							// since this will be the first node on the new line, set LastYL to the vertical extent of this node
							LastYL = StringNode->Extent.Y;

							StringExtent.X = Max<FLOAT>(StringExtent.X, StringNode->Extent.X);
						}
						else
						{
							// wrap this node's processed text into multiple lines
							TArray<FWrappedStringElement> Lines;
							UUIString::WrapString(Parameters, CurX, *TextNode->RenderedText, Lines, TEXT("\n"));

							// this is the width of all previous nodes on this line
							FLOAT CurrentLineExtent = CurX - OrigX;
							StringExtent.X = Max<FLOAT>(StringExtent.X, CurrentLineExtent);
							if ( Lines.Num() > 0 )
							{
								// this is the node that will contain the unwrapped version of the string
								FUIStringNode_FormattedNodeParent* WrappingParent = new FUIStringNode_FormattedNodeParent(*TextNode);
								WrappingParent->CalculateExtent(FormatData.DefaultLineHeight, Parameters.ViewportHeight);

								// we'll replace the existing node with a WrappedNodeParent and a single UIStringNode_Text node for each line, so replace the original version
								Nodes(NodeIndex) = WrappingParent;

								for ( INT LineIndex = 0; LineIndex < Lines.Num(); LineIndex++ )
								{
									FWrappedStringElement& CurrentLine = Lines(LineIndex);

									// if the first line's value is blank and its horizontal extent is 0, there wasn't enough room to render the first word of NodeA,
									// so it was moved to the next line...in this case, we want to increase the string's extent, but not include
									// this node in the string's list of nodes
									UBOOL bIndentedDummyNode = (LineIndex == 0 && CurrentLine.Value.Len() == 0 && CurrentLine.LineExtent.X == 0);
									if ( bIndentedDummyNode == FALSE )
									{
										// create a new text node for each line of the original node
										FUIStringNode_Text* SlaveNode = new FUIStringNode_Text(TEXT(""));

										// Propagate the formatting values from the pre-existing node to the newly created node that will contain only the wrapped text
										FUICombinedStyleData& SlaveNodeStyleData = SlaveNode->GetNodeStyleData();

										SlaveNodeStyleData = TextNode->GetNodeStyleData();
										SlaveNode->ParentNode = WrappingParent;
										SlaveNode->Scaling = TextNode->Scaling;
										SlaveNode->Extent = CurrentLine.LineExtent;
										SlaveNode->bForceWrap = LineIndex > 0;
										SlaveNode->SetRenderText(*CurrentLine.Value);
										Nodes.InsertItem(SlaveNode, ++NodeIndex);
									}

									/*
									when determining how to manipulate CurX, CurY, LastYL, there are a number of cases to consider.

									NodeA represents the node that contains the first line of wrapped text.
									1: NodeA is the first node in the original line
										1.1: there is enough space on NodeA's line for the first word of the next non-wrapped node
											* illegal case, as this would mean that the wrapping code didn't work correctly.
										1.2: there is not enough space on NodeA's line for the first word of the next non-wrapped node
											= subsequent nodes will be placed on the next line, at OrigX
									2: NodeA is middle node on the original line
										* illegal, should not be wrapped if this occurs
									3: NodeA is the last node on the original line
										3.1: NodeA != NodeZ
											= subsequent nodes will be placed on the next line, at OrigX
										3.2: NodeA == NodeZ; occurs if the final character in the source node is a space,
											and the space is what caused the original text to overflow the bounding region
											= subsequent nodes will be placed on the next line, at OrigX

									NodeZ represents the node that contains the last line of wrapped text.
									4: NodeZ is the first node on a new line
										4.1: NodeZ's horizontal extent is less than the width of the bounding region
											4.1.a: there is enough space on NodeZ's line for the first word of the next non-wrapped node
												= subsequent nodes will be placed on the same line, at OrigX + CurrentLine.LineExtent.X
											4.1.b: there is not enough space on NodeZ's line for the first word of the next non-wrapped node
												= subsequent nodes will be placed on the next line, at OrigX (this will actually be handled by wrapping the next node)

									5: NodeZ is NOT the first node on a line - this occurs if NodeA == NodeZ.
										5.1: there is enough space on NodeZ's line for the first word of the next non-wrapped node
											* illegal
										5.2: there is not enough space on NodeZ's line for the first word of the next non-wrapped node
											= subsequent nodes will be placed on the next line, at OrigX
									*/


									FLOAT TotalLineExtent = CurrentLine.LineExtent.X;
									const UBOOL bLastWrappedLine = LineIndex == Lines.Num() - 1;
									if ( LineIndex == 0 )
									{
										// if this is the first wrapped node and it is going on the same line as previous nodes in the string, then LastYL is already set to
										// the maximum height of all of the previous nodes on this same line
										if ( bIndentedDummyNode == FALSE )
										{
											// if this is the first line of the wrapped set and it's the last node on the original line (case 3.1)
											// the extent for this line must include the extent for the previous nodes on this line in the string
											TotalLineExtent += CurrentLineExtent;
											LastYL = Max<FLOAT>(LastYL, CurrentLine.LineExtent.Y);
										}

										// move the cursor to the next line
										CurY += LastYL;
										if ( bIndentedDummyNode == TRUE )
										{
											LastYL = CurrentLine.LineExtent.Y;
										}

										// case 1: no special handling necessary
										CurX = OrigX;
									}
									else
									{
										if ( !bLastWrappedLine )
										{
											// if this is the last wrapped node, then other nodes in the string may go on the same line, so
											// we don't move the cursor to the next line if this is the last wrapped node
											CurY += LastYL;
										}
							
										// otherwise, LastYL is the vertical extent of this wrapped line
										LastYL = CurrentLine.LineExtent.Y;
									}

									StringExtent.X = Max<FLOAT>(StringExtent.X, TotalLineExtent);

									if ( bLastWrappedLine )
									{
										if ( LineIndex > 0 )
										{
											// case 4:
											CurX = OrigX + CurrentLine.LineExtent.X;
										}
										else
										{
											// case 5:
											CurX = OrigX;

											// we need to ensure that the next node in the string is drawn on the next line (case 5.2)
											if ( NodeIndex < Nodes.Num() - 1 )
											{
												Nodes(NodeIndex+1)->bForceWrap = TRUE;
											}
										}
									}
								}

								delete StringNode;
								StringNode = NULL;
							}
						}
					}
					else if ( StringStyleData.TextClipMode == CLIP_Ellipsis )
					{
						if ( bOverflowsBounds )
						{
							// this is the node that will contain the pre-ellipsised version of the string
							FUIStringNode_FormattedNodeParent* EllipsisParent = new FUIStringNode_FormattedNodeParent(*TextNode);

							// clear the extent for the formatting parent
							EllipsisParent->CalculateExtent(FormatData.DefaultLineHeight, Parameters.ViewportHeight);

							// we'll replace the existing node with a FormattingNodeParent
							Nodes(NodeIndex) = EllipsisParent;


							// then we'll create two slave nodes - one containing all characters which fit into the bounding region, and another to contain the ellipsis text

							// first create the ellipsis node, so that we know how much space is available for the other node
							FUIStringNode_Text* EllipsisNode = new FUIStringNode_Text(TEXT(""));

							// Propagate the formatting values from the original node to the ellipsis node
							FUICombinedStyleData& EllipsisNodeStyleData = EllipsisNode->GetNodeStyleData();
							EllipsisNodeStyleData = TextNode->GetNodeStyleData();
							EllipsisNode->ParentNode = EllipsisParent;
					//@fixme ronp - this might not be accurate, since the scaling should be coming from EllipsisNodeStyleData.TextScale...
							EllipsisNode->Scaling = TextNode->Scaling;
							EllipsisNode->bForceWrap = FALSE;
							EllipsisNode->SetRenderText(ELLIPSIS_TEXT);
							EllipsisNode->CalculateExtent(FormatData.DefaultLineHeight, Parameters.ViewportHeight);


							// create a new text node to contain the clipped text
							FUIStringNode_Text* SlaveNode = new FUIStringNode_Text(TEXT(""));

							// when clipping, the bounding region always begins at the current X position
							Parameters.DrawX = CurX;
							// reduce the horizontal size of the bounding region by the size of the ellipsis text
							Parameters.DrawXL -= EllipsisNode->Extent.X;

							// now we clip the text so that it fits into the remaining area
							FString ClippedText;
							UUIString::ClipString(Parameters, *TextNode->RenderedText, ClippedText, static_cast<EUIAlignment>(NodeStyleData.TextAlignment[UIORIENT_Horizontal]));

							// Propagate the formatting values from the original node
							FUICombinedStyleData& SlaveNodeStyleData = SlaveNode->GetNodeStyleData();
							SlaveNodeStyleData = TextNode->GetNodeStyleData();
							SlaveNode->ParentNode = EllipsisParent;
					//@fixme ronp - this might not be accurate, since the scaling should be coming from SlaveNodeStyleData.TextScale...
							SlaveNode->Scaling = TextNode->Scaling;
							SlaveNode->bForceWrap = FALSE;

							// now set the slave node's render text to the clipped value
							SlaveNode->SetRenderText(*ClippedText);

							// there is no need to call CalculateExtent to determine the clipped string's extent, as ClipString() should set the length
							// and height of the clipped string in the Parameters that were passed to it
							SlaveNode->Extent.X = Parameters.DrawXL;
							SlaveNode->Extent.Y = Parameters.DrawYL;

							// now insert the new slave nodes into the list
							Nodes.InsertItem(SlaveNode, ++NodeIndex);
							Nodes.InsertItem(EllipsisNode, ++NodeIndex);

							// can't imagine that the ellipsis characters would ever be taller than any of the characters in the original string, but no sense
							// making assumptions when this calculation is pretty much free
							const FLOAT FormattedYL = Max(SlaveNode->Extent.Y, EllipsisNode->Extent.Y);

							// advance the cursor position past the end of these nodes
							CurX += SlaveNode->Extent.X + EllipsisNode->Extent.X;

							// set the height of the last line
							LastYL = Max(FormattedYL, LastYL);

							// cleanup the node we replaced
							delete StringNode;
							StringNode = NULL;
							TextNode = NULL;

							// no more room so stop processing nodes
							break;
						}
						else
						{
							CurX += StringNode->Extent.X * ForcedScaleX;
							LastYL = Max(StringNode->Extent.Y * ForcedScaleY, LastYL);
						}
					}
					else if ( bOverflowsBounds && StringStyleData.TextClipMode == CLIP_Normal )
					{
						// this is the node that will contain the pre-clipped version of the string
						FUIStringNode_FormattedNodeParent* ClipParent = new FUIStringNode_FormattedNodeParent(*TextNode);

						// clear the extent for the formatting parent
						ClipParent->CalculateExtent(FormatData.DefaultLineHeight, Parameters.ViewportHeight);

						// we'll replace the existing node with a FormattingNodeParent
						Nodes(NodeIndex) = ClipParent;

						// only render the characters that fit into the bounding box
						FString ClippedText;
						// for clipping, the bounding region always begins at the current X position
						Parameters.DrawX = CurX;
						UUIString::ClipString(Parameters, *TextNode->RenderedText, ClippedText, bForceLeftClipAlignment ? UIALIGN_Left : static_cast<EUIAlignment>(NodeStyleData.TextClipAlignment));

						// now we'll create a slave node to contain the clipped version of the string
						FUIStringNode_Text* ClippedNode = new FUIStringNode_Text(TEXT(""));

						// Propagate the formatting values from the original node to the ellipsis node
						FUICombinedStyleData& ClippedNodeStyleData = ClippedNode->GetNodeStyleData();
						ClippedNodeStyleData = TextNode->GetNodeStyleData();

						// and initialize the clipped slave node
						ClippedNode->ParentNode = ClipParent;
						ClippedNode->Scaling = TextNode->Scaling;
						ClippedNode->bForceWrap = FALSE;
						ClippedNode->SetRenderText(*ClippedText);

						// the string node's extent has changed - there is no need to recall CalculateExtent, as ClipString() should set the length
						// and height of the clipped string in the Parameters that were passed to it
						ClippedNode->Extent.X = Parameters.DrawXL;
						ClippedNode->Extent.Y = Parameters.DrawYL;

						// now insert the new slave node into the list
						Nodes.InsertItem(ClippedNode, ++NodeIndex);

						// advance the cursor position past the end of this node
						CurX += ClippedNode->Extent.X;

						// set the height of the last line
						LastYL = Max(ClippedNode->Extent.Y, LastYL);

						// cleanup the node we replaced
						delete StringNode;
						StringNode = NULL;
						TextNode = NULL;

						// no more room so stop processing nodes
						break;
					}
					else
					{
						CurX += (StringNode->Extent.X * ForcedScaleX);
						LastYL = Max( (StringNode->Extent.Y * ForcedScaleY), LastYL);
					}
				}
				else	// bNeedsWrap
				{
					CurX += (StringNode->Extent.X * ForcedScaleX);
					LastYL = Max( (StringNode->Extent.Y * ForcedScaleY), LastYL);
				}

				if ( StringNode )
				{
					if ( AutoScaleMode == UIAUTOSCALE_Normal || AutoScaleMode == UIAUTOSCALE_Justified )
					{
						StringNode->Scaling.X = ForcedScaleX;
						StringNode->Scaling.Y = ForcedScaleY;
						StringNode->Extent.X *= ForcedScaleX;
						StringNode->Extent.Y *= ForcedScaleY;
					}
					else
					{
						//@todo - huh?  these should always be pointing to the same thing...
						StringNode->Scaling.X = TextNode->Scaling.X;
						StringNode->Scaling.Y = TextNode->Scaling.Y;
					}
				}
			}
			else
			{
				StringNode->Extent.X *= ForcedScaleX;
				StringNode->Extent.Y *= ForcedScaleY;

				// image node
				if ( CurX + StringNode->Extent.X > ClipX + KINDA_SMALL_NUMBER )
				{
					// this image cannot fit inside the bounding region...
					if ( StringStyleData.TextClipMode == CLIP_Wrap  )
					{
						//... and this string is configured to wrap, wrap the image to the next line
						CurX = OrigX + StringNode->Extent.X;
						CurY += StringNode->Extent.Y;
						LastYL = StringNode->Extent.Y;
					}
					else if ( StringStyleData.TextClipMode == CLIP_Ellipsis )
					{
						//@todo - replace the node with a node containing only the ellipsis
						CurX += StringNode->Extent.X;
						LastYL = Max(StringNode->Extent.Y, LastYL);

						// no more room so stop processing nodes
						break;
					}
					else if ( StringStyleData.TextClipMode == CLIP_Normal )
					{
						// stop here
						CurX += StringNode->Extent.X;

						// no more room so stop processing nodes
						break;
					}
					else
					{
						CurX += StringNode->Extent.X;
						LastYL = Max(StringNode->Extent.Y, LastYL);
					}
				}
				else
				{
					CurX += StringNode->Extent.X;
					LastYL = Max(StringNode->Extent.Y, LastYL);
				}
			}
		}
	}

	// if we didn't process all of the nodes, it means we ran out of room
	// clear the extents for the remaining nodes so that they aren't rendered
	while ( ++NodeIndex < Nodes.Num() )
	{
		FUIStringNode* StringNode = Nodes(NodeIndex);
		StringNode->Extent.X = StringNode->Extent.Y = 0.f;
	}


	// set the output values - 
	Parameters.DrawX = CurX;			// DrawX needs to be the X pos (in pixels) of the end of the last line
	Parameters.DrawY = CurY + LastYL;	// DrawY needs to be the Y pos of the bottom of the last line
										// LastYL needs to be the height of the last line

	StringExtent.X = Max<FLOAT>(StringExtent.X, CurX - OrigX);
	StringExtent.Y = Parameters.DrawY - OrigY;
}

inline DWORD GetTypeHash(const TArray<FUIStringNode*>* NodeArray )
{
	return PointerHash(NodeArray);
}

/**
 * Render this UIString using the parameters specified.
 *
 * @param	Canvas		the canvas to use for rendering this string
 * @param	Parameters	the bounds for the region that this string can render to.
 */
void UUIString::Render_String( FCanvas* Canvas, const FRenderParameters& Parameters )
{
	checkSlow(StringStyleData.IsInitialized());
	checkSlow(StringStyleData.DrawFont);

	if ( Nodes.Num() == 0 )
	{
		return;
	}

	// tracks the extent for each line of the string....only wrapped strings should have more than one line
	TMap< TArray<FUIStringNode*>*, FVector2D > LineExtents;

	// each element of this array corresponds to a single line in the string, and each line may contain multiple nodes
	// only wrapped strings should have more than one line.
	TIndirectArray< TArray<FUIStringNode*> > NodeLines;

	// create the container for the nodes in the first line
	TArray<FUIStringNode*>* CurrentLine = new TArray<FUIStringNode*>;
	NodeLines.AddRawItem(CurrentLine);

	// as each node is processed, DrawX is advanced forward by the node's horz extent.  DrawX always represents the location
	// where the current node will be drawn.  DrawY is increased when we move to a new line (only for wrapped strings).  The
	// height of the tallest node in the previous line is added to DrawY when advance to a new line.
	FLOAT DrawX = Parameters.DrawX;
	FLOAT DrawY = Parameters.DrawY;

	const UBOOL bInvertedDraw = ( StringStyleData.TextClipMode == CLIP_Wrap && StringStyleData.TextClipAlignment == UIALIGN_Right );

	//@fixme ronp invertwrap - this inverted wrap mode goes away once support for multi-line editboxes is added (must be able to
	// start rendering at any line) - we'll always be rendering from the top down but the first node that is rendered
	// would be the line at (LastLine - VisibleLines) in the array of wrappped lines.
	INT StartIndex = 0, EndIndex = Nodes.Num(), Increment = 1;
	if ( bInvertedDraw )
	{
		//@fixme ronp invertwrap - here is where we might want to calculate the index of the first visible line
		DrawY = Parameters.DrawY + Parameters.DrawYL;
		StartIndex = Nodes.Num() - 1;
		EndIndex = Increment = -1;
	}

	FVector2D CurrentLineExtent(0,0);

	// In order to support wrapping arbitrarily sized nodes which can potentially each have their own alignment within their bounding region,
	// we need to first calculate the size of each line (and the total size that all will take to render) to determine exactly how to sub-divide
	// the overall bounding region into even chunks....each node will then align itself within its "chunk" as best it can.

	// iterate through each of this UIString's nodes, building the matrix of node & line extents
	for ( INT NodeIndex = StartIndex; NodeIndex != EndIndex; NodeIndex += Increment )
	{
		FUIStringNode* StringNode = Nodes(NodeIndex);
		if ( !bInvertedDraw )
		{
			if ( NodeIndex > 0 && StringStyleData.TextClipMode == CLIP_Wrap )
			{
				// if we're allowed to wrap, check whether the next word would overflow our horizontal bounds...
				if ( StringNode->bForceWrap /*|| (DrawX + StringNode->Extent.X - Parameters.DrawX > Parameters.DrawXL + KINDA_SMALL_NUMBER)*/ )
				{
					// this node won't fit on the current line....attempt to move it to the next line
					DrawY += CurrentLineExtent.Y;

					// check whether there is room to draw this node on the next line
					if ( DrawY + StringNode->Extent.Y - Parameters.DrawY < Parameters.DrawYL + KINDA_SMALL_NUMBER )
					{
						CurrentLine = new TArray<FUIStringNode*>;
						NodeLines.AddRawItem(CurrentLine);

						// we've moved to the next line, so reset the line's DrawX to the start of the bounding region
						DrawX = Parameters.DrawX;
						CurrentLineExtent.X = CurrentLineExtent.Y = 0.f;
					}
					else
					{
						// we've run out of vertical space - stop adding nodes
						break;
					}
				}
			}
		}
		else
		{
			//@fixme ronp invertwrap / joew - this block of code is assuming that every node represents a single line of text, which is not the case
			//@{
			// this node won't fit on the current line....attempt to move it to the next line
			DrawY -= StringNode->Extent.Y;

			// check whether there is room to draw this node on the next line
			if ( DrawY > Parameters.DrawY - KINDA_SMALL_NUMBER )
			{
				CurrentLine = new TArray<FUIStringNode*>;
				NodeLines.AddRawItem(CurrentLine);

				// we've moved to the next line, so reset the line's DrawX to the start of the bounding region
				DrawX = Parameters.DrawX;
				CurrentLineExtent.X = CurrentLineExtent.Y = 0.f;
			}
			else
			{
				// we've run out of vertical space - stop adding nodes
				break;
			}
			//@}
		}
			
		if ( StringNode->Extent.X > 0 && StringNode->Extent.Y > 0 )
		{
			// add this node to the current line's array of nodes.
			CurrentLine->AddItem(StringNode);

			// advance the current draw position past this node
			DrawX += StringNode->Extent.X;

			// calculate the total extent for the line
			CurrentLineExtent.X = Max<FLOAT>(CurrentLineExtent.X, DrawX - Parameters.DrawX);
			CurrentLineExtent.Y = Max<FLOAT>(CurrentLineExtent.Y, StringNode->Extent.Y);

			// store the line's extent in the map for lookup later when we are rendering the nodes
			LineExtents.Set(CurrentLine, CurrentLineExtent);
		}
	}

	// now we can actually render the nodes
	FRenderParameters NodeParameters = Parameters;
	
	// Manage the InvWrap
	if ( bInvertedDraw )
	{
		NodeParameters.DrawY += NodeParameters.DrawYL;
	}

	for ( INT LineIndex = 0; LineIndex < NodeLines.Num(); LineIndex++ )
	{
		TArray<FUIStringNode*>& CurrentLineRef = NodeLines(LineIndex);

		FVector2D* pCurrentExtent = LineExtents.Find(&CurrentLineRef);
		if ( pCurrentExtent != NULL )
		{
			// retrieve the width/height of this line
			CurrentLineExtent = *pCurrentExtent;

			// If we are in InvertWrap mode, we adjust the DrawY before the render not after
			//@fixme ronp invertwrap / joew - there is no reason for this part of the code to be different when we're rendering
			// the last of the visible lines instead of the first ones.
			if ( bInvertedDraw )
			{
				NodeParameters.DrawY -= CurrentLineExtent.Y;
			}

			// determine where to start drawing the first node in the line
			const BYTE HorizontalAlign = Parameters.TextAlignment[UIORIENT_Horizontal];
			check(HorizontalAlign<UIALIGN_MAX);

			switch ( HorizontalAlign )
			{
			case UIALIGN_Left:
			case UIALIGN_Default:
				NodeParameters.DrawX = Parameters.DrawX;
				break;

			case UIALIGN_Center:
				NodeParameters.DrawX = Parameters.DrawX + (Parameters.DrawXL * 0.5 - CurrentLineExtent.X * 0.5f);
				break;

			case UIALIGN_Right:
				NodeParameters.DrawX = Parameters.DrawX + (Parameters.DrawXL - CurrentLineExtent.X);
				break;
			}

			// the vertical space is divided by the number of lines in the string..unless there is
			// more space available than is required, or one of the lines is taller than the rest.
//			NodeParameters.DrawYL = Max(Parameters.DrawYL / NodeLines.Num(), CurrentLineExtent.Y);
			if ( CurrentLineExtent.Y == 0 )
			{
				NodeParameters.DrawYL = Parameters.DrawYL / NodeLines.Num();
			}
			else
			{
				NodeParameters.DrawYL = CurrentLineExtent.Y;
			}

			for ( INT NodeIndex = 0; NodeIndex < CurrentLineRef.Num(); NodeIndex++ )
			{
				FUIStringNode* StringNode = CurrentLineRef(NodeIndex);
				NodeParameters.DrawXL = StringNode->Extent.X;
				NodeParameters.ImageExtent = StringNode->Extent;

				// Apply Scaling
				NodeParameters.Scaling = StringNode->Scaling;

				if(StringNode->IsTextNode())
				{
					FUIStringNode_Text *TextNode = (FUIStringNode_Text*)StringNode;
					const FUICombinedStyleData &TextStyles = TextNode->GetNodeStyleData();

					NodeParameters.SpacingAdjust = TextStyles.TextSpacingAdjust;
					NodeParameters.TextAlignment[0] = TextStyles.TextAlignment[0];
					NodeParameters.TextAlignment[1] = TextStyles.TextAlignment[1];
				}
				else if ( StringNode->IsImageNode() )
				{
					FUIStringNode_Image* ImageNode = static_cast<FUIStringNode_Image*>(StringNode);
					NodeParameters.ImageExtent = ImageNode->ForcedExtent;
				}

				//@todo - hmmm....should we do not call Render_Node if X/Y is less than zero?
				StringNode->Render_Node(Canvas, NodeParameters);

				// advance the position of the pen past this node
				NodeParameters.DrawX += NodeParameters.DrawXL;
			}


			if ( !bInvertedDraw )
			{
				// we've finished with that line...advance the pen to the next line
				//@todo - hmm, seems like I should use NodeParameters.DrawYL rather than CurrentLineExtent.Y, in case the
				// lines were spread out [vertically] for some reason
				NodeParameters.DrawY += CurrentLineExtent.Y;
			}
		}
	}
}

/**
 * Parses a string containing optional markup (such as tokens and inline images) and stores the result in Nodes.
 *
 * @param	InputString		A string containing optional markup.
 *
 * @return	TRUE if the string was successfully parsed into the Nodes array.
 */
UBOOL UUIString::SetValue( const FString& InputString, UBOOL bIgnoreMarkup )
{
	UBOOL bResult = FALSE;

	//@fixme ronp - clearing the nodes breaks UIStrings that are persistent, since the nodes are setup in the editor
	ClearNodes();
	if ( InputString.Len() > 0 )
	{
		bResult = ParseString(InputString,bIgnoreMarkup,Nodes);
	}
	else
	{
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Returns the complete text value contained by this UIString, in either the processed or unprocessed state.
 *
 * @param	bReturnProcessedText	Determines whether the processed or raw version of the value string is returned.
 *									The raw value will contain any markup; the processed string will be text only.
 *									Any image tokens are converted to their text counterpart.
 *
 * @return	the complete text value contained by this UIString, in either the processed or unprocessed state.
 */
FString UUIString::GetValue( UBOOL bReturnProcessedText/*=TRUE*/ ) const
{
	FString Result;
	for ( INT i = 0; i < Nodes.Num(); i++ )
	{
		FUIStringNode* StringNode = Nodes(i);
		if ( StringNode->IsNestParent() )
		{
			if ( bReturnProcessedText )
			{
				continue;
			}
			else
			{
				// add the source text to the result
				const TCHAR* NodeValue = StringNode->GetValue(FALSE);
				check(NodeValue);

				Result += NodeValue;

				// then advance past all nodes that have this node as the parent, since we only want to return the markup value
				while ( ++i < Nodes.Num() )
				{
					if ( !Nodes(i)->IsSlaveNode(StringNode) )
					{
						// if the next node isn't a slave, we'll need to process it
						i--;
						break;
					}
				}
			}
		}
		else if ( StringNode->IsFormattingParent() )
		{
			// this node contains a string that was wrapped into multiple lines (or otherwise formatted)

			// add the pre-wrapped version of the text to the result
			const TCHAR* NodeValue = StringNode->GetValue(bReturnProcessedText);
			if ( NodeValue != NULL )
			{
				Result += NodeValue;
			}

			// then advance past all nodes which were created to contain the text for the wrapped lines
			while ( ++i < Nodes.Num() && Nodes(i)->IsSlaveNode(StringNode) )
			{
				// nothing
			}
		}
		else
		{
			const TCHAR* NodeValue = StringNode->GetValue(bReturnProcessedText);
			if ( NodeValue != NULL )
			{
				Result += NodeValue;
			}
		}
	}

	return Result;
}

/**
 * @return	TRUE if this string's value contains markup text
 */
UBOOL UUIString::ContainsMarkup() const
{
	UBOOL bResult = FALSE;
	for ( INT NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++ )
	{
		if ( FUIStringParser::StringContainsMarkup(Nodes(NodeIndex)->SourceText) )
		{
			bResult = TRUE;
			break;
		}
	}
	return bResult;
}

/**
 * Deletes all nodes allocated by this UIString and empties the Nodes array
 */
void UUIString::ClearNodes()
{
	for ( INT i = 0; i < Nodes.Num(); i++ )
	{
		FUIStringNode* StringNode = Nodes(i);
		delete StringNode;
	}

	Nodes.Empty();
}

/**
 * Strips all escape characters from the text specified.
 *
 * @param	InputText	the text to strip escape sequences from
 *
 * @return	an FString containing InputText with all escape characters removed
 */
static FString StripEscapeCharacters( const TCHAR* InputText )
{
	UBOOL bEscaped = FALSE;

	FString Result;
	if ( InputText )
	{
		while ( *InputText )
		{
			if ( bEscaped )
			{
				bEscaped = FALSE;
				Result += *InputText;
			}
			else if ( !bEscaped && *InputText == TCHAR('\\') )
			{
				bEscaped = TRUE;
			}
			else
			{
				Result += *InputText;
			}

			InputText++;
		}

		// if the last character was the escape character, add it to the string
		if ( bEscaped )
		{
			Result += TCHAR('\\');
		}
	}

	return Result;
}

/**
 * Converts the raw source text containing optional markup (such as tokens and inline images)
 * into renderable data structures.
 *
 * @param	InputString			A string containing optional markup.
 * @param	bSystemMarkupOnly	if TRUE, only system generated markup will be processed (such as markup for rendering carets, etc.)
 * @param	out_Nodes			A collection of UITextNodes which will contain the parsed nodes.
 * @param	StringNodeModifier	the style data to use as the starting point for string node modifications.  If not specified, uses the
 *								string's StringStyleData as the starting point.  Generally only specified when recursively calling
 *								ParseString.
 *
 * @return	TRUE if InputString was successfully parsed into out_Nodes
 */
UBOOL UUIString::ParseString(const FString& InputString, UBOOL bSystemMarkupOnly, TArray<FUIStringNode*>& out_Nodes, FUIStringNodeModifier* StringNodeModifier/*=NULL*/ ) const
{
	SCOPE_CYCLE_COUNTER(STAT_UIParseString);
	UBOOL bResult = FALSE;

	FUIStringParser Parser;

	// break up the string into normal text and markup chunks
	Parser.ScanString(InputString, bSystemMarkupOnly);

	FUIStringNodeModifier DefaultStyleModifier(StringStyleData, GetCurrentMenuState());

	/** this tracks the modifications that have been applied by inline markup */
	FUIStringNodeModifier& StyleModifier = StringNodeModifier != NULL ? *StringNodeModifier : DefaultStyleModifier;

	// grab the list of chunks parsed from the input string
	const TIndirectArray<FTextChunk>* Chunks = Parser.GetTextChunks();
	for ( INT ChunkIndex = 0; ChunkIndex < Chunks->Num(); ChunkIndex++ )
	{
		const FTextChunk* NextChunk = &((*Chunks)(ChunkIndex));
		if ( !NextChunk->IsMarkup() )
		{
			FUIProviderFieldValue ResolvedValue(EC_EventParm);
			ResolvedValue.PropertyType = DATATYPE_Property;
			ResolvedValue.StringValue = NextChunk->ChunkValue;

			// this chunk represents normal text - create a simple text node
			FUIStringNode* StringNode = UUIDataProvider::CreateStringNode(NextChunk->ChunkValue, ResolvedValue);
			if ( StringNode != NULL )
			{
				// initialize the string node's style data using the inline style modifier
				StringNode->InitializeStyle(StyleModifier.GetCustomStyleData());

				// add this node to the list
				out_Nodes.AddItem(StringNode);
				bResult = TRUE;
			}
			else
			{
				// nothing...
			}
		}
		else
		{
			// this chunk is a data store reference - call into the data store system to resolve the markup into a UIStringNode
			FUIStringNode* StringNode = NULL;
			FString DataStoreTag, DataStoreValue;
			if ( NextChunk->GetDataStoreMarkup(DataStoreTag, DataStoreValue) )
			{
				UUIDataStore* DataStore = ResolveDataStore(*DataStoreTag);
				if ( DataStore != NULL )
				{
					FUIProviderFieldValue ResolvedValue(EC_EventParm);
					if ( DataStore->GetDataStoreValue(DataStoreValue, ResolvedValue) )
					{
						StringNode = UUIDataProvider::CreateStringNode(NextChunk->ChunkValue, ResolvedValue);
						checkSlow(StringNode);

						StringNode->NodeDataStore = DataStore;

						// if this is a text node, the resolved value of the data binding might contain markup
						if ( StringNode->IsTextNode() )
						{
							// Parse the resolved text for any embedded markup
							FUIStringNode_Text* TextNode = (FUIStringNode_Text*)StringNode;
							if ( FUIStringParser::StringContainsMarkup(TextNode->RenderedText, bSystemMarkupOnly) )
							{
								TArray<FUIStringNode*> NestedNodes;
								if ( ParseString(TextNode->RenderedText, bSystemMarkupOnly, NestedNodes, &StyleModifier) )
								{
									// create a parent node to contain the original source text and resolved data store
									FUIStringNode_NestedMarkupParent* ParentNode = new FUIStringNode_NestedMarkupParent(*NextChunk->ChunkValue);
									ParentNode->NodeDataStore = DataStore;

									out_Nodes.AddItem(ParentNode);
									for ( INT NestIndex = 0; NestIndex < NestedNodes.Num(); NestIndex++ )
									{
										FUIStringNode* NestedNode = NestedNodes(NestIndex);

										if ( NestedNode->ParentNode == NULL )
										{
											NestedNode->ParentNode = ParentNode;
										}

										out_Nodes.AddItem(NestedNode);
									}

									// don't add the node we resolved to the list of nodes, since the recursive call to ParseString
									// already added nodes for this text to the list
									delete StringNode;
									StringNode = NULL;

									// string node is NULL, but in this case, it doesn't mean we should stop processing nodes, so continue
									bResult = TRUE;
									continue;
								}
							}
						}
					}
					else if ( DataStore->ParseStringModifier(DataStoreValue, StyleModifier) )
					{
						// this markup only modifies the style data to use when parsing subsequent nodes;
						// so the node that we create shouldn't have any render text
						StringNode = new FUIStringNode_Text(*NextChunk->ChunkValue);
						StringNode->NodeDataStore = DataStore;
					}
					else
					{
						//@fixme - for debugging purposes only, display the raw markup text so we know that it wasn't resolved properly
						ResolvedValue.PropertyType = DATATYPE_Property;
						ResolvedValue.StringValue = NextChunk->ChunkValue;
						StringNode = UUIDataProvider::CreateStringNode(NextChunk->ChunkValue, ResolvedValue);
						StringNode->NodeDataStore = DataStore;
#if FINAL_RELEASE || SHIPPING_PC_GAME
						// in FINAL_RELEASE, don't display the markup text
						if ( StringNode->IsTextNode() )
						{
							((FUIStringNode_Text*)StringNode)->SetRenderText(TEXT(""));
						}
#endif
					}
				}
			}
			
			if ( StringNode != NULL )
			{
				// initialize the string node's style data using the inline style modifier
				StringNode->InitializeStyle(StyleModifier.GetCustomStyleData());

				// add this node to the list
				out_Nodes.AddItem(StringNode);
				bResult = TRUE;
			}
			else
			{
				// invalid data store markup...do nothing
			}
		}
	}

	return bResult;
}

/**
 * Retrieves a list of all data stores resolved by this UIString.
 *
 * @param	StringDataStores	receives the list of data stores that have been resolved by this string.  Appends all
 *								entries to the end of the array and does not clear the array first.
 */
void UUIString::GetResolvedDataStores( TArray<UUIDataStore*>& StringDataStores )
{
	for ( INT NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++ )
	{
		FUIStringNode* Node = Nodes(NodeIndex);
		if ( Node->NodeDataStore != NULL )
		{
			StringDataStores.AddUniqueItem(Node->NodeDataStore);
		}
	}
}
	
/**
 * Gets the size of the viewport.
 *
 * @param	out_ViewportSize	receives the viewport size.
 *
 * @return	TRUE if the viewport size was retrieved successfully.
 */
UBOOL UUIString::GetViewportSize( FVector2D& out_ViewportSize ) const
{
	UBOOL bResult = FALSE;

	UUIScreenObject* Owner = GetOuterUUIScreenObject();
	if ( !Owner->HasAnyFlags(RF_ClassDefaultObject) )
	{
		bResult = Owner->GetViewportSize(out_ViewportSize);
	}

	return bResult;
}

/* === UObject interface === */
void UUIString::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects(ObjectArray);

	for ( INT i = 0; i < Nodes.Num(); i++ )
	{
		Nodes(i)->AddReferencedObjects(this, ObjectArray);
	}
}

/**
 * I/O function
 */
void UUIString::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);

	if ( !Ar.IsPersistent() )
	{
		for ( INT i = 0; i < Nodes.Num(); i++ )
		{
			Ar << *Nodes(i);
		}
	}
}

void UUIString::FinishDestroy()
{
	ClearNodes();

	Super::FinishDestroy();
}

/**
 * Determines whether this object is contained within a UIPrefab.
 *
 * @param	OwnerPrefab		if specified, receives a pointer to the owning prefab.
 *
 * @return	TRUE if this object is contained within a UIPrefab; FALSE if this object IS a UIPrefab or is not
 *			contained within a UIPrefab.
 */
UBOOL UUIString::IsAPrefabArchetype( UObject** OwnerPrefab/*=NULL*/ ) const
{
	UBOOL bResult = FALSE;
	
	UUIScreenObject* WidgetOwner = GetOuterUUIScreenObject();
	if ( !WidgetOwner->HasAnyFlags(RF_ClassDefaultObject) )
	{
		bResult = WidgetOwner->IsAPrefabArchetype(OwnerPrefab);
	}
	else
	{
		bResult = Super::IsAPrefabArchetype(OwnerPrefab);
	}
	return bResult;
}

/**
 * @return	TRUE if the object is contained within a UIPrefabInstance.
 */
UBOOL UUIString::IsInPrefabInstance() const
{
	UBOOL bResult = FALSE;

	UUIScreenObject* WidgetOwner = GetOuterUUIScreenObject();
	if ( !WidgetOwner->HasAnyFlags(RF_ClassDefaultObject) )
	{
		bResult = WidgetOwner->IsInPrefabInstance();
	}
	else
	{
		bResult = Super::IsInPrefabInstance();
	}
	return bResult;
}

/* ==========================================================================================================
	UUIEditboxString
========================================================================================================== */
/**
 * Parses a string containing optional markup (such as tokens and inline images) and stores the result in Nodes.
 *
 * This version replaces the RenderText in all nodes with asterisks if the editbox's bPasswordMode is enabled.
 *
 * @param	InputString		A string containing optional markup.
 *
 * @return	TRUE if the string was successfully parsed into the Nodes array.
 */
UBOOL UUIEditboxString::SetValue( const FString& InputString, UBOOL bIgnoreMarkup )
{
	UBOOL bResult = Super::SetValue(InputString, bIgnoreMarkup);

	if ( bResult )
	{
		UUIEditBox* OwnerEditbox = GetOuterUUIEditBox();
		if ( OwnerEditbox->StringRenderComponent != NULL )
		{
			// update the string component's UserText member; this must be done prior to
			// replacing with asterisks (if applicable)
			OwnerEditbox->StringRenderComponent->UserText = Super::GetValue(TRUE);
		}

		if ( InputString.Len() > 0 && OwnerEditbox->bPasswordMode )
		{
			// replace the RenderText in all nodes with *
			for ( INT NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++ )
			{
				if ( Nodes(NodeIndex)->IsTextNode() )
				{
					FUIStringNode_Text* TextNode = static_cast<FUIStringNode_Text*>(Nodes(NodeIndex));
					for ( INT i = 0; i < TextNode->RenderedText.Len(); i++ )
					{
						TextNode->RenderedText[i] = TEXT('*');
					}
				}
			}
		}
	}

	return bResult;
}

/**
 * Wrapper for calculating the extent of the node associated with FormatData.
 *
 * This version clears the RenderedText for the nodes that contain the characters that come before the FirstCharacterPosition
 * in the editbox.
 *
 * It also determines the screen location for rendering the caret immediately following the character located at StringCaret.CaretPosition
 * in the editbox's string component's UserText string.
 */
UBOOL UUIEditboxString::AdjustNodeExtents( FNodeFormattingData& FormatData )
{
	// the width of the characters preceding the character at FirstCharacterPosition 
	FLOAT RequiredLeadPixels = 0.f;

	// the width of the characters preceding the character at StringCaret.CaretPosition
	FLOAT RequiredCaretPixels = 0.f;
	FLOAT LeadingPixels = 0.f;
	FLOAT CaretPixels = 0.f;

	// DrawYL is 0 if there are no existing characters in the string
	if ( FormatData.Parameters.DrawYL == 0 )
	{
		FormatData.Parameters.DrawYL = FormatData.DefaultLineHeight * FormatData.AutoScale.Y;
	}

	// we'' need to restore the FormatData.Parameters.Scaling to it's original value after we do our calculations
	// because the UIString::ApplyFormatting is going to apply the scaling itself
	const FVector2D OriginalScaling = FormatData.Parameters.Scaling;

	UUIEditBox* OwnerEditbox = GetOuterUUIEditBox();
	if ( OwnerEditbox->StringRenderComponent != NULL )
	{
		//@note: we used to subtract out the caret width here, but now we do this in UpdateFirstVisibleCharacter...
		checkSlow(StringStyleData.IsInitialized());
		checkSlow(StringStyleData.DrawFont);

		// ensure that the render parameters have a valid font
		FormatData.Parameters.DrawFont = StringStyleData.DrawFont;

		FormatData.Parameters.Scaling *= FormatData.AutoScale;

		// calculate the number of pixels that must be skipped to get to the first visible character
		RequiredLeadPixels = OwnerEditbox->StringRenderComponent->CalculateFirstVisibleOffset(FormatData.Parameters);

		// calculate the number of pixels that must be skipped in order to get to the character immediately preceding the caret position
		RequiredCaretPixels = OwnerEditbox->StringRenderComponent->CalculateCaretOffset(FormatData.Parameters);

		// so now we're going to iterate over all nodes, looking for the node that contains the character located at FirstCharacterPosition.
		INT NodeIndex;
		for ( NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++ )
		{
			FUIStringNode* StringNode = Nodes(NodeIndex);

			// store the node's calculated extent, as this will be set to zero if this node is clipped, but later on 
			// we'll need to know how wide this node was *supposed* to be
			FLOAT NodeWidth = StringNode->Extent.X;

			if ( LeadingPixels >= RequiredLeadPixels )
			{
				// the rest of the nodes can be rendered in full - stop here
				break;
			}

			// if the number of pixels we've counted so far + the width of this next node is greater than the width of the characters
			// prior to the first visible characters, then we've found the node containing the first visible character
			if ( LeadingPixels + NodeWidth > RequiredLeadPixels )
			{
				if ( StringNode->IsTextNode() && !StringNode->IsFormattingParent() )
				{
					FUIStringNode_Text* TextNode = (FUIStringNode_Text*)StringNode;

					// this node contains the editbox's first visible character; clip this node's text to the characters that can
					// fit within the bounding region - this removes all characters prior to the first visible character from this
					// node's RenderedText
					FRenderParameters ClipParameters = FormatData.Parameters;
					ClipParameters.DrawX = 0;
//					ClipParameters.DrawXL = Min(FormatData.Parameters.DrawXL, LeadingPixels + NodeWidth) - RequiredLeadPixels;

					// FormatData.Parameters.DrawXL represents the width of the editbox (minus any padding, etc.) - think we always want to
					// use that value for clipping this string (hmm, I believe the original intent was for supporting editboxes with strings
					// containing multiple nodes...might need to fix that case eventually.
					ClipParameters.DrawXL = FormatData.Parameters.DrawXL;

					FUICombinedStyleData& NodeStyleData = TextNode->GetNodeStyleData();
					FString NodeRenderText;

					// always use UIALIGN_Left so that the clipping code strips everything from the first character to the right
					UUIString::ClipString(ClipParameters, *OwnerEditbox->StringRenderComponent->GetUserTextRef().Mid(OwnerEditbox->StringRenderComponent->FirstCharacterPosition),
						NodeRenderText, UIALIGN_Left);

					//@todo ronp - support for editboxes which contain more than one node..
// 					UUIString::ClipString(ClipParameters, *TextNode->RenderedText.Mid(OwnerEditbox->StringRenderComponent->FirstCharacterPosition),
// 						NodeRenderText, UIALIGN_Left);

					TextNode->SetRenderText(*NodeRenderText);
					NodeWidth = TextNode->Extent.X = ClipParameters.DrawXL;
					break;
				}
				else
				{
					// this is an image node - for now, just skip rendering image nodes by setting their extent to 0
					//@todo - later, we can render partial image nodes by adjusting the node's TextureCoordinates
					StringNode->Extent.X = 0;
				}
			}
			else
			{
				// the entire node occurs before the first visible character
				StringNode->Extent.X = 0;
				if ( StringNode->IsTextNode() )
				{
					// formatting parent nodes should always have a zero extent, so we should never end up in this block if the node is a wrap parent
					check(!StringNode->IsFormattingParent());
					((FUIStringNode_Text*)StringNode)->SetRenderText(TEXT(""));
				}
			}

			LeadingPixels += NodeWidth;
		}

		for ( ; NodeIndex < Nodes.Num(); NodeIndex++ )
		{
			FUIStringNode* StringNode = Nodes(NodeIndex);
			FLOAT NodeWidth = StringNode->Extent.X;

			if ( LeadingPixels >= RequiredCaretPixels )
			{
				// this caret is at the same position as the first character in the string?
				break;
			}

			// if the number of pixels we've counted so far + the width of this next node is greater than the width of the characters
			// prior to the caret character, then we've found the node containing the character at the caret position
			// (compare using >= since the caret is rendered to the right of the character at caret position)
			if ( LeadingPixels + NodeWidth >= RequiredCaretPixels )
			{
				if ( StringNode->IsTextNode() )
				{
					// formatting parent nodes should always have a zero extent, so we should never end up in this block if the node is a wrap parent
					check(!StringNode->IsFormattingParent());

					// this node contains the character that will be just before the caret
					FUIStringNode_Text* TextNode = (FUIStringNode_Text*)StringNode;
					if( TextNode != NULL )
					{
						CaretPixels = RequiredCaretPixels - LeadingPixels;
						break;
					}
				}
			}

			LeadingPixels += NodeWidth;
		}

		OwnerEditbox->StringRenderComponent->CaretOffset = CaretPixels;
		if ( OwnerEditbox->StringRenderComponent->CaretNode != NULL )
		{
			// tell the caret node to calculate its extents for rendering
			OwnerEditbox->StringRenderComponent->CaretNode->Scaling = FormatData.Parameters.Scaling;
			OwnerEditbox->StringRenderComponent->CaretNode->CalculateExtent(FormatData.DefaultLineHeight, FormatData.Parameters.ViewportHeight);
			FormatData.LastYL = Max(FormatData.LastYL, OwnerEditbox->StringRenderComponent->CaretNode->Extent.Y);
		}
	}

	// ok now restore the scaling value.
	FormatData.Parameters.Scaling = OriginalScaling;

	// since we just clipped off the part of the string that is left of the first visible character, we
	// want the UIString to clip off everything that falls outside the bounding box to the right...so return
	// TRUE to tell the UIString to always use UIALIGN_Left for clip alignment if it needs to call ClipString
	return TRUE;
}

/* ==========================================================================================================
	UUIListString
========================================================================================================== */
void UUIListString::SetValue( FUIStringNode* ValueNode )
{
	check(ValueNode);

	// we're responsible for deleting the nodes contained by this string, so delete any existing nodes now
	ClearNodes();

	Nodes.InsertItem(ValueNode,0);

	if ( StringStyleData.IsInitialized() )
	{
		RefreshNodeStyles();
	}

	// do some more stuff
}

/* ==========================================================================================================
	FUIStringNode
========================================================================================================== */
FArchive& operator<<( FArchive& Ar, FUIStringNode& StringNode )
{
	// serialize the members of FUIStringNode
	Ar << (UObject*&)StringNode.NodeDataStore << StringNode.SourceText << StringNode.Extent << StringNode.Scaling;

	// serialize the members of the derived struct
	StringNode.Serialize(Ar);
	return Ar;
}

/**
 * Returns the value of this UIStringNode
 *
 * @param	bProcessedValue		indicates whether the raw or processed version of the value is desired
 *								The raw value will contain any markup; the processed string will be text only.
 *								Any image tokens are converted to their text counterpart.
 *
 * @return	the value of this UIStringNode, or NULL if this node has no value
 */
const TCHAR* FUIStringNode::GetValue( UBOOL bProcessedValue ) const
{
	if ( !bProcessedValue )
	{
		return SourceText.Len() ? *SourceText : NULL;
	}

	return NULL;
}

/**
 * Determines whether this node was created to contain additional text as a result of wrapping or clipping.
 *
 * @param	SearchParent	if specified, will iterate up the ParentNode chain to determine whether this string node is a direct or indirect
 *							slave of the specified parent node.
 */
UBOOL FUIStringNode::IsSlaveNode( FUIStringNode* SearchParent/*=NULL*/ ) const
{
	UBOOL bResult = FALSE;

	// we are a slave node if we have a parent node
	if ( ParentNode != NULL )
	{
		// if we aren't checking for a specific parent, return TRUE
		if ( SearchParent == NULL )
		{
			bResult = TRUE;
		}
		else
		{
			// otherwise, see if the specified parent node is in this node's ParentNode chain
			for ( FUIStringNode* CheckNode = ParentNode; CheckNode; CheckNode = CheckNode->ParentNode )
			{
				if ( CheckNode == SearchParent )
				{
					bResult = TRUE;
					break;
				}
			}
		}
	}

	return bResult;
}

/* ==========================================================================================================
	FUIStringNode_NestedMarkupParent
========================================================================================================== */
/**
 * Calculates the precise extent of this text node, and assigns the result to UIStringNode::Extent.
 * This implementation zeroes the extent of this node as NestedMarkupParent nodes should never be rendered.
 *
 * @param	DefaultLineHeight		the default height of a single line in the string...used by UIStringNode_Image to
 *									scale the image correctly.
 */
void FUIStringNode_NestedMarkupParent::CalculateExtent( FLOAT DefaultLineHeight, FLOAT ViewportHeight )
{
	Extent.X = Extent.Y = 0.f;
}


/* ==========================================================================================================
	FUIStringNode_FormattedNodeParent
========================================================================================================== */
/** constructor */
FUIStringNode_FormattedNodeParent::FUIStringNode_FormattedNodeParent( FUIStringNode_Text& SourceNode )
: FUIStringNode_Text(*SourceNode.SourceText)
{
	SetRenderText(*SourceNode.RenderedText);

	NodeStyleParameters = SourceNode.GetNodeStyleData();
	NodeDataStore = SourceNode.NodeDataStore;
	ParentNode = SourceNode.ParentNode;
	Scaling = SourceNode.Scaling;
	bForceWrap = SourceNode.bForceWrap;
}

/* ==========================================================================================================
	FUIStringNode_Text
========================================================================================================== */
/** Conversion constructor - copies the data from a wrapping parent to a text node */
FUIStringNode_Text::FUIStringNode_Text( const FUIStringNode_FormattedNodeParent& SourceNode )
: FUIStringNode(*SourceNode.SourceText), RenderedText(SourceNode.RenderedText), NodeStyleParameters(SourceNode.NodeStyleParameters)
{
	NodeDataStore = SourceNode.NodeDataStore;
	ParentNode = SourceNode.ParentNode;
	Scaling = SourceNode.Scaling;
	bForceWrap = SourceNode.bForceWrap;
}

/**
 * Callback used to allow object register its direct object references that are not already covered by
 * the token stream.
 *
 * @param Owner			the UIString that owns this node.  used to provide access to UObject::AddReferencedObject
 * @param ObjectArray	array to add referenced objects to via AddReferencedObject
 */
void FUIStringNode_Text::AddReferencedObjects( UUIString* Owner, TArray<UObject*>& Objects )
{
	checkSlow(Owner);

	Owner->AddReferencedObject(Objects, NodeStyleParameters.DrawFont);
}

/**
 * Initializes this node's style
 */
void FUIStringNode_Text::InitializeStyle( UUIStyle_Data* StringStyle )
{
	UUIStyle_Text* TextStyle = Cast<UUIStyle_Text>(StringStyle);
	if ( TextStyle == NULL )
	{
		UUIStyle_Combo* ComboStyle = Cast<UUIStyle_Combo>(StringStyle);
		if ( ComboStyle != NULL )
		{
			TextStyle = Cast<UUIStyle_Text>(ComboStyle->TextStyle.GetStyleData());
		}
	}

	if ( TextStyle != NULL )
	{
		NodeStyleParameters.InitializeStyleDataContainer(TextStyle);
	}
}

/**
 * Initializes this node's style
 */
void FUIStringNode_Text::InitializeStyle( const FUICombinedStyleData& StyleData )
{
	NodeStyleParameters = StyleData;
}

/**
 * Return the style data for this node.
 */
FUICombinedStyleData& FUIStringNode_Text::GetNodeStyleData()
{
	return NodeStyleParameters;
}

/**
 * Calculates the precise extent of this text node, and assigns the result to UIStringNode::Extent
 *
 * @param	DefaultLineHeight		the default height of a single line in the string...used by UIStringNode_Image to
 *									scale the image correctly.
 */
void FUIStringNode_Text::CalculateExtent( FLOAT DefaultLineHeight, FLOAT ViewportHeight )
{
	if ( NodeStyleParameters.DrawFont != NULL )
	{
		FRenderParameters Parameters(NodeStyleParameters.DrawFont, Scaling.X, Scaling.Y, ViewportHeight);
		Parameters.SpacingAdjust = NodeStyleParameters.TextSpacingAdjust;

		UUIString::StringSize( Parameters, bForceWrap && RenderedText.Right(1) == TEXT(" ") ? *RenderedText.LeftChop(1) : *RenderedText );

		Extent.X = Parameters.DrawXL;
		Extent.Y = Parameters.DrawYL;
	}
}

/**
 * Assigns the RenderedText to the value specified.
 */
void FUIStringNode_Text::SetRenderText( const TCHAR* NewRenderText )
{
	if ( RenderedText != NewRenderText )
	{
		RenderedText = StripEscapeCharacters(NewRenderText);
	}
}

/**
 * Returns the value of this UIStringNode
 *
 * @param	bProcessedValue		indicates whether the raw or processed version of the value is desired
 *								The raw value will contain any markup; the processed string will be text only.
 *								Any image tokens are converted to their text counterpart.
 *
 * @return	the value of this UIStringNode, or NULL if this node has no value
 */
const TCHAR* FUIStringNode_Text::GetValue( UBOOL bProcessedValue ) const
{
	if ( bProcessedValue )
	{
		return RenderedText.Len() ? *RenderedText : NULL;
	}
	else
	{
		return FUIStringNode::GetValue(bProcessedValue);
	}
}

/**
 * Renders this UIStringNode using the parameters specified.
 *
 * @param	Canvas		the canvas to use for rendering this node
 * @param	Parameters	the bounds for the region that this node should render to
 *						the Scaling value of Parameters will be applied against the Scaling
 *						value for this node.  The DrawXL/YL of the Parameters are used to
 *						determine whether this node has enough room to render itself completely.
 */
void FUIStringNode_Text::Render_Node( FCanvas* Canvas, const FRenderParameters& Parameters )
{
	if ( NodeStyleParameters.DrawFont != NULL )
	{
		UFont* DrawFont = Parameters.DrawFont == NULL
			? NodeStyleParameters.DrawFont
			: Parameters.DrawFont;

		FLOAT DrawX = Parameters.DrawX;
		FLOAT DrawY = Parameters.DrawY;

		const BYTE VerticalAlign = Parameters.TextAlignment[UIORIENT_Vertical];
		checkSlow(VerticalAlign<UIALIGN_MAX);

		switch ( VerticalAlign )
		{
		case UIALIGN_Left:
		case UIALIGN_Default:
			// do nothing
			break;

		case UIALIGN_Center:
			DrawY = Parameters.DrawY + (Parameters.DrawYL * 0.5f - Extent.Y * 0.5f);
			break;

		case UIALIGN_Right:
			DrawY = Parameters.DrawY + (Parameters.DrawYL - Extent.Y);
			break;
		}

		DrawString(Canvas, DrawX, DrawY, *RenderedText, DrawFont, NodeStyleParameters.TextColor, 
			Parameters.Scaling.X, Parameters.Scaling.Y, Parameters.SpacingAdjust.X, &Parameters.ViewportHeight );
	}
}

/**
 * Determines whether this node contains only modification markup.
 */
UBOOL FUIStringNode_Text::IsModifierNode() const
{
	// we are a modifier node if we contain source text, but no render text
	return SourceText.Len() > 0 && RenderedText.Len() == 0 && FUIStringParser::StringContainsMarkup(SourceText, FALSE);
}

/* ==========================================================================================================
	FUIStringNode_Image
========================================================================================================== */

/**
 * Callback used to allow object register its direct object references that are not already covered by
 * the token stream.
 *
 * @param Owner			the UIString that owns this node.  used to provide access to UObject::AddReferencedObject
 * @param ObjectArray	array to add referenced objects to via AddReferencedObject
 */
void FUIStringNode_Image::AddReferencedObjects( UUIString* Owner, TArray<UObject*>& Objects )
{
	checkSlow(Owner);

	Owner->AddReferencedObject(Objects, RenderedImage);
}

/**
 * Initializes this node's style
 */
void FUIStringNode_Image::InitializeStyle( UUIStyle_Data* StringStyle )
{
	if ( RenderedImage != NULL )
	{
		UUIStyle_Image* ImageStyle = Cast<UUIStyle_Image>(StringStyle);
		if ( ImageStyle == NULL )
		{
			UUIStyle_Combo* ComboStyle = Cast<UUIStyle_Combo>(StringStyle);
			if ( ComboStyle != NULL )
			{
				ImageStyle = Cast<UUIStyle_Image>(ComboStyle->ImageStyle.GetStyleData());
			}
		}

		RenderedImage->SetImageStyle(ImageStyle);
	}
}

/**
 * Initializes this node's style
 */
void FUIStringNode_Image::InitializeStyle( const FUICombinedStyleData& StyleData )
{
	if ( RenderedImage != NULL )
	{
		RenderedImage->SetImageStyle(StyleData);
	}
}

/**
 * Calculates the precise extent of this text node, and assigns the result to UIStringNode::Extent
 *
 * @param	DefaultLineHeight		the default height of a single line in the string...used by UIStringNode_Image to
 *									scale the image correctly.
 */
void FUIStringNode_Image::CalculateExtent( FLOAT DefaultLineHeight, FLOAT ViewportHeight )
{
	if ( RenderedImage != NULL )
	{
		RenderedImage->CalculateExtent(Extent);
		if ( ForcedExtent.X != 0 )
		{
			Extent.X = ForcedExtent.X;
		}

		if ( ForcedExtent.Y != 0 )
		{
			Extent.Y = ForcedExtent.Y;
		}
		else
		{
			Extent.Y = DefaultLineHeight;
		}

		Extent.X *= Scaling.X;
		Extent.Y *= Scaling.Y;
	}
}

/**
 * Renders this UIStringNode using the parameters specified.
 *
 * @param	Canvas		the canvas to use for rendering this node
 * @param	Parameters	the bounds for the region that this node should render to
 *						the Scaling value of Parameters will be applied against the Scaling
 *						value for this node.  The DrawXL/YL of the Parameters are used to
 *						determine whether this node has enough room to render itself completely.
 */
void FUIStringNode_Image::Render_Node( FCanvas* Canvas, const FRenderParameters& Parameters )
{
	if ( RenderedImage != NULL )
	{
		// @todo: scaling support
		if(TexCoords.IsZero())
		{
			RenderedImage->Render_Texture(Canvas, Parameters);
		}
		else
		{
			FRenderParameters AdjustedParameters = Parameters;
			AdjustedParameters.DrawCoords = TexCoords;

			RenderedImage->Render_Texture(Canvas, AdjustedParameters);
		}
	}
}


/* ==========================================================================================================
	FUIDataStoreBinding
========================================================================================================== */
/**
 * Unregisters any bound data stores and clears all references.
 */
UBOOL FUIDataStoreBinding::ClearDataBinding()
{
	UBOOL bResult = Subscriber || (ResolvedDataStore != NULL);

	UnregisterSubscriberCallback();
	ResolvedDataStore = NULL;
	return bResult;
}

/**
 * Registers the current subscriber with ResolvedDataStore's list of RefreshSubscriberNotifies
 */
void FUIDataStoreBinding::RegisterSubscriberCallback()
{
	if ( Subscriber && ResolvedDataStore != NULL )
	{
		ResolvedDataStore->eventSubscriberAttached(Subscriber);
	}
}

/**
 * Removes the current subscriber from ResolvedDataStore's list of RefreshSubscriberNotifies.
 */
void FUIDataStoreBinding::UnregisterSubscriberCallback()
{
	if ( Subscriber && ResolvedDataStore != NULL )
	{
		// if we have an existing value for both subscriber and ResolvedDataStore, unregister our current subscriber
		// from the data store's list of refresh notifies
		ResolvedDataStore->eventSubscriberDetached(Subscriber);
	}
}

/**
 * Determines whether the specified data field can be assigned to this data store binding.
 *
 * @param	DataField	the data field to verify.
 *
 * @return	TRUE if DataField's FieldType is compatible with the RequiredFieldType for this data binding.
 */
UBOOL FUIDataStoreBinding::IsValidDataField( const FUIDataProviderField& DataField ) const
{
	return IsValidDataField((EUIDataProviderFieldType)DataField.FieldType);
}

/**
 * Determines whether the specified field type is valid for this data store binding.
 *
 * @param	FieldType	the data field type to check
 *
 * @return	TRUE if FieldType is compatible with the RequiredFieldType for this data binding.
 */
UBOOL FUIDataStoreBinding::IsValidDataField( EUIDataProviderFieldType FieldType ) const
{
	UBOOL bResult = FALSE;

	if ( RequiredFieldType != DATATYPE_MAX && RequiredFieldType != DATATYPE_Provider )
	{
		if ( RequiredFieldType == DATATYPE_Collection )
		{
			bResult = (FieldType == DATATYPE_Collection || FieldType == DATATYPE_ProviderCollection);
		}
		else
		{
			bResult = FieldType == RequiredFieldType;
		}
	}

	return bResult;
}

/**
 * Resolves the value of MarkupString into a data store reference, and fills in the values for all members of this struct
 *
 * @param	InSubscriber	the subscriber that contains this data store binding
 *
 * @return	TRUE if the markup was successfully resolved.
 */
UBOOL FUIDataStoreBinding::ResolveMarkup( TScriptInterface<IUIDataStoreSubscriber> InSubscriber )
{
	UBOOL bResult = FALSE;

	UnregisterSubscriberCallback();

	Subscriber = InSubscriber;
	if ( Subscriber && MarkupString.Len() > 0 )
	{
		UUIScene* OwnerScene = CastChecked<UUIScreenObject>(Subscriber.GetObject())->GetScene();
		FUIStringParser Parser;

		// break up the string into normal text and markup chunks
		Parser.ScanString(MarkupString);

		const TIndirectArray<FTextChunk>* Chunks = Parser.GetTextChunks();
		if ( Chunks->Num() == 1 )
		{
			const FTextChunk* NextChunk = &((*Chunks)(0));
			if ( NextChunk->IsMarkup() )
			{
				FString DataStoreTag, DataStoreValue;
				if ( NextChunk->GetDataStoreMarkup(DataStoreTag, DataStoreValue) )
				{
					ResolvedDataStore = OwnerScene->ResolveDataStore(*DataStoreTag);
					if ( ResolvedDataStore != NULL )
					{
						// save the resolved tags for later use
						DataStoreName = *DataStoreTag;
						DataStoreField = *DataStoreValue;

						// and register this subscriber with the newly resolved data store
						RegisterSubscriberCallback();
						bResult = TRUE;
					}
				}
			}
		}
	}

	return bResult;
}

	
/**
 * Retrieves the value for this data store binding from the ResolvedDataStore.
 *
 * @param	out_ResolvedValue	will contain the value of the data store binding.
 *
 * @return	TRUE if the value for this data store binding was successfully retrieved from the data store.
 */
UBOOL FUIDataStoreBinding::GetBindingValue( FUIProviderFieldValue& out_ResolvedValue ) const
{
	UBOOL bResult = FALSE;

	if ( ResolvedDataStore != NULL && DataStoreField != NAME_None )
	{
		bResult = ResolvedDataStore->GetDataStoreValue(DataStoreField.ToString(), out_ResolvedValue);
	}

	return bResult;
}

/**
 * Publishes the value for this data store binding to the ResolvedDataStore.
 *
 * @param	NewValue	contains the value that should be published to the data store
 *
 * @return	TRUE if the value was successfully published to the data store.
 */
UBOOL FUIDataStoreBinding::SetBindingValue( const FUIProviderScriptFieldValue& NewValue ) const
{
	UBOOL bResult = FALSE;

	if ( ResolvedDataStore != NULL && DataStoreField != NAME_None )
	{
		bResult = ResolvedDataStore->SetDataStoreValue(DataStoreField.ToString(), NewValue);
	}

	return bResult;
}

/* ==========================================================================================================
	FUITextAttributes
========================================================================================================== */
/**
 * Resets the values for all attributes to false.
 */
void FUITextAttributes::Reset()
{
	Bold = FALSE;
	Italic = FALSE;
	Underline = FALSE;
	Shadow = FALSE;
	Strikethrough = FALSE;
}

/* ==========================================================================================================
	FUIStringNodeModifier
========================================================================================================== */
/**
 * Constructor (deprecated)
 *
 * @param	SourceStyle		the style to use for initializing the CustomStyleData member;  normally the UIString's DefaultStringStyle
 * @param	MenuState		the current menu state of the widget that owns the UIString.
 */
FUIStringNodeModifier::FUIStringNodeModifier( UUIStyle_Data* SourceStyle, UUIState* MenuState )
: CurrentMenuState(MenuState)
{
	check(CurrentMenuState);

	AddStyle(SourceStyle);
	check(CustomStyleData.DrawFont);
}

FUIStringNodeModifier::FUIStringNodeModifier( const FUICombinedStyleData& SourceStyleData, UUIState* MenuState )
: CustomStyleData(SourceStyleData), BaseStyleData(SourceStyleData), CurrentMenuState(MenuState)
{
	check(CurrentMenuState);
	check(CustomStyleData.DrawFont);
	ModifierStack.AddZeroed();
}

/** Copy constructor */
FUIStringNodeModifier::FUIStringNodeModifier( const FUIStringNodeModifier& Other )
{
	check(Other.CurrentMenuState);
	check(CustomStyleData.DrawFont);

	CustomStyleData = Other.CustomStyleData;
	BaseStyleData = Other.BaseStyleData;
	CurrentMenuState = Other.CurrentMenuState;
	ModifierStack = Other.ModifierStack;
}

/**
 * Adds the specified font to the InlineFontStack of the current ModifierData, then updates the DrawFont of CustomStyleData to point to the new font
 *
 * @param	NewFont	the font to use when creating new string nodes
 *
 * @return	TRUE if the specified font was successfully added to the list.
 */
UBOOL FUIStringNodeModifier::AddFont( UFont* NewFont )
{
	// we should always have at least one element in the ModifierStack.
	check(ModifierStack.Num() > 0);

	UBOOL bResult = FALSE;

	if ( NewFont != NULL )
	{
		FModifierData& ActiveModifier = ModifierStack.Last();

		// each element in the font stack must be unique, so only add the new font if it isn't already in the list
		if ( !ActiveModifier.InlineFontStack.ContainsItem(NewFont) )
		{
			// add this font to the top of the stack
			ActiveModifier.InlineFontStack.Push(NewFont);

			// set the current font to this new font
			CustomStyleData.DrawFont = NewFont;
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Removes a font from the InlineFontStack of the current ModifierData.  If the font that was removed was the style data container's
 * current DrawFont, updates CustomStyleData's font as well.
 *
 * @param	FontToRemove	if specified, the font to remove.  If NULL, removes the font at the top of the stack.
 *
 * @return	TRUE if the font was successfully removed from the InlineFontStack.  FALSE if the font wasn't part of the InlineFontStack
 */
UBOOL FUIStringNodeModifier::RemoveFont( UFont* FontToRemove/*=NULL*/ )
{
	// we should always have at least one element in the ModifierStack.
	check(ModifierStack.Num() > 0);

	UBOOL bResult = FALSE;

	FModifierData& ActiveModifier = ModifierStack.Last();
	if ( FontToRemove == NULL && ActiveModifier.InlineFontStack.Num() > 0 )
	{
		FontToRemove = ActiveModifier.InlineFontStack.Last();
	}

	if ( FontToRemove != NULL )
	{
		if ( ActiveModifier.InlineFontStack.ContainsItem(FontToRemove) )
		{
			// remove this font from the stack of fonts
			ActiveModifier.InlineFontStack.RemoveItem(FontToRemove);

			// if the font being removed is the current font, set the current font to the top element in the FontStack
			if ( FontToRemove == CustomStyleData.DrawFont )
			{
				// if there are other inline fonts, use that one
				if ( ActiveModifier.InlineFontStack.Num() > 0 )
				{
					CustomStyleData.DrawFont = ActiveModifier.InlineFontStack.Last();
				}
				else
				{
					// no more inline fonts, reinitialize using the current inline style or base style
					if ( ActiveModifier.Style != NULL )
					{
						CustomStyleData.InitializeStyleDataContainer(ActiveModifier.Style,FALSE);
					}
					else
					{
						check(BaseStyleData.IsInitialized());
						check(BaseStyleData.DrawFont!=NULL);
						CustomStyleData = BaseStyleData;
					}
				}
			}

			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Adds a new element to the ModifierStack using the specified style, then reinitializes the CustomStyleData with the values from this style.
 *
 * @param	NewStyle	the style to add to the stack
 *
 * @return	TRUE if the specified style was successfully added to the list.
 */
UBOOL FUIStringNodeModifier::AddStyle( UUIStyle_Data* NewStyle )
{
	UBOOL bResult = FALSE;

	if ( NewStyle != NULL )
	{
		// each element in the style stack must be unique, so only add the new style if it isn't already in the list
		INT ExistingIndex = FindModifierIndex(NewStyle);
		if ( ExistingIndex == INDEX_NONE )
		{
			// create a new ModifierData for this style, and add it to the top of the ModifierStack
			INT Index = ModifierStack.AddZeroed();
			FModifierData& Modifier = ModifierStack(Index);

			// assign the new style
			Modifier.Style = NewStyle;

			// reinitialize our local style data using this new style
			CustomStyleData.InitializeStyleDataContainer(NewStyle,FALSE);

			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Removes the element containing StyleToRemove from ModifierStack.  If the style that was removed was style at the top of the StyleStack,
 * reinitializes CustomStyleData with the style data from the previous style in the stack.
 *
 * @param	StyleToRemove	if specified, the style to remove.  If NULL, removes the style at the top of the stack.
 *
 * @return	TRUE if the style was successfully removed from the ModifierStack.  FALSE if the style wasn't part of the ModifierStack or it
 *			was the last node in the ModifierStack (which cannot be removed).
 */
UBOOL FUIStringNodeModifier::RemoveStyle( UUIStyle_Data* StyleToRemove/*=NULL*/ )
{
	UBOOL bResult = FALSE;

	if ( StyleToRemove == NULL && ModifierStack.Num() > 0 )
	{
		FModifierData& Modifier = ModifierStack.Last();
		StyleToRemove = Modifier.Style;
	}

	// can't remove the last item, so StyleStack must contain at least 2 elements in order to remove a style
	if ( StyleToRemove != NULL && ModifierStack.Num() > 1 )
	{
		INT ModifierIndex = FindModifierIndex(StyleToRemove);
		UBOOL bIsCurrentStyle = (ModifierIndex == ModifierStack.Num() - 1);

		// remove this style from the stack
		ModifierStack.Remove(ModifierIndex);

		// if we are popping the current style, reinitialize our local style data with the previous style in the stack
		if ( bIsCurrentStyle == TRUE )
		{
			FModifierData& ActiveModifier = ModifierStack.Last();
			if ( ActiveModifier.Style != NULL )
			{
				CustomStyleData.InitializeStyleDataContainer(ActiveModifier.Style,FALSE);
			}
			else
			{
				check(BaseStyleData.IsInitialized());
				check(BaseStyleData.DrawFont!=NULL);
				CustomStyleData = BaseStyleData;
			}

			// if the ActiveModifier contained inline fonts, set the DrawFont to the top-most font from ActiveModifier
			if ( ActiveModifier.InlineFontStack.Num() > 0 )
			{
				CustomStyleData.DrawFont = ActiveModifier.InlineFontStack.Last();

				// we check for NULL fonts before we add them to the InlineFontStack so if we have a null font here, something really bad happened! 
				checkf(CustomStyleData.DrawFont, TEXT("NULL entry detected in InlineFontStack when removing style data %s (%s)"), *StyleToRemove->GetFullName(), 
					StyleToRemove->GetOwnerStyle() ? *StyleToRemove->GetOwnerStyle()->GetStyleName() : TEXT("N/A"));
			}
		}

		bResult = TRUE;
	}

	return bResult;
}

/**
 * Returns the location of the ModifierData that contains the specified style.
 *
 * @param	SearchStyle	the style to search for
 *
 * @return	an index into the ModifierStack array for the ModifierData that contains the specified style, or INDEX_NONE
 *			if there are no elements referencing that style.
 */
INT FUIStringNodeModifier::FindModifierIndex( UUIStyle_Data* SearchStyle )
{
	INT Result = INDEX_NONE;

	for ( INT ModifierIndex = ModifierStack.Num() - 1; ModifierIndex >= 0; ModifierIndex-- )
	{
		UUIStyle_Data* ModifierStyle = ModifierStack(ModifierIndex).Style;
		if ( ModifierStyle == SearchStyle )
		{
			Result = ModifierIndex;
			break;
		}
	}

	return Result;
}

/**
 * Sets the Custom Text Color to use
 *
 * @param	CustomTextColor		The linear color to use
 */

void FUIStringNodeModifier::SetCustomTextColor(FLinearColor CustomTextColor)
{
	CustomStyleData.TextColor = CustomTextColor;
}
		
/**
 * Resets the current text color to the active style's draw color.
 */
void FUIStringNodeModifier::ClearCustomTextColor()
{
	if ( ModifierStack.Num() > 0 )
	{
		FModifierData& ActiveModifier = ModifierStack.Last();
		if ( ActiveModifier.Style != NULL )
		{
			UUIStyle_Text* TextStyle = Cast<UUIStyle_Text>(ActiveModifier.Style);
			if ( TextStyle == NULL )
			{
				UUIStyle_Combo* ComboStyle = Cast<UUIStyle_Combo>(ActiveModifier.Style);
				if ( ComboStyle != NULL )
				{
					TextStyle = Cast<UUIStyle_Text>(ComboStyle->TextStyle.GetStyleData());
				}
			}

			if ( TextStyle != NULL )
			{
				CustomStyleData.TextColor = TextStyle->StyleColor;
			}
			else
			{
				CustomStyleData.TextColor = ActiveModifier.Style->StyleColor;
			}
		}
		else
		{
			check(BaseStyleData.IsInitialized());
			check(BaseStyleData.DrawFont!=NULL);
			CustomStyleData.TextColor = BaseStyleData.TextColor;
		}
	}
}

FLinearColor FUIStringNodeModifier::GetCustomTextColor()
{
	return CustomStyleData.TextColor;
}

/**
 * Returns the style data contained by this string customizer
 */
const FUICombinedStyleData& FUIStringNodeModifier::GetCustomStyleData() const
{
	return CustomStyleData;
}
