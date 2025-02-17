/*=============================================================================
    XeFileCaching.cpp: Unreal Xenon harddisk file caching.
    Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "Engine.h"
#include "FFileManagerXenon.h"
#include "XeFileCaching.h"

extern UBOOL GEnableHDDCaching;

/** Default constructor */
FXeHDDCacheManager::FXeHDDCacheManager()
:	bIsUtilityMounted(FALSE)
{
	FileCacheRoot = TEXT("cache:");

	UBOOL bClearCache = ParseParam(appCmdLine(), TEXT("ClearHDDCache"));
	GEnableHDDCaching = GEnableHDDCaching && ParseParam(appCmdLine(), TEXT("DisableHDDCache")) == FALSE;
	if ( GEnableHDDCaching )
	{
		DWORD ReturnCode = XMountUtilityDrive( bClearCache, 16*1024, 64*1024 );
		ReturnCode = (ReturnCode == ERROR_SUCCESS) ? XFlushUtilityDrive() : ReturnCode;

		WIN32_FIND_DATA FindData;
		HANDLE FindHandle = FindFirstFileA("cache:\\*", &FindData);

		// Try to re-format the cache partition if we detect a problem.
		if ( ReturnCode != ERROR_SUCCESS || FindHandle == INVALID_HANDLE_VALUE )
		{
			XUnmountUtilityDrive();
			ReturnCode = XMountUtilityDrive( TRUE, 16*1024, 64*1024 );
		}

		if ( FindHandle != INVALID_HANDLE_VALUE )
		{
			FindClose( FindHandle );
		}

		if ( ReturnCode == ERROR_SUCCESS )
		{
			bEnableCaching = TRUE;
			bIsUtilityMounted = TRUE;
			ULARGE_INTEGER AvailableNumBytes, TotalNumBytes, FreeNumBytes;
			GetDiskFreeSpaceEx( "cache:\\", &AvailableNumBytes, &TotalNumBytes, &FreeNumBytes );
			debugf( NAME_DevHDDCaching, TEXT("Mounted CACHE: partition. TotalSize=%.3f MB, Available=%.3f MB, Free=%.3f MB"),
				DOUBLE(TotalNumBytes.QuadPart)/1024.0/1024.0, DOUBLE(AvailableNumBytes.QuadPart)/1024.0/1024.0, DOUBLE(FreeNumBytes.QuadPart)/1024.0/1024.0 );
		}
		else
		{
			bEnableCaching = FALSE;
			debugf( NAME_DevHDDCaching, TEXT("ERROR - Failed to mount CACHE: partition! Disabling HDD caching.") );
		}
	}
}

/** Virtual destructor */
FXeHDDCacheManager::~FXeHDDCacheManager()
{
	if ( bIsUtilityMounted )
	{
		XUnmountUtilityDrive();
	}
}

/**
 *	Make sure that all cache files has matching files and delete invalid files.
 *	@param bClearCache	Whether to clear the whole HDD cache
 */
void FXeHDDCacheManager::ValidateCache( UBOOL bClearCache )
{
	FHDDCacheManager::ValidateCache( bClearCache );
	XFlushUtilityDrive();
}

/**
 * Reserves HDD cache space for the specified file.
 *
 * @param FileHandle			File to reserve space for
 * @param NumBytesRequired		Amount of cache space required for the file, in total
 * @return						TRUE if the HDD cache has enough space
 */
UBOOL FXeHDDCacheManager::ReserveCacheSpace( FFileHandle FileHandle, INT NumBytesRequired )
{
	HANDLE WinHandle = (HANDLE) FileHandle.Handle;
	DWORD CurrentFileSize = ::GetFileSize(WinHandle, NULL);
	DWORD CurrentFilePointer = ::SetFilePointer(WinHandle, 0, NULL, FILE_CURRENT);
	DWORD NewFileSize = ::SetFilePointer(WinHandle, NumBytesRequired, NULL, FILE_BEGIN);
	UBOOL bSuccess = ::SetEndOfFile(WinHandle);
	::SetFilePointer(WinHandle, CurrentFilePointer, NULL, FILE_BEGIN);
	return bSuccess && (NewFileSize == NumBytesRequired);
}

/**
 *	Determines whether the file path refers to a partition or folder that is allowed to be cached.
 *	@param FilePath	- Path to the file that we're considering for caching.
 *	@return			- TRUE if the file is contained in a path that is allowed to be cached
 */
UBOOL FXeHDDCacheManager::ShouldCachePath( const TCHAR* FilePath )
{
	// DLC partition (i.e. "DLCnn:" or "DLCn:") should never be cached.
	if ( appStrnicmp(FilePath, TEXT("DLC"), 3) == 0 )
	{
		// Skip digits.
		const TCHAR* FilePathMatching = FilePath + 3;
		while ( *FilePathMatching && appIsDigit(*FilePathMatching) )
		{
			FilePathMatching++;
		}

		// Check for ':' character.
		if ( *FilePathMatching == TEXT(':') )
		{
			return FALSE;
		}
	}

	// Update (patch) partition should never be cached.
	if ( appStrnicmp(FilePath, TEXT("UPDATE:"), 7) == 0 )
	{
		return FALSE;
	}

	return FHDDCacheManager::ShouldCachePath( FilePath );
}

/**
 *	Returns the platform-specific maximum length of a filename with full path.
 **/
INT FXeHDDCacheManager::GetMaxFilePathLength()
{
	return XCONTENT_MAX_FILENAME_LENGTH;
}

/**
 * Returns the full path to the cached version of the file
 *
 * @param Filename	Source file
 * @return			Full path of the cached file
 */
FString FXeHDDCacheManager::GetCachedFilepath(const TCHAR* Filename)
{
	// Skip leading "..\".
	if( Filename[0] == L'.' )
	{
		Filename += 3;
	}
	if( Filename[0] == L'.' )
	{
		appErrorf(TEXT("No support for relative path names apart from leading \"..\\\" [%s]."), Filename);
	}
	if( Filename[0] == L'\\')
	{
		Filename += 1;
	}

	// Insert the root before the file
	FString CachedFile = GetFileCacheRoot();
	CachedFile += PATH_SEPARATOR;
	CachedFile += Filename;
	return CachedFile;
}
