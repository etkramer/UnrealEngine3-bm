/*=============================================================================
	XeD3DTextureAllocator.cpp: Xbox 360 D3D texture allocator.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "XeD3DDrvPrivate.h"
#include "BestFitAllocator.h"


/** Enables/disables the texture memory defragmentation feature. */
UBOOL GEnableTextureMemoryDefragmentation = TRUE;


#if USE_XeD3D_RHI

#define USE_CUSTOM_TEXTURE_ALLOCATOR	1


#if USE_CUSTOM_TEXTURE_ALLOCATOR

/*-----------------------------------------------------------------------------
	FXeTexturePool implementation.
-----------------------------------------------------------------------------*/

class FXeTexturePool : protected FPresizedMemoryPool
{
public:
	
	/**
	 * Default constructor
	 */
	FXeTexturePool( );

	/**
	 * Destructor
	 */
	~FXeTexturePool( );

	/**
	 * Returns whether allocator has been initialized or not.
	 */
	UBOOL IsInitialized()
	{
		return FPresizedMemoryPool::IsInitialized();
	}

	/**
	 * Initializes the FXeTexturePool (will allocate physical memory!)
	 */
	void	Initialize( );

	/**
	 * Allocates texture memory.
	 *
	 * @param	Size			Size of allocation
	 * @param	bAllowFailure	Whether to allow allocation failure or not
	 * @param	bDisableDefrag	Whether to disable automatic defragmentation if the allocation failed
	 * @returns					Pointer to allocated memory
	 */
	void*	Allocate( DWORD Size, UBOOL bAllowFailure, UBOOL bDisableDefrag );

	/**
	 * Frees texture memory allocated via Allocate
	 *
	 * @param	Pointer		Allocation to free
	 */
	void	Free( void* Pointer );

	/**
	 * Tries to reallocate texture memory in-place (without relocating),
	 * by adjusting the base address of the allocation but keeping the end address the same.
	 *
	 * @param	OldBaseAddress	Pointer to the original allocation
	 * @param	NewBaseAddress	New desired baseaddress for the allocation (adjusting the size so the end stays the same)
	 * @returns	TRUE if it succeeded
	 **/
	UBOOL	Reallocate( void* OldBaseAddress, void* NewBaseAddress );

	/**
	 * Retrieves allocation stats.
	 *
	 * @param AllocatedSize					Upon return, contains the size of all texture memory allocations
	 * @param AvailableSize					Upon return, contains how much free space there is in the texture memory
	 * @param LargestAvailableAllocation	Upon return, contains the size of the largest allocation possible. Optional - can be NULL.
	 * @param NumFreeChunks					Upon return, contains the total number of free chunks (fragmentation holes). Optional - can be NULL.
	 */
	void	GetMemoryStats( INT& AllocatedSize, INT& AvailableSize, INT* LargestAvailableAllocation=NULL, INT* NumFreeChunks=NULL );

	/**
	 * Scans the free chunks and returns the largest size you can allocate.
	 *
	 * @param OutNumFreeChunks	Upon return, contains the total number of free chunks. May be NULL.
	 * @return					The largest size of all free chunks.
	 */
	INT		GetLargestAvailableAllocation( INT* OutNumFreeChunks=NULL );

	/**
	 * Locks the texture pool to prevent any changes to it (i.e. preventing defragmentation)
	 */
	void	Lock();

	/**
	 * Unlocks the texture pool to allow changes to it again (i.e. allowing defragmentation)
	 */
	void	Unlock();

	/**
	 * Determines whether this pointer resides within the texture memory pool.
	 *
	 * @param	Pointer		Pointer to check
	 * @return				TRUE if the pointer resides within the texture memory pool, otherwise FALSE.
	 */
	UBOOL	IsTextureMemory( void* Pointer );

	/**
	 * Checks whether the pointer represents valid texture data.
	 * @param	Pointer		Baseaddress of the texture data to check
	 * @return				TRUE if it points to valid texture data
	 */
	UBOOL	IsValidTextureData( void* Pointer );

	/**
	 * Notify the texture pool that a new texture has been created.
	 * This allows its memory to be relocated during defragmention.
	 *
	 * @param Texture	The new texture object that has been created
	 */
	void	AddTexture( FXeTextureBase* Texture );

	/**
	 * Notify the texture pool that a texture has been deleted.
	 *
	 * @param Texture	The texture object that is being deleted
	 */
	void	RemoveTexture( FXeTextureBase* Texture );

	/**
	 * Fills in the texture pointer array with a list of textures matching the given base address.
	 *
	 * @param BaseAddress	The base address to search for
	 * @param FoundTextures	Array of texture pointers to fill in with matching textures
	 * @param NumTextures	(IN/OUT) Pointer to the size of the array and returned as the number found 
	 * @param bSecondary	Use SecondarBaseAddress as the search key 
	 */
	void	FindTextures( void* BaseAddress, FXeTextureBase** FoundTextures, UINT *NumTextures, UBOOL bSecondary = FALSE);

	/**
	 * Returns the number of textures matching the given parameters.
	 *
	 * @param MatchSharingFlags		The Sharing flags to match on
	 * @param bMatchSharedOnly		If true, only match shared textures
	 * @return						Number of matching textures
	 */
	UINT	FindTextures( DWORD MatchSharingFlags, UBOOL bMatchSharedOnly = FALSE);

	/**
	 * Add a new donor texture to the head of the list.
	 *
	 * @param Texture	The texture object that is being added
	 */
	void	PushDonorTexture( FXeTextureBase* Texture );

	/**
	 * Get a donor texture from the head of the list.
	 *
	 * @param DonorPoolIndex	Index into the array of donor pools to pop from
	 * @return					Donor texture that can share its texture memory with another texture
	 */
	FXeTextureBase*	PopDonorTexture( DWORD DonorPoolIndex );

	/**
	 * Add a new donor texture to the end of the list.
	 *
	 * @param Texture	The texture object that is being added
	 */
	void	AppendDonorTexture( FXeTextureBase* Texture );

	/**
	 * Defragment the texture memory. This function can be called from both gamethread and renderthread.
	 * Texture memory is shuffled around primarily using GPU transfers. Texture memory that can't be associated
	 * with a tracked texture object will not be relocated. Locked textures will not be relocated either.
	 */
	void	DefragmentTextureMemory( );

	/**
	 * Helper class to carry out the task of moving memory around.
	 */
	struct FTextureMemoryDefragmentationPolicy : public FBestFitAllocator::FDefragmentationPolicy
	{
		/**
		 * Copy memory from one location to another. If it returns FALSE (e.g. it doesn't belong to
		 * a tracked texture resource, or the texture is locked), the defragmentation process will
		 * assume the memory is not relocatable and keep it in place.
		 * If the transfer is non-overlapping, it will use the GPU to perform the memory transfer.
		 *
		 * @param Dest		Destination memory start address
		 * @param Source	Source memory start address
		 * @param Size		Number of bytes to copy
		 * @return			FALSE if the copy failed
		 */
		virtual UBOOL Relocate( void* Dest, const void* Source, INT Size );
	};

protected:

	/** Contains all textures resources that can be relocated during defragmentation. */
	TArray<FXeTextureBase*>	Textures;
	/** Contains all texture that can share their texture memory with another texture. */
	TArray<FXeTextureBase*>	DonorTextures[TEXTURESHARING_NumTypes];
};


/** Static best fit texture allocator. */
static FXeTexturePool GTexturePool;

/**
 * Copy memory from one location to another. If it returns FALSE (e.g. it doesn't belong to
 * a tracked texture resource, or the texture is locked), the defragmentation process will
 * assume the memory is not relocatable and keep it in place.
 * If the transfer is non-overlapping, it will use the GPU to perform the memory transfer.
 *
 * @param Dest		Destination memory start address
 * @param Source	Source memory start address
 * @param Size		Number of bytes to copy
 * @return			FALSE if the copy failed
 */
UBOOL FXeTexturePool::FTextureMemoryDefragmentationPolicy::Relocate( void* Dest, const void* Source, INT Size )
{
	FXeTextureBase* FoundTextures[2];
	UINT NumTextures = 2;
	UBOOL bMatchedSecondary = FALSE;

	// if it can be shared, we want to look for another texture using the base address
	// before we free the memory
	GTexturePool.FindTextures( (void*)Source , &(FoundTextures[0]), &NumTextures);
	if ( NumTextures == 0 )
	{
		bMatchedSecondary = TRUE;
		GTexturePool.FindTextures( (void*)Source , &(FoundTextures[0]), &NumTextures, TRUE);
	}

	if ( NumTextures != 0 )
	{
		FXeTextureBase* Texture = FoundTextures[0];
		if ( Texture->CanRelocate() == FALSE )
		{
			return FALSE;
		}

		DWORD Flags = Texture->GetCreationFlags();
		D3DRESOURCETYPE TextureType = Texture->Resource->GetType();
		switch ( TextureType )
		{
			case D3DRTYPE_TEXTURE:
			{
				if(Texture->bIsShared && (NumTextures == 2))
				{
					FXeTextureBase* Texture1 = NULL;
					FXeTextureBase* Texture2 = FoundTextures[1];
					if ( Texture2->CanRelocate() == FALSE )
					{
						return FALSE;
					}

					// The texture with the secondary base address should be the second one
					// when calling XGSetTextureHeaderPair()
					if(Texture->SecondaryBaseAddress != NULL || !Texture->IsDonor())
					{
						Texture1 = Texture2;
						Texture2 = Texture;
					}
					else
					{
						Texture1 = Texture;
					}

					// we have a paired resource, so we have to re-offset both texture headers
					D3DSURFACE_DESC Desc1;
					IDirect3DTexture9* XeTexture1 = (IDirect3DTexture9*) Texture1->Resource;
					XeTexture1->GetLevelDesc(0, &Desc1);
					DWORD NumMips1 = XeTexture1->GetLevelCount();

					D3DSURFACE_DESC Desc2;
					IDirect3DTexture9* XeTexture2 = (IDirect3DTexture9*) Texture2->Resource;
					XeTexture2->GetLevelDesc(0, &Desc2);
					DWORD NumMips2 = XeTexture2->GetLevelCount();

					XGSetTextureHeaderPair(
						0,
						((Texture2->SecondaryBaseAddress != NULL) ? 0 : XGHEADER_CONTIGUOUS_MIP_OFFSET),
						Desc1.Width,
						Desc1.Height,
						NumMips1, 
						0,
						Desc1.Format,
						0,
						0, // note that XGHEADEREX_NONPACKED will neve be set for paired textures
						0,
						XeTexture1,
						Desc2.Width,
						Desc2.Height,
						NumMips2, 
						0,
						Desc2.Format,
						0,
						0, // note that XGHEADEREX_NONPACKED will neve be set for paired textures
						0,
						XeTexture2,
						NULL,
						NULL
					);

					// setup the base address offsets for both textures
					XGOffsetResourceAddress( XeTexture1, Dest );
					Texture1->BaseAddress = Dest;

					if(Texture2->SecondaryBaseAddress == NULL)
					{
						XGOffsetResourceAddress( XeTexture2, Dest );
					}
					else
					{
						XGOffsetBaseTextureAddress(XeTexture2, Texture2->SecondaryBaseAddress, Dest);
					}
					Texture2->BaseAddress = Dest;
				}
				else
				{
					D3DSURFACE_DESC Desc;
					IDirect3DTexture9* XeTexture = (IDirect3DTexture9*) Texture->Resource;
					XeTexture->GetLevelDesc(0, &Desc);
					DWORD NumMips = XeTexture->GetLevelCount();

					DWORD TextureSize = XGSetTextureHeaderEx( 
						Desc.Width,
						Desc.Height,
						NumMips, 
						0,
						Desc.Format,
						0,
						(Flags&TexCreate_NoMipTail) ? XGHEADEREX_NONPACKED : 0,
						0,
						Texture->SecondaryBaseAddress != NULL ? 0 : XGHEADER_CONTIGUOUS_MIP_OFFSET,
						0,
						XeTexture,
						NULL,
						NULL
						); 

					if(bMatchedSecondary)
					{
						// the secondary base address should only be matched once
						// unless something is really wrong
						XGOffsetBaseTextureAddress(XeTexture, Dest, Texture->BaseAddress);
						Texture->SecondaryBaseAddress = Dest;
					}
					else
					{
						if(Texture->SecondaryBaseAddress == NULL)
						{
					XGOffsetResourceAddress( XeTexture, Dest );
						}
						else
						{
							XGOffsetBaseTextureAddress(XeTexture, Texture->SecondaryBaseAddress, Dest);
						}
						Texture->BaseAddress = Dest;
					}
				}
				break;
			}
			case D3DRTYPE_CUBETEXTURE:
			{
				D3DSURFACE_DESC Desc;
				IDirect3DCubeTexture9* XeTexture = (IDirect3DCubeTexture9*) Texture->Resource;
				XeTexture->GetLevelDesc(0, &Desc);
				DWORD NumMips = XeTexture->GetLevelCount();
				DWORD TextureSize = XGSetCubeTextureHeaderEx( 
					Desc.Width,
					NumMips, 
					0,
					Desc.Format,
					0,
					(Flags&TexCreate_NoMipTail) ? XGHEADEREX_NONPACKED : 0,
					0,
					XGHEADER_CONTIGUOUS_MIP_OFFSET,
					XeTexture,
					NULL,
					NULL 
					);
				XGOffsetResourceAddress( XeTexture, Dest );
				Texture->BaseAddress = Dest;
				break;
			}
			default:
			{
				check( FALSE );
				return FALSE;
			}
		}
		appMemmove( Dest, Source, Size );
		return TRUE;
	}
	return FALSE;
}

/**
 * Default constructor
 */
FXeTexturePool::FXeTexturePool( )
{
}

/**
 * Destructor
 */
FXeTexturePool::~FXeTexturePool()
{
}


/**
 * Initializes the FXeTexturePool (will allocate physical memory!)
 */
void FXeTexturePool::Initialize()
{
	FScopeLock ScopeLock( &SynchronizationObject );
	if ( !IsInitialized() )
	{
		// Retrieve texture memory pool size from ini.
		INT MemoryPoolSize = 0;
			check(GConfig);	
			verify(GConfig->GetInt(TEXT("TextureStreaming"), TEXT("PoolSize"), MemoryPoolSize, GEngineIni));
		// Convert from MByte to byte and align.
		MemoryPoolSize = Align(MemoryPoolSize * 1024 * 1024, D3DTEXTURE_ALIGNMENT);

		// Initialize the allocator
		FPresizedMemoryPool::Initialize( MemoryPoolSize, D3DTEXTURE_ALIGNMENT );
	}
}


/**
 * Allocates texture memory.
 *
 * @param	Size			Size of allocation
 * @param	bAllowFailure	Whether to allow allocation failure or not
 * @param	bDisableDefrag	Whether to disable automatic defragmentation if the allocation failed
 * @returns	Pointer to allocated memory
 */
void* FXeTexturePool::Allocate( DWORD Size, UBOOL bAllowFailure, UBOOL bDisableDefrag )
{
	void* Pointer = FPresizedMemoryPool::Allocate( Size, GEnableTextureMemoryDefragmentation || bAllowFailure );

	// Only do a panic defragmentation if we're not allowing failure.
	if ( Pointer == AllocationFailurePointer && GEnableTextureMemoryDefragmentation && !bDisableDefrag )
	{
		INC_DWORD_STAT( STAT_PanicDefragmentations );
		DefragmentTextureMemory();
		Pointer = FPresizedMemoryPool::Allocate( Size, bAllowFailure );
	}

	// Did the allocation fail?
	if ( Pointer == AllocationFailurePointer && !bAllowFailure )
	{
		// Mark texture memory as having been corrupted or not
		extern UBOOL GIsTextureMemoryCorrupted;
		if ( !GIsTextureMemoryCorrupted )
		{
#if !FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE
			INT AllocatedSize, AvailableSize, LargestAvailableAllocation, NumFreeChunks;
			GetMemoryStats( AllocatedSize, AvailableSize, &LargestAvailableAllocation, &NumFreeChunks );
			GLog->Logf( TEXT("RAN OUT OF TEXTURE MEMORY, EXPECT CORRUPTION AND GPU HANGS!") );
			GLog->Logf( TEXT("Tried to allocate %d bytes"), Size );
			GLog->Logf( TEXT("Texture memory available: %.3f MB"), FLOAT(AvailableSize)/1024.0f/1024.0f );
			GLog->Logf( TEXT("Largest available allocation: %.3f MB (%d holes)"), FLOAT(LargestAvailableAllocation)/1024.0f/1024.0f, NumFreeChunks );
#endif
			GIsTextureMemoryCorrupted = TRUE;
		}
	}
	return Pointer;
}

/**
 * Frees texture memory allocated via Allocate
 *
 * @param	Pointer		Allocation to free
 */
void FXeTexturePool::Free( void* Pointer )
{
	FPresizedMemoryPool::Free( Pointer );
}

/**
 * Tries to reallocate texture memory in-place (without relocating),
 * by adjusting the base address of the allocation but keeping the end address the same.
 *
 * @param	OldBaseAddress	Pointer to the original allocation
 * @param	NewBaseAddress	New desired baseaddress for the allocation (adjusting the size so the end stays the same)
 * @returns	TRUE if it succeeded
 **/
UBOOL FXeTexturePool::Reallocate( void* OldBaseAddress, void* NewBaseAddress )
{
	return FPresizedMemoryPool::Reallocate( OldBaseAddress, NewBaseAddress );
}


/**
 * Retrieves allocation stats.
 *
 * @param AllocatedSize					Upon return, contains the size of all texture memory allocations
 * @param AvailableSize					Upon return, contains how much free space there is in the texture memory
 * @param LargestAvailableAllocation	Upon return, contains the size of the largest allocation possible.  Optional - can be NULL.
 * @param NumFreeChunks					Upon return, contains the total number of free chunks (fragmentation holes).  Optional - can be NULL.
 */
void FXeTexturePool::GetMemoryStats( INT& AllocatedSize, INT& AvailableSize, INT* LargestAvailableAllocation/*=NULL*/, INT* NumFreeChunks/*=NULL*/ )
{
	FPresizedMemoryPool::GetMemoryStats( AllocatedSize, AvailableSize );
	INT TempLargestAvailableAllocation = 0;
	if ( LargestAvailableAllocation || NumFreeChunks )
	{
		TempLargestAvailableAllocation = GTexturePool.GetLargestAvailableAllocation( NumFreeChunks );
	}
	if ( LargestAvailableAllocation )
	{
		*LargestAvailableAllocation = TempLargestAvailableAllocation;
	}
}

/**
 * Scans the free chunks and returns the largest size you can allocate.
 *
 * @param OutNumFreeChunks	Upon return, contains the total number of free chunks. May be NULL.
 * @return					The largest size of all free chunks.
 */
INT FXeTexturePool::GetLargestAvailableAllocation( INT* OutNumFreeChunks/*=NULL*/ )
{
	return FPresizedMemoryPool::GetLargestAvailableAllocation( OutNumFreeChunks );
}

/**
 * Locks the texture pool to prevent any changes to it (i.e. preventing defragmentation)
 */
void FXeTexturePool::Lock()
{
	SynchronizationObject.Lock();
}

/**
 * Unlocks the texture pool to allow changes to it again (i.e. allowing defragmentation)
 */
void FXeTexturePool::Unlock()
{
	SynchronizationObject.Unlock();
}

/**
 * Determines whether this pointer resides within the texture memory pool.
 *
 * @param	Pointer		Pointer to check
 * @return				TRUE if the pointer resides within the texture memory pool, otherwise FALSE.
 */
UBOOL FXeTexturePool::IsTextureMemory( void* Pointer )
{
	UBOOL bIsWithinTexturePool = ( PTRINT(Pointer) >= PTRINT(PhysicalMemoryBase) && PTRINT(Pointer) < (PTRINT(PhysicalMemoryBase) + PTRINT(PhysicalMemorySize)) );
	return bIsWithinTexturePool;
}

/**
 * Checks whether the pointer represents valid texture data.
 * @param	Pointer		Baseaddress of the texture data to check
 * @return				TRUE if it points to valid texture data
 */
UBOOL FXeTexturePool::IsValidTextureData( void* Pointer )
{
	return Pointer && Pointer != AllocationFailurePointer;
}

/**
 * Notify the texture pool that a new texture has been created.
 * This allows its memory to be relocated during defragmention.
 *
 * @param Texture	The new texture object that has been created
 */
void FXeTexturePool::AddTexture( FXeTextureBase* Texture )
{
	FScopeLock ScopeLock( &SynchronizationObject );
	Textures.AddItem( Texture );
}

/**
 * Notify the texture pool that a texture has been deleted.
 *
 * @param Texture	The texture object that is being deleted
 */
void FXeTexturePool::RemoveTexture( FXeTextureBase* Texture )
{
	FScopeLock ScopeLock( &SynchronizationObject );
	INT Index = Textures.FindItemIndex( Texture );
	if ( Index != INDEX_NONE )
	{
		Textures.RemoveSwap( Index );
	}

	if(Texture->GetSharingFlags()&TEXTURESHARING_Donor)
	{
		UINT DonorPoolIndex = Texture->GetSharingFlags()&TEXTURESHARING_TypeMask;
		Index = DonorTextures[DonorPoolIndex].FindItemIndex( Texture );
		if ( Index != INDEX_NONE )
		{
			DonorTextures[DonorPoolIndex].RemoveSwap( Index );
		}
	}
}

/**
 * Fills in the texture pointer array with a list of textures matching the given base address.
 *
 * @param BaseAddress	The base address to search for
 * @param FoundTextures	Array of texture pointers to fill in with matching textures
 * @param NumTextures	(IN/OUT) Pointer to the size of the array and returned as the number found 
 * @param bSecondary	Use SecondarBaseAddress as the search key 
 */
void FXeTexturePool::FindTextures( void* BaseAddress, FXeTextureBase** FoundTextures, UINT *NumTextures, UBOOL bSecondary )
{
	UINT MaxTextures = *NumTextures;
	*NumTextures = 0;

	UINT OffsetOfAddress = bSecondary ? offsetof(FXeTextureBase,SecondaryBaseAddress) : offsetof(FXeTextureBase,BaseAddress);

	for ( INT Index=0;(Index < Textures.Num()) && (*NumTextures < MaxTextures); ++Index )
	{
		FXeTextureBase* Texture = Textures( Index );
		if ( *((void**)((BYTE*)Texture + OffsetOfAddress)) == BaseAddress )
		{
			FoundTextures[(*NumTextures)++] = Texture;
		}
	}
}

/**
 * Returns the number of textures matching the given parameters.
 *
 * @param MatchSharingFlags		The Sharing flags to match on
 * @param bMatchSharedOnly		If true, only match shared textures
 * @return						Number of matching textures
 */
UINT FXeTexturePool::FindTextures( DWORD MatchSharingFlags, UBOOL bMatchSharedOnly)
{
	UINT MatchCount = 0;
	for ( INT Index=0;Index < Textures.Num(); ++Index )
	{
		if( (Textures(Index)->GetSharingFlags() & MatchSharingFlags) &&
			(bMatchSharedOnly ? Textures(Index)->bIsShared : TRUE) )
		{
			MatchCount++;
		}
	}
	return MatchCount;
}

/**
 * Add a new donor texture to the head of the list.
 *
 * @param Texture	The texture object that is being added
 */
void	FXeTexturePool::PushDonorTexture( FXeTextureBase* Texture)
{
	FScopeLock ScopeLock( &SynchronizationObject );

	UINT DonorPoolIndex = Texture->GetSharingFlags()&TEXTURESHARING_TypeMask;
	DonorTextures[DonorPoolIndex].Push( Texture );

}

/**
 * Get a donor texture from the head of the list.
 *
 * @param DonorPoolIndex	Index into the array of donor pools to pop from
 * @return					Donor texture that can share its texture memory with another texture
 */
FXeTextureBase*	FXeTexturePool::PopDonorTexture( DWORD DonorPoolIndex )
{
	FScopeLock ScopeLock( &SynchronizationObject );

	FXeTextureBase* Texture = NULL;

	if(DonorTextures[DonorPoolIndex].Num())
	{
		Texture = DonorTextures[DonorPoolIndex].Pop();
	}

	return Texture;
}

/**
 * Add a new donor texture to the end of the list.
 *
 * @param Texture	The texture object that is being added
 */
void	FXeTexturePool::AppendDonorTexture( FXeTextureBase* Texture)
{
	FScopeLock ScopeLock( &SynchronizationObject );

	UINT DonorPoolIndex = Texture->GetSharingFlags()&TEXTURESHARING_TypeMask;
	DonorTextures[DonorPoolIndex].InsertItem( Texture , 0 );

}

/**
 * Defragment the texture memory. This function can be called from both gamethread and renderthread.
 * Texture memory is shuffled around primarily using GPU transfers. Texture memory that can't be associated
 * with a tracked texture object will not be relocated. Locked textures will not be relocated either.
 */
void FXeTexturePool::DefragmentTextureMemory()
{
	extern void DeleteUnusedXeResources();

	if ( IsInRenderingThread() )
	{
		FTextureMemoryDefragmentationPolicy Policy;
		RHIBeginScene();
		XeBlockUntilGPUIdle();
		DeleteUnusedXeResources();
		DeleteUnusedXeResources();
		check( PhysicalMemoryBase );
		XeLockTexturePool();
		XPhysicalProtect( PhysicalMemoryBase, PhysicalMemorySize, PAGE_READWRITE );
		FPresizedMemoryPool::DefragmentMemory( Policy );
		XPhysicalProtect( PhysicalMemoryBase, PhysicalMemorySize, PAGE_READWRITE | PAGE_WRITECOMBINE );
		XeUnlockTexturePool();
		XeBlockUntilGPUIdle();
		RHIEndScene();
	}
	else
	{
		// Flush and delete all gamethread-deferred-deletion objects (like textures).
		FlushRenderingCommands();

		ENQUEUE_UNIQUE_RENDER_COMMAND(
			DefragmentTextureMemoryCommand,
		{
			FTextureMemoryDefragmentationPolicy Policy;
			RHIBeginScene();
			XeBlockUntilGPUIdle();
			DeleteUnusedXeResources();
			DeleteUnusedXeResources();
			check( GTexturePool.PhysicalMemoryBase );
			XeLockTexturePool();
			XPhysicalProtect( GTexturePool.PhysicalMemoryBase, GTexturePool.PhysicalMemorySize, PAGE_READWRITE );
			GTexturePool.DefragmentMemory( Policy );
			XPhysicalProtect( GTexturePool.PhysicalMemoryBase, GTexturePool.PhysicalMemorySize, PAGE_READWRITE | PAGE_WRITECOMBINE );
			XeUnlockTexturePool();
			XeBlockUntilGPUIdle();
			RHIEndScene();
		});

		// Flush and blocks until defragmentation is completed.
		FlushRenderingCommands();
	}
}

/**
 * Allocates texture memory.
 *
 * @param	Size			Size of allocation
 * @param	bAllowFailure	Whether to allow allocation failure or not
 * @param	bDisableDefrag	Whether to disable automatic defragmentation if the allocation failed
 * @returns					Pointer to allocated memory
 */
void* XeAllocateTextureMemory( DWORD Size, UBOOL bAllowFailure, UBOOL bDisableDefrag )
{
	if ( !GTexturePool.IsInitialized() )
	{
		GTexturePool.Initialize();
	}

	// allocate some texture memory
	void* Pointer = GTexturePool.Allocate(Size, bAllowFailure, bDisableDefrag);
	return Pointer;
}

/**
 * Frees texture memory allocated via AllocateTextureMemory
 *
 * @param	Pointer		Allocation to free
 * @param	bIsShared	TRUE if resource can use shared memory
 */
void XeFreeTextureMemory( void* Pointer, UBOOL bIsShared )
{
	if(bIsShared)
	{
		FXeTextureBase* Texture = NULL;
		UINT NumTextures = 1;

		// if it can be shared, we want to look for another texture using the base address
		// before we free the memory
		GTexturePool.FindTextures( Pointer, &Texture, &NumTextures );

		if(Texture != NULL)
		{
			Texture->Lock(0);
			Texture->bIsShared = FALSE;
			if(Texture->GetSharingFlags()&TEXTURESHARING_Recipient)
			{
				Texture->SetOrphan();
			}
			else if(Texture->GetSharingFlags()&TEXTURESHARING_Donor)
			{
				GTexturePool.AppendDonorTexture(Texture);
			}
			Texture->Unlock(0);
		}
		// memory is still being used, so return from here without freeing it
		return;
	}
	// free the pointer
	GTexturePool.Free(Pointer);
}

/**
 * Checks whether the pointer represents valid texture data.
 * @param	Pointer		Baseaddress of the texture data to check
 * @return				TRUE if it points to valid texture data
 */
UBOOL XeIsValidTextureData( void* Pointer )
{
	return GTexturePool.IsValidTextureData( Pointer );
}

/**
 * Tries to reallocate texture memory in-place (without relocating),
 * by adjusting the base address of the allocation but keeping the end address the same.
 *
 * @param	OldBaseAddress	Pointer to the original allocation
 * @param	NewBaseAddress	New desired baseaddress for the allocation (adjusting the size so the end stays the same)
 * @returns	TRUE if it succeeded
 **/
UBOOL XeReallocateTextureMemory( void* OldBaseAddress, void* NewBaseAddress )
{
	if ( !GTexturePool.IsInitialized() )
	{
		GTexturePool.Initialize();
	}

	// Try to reallocate the texture data in place.
	UBOOL bSuccess = GTexturePool.Reallocate(OldBaseAddress, NewBaseAddress);
	return bSuccess;
}

/**
 * Add a new donor texture to the head of the list.
 *
 * @param Texture	The texture object that is being added
 */
void XePushDonorTexture(FXeTextureBase* Texture)
{
	GTexturePool.PushDonorTexture(Texture);
}

/**
 * Get a donor texture from the head of the list.
 *
 * @param DonorPoolIndex	Index into the array of donor pools to pop from
 * @return					Donor texture that can share its texture memory with another texture
 */
FXeTextureBase* XePopDonorTexture(DWORD DonorPoolIndex)
{
	return 	GTexturePool.PopDonorTexture(DonorPoolIndex);
}

/**
 * Add a new donor texture to the end of the list.
 *
 * @param Texture	The texture object that is being added
 */
void XeAppendDonorTexture(FXeTextureBase* Texture)
{
	GTexturePool.AppendDonorTexture(Texture);
}

/**
 * Registers a new texture object with the allocator. This enables it to be considered
 * for relocation during texture memory defragmentation.
 *
 * @param	Texture		A texture that can be relocated during defragmentation.
 */
void XeRegisterTexture( FXeTextureBase* Texture )
{
	GTexturePool.AddTexture( Texture );
}

/**
 * Unregisters a texture object from the allocator and its defragmentation feature.
 *
 * @param	Texture		A previously registered texture.
 */
void XeUnregisterTexture( FXeTextureBase* Texture )
{
	GTexturePool.RemoveTexture( Texture );
}

/**
 * Locks the texture pool to prevent any changes to it (i.e. preventing defragmentation)
 */
void XeLockTexturePool()
{
	GTexturePool.Lock();
}

/**
 * Unlocks the texture pool to allow changes to it again (i.e. allowing defragmentation)
 */
void XeUnlockTexturePool()
{
	GTexturePool.Unlock();
}

/**
 * Determines whether this pointer resides within the texture memory pool.
 *
 * @param	Pointer		Pointer to check
 * @return				TRUE if the pointer resides within the texture memory pool, otherwise FALSE.
 */
UBOOL XeIsTextureMemory( void* Pointer )
{
	return GTexturePool.IsTextureMemory( Pointer );
}

/**
 * Defragment the texture pool.
 */
void appDefragmentTexturePool()
{
	if ( GEnableTextureMemoryDefragmentation )
	{
		GTexturePool.DefragmentTextureMemory();
	}
}

/**
 * Retrieves texture memory stats.
 *
 * @param	OutAllocatedMemorySize	[out]	Size of allocated memory
 * @param	OutAvailableMemorySize	[out]	Size of available memory
 *
 * @return TRUE, indicating that out variables are being set
 */
UBOOL RHIGetTextureMemoryStats( INT& AllocatedMemorySize, INT& AvailableMemorySize )
{
	GTexturePool.GetMemoryStats( AllocatedMemorySize, AvailableMemorySize );
	return TRUE;
}

/**
 * Log the current texture memory stats.
 *
 * @param Message	This text will be included in the log
 */
void appDumpTextureMemoryStats(const TCHAR* Message)
{
#if !FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE
	INT AllocatedSize, AvailableSize, LargestAvailableAllocation, NumFreeChunks, 
		NumDonorTextures, NumPossibleTextureShares, TotalTextureShares, KbMemorySaved,
		NumSharedDXT1Textures, NumSharedDXT3or5Textures;

	GTexturePool.GetMemoryStats( AllocatedSize, AvailableSize, &LargestAvailableAllocation, &NumFreeChunks );
	debugf(NAME_DevMemory,TEXT("%s - Texture memory available: %.3f MB. Largest available allocation: %.3f MB (%d holes)"), Message, FLOAT(AvailableSize)/1024.0f/1024.0f, FLOAT(LargestAvailableAllocation)/1024.0f/1024.0f, NumFreeChunks );

	NumDonorTextures = GTexturePool.FindTextures(TEXTURESHARING_Donor);
	NumPossibleTextureShares = GTexturePool.FindTextures(TEXTURESHARING_Recipient);
	NumSharedDXT1Textures = GTexturePool.FindTextures(TEXTURESHARING_DXT1, TRUE) / 2;
	NumSharedDXT3or5Textures = GTexturePool.FindTextures(TEXTURESHARING_DXT3or5, TRUE) / 2;
	TotalTextureShares = NumSharedDXT1Textures + NumSharedDXT3or5Textures;
	KbMemorySaved =  (NumSharedDXT1Textures * 24) + (NumSharedDXT3or5Textures * 48);

	debugf(NAME_DevMemory,TEXT("%s - Number of textures using shared MIP memory: %d. Memory saved: %d Kb"),
		Message, TotalTextureShares, KbMemorySaved );
#endif
}

#else	//USE_CUSTOM_TEXTURE_ALLOCATOR


/*-----------------------------------------------------------------------------
	Fallback allocator, using appPhysicalAlloc/Free.
-----------------------------------------------------------------------------*/

/**
 * Allocates texture memory.
 *
 * @param	Size			Size of allocation
 * @param	bAllowFailure	Whether to allow allocation failure or not
 * @param	bDisableDefrag	Whether to disable automatic defragmentation if the allocation failed
 * @returns					Pointer to allocated memory
 */
void* XeAllocateTextureMemory( DWORD Size, UBOOL bAllowFailure, UBOOL bDisableDefrag )
{
	void* Pointer = appPhysicalAlloc( Size, CACHE_WriteCombine );
#if LOG_EVERY_ALLOCATION
	debugf( TEXT("Texture Alloc: %p  Size: %i"), Pointer, Size );
#endif
	return Pointer;
}

/**
 * Frees texture memory allocated via AllocateTextureMemory
 *
 * @param	Pointer		Allocation to free
 * @param	bIsShared	TRUE if resource can use shared memory
 */
void XeFreeTextureMemory( void* Pointer, UBOOL bIsShared )
{
#if LOG_EVERY_ALLOCATION
	debugf( TEXT("Texture Free : %p"), Pointer );
#endif
	appPhysicalFree( Pointer );
}

/**
 * Checks whether the pointer represents valid texture data.
 * @param	Pointer		Baseaddress of the texture data to check
 * @return				TRUE if it points to valid texture data
 */
UBOOL XeIsValidTextureData( void* Pointer )
{
	return Pointer != NULL;
}

/**
 * Tries to reallocate texture memory in-place (without relocating),
 * by adjusting the base address of the allocation but keeping the end address the same.
 *
 * @param	OldBaseAddress	Pointer to the original allocation
 * @param	NewBaseAddress	New desired baseaddress for the allocation (adjusting the size so the end stays the same)
 * @returns	TRUE if it succeeded
 **/
UBOOL XeReallocateTextureMemory( void* OldBaseAddress, void* NewBaseAddress )
{
	return FALSE;
}

/**
 * Retrieves texture memory stats. Unsupported with this allocator.
 *
 * @return FALSE, indicating that out variables were left unchanged.
 */
UBOOL RHIGetTextureMemoryStats( INT& /*AllocatedMemorySize*/, INT& /*AvailableMemorySize*/ )
{
	return FALSE;
}

/**
 * Registers a new texture object with the allocator. This enables it to be considered
 * for relocation during texture memory defragmentation.
 *
 * @param	Texture		A texture that can be relocated during defragmentation.
 */
void XeRegisterTexture( FXeTextureBase* /*Texture*/ )
{
}

/**
 * Unregisters a texture object from the allocator and its defragmentation feature.
 *
 * @param	Texture		A previously registered texture.
 */
void XeUnregisterTexture( FXeTextureBase* /*Texture*/ )
{
}

/**
 * Add a new donor texture to the head of the list.
 *
 * @param Texture	The texture object that is being added
 */
void XePushDonorTexture(FXeTextureBase* Texture)
{
}

/**
 * Get a donor texture from the head of the list.
 *
 * @param DonorPoolIndex	Index into the array of donor pools to pop from
 * @return					Donor texture that can share its texture memory with another texture
 */
FXeTextureBase* XePopDonorTexture(DWORD DonorPoolIndex)
{
	return 	NULL;
}

/**
 * Add a new donor texture to the end of the list.
 *
 * @param Texture	The texture object that is being added
 */
void XeAppendDonorTexture(FXeTextureBase* Texture)
{
}

/**
 * Defragment the texture pool.
 */
void appDefragmentTexturePool()
{
}

/**
 * Log the current texture memory stats.
 *
 * @param Message	This text will be included in the log
 */
void appDumpTextureMemoryStats(const TCHAR* /*Message*/)
{
}


#endif	//USE_CUSTOM_TEXTURE_ALLOCATOR

#endif	//USE_XeD3D_RHI


#if !USE_XeD3D_RHI || !USE_CUSTOM_TEXTURE_ALLOCATOR

/**
 * Locks the texture pool to prevent any changes to it (i.e. preventing defragmentation)
 */
void XeLockTexturePool()
{
}

/**
 * Unlocks the texture pool to allow changes to it again (i.e. allowing defragmentation)
 */
void XeUnlockTexturePool()
{
}

/**
 * Determines whether this pointer resides within the texture memory pool.
 *
 * @param	Pointer		Pointer to check
 * @return				TRUE if the pointer resides within the texture memory pool, otherwise FALSE.
 */
UBOOL XeIsTextureMemory( void* Pointer )
{
	return FALSE;
}


#endif	//!USE_XeD3D_RHI || !USE_CUSTOM_TEXTURE_ALLOCATOR
