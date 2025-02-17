/*=============================================================================
	FMallocXenonDL.h: Xenon support for Doug Lea allocator
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _F_MALLOC_XENON_DL_H_
#define _F_MALLOC_XENON_DL_H_

/**
* Doug Lea allocator for Xenon (http://g.oswego.edu/)
*/
class FMallocXenonDL : public FMalloc
{
public:
	FMallocXenonDL();
	~FMallocXenonDL();

	virtual void*	Malloc( DWORD Size, DWORD Alignment );
	virtual void	Free( void* Ptr );
	virtual void*	Realloc( void* Ptr, DWORD NewSize, DWORD Alignment );
	virtual void* PhysicalAlloc( DWORD Count, ECacheBehaviour CacheBehaviour = CACHE_WriteCombine );
	virtual void PhysicalFree( void* Original );

	/**
	* Returns if the allocator is guaranteed to be thread-safe and therefore
	* doesn't need a unnecessary thread-safety wrapper around it.
	*/
	virtual UBOOL IsInternallyThreadSafe() const;

	/**
	* Gathers memory allocations for both virtual and physical allocations.
	*
	* @param Virtual	[out] size of virtual allocations (malloc)
	* @param Physical	[out] size of physical allocations (includes the pool used by malloc)
	*/
	virtual void GetAllocationInfo( SIZE_T& Virtual, SIZE_T& Physical );

	/**
	* If possible give memory back to the os from unused segments
	*
	* @param ReservePad - amount of space to reserve when trimming
	* @param bShowStats - log stats about how much memory was actually trimmed. Disable this for perf
	* @return TRUE if succeeded
	*/
	virtual UBOOL TrimMemory(SIZE_T ReservePad,UBOOL bShowStats=FALSE);

	/** Exec command handler for this allocator */
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar );


private:
	void DumpDLStats( FOutputDevice* Ar );
	FORCENOINLINE void	OutOfMemory( DWORD Size );	

#if !FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE
	/** Update allocated bytes */
	void	TrackMalloc( void* Ptr, DWORD Size );
	void	TrackFree( void* Ptr );

	/** Total amount of memory that we has been allocated to the game (this is less than what will actually be allocated by malloc, but more than requested by game) */
	INT	CurrentAllocatedBytes;

	/** Extra padded data that was allocated, this will never decrement - just used to average out the waste */
	INT	TotalExtraPadding;

	/** This is used to track the average amount of extra padding */
	INT	TotalNumberOfAllocations;

	/** This is used to track the average amount of extra padding */
	INT	CurrentNumberOfAllocations;
#endif
};

#endif // _F_MALLOC_XENON_DL_H_

