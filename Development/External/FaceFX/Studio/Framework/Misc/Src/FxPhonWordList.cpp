//------------------------------------------------------------------------------
// Stores a list of phonemes and words, and their timings, for a given animation
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxPhonWordList.h"
#include "FxMath.h"
#include "FxConsole.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxPhonWordListVersion 3
FX_IMPLEMENT_CLASS(FxPhonWordList,kCurrentFxPhonWordListVersion,FxObject);

FxPhonWordList::FxPhonWordList()
{
	_startWord = FxInvalidIndex;
	_startPhon = FxInvalidIndex;
	_minPhonLen = 0.010f;
	_isDirty   = FxFalse;
}

FxPhonWordList::FxPhonWordList( const FxPhonWordList& other )
	: _phonemes( other._phonemes )
	, _words( other._words )
	, _startWord( FxInvalidIndex )
	, _startPhon( FxInvalidIndex )
	, _minPhonLen(0.010f)
	, _isDirty( other._isDirty )
{
}

FxPhonWordList& FxPhonWordList::operator=( const FxPhonWordList& other )
{
	_phonemes   = other._phonemes;
	_words      = other._words;
	_startWord  = FxInvalidIndex;
	_startPhon  = FxInvalidIndex;
	_minPhonLen = other._minPhonLen;
	_isDirty   = other._isDirty;
	return *this;
}

FxPhonWordList::~FxPhonWordList()
{
}

// Returns FxTrue if the phoneme list has changed since the last call to
// ClearDirtyFlag().
FxBool FxPhonWordList::IsDirty( void ) const
{
	return _isDirty;
}

// Clears the internal dirty flag.
void FxPhonWordList::ClearDirtyFlag( void )
{
	_isDirty = FxFalse;
}

// Returns the number of phonemes
FxSize FxPhonWordList::GetNumPhonemes() const
{
	return _phonemes.Length();
}

// Returns the phoneme enumeration
FxPhoneme FxPhonWordList::GetPhonemeEnum( FxSize phonIndex ) const
{
	return _phonemes[phonIndex].phoneme;
}

// Returns the phoneme start time
FxReal FxPhonWordList::GetPhonemeStartTime( FxSize phonIndex ) const
{
	return _phonemes[phonIndex].start;
}

// Returns the phoneme end time
FxReal FxPhonWordList::GetPhonemeEndTime( FxSize phonIndex ) const
{
	return _phonemes[phonIndex].end;
}

// Returns the phoneme duration
FxReal FxPhonWordList::GetPhonemeDuration( FxSize phonIndex ) const
{
	return GetPhonemeEndTime( phonIndex ) - GetPhonemeStartTime( phonIndex );
}

// Checks if a phoneme is selected
FxBool FxPhonWordList::GetPhonemeSelected( FxSize phonIndex ) const
{
	return _phonemes[phonIndex].selected;
}

// Returns true if the phoneme is contained in a word
FxBool FxPhonWordList::IsPhonemeInWord( FxSize phonIndex, FxSize* wordIndex )
{
	if( wordIndex )
	{
		*wordIndex = FxInvalidIndex;
	}
	for( FxSize i = 0; i < _words.Length(); ++i )
	{
		if( _words[i].first <= phonIndex && phonIndex <= _words[i].last )
		{
			if( wordIndex )
			{
				*wordIndex = i;
			}
			return FxTrue;
		}
	}
	return FxFalse;
}

// Selects a phoneme
void FxPhonWordList::SetPhonemeSelected( FxSize phonIndex, FxBool selected )
{
	_phonemes[phonIndex].selected = selected;
}

// Sets a phoneme type
void FxPhonWordList::SetPhonemeEnum( FxSize phonIndex, FxPhoneme phoneme )
{
	_phonemes[phonIndex].phoneme = phoneme;
	_isDirty = FxTrue;
}

// Removes all phonemes from the list
void FxPhonWordList::ClearPhonemes()
{
	// If there are no phonemes, there can be no words.
	_words.Clear();
	_phonemes.Clear();
	_isDirty = FxTrue;
}

// Removes a single phoneme from the list
void FxPhonWordList::RemovePhoneme( FxSize phonIndex )
{
	// Attempt to find the word containing the phoneme.
	FxSize word = FxInvalidIndex;
	for( FxSize i = 0; i < _words.Length(); ++i )
	{
		if( _words[i].first <= phonIndex && phonIndex <= _words[i].last )
		{
			word = i;
		}
	}

	// There are four basic cases of phoneme deletion
	if( word != FxInvalidIndex )
	{
		// Case 1) The phoneme was the last phoneme in a given word.  The word 
		// must also be removed from the word list.
		if( _words[word].first == _words[word].last )
		{
			_words.Remove( word );
		}
		// Case 2) The phoneme was the end phoneme of a given word.  The word's 
		// end phoneme must be moved to the preceding phoneme.
		else if( _words[word].last == phonIndex )
		{
			--_words[word].last;
		}
		// Case 3) The phoneme was the start phoneme of a given word.  The 
		// word's start phoneme must be moved to the succeeding phoneme.
		else if( _words[word].first == phonIndex )
		{
			++_words[word].first;
		}
		// Case 4) The phoneme was an interior phoneme in a word, or was not 
		// associated with a word.  
		// Nothing special needs to happen to the word list in this case.
	}

	// Next, we must decrement any index in the word list which refers to a
	// phoneme after phonIndex by 1.
	for( FxSize i = 0; i < _words.Length(); ++i )
	{
		if( _words[i].first >= phonIndex )
		{
			--_words[i].first;
		}
		if( _words[i].last >= phonIndex )
		{
			--_words[i].last;
		}
	}

	// Finally, we can remove the phoneme from the list.
	if( phonIndex == 0 )
	{
		if( phonIndex+1 < _phonemes.Length() )
		{
			_phonemes[phonIndex+1].start = 0.0f;
		}
	}
	else if( phonIndex == _phonemes.Length() - 1 )
	{
		if( phonIndex-1 >= 0 )
		{
			_phonemes[phonIndex-1].end = _phonemes[phonIndex].end;
		}
	}
	else
	{
		if( phonIndex+1 < _phonemes.Length() && phonIndex-1 >= 0 )
		{
			_phonemes[phonIndex-1].end   = _phonemes[phonIndex].start;
			_phonemes[phonIndex+1].start = _phonemes[phonIndex].start;
		}
	}
	_phonemes.Remove( phonIndex );
	_isDirty = FxTrue;
}

// Obliterates a phoneme from the list.
void FxPhonWordList::ObliteratePhoneme( FxSize phonIndex )
{
	FxReal duration = _phonemes[phonIndex].end - _phonemes[phonIndex].start;
	RemovePhoneme(phonIndex);
	if( _phonemes.Length() )
	{
		_phonemes[phonIndex].start += duration;
		for( FxSize i = phonIndex; i < _phonemes.Length(); ++i )
		{
			_phonemes[i].start -= duration;
			_phonemes[i].end   -= duration;
		}
	}
}

// Inserts a phoneme into the list
void FxPhonWordList::InsertPhoneme( FxPhoneme phoneme, FxSize index, InsertionMethod method )
{
	ClearSelection();
	// Add a minimum-duration phoneme to the end if we're inserting at the end of the list
	if( index >= _phonemes.Length() )
	{
		FxPhonInfo phonInfo(phoneme, 
			_phonemes[_phonemes.Length()-1].end, 
			_phonemes[_phonemes.Length()-1].end + _minPhonLen, 
			FxFalse);
		_phonemes.PushBack( phonInfo );
		return;
	}

	// Otherwise, perform the insertion - patch up the words
	for( FxSize i = 0; i < _words.Length(); ++i )
	{
		if( _words[i].first >= index ) ++_words[i].first;
		if( _words[i].last  >= index ) ++_words[i].last;
	}

	// Add the phoneme to the list
	FxPhonInfo newPhoneme;
	newPhoneme.phoneme = phoneme;

	newPhoneme.start = _phonemes[index].start;
	if( method == PUSH_OUT )
	{
		newPhoneme.end = newPhoneme.start + _minPhonLen;
		for( FxSize i = index; i < _phonemes.Length(); ++i )
		{
			_phonemes[i].start += _minPhonLen;
			_phonemes[i].end   += _minPhonLen;
		}
	}
	else if( method == SPLIT_TIME )
	{
		FxReal miTemps = (_phonemes[index].end + _phonemes[index].start) / 2;
		newPhoneme.end = miTemps;
		_phonemes[index].start = miTemps;
	}
	newPhoneme.selected	 = FxTrue;
	_phonemes.Insert( newPhoneme, index );
	_isDirty = FxTrue;
}

// Inserts a phoneme with given duration in the list
void FxPhonWordList::InsertPhoneme( FxPhoneme phoneme, FxReal duration, FxSize index )
{
	// If we're inserting the very first phoneme, put it on the front.
	if( !_phonemes.Length() )
	{
		FxPhonInfo phonInfo(phoneme, 0.0f, duration, FxTrue);
		_phonemes.PushBack( phonInfo );
		return;
	}

	// Add the phoneme to the end if we're inserting at the end of the list
	if( index >= _phonemes.Length() )
	{
		FxPhonInfo phonInfo(phoneme, 
			_phonemes[_phonemes.Length()-1].end, 
			_phonemes[_phonemes.Length()-1].end + duration, 
			FxTrue);
		_phonemes.PushBack( phonInfo );
		return;
	}

	// Otherwise, perform the insertion - patch up the words
	for( FxSize i = 0; i < _words.Length(); ++i )
	{
		if( _words[i].first >= index ) ++_words[i].first;
		if( _words[i].last  >= index ) ++_words[i].last;
	}

	// Add the phoneme to the list
	FxPhonInfo newPhoneme;
	newPhoneme.phoneme = phoneme;

	newPhoneme.start = _phonemes[index].start;
	newPhoneme.end = newPhoneme.start + duration;
	for( FxSize i = index; i < _phonemes.Length(); ++i )
	{
		_phonemes[i].start += duration;
		_phonemes[i].end   += duration;
	}
	newPhoneme.selected	 = FxTrue;

	_phonemes.Insert( newPhoneme, index );
	_isDirty = FxTrue;
}

// Adds a phoneme to the end of the list
void FxPhonWordList::AppendPhoneme( const FxPhonInfo& phoneme )
{
	_phonemes.PushBack( phoneme );
	_isDirty = FxTrue;
}

void FxPhonWordList::AppendPhoneme( FxPhoneme phoneme, FxReal startTime, FxReal endTime )
{
	FxPhonInfo temp(phoneme, startTime, endTime, FxFalse);
	AppendPhoneme( temp );
	_isDirty = FxTrue;
}

// Modifies a phoneme's start time.  Returns the actual start time.
FxReal FxPhonWordList::ModifyPhonemeStartTime( FxSize phonIndex, FxReal newStartTime )
{
	// Get a couple of pointers into the phoneme list
	FxPhonInfo* thisPhon = &_phonemes[phonIndex];
	FxPhonInfo* prevPhon = phonIndex > 0 ? &_phonemes[phonIndex-1] : NULL;

	if( newStartTime > ( thisPhon->end - _minPhonLen ) )
	{
		newStartTime = thisPhon->end - _minPhonLen;
	}
	if( prevPhon && newStartTime < ( prevPhon->start + _minPhonLen ) )
	{
		newStartTime = prevPhon->start + _minPhonLen;
	}

	thisPhon->start = newStartTime;
	if( prevPhon )
	{
		prevPhon->end   = newStartTime;
	}
	_isDirty = FxTrue;
	return newStartTime;
}

// Modifies a phoneme's end time.  Returns the actual end time.
FxReal FxPhonWordList::ModifyPhonemeEndTime( FxSize phonIndex, FxReal newEndTime )
{
	FxPhonInfo* thisPhon = &_phonemes[phonIndex];
	FxPhonInfo* nextPhon = NULL;
	if( phonIndex < _phonemes.Length()-1 )
	{
		nextPhon = &_phonemes[phonIndex+1];
	}
	if( newEndTime < thisPhon->start + _minPhonLen )
	{
		newEndTime = thisPhon->start + _minPhonLen;
	}
	if( nextPhon && newEndTime > nextPhon->end - _minPhonLen )
	{
		newEndTime = nextPhon->end - _minPhonLen;
	}

	thisPhon->end   = newEndTime;
	if( nextPhon )
	{
		nextPhon->start = newEndTime;
	}
	_isDirty = FxTrue;
	return newEndTime;
}

// Pushes a phoneme's start time, moving each preceding phoneme by a delta.
FxReal FxPhonWordList::PushPhonemeStartTime( FxSize phonIndex, 
											   FxReal newStartTime )
{
	if( GetPhonemeEndTime( phonIndex ) - newStartTime < _minPhonLen )
	{
		newStartTime = GetPhonemeEndTime( phonIndex ) - _minPhonLen;
	}
	FxReal delta = newStartTime - GetPhonemeStartTime( phonIndex );
	for( FxSize i = 0; i <= phonIndex; ++i )
	{
		_phonemes[i].start += delta;
		if( i != phonIndex )
		{
			_phonemes[i].end += delta;
		}
	}
	_isDirty = FxTrue;
	return GetPhonemeStartTime( phonIndex );
}

// Pushes a phoneme's end time, moving every succeeding phoneme by a delta.
FxReal FxPhonWordList::PushPhonemeEndTime( FxSize phonIndex, 
											 FxReal newEndTime )
{
	if( newEndTime - GetPhonemeStartTime( phonIndex ) < _minPhonLen )
	{
		newEndTime = GetPhonemeStartTime( phonIndex ) + _minPhonLen;
	}
	FxReal delta = newEndTime - GetPhonemeEndTime( phonIndex );
	for( FxSize i = phonIndex; i < GetNumPhonemes(); ++i )
	{
		if( i != phonIndex )
		{
			_phonemes[i].start += delta;
		}
		_phonemes[i].end += delta;
	}
	_isDirty = FxTrue;
	return GetPhonemeEndTime( phonIndex );
}

// Swaps two neighboring phonemes
FxSize FxPhonWordList::SwapPhonemes( FxSize primary, FxSize secondary )
{
	FxAssert( secondary == primary + 1 || secondary == primary - 1 );
	FxReal primaryDuration = GetPhonemeDuration(primary);
	FxReal secondaryDuration = GetPhonemeDuration(secondary);
	if( primary < secondary )
	{
		FxPhonInfo first = _phonemes[secondary];
		first.start = _phonemes[primary].start;
		first.end   = first.start + secondaryDuration;

		FxPhonInfo second = _phonemes[primary];
		second.start = first.end;
		second.end   = _phonemes[secondary].end;

		_phonemes[primary] = first;
		_phonemes[secondary]   = second;
	}
	else if( primary > secondary )
	{
		FxPhonInfo first = _phonemes[primary];
		first.start = _phonemes[secondary].start;
		first.end   = first.start + primaryDuration;

		FxPhonInfo second = _phonemes[secondary];
		second.start = first.end;
		second.end   = _phonemes[primary].end;

		_phonemes[secondary] = first;
		_phonemes[primary]   = second;
	}
	_isDirty = FxTrue;
	return secondary;
}

// Swaps two phoneme ranges, updating words.  They must be non-overlapping.
FxSize FxPhonWordList::SwapPhoneRanges( FxSize primaryStart, FxSize primaryEnd, 
						FxSize secondaryStart, FxSize secondaryEnd )
{
	FxSize leftStart  = FxMin( primaryStart, secondaryStart );
	FxSize leftEnd    = FxMin( primaryEnd, secondaryEnd );
	FxSize leftWIndex = FxInvalidIndex;
	FxSize rightStart = FxMax( primaryStart, secondaryStart );
	FxSize rightEnd   = FxMax( primaryEnd, secondaryEnd );
	FxSize rightWIndex= FxInvalidIndex;

	for( FxSize i = 0; i < _words.Length(); ++i )
	{
		if( _words[i].first == leftStart && 
			_words[i].last  == leftEnd )
		{
			leftWIndex = i;
		}
		if( _words[i].first == rightStart && 
			_words[i].last  == rightEnd )
		{
			rightWIndex = i;
		}
	}

	 FxSize numSwaps = leftEnd - leftStart + 1;
	 for( FxSize i = rightStart; i <= rightEnd; ++i )
	 {
		 FxSize current = i;
		 for( FxSize num = 0; num < numSwaps; ++num )
		 {
			current = SwapPhonemes( current, current-1 );
		 }
	 }

	 // If both words were valid, need to swap their data
	 if( leftWIndex != FxInvalidIndex && rightWIndex != FxInvalidIndex )
	 {
		 FxWordInfo tempLeft;
		 FxWordInfo tempRight;

		 tempLeft.text   = _words[rightWIndex].text;
		 tempLeft.first  = _words[leftWIndex].first;
		 tempLeft.last   = tempLeft.first + _words[rightWIndex].last - _words[rightWIndex].first;

		 tempRight.text  = _words[leftWIndex].text;
		 tempRight.first = tempLeft.last + 1;
		 tempRight.last  = tempRight.first + _words[leftWIndex].last - _words[leftWIndex].first;

		 _words[leftWIndex]  = tempLeft;
		 _words[rightWIndex] = tempRight;

		 return GetWordSelected(leftWIndex) ? leftWIndex : rightWIndex;
	 }
	 else if( leftWIndex == FxInvalidIndex && rightWIndex != FxInvalidIndex )
	 {
		 FxSize wordLen = _words[rightWIndex].last - _words[rightWIndex].first;
		_words[rightWIndex].first = leftStart;
		_words[rightWIndex].last  = _words[rightWIndex].first + wordLen;
	 }
	 else if( leftWIndex != FxInvalidIndex && rightWIndex == FxInvalidIndex )
	 {
		 FxSize wordLen = _words[leftWIndex].last - _words[leftWIndex].first;
		 _words[leftWIndex].first = leftStart + 1;
		 _words[leftWIndex].last  = _words[leftWIndex].first + wordLen;
	 }

	 _isDirty = FxTrue;
	 return FxInvalidIndex;
}

// Returns the index of the phoneme at a time.
FxSize FxPhonWordList::FindPhoneme( FxReal time )
{
	for( FxSize i = 0; i < _phonemes.Length(); ++i )
	{
		if( _phonemes[i].start <= time && time < _phonemes[i].end )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

// Returns the number of words
FxSize FxPhonWordList::GetNumWords() const
{
	return _words.Length();
}

// Returns the word text for a given word
wxString FxPhonWordList::GetWordText( FxSize wordIndex ) const
{
	return _words[wordIndex].text;
}

// Returns the start time for a given word
FxReal FxPhonWordList::GetWordStartTime( FxSize wordIndex ) const
{
	return GetPhonemeStartTime( _words[wordIndex].first );
}

// Returns the end time for a given word
FxReal FxPhonWordList::GetWordEndTime( FxSize wordIndex ) const
{
	return GetPhonemeEndTime( _words[wordIndex].last );
}

// Returns the duration for a given word
FxReal FxPhonWordList::GetWordDuration( FxSize wordIndex ) const
{
	return GetWordEndTime( wordIndex ) - GetWordStartTime( wordIndex );
}

// Returns if a word is selected.  A word is considered selected if every
// phoneme in the word is selected.
FxBool FxPhonWordList::GetWordSelected( FxSize wordIndex ) const
{
	FxBool selected = FxTrue;
	for( FxSize phon = _words[wordIndex].first;
		 phon <= _words[wordIndex].last; ++ phon )
	{
		selected &= GetPhonemeSelected( phon );
	}
	return selected;
}

// Returns the index of the first phoneme in a word
FxSize FxPhonWordList::GetWordStartPhoneme( FxSize wordIndex ) const
{
	return _words[wordIndex].first;
}

// Returns the index of the last phoneme in a word
FxSize FxPhonWordList::GetWordEndPhoneme( FxSize wordIndex ) const
{
	return _words[wordIndex].last;
}

// Sets a word selected
void FxPhonWordList::SetWordSelected( FxSize wordIndex, FxBool selected )
{
	for( FxSize phon = _words[wordIndex].first;
		phon <= _words[wordIndex].last; ++ phon )
	{
		SetPhonemeSelected( phon, selected );
	}
}

// Removes all words from the list
void FxPhonWordList::ClearWords()
{
	_words.Clear();
}

// Groups a set of phonemes into a word
void FxPhonWordList::GroupToWord( const wxString& text, FxSize start, FxSize end )
{
	Ungroup( start, end );
	
	FxWordInfo temp(text, start, end);
	FxSize place = FxInvalidIndex;

	for( FxSize i = 0; i < _words.Length(); ++i )
	{
		if( _words[i].first > end )
		{
			break;
		}
		place = i + 1;
	}

	if( place == FxInvalidIndex )
	{
		place = 0;
	}

	_words.Insert( temp, place );
}

// Groups phonemes in the specified time range to the word.
void FxPhonWordList::
GroupToWord( const wxString& text, FxReal wordStartTime, FxReal wordEndTime )
{
	FxSize firstPhonemeIndex  = FxInvalidIndex;
	FxSize secondPhonemeIndex = FxInvalidIndex;

	for( FxSize i = 0; i < _phonemes.Length(); ++i )
	{
		if( FxRealEquality(wordStartTime, _phonemes[i].start) )
		{
			firstPhonemeIndex = i;
		}
		if( FxRealEquality(wordEndTime, _phonemes[i].end) )
		{
			secondPhonemeIndex = i;
		}
	}

	if( FxInvalidIndex != firstPhonemeIndex &&
		FxInvalidIndex != secondPhonemeIndex )
	{
		Ungroup(firstPhonemeIndex, secondPhonemeIndex);

		FxWordInfo temp(text, firstPhonemeIndex, secondPhonemeIndex);
		FxSize place = FxInvalidIndex;

		for( FxSize i = 0; i < _words.Length(); ++i )
		{
			if( _words[i].first > secondPhonemeIndex )
			{
				break;
			}
			place = i + 1;
		}

		if( place == FxInvalidIndex )
		{
			place = 0;
		}

		_words.Insert( temp, place );
	}
}

// Ungroups a range of phonemes
void FxPhonWordList::Ungroup( FxSize start, FxSize end )
{
	// Find any words that were completely contained by the range and remove
	for( FxSize i = 0; i < _words.Length(); ++i )
	{
		if( _words[i].first >= start && _words[i].last <= end )
		{
			_words.Remove( i );
			--i;
		}
		else if( _words[i].first >= start && _words[i].first <= end && _words[i].last > end )
		{
			_words[i].first = end + 1;
		}
		else if ( _words[i].first < start && _words[i].last >= start && _words[i].last <= end )
		{
			_words[i].last = start - 1;
		}
	}
}

// Modifies a word's start time
FxReal FxPhonWordList::ModifyWordStartTime( FxSize wordIndex, 
											  FxReal newStartTime, 
											  FxBool dontChangePreceding )
{
	// Cache the phonetic proportions
	FxReal startTime = GetPhonemeStartTime( _words[wordIndex].first );
	FxReal endTime   = GetPhonemeEndTime( _words[wordIndex].last );
	FxReal wordDuration  = endTime - startTime;
	FxBool allAtMinimum = FxTrue;

	FxArray<FxReal> proportion;
	for( FxSize phone = _words[wordIndex].first; 
		phone <= _words[wordIndex].last; ++phone )
	{
		FxReal phoneDuration = GetPhonemeDuration( phone );
		allAtMinimum &= phoneDuration <= _minPhonLen + FX_REAL_EPSILON;
		proportion.PushBack( phoneDuration / wordDuration );
	}

	if( allAtMinimum && newStartTime > startTime )
	{
		return startTime;
	}

	FxReal minimumWordDuration = endTime - newStartTime;

	// Figure out if what precedes the word is a single phoneme or a word.
	if( wordIndex != 0 && _words[wordIndex].first - 1 == _words[wordIndex-1].last )
	{
		// We're preceded by a word
		if( !dontChangePreceding && newStartTime > startTime)
		{
			// Validate the move with the predecessor
			newStartTime = ModifyWordEndTime( wordIndex - 1, newStartTime, FxTrue );
		}
	}
	else // We're preceded by a phoneme or nothing
	{
		// Validate the move with the predecessor if we might be intruding into
		// it's minimum space.
		if( _words[wordIndex].first != 0 && newStartTime < startTime )
		{
			newStartTime = ModifyPhonemeEndTime( _words[wordIndex].first - 1, newStartTime );
		}
	}
	minimumWordDuration = endTime - newStartTime;

	// Now move the phonemes
	for( FxInt32 phone = _words[wordIndex].last; 
		phone >= static_cast<FxInt32>(_words[wordIndex].first); --phone )
	{
		FxSize proportionIdx = phone - _words[wordIndex].first;
		FxReal phoneStartTime = GetPhonemeEndTime( phone ) - 
			minimumWordDuration * proportion[proportionIdx];
		ModifyPhonemeStartTime( phone, phoneStartTime );
	}

	return newStartTime;
}

// Modifies a word's end time
FxReal FxPhonWordList::ModifyWordEndTime( FxSize wordIndex, 
											FxReal newEndTime, 
											FxBool dontChangeSuceeding )
{
	// Cache the phonetic proportions
	FxReal startTime = GetPhonemeStartTime( _words[wordIndex].first );
	FxReal endTime   = GetPhonemeEndTime( _words[wordIndex].last );
	FxReal wordDuration  = endTime - startTime;
	FxBool allAtMinimum = FxTrue;

	FxArray<FxReal> proportion;
	proportion.Reserve( _words[wordIndex].last - _words[wordIndex].last );
	for( FxSize phone = _words[wordIndex].first; 
		phone <= _words[wordIndex].last; ++phone )
	{
		FxReal phoneDuration = GetPhonemeDuration( phone );
		allAtMinimum &= phoneDuration <= _minPhonLen + FX_REAL_EPSILON;
		proportion.PushBack( phoneDuration / wordDuration );
	}

	if( allAtMinimum && newEndTime < endTime )
	{
		return endTime;
	}

	FxReal minimumWordDuration = newEndTime - startTime;

	// Figure out if what follows the word is a single phoneme or a word.
	if( wordIndex < _words.Length() - 1 && 
		_words[wordIndex].last + 1 == _words[wordIndex+1].first )
	{
		// We're succeeded by a word
		if( !dontChangeSuceeding )
		{
			// Validate the move with the successor
			newEndTime = ModifyWordStartTime( wordIndex + 1, newEndTime, FxTrue );
		}
	}
	else // We're succeeded by a phoneme or nothing
	{
		if( _words[wordIndex].last != _phonemes.Length()-1 && newEndTime > endTime )
		{
			// Validate the move with the successor if we might be intruding into
			// it's minimum space.
			newEndTime = ModifyPhonemeStartTime( _words[wordIndex].last + 1, newEndTime );
		}
	}
	minimumWordDuration = newEndTime - startTime;


	// Now move the phonemes
	for( FxSize phone = _words[wordIndex].first; 
		phone <= _words[wordIndex].last; ++phone )
	{
		FxSize proportionIdx = phone - _words[wordIndex].first;
		FxReal phoneEndTime = GetPhonemeStartTime( phone ) + 
			minimumWordDuration * proportion[proportionIdx];
		ModifyPhonemeEndTime( phone, phoneEndTime );
	}

	return newEndTime;
}

// Clears both the phoneme and word lists
void FxPhonWordList::Clear()
{
	ClearWords();
	ClearPhonemes();
	_isDirty = FxTrue;
}

void FxPhonWordList::StartSelection( FxSize wordIndex, FxSize phonemeIndex, FxBool selected )
{
	_startWord = wordIndex;
	_startPhon = phonemeIndex;

	if( _startWord != FxInvalidIndex )
	{
		SetWordSelected( wordIndex, selected );
	}
	else if( _startPhon != FxInvalidIndex )
	{
		SetPhonemeSelected( phonemeIndex, selected );
	}
}

void FxPhonWordList::EndSelection( FxSize wordIndex, FxSize phonemeIndex )
{
	FxSize minIndex = FxInvalidIndex;
	FxSize maxIndex = FxInvalidIndex;

	// If we started on a word and ended on a phoneme
	if( _startWord != FxInvalidIndex && phonemeIndex != FxInvalidIndex )
	{
		// If the phoneme was before the beginning of the word
		if( phonemeIndex < _words[_startWord].first )
		{
			minIndex = phonemeIndex;
			maxIndex = _words[_startWord].last;
		}
		// if the phoneme was after the end of the word
		else if( phonemeIndex > _words[_startWord].last )
		{
			minIndex = _words[_startWord].first;
			maxIndex = phonemeIndex;
		}
		else // the phoneme was inside the word
		{
			minIndex = _words[_startWord].first;
			maxIndex = _words[_startWord].last;
		}
	}
	// If we started on a phoneme and ended on a word
	else if( _startPhon != FxInvalidIndex && wordIndex != FxInvalidIndex )
	{
		// If the phoneme was before the beginning of the word
		if( _startPhon < _words[wordIndex].first )
		{
			minIndex = _startPhon;
			maxIndex = _words[wordIndex].last;
		}
		// If the phoneme was after the end of the word
		else if( _startPhon > _words[wordIndex].last )
		{
			minIndex = _words[wordIndex].first;
			maxIndex = _startPhon;
		}
		else // The phoneme was inside the word
		{
			minIndex = _words[wordIndex].first;
			maxIndex = _words[wordIndex].last;
		}
	}
	// If we started and ended on a word
	else if( _startWord != FxInvalidIndex && wordIndex != FxInvalidIndex )
	{
		// If the first word was before the second
		if( _startWord < wordIndex )
		{
			minIndex = _words[_startWord].first;
			maxIndex = _words[wordIndex].last;
		}
		// If the first word was after the second
		else if( _startWord > wordIndex )
		{
			minIndex = _words[wordIndex].first;
			maxIndex = _words[_startWord].last;
		}
		else // They were the same word
		{
			minIndex = _words[wordIndex].first;
			maxIndex = _words[wordIndex].last;
		}
	}
	else // We started and ended on phonemes - the easy case.
	{
		minIndex = FxMin( _startPhon, phonemeIndex );
		maxIndex = FxMax( _startPhon, phonemeIndex );
	}

	// Now that we've broken the selection down into simple phonemes, select
	// the phonemes from the minimum index to the maximum index.
	if( minIndex != FxInvalidIndex && maxIndex != FxInvalidIndex )
	{
		ClearSelection();
		for( FxSize phon = minIndex; phon <= maxIndex; ++phon )
		{
			SetPhonemeSelected( phon, FxTrue );
		}
	}
	else
	{
		_startPhon = FxInvalidIndex;
		_startWord = FxInvalidIndex;
	}
}

// Clears current selection
void FxPhonWordList::ClearSelection()
{
	for( FxSize i = 0; i < _phonemes.Length(); ++i )
	{
		SetPhonemeSelected( i, FxFalse );
	}
}

// Removes all the phonemes in the current selection
void FxPhonWordList::RemoveSelection()
{
	for( FxSize i = 0; i < _phonemes.Length(); ++i )
	{
		if( GetPhonemeSelected( i ) )
		{
			RemovePhoneme( i );
			--i;
		}
	}
}

// Obliterates the selection (collapses remaining phonemes <- that way).
void FxPhonWordList::ObliterateSelection()
{
	for( FxSize i = 0; i < _phonemes.Length(); ++i )
	{
		if( GetPhonemeSelected( i ) )
		{
			ObliteratePhoneme( i );
			--i;
		}
	}
}

// Returns true if there is any selection
FxBool FxPhonWordList::HasSelection() const
{
	for( FxSize i = 0; i < _phonemes.Length(); ++i )
	{
		if( GetPhonemeSelected( i ) )
		{
			return FxTrue;
		}
	}
	return FxFalse;
}

void FxPhonWordList::ChangeSelection( FxPhoneme phonEnum )
{
	// Determine the length of the selection.
	FxSize firstPhon = FxInvalidIndex;
	FxSize lastPhon = 0;
	for( FxSize i = 0; i < _phonemes.Length(); ++i )
	{
		if( GetPhonemeSelected(i) )
		{
			firstPhon = FxMin(firstPhon, i);
			lastPhon  = FxMax(lastPhon,  i);
		}
	}

	// If all the phonemes are contained in a single word, preserve that word's
	// grouping.
	FxSize numWords  = GetNumWords();
	FxReal startTime = FxInvalidValue;
	FxReal endTime   = FxInvalidValue;
	wxString text;
	for( FxSize i = 0; i < numWords; ++i )
	{
		if( GetWordStartPhoneme(i) <= firstPhon && GetWordEndPhoneme(i) >= lastPhon )
		{
			startTime = GetWordStartTime(i);
			endTime   = GetWordEndTime(i);
			text      = GetWordText(i);
		}
	}

	if( firstPhon != FxInvalidIndex )
	{
		FxReal selStartTime     = GetPhonemeStartTime(firstPhon);
		FxReal selEndTime       = GetPhonemeEndTime(lastPhon);
		FxReal nextPhonDuration = 0.f;
		FxBool removingLast     = lastPhon == _phonemes.Length() - 1;

		if( !removingLast )
		{
			nextPhonDuration = GetPhonemeDuration(lastPhon + 1);
		}

		RemoveSelection();
		if( !removingLast )
		{
			PushPhonemeEndTime(firstPhon, GetPhonemeStartTime(firstPhon) + nextPhonDuration);
		}
		else if( removingLast && firstPhon != 0 )
		{
			PushPhonemeEndTime(firstPhon-1, selStartTime);
		}
		InsertPhoneme(phonEnum, selEndTime-selStartTime, firstPhon);
	}

	if( startTime != FxInvalidValue && endTime != FxInvalidValue )
	{
		GroupToWord(text, startTime, endTime);
	}
}

// Converts the list to a generic format.
void FxPhonWordList::ToGenericFormat( FxPhonemeList& genericPhonList )
{
	genericPhonList.Clear();
	genericPhonList.Reserve(_phonemes.Length());
	for( FxSize i = 0; i < _phonemes.Length(); ++i )
	{
		genericPhonList.AppendPhoneme(_phonemes[i].phoneme, _phonemes[i].start, _phonemes[i].end);
	}
}

// Splice in a phoneme list.
void FxPhonWordList::Splice( const FxPhonWordList& other, FxSize startIndex, FxSize endIndex )
{
	// Detect the case of appending an entire list to the current one.
    if( FxInvalidIndex == startIndex &&
		FxInvalidIndex == endIndex )
	{
		FxReal offset = 0.0f;
		FxSize wordOffset = _phonemes.Length();
		if( wordOffset )
		{
			offset = _phonemes.Back().end;
		}
		for( FxSize i = 0; i < other._phonemes.Length(); ++i )
		{
			FxPhonInfo tempPhon = other._phonemes[i];
			tempPhon.start += offset;
			tempPhon.end   += offset;
			_phonemes.PushBack(tempPhon);
		}
		for( FxSize i = 0; i < other._words.Length(); ++i )
		{
			FxWordInfo tempWordInfo = other._words[i];
			tempWordInfo.first += wordOffset;
			tempWordInfo.last  += wordOffset;
			_words.PushBack(tempWordInfo);
		}
		return;
	}

	if( FxInvalidIndex == endIndex )
	{
		endIndex = _phonemes.Length() - 1;
	}
	FxAssert(other.GetNumPhonemes());
	if( other.GetNumPhonemes() )
	{
		FxAssert(endIndex > startIndex && endIndex < _phonemes.Length());
		FxArray<FxPhonInfo> tempPhonemes;
		FxArray<FxWordInfo> tempWords;

		for( FxSize i = 0; i < startIndex; ++i )
		{
			tempPhonemes.PushBack(_phonemes[i]);
		}
		FxReal offset = tempPhonemes.Length() ? tempPhonemes.Back().end : 0.0f;
		FxPhonInfo tempPhonInfo = other._phonemes[0];
		tempPhonInfo.start = offset;
		tempPhonInfo.end += offset;
		tempPhonemes.PushBack(tempPhonInfo);
		for( FxSize i = 1; i < other._phonemes.Length(); ++i )
		{
			FxPhonInfo phonInfo = other._phonemes[i];
			phonInfo.start += offset;
			phonInfo.end += offset;
			tempPhonemes.PushBack(phonInfo);
		}
		if( endIndex < _phonemes.Length() - 1 )
		{
			FxPhonInfo tempPhonInfo = _phonemes[endIndex];
			tempPhonInfo.start = tempPhonemes.Back().end;
			tempPhonemes.PushBack(tempPhonInfo);
		}
		for( FxSize i = endIndex+1; i < _phonemes.Length(); ++i )
		{
			tempPhonemes.PushBack(_phonemes[i]);
		}

		// Deal with the words.
		for( FxSize i = 0; i < _words.Length(); ++i )
		{
			if( _words[i].last < startIndex )
			{
				tempWords.PushBack(_words[i]);
			}
		}
		for( FxSize i = 0; i < other._words.Length(); ++i )
		{
			FxWordInfo tempWordInfo = other._words[i];
			tempWordInfo.first += startIndex;
			tempWordInfo.last  += startIndex;
			tempWords.PushBack(tempWordInfo);
		}
		for( FxSize i = 0; i < _words.Length(); ++i )
		{
			if( _words[i].first >= endIndex )
			{
				FxWordInfo tempWordInfo = _words[i];
				tempWordInfo.first = tempWordInfo.first - (endIndex - startIndex) + other._phonemes.Length();
				tempWordInfo.last = tempWordInfo.last - (endIndex - startIndex) + other._phonemes.Length();
				tempWords.PushBack(tempWordInfo);
			}
		}

		_phonemes = tempPhonemes;
		_words = tempWords;
	}
}

FxArchive& operator<<( FxArchive& arc, FxPhonInfo& phonInfo )
{
	FxInt32 tempPhonEnum = static_cast<FxInt32>(phonInfo.phoneme);
	arc << tempPhonEnum << phonInfo.start << phonInfo.end;
	phonInfo.phoneme = static_cast<FxPhoneme>(tempPhonEnum);
	if( arc.IsLoading() )
	{
		phonInfo.selected = FxFalse;
	}
	return arc;
}

FxArchive& operator<<( FxArchive& arc, FxWordInfo& wordInfo )
{
	FxWString tempword(wordInfo.text.GetData());
	arc << tempword << wordInfo.first << wordInfo.last;
	wordInfo.text = wxString(tempword.GetData());
	return arc;
}

// Serialize the object
void FxPhonWordList::Serialize( FxArchive& arc )
{
	if( arc.IsSaving() )
	{
		Proof();
	}

	Super::Serialize( arc );

	FxUInt16 version = arc.SerializeClassVersion("FxPhonWordList");
	
	arc << _phonemes;
	if( arc.IsSaving() || (arc.IsLoading() && version >= 2) )
	{
		// Run the normal serialization operator
		arc	<< _words;
	}
	else
	{
		// We're loading an old version of the phoneme/word list that stored
		// the words in ASCII format.  Since FxWordInfo isn't itself versioned, 
		// this is a small hack to manually load the array and run the old 
		// version's serialization operator.
		FxUInt16 arrayVer;
		FxSize arrayLen;
		arc << arrayVer << arrayLen;
		_words.Clear();
		_words.Reserve(arrayLen);
		for( FxSize i = 0; i < arrayLen; ++i )
		{
			FxWordInfo wordInfo;
			FxString tempword(wordInfo.text.mb_str(wxConvLibc));
			arc << tempword << wordInfo.first << wordInfo.last;
			wordInfo.text = wxString::FromAscii(tempword.GetData());
			_words.PushBack(wordInfo);
		}
	}

	if( version >= 1 )
	{
		arc << _minPhonLen;
	}
	else
	{
		_minPhonLen = 0.016f;
	}

	if( arc.IsLoading() )
	{
		// If we're loading old versions of the phoneme word list, update the phoneme enums.
		if( version < 3 )
		{
			for( FxSize i = 0; i < _phonemes.Length(); ++i )
			{
				_phonemes[i].phoneme = FxPhonemeUtility::UpdatePhoneme(_phonemes[i].phoneme);
			}
		}
		Proof();
	}
}

FxBool FxPhonWordList::Proof()
{
	FxSize numOffenders = 0;
	for( FxSize i = 0; i < _words.Length(); ++i )
	{
		// Remove any words that are hanging off the end of the phoneme array.
		if( _words[i].first >= _phonemes.Length() ||
			_words[i].last >= _phonemes.Length() )
		{
			FxWarn("Corruption detected in phoneme list: word %s (index %d) refers to phonemes off the end of the list. The offending word has been removed.", _words[i].text.mb_str(wxConvLibc), i + numOffenders);
			_words.Remove(i);
			--i;
			++numOffenders;
		}

		// Remove any words that overlap other words.
		if( (i > 0 && _words[i].first <= _words[i-1].last) ||
			(i < _words.Length() - 1 && _words[i].last >= _words[i+1].first) )
		{
			FxWarn("Corruption detected in phoneme list: word %s (index %d) overlaps an adjacent phoneme. The offending word has been removed.", _words[i].text.mb_str(wxConvLibc), i + numOffenders);
			_words.Remove(i);
			--i;
			++numOffenders;
		}
	}

	return FxTrue;
}

} // namespace Face

} // namespace OC3Ent
