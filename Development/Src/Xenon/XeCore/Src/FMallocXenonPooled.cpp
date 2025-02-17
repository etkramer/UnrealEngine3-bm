/*=============================================================================
FMallocXenonPooled.h: Xenon support for Pooled allocator
Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

WARNING: this code is not 64 bit clean as it assumes sizeof(void*)==4
WARNING: and also uses DWORD (== 4 bytes) for size instead of size_t

@todo : synchronization with GPU for physical alloc/ free
=============================================================================*/

#include "CorePrivate.h"
#include "FMallocXenon.h"
#include "PerfMem.h"
#include "Database.h"

#if USE_FMALLOC_POOLED || USE_FMALLOC_DL

/** 
 * If enabled, 4K pages are used for large (>32k) allocations. This saves an average of 10 MByte for large maps
 * but rules out stats tracking as the indirect pool relies on 64k pointer alignment.
 */
#define USE_4K_PAGES 1

#define MEM_TIME(st)
#define OS_PAGE_SIZE 65536

// Macros to determine whether an allocation is using a 4k page or not.
#define IS_VIRTUAL_4K(Ptr)	((((DWORD)(Ptr)) & 0xC0000000) == 0x00000000)
#define IS_VIRTUAL_64K(Ptr)	((((DWORD)(Ptr)) & 0xC0000000) == 0x40000000)
#define IS_PHYSICAL(Ptr)	((((DWORD)(Ptr)) & 0x80000000) == 0x80000000)
#define IS_PHYSICAL_4K(Ptr)	((((DWORD)(Ptr)) & 0xE0000000) == 0xE0000000)
#define IS_PHYSICAL_64K(Ptr)((((DWORD)(Ptr)) & 0xE0000000) == 0xA0000000)

/*-----------------------------------------------------------------------------
	FLargeAllocInfo
-----------------------------------------------------------------------------*/

void FLargeAllocInfo::Unlink()
{
	if( Next )
	{
		Next->PrevLink = PrevLink;
	}
	*PrevLink = Next;
}
void FLargeAllocInfo::Link(FLargeAllocInfo*& Before)
{
	if(Before)
	{
		Before->PrevLink = &Next;
	}
	Next = Before;
	PrevLink = &Before;
	Before = this;
}

/*-----------------------------------------------------------------------------
	FLargeAlloc
-----------------------------------------------------------------------------*/

FLargeAllocInfo* FLargeAlloc::Malloc( DWORD Size, UBOOL IsPhysicalAlloc, UBOOL IsPool, ECacheBehaviour CacheBehaviour )
{
	// Obtain available allocation info and unlink it from pool.
	check( AvailablePool );
	FLargeAllocInfo* AllocInfo = AvailablePool;
	AvailablePool->Unlink();
	AllocInfo->Flags = 0;

	// Allocate memory and let the OS pick the base address.
	check( AllocInfo->BaseAddress == 0 );
	if( IsPhysicalAlloc )
	{
		AllocInfo->BaseAddress = XPhysicalAlloc( Size, MAXULONG_PTR, 0x10000, PAGE_READWRITE | MEM_LARGE_PAGES );
		if( AllocInfo->BaseAddress )
		{
			if( CacheBehaviour == CACHE_None )
			{
				AllocInfo->Flags |= MEMORY_NoCache;
				XPhysicalProtect( AllocInfo->BaseAddress, Size, PAGE_READWRITE | PAGE_NOCACHE );
			}
			else if( CacheBehaviour == CACHE_WriteCombine )
			{
				AllocInfo->Flags |= MEMORY_WriteCombine;
				XPhysicalProtect( AllocInfo->BaseAddress, Size, PAGE_READWRITE | PAGE_WRITECOMBINE );
			}
		}
	} 
	else
	{
		// use 64 KB aligned pages for all allocations
		// note that there is some wasted virtual address space here for 32 KB allocations atm
		AllocInfo->BaseAddress = VirtualAlloc( NULL, Size, MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE );
	}

	AllocInfo->Size			= Size;
	AllocInfo->Flags		|= (IsPool ? MEMORY_Pool : 0) | MEMORY_Used | (IsPhysicalAlloc ? MEMORY_Physical : 0);

	// address must be aligned to 64 KB
	check( ((DWORD)AllocInfo->BaseAddress & 0xFFFF) == 0 );
	return AllocInfo;
}
void FLargeAlloc::Free( FLargeAllocInfo* AllocInfo, UBOOL IsPhysicalAlloc )
{
	check( AllocInfo );
	check( (AllocInfo->Flags & MEMORY_Physical) == (IsPhysicalAlloc ? MEMORY_Physical : 0) );

	// Release memory.
	if( IsPhysicalAlloc )
	{
		XPhysicalFree( AllocInfo->BaseAddress );
	}
	else
	{
		VirtualFree( AllocInfo->BaseAddress, 0, MEM_RELEASE );
	}

	// Reset status information and add allocation info to available pool.
	AllocInfo->BaseAddress	= NULL;
	AllocInfo->Size			= 0;
	AllocInfo->Flags		= MEMORY_Free;

	AllocInfo->Link( AvailablePool );
}
/**
* Gathers memory allocations for both virtual and physical allocations.
*
* @param Virtual	[out] size of virtual allocations
* @param Physical	[out] size of physical allocations	
*/
void FLargeAlloc::GetAllocationInfo( SIZE_T& Virtual, SIZE_T& Physical )
{
	Virtual		= 0;
	Physical	= 0;
	for( DWORD i=0; i<LARGEALLOC_COUNT; i++ )
	{
		if( LargeAllocInfo[i].Flags & MEMORY_Used )
		{
			if( LargeAllocInfo[i].Flags & MEMORY_Physical )
			{
				Physical += LargeAllocInfo[i].Size;
			}
			else
			{
				Virtual += LargeAllocInfo[i].Size;
			}
		}
	}
}

SIZE_T FLargeAlloc::DumpAllocs( FOutputDevice& Ar )
{
	struct FAllocStats
	{
		DWORD	SizePool,
				SizeOther;
		DWORD	AllocsPool,
				AllocsOther;
		DWORD	Waste;
	};

	FAllocStats Virtual		= { 0 },
				Physical	= { 0 },
				*AllocStats	= NULL;

	for( DWORD i=0; i<LARGEALLOC_COUNT; i++ )
	{
		if( LargeAllocInfo[i].Flags & MEMORY_Used )
		{
			AllocStats = LargeAllocInfo[i].Flags & MEMORY_Physical ? &Physical : &Virtual;
			if( LargeAllocInfo[i].Flags & MEMORY_Pool )
			{
				AllocStats->SizePool += LargeAllocInfo[i].Size;
				AllocStats->AllocsPool++;
			}
			else
			{
				AllocStats->SizeOther += LargeAllocInfo[i].Size;
				AllocStats->AllocsOther++;
			}
			AllocStats->Waste += Align( LargeAllocInfo[i].Size, OS_PAGE_SIZE ) - LargeAllocInfo[i].Size;
		}
	}

	Ar.Logf(TEXT("Large Allocation Status:"));
	Ar.Logf(TEXT("Phys Memory (pool/other): % 5.3fM (%i) / % 5.3fM (%i), Waste: % 5.3fM"), Physical.SizePool / 1024.f / 1024.f, Physical.AllocsPool, Physical.SizeOther / 1024.f / 1024.f , Physical.AllocsOther, Physical.Waste / 1024.f / 1024.f );
	Ar.Logf(TEXT("Virt Memory (pool/other): % 5.3fM (%i) / % 5.3fM (%i), Waste: % 5.3fM"), Virtual.SizePool / 1024.f / 1024.f, Virtual.AllocsPool, Virtual.SizeOther / 1024.f / 1024.f , Virtual.AllocsOther, Virtual.Waste / 1024.f / 1024.f );

	return Physical.SizeOther + Physical.SizePool + Virtual.SizeOther + Virtual.SizePool + Physical.Waste + Virtual.Waste;
}

void FLargeAlloc::Init()
{
	// Initialize allocation info and pointer data.
	AvailablePool = NULL;
	for( DWORD i=0; i<LARGEALLOC_COUNT; i++ )
	{
		LargeAllocInfo[i].BaseAddress	= 0;
		LargeAllocInfo[i].Size			= 0;
		LargeAllocInfo[i].Flags			= MEMORY_Free;
		LargeAllocInfo[i].Link( AvailablePool );
	}
}

/*-----------------------------------------------------------------------------
	FMallocXenonPooled
-----------------------------------------------------------------------------*/

enum EMemoryDetailedStats
{
	STAT_MemDetailedTFP = STAT_MemoryDetailedFirstStat,
	STAT_MemDetailedOsDiff,
	STAT_MemDetailedPhysAllocSize,
	STAT_MemDetailedPhysPooled,
	STAT_MemDetailedPhysPooledWaste,
	STAT_MemDetailedPhysNonPooled,
	STAT_MemDetailedPhysNonPooledWaste,
	STAT_MemDetailedVirtualAllocSize,
	STAT_MemDetailedVirtualPooled,
	STAT_MemDetailedVirtualPooledWaste,
	STAT_MemDetailedVirtualNonPooled,
	STAT_MemDetailedVirtualNonPooledWaste,
	STAT_MemDetailedTotalPooledWaste,
	STAT_MemDetailedTotalNonPooledWaste,
	STAT_MemDetailedXMemAlloc,
};

DECLARE_STATS_GROUP(TEXT("MemDetailed"),STATGROUP_MemoryDetailed);
DECLARE_MEMORY_STAT(TEXT("Title Free Pages"),STAT_MemDetailedTFP,STATGROUP_MemoryDetailed);
DECLARE_MEMORY_STAT(TEXT("  OS - Accounted"),STAT_MemDetailedOsDiff,STATGROUP_MemoryDetailed);
DECLARE_MEMORY_STAT(TEXT("Physical allocations"),STAT_MemDetailedPhysAllocSize,STATGROUP_MemoryDetailed);
DECLARE_MEMORY_STAT(TEXT("  Pooled"),STAT_MemDetailedPhysPooled,STATGROUP_MemoryDetailed);
DECLARE_MEMORY_STAT(TEXT("  Pooled waste"),STAT_MemDetailedPhysPooledWaste,STATGROUP_MemoryDetailed);
DECLARE_MEMORY_STAT(TEXT("  Non-pooled"),STAT_MemDetailedPhysNonPooled,STATGROUP_MemoryDetailed);
DECLARE_MEMORY_STAT(TEXT("  Non-pooled waste"),STAT_MemDetailedPhysNonPooledWaste,STATGROUP_MemoryDetailed);
DECLARE_MEMORY_STAT(TEXT("Virtual allocations"),STAT_MemDetailedVirtualAllocSize,STATGROUP_MemoryDetailed);
DECLARE_MEMORY_STAT(TEXT("  Pooled"),STAT_MemDetailedVirtualPooled,STATGROUP_MemoryDetailed);
DECLARE_MEMORY_STAT(TEXT("  Pooled waste"),STAT_MemDetailedVirtualPooledWaste,STATGROUP_MemoryDetailed);
DECLARE_MEMORY_STAT(TEXT("  Non-pooled"),STAT_MemDetailedVirtualNonPooled,STATGROUP_MemoryDetailed);
DECLARE_MEMORY_STAT(TEXT("  Non-pooled waste"),STAT_MemDetailedVirtualNonPooledWaste,STATGROUP_MemoryDetailed);
DECLARE_MEMORY_STAT(TEXT("Total pooled waste"),STAT_MemDetailedTotalPooledWaste,STATGROUP_MemoryDetailed);
DECLARE_MEMORY_STAT(TEXT("Total non-pooled waste"),STAT_MemDetailedTotalNonPooledWaste,STATGROUP_MemoryDetailed);
DECLARE_MEMORY_STAT(TEXT("XMemAlloc allocations"),STAT_MemDetailedXMemAlloc,STATGROUP_MemoryDetailed);

/** Called every game thread tick */
void FMallocXenonPooled::Tick( FLOAT DeltaTime )
{
#if EXPORT_MEM_STATS_TO_PIX && !USE_4K_PAGES
	appSetCounterValue( TEXT("XePool.UsedCurrent"), UsedCurrent / 1024.0f / 1024.0f  );
	appSetCounterValue( TEXT("XePool.OsCurrent"), OsCurrent / 1024.0f / 1024.0f );
	appSetCounterValue( TEXT("XePool.CurrentAllocs"), ( FLOAT )( CurrentAllocs ) );
	appSetCounterValue( TEXT("XePool.AvailableAtStartupMB"), ( FLOAT )( AvailableAtStartupMB ) );
#endif

#if !USE_4K_PAGES && ALLOW_NON_APPROVED_FOR_SHIPPING_LIB && STATS
	// Calculate pooled use and waste.
	INT PhysTotalPooled			= 0;
	INT PhysTotalPooledWaste	= 0;
	INT VirtualTotalPooled		= 0;
	INT VirtualTotalPooledWaste	= 0;

	// Go over each set of FPoolTables that corresponds to a cache type
	for( INT CacheTypeIndex = 0; CacheTypeIndex < CACHE_MAX; CacheTypeIndex++ )
	{
		// Only CACHE_Virtual memory is in the virtual pool.
		UBOOL bIsPhysicalAllocation = TRUE;
		if( CacheTypeIndex == CACHE_Virtual )
		{
			bIsPhysicalAllocation = FALSE;
		}

		// Iterate over all pools and their available and exhausted lists to gather information
		// of memory allocated and memory wasted. Together they should add up to the amount of
		// memory allocated from the OS.
		INT TotalPooled = 0;
		INT TotalPooledWaste = 0;
		for( INT i=0; i<POOL_COUNT; i++ )
		{
			FPoolTable* Table = &PoolTable[CacheTypeIndex][i];
			INT AllocCount=0;
			INT OsAllocation=0;
			for( INT i=0; i<2; i++ )
			{
				for( FPoolInfo* Pool=(i?Table->FirstPool:Table->ExhaustedPool); Pool; Pool=Pool->Next )
				{
					AllocCount += Pool->Taken;
					OsAllocation += Pool->OsBytes;
				}
			}
			TotalPooled	+= AllocCount*Table->BlockSize;
			TotalPooledWaste += OsAllocation - AllocCount*Table->BlockSize;
		}

		// Increment global trackers with this summary for this cache type.
		if( bIsPhysicalAllocation )
		{
			PhysTotalPooled += TotalPooled;
			PhysTotalPooledWaste += TotalPooledWaste;
		}
		else
		{
			VirtualTotalPooled += TotalPooled;
			VirtualTotalPooledWaste += TotalPooledWaste;
		}
	}

	// Iterate over large alloc pool.
	INT PhysTotalNonPooled				= 0;
	INT PhysTotalNonPooledWaste			= 0;
	INT VirtualTotalNonPooled			= 0;
	INT VirtualTotalNonPooledWaste		= 0;
	for( DWORD i=0; i<LARGEALLOC_COUNT; i++ )
	{
		// We only care about non-pooled used allocations.
		if( (LargeAlloc.LargeAllocInfo[i].Flags & (MEMORY_Used | MEMORY_Pool)) == MEMORY_Used )
		{
			// Waste is measured with 64k pages.
			INT OsAllocated = Align( LargeAlloc.LargeAllocInfo[i].Size, OS_PAGE_SIZE );
			INT Waste = OsAllocated - LargeAlloc.LargeAllocInfo[i].Size;
			
			if( LargeAlloc.LargeAllocInfo[i].Flags & MEMORY_Physical )
			{
				PhysTotalNonPooled += OsAllocated;
				PhysTotalNonPooledWaste += Waste;
			}
			else
			{
				VirtualTotalNonPooled += OsAllocated;
				VirtualTotalNonPooledWaste += Waste;
			}
		}
	}

	// Get TFP stats as reported by PIX from OS.
	DM_MEMORY_STATISTICS DmMemStat;
	DmMemStat.cbSize = sizeof(DmMemStat);
	DmQueryTitleMemoryStatistics( &DmMemStat );
	extern SIZE_T XMemAllocationSize;

	INT TotalPhys = PhysTotalPooled + PhysTotalPooledWaste + PhysTotalNonPooled + PhysTotalNonPooledWaste;
	INT TotalVirtual = VirtualTotalPooled + VirtualTotalPooledWaste + VirtualTotalNonPooled + VirtualTotalNonPooledWaste;
	INT OsDiff = (DmMemStat.TotalPages - DmMemStat.AvailablePages) * 4096 - TotalPhys - TotalVirtual - XMemAllocationSize;

	SET_DWORD_STAT( STAT_MemDetailedTFP, (DmMemStat.AvailablePages * 4096) );
	SET_DWORD_STAT( STAT_MemDetailedOsDiff, OsDiff );

	SET_DWORD_STAT( STAT_MemDetailedPhysAllocSize, TotalPhys );
	SET_DWORD_STAT( STAT_MemDetailedPhysPooled, PhysTotalPooled );
	SET_DWORD_STAT( STAT_MemDetailedPhysPooledWaste, PhysTotalPooledWaste );
	SET_DWORD_STAT( STAT_MemDetailedPhysNonPooled, PhysTotalNonPooled );
	SET_DWORD_STAT( STAT_MemDetailedPhysNonPooledWaste, PhysTotalNonPooledWaste );

	SET_DWORD_STAT( STAT_MemDetailedVirtualAllocSize, TotalVirtual );
	SET_DWORD_STAT( STAT_MemDetailedVirtualPooled, VirtualTotalPooled );
	SET_DWORD_STAT( STAT_MemDetailedVirtualPooledWaste, VirtualTotalPooledWaste );
	SET_DWORD_STAT( STAT_MemDetailedVirtualNonPooled, VirtualTotalNonPooled );
	SET_DWORD_STAT( STAT_MemDetailedVirtualNonPooledWaste, VirtualTotalNonPooledWaste );

	SET_DWORD_STAT( STAT_MemDetailedTotalPooledWaste, (PhysTotalPooledWaste + VirtualTotalPooledWaste) );
	SET_DWORD_STAT( STAT_MemDetailedTotalNonPooledWaste, (PhysTotalNonPooledWaste + VirtualTotalNonPooledWaste) );
	SET_DWORD_STAT( STAT_MemDetailedXMemAlloc, XMemAllocationSize );
#endif
}

// Implementation.
void FMallocXenonPooled::OutOfMemory( INT RequestedSize )
{
	// do not end the world if we are testing for fragmentation
	if( bTestingForFragmentation == TRUE )
	{
		return;
	}

	static UBOOL bHasAlreadyBeenCalled = FALSE;
	// Avoid re-entrancy if we don't have enough memory left to print out error message.
	if( !bHasAlreadyBeenCalled )
	{
		bHasAlreadyBeenCalled = TRUE;

		if( FString(appCmdLine()).InStr( TEXT( "DoingASentinelRun=1" ), FALSE, TRUE ) != -1 )
		{
			const FString EndRun = FString::Printf(TEXT("EXEC EndRun @RunID=%i, @ResultDescription='%s'")
				, GSentinelRunID
				, *PerfMemRunResultStrings[ARR_OOM] 
			);

			//warnf( TEXT("%s"), *EndRun );
			GTaskPerfMemDatabase->SendExecCommand( *EndRun );
		}

		DumpAllocs( *GLog );

		appErrorf( TEXT("OOM: Ran out of memory trying to allocate %i bytes"), RequestedSize );
		// Log OOM value if we allow linking with non approved libs like we do in LTCG-DC.
		appOutputDebugString( *FString::Printf(TEXT("OOM: Ran out of memory trying to allocate %i bytes") LINE_TERMINATOR, RequestedSize ) );

		// Force a consistent crash even in final release configurations.
		INT* ForceCrash = NULL;
		(*ForceCrash) = 3;
	}
	else
	{
		appDebugBreak();
	}
}
FMallocXenonPooled::FPoolInfo* FMallocXenonPooled::CreateIndirect()
{
	FLargeAllocInfo* AllocInfo = LargeAlloc.Malloc( 256*sizeof(FPoolInfo), FALSE, TRUE, CACHE_Normal );
	// We're never going to free this memory so we don't have to keep track of this allocation.
	FPoolInfo* Indirect = (FPoolInfo*) AllocInfo->BaseAddress;
	if( !Indirect )
	{
		if( bTestingForFragmentation == TRUE )
		{
			return NULL;
		}

		OutOfMemory( 256*sizeof(FPoolInfo) );
	}
	return Indirect;
}

/**
* Returns a pool table fitting the passed in parameters.
*
* @param	Size					size of allocation
* @param	bIsPhysicalAllocation	whether allocation is physical or virtual
* @param	CacheBehaviour			cache type for allocation
* @return	matching pool table
*/
FMallocXenonPooled::FPoolTable* FMallocXenonPooled::GetPoolTable( INT Size, UBOOL bIsPhysicalAllocation, ECacheBehaviour CacheBehaviour )
{
	if( Size>=POOL_MAX )
	{
		return &OsTable;
	}
	else
	{
		if( bIsPhysicalAllocation )
		{
			return &PoolTable[CacheBehaviour][MemSizeToIndexInPoolTable[Size]];
		}
		else
		{
			return &PoolTable[CACHE_Virtual][MemSizeToIndexInPoolTable[Size]];
		}
	}
}

FMallocXenonPooled::FMallocXenonPooled()
:	OsCurrent( 0 )
,	OsPeak( 0 )
,	UsedCurrent( 0 )
,	UsedPeak( 0 )
,	CurrentAllocs( 0 )
,	TotalAllocs( 0 )
,	AvailableAtStartupMB( 0 )
,	MemTime( 0.0 )
,	bIsCurrentAllocationPhysical( FALSE )
,	CurrentAllocationCacheBehaviour( CACHE_Normal )
,   bTestingForFragmentation( FALSE )
{
#if EXPORT_MEM_STATS_TO_PIX && !USE_4K_PAGES
	DM_MEMORY_STATISTICS DmMemStat;
	DmMemStat.cbSize = sizeof(DmMemStat);
	DmQueryTitleMemoryStatistics( &DmMemStat );
	AvailableAtStartupMB = DmMemStat.AvailablePages * 4.0f/1024.f; //4KB pages
#endif

	bIsCurrentAllocationPhysical	= FALSE;
	CurrentAllocationCacheBehaviour	= CACHE_Normal;

	// Init OS tables and fill in unused variables with invalid entries.
	OsTable.bIsPhysicalAllocation	= -1;
	OsTable.CacheBehaviour			= CACHE_MAX;
	OsTable.FirstPool				= NULL;
	OsTable.ExhaustedPool			= NULL;
	OsTable.BlockSize				= 0;

	for( INT MemoryType=0; MemoryType<CACHE_MAX; MemoryType++ )
	{
		// Every allocation is at least 8 byte aligned, every allocation >= 16 bytes is at least 16 byte aligned and
		// every allocation that is a power of two is aligned by the same power of two.
		PoolTable[MemoryType][0].bIsPhysicalAllocation		= MemoryType != CACHE_Virtual ? TRUE : FALSE;
		PoolTable[MemoryType][0].CacheBehaviour				= (ECacheBehaviour) MemoryType;
		PoolTable[MemoryType][0].FirstPool					= NULL;
		PoolTable[MemoryType][0].ExhaustedPool				= NULL;
		PoolTable[MemoryType][0].BlockSize					= 8;
		for( DWORD i=1; i<5; i++ )
		{
			PoolTable[MemoryType][i].bIsPhysicalAllocation	= MemoryType != CACHE_Virtual ? TRUE : FALSE;
			PoolTable[MemoryType][i].CacheBehaviour			= (ECacheBehaviour) MemoryType;
			PoolTable[MemoryType][i].FirstPool				= NULL;
			PoolTable[MemoryType][i].ExhaustedPool			= NULL;
			PoolTable[MemoryType][i].BlockSize				= (8<<((i+1)>>2)) + (2<<i);
		}
		for( DWORD i=5; i<POOL_COUNT; i++ )
		{
			PoolTable[MemoryType][i].bIsPhysicalAllocation	= MemoryType != CACHE_Virtual ? TRUE : FALSE;
			PoolTable[MemoryType][i].CacheBehaviour			= (ECacheBehaviour) MemoryType;
			PoolTable[MemoryType][i].FirstPool				= NULL;
			PoolTable[MemoryType][i].ExhaustedPool			= NULL;
			PoolTable[MemoryType][i].BlockSize				= (4+((i+7)&3)) << (1+((i+7)>>2));
		}

		for( DWORD i=0; i<POOL_MAX; i++ )
		{
			DWORD Index;
			for( Index=0; PoolTable[MemoryType][Index].BlockSize<i; Index++ );
			checkSlow(Index<POOL_COUNT);
			MemSizeToIndexInPoolTable[i] = Index;
		}
		check(POOL_MAX-1==PoolTable[MemoryType][POOL_COUNT-1].BlockSize);
	}

	for( DWORD i=0; i<ARRAY_COUNT(PoolIndirect); i++ )
	{
		PoolIndirect[i] = NULL;
	}

	LargeAlloc.Init();
}

void* FMallocXenonPooled::Malloc( DWORD Size, DWORD Alignment )
{
	check(Alignment == DEFAULT_ALIGNMENT && "Alignment is not supported on Xenon, yet");
	checkSlow(Size>=0);
	MEM_TIME(MemTime -= appSeconds());
	STAT(CurrentAllocs++);
	STAT(TotalAllocs++);
	FFreeMem* Free;
	if( Size<POOL_MAX )
	{
		// Allocate from pool.
		FPoolTable* Table			= GetPoolTable( Size, bIsCurrentAllocationPhysical, CurrentAllocationCacheBehaviour );
		FPoolInfo*&	FirstPool		= Table->FirstPool;
		FPoolInfo*& ExhaustedPool	= Table->ExhaustedPool;
		checkSlow(Size<=Table->BlockSize);
		FPoolInfo* Pool = FirstPool;
		if( !Pool )
		{
			// Must create a new pool.
			DWORD Blocks  = 65536 / Table->BlockSize;
			DWORD Bytes   = Blocks * Table->BlockSize;
			checkSlow(Blocks>=1);
			checkSlow(Blocks*Table->BlockSize<=Bytes);

			// Allocate memory.
			FLargeAllocInfo* AllocInfo = LargeAlloc.Malloc( Bytes, bIsCurrentAllocationPhysical, TRUE, CurrentAllocationCacheBehaviour );
			Free = (FFreeMem*) AllocInfo->BaseAddress;
			if( !Free )
			{
				if( bTestingForFragmentation == TRUE )
				{
					return NULL;
				}
				OutOfMemory( Bytes );
			}

			// Create pool in the indirect table.
			FPoolInfo*& Indirect = PoolIndirect[((DWORD)Free>>27)];
			if( !Indirect )
			{
				Indirect = CreateIndirect();
			}
			Pool = &Indirect[((DWORD)Free>>16)&2047];

			// Init pool.
			Pool->Link( FirstPool );
			Pool->Bytes			= Bytes;
			Pool->OsBytes		= Align(Bytes,OS_PAGE_SIZE);
			STAT(OsPeak			= Max(OsPeak,OsCurrent+=Pool->OsBytes));
			Pool->Table			= Table;
			Pool->Taken			= 0;
			Pool->FirstMem		= Free;
			Pool->AllocInfo		= AllocInfo;

			// Create first free item.
			Free->Blocks		= Blocks;
			Free->Next			= NULL;
		}

		// Pick first available block and unlink it.
		Pool->Taken++;
		checkSlow(Pool->FirstMem);
		checkSlow(Pool->FirstMem->Blocks>0);
		Free = (FFreeMem*)((BYTE*)Pool->FirstMem + --Pool->FirstMem->Blocks * Table->BlockSize);
		if( Pool->FirstMem->Blocks==0 )
		{
			Pool->FirstMem = Pool->FirstMem->Next;
			if( !Pool->FirstMem )
			{
				// Move to exhausted list.
				Pool->Unlink();
				Pool->Link( ExhaustedPool );
			}
		}
		STAT(UsedPeak = Max(UsedPeak,UsedCurrent+=Table->BlockSize));
	}
#if USE_4K_PAGES
	else
	{
		if( bIsCurrentAllocationPhysical )
		{
			void* Ptr = XPhysicalAlloc( Size, MAXULONG_PTR, 0x1000, PAGE_READWRITE );
			if( Ptr )
			{
				if( CurrentAllocationCacheBehaviour == CACHE_None )
				{
					XPhysicalProtect( Ptr, Size, PAGE_READWRITE | PAGE_NOCACHE );
				}
				else if( CurrentAllocationCacheBehaviour == CACHE_WriteCombine )
				{
					XPhysicalProtect( Ptr, Size, PAGE_READWRITE | PAGE_WRITECOMBINE );
				}
			}
			else
			{
				if( bTestingForFragmentation == TRUE )
				{
					return NULL;
				}
				OutOfMemory( Size );
			}
			MEM_TIME(MemTime += appSeconds());
			return Ptr;
		}
		else
		{
			void* Ptr = VirtualAlloc( NULL, Size, MEM_COMMIT, PAGE_READWRITE );
			if( !Ptr )
			{
				if( bTestingForFragmentation == TRUE )
				{
					return NULL;
				}
				OutOfMemory( Size );
			}
			MEM_TIME(MemTime += appSeconds());
			return Ptr;
		}
	}
#else
	else
	{
		// Use OS for large allocations.
		INT AlignedSize = Align(Size,OS_PAGE_SIZE);
		FLargeAllocInfo* AllocInfo = LargeAlloc.Malloc( Size, bIsCurrentAllocationPhysical, FALSE, CurrentAllocationCacheBehaviour );

		Free = (FFreeMem*) AllocInfo->BaseAddress;
		if( !Free )
		{
			if( bTestingForFragmentation == TRUE )
			{
				return NULL;
			}
			OutOfMemory( Size );
		}
		checkSlow(!((DWORD)Free&65535));

		// Create indirect.
		FPoolInfo*& Indirect = PoolIndirect[((DWORD)Free>>27)];
		if( !Indirect )
		{
			Indirect = CreateIndirect();
		}

		// Init pool.
		FPoolInfo* Pool = &Indirect[((DWORD)Free>>16)&2047];
		Pool->Bytes		= Size;
		Pool->OsBytes	= AlignedSize;
		Pool->Table		= &OsTable;
		Pool->AllocInfo	= AllocInfo;
		STAT(OsPeak   = Max(OsPeak,  OsCurrent+=AlignedSize));
		STAT(UsedPeak = Max(UsedPeak,UsedCurrent+=Size));
	}
#endif
	MEM_TIME(MemTime += appSeconds());
	return Free;
}
void* FMallocXenonPooled::ReallocOriginal( void* Ptr, DWORD NewSize, DWORD Alignment )
{
	check(Alignment == DEFAULT_ALIGNMENT && "Alignment is not supported on Xenon, yet");
	MEM_TIME(MemTime -= appSeconds());
	check(NewSize>=0);
	void* NewPtr = Ptr;
	if( Ptr && NewSize )
	{
		FPoolInfo* Pool = &PoolIndirect[(DWORD)Ptr>>27][((DWORD)Ptr>>16)&2047];
		if( Pool->Table!=&OsTable )
		{
			// Allocated from pool, so grow or shrink if necessary.
			if( NewSize>Pool->Table->BlockSize || GetPoolTable( NewSize, Pool->Table->bIsPhysicalAllocation, Pool->Table->CacheBehaviour ) !=Pool->Table )
			{
				NewPtr = Malloc( NewSize, Alignment );
				appMemcpy( NewPtr, Ptr, Min(NewSize,Pool->Table->BlockSize) );
				Free( Ptr );
			}
		}
		else
		{
			// Allocated from OS.
			checkSlow(!((INT)Ptr&65535));
			if( NewSize>Pool->OsBytes || NewSize*3<Pool->OsBytes*2 )
			{
				// Grow or shrink.
				NewPtr = Malloc( NewSize, Alignment );
				appMemcpy( NewPtr, Ptr, Min(NewSize,Pool->Bytes) );
				Free( Ptr );
			}
			else
			{
				// Keep as-is, reallocation isn't worth the overhead.
				STAT(UsedCurrent+=NewSize-Pool->Bytes);
				STAT(UsedPeak=Max(UsedPeak,UsedCurrent));
				Pool->Bytes = NewSize;
			}
		}
	}
	else if( Ptr == NULL )
	{
		NewPtr = Malloc( NewSize, Alignment );
	}
	else
	{
		if( Ptr )
		{
			Free( Ptr );
		}
		NewPtr = NULL;
	}
	MEM_TIME(MemTime += appSeconds());
	return NewPtr;
}

void* FMallocXenonPooled::Realloc( void* Ptr, DWORD NewSize, DWORD Alignment )
{
#if USE_4K_PAGES
	void* NewPtr = Ptr;
	if( Ptr && NewSize )
	{
		// Is not pool?
		if (IS_VIRTUAL_64K(Ptr))
		{
			// Virtual 64KB Alloc
			return ReallocOriginal(Ptr, NewSize, Alignment);
		}

		MEMORY_BASIC_INFORMATION MemoryInfo;
		DWORD OsBytes;
		VirtualQuery(Ptr, &MemoryInfo, sizeof(MemoryInfo));
		OsBytes = MemoryInfo.RegionSize;

		// Allocated from OS.
		if( NewSize>OsBytes || NewSize*3<OsBytes*2 )
		{
			// Grow or shrink.
			NewPtr = Malloc( NewSize, Alignment );
			appMemcpy( NewPtr, Ptr, Min(NewSize,OsBytes) );
			Free( Ptr );
		}
		else
		{
			// Keep as-is, reallocation isn't worth the overhead.
		}
	}
	else if( Ptr == NULL )
	{
		NewPtr = Malloc( NewSize, Alignment );
	}
	else
	{
		if( Ptr )
		{
			Free( Ptr );
		}
		NewPtr = NULL;
	}
	MEM_TIME(MemTime += appSeconds());
	return NewPtr;
#else
	return ReallocOriginal(Ptr, NewSize, Alignment);
#endif
}

void FMallocXenonPooled::Free( void* Ptr )
{
	if( !Ptr )
	{
		return;
	}
	MEM_TIME(MemTime -= appSeconds());
	STAT(CurrentAllocs--);

#if USE_4K_PAGES
	if( IS_VIRTUAL_4K(Ptr) )
	{
		VirtualFree(Ptr, 0, MEM_RELEASE);
	}
	else if( IS_PHYSICAL_4K(Ptr) )
	{
		XPhysicalFree(Ptr);
	} 
	else
#endif
	{
		// Windows version.
		FPoolInfo* Pool = &PoolIndirect[(DWORD)Ptr>>27][((DWORD)Ptr>>16)&2047];
		checkSlow(Pool->Bytes!=0);
		if( Pool->Table!=&OsTable )
		{
			// If this pool was exhausted, move to available list.
			if( !Pool->FirstMem )
			{
				Pool->Unlink();
				Pool->Link( Pool->Table->FirstPool );
			}

			// Free a pooled allocation.
			FFreeMem* Free		= (FFreeMem *)Ptr;
			Free->Blocks		= 1;
			Free->Next			= Pool->FirstMem;
			Pool->FirstMem		= Free;
			STAT(UsedCurrent   -= Pool->Table->BlockSize);

			// Free this pool.
			checkSlow(Pool->Taken>=1);
			if( --Pool->Taken == 0 )
			{
				// Free the OS memory.
				Pool->Unlink();
				checkSlow(Pool->Table);
				checkSlow(Pool->Table->bIsPhysicalAllocation == bIsCurrentAllocationPhysical);
				LargeAlloc.Free( Pool->AllocInfo, Pool->Table->bIsPhysicalAllocation );
				STAT(OsCurrent -= Pool->OsBytes);
			}
		}
		else
		{
			// Free an OS allocation.
			checkSlow(!((INT)Ptr&65535));
			checkSlow(Pool->Table);
			STAT(UsedCurrent -= Pool->Bytes);
			STAT(OsCurrent   -= Pool->OsBytes);
			LargeAlloc.Free( Pool->AllocInfo, bIsCurrentAllocationPhysical );	
		}
	}
	MEM_TIME(MemTime += appSeconds());
}
void* FMallocXenonPooled::PhysicalAlloc( DWORD Count, ECacheBehaviour InCacheBehaviour )
{
	bIsCurrentAllocationPhysical	= TRUE;
	CurrentAllocationCacheBehaviour	= InCacheBehaviour;
	void* Pointer = Malloc( Count, DEFAULT_ALIGNMENT );
	bIsCurrentAllocationPhysical	= FALSE;
	CurrentAllocationCacheBehaviour	= CACHE_Normal;
	return Pointer;
}
void FMallocXenonPooled::PhysicalFree( void* Original )
{
	bIsCurrentAllocationPhysical = TRUE;
	Free( Original );
	bIsCurrentAllocationPhysical = FALSE;
}
/**
* Gathers memory allocations for both virtual and physical allocations.
*
* @param Virtual	[out] size of virtual allocations
* @param Physical	[out] size of physical allocations	
*/
void FMallocXenonPooled::GetAllocationInfo( SIZE_T& Virtual, SIZE_T& Physical )
{
#if USE_4K_PAGES
#if ALLOW_NON_APPROVED_FOR_SHIPPING_LIB
	DM_MEMORY_STATISTICS DmMemStat;
	DmMemStat.cbSize = sizeof(DmMemStat);
	DM_MEMORY_STATISTICS DmTitleMemStat;
	DmTitleMemStat.cbSize = sizeof(DmTitleMemStat);
	DmQueryMemoryStatistics( &DmMemStat );
	DmQueryTitleMemoryStatistics( &DmTitleMemStat );
	Virtual = (DmMemStat.TotalPages - DmTitleMemStat.AvailablePages) * 4096;
#else
	Virtual	= 0;
#endif
	Physical = 0;
#else
	LargeAlloc.GetAllocationInfo( Virtual, Physical );
#endif
}


/**
* Keeps trying to allocate memory until we fail
*
* @param Ar Device to send output to
*/
void FMallocXenonPooled::CheckMemoryFragmentationLevel( class FOutputDevice& Ar )
{
	Ar.Logf( TEXT( "Starting CheckMemoryFragmentationLevel" ));

	bTestingForFragmentation = TRUE;

	DWORD Alignment = DEFAULT_ALIGNMENT;

	DWORD Size = 0;
	UBOOL bDidWeAllocate = TRUE;
	while( bDidWeAllocate == TRUE )
	{
		Size += 1024*32; // 32k

		void* Ptr;

		Ptr = PhysicalAlloc( Size, CACHE_WriteCombine );

		if( Ptr == NULL )
		{
			Ar.Logf( TEXT( "Fragmentation at: %d b %5.2f kb %5.2f mb" ), Size, Size/1024.0f, Size/1024.0f/1024.0f );
			bDidWeAllocate = FALSE;
		}
		else
		{
			//Ar.Logf( TEXT( "All Good at: %d %d" ), Size, Size/1024 );
			PhysicalFree( Ptr );
		}
	}

	bTestingForFragmentation = FALSE;

	Ar.Logf( TEXT( "Ended CheckMemoryFragmentationLevel" ));
}


UBOOL FMallocXenonPooled::Exec( const TCHAR* Cmd, FOutputDevice& Ar ) 
{ 
	if( ParseCommand(&Cmd,TEXT("HEAPCHECK")) )
	{
		HeapCheck();
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("DUMPALLOCS")) )
	{
		DumpAllocs( Ar );
		return TRUE;
	}

	return FALSE;
}

void FMallocXenonPooled::DumpAllocs( FOutputDevice& Ar )
{
	FMallocXenonPooled::HeapCheck();

	STAT(Ar.Logf( TEXT("Memory Allocation Status") ));
#if !USE_4K_PAGES
	STAT(Ar.Logf( TEXT("Curr Memory % 5.3fM / % 5.3fM"), UsedCurrent/1024.0/1024.0, OsCurrent/1024.0/1024.0 ));
	STAT(Ar.Logf( TEXT("Peak Memory % 5.3fM / % 5.3fM"), UsedPeak   /1024.0/1024.0, OsPeak   /1024.0/1024.0 ));
#endif
	STAT(Ar.Logf( TEXT("Allocs      % 6i Current / % 6i Total"), CurrentAllocs, TotalAllocs ));
	STAT(Ar.Logf( TEXT("") ));
	MEM_TIME(Ar.Logf( "Seconds     % 5.3f", MemTime ));
	MEM_TIME(Ar.Logf( "MSec/Allc   % 5.5f", 1000.0 * MemTime / MemAllocs ));

#if ALLOW_NON_APPROVED_FOR_SHIPPING_LIB
	DM_MEMORY_STATISTICS DmMemStat;
	DmMemStat.cbSize = sizeof(DmMemStat);
	DmQueryTitleMemoryStatistics( &DmMemStat );

	Ar.Logf(TEXT("DmQueryTitleMemoryStatistics"));
	Ar.Logf(TEXT("TotalPages              %i"), DmMemStat.TotalPages				);
	Ar.Logf(TEXT("AvailablePages          %i"), DmMemStat.AvailablePages			);
	Ar.Logf(TEXT("StackPages              %i"), DmMemStat.StackPages				);
	Ar.Logf(TEXT("VirtualPageTablePages   %i"), DmMemStat.VirtualPageTablePages	);
	Ar.Logf(TEXT("SystemPageTablePages    %i"), DmMemStat.SystemPageTablePages	);
	Ar.Logf(TEXT("VirtualMappedPages      %i"), DmMemStat.VirtualMappedPages		);
	Ar.Logf(TEXT("ImagePages              %i"), DmMemStat.ImagePages				);
	Ar.Logf(TEXT("FileCachePages          %i"), DmMemStat.FileCachePages			);
	Ar.Logf(TEXT("ContiguousPages         %i"), DmMemStat.ContiguousPages			);
	Ar.Logf(TEXT("DebuggerPages           %i"), DmMemStat.DebuggerPages			);
	Ar.Logf(TEXT(""));

	MEMORYSTATUS MemStat;
	MemStat.dwLength = sizeof(MemStat);
	GlobalMemoryStatus( &MemStat );

	Ar.Logf(TEXT("GlobalMemoryStatus"));
	Ar.Logf(TEXT("dwTotalPhys             %i"), MemStat.dwTotalPhys				);
	Ar.Logf(TEXT("dwAvailPhys             %i"), MemStat.dwAvailPhys				);
	Ar.Logf(TEXT("dwTotalVirtual          %i"), MemStat.dwTotalVirtual			);
	Ar.Logf(TEXT("dwAvailVirtual          %i"), MemStat.dwAvailVirtual			);
	Ar.Logf(TEXT(""));

#if TRUE
	// Go over each set of FPoolTables that corresponds to a cache type
	for( INT CacheTypeIndex = 0; CacheTypeIndex < CACHE_MAX; CacheTypeIndex++ )
	{
		switch (CacheTypeIndex)
		{
		case CACHE_Normal: 
			Ar.Logf(TEXT("CacheType Normal")); 
			break;
		case CACHE_WriteCombine: 
			Ar.Logf(TEXT("CacheType WriteCombine")); 
			break;
		case CACHE_None: 
			continue;
		case CACHE_Virtual: 
			Ar.Logf(TEXT("CacheType Virtual")); 
			break;
		default: 
			appErrorf(TEXT("Unhandled cache type"));
		}

		Ar.Logf( TEXT("Block Size Num Pools Cur Allocs Mem Used  Mem Waste Efficiency") );
		Ar.Logf( TEXT("---------- --------- ---------- --------- --------- ----------") );
		INT TotalPoolCount  = 0;
		INT TotalAllocCount = 0;
		INT TotalMemUsed    = 0;
		INT TotalMemWaste   = 0;
		for( INT i=0; i<POOL_COUNT; i++ )
		{
			FPoolTable* Table = &PoolTable[CacheTypeIndex][i];
			INT PoolCount=0;
			INT AllocCount=0;
			INT MemUsed=0;
			for( INT i=0; i<2; i++ )
			{
				for( FPoolInfo* Pool=(i?Table->FirstPool:Table->ExhaustedPool); Pool; Pool=Pool->Next )
				{
					PoolCount++;
					AllocCount += Pool->Taken;
					MemUsed += Pool->OsBytes;
					INT FreeCount=0;
					for( FFreeMem* Free=Pool->FirstMem; Free; Free=Free->Next )
					{
						FreeCount += Free->Blocks;
					}
				}
			}
			INT MemWaste = MemUsed - AllocCount*Table->BlockSize;
			Ar.Logf
				(
				TEXT("% 10i % 9i % 10i % 7i K % 7i K % 9.2f%%"),
				Table->BlockSize,
				PoolCount,
				AllocCount,
				MemUsed /1024,
				MemWaste/1024,
				MemUsed ? 100.0 * (MemUsed-MemWaste) / MemUsed : 100.0
				);
			TotalPoolCount  += PoolCount;
			TotalAllocCount += AllocCount;
			TotalMemUsed    += MemUsed;
			TotalMemWaste   += MemWaste;
		}
		Ar.Logf
			(
			TEXT("BlkOverall % 9i % 10i % 7i K % 7i K % 9.2f%%"),
			TotalPoolCount,
			TotalAllocCount,
			TotalMemUsed /1024,
			TotalMemWaste/1024,
			TotalMemUsed ? 100.0 * (TotalMemUsed-TotalMemWaste) / TotalMemUsed : 100.0
			);
	}
#endif

#if !USE_4K_PAGES
	SIZE_T SystemAllocationSize	= MemStat.dwTotalPhys - MemStat.dwAvailPhys;
	SIZE_T UnrealAllocationSize = LargeAlloc.DumpAllocs(Ar);
	SIZE_T MiscAllocationSize	= 4096 * (DmMemStat.StackPages + DmMemStat.VirtualPageTablePages + DmMemStat.SystemPageTablePages + DmMemStat.ImagePages + DmMemStat.FileCachePages + DmMemStat.DebuggerPages);
	extern SIZE_T XMemAllocationSize;

	Ar.Logf(TEXT(""));
	Ar.Logf(TEXT("System: %i MByte, Unreal: %i MByte, OSBytes: %i MByte, XMemAlloc: %i MByte, Misc: %i MByte, Reserved: 32 MByte Difference: %i MByte"), 
		SystemAllocationSize / 1024 / 1024, 
		UnrealAllocationSize / 1024 / 1024,
		OsCurrent / 1024 / 1024,
		XMemAllocationSize / 1024 / 1024,
		MiscAllocationSize / 1024 / 1024,
		(SystemAllocationSize - UnrealAllocationSize - XMemAllocationSize - MiscAllocationSize) / 1024 / 1024 - 32 );
#endif
#endif

	// Dump XMemAlloc info including audio allocator.
	extern void XMemDumpAllocationInfo( FOutputDevice& Ar );
	XMemDumpAllocationInfo( Ar );
}

void FMallocXenonPooled::HeapCheck()
{
	for( INT MemoryType=0; MemoryType<CACHE_MAX; MemoryType++ )
	{
		for( INT i=0; i<POOL_COUNT; i++ )
		{
			FPoolTable* Table = &PoolTable[MemoryType][i];
			for( FPoolInfo** PoolPtr=&Table->FirstPool; *PoolPtr; PoolPtr=&(*PoolPtr)->Next )
			{
				FPoolInfo* Pool=*PoolPtr;
				check(Pool->PrevLink==PoolPtr);
				check(Pool->FirstMem);
				for( FFreeMem* Free=Pool->FirstMem; Free; Free=Free->Next )
				{				
					check(Free->Blocks>0);
				}
			}
			for( FPoolInfo** PoolPtr=&Table->ExhaustedPool; *PoolPtr; PoolPtr=&(*PoolPtr)->Next )
			{
				FPoolInfo* Pool=*PoolPtr;
				check(Pool->PrevLink==PoolPtr);
				check(!Pool->FirstMem);
			}
		}
	}
}

#endif //USE_FMALLOC_POOLED
