/**
 *
 * A font object, containing information about a set of glyphs.
 * The glyph bitmaps are stored in the contained textures, while
 * the font database only contains the coordinates of the individual
 * glyph.
 * 
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Font extends Object
	hidecategories(object)
	dependsOn(FontImportOptions)
	native;


// This is the character that RemapChar will return if the specified character doesn't exist in the font
const NULLCHARACTER = 127;

/** this struct is serialized using binary serialization so any changes to it require a package version bump */
struct immutable native FontCharacter
{
	var() int StartU;
	var() int StartV;
	var() int USize;
	var() int VSize;
	var() BYTE TextureIndex;
	var() int VerticalOffset;

	structcpptext
	{
		// Serializer.
		friend FArchive& operator<<( FArchive& Ar, FFontCharacter& Ch )
		{
			Ar << Ch.StartU << Ch.StartV << Ch.USize << Ch.VSize << Ch.TextureIndex;

			if( Ar.Ver() < VER_FONT_FORMAT_AND_UV_TILING_CHANGES )
			{
				Ch.VerticalOffset = 0;
			}
			else
			{
				Ar << Ch.VerticalOffset;
			}

			return Ar;
		}
	}
};


/** List of characters in the font.  For a MultiFont, this will include all characters in all sub-fonts!  Thus,
    the number of characters in this array isn't necessary the number of characters available in the font */
var() editinline array<FontCharacter> Characters;

/** Textures that store this font's glyph image data */
//NOTE: Do not expose this to the editor as it has nasty crash potential
var array<Texture2D> Textures;

/** When IsRemapped is true, this array maps unicode values to entries in the Characters array */
var private const native Map{WORD,WORD} CharRemap;

/** True if font is 'remapped'.  That is, the character array is not a direct mapping to unicode values.  Instead,
    all characters are indexed indirectly through the CharRemap array */
var int IsRemapped;

/** Default horizontal spacing between characters when rendering text with this font */
var() int Kerning;

/** Options used when importing this font */
var() FontImportOptionsData ImportOptions;

/** Number of characters in the font, not including multiple instances of the same character (for multi-fonts).
    This is cached at load-time or creation time, and is never serialized. */
var transient int NumCharacters;

/** The maximum height of a character in this font.  For multi-fonts, this array will contain a maximum
    character height for each multi-font, otherwise the array will contain only a single element.  This is
    cached at load-time or creation time, and is never serialized. */
var transient array<int> MaxCharHeight;


cpptext
{
	/**
	 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
	 *
	 * @return		Size of resource as to be displayed to artists/ LDs in the Editor.
	 */
	virtual INT GetResourceSize();

	// UFont interface
	FORCEINLINE TCHAR RemapChar(TCHAR CharCode) const
	{
		const WORD UCode = ToUnicode(CharCode);
		if ( IsRemapped )
		{
			// currently, fonts are only remapped if they contain Unicode characters.
			// For remapped fonts, all characters in the CharRemap map are valid, so
			// if the characters exists in the map, it's safe to use - otherwise, return
			// the null character (an empty square on windows)
			const WORD* FontChar = CharRemap.Find(UCode);
			if ( FontChar == NULL )
				return UCONST_NULLCHARACTER;

			return (TCHAR)*FontChar;
		}

		// Otherwise, our Characters array will contains 256 members, and is
		// a one-to-one mapping of character codes to array indexes, though
		// not every character is a valid character.
		if ( UCode >= NumCharacters )
		{
			return UCONST_NULLCHARACTER;
		}

		// If the character's size is 0, it's non-printable or otherwise unsupported by
		// the font.  Return the default null character (an empty square on windows).
		if ( Characters(UCode).VSize == 0 && UCode >= TEXT(' ') )
		{
			return UCONST_NULLCHARACTER;
		}

		return CharCode;
	}

	FORCEINLINE void GetCharSize(TCHAR InCh, FLOAT& Width, FLOAT& Height, INT ResolutionPageIndex=0) const
	{
		Width = Height = 0.f;

		const INT Ch = (INT)RemapChar(InCh) + ResolutionPageIndex;
		if( Ch < Characters.Num() )
		{
			const FFontCharacter& Char = Characters(Ch);
			if( Char.TextureIndex < Textures.Num() && Textures(Char.TextureIndex) != NULL )
			{
				Width = Char.USize;
				
				// The height of the character will always be the maximum height of any character in this
				// font.  This ensures consistent vertical alignment of text.  For example, we don't want
				// vertically centered text to visually shift up and down as characters are added to a string.
				// NOTE: This also gives us consistent alignment with fonts generated by the legacy importer.
				const INT MultiFontIndex = Ch / NumCharacters;
				Height = MaxCharHeight( MultiFontIndex );
			}
		}
	}

	/**
	 * Calculate the width of the string using this font's default size and scale.
	 *
	 * @param	Text					the string to size
	 * @param	ResolutionPageIndex		the index for the multi-font page to use; get by calling GetResolutionPageIndex()
	 *
	 * @return	the width (in pixels) of the specified text, or 0 if Text was NULL.
	 */
	FORCEINLINE INT GetStringSize( const TCHAR *Text, INT ResolutionPageIndex=0 ) const
	{
		FLOAT	Width, Height, Total;

		Total = 0.0f;
		while( *Text )
		{
			GetCharSize( *Text++, Width, Height, ResolutionPageIndex );
			Total += Width;
		}

		return( appCeil( Total ) );
	}

	// UObject interface

	/**
	* Serialize the object struct with the given archive
	*
	* @param Ar - archive to serialize with
	*/
	virtual void Serialize( FArchive& Ar );

	/**
	* Called after object and all its dependencies have been serialized.
	*/
	virtual void PostLoad();

    /**
     * Caches the character count and maximum character height for this font (as well as sub-fonts, in the multi-font case)
     */
    virtual void CacheCharacterCountAndMaxCharHeight();
    
	virtual UBOOL IsLocalizedResource();
}

/**
 * Calulate the index for the texture page containing the multi-font character set to use, based on the specified screen resolution.
 *
 * @param	HeightTest	the height (in pixels) of the viewport being rendered to.
 *
 * @return	the index of the multi-font "subfont" that most closely matches the specified resolution.  this value is used
 *			as the value for "ResolutionPageIndex" when calling other font-related methods.
 */
native function int GetResolutionPageIndex(float HeightTest) const;

/**
 * Calculate the amount of scaling necessary to match the multi-font subfont which most closely matches the specified resolution.
 *
 * @param	HeightTest	the height (in pixels) of the viewport being rendered to.
 *
 * @return	the percentage scale required to match the size of the multi-font's closest matching subfont.
 */
native function float GetScalingFactor(float HeightTest) const;

/**
 * Determine the height of the mutli-font resolution page which will be used for the specified resolution.
 *
 * @param	ViewportHeight	the height (in pixels) of the viewport being rendered to.
 */
native final function virtual float GetAuthoredViewportHeight( float ViewportHeight ) const;

/**
 * Returns the maximum height for any character in this font
 */
native function float GetMaxCharHeight() const;

