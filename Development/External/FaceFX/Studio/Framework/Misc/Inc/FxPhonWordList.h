//------------------------------------------------------------------------------
// Stores a list of phonemes and words, and their timings, for a given animation
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxPhonWordList_H__
#define FxPhonWordList_H__

#include "FxPlatform.h"
#include "FxObject.h"
#include "FxTypeTraits.h"
#include "FxString.h"
#include "FxList.h"
#include "FxPhonemeInfo.h"
#include "FxSimplePhonemeList.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/string.h"
#endif

namespace OC3Ent
{

namespace Face
{

// Stores the information for a single phoneme.
struct FxPhonInfo
{
	FxPhonInfo()
		: phoneme(PHON_SIL)
		, start(0.f)
		, end(0.f)
		, selected(FxFalse)
	{
	}

	FxPhonInfo(const FxPhoneme iPhon, const FxReal iStart, const FxReal iEnd, const FxBool iSel)
		: phoneme(iPhon)
		, start(iStart)
		, end(iEnd)
		, selected(iSel)
	{
	}

	FxPhoneme phoneme;
	FxReal    start;
	FxReal    end;
	FxBool    selected;
};

struct FxWordInfo
{
	FxWordInfo() {}
	FxWordInfo(const wxString& iText, FxSize iFirst, FxSize iLast)
		: text(iText)
		, first(iFirst)
		, last(iLast)
	{
	}
	wxString  text;
	FxSize   first;
	FxSize   last;
};

// Stores a synchronized list of phonemes and words.
class FxPhonWordList : public FxObject
{
	// Declare the class.
	FX_DECLARE_CLASS(FxPhonWordList, FxObject);
public:
	// Default constructor.
	FxPhonWordList();
	// Copy constructor.
	FxPhonWordList( const FxPhonWordList& other );
	// Assignment operator.
	FxPhonWordList& operator=( const FxPhonWordList& other );
	// Destructor.
	~FxPhonWordList();

	// Returns FxTrue if the phoneme list has changed since the last call to
	// ClearDirtyFlag().
	FxBool IsDirty( void ) const;
	// Clears the internal dirty flag.
	void ClearDirtyFlag( void );

	// Sets the minimum phoneme length.
	FX_INLINE void SetMinPhonemeLength( FxReal minPhonLen ) { _minPhonLen = minPhonLen; }
	// Returns the minimum phoneme length.
	FX_INLINE FxReal GetMinPhonemeLength( void ) { return _minPhonLen; }

//------------------------------------------------------------------------------
// Phoneme Functions
//------------------------------------------------------------------------------
	// Returns the number of phonemes
	FxSize GetNumPhonemes() const;
	// Returns the phoneme enumeration.
	FxPhoneme GetPhonemeEnum( FxSize phonIndex ) const;
	// Returns the phoneme start time.
	FxReal GetPhonemeStartTime( FxSize phonIndex ) const;
	// Returns the phoneme end time.
	FxReal GetPhonemeEndTime( FxSize phonIndex ) const;
	// Returns the phoneme duration.
	FxReal GetPhonemeDuration( FxSize phonIndex ) const;
	// Returns if a phoneme is selected.
	FxBool GetPhonemeSelected( FxSize phonIndex ) const;
	
	// Returns true if the phoneme is contained in a word.
	FxBool IsPhonemeInWord( FxSize phonIndex, FxSize* wordIndex = NULL);

	// Selects a phoneme.
	void SetPhonemeSelected( FxSize phonIndex, FxBool selected );
	// Sets a phoneme type.
	void SetPhonemeEnum( FxSize phonIndex, FxPhoneme phoneme );

	// Removes all phonemes from the list.
	void ClearPhonemes();
	// Removes a single phoneme from the list.
	void RemovePhoneme( FxSize phonIndex );
	// Obliterates a phoneme from the list.
	void ObliteratePhoneme( FxSize phonIndex );

	enum InsertionMethod
	{
		SPLIT_TIME    = 1,
		PUSH_OUT      = 2
	};

	// Inserts a phoneme into the list
	void InsertPhoneme( FxPhoneme phoneme, FxSize index, InsertionMethod method );
	// Inserts a phoneme with given duration in the list
	void InsertPhoneme( FxPhoneme phoneme, FxReal duration, FxSize index );

	// Adds a phoneme to the end of the list.
	void AppendPhoneme( const FxPhonInfo& phoneme );
	void AppendPhoneme( FxPhoneme phoneme, FxReal startTime, FxReal endTime );

	// Modifies a phoneme's start time.  Returns the actual start time.
	FxReal ModifyPhonemeStartTime( FxSize phonIndex, FxReal newStartTime );
	// Modifies a phoneme's end time.  Returns the actual end time.
	FxReal ModifyPhonemeEndTime( FxSize phonIndex, FxReal newEndTime );
	// Pushes a phoneme's start time, moving every preceding phoneme by a delta.
	FxReal PushPhonemeStartTime( FxSize phonIndex, FxReal newStartTime );
	// Pushes a phoneme's end time, moving every succeeding phoneme by a delta.
	FxReal PushPhonemeEndTime( FxSize phonIndex, FxReal newEndTime );

	// Swaps two neighboring phonemes.
	FxSize SwapPhonemes( FxSize primary, FxSize secondary );

	// Swaps two phoneme ranges, updating words.  They must be non-overlapping.
	FxSize SwapPhoneRanges( FxSize primaryStart, FxSize primaryEnd, 
							 FxSize secondaryStart, FxSize secondaryEnd );

	// Returns the index of the phoneme at a time.
	FxSize FindPhoneme( FxReal time );

//------------------------------------------------------------------------------
// Word Functions.
//------------------------------------------------------------------------------
	// Returns the number of words.
	FxSize GetNumWords() const;
	// Returns the word text for a given word.
	wxString GetWordText( FxSize wordIndex ) const;
	// Returns the start time for a given word.
	FxReal GetWordStartTime( FxSize wordIndex ) const;
	// Returns the end time for a given word.
	FxReal GetWordEndTime( FxSize wordIndex ) const;
	// Returns the duration for a given word.
	FxReal GetWordDuration( FxSize wordIndex ) const;
	// Returns if a word is selected.  A word is considered selected if every
	// phoneme in the word is selected.
	FxBool GetWordSelected( FxSize wordIndex ) const;
	// Returns the index of the first phoneme in a word
	FxSize GetWordStartPhoneme( FxSize wordIndex ) const;
	// Returns the index of the last phoneme in a word
	FxSize GetWordEndPhoneme( FxSize wordIndex ) const;

	// Sets a word selected.
	void SetWordSelected( FxSize wordIndex, FxBool selected );

	// Removes all words from the list.
	void ClearWords();

	// Groups a set of phonemes into a word.
	void GroupToWord( const wxString& text, FxSize start, FxSize end );
	// Groups phonemes in the specified time range to the word.
	void GroupToWord( const wxString& text, FxReal wordStartTime, FxReal wordEndTime );
	// Ungroups a range of phonemes.
	void Ungroup( FxSize start, FxSize end );
	
	// Modifies a word's start time.
	FxReal ModifyWordStartTime( FxSize wordIndex, FxReal newStartTime, FxBool dontChangePreceding = FxFalse );
	// Modifies a word's end time.
	FxReal ModifyWordEndTime( FxSize wordIndex, FxReal newEndTime, FxBool dontChangeSuceeding = FxFalse );

//------------------------------------------------------------------------------
// General functions.
//------------------------------------------------------------------------------
	// Clears both the phoneme and word lists.
	void Clear();

	// Starts a selection.
	void StartSelection( FxSize wordIndex, FxSize phonemeIndex, FxBool selected = FxTrue );
	// Ends a selection.
	void EndSelection( FxSize wordIndex, FxSize phonemeIndex );
	// Clears current selection.
	void ClearSelection();
	// Removes all the phonemes in the current selection.
	void RemoveSelection();
	// Obliterates the selection (collapses remaining phonemes <- that way).
	void ObliterateSelection();
	// Returns true if there is any selection.
	FxBool HasSelection() const;

	// Changes the entire selection to a single phoneme.
	void ChangeSelection( FxPhoneme phonEnum );

	// Converts the list to a generic format.
	void ToGenericFormat( FxPhonemeList& genericPhonList );

	// Splice in a phoneme list.
	void Splice( const FxPhonWordList& other, FxSize startIndex, FxSize endIndex);

//------------------------------------------------------------------------------
// Serialization.
//------------------------------------------------------------------------------
	// Serialize the object.
	virtual void Serialize( FxArchive& arc );

	// Proofs the phoneme/word list for any consistancy and corruption issues.
	// Returns false if an "unfixable" issue was found.
	FxBool Proof();

private:

	// The list of phonemes.
	FxArray<FxPhonInfo> _phonemes;
	// The list of words.
	FxArray<FxWordInfo> _words;

	// Internal use only.
	FxSize _startWord;
	// Internal use only.
	FxSize _startPhon;

	// The minimum phoneme length.
	FxReal _minPhonLen;

	// The "dirty" flag.  This flag is FxTrue if the phoneme / word list has
	// been modified since the last call to ClearDirtyFlag().  This is used
	// to determine when to re-coarticulate via a call to IsDirty().
	FxBool _isDirty;
};

} // namespace Face

} // namespace OC3Ent

#endif
