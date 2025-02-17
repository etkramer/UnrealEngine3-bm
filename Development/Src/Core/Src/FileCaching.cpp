/*=============================================================================
    FileCaching.cpp: Harddisk file caching.
    Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"
#include "FileCaching.h"


// value in the mapped chunks array of an unmapped chunk
#define UNMAPPED_CHUNK 0xFFFF

// Number of seconds between each metadata save (minimum)
#define TIME_BETWEEN_METADATA_SAVES	20.0


/** Global HDD Cache Manager object */
FHDDCacheManager* GHDDCacheManager = NULL;


/** Default constructor */
FHDDCacheManager::FHDDCacheManager()
:	bPreferencesInitialized(FALSE)
,	bEnableCaching(FALSE)
{
}

/** Virtual destructor */
FHDDCacheManager::~FHDDCacheManager()
{
}

/** Initializer function, to be called right after construction. */
void FHDDCacheManager::Initialize()
{
	UBOOL bClearCache = ParseParam(appCmdLine(), TEXT("ClearHDDCache"));
	bEnableCaching = bEnableCaching && (ParseParam(appCmdLine(), TEXT("DisableHDDCache")) == FALSE);
	if ( bEnableCaching )
	{
		ValidateCache( bClearCache );
	}
}

/**
 *	Programmatically adds cache preferences for all files matching the specified pattern.
 *	@param	FilenamePattern		Filename pattern for match files to the specified caching mechanism
 *	@param	CacheType			Cache mechanism to use
 **/
void FHDDCacheManager::AddCachePreferences( const TCHAR* Filename, EFileCacheType CacheType )
{
	FScopeLock ScopeLock(&FileCacheLock);

	// Using wildcard?
	TCHAR* Wildcard = appStrchr( Filename, TEXT('*') );
	if ( Wildcard )
	{
		INT WildcardPosition = INT(Wildcard - Filename);
		WildcardPreferences.Set( FWildCardPattern(Filename,WildcardPosition), CacheType );
	}
	else
	{
		Preferences.Set( Filename, CacheType );
	}
}

/**
 *	Returns the filepath prefix to access the HDD cache (e.g. "cache:"), not containing file path separator.
 *	@return						Filepath prefix to access the HDD cache
 */
const FString& FHDDCacheManager::GetFileCacheRoot( ) const
{
	return FileCacheRoot;
}

/**
 *	Initializes the preference settings from the .ini file.
 **/
void FHDDCacheManager::InitPreferences()
{
	// We don't want to check the ini before the coalesced ini has been read in :)
	if ( GConfig && GConfig->Num() > 0 )
	{
		// this can return NULL, in which case we are still initialized
		TMultiMap<FString,FString>* Section = GConfig->GetSectionPrivate(TEXT("HDDCachePreferences"), FALSE, TRUE, GEngineIni);
		if ( Section )
		{
			for ( TMultiMap<FString,FString>::TConstIterator SecIt(*Section); SecIt; ++SecIt )
			{
				const FString& FilenamePattern = SecIt.Value();
				INT WildcardPosition = FilenamePattern.InStr( TEXT("*") );
				if ( WildcardPosition != INDEX_NONE )
				{
					if ( SecIt.Key() == TEXT("MappedCache") )
					{
						WildcardPreferences.Set( FWildCardPattern(FilenamePattern,WildcardPosition), FCT_MappedCache );
					}
					else if ( SecIt.Key() == TEXT("OneDirectionalCache") )
					{
						WildcardPreferences.Set( FWildCardPattern(FilenamePattern,WildcardPosition), FCT_OneDirectionalCache );
					}
					else if ( SecIt.Key() == TEXT("NotCached") )
					{
						WildcardPreferences.Set( FWildCardPattern(FilenamePattern,WildcardPosition), FCT_NotCached );
					}
				}
				else if ( SecIt.Key() == TEXT("MappedCache") )
				{
					Preferences.Set( FilenamePattern, FCT_MappedCache );
				}
				else if ( SecIt.Key() == TEXT("OneDirectionalCache") )
				{
					Preferences.Set( FilenamePattern, FCT_OneDirectionalCache );
				}
				else if ( SecIt.Key() == TEXT("NotCached") )
				{
					Preferences.Set( FilenamePattern, FCT_NotCached );
				}
			}
		}
		bPreferencesInitialized = TRUE;
	}
}

/**
 *	Determines whether the file path refers to a partition or folder that is allowed to be cached.
 *	@param FilePath	- Path to the file that we're considering for caching.
 *	@return			- TRUE if the file is contained in a path that is allowed to be cached
 */
UBOOL FHDDCacheManager::ShouldCachePath( const TCHAR* FilePath )
{
	return TRUE;
}

/**
 *	Determines what cache mechanism to use for the specified file.
 *	@param	Filename		Unreal filename for the source file
 *	@return					Cache mechanism to use for the file
 **/
FHDDCacheManager::EFileCacheType FHDDCacheManager::GetCacheType(const TCHAR* Filename )
{
	FScopeLock ScopeLock(&FileCacheLock);

	if ( bEnableCaching == FALSE )
	{
		return FCT_NotCached;
	}

	// Is the path referring to a partition or folder that should never be cached?
	if ( ShouldCachePath(Filename) == FALSE )
	{
		return FCT_NotCached;
	}

	// get just the filename without the path
	FFilename CleanFilename = FFilename(Filename).GetCleanFilename();

	// initialize the no cache list if we haven't already
	if ( bPreferencesInitialized == FALSE )
	{
		InitPreferences();
	}

	// Do we have cache preferences?
	if (bPreferencesInitialized)
	{
		// Look for the explicit filename in the preferences.
		const EFileCacheType* CacheType = Preferences.Find(*CleanFilename);
		if ( CacheType )
		{
			return *CacheType;
		}

		// Look for file extension entries.
		for ( TMap<FWildCardPattern, EFileCacheType>::TConstIterator PatternIt(WildcardPreferences); PatternIt; ++PatternIt )
		{
			const FWildCardPattern& Pattern = PatternIt.Key();

			// The source filename must be at least the length of the pattern (excluding the wildcard character).
			if ( CleanFilename.Len() >= (Pattern.Pattern.Len() - 1) )
			{
				// Does the left side match?
				if ( appStrnicmp(*CleanFilename, *Pattern.Pattern, Pattern.WildCardPosition ) == 0 )
				{
					// Does the right side match?
					INT RightSideLength = Pattern.Pattern.Len() - Pattern.WildCardPosition - 1;
					INT Offset1 = CleanFilename.Len() - RightSideLength;
					INT Offset2 = Pattern.Pattern.Len() - RightSideLength;
					if ( appStricmp(*CleanFilename + Offset1, *Pattern.Pattern + Offset2) == 0 )
					{
						return PatternIt.Value();
					}
				}
			}
		}
	}

	// Default is "Not Cached".
	return FCT_NotCached;
}

/**
 *	Returns a HDD cache object for the specified file.
 *	@param	Filename		Unreal filename for the source file
 *	@param	OverrideMappedChunkSize	[Optional] Size of each cached chunk, in bytes (only used for Mapped caches)
 *	@return					New cache object, representing both the source file and the cached version of file
 */
FSideBySideCache* FHDDCacheManager::GetCache(const TCHAR* Filename, INT OverrideMappedChunkSize /*= DVD_ECC_BLOCK_SIZE*/)
{
	FSideBySideCache* NewCache = NULL;
	EFileCacheType CacheType = GetCacheType(Filename);

	switch (CacheType)
	{
		case FCT_MappedCache:
			NewCache = CreateMappedCache( Filename, OverrideMappedChunkSize );
			break;
		case FCT_OneDirectionalCache:
			NewCache = CreateOneDirectionalCache( Filename, TRUE );
			break;
		case FCT_NotCached:
		default:
			NewCache = CreateOneDirectionalCache( Filename, FALSE );
	}

	if (NewCache && NewCache->Initialize() == FALSE)
	{
		delete NewCache;
		return NULL;
	}

	return NewCache;
}


/**
 *	Creates a platform-specific mapped cache object.
 *	@param	Filename		Unreal filename for the source file
 *	@param	OverrideMappedChunkSize	[Optional] Size of each cached chunk, in bytes
 *	@return					New mapped cache object, representing both the source file and the cached version of file
 **/
FMappedCache* FHDDCacheManager::CreateMappedCache( const TCHAR* Filename, INT OverrideMappedChunkSize /*= DVD_ECC_BLOCK_SIZE*/ )
{
	return new FMappedCache( Filename, OverrideMappedChunkSize, TRUE );
}

/**
 *	Creates a platform-specific one-directional cache object.
 *	@param InFilename	Unreal filename for the file this represents
 *	@param bShouldCache	Whether we should actually be caching or not
 **/
FOneDirectionCache* FHDDCacheManager::CreateOneDirectionalCache( const TCHAR* Filename, UBOOL bShouldCache )
{
	return new FOneDirectionCache( Filename, bShouldCache );
}

/**
 *	Make sure that all cache files has matching files and delete invalid files.
 *	@param bClearCache	Whether to clear the whole HDD cache
 */
void FHDDCacheManager::ValidateCache( UBOOL bClearCache )
{
	// Get all files on the HDD cache
	TArray<FString> Results;
	appFindFilesInDirectory(Results, *FileCacheRoot, TRUE, TRUE);

	// 0 means unknown, 1 means valid, -1 means invalid
	TArray<INT> ValidatedFiles;
	ValidatedFiles.AddZeroed( Results.Num() );

	INT NumTotalFiles = Results.Num();
	INT NumMappedFiles = 0;
	INT NumOneDirectionalFiles = 0;
	INT NumFilesDeleted = 0;

	// Validate all files
	for (INT FileIndex = 0; FileIndex < Results.Num(); FileIndex++)
	{
		// Already validated?
		if ( ValidatedFiles(FileIndex) != 0 )
		{
			continue;
		}

		// Check for valid cached files.
		UBOOL ValidFile = FALSE;
		INT CacheIndex = INDEX_NONE;
		INT TimeIndex = INDEX_NONE;
		const FString& Filename = Results(FileIndex);
		if ( appStricmp(*Filename + Max(0, Filename.Len()-6), TEXT("__time")) == 0 )
		{
			FString CleanFilename = Filename.Left(Filename.Len() - 6);
			UBOOL bFound1 = Results.FindItem( CleanFilename, CacheIndex );
			UBOOL bFound2 = Results.FindItem( CleanFilename + TEXT("__metx"), TimeIndex );
			ValidFile = bFound1;
			NumMappedFiles += (bFound1 && bFound2) ? 1 : 0;
			NumOneDirectionalFiles += (bFound1 && !bFound2) ? 1 : 0;

			// Make sure the indices are still INDEX_NONE if FindItem fails. Older implementations of FindItem set those variables anyway.
			if (!bFound1)
			{
				CacheIndex = INDEX_NONE;
			}
			if (!bFound2)
			{
				TimeIndex = INDEX_NONE;
			}
		}

		// Mark the files
		ValidatedFiles(FileIndex) = ValidFile ? 1 : -1;
		if ( CacheIndex != INDEX_NONE )
		{
			ValidatedFiles(CacheIndex) = ValidFile ? 1 : -1;
		}
		if ( TimeIndex != INDEX_NONE )
		{
			ValidatedFiles(TimeIndex) = ValidFile ? 1 : -1;
		}
	}

	// Delete invalid files.
	for (INT FileIndex = 0; FileIndex < Results.Num(); FileIndex++)
	{
		INT ValidationCode = ValidatedFiles(FileIndex) ;
		const FString& Filename = Results(FileIndex);
		if ( bClearCache || ValidationCode != 1 )
		{
			INT FileSize = GFileManager->FileSize( *Filename );
			debugf( NAME_DevHDDCaching, TEXT("Deleting invalid file: %s (%.3f MB)"), *Filename, (FileSize >= 0) ? DOUBLE(FileSize)/1024.0/1024.0 : 0.0 );
			NumFilesDeleted += GFileManager->Delete( *Filename ) ? 1 : 0;
		}
	}

	debugf( NAME_DevHDDCaching, TEXT("Cache contained %d files (%d Mapped, %d OneDirectional). %d invalid files were deleted."), NumTotalFiles, NumMappedFiles, NumOneDirectionalFiles, NumFilesDeleted );

	UBOOL bDumpCacheInfo = ParseParam(appCmdLine(), TEXT("ShowHDDCache"));
	if ( bDumpCacheInfo )
	{
		DumpCacheInfo();
	}
}

/**
 *	Dump all cache info to the log.
 */
void FHDDCacheManager::DumpCacheInfo( )
{
	// Get all files on the HDD cache
	TArray<FString> Results;
	appFindFilesInDirectory(Results, *FileCacheRoot, TRUE, TRUE);
	debugf( NAME_DevHDDCaching, TEXT("All files on the HDD Cache (%s):"), *GetFileCacheRoot() );
	for (INT FileIndex = 0; FileIndex < Results.Num(); FileIndex++)
	{
		const FString& Filename = Results(FileIndex);
		INT FileSize = GFileManager->FileSize( *Filename );
		debugf( NAME_DevHDDCaching, TEXT("%s (%.3f MB)"), *Filename, (FileSize >= 0) ? DOUBLE(FileSize)/1024.0/1024.0 : 0.0 );
	}
}


/**
 * Check if the HD file exists and is as new as the source file. If not, it will create the file time
 * side file.
 *
 * @param Filename				Source file
 * @param bIsHDDFileGood		[out] Is hard drive file pre-existant and usable
 * @param bIsHDDFileUpToDate	[out] Is hard drive file up to date
 * @param bIsTimestampFileGood	[out] Is time stamp file now-existant and usable
 * @return						Total size of the file in bytes, or -1 on error
 */
INT FHDDCacheManager::CheckFiles(const TCHAR* Filename, UBOOL& bIsHDDFileGood, UBOOL& bIsHDDFileUpToDate, UBOOL& bIsTimestampFileValid)
{
	// Set the values to defaults
	INT SourceFileSize = -1;
	bIsHDDFileGood = FALSE;
	bIsHDDFileUpToDate = FALSE;
	bIsTimestampFileValid = FALSE;

	// check source file
	DOUBLE SourceTimestamp = GFileManager->GetFileTimestamp(Filename);
	SourceFileSize = GFileManager->FileSize(Filename);

	// Return if required, or if there's something wrong with the source file.
	if (GIsRequestingExit || SourceFileSize == -1 || SourceTimestamp <= 0.001 )
	{
		return -1;
	}

	// check hard drive file
	FString CachedFilepath = GetCachedFilepath(Filename);
	DOUBLE HDDTimestamp = GFileManager->GetFileTimestamp(*CachedFilepath);
	bIsHDDFileGood = (HDDTimestamp > 0.0);

	// check for timestamp file
	FString TimestampFilename = CachedFilepath;
	TimestampFilename += TEXT("__time");
	DOUBLE TimeStampFile = GFileManager->GetFileTimestamp(*TimestampFilename);

	// If the HDD file exist and the timestamp file has the same time as the source file, then it's up to date.
	if ( bIsHDDFileGood && Abs<DOUBLE>(TimeStampFile - SourceTimestamp) < 0.001 )
	{
		bIsHDDFileUpToDate = TRUE;
		bIsTimestampFileValid = TRUE;
	}

	// if the HD file is out of date, delete it and recreate the timestamp file
	if ( bIsHDDFileUpToDate == FALSE )
	{
		debugf( NAME_DevHDDCaching, TEXT("File [%s] is not up to date [%f vs %f], caching to HD..."), *CachedFilepath, TimeStampFile, SourceTimestamp );

		GFileManager->Delete( *CachedFilepath );

		// Create the timestamp file (creates the directory structure automatically)
		FFileHandle FileHandle = GFileManager->FileOpen( *TimestampFilename, IO_WRITE );
		if ( FileHandle.IsValid() )
		{
			// Close it and manually set the timestamp to match the source file.
			GFileManager->FileClose( FileHandle );
			bIsTimestampFileValid = GFileManager->SetFileTimestamp( *TimestampFilename, SourceTimestamp );
		}
		else
		{
			debugf( NAME_DevHDDCaching, TEXT("ERROR - Failed to create timestamp file [%s]!\n"), *TimestampFilename );
		}
	}

	return SourceFileSize;
}

/**
 * Returns the full path to the cached version of the file
 *
 * @param Filename	Source file
 * @return			Full path of the cached file
 */
FString FHDDCacheManager::GetCachedFilepath(const TCHAR* Filename)
{
	FString CachedFile = GetFileCacheRoot();
	CachedFile += PATH_SEPARATOR;
	CachedFile += Filename;
	return CachedFile;
}

/*=============================================================================
    FSideBySideCache: Base class representing a file cached to HDD
=============================================================================*/

/** Next unique ID to assign */
QWORD FSideBySideCache::GSideBySideNextID = 0;

/** Critical section for hard drive caching access so that two threads don't fight over the BD */
FCriticalSection FSideBySideCache::SideBySideCacheSection;


/**
 * Constructor
 *
 * @param InFilename Unreal filename for the file this represents
 */
FSideBySideCache::FSideBySideCache(const TCHAR* InFilename, UBOOL bShouldCache)
:	Filename(InFilename)
,	SourceFileSize(INDEX_NONE)
,	UniqueID(GSideBySideNextID++)
,	bEnableCaching(bShouldCache)
{
}


/**
 * Destructor
 */
FSideBySideCache::~FSideBySideCache()
{
	// close the HD file if open
	if ( HardDriveFile.IsValid() )
	{
		GFileManager->FileClose( HardDriveFile );
		HardDriveFile.Invalidate();
	}

	// close the source file if still open
	if ( SourceFile.IsValid() )
	{
		GFileManager->FileClose(SourceFile);
		SourceFile.Invalidate();
	}
}


/**
 * Open the source and HD files for caching. If caching is disabled, only open the source file.
 *
 * @param CacheFilename		Optional filename for the cache file (NULL to use the same as the source).
 * @return					TRUE if successful
 */
UBOOL FSideBySideCache::OpenFilesForCaching( const TCHAR* CacheFilename/*=NULL*/ )
{
	// open the source file
	SourceFile = GFileManager->FileOpen( *Filename, IO_READ );

	if (GIsRequestingExit)
	{
		return TRUE;
	}
	if (SourceFile.IsValid() == FALSE)
	{
		return FALSE;
	}

	// only make the HD file if we want to cache it
	if ( bEnableCaching )
	{
		// open the file for writing and reading so it can be read from after caching is complete (creates the directory structure automatically)
		FFilename CacheFilenameToUse = CacheFilename ? CacheFilename : *GHDDCacheManager->GetCachedFilepath(*Filename);
		HardDriveFile = GFileManager->FileOpen( *CacheFilenameToUse, IO_READWRITE|IO_APPEND );
		if ( HardDriveFile.IsValid() && ReserveCacheSpace() )
		{
			// IO_APPEND sets the filepointer to the end of the file. Set it back to the beginning.
			GFileManager->FileSeek( HardDriveFile, 0 );
		}
		else
		{
			GFileManager->FileClose( HardDriveFile );
			HardDriveFile.Invalidate();
			bEnableCaching = FALSE;
			debugf( NAME_DevHDDCaching, TEXT("ERROR - Failed to create cache file on HD for caching [%s]"), *CacheFilenameToUse );
		}
	}

	return TRUE;
}

/**
 * Constructor
 *
 * @param InFilename Unreal filename for the file this represents
 * @param bShouldCache	Whether we should actually be caching or not
 */
FOneDirectionCache::FOneDirectionCache(const TCHAR* InFilename, UBOOL bShouldCache)
:	FSideBySideCache(InFilename, bShouldCache)
,	AmountWritten(0)
,	FilePos(0)
{
}

/**
 * Destructor
 */
FOneDirectionCache::~FOneDirectionCache()
{
	// if we are shutting down, don't serialize the rest
	if (!GIsRequestingExit)
	{
		// make sure the cache is ready to close
		FullyCache();
		FinalizeCache();
	}
}

/**
 * Initialize the cache. Will open the destination file for writing.
 * 
 * @return TRUE if successful
 */
UBOOL FOneDirectionCache::Initialize()
{
	FScopeLock ScopeLock(&SideBySideCacheSection);

	if ( bEnableCaching == FALSE )
	{
		// Just open the source file
		SourceFileSize = GFileManager->FileSize( *Filename );
		SourceFile = GFileManager->FileOpen( *Filename, IO_READ );
		return SourceFile.IsValid();
	}

	// make the var to query file state
	UBOOL bIsHDDFileGood;
	UBOOL bIsHDDFileUpToDate;
	UBOOL bIsTimestampFileValid;

	// check the source/hd files, and do timestamp checking
	SourceFileSize = GHDDCacheManager->CheckFiles( *Filename, bIsHDDFileGood, bIsHDDFileUpToDate, bIsTimestampFileValid);

	if (GIsRequestingExit)
	{
		return FALSE;
	}

	FString CachedFile = GHDDCacheManager->GetCachedFilepath( *Filename );

	// delete the cached HD file if it can't be reused
	if ( !bIsHDDFileGood )
	{
		GFileManager->Delete( *CachedFile );
	}

	if ( bIsHDDFileGood && bIsHDDFileUpToDate )
	{
		// HD is up to date, then just use the HD copy
		SourceFile = GFileManager->FileOpen( *CachedFile, IO_READ );
		if ( SourceFile.IsValid() )
		{
			// success
			bEnableCaching = FALSE;
			debugf( NAME_DevHDDCaching, TEXT("Opened fully cached file (OneDirectional): %s"), *CachedFile );
			return TRUE;
		}
		else
		{
			// show HD failed message, since it existed but then failed to open - this will never return
			appHandleIOFailure( *CachedFile );
		}
	}

	// if we get here, we need a source file
	if (SourceFileSize == -1)
	{
		debugf( NAME_DevHDDCaching, TEXT("ERROR - Source file %s is unusable, cannot continue"), *Filename );
		return FALSE;
	}

	// open the source and HD files for caching, and use its success as our success
	UBOOL bSuccess = OpenFilesForCaching( *(CachedFile + TEXT("__tmp")) );

	debugf( NAME_DevHDDCaching, TEXT("Created file cache (OneDirectional): %s"), *(CachedFile + TEXT("__tmp")) );

	return bSuccess;
}

/**
 * Handle the source archive moving position
 *
 * @param Pos New position the source file will be reading from
 *
 * @return TRUE if successful
 */
UBOOL FOneDirectionCache::Seek(INT Pos)
{
	FScopeLock ScopeLock(&SideBySideCacheSection);
	check(Pos <= SourceFileSize);

	if ( Pos == FilePos )
	{
		return TRUE;
	}

	// if we have both source and HD files open, that means we're caching from BD to HD while reading
	if ( SourceFile.IsValid() && HardDriveFile.IsValid() )
	{
		// if we are seeking past the end of where we have already cached, read in the gap
		if (Pos > AmountWritten)
		{
			// we need to read what we are skipping over, so go to right after what we've read in
			GFileManager->FileSeek( SourceFile, AmountWritten );

			// sync the position
			FilePos = AmountWritten;

debugf( NAME_DevHDDCaching, TEXT("Filling in a gap from %d to %d (%s)"), AmountWritten, Pos, *Filename );
#define TEMP_BUFFER_SIZE (128 * 1024)
			// allocate a buffer for reading/writing
			BYTE* Buffer = (BYTE*)appMalloc(TEMP_BUFFER_SIZE);

			// loop until we've read/written it all
			do 
			{
				INT AmountToRead = Min<INT>(TEMP_BUFFER_SIZE, Pos - AmountWritten);
				if (Serialize(Buffer, AmountToRead) == FALSE)
				{
					appFree(Buffer);
					return FALSE;
				}
			} while (AmountWritten < Pos && !GIsRequestingExit);

			appFree(Buffer);
		}
		else
		{
			//@TODO Seek on the HDD instead, since we have cached at least this much already.
			GFileManager->FileSeek(SourceFile, Pos);
		}
	}
	else if (SourceFile.IsValid())
	{
		// if we have just the source file open, seek that
		GFileManager->FileSeek(SourceFile, Pos);
	}

	// keep local pos in sync
	FilePos = Pos;

	return TRUE;
}

/** 
 * Serialize from SourceFile, while writing it out to the cache at the same time.
 *
 * @param Buffer The buffer to read into
 * @param Size Amount to read
 *
 * @return TRUE if successful
 */
UBOOL FOneDirectionCache::Serialize(void* Buffer, INT Size)
{
	FScopeLock ScopeLock(&SideBySideCacheSection);

	if ( !SourceFile.IsValid() )
	{
		return FALSE;
	}

	// can't read off the end (even if async IO might want to?)
	Size = Min(Size, SourceFileSize - FilePos);

	// first, read in from source file
	INT BytesRead = GFileManager->FileRead( SourceFile, Buffer, Size );

	// if the read failed, DON'T write to the HD
	if (GIsRequestingExit || BytesRead != Size)
	{
		return FALSE;
	}

	// keep it up to date
	FilePos += Size;

#if DO_CHECK
	// Verify that the file's current offset is where we think it is.
	INT ActualFilePos = GFileManager->GetFilePosition( SourceFile );
	checkf(ActualFilePos == FilePos, TEXT("ActualFilePos %d != FilePos %d [Size = %d, Name = %s]"), ActualFilePos, FilePos, SourceFileSize, *Filename);
#endif

	// Cache to HDD if possible
	INT AmountToCache = FilePos - AmountWritten;	// we may need to only write a portion if part was already written out
	if ( HardDriveFile.IsValid() && AmountToCache > 0 )
	{
		check( Size >= AmountToCache );
		BYTE* BufferToCache = ((BYTE*)Buffer) + Size - AmountToCache;
		INT BytesWritten = GFileManager->FileWrite( HardDriveFile, BufferToCache, AmountToCache );
		if (BytesWritten != AmountToCache)
		{
			debugf( NAME_DevHDDCaching, TEXT("Failed to write enough data to one direction cache (%d / %d): %s"), BytesWritten, Size, *Filename );
			return FALSE;
		}

		// update how much we've done
		AmountWritten += AmountToCache;

		// if we are at the end, we can stop syncing and close down the file
		if (AmountWritten == SourceFileSize && !GIsRequestingExit)
		{
//printf("Read all of source, calling FinalizeCache\n");
			FinalizeCache();
		}
	}

	return TRUE;
}

/**
 * Fully caches a file to HD
 */
UBOOL FOneDirectionCache::FullyCache()
{
	// by going to the end, we can guarantee that the entire file has been written to HD
	if (SourceFileSize > 0 && HardDriveFile.IsValid())
	{
		return Seek(SourceFileSize);
	}

	// if it wasn't supposed to be cached, then there's no reason to return an error
	return bEnableCaching == FALSE;
}

/**
 * Reserves HDD cache space to fit the entire file
 *
 * @return TRUE if the HD had enough space
 */
UBOOL FOneDirectionCache::ReserveCacheSpace()
{
	if ( HardDriveFile.IsValid() )
	{
		return GHDDCacheManager->ReserveCacheSpace(HardDriveFile, SourceFileSize);
	}
	return TRUE;
}


/**
 * Closes source file, syncs up dates, etc
 *
 * @return TRUE if successful
 */
UBOOL FOneDirectionCache::FinalizeCache()
{
	// were we caching?
	if (SourceFile.IsValid() && HardDriveFile.IsValid())
	{
		// close the source file
		GFileManager->FileClose(SourceFile);
		GFileManager->FileClose(HardDriveFile);
		SourceFile.Invalidate();
		HardDriveFile.Invalidate();

		FString CacheFilename = GHDDCacheManager->GetCachedFilepath( *Filename );
		FString CacheFilenameTmp = CacheFilename + TEXT("__tmp");

		UBOOL bRenameSucceeded = GFileManager->Move( *CacheFilename, *CacheFilenameTmp );
		SourceFile = GFileManager->FileOpen(*CacheFilename, IO_READ);
		if ( bRenameSucceeded && SourceFile.IsValid() && GFileManager->FileSeek( SourceFile, FilePos ) )
		{
			debugf( NAME_DevHDDCaching, TEXT("%s is now fully cached (OneDirectional)!"), *Filename );
			return TRUE;
		}
		else
		{
//			debugf( NAME_DevHDDCaching, TEXT("Failed to reopen HD file after renaming it [%s -> %s]"), *CacheFilenameTmp, *CacheFilename );
			return FALSE;
		}
	}
	return TRUE;
}


/**
 * Constructor
 *
 * @param InFilename		Unreal filename for the file this represents
 * @param InChunkSize		Size of each mapped chunk
 * @param bReserveSpace		Whether HDD cache space should be reserved for this file
 */
FMappedCache::FMappedCache(const TCHAR* InFilename, INT InChunkSize, UBOOL bInReserveSpace )
:	FSideBySideCache(InFilename, TRUE)
,	ChunkSize(InChunkSize)
,	NumChunks(0)
,	NumCachedChunks(0)
,	MappedChunks(NULL)
,	bIsFileFullyMapped(FALSE)
,	FilePos(0)
,	HardDriveFilePos(0)
,	SourceFilePos(0)
,	LastTimeMetadataWasSaved(appSeconds())
,	bReserveSpace( bInReserveSpace )
{
}

/**
 * Destructor
 */
FMappedCache::~FMappedCache()
{
	if ( (!GIsRequestingExit || NumChunks > 0) && HardDriveFile.IsValid() )
	{
		SaveMetadata();
	}

	delete MappedChunks;

	if ( MetadataFile.IsValid() )
	{
		GFileManager->FileClose(MetadataFile);
		MetadataFile.Invalidate();
	}
}

/**
 * Initialize the cache.
 *
 * @return TRUE if successful
 */
UBOOL FMappedCache::Initialize()
{
	FScopeLock ScopeLock(&SideBySideCacheSection);

	FString CacheFilename = GHDDCacheManager->GetCachedFilepath( *Filename );
	FString MetaFilename = CacheFilename + TEXT("__metx");

	// make the var to query file state
	UBOOL bIsHDDFileGood;
	UBOOL bIsHDDFileUpToDate;
	UBOOL bIsTimestampFileValid;
//printf("Initializing %s\n", PlatformFilenameSource);

	// check the source/hd files, and do timestamp checking
	SourceFileSize = GHDDCacheManager->CheckFiles(*Filename, bIsHDDFileGood, bIsHDDFileUpToDate, bIsTimestampFileValid);

	// cache how many chunks are 
	if (SourceFileSize != -1)
	{
		NumChunks = (SourceFileSize + (ChunkSize - 1)) / ChunkSize;
	}

	// if the HD file can be reused, do so
	UBOOL bIsHDFileReusable = bIsHDDFileGood && (bIsHDDFileUpToDate || SourceFileSize == -1);

	// delete the metadata file if the HD file can't be reused
	if (!bIsHDFileReusable && !GIsRequestingExit )
	{
		GFileManager->Delete(*MetaFilename);
		GFileManager->Delete(*CacheFilename);
	}

	// create/open the metadata file for read/write
	MetadataFile = GFileManager->FileOpen(*MetaFilename, IO_READWRITE|IO_APPEND);
	if ( MetadataFile.IsValid() == FALSE )
	{
		debugf( NAME_DevHDDCaching, TEXT("Failed to create metadata file %s"), *MetaFilename );
	}

	// check to see if the HD file is as new as the source file
	// if so, we don't need to re-cache
	if (bIsHDFileReusable && MetadataFile.IsValid())
	{
		// read in metadata only if the HD file is up to date
		LoadMetadata();
	}

	// if we get here, we need a source file
	if (SourceFileSize == -1)
	{
		debugf( NAME_DevHDDCaching, TEXT("Source file %s is unusable, cannot continue"), *Filename );
		return FALSE;
	}

	// if we didn't already open the metadata previously, make new metadata
	if (MappedChunks == NULL)
	{
		// make a bit array with enough slots for all chunks in the file
		MappedChunks = new TArray<WORD>();
		MappedChunks->Add(NumChunks);
		NumCachedChunks = 0;

		// set all bits to unmapped
		appMemset(MappedChunks->GetData(), 0xFF, NumChunks * sizeof(WORD));
	}

	// open the source and HD files for caching, and use its success as our success
	UBOOL bSuccess = OpenFilesForCaching();

	debugf( NAME_DevHDDCaching, TEXT("%s Mapped file cache for %s (%d of %d chunks cached)"), bIsHDFileReusable ? TEXT("Opened") : TEXT("Created"), *Filename, NumCachedChunks, NumChunks );

	return bSuccess;
}

/**
 * Handle the source archive moving position
 *
 * @param Pos New position the source file will be reading from
 *
 * @return TRUE if successful
 */
UBOOL FMappedCache::Seek(INT Pos)
{
	// cache the desired location
	FilePos = Pos;

	return TRUE;
}

/** 
 * Serialize from source and/or HD
 *
 * @param Buffer The buffer to read into
 * @param Size Amount to read
 *
 * @return TRUE if successful
 */
UBOOL FMappedCache::Serialize(void* Buffer, INT Size)
{
	FScopeLock ScopeLock(&SideBySideCacheSection);

	// make sure we are reading aligned
	checkf((FilePos % ChunkSize) == 0, TEXT("Attempted to serialize a misaligned chunk: %d is not a multiple of %d"), FilePos, ChunkSize);
	checkf(Size < ChunkSize || ((Size % ChunkSize) == 0), TEXT("Attempted serialize non-chunk-multiple bytes: %d is not a multiple of %d and is > chunk size"), Size, ChunkSize);

	// access through the buffer, one chunk at a time
	BYTE* BufferBase = (BYTE*)Buffer;

	// how many chunks do we need for Size?
	INT NumSubChunks = (Size + ChunkSize - 1) / ChunkSize;

	// one chunk at a time
	Size = Min(Size, ChunkSize);

	// go over all subchunks
	for (INT SubChunk = 0; SubChunk < NumSubChunks && !GIsRequestingExit; SubChunk++, BufferBase += ChunkSize)
	{
		// get which chunk this is
		INT ChunkIndex = FilePos / ChunkSize;

		// make sure we don't read past end of file
		// this is not an error case, because the async io system will try to read past the end for its big chunks
		if (ChunkIndex >= NumChunks)
		{
			break;
		}

		// it should fit in the mapped chunks array
		check(bIsFileFullyMapped || ChunkIndex < MappedChunks->Num());

		// is the chunk mapped to HD already?
		UBOOL bChunkIsMapped = (*MappedChunks)(ChunkIndex) != UNMAPPED_CHUNK;

		INT BytesRead;
		INT BytesWritten;

		// if it's mapped, read from HD file
		if (bChunkIsMapped)
		{
//printf("Chunk %d is mapped, reading from HD [virtual pos: %d, actual pos %d\n", ChunkIndex, FilePos, HardDriveFilePos);
			check(HardDriveFile.IsValid());

			// calcualate reamapped position in HD file
			INT HDFilePos = (*MappedChunks)(ChunkIndex) * ChunkSize;

			// seek the HD file to the proper location if its not already
			if (HDFilePos != HardDriveFilePos)
			{
				GFileManager->FileSeek( HardDriveFile, HDFilePos );
				HardDriveFilePos = HDFilePos;
			}

			// read from file
			BytesRead = GFileManager->FileRead(HardDriveFile, BufferBase, Size);

			// update file positions
			HardDriveFilePos += Size;
			FilePos += Size;
		}
		// if it's not mapped, read from Source file and write out to HD
		else
		{
			// we need to have the source file if it wasn't already mapped
			checkf(SourceFile.IsValid(), TEXT("Source file for %s is -1??? [%d, %d, %d, %d]"), *Filename, HardDriveFile.Handle, SourceFileSize, ChunkIndex, bIsFileFullyMapped);

			// move to proper position
			if (FilePos != SourceFilePos)
			{
				GFileManager->FileSeek(SourceFile, FilePos);
				SourceFilePos = FilePos;
			}

			// read from source
			BytesRead = GFileManager->FileRead(SourceFile, BufferBase, Size);

			// Read error?
			if ( BytesRead < 0 )
			{
				// show HD failed message, since it existed but then failed to open - this will never return
				appHandleIOFailure( *Filename );
			}

			SourceFilePos += Size;

			if (GIsRequestingExit)
			{
				return TRUE;
			}

			// write to HD if possible
			if (HardDriveFile.IsValid() && MetadataFile.IsValid() && !GIsRequestingExit)
			{
				// Seek to end of cached data, if we're not already there.
				if ( HardDriveFilePos != NumCachedChunks*ChunkSize )
				{
					HardDriveFilePos = GFileManager->FileSeek( HardDriveFile, NumCachedChunks*ChunkSize );
				}

				check((HardDriveFilePos % ChunkSize) == 0);

				// calculate which remapped (HD) chunk we are writing to
				INT HDChunkIndex = HardDriveFilePos / ChunkSize;

				// must fit in a short
				check(HDChunkIndex < 65535);

				// write to the HD file
				BytesWritten = GFileManager->FileWrite( HardDriveFile, BufferBase, Size );
				if (BytesWritten != Size)
				{
					appHandleIOFailure(*Filename);
				}

				// make sure we write a full chunk to HD so that all sectors are mapped
				// as we can't leave a single sector unwritten to, and next chunk must start at 
				// multiple of ChunkSize
				if (Size < ChunkSize)
				{
					FMemMark Mark(GMainThreadMemStack);
					BYTE* GarbageData = new (GMainThreadMemStack, ChunkSize - Size) BYTE;
					BytesWritten = GFileManager->FileWrite( HardDriveFile, GarbageData, ChunkSize - Size );
					if ( BytesWritten != (ChunkSize - Size) )
					{
						appHandleIOFailure(*Filename);
					}
					Mark.Pop();
				}

				// set the chunk to be mapped
				(*MappedChunks)(ChunkIndex) = HDChunkIndex;
				++NumCachedChunks;

				// move to the end of the chunk (also, EOF)
				HardDriveFilePos += ChunkSize;

				// decrement how many caches until save metadata
				DOUBLE TimeDifference = appSeconds() - LastTimeMetadataWasSaved;
				if ( TimeDifference > TIME_BETWEEN_METADATA_SAVES )
				{
					SaveMetadata();
				}
			}

			// update virtual file pos
			FilePos += Size;
		}
	}

	return TRUE;
}

/**
 * Fully caches a file to HD - not done for a mapped cache.
 */
UBOOL FMappedCache::FullyCache()
{
	return TRUE;
}

/**
 * Reserves HDD cache space to fit the entire file
 *
 * @return TRUE if the HD had enough space
 */
UBOOL FMappedCache::ReserveCacheSpace()
{
	UBOOL bSuccess = TRUE;
	if ( bReserveSpace )
	{
		if ( HardDriveFile.IsValid() )
		{
			DWORD CachedFileSize = NumChunks * ChunkSize;
			bSuccess = bSuccess && GHDDCacheManager->ReserveCacheSpace( HardDriveFile, CachedFileSize );
		}
		if ( MetadataFile.IsValid() )
		{
			DWORD MetadataFileSize = NumChunks * sizeof(WORD) + 2*sizeof(INT) + sizeof(UBOOL);
			bSuccess = bSuccess && GHDDCacheManager->ReserveCacheSpace( MetadataFile, MetadataFileSize );
		}
	}
	// don't check space for the whole file, we check per chunk
	return bSuccess;
}


/**
 * Read the metadata file from HD
 *
 * @return TRUE if successful
 */
UBOOL FMappedCache::LoadMetadata()
{
	FScopeLock ScopeLock(&SideBySideCacheSection);

	// reset state
	bIsFileFullyMapped = FALSE;
	delete MappedChunks;
	MappedChunks = NULL;
	NumCachedChunks = 0;

	// Get the filesize and go to beginning of file
	GFileManager->FileSeek( MetadataFile, 0, IO_SEEK_END );
	INT FileSize = GFileManager->GetFilePosition( MetadataFile );
	GFileManager->FileSeek( MetadataFile, 0 );

	// the file should exist, since we created it in Init, but if not, or if it's empty (just created)
	// then there is nothing to load
	if ( MetadataFile.IsValid() == FALSE || FileSize <= 0 )
	{
		return FALSE;
	}

	// make space for file contents
	TArray<BYTE> ScratchMemory;
	ScratchMemory.Add( FileSize );

	// read file into memory
	INT BytesRead = GFileManager->FileRead(MetadataFile, ScratchMemory.GetData(), ScratchMemory.Num());

	// make an archive to serialize from
	FMemoryReader MemoryReader(ScratchMemory, TRUE);

	// read the CRC and compare to what was saved out
	DWORD CRC;
	MemoryReader << CRC;

	DWORD ExpectedCRC = appMemCrc(ScratchMemory.GetTypedData() + 4, ScratchMemory.Num() - 4);
	if (ScratchMemory.Num() <= 4 || CRC != ExpectedCRC)
	{
		debugf( NAME_DevHDDCaching, TEXT("Detected corrupted metadata! Restarting file caching (Mapped): %s"), *Filename );
		return FALSE;
	}

	// read in if the file was fully mapped
	MemoryReader << bIsFileFullyMapped;

	// read in the number of cached chunks
	MemoryReader << NumCachedChunks;

	MappedChunks = new TArray<WORD>;
	MemoryReader << *MappedChunks;

//debugf( NAME_DevHDDCaching, TEXT("Loaded, fully mapped: %d, NumChunks: %d, CRC: %08X"), bIsFileFullyMapped, MappedChunks ? MappedChunks->Num() : 0, CRC );
//debugf( NAME_DevHDDCaching, TEXT("First 8 bytes on save: %08x %08x"), ((DWORD*)ScratchMemory.GetData())[0], ((DWORD*)ScratchMemory.GetData())[1] );

	return TRUE;
}

/**
 * Read the metadata file from HD
 *
 * @return TRUE if successful
 */
UBOOL FMappedCache::SaveMetadata()
{
	if (MappedChunks == NULL)
	{
		return FALSE;
	}

	// make sure HD file is fully written out before touching the metadata file
	GFileManager->FileFlush(HardDriveFile);

	// Reset the timestamp
	LastTimeMetadataWasSaved = appSeconds();

	debugf( NAME_DevHDDCaching, TEXT("Saving metadata for %s (%d of %d chunks cached)"), *Filename, NumCachedChunks, MappedChunks->Num() );

	// go to beginning of file
	GFileManager->FileSeek( MetadataFile, 0 );

	// make space for file contents
	TArray<BYTE> ScratchMemory;

	// make an archive to serialize from
	FMemoryWriter MemoryWriter(ScratchMemory, TRUE);

	// skip over the CRC bits
	INT Zero = 0;
	MemoryWriter << Zero;

	// write out if it's fully mapped
	MemoryWriter << bIsFileFullyMapped;

	// write out the number of cached chunks
	MemoryWriter << NumCachedChunks;

	// if it's not fully mapped, write out which chunks were mapped
	MemoryWriter << *MappedChunks;

	// calc CRC
	DWORD CRC = appMemCrc(ScratchMemory.GetTypedData() + sizeof(DWORD), ScratchMemory.Num() - sizeof(DWORD));
	appMemcpy(ScratchMemory.GetData(), &CRC, sizeof(DWORD));

//debugf( NAME_DevHDDCaching, TEXT("Saving metadata, %d bytes [CRC: %08X]"), ScratchMemory.Num(), CRC );
//debugf( NAME_DevHDDCaching, TEXT("First 8 bytes on save: %08x %08x"), ((DWORD*)ScratchMemory.GetData())[0], ((DWORD*)ScratchMemory.GetData())[1] );
	// write file to disk
	INT BytesWritten = GFileManager->FileWrite( MetadataFile, ScratchMemory.GetData(), ScratchMemory.Num() );
	if ( BytesWritten != ScratchMemory.Num() )
	{
		appHandleIOFailure(*Filename);
		return FALSE;
	}

	// make sure metadata file is fully written out
	GFileManager->FileFlush(MetadataFile);

	return TRUE;
}
