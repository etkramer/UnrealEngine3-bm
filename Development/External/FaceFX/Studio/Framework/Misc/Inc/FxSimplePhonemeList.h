//------------------------------------------------------------------------------
// A very simple phoneme list used for exporting phonemes to external libraries.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2006 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxSimplePhonemeList_H__
#define FxSimplePhonemeList_H__

#include "FxArray.h"
#include "FxPhonemeEnum.h"

namespace OC3Ent
{

namespace Face
{

// A simple phoneme list.
class FxPhonemeList
{
public:
	FxPhonemeList()
		: _lastPhonEndTime(0.0f)
	{
	}
	~FxPhonemeList() {}

	// Clears the list.
	FX_INLINE void Clear()
	{
		_phonEnum.Clear();
		_phonStartTime.Clear();
	}

	// Reserves room for n phonemes
	FX_INLINE void Reserve( FxSize n )
	{
		_phonEnum.Reserve(n);
		_phonStartTime.Reserve(n);
	}

	// Appends a phoneme.
	FX_INLINE void AppendPhoneme(FxPhoneme phonEnum, FxReal phonStart, FxReal phonEnd)
	{
		_phonEnum.PushBack(phonEnum);
		_phonStartTime.PushBack(phonStart);
		_lastPhonEndTime = phonEnd;
	}

	// Gets the number of phonemes in the list.
	FX_INLINE FxSize GetNumPhonemes() const { return _phonEnum.Length(); }
	// Gets the enumeration of a phoneme.
	FX_INLINE FxPhoneme GetPhonemeEnum( FxSize index ) const { return _phonEnum[index]; }
	// Gets the start time of a phoneme.
	FX_INLINE FxReal GetPhonemeStartTime( FxSize index ) const { return _phonStartTime[index]; }
	// Gets the end time of a phoneme.
	FX_INLINE FxReal GetPhonemeEndTime( FxSize index ) const
	{
		if( index < _phonStartTime.Length() - 1 )
		{
			return _phonStartTime[index+1];
		}
		return _lastPhonEndTime;
	}
	// Gets the duration of a phoneme.
	FX_INLINE FxReal GetPhonemeDuration( FxSize index ) const
	{ 
		if( index < _phonStartTime.Length() - 1 )
		{
			return _phonStartTime[index+1] - _phonStartTime[index];
		}
		return _lastPhonEndTime - _phonStartTime[index];
	}

private:
	FxArray<FxPhoneme> _phonEnum;
	FxArray<FxReal>    _phonStartTime;
	FxReal			   _lastPhonEndTime;
};

} // namespace Face

} // namespace OC3Ent

#endif

