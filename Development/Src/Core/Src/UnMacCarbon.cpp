/*=============================================================================
	UnMacCarbon.cpp: Mac OS X support routines that reference the Carbon API.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"

// !!! FIXME: would be nice to clean up the header conficts some day...

// Please note that most of the Mac OS X codepath aligns with the Unix
//  routines, but occasionally we need something that's in a Mac-specific
//  header, which tends to define things that conflict with Unreal's
//  headers, so we try to implement small pieces here and hook into them
//  from the larger pieces of Unreal source code.

// Only compile this on Mac OS X, but PLATFORM_MACOSX won't be defined here...
#if (defined(__APPLE__) && defined(__MACH__))

// Man, you can't avoid header conflicts with the precompiled headers!
#define FVector MACOSX_FVector

#include <Carbon/Carbon.h>

// Return the prefpath as a POSIX path in UTF-8 encoding, or NULL if it
//  couldn't determine it. Caller must delete[] the retval.
char *appMacGetBasePrefPath()
{
	char *RetVal = NULL;
	FSRef FolderRef;
	OSErr err = FSFindFolder(kUserDomain, kApplicationSupportFolderType, kCreateFolder, &FolderRef);
	if (err == noErr)
	{
		CFURLRef FolderURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &FolderRef);
		if (FolderURL != NULL)
		{
			CFStringRef FolderPath = CFURLCopyFileSystemPath(FolderURL, kCFURLPOSIXPathStyle);
			CFRelease(FolderURL);
			if (FolderPath != NULL)
			{
				const CFIndex Len = CFStringGetMaximumSizeForEncoding(CFStringGetLength(FolderPath) + 1, kCFStringEncodingUTF8);
				RetVal = new char[Len];
				if (!CFStringGetCString(FolderPath, RetVal, Len, kCFStringEncodingUTF8))
				{
					delete[] RetVal;
					RetVal = NULL;
				}
				CFRelease(FolderPath);
			}
		}
	}
	return RetVal;
}

int appMacWideStricmp(const wchar_t *a, const wchar_t *b)
{
// !!! FIXME: These don't seem to like UTF32...bug in OS X 10.4?
#if 0
	int RetVal = 0;
	CFStringRef CfStrA = CFStringCreateWithBytes(NULL, (const UInt8 *) a,
	                                             wcslen(a) * sizeof (wchar_t),
	                                             kCFStringEncodingUTF32, 0);
	if (CfStrA != NULL)
	{
		CFStringRef CfStrB = CFStringCreateWithBytes(NULL, (const UInt8 *) b,
		                                             wcslen(b) * sizeof (wchar_t),
		                                             kCFStringEncodingUTF32, 0);
		if (CfStrB != NULL)
		{
			RetVal = (int) CFStringCompare(CfStrA, CfStrB, kCFCompareCaseInsensitive);
			CFRelease(CfStrB);
		}
		CFRelease(CfStrA);
	}
	return RetVal;

#else
	int RetVal = 0;

	UniChar *UniA = (UniChar *) alloca((wcslen(a) + 1) * sizeof (UniChar));
	UniChar *UniB = (UniChar *) alloca((wcslen(b) + 1) * sizeof (UniChar));
	for (int i = 0; a[i]; i++) { UniA[i] = (UniChar) a[i]; }
	for (int i = 0; b[i]; i++) { UniB[i] = (UniChar) b[i]; }

	CFStringRef CfStrA = CFStringCreateWithBytes(NULL, (const UInt8 *) UniA,
	                                             wcslen(a) * sizeof (UniChar),
	                                             kCFStringEncodingUnicode, 0);
	if (CfStrA != NULL)
	{
		CFStringRef CfStrB = CFStringCreateWithBytes(NULL, (const UInt8 *) UniB,
		                                             wcslen(b) * sizeof (UniChar),
		                                             kCFStringEncodingUnicode, 0);
		if (CfStrB != NULL)
		{
			RetVal = (int) CFStringCompare(CfStrA, CfStrB, kCFCompareCaseInsensitive);
			CFRelease(CfStrB);
		}
		CFRelease(CfStrA);
	}
	return RetVal;
#endif
}

int appMacWideStrnicmp(const wchar_t *a, const wchar_t *b, size_t n)
{
// !!! FIXME: These don't seem to like UTF32...bug in OS X 10.4?
#if 0
	int RetVal = 0;
	size_t MaxLen = wcslen(a);
	if (MaxLen > n)
	{
		MaxLen = n;
	}
	CFStringRef CfStrA = CFStringCreateWithBytes(NULL, (const UInt8 *) a,
	                                             MaxLen * sizeof (wchar_t),
	                                             kCFStringEncodingUTF32, 0);
	if (CfStrA != NULL)
	{
		MaxLen = wcslen(b);
		if (MaxLen > n)
		{
			MaxLen = n;
		}
		CFStringRef CfStrB = CFStringCreateWithBytes(NULL, (const UInt8 *) b,
		                                             MaxLen * sizeof (wchar_t),
		                                             kCFStringEncodingUTF32, 0);
		if (CfStrB != NULL)
		{
			RetVal = (int) CFStringCompare(CfStrA, CfStrB, kCFCompareCaseInsensitive);
			CFRelease(CfStrB);
		}
		CFRelease(CfStrA);
	}
	return RetVal;
#else
	int RetVal = 0;
	size_t MaxLen = wcslen(a);
	if (MaxLen > n)
	{
		MaxLen = n;
	}

	UniChar *UniA = (UniChar *) alloca((wcslen(a) + 1) * sizeof (UniChar));
	UniChar *UniB = (UniChar *) alloca((wcslen(b) + 1) * sizeof (UniChar));
	for (int i = 0; a[i]; i++) { UniA[i] = (UniChar) a[i]; }
	for (int i = 0; b[i]; i++) { UniB[i] = (UniChar) b[i]; }

	CFStringRef CfStrA = CFStringCreateWithBytes(NULL, (const UInt8 *) UniA,
	                                             MaxLen * sizeof (UniChar),
	                                             kCFStringEncodingUnicode, 0);
	if (CfStrA != NULL)
	{
		MaxLen = wcslen(b);
		if (MaxLen > n)
		{
			MaxLen = n;
		}
		CFStringRef CfStrB = CFStringCreateWithBytes(NULL, (const UInt8 *) UniB,
		                                             MaxLen * sizeof (UniChar),
		                                             kCFStringEncodingUnicode, 0);
		if (CfStrB != NULL)
		{
			RetVal = (int) CFStringCompare(CfStrA, CfStrB, kCFCompareCaseInsensitive);
			CFRelease(CfStrB);
		}
		CFRelease(CfStrA);
	}
	return RetVal;
#endif
}

#endif  // PLATFORM_MACOSX

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

