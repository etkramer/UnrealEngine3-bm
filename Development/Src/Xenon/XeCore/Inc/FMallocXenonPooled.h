/*=============================================================================
FMallocXenonPooled.h: Xenon support for Pooled allocator
Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

WARNING: this code is not 64 bit clean as it assumes sizeof(void*)==4
WARNING: and also uses DWORD (== 4 bytes) for size instead of size_t

@todo : synchronization with GPU for physical alloc/ free
=============================================================================*/

#ifndef _F_MALLOC_XENON_POOLED_H_
#define _F_MALLOC_XENON_POOLED_H_

struct FLargeAllocInfo
{
	void*				BaseAddress;		// Virtual base address, required for reallocating memory on load.
	DWORD				Size;				// Size of allocation, required for restoring allocations on load.
	DWORD				Flags;				// Usage flags, useful for gathering statistics (should be removed for final release).
	FLargeAllocInfo*	Next;
	FLargeAllocInfo**	PrevLink;

	void Unlink();
	void Link(FLargeAllocInfo*& Before);
};

// 32768 (== minimum allocation size for non indirect allocations [of which there are a maximum of 256]) * 8192 == 256 MByte
//@todo xenon: this could be significantly less to save more memory.
enum {LARGEALLOC_COUNT	= 8192 };

// Memory flags.
enum EMemoryFlags
{
	MEMORY_Free			= 0,
	MEMORY_Used			= 1,
	MEMORY_Pool			= 2,
	MEMORY_Physical		= 4,
	MEMORY_NoCache		= 8,
	MEMORY_WriteCombine	= 16
};

//@obsolete: the below could be rolled into FPoolTable
class FLargeAlloc
{
public:
	// Variables.

	FLargeAllocInfo			LargeAllocInfo[LARGEALLOC_COUNT];
	FLargeAllocInfo*		AvailablePool;

	// Functions.
	FLargeAllocInfo* Malloc( DWORD Size, UBOOL IsPhysicalAlloc, UBOOL IsPool, ECacheBehaviour CacheBehaviour );
	void Free( FLargeAllocInfo* AllocInfo, UBOOL IsPhysicalAlloc );
	void GetAllocationInfo( SIZE_T& Virtual, SIZE_T& Physical );
	SIZE_T DumpAllocs( FOutputDevice& Ar=*GLog );
	void Init();
};

//
// Optimized Windows virtual memory allocator.
//
class FMallocXenonPooled : public FMalloc
{
private:
	// Counts.
	enum {POOL_COUNT		= 42     };
	enum {POOL_MAX			= 32768+1};

	// Forward declares.
	struct FFreeMem;
	struct FPoolTable;

	// Memory pool info. 32 bytes.
	struct FPoolInfo
	{
		DWORD				Bytes;		// Bytes allocated for pool.
		DWORD				OsBytes;	// Bytes aligned to page size.
		DWORD				Taken;      // Number of allocated elements in this pool, when counts down to zero can free the entire pool.
		FPoolTable*			Table;		// Index of pool.
		FFreeMem*			FirstMem;   // Pointer to first free memory in this pool.
		FLargeAllocInfo*	AllocInfo;	// Pointer to large allocation info.
		FPoolInfo*			Next;
		FPoolInfo**			PrevLink;

		void Link( FPoolInfo*& Before )
		{
			if( Before )
			{
				Before->PrevLink = &Next;
			}
			Next     = Before;
			PrevLink = &Before;
			Before   = this;
		}
		void Unlink()
		{
			if( Next )
			{
				Next->PrevLink = PrevLink;
			}
			*PrevLink = Next;
		}
	};

	// Information about a piece of free memory. 8 bytes.
	struct FFreeMem
	{
		FFreeMem*		Next;		// Next or MemLastPool[], always in order by pool.
		DWORD			Blocks;		// Number of consecutive free blocks here, at least 1.
		FPoolInfo* GetPool()
		{
			return (FPoolInfo*)((INT)this & 0xffff0000);
		}
	};

	// Pool table.
	struct FPoolTable
	{
		/** Indicates whether the pool maps to physical or virtual memory allocations */
		UBOOL				bIsPhysicalAllocation;
		/** Defines the cache behaviour for physical allocations */
		ECacheBehaviour		CacheBehaviour;
		FPoolInfo*			FirstPool;
		FPoolInfo*			ExhaustedPool;
		DWORD				BlockSize;
	};

	// Variables.
	FPoolTable			PoolTable[CACHE_MAX][POOL_COUNT], OsTable;
	FPoolInfo*			PoolIndirect[32];
	INT					MemSizeToIndexInPoolTable[POOL_MAX];
	INT					OsCurrent,OsPeak,UsedCurrent,UsedPeak,CurrentAllocs,TotalAllocs;
	FLOAT				AvailableAtStartupMB;
	DOUBLE				MemTime;
	FLargeAlloc			LargeAlloc;

	// Temporary status information used to not have to break interface for Malloc.
	UBOOL				bIsCurrentAllocationPhysical;
	ECacheBehaviour		CurrentAllocationCacheBehaviour;

    /** Whether or not we are testing for fragmentation.  This will make it so we do not appErrorf but instead return NULL when OOM **/
	UBOOL bTestingForFragmentation;

	// Implementation.
	void OutOfMemory( INT RequestedSize );
	FPoolInfo* CreateIndirect();

	/**
	 * Returns a pool table fitting the passed in parameters.
	 *
	 * @param	Size					size of allocation
	 * @param	bIsPhysicalAllocation	whether allocation is physical or virtual
	 * @param	CacheBehaviour			cache type for allocation
	 * @return	matching pool table
	 */
	FPoolTable* GetPoolTable( INT Size, UBOOL bIsPhysicalAllocation, ECacheBehaviour CacheBehaviour );

public:
	FMallocXenonPooled();

	// FMalloc interface.
	virtual void* Malloc( DWORD Size, DWORD Alignment );
	void* ReallocOriginal( void* Ptr, DWORD NewSize, DWORD Alignment );
	virtual void* Realloc( void* Ptr, DWORD NewSize, DWORD Alignment );
	virtual void Free( void* Ptr );
	virtual void* PhysicalAlloc( DWORD Count, ECacheBehaviour InCacheBehaviour );
	virtual void PhysicalFree( void* Original );
	/**
	 * Gathers memory allocations for both virtual and physical allocations.
	 *
	 * @param Virtual	[out] size of virtual allocations
	 * @param Physical	[out] size of physical allocations	
	 */
	virtual void GetAllocationInfo( SIZE_T& Virtual, SIZE_T& Physical );
	/**
 	 * Keeps trying to allocate memory until we fail
	 *
	 * @param Ar Device to send output to
	 */
	virtual void CheckMemoryFragmentationLevel( class FOutputDevice& Ar );
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar );
	void DumpAllocs( FOutputDevice& Ar );	
	void HeapCheck();

	/** Called every game thread tick */
	virtual void Tick( FLOAT DeltaTime );
};

#endif //_F_MALLOC_XENON_POOLED_H_
