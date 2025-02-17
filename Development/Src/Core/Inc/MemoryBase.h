/*=============================================================================
	MemoryBase.h: Base memory management definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __MEMORYBASE_H__
#define __MEMORYBASE_H__

/** The global memory allocator. */
extern class FMalloc* GMalloc;

// Memory allocator.

enum ECacheBehaviour
{
	CACHE_Normal		= 0,
	CACHE_WriteCombine	= 1,
	CACHE_None			= 2,
	CACHE_Virtual		= 3,
	CACHE_MAX			// needs to be last entry
};

//
// C style memory allocation stubs that fall back to C runtime
//
#ifndef appSystemMalloc
#define appSystemMalloc		malloc
#endif
#ifndef appSystemFree
#define appSystemFree		free
#endif
#ifndef appSystemRealloc
#define appSystemRealloc	realloc
#endif

/**
 * Inherit from FUseSystemMallocForNew if you want your objects to be placed in memory
 * alloced by the system malloc routines, bypassing GMalloc. This is e.g. used by FMalloc
 * itself.
 */
class FUseSystemMallocForNew
{
public:
	/**
	 * Overloaded new operator using the system allocator.
	 *
	 * @param	Size	Amount of memory to allocate (in bytes)
	 * @return			A pointer to a block of memory with size Size or NULL
	 */
	void* operator new( size_t Size )
	{
		return appSystemMalloc( Size );
	}

	/**
	 * Overloaded delete operator using the system allocator
	 *
	 * @param	Ptr		Pointer to delete
	 */
	void operator delete( void* Ptr )
	{
		appSystemFree( Ptr );
	}

	/**
	 * Overloaded array new operator using the system allocator.
	 *
	 * @param	Size	Amount of memory to allocate (in bytes)
	 * @return			A pointer to a block of memory with size Size or NULL
	 */
	void* operator new[]( size_t Size )
	{
		return appSystemMalloc( Size );
	}

	/**
	 * Overloaded array delete operator using the system allocator
	 *
	 * @param	Ptr		Pointer to delete
	 */
	void operator delete[]( void* Ptr )
	{
		appSystemFree( Ptr );
	}
};

/** The global memory allocator's interface. */
class FMalloc  : public FUseSystemMallocForNew, public FExec
{
public:
	virtual void* Malloc( DWORD Count, DWORD Alignment=DEFAULT_ALIGNMENT ) = 0;
	virtual void* Realloc( void* Original, DWORD Count, DWORD Alignment=DEFAULT_ALIGNMENT ) = 0;
	virtual void Free( void* Original ) = 0;
	virtual void* PhysicalAlloc( DWORD Count, ECacheBehaviour CacheBehaviour = CACHE_WriteCombine ) { return NULL; }
	virtual void PhysicalFree( void* Original ) {}
		
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar ) { return 0; }
	
	/** Called every game thread tick */
	virtual void Tick( FLOAT DeltaTime ) { }

	/**
	 * Returns if the allocator is guaranteed to be thread-safe and therefore
	 * doesn't need a unnecessary thread-safety wrapper around it.
	 */
	virtual UBOOL IsInternallyThreadSafe() const { return FALSE; }

	/**
	 * Gathers memory allocations for both virtual and physical allocations.
	 *
	 * @param Virtual	[out] size of virtual allocations
	 * @param Physical	[out] size of physical allocations	
	 */
	virtual void GetAllocationInfo( SIZE_T& Virtual, SIZE_T& Physical ) { Virtual = Physical = 0; }

	/**
	* Gathers PS3 specific allocation information
	* 
	* @param AllocatedBytes [out] Total number of bytes that are currently allocated for use by game, however this is more than was requested by game, but less than amount actually in-use by malloc
	* @param AveragePaddedBytes [out] Average (per allocation) of the number of extra bytes between requested by game and returned by malloc as usable
	* @param CurrentNumAllocations [out] Total number of current allocations
	*/
	virtual void GetPS3AllocationInfo(INT& AllocatedBytes, FLOAT& AveragePaddedBytes, INT& CurrentNumAllocations) { AllocatedBytes=0; AveragePaddedBytes=0.0f; CurrentNumAllocations=0; }

	/**
	 * Keeps trying to allocate memory until we fail
	 *
	 * @param Ar Device to send output to
	 */
	virtual void CheckMemoryFragmentationLevel( class FOutputDevice& Ar ) { Ar.Log( TEXT("CheckMemoryFragmentationLevel not implemented") ); }

	/**
	* If possible give memory back to the os from unused segments
	*
	* @param ReservePad - amount of space to reserve when trimming
	* @param bShowStats - log stats about how much memory was actually trimmed. Disable this for perf
	* @return TRUE if succeeded
	*/
	virtual UBOOL TrimMemory(SIZE_T /*ReservePad*/,UBOOL bShowStats=FALSE) { return FALSE; }

	/** Total number of calls Malloc, if implemented by derived class. */
	static QWORD TotalMallocCalls;
	/** Total number of calls Malloc, if implemented by derived class. */
	static QWORD TotalFreeCalls;
	/** Total number of calls Malloc, if implemented by derived class. */
	static QWORD TotalReallocCalls;
	/** Total number of calls to PhysicalAlloc, if implemented by derived class. */
	static QWORD TotalPhysicalAllocCalls;
	/** Total number of calls to PhysicalFree, if implemented by derived class. */
	static QWORD TotalPhysicalFreeCalls;
};

/*-----------------------------------------------------------------------------
	Memory functions.
-----------------------------------------------------------------------------*/

/** @name Memory functions */
//@{
/** Copies count bytes of characters from Src to Dest. If some regions of the source
 * area and the destination overlap, memmove ensures that the original source bytes
 * in the overlapping region are copied before being overwritten.  NOTE: make sure
 * that the destination buffer is the same size or larger than the source buffer!
 */
void* appMemmove( void* Dest, const void* Src, INT Count );
INT appMemcmp( const void* Buf1, const void* Buf2, INT Count );
UBOOL appMemIsZero( const void* V, int Count );
DWORD appMemCrc( const void* Data, INT Length, DWORD CRC=0 );
void appMemswap( void* Ptr1, void* Ptr2, DWORD Size );

/**
 * Sets the first Count chars of Dest to the character C.
 */
#define appMemset( Dest, C, Count )			memset( Dest, C, Count )

#ifndef DEFINED_appMemcpy
	#define appMemcpy( Dest, Src, Count )	memcpy( Dest, Src, Count )
#endif

#ifndef DEFINED_appMemzero
	#define appMemzero( Dest, Count )		memset( Dest, 0, Count )
#endif

/** Helper function called on first allocation to create and initialize GMalloc */
extern void GCreateMalloc();

//
// C style memory allocation stubs.
//
inline void* appMalloc( DWORD Count, DWORD Alignment=DEFAULT_ALIGNMENT ) 
{ 
	if( !GMalloc )
	{
		GCreateMalloc();
	}
	return GMalloc->Malloc( Count, Alignment );
}
inline void* appRealloc( void* Original, DWORD Count, DWORD Alignment=DEFAULT_ALIGNMENT ) 
{ 
	if( !GMalloc )
	{
		GCreateMalloc();	
	}
	return GMalloc->Realloc( Original, Count, Alignment );
}	
inline void appFree( void* Original )
{
	if( !GMalloc )
	{
		GCreateMalloc();
	}
	return GMalloc->Free( Original );
}
inline void* appPhysicalAlloc( DWORD Count, ECacheBehaviour CacheBehaviour = CACHE_WriteCombine ) 
{ 
	if( !GMalloc )
	{
		GCreateMalloc();
	}
	return GMalloc->PhysicalAlloc( Count, CacheBehaviour );
}
inline void appPhysicalFree( void* Original )
{
	if( !GMalloc )
	{
		GCreateMalloc();	
	}
	return GMalloc->PhysicalFree( Original );
}

#endif
