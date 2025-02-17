//------------------------------------------------------------------------------
// A templated string for both ansi and wide character support.
//
// Owner: John Briggs
// 
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxString_H__
#define FxString_H__

#include "FxPlatform.h"
#include "FxCharTraits.h"
#include "FxArchive.h"
#include "FxUtil.h"

namespace OC3Ent
{

namespace Face
{

// Forward declare the array base.
template<typename Elem> class FxArrayBase;

// Forward declare the temp buffer classes.
template<typename T> class FxTempCharBuffer;

#define kCurrentFxStringVersion 1

/// A templated string for both ansi and wide character support.
///
/// The string, like the array, uses the array base as the underlying storage
/// mechanism to help provide exception safety and ensure proper cleanup.
///
/// The underlying string is not necessarily always <tt>NULL</tt>-terminated, 
/// and thus should not be used with that assumption. However, immediately after
/// calling FxStringBase::operator*(), the underlying string will be 
/// <tt>NULL</tt>-terminated and safe to pass to any functions expecting a 
/// C-style string.  Any further modifications will invalidate this state.
///
/// Constructors are provided that take both ANSI and UNICODE strings.  If 
/// necessary, a conversion will be applied.  The string always stores the data
/// internally in its native format.
///
/// A quick note on terminology.  Throughout the class documentation, a 
/// \em string refers to FxStringBase<char_type>.  A \em native \em string 
/// refers to a C-style string, i.e. \p char_type*.  Note that the character 
/// types for \em string and \em native \em string are the same.  A \em foreign 
/// \em string is the "other" kind of string.  For a \p char_type of \p char, a 
/// \em foreign \em string is a \p wchar_t* string, and vice-versa.
/// \ingroup support
template<typename T> class FxStringBase
{
	typedef FxArrayBase<T>	_container_type;
	typedef FxStringBase<T>	_string_type;

public:
	/// The native character type for the string.
	typedef T char_type;
	/// The character traits type for the string.
	typedef FxCharTraits<char_type> traits_type;
	/// The foreign character type for the string.
	/// For a string with char_type \p char this will be \p wchar_t, and 
	/// vice-versa.
	typedef typename traits_type::foreign_char_type foreign_char_type;

	/// Constructs a string with room for \a numReserved characters.
	/// \param numReserved The maximum length the string could attain without a 
	/// reallocation.
	/// \note This only allocates enough room for \a numReserved characters.  It
	/// does not allow random assignment into the string:
	/// \code
	/// FxString example(50); // Allocate enough room for a 50 character string.
	/// example[25] = 'c'; // Error! The string still thinks its length is 0.
	/// \endcode
	explicit FxStringBase(FxSize numReserved = 0);
	/// Constructs a string from a native string.
	/// \param str The string from which to construct.
	/// \param numChars The number of characters of str to use. \p FxInvalidIndex
	/// indicates to use all of \a str up to the terminating \p NULL.
	FxStringBase(const char_type* str, FxSize numChars = FxInvalidIndex);
	/// Constructs a string from a native character.
	/// \param c The character from which to construct.
	/// \param numChars The length of the resulting string, each character of which
	/// will be equal to \a c.
	explicit FxStringBase(const char_type& c, FxSize numChars = 1);
	/// Constructs a string from a foreign string.
	/// For an ANSI string, constructs from a UNICODE string, and vice versa.
	/// \param str The string from which to construct.
	explicit FxStringBase(const foreign_char_type* str);
	/// Copy constructor.
	/// \param other The string to copy.
	FxStringBase(const FxStringBase& other);
	/// Assignment operator.
	/// \param other The string to copy.
	FxStringBase& operator=(const FxStringBase& other);
	/// Assignment from native string.
	/// \param str The native string to copy.
	FxStringBase& operator=(const char_type* str);

	/// Clears the string.
	/// This function does not free the memory allocated by the string.  To free
	/// the memory allocated by a string, use 
	/// \code stringToClear.Swap(FxString()); \endcode
	void Clear();
	/// Returns the string length.
	FxSize Length() const;
	/// Returns the amount of memory allocated by the string.
	FxSize Allocated() const;
	/// Returns true if the string is empty.
	FxBool IsEmpty() const;

	/// Character access.
	/// \param index The index of the character to access.
	/// \return A reference to the character in the string at \a index.
	char_type& operator[](FxSize index);
	/// \param index The index of the character to access.
	/// \return A const reference to the character in the string at \a index.
	const char_type& operator[](FxSize index) const;
	/// \param index The index of the character to access.
	/// \return The character at \a index.
	char_type GetChar(FxSize index) const;

	/// Appends a string.
	/// \param other The string to append.
	/// \return A reference to this string with \a other appended.
	FxStringBase& operator+=(const FxStringBase& other);
	/// Appends a native string.
	/// \param other The native string to append.
	/// \return A reference to this string with \a other appended.
	FxStringBase& operator+=(const char_type* other);
	/// Appends a native character.
	/// \param other The native character to append.
	/// \return A reference to this string with \a other appended.
	FxStringBase& operator+=(const char_type& other);
	/// Returns the native string with no additional allocation overhead.
	/// Calling GetCstr() or GetWCstr() must allocate a temporary buffer to 
	/// convert into because the class doesn't necessarily know whether or not 
	/// an ANSI or UNICODE, respectively, string is the underlying type.
	/// \return A pointer to the underlying native string, <tt>NULL</tt>-terminated.
	const char_type* operator*() const;
	/// Returns the native string with no additional allocation overhead.
	/// \return A pointer to the underlying native string, <tt>NULL</tt>-terminated.
	const char_type* GetData() const;

	/// iostream-style string streaming.
	/// \param str The string to append.
	/// \return A reference to the string with \a str appended.
	FxStringBase& operator<<(const char_type* str);
	/// iostream-style integer streaming.
	/// \param num The integer to append.
	/// \return A reference to the string with \a num appended.
	FxStringBase& operator<<(const FxInt32 num);
	/// iostream-style unsigned integer streaming.
	/// \param num The unsigned integer to append.
	/// \return A reference to the string with \a num appended.
	FxStringBase& operator<<(const FxUInt32 num);
	/// iostream-style float streaming.
	/// \param num The float to append.
	/// \return A reference to the string with \a num appended.
	FxStringBase& operator<<(const FxReal num);

	/// Finds the first instance of a character in the string.
	/// \param c The character to find.
	/// \param start The character on which to begin the search.
	/// \return The index of the first instance of a character equal to \a c
	/// in the string, or \p FxInvalidIndex if \a c was not found.
	FxSize FindFirst(const char_type& c, FxSize start = 0) const;
	/// Returns the part of the string before the first instance of a character.
	/// \param c The character to find.
	/// \return The portion of the string up to, but not including the first 
	/// instance of \a c.  If \a c was not found, returns the entire string.
	FxStringBase BeforeFirst(const char_type& c) const;
	/// Returns the part of the string after the first instance of a character.
	/// \param c The character to find.
	/// \return The portion of the string after, but not including the first 
	/// instance of \a c.  If \a c was not found, returns an empty string.
	FxStringBase AfterFirst(const char_type& c) const;

	/// Finds the last instance of a character in the string.
	/// \param c The character to find.
	/// \param start The index of the character from which to start the 
	/// reverse search.
	/// \return The index of the last instance of a character equal to \a c
	/// in the string, or \p FxInvalidIndex if \a c was not found.
	FxSize FindLast(const char_type& c, FxSize start = FxInvalidIndex) const;
	/// Returns the part of the string before the last instance of a character.
	/// \param c The character to find.
	/// \return The portion of the string up to, but not including the last 
	/// instance of \a c.  If \a c was not found, returns an empty string.
	FxStringBase BeforeLast(const char_type& c) const;
	/// Returns the part of the string after the last instance of a character.
	/// \param c The character to find.
	/// \return The portion of the string after, but not including the first 
	/// instance of \a c.  If \a c was not found, returns the entire string.
	FxStringBase AfterLast(const char_type& c) const;

	/// Find the first character not equal to a given character.
	/// \param c The character to find.
	/// \return The index of the first character not equal to \a c.
	FxSize FindFirstNotOf(const char_type& c) const;
	/// Find the last character not equal to a given character.
	/// \param c The character to find.
	/// \return The index of the last character not equal to \a c.
	FxSize FindLastNotOf(const char_type& c) const;

	/// Finds a substring.
	/// \param str The substring to search for.
	/// \return The index of the first character of the substring, or
	/// \p FxInvalidIndex if \a str was not found in the string.
	FxSize Find(const FxStringBase& str) const;
	/// Extracts a substring from the string.
	/// \param start The index of the first character in the substring.
	/// \param length The length of the substring to extract.  \p FxInvalidIndex
	/// returns the substring from \a start to the end of the string.
	/// \return The substring.
	FxStringBase Substring(FxSize start, FxSize length = FxInvalidIndex) const;
	/// Replaces a substring with another.
	/// \param toFind The substring to find.
	/// \param toReplace The substring to replace \a toFind with.
	/// \return \p FxTrue if the operation completed successfully, 
	/// \p FxFalse otherwise.
	FxBool Replace(const FxStringBase& toFind, const FxStringBase& toReplace);

	/// Removes whitespace from the beginning and end of the string.
	/// \return A string with the whitespace removed.
	FxStringBase StripWhitespace() const;

	/// Converts the string to upper case.
	/// \return The string as upper case.
	FxStringBase ToUpper() const;
	/// Converts the string to lower case.
	/// \return The string as lower case.
	FxStringBase ToLower() const;

	/// Converts the string to an ANSI string.
	/// Must allocate a new buffer for the return value, since the string may 
	/// need to convert UNICODE to ANSI.
	/// \return The string in ANSI format.  The temporary buffer will automatically
	/// be cleaned up when it goes out of scope.  If you need to keep it around,
	/// either copy the contents, or detach the string from the FxTempCharBuffer
	/// using FxTempCharBuffer::Release().
	FxTempCharBuffer<FxAChar> GetCstr() const;
	// Converts the string to a UNICODE string.
	/// Must allocate a new buffer for the return value, since the string may
	/// need to convert ANSI to UNICODE.
	/// \return The string in UNICODE format.  The temporary buffer will automatically
	/// be cleaned up when it goes out of scope.  If you need to keep it around,
	/// either copy the contents, or detach the string from the FxTempCharBuffer
	/// using FxTempCharBuffer::Release().
	FxTempCharBuffer<FxWChar> GetWCstr() const;

	/// Swaps the contents with another string.
	void Swap(FxStringBase& other);

	/// Concatenate two strings.
	/// \deprecated as of version 1.3.  Use operator+ or operator+=.
	static FxStringBase Concat(const FxStringBase& first, const FxStringBase& second);
	/// Concatenate a character to a string.
	/// \deprecated as of version 1.3.  Use operator+ or operator+=.
	static FxStringBase Concat(const FxStringBase& first, const char_type& c);

	/// Serializes the string to an archive.
	FxArchive& Serialize( FxArchive& arc )
	{
		// As of version 1.7, container classes have the same version as the 
		// SDK itself so there is no need to explicitly serialize it.  This eats
		// the version when loading pre-1.7 archives.  For FxString, however,
		// the version is always kCurrentFxStringVersion until there is a 
		// compelling reason to change it since the code is already structured
		// in that way.
		FxUInt16 version = kCurrentFxStringVersion;
		if( arc.GetSDKVersion() < 1700 && arc.IsLoading() )
		{
			arc << version;
		}

		FxSize length = Length();
		arc << length;

		if( arc.IsSaving() )
		{
			arc.SerializePODArray(_container._v, length);
		}
		else
		{
			// If we're loading an old version (implicit ANSI) string into a 
			// new wide-character string we need to perform a conversion.
			if( version < 1 && sizeof(T) > 1 )
			{
				FxArrayBase<foreign_char_type> foreignTemp(length);
				arc.SerializePODArray(foreignTemp._v, length);

				_container_type nativeTemp(length + 1);
				traits_type::Convert(nativeTemp._v, foreignTemp._v, length);
				nativeTemp._usedCount += length;

				nativeTemp.Swap(_container);
			}
			// Otherwise, we're loading into an ANSI string or a current
			// version string, and don't need special conversion code.
			else
			{
				// Clear the underlying storage and reserve room for the length of the
				// string plus the terminating null.
				_container_type temp(length + 1);
				arc.SerializePODArray(temp._v, length);
				temp._usedCount += length;
				temp.Swap(_container);
			}
			_container._v[_container._usedCount] = static_cast<char_type>(0);
		}
		return arc;
	}

private:
	// Append helper.
	void _DoAppendString(const char_type* str, FxSize length);
	// Ensures the string has valid data.
	void _EnsureValid() const;

	// The container for the data in the string.
	mutable _container_type _container;
};

/// String serialization.
template<typename T> FxArchive& operator<<( FxArchive& arc, FxStringBase<T>& str );

/// String equality.
/// Strings are equal if their lengths are equal and each corresponding pair of
/// characters are equal.
/// \param lhs The first string to be compared.
/// \param rhs The second string to be compared.
/// \return \p FxTrue if the strings are equal, \p FxFalse otherwise.
/// \relates FxStringBase
template<typename T> FxBool operator==(const FxStringBase<T>& lhs, const FxStringBase<T>& rhs);
/// String equality.
/// \note This is an overloaded function, provided for convenience.
/// \relates FxStringBase
template<typename T> FxBool operator==(const FxStringBase<T>& lhs, const T* rhs);
/// String equality.
/// \note This is an overloaded function, provided for convenience.
/// \relates FxStringBase
template<typename T> FxBool operator==(const T* lhs, const FxStringBase<T>& rhs);
/// String inequality.
/// \relates FxStringBase
template<typename T> FxBool operator!=(const FxStringBase<T>& lhs, const FxStringBase<T>& rhs);
/// String inequality.
/// \note This is an overloaded function, provided for convenience.
/// \relates FxStringBase
template<typename T> FxBool operator!=(const FxStringBase<T>& lhs, const T* rhs);
/// String inequality.
/// \note This is an overloaded function, provided for convenience.
/// \relates FxStringBase
template<typename T> FxBool operator!=(const T* lhs, const FxStringBase<T>& rhs);
/// Lexicographic less-than
/// \relates FxStringBase
template<typename T> FxBool operator<(const FxStringBase<T>& lhs, const FxStringBase<T>& rhs);
/// Lexicographic less-than-equal-to
/// \relates FxStringBase
template<typename T> FxBool operator<=(const FxStringBase<T>& lhs, const FxStringBase<T>& rhs);
/// Lexicographic greater-than
/// \relates FxStringBase
template<typename T> FxBool operator>(const FxStringBase<T>& lhs, const FxStringBase<T>& rhs);
/// Lexicographic greater-than-equal-to
/// \relates FxStringBase
template<typename T> FxBool operator>=(const FxStringBase<T>& lhs, const FxStringBase<T>& rhs);

/// String concatenation.
/// \relates FxStringBase
template<typename T> FxStringBase<T> operator+(const FxStringBase<T>& lhs, const FxStringBase<T>& rhs);
/// String concatenation.
/// \note This is an overloaded function, provided for convenience.
/// \relates FxStringBase
template<typename T> FxStringBase<T> operator+(const FxStringBase<T>& lhs, const T* rhs);
/// String concatenation.
/// \note This is an overloaded function, provided for convenience.
/// \relates FxStringBase
template<typename T> FxStringBase<T> operator+(const T* lhs, const FxStringBase<T>& rhs);
/// String concatenation.
/// \note This is an overloaded function, provided for convenience.
/// \relates FxStringBase
template<typename T> FxStringBase<T> operator+(const FxStringBase<T>& lhs, const T& rhs);
/// String concatenation.
/// \note This is an overloaded function, provided for convenience.
/// \relates FxStringBase
template<typename T> FxStringBase<T> operator+(const T& lhs, const FxStringBase<T>& rhs);

/// The default string type for FaceFX.
typedef FxStringBase<FxAChar> FxString;
/// An ANSI string.
typedef FxStringBase<FxAChar> FxAString;
/// A UNICODE string.
typedef FxStringBase<FxWChar> FxWString;

/// A temporary buffer for holding a return string type.
/// 
/// The temp char buffer using strict ownership semantics, much like
/// \p std::auto_ptr.  When copy constructed or assigned, it takes ownership of 
/// the contained pointer.  Thus, it can be used to return dynamically allocated
/// memory without the client needing to deal with freeing the memory.
///
/// If the client ends up needing to keep the string contained in a temp buffer,
/// the simplest way is to copy the data.  If a higher-performance solution is
/// needed, the client may also take ownership of the contained c-style string
/// by calling FxTempCharBuffer::Release().  In this case, the client becomes
/// responsible for freeing the memory using FxFree().
template<typename T> class FxTempCharBuffer
{
public:
	// Typedefs.
	typedef T char_type;
	typedef FxCharTraits<char_type> traits_type;
	typedef typename traits_type::foreign_char_type foreign_char_type;

	/// Construct a temp buffer from a native string.
	explicit FxTempCharBuffer(const char_type* pNativeStr);
	/// Construct a temp buffer from a foreign string.
	explicit FxTempCharBuffer(const foreign_char_type* pForeignStr);
	/// Copy construction.
	FxTempCharBuffer(FxTempCharBuffer& other);
	/// Destructor.
	~FxTempCharBuffer();

	/// Allow implicit conversion to a native string.
	operator const char_type*() const;
	/// Release the native string from the temp buffer.
	/// The returned pointer must be freed using FxFree().
	char_type* Release();
	/// Return the size of the memory used by the native string in the temp
	/// buffer (in bytes, including the NULL terminator).
	FxSize GetSize() const;

private:
	char_type* _buffer;
	FxSize _bufferSize;
};

/// A macro for wide-character strings.
#define FxW(str) L##str

} // namespace Face

} // namespace OC3Ent

#include "FxStringImpl.h"

#endif
