//------------------------------------------------------------------------------
// A text tag parser for UNICODE strings.
// 
// This parser changes the format from HTML to a simpler format.
// The [happy] van went over a [other] bump [/happy] the size of an [/other] ox.
// The format of the tag is as such:
// [curvename (type={quad|lt|ct||tt}) (duration=real) (timeshift=real)
//		(easein=real) (easeout=real) (v1=real) (v2=real) (v3=real) (v4=real)]
// To close a tag, simply use:
// [/curvename]
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxUnicodeTextTagParser_H__
#define FxUnicodeTextTagParser_H__

#include "FxPlatform.h"
#include "FxArray.h"
#include "FxString.h"
#include "FxAnalysis.h"

namespace OC3Ent
{

namespace Face
{

// Forward-declare the phoneme/word list.
class FxPhonWordList;

//------------------------------------------------------------------------------
// FxUnicodeTextTagInfo
// Contains the information for a single text tag.
//------------------------------------------------------------------------------
class FxUnicodeTextTagInfo
{
public:
	enum FxCurveType
	{
		CT_Quadruplet      = 0,
		CT_LeadingTriplet  = 1,
		CT_CenterTriplet   = 2,
		CT_TrailingTriplet = 3
	};

	enum FxTagError
	{
		TE_None,				// The tag was parsed without error.
		TE_UnknownParameter,	// The tag contained an unknown parameter.
		TE_MalformedTag			// The tag was malformed.
	};

	// Default constructor.
	FxUnicodeTextTagInfo();
	// Constructs from a tag.
	FxUnicodeTextTagInfo(const FxWString& tag);
	// Destructor.
	~FxUnicodeTextTagInfo();

	// Initializes the tag info from a tag.
	void InitializeTag(const FxWString& tag);

	// Returns the error state of the tag.
	FxTagError GetError() const;

	// Returns the curve name.
	const FxString& GetCurveName( void ) const;
	// Sets the curve name.
	void SetCurveName( const FxString& curveName );

	// Returns the curve creation type.
	FxCurveType GetCurveType( void ) const;
	// Sets the curve creation type.
	void SetCurveType( FxCurveType curveType );

	// Returns the duration.
	FxReal GetDuration( void ) const;
	// Sets the duration.
	void SetDuration( FxReal duration );

	// Returns the time shift.
	FxReal GetTimeShift( void ) const;
	// Sets the time shift.
	void SetTimeShift( FxReal timeShift );

	// Returns the ease in time.
	FxReal GetEaseIn( void ) const;
	// Sets the ease in time.
	void SetEaseIn( FxReal easeIn );

	// Returns the ease out time.
	FxReal GetEaseOut( void ) const;
	// Sets the ease out time.
	void SetEaseOut( FxReal easeOut );

	// Returns the value of the first key.
	FxReal GetV1( void ) const;
	// Sets the value of the first key.
	void SetV1( FxReal v1 );

	// Returns the value of the second key.
	FxReal GetV2( void ) const;
	// Sets the value of the second key.
	void SetV2( FxReal v2 );

	// Returns the value of the third key.
	FxReal GetV3( void ) const;
	// Sets the value of the third key.
	void SetV3( FxReal v3 );

	// Returns the value of the fourth key.
	FxReal GetV4( void ) const;
	// Sets the value of the fourth key.
	void SetV4( FxReal v4 );

	// Returns the index of the start word.
	FxSize GetStartWord( void ) const;
	// Sets the index of the start word.
	void SetStartWord( FxSize startWord );

	// Returns the index of the end word.
	FxSize GetEndWord( void ) const;
	// Sets the index of the end word.
	void SetEndWord( FxSize endWord );

	// Returns the original tag.
	const FxWString& GetOriginalTag() const;
	// Returns true if this is a closing tag.
	FxBool IsOpeningTag() const;

protected:
	// Preprocesses the tag, removing unneeded spaces and converting it to uppercase.
	void PreprocessTag(const FxWString& tagBody);
	// Extracts the curve name from a tag body.  Returns the body minus the name.
	FxWString ExtractCurveName(const FxWString& tagBody);
	// Process the tag, extracting the name and any parameters.
	void ProcessTag(const FxWString& tag);
	// Handles a single tag parameter
	void ProcessParameter(const FxWString& parameter);

	// Tokenizes a string with whitespace as the tokens.
	FxArray<FxWString> Tokenize(FxWString str);

	// Tag processing storage.
	FxWString _originalTag;		// The tag in its original format.
	FxWString _preprocessedTag;	// The tag after being preprocessed;

	FxTagError	_tagError;		// Whatever error we found processing the tag.

	// Extracted tag information.
	FxString	_curveName;		// The name of the curve.
	FxCurveType _curveType;		// The type of curve to be created.
	FxReal		_duration;		// The amount of time to hold the curve.
	FxReal		_timeShift; 	// The time shift.
	FxReal		_easeIn;		// The ease in time.
	FxReal		_easeOut;		// The east out time.
	FxReal		_v1;			// The value of the first key.
	FxReal		_v2;			// The value of the second key.
	FxReal		_v3;			// The value of the third key.
	FxReal		_v4;			// The value of the fourth key.
	FxSize		_startWord;		// The index of the start word.
	FxSize		_endWord;		// The index of the end word.

	FxBool		_isOpeningTag;	// True if this is an opening tag.
};

//------------------------------------------------------------------------------
// FxUnicodeTextTagParser
// Parses texts for analysis containing embedded text tags.
//------------------------------------------------------------------------------
class FxUnicodeTextTagParser
{
public:
	enum FxTextTagError
	{
		TTE_None = 0,				// The text was parsed without error.
		TTE_UnmatchedTag = 1,		// The text contained an extra opening or closing tag.
		TTE_UnmatchedBracket = 2,	// The text contained an extra opening or closing bracket.
		TTE_UnmatchedWord = 3		// The word list contained a word that was not found in the analysis text.
	};

	// Constructor.
	FxUnicodeTextTagParser();
	// Constructs from a tagged analysis text.
	FxUnicodeTextTagParser(const FxWString& analysisText);
	// Destructor.
	~FxUnicodeTextTagParser();

	// Initializes the parser with a tagged analysis text.
	FxBool InitializeParser(const FxWString& analysisText);
	// Parses according to the words in the word list.
	void Parse(const FxArray<FxWordTimeInfo>& wordList);

	// Gets the string stripped of all its punctuation.
	const FxWString& GetPartiallyStrippedText() const;
	// Gets the string stripped of all its text tags and punctuation.
	const FxWString& GetStrippedText() const;

	// Returns the number of text tags.
	FxSize GetNumTextTags( void ) const;
	// Returns the text tag at index.
	const FxUnicodeTextTagInfo& GetTextTag( const FxSize index ) const;

	// Returns the error that occurred during parsing.
	FxTextTagError GetError() const;

protected:
	// Strips any unsupported punctuation from the string.
	void StripPunctuation(const FxWString text);
	// Replaces the text tags with markers.
	FxBool ReplaceTextTagsWithMarkers(const FxWString& text);
	// Replaces words with markers.
	FxBool ReplaceWordsWithMarkers(const FxWString& text, const FxArray<FxWordTimeInfo>& wordList);
	// Strips all extraneous characters.
	FxBool StripExtraneousCharacters(const FxWString& text);

	// Makes a tag marker for the tag at index.
	FxWString MakeTagMarker(FxSize index);
	// Makes a word marker for the word at index.
	FxWString MakeWordMarker(FxSize index);
	// Returns the tag index for a tag marker
	FxSize ReadTagMarker(const FxWString& tagMarker);
	// Returns the word index for a word marker
	FxSize ReadWordMarker(const FxWString& wordMarker);

	FxWString _originalText; // The text in its original format.
	FxWString _noPuncText;	 // The text with the appropriate punctuation stripped.
	FxWString _strippedText; // The text with tags stripped out.
	FxWString _stageOne;	 // The text with text tags replaced with markers.
	FxWString _stageTwo;	 // The text with words replaced with markers.
	FxWString _stageThree;	 // The text with all extraneous characters stripped.

	FxTextTagError _error;	 // The error state of the parser.

	FxArray<FxUnicodeTextTagInfo> _tagInfos; // The tagged information from the text.
};

} // namespace Face

} // namespace OC3Ent

#endif

