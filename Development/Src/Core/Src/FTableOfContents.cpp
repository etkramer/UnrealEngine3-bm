/*=============================================================================
	FTableOfContents.cpp
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"
#include "FTableOfContents.h"


/**
 * Constructor
 */
FTableOfContents::FTableOfContents()
{

}


/**
 * Parse the table of contents from a given buffer
 *
 * @param Buffer Buffer of text representing a TOC, zero terminated
 *
 * @return TRUE if successful
 */
UBOOL FTableOfContents::ParseFromBuffer(ANSICHAR* Buffer)
{
	// protect TOC access 
	FScopeLock ScopeLock(&TOCCriticalSection);

	// init traveling pointer
	ANSICHAR* Ptr = Buffer;

	// iterate over the lines until complete
	UBOOL bIsDone = FALSE;
	while (!bIsDone)
	{
		// Advance past new line characters
		while (*Ptr=='\r' || *Ptr=='\n')
		{
			Ptr++;
		}

		// Store the location of the first character of this line
		ANSICHAR* Start = Ptr;

		// Advance the char pointer until we hit a newline character
		while (*Ptr && *Ptr!='\r' && *Ptr!='\n')
		{
			Ptr++;
		}

		// If this is the end of the file, we're done
		if (*Ptr == 0)
		{
			bIsDone = 1;
		}

		// Terminate the current line, and advance the pointer to the next character in the stream
		*Ptr++ = 0;

		// parse the line of text
		INT FileSize, UncompressedFileSize, StartSector;
		TCHAR Filename[1024];
#if PS3
		appSSCANF(ANSI_TO_TCHAR(Start), TEXT("%d %d %d %ls"), &FileSize, &UncompressedFileSize, &StartSector, Filename);
#else
		appSSCANF(ANSI_TO_TCHAR(Start), TEXT("%d %d %d %s"), &FileSize, &UncompressedFileSize, &StartSector, Filename);
#endif
		// Don't allow overwriting existing entries.
		// This assumes ALL languages will be generated from the same content!!!
		FTOCEntry* ExistingEntry = Entries.Find(Filename);
		if (ExistingEntry == NULL)
		{
			// insert this entry into the map
			FTOCEntry& Entry = Entries.Set(Filename, FTOCEntry());
			Entry.FileSize = FileSize;
			Entry.UncompressedFileSize = UncompressedFileSize;
			Entry.StartSector = StartSector;
		}
	}

	return TRUE;
}


/**
 * Return the size of a file in the TOC
 * 
 * @param Filename Name of file to look up
 *
 * @return Size of file, or -1 if the filename was not found
 */
INT FTableOfContents::GetFileSize(const TCHAR* Filename)
{
	// protect TOC access so loading thread can use it
	FScopeLock ScopeLock(&TOCCriticalSection);

	//default to not found
	INT FileSize = -1;

	// lookup the file in the TOC
	FTOCEntry* Entry = Entries.Find(Filename);

	// if the file doesn't exist, return -1
	if (Entry)
	{
		// if the file was compressed, then return that size
		FileSize = Entry->FileSize;
	}

	return FileSize;
}


/**
 * Return the uncompressed size of a file in the TOC
 * 
 * @param Filename Name of file to look up
 *
 * @return Size of file, or -1 if the filename was not found
 */
INT FTableOfContents::GetUncompressedFileSize(const TCHAR* Filename)
{
	// protect TOC access so loading thread can use it
	FScopeLock ScopeLock(&TOCCriticalSection);

	//default to not found
	INT FileSize = -1;

	// lookup the file in the TOC
	FTOCEntry* Entry = Entries.Find(Filename);

	// if the file doesn't exist or wasn't compressed, return -1
	if (Entry && Entry->UncompressedFileSize != 0)
	{
		// if the file was compressed, then return that size
		FileSize = Entry->UncompressedFileSize;
	}

	return FileSize;
}


/**
 * Return the starting sector of a file in the TOC
 * 
 * @param Filename Name of file to look up
 *
 * @return Size of file, or -1 if the filename was not found
 */
INT FTableOfContents::GetFileStartSector(const TCHAR* Filename)
{
	// protect TOC access so loading thread can use it
	FScopeLock ScopeLock(&TOCCriticalSection);

	//default to not found
	INT StartSector = -1;

	// lookup the file in the TOC
	FTOCEntry* Entry = Entries.Find(Filename);

	// if the file doesn't exist or sector info isn't known, return -1
	if (Entry && Entry->StartSector != 0)
	{
		// if the file was on the DVD, return the sector
		StartSector = Entry->StartSector;
	}

	return StartSector;

}

/**
 * Finds files in the TOC based on a wildcard
 *
 * @param Result Resulting filename strings
 * @param Wildcard Wildcard to match against
 * @param bShouldFindFiles If TRUE, return files in Result
 * @param bShouldFindDirectories If TRUE, return directories in Result
 */
void FTableOfContents::FindFiles(TArray<FString>& Result, const TCHAR* Wildcard, UBOOL bShouldFindFiles, UBOOL bShouldFindDirectories)
{
	// protect TOC access so loading thread can use it
	FScopeLock ScopeLock(&TOCCriticalSection);

	// cache a ffilename version
	FFilename			FullWildcard(Wildcard);

	// get the start of the path
	FFilename			BasePath	= FullWildcard.GetPath() + TEXT("\\");
	// get the part of the wildcard after last slash, before . (like '*' in ..\Config\*.ini)
	FString				FileWildcard = FullWildcard.GetBaseFilename();

	// make sure we are just doing a *.* or *.EXT type search and
	// that the search start with .. (so it matches the TOC format) and
	// that we aren't searching for directories
	UBOOL bIsValidSearch = !bShouldFindDirectories && (FileWildcard == TEXT("*")) && (BasePath.Left(2) == TEXT(".."));

	if (bIsValidSearch)
	{
		// get the part after the .
		FString Ext = FullWildcard.GetExtension();
		// are we doing *.*?
		UBOOL bMatchAnyExtension = Ext == TEXT("*");

		// get the length of the string to match
		INT BasePathLen = BasePath.Len();
		
		// go through the TOC looking for matches
		for (TMap<FFilename, FTOCEntry>::TConstIterator It(Entries); It; ++It)
		{
			const FFilename& Filename = It.Key();
			// if the start of the TOC string matches the start of the wildcard search, check extension
			if (appStrnicmp(*Filename, *BasePath, BasePathLen) == 0)
			{
				// path matches, how about extension (since we know the filename without extension is using *)
				if (bMatchAnyExtension || Ext == Filename.GetExtension())
				{
					// we have a match!
					new(Result)FString(*Filename.GetCleanFilename());
				}
			}
		}
	}
	else
	{
		debugf(TEXT("Attempting invalid FindFiles in FTableOfContents with '%s'. It should be in the form '..\\Config\\*.ini, no directories'"), Wildcard);
	}

}

/**
 * Add a file to the TOC at runtime
 * 
 * @param Filename Name of the file
 * @param FileSize Size of the file
 * @param UncompressedFileSize Size of the file when uncompressed, defaults to 0
 * @param StartSector Sector on disk for the file, defaults to 0
 */
void FTableOfContents::AddEntry(const TCHAR* Filename, INT FileSize, INT UncompressedFileSize, INT StartSector)
{
	// insert this entry into the map
	FTOCEntry& Entry = Entries.Set(Filename, FTOCEntry());
	Entry.FileSize = FileSize;
	Entry.UncompressedFileSize = UncompressedFileSize;
	Entry.StartSector = StartSector;
}
