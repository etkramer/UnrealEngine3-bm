/*=============================================================================
	XeD3DTextureAllocator.h: Xbox 360 D3D texture allocator definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if USE_XeD3D_RHI


/**
 * Allocates texture memory.
 *
 * @param	Size			Size of allocation
 * @param	bAllowFailure	Whether to allow allocation failure or not
 * @param	bDisableDefrag	Whether to disable automatic defragmentation if the allocation failed
 * @returns					Pointer to allocated memory
 */
void* XeAllocateTextureMemory( DWORD Size, UBOOL bAllowFailure, UBOOL bDisableDefrag );

/**
 * Frees texture memory allocated via AllocateTextureMemory
 *
 * @param	Pointer		Allocation to free
 * @param	bIsShared	TRUE if resource can use shared memory
 */
void XeFreeTextureMemory( void* Pointer, UBOOL bIsShared );

/**
 * Checks whether the pointer represents valid texture data.
 * @param	Pointer		Baseaddress of the texture data to check
 * @return				TRUE if it points to valid texture data
 */
UBOOL XeIsValidTextureData( void* Pointer );

/**
 * Tries to reallocate texture memory in-place (without relocating),
 * by adjusting the base address of the allocation but keeping the end address the same.
 *
 * @param	OldBaseAddress	Pointer to the original allocation
 * @param	NewBaseAddress	New desired baseaddress for the allocation (adjusting the size so the end stays the same)
 * @returns	TRUE if it succeeded
 **/
UBOOL XeReallocateTextureMemory( void* OldBaseAddress, void* NewBaseAddress );

/**
 * Registers a new texture object with the allocator. This enables it to be considered
 * for relocation during texture memory defragmentation.
 *
 * @param	Texture		A texture that can be relocated during defragmentation.
 */
void XeRegisterTexture( class FXeTextureBase* Texture );

/**
 * Unregisters a texture object from the allocator and its defragmentation feature.
 *
 * @param	Texture		A previously registered texture.
 */
void XeUnregisterTexture( class FXeTextureBase* Texture );

/**
 * Add a new donor texture to the head of the list.
 *
 * @param Texture	The texture object that is being added
 */
void XePushDonorTexture(FXeTextureBase* Texture);

/**
 * Get a donor texture from the head of the list.
 *
 * @param DonorPoolIndex	Index into the array of donor pools to pop from
 * @return					Donor texture that can share its texture memory with another texture
 */
FXeTextureBase* XePopDonorTexture(DWORD DonorPoolIndex);

/**
 * Add a new donor texture to the end of the list.
 *
 * @param Texture	The texture object that is being added
 */
void XeAppendDonorTexture(FXeTextureBase* Texture);


#endif	//USE_XeD3D_RHI


/**
 * Locks the texture pool to prevent any changes to it (i.e. preventing defragmentation)
 */
void XeLockTexturePool();

/**
 * Unlocks the texture pool to allow changes to it again (i.e. allowing defragmentation)
 */
void XeUnlockTexturePool();

/**
 * Determines whether this pointer resides within the texture memory pool.
 *
 * @param	Pointer		Pointer to check
 * @return				TRUE if the pointer resides within the texture memory pool, otherwise FALSE.
 */
UBOOL XeIsTextureMemory( void* Pointer );
