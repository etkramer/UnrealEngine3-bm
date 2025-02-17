/*=============================================================================
	UnFile.cpp: ANSI C core.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"

#include <time.h>
#include <stdio.h>
#include <stdarg.h>

/*-----------------------------------------------------------------------------
	Time.
-----------------------------------------------------------------------------*/

#if _MSC_VER
/**
* Get the system date
* 
* @param Dest - destination buffer to copy to
* @param DestSize - size of destination buffer in characters
* @return date string
*/
inline TCHAR* appStrDate( TCHAR* Dest, SIZE_T DestSize )
{
#if USE_SECURE_CRT
	return (TCHAR*)_tstrdate_s(Dest,DestSize);
#else
	return (TCHAR*)_tstrdate(Dest);
#endif
}

/**
* Get the system time
* 
* @param Dest - destination buffer to copy to
* @param DestSize - size of destination buffer in characters
* @return time string
*/
inline TCHAR* appStrTime( TCHAR* Dest, SIZE_T DestSize )
{
#if USE_SECURE_CRT
	return (TCHAR*)_tstrtime_s(Dest,DestSize);
#else
	return (TCHAR*)_tstrtime(Dest);
#endif
}
#endif

/** 
* Returns string timestamp.  NOTE: Only one return value is valid at a time! 
*/
const TCHAR* appTimestamp()
{
	static TCHAR Result[1024];
	*Result = 0;
#if _MSC_VER
	//@todo gcc: implement appTimestamp (and move it into platform specific .cpp
	appStrDate( Result, ARRAY_COUNT(Result) );
	appStrcat( Result, TEXT(" ") );
	appStrTime( Result + appStrlen(Result), ARRAY_COUNT(Result) - appStrlen(Result) );
#endif
	return Result;
}

/*-----------------------------------------------------------------------------
	Memory functions.
-----------------------------------------------------------------------------*/

INT appMemcmp( const void* Buf1, const void* Buf2, INT Count )
{
	return memcmp( Buf1, Buf2, Count );
}

UBOOL appMemIsZero( const void* V, int Count )
{
	BYTE* B = (BYTE*)V;
	while( Count-- > 0 )
		if( *B++ != 0 )
			return 0;
	return 1;
}

void* appMemmove( void* Dest, const void* Src, INT Count )
{
	return memmove( Dest, Src, Count );
}


/*-----------------------------------------------------------------------------
	String functions.
-----------------------------------------------------------------------------*/

/** 
* Copy a string with length checking. Behavior differs from strncpy in that last character is zeroed. 
*
* @param Dest - destination buffer to copy to
* @param Src - source buffer to copy from
* @param MaxLen - max characters in the buffer (including null-terminator)
* @return pointer to resulting string buffer
*/
TCHAR* appStrncpy( TCHAR* Dest, const TCHAR* Src, INT MaxLen )
{
	check(MaxLen>0);
#if USE_SECURE_CRT
	// length of string must be strictly < total buffer length so use (MaxLen-1)
	_tcsncpy_s(Dest,MaxLen,Src,MaxLen-1);	
#else
	// length of string includes null terminating character so use (MaxLen)
	_tcsncpy(Dest,Src,MaxLen);
	Dest[MaxLen-1]=0;
#endif
	return Dest;
}

/** 
* Concatenate a string with length checking.
*
* @param Dest - destination buffer to append to
* @param Src - source buffer to copy from
* @param MaxLen - max length of the buffer (including null-terminator)
* @return pointer to resulting string buffer
*/
TCHAR* appStrncat( TCHAR* Dest, const TCHAR* Src, INT MaxLen )
{
	INT Len = appStrlen(Dest);
	TCHAR* NewDest = Dest + Len;
	if( (MaxLen-=Len) > 0 )
	{
		appStrncpy( NewDest, Src, MaxLen );
	}
	return Dest;
}

/** 
* Copy a string with length checking. Behavior differs from strncpy in that last character is zeroed. 
* (ANSICHAR version) 
*
* @param Dest - destination char buffer to copy to
* @param Src - source char buffer to copy from
* @param MaxLen - max length of the buffer (including null-terminator)
* @return pointer to resulting string buffer
*/
ANSICHAR* appStrncpyANSI( ANSICHAR* Dest, const ANSICHAR* Src, INT MaxLen )
{
#if USE_SECURE_CRT	
	// length of string must be strictly < total buffer length so use (MaxLen-1)
	strncpy_s(Dest,MaxLen,Src,MaxLen-1);
#else
	strncpy(Dest,Src,MaxLen);
	// length of string includes null terminating character so use (MaxLen)
	Dest[MaxLen-1]=0;
#endif
	return Dest;
}

/** 
* Standard string formatted print. 
* @warning: make sure code using appSprintf allocates enough (>= MAX_SPRINTF) memory for the destination buffer
*/
VARARG_BODY( INT, appSprintf, const TCHAR*, VARARG_EXTRA(TCHAR* Dest) )
{
	INT	Result = -1;
	GET_VARARGS_RESULT( Dest, MAX_SPRINTF, MAX_SPRINTF-1, Fmt, Fmt, Result );
	return Result;
}
/**
* Standard string formatted print (ANSI version).
* @warning: make sure code using appSprintf allocates enough (>= MAX_SPRINTF) memory for the destination buffer
*/
VARARG_BODY( INT, appSprintfANSI, const ANSICHAR*, VARARG_EXTRA(ANSICHAR* Dest) )
{
	INT	Result = -1;
	GET_VARARGS_RESULT_ANSI( Dest, MAX_SPRINTF, MAX_SPRINTF-1, Fmt, Fmt, Result );
	return Result;
}

/*-----------------------------------------------------------------------------
	Sorting.
-----------------------------------------------------------------------------*/

void appQsort( void* Base, INT Num, INT Width, int(CDECL *Compare)(const void* A, const void* B ) )
{
	qsort( Base, Num, Width, Compare );
}


