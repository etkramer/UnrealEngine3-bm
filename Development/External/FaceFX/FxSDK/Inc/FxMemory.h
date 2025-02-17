//------------------------------------------------------------------------------
// Encapsulation of memory acquisition and release.
//
// Owner: Jamie Redmond
// 
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMemory_H__
#define FxMemory_H__

#include "FxPlatform.h"
#include "FxUtil.h"

namespace OC3Ent
{

namespace Face
{

/// The available memory allocation types.
enum FxMemoryAllocationType
{
	MAT_FreeStore = 0, ///< Use the built-in allocator functions that utilize the global new and delete operators.
	MAT_Heap      = 1, ///< Use the built-in allocator functions that utilize malloc() and free().
	MAT_Custom    = 2  ///< Use user-defined custom allocator functions.
};

/// The object interface to allocating and freeing memory.
/// A client wishing to change the way memory is allocated and freed should
/// wrap their own memory allocation and freeing functions with the correct
/// function signatures defined by FxMemoryAllocationPolicy, construct a 
/// proper FxMemoryAllocationPolicy on the stack, and pass it to FxSDKStartup().
struct FxMemoryAllocationPolicy
{
	/// The function signature for user allocation.
	typedef void* (FX_CALL *FxUserAllocateFunc)(FxSize numBytes);
	/// The function signature for debug user allocation.
	typedef void* (FX_CALL *FxUserAllocateDebugFunc)(FxSize numBytes, const FxChar* system);
	/// The function signature for user free.
	typedef void  (FX_CALL *FxUserFreeFunc)(void* ptr, FxSize n);

	/// Default constructor.  This will initialize the memory allocation policy
	/// to the default FaceFX SDK memory allocation policy.  The default policy
	/// will either use the global new / delete operators or malloc() / free()
	/// depending on the value of the FX_DEFAULT_TO_OPERATOR_NEW #define that
	/// the library was compiled with.  Note that the default policy does not 
	/// use memory pools.  When enabling memory pooling be very careful that
	/// no FaceFX memory needs to be allocated before calling FxSDKStartup() and
    /// that no FaceFX allocated memory needs to be freed after calling 
	/// FxSDKShutdown().  If you have structured your FaceFX integration such
	/// that no FaceFX objects need to be created and no FaceFX memory needs to
	/// be allocated before FxSDKStartup() and that no FaceFX objects are 
	/// destroyed and no FaceFX allocated memory freed after FxSDKShutdown() has
	/// been called, you can enable memory pooling for a slight performance 
	/// improvement.
	FxMemoryAllocationPolicy();
	
	/// Constructs a memory allocation policy.
	/// \param inAllocationType The type of memory allocation used.
	/// \param inbUseMemoryPools FxTrue if memory pools should be created and used.
	/// \param inAllocate A pointer to the user-defined memory allocation function.
	/// \param inAllocateDebug A pointer to the user-defined debug memory allocation function.
	/// \param inFree A pointer to the user-defined memory free function.
	/// \note If inAllocationType is MAT_Custom, inAllocate, inAllocateDebug,
	/// and inFree should all be non-NULL function pointers.
	FxMemoryAllocationPolicy(FxMemoryAllocationType inAllocationType,
        FxBool inbUseMemoryPools, FxUserAllocateFunc inAllocate,
		FxUserAllocateDebugFunc inAllocateDebug, FxUserFreeFunc inFree);

	/// Copy constructor.
	FxMemoryAllocationPolicy(const FxMemoryAllocationPolicy& other);
	/// Assignment operator.
	FxMemoryAllocationPolicy& operator=(const FxMemoryAllocationPolicy& other);

	/// Destructor.
	~FxMemoryAllocationPolicy();

	/// The type of memory allocation used under the memory allocation policy.
	FxMemoryAllocationType allocationType;
	/// FxTrue if the FaceFX SDK should create and use memory pools under the
	/// memory allocation policy.  FxFalse if no memory pools should be created
	/// and used.
	FxBool bUseMemoryPools;
	/// A pointer to the user-defined memory allocation function used under
	/// the memory allocation policy.
	FxUserAllocateFunc userAllocate;
	/// A pointer to the user-defined debug memory allocation function used
	/// under the memory allocation policy.
	FxUserAllocateDebugFunc userAllocateDebug;
	/// A pointer to the user-defined memory free function used under the
	/// memory allocation policy.
	FxUserFreeFunc userFree;
};

/// Starts up the memory system.
/// \note For internal use only.  Never call this directly.
void FX_CALL FxMemoryStartup(const FxMemoryAllocationPolicy& allocationPolicy);
/// Shuts down the memory system.
/// \note For internal use only.  Never call this directly.
void FX_CALL FxMemoryShutdown();

/// Returns a copy of the current memory allocation policy.
FxMemoryAllocationPolicy FX_CALL FxGetMemoryAllocationPolicy();

/// Removes all but one empty memory chunk from each memory pool if memory 
/// pooling is enabled with the current memory allocation policy.  This is 
/// useful to call periodically, say between level changes for instance.
void FX_CALL FxTrimMemoryPools();

/// Allocates numBytes.  Use FxAlloc #define for easy-of-use.
void* FX_CALL FxAllocate(FxSize numBytes);
/// Frees memory allocated by FxAllocate/FxAlloc.
void FX_CALL FxFree(void* ptr, FxSize n);

// If we should track memory stats, define an overloaded version of the allocate
// function which inserts information about the allocation into a list.  This
// allows us to find potential memory leaks.
#if defined(FX_TRACK_MEMORY_STATS)
	void* FX_CALL FxAllocateDebug(FxSize numBytes, const FxChar* system = "");
	#define FxAlloc(size, system) FxAllocateDebug((size), (system))
	#define FxNew(val, system) FxNewDebug(val, (system))
	#define FxNewArray(n, val, system), FxNewArrayDebug(n, val, (system))
#else
	#define FxAlloc(size, system) FxAllocate((size))
	#define FxNew(val, system) FxNewNotrack(val)
	#define FxNewArray(n, val, system) FxNewArrayNotrack(n, val)
#endif

/// Constructs an object in allocated memory.
template<class T1, class T2> void FX_CALL FxConstruct( T1* p, const T2& value )
{
	::new(static_cast<void*>(p)) T1(value);
}

/// Constructs an object in allocated memory.
/// For use with types that do not define a copy constructor.
#define FxConstructDirect(p, val) ::new(static_cast<void*>(p)) val

/// Default constructs an object in allocated memory.
template<class T> void FX_CALL FxDefaultConstruct( T* p)
{
	::new(static_cast<void*>(p)) T();
}

/// Destroy an object.
template<class T> void FX_CALL FxDestroy( T* p )
{
	if( p )
	{
		p->~T();
	}
}

/// Destroys a range of objects.
template<class FwdIter> void FX_CALL FxDestroy( FwdIter first, FwdIter last )
{
	while( first != last )
	{
		FxDestroy(&*first);
		++first;
	}
}

/// Provide a nicer syntax for allocating and constructing an object in one pass.
template<class T> T* FX_CALL FxNewNotrack( const T& value )
{
	T* retval = static_cast<T*>(FxAlloc(sizeof(T), "FxNew"));
	FxConstruct(retval, value);
	return retval;
}

/// Provide a nicer syntax for allocating and constructing an array in one pass.
template<class T> T* FX_CALL FxNewArrayNotrack( const FxSize n, const T& value )
{
	T* retval = static_cast<T*>(FxAlloc(sizeof(T) * n, "FxNewArray"));
	for( FxSize i = 0; i < n; ++i )
	{
		FxConstruct(retval+i, value);
	}
	return retval;
}

#if defined(FX_TRACK_MEMORY_STATS)
/// Memory tracking version of new.
template<class T> T* FX_CALL FxNewDebug( const T& value, const FxChar* system )
{
	T* retval = static_cast<T*>(FxAllocateDebug(sizeof(T), system));
	FxConstruct(retval, value);
	return retval;
}

/// Memory tracking version of array new.
template<class T> T* FX_CALL FxNewArrayDebug( const FxSize n, const T& value, const FxChar* system  )
{
	T* retval = static_cast<T*>(FxAllocateDebug(sizeof(T) * n, system));
	for( FxSize i = 0; i < n; ++i )
	{
		FxConstruct(retval+i, value);
	}
	return retval;
}
#endif // defined(FX_TRACK_MEMORY_STATS)

/// Provide a nicer syntax for destroying and freeing an object in one pass.
/// \note Be very careful when using FxDelete() on a base class pointer when
/// memory pools are enabled as the improper size could be reported.  If you
/// need to destroy an object via a base class pointer and reclaim its memory
/// you should get the size of the object's class via 
/// FxObject::GetClassDesc()->GetSize(), call the base class destructor 
/// directly, then call FxFree() passing the correct size of the object's class
/// or simply use the C++ delete operator.
template<class T> void FX_CALL FxDelete( T*& p )
{
	FxDestroy(p);
	FxFree(p, sizeof(T));
	p = NULL;
}

/// Provide a nicer syntax for destroying and freeing an array in one pass.
/// \note Do not use FxDeleteArray() on an array of base class pointers when
/// memory pools are enabled.  Instead you should perform the following 
/// operation on each object in the array:  for each object in the array, get 
/// the size of the object's class via FxObject::GetClassDesc()->GetSize(), call
/// the base class destructor directly, then call FxFree() passing the correct 
/// size of the object's class.
template<class T> void FX_CALL FxDeleteArray( T*& p, FxSize n )
{
	FxDestroy(p, p + n);
	FxFree(p, sizeof(T) * n);
	p = NULL;
}

/// If memory tracking is enabled, returns the current number of allocations.
/// Otherwise returns 0.
FxSize FX_CALL FxGetCurrentNumAllocations();
/// If memory tracking is enabled, returns the total number of allocation
/// operations performed.  Otherwise returns 0.
FxSize FX_CALL FxGetPeakNumAllocations();
/// If memory tracking is enabled, returns the size of the smallest single
/// allocation operation performed.  Otherwise returns 0.
FxSize FX_CALL FxGetMinAllocationSize();
/// If memory tracking is enabled, returns the size of the largest single
/// allocation operation performed.  Otherwise returns 0.
FxSize FX_CALL FxGetMaxAllocationSize();
/// If memory tracking is enabled, returns the median size of all allocation
/// operations performed.  Otherwise returns 0.
FxSize FX_CALL FxGetMedianAllocationSize();
/// If memory tracking is enabled, returns the average size of all allocation
/// operations performed.  Otherwise returns 0.
FxSize FX_CALL FxGetAvgAllocationSize();
/// If memory tracking is enabled, returns the current number of bytes allocated.
/// Otherwise returns 0.
FxSize FX_CALL FxGetCurrentBytesAllocated();
/// If memory tracking is enabled, returns the total number of bytes ever 
/// allocated.  Otherwise returns 0.
FxSize FX_CALL FxGetPeakBytesAllocated();

/// Returns the number of fixed size memory allocators currently in use by the
/// memory pooling system.  If the memory allocation policy does not support
/// memory pools 0 is returned.
FxSize FX_CALL FxGetNumFixedSizeAllocators();

/// Structure encapsulating statistics for a single fixed size memory allocator
/// in the memory pooling system.
struct FxFixedSizeAllocatorInfo
{
	/// The size (in bytes) of an allocation from the allocator.
	FxSize blockSize;
	/// The number of blocks (allocations) per memory chunk in the allocator.
	FxSize numBlocksPerChunk;
	/// The size (in bytes) of a memory chunk in the allocator.
	FxSize chunkSize;
	/// The number of memory chunks in the allocator.
	FxSize numChunks;
	/// The number of empty memory chunks in the allocator.
	FxSize numEmptyChunks;
	/// The number of full memory chunks in the allocator.
	FxSize numFullChunks;
	/// The number of partially full memory chunks in the allocator.
	FxSize numPartialChunks;
	/// The average number of blocks currently in use per memory chunk in the allocator.
	FxSize avgNumBlocksInUsePerChunk;
	/// The total size (in bytes) of the allocator.
	FxSize totalSize;
};
/// Returns statistical information on for each fixed size memory allocator
/// currently in use by the memory pooling system.  No range checking is
/// performed on index, so it must be between 0 and 
/// FxGetNumFixedSizeAllocators() - 1.
FxFixedSizeAllocatorInfo FX_CALL FxGetFixedSizeAllocatorInfo( FxSize index );

/// An allocation iterator.
typedef void* FxAllocIterator;
/// Returns the first allocation in the allocation list, or NULL if no allocations.
FxAllocIterator FX_CALL FxGetAllocStart();
/// Returns the next allocation in the allocation list, or NULL if no more allocations.
FxAllocIterator FX_CALL FxGetAllocNext(const FxAllocIterator iter);
/// Returns the previous allocation in the allocation list, or NULL if no more allocations.
FxAllocIterator FX_CALL FxGetAllocPrev(const FxAllocIterator iter);
/// Gets information about a particular allocation.
/// \param[in] iter An iterator to the desired allocation.
/// \param[out] allocNum The allocation number of the allocation.
/// \param[out] size The size of the allocation.
/// \param[out] system The system tag of the allocation.
/// \return FxTrue if the returned data is valid.  FxFalse otherwise.
FxBool FX_CALL FxGetAllocInfo(const FxAllocIterator iter, FxSize& allocNum, FxSize& size, FxChar*& system);

/// Derive classes needing to allocate memory only through FxAlloc/FxFree from
/// this.
class FxUseAllocator
{
public:
	/// Overloaded operator new for built-in FaceFX classes.
	static void* operator new(size_t size)
	{
		return FxAlloc(static_cast<FxSize>(size), "Built-in FaceFX class via operator new");
	}

	/// Overloaded operator array new for built-in FaceFX classes.
	static void* operator new[](size_t size)
	{
		return FxAlloc(static_cast<FxSize>(size), "Built-in FaceFX class via operator new[]");
	}

	/// Overloaded operator delete for built-in FaceFX classes.
	static void operator delete(void* ptr, size_t size)
	{
		FxFree(ptr, static_cast<FxSize>(size));
	}

	/// Overloaded operator array delete for built-in FaceFX classes.
	static void operator delete[](void* ptr, size_t size)
	{
		FxFree(ptr, static_cast<FxSize>(size));
	}
};

} // namespace Face

} // namespace OC3Ent

#endif
