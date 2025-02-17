//------------------------------------------------------------------------------
// A fixed-size templated array for buffering data.
//
// Owner: John Briggs
// 
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFixedBuffer_H__
#define FxFixedBuffer_H__

#include "FxPlatform.h"
//@todo temp
#include "FxString.h"

namespace OC3Ent
{

namespace Face
{

template<typename Elem, FxSize Size> class FxFixedBuffer
{
public:
	FxFixedBuffer()
		: _firstElem(FxInvalidIndex)
		, _lastElem(FxInvalidIndex)
		, _numUsed(0)
	{
	}

	void Clear()
	{
		_firstElem = FxInvalidIndex;
		_lastElem = FxInvalidIndex;
		_numUsed = 0;
	}

	FxSize Length()
	{
		return _numUsed;
	}

	FxBool PushBack( const Elem& item )
	{
		FxBool retval = FxFalse;
		if( _numUsed == 0 )
		{
			_firstElem = _lastElem = 0;
			++_numUsed;
		}
		else
		{
			if( _lastElem == Size - 1 )
			{
				_lastElem = 0;
			}
			else
			{
				++_lastElem;
			}
			if( _lastElem == _firstElem )
			{
				if( _firstElem == Size - 1 )
				{
					_firstElem = 0;
				}
				else
				{
					++_firstElem;
				}
				retval = FxTrue; // Flag that an item was overwritten.
			}
			_numUsed = FxMin(Size, _numUsed+1);
		}
		_buffer[_lastElem] = item;
		return retval;
	}

	Elem& operator[](FxSize index)
	{
		return _buffer[index];
	}

	const Elem& operator[](FxSize index) const
	{
		return _buffer[index];
	}

	FxSize Begin()
	{
		return _firstElem;
	}

	FX_INLINE FxSize Next(FxSize curr)
	{
		if( curr == _lastElem )
		{
			return FxInvalidIndex;
		}

		++curr;
		if( curr == _numUsed )
		{
			curr = 0;
		}
		return curr;
	}

	FX_INLINE FxSize Prev(FxSize curr)
	{
		if( curr == _firstElem )
		{
			return FxInvalidIndex;
		}

		--curr;
		if( curr == FxInvalidIndex )
		{
			curr = _numUsed - 1;
		}
		return curr;
	}

	Elem& Back()
	{
		return _buffer[_lastElem];
	}

protected:
	Elem	_buffer[Size];
	FxSize	_firstElem;
	FxSize	_lastElem;
	FxSize	_numUsed;
};

} // namespace Face

} // namespace OC3Ent

#endif
