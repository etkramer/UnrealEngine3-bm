/*=============================================================================
	UnAsyncLoading.cpp: Unreal async loading code, Xbox 360 implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"
#include "FileCaching.h"
#include "FFileManagerGeneric.h"

extern UBOOL GEnableHDDCaching;

/*-----------------------------------------------------------------------------
	Async loading stats.
-----------------------------------------------------------------------------*/

DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("Platform read time"),STAT_AsyncIO_PlatformReadTime,STATGROUP_AsyncIO);


/*-----------------------------------------------------------------------------
	FAsyncIOSystemXenon implementation.
-----------------------------------------------------------------------------*/

/** Constructor, initializing member variables and allocating buffer */
FAsyncIOSystemXenon::FAsyncIOSystemXenon()
:	BufferedHandle( INVALID_HANDLE_VALUE )
,	BufferedCacheID( INDEX_NONE )
,	BufferedOffset( INDEX_NONE )
,	BufferedData( NULL )
,	CurrentOffset( INDEX_NONE )
,	LastReadSector( INDEX_NONE )
{
	BufferedData = (BYTE*) appMalloc( DVD_MIN_READ_SIZE );
}

/** Virtual destructor, freeing allocated memory. */
FAsyncIOSystemXenon::~FAsyncIOSystemXenon()
{
	appFree( BufferedData );
	BufferedData = NULL;
}

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
UBOOL FAsyncIOSystemXenon::PlatformRead( FAsyncIOHandle FileHandle, INT Offset, INT Size, void* Dest )
{
	// Total number of bytes read from device.
	STAT(INT TotalBytesRead = 0);
	// Scoped stats.
	STAT(DOUBLE ReadTime = 0);
	{
		SCOPE_SECONDS_COUNTER(ReadTime);
		FSideBySideCache* Cache = (FSideBySideCache*) FileHandle.Handle;

		// Continue reading from file at current offset.
		if( Offset == INDEX_NONE )
		{
			// Make sure that the file handle hasn't changed if we're not specifying an offset.
			if ( GEnableHDDCaching )
			{
				checkf( BufferedCacheID == Cache->GetUniqueID(), TEXT("BufferedCacheID=%x CacheID=%x"), BufferedCacheID, Cache->GetUniqueID() );
			}
			else
			{
				checkf( BufferedHandle == FileHandle.Handle, TEXT("BufferedHandle=%x FileHandle.Handle=%x"), BufferedHandle, FileHandle.Handle );
			}
			check( CurrentOffset != INDEX_NONE );
		}
		// Request a seek.
		else
		{
			// Invalidate cached data if we changed file handles.
			if( GEnableHDDCaching == TRUE && BufferedCacheID != Cache->GetUniqueID() )
			{
				BufferedOffset = INDEX_NONE;
				BufferedCacheID	= Cache->GetUniqueID();
			}
			if ( GEnableHDDCaching == FALSE && BufferedHandle != FileHandle.Handle )
			{
				BufferedOffset = INDEX_NONE;
				BufferedHandle	= FileHandle.Handle;
			}
			CurrentOffset	= Offset;
		}

		/** Local version of dest buffer that is being incremented when read into.					*/
		BYTE* DestBuffer = (BYTE*) Dest;
		/**	Remaining size to read into destination buffer.											*/
		INT	RemainingSize = Size;

		// Check whether data is already cached and fulfill read via cache.
		if( BufferedOffset != INDEX_NONE
		&&	(CurrentOffset >= BufferedOffset) 
		&&  (CurrentOffset < BufferedOffset + DVD_MIN_READ_SIZE) )
		{
			// Copy cached data to destination and move offsets.
			INT BytesToCopy = Min( BufferedOffset + DVD_MIN_READ_SIZE - CurrentOffset, Size );
			appMemcpy( DestBuffer, BufferedData + CurrentOffset - BufferedOffset, BytesToCopy );
			RemainingSize	-= BytesToCopy;
			DestBuffer		+= BytesToCopy;

			// Advance 'logical' offset in file.
			CurrentOffset	+= BytesToCopy;
		}
		
		// Fulfill any remaining requests from disk.
		if( RemainingSize > 0 )
		{
			/** Offset in file aligned on ECC boundary. Will be less than Offset in most cases.			*/
			INT		AlignedOffset		= Align( CurrentOffset - DVD_ECC_BLOCK_SIZE + 1, DVD_ECC_BLOCK_SIZE);
			/** Number of bytes padded at beginning for ECC alignment.									*/
			INT		AlignmentPadding	= CurrentOffset - AlignedOffset;
			/** Total size of reads we're going to request. Will be integer multiple of min read size.	*/
			STAT(TotalBytesRead			= Align( AlignmentPadding + Size, DVD_MIN_READ_SIZE ));

			// Advance 'logical' offset in file.
			CurrentOffset += RemainingSize;

			// Sit and spin...

			while(SuspendCount.GetValue() > 0)
			{
				appSleep(0);
			}

			// Seek to aligned offset.
			UBOOL bSeekSucceeded = TRUE;
			if ( GEnableHDDCaching )
			{
				Cache->Seek( AlignedOffset );
			}
			else
			{
				bSeekSucceeded = SetFilePointer( FileHandle.Handle, AlignedOffset, NULL, FILE_BEGIN ) != INVALID_SET_FILE_POINTER;
			}
			if ( bSeekSucceeded == FALSE )
			{
				appHandleIOFailure( NULL );
			}
			BufferedOffset = AlignedOffset;

			// Read initial DVD_MIN_READ_SIZE bytes.
			DWORD NumBytesRead = 0;
			{
				SCOPED_FILE_IO_ASYNC_READ_STATS(FileHandle.StatsHandle,DVD_MIN_READ_SIZE,BufferedOffset);
				UBOOL bReadSuccessful = TRUE;
				if ( GEnableHDDCaching )
				{
					bReadSuccessful = Cache->Serialize( BufferedData, DVD_MIN_READ_SIZE );
				}
				else
				{
					bReadSuccessful = ReadFile( FileHandle.Handle, BufferedData, DVD_MIN_READ_SIZE, &NumBytesRead, NULL ) != 0;
				}
				if ( !bReadSuccessful )
				{
					appHandleIOFailure( NULL );
				}
			}

			// Copy read data to destination, skipping alignment data at the beginning.
			INT BytesToCopy = Min( RemainingSize, DVD_MIN_READ_SIZE - AlignmentPadding );
			appMemcpy( DestBuffer, BufferedData + AlignmentPadding, BytesToCopy );
			DestBuffer		+= BytesToCopy;
			RemainingSize	-= BytesToCopy;

			// Read and copy remaining data.
			while( RemainingSize > 0 )
			{
				while(SuspendCount.GetValue() > 0)
				{
					appSleep(0);
				}

				DWORD NumBytesRead = 0;
				BufferedOffset	+= DVD_MIN_READ_SIZE;
				{
					SCOPED_FILE_IO_ASYNC_READ_STATS(FileHandle.StatsHandle,DVD_MIN_READ_SIZE,BufferedOffset);
					UBOOL bReadSuccessful = TRUE;
					if ( GEnableHDDCaching )
					{
						bReadSuccessful = Cache->Serialize( BufferedData, DVD_MIN_READ_SIZE );
					}
					else
					{
						bReadSuccessful = ReadFile( FileHandle.Handle, BufferedData, DVD_MIN_READ_SIZE, &NumBytesRead, NULL ) != 0;
					}
					if ( !bReadSuccessful )
					{
						appHandleIOFailure( NULL );
					}
				}
		
				INT BytesToCopy = Min( RemainingSize, DVD_MIN_READ_SIZE );
				appMemcpy( DestBuffer, BufferedData, BytesToCopy );
				RemainingSize	-= DVD_MIN_READ_SIZE;
				DestBuffer		+= DVD_MIN_READ_SIZE;
			}
		}
	}
	INC_FLOAT_STAT_BY(STAT_AsyncIO_PlatformReadTime,(FLOAT)ReadTime);

	// Constrain bandwidth if wanted.
	STAT(ConstrainBandwidth(TotalBytesRead, ReadTime));

	// We assert on failure so we're gtg if we made it till here.
	return TRUE;
}

/** 
 * Creates a file handle for the passed in file name
 *
 * @param	FileName	Pathname to file
 *
 * @return	INVALID_HANDLE if failure, handle on success
 */
FAsyncIOHandle FAsyncIOSystemXenon::PlatformCreateHandle( const TCHAR* FileName )
{
	FAsyncIOHandle FileHandle;

	FFilename CookedFileName	= FileName;
	CookedFileName				= CookedFileName.GetPath() + TEXT("\\") + CookedFileName.GetBaseFilename() + TEXT(".xxx");

	FileHandle.StatsHandle	= FILE_IO_STATS_GET_HANDLE( *CookedFileName );
	SCOPED_FILE_IO_READ_OPEN_STATS( FileHandle.StatsHandle );

	UBOOL bSuccess = TRUE;
	if ( GEnableHDDCaching )
	{
		// create a wrapper around the file handle and a cache mechanism
		FSideBySideCache* Cache = GHDDCacheManager->GetCache( *CookedFileName );
		if ( Cache == NULL )
		{
			Cache = GHDDCacheManager->GetCache( FileName );
		}

		// store the wrapper in the platform independent Handle
		FileHandle.Handle = Cache;

		bSuccess = (Cache != NULL);
	}
	else
	{
		FileHandle.Handle	= CreateFileA( 
								TCHAR_TO_ANSI(*GFileManager->GetPlatformFilepath(*CookedFileName)), 
								GENERIC_READ, 
								FILE_SHARE_READ, 
								NULL, 
								OPEN_EXISTING, 
								FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, 
								NULL );

		// if that failed to be found, attempt the direct filename instead
		if( FileHandle.Handle == INVALID_HANDLE_VALUE )
		{
			FileHandle.Handle = CreateFileA( 
								TCHAR_TO_ANSI(*GFileManager->GetPlatformFilepath(FileName)), 
								GENERIC_READ, 
								FILE_SHARE_READ, 
								NULL, 
								OPEN_EXISTING, 
								FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, 
								NULL );
		}

		bSuccess = (FileHandle.Handle != INVALID_HANDLE_VALUE);
	}

	if( !bSuccess )
	{
		appErrorf(TEXT("Failed to open file '%s' for async IO operation."), FileName);
		appHandleIOFailure( FileName );
	}

	return FileHandle;
}

/**
 * Closes passed in file handle.
 */
void FAsyncIOSystemXenon::PlatformDestroyHandle( FAsyncIOHandle FileHandle )
{
	// Ensure that we don't re-use the buffer once the file has been
	// closed.  File handles may be recyled.
	FSideBySideCache* Cache = (FSideBySideCache*) FileHandle.Handle;
	if( GEnableHDDCaching == TRUE && BufferedCacheID == Cache->GetUniqueID() )
	{
		BufferedCacheID = INDEX_NONE;
		BufferedOffset = INDEX_NONE;
	}
	if ( GEnableHDDCaching == FALSE && BufferedHandle == FileHandle.Handle )
	{
		BufferedHandle = INVALID_HANDLE_VALUE;
		BufferedOffset = INDEX_NONE;
	}
	FILE_IO_STATS_CLOSE_HANDLE( FileHandle.StatsHandle );
	if ( GEnableHDDCaching )
	{
		delete Cache;
	}
	else
	{
		CloseHandle( FileHandle.Handle );
	}
}

/**
 * Returns whether the passed in handle is valid or not.
 *
 * @param	FileHandle	File hande to check validity
 *
 * @return	TRUE if file handle is valid, FALSE otherwise
 */
UBOOL FAsyncIOSystemXenon::PlatformIsHandleValid( FAsyncIOHandle FileHandle )
{
	if ( GEnableHDDCaching )
	{
		return FileHandle.Handle != NULL;
	}
	else
	{
		return FileHandle.Handle != INVALID_HANDLE_VALUE;
	}
}

/**
 * Determines the next request index to be fulfilled by taking into account previous and next read
 * requests and ordering them to avoid seeking.
 *
 * This function is being called while there is a scope lock on the critical section so it
 * needs to be fast in order to not block QueueIORequest and the likes.
 *
 * @return	index of next to be fulfilled request or INDEX_NONE if there is none
 */
INT FAsyncIOSystemXenon::PlatformGetNextRequestIndex()
{
	// Keep track of index of request with least cost.
	INT BestRequestIndex				= INDEX_NONE;
	INT BestRequestIndexCost			= INT_MAX;
	
	// Keep track of index of request with smallest start sector.
	INT SmallestStartSectorIndex		= INDEX_NONE;
	INT SmallestStartSector				= INT_MAX;

	// Keep track of highest encountered priority.
	EAsyncIOPriority HighestPriority	= MinPriority;

	// Calling code already entered critical section so we can access OutstandingRequests.
	if( OutstandingRequests.Num() )
	{
		// Iterate over all requests and find the one with the least cost.
		for( INT CurrentRequestIndex=0; CurrentRequestIndex<OutstandingRequests.Num(); CurrentRequestIndex++ )
		{
			const FAsyncIORequest& IORequest = OutstandingRequests(CurrentRequestIndex);

			// Highest IO priority always wins.
			if( IORequest.Priority > HighestPriority )
			{
				// Keep track of higher priority.
				HighestPriority				= IORequest.Priority;
				// Reset tracked data as we have a higher priority read request.
				BestRequestIndex			= INDEX_NONE;
				BestRequestIndexCost		= INT_MAX;
				SmallestStartSectorIndex	= INDEX_NONE;
				SmallestStartSector			= INT_MAX;
			}			
			// Skip over requests that are below highest encountered or min priority.
			else if( IORequest.Priority < HighestPriority )
			{
				continue;
			}
			
			// Figure out request start sector based on file start sector and offset in file.
			INT RequestStartSector  = IORequest.FileStartSector + IORequest.Offset / DVD_SECTOR_SIZE;
			
			// Calculate cost, backward seeking has max cost.
			INT RequestCost			= INT_MAX;
			if( RequestStartSector >= LastReadSector )
			{
				RequestCost = RequestStartSector - LastReadSector;
			}

			// Keep track of min cost and request index.
			if( RequestCost < BestRequestIndexCost )
			{
				BestRequestIndexCost	= RequestCost;
				BestRequestIndex		= CurrentRequestIndex;
			}

			// Keep track of smallest start sector and its index.
			if( RequestStartSector < SmallestStartSector )
			{
				SmallestStartSector			= RequestStartSector;
				SmallestStartSectorIndex	= CurrentRequestIndex;
			}
		}

		// No forward seeks found, use smallest request index.
		if( BestRequestIndex == INDEX_NONE )
		{
			BestRequestIndex = SmallestStartSectorIndex;
		}

		// Update LastReadSector for subsequent requests.
		if( BestRequestIndex != INDEX_NONE )
		{
			const FAsyncIORequest& IORequest = OutstandingRequests( BestRequestIndex );
			LastReadSector = IORequest.FileStartSector + (IORequest.Offset + IORequest.Size) / DVD_SECTOR_SIZE;
		}
	}

	return BestRequestIndex;
}

/*----------------------------------------------------------------------------
	End.
----------------------------------------------------------------------------*/

