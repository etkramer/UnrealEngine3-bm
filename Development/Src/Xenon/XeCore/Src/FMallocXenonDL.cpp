/*=============================================================================
	FMallocXenonDL.cpp: Xenon support for Doug Lea allocator
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"
#include "FMallocXenon.h"

#if USE_FMALLOC_DL

#include "UnMallocDLXe.inl" 

FMallocXenonDL::FMallocXenonDL()
#if !FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE
: CurrentAllocatedBytes(0)
, TotalExtraPadding(0)
, TotalNumberOfAllocations(0)
, CurrentNumberOfAllocations(0)
#endif
{	
}

FMallocXenonDL::~FMallocXenonDL()
{	
}

void* FMallocXenonDL::Malloc( DWORD Size, DWORD Alignment )
{
	void* Ptr = NULL;

	if( Alignment != DEFAULT_ALIGNMENT )
	{
		Ptr = dlmemalign( Alignment, Size );
	}
	else
	{
		Ptr = dlmalloc( Size );
	}

	if( !Ptr )
	{
		OutOfMemory(Size);
	}
#if !FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE
	else
	{
		TrackMalloc(Ptr,Size);
	}
#endif

	return Ptr;
}

void FMallocXenonDL::Free( void* Ptr )
{
#if !FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE
	if( Ptr )
	{
		TrackFree(Ptr);
	}
#endif

	dlfree( Ptr );
}

void* FMallocXenonDL::Realloc( void* Ptr, DWORD NewSize, DWORD Alignment )
{
	checkf(Alignment == DEFAULT_ALIGNMENT, TEXT("Alignment with realloc is not supported with FMallocXenonDL"));

	void* NewPtr = NULL;
	if( Ptr && NewSize > 0 )
	{
		NewPtr = Malloc( NewSize, Alignment );
		DWORD PtrSize = dlmalloc_usable_size(Ptr);
		appMemcpy( NewPtr, Ptr, Min<DWORD>(PtrSize,NewSize));
		Free( Ptr );
	}
	else if( !Ptr )
	{
		NewPtr = Malloc( NewSize, Alignment );
	}
	else
	{
		Free( Ptr );
	}

	return NewPtr;
}

void FMallocXenonDL::DumpDLStats( FOutputDevice* Ar )
{
#if !FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE

	mallinfo Info = dlmallinfo();
	SIZE_T Footprint = dlmalloc_footprint();
	SIZE_T MaxFootprint = dlmalloc_max_footprint();

	// @note
	// Info.uordblks == std::malloc_managed_size.current_inuse_size
	// dlmalloc_max_footprint() == std::malloc_managed_size.size_t max_system_size
	// dlmalloc_footprint() == std::malloc_managed_size.current_system_size;

	check(Ar);
	Ar->Logf(TEXT(""));
	Ar->Logf(TEXT("---- dlmalloc stats begin ----"));
	Ar->Logf(TEXT("non-mmapped space allocated from system %.2f KB"), Info.arena/1024.f);
	Ar->Logf(TEXT("number of free chunks %d"), Info.ordblks);
	Ar->Logf(TEXT("space in mmapped regions %.2f KB"), Info.hblkhd/1024.f);
	Ar->Logf(TEXT("maximum total allocated space %.2f KB"), Info.usmblks/1024.f);
	Ar->Logf(TEXT("total allocated space %.2f KB"), Info.uordblks/1024.f);
	Ar->Logf(TEXT("total free space %.2f KB"), Info.fordblks/1024.f);
	Ar->Logf(TEXT("releasable (via malloc_trim) space %.2f KB"), Info.keepcost/1024.f);
	Ar->Logf(TEXT("** total footprint %.2f [%.2f max] KB"), Footprint/1024.f,MaxFootprint/1024.f);
	Ar->Logf(TEXT("---- dlmalloc stats end ----"));
	Ar->Logf(TEXT(""));
#endif
}

static void PrintOSMemStats(FOutputDevice& Ar)
{
#if ALLOW_NON_APPROVED_FOR_SHIPPING_LIB
	DM_MEMORY_STATISTICS DmMemStat;
	DmMemStat.cbSize = sizeof(DmMemStat);
	DmQueryMemoryStatistics( &DmMemStat );

	Ar.Logf(TEXT("--- DmQueryMemoryStatistics BEGIN ---"));
	Ar.Logf(TEXT("TotalPages				%i"), DmMemStat.TotalPages				);
	Ar.Logf(TEXT("AvailablePages				%i"), DmMemStat.AvailablePages			);
	Ar.Logf(TEXT("StackPages				%i"), DmMemStat.StackPages				);
	Ar.Logf(TEXT("VirtualPageTablePages			%i"), DmMemStat.VirtualPageTablePages	);
	Ar.Logf(TEXT("SystemPageTablePages			%i"), DmMemStat.SystemPageTablePages	);
	Ar.Logf(TEXT("VirtualMappedPages			%i"), DmMemStat.VirtualMappedPages		);
	Ar.Logf(TEXT("ImagePages				%i"), DmMemStat.ImagePages				);
	Ar.Logf(TEXT("FileCachePages				%i"), DmMemStat.FileCachePages			);
	Ar.Logf(TEXT("ContiguousPages				%i"), DmMemStat.ContiguousPages			);
	Ar.Logf(TEXT("DebuggerPages				%i"), DmMemStat.DebuggerPages			);
	Ar.Logf(TEXT("--- DmQueryMemoryStatistics END ---"));

	MEMORYSTATUS MemStat;
	MemStat.dwLength = sizeof(MemStat);
	GlobalMemoryStatus( &MemStat );

	Ar.Logf(TEXT("--- GlobalMemoryStatus BEGIN ---"));
	Ar.Logf(TEXT("dwTotalPhys				%i"), MemStat.dwTotalPhys				);
	Ar.Logf(TEXT("dwAvailPhys				%i"), MemStat.dwAvailPhys				);
	Ar.Logf(TEXT("dwTotalVirtual				%i"), MemStat.dwTotalVirtual			);
	Ar.Logf(TEXT("dwAvailVirtual			%i"), MemStat.dwAvailVirtual			);
	Ar.Logf(TEXT("--- GlobalMemoryStatus END ---"));
#endif
}

/**
* If possible give memory back to the os from unused segments
*
* @param ReservePad - amount of space to reserve when trimming
* @param bShowStats - log stats about how much memory was actually trimmed. Disable this for perf
* @return TRUE if succeeded
*/ 
UBOOL FMallocXenonDL::TrimMemory(SIZE_T ReservePad,UBOOL bShowStats)
{
	SCOPE_CYCLE_COUNTER(STAT_TrimMemoryTime);

	// trim default mem space
	return dlmalloc_trim(ReservePad);
}

UBOOL FMallocXenonDL::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
#if !FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE
	if( ParseCommand( &Cmd, TEXT("DUMPINFO") ) )
	{
		DumpDLStats(&Ar);
		PrintOSMemStats(Ar);
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("dumperfile")) )
	{
		const FString SystemTime = appSystemTimeString();
		const FString Filename = FString::Printf(TEXT("%sDlMemCheck-%s.bmp"),*appProfilingDir(),*SystemTime);

		const INT EntryHeight = 4;
		// total number of segments (start with default heap)
		INT NumSegments = get_num_segments(NULL) + 1;
		const INT Height = EntryHeight * NumSegments;
		const INT Width = 64*1024 / 16;

		TArray<FColor> AllocData;
		AllocData.Empty( Height * Width );
		AllocData.AddZeroed( Height * Width );

		INT CurIdx=0;
		dump_segments_to_bmp( NULL, AllocData, CurIdx, Width, EntryHeight );

		appCreateBitmap( *Filename, Width, Height, AllocData.GetTypedData() );
		return TRUE;
	}
#endif
	return FALSE;
}

void FMallocXenonDL::OutOfMemory( DWORD Size )
{
	appErrorf(TEXT("OOM %d %f %f"), Size, Size/1024.0f, Size/1024.0f/1024.0f);
}

UBOOL FMallocXenonDL::IsInternallyThreadSafe() const
{
	return FALSE;
}


void FMallocXenonDL::GetAllocationInfo( SIZE_T& Virtual, SIZE_T& Physical )
{
	mallinfo Info = dlmallinfo();
	Virtual = 0;
	Physical = Info.uordblks;
}

#if !FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE

/** Update allocated bytes */
void FMallocXenonDL::TrackMalloc(void* Ptr, DWORD Size)
{
	// get how much malloc is marked as usable
	INT UsableSize = dlmalloc_usable_size(Ptr);

	// update total bytes
	CurrentAllocatedBytes += UsableSize;

	// update the waste
	TotalExtraPadding += UsableSize - Size;

	// update count of allocations
	TotalNumberOfAllocations++;
	CurrentNumberOfAllocations++;
}

void FMallocXenonDL::TrackFree(void* Ptr)
{
	// get how much malloc is marked as usable
	INT UsableSize = dlmalloc_usable_size(Ptr);

	// update total bytes
	CurrentAllocatedBytes -= UsableSize;

	CurrentNumberOfAllocations--;
}

FMalloc* GMallocPooled = NULL;

void* FMallocXenonDL::PhysicalAlloc( DWORD Size, ECacheBehaviour CacheBehaviour )
{
#if 1
	if( !GMallocPooled )
	{
		GMallocPooled = new FMallocXenonPooled();
	}
	return GMallocPooled->PhysicalAlloc(Size,CacheBehaviour);
#else
	void* BaseAddr = XPhysicalAlloc( Size, MAXULONG_PTR, 0x10000, PAGE_READWRITE | MEM_LARGE_PAGES );
	if( BaseAddr )
	{
		if( CacheBehaviour == CACHE_None )
		{
			XPhysicalProtect( BaseAddr, Size, PAGE_READWRITE | PAGE_NOCACHE );			
		}
		else if( CacheBehaviour == CACHE_WriteCombine )
		{
			XPhysicalProtect( BaseAddr, Size, PAGE_READWRITE | PAGE_WRITECOMBINE );
		}
	}
	return BaseAddr;
#endif
}

void FMallocXenonDL::PhysicalFree( void* Original )
{
#if 1
	if( !GMallocPooled )
	{
		GMallocPooled = new FMallocXenonPooled();
	}
	GMallocPooled->PhysicalFree(Original);
#else
	XPhysicalFree( Original );
#endif

}

#endif //!FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE

#else

// Suppress linker warning "warning LNK4221: no public symbols found; archive member will be inaccessible"
INT FMallocXenonDLLinkerHelper;


#endif // USE_FMALLOC_DL

