/*=============================================================================
	BestFitAllocator.cpp: Unreal memory best fit allocator
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"
#include "BestFitAllocator.h"

/*-----------------------------------------------------------------------------
	FBestFitAllocator implementation.
-----------------------------------------------------------------------------*/

// Determines whether memory defragmentation will try to minimize the
// number of overlapping relocations by doing a few extra temporary relocations.
// This can be faster if the platform can do non-overlapped memory transfers
// very fast compared to overlapped memory transfers (e.g. PS3).
#if PS3
	#define MINIMIZE_NUMBER_OF_OVERLAPPING_RELOCATIONS 1
#else
	#define MINIMIZE_NUMBER_OF_OVERLAPPING_RELOCATIONS 0
#endif


/**
 * Allocate physical memory.
 *
 * @param	AllocationSize	Size of allocation
 * @param	bAllowFailure	Whether to allow allocation failure or not
 * @return	Pointer to allocated memory
 */
void* FBestFitAllocator::Allocate( INT AllocationSize, UBOOL bAllowFailure )
{
	SCOPE_SECONDS_COUNTER(TimeSpentInAllocator);
	check( FirstChunk );

	// Make sure everything is appropriately aligned.
	AllocationSize = Align( AllocationSize, AllocationAlignment );

	// Perform a "best fit" search, returning first perfect fit if there is one.
	FMemoryChunk* CurrentChunk	= FirstFreeChunk;
	FMemoryChunk* BestChunk		= NULL;
	while( CurrentChunk )
	{
		// Check whether chunk is available and large enough to hold allocation.
		check( CurrentChunk->bIsAvailable );
		if( CurrentChunk->Size >= AllocationSize )
		{
			// Compare with current best chunk if one exists. 
			if( BestChunk )
			{
				// Tighter fits are preferred.
				if( CurrentChunk->Size < BestChunk->Size )
				{
					BestChunk = CurrentChunk;
				}
			}
			// No existing best chunk so use this one.
			else
			{
				BestChunk = CurrentChunk;
			}

			// We have a perfect fit, no need to iterate further.
			if( BestChunk->Size == AllocationSize )
			{
				break;
			}
		}
		CurrentChunk = CurrentChunk->NextFreeChunk;
	}

	// Dump allocation info and return NULL if we weren't able to satisfy allocation request.
	if( !BestChunk )
	{
		if ( !bAllowFailure )
		{
#if !FINAL_RELEASE
			DumpAllocs();
			debugf(TEXT("Ran out of memory for allocation in best-fit allocator of size %i KByte"), AllocationSize / 1024);
			GLog->FlushThreadedLogs();
#endif
		}
		return NULL;
	}

	// Mark as being in use.
	BestChunk->UnlinkFree();

	// Split chunk to avoid waste.
	if( BestChunk->Size > AllocationSize )
	{
		Split( BestChunk, AllocationSize );
	}

	// Ensure that everything's in range.
	check( (BestChunk->Base + BestChunk->Size) <= (MemoryBase + MemorySize) );
	check( BestChunk->Base >= MemoryBase );

	// Update usage stats in a thread safe way.
	appInterlockedAdd( &AllocatedMemorySize, +BestChunk->Size );
	appInterlockedAdd( &AvailableMemorySize, -BestChunk->Size );

	// Keep track of mapping and return pointer.
	PointerToChunkMap.Set( (PTRINT) BestChunk->Base, BestChunk );
	return BestChunk->Base;
}

/**
 * Frees allocation associated with passed in pointer.
 *
 * @param	Pointer		Pointer to free.
 */
void FBestFitAllocator::Free( void* Pointer )
{
	SCOPE_SECONDS_COUNTER(TimeSpentInAllocator);

	// Look up pointer in TMap.
	FMemoryChunk* MatchingChunk = PointerToChunkMap.FindRef( (PTRINT) Pointer );
	check( MatchingChunk );
	// Remove the entry
	PointerToChunkMap.Remove((PTRINT) Pointer);

	// Update usage stats in a thread safe way.
	appInterlockedAdd( &AllocatedMemorySize, -MatchingChunk->Size );
	appInterlockedAdd( &AvailableMemorySize, +MatchingChunk->Size );

	// Free the chunk.
	FreeChunk(MatchingChunk);
}

/**
 * Tries to reallocate texture memory in-place (without relocating),
 * by adjusting the base address of the allocation but keeping the end address the same.
 *
 * @param	OldBaseAddress	Pointer to the original allocation
 * @param	NewBaseAddress	New desired baseaddress for the allocation (adjusting the size so the end stays the same)
 * @returns	TRUE if it succeeded
 **/
UBOOL FBestFitAllocator::Reallocate( void* OldBaseAddress, void* NewBaseAddress )
{
	SCOPE_SECONDS_COUNTER(TimeSpentInAllocator);

	// Look up pointer in TMap.
	FMemoryChunk* MatchingChunk = PointerToChunkMap.FindRef( PTRINT(OldBaseAddress) );
	check( MatchingChunk && PTRINT(OldBaseAddress) == PTRINT(MatchingChunk->Base) );

	INT MemoryAdjustment = Abs<INT>(PTRINT(NewBaseAddress) - PTRINT(OldBaseAddress));

	// Are we growing the allocation?
	if ( PTRINT(NewBaseAddress) < PTRINT(OldBaseAddress) )
	{
		// Is there enough free memory immediately before this chunk?
		FMemoryChunk* PrevChunk = MatchingChunk->PreviousChunk;
		if ( PrevChunk && PrevChunk->bIsAvailable && PrevChunk->Size >= MemoryAdjustment )
		{
			PointerToChunkMap.Remove( PTRINT(OldBaseAddress) );

			// Shrink the previous and grow the current chunk.
			PrevChunk->Size -= MemoryAdjustment;
			MatchingChunk->Base -= MemoryAdjustment;
			MatchingChunk->Size += MemoryAdjustment;

			check(PTRINT(NewBaseAddress) == PTRINT(MatchingChunk->Base));
			PointerToChunkMap.Set( PTRINT(NewBaseAddress), MatchingChunk );

			if ( PrevChunk->Size == 0 )
			{
				delete PrevChunk;
			}

			// Update usage stats in a thread safe way.
			appInterlockedAdd( &AllocatedMemorySize, +MemoryAdjustment );
			appInterlockedAdd( &AvailableMemorySize, -MemoryAdjustment );
			return TRUE;
		}
	}
	else
	{
		// We're shrinking the allocation.
		check( MemoryAdjustment <= MatchingChunk->Size );

		FMemoryChunk* PrevChunk = MatchingChunk->PreviousChunk;
		if ( PrevChunk )
		{
			// Shrink the current chunk.
			MatchingChunk->Base += MemoryAdjustment;
			MatchingChunk->Size -= MemoryAdjustment;

			// Grow the previous chunk.
			INT OriginalPrevSize = PrevChunk->Size;
			PrevChunk->Size += MemoryAdjustment;

			// If the previous chunk was "in use", split it and insert a 2nd free chunk.
			if ( !PrevChunk->bIsAvailable )
			{
				Split( PrevChunk, OriginalPrevSize );
			}
		}
		else
		{
			// This was the first chunk, split it.
			Split( MatchingChunk, MemoryAdjustment );

			// We're going to use the new chunk. Mark it as "used memory".
			MatchingChunk = MatchingChunk->NextChunk;
			MatchingChunk->UnlinkFree();

			// Make the original chunk "free memory".
			FreeChunk( MatchingChunk->PreviousChunk );
		}

		check(PTRINT(NewBaseAddress) == PTRINT(MatchingChunk->Base));
		PointerToChunkMap.Remove( PTRINT(OldBaseAddress) );
		PointerToChunkMap.Set( PTRINT(NewBaseAddress), MatchingChunk );

		// Update usage stats in a thread safe way.
		appInterlockedAdd( &AllocatedMemorySize, -MemoryAdjustment );
		appInterlockedAdd( &AvailableMemorySize, +MemoryAdjustment );
		return TRUE;
	}
	return FALSE;
}

/**
 * Dump allocation information.
 */
void FBestFitAllocator::DumpAllocs( FOutputDevice& Ar/*=*GLog*/ )
{		
	// Memory usage stats.
	INT				UsedSize		= 0;
	INT				FreeSize		= 0;
	INT				NumUsedChunks	= 0;
	INT				NumFreeChunks	= 0;
	
	// Fragmentation and allocation size visualization.
	INT				NumBlocks		= MemorySize / AllocationAlignment;
	INT				Dimension		= 1 + NumBlocks / appTrunc(appSqrt(NumBlocks));			
	TArray<FColor>	AllocationVisualization;
	AllocationVisualization.AddZeroed( Dimension * Dimension );
	INT				VisIndex		= 0;

	// Traverse linked list and gather allocation information.
	FMemoryChunk*	CurrentChunk	= FirstChunk;	
	while( CurrentChunk )
	{
		FColor VisColor;
		// Free chunk.
		if( CurrentChunk->bIsAvailable )
		{
			NumFreeChunks++;
			FreeSize += CurrentChunk->Size;
			VisColor = FColor(0,255,0);
		}
		// Allocated chunk.
		else
		{
			NumUsedChunks++;
			UsedSize += CurrentChunk->Size;
			
			// Slightly alternate coloration to also visualize allocation sizes.
			if( NumUsedChunks % 2 == 0 )
			{
				VisColor = FColor(255,0,0);
			}
			else
			{
				VisColor = FColor(192,0,0);
			}
		}

		for( INT i=0; i<(CurrentChunk->Size/AllocationAlignment); i++ )
		{
			AllocationVisualization(VisIndex++) = VisColor;
		}

		CurrentChunk = CurrentChunk->NextChunk;
	}

	check(UsedSize == AllocatedMemorySize);
	check(FreeSize == AvailableMemorySize);

	// Write out bitmap for visualization of fragmentation and allocation patterns.
	appCreateBitmap( TEXT("..\\Binaries\\TextureMemory"), Dimension, Dimension, AllocationVisualization.GetTypedData() );
	Ar.Logf( TEXT("BestFitAllocator: Allocated %i KByte in %i chunks, leaving %i KByte in %i chunks."), UsedSize / 1024, NumUsedChunks, FreeSize / 1024, NumFreeChunks );
	Ar.Logf( TEXT("BestFitAllocator: %5.2f ms in allocator"), TimeSpentInAllocator * 1000 );
}

/**
 * Scans the free chunks and returns the largest size you can allocate.
 *
 * @param OutNumFreeChunks	Upon return, contains the total number of free chunks. May be NULL.
 * @return					The largest size of all free chunks.
 */
INT FBestFitAllocator::GetLargestAvailableAllocation( INT* OutNumFreeChunks/*=NULL*/ )
{
	INT NumFreeChunks = 0;
	INT LargestChunkSize = 0;
	FMemoryChunk* FreeChunk = FirstFreeChunk;
	while (FreeChunk)
	{
		NumFreeChunks++;
		LargestChunkSize = Max<INT>( LargestChunkSize, FreeChunk->Size );
		FreeChunk = FreeChunk->NextFreeChunk;
	}
	if ( OutNumFreeChunks )
	{
		*OutNumFreeChunks = NumFreeChunks;
	}
	return LargestChunkSize;
}


/**
 * Defragment the memory. Memory is moved around using the specified policy.
 * The function tries to perform non-overlapping memory transfers as much as possible.
 */
void FBestFitAllocator::DefragmentMemory( FDefragmentationPolicy &Policy )
{
#if !FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE
	DOUBLE StartTime		= appSeconds();
	INT NumHolesBefore		= 0;
	INT NumHolesAfter		= 0;
	INT LargestHoleBefore	= GetLargestAvailableAllocation(&NumHolesBefore);
	INT LargestHoleAfter	= 0;
#endif

	INT TotalRelocationSize	= 0;
	INT NumRelocations		= 0;

	// Find the first free chunk.
	FMemoryChunk* AvailableChunk = FirstChunk;
	while ( AvailableChunk && !AvailableChunk->bIsAvailable )
	{
		AvailableChunk = AvailableChunk->NextChunk;
	}
	// Process the next used chunk.
	FMemoryChunk* Chunk = AvailableChunk ? AvailableChunk->NextChunk : NULL;

	// Relocate all subsequent used chunks to the beginning of the free chunk.
	while ( Chunk )
	{
		FMemoryChunk* BestChunk = AvailableChunk;

#if MINIMIZE_NUMBER_OF_OVERLAPPING_RELOCATIONS
		// Would this be an overlapped memory-move?
		if ( Chunk->Size > AvailableChunk->Size )
		{
			// Try to move it out of the way (to the last possible free chunk)!
			FMemoryChunk* AnotherAvailableChunk = FirstFreeChunk;
			while ( AnotherAvailableChunk )
			{
				if ( AnotherAvailableChunk->Size >= Chunk->Size && AnotherAvailableChunk->Base > BestChunk->Base )
				{
					BestChunk = AnotherAvailableChunk;
				}
				AnotherAvailableChunk = AnotherAvailableChunk->NextFreeChunk;
			}
		}
#endif

		UBOOL bCouldRelocate = Policy.Relocate(BestChunk->Base, Chunk->Base, Chunk->Size);
		if ( bCouldRelocate )
		{
			NumRelocations++;
			TotalRelocationSize += Chunk->Size;

			// Update our book-keeping.
			PointerToChunkMap.Remove((PTRINT) Chunk->Base);
			PointerToChunkMap.Set((PTRINT) BestChunk->Base, BestChunk);
			BestChunk->UnlinkFree();			// Mark as being in use.
			if ( BestChunk->Size > Chunk->Size )
			{
				Split(BestChunk, Chunk->Size);	// Split to create a new free chunk
			}
			else if ( BestChunk->Size < Chunk->Size )
			{
				// Overlapping relocation. We're just "sliding" memory down one step.
				check( Chunk->PreviousChunk == BestChunk );
				INT HoleSize	= BestChunk->Size;
				BestChunk->Size	= Chunk->Size;
				Chunk->Base		= BestChunk->Base + BestChunk->Size;
				Chunk->Size		= HoleSize;
			}

			// Free this chunk and Coalesce.
			FreeChunk(Chunk);
		}
		else
		{
			// Got a non-relocatable used chunk. Try relocate as many others into the free chunk as we can and move on.
			FMemoryChunk* AnotherUsedChunk = Chunk->NextChunk;
			while ( AnotherUsedChunk && AnotherUsedChunk->bIsAvailable )
			{
				AnotherUsedChunk = AnotherUsedChunk->NextChunk;
			}
			while ( AnotherUsedChunk && AvailableChunk && AvailableChunk->bIsAvailable )
			{
				// Find the next used chunk now, before we free the current one.
				FMemoryChunk* NextUsedChunk = AnotherUsedChunk->NextChunk;
				while ( NextUsedChunk && NextUsedChunk->bIsAvailable )
				{
					NextUsedChunk = NextUsedChunk->NextChunk;
				}
				if ( AnotherUsedChunk->Size <= AvailableChunk->Size )
				{
					if ( Policy.Relocate(AvailableChunk->Base, AnotherUsedChunk->Base, AnotherUsedChunk->Size) )
					{
						NumRelocations++;
						TotalRelocationSize += AnotherUsedChunk->Size;

						// Update our book-keeping.
						PointerToChunkMap.Remove((PTRINT) AnotherUsedChunk->Base);
						PointerToChunkMap.Set((PTRINT) AvailableChunk->Base, AvailableChunk);
						AvailableChunk->UnlinkFree();						// Mark as being in use.
						if ( AvailableChunk->Size > AnotherUsedChunk->Size )
						{
							Split(AvailableChunk, AnotherUsedChunk->Size);	// Split to create a new free chunk
							AvailableChunk = AvailableChunk->NextChunk;
						}
						else
						{
							check( AvailableChunk->Size == AnotherUsedChunk->Size );
						}
						FreeChunk(AnotherUsedChunk);
					}
				}
				AnotherUsedChunk = NextUsedChunk;
			}
			// AvailableChunk is now filled up as much as possible. Skip it.
			AvailableChunk = AvailableChunk ? AvailableChunk->NextChunk : NULL;
		}

		// If we used up our current free chunk, find the next one.
		while ( AvailableChunk && !AvailableChunk->bIsAvailable )
		{
			AvailableChunk = AvailableChunk->NextChunk;
		}

		// Process the next used chunk.
		Chunk = AvailableChunk ? AvailableChunk->NextChunk : NULL;
	}

#if !FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE
	DOUBLE Duration = appSeconds() - StartTime;
	LargestHoleAfter = GetLargestAvailableAllocation( &NumHolesAfter );
	GLog->Logf( TEXT("DEFRAG: %.1f ms, Available: %.3f MB, NumRelocations: %d, Relocated: %.3f MB, NumHolesBefore: %d, NumHolesAfter: %d, LargestHoleBefore: %.3f MB, LargestHoleAfter: %.3f MB"),
		Duration*1000.0, AvailableMemorySize/1024.0f/1024.0f, NumRelocations, FLOAT(TotalRelocationSize)/1024.0f/1024.0f, NumHolesBefore, NumHolesAfter, FLOAT(LargestHoleBefore)/1024.f/1024.0f, FLOAT(LargestHoleAfter)/1024.0f/1024.0f );
#endif
}

/**
 * Tries to coalesce/ merge chunks now that we have a new freed one.
 *
 * @param	FreedChunk	Chunk that just became available.
 */
void FBestFitAllocator::Coalesce( FMemoryChunk* FreedChunk )
{
	check( FreedChunk );

	FMemoryChunk* PreviousChunk = FreedChunk->PreviousChunk;
	FMemoryChunk* NextChunk		= FreedChunk->NextChunk;

	// If previous chunk is available, try to merge with it and following ones.
	if( PreviousChunk && PreviousChunk->bIsAvailable )
	{
		FMemoryChunk* CurrentChunk = FreedChunk;
		// This is required to handle the case of AFA (where A=available and F=freed) to ensure that
		// there is only a single A afterwards.
		while( CurrentChunk && CurrentChunk->bIsAvailable )
		{
			PreviousChunk->Size += CurrentChunk->Size;
			// Chunk is no longer needed, delete and move on to next.
			CurrentChunk = DeleteAndReturnNext( CurrentChunk );
		}
	}
	// Try to merge with next chunk.
	else if( NextChunk && NextChunk->bIsAvailable )
	{
		NextChunk->Base -= FreedChunk->Size;
		NextChunk->Size += FreedChunk->Size;
		// Chunk is no longer needed, delete it.
		DeleteAndReturnNext( FreedChunk );
	}
}



/*-----------------------------------------------------------------------------
	FBestFitAllocator implementation.
-----------------------------------------------------------------------------*/

/**
 * Allocates texture memory.
 *
 * @param	Size			Size of allocation
 * @param	bAllowFailure	Whether to allow allocation failure or not
 * @returns					Pointer to allocated memory
 */
void* FPresizedMemoryPool::Allocate(DWORD Size, UBOOL bAllowFailure)
{
	FScopeLock ScopeLock(&SynchronizationObject);

#if DUMP_ALLOC_FREQUENCY
	static INT AllocationCounter;
	if( ++AllocationCounter % DUMP_ALLOC_FREQUENCY == 0 )
	{
		Allocator.DumpAllocs();
	}
#endif

	// Initialize allocator if it hasn't already.
	if (!Allocator.IsInitialized())
	{
	}

	// actually do the allocation
	void* Pointer = Allocator.Allocate(Size, bAllowFailure);

	// We ran out of memory. Instead of crashing rather corrupt the content and display an error message.
	if (Pointer == NULL)
	{
		if ( !bAllowFailure )
		{
			// Mark texture memory as having been corrupted.
			bIsCorrupted = TRUE;
		}

		// Use special pointer, which is being identified by free.
		Pointer = AllocationFailurePointer;
	}

#if LOG_EVERY_ALLOCATION
	INT AllocSize, AvailSize;
	Allocator.GetMemoryStats( AllocSize, AvailSize );
	debugf(TEXT("Texture Alloc: %p  Size: %6i     Alloc: %8i Avail: %8i"), Pointer, Size, AllocSize, AvailSize );
#endif
	return Pointer;
}


/**
 * Frees texture memory allocated via Allocate
 *
 * @param	Pointer		Allocation to free
 */
void FPresizedMemoryPool::Free(void* Pointer)
{
	FScopeLock ScopeLock(&SynchronizationObject);

#if LOG_EVERY_ALLOCATION
	INT AllocSize, AvailSize;
	Allocator.GetMemoryStats( AllocSize, AvailSize );
	debugf(TEXT("Texture Free : %p   Before free     Alloc: %8i Avail: %8i"), Pointer, AllocSize, AvailSize );
#endif
	// we never want to free the special pointer
	if (Pointer != AllocationFailurePointer)
	{
		// do the free
		Allocator.Free(Pointer);
	}
}

/**
 * Tries to reallocate texture memory in-place (without relocating),
 * by adjusting the base address of the allocation but keeping the end address the same.
 *
 * @param	OldBaseAddress	Pointer to the original allocation
 * @param	NewBaseAddress	New desired baseaddress for the allocation (adjusting the size so the end stays the same)
 * @returns	TRUE if it succeeded
 **/
UBOOL FPresizedMemoryPool::Reallocate( void* OldBaseAddress, void* NewBaseAddress )
{
	FScopeLock ScopeLock(&SynchronizationObject);

	// Initialize allocator if it hasn't already.
	if (!Allocator.IsInitialized())
	{
	}

	// Actually try to do the reallocation.
	return Allocator.Reallocate(OldBaseAddress, NewBaseAddress);
}
