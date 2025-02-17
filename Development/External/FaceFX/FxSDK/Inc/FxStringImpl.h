//------------------------------------------------------------------------------
// A templated string for both ansi and wide character support.
// Do not include this file.  Include FxString.h instead.
//
// Owner: John Briggs
// 
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxArrayBase.h"
#include "FxMemory.h"

namespace OC3Ent
{

namespace Face
{

// Constructs a string with room for numReserved characters.
template<typename T>
FxStringBase<T>::FxStringBase(FxSize numReserved)
	: _container(numReserved == 0 ? 0 : numReserved+1)
{
}

// Constructs a string from a native character format string.
template<typename T>
FxStringBase<T>::FxStringBase(const char_type* str, FxSize numChars )
	: _container(0)
{
	if( str )
	{
		FxSize length = numChars == FxInvalidIndex ? traits_type::Length(str) : numChars;
		if( length )
		{
			_container_type temp(length+1);
			traits_type::Copy(temp._v, str, length);
			temp._usedCount = length;
			_container.Swap(temp);
			_container._v[_container._usedCount] = static_cast<char_type>(0);
		}
	}
}

// Constructs a string from a native character.
template<typename T>
FxStringBase<T>::FxStringBase(const char_type& c, FxSize numChars )
{
	_container_type temp(numChars+1);
	while( temp._usedCount < numChars )
	{
		traits_type::Copy(temp._v + temp._usedCount, &c, 1);
		temp._usedCount++;
	}
	_container.Swap(temp);
	_container._v[_container._usedCount] = static_cast<char_type>(0);
}

// Constructs a string from a foreign string.
template<typename T>
FxStringBase<T>::FxStringBase(const foreign_char_type* str)
	: _container(0)
{
	if( str )
	{
		FxSize length = traits_type::other_traits_type::Length(str);
		if( length )
		{
			_container_type temp(length+1);
			traits_type::Convert(temp._v, str, length);
			temp._usedCount = length;
			_container.Swap(temp);
			_container._v[_container._usedCount] = static_cast<char_type>(0);
		}
	}
}

// Copy constructor.
template<typename T>
FxStringBase<T>::FxStringBase(const FxStringBase& other)
	: _container( other._container._usedCount==0 ? 0 : other._container._usedCount + 1)
{
	traits_type::Copy(_container._v, other._container._v, other._container._usedCount);
	_container._usedCount = other._container._usedCount;
	if( _container._usedCount )
	{
		_container._v[_container._usedCount] = static_cast<char_type>(0);
	}
}

// Assignment operator.
template<typename T>
FxStringBase<T>& FxStringBase<T>::operator=(const FxStringBase& other)
{
	FxStringBase temp(other);
	_container.Swap(temp._container);
	return *this;
}

// Assignment from native string.
template<typename T>
FxStringBase<T>& FxStringBase<T>::operator=(const char_type* str)
{
	FxStringBase temp(str);
	_container.Swap(temp._container);
	return *this;
}

// Clears the string.
template<typename T>
void FxStringBase<T>::Clear()
{
	_container._usedCount = 0;
}

// Returns the string length.
template<typename T> FX_INLINE
FxSize FxStringBase<T>::Length() const
{
	return _container._usedCount;
}

// Returns the amount of memory allocated by the string.
template<typename T> FX_INLINE
FxSize FxStringBase<T>::Allocated() const
{
	return _container._allocatedCount;
}

// Returns true if the string is empty.
template<typename T> FX_INLINE
FxBool FxStringBase<T>::IsEmpty() const
{
	return _container._usedCount == 0;
}

// Character access.
template<typename T> FX_INLINE
typename FxStringBase<T>::char_type& FxStringBase<T>::operator[](FxSize index)
{
	FxAssert(index < _container._usedCount);
	return _container._v[index];
}

// Const character access.
template<typename T> FX_INLINE
const typename FxStringBase<T>::char_type& FxStringBase<T>::operator[](FxSize index) const
{
	FxAssert(index < _container._usedCount);
	return _container._v[index];
}

// Character retrieval.
template<typename T> FX_INLINE
typename FxStringBase<T>::char_type FxStringBase<T>::GetChar(FxSize index) const
{
	FxAssert(index < _container._usedCount);
	return _container._v[index];
}

// Appends a string.
template<typename T>
FxStringBase<T>& FxStringBase<T>::operator+=(const FxStringBase<T>& other)
{
	_DoAppendString(other._container._v, other._container._usedCount);
	return *this;
}

// Appends a native string.
template<typename T>
FxStringBase<T>& FxStringBase<T>::operator+=(const char_type* other)
{
	if( other )
	{
		_DoAppendString(other, traits_type::Length(other));
	}
	return *this;
}

// Appends a native character.
template<typename T>
FxStringBase<T>& FxStringBase<T>::operator+=(const char_type& other)
{
	_DoAppendString(&other, 1);
	return *this;
}

// Returns the native string.
template<typename T> FX_INLINE
const typename FxStringBase<T>::char_type* FxStringBase<T>::operator*() const
{
	_EnsureValid();
	return _container._v;
}

// Returns the native string.
template<typename T> FX_INLINE
const typename FxStringBase<T>::char_type* FxStringBase<T>::GetData() const
{
	_EnsureValid();
	return _container._v;
}

// iostream-style string streaming.
template<typename T>
FxStringBase<T>& FxStringBase<T>::operator<<(const char_type* str)
{
	if( str )
	{
		_DoAppendString(str, traits_type::Length(str));
	}
	return *this;
}

// iostream-style integer streaming.
template<typename T>
FxStringBase<T>& FxStringBase<T>::operator<<(const FxInt32 num)
{
	char_type buffer[64] = {0};
	FxItoa(num, buffer, 64);
	_DoAppendString(buffer, traits_type::Length(buffer));
	return *this;
}

// iostream-style unsigned integer streaming.
template<typename T>
FxStringBase<T>& FxStringBase<T>::operator<<(const FxUInt32 num)
{
	char_type buffer[64] = {0};
	FxItoa(num, buffer, 64);
	_DoAppendString(buffer, traits_type::Length(buffer));
	return *this;
}

template<typename T>
FxStringBase<T>& FxStringBase<T>::operator<<(const FxReal num)
{
	char_type buffer[64] = {0};
	FxFtoa(num, buffer);
	_DoAppendString(buffer, traits_type::Length(buffer));
	return *this;
}

// Finds the first instance of a character in the string.
template<typename T>
FxSize FxStringBase<T>::FindFirst(const char_type& c, FxSize start) const
{
	FxAssert(start <= _container._usedCount);
	if( _container._usedCount > 0 )
	{
		const char_type* pos = traits_type::Find(_container._v + start, c, _container._usedCount - start);
		if( pos != NULL )
		{
			return static_cast<FxSize>(pos - _container._v);
		}
	}
	return FxInvalidIndex;
}

// Returns the part of the string before the first instance of a character.
template<typename T>
FxStringBase<T> FxStringBase<T>::BeforeFirst(const char_type& c) const
{
	FxSize pos = FindFirst(c);
	if( pos != FxInvalidIndex )
	{
		FxStringBase<T> retval(_container._v, pos);
		return retval;
	}
	return FxStringBase<T>(_container._v, _container._usedCount);
}

// Returns the part of the string after the first instance of a character.
template<typename T>
FxStringBase<T> FxStringBase<T>::AfterFirst(const char_type& c) const
{
	FxSize pos = FindFirst(c);
	if( pos != FxInvalidIndex )
	{
		FxStringBase<T> retval(_container._v + pos + 1, _container._usedCount - pos - 1);
		return retval;
	}
	return FxStringBase<T>();
}

// Finds the last instance of a character in the string.
template<typename T>
FxSize FxStringBase<T>::FindLast(const char_type& c, FxSize start) const
{
	if( _container._usedCount > 0 )
	{
		FxInt32 pos = static_cast<FxInt32>(start);
		if( start == FxInvalidIndex )
		{
			pos = _container._usedCount - 1;
		}

		for(; pos >= 0; --pos )
		{
			if( _container._v[pos] == c )
			{
				return pos;
			}
		}
	}

	return FxInvalidIndex;
}

// Returns the part of the string before the last instance of a character.
template<typename T>
FxStringBase<T> FxStringBase<T>::BeforeLast(const char_type& c) const
{
	FxSize pos = FindLast(c);
	if( pos != FxInvalidIndex )
	{
		FxStringBase<T> retval(_container._v, pos);
		return retval;
	}
	return FxStringBase<T>();
}

// Returns the part of the string after the last instance of a character.
template<typename T>
FxStringBase<T> FxStringBase<T>::AfterLast(const char_type& c) const
{
	FxSize pos = FindLast(c);
	if( pos != FxInvalidIndex )
	{
		FxStringBase<T> retval(_container._v + pos + 1, _container._usedCount - pos - 1);
		return retval;
	}
	return FxStringBase<T>(_container._v, _container._usedCount);
}

// Find the first character not equal to a given character.
template<typename T>
FxSize FxStringBase<T>::FindFirstNotOf(const char_type& c) const
{
	for( FxSize i = 0; i < _container._usedCount; ++i )
	{
		if( traits_type::Compare(_container._v + i, &c, 1) != 0 )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

// Find the last character not equal to a given character.
template<typename T>
FxSize FxStringBase<T>::FindLastNotOf(const char_type& c) const
{
	for( FxInt32 i = _container._usedCount - 1; i >= 0; --i )
	{
		if( traits_type::Compare(_container._v + i, &c, 1) != 0 )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

// Finds a substring.
template<typename T>
FxSize FxStringBase<T>::Find(const FxStringBase& str) const
{
	FxSize substringLength = str.Length();
	if( _container._usedCount >= substringLength )
	{
		for( FxSize i = 0; i <= _container._usedCount - substringLength; ++i )
		{
			if( traits_type::Compare(_container._v + i, str._container._v, substringLength) == 0 )
			{
				return i;
			}
		}
	}
	return FxInvalidIndex;
}

// Extracts a substring from the string.
template<typename T>
FxStringBase<T> FxStringBase<T>::Substring(FxSize start, FxSize length) const
{
	if( start < _container._usedCount )
	{
		if( length == FxInvalidIndex )
		{
			length = Length() - start;
		}
		return FxStringBase<T>(_container._v + start, length);
	}
	else
	{
		return FxStringBase<T>();
	}
}

// Replaces a substring with another.
template<typename T>
FxBool FxStringBase<T>::Replace(const FxStringBase& toFind, const FxStringBase& toReplace)
{
	FxSize location = Find(toFind);
	if( location != FxInvalidIndex )
	{
		FxSize newStringLength = _container._usedCount - toFind.Length() + toReplace.Length() + 1;
		_container_type temp(newStringLength);
		traits_type::Copy(temp._v, _container._v, location);
		traits_type::Copy(temp._v + location, toReplace._container._v, toReplace._container._usedCount);
		traits_type::Copy(temp._v + location + toReplace.Length(), _container._v + location + toFind.Length(), _container._usedCount - location - toFind.Length());
		temp._usedCount = newStringLength - 1;
		_container.Swap(temp);
		_EnsureValid();
		return FxTrue;
	}
	return FxFalse;
}

// Removes whitespace from the beginning and end of the string.
template<typename T>
FxStringBase<T> FxStringBase<T>::StripWhitespace() const
{
	FxSize start = FxInvalidIndex;
	for( FxSize i = 0; i < _container._usedCount; ++i )
	{
		if( !traits_type::IsWhitespace(_container._v[i]) )
		{
			start = i;
			break;
		}
	}

	FxSize end = FxInvalidIndex;
	for( FxInt32 i = _container._usedCount - 1; i >= 0; --i )
	{
		if( !traits_type::IsWhitespace(_container._v[i]) )
		{
			end = i;
			break;
		}
	}

	if( start != FxInvalidIndex && end != FxInvalidIndex && start < end )
	{
		return Substring(start, end - start + 1);
	}
	return FxStringBase();
}

// Converts the string to upper case.
template<typename T>
FxStringBase<T> FxStringBase<T>::ToUpper() const
{
	FxStringBase retval(*this);
	for( FxSize i = 0; i < _container._usedCount; ++i )
	{
		retval._container._v[i] = traits_type::ToUpper(_container._v[i]);
	}
	return retval;
}

// Converts the string to lower case.
template<typename T>
FxStringBase<T> FxStringBase<T>::ToLower() const
{
	FxStringBase retval(*this);
	for( FxSize i = 0; i < _container._usedCount; ++i )
	{
		retval._container._v[i] = traits_type::ToLower(_container._v[i]);
	}
	return retval;
}

// Converts the string to an ansi string.
template<typename T>
FxTempCharBuffer<FxAChar> FxStringBase<T>::GetCstr() const
{
	_EnsureValid();
	FxTempCharBuffer<FxAChar> retval(_container._v);
	return retval;
}

// Converts the string to a unicode string.
template<typename T>
FxTempCharBuffer<FxWChar> FxStringBase<T>::GetWCstr() const
{
	_EnsureValid();
	FxTempCharBuffer<FxWChar> retval(_container._v);
	return retval;
}

// Swaps two strings.
template<typename T>
void FxStringBase<T>::Swap(FxStringBase<T>& other)
{
	_container.Swap(other._container);
}

// Concatenate two strings.
template<typename T>
FxStringBase<T> FxStringBase<T>::Concat(const FxStringBase<T>& first, 
									  const FxStringBase<T>& second)
{
	FxStringBase retval(first);
	retval += second;
	return retval;
}

// Concatenate a character to a string.
template<typename T>
FxStringBase<T> FxStringBase<T>::Concat(const FxStringBase<T>& first, const char_type& c)
{
	FxStringBase retval(first);
	retval += c;
	return retval;
}

// Append helper.
template<typename T>
void FxStringBase<T>::_DoAppendString(const char_type* str, FxSize length)
{
	if( _container._allocatedCount < _container._usedCount + length + 1 )
	{
		// Reallocate and copy.
		_container_type temp(_container._usedCount + length + 1);
		traits_type::Copy(temp._v, _container._v, _container._usedCount);
		temp._usedCount += _container._usedCount;
		traits_type::Copy(temp._v + temp._usedCount, str, length);
		temp._usedCount += length;
		_container.Swap(temp);
	}
	else
	{
		// Otherwise, append it on the end.
		traits_type::Copy(_container._v + _container._usedCount, str, length);
		_container._usedCount += length;
	}
	_container._v[_container._usedCount] = static_cast<char_type>(0);
}

// Ensures the string has valid data.
template<typename T> FX_INLINE
void FxStringBase<T>::_EnsureValid() const
{
	if( _container._v == NULL )
	{
		_container_type temp(16);
		_container.Swap(temp);
	}
	_container._v[_container._usedCount] = static_cast<char_type>(0);
}

/// String serialization.
template<typename T>
FxArchive& operator<<( FxArchive& arc, FxStringBase<T>& str )
{
	return str.Serialize(arc);
}

// String equality.
template<typename T>
FxBool operator==(const FxStringBase<T>& lhs, const FxStringBase<T>& rhs)
{
	if( lhs.Length() == rhs.Length() )
	{
		return FxStringBase<T>::traits_type::Compare(*lhs, *rhs, lhs.Length()) == 0;
	}
	return FxFalse;
}

template<typename T>
FxBool operator==(const FxStringBase<T>& lhs, const T* rhs)
{
	if( rhs && lhs.Length() == FxCharTraits<T>::Length(rhs) )
	{
		return FxStringBase<T>::traits_type::Compare(*lhs, rhs, lhs.Length()) == 0;
	}
	return FxFalse;
}

template<typename T>
FxBool operator==(const T* lhs, const FxStringBase<T>& rhs)
{
	if( lhs && FxCharTraits<T>::Length(lhs) == rhs.Length() )
	{
		return FxStringBase<T>::traits_type::Compare(lhs, *rhs, rhs.Length()) == 0;
	}
	return FxFalse;
}

// String inequality.
template<typename T>
FxBool operator!=(const FxStringBase<T>& lhs, const FxStringBase<T>& rhs)
{
	if( lhs.Length() == rhs.Length() )
	{
		return FxStringBase<T>::traits_type::Compare(*lhs, *rhs, lhs.Length()) != 0;
	}
	return FxTrue;
}

template<typename T>
FxBool operator!=(const FxStringBase<T>& lhs, const T* rhs)
{
	if( rhs && lhs.Length() == FxCharTraits<T>::Length(rhs) )
	{
		return FxStringBase<T>::traits_type::Compare(*lhs, rhs, lhs.Length()) != 0;
	}
	return FxTrue;
}

template<typename T>
FxBool operator!=(const T* lhs, const FxStringBase<T>& rhs)
{
	if( lhs && FxCharTraits<T>::Length(lhs) == rhs.Length() )
	{
		return FxStringBase<T>::traits_type::Compare(lhs, *rhs, rhs.Length()) != 0;
	}
	return FxTrue;
}

// Lexicographic less-than
template<typename T>
FxBool operator<(const FxStringBase<T>& lhs, const FxStringBase<T>& rhs)
{
	return FxStringBase<T>::traits_type::Compare(*lhs, *rhs, FxMin(lhs.Length(), rhs.Length())) < 0;
}

// Lexicographic less-than-equal-to
template<typename T>
FxBool operator<=(const FxStringBase<T>& lhs, const FxStringBase<T>& rhs)
{
	return FxStringBase<T>::traits_type::Compare(*lhs, *rhs, FxMin(lhs.Length(), rhs.Length())) <= 0;
}

// Lexicographic greater-than
template<typename T>
FxBool operator>(const FxStringBase<T>& lhs, const FxStringBase<T>& rhs)
{
	return FxStringBase<T>::traits_type::Compare(*lhs, *rhs, FxMin(lhs.Length(), rhs.Length())) > 0;
}

// Lexicographic greater-than-equal-to
template<typename T>
FxBool operator>=(const FxStringBase<T>& lhs, const FxStringBase<T>& rhs)
{
	return FxStringBase<T>::traits_type::Compare(*lhs, *rhs, FxMin(lhs.Length(), rhs.Length())) >= 0;
}

// String concatenation
template<typename T>
FxStringBase<T> operator+(const FxStringBase<T>& lhs, const FxStringBase<T>& rhs)
{
	FxStringBase<T> retval(lhs);
	retval += rhs;
	return retval;
}

template<typename T>
FxStringBase<T> operator+(const FxStringBase<T>& lhs, const T* rhs)
{
	FxStringBase<T> retval(lhs);
	retval += rhs;
	return retval;
}

template<typename T>
FxStringBase<T> operator+(const T* lhs, const FxStringBase<T>& rhs)
{
	FxStringBase<T> retval(lhs);
	retval += rhs;
	return retval;
}

template<typename T>
FxStringBase<T> operator+(const FxStringBase<T>& lhs, const T& rhs)
{
	FxStringBase<T> retval(lhs);
	retval += rhs;
	return retval;
}

template<typename T>
FxStringBase<T> operator+(const T& lhs, const FxStringBase<T>& rhs)
{
	FxStringBase<T> retval(lhs);
	retval += rhs;
	return retval;
}

// Construct a temp buffer from a native string.
template<typename T>
FxTempCharBuffer<T>::FxTempCharBuffer(const char_type* pNativeStr)
{
	FxSize length = traits_type::Length(pNativeStr);
	_bufferSize = static_cast<FxSize>(sizeof(char_type) * (length + 1));
	_buffer = static_cast<char_type*>(FxAlloc(_bufferSize, "FxTempCharBuffer"));
	traits_type::Copy(_buffer, pNativeStr, length);
	_buffer[length] = static_cast<char_type>(0);
}

// Construct a temp buffer from a foreign string.
template<typename T>
FxTempCharBuffer<T>::FxTempCharBuffer(const foreign_char_type* pForeignStr)
{
	FxSize length = traits_type::other_traits_type::Length(pForeignStr);
	_bufferSize = static_cast<FxSize>(sizeof(char_type) * (length + 1));
	_buffer = static_cast<char_type*>(FxAlloc(_bufferSize, "FxTempCharBuffer"));
	traits_type::Convert(_buffer, pForeignStr, length);
	_buffer[length] = static_cast<char_type>(0);
}

// Copy construction.
template<typename T>
FxTempCharBuffer<T>::FxTempCharBuffer(FxTempCharBuffer& other)
	: _buffer(other.Release())
	, _bufferSize(other._bufferSize)
{
}

// Destructor.
template<typename T>
FxTempCharBuffer<T>::~FxTempCharBuffer()
{
	if( _buffer )
	{
		FxFree(_buffer, _bufferSize);
		_buffer = NULL;
		_bufferSize = 0;
	}
}

// Allow implicit conversion to a native string.
template<typename T> FX_INLINE
FxTempCharBuffer<T>::operator const typename FxTempCharBuffer<T>::char_type*() const
{
	return _buffer;
}

// Release the native string from the temp buffer.
// The returned pointer must be freed using FxFree().
template<typename T>
typename FxTempCharBuffer<T>::char_type* FxTempCharBuffer<T>::Release()
{
	char_type* retval = _buffer;
	_buffer = NULL;
	return retval;
}

// Return the size of the memory used by the native string in the temp
// buffer (in bytes, including the NULL terminator).
template<typename T> FX_INLINE
FxSize FxTempCharBuffer<T>::GetSize() const
{
	return _bufferSize;
}

} // namespace Face

} // namespace OC3Ent
