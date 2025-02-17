/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


#ifndef BEST_FIT_ALLOCATOR_H
#define BEST_FIT_ALLOCATOR_H


#define LOG_EVERY_ALLOCATION			0
#define DUMP_ALLOC_FREQUENCY			0 // 100

/*-----------------------------------------------------------------------------
	Custom fixed size pool best fit texture memory allocator
-----------------------------------------------------------------------------*/

/**
 * Simple best fit allocator, splitting and coalescing whenever/ wherever possible.
 * NOT THREAD-SAFE.
 *
 * - uses TMap to find memory chunk given a pointer (potentially colliding with malloc/ free from main thread)
 * - uses separate linked list for free allocations, assuming that relatively few free chunks due to coalescing
 */
struct FBestFitAllocator
{
	/**
	 * Contains information of a single allocation or free block.
	 */
	class FMemoryChunk
	{
	public:
		/**
		 * Private constructor.
		 *
		 * @param	InBase					Pointer to base of chunk
		 * @param	InSize					Size of chunk
		 * @param	ChunkToInsertAfter		Chunk to insert this after.
		 * @param	FirstFreeChunk			Reference to first free chunk pointer.
		 */
		FMemoryChunk( BYTE* InBase, INT InSize, FMemoryChunk*& ChunkToInsertAfter, FMemoryChunk*& InFirstChunk, FMemoryChunk*& InFirstFreeChunk )
		:	Base(InBase)
		,	Size(InSize)
		,	bIsAvailable(FALSE)
		,	FirstChunk(InFirstChunk)
		,	FirstFreeChunk(InFirstFreeChunk)
		{
			Link( ChunkToInsertAfter );
			// This is going to change bIsAvailable.
			LinkFree();
		}
		
		/**
		 * Unlinks/ removes the chunk from the linked lists it belongs to.
		 */
		~FMemoryChunk()
		{
			// Remove from linked lists.
			Unlink();
			UnlinkFree();
		}

		/**
		 * Inserts this chunk after the passed in one.
		 *
		 * @param	ChunkToInsertAfter	Chunk to insert after
		 */
		void Link( FMemoryChunk*& ChunkToInsertAfter )
		{
			if( ChunkToInsertAfter )
			{
				NextChunk		= ChunkToInsertAfter->NextChunk;
				PreviousChunk	= ChunkToInsertAfter;
				ChunkToInsertAfter->NextChunk = this;
				if( NextChunk )
				{
					NextChunk->PreviousChunk = this;
				}				
			}
			else
			{
				PreviousChunk		= NULL;
				NextChunk			= NULL;
				ChunkToInsertAfter	= this;
			}
		}

		/**
		 * Inserts this chunk at the head of the free chunk list.
		 */
		void LinkFree()
		{
			check(!bIsAvailable);
			bIsAvailable = TRUE;

			if( FirstFreeChunk )
			{
				NextFreeChunk		= FirstFreeChunk;
				PreviousFreeChunk	= NULL;
				FirstFreeChunk->PreviousFreeChunk = this;
				FirstFreeChunk		= this;
			}
			else
			{
				PreviousFreeChunk	= NULL;
				NextFreeChunk		= NULL;
				FirstFreeChunk		= this;
			}
		}

		/**
		 * Removes itself for linked list.
		 */
		void Unlink()
		{
			if( PreviousChunk )
			{
				PreviousChunk->NextChunk = NextChunk;
			}
			else
			{
				FirstChunk = NextChunk;
			}
			
			if( NextChunk )
			{
				NextChunk->PreviousChunk = PreviousChunk;
			}

			PreviousChunk	= NULL;
			NextChunk		= NULL;
		}

		/**
		 * Removes itself for linked "free" list.
		 */
		void UnlinkFree()
		{
			check(bIsAvailable);
			bIsAvailable = FALSE;

			if( PreviousFreeChunk )
			{
				PreviousFreeChunk->NextFreeChunk = NextFreeChunk;
			}
			else
			{
				FirstFreeChunk = NextFreeChunk;
			}

			if( NextFreeChunk )
			{
				NextFreeChunk->PreviousFreeChunk = PreviousFreeChunk;
			}

			PreviousFreeChunk	= NULL;
			NextFreeChunk		= NULL;
		}

#if USE_ALLOCATORFIXEDSIZEFREELIST
		/** Custom new/delete */
		void* operator new(size_t Size);
		void operator delete(void *RawMemory);
#endif

		/** Base of chunk.								*/
		BYTE*					Base;
		/** Size of chunk.								*/
		INT						Size;
		/** Whether chunk is available or not.			*/
		UBOOL					bIsAvailable;

		/** First chunk.								*/
		FMemoryChunk*&			FirstChunk;
		/** First free chunk.							*/
		FMemoryChunk*&			FirstFreeChunk;
		/** Pointer to previous chunk.					*/
		FMemoryChunk*			PreviousChunk;
		/** Pointer to next chunk.						*/
		FMemoryChunk*			NextChunk;

		/** Pointer to previous free chunk.				*/
		FMemoryChunk*			PreviousFreeChunk;
		/** Pointer to next free chunk.					*/
		FMemoryChunk*			NextFreeChunk;
	};

	/** Constructor, zero initializing all member variables */
	FBestFitAllocator()
	:	MemorySize(0)
	,	MemoryBase(NULL)
	,	AllocationAlignment(0)
	,	FirstChunk(NULL)
	,	FirstFreeChunk(NULL)
	,	TimeSpentInAllocator(0.0)
	,	AllocatedMemorySize(0)
	,	AvailableMemorySize(0)
	{}

	/**
	 * Initialize this allocator with 
	 */
	void Initialize( BYTE* InMemoryBase, INT InMemorySize, INT InAllocationAlignment )
	{
		// Update size, pointer and alignment.
		MemoryBase			= InMemoryBase;
		MemorySize			= InMemorySize;
		AllocationAlignment	= InAllocationAlignment;
		check( Align( MemoryBase, AllocationAlignment ) == MemoryBase );

		// Update stats in a thread safe way.
		appInterlockedExchange( &AvailableMemorySize, MemorySize );
		// Allocate initial chunk.
		FirstChunk			= new FMemoryChunk( MemoryBase, MemorySize, FirstChunk, FirstChunk, FirstFreeChunk );
	}

	/**
	 * Returns whether allocator has been initialized.
	 */
	UBOOL IsInitialized()
	{
		return MemoryBase != NULL;
	}

	/**
	 * Allocate physical memory.
	 *
	 * @param	AllocationSize	Size of allocation
	 * @param	bAllowFailure	Whether to allow allocation failure or not
	 * @return	Pointer to allocated memory
	 */
	void* Allocate( INT AllocationSize, UBOOL bAllowFailure );

	/**
	 * Frees allocation associated with passed in pointer.
	 *
	 * @param	Pointer		Pointer to free.
	 */
	void Free( void* Pointer );

	/**
	 * Tries to reallocate texture memory in-place (without relocating),
	 * by adjusting the base address of the allocation but keeping the end address the same.
	 *
	 * @param	OldBaseAddress	Pointer to the original allocation
	 * @param	NewBaseAddress	New desired baseaddress for the allocation (adjusting the size so the end stays the same)
	 * @returns	TRUE if it succeeded
	 **/
	UBOOL Reallocate( void* OldBaseAddress, void* NewBaseAddress );

	/**
	 * Dump allocation information.
	 */
	void DumpAllocs( FOutputDevice& Ar=*GLog );

	/**
	 * Retrieves allocation stats.
	 *
	 * @param	OutAllocatedMemorySize	[out]	Size of allocated memory
	 * @param	OutAvailableMemorySize	[out]	Size of available memory
	 */
	void GetMemoryStats( INT& OutAllocatedMemorySize, INT& OutAvailableMemorySize )
	{
		OutAllocatedMemorySize = AllocatedMemorySize;
		OutAvailableMemorySize = AvailableMemorySize;
	}

	/**
	 * Scans the free chunks and returns the largest size you can allocate.
	 *
	 * @param OutNumFreeChunks	Upon return, contains the total number of free chunks. May be NULL.
	 * @return					The largest size of all free chunks.
	 */
	INT GetLargestAvailableAllocation( INT* OutNumFreeChunks=NULL );

	/**
	 * Helper class to carry out the task of moving memory around.
	 */
	struct FDefragmentationPolicy
	{
		/**
		 * Copy memory from one location to another. If it returns FALSE, the defragmentation
		 * process will assume the memory is not relocatable and keep it in place.
		 *
		 * @param Dest		Destination memory start address
		 * @param Source	Source memory start address
		 * @param Size		Number of bytes to copy
		 * @return			FALSE if the copy failed
		 */
		virtual UBOOL Relocate( void* Dest, const void* Source, INT Size ) = 0;
	};

	/**
	 * Defragment the memory. Memory is moved around using the specified policy.
	 * The function tries to perform non-overlapping memory transfers as much as possible.
	 *
	 * @param Policy	Helper object that carries out the task of moving around memory
	 */
	void DefragmentMemory( FDefragmentationPolicy &Policy );

protected:

	/**
	 * Split allocation into two, first chunk being used and second being available.
	 *
	 * @param	BaseChunk	Chunk to split
	 * @param	FirstSize	New size of first chunk
	 */
	void Split( FMemoryChunk* BaseChunk, INT FirstSize )
	{
		check( BaseChunk );
		check( FirstSize < BaseChunk->Size );
		check( FirstSize > 0 );
		check( !BaseChunk->NextChunk || !BaseChunk->NextChunk->bIsAvailable );
		check( !BaseChunk->PreviousChunk || !BaseChunk->PreviousChunk->bIsAvailable || !BaseChunk->bIsAvailable );

		// Calculate size of second chunk...
		INT SecondSize = BaseChunk->Size - FirstSize;
		// ... and create it.
		new FMemoryChunk( BaseChunk->Base + FirstSize, SecondSize, BaseChunk, FirstChunk, FirstFreeChunk );

		// Resize base chunk.
		BaseChunk->Size = FirstSize;
	}

	/**
	 * Frees the passed in chunk and coalesces if possible.
	 *
	 * @param	Chunk	 Chunk to mark as available. 
	 */
	void FreeChunk( FMemoryChunk* Chunk )
	{
		check(Chunk);
		// Mark chunk as available.
		Chunk->LinkFree();
		// Kick of merge pass.
		Coalesce( Chunk );
		// Not save to access Chunk at this point as Coalesce might have deleted it!
	}

	/**
	 * Tries to coalesce/ merge chunks now that we have a new freed one.
	 *
	 * @param	FreedChunk	Chunk that just became available.
	 */
	void Coalesce( FMemoryChunk* FreedChunk );

	/**
	 * Deletes the passed in chunk after unlinking and returns the next one.
	 *
	 * @param	Chunk	Chunk to delete
	 * @return	the next chunk pointed to by deleted one
	 */
	FMemoryChunk* DeleteAndReturnNext( FMemoryChunk* Chunk )
	{
		// Keep track of chunk to return.
		FMemoryChunk* ChunkToReturn = Chunk->NextChunk;
		// Deletion will unlink.
		delete Chunk;
		return ChunkToReturn;
	}

	/** Size of memory pool.										*/
	INT									MemorySize;
	/** Base of memory pool.										*/
	BYTE*								MemoryBase;
	/** Allocation alignment requirements.							*/
	INT									AllocationAlignment;
	/** Head of linked list of chunks. Sorted by memory address.	*/
	FMemoryChunk*						FirstChunk;
	/** Head of linked list of free chunks.	Unsorted.				*/
	FMemoryChunk*						FirstFreeChunk;
	/** Cummulative time spent in allocator.						*/
	DOUBLE								TimeSpentInAllocator;
	/** Allocated memory in bytes.									*/
	volatile INT						AllocatedMemorySize;
	/** Available memory in bytes.									*/
	volatile INT						AvailableMemorySize;
	/** Mapping from pointer to chunk for fast removal.				*/
	TMap<PTRINT,FMemoryChunk*>			PointerToChunkMap;
};


/**
 * Wrapper class around a best fit allocator that handles running out of memory without
 * returning NULL, but instead returns a preset pointer and marks itself as "corrupt".
 * THREAD-SAFE.
 */
struct FPresizedMemoryPool
{
	/**
	 * Constructor, initializes the BestFitAllocator (will allocate physical memory!)
	 * 
	 * @param PoolSize Size of the memory pool
	 * @param Alignment Default alignment for each allocation
	 */
	FPresizedMemoryPool(INT PoolSize, INT Alignment)
	: bIsCorrupted(FALSE)
	, AllocationFailurePointer(NULL)
	, PhysicalMemoryBase(NULL)
	, PhysicalMemorySize(0)
	{
		// Don't add any extra bytes when allocating, so we don't cause unnecessary fragmentation.
		AllocationFailurePointer = (BYTE*)appPhysicalAlloc(PoolSize, CACHE_WriteCombine);

		// Initialize the pool slightly in, so we can distinguish between a real and a failed allocation when we are freeing memory.
		BYTE* PoolMemory = AllocationFailurePointer + Alignment;
		Allocator.Initialize( PoolMemory, PoolSize - Alignment, Alignment );
	}

	/**
	 * Constructor, initializes the BestFitAllocator with already allocated memory
	 * 
	 * @param PoolSize Size of the memory pool
	 * @param Alignment Default alignment for each allocation
	 */
	FPresizedMemoryPool(BYTE* PoolMemory, BYTE* FailedAllocationMemory, INT PoolSize, INT Alignment)
	: bIsCorrupted(FALSE)
	, AllocationFailurePointer(FailedAllocationMemory)
	, PhysicalMemoryBase(NULL)
	, PhysicalMemorySize(0)
	{
		// Initialize allocator.
		Allocator.Initialize(PoolMemory, PoolSize, Alignment );
	}

	/**
	 * Default constructor, to initialize the BestFitAllocator at a later time
	 * 
	 * @param PoolSize Size of the memory pool
	 * @param Alignment Default alignment for each allocation
	 */
	FPresizedMemoryPool()
	: bIsCorrupted(FALSE)
	, AllocationFailurePointer(NULL)
	, PhysicalMemoryBase(NULL)
	, PhysicalMemorySize(0)
	{
	}

	/**
	 * Returns whether allocator has been initialized or not.
	 */
	UBOOL IsInitialized()
	{
		return Allocator.IsInitialized();
	}

	/**
	 * Initializes the BestFitAllocator (will allocate physical memory!)
	 * 
	 * @param PoolSize Size of the memory pool
	 * @param Alignment Default alignment for each allocation
	 * @return TRUE if the initialization succeeded or FALSE if it was already initialized.
	 */
	UBOOL Initialize(INT PoolSize, INT Alignment)
	{
		FScopeLock ScopeLock(&SynchronizationObject);
		if ( !IsInitialized() )
		{
			// Don't add any extra bytes when allocating, so we don't cause unnecessary fragmentation.
			PhysicalMemorySize = PoolSize;
			PhysicalMemoryBase = appPhysicalAlloc(PhysicalMemorySize, CACHE_WriteCombine);

			AllocationFailurePointer = (BYTE*) PhysicalMemoryBase;

			// Initialize the pool slightly in, so we can distinguish between a real and a failed allocation when we are freeing memory.
			BYTE* PoolMemory = AllocationFailurePointer + Alignment;
			Allocator.Initialize( PoolMemory, PoolSize - Alignment, Alignment );
			return TRUE;
		}
		return FALSE;
	}

	/**
	 * Initializes the BestFitAllocator with already allocated memory
	 * 
	 * @param PoolSize Size of the memory pool
	 * @param Alignment Default alignment for each allocation
	 * @return TRUE if the initialization succeeded or FALSE if it was already initialized.
	 */
	UBOOL Initialize(BYTE* PoolMemory, BYTE* FailedAllocationMemory, INT PoolSize, INT Alignment)
	{
		FScopeLock ScopeLock(&SynchronizationObject);
		if ( !IsInitialized() )
		{
			AllocationFailurePointer = FailedAllocationMemory;
			Allocator.Initialize( PoolMemory, PoolSize, Alignment );
			return TRUE;
		}
		return FALSE;
	}

	/**
	 * Allocates texture memory.
	 *
	 * @param	Size			Size of allocation
	 * @param	bAllowFailure	Whether to allow allocation failure or not
	 * @returns					Pointer to allocated memory
	 */
	void* Allocate(DWORD Size, UBOOL bAllowFailure);

	/**
	 * Frees texture memory allocated via Allocate
	 *
	 * @param	Pointer		Allocation to free
	 */
	void Free(void* Pointer);

	/**
	 * Tries to reallocate texture memory in-place (without relocating),
	 * by adjusting the base address of the allocation but keeping the end address the same.
	 *
	 * @param	OldBaseAddress	Pointer to the original allocation
	 * @param	NewBaseAddress	New desired baseaddress for the allocation (adjusting the size so the end stays the same)
	 * @returns	TRUE if it succeeded
	 **/
	UBOOL Reallocate( void* OldBaseAddress, void* NewBaseAddress );

	/**
	 * Retrieves allocation stats.
	 *
	 * @param	OutAllocatedMemorySize	[out]	Size of allocated memory
	 * @param	OutAvailableMemorySize	[out]	Size of available memory
	 */
	inline void GetMemoryStats( INT& AllocatedMemorySize, INT& AvailableMemorySize )
	{
		Allocator.GetMemoryStats(AllocatedMemorySize, AvailableMemorySize);
	}

	/**
	 * Scans the free chunks and returns the largest size you can allocate.
	 *
	 * @param OutNumFreeChunks	Upon return, contains the total number of free chunks. May be NULL.
	 * @return					The largest size of all free chunks.
	 */
	INT GetLargestAvailableAllocation( INT* OutNumFreeChunks=NULL )
	{
		FScopeLock ScopeLock(&SynchronizationObject);
		return Allocator.GetLargestAvailableAllocation( OutNumFreeChunks );
	}

	void DefragmentMemory( FBestFitAllocator::FDefragmentationPolicy &Policy )
	{
		FScopeLock ScopeLock(&SynchronizationObject);
		Allocator.DefragmentMemory( Policy );
	}

	/** True if we have run out of memory in the pool (and therefore returned AllocationFailurePointer) */
	UBOOL bIsCorrupted;

	/** Single pointer to return when an allocation fails */
	BYTE* AllocationFailurePointer;

	/** The pool itself */
	FBestFitAllocator Allocator;

	/** Object used for synchronization via a scoped lock */
	FCriticalSection SynchronizationObject;

	/** Base address of the physical memory */
	void* PhysicalMemoryBase;

	/** Size of the physical memory */
	DWORD PhysicalMemorySize;

};


#endif
