//------------------------------------------------------------------------------
// Extended information about the phonemes.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxPhonemeInfo_H__
#define FxPhonemeInfo_H__

#include "FxPlatform.h"
#include "FxPhonemeEnum.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/string.h"
	#include "wx/colour.h"
#endif

namespace OC3Ent
{

namespace Face
{

// Defines the large phoneme classes we use to group related phonemes.  
// This enum has been extended by including selected, min sized, word and 
// background, to make it easy to implement the options dialog.  Those are
// obviously not phoneme classes.
//@todo Get rid of the "extended" elements of this enum and implement that
//      properly.
enum FxPhonemeClass
{
	PHONCLASS_NONE = 0,
	PHONCLASS_VOWEL,
	PHONCLASS_DIPHTHONG,
	PHONCLASS_FRICATIVE,
	PHONCLASS_NASAL,
	PHONCLASS_APPROXIMATE,
	PHONCLASS_PLOSIVE,
	PHONCLASS_TRILL,
	PHONCLASS_SILENCE,

	PHONCLASS_SELECTED,
	PHONCLASS_MINSIZED,
	PHONCLASS_WORD,
	PHONCLASS_TEXTFOREGROUND,
	PHONCLASS_TEXTBACKGROUND,
	PHONCLASS_BACKGROUND,
	PHONCLASS_CURRTIME,

	// Note that meta information is here because it breaks the 
	// phoneme widget if the order is disrupted.
	PHONCLASS_FIRST = PHONCLASS_VOWEL,
	PHONCLASS_LAST  = PHONCLASS_SILENCE,
	NUM_PHONCLASSES = PHONCLASS_LAST - PHONCLASS_FIRST + 1, 

};

// Contains the information for a phoneme class, namely a human-readable label
// and the colour with which to draw any phoneme in the class.  Used only by the 
// phoneme widget.
struct FxPhonemeClassInfo
{
	FxPhonemeClassInfo(FxPhonemeClass cls = PHONCLASS_SILENCE, wxString str = wxString(wxT("")), wxColour c1 = *wxRED, wxColour c2 = *wxRED )
		: phonClass(cls)
		, label(str)
		, colour(c1)
		, defaultColour(c2)
	{
	}
	FxPhonemeClass phonClass;
	wxString label;
	wxColour colour;
	wxColour defaultColour;
};

// Contains the extended information for a phoneme class: a string identifier
// and a description string.  Used for automatically exposing phoneme class
// information to Python scripting on program startup.
struct FxPhonClassExtendedInfo
{
	FxPhonClassExtendedInfo(FxPhonemeClass cls, wxString nameStr, wxString descStr )
		: phonClass(cls)
		, name(nameStr)
		, desc(descStr)
	{
	}
	FxPhonemeClass phonClass;
	wxString name;
	wxString desc;
};

// Contains three equivalent human-readable labels for a phoneme, the class to
// which it belongs, and a sample word.
struct FxPhonExtendedInfo
{
	FxPhonExtendedInfo()
	{
	}
	FxPhonExtendedInfo(FxPhoneme phon, FxPhonemeClass cls, wxString tb, wxString smpa, wxString ip, wxString sw, wxString cfc)
		: phoneme(phon)
		, phonClass(cls)
		, talkback(tb)
		, sampa(smpa)
		, ipa(ip)
		, sampleWords(sw)
		, classification(cfc)
	{
	}
	FxPhoneme phoneme;
	FxPhonemeClass phonClass;
	wxString talkback;
	wxString sampa;
	wxString ipa;
	wxString sampleWords;
	wxString classification;
};

// The phonetic alphabets
enum FxPhoneticAlphabet
{
	PA_DEFAULT,
	PA_SAMPA,
	PA_IPA
};

// Gets information about a phoneme
const FxPhonExtendedInfo& FxGetPhonemeInfo( const FxPhoneme& phon );
const FxPhonExtendedInfo& FxGetPhonemeInfo( FxSize phonIndex );

// Gets extended information about a phoneme class.
const FxPhonClassExtendedInfo& FxGetPhonemeClassInfo( const FxPhonemeClass& phonClass );
const FxPhonClassExtendedInfo& FxGetPhonemeClassInfo( FxSize phonClassIndex );

} // namespace Face

} // namespace OC3Ent

#endif
