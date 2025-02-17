//------------------------------------------------------------------------------
// Encapsulation of memory acquisition and release.
//
// Owner: Jamie Redmond
// 
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMemory.h"

#ifdef FX_TRACK_MEMORY_STATS
	#pragma message( "FX_TRACK_MEMORY_STATS is defined." )
#endif

namespace OC3Ent
{

namespace Face
{

FxMemoryAllocationPolicy::FxMemoryAllocationPolicy()
#ifdef FX_DEFAULT_TO_OPERATOR_NEW
	: allocationType(MAT_FreeStore)
#else
	: allocationType(MAT_Heap)
#endif // FX_DEFAULT_TO_OPERATOR_NEW
	, bUseMemoryPools(FxFalse)
	, userAllocate(NULL)
	, userAllocateDebug(NULL)
	, userFree(NULL) 
{
}

FxMemoryAllocationPolicy::
FxMemoryAllocationPolicy(FxMemoryAllocationType inAllocationType,
						 FxBool inbUseMemoryPools, 
						 FxUserAllocateFunc inAllocate,
						 FxUserAllocateDebugFunc inAllocateDebug, 
						 FxUserFreeFunc inFree)
	: allocationType(inAllocationType)
	, bUseMemoryPools(inbUseMemoryPools)
	, userAllocate(inAllocate)
	, userAllocateDebug(inAllocateDebug)
	, userFree(inFree)
{
}

FxMemoryAllocationPolicy::
FxMemoryAllocationPolicy(const FxMemoryAllocationPolicy& other)
	: allocationType(other.allocationType)
	, bUseMemoryPools(other.bUseMemoryPools)
	, userAllocate(other.userAllocate)
	, userAllocateDebug(other.userAllocateDebug)
	, userFree(other.userFree)
{
}

FxMemoryAllocationPolicy& FxMemoryAllocationPolicy::
operator=(const FxMemoryAllocationPolicy& other)
{
	if( this == &other ) return *this;
	allocationType = other.allocationType;
	bUseMemoryPools = other.bUseMemoryPools;
	userAllocate = other.userAllocate;
	userAllocateDebug = other.userAllocateDebug;
	userFree = other.userFree;
	return *this;
}

FxMemoryAllocationPolicy::~FxMemoryAllocationPolicy() 
{
}

// Built-in free store and heap memory allocation / free functions.
void* FxHeapAllocate(FxSize numBytes)
{
	return fx_std(malloc)(numBytes);
}

void* FxHeapAllocateDebug(FxSize numBytes, const FxChar* FxUnused(system))
{
	return fx_std(malloc)(numBytes);
}

void FxHeapFree(void* ptr, FxSize FxUnused(n))
{
	fx_std(free)(ptr);
}

void* FxFreeStoreAllocate(FxSize numBytes)
{
	return ::operator new(numBytes);
}

void* FxFreeStoreAllocateDebug(FxSize numBytes, const FxChar* FxUnused(system))
{
	return ::operator new(numBytes);
}

void FxFreeStoreFree(void* ptr, FxSize FxUnused(n))
{
	::operator delete(ptr);
}

// The global memory allocation policy.
static FxMemoryAllocationPolicy gAllocationPolicy;

// The fixed size memory allocation system used here is based on the small 
// object allocation system developed for the C++ library Loki and presented in
// the book "Modern C++ Design: Generic Programming and Design Patterns Applied"
// by Andrei Alexandrescu (ISBN 0-201-70431-5).
#define FX_DEFAULT_MEMORY_CHUNK_SIZE 4096
class FxFixedSizeMemoryAllocator
{
public:
	// Constructor.
	FxFixedSizeMemoryAllocator();
	// Destructor.
	~FxFixedSizeMemoryAllocator();
	
	// Initializes the fixed size memory allocator to allocate memory blocks
	// of blockSize bytes.
	void  Initialize(FxSize blockSize);
	// Destroys the fixed size memory allocator.
	void  Destroy();
	// Frees all but one unused memory chunk in the fixed size memory allocator.
	void  Trim();
	// Allocates a memory block from the fixed size memory allocator.
	void* Allocate();
	// Frees the block of memory pointed to by ptr from the fixed size memory
	// allocator.
	void  Free(void* ptr);
	
	// Returns the size of memory blocks allocated by the fixed size memory
	// allocator.
	FxSize GetBlockSize() const;

	// Returns statistical information on the fixed size memory allocator.
	FxFixedSizeAllocatorInfo GetInfo( void ) const;

private:
	// Disable copy construction.
	FxFixedSizeMemoryAllocator(const FxFixedSizeMemoryAllocator& other);
	// Disable assignment.
	FxFixedSizeMemoryAllocator& operator=(const FxFixedSizeMemoryAllocator& other);
	
	// A memory chunk.
	struct FxMemoryChunk
	{
		// Initializes the memory chunk.
		void  Initialize(FxSize blockSize, FxByte numBlocks);
		// Destroys the memory chunk.
		void  Destroy();
		// Resets the memory chunk (utility function used internally).
		void  Reset(FxSize blockSize, FxByte numBlocks);
		// Allocates a block of memory from the memory chunk.
		void* Allocate(FxSize blockSize);
		// Frees the block of memory pointed to by ptr from the memory chunk.
		void  Free(void* ptr, FxSize blockSize);
		
		// The raw memory in the chunk.
		FxByte* pMemory;
		// The total size of the memory pointed to by pMemory.
		FxSize  memorySize;
		// Index of the first available memory block in the chunk.
		FxByte  firstAvailableBlock;
		// The number of available unused memory blocks in the chunk.
		FxByte  numBlocksAvailable;
	};

	// An entry in the singly linked list of memory chunks.
	struct FxMemoryChunkEntry
	{
		FxMemoryChunk chunk;
		FxMemoryChunkEntry* pNext;
	};

	// Finds the chunk that owns ptr. Returns NULL if no chunk owns ptr.
	FxMemoryChunk* FindOwnerChunk(void* ptr);

	// The size of each memory block in each memory chunk.
	FxSize _blockSize;
	// The number of memory blocks in each memory chunk.
	FxByte _numBlocks;
	// Singly linked list of memory chunks.
	FxMemoryChunkEntry* _pChunks;
};

FxFixedSizeMemoryAllocator::FxFixedSizeMemoryAllocator()
	: _blockSize(0)
	, _numBlocks(0)
	, _pChunks(NULL)
{
}

FxFixedSizeMemoryAllocator::~FxFixedSizeMemoryAllocator()
{
	FxAssert(_pChunks == NULL);
	FxAssert(_blockSize == 0);
	FxAssert(_numBlocks == 0);
}

void FxFixedSizeMemoryAllocator::Initialize(FxSize blockSize)
{
	FxAssert(blockSize > 0);
	_blockSize = blockSize;

	FxSize numBlocks = FX_DEFAULT_MEMORY_CHUNK_SIZE / _blockSize;
	if( numBlocks > FX_BYTE_MAX ) numBlocks = FX_BYTE_MAX;
	else if( 0 == numBlocks ) numBlocks = 8 * _blockSize;

	_numBlocks = static_cast<FxByte>(numBlocks);
	FxAssert(_numBlocks == numBlocks);

	FxAssert(_pChunks == NULL);
#ifdef FX_TRACK_MEMORY_STATS
	FxAssert(gAllocationPolicy.userAllocateDebug);
	_pChunks = static_cast<FxMemoryChunkEntry*>(gAllocationPolicy.userAllocateDebug(sizeof(FxMemoryChunkEntry), "FxMemoryChunkEntry"));
#else
	FxAssert(gAllocationPolicy.userAllocate);
	_pChunks = static_cast<FxMemoryChunkEntry*>(gAllocationPolicy.userAllocate(sizeof(FxMemoryChunkEntry)));
#endif // FX_TRACK_MEMORY_STATS
	_pChunks->pNext = NULL;
	_pChunks->chunk.Initialize(_blockSize, _numBlocks);
}

void FxFixedSizeMemoryAllocator::Destroy()
{
	FxMemoryChunkEntry* pChunkEntry = _pChunks;
	while( pChunkEntry )
	{
		pChunkEntry->chunk.Destroy();
		FxMemoryChunkEntry* pTemp = pChunkEntry;
		pChunkEntry = pChunkEntry->pNext;
		FxAssert(gAllocationPolicy.userFree);
		gAllocationPolicy.userFree(pTemp, sizeof(FxMemoryChunkEntry));
	}
	_pChunks = NULL;
	_blockSize = 0;
	_numBlocks = 0;
}

void FxFixedSizeMemoryAllocator::Trim()
{
	FxBool bFoundFirstEmptyChunk = FxFalse;
	FxMemoryChunkEntry* pChunkEntry = _pChunks;
	while( pChunkEntry )
	{
		FxMemoryChunk& memoryChunk = pChunkEntry->chunk;
		if( _numBlocks == memoryChunk.numBlocksAvailable )
		{
			if( bFoundFirstEmptyChunk )
			{
				memoryChunk.Destroy();
				FxMemoryChunkEntry* pTemp = pChunkEntry;
				pChunkEntry = pChunkEntry->pNext;
				FxAssert(gAllocationPolicy.userFree);
				gAllocationPolicy.userFree(pTemp, sizeof(FxMemoryChunkEntry));
			}
			else
			{
				bFoundFirstEmptyChunk = FxTrue;
				pChunkEntry = pChunkEntry->pNext;
			}
		}
		else
		{
			pChunkEntry = pChunkEntry->pNext;
		}
	}
}

FX_INLINE void* FxFixedSizeMemoryAllocator::Allocate()
{
	FxAssert(_pChunks != NULL);
	FxMemoryChunkEntry* pPreviousChunkEntry = NULL;
	FxMemoryChunkEntry* pChunkEntry = _pChunks;
	while( pChunkEntry )
	{
		FxMemoryChunk& memoryChunk = pChunkEntry->chunk;
		if( memoryChunk.numBlocksAvailable )
		{
			return memoryChunk.Allocate(_blockSize);
		}
		pPreviousChunkEntry = pChunkEntry;
		pChunkEntry = pChunkEntry->pNext;
	}
	FxAssert(pPreviousChunkEntry != NULL);
	FxAssert(pPreviousChunkEntry->pNext == NULL);
	FxAssert(pChunkEntry == NULL);
#ifdef FX_TRACK_MEMORY_STATS
	FxAssert(gAllocationPolicy.userAllocateDebug);
	pChunkEntry = static_cast<FxMemoryChunkEntry*>(gAllocationPolicy.userAllocateDebug(sizeof(FxMemoryChunkEntry), "FxMemoryChunkEntry"));
#else
	FxAssert(gAllocationPolicy.userAllocate);
	pChunkEntry = static_cast<FxMemoryChunkEntry*>(gAllocationPolicy.userAllocate(sizeof(FxMemoryChunkEntry)));
#endif // FX_TRACK_MEMORY_STATS
	pChunkEntry->pNext = NULL;
	pPreviousChunkEntry->pNext = pChunkEntry;
	FxMemoryChunk& memoryChunk = pChunkEntry->chunk;
	memoryChunk.Initialize(_blockSize, _numBlocks);
	return memoryChunk.Allocate(_blockSize);
}

FX_INLINE void FxFixedSizeMemoryAllocator::Free(void* ptr)
{
	FxMemoryChunk* pOwnerChunk = FindOwnerChunk(ptr);
	FxAssert(pOwnerChunk != NULL);
	pOwnerChunk->Free(ptr, _blockSize);
}

FX_INLINE FxSize FxFixedSizeMemoryAllocator::GetBlockSize() const
{ 
	return _blockSize; 
}

FxFixedSizeAllocatorInfo FxFixedSizeMemoryAllocator::GetInfo( void ) const
{
	FxFixedSizeAllocatorInfo info;
	info.blockSize = _blockSize;
	info.numBlocksPerChunk = _numBlocks;
	info.chunkSize = static_cast<FxSize>(_numBlocks * _blockSize);
	info.numChunks = 0;
	info.numEmptyChunks = 0;
	info.numFullChunks = 0;
	info.numPartialChunks = 0;
	info.avgNumBlocksInUsePerChunk = 0;
	info.totalSize = 0;
	
	FxMemoryChunkEntry* pChunkEntry = _pChunks;
	while( pChunkEntry )
	{
		info.numChunks++;
		FxMemoryChunk& memoryChunk = pChunkEntry->chunk;
		if( _numBlocks == memoryChunk.numBlocksAvailable ) info.numEmptyChunks++;
		if( 0 == memoryChunk.numBlocksAvailable ) info.numFullChunks++;
		info.avgNumBlocksInUsePerChunk += _numBlocks - memoryChunk.numBlocksAvailable;
		pChunkEntry = pChunkEntry->pNext;
	}

	info.numPartialChunks = info.numChunks - info.numEmptyChunks - info.numFullChunks;
	info.avgNumBlocksInUsePerChunk /= info.numChunks;
	info.totalSize = info.numChunks * info.chunkSize;
		
	return info;
}

FX_INLINE FxFixedSizeMemoryAllocator::FxMemoryChunk* FxFixedSizeMemoryAllocator::FindOwnerChunk(void* ptr)
{
	// This simple linear search could be improved...
	const FxSize memoryChunkSize = static_cast<FxSize>(_numBlocks * _blockSize);
	FxMemoryChunkEntry* pChunkEntry = _pChunks;
	while( pChunkEntry )
	{
		FxMemoryChunk* pMemoryChunk = &pChunkEntry->chunk;
		FxByte* pChunkMemoryStart = pMemoryChunk->pMemory;
		if( ptr >= pChunkMemoryStart && ptr < pChunkMemoryStart + memoryChunkSize )
		{
			return pMemoryChunk;
		}
		pChunkEntry = pChunkEntry->pNext;
	}
	return NULL;
}

void FxFixedSizeMemoryAllocator::FxMemoryChunk::Initialize(FxSize blockSize, FxByte numBlocks)
{
	FxAssert(blockSize > 0);
	FxAssert(numBlocks > 0);
	// Overflow check.
	FxAssert(numBlocks == ((blockSize * numBlocks) / blockSize));
	memorySize = static_cast<FxSize>(blockSize * numBlocks);
#ifdef FX_TRACK_MEMORY_STATS
	FxAssert(gAllocationPolicy.userAllocateDebug);
	pMemory = static_cast<FxByte*>(gAllocationPolicy.userAllocateDebug(memorySize, "FxMemoryChunk"));
#else
	FxAssert(gAllocationPolicy.userAllocate);
	pMemory = static_cast<FxByte*>(gAllocationPolicy.userAllocate(memorySize));
#endif // FX_TRACK_MEMORY_STATS
	Reset(blockSize, numBlocks);
}

void FxFixedSizeMemoryAllocator::FxMemoryChunk::Destroy()
{
	FxAssert(pMemory != NULL);
	FxAssert(gAllocationPolicy.userFree);
	gAllocationPolicy.userFree(pMemory, memorySize);
	pMemory = NULL;
	memorySize = 0;
	firstAvailableBlock = 0;
	numBlocksAvailable = 0;
}

FX_INLINE void FxFixedSizeMemoryAllocator::FxMemoryChunk::Reset(FxSize blockSize, FxByte numBlocks)
{
	firstAvailableBlock = 0;
	numBlocksAvailable = numBlocks;
	// Initialize the singly linked list structure that keeps track of 
	// available blocks.
	FxByte* p = pMemory;
	for( FxByte i = 0; i != numBlocks; p += blockSize )
	{
		*p = ++i;
	}
}

FX_INLINE void* FxFixedSizeMemoryAllocator::FxMemoryChunk::Allocate(FxSize blockSize)
{
	if( 0 == numBlocksAvailable ) return NULL;
	FxAssert(firstAvailableBlock == ((firstAvailableBlock * blockSize) / blockSize));
	FxByte* pResult = pMemory + (firstAvailableBlock * blockSize);
	// Update firstAvailableBlock to point to the next available block.
	firstAvailableBlock = *pResult;
	--numBlocksAvailable;
	return pResult;
}

FX_INLINE void FxFixedSizeMemoryAllocator::FxMemoryChunk::Free(void* ptr, FxSize blockSize)
{
	// Check that ptr actually belongs to this chunk.
	FxAssert(ptr >= pMemory);
	FxAssert(ptr < (pMemory + memorySize));
	FxByte* pToRelease = static_cast<FxByte*>(ptr);
	// Alignment check.
	FxAssert(0 == ((pToRelease - pMemory) % blockSize));
	*pToRelease = firstAvailableBlock;
	firstAvailableBlock = static_cast<FxByte>((pToRelease - pMemory) / blockSize);
	// Truncation check.
	FxAssert(firstAvailableBlock == ((pToRelease - pMemory) / blockSize));
	++numBlocksAvailable;
}

// The memory pool of fixed size allocators.  It is *very* important that the 
// fixed sized allocators be sorted in ascending order of allocation size in
// gMemoryPool.
#define FX_NUM_FIXED_SIZE_MEMORY_ALLOCATORS 16
static FxFixedSizeMemoryAllocator gMemoryPool[FX_NUM_FIXED_SIZE_MEMORY_ALLOCATORS];

void FX_CALL FxMemoryStartup(const FxMemoryAllocationPolicy& allocationPolicy)
{
	gAllocationPolicy = allocationPolicy;

	switch( gAllocationPolicy.allocationType )
	{
	case MAT_FreeStore:
		gAllocationPolicy.userAllocate      = &FxFreeStoreAllocate;
		gAllocationPolicy.userAllocateDebug = &FxFreeStoreAllocateDebug;
		gAllocationPolicy.userFree          = &FxFreeStoreFree;
		break;
	case MAT_Heap:
		gAllocationPolicy.userAllocate      = &FxHeapAllocate;
		gAllocationPolicy.userAllocateDebug = &FxHeapAllocateDebug;
		gAllocationPolicy.userFree          = &FxHeapFree;
		break;
	case MAT_Custom:
		break;
	default:
		FxAssert(!"Invalid FxMemoryAllocationPolicy passed to FxMemoryStartup()!");
		break;
	}

	FxAssert(gAllocationPolicy.userAllocate);
	FxAssert(gAllocationPolicy.userAllocateDebug);
	FxAssert(gAllocationPolicy.userFree);

	if( gAllocationPolicy.bUseMemoryPools )
	{
		// Remember that these should always be initialized in ascending order.
		gMemoryPool[0].Initialize(8);
		gMemoryPool[1].Initialize(16);
		gMemoryPool[2].Initialize(24);
		gMemoryPool[3].Initialize(32);
		gMemoryPool[4].Initialize(40);
		gMemoryPool[5].Initialize(48);
		gMemoryPool[6].Initialize(56);
		gMemoryPool[7].Initialize(64);
		gMemoryPool[8].Initialize(72);
		gMemoryPool[9].Initialize(80);
		gMemoryPool[10].Initialize(88);
		gMemoryPool[11].Initialize(96);
		gMemoryPool[12].Initialize(104);
		gMemoryPool[13].Initialize(112);
		gMemoryPool[14].Initialize(120);
		gMemoryPool[15].Initialize(128);
	}
}

void FX_CALL FxMemoryShutdown()
{
	if( gAllocationPolicy.bUseMemoryPools )
	{
		for( FxSize i = 0; i < FX_NUM_FIXED_SIZE_MEMORY_ALLOCATORS; ++i )
		{
			gMemoryPool[i].Destroy();
		}
	}
}

FxMemoryAllocationPolicy FX_CALL FxGetMemoryAllocationPolicy()
{
	return gAllocationPolicy;
}

void FX_CALL FxTrimMemoryPools()
{
	if( gAllocationPolicy.bUseMemoryPools )
	{
		for( FxSize i = 0; i < FX_NUM_FIXED_SIZE_MEMORY_ALLOCATORS; ++i )
		{
			gMemoryPool[i].Trim();
		}
	}
}

// Declare these functions so that the memory tracking code can use them
// as well.
void* FxPoolAllocate(FxSize numBytes);
void FxPoolFree(void* ptr, FxSize n);

#if defined(FX_TRACK_MEMORY_STATS)
struct FxMemoryAllocation
{
	FxMemoryAllocation(void* p, FxSize sz, FxSize an, const FxChar* sys = "UnknownSystem")
		: ptr(p)
		, size(sz)
		, allocNum(an)
		, next(NULL)
		, prev(NULL)
	{
		FxMemset(system, 0, 24*sizeof(FxChar));
		FxStrncpy(system, 24, sys, 24);
		system[23] = '\0'; // Ensure null-terminated.
	}

	void* ptr;
	FxSize size;
	FxSize allocNum;
	FxChar system[24];
	FxMemoryAllocation* next;
	FxMemoryAllocation* prev;
};

class FxMemoryTracker
{
public:
	static void AddAllocation(void* ptr, FxSize size, const FxChar* system = "UnknownSystem");
	static void RemoveAllocation(void* ptr);
	static FxSize GetCurrentNumAllocations();
	static FxSize GetPeakNumAllocations();
	static FxSize GetMinAllocationSize();
	static FxSize GetMaxAllocationSize();
	static FxSize GetMedianAllocationSize();
	static FxSize GetAvgAllocationSize();
	static FxSize GetCurrentBytesAllocated();
	static FxSize GetPeakBytesAllocated();

	static void* GetFirstAlloc()
	{
		return static_cast<void*>(_head.next);
	}
private:
	static FxMemoryAllocation _head;
	static FxSize _currentAllocationCount;
	static FxSize _peakAllocationCount;
	static FxSize _minAllocationSize;
	static FxSize _maxAllocationSize;
	static FxSize _currentAllocatedBytes;
	static FxSize _peakAllocatedBytes;
	static FxSize _allocationNumber;
};

FxMemoryAllocation FxMemoryTracker::_head = FxMemoryAllocation(NULL, 0, FxInvalidIndex, "AllocListHeadDummy");
FxSize FxMemoryTracker::_currentAllocationCount = 0;
FxSize FxMemoryTracker::_peakAllocationCount    = 0;
FxSize FxMemoryTracker::_minAllocationSize      = FxInvalidIndex;
FxSize FxMemoryTracker::_maxAllocationSize      = 0;
FxSize FxMemoryTracker::_currentAllocatedBytes  = 0;
FxSize FxMemoryTracker::_peakAllocatedBytes     = 0;
FxSize FxMemoryTracker::_allocationNumber       = 0;

static const FxUInt32 FxStartFlag = 0x12345678;
static const FxUInt32 FxEndFlag   = 0x87654321;
static const FxUChar  FxAllocFill = 0xfe;
static const FxUChar  FxFreeFill  = 0xfd;

void* FxPoolAllocateDebug(FxSize numBytes, const FxChar* system)
{
	if( gAllocationPolicy.bUseMemoryPools )
	{
		for( FxSize i = 0; i < FX_NUM_FIXED_SIZE_MEMORY_ALLOCATORS; ++i )
		{
			if( gMemoryPool[i].GetBlockSize() >= numBytes )
			{
				return gMemoryPool[i].Allocate();
			}
		}
	}
	FxAssert(gAllocationPolicy.userAllocateDebug);
	return gAllocationPolicy.userAllocateDebug(numBytes, system);
}

void* FX_CALL FxAllocateDebug(FxSize numBytes, const FxChar* system)
{
	FxAssert(numBytes);

	FxSize bytesToAlloc = sizeof(FxMemoryAllocation) + sizeof(FxStartFlag) + numBytes + sizeof(FxEndFlag);
	void* retval = FxPoolAllocateDebug(bytesToAlloc, system);
	
	FxMemoryTracker::AddAllocation(retval, numBytes, system);

	// Set memory bound flags.
	*reinterpret_cast<FxUInt32*>(static_cast<FxByte*>(retval) + sizeof(FxMemoryAllocation)) = FxStartFlag;
	*reinterpret_cast<FxUInt32*>(static_cast<FxByte*>(retval) + sizeof(FxMemoryAllocation) + sizeof(FxStartFlag) + numBytes) = FxEndFlag;
	retval = static_cast<void*>(static_cast<FxByte*>(retval) + sizeof(FxMemoryAllocation) + sizeof(FxStartFlag));
	// Fill the allocation with a known value.
	FxMemset(retval, FxAllocFill, numBytes);

	return retval;
}

#else // FX_TRACK_MEMORY_STATS isn't defined.

// Define a pass-through version of FxAllocateDebug so licensees evaluating
// with the release libraries in a debug build won't get a linker error.
void* FX_CALL FxAllocateDebug(FxSize numBytes, const FxChar* FxUnused(system))
{
	return FxPoolAllocate(numBytes);
}

#endif // FX_TRACK_MEMORY_STATS

FX_INLINE void* FxPoolAllocate(FxSize numBytes)
{
	if( gAllocationPolicy.bUseMemoryPools )
	{
		for( FxSize i = 0; i < FX_NUM_FIXED_SIZE_MEMORY_ALLOCATORS; ++i )
		{
			if( gMemoryPool[i].GetBlockSize() >= numBytes )
			{
				return gMemoryPool[i].Allocate();
			}
		}
	}
	FxAssert(gAllocationPolicy.userAllocate);
	return gAllocationPolicy.userAllocate(numBytes);
}

FX_INLINE void FxPoolFree(void* ptr, FxSize n)
{
	if( ptr )
	{
#ifdef FX_TRACK_MEMORY_STATS
		// Compensate for additional memory stat tracking overhead.
		n += sizeof(FxMemoryAllocation) + sizeof(FxStartFlag) + sizeof(FxEndFlag);
#endif // FX_TRACK_MEMORY_STATS
		if( gAllocationPolicy.bUseMemoryPools )
		{
			for( FxSize i = 0; i < FX_NUM_FIXED_SIZE_MEMORY_ALLOCATORS; ++i )
			{
				if( gMemoryPool[i].GetBlockSize() >= n )
				{
					gMemoryPool[i].Free(ptr);
					return;
				}
			}
		}
		FxAssert(gAllocationPolicy.userFree);
		gAllocationPolicy.userFree(ptr, n);
	}
}

void* FX_CALL FxAllocate(FxSize numBytes)
{
	return FxPoolAllocate(numBytes);
}

void FX_CALL FxFree(void* ptr, FxSize n)
{
#if defined(FX_TRACK_MEMORY_STATS)
	if( ptr )
	{
		ptr = static_cast<FxByte*>(ptr) - sizeof(FxStartFlag) - sizeof(FxMemoryAllocation);
		FxMemoryTracker::RemoveAllocation(ptr);
	}
#endif // FX_TRACK_MEMORY_STATS

	FxPoolFree(ptr, n);
}

#if defined(FX_TRACK_MEMORY_STATS)
void FxMemoryTracker::AddAllocation(void* ptr, FxSize size, const FxChar* system)
{
	// Ensure the head pointers are valid.
	if( _head.next == NULL )
	{
		_head.next = &_head;
	}
	if( _head.prev == NULL )
	{
		_head.prev = &_head;
	}

	// Set the pointers in the new allocation.
	void* ptrToReturnedAlloc = static_cast<void*>(static_cast<FxByte*>(ptr) + sizeof(FxMemoryAllocation) + sizeof(FxStartFlag));
	FxMemoryAllocation* currAlloc = ::new(ptr) FxMemoryAllocation(ptrToReturnedAlloc, size, _allocationNumber, system);
	currAlloc->next = _head.next;
	currAlloc->prev = &_head;
	// Include the allocation in the list.
	_head.next->prev = currAlloc;
	_head.next = currAlloc;

	// Update some running stats.
	++_currentAllocationCount;
	++_peakAllocationCount;
	_minAllocationSize = size < _minAllocationSize ? size : _minAllocationSize;
	_maxAllocationSize = size > _maxAllocationSize ? size : _maxAllocationSize;
	_currentAllocatedBytes += size;
	_peakAllocatedBytes = _currentAllocatedBytes > _peakAllocatedBytes ? _currentAllocatedBytes : _peakAllocatedBytes;
	++_allocationNumber;
}

void FxMemoryTracker::RemoveAllocation(void* ptr)
{
	FxMemoryAllocation* currAlloc = reinterpret_cast<FxMemoryAllocation*>(ptr);
	FxByte* dataStartPtr = static_cast<FxByte*>(ptr) + sizeof(FxStartFlag) + sizeof(FxMemoryAllocation);

	// First, a sanity check: the allocation information structure stores a
	// pointer to the location in memory that was actually returned to the 
	// client.  Make sure this is still the case.  If it is not, there was 
	// either massive damage preceding the block (i.e., the client wrote off 
	// the beginning of the array by at least 36 bytes) or the memory chunk 
	// was not originally allocated with our system.
	if( static_cast<FxByte*>(currAlloc->ptr) != dataStartPtr )
	{
		FxAssert(!"FxMemoryTracker Error: Block not allocated by this system, or damage detected before block");
		//@todo: abort? continue?
		return;
	}

	// Check that we're not doing a duplicate free.
	FxBool freed = FxTrue;
	for( FxSize i = 0; i < currAlloc->size; ++i )
	{
		freed &= dataStartPtr[i] == FxFreeFill;
	}
	if( freed )
	{
		FxAssert(!"FxMemoryTracker Error: Block has already been freed.");
	}

	// Remove the allocation from the system.
	currAlloc->next->prev = currAlloc->prev;
	currAlloc->prev->next = currAlloc->next;

	// Remove some stats
	--_currentAllocationCount;
	_currentAllocatedBytes -= currAlloc->size;

	// Check the bounds of the memory being freed, if necessary.
	FxByte* bytePtr = static_cast<FxByte*>(ptr) + sizeof(FxMemoryAllocation);
	if( static_cast<FxUInt32>(*reinterpret_cast<FxUInt32*>(bytePtr)) != FxStartFlag )
	{
		FxAssert(!"FxMemoryTracker Warning: Damage detected before block.");
	}
	// Advance to the start of the real allocation.
	bytePtr += sizeof(FxStartFlag);
	// Clear the allocation to a known value.
	FxMemset(bytePtr, FxFreeFill, currAlloc->size);
	// Advance to the start of the end block.
	bytePtr += currAlloc->size;
	if( static_cast<FxUInt32>(*reinterpret_cast<FxUInt32*>(bytePtr)) != FxEndFlag )
	{
		FxAssert(!"FxMemoryTracker Warning: Damage detected after block.");
	}

	// Destroy the allocation information.
	FxDestroy(currAlloc);
}

FxSize FxMemoryTracker::GetCurrentNumAllocations()
{
	return _currentAllocationCount;
}

FxSize FxMemoryTracker::GetPeakNumAllocations()
{
	return _peakAllocationCount;
}

FxSize FxMemoryTracker::GetMinAllocationSize()
{
	return _minAllocationSize;
}

FxSize FxMemoryTracker::GetMaxAllocationSize()
{
	return _maxAllocationSize;
}

FxSize FxMemoryTracker::GetMedianAllocationSize()
{
	if( FxInvalidIndex != _minAllocationSize )
	{
		return (_maxAllocationSize - _minAllocationSize) / 2;
	}
	return 0;
}

FxSize FxMemoryTracker::GetAvgAllocationSize()
{
	if( _peakAllocationCount > 0 )
	{
		return _peakAllocatedBytes / _peakAllocationCount;
	}
	return 0;
}

FxSize FxMemoryTracker::GetCurrentBytesAllocated()
{
	return _currentAllocatedBytes;
}

FxSize FxMemoryTracker::GetPeakBytesAllocated()
{
	return _peakAllocatedBytes;
}

#endif // FX_TRACK_MEMORY_STATS

FxSize FX_CALL FxGetCurrentNumAllocations()
{
#if defined(FX_TRACK_MEMORY_STATS)
	return FxMemoryTracker::GetCurrentNumAllocations();
#else
	return 0;
#endif
}

FxSize FX_CALL FxGetPeakNumAllocations()
{
#if defined(FX_TRACK_MEMORY_STATS)
	return FxMemoryTracker::GetPeakNumAllocations();
#else
	return 0;
#endif
}

FxSize FX_CALL FxGetMinAllocationSize()
{
#if defined(FX_TRACK_MEMORY_STATS)
	return FxMemoryTracker::GetMinAllocationSize();
#else
	return 0;
#endif
}

FxSize FX_CALL FxGetMaxAllocationSize()
{
#if defined(FX_TRACK_MEMORY_STATS)
	return FxMemoryTracker::GetMaxAllocationSize();
#else
	return 0;
#endif
}

FxSize FX_CALL FxGetMedianAllocationSize()
{
#if defined(FX_TRACK_MEMORY_STATS)
	return FxMemoryTracker::GetMedianAllocationSize();
#else
	return 0;
#endif
}

FxSize FX_CALL FxGetAvgAllocationSize()
{
#if defined(FX_TRACK_MEMORY_STATS)
	return FxMemoryTracker::GetAvgAllocationSize();
#else
	return 0;
#endif
}

FxSize FX_CALL FxGetCurrentBytesAllocated()
{
#if defined(FX_TRACK_MEMORY_STATS)
	return FxMemoryTracker::GetCurrentBytesAllocated();
#else
	return 0;
#endif
}

FxSize FX_CALL FxGetPeakBytesAllocated()
{
#if defined(FX_TRACK_MEMORY_STATS)
	return FxMemoryTracker::GetPeakBytesAllocated();
#else
	return 0;
#endif // FX_TRACK_MEMORY_STATS
}

FxSize FX_CALL FxGetNumFixedSizeAllocators()
{
	if( gAllocationPolicy.bUseMemoryPools )
	{
		return FX_NUM_FIXED_SIZE_MEMORY_ALLOCATORS;
	}
	return 0;
}

FxFixedSizeAllocatorInfo FX_CALL FxGetFixedSizeAllocatorInfo( FxSize index )
{
	if( gAllocationPolicy.bUseMemoryPools )
	{
		return gMemoryPool[index].GetInfo();
	}
	else
	{
		FxFixedSizeAllocatorInfo info;
		info.blockSize = 0;
		info.numBlocksPerChunk = 0;
		info.chunkSize = 0;
		info.numChunks = 0;
		info.numEmptyChunks = 0;
		info.numFullChunks = 0;
		info.numPartialChunks = 0;
		info.avgNumBlocksInUsePerChunk = 0;
		info.totalSize = 0;
		return info;
	}
}

FxAllocIterator FX_CALL FxGetAllocStart()
{
#if defined(FX_TRACK_MEMORY_STATS)
	return FxMemoryTracker::GetFirstAlloc();
#else
	return NULL;
#endif // FX_TRACK_MEMORY_STATS
}

#ifdef FX_TRACK_MEMORY_STATS
	#define FxPossibleParam(val) val
#else
	#define FxPossibleParam(val) /*val*/
#endif

FxAllocIterator FX_CALL FxGetAllocNext(const FxAllocIterator FxPossibleParam(iter))
{
#if defined(FX_TRACK_MEMORY_STATS)
	if( iter )
	{
		FxAllocIterator nextAlloc = static_cast<FxAllocIterator>(static_cast<FxMemoryAllocation*>(iter)->next);
		if( nextAlloc != FxMemoryTracker::GetFirstAlloc() )
		{
			return nextAlloc;
		}
	}
#endif // FX_TRACK_MEMORY_STATS
	return NULL;
}

FxAllocIterator FX_CALL FxGetAllocPrev(const FxAllocIterator FxPossibleParam(iter))
{
#if defined(FX_TRACK_MEMORY_STATS)
	if( iter )
	{
		FxAllocIterator prevAlloc = static_cast<FxAllocIterator>(static_cast<FxMemoryAllocation*>(iter)->prev);
		if( prevAlloc != FxMemoryTracker::GetFirstAlloc() )
		{
			return prevAlloc;
		}
	}
#endif // FX_TRACK_MEMORY_STATS
	return NULL;
}

FxBool FX_CALL FxGetAllocInfo(const FxAllocIterator FxPossibleParam(iter), 
					          FxSize& FxPossibleParam(allocNum), 
					          FxSize& FxPossibleParam(size), 
					          FxChar*& FxPossibleParam(system))
{
#if defined(FX_TRACK_MEMORY_STATS)
	if( iter && iter != FxMemoryTracker::GetFirstAlloc() )
	{
		FxMemoryAllocation* currAlloc = static_cast<FxMemoryAllocation*>(iter);
		allocNum = currAlloc->allocNum;
		size = currAlloc->size;
		system = currAlloc->system;
		return FxTrue;
	}
#endif // FX_TRACK_MEMORY_STATS

	return FxFalse;
}

#ifdef FxPossibleParam
	#undef FxPossibleParam
#endif

} // namespace Face

} // namespace OC3Ent
