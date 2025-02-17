/*=============================================================================
	D3DVertexBuffer.cpp: D3D texture RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "XeD3DDrvPrivate.h"

#if USE_XeD3D_RHI

// definitions for texture mip memory sharing
#define XEMIPMEMSHARING_NUM_LAYOUT_MIPS						3
#define XEMIPMEMSHARING_MAX_TEXTURE_DIMENSION				128
#define XEMIPMEMSHARING_FIRST_SHARED_MIP_DIMENSION			64
#define XEMIPMEMSHARING_SHARED_MIPS_COUNT					7	// appLog2(XEMIPMEMSHARING_FIRST_SHARED_MIP_TEXTURE_DIMENSION) + 1

/** Whether we allow growing in-place reallocations. */
UBOOL GEnableGrowingReallocations = TRUE;

/** Whether we allow shrinking in-place reallocations. */
UBOOL GEnableShrinkingReallocations = TRUE;

/*-----------------------------------------------------------------------------
 DXT Mip Memory layouts for tiled textures

 https://xds.xbox.com/xbox360/nav.aspx?page=/1033/xdksoftware/whitepapers/xbox_360_texture_storage.doc
 Refer to Figure 7 and Figure 8 for layout of DXT1 and DXT5 tiled textures
-----------------------------------------------------------------------------*/
XGLAYOUT_REGION GXeMipLayoutDXT1_64x64[] =
	{	
		{0,128},	{256,384},	{512,640},	{768, 896},
		{1024,1152},{1280,1408},{1536,1664},{1792,1920},
		{4224,4352},{4480,4608},{4736,4864},{4992,5120},
		{5248,5376},{5504,5632},{5760,5888},{6016,6144},
		{0xFFFFFFFF,0xFFFFFFFF}
	};
XGLAYOUT_REGION GXeMipLayoutDXT1_32x32[] =
	{	
		{0,64},		{256,320},	{512,576},	{768, 832},
		{1024,1088},{1280,1344},{1536,1600},{1792,1856},
		{0xFFFFFFFF,0xFFFFFFFF}
	};
XGLAYOUT_REGION GXeMipLayoutDXT1_Tail[] =
	{
		{0,64},		{256,320},	{512,576},	{768, 832},
		{0xFFFFFFFF,0xFFFFFFFF}
	};
XGLAYOUT_REGION GXeMipLayoutDXT5_64x64[] =
	{	
		{0,128},	{256,384},	{512,640},	{768, 896},
		{1024,1152},{1280,1408},{1536,1664},{1792,1920},
		{4096,4224},{4352,4480},{4608,4736},{4864,4992},
		{5120,5248},{5376,5504},{5632,5760},{5888,6016},
		{8320,8448},{8576,8704},{8832,8960},{9088,9216},
		{9344,9472},{9600,9728},{9856,9984},{10112,10240},
		{12416,12544},{12672,12800},{12928,13056},{13184,13312},
		{13440,13568},{13696,13824},{13952,14080},{14208,14336},
		{0xFFFFFFFF,0xFFFFFFFF}
	};
XGLAYOUT_REGION GXeMipLayoutDXT5_32x32[] =
	{	
		{0,64},		{256,320},	{512,576},	{768, 832},
		{1024,1088},{1280,1344},{1536,1600},{1792,1856},
		{4096,4160},{4352,4416},{4608,4672},{4864,4928},
		{5120,5184},{5376,5440},{5632,5696},{5888,5952},
		{0xFFFFFFFF,0xFFFFFFFF}
	};
XGLAYOUT_REGION GXeMipLayoutDXT5_Tail[] =
	{	
		{0,64},		{256,320},	{512,576},	{768, 832},
		{1024,1088},{1280,1344},{1536,1600},{1792,1856},
		{0xFFFFFFFF,0xFFFFFFFF}
	};

// DXT1 and DXT5 mip layout arrays (indexed by bottum-up mip index)
XGLAYOUT_REGION *GXeDXT1MipMemLayouts[XEMIPMEMSHARING_NUM_LAYOUT_MIPS] =
		{GXeMipLayoutDXT1_Tail, GXeMipLayoutDXT1_32x32, GXeMipLayoutDXT1_64x64};
XGLAYOUT_REGION *GXeDXT5MipMemLayouts[XEMIPMEMSHARING_NUM_LAYOUT_MIPS] =
		{GXeMipLayoutDXT5_Tail, GXeMipLayoutDXT5_32x32, GXeMipLayoutDXT5_64x64};


/** 
* @param	Format - unreal format type
* @param	Flags - creation flags
* @return	D3D texture format for the unreal texture format 
*/
D3DFORMAT XeGetTextureFormat(BYTE Format,DWORD Flags)
{
	// convert to platform specific format
	D3DFORMAT D3DFormat = (D3DFORMAT)GPixelFormats[Format].PlatformFormat;
	// sRGB is handled as a surface format on Xe
	if( Flags&TexCreate_SRGB )
	{
		D3DFormat = (D3DFORMAT)MAKESRGBFMT(D3DFormat);
	}	
	// handle un-tiled formats
	if( (Flags&TexCreate_NoTiling) || (Flags & TexCreate_Uncooked) )
	{
		D3DFormat = (D3DFORMAT)MAKELINFMT(D3DFormat);
	}
	return D3DFormat;
}

/** 
 * Returns flags if the texture params meet the requirements to be a mip memory donor or recipient 
 *
 * @param	SizeX - width of texture
 * @param	SizeY - height of texture
 * @param	Format - format of texture
 * @param	NumMips - number of mips
 * @param	Flags - creation flags
 * @return	Returns what kind of memory sharing is possible, or TEXTURESHARING_None if no sharing is possible
 */
inline DWORD XeGetTexture2DSharingFlags(UINT SizeX,UINT SizeY,BYTE Format,UINT NumMips,DWORD Flags)
{
	if( !GTexturesCanShareMipMemory || 
		(appRoundUpToPowerOfTwo(SizeX) != appRoundUpToPowerOfTwo(SizeY)) ||
		((Flags&TexCreate_NoMipTail) != 0) )
	{
		return TEXTURESHARING_None;
	}
	DWORD TextureSharingFlags = TEXTURESHARING_None;
	D3DFORMAT D3dFormat = XeGetTextureFormat(Format,Flags);
	switch(D3dFormat)
	{
		case D3DFMT_DXT1:
			TextureSharingFlags |= TEXTURESHARING_DXT1;
			break;
		case D3DFMT_DXT3:
		case D3DFMT_DXT5:
			TextureSharingFlags |= TEXTURESHARING_DXT3or5;
			break;
		default:
			return TEXTURESHARING_None;
	}
	if( (NumMips >= XEMIPMEMSHARING_SHARED_MIPS_COUNT) &&
		(SizeX >= XEMIPMEMSHARING_FIRST_SHARED_MIP_DIMENSION) && 
		(SizeY >= XEMIPMEMSHARING_FIRST_SHARED_MIP_DIMENSION) )
	{
		TextureSharingFlags |= TEXTURESHARING_Donor;
	}
	if( (SizeX <= XEMIPMEMSHARING_MAX_TEXTURE_DIMENSION) && 
		(SizeY <= XEMIPMEMSHARING_MAX_TEXTURE_DIMENSION) )
	{
		TextureSharingFlags |= TEXTURESHARING_Recipient;
	}
	return TextureSharingFlags;
}

/**
 *	Calculates the offset to the specified mip-level, in bytes from the baseaddress.
 *	@param D3DTexture	- D3D9 texture struct
 *	@param MipIndex		- Mip-level to calculate the offset for
 *	@return				- Mip-level offset, in bytes from the baseaddress
 */
INT XeGetMipOffset( IDirect3DTexture9* D3DTexture, INT MipIndex )
{
	UINT BaseAddress = 0;
	UINT MipAddress = 0;
	XGGetTextureLayout(D3DTexture,&BaseAddress,NULL,NULL,NULL,0,&MipAddress,NULL,NULL,NULL,0);
	INT MipChainOffset = INT(MipAddress - BaseAddress);

	XGTEXTURE_DESC Desc;
	XGGetTextureDesc(D3DTexture, 0, &Desc);
	INT MipOffset = XGGetMipLevelOffset(D3DTexture, 0, MipIndex);
	DWORD MipTailIndex = XGGetMipTailBaseLevel(Desc.Width, Desc.Height, FALSE);
	UBOOL bHasPackedMipTail = XGIsPackedTexture(D3DTexture);

	// Is this the base level?
	if ( MipIndex == 0 || MipAddress == 0 )
	{
		// Is the base level a packed mip-map tail?
		if ( bHasPackedMipTail && MipIndex == MipTailIndex )
		{
			MipOffset = 0;
		}
	}
	// Is this a packed mip tail?
	else if ( bHasPackedMipTail && MipIndex == MipTailIndex )
	{
		// Get the address of the previous mip-level.
		INT PrevMipOffset;
		if ( MipIndex == 1 )
		{
			PrevMipOffset = 0;
		}
		else
		{
			PrevMipOffset = MipChainOffset + XGGetMipLevelOffset(D3DTexture, 0, MipIndex - 1);
		}

		// Add its mip-size to reach the requested mip-level.
		XGTEXTURE_DESC Desc;
		XGGetTextureDesc(D3DTexture, MipIndex - 1, &Desc);
		MipOffset = PrevMipOffset + Desc.SlicePitch;
	}
	else
	{
		MipOffset += MipChainOffset;
	}

	return MipOffset;
}


/*-----------------------------------------------------------------------------
	FXeTextureBase
-----------------------------------------------------------------------------*/

/** Destructor */
FXeTextureBase::~FXeTextureBase()
{
	XeUnregisterTexture( this );

	// Check if we're using in-place reallocation (only for 2D textures).
	if ( ReallocatedTexture )
	{
		IDirect3DTexture9* MyD3DTexture = (IDirect3DTexture9*) Resource;
		IDirect3DTexture9* OtherD3DTextures = (IDirect3DTexture9*) ReallocatedTexture->Resource;
		INT MyMipCount = MyD3DTexture->GetLevelCount();
		INT OtherMipCount = OtherD3DTextures->GetLevelCount();

		// Am I the larger version?
		if ( MyMipCount > OtherMipCount )
		{
			// Defer the shrinking of the allocation.
			AddUnusedXeResource( Resource, BaseAddress, TRUE, FALSE, FALSE, ReallocatedTexture->BaseAddress );
		}

		// The other resource is now solely responsible for the remaining texture memory.
		ReallocatedTexture->SetReallocatedTexture( NULL );
		BaseAddress = NULL;
		Resource = NULL;
	}
}

/**
 *	Notifies that the texture data is being reallocated and is shared for a brief moment,
 *	so that this texture can do the right thing upon destruction.
 *
 *	@param InReallocatedTexture	- The other texture that briefly shares the texture data when it's reallocated
 **/
void FXeTextureBase::SetReallocatedTexture( FXeTextureBase* InReallocatedTexture )
{
	ReallocatedTexture = InReallocatedTexture;
}

/** Checks whether this texture can be relocated or not by the defragmentation process. */
UBOOL FXeTextureBase::CanRelocate()
{
	return (IsLocked() == FALSE) && (ReallocatedTexture == NULL);
}


/*-----------------------------------------------------------------------------
	2D texture support.
-----------------------------------------------------------------------------*/

/** 
 * Sets D3D header and allocates memory for a D3D resource 
 *
 * @param SizeX			- width of texture
 * @param SizeY			- height of texture
 * @param Format		- format of texture
 * @param NumMips		- number of mips
 * @param Flags			- creation flags
 * @param Texture		- [in/out optional] IDirect3DTexture9 header to initialize
 * @param OutMemorySize	- [out] Memory size of the allocation in bytes
 * @param BulkData		- [optional] pre-allocated memory containing the bulk data
 * @return				- Base address of the allocation
 */
void* FXeTexture2D::AllocTexture2D(UINT SizeX,UINT SizeY,BYTE Format,UINT NumMips,DWORD Flags,IDirect3DTexture9* Texture,DWORD& OutMemorySize,FResourceBulkDataInterface* BulkData)
{
	// set the texture header
	DWORD TextureSize = XGSetTextureHeaderEx( 
		SizeX,
		SizeY,
		NumMips, 
		0,
		XeGetTextureFormat(Format,Flags),
		0,
		(Flags&TexCreate_NoMipTail) ? XGHEADEREX_NONPACKED : 0,
		0,
		XGHEADER_CONTIGUOUS_MIP_OFFSET,
		0,
		Texture,
		NULL,
		NULL
		); 
    OutMemorySize = Align(TextureSize,D3DTEXTURE_ALIGNMENT);
	void* BaseAddress;
	if ( BulkData )
	{
		checkf(OutMemorySize == BulkData->GetResourceBulkDataSize(), TEXT("Resource bulkdata size mismatch Size=%d BulkDataSize=%d"),OutMemorySize,BulkData->GetResourceBulkDataSize());
		// get preallocated memory for this resource and hold onto it 
		BaseAddress = BulkData->GetResourceBulkData();
		// discard will just clear data pointer since it will be freed by when this resource is released 
		BulkData->Discard();
	}
	else
	{
		BaseAddress = XeAllocateTextureMemory( OutMemorySize, (Flags & TexCreate_AllowFailure) ? TRUE : FALSE, (Flags & TexCreate_DisableAutoDefrag) ? TRUE : FALSE );
	}

	// set data address for this resource
	XGOffsetResourceAddress( Texture, BaseAddress );

	return BaseAddress;
}

/** 
 * Tries to add or remove mip-levels of the texture without relocation.
 *
 * @param Texture2D		- Source texture
 * @param NewMipCount	- Number of desired mip-levels
 * @return				- A newly allocated FXeTexture2D if successful, or NULL if it failed.
 */
FXeTexture2D* FXeTexture2D::ReallocateTexture2D(FXeTexture2D* Texture2D, INT NewMipCount)
{
	IDirect3DTexture9* OldD3DTexture = (IDirect3DTexture9*) Texture2D->Resource;
	XGTEXTURE_DESC Desc;
	XGGetTextureDesc(OldD3DTexture, 0, &Desc);
	DWORD TextureFlags = XGIsPackedTexture(OldD3DTexture) ? 0 : XGHEADEREX_NONPACKED;
	INT OldMipCount = OldD3DTexture->GetLevelCount();
	void* NewBaseAddress = NULL;
	INT NewSizeX = 0;
	INT NewSizeY = 0;
	INT MipCountDiff = Abs<INT>(NewMipCount - OldMipCount);

	// Are we shrinking?
	if ( NewMipCount < OldMipCount )
	{
		if ( GEnableShrinkingReallocations )
		{
			INT MipOffset = XeGetMipOffset( OldD3DTexture, MipCountDiff );

			// MipOffset should always be 4K aligned. If it's not, then XeGetMipOffset() is buggy and we can't shrink.
			if ( MipOffset == Align(MipOffset, D3DTEXTURE_ALIGNMENT) )
			{
				UINT BlockSizeX = 1;
				UINT BlockSizeY = 1;
				XGGetBlockDimensions( XGGetGpuFormat(Desc.Format), &BlockSizeX, &BlockSizeY );
				NewSizeX = Max<INT>(Desc.Width >> MipCountDiff, BlockSizeX);
				NewSizeY = Max<INT>(Desc.Height >> MipCountDiff, BlockSizeY);

				// We're shrinking. Move the baseaddress up.
				NewBaseAddress = (BYTE*)Texture2D->BaseAddress + MipOffset;
			}
			else
			{
				INT BreakPointHere = 0;
			}
		}
	}
	else
	{
		if ( GEnableGrowingReallocations )
		{
			NewSizeX = Desc.Width << MipCountDiff;
			NewSizeY = Desc.Height << MipCountDiff;

			// Setup a dummy header that would represent the new texture.
			IDirect3DTexture9 DummyD3DTexture;
			XGSetTextureHeaderEx( NewSizeX, NewSizeY, NewMipCount, 0, Desc.Format, 0, TextureFlags, 0, XGHEADER_CONTIGUOUS_MIP_OFFSET, 0, &DummyD3DTexture, NULL, NULL ); 

			INT MipOffset = XeGetMipOffset( &DummyD3DTexture, MipCountDiff );

			// MipOffset should always be 4K aligned. If it's not, then XeGetMipOffset() is buggy and we can't shrink.
			if ( MipOffset > 0 && MipOffset == Align(MipOffset, D3DTEXTURE_ALIGNMENT) )
			{
				// We're growing the texture. Move the baseaddress down to encompass the new mip-levels.
				NewBaseAddress = (BYTE*)Texture2D->BaseAddress - MipOffset;

				// Try to grow the texture in-place.
				UBOOL bSuccess = XeReallocateTextureMemory( Texture2D->BaseAddress, NewBaseAddress );

				// Did it fail?
				if ( !bSuccess )
				{
					NewBaseAddress = NULL;
				}
			}
			else
			{
				INT BreakPointHere = 0;
			}
		}
	}

	// Did it succeed?
	if ( NewBaseAddress )
	{
		// Create and setup a new D3D struct.
		IDirect3DTexture9* NewD3DTexture = new IDirect3DTexture9;
		DWORD TextureSize = XGSetTextureHeaderEx( NewSizeX, NewSizeY, NewMipCount, 0, Desc.Format, 0, TextureFlags, 0, XGHEADER_CONTIGUOUS_MIP_OFFSET, 0, NewD3DTexture, NULL, NULL ); 
		XGOffsetResourceAddress( NewD3DTexture, NewBaseAddress );

		// Create a 2D texture resource.
		FXeTexture2D* NewTexture = new FXeTexture2D( NewD3DTexture, Texture2D->GetCreationFlags(), Texture2D->GetSharingFlags() );
		NewTexture->BaseAddress = NewBaseAddress;

#if TRACK_GPU_RESOURCES
	    TextureSize = Align(TextureSize,D3DTEXTURE_ALIGNMENT);
		NewTexture->PhysicalSize = TextureSize;
		NewTexture->VirtualSize = sizeof(IDirect3DTexture9);
#endif

		// Let the old and new resources know about the reallocation, so they know what to do with the memory upon destruction.
		Texture2D->SetReallocatedTexture( NewTexture );
		NewTexture->SetReallocatedTexture( Texture2D );

		// Return the new resource.
		return NewTexture;
	}

	return NULL;
}

/** 
 * Sets D3D header to use memory holes from a given donor texture 
 * The recipient texture and its donor must pass requirements of XeGetTexture2DSharingFlags()
 *
 * @param	SizeX - width of texture
 * @param	SizeY - height of texture
 * @param	Format - format of texture
 * @param	NumMips - number of mips
 * @param	Flags - creation flags
 * @return	TRUE if memory was re-used, otherwise FALSE
 */
UBOOL FXeTexture2D::PairTexture2D(UINT SizeX, UINT SizeY, BYTE Format, UINT NumMips, DWORD Flags)
{
	FXeTextureBase* DonorTexture = NULL;
	do
	{
		if((DonorTexture = XePopDonorTexture(GetSharingFlags()&TEXTURESHARING_TypeMask)) == NULL)
		{
			return FALSE;
		}
	}
	while(!DonorTexture->IsDonor());

	DonorTexture->Lock(0);

	IDirect3DTexture9* Texture = (IDirect3DTexture9*) Resource;

	// get the donor texture params
	IDirect3DTexture9* Donor = (IDirect3DTexture9*) DonorTexture->Resource;
	D3DSURFACE_DESC Desc;
	Donor->GetLevelDesc(0, &Desc);
	DWORD DonorMips = Donor->GetLevelCount();
	UINT MipOffset = (SizeX > XEMIPMEMSHARING_FIRST_SHARED_MIP_DIMENSION || 
		SizeY > XEMIPMEMSHARING_FIRST_SHARED_MIP_DIMENSION) ? 0 :
		XGHEADER_CONTIGUOUS_MIP_OFFSET;

	UINT BaseSize = 0;
	UINT MipSize = 0;

	// setup the recipient's texture header based on its donor
	XGSetTextureHeaderPair(
		  0,
		  MipOffset,
		  Desc.Width,
		  Desc.Height,
		  DonorMips,
		  0,
		  Desc.Format,
		  0,
		  0, // note that XGHEADEREX_NONPACKED will neve be set for paired textures
		  0,
		  NULL,
		  SizeX,
		  SizeY,
		  NumMips,
		  0,
		  XeGetTextureFormat(Format,Flags),
		  0,
		  0, // note that XGHEADEREX_NONPACKED will neve be set for paired textures
		  0,
		  Texture,
		  &BaseSize,
		  &MipSize
		  );

	// same base address, but it will be offset
	BaseAddress = DonorTexture->BaseAddress;

	// can no longer be a donor since its base address will be offset
	SetDonor(FALSE);

	if(MipOffset == XGHEADER_CONTIGUOUS_MIP_OFFSET)
	{
		// the mip data is contiguous with the base texture data
		// base texture address is offset from the donor texture (this is usually 4k(DXT1) or 8k(DXT3/5))
		XGOffsetResourceAddress( Texture, BaseAddress );
	}
	else 
	{
		// seperate memory for the top-level texture data and the mip data
		SecondaryBaseAddress = XeAllocateTextureMemory( BaseSize, (Flags & TexCreate_AllowFailure) ? TRUE : FALSE, (Flags & TexCreate_DisableAutoDefrag) ? TRUE : FALSE );
		// mip address is offset from the donor texture (this is usually 4k(DXT1) or 8k(DXT3/5))
		XGOffsetBaseTextureAddress(Texture, SecondaryBaseAddress, BaseAddress);
	}
	DonorTexture->bIsShared = TRUE;
	bIsShared = TRUE;
	DonorTexture->Unlock(0);
	// succeeded
	return TRUE;
}

/** 
 * Writes out the actual memory layout of a textures bottom mip levels to a .bmp 
 * it is assumed this texture is using a packed mip tail
 *
 * @param	Texture - the texture to write out
 * @param	FileName - name of file (.bmp extension will be added)
 */
void XeWriteOutTextureMipTail(FTexture2DRHIRef Texture, TCHAR *FileName)
{
	int MipIdx = RHIGetMipTailIdx(Texture) - 2;

	if(MipIdx >= 0)
	{
		UINT DestStride = 0;
		VOID *BaseAddress = RHILockTexture2D(Texture,MipIdx,FALSE,DestStride,FALSE);

		IDirect3DTexture9* ResourceTexture = (IDirect3DTexture9*)Texture->Resource;
		D3DSURFACE_DESC Desc;
		ResourceTexture->GetLevelDesc(0, &Desc);

		IDirect3DTexture9 *TempD3DTexture = new IDirect3DTexture9();
		XGSetTextureHeaderEx(128,384,1,0,(D3DFORMAT)(MAKELINFMT(Desc.Format)),0,0,0,0,0,TempD3DTexture,NULL,NULL);
		XGOffsetResourceAddress( TempD3DTexture, BaseAddress );

		FString UniqueFilePath = FString("GAME:") * FileName + TEXT(".bmp");

		D3DXSaveTextureToFileA(TCHAR_TO_ANSI(*UniqueFilePath), D3DXIFF_BMP, TempD3DTexture, NULL);

		delete TempD3DTexture;
		RHIUnlockTexture2D(Texture,MipIdx,FALSE);

	}
}

/**
 * Creates a 2D RHI texture resource
 * @param SizeX		- width of the texture to create
 * @param SizeY		- height of the texture to create
 * @param Format	- EPixelFormat texture format
 * @param NumMips	- number of mips to generate or 0 for full mip pyramid
 * @param Flags		- ETextureCreateFlags creation flags
 */
FTexture2DRHIRef RHICreateTexture2D(UINT SizeX,UINT SizeY,BYTE Format,UINT NumMips,DWORD Flags,FResourceBulkDataInterface* BulkData)
{
	FXeTexture2D* XeTexture = NULL;

	if( (Flags&TexCreate_ResolveTargetable) || (Flags & TexCreate_Uncooked) )
	{
		check(!BulkData);
		check( !((Flags&TexCreate_ResolveTargetable) && NumMips > 1) );

		IDirect3DTexture9* pD3DTexture2D;
		// create texture in memory for resolving surface
		HRESULT D3DResult = GDirect3DDevice->CreateTexture( 
			SizeX, 
			SizeY, 
			NumMips, 
			0, 
			XeGetTextureFormat(Format,Flags), 
			D3DPOOL_DEFAULT, 
			&pD3DTexture2D, 
			NULL );

		if ( Flags & TexCreate_AllowFailure )
		{
			XeTexture = (D3DResult == S_OK) ? new FXeTexture2D(pD3DTexture2D,Flags) : NULL;
		}
		else
		{
			VERIFYD3DRESULT( D3DResult );
			XeTexture = new FXeTexture2D(pD3DTexture2D,Flags);
		}
#if TRACK_GPU_RESOURCES
		if (XeTexture)
		{
			XeTexture->VirtualSize = sizeof(D3DTexture);
			IDirect3DTexture9 DummyTexture;
			// Create a dummy texture we can query the physical size from
			XeTexture->PhysicalSize = XGSetTextureHeaderEx( 
				SizeX,							// Width
				SizeY,							// Height
				NumMips,						// Levels
				0,								// Usage
				XeGetTextureFormat(Format,Flags),		// Format
				0,								// ExpBias
				0,								// Flags
				0,								// BaseOffset
				XGHEADER_CONTIGUOUS_MIP_OFFSET,	// MipOffset
				0,								// Pitch
				&DummyTexture,					// D3D texture
				NULL,							// unused
				NULL							// unused
				);
		}
#endif
	}
	else
	{
		DWORD ShareFlags = XeGetTexture2DSharingFlags(SizeX,SizeY,Format,NumMips,Flags);

		// try to pair this texture with an existing donor texture's memory, unless the memory is already allocated in BulkData
		if (!BulkData && (ShareFlags & TEXTURESHARING_Recipient))
		{
			XeTexture = new FXeTexture2D(Flags,ShareFlags);
			if (XeTexture->PairTexture2D(SizeX,SizeY,Format,NumMips,Flags))
			{
				return FTexture2DRHIRef(XeTexture);
			}
		}

		// create the 2d texture resource
		IDirect3DTexture9* D3DResource = new IDirect3DTexture9;
		DWORD MemorySize = 0;
		void* BaseAddress = FXeTexture2D::AllocTexture2D(SizeX, SizeY, Format, NumMips, Flags, D3DResource, MemorySize, BulkData);
		if ( XeIsValidTextureData(BaseAddress) )
		{
			XeTexture = new FXeTexture2D(D3DResource, Flags, ShareFlags);
			XeTexture->BaseAddress = BaseAddress;
#if TRACK_GPU_RESOURCES
			XeTexture->PhysicalSize = MemorySize;
			XeTexture->VirtualSize = sizeof(IDirect3DTexture9);
#endif
			if (XeTexture->GetSharingFlags() & TEXTURESHARING_Donor)
			{
				// add to donor texture stack
				XePushDonorTexture(XeTexture);
			}
		}
		else
		{
			delete D3DResource;
			XeTexture = NULL;
		}
	}
	return FTexture2DRHIRef(XeTexture);
}

/**
 * Tries to reallocate the texture without relocation. Returns a new valid reference to the resource if successful.
 * Both the old and new reference refer to the same texture (at least the shared mip-levels) and both can be used or released independently.
 *
 * @param	Texture2D		- Texture to reallocate
 * @param	NewMipCount		- New number of mip-levels
 * @return					- New reference to the updated texture, or invalid if the reallocation failed
 */
FTexture2DRHIRef RHIReallocateTexture2D( FTexture2DRHIParamRef Texture2D, INT NewMipCount )
{
	FTexture2DRHIRef Texture2DRef( FXeTexture2D::ReallocateTexture2D(Texture2D, NewMipCount) );
	return Texture2DRef;
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
void* RHILockTexture2D(FTexture2DRHIParamRef Texture,UINT MipIndex,UBOOL bIsDataBeingWrittenTo,UINT& DestStride,UBOOL bLockWithinMiptail)
{
	Texture->Lock( MipIndex );
	void* Result=NULL;
	IDirect3DTexture9* D3DTexture2D = (IDirect3DTexture9*) Texture->Resource;
	DWORD LockFlags = 0;

//@todo xenon - triggers D3D error due to 64k page allocs
#ifndef _DEBUG
	if( !bIsDataBeingWrittenTo )
	{
		LockFlags |= D3DLOCK_READONLY;
	}
#endif

	UBOOL bLockMipTail = FALSE;
	if( !(Texture->GetCreationFlags()&TexCreate_ResolveTargetable) &&
		!(Texture->GetCreationFlags()&TexCreate_Uncooked) &&
		!(Texture->GetCreationFlags()&TexCreate_NoMipTail) &&
		!bLockWithinMiptail )
	{
		XGMIPTAIL_DESC MipTailDesc;
		XGGetMipTailDesc(D3DTexture2D, &MipTailDesc);
		if( MipIndex == MipTailDesc.BaseLevel )
		{
			bLockMipTail = TRUE;
		}
	}	

	if( bLockMipTail )
	{
		// Lock and fill the tail
		D3DLOCKED_TAIL LockedTail;
		D3DTexture2D->LockTail(&LockedTail, LockFlags);
		DestStride = LockedTail.RowPitch;
		Result = LockedTail.pBits;
	}
	else
	{
		// Lock and fill the mip level
		D3DLOCKED_RECT LockedRect;
		D3DTexture2D->LockRect(MipIndex, &LockedRect, NULL, LockFlags);
		DestStride = LockedRect.Pitch;
		Result = LockedRect.pBits;
	}

	return Result;
}

/**
* Unlocks a previously locked RHI texture resource
* @param Texture - the RHI texture resource to unlock
* @param MipIndex - mip level index for the surface to unlock
* @param bLockWithinMiptail - for platforms that support packed miptails allow locking of individual mip levels within the miptail
*/
void RHIUnlockTexture2D(FTexture2DRHIParamRef Texture,UINT MipIndex,UBOOL bLockWithinMiptail)
{
	IDirect3DTexture9* D3DTexture2D = (IDirect3DTexture9*) Texture->Resource;

	UBOOL bLockMipTail = FALSE;
	if( !(Texture->GetCreationFlags()&TexCreate_ResolveTargetable) &&
		!(Texture->GetCreationFlags()&TexCreate_Uncooked) &&
		!(Texture->GetCreationFlags()&TexCreate_NoMipTail) &&
		!bLockWithinMiptail )
	{
		XGMIPTAIL_DESC MipTailDesc;
		XGGetMipTailDesc(D3DTexture2D, &MipTailDesc);
		if( MipIndex == MipTailDesc.BaseLevel )
		{
			bLockMipTail = TRUE;
		}
	}

	if( bLockMipTail )
	{
		// Unlock the tail
		D3DTexture2D->UnlockTail();
	}
	else
	{
		// Unlock the mip level
		D3DTexture2D->UnlockRect(MipIndex);		
	}
	Texture->Unlock( MipIndex );
}

/**
 * Checks if a texture is still in use by the GPU.
 * @param Texture - the RHI texture resource to check
 * @param MipIndex - Which mipmap we're interested in
 * @return TRUE if the texture is still in use by the GPU, otherwise FALSE
 */
UBOOL RHIIsBusyTexture2D(FTexture2DRHIParamRef Texture, UINT MipIndex)
{
	check( Texture );
	IDirect3DTexture9* D3DTexture2D = (IDirect3DTexture9*) Texture->Resource;
	return D3DTexture2D && (D3DTexture2D->IsSet(GDirect3DDevice) || D3DTexture2D->IsBusy());
}

/**
 * For platforms that support packed miptails return the first mip level which is packed in the mip tail
 * @return mip level for mip tail or -1 if mip tails are not used
 */
INT RHIGetMipTailIdx(FTexture2DRHIParamRef Texture)
{
	check( Texture->Resource );
	XGMIPTAIL_DESC MipTailDesc;
	XGGetMipTailDesc( (IDirect3DTexture9*)Texture->Resource, &MipTailDesc );
	return MipTailDesc.BaseLevel;
}

/**
* Copies a region within the same mip levels of one texture to another.  An optional region can be speci
* Note that the textures must be the same size and of the same format.
*
* @param DstTexture - destination texture resource to copy to
* @param MipIdx - mip level for the surface to copy from/to. This mip level should be valid for both source/destination textures
* @param BaseSizeX - width of the texture (base level). Same for both source/destination textures
* @param BaseSizeY - height of the texture (base level). Same for both source/destination textures 
* @param Format - format of the texture. Same for both source/destination textures
* @param Region - list of regions to specify rects and source textures for the copy
*/
void RHICopyTexture2D(FTexture2DRHIParamRef DstTexture, UINT MipIdx, INT BaseSizeX, INT BaseSizeY, INT Format, const TArray<FCopyTextureRegion2D>& Regions)
{
	check( DstTexture->Resource );
	check( !(DstTexture->GetCreationFlags()&TexCreate_ResolveTargetable) );	
	UBOOL bPackedMipTail = (DstTexture->GetCreationFlags()&TexCreate_NoMipTail) ? FALSE : TRUE;

	// scale the base SizeX,SizeY for the current mip level
	INT MipSizeX = Max((INT)GPixelFormats[Format].BlockSizeX,BaseSizeX >> MipIdx);
	INT MipSizeY = Max((INT)GPixelFormats[Format].BlockSizeY,BaseSizeY >> MipIdx);

	// lock the destination texture
	UINT DstStride;
	BYTE* DstData = (BYTE*)RHILockTexture2D( DstTexture, MipIdx, TRUE, DstStride, FALSE );	

	// get description of destination texture
	XGTEXTURE_DESC DstDesc;
	XGGetTextureDesc( (IDirect3DTexture9*)DstTexture->Resource, MipIdx, &DstDesc );

	// get the offset of the current mip into the miptail for the destination texture. 0 if no miptail
	UINT DstMipTailOffset = XGGetMipTailLevelOffset( 
		BaseSizeX, 
		BaseSizeY, 
		1, 
		MipIdx, 
		XGGetGpuFormat((D3DFORMAT)MAKELINFMT(DstDesc.Format)),
		XGIsTiledFormat(DstDesc.Format),
		false
		);
	// default the non tiled version to the same offset
	INT DstNonTiledMipTailOffset = DstMipTailOffset;

	// ptr to untiled result data
	BYTE* DstDataNonTiled = DstData;
	// temp texture to store untiled result
	FTexture2DRHIRef TempTextureNonTiled;
	// if the destination texture is tiled then need to use a temp texture to store the untiled result
	if( XGIsTiledFormat(DstDesc.Format) )
	{
		DWORD TempFlags = DstTexture->GetCreationFlags()|TexCreate_NoTiling;
		if( !bPackedMipTail )
		{
			TempFlags |= TexCreate_NoMipTail;
		}
		// create a temp texture to store the untiled result
		TempTextureNonTiled = RHICreateTexture2D(BaseSizeX,BaseSizeY,Format,0,TempFlags,NULL);
		// lock this texture and use its data ptr as the untiled result
		UINT TempStride=0;
		DstDataNonTiled = (BYTE*)RHILockTexture2D(
			TempTextureNonTiled,
			MipIdx,
			TRUE,
			TempStride,
			FALSE
			);
		// get temp texture description
		XGTEXTURE_DESC TempDesc;
		XGGetTextureDesc( (IDirect3DTexture9*)TempTextureNonTiled->Resource, MipIdx, &TempDesc );
		// get the offset of the current mip into the miptail for this temp texture. 0 if no miptail
		DstNonTiledMipTailOffset = XGGetMipTailLevelOffset( 
			BaseSizeX, 
			BaseSizeY, 
			1, 
			MipIdx, 
			XGGetGpuFormat((D3DFORMAT)MAKELINFMT(TempDesc.Format)),
			false,
			false
			);	
	}

	for( INT RegionIdx=0; RegionIdx < Regions.Num(); RegionIdx++ )		
	{
		const FCopyTextureRegion2D& Region = Regions(RegionIdx);
		check( Region.SrcTexture->Resource );		

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
		
		// get description of source texture
		XGTEXTURE_DESC SrcDesc;
		XGGetTextureDesc( (IDirect3DTexture9*)Region.SrcTexture->Resource, Region.FirstMipIdx + MipIdx, &SrcDesc );

		// calc the offset from the miptail base level for this mip level. If no miptail then it is just 0
		UINT SrcMipTailOffset = XGGetMipTailLevelOffset( 
			BaseSizeX, 
			BaseSizeY, 
			1, 
			MipIdx, 
			XGGetGpuFormat((D3DFORMAT)MAKELINFMT(SrcDesc.Format)),
			true,
			false
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

		// copying from tiled to untiled
		if( XGIsTiledFormat(SrcDesc.Format) )
		{
			// destination point to copy to
			POINT Point = 
			{
				RegionOffsetX / GPixelFormats[Format].BlockSizeX,
				RegionOffsetY / GPixelFormats[Format].BlockSizeY
			};
			// source rect to copy from			
			RECT Rect;
			SetRect(
				&Rect,
				RegionOffsetX / GPixelFormats[Format].BlockSizeX,
				RegionOffsetY / GPixelFormats[Format].BlockSizeY,
				RegionOffsetX / GPixelFormats[Format].BlockSizeX + RegionSizeX / GPixelFormats[Format].BlockSizeX,
				RegionOffsetY / GPixelFormats[Format].BlockSizeY + RegionSizeY / GPixelFormats[Format].BlockSizeY 
				);

			// untile from source texture into the destination data
			XGUntileSurface(
				DstDataNonTiled + DstNonTiledMipTailOffset,
				DstDesc.RowPitch,
				&Point,
				SrcData + SrcMipTailOffset,
				SrcDesc.WidthInBlocks,
				SrcDesc.HeightInBlocks,
				&Rect,
				SrcDesc.BytesPerBlock
				);
		}
		// copying from untiled to untiled
		else
		{
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

				BYTE* SrcOffset = SrcData + SrcMipTailOffset + (CurBlockOffsetY * PitchBytes) + RowOffsetBytes;
				BYTE* DstOffset = DstDataNonTiled + DstNonTiledMipTailOffset + (CurBlockOffsetY * PitchBytes) + RowOffsetBytes;
				appMemcpy( DstOffset, SrcOffset, RowSizeBytes );
			}
		}

		// done reading from source mip so unlock it
		RHIUnlockTexture2D( Region.SrcTexture, Region.FirstMipIdx + MipIdx, FALSE );
	}

	// copy from the untiled temp result to the tiled result
	if( DstDataNonTiled != DstData )
	{
		check( IsValidRef(TempTextureNonTiled) );
		check( XGIsTiledFormat(DstDesc.Format) );

		XGTileSurface(
			DstData + DstMipTailOffset,
			DstDesc.WidthInBlocks,
			DstDesc.HeightInBlocks,
			NULL,
			DstDataNonTiled + DstNonTiledMipTailOffset,
			DstDesc.RowPitch,
			NULL,
			DstDesc.BytesPerBlock
			);
		
		RHIUnlockTexture2D( TempTextureNonTiled, MipIdx, FALSE );
	}

	// unlock the destination texture
	RHIUnlockTexture2D( DstTexture, MipIdx, FALSE );	
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
void RHISelectiveCopyMipData(FTexture2DRHIParamRef Texture, BYTE *Src, BYTE *Dst, UINT MemSize, UINT MipIdx)
{
	UINT BottumUpMipIdx = RHIGetMipTailIdx(Texture) - MipIdx;

	if(GTexturesCanShareMipMemory && BottumUpMipIdx < XEMIPMEMSHARING_NUM_LAYOUT_MIPS)
	{
		XGLAYOUT_REGION *Regions;

		IDirect3DTexture9* D3DTexture2D = (IDirect3DTexture9*) Texture->Resource;
		XGTEXTURE_DESC Desc;
		XGGetTextureDesc(D3DTexture2D, MipIdx, &Desc );

		// see if this format has a special memory layout
		switch(Desc.Format)
		{
			case D3DFMT_DXT1:
				Regions = GXeDXT1MipMemLayouts[BottumUpMipIdx];
				break;
			case D3DFMT_DXT3:
			case D3DFMT_DXT5:
				Regions = GXeDXT5MipMemLayouts[BottumUpMipIdx];
				break;
			default:
				Regions = NULL;
		}

		if(Regions != NULL)
		{
			// iterate through all the seperate memory regions and copy them individually
			for(UINT i = 0; Regions[i].StartOffset < MemSize ; i++)
			{
				UINT Offset = Regions[i].StartOffset;
				UINT Size = Regions[i].EndOffset - Regions[i].StartOffset;
				appMemcpy(Dst + Offset, Src + Offset, Size);
			}
			// done
			return;
		}
	}
	// default: just copy the whole memory section
	appMemcpy(Dst, Src, MemSize);
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
void RHICopyMipToMipAsync(FTexture2DRHIParamRef SrcTexture, INT SrcMipIndex, FTexture2DRHIParamRef DestTexture, INT DestMipIndex, INT Size, FThreadSafeCounter& Counter)
{
	// Lock old and new texture.
	UINT SrcStride;
	UINT DestStride;

	BYTE* Src = (BYTE*) RHILockTexture2D( SrcTexture, SrcMipIndex, FALSE, SrcStride, FALSE );
	BYTE* Dst = (BYTE*) RHILockTexture2D( DestTexture, DestMipIndex, TRUE, DestStride, FALSE );

#if PLATFORM_SUPPORTS_RENDERING_FROM_LOCKED_TEXTURES
	// Don't use async copy for small requests. @todo streaming: actually tweak size
	if( Size <= 8192 )
	{
		appMemcpy( Dst, Src, Size );
	}
	// Fire off async copy for larger requests so be don't need to block.
	else
	{
		// Increment request counter as we have a request to wait for.
		Counter.Increment();
		// Kick of async memory copy. @todo streaming: this could be aligned on at least consoles
		GThreadPool->AddQueuedWork( new	FAsyncCopy( Dst, Src, Size, &Counter ) );
	}
#else
	RHISelectiveCopyMipData( SrcTexture, Src, Dst, Size, SrcMipIndex);
	RHIUnlockTexture2D( SrcTexture, SrcMipIndex, FALSE );
	RHIUnlockTexture2D( DestTexture, DestMipIndex, FALSE );
#endif
}

void RHIFinalizeAsyncMipCopy(FTexture2DRHIParamRef SrcTexture, INT SrcMipIndex, FTexture2DRHIParamRef DestTexture, INT DestMipIndex)
{
//@TODO: Need to block on the async copy and then unlock the textures (for texture defragmentation)
#if PLATFORM_SUPPORTS_RENDERING_FROM_LOCKED_TEXTURES
#error TODO - IMPLEMENT THIS
#endif
}

/*-----------------------------------------------------------------------------
Cubemap texture support.
-----------------------------------------------------------------------------*/

/** 
* Sets D3D header and allocates memory for the resource 
*
* @param	Size - width of texture
* @param	Format - format of texture
* @param	NumMips - number of mips
* @param	Flags - creation flags
*/
void FXeTextureCube::AllocTextureCube(UINT Size,BYTE Format,UINT NumMips,DWORD Flags)
{
	// set the texture header for the driver	
	DWORD TextureSize = XGSetCubeTextureHeaderEx( 
		Size,
		NumMips, 
		0,
		XeGetTextureFormat(Format,Flags),
		0,
		(Flags&TexCreate_NoMipTail) ? XGHEADEREX_NONPACKED : 0,
		0,
		XGHEADER_CONTIGUOUS_MIP_OFFSET,
		(IDirect3DCubeTexture9*)Resource,
		NULL,
		NULL 
		);
	DWORD AlignedSize = Align(TextureSize,D3DTEXTURE_ALIGNMENT);
#if TRACK_GPU_RESOURCES
	PhysicalSize = AlignedSize;
	VirtualSize	 = sizeof(IDirect3DCubeTexture9);
#endif
	// allocate memory for texture data
	BaseAddress	= XeAllocateTextureMemory( AlignedSize, (Flags & TexCreate_AllowFailure) ? TRUE : FALSE, (Flags & TexCreate_DisableAutoDefrag) ? TRUE : FALSE );
	// set data address for this resource
	XGOffsetResourceAddress( Resource, BaseAddress ); 
}

/**
* Creates a Cube RHI texture resource
* @param Size - width/height of the texture to create
* @param Format - EPixelFormat texture format
* @param NumMips - number of mips to generate or 0 for full mip pyramid
* @param Flags - ETextureCreateFlags creation flags
*/
FTextureCubeRHIRef RHICreateTextureCube( UINT Size, BYTE Format, UINT NumMips, DWORD Flags, FResourceBulkDataInterface* BulkData )
{
	if( (Flags&TexCreate_ResolveTargetable) || (Flags & TexCreate_Uncooked) )
	{
		check( !((Flags&TexCreate_ResolveTargetable) && NumMips > 1) );

		IDirect3DCubeTexture9* pD3DTextureCube;		
		// create texture in memory for resolving surface
		VERIFYD3DRESULT( GDirect3DDevice->CreateCubeTexture( 
			Size,
			NumMips, 
			0, 
			XeGetTextureFormat(Format,Flags), 
			D3DPOOL_DEFAULT, 
			&pD3DTextureCube, 
			NULL ));

		FTextureCubeRHIRef TextureCube(new FXeTextureCube(pD3DTextureCube,Flags));
		return TextureCube;
	}
	else
	{
		// create the new cube texture resource
		FTextureCubeRHIRef TextureCube(new FXeTextureCube(Flags));
		// manually allocate resource and set header
		TextureCube->AllocTextureCube(Size,Format,NumMips,Flags);

		return TextureCube;
	}
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
void* RHILockTextureCubeFace(FTextureCubeRHIParamRef TextureCube,UINT FaceIndex,UINT MipIndex,UBOOL bIsDataBeingWrittenTo,UINT& DestStride,UBOOL bLockWithinMiptail)
{
	TextureCube->Lock( MipIndex );
	void* Result=NULL;
	IDirect3DCubeTexture9* D3DTextureCube = (IDirect3DCubeTexture9*) TextureCube->Resource;

	DWORD LockFlags = 0;

//@todo xenon - triggers D3D error due to 64k page allocs
#ifndef _DEBUG
	if( !bIsDataBeingWrittenTo )
	{
		LockFlags |= D3DLOCK_READONLY;
	}
#endif

	UBOOL bLockMipTail = FALSE;
	if( !(TextureCube->GetCreationFlags()&TexCreate_ResolveTargetable) &&
		!(TextureCube->GetCreationFlags()&TexCreate_Uncooked) &&
		!(TextureCube->GetCreationFlags()&TexCreate_NoMipTail) &&
		!bLockWithinMiptail )
	{
		XGMIPTAIL_DESC MipTailDesc;
		XGGetMipTailDesc(D3DTextureCube, &MipTailDesc);
		if( MipIndex == MipTailDesc.BaseLevel )
		{
			bLockMipTail = TRUE;
		}
	}

	if( bLockMipTail )
	{
		// Lock and fill the tail
		D3DLOCKED_TAIL LockedTail;
		D3DTextureCube->LockTail((D3DCUBEMAP_FACES) FaceIndex, &LockedTail, LockFlags);
		DestStride = LockedTail.RowPitch;
		Result = LockedTail.pBits;
	}
	else
	{
		// Lock and fill the mip level
		D3DLOCKED_RECT LockedRect;
		D3DTextureCube->LockRect((D3DCUBEMAP_FACES) FaceIndex, MipIndex, &LockedRect, NULL, LockFlags);
		DestStride = LockedRect.Pitch;
		Result = LockedRect.pBits;
	}

	return Result;
}

/**
* Unlocks a previously locked RHI texture resource
* @param Texture - the RHI texture resource to unlock
* @param MipIndex - mip level index for the surface to unlock
* @param bLockWithinMiptail - for platforms that support packed miptails allow locking of individual mip levels within the miptail
*/
void RHIUnlockTextureCubeFace(FTextureCubeRHIParamRef TextureCube,UINT FaceIndex,UINT MipIndex,UBOOL bLockWithinMiptail)
{
	IDirect3DCubeTexture9* D3DTextureCube = (IDirect3DCubeTexture9*) TextureCube->Resource;

	UBOOL bLockMipTail = FALSE;
	if( !(TextureCube->GetCreationFlags()&TexCreate_ResolveTargetable) &&
		!(TextureCube->GetCreationFlags()&TexCreate_Uncooked) &&
		!(TextureCube->GetCreationFlags()&TexCreate_NoMipTail) &&
		!bLockWithinMiptail )
	{
		XGMIPTAIL_DESC MipTailDesc;
		XGGetMipTailDesc(D3DTextureCube, &MipTailDesc);
		if( MipIndex == MipTailDesc.BaseLevel )
		{
			bLockMipTail = TRUE;
		}
	}

	if( bLockMipTail )
	{
		// Unlock the tail
		D3DTextureCube->UnlockTail((D3DCUBEMAP_FACES) FaceIndex);
	}
	else
	{
		// Unlock the mip level
		D3DTextureCube->UnlockRect((D3DCUBEMAP_FACES) FaceIndex, MipIndex);		
	}
	TextureCube->Unlock( MipIndex );
}

/*-----------------------------------------------------------------------------
Shared texture support.
-----------------------------------------------------------------------------*/

/**
* Computes the device-dependent amount of memory required by a texture.  This size may be passed to RHICreateSharedMemory.
* @param SizeX - The width of the texture.
* @param SizeY - The height of the texture.
* @param SizeZ - The depth of the texture.
* @param Format - The format of the texture.
* @return The amount of memory in bytes required by a texture with the given properties.
*/
SIZE_T XeCalculateTextureBytes(DWORD SizeX,DWORD SizeY,DWORD SizeZ,BYTE Format)
{
	const DWORD TiledSizeX = Align(SizeX,32);
	const DWORD TiledSizeY = Align(SizeY,32);
	const UBOOL bIsTiled = (XeGetTextureFormat(Format,0) & D3DFORMAT_TILED_MASK) != 0;
	return Align(
			CalculateImageBytes(
				bIsTiled ? TiledSizeX : SizeX,
				bIsTiled ? TiledSizeY : SizeY,
				SizeZ,
				Format
				),
			D3DTEXTURE_ALIGNMENT
			);
}

/**
* Create resource memory to be shared by multiple RHI resources
* @param Size - aligned size of allocation
* @return shared memory resource RHI ref
*/
FSharedMemoryResourceRHIRef RHICreateSharedMemory(SIZE_T Size)
{
	// create the shared memory resource
	FSharedMemoryResourceRHIRef SharedMemory(new FXeSharedMemoryResource(Size));
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
FSharedTexture2DRHIRef RHICreateSharedTexture2D(UINT SizeX,UINT SizeY,BYTE Format,UINT NumMips,FSharedMemoryResourceRHIParamRef SharedMemory,DWORD Flags)
{
	// create the D3D resource
	IDirect3DTexture9* Resource = new IDirect3DTexture9;

	// set the texture header
	const DWORD TextureSize = XGSetTextureHeaderEx( 
		SizeX,
		SizeY,
		NumMips, 
		0,
		XeGetTextureFormat(Format,Flags),
		0,
		0,	//XGHEADEREX_NONPACKED,
		0,
		XGHEADER_CONTIGUOUS_MIP_OFFSET,
		0,
		(IDirect3DTexture9*)Resource,
		NULL,
		NULL
		);

    DWORD AlignedSize = Align(TextureSize,D3DTEXTURE_ALIGNMENT);
	check( AlignedSize <= SharedMemory->Size );

	if ( AlignedSize > SharedMemory->Size )
	{
		DebugBreak();
	}

	// setup the shared memory
	XGOffsetResourceAddress( Resource, SharedMemory->BaseAddress );

	// keep track of the fact that the D3D resource is using the shared memory
	SharedMemory->AddResource(Resource);

	// create the 2d texture resource
	FSharedTexture2DRHIRef SharedTexture2D(new FXeSharedTexture2D(Resource, SharedMemory));

#if TRACK_GPU_RESOURCES
	SharedTexture2D->VirtualSize = sizeof(D3DTexture);
	SharedTexture2D->PhysicalSize = SharedMemory->Size;
#endif

	return SharedTexture2D;
}


/*-----------------------------------------------------------------------------
	FXeTexture2DResourceMem
-----------------------------------------------------------------------------*/

/** 
 * Init Constructor 
 * Allocates texture memory during construction
 * 
 * @param InSizeX			- width of texture
 * @param InSizeY			- height of texture
 * @param InNumMips			- total number of mips to allocate memory for
 * @param InFormat			- EPixelFormat texture format
 * @param InTexCreateFlags	- ETextureCreateFlags bit flags
 */
FXeTexture2DResourceMem::FXeTexture2DResourceMem(INT InSizeX, INT InSizeY, INT InNumMips, EPixelFormat InFormat, DWORD InTexCreateFlags)
:	SizeX(InSizeX)
,	SizeY(InSizeY)
,	NumMips(InNumMips)
,	Format(InFormat)
,	BaseAddress(NULL)
,	TextureSize(0)
,	TexCreateFlags(InTexCreateFlags)
{
	// Allow failure the first attempt.
	TexCreateFlags |= TexCreate_AllowFailure;
	AllocateTextureMemory();

	// Did it fail? Retry 5 times
	for ( INT Retries=0; IsValid() == FALSE && Retries < 5; Retries++ )
	{
		FreeTextureMemory();

		// Try to stream out texture data to allow for this texture to fit.
		GStreamingManager->StreamOutTextureData( TextureSize );

		if ( Retries == 4 )
		{
			// Attempt to create the texture for the last time. Don't allow failure this time.
			TexCreateFlags &= ~TexCreate_AllowFailure;
		}
		AllocateTextureMemory();
	}
}

/** 
 * Destructor
 */
FXeTexture2DResourceMem::~FXeTexture2DResourceMem()
{
	// mip data gets deleted by a texture resource destructor once this memory is used for RHI resource creation
	// but, if it was never used then free the memory here
	if( BaseAddress != NULL )
	{
		FreeTextureMemory();
	}
}

/** 
 * @return ptr to the resource memory which has been preallocated
 */
void* FXeTexture2DResourceMem::GetResourceBulkData() const
{
	return BaseAddress;
}

/** 
 * @return size of resource memory
 */
DWORD FXeTexture2DResourceMem::GetResourceBulkDataSize() const
{
	return TextureSize;
}

/**
 * Free memory after it has been used to initialize RHI resource 
 */
void FXeTexture2DResourceMem::Discard()
{
	// no longer maintain a pointer to mip data since it is owned by RHI resource after creation	
	BaseAddress = NULL;
	TextureSize = 0;
}

/**
 * @param MipIndex index for mip to retrieve
 * @return ptr to the offset in bulk memory for the given mip
 */
void* FXeTexture2DResourceMem::GetMipData(INT MipIndex)
{
	UINT MipAddress;
	XGGetTextureLayout(&TextureHeader,NULL,NULL,NULL,NULL,0,&MipAddress,NULL,NULL,NULL,0);
	DWORD Offset = XGGetMipLevelOffset(&TextureHeader, 0, MipIndex);
	DWORD MipTailIndex = XGGetMipTailBaseLevel(SizeX, SizeY, FALSE);
	void* MipData;

	// Is this the base level?
	if ( MipIndex == 0 || MipAddress == 0 )
	{
		// Is the base level a packed mipmap tail?
		if ( !(TexCreateFlags&TexCreate_NoMipTail) && MipIndex == MipTailIndex )
		{
			Offset = 0;
		}
		MipData = (BYTE*)BaseAddress + Offset;
	}
	// Is this a packed mip tail?
	else if ( !(TexCreateFlags&TexCreate_NoMipTail) && MipIndex == MipTailIndex )
	{
		// Get the address of the previous mip-level.
		BYTE* PrevMipAddress;
		if ( MipIndex == 1 )
		{
			PrevMipAddress = (BYTE*)BaseAddress;
		}
		else
		{
			Offset = XGGetMipLevelOffset(&TextureHeader, 0, MipIndex - 1);
			PrevMipAddress = (BYTE*)MipAddress + Offset;
		}

		// Add its mip-size to reach the requested mip-level.
		XGTEXTURE_DESC Desc;
		XGGetTextureDesc(&TextureHeader, MipIndex - 1, &Desc);
		MipData = PrevMipAddress + Desc.SlicePitch;
	}
	// Is this a regular mip-level?
	else
	{
		MipData = (BYTE*)MipAddress + Offset;
	}

	return MipData;
}

/**
 * @return total number of mips stored in this resource
 */
INT	FXeTexture2DResourceMem::GetNumMips()
{
	return NumMips;
}

/** 
 * @return width of texture stored in this resource
 */
INT FXeTexture2DResourceMem::GetSizeX()
{
	return SizeX;
}

/** 
 * @return height of texture stored in this resource
 */
INT FXeTexture2DResourceMem::GetSizeY()
{
	return SizeY;
}

/** 
 * Calculate size needed to store this resource and allocate texture memory for it
 */
void FXeTexture2DResourceMem::AllocateTextureMemory()
{
	check(BaseAddress == NULL);
	BaseAddress = FXeTexture2D::AllocTexture2D(SizeX, SizeY, Format, NumMips, TexCreateFlags, &TextureHeader, TextureSize, NULL);
}

/**
 * Free the memory from the texture pool
 */
void FXeTexture2DResourceMem::FreeTextureMemory()
{
	if ( XeIsValidTextureData(BaseAddress) )
	{
		// add this texture resource to the list of resources to be deleted later
		// this will free the memory from that was allocated from the texture pool
		AddUnusedXeResource(NULL, BaseAddress, TRUE, FALSE);
	}
	BaseAddress = NULL;
}


#endif
