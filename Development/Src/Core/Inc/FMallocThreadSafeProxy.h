/*=============================================================================
	FMallocThreadSafeProxy.h: FMalloc proxy used to render any FMalloc thread
							  safe.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __FMALLOCTHREADSAFEPROXY_H__
#define __FMALLOCTHREADSAFEPROXY_H__

/**
 * FMalloc proxy that synchronizes access, making the used malloc thread safe.
 */
class FMallocThreadSafeProxy : public FMalloc
{
private:
	/** Malloc we're based on, aka using under the hood							*/
	FMalloc*			UsedMalloc;
	/** Object used for synchronization via a scoped lock						*/
	FCriticalSection	SynchronizationObject;

public:
	/**
	 * Constructor for thread safe proxy malloc that takes a malloc to be used and a
	 * synchronization object used via FScopeLock as a parameter.
	 * 
	 * @param	InMalloc					FMalloc that is going to be used for actual allocations
	 */
	FMallocThreadSafeProxy( FMalloc* InMalloc)
	:	UsedMalloc( InMalloc )
	{}

	// FMalloc interface.

	void* Malloc( DWORD Size, DWORD Alignment )
	{
		FScopeLock ScopeLock( &SynchronizationObject );
		STAT(TotalMallocCalls++);
		return UsedMalloc->Malloc( Size, Alignment );
	}
	void* Realloc( void* Ptr, DWORD NewSize, DWORD Alignment )
	{
		FScopeLock ScopeLock( &SynchronizationObject );
		STAT(TotalReallocCalls++);
		return UsedMalloc->Realloc( Ptr, NewSize, Alignment );
	}
	void Free( void* Ptr )
	{
		if( Ptr )
		{
			FScopeLock ScopeLock( &SynchronizationObject );
			STAT(TotalFreeCalls++);
			UsedMalloc->Free( Ptr );
		}
	}
	void* PhysicalAlloc( DWORD Size, ECacheBehaviour InCacheBehaviour )
	{
		FScopeLock ScopeLock( &SynchronizationObject );
		STAT(TotalPhysicalAllocCalls++);
		return UsedMalloc->PhysicalAlloc( Size, InCacheBehaviour );
	}
	void PhysicalFree( void* Ptr )
	{
		if( Ptr )
		{
			FScopeLock ScopeLock( &SynchronizationObject );
			STAT(TotalPhysicalFreeCalls++);
			UsedMalloc->PhysicalFree( Ptr );
		}
	}

	/**
	 * Passes request for gathering memory allocations for both virtual and physical allocations
	 * on to used memory manager.
	 *
	 * @param Virtual	[out] size of virtual allocations
	 * @param Physical	[out] size of physical allocations	
	 */
	void GetAllocationInfo( SIZE_T& Virtual, SIZE_T& Physical )
	{
		FScopeLock ScopeLock( &SynchronizationObject );
		UsedMalloc->GetAllocationInfo( Virtual, Physical );
	}

	/**
	* Gathers PS3 specific allocation information. passes request to internal allocator
	* 
	* @param AllocatedBytes [out] Total number of bytes that are currently allocated for use by game, however this is more than was requested by game, but less than amount actually in-use by malloc
	* @param AveragePaddedBytes [out] Average (per allocation) of the number of extra bytes between requested by game and returned by malloc as usable
	* @param CurrentNumAllocations [out] Total number of current allocations
	*/
	void GetPS3AllocationInfo(INT& AllocatedBytes, FLOAT& AveragePaddedBytes, INT& CurrentNumAllocations)
	{
		FScopeLock ScopeLock( &SynchronizationObject );
		UsedMalloc->GetPS3AllocationInfo(AllocatedBytes,AveragePaddedBytes,CurrentNumAllocations);
	}

	/**
	 * Keeps trying to allocate memory until we fail
	 *
	 * @param Ar Device to send output to
	 */
	void CheckMemoryFragmentationLevel( class FOutputDevice& Ar )
	{
		FScopeLock ScopeLock( &SynchronizationObject );
		UsedMalloc->CheckMemoryFragmentationLevel( Ar );
	}


	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar )
	{
		FScopeLock ScopeLock( &SynchronizationObject );
		return UsedMalloc->Exec(Cmd, Ar);
	}


	/** Called every game thread tick */
	void Tick( FLOAT DeltaTime )
	{
		FScopeLock ScopeLock( &SynchronizationObject );
		return UsedMalloc->Tick( DeltaTime );
	}


	/**
	* If possible give memory back to the os from unused segments
	*
	* @param ReservePad - amount of space to reserve when trimming
	* @param bShowStats - log stats about how much memory was actually trimmed. Disable this for perf
	* @return TRUE if succeeded
	*/
	UBOOL TrimMemory(SIZE_T ReservePad,UBOOL bShowStats=FALSE)
	{
		FScopeLock ScopeLock( &SynchronizationObject );
		return UsedMalloc->TrimMemory(ReservePad,bShowStats);
	}
};

#endif
