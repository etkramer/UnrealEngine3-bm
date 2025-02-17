//------------------------------------------------------------------------------
// ANSI low-level platform wrappers.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxPlatformANSI_H__
#define FxPlatformANSI_H__

//@todo Move this to FxPlatformPS3.h.
#ifdef PS3
	#include "stdlib.h"
	#include "string.h"
	#include "wchar.h"
	#include "math.h"
	#include "ctype.h" // Needed for toupper, tolower in char traits.
	#include "stdio.h" // Needed for sprintf in FxFtoa.
#endif

#include <cstring>
#include <cstdlib>
#include <climits>
#include <cfloat>
#include <cassert>
#include <memory>
#include <cmath>  // Needed for log10 in Itoa.
#include <cctype> // Needed for toupper, tolower in char traits.
#include <cstdio> // Needed for sprintf in FxFtoa.

#ifdef __UNREAL__
        #if FINAL_RELEASE
                #define FxAssert(x) ((void)0)
        #else
                #define FxAssert assert
        #endif
#else 
        #define FxAssert assert
#endif // __UNREAL__

#define FX_REAL_EPSILON FLT_EPSILON
#define FX_REAL_MAX     FLT_MAX
#define FX_REAL_MIN     FLT_MIN
#define FX_INT32_MAX    INT_MAX
#define FX_INT32_MIN    INT_MIN
#define FX_BYTE_MAX     UCHAR_MAX
#define FX_CHAR_BIT     CHAR_BIT

namespace OC3Ent
{

namespace Face
{

/// Wrapper for memcpy().
FX_INLINE void* FX_CALL FxMemcpy( void* dest, const void* src, FxSize cnt )
{
#ifdef FX_XBOX_360
	return XMemCpy(dest, src, cnt);
#else
	return fx_std(memcpy)(dest, src, cnt);
#endif // FX_XBOX_360
}

/// Wrapper for memmove().
FX_INLINE void* FX_CALL FxMemmove( void* dest, const void* src, FxSize numBytes )
{
	return fx_std(memmove)(dest, src, numBytes);
}

/// Wrapper for memset().
FX_INLINE void* FX_CALL FxMemset( void* dest, FxInt32 val, FxSize cnt )
{
#ifdef FX_XBOX_360
	return XMemSet(dest, val, cnt);
#else
	return fx_std(memset)(dest, val, cnt);
#endif // FX_XBOX_360
}

/// Wrapper for memcmp().
FX_INLINE FxInt32 FX_CALL FxMemcmp( const void* mem1, const void* mem2, 
								   FxSize cnt )
{
	return fx_std(memcmp)(mem1, mem2, cnt);
}

/// Wrapper for memchr().
FX_INLINE const void* FX_CALL FxMemchr(const void* buffer, FxInt32 val, FxSize searchLength)
{
	return fx_std(memchr)(buffer, val, searchLength);
}

/// Wrapper for strcpy().
FX_INLINE FxChar* FX_CALL FxStrcpy( FxChar* dest, FxSize destLen, const FxChar* src )
{
#if _MSC_VER >= 1400
	strcpy_s(dest, destLen, src);
	return dest;
#else
	// Shut up non-Microsoft compilers.
	destLen = 0;
	return fx_std(strcpy)(dest, src);
#endif
}

/// Wrapper for strncpy().
FX_INLINE FxChar* FX_CALL FxStrncpy( FxChar* dest, FxSize destLen, const FxChar* src, FxSize n )
{
#if _MSC_VER >= 1400
	n;
	strncpy_s(dest, destLen, src, _TRUNCATE);
	return dest;
#else
	// Shut up non-Microsoft compilers.
	destLen = 0;
	return fx_std(strncpy)(dest, src, n);
#endif
}

/// Wrapper for strlen().
FX_INLINE FxSize FX_CALL FxStrlen( const FxChar* str )
{
	return (FxSize)fx_std(strlen)(str);
}

/// Wrapper for strcmp().
FX_INLINE FxInt32 FX_CALL FxStrcmp( const FxChar* str1, const FxChar* str2 )
{
	return fx_std(strcmp)(str1, str2);
}

/// Wrapper for strcat().
FX_INLINE FxChar* FX_CALL FxStrcat( FxChar* dest, FxSize destLen, const FxChar* src )
{
#if _MSC_VER >= 1400
	strcat_s(dest, destLen, src);
	return dest;
#else
	// Shut up non-Microsoft compilers.
	destLen = 0;
	return fx_std(strcat)(dest, src);
#endif
}

/// Wrapper for strrchr().
FX_INLINE const FxChar* FX_CALL FxStrrchr( const FxChar* str, FxChar val )
{
	return fx_std(strrchr)(str, val);
}

/// Wrapper for strstr().
FX_INLINE const FxChar* FX_CALL FxStrstr( const FxChar* str, const FxChar* substr )
{
	return fx_std(strstr)(str, substr);
}

/// Wrapper for wmemcpy().
FX_INLINE FxWChar* FX_CALL FxWMemcpy( FxWChar* dest, const FxWChar* src, FxSize numChars )
{
	return fx_std(wmemcpy)(dest, src, numChars);
}

/// Wrapper for wmemmove().
FX_INLINE FxWChar* FX_CALL FxWMemmove( FxWChar* dest, const FxWChar* src, FxSize numChars )
{
	return fx_std(wmemmove)(dest, src, numChars);
}

/// Wrapper for wmemset().
FX_INLINE FxWChar* FX_CALL FxWMemset( FxWChar* dest, FxWChar val, FxSize numChars )
{
	return fx_std(wmemset)(dest, val, numChars);
}

/// Wrapper for wmemchr().
FX_INLINE const FxWChar* FX_CALL FxWMemchr( const FxWChar* buffer, FxWChar val, FxSize searchLength )
{
	return fx_std(wmemchr)(buffer, val, searchLength);
}

/// Wrapper for wmemcmp().
FX_INLINE FxInt32 FX_CALL FxWMemcmp( const FxWChar* first, const FxWChar* second, FxSize length )
{
	return fx_std(wmemcmp)(first, second, length);
}

/// Wrapper for wcslen().
FX_INLINE FxSize FX_CALL FxWStrlen( const FxWChar* str )
{
	return static_cast<FxSize>(fx_std(wcslen)(str));
}

/// Wrapper for wcsrchr().
FX_INLINE const FxWChar* FX_CALL FxWStrrchr( const FxWChar* str, FxWChar val )
{
	return fx_std(wcsrchr)(str, val);
}

/// Wrapper for wcsstr().
FX_INLINE const FxWChar* FX_CALL FxWStrstr( const FxWChar* str, const FxWChar* substr )
{
	return fx_std(wcsstr)(str, substr);
}

/// Wrapper for atof().
FX_INLINE FxDReal FX_CALL FxAtof( const FxChar* str )
{
	return fx_std(atof)(str);
}

/// Wrapper for atoi().
FX_INLINE FxInt32 FX_CALL FxAtoi( const FxChar* str )
{
	return fx_std(atoi)(str);
}

/// Converts a 32-bit integer to a string.
FX_INLINE FxChar* FX_CALL FxItoa( FxInt32 val, FxChar* buf, FxSize bufLen )
{
	FxChar buffer[64] = {0};
	if( val == 0 )
	{
		buffer[0] = '0';
	}
	else
	{
		FxBool needsNegative = val < 0;
		val = fx_std(abs)(val);
		FxInt32 numLen = static_cast<FxInt32>(fx_std(log10)(static_cast<FxReal>(val))) + (needsNegative ? 1 : 0);
		FxInt32 cnt = 0;
		while( val != 0 )
		{
			buffer[numLen - cnt] = static_cast<FxChar>((val % 10) + 48);
			val = val / 10;
			++cnt;
		}
		if( needsNegative )
		{
			buffer[0] = '-';
		}
	}
	return FxStrcpy(buf, bufLen, buffer);
}

/// Converts a floating-point number to a string.
FX_INLINE FxChar* FX_CALL FxFtoa( FxReal val, FxChar* buf )
{
	//@todo revise.
	FxChar buffer[64] = {0};
#if _MSC_VER >= 1400
	sprintf_s(buffer, 64, "%f", val);
#else
	sprintf(buffer, "%f", val);
#endif
	return FxStrcpy(buf, 64, buffer);
}

} // namespace Face

} // namespace OC3Ent

#endif
