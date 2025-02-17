/*=============================================================================
	UnAsyncLoadingUnix.cpp: Unreal async loading code, Unix implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"

#if PLATFORM_UNIX

#include "FFileManagerUnix.h"

/*-----------------------------------------------------------------------------
	Async loading stats.
-----------------------------------------------------------------------------*/

DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("Platform read time"),STAT_AsyncIO_PlatformReadTime,STATGROUP_AsyncIO);


/*-----------------------------------------------------------------------------
	FAsyncIOSystemUnix implementation.
-----------------------------------------------------------------------------*/

/** 
 * Reads passed in number of bytes from passed in file handle.
 *
 * @param	FileHandle	Handle of file to read from.
 * @param	Offset		Offset in bytes from start, INDEX_NONE if file pointer shouldn't be changed
 * @param	Size		Size in bytes to read at current position from passed in file handle
 * @param	Dest		Pointer to data to read into
 *
 * @return	TRUE if read was successful, FALSE otherwise
 */
UBOOL FAsyncIOSystemUnix::PlatformRead( FAsyncIOHandle FileHandle, INT Offset, INT Size, void* Dest )
{
	ssize_t BytesRead = 0;
	UBOOL bSeekFailed = FALSE;
	UBOOL bReadFailed = FALSE;
	STAT(DOUBLE ReadTime = 0);
	{
		SCOPE_SECONDS_COUNTER(ReadTime);
		SCOPED_FILE_IO_ASYNC_READ_STATS(FileHandle.StatsHandle,Size,Offset);

		if (FileHandle.Handle != NULL)
		{
			const int Handle = (int) ((PTRINT) FileHandle.Handle);
			if( Offset != INDEX_NONE )
			{
				bSeekFailed = lseek( Handle, Offset, SEEK_SET ) == -1;
			}
			if( !bSeekFailed )
			{
				BytesRead = read( Handle, Dest, Size );
			}
		}
	}
	INC_FLOAT_STAT_BY(STAT_AsyncIO_PlatformReadTime,(FLOAT)ReadTime);
	// Constrain bandwidth if wanted.
	STAT(ConstrainBandwidth(Size, ReadTime));
	return BytesRead == ((ssize_t) Size);
}

/** 
 * Creates a file handle for the passed in file name
 *
 * @param	Filename	Pathname to file
 *
 * @return	INVALID_HANDLE if failure, handle on success
 */
FAsyncIOHandle FAsyncIOSystemUnix::PlatformCreateHandle( const TCHAR* Filename )
{
	FAsyncIOHandle FileHandle;
	int Handle = -1;

	FileHandle.StatsHandle	= FILE_IO_STATS_GET_HANDLE( Filename );
	SCOPED_FILE_IO_READ_OPEN_STATS( FileHandle.StatsHandle );

	// Convert to an absolute path in order to be able to handle current working directory changing on us.
	FFileManagerUnix* FileManager = (FFileManagerUnix*)GFileManager;

	// first look in the User Directory (look in FFileManagerUnix.cpp)
	FString AbsPath = FileManager->ConvertAbsolutePathToUserPath(*appConvertRelativePathToFull( Filename ));
	Handle = open(TCHAR_TO_UTF8(*AbsPath), O_RDONLY);

	// if that failed, then look in install dir
	if (Handle == -1)
	{
		FString AbsPath = appConvertRelativePathToFull( Filename );
		Handle = open(TCHAR_TO_UTF8(*AbsPath), O_RDONLY);
	}

	FileHandle.Handle = (void *) ((PTRINT) Handle);
	return FileHandle;
}

/**
 * Closes passed in file handle.
 */
void FAsyncIOSystemUnix::PlatformDestroyHandle( FAsyncIOHandle FileHandle )
{
	FILE_IO_STATS_CLOSE_HANDLE( FileHandle.StatsHandle );
	const int Handle = (int) ((PTRINT) FileHandle.Handle);
	close(Handle);
}

/**
 * Returns whether the passed in handle is valid or not.
 *
 * @param	FileHandle	File hande to check validity
 *
 * @return	TRUE if file handle is valid, FALSE otherwise
 */
UBOOL FAsyncIOSystemUnix::PlatformIsHandleValid( FAsyncIOHandle FileHandle )
{
	const int Handle = (int) ((PTRINT) FileHandle.Handle);
	return Handle != -1;
}

/**
 * Returns 0 as we are operating on FIFO principle.
 *
 * @return	0
 */
INT FAsyncIOSystemUnix::PlatformGetNextRequestIndex()
{
	return 0;
}

#else

// Suppress linker warning "warning LNK4221: no public symbols found; archive member will be inaccessible"
INT UnAsyncLoadingUnixLinkerHelper;

#endif // PLATFORM_UNIX
