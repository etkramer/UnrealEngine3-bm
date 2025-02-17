/*=============================================================================
	XeLaunch.cpp: Game launcher.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "LaunchPrivate.h"

/* Include this here to get access to the LINK_AGAINST_PROFILING_D3D_LIBRARIES define */
#include "XeD3DDrv.h"

/*-----------------------------------------------------------------------------
	Linking mojo.
-----------------------------------------------------------------------------*/

#if _DEBUG

#pragma comment(lib, "xapilibd.lib")
#pragma comment(lib, "d3d9d.lib")

#elif FINAL_RELEASE

#pragma comment(lib, "xapilib.lib")
#pragma comment(lib, "d3d9ltcg.lib")

#elif LINK_AGAINST_PROFILING_D3D_LIBRARIES
#pragma comment(lib, "xapilibi.lib")
#pragma comment(lib, "d3d9i.lib")

#else

#pragma comment(lib, "xapilib.lib")
#pragma comment(lib, "d3d9.lib")

#endif

/**
 * This can be overridden by a #define in the XeCR code (Warfare) but otherwise
 * will function normally for games other than WarGame. 
 *
 * @see XeViewport.cpp
 **/
DWORD UnrealXInputGetState( DWORD ControllerIndex, PXINPUT_STATE CurrentInput )
{
	return XInputGetState( ControllerIndex, CurrentInput );
}

/** Global engine loop object												*/
FEngineLoop							EngineLoop;

#if USE_XeD3D_RHI
/** Global D3D device, residing in non- clobbered segment					*/
extern IDirect3DDevice9*			GDirect3DDevice;
#endif // USE_XeD3D_RHI

#if STATS
/** Global FPS counter */
FFPSCounter GFPSCounter;
#endif // STATS

/*-----------------------------------------------------------------------------
	Guarded code.
-----------------------------------------------------------------------------*/

static void GuardedMain( const TCHAR* CmdLine )
{
	CmdLine = RemoveExeName(CmdLine);
	EngineLoop.PreInit( CmdLine );
	EngineLoop.Init( );

	while( !GIsRequestingExit )
	{
#if STATS
		GFPSCounter.Update(appSeconds());
#endif // STATS
		{
			// Start tracing game thread if requested.
			appStartCPUTrace( NAME_Game, FALSE, TRUE, 40, NULL );

			// Main game thread tick.
			{
				SCOPE_CYCLE_COUNTER(STAT_FrameTime);
				EngineLoop.Tick();
			}

			appStopCPUTrace( NAME_Game );
		}
#if STATS
		// Write all stats for this frame out
		GStatManager.AdvanceFrame();

		if(GIsThreadedRendering)
		{
			ENQUEUE_UNIQUE_RENDER_COMMAND(AdvanceFrame,{GStatManager.AdvanceFrameForThread();});
		}
#endif
	}

	EngineLoop.Exit();
}

/*-----------------------------------------------------------------------------
	Main.
-----------------------------------------------------------------------------*/

void __cdecl main()
{
	// Initialize all timing info
	appInitTiming();

	// default to no game
	appStrcpy(GGameName, TEXT("None"));

	TCHAR* CommandLine = NULL;

	// see if image was relaunched and was passed any launch data 
	DWORD LaunchDataSize=0;
	const SIZE_T LaunchCmdLineSize = 4096;
	TCHAR* LaunchCmdLine = new TCHAR[LaunchCmdLineSize];
	if( ERROR_SUCCESS == XGetLaunchDataSize( &LaunchDataSize ) )
	{
		// should always be true unless we are coming from the demo launcher
		check( LaunchDataSize == sizeof(FXenonLaunchData) );

		// get the launch data that was passed to this exe
		FXenonLaunchData XeLaunchData;
		appMemzero( &XeLaunchData,sizeof(FXenonLaunchData) );
        XGetLaunchData( &XeLaunchData, LaunchDataSize );

		// tack on dummy exe name to command line 
		appStrcat( LaunchCmdLine, LaunchCmdLineSize, TEXT("dummy ") );
		appStrcat( LaunchCmdLine, LaunchCmdLineSize, ANSI_TO_TCHAR( XeLaunchData.CmdLine ) );

		// set cmd line
		CommandLine = LaunchCmdLine;
	}
	else
	{
		// Command line contains executable name (though not fully qualified)
		appStrcpy( LaunchCmdLine, LaunchCmdLineSize, ANSI_TO_TCHAR( GetCommandLine() ) );
		CommandLine = LaunchCmdLine;
	}

	// Perform initial platform initialization.
	appXenonInit( CommandLine );

#if _TICKET_TRACKER_
	LoadTicketTrackerSettings();
#endif

#if ALLOW_NON_APPROVED_FOR_SHIPPING_LIB
	// Initialize remote debugging tool.
	HRESULT extern __stdcall DebugConsoleCmdProcessor( const CHAR* Command, CHAR* Response, DWORD ResponseLen, PDM_CMDCONT pdmcc );
	verify( SUCCEEDED( DmRegisterCommandProcessor( "UNREAL", DebugConsoleCmdProcessor ) ));
#endif

	// Begin guarded code.
	GIsStarted = 1;
	GIsGuarded = 0; //@todo xenon: "guard" up GuardedMain at some point for builds running outside the debugger
	GuardedMain( CommandLine );
	GIsGuarded = 0;

	// Final shut down.
	appExit();
	GIsStarted = 0;

#if ALLOW_NON_APPROVED_FOR_SHIPPING_LIB
	// We're done - reboot.
	DmReboot( DMBOOT_TITLE );
#endif
}

/**
 * System memory allocators.
 *
 * These are overridden such that all memory allocations from xenon libs are 
 * routed through Unreal's Memory allocator.
 */

// This can cause deadlocks and therefore should only ever be enabled locally.
#ifndef KEEP_XMEM_ALLOC_STATS
#define KEEP_XMEM_ALLOC_STATS 0
#endif


/** Size of allocations routed through XMemAlloc. */
SIZE_T	XMemAllocationSize	= 0;
/** Number of allocations routed through XMemAlloc. */
INT		XMemAllocationCount = 0;

#if KEEP_XMEM_ALLOC_STATS
/** Helper structure encapsulating XMemAlloc request */
struct FXMemAllocInfo
{
	/** Constructor, initializing all members */
	FXMemAllocInfo( SIZE_T InSize, SIZE_T InAlignedSize, XALLOC_ATTRIBUTES InAttributes )
	:	Size( InSize )
	,	AlignedSize( InAlignedSize )
	,	Attributes( InAttributes )
	{}

	/** Size of allocation. */
	SIZE_T				Size;
	/** Aligned size of allocation. */
	SIZE_T				AlignedSize;
	/** Attributes passed to XMemAlloc. */
	XALLOC_ATTRIBUTES	Attributes;
};
/** Map from pointer to allocations routed through XMemAlloc. */
TMap<PTRINT,FXMemAllocInfo> XMemPointerToAllocationMap;
#endif

/**
 * Pooled allocator for audio allocations. Never frees memory!
 */
struct FXeAudioAllocator
{	
	/**
	 * Default constructor, initializing members.
	 */
	FXeAudioAllocator()
	:	NumAllocateCalls(0)
	,	NumFreeCalls(0)
	,	NumGetAllocationSizeCalls(0)
	{
	}

	/**
	 * Allocates audio memory. First tries to find in available pool and if not found
	 * creates new pool entry.
	 *
	 * @param	Size	Size of allocation
	 * @return	Pointer of size Size
	 */
	void* Allocate( INT Size )
	{
		NumAllocateCalls++;
		void* Ptr = NULL;

		// Iterate in reverse order as usual pattern is free followed by allocation.
		for( INT AllocIndex=AvailablePool.Num()-1; AllocIndex>=0; AllocIndex-- )
		{
			const FPointerSize& Allocation = AvailablePool(AllocIndex);
			// We've found a match!
			if( Allocation.Size == Size )
			{
				// Use this pointer.
				Ptr = Allocation.Ptr;

				// Add to allocated pool...
				AllocatedPool.AddItem( Allocation );
				// ... and remove from available one.
				AvailablePool.Remove( AllocIndex );
				// Can't access Allocation after this as the above removes it!
				break;
			}
		}

		// Not enough space in pool, grow it.
		if( Ptr == NULL )
		{
			Ptr = appPhysicalAlloc( Align(Size, 128), CACHE_Normal );
			
			// Add allocation to pool.
			FPointerSize Allocation;
			Allocation.Ptr  = Ptr;
			Allocation.Size = Size;
			AllocatedPool.AddItem( Allocation );
		}

		return Ptr;
	}

	/**
	 * Moves allocation from allocated to available pool.
	 *
	 * @param Ptr	Ptr associated with allocation.
	 */
	void Free( void* Ptr )
	{
		NumFreeCalls++;
		for( INT AllocIndex=0; AllocIndex<AllocatedPool.Num(); AllocIndex++ )
		{
			const FPointerSize& Allocation = AllocatedPool(AllocIndex);
			// We've found a match.
			if( Allocation.Ptr == Ptr )
			{
				// Add to available pool...
				AvailablePool.AddItem( Allocation );
				// ... and remove from allocated one.
				AllocatedPool.Remove( AllocIndex );
				// Can't access Allocation after this as the above removes it!
				break;
			}
		}
	}

	/**
	 * Returns allocation size associated with a pointer.
	 *
	 * @param	Ptr		Pointer to retrieve allocation size for
	 * @return	Size of associated allocation
	 */
	INT GetAllocationSize( void* Ptr )
	{
		NumGetAllocationSizeCalls++;
		INT AllocationSize = 0;
		for( INT AllocIndex=0; AllocIndex<AllocatedPool.Num(); AllocIndex++ )
		{
			const FPointerSize& Allocation = AllocatedPool(AllocIndex);
			// We've found a match!
			if( Allocation.Ptr == Ptr )
			{
				AllocationSize = Allocation.Size;
				break;
			}
		}
		return AllocationSize;
	}

	/**
	 * Dumps alloc info/ summary to the log.
	 */
	void DumpAllocInfo( FOutputDevice& Ar )
	{
		// Gather memory in available pool.
		INT AvailableSize = 0;
		for( INT AllocIndex=0; AllocIndex<AvailablePool.Num(); AllocIndex++ )
		{
			const FPointerSize& Allocation = AvailablePool(AllocIndex);
			AvailableSize += Allocation.Size;
		}
		// Gather memory in allocated pool.
		INT AllocatedSize = 0;
		for( INT AllocIndex=0; AllocIndex<AllocatedPool.Num(); AllocIndex++ )
		{
			const FPointerSize& Allocation = AllocatedPool(AllocIndex);
			AllocatedSize += Allocation.Size;
		}
		Ar.Logf(TEXT("XeAudioAllocator uses %i KByte for %i allocations, (%i KByte avail in %i allocs, %i KByte used by %i allocs"),
					(AllocatedSize + AvailableSize) / 1024,
					AllocatedPool.Num() + AvailablePool.Num(),
					AvailableSize / 1024,
					AvailablePool.Num(),
					AllocatedSize / 1024,
					AllocatedPool.Num());
		Ar.Logf(TEXT("XeAudioAllocator routed Allocate: %i, Free: %i, GetAllocationSize: %i calls."), 
					NumAllocateCalls,
					NumFreeCalls,
					NumGetAllocationSizeCalls);
	}

private:
	/** Helper structure encapsulating pointer base address and size. */
	struct FPointerSize
	{
		void*	Ptr;
		INT		Size;
	};

	/** Pool of allocations in use.					*/
	TArray<FPointerSize> AllocatedPool;
	/** Pool of available allocations.				*/
	TArray<FPointerSize> AvailablePool;

	/** Total number of Allocate calls.				*/
	INT NumAllocateCalls;
	/** Total number of Free calls.					*/
	INT NumFreeCalls;
	/** Total number of GetAllocationSize calls.	*/
	INT NumGetAllocationSizeCalls;
} XeAudioAllocator;


LPVOID WINAPI XMemAlloc( SIZE_T Size, DWORD Attributes )
{
	void* Address = NULL;
	// Route XMA decoder allocations through our allocator to avoid calls to XPhysicalAlloc/ Free.
	if( Attributes == XAUDIOMEMATTR_XMADRIVER )
	{
		check( (XAUDIOMEMATTR_XMADRIVER) == MAKE_XALLOC_ATTRIBUTES(XAUDIOMEMTYPE_XMADRIVER, FALSE, FALSE, FALSE, eXALLOCAllocatorId_XAUDIO, XALLOC_PHYSICAL_ALIGNMENT_128, XALLOC_MEMPROTECT_READWRITE, FALSE, XALLOC_MEMTYPE_PHYSICAL) );
		Address = XeAudioAllocator.Allocate( Size );
	}
	else
	{
		Address = XMemAllocDefault( Size, Attributes );
	}

#if !FINAL_RELEASE
	if( Address == NULL )
	{
		appErrorf(TEXT("OOM: XMemAlloc failed to allocate %i bytes %5.2f kb %5.2f mb [%x]"), Size, Size/1024.0f, Size/1024.0f/1024.0f, Attributes);
	}
	INT AlignedSize = XMemSize( Address, Attributes );
	XMemAllocationSize += AlignedSize;
	XMemAllocationCount++;
#if KEEP_XMEM_ALLOC_STATS
	XMemPointerToAllocationMap.Set( (PTRINT) Address, FXMemAllocInfo( Size, AlignedSize, *((XALLOC_ATTRIBUTES*)&Attributes) ) );
#endif
#endif
	return Address;
}

VOID WINAPI XMemFree( PVOID Address, DWORD Attributes )
{
#if !FINAL_RELEASE
	if( Address )
	{
		XMemAllocationSize -= XMemSize( Address, Attributes );
		XMemAllocationCount--;
#if KEEP_XMEM_ALLOC_STATS
		XMemPointerToAllocationMap.Remove( (PTRINT) Address );
#endif

	}
#endif

	// Route XMA decoder allocations through our allocator to avoid calls to XPhysicalAlloc/ Free.
	if( Attributes == XAUDIOMEMATTR_XMADRIVER )
	{
		XeAudioAllocator.Free( Address );
	}
	else
	{
		XMemFreeDefault( Address, Attributes );
	}
}

SIZE_T WINAPI XMemSize( PVOID Address, DWORD Attributes )
{
	SIZE_T AllocationSize = 0;
	if( Attributes == XAUDIOMEMATTR_XMADRIVER )
	{
		AllocationSize = XeAudioAllocator.GetAllocationSize( Address );
	}
	else
	{
		AllocationSize = XMemSizeDefault( Address, Attributes );
	}
	return AllocationSize;
}

/**
 * Dumps info for active XMemAlloc allocations.
 */
void XMemDumpAllocationInfo( FOutputDevice& Ar )
{
#if KEEP_XMEM_ALLOC_STATS
	Ar.Logf( TEXT("XMemAlloc Allocation Summary:") );
	INT TotalSize			= 0;
	INT TotalAlignedSize	= 0;
	for( TMap<PTRINT,FXMemAllocInfo>::TIterator It(XMemPointerToAllocationMap); It; ++It )
	{
		const PTRINT&	Address		= It.Key();
		FXMemAllocInfo& AllocInfo	= It.Value();
		Ar.Logf( TEXT("Size: %10i    AlignedSize: %10i    AllocatorId: %5i    Pointer: %p"), 
			AllocInfo.Size, 
			AllocInfo.AlignedSize, 
			AllocInfo.Attributes.dwAllocatorId, 
			Address );
		TotalSize			+= AllocInfo.Size;
		TotalAlignedSize	+= AllocInfo.AlignedSize;
	}
	Ar.Logf( TEXT("Total Size: %7i KByte    Total Aligned Size: %7i KByte    Allocation Count: %7i"), 
		TotalSize / 1024, 
		TotalAlignedSize / 1024,
		XMemPointerToAllocationMap.Num() );
#else
	Ar.Logf( TEXT("XMemAlloc has %i active allocations consuming %i KByte of memory"), 
		XMemAllocationCount, 
		XMemAllocationSize / 1024 );
#endif
	XeAudioAllocator.DumpAllocInfo( Ar );
}

