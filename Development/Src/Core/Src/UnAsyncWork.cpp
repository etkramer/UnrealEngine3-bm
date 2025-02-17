/*=============================================================================
	UnAsyncWork.cpp: Implementations of queued work classes
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"

/*-----------------------------------------------------------------------------
	FAsyncWorkBase implementation.
-----------------------------------------------------------------------------*/

/**
 * Caches the thread safe counter and uses self deletion to free itself.
 *
 * @param InWorkCompletionCounter	Counter to decrement on completion of task
 */
FAsyncWorkBase::FAsyncWorkBase( FThreadSafeCounter* InWorkCompletionCounter ) 
{
	// Cache the counter used to signal completion by decrementing.
	WorkCompletionCounter = InWorkCompletionCounter;
#if !CONSOLE
	// We're using a thread safe counter to determine whether work is completed.
	if( InWorkCompletionCounter )
	{
		DoneEvent = NULL;
	}
	// Create a manual reset event only if we're not using the counter.
	else
	{
		DoneEvent = GSynchronizeFactory->CreateSynchEvent(TRUE);
	}
#else
	bIsDone = FALSE;
#endif
}

/**
 * Cleans up the event if on non-Console platforms
 */ 
FAsyncWorkBase::~FAsyncWorkBase(void)
{
#if !CONSOLE
	if( DoneEvent )
	{
		GSynchronizeFactory->Destroy(DoneEvent);
	}
#endif
}

/**
 * Called after work has finished. Marks object as being done.
 */ 
void FAsyncWorkBase::Dispose()
{
	if( WorkCompletionCounter )
	{
		// Signal work as being completed by decrementing counter.
		WorkCompletionCounter->Decrement();
		// The queued work code won't access "this" after calling dispose so it's safe to delete here.
		delete this;
	}
	else
	{
#if CONSOLE
		appInterlockedExchange(&bIsDone,TRUE);
#else
		// Now use the event to indicate it's done
		DoneEvent->Trigger();
#endif
	}
}

/**
 * Returns whether the work is done.
 *
 * @return TRUE if work has been completed, FALSE otherwise
 */
UBOOL FAsyncWorkBase::IsDone(void)
{
	// We cannot call this function for auto- destroy objects that use a thread safe counter
	// to signal completion.
	check( WorkCompletionCounter == NULL );
#if CONSOLE
	// Use spin lock on consoles.
	return bIsDone;
#else
	// Use signal to avoid deadlock scenarios on the PC where it is not clear whether each
	// worker thread has its own CPU core or hyper thread.
	check(DoneEvent);
	// Returns true if the event was signaled
	return DoneEvent->Wait(0);
#endif
}

/*-----------------------------------------------------------------------------
	FAsyncCopy.
-----------------------------------------------------------------------------*/

/**
 * Initializes the data.
 */
FAsyncCopy::FAsyncCopy(void* InDest,void* InSrc,DWORD InSize,FThreadSafeCounter* Counter) 
: FAsyncWorkBase( Counter )
, Src(InSrc)
, Dest(InDest)
, Size(InSize)
{}

/**
 * Performs the async copy
 */
void FAsyncCopy::DoWork(void)
{
	// Do the copy
	appMemcpy(Dest,Src,Size);
}

/*-----------------------------------------------------------------------------
	FAsyncCopyAligned.
-----------------------------------------------------------------------------*/

/**
 * Performs the async copy
 */
void FAsyncCopyAligned::DoWork(void)
{
	// Do the cop
#if XBOX
#if _XTL_VER < 2732
	XMemCpy128(Dest,Src,Size);
#else
	XMemCpy128(Dest,Src,Size);
#endif	
#else
	appMemcpy(Dest,Src,Size);
#endif
}

/*-----------------------------------------------------------------------------
	FAsyncUncompress.
-----------------------------------------------------------------------------*/

/**
 * Initializes the data.
 *
 * @param	Flags					Flags to control what method to use for decompression
 * @param	InUncompressedBuffer	Buffer containing uncompressed data
 * @param	InUncompressedSize		Size of uncompressed data in bytes
 * @param	InCompressedBuffer		Buffer compressed data is going to be written to
 * @param	InCompressedSize		Size of CompressedBuffer data in bytes
 * @param	Counter					Counter that is being decremented on completion of work
 * @param	bIsSourcePadded			Whether the source memory is padded with a full cache line at the end
 */
FAsyncUncompress::FAsyncUncompress( ECompressionFlags InFlags, void* InUncompressedBuffer, INT InUncompressedSize, void* InCompressedBuffer, INT InCompressedSize, FThreadSafeCounter* Counter, UBOOL bIsSourcePadded /*= FALSE*/ )
:	FAsyncWorkBase( Counter )
,	UncompressedBuffer( InUncompressedBuffer )
,	UncompressedSize( InUncompressedSize )
,	CompressedBuffer( InCompressedBuffer )
,	CompressedSize( InCompressedSize )
,	Flags( InFlags )
,	bIsSourceMemoryPadded( bIsSourcePadded )
{}

/**
 * Performs the decompression
 */
void FAsyncUncompress::DoWork(void)
{
	// Uncompress from memory to memory.
	verify( appUncompressMemory( Flags, UncompressedBuffer, UncompressedSize, CompressedBuffer, CompressedSize, bIsSourceMemoryPadded ) );
}


/*-----------------------------------------------------------------------------
	FAsyncCompress.
-----------------------------------------------------------------------------*/

/**
 * Initializes the data and creates the event.
 *
 * @param	Flags						Flags to control what method to use and optionally control memory vs speed
 * @param	CompressedBuffer			Buffer compressed data is going to be written to
 * @param	CompressedSize	[in/out]	Size of CompressedBuffer, at exit will be size of compressed data
 * @param	UncompressedBuffer			Buffer containing uncompressed data
 * @param	UncompressedSize			Size of uncompressed data in bytes
 */
FAsyncCompress::FAsyncCompress( ECompressionFlags InFlags, void* InCompressedBuffer, INT& InCompressedSize, void* InUncompressedBuffer, INT InUncompressedSize, FThreadSafeCounter* Counter )
:	FAsyncWorkBase( Counter )
,	CompressedBuffer( InCompressedBuffer )
,	CompressedSize( InCompressedSize )
,	UncompressedBuffer( InUncompressedBuffer )
,	UncompressedSize( InUncompressedSize )
,	Flags( InFlags )
{}

/**
 * Performs the compression
 */
void FAsyncCompress::DoWork(void)
{
	// Compress from memory to memory.
	verify( appCompressMemory( Flags, CompressedBuffer, CompressedSize, UncompressedBuffer, UncompressedSize ) );
}


/*-----------------------------------------------------------------------------
	FAsyncVerify.
-----------------------------------------------------------------------------*/
/**
 * Constructor. 
 * 
 * @param InBuffer Buffer of data to calculate has on. MUST be valid until this task completes (use Counter or pass ownership via bInShouldDeleteBuffer)
 * @param InBufferSize Size of InBuffer
 * @param bInShouldDeleteBuffer TRUE if this task should appFree InBuffer on completion of the verification. Useful for a fire & forget verification
 *        NOTE: If you pass ownership to the task MAKE SURE you are done using the buffer as it could go away at ANY TIME
 * @param InHash Precalculated hash to test the InBuffer's hash against
 * @param Counter Counter to decrement on completion of task if non- NULL
 */
FAsyncSHAVerify::FAsyncSHAVerify(void* InBuffer, INT InBufferSize, UBOOL bInShouldDeleteBuffer, BYTE* InHash, FThreadSafeCounter* Counter)
: FAsyncWorkBase(Counter)
, Buffer(InBuffer)
, BufferSize(InBufferSize)
, bShouldDeleteBuffer(bInShouldDeleteBuffer)
{
	// store a copy of the hash
	appMemcpy(Hash, InHash, sizeof(Hash));
}

/**
 * Constructor. 
 * 
 * @param InBuffer Buffer of data to calculate has on. MUST be valid until this task completes (use Counter or pass ownership via bInShouldDeleteBuffer)
 * @param InBufferSize Size of InBuffer
 * @param bInShouldDeleteBuffer TRUE if this task should appFree InBuffer on completion of the verification. Useful for a fire & forget verification
 *        NOTE: If you pass ownership to the task MAKE SURE you are done using the buffer as it could go away at ANY TIME
 * @param Pathname Pathname to use to have the platform lookup the hash value
 * @param bInIsUnfoundHashAnError TRUE if failing to lookup the 
 * @param Counter Counter to decrement on completion of task if non- NULL
 */
FAsyncSHAVerify::FAsyncSHAVerify(void* InBuffer, INT InBufferSize, UBOOL bInShouldDeleteBuffer, const TCHAR* InPathname, UBOOL bInIsUnfoundHashAnError, FThreadSafeCounter* Counter)
: FAsyncWorkBase(Counter)
, Buffer(InBuffer)
, BufferSize(InBufferSize)
, Pathname(InPathname)
, bIsUnfoundHashAnError(bInIsUnfoundHashAnError)
, bShouldDeleteBuffer(bInShouldDeleteBuffer)
{
	// nothing to do
}

/**
 * Performs the async hash verification
 */
void FAsyncSHAVerify::DoWork()
{
	// default to success
	UBOOL bFailedHashLookup = FALSE;

	debugfSuppressed( NAME_DevSHA, TEXT("FAsyncSHAVerify running for hash [%s]"),*Pathname );

	// if we stored a filename to use to get the hash, get it now
	if (Pathname.Len() > 0)
	{
		// lookup the hash for the file. 
		if (appGetFileSHAHash(*Pathname, Hash) == FALSE)
		{
			// if it couldn't be found, then we don't calculate the hash, and we "succeed" since there's no
			// hash to check against
			bFailedHashLookup = TRUE;
		}
	}

	UBOOL bFailed;

	// if we have a valid hash, check it
	if (!bFailedHashLookup)
	{
		BYTE CompareHash[20];
		// hash the buffer (finally)
		FSHA1::HashBuffer(Buffer, BufferSize, CompareHash);
		
		// make sure it matches
		bFailed = appMemcmp(Hash, CompareHash, sizeof(Hash)) != 0;
	}
	else
	{
		// if it's an error if the hash is unfound, then mark the failure
		bFailed = bIsUnfoundHashAnError;
	}

	// delete the buffer if we should, now that we are done it
	if (bShouldDeleteBuffer)
	{
		appFree(Buffer);
	}

	// if we failed, then call the failure callback
	if (bFailed)
	{
		appOnFailSHAVerification(*Pathname, bFailedHashLookup);
	}
}
