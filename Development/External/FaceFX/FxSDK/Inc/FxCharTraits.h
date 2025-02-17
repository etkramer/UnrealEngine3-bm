//------------------------------------------------------------------------------
// Wrap and use the appropriate functions according to character type.
//
// Owner: John Briggs
// 
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCharTraits_H__
#define FxCharTraits_H__

#include "FxPlatform.h"

namespace OC3Ent
{

namespace Face
{

/// Typedefs and functions for working with a specific char type.
template<class T> struct FxCharTraits
{
public:
	/// The type of the character.
	typedef T char_type;
	/// The 'foreign' type of the character.
	/// For a \p char_type of \p char, \p foreign_char_type would be \p wchar_t.
	typedef FxWChar foreign_char_type;
	/// The 'foreign' character's traits type.
	typedef FxCharTraits<FxWChar> other_traits_type;

	/// The length of a <TT>NULL</TT>-terminated string.
	/// \param str The string.
	/// \return The length of the string.
	static FxSize FX_CALL Length(const char_type* str);
	/// Moves characters between (or within) strings.
	/// \param dest The destination string.
	/// \param src The source string.
	/// \param num The number of characters to move.
	/// \return A pointer to the destination string.
	/// \note \a dest and \a src may overlap.
	static char_type* FX_CALL Move(char_type* dest, const char_type* src, FxSize num);
	/// Copies characters between strings.
	/// \param dest The destination string.
	/// \param src The source string.
	/// \param num The number of characters to copy.
	/// \return A pointer to the destination string.
	/// \note \a dest and \a src may not overlap.
	static char_type* FX_CALL Copy(char_type* dest, const char_type* src, FxSize num);
	/// Finds a character in a string.
	/// \param str The string to search.
	/// \param c The character to find.
	/// \param num The maximum number of characters in \a str to consider.
	/// \return A pointer to the character in \a str.
	static const char_type* FX_CALL Find(const char_type* str, const char_type& c, FxSize num);
	/// Finds a character in a string, searching from back to front.
	/// \param str The string to search.
	/// \param c The character to find.
	/// \param num The maximum number of characters in \a str to consider.
	/// \return A pointer to the character in \a str.
	static const char_type* FX_CALL RFind(const char_type* str, const char_type& c);
	/// Compares two strings.
	/// \param first One string to compare.
	/// \param second The other string to compare.
	/// \param num The number of characters to compare.
	/// \return Same as \a strcmp.
	static FxInt32 FX_CALL Compare(const char_type* first, const char_type* second, FxSize num);
	/// Converts a foreign string to its native equivalent.
	/// This would convert UNICODE to ANSI, and vice-versa.
	/// \param dest The destination native string.
	/// \param src The source foreign string.
	/// \param num The number of characters to convert.
	/// \return A pointer to \a dest.
	static char_type* FX_CALL Convert(char_type* dest, const foreign_char_type* src, FxSize num);

	/// Converts a character to upper case.
	/// \param c The character to convert.
	/// \return The character as upper case.
	static char_type FX_CALL ToUpper(char_type c);
	/// Converts a character to lower case.
	/// \param c The character to convert.
	/// \return The character as lower case.
	static char_type FX_CALL ToLower(char_type c);
	/// Returns true if a character is whitespace.
	static FxBool FX_CALL IsWhitespace(char_type c);

	static const char_type SpaceCharacter;
	static const char_type TabCharacter;
	static const char_type NewlineCharacter;
	static const char_type ReturnCharacter;
};

/// Typedefs and functions for working with ANSI characters and strings.
template<> struct FxCharTraits<FxAChar>
{
public:
	typedef FxAChar char_type;
	typedef FxWChar foreign_char_type;
	typedef FxCharTraits<FxWChar> other_traits_type;

	static FxSize FX_CALL Length(const char_type* str)
	{
		return FxStrlen(str);
	}

	static char_type* FX_CALL Move(char_type* dest, const char_type* src, FxSize num)
	{
		return static_cast<char_type*>(FxMemmove(dest, src, num));
	}

	static char_type* FX_CALL Copy(char_type* dest, const char_type* src, FxSize num)
	{
		return static_cast<char_type*>(FxMemcpy(dest, src, num));
	}

	static const char_type* FX_CALL Find(const char_type* str, const char_type& c, FxSize num)
	{
		return static_cast<const char_type*>(FxMemchr(str, c, num));
	}

	static const char_type* FX_CALL RFind(const char_type* str, const char_type& c)
	{
		return FxStrrchr(str, c);
	}

	static FxInt32 FX_CALL Compare(const char_type* first, const char_type* second, FxSize num)
	{
		return FxMemcmp(first, second, num);
	}

	static char_type* FX_CALL Convert(char_type* dest, const foreign_char_type* src, FxSize num)
	{
		// not even close to correct...
		FxSize i = 0;
		while( i < num )
		{
			dest[i] = static_cast<char_type>(fx_std(wctob)(src[i]));
			++i;
		}
		return dest;
	}

	static char_type FX_CALL ToUpper(char_type c)
	{
		return static_cast<char_type>(toupper(c));
	}

	static char_type FX_CALL ToLower(char_type c)
	{
		return static_cast<char_type>(tolower(c));
	}

	static const char_type SpaceCharacter = ' ';
	static const char_type TabCharacter = '\t';
	static const char_type NewlineCharacter = '\n';
	static const char_type ReturnCharacter = '\r';

	static FxBool FX_CALL IsWhitespace(char_type c)
	{
		return c == SpaceCharacter || c == TabCharacter || c == NewlineCharacter || c == ReturnCharacter;
	}

};

/// Typedefs and functions for working with UNICODE characters and strings.
template<> struct FxCharTraits<FxWChar>
{
public:
	typedef FxWChar char_type;
	typedef FxAChar foreign_char_type;
	typedef FxCharTraits<FxAChar> other_traits_type;

	static FxSize FX_CALL Length( const char_type* str )
	{
		return FxWStrlen(str);
	}

	static char_type* FX_CALL Move(char_type* dest, const char_type* src, FxSize num)
	{
		return static_cast<char_type*>(FxWMemmove(dest, src, num));
	}

	static char_type* FX_CALL Copy(char_type* dest, const char_type* src, FxSize num)
	{
		return static_cast<char_type*>(FxWMemcpy(dest, src, num));
	}

	static const char_type* FX_CALL Find(const char_type* str, const char_type& c, FxSize num)
	{
		return static_cast<const char_type*>(FxWMemchr(str, c, num));
	}

	static const char_type* FX_CALL RFind(const char_type* str, const char_type& c)
	{
		return FxWStrrchr(str, c);
	}

	static FxInt32 FX_CALL Compare(const char_type* first, const char_type* second, FxSize num)
	{
		return FxWMemcmp(first, second, num);
	}

	static char_type* FX_CALL Convert(char_type* dest, const foreign_char_type* src, FxSize num)
	{
		// not even close to correct...
		FxSize i = 0;
		while( i < num )
		{
			dest[i] = fx_std(btowc)(src[i]);
			++i;
		}
		return dest;
	}

	static char_type FX_CALL ToUpper(char_type c)
	{
		return static_cast<char_type>(toupper(c));
	}

	static char_type FX_CALL ToLower(char_type c)
	{
		return static_cast<char_type>(tolower(c));
	}

	static const char_type SpaceCharacter = L' ';
	static const char_type TabCharacter = L'\t';
	static const char_type NewlineCharacter = L'\n';
	static const char_type ReturnCharacter = L'\r';

	static FxBool FX_CALL IsWhitespace(char_type c)
	{
		return c == SpaceCharacter || c == TabCharacter || c == NewlineCharacter || c == ReturnCharacter;
	}
};

} // namespace Face

} // namespace OC3Ent

#endif
