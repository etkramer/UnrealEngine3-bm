/*=============================================================================
	D3D10VertexBuffer.cpp: D3D texture RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D10DrvPrivate.h"

/*-----------------------------------------------------------------------------
	Texture allocator support.
-----------------------------------------------------------------------------*/

/**
 * Retrieves texture memory stats. Unsupported with this allocator.
 *
 * @return FALSE, indicating that out variables were left unchanged.
 */
UBOOL FD3D10DynamicRHI::GetTextureMemoryStats( INT& /*AllocatedMemorySize*/, INT& /*AvailableMemorySize*/ )
{
	return FALSE;
}

/**
 * Find the appropriate SRGB format for the input texture format.
 *
 * @InFormat The input format who's SRGB format we must find
 */
DXGI_FORMAT FindSRGBFormat(DXGI_FORMAT InFormat,UBOOL bSRGB)
{
	if(bSRGB)
	{
	    switch(InFormat)
	    {
	    case DXGI_FORMAT_R8G8B8A8_UNORM:
		    return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	    case DXGI_FORMAT_BC1_UNORM:
		    return DXGI_FORMAT_BC1_UNORM_SRGB;
	    case DXGI_FORMAT_BC2_UNORM:
		    return DXGI_FORMAT_BC2_UNORM_SRGB;
	    case DXGI_FORMAT_BC3_UNORM:
		    return DXGI_FORMAT_BC3_UNORM_SRGB;
	    };
	}
	return InFormat;
}

FD3D10Texture2D* FD3D10DynamicRHI::CreateD3D10Texture(UINT SizeX,UINT SizeY,UBOOL CubeTexture,BYTE Format,UINT NumMips,DWORD Flags)
{
	SCOPE_CYCLE_COUNTER(STAT_D3D10CreateTextureTime);

	DXGI_FORMAT PlatformFormat = FindSRGBFormat((DXGI_FORMAT)GPixelFormats[Format].PlatformFormat,Flags&TexCreate_SRGB);
	D3D10_TEXTURE2D_DESC TextureDesc;
	TextureDesc.Width = SizeX;
	TextureDesc.Height = SizeY;
	TextureDesc.MipLevels = NumMips;
	TextureDesc.ArraySize = CubeTexture ? 6 : 1;
	TextureDesc.Format = PlatformFormat;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Usage = D3D10_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = CubeTexture ? D3D10_RESOURCE_MISC_TEXTURECUBE : 0;

	if (Flags & TexCreate_ResolveTargetable)
	{
		TextureDesc.BindFlags |= D3D10_BIND_RENDER_TARGET; 
	}
	else if (Flags & TexCreate_DepthStencil)
	{
		// Overwrite the shader_resource binding flag
		TextureDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL; 
	}

	TRefCountPtr<ID3D10Texture2D> TextureResource;
	VERIFYD3D10CREATETEXTURERESULT(
		Direct3DDevice->CreateTexture2D(
			&TextureDesc,
			NULL,
			TextureResource.GetInitReference()),
		SizeX,
		SizeY,
		PlatformFormat,
		NumMips,
		TextureDesc.BindFlags
		);

	// Create a shader resource view for the texture.
	TRefCountPtr<ID3D10ShaderResourceView> TextureView;
	if ((TextureDesc.BindFlags & D3D10_BIND_RENDER_TARGET && !(Flags&TargetSurfCreate_Dedicated)) ||
		TextureDesc.BindFlags & D3D10_BIND_SHADER_RESOURCE)
	{
		D3D10_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		SRVDesc.Format = TextureDesc.Format;
		SRVDesc.ViewDimension = CubeTexture ? D3D10_SRV_DIMENSION_TEXTURECUBE : D3D10_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = NumMips;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		SRVDesc.Texture2DArray.MostDetailedMip = 0;
		SRVDesc.Texture2DArray.MipLevels = NumMips;
		SRVDesc.Texture2DArray.FirstArraySlice = 0;
		SRVDesc.Texture2DArray.ArraySize = TextureDesc.ArraySize;
		VERIFYD3D10RESULT(Direct3DDevice->CreateShaderResourceView(TextureResource,&SRVDesc,TextureView.GetInitReference()));
	}

	return new FD3D10Texture2D(this,TextureResource,TextureView,SizeX,SizeY,NumMips,(EPixelFormat)Format,CubeTexture);;
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
FTexture2DRHIRef FD3D10DynamicRHI::CreateTexture2D(UINT SizeX,UINT SizeY,BYTE Format,UINT NumMips,DWORD Flags,FResourceBulkDataInterface* BulkData)
{
	return CreateD3D10Texture(SizeX,SizeY,FALSE,Format,NumMips,Flags);
}

/**
 * Tries to reallocate the texture without relocation. Returns a new valid reference to the resource if successful.
 * Both the old and new reference refer to the same texture (at least the shared mip-levels) and both can be used or released independently.
 *
 * @param	Texture2D		- Texture to reallocate
 * @param	NewMipCount		- New number of mip-levels
 * @return					- New reference to the updated texture, or invalid if the reallocation failed
 */
FTexture2DRHIRef FD3D10DynamicRHI::ReallocateTexture2D( FTexture2DRHIParamRef Texture2D, INT NewMipCount )
{
	return FTexture2DRHIRef();
}

template<ERHIResourceTypes ResourceTypeEnum>
void* TD3D10Texture<ResourceTypeEnum>::Lock(UINT MipIndex,UINT ArrayIndex,UBOOL bIsDataBeingWrittenTo,UINT& DestStride)
{
	SCOPE_CYCLE_COUNTER(STAT_D3D10LockTextureTime);

	// Calculate the subresource index corresponding to the specified mip-map.
	const UINT Subresource = D3D10CalcSubresource(MipIndex,ArrayIndex,NumMips);

	// Calculate the dimensions of the mip-map.
	const UINT BlockSizeX = GPixelFormats[Format].BlockSizeX;
	const UINT BlockSizeY = GPixelFormats[Format].BlockSizeY;
	const UINT BlockBytes = GPixelFormats[Format].BlockBytes;
	const UINT MipSizeX = Max(SizeX >> MipIndex,BlockSizeX);
	const UINT MipSizeY = Max(SizeY >> MipIndex,BlockSizeY);
	const UINT NumBlocksX = (MipSizeX + BlockSizeX - 1) / BlockSizeX;
	const UINT NumBlocksY = (MipSizeY + BlockSizeY - 1) / BlockSizeY;
	const UINT MipBytes = NumBlocksX * NumBlocksY * BlockBytes;

	FD3D10LockedData LockedData;
	if( bIsDataBeingWrittenTo )
	{
		// If we're writing to the texture, allocate a system memory buffer to receive the new contents.
		LockedData.Data = new BYTE[ MipBytes ];
		LockedData.Pitch = DestStride = NumBlocksX * BlockBytes;
	}
	else
	{
		// If we're reading from the texture, we create a staging resource, copy the texture contents to it, and map it.

		// Create the staging texture.
		D3D10_TEXTURE2D_DESC StagingTextureDesc;
		Resource->GetDesc(&StagingTextureDesc);
		StagingTextureDesc.Width = MipSizeX;
		StagingTextureDesc.Height = MipSizeY;
		StagingTextureDesc.MipLevels = 1;
		StagingTextureDesc.ArraySize = 1;
		StagingTextureDesc.Usage = D3D10_USAGE_STAGING;
		StagingTextureDesc.BindFlags = 0;
		StagingTextureDesc.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
		TRefCountPtr<ID3D10Texture2D> StagingTexture;
		VERIFYD3D10CREATETEXTURERESULT(
			D3DRHI->GetDevice()->CreateTexture2D(&StagingTextureDesc,NULL,StagingTexture.GetInitReference()),
			SizeX,
			SizeY,
			StagingTextureDesc.Format,
			1,
			0
			);
		LockedData.StagingResource = StagingTexture;

		// Copy the mip-map data from the real resource into the staging resource
		D3DRHI->GetDevice()->CopySubresourceRegion(StagingTexture,0,0,0,0,Resource,Subresource,NULL);

		// Map the staging resource, and return the mapped address.
		D3D10_MAPPED_TEXTURE2D MappedTexture;
		VERIFYD3D10RESULT(StagingTexture->Map(0,D3D10_MAP_READ,0,&MappedTexture));
		LockedData.Data = (BYTE*)MappedTexture.pData;
		LockedData.Pitch = DestStride = MappedTexture.RowPitch;
	}

	// Add the lock to the outstanding lock list.
	D3DRHI->OutstandingLocks.Set(FD3D10LockedKey(Resource,Subresource),LockedData);

	return (void*)LockedData.Data;
}

template<ERHIResourceTypes ResourceTypeEnum>
void TD3D10Texture<ResourceTypeEnum>::Unlock(UINT MipIndex,UINT ArrayIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_D3D10UnlockTextureTime);

	// Calculate the subresource index corresponding to the specified mip-map.
	const UINT Subresource = D3D10CalcSubresource(MipIndex,ArrayIndex,NumMips);

	// Find the object that is tracking this lock
	const FD3D10LockedKey LockedKey(Resource,Subresource);
	const FD3D10LockedData* LockedData = D3DRHI->OutstandingLocks.Find(LockedKey);
	check(LockedData);

	if(!LockedData->StagingResource)
	{
		// If we're writing, we need to update the subresource
		D3DRHI->GetDevice()->UpdateSubresource(Resource,Subresource,NULL,LockedData->Data,LockedData->Pitch,0);
		delete[] LockedData->Data;
	}

	// Remove the lock from the outstanding lock list.
	D3DRHI->OutstandingLocks.Remove(LockedKey);
}

void* FD3D10DynamicRHI::LockTexture2D(FTexture2DRHIParamRef TextureRHI,UINT MipIndex,UBOOL bIsDataBeingWrittenTo,UINT& DestStride,UBOOL bLockWithinMiptail)
{
	DYNAMIC_CAST_D3D10RESOURCE(Texture2D,Texture);
	return Texture->Lock(MipIndex,0,bIsDataBeingWrittenTo,DestStride);
}

void FD3D10DynamicRHI::UnlockTexture2D(FTexture2DRHIParamRef TextureRHI,UINT MipIndex,UBOOL bLockWithinMiptail)
{
	DYNAMIC_CAST_D3D10RESOURCE(Texture2D,Texture);
	Texture->Unlock(MipIndex,0);
}

/**
 * Checks if a texture is still in use by the GPU.
 * @param Texture - the RHI texture resource to check
 * @param MipIndex - Which mipmap we're interested in
 * @return TRUE if the texture is still in use by the GPU, otherwise FALSE
 */
UBOOL FD3D10DynamicRHI::IsBusyTexture2D(FTexture2DRHIParamRef Texture,UINT MipIndex)
{
	//@TODO: Implement somehow! (Perhaps with D3D10_MAP_FLAG_DO_NOT_WAIT)
	return FALSE;
}

INT FD3D10DynamicRHI::GetMipTailIdx(FTexture2DRHIParamRef Texture)
{
	return -1;
}

void FD3D10DynamicRHI::CopyTexture2D(FTexture2DRHIParamRef DestTextureRHI, UINT MipIdx, INT BaseSizeX, INT BaseSizeY, INT Format, const TArray<FCopyTextureRegion2D>& Regions)
{
	SCOPE_CYCLE_COUNTER(STAT_D3D10CopyTextureTime);

	DYNAMIC_CAST_D3D10RESOURCE(Texture2D,DestTexture);
	check( DestTexture );

	// scale the base SizeX,SizeY for the current mip level
	const INT MipSizeX = Max(BaseSizeX >> MipIdx,(INT)GPixelFormats[Format].BlockSizeX);
	const INT MipSizeY = Max(BaseSizeY >> MipIdx,(INT)GPixelFormats[Format].BlockSizeY);

	for( INT RegionIdx=0; RegionIdx < Regions.Num(); RegionIdx++ )		
	{
		const FCopyTextureRegion2D& Region = Regions(RegionIdx);
		FTexture2DRHIParamRef SourceTextureRHI = Region.SrcTexture;
		DYNAMIC_CAST_D3D10RESOURCE(Texture2D,SourceTexture);

		// align/truncate the region offset to block size
		const UINT RegionOffsetX = (UINT)(Clamp( Region.OffsetX, 0, MipSizeX - GPixelFormats[Format].BlockSizeX ) / GPixelFormats[Format].BlockSizeX) * GPixelFormats[Format].BlockSizeX;
		const UINT RegionOffsetY = (UINT)(Clamp( Region.OffsetY, 0, MipSizeY - GPixelFormats[Format].BlockSizeY ) / GPixelFormats[Format].BlockSizeY) * GPixelFormats[Format].BlockSizeY;
		// scale region size to the current mip level. Size is aligned to the block size
		check(Region.SizeX != 0 && Region.SizeY != 0);
		UINT RegionSizeX = (UINT)Clamp( Align( Region.SizeX, GPixelFormats[Format].BlockSizeX), 0, MipSizeX );
		UINT RegionSizeY = (UINT)Clamp( Align( Region.SizeY, GPixelFormats[Format].BlockSizeY), 0, MipSizeY );
		// handle special case for full copy
		if( Region.SizeX == -1 || Region.SizeY == -1 )
		{
			RegionSizeX = MipSizeX;
			RegionSizeY = MipSizeY;
		}

		// Set up a box for the copy region.
		D3D10_BOX CopyBox;
		CopyBox.left = RegionOffsetX;
		CopyBox.top = RegionOffsetY;
		CopyBox.right = RegionOffsetX + RegionSizeX;
		CopyBox.bottom = RegionOffsetY + RegionSizeY;
		CopyBox.front = 0;
		CopyBox.back = 1;

		// Use the GPU to copy the texture region.
		Direct3DDevice->CopySubresourceRegion(
			DestTexture->Resource,
			D3D10CalcSubresource(MipIdx,0,1),
			RegionOffsetX,
			RegionOffsetY,
			0,
			SourceTexture->Resource,
			D3D10CalcSubresource(Region.FirstMipIdx + MipIdx,0,1),
			&CopyBox
			);
	}
}

void FD3D10DynamicRHI::CopyMipToMipAsync(FTexture2DRHIParamRef SrcTextureRHI, INT SrcMipIndex, FTexture2DRHIParamRef DestTextureRHI, INT DestMipIndex, INT Size, FThreadSafeCounter& Counter)
{
	SCOPE_CYCLE_COUNTER(STAT_D3D10CopyMipToMipAsyncTime);

	DYNAMIC_CAST_D3D10RESOURCE(Texture2D,SrcTexture);
	DYNAMIC_CAST_D3D10RESOURCE(Texture2D,DestTexture);

	// Use the GPU to copy between mip-maps.
	// This is serialized with other D3D commands, so it isn't necessary to increment Counter to signal a pending asynchronous copy.
	Direct3DDevice->CopySubresourceRegion(
		DestTexture->Resource,
		D3D10CalcSubresource(DestMipIndex,0,DestTexture->NumMips),
		0,
		0,
		0,
		SrcTexture->Resource,
		D3D10CalcSubresource(SrcMipIndex,0,SrcTexture->NumMips),
		NULL
		);
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
void FD3D10DynamicRHI::SelectiveCopyMipData(FTexture2DRHIParamRef Texture, BYTE *Src, BYTE *Dst, UINT MemSize, UINT MipIdx)
{
	appMemcpy(Dst, Src, MemSize);
}

void FD3D10DynamicRHI::FinalizeAsyncMipCopy(FTexture2DRHIParamRef SrcTextureRHI, INT SrcMipIndex, FTexture2DRHIParamRef DestTextureRHI, INT DestMipIndex)
{
	// We don't need to do any work here, because the asynchronous mip copy is performed by the GPU and serialized with other D3D commands.
}

/*-----------------------------------------------------------------------------
Shared texture support.
-----------------------------------------------------------------------------*/
FSharedMemoryResourceRHIRef FD3D10DynamicRHI::CreateSharedMemory(SIZE_T Size)
{
	// create the shared memory resource
	FSharedMemoryResourceRHIRef SharedMemory(NULL);
	return SharedMemory;
}
FSharedTexture2DRHIRef FD3D10DynamicRHI::CreateSharedTexture2D(UINT SizeX,UINT SizeY,BYTE Format,UINT NumMips,FSharedMemoryResourceRHIParamRef SharedMemoryRHI,DWORD Flags)
{
	DYNAMIC_CAST_D3D10RESOURCE(SharedMemoryResource,SharedMemory);
	return FSharedTexture2DRHIRef();
}

/*-----------------------------------------------------------------------------
	Cubemap texture support.
-----------------------------------------------------------------------------*/
FTextureCubeRHIRef FD3D10DynamicRHI::CreateTextureCube( UINT Size, BYTE Format, UINT NumMips, DWORD Flags, FResourceBulkDataInterface* BulkData )
{
	return (FD3D10TextureCube*)CreateD3D10Texture(Size,Size,TRUE,Format,NumMips,Flags);
}
void* FD3D10DynamicRHI::LockTextureCubeFace(FTextureCubeRHIParamRef TextureCubeRHI,UINT FaceIndex,UINT MipIndex,UBOOL bIsDataBeingWrittenTo,UINT& DestStride,UBOOL bLockWithinMiptail)
{
	DYNAMIC_CAST_D3D10RESOURCE(TextureCube,TextureCube);
	return TextureCube->Lock(MipIndex,FaceIndex,bIsDataBeingWrittenTo,DestStride);
}
void FD3D10DynamicRHI::UnlockTextureCubeFace(FTextureCubeRHIParamRef TextureCubeRHI,UINT FaceIndex,UINT MipIndex,UBOOL bLockWithinMiptail)
{
	DYNAMIC_CAST_D3D10RESOURCE(TextureCube,TextureCube);
	TextureCube->Unlock(MipIndex,FaceIndex);
}
