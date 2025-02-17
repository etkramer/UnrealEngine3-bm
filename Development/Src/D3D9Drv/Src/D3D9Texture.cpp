/*=============================================================================
	D3D9Texture.cpp: D3D texture RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D9DrvPrivate.h"

/** Determines the usage flags of a RHI texture based on the creation flags. */
static DWORD GetD3DTextureUsageFlags(DWORD RHIFlags)
{
	DWORD D3DUsageFlags = 0;
	if (RHIFlags & TexCreate_ResolveTargetable)
	{
		D3DUsageFlags |= D3DUSAGE_RENDERTARGET;
	}
	if (RHIFlags & TexCreate_DepthStencil)
	{
		D3DUsageFlags |= D3DUSAGE_DEPTHSTENCIL;
	}
#if 0
	// Don't use the D3DUSAGE_DYNAMIC flag for dynamic RHI textures, as they can only be written to indirectly
	// through a D3DPOOL_SYSTEMMEM texture.
	if (RHIFlags & TexCreate_Dynamic)
	{
		D3DUsageFlags |= D3DUSAGE_DYNAMIC;
	}
#endif
	return D3DUsageFlags;
}

/** Determines the pool to create a RHI texture in based on the creation flags. */
static D3DPOOL GetD3DTexturePool(DWORD RHIFlags)
{
	if(RHIFlags & (TexCreate_ResolveTargetable | TexCreate_DepthStencil/* | TexCreate_Dynamic*/))
	{
		// Put resolve targets, depth stencil textures, and dynamic textures in the default pool.
		return D3DPOOL_DEFAULT;
	}
	else
	{
		// All other textures go in the managed pool.
		return D3DPOOL_MANAGED;
	}
}

/*-----------------------------------------------------------------------------
	Texture allocator support.
-----------------------------------------------------------------------------*/

/**
 * Retrieves texture memory stats. Unsupported with this allocator.
 *
 * @return FALSE, indicating that out variables were left unchanged.
 */
UBOOL FD3D9DynamicRHI::GetTextureMemoryStats( INT& /*AllocatedMemorySize*/, INT& /*AvailableMemorySize*/ )
{
	return FALSE;
}


/*-----------------------------------------------------------------------------
	2D texture support.
-----------------------------------------------------------------------------*/

/**
* Creates a 2D RHI texture resource
* @param SizeX - width of the texture to create
* @param SizeY - height of the texture to create
* @param Format - EPixelFormat texture format
* @param NumMips - number of mips to generate or 0 for full mip pyramid
* @param Flags - ETextureCreateFlags creation flags
*/
FTexture2DRHIRef FD3D9DynamicRHI::CreateTexture2D(UINT SizeX,UINT SizeY,BYTE Format,UINT NumMips,DWORD Flags,FResourceBulkDataInterface* BulkData)
{
	FD3D9Texture2D* Texture2D = new FD3D9Texture2D( Flags&TexCreate_SRGB, (GetD3DTextureUsageFlags(Flags)&D3DUSAGE_DYNAMIC) );
	VERIFYD3D9CREATETEXTURERESULT(Direct3DDevice->CreateTexture(
		SizeX,
		SizeY,
		NumMips,
		GetD3DTextureUsageFlags(Flags),
		(D3DFORMAT)GPixelFormats[Format].PlatformFormat,
		GetD3DTexturePool(Flags),
		Texture2D->GetInitReference(),
		NULL
		),
		SizeX, SizeY, Format, NumMips, GetD3DTextureUsageFlags(Flags));
	return Texture2D;
}

/**
 * Tries to reallocate the texture without relocation. Returns a new valid reference to the resource if successful.
 * Both the old and new reference refer to the same texture (at least the shared mip-levels) and both can be used or released independently.
 *
 * @param	Texture2D		- Texture to reallocate
 * @param	NewMipCount		- New number of mip-levels
 * @return					- New reference to the updated texture, or invalid if the reallocation failed
 */
FTexture2DRHIRef FD3D9DynamicRHI::ReallocateTexture2D( FTexture2DRHIParamRef Texture2D, INT NewMipCount )
{
	return FTexture2DRHIRef();
}

/**
* Locks an RHI texture's mip surface for read/write operations on the CPU
* @param Texture - the RHI texture resource to lock
* @param MipIndex - mip level index for the surface to retrieve
* @param bIsDataBeingWrittenTo - used to affect the lock flags 
* @param DestStride - output to retrieve the textures row stride (pitch)
* @param bLockWithinMiptail - for platforms that support packed miptails allow locking of individual mip levels within the miptail
* @return pointer to the CPU accessible resource data
*/
void* FD3D9DynamicRHI::LockTexture2D(FTexture2DRHIParamRef TextureRHI,UINT MipIndex,UBOOL bIsDataBeingWrittenTo,UINT& DestStride,UBOOL bLockWithinMiptail)
{
	DYNAMIC_CAST_D3D9RESOURCE(Texture2D,Texture);

	D3DLOCKED_RECT	LockedRect;
	DWORD			LockFlags = D3DLOCK_NOSYSLOCK;
	if( !bIsDataBeingWrittenTo )
	{
		LockFlags |= D3DLOCK_READONLY;
	}
	if( Texture->IsDynamic() )
	{
		// Discard the previous contents of the texture if it's dynamic.
		LockFlags |= D3DLOCK_DISCARD;
	}
	VERIFYD3D9RESULT((*Texture)->LockRect(MipIndex,&LockedRect,NULL,LockFlags));
	DestStride = LockedRect.Pitch;
	return LockedRect.pBits;
}

/**
* Unlocks a previously locked RHI texture resource
* @param Texture - the RHI texture resource to unlock
* @param MipIndex - mip level index for the surface to unlock
* @param bLockWithinMiptail - for platforms that support packed miptails allow locking of individual mip levels within the miptail
*/
void FD3D9DynamicRHI::UnlockTexture2D(FTexture2DRHIParamRef TextureRHI,UINT MipIndex,UBOOL bLockWithinMiptail)
{
	DYNAMIC_CAST_D3D9RESOURCE(Texture2D,Texture);
	VERIFYD3D9RESULT((*Texture)->UnlockRect(MipIndex));
}

/**
 * Checks if a texture is still in use by the GPU.
 * @param Texture - the RHI texture resource to check
 * @param MipIndex - Which mipmap we're interested in
 * @return TRUE if the texture is still in use by the GPU, otherwise FALSE
 */
UBOOL FD3D9DynamicRHI::IsBusyTexture2D(FTexture2DRHIParamRef Texture, UINT MipIndex)
{
	//@TODO: Implement somehow!
	return FALSE;
}

/**
* For platforms that support packed miptails return the first mip level which is packed in the mip tail
* @return mip level for mip tail or -1 if mip tails are not used
*/
INT FD3D9DynamicRHI::GetMipTailIdx(FTexture2DRHIParamRef Texture)
{
	return -1;
}

/**
* Copies a region within the same mip levels of one texture to another.  An optional region can be speci
* Note that the textures must be the same size and of the same format.
* @param DstTexture - destination texture resource to copy to
* @param MipIdx - mip level for the surface to copy from/to. This mip level should be valid for both source/destination textures
* @param BaseSizeX - width of the texture (base level). Same for both source/destination textures
* @param BaseSizeY - height of the texture (base level). Same for both source/destination textures 
* @param Format - format of the texture. Same for both source/destination textures
* @param Region - list of regions to specify rects and source textures for the copy
*/
void FD3D9DynamicRHI::CopyTexture2D(FTexture2DRHIParamRef DstTextureRHI, UINT MipIdx, INT BaseSizeX, INT BaseSizeY, INT Format, const TArray<FCopyTextureRegion2D>& Regions)
{
	DYNAMIC_CAST_D3D9RESOURCE(Texture2D,DstTexture);
	check( DstTexture );

	// scale the base SizeX,SizeY for the current mip level
	INT MipSizeX = Max((INT)GPixelFormats[Format].BlockSizeX,BaseSizeX >> MipIdx);
	INT MipSizeY = Max((INT)GPixelFormats[Format].BlockSizeY,BaseSizeY >> MipIdx);

	// lock the destination texture
	UINT DstStride;
	BYTE* DstData = (BYTE*)RHILockTexture2D( DstTexture, MipIdx, TRUE, DstStride, FALSE );

	for( INT RegionIdx=0; RegionIdx < Regions.Num(); RegionIdx++ )		
	{
		const FCopyTextureRegion2D& Region = Regions(RegionIdx);
		check( Region.SrcTexture );

		// lock source RHI texture
		UINT SrcStride=0;
		BYTE* SrcData = (BYTE*)RHILockTexture2D( 
			Region.SrcTexture,
			// it is possible for the source texture to have > mips than the dest so start at the FirstMipIdx
			Region.FirstMipIdx + MipIdx,
			FALSE,
			SrcStride,
			FALSE
			);	

		// align/truncate the region offset to block size
		INT RegionOffsetX = (Clamp( Region.OffsetX, 0, MipSizeX - GPixelFormats[Format].BlockSizeX ) / GPixelFormats[Format].BlockSizeX) * GPixelFormats[Format].BlockSizeX;
		INT RegionOffsetY = (Clamp( Region.OffsetY, 0, MipSizeY - GPixelFormats[Format].BlockSizeY ) / GPixelFormats[Format].BlockSizeY) * GPixelFormats[Format].BlockSizeY;
		// scale region size to the current mip level. Size is aligned to the block size
		check(Region.SizeX != 0 && Region.SizeY != 0);
		INT RegionSizeX = Clamp( Align( Region.SizeX, GPixelFormats[Format].BlockSizeX), 0, MipSizeX );
		INT RegionSizeY = Clamp( Align( Region.SizeY, GPixelFormats[Format].BlockSizeY), 0, MipSizeY );
		// handle special case for full copy
		if( Region.SizeX == -1 || Region.SizeY == -1 )
		{
			RegionSizeX = MipSizeX;
			RegionSizeY = MipSizeY;
		}

		// size in bytes of an entire row for this mip
		DWORD PitchBytes = (MipSizeX / GPixelFormats[Format].BlockSizeX) * GPixelFormats[Format].BlockBytes;
		// size in bytes of the offset to the starting part of the row to copy for this mip
		DWORD RowOffsetBytes = (RegionOffsetX / GPixelFormats[Format].BlockSizeX) * GPixelFormats[Format].BlockBytes;
		// size in bytes of the amount to copy within each row
		DWORD RowSizeBytes = (RegionSizeX / GPixelFormats[Format].BlockSizeX) * GPixelFormats[Format].BlockBytes;

		// copy each region row in increments of the block size
		for( INT CurOffsetY=RegionOffsetY; CurOffsetY < (RegionOffsetY+RegionSizeY); CurOffsetY += GPixelFormats[Format].BlockSizeY )
		{
			INT CurBlockOffsetY = CurOffsetY / GPixelFormats[Format].BlockSizeY;

			BYTE* SrcOffset = SrcData + (CurBlockOffsetY * PitchBytes) + RowOffsetBytes;
			BYTE* DstOffset = DstData + (CurBlockOffsetY * PitchBytes) + RowOffsetBytes;
			appMemcpy( DstOffset, SrcOffset, RowSizeBytes );
		}

		// done reading from source mip so unlock it
		RHIUnlockTexture2D( Region.SrcTexture, Region.FirstMipIdx + MipIdx, FALSE );
	}

	// unlock the destination texture
	RHIUnlockTexture2D( DstTexture, MipIdx, FALSE );
}

/**
 * Copies texture data freom one mip to another
 * Note that the mips must be the same size and of the same format.
 * @param SrcText Source texture to copy from
 * @param SrcMipIndex Mip index into the source texture to copy data from
 * @param DestText Destination texture to copy to
 * @param DestMipIndex Mip index in the destination texture to copy to - note this is probably different from source mip index if the base widths/heights are different
 * @param Size Size of mip data
 * @param Counter Thread safe counter used to flag end of transfer
 */
void FD3D9DynamicRHI::CopyMipToMipAsync(FTexture2DRHIParamRef SrcTextureRHI, INT SrcMipIndex, FTexture2DRHIParamRef DestTextureRHI, INT DestMipIndex, INT Size, FThreadSafeCounter& Counter)
{
	DYNAMIC_CAST_D3D9RESOURCE(Texture2D,SrcTexture);
	DYNAMIC_CAST_D3D9RESOURCE(Texture2D,DestTexture);

	// Lock old and new texture.
	UINT SrcStride;
	UINT DestStride;

	void* Src = RHILockTexture2D( SrcTexture, SrcMipIndex, FALSE, SrcStride, FALSE );
	void* Dst = RHILockTexture2D( DestTexture, DestMipIndex, TRUE, DestStride, FALSE );
	check(SrcStride == DestStride);
	appMemcpy( Dst, Src, Size );
	RHIUnlockTexture2D( SrcTexture, SrcMipIndex, FALSE );
	RHIUnlockTexture2D( DestTexture, DestMipIndex, FALSE );
}

/**
 * Copies mip data from one location to another, selectively copying only used memory based on
 * the texture tiling memory layout of the given mip level
 * Note that the mips must be the same size and of the same format.
 * @param Texture - texture to base memory layout on
 * @param Src - source memory base address to copy from
 * @param Dst - destination memory base address to copy to
 * @param MemSize - total size of mip memory
 * @param MipIdx - mip index to base memory layout on
 */
void FD3D9DynamicRHI::SelectiveCopyMipData(FTexture2DRHIParamRef Texture, BYTE *Src, BYTE *Dst, UINT MemSize, UINT MipIdx)
{
	appMemcpy(Dst, Src, MemSize);
}

void FD3D9DynamicRHI::FinalizeAsyncMipCopy(FTexture2DRHIParamRef SrcTextureRHI, INT SrcMipIndex, FTexture2DRHIParamRef DestTextureRHI, INT DestMipIndex)
{
}

/*-----------------------------------------------------------------------------
Shared texture support.
-----------------------------------------------------------------------------*/

/**
* Create resource memory to be shared by multiple RHI resources
* @param Size - aligned size of allocation
* @return shared memory resource RHI ref
*/
FSharedMemoryResourceRHIRef FD3D9DynamicRHI::CreateSharedMemory(SIZE_T Size)
{
	// create the shared memory resource
	FSharedMemoryResourceRHIRef SharedMemory(NULL);
	return SharedMemory;
}

/**
 * Creates a RHI texture and if the platform supports it overlaps it in memory with another texture
 * Note that modifying this texture will modify the memory of the overlapped texture as well
 * @param SizeX - The width of the surface to create.
 * @param SizeY - The height of the surface to create.
 * @param Format - The surface format to create.
 * @param ResolveTargetTexture - The 2d texture to use the memory from if the platform allows
 * @param Flags - Surface creation flags
 * @return The surface that was created.
 */
FSharedTexture2DRHIRef FD3D9DynamicRHI::CreateSharedTexture2D(UINT SizeX,UINT SizeY,BYTE Format,UINT NumMips,FSharedMemoryResourceRHIParamRef SharedMemoryRHI,DWORD Flags)
{
	DYNAMIC_CAST_D3D9RESOURCE(SharedMemoryResource,SharedMemory);

	FD3D9SharedTexture2D* Texture2D = new FD3D9SharedTexture2D( Flags&TexCreate_SRGB, (GetD3DTextureUsageFlags(Flags)&D3DUSAGE_DYNAMIC) );
	VERIFYD3D9RESULT(Direct3DDevice->CreateTexture(
		SizeX,
		SizeY,
		NumMips,
		GetD3DTextureUsageFlags(Flags),
		(D3DFORMAT)GPixelFormats[Format].PlatformFormat,
		GetD3DTexturePool(Flags),
		Texture2D->GetInitReference(),
		NULL
		));
	return Texture2D;
}

/*-----------------------------------------------------------------------------
	Cubemap texture support.
-----------------------------------------------------------------------------*/

/**
* Creates a Cube RHI texture resource
* @param Size - width/height of the texture to create
* @param Format - EPixelFormat texture format
* @param NumMips - number of mips to generate or 0 for full mip pyramid
* @param Flags - ETextureCreateFlags creation flags
*/
FTextureCubeRHIRef FD3D9DynamicRHI::CreateTextureCube( UINT Size, BYTE Format, UINT NumMips, DWORD Flags,FResourceBulkDataInterface* BulkData )
{
	FD3D9TextureCube* TextureCube = new FD3D9TextureCube( Flags&TexCreate_SRGB, (GetD3DTextureUsageFlags(Flags)&D3DUSAGE_DYNAMIC) );
	VERIFYD3D9RESULT( Direct3DDevice->CreateCubeTexture(
		Size,
		NumMips,
		GetD3DTextureUsageFlags(Flags),
		(D3DFORMAT)GPixelFormats[Format].PlatformFormat,
		GetD3DTexturePool(Flags),
		TextureCube->GetInitReference(),
		NULL
		));
	return TextureCube;
}

/**
* Locks an RHI texture's mip surface for read/write operations on the CPU
* @param Texture - the RHI texture resource to lock
* @param MipIndex - mip level index for the surface to retrieve
* @param bIsDataBeingWrittenTo - used to affect the lock flags 
* @param DestStride - output to retrieve the textures row stride (pitch)
* @param bLockWithinMiptail - for platforms that support packed miptails allow locking of individual mip levels within the miptail
* @return pointer to the CPU accessible resource data
*/
void* FD3D9DynamicRHI::LockTextureCubeFace(FTextureCubeRHIParamRef TextureCubeRHI,UINT FaceIndex,UINT MipIndex,UBOOL bIsDataBeingWrittenTo,UINT& DestStride,UBOOL bLockWithinMiptail)
{
	DYNAMIC_CAST_D3D9RESOURCE(TextureCube,TextureCube);

	D3DLOCKED_RECT LockedRect;
	VERIFYD3D9RESULT((*TextureCube)->LockRect( (D3DCUBEMAP_FACES) FaceIndex, MipIndex, &LockedRect, NULL, 0 ));
	DestStride = LockedRect.Pitch;
	return LockedRect.pBits;
}

/**
* Unlocks a previously locked RHI texture resource
* @param Texture - the RHI texture resource to unlock
* @param MipIndex - mip level index for the surface to unlock
* @param bLockWithinMiptail - for platforms that support packed miptails allow locking of individual mip levels within the miptail
*/
void FD3D9DynamicRHI::UnlockTextureCubeFace(FTextureCubeRHIParamRef TextureCubeRHI,UINT FaceIndex,UINT MipIndex,UBOOL bLockWithinMiptail)
{
	DYNAMIC_CAST_D3D9RESOURCE(TextureCube,TextureCube);

	VERIFYD3D9RESULT((*TextureCube)->UnlockRect( (D3DCUBEMAP_FACES) FaceIndex, MipIndex ));
}
