/*=============================================================================
	XeD3DTexture.h: XeD3D texture RHI definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if USE_XeD3D_RHI

// Texture Sharing flags.
enum ETextureSharingFlags
{
	TEXTURESHARING_None			= 0,
	TEXTURESHARING_DXT1			= 1,
	TEXTURESHARING_DXT3or5		= 2,
	TEXTURESHARING_NumTypes		= 3,
	TEXTURESHARING_TypeMask		= 0x03,
	TEXTURESHARING_Recipient	= 4,
	TEXTURESHARING_Donor		= 8,
	TEXTURESHARING_Orphan		= 16
};

/** base texture resource */
class FXeTextureBase : public FXeGPUResource
{
public:
	/** default constructor */
	FXeTextureBase(DWORD InFlags=0, DWORD InSharingFlags=0)
	:	FXeGPUResource(RUF_Static)
	,	CreationFlags(InFlags)
	,	SharingFlags(InSharingFlags)
	,	LockedMipLevels(0)
	,	ReallocatedTexture(NULL)
	{
		bIsTexture = TRUE;
		XeRegisterTexture( this );
	}

	/** constructor initialized w/ existing D3D resource */
	FXeTextureBase(IDirect3DBaseTexture9* InResource,DWORD InFlags=0, DWORD InSharingFlags=0)
	:	FXeGPUResource(InResource, RUF_Static)
	,	CreationFlags(InFlags)
	,	SharingFlags(InSharingFlags)
	,	LockedMipLevels(0)
	,	ReallocatedTexture(NULL)
	{
		bIsTexture = TRUE;
		XeRegisterTexture( this );
	}

	/** Destructor */
	virtual ~FXeTextureBase();

	FORCEINLINE DWORD GetCreationFlags()
	{
		return CreationFlags;
	}

	/**
	 *	Returns the texture memory sharing flags for this texture.
	 *	@return		Texture memory sharing flags (ETextureSharingFlags).
	 */
	FORCEINLINE DWORD GetSharingFlags()
	{
		return SharingFlags;
	}

	/**
	 *	Makes this texture an orphan (for texture memory sharing purposes).
	 */
	FORCEINLINE void SetOrphan()
	{
		SharingFlags |= TEXTURESHARING_Orphan;
	}

	/**
	 *	Returns whether this texture is an orphan for texture memory sharing.
	 *	@return		TRUE if this texture is an orphan
	 */
	FORCEINLINE UBOOL IsOrphan()
	{
		return SharingFlags & TEXTURESHARING_Orphan;
	}

	/**
	 *	Enables/disables whether this texture should be available as a donor for texture memory sharing.
	 *	@param bNewValue	TRUE if this texture should be considered for texture memory sharing, otherwise FALSE
	 */
	FORCEINLINE void SetDonor(UBOOL bNewValue)
	{
		SharingFlags = bNewValue ? SharingFlags | TEXTURESHARING_Donor : SharingFlags & ~TEXTURESHARING_Donor;
	}

	/**
	 *	Returns whether this texture can be considered for texture memory sharing.
	 *	@return		TRUE if this texture can be considered for texture memory sharing
	 */
	FORCEINLINE UBOOL IsDonor()
	{
		return SharingFlags & TEXTURESHARING_Donor;
	}

	/**
	 *	Returns whether this texture is locked or not.
	 *	@return		TRUE if this texture is locked
	 */
	UBOOL IsLocked() const
	{
		return LockedMipLevels != 0;
	}

	/**
	 * Locks the resource, blocks if texture memory is currently being defragmented.
	 * @param MipIndex	Mip-level to lock
	 */
	void Lock( INT MipIndex )
	{
		// Block until the texture memory is available (a defragmentation process may be running).
		XeLockTexturePool();
		LockedMipLevels |= GBitFlag[MipIndex];
		XeUnlockTexturePool();
	}

	/**
	 * Unlocks the resource.
	 * @param MipIndex	Mip-level to unlock
	 */
	void Unlock( DWORD MipIndex )
	{
		LockedMipLevels &= ~GBitFlag[MipIndex];
	}

	/** Checks whether this texture can be relocated or not by the defragmentation process. */
	UBOOL CanRelocate();

	/**
	 *	Notifies that the texture data is being reallocated and is shared for a brief moment,
	 *	so that this texture can do the right thing upon destruction.
	 *
	 *	@param ReallocatedTexture	- The other texture that briefly shares the texture data when it's reallocated
	 **/
	void SetReallocatedTexture( FXeTextureBase* ReallocatedTexture );

private:
	DWORD	CreationFlags;
	DWORD	SharingFlags;

	/** One bit for each mip-level. A "one" means that mip-level is locked. */
	DWORD	LockedMipLevels;

	/** Another texture that briefly shares the texture data when it's reallocated. */
	FXeTextureBase*	ReallocatedTexture;
};

/** 2D texture resource */
class FXeTexture2D : public FXeTextureBase
{
public:
	/** default constructor */
	FXeTexture2D(DWORD InFlags=0,DWORD InSharingFlags=0)
	:	FXeTextureBase(new IDirect3DTexture9,InFlags,InSharingFlags)
	{
	}

	/** constructor initialized w/ existing D3D resource */
	FXeTexture2D(IDirect3DTexture9* InResource,DWORD InFlags=0,DWORD InSharingFlags=0)
	:	FXeTextureBase(InResource,InFlags,InSharingFlags)
	{
	}

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
	static void* AllocTexture2D(UINT SizeX,UINT SizeY,BYTE Format,UINT NumMips,DWORD Flags,IDirect3DTexture9* Texture,DWORD& OutMemorySize,FResourceBulkDataInterface* BulkData);

	/** 
	 * Tries to add or remove mip-levels of the texture without relocation.
	 *
	 * @param Texture2D		- Source texture
	 * @param NewMipCount	- Number of desired mip-levels
	 * @return				- A newly allocated FXeTexture2D if successful, or NULL if it failed.
	 */
	static FXeTexture2D* ReallocateTexture2D(FXeTexture2D* Texture2D, INT NewMipCount);

	/** 
	 * Sets D3D header and pairs the resource with a given donor resource to share memory 
	 *
	 * @param	SizeX		- width of texture
	 * @param	SizeY		- height of texture
	 * @param	Format		- format of texture
	 * @param	NumMips		- number of mips
	 * @param	Flags		- creation flags
	 */
	UBOOL PairTexture2D(UINT SizeX, UINT SizeY, BYTE Format, UINT NumMips, DWORD Flags);

	/**
	 * @return Resource type name.
	 */
	virtual TCHAR* GetTypeName() const
	{
		return TEXT("Texture2D");
	}

	/** Dynamic cast to a shared texture */
	virtual const class FXeSharedTexture2D* const GetSharedTexture() const
	{
		return NULL;
	}
};

/** cube texture resource */
class FXeTextureCube : public FXeTextureBase
{
public:
	/** default constructor */
	FXeTextureCube(DWORD InFlags=0)
	:	FXeTextureBase(new IDirect3DCubeTexture9,InFlags)
	{
	}

	/** constructor initialized w/ existing D3D resource */
	FXeTextureCube(IDirect3DCubeTexture9* InResource,DWORD InFlags=0)
	:	FXeTextureBase(InResource,InFlags)
	{
	}

	/** 
	 * Sets D3D header and allocates memory for the resource 
	 *
	 * @param	Size - width of texture
	 * @param	Format - format of texture
	 * @param	NumMips - number of mips
	 * @param	Flags - creation flags
	 */
	void AllocTextureCube(UINT Size,BYTE Format,UINT NumMips,DWORD Flags);

	/**
	 * @return Resource type name.
	 */
	virtual TCHAR* GetTypeName() const
	{
		return TEXT("TextureCube");
	}
};


class FXeSharedTexture2D : public FXeTexture2D
{
public:
	/** default constructor */
	FXeSharedTexture2D(void)
	{
	}

	/** constructor initialized w/ existing D3D resource  */
	FXeSharedTexture2D(IDirect3DTexture9* InResource, FSharedMemoryResourceRHIRef InMemory)
	:	FXeTexture2D(InResource)
	,	Memory(InMemory)
	{
	}

	/** destructor */
	virtual ~FXeSharedTexture2D()
	{
		// This is required because of the shared memory member variable which
		// must have its destructor called.
	}

	/** Dynamic cast to a shared texture */
	virtual const FXeSharedTexture2D* const GetSharedTexture() const
	{
		return this;
	}

	/** Accessor for the shared memory reference */
	const TRefCountPtr<FXeSharedMemoryResource> GetSharedMemory() const
	{
		return Memory;
	}

private:
	FSharedMemoryResourceRHIRef Memory;
};


/*-----------------------------------------------------------------------------
	RHI texture types
-----------------------------------------------------------------------------*/
typedef TXeGPUResourceRef<IDirect3DBaseTexture9,FXeTextureBase> FTextureRHIRef;

typedef FXeTextureBase* FTextureRHIParamRef;

class FTexture2DRHIRef : public TXeGPUResourceRef<IDirect3DTexture9,FXeTexture2D>
{
public:

	/** Default constructor. */
	FTexture2DRHIRef() {}

	/** Initialization constructor. */
	FTexture2DRHIRef(FXeTexture2D* InResource): TXeGPUResourceRef<IDirect3DTexture9,FXeTexture2D>(InResource)
	{}

	/** Conversion to base texture reference operator. */
	operator FTextureRHIRef() const
	{
		return FTextureRHIRef((FXeTextureBase*)*this);
	}
};

typedef FXeTexture2D* FTexture2DRHIParamRef;

class FTextureCubeRHIRef : public TXeGPUResourceRef<IDirect3DCubeTexture9,FXeTextureCube>
{
public:
	
	/** Default constructor. */
	FTextureCubeRHIRef() {}

	/** Initialization constructor. */
	FTextureCubeRHIRef(FXeTextureCube* InResource): TXeGPUResourceRef<IDirect3DCubeTexture9,FXeTextureCube>(InResource)
	{}

	/** Conversion to base texture reference operator. */
	operator FTextureRHIRef() const
	{
		return FTextureRHIRef((FXeTextureBase*)*this);
	}
};

typedef FXeTextureCube* FTextureCubeRHIParamRef;

class FSharedTexture2DRHIRef : public TXeGPUResourceRef<IDirect3DTexture9,FXeSharedTexture2D>
{
public:

	/** Default constructor. */
	FSharedTexture2DRHIRef() {}

	/** Initialization constructor. */
	FSharedTexture2DRHIRef(FXeSharedTexture2D* InResource): TXeGPUResourceRef<IDirect3DTexture9,FXeSharedTexture2D>(InResource)
	{}

	/** Conversion to base texture reference operator. */
	operator FTextureRHIRef() const
	{
		return FTextureRHIRef((FXeTextureBase*)*this);
	}

	/** Conversion to base texture reference operator. */
	operator FTexture2DRHIRef() const
	{
		return FTexture2DRHIRef((FXeTexture2D*)*this);
	}
};

typedef FXeTexture2D* FTexture2DRHIParamRef;

/**
 * Allocates memory for directly loading texture mip data 
 */
class FXeTexture2DResourceMem : public FTexture2DResourceMem
{
public:	
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
	FXeTexture2DResourceMem(INT InSizeX, INT InSizeY, INT InNumMips, EPixelFormat InFormat, DWORD InTexCreateFlags);
	/** 
	 * Destructor
	 */
	virtual ~FXeTexture2DResourceMem();

	// FResourceBulkDataInterface interface	
	
	/** 
	 * @return ptr to the resource memory which has been preallocated
	 */
	void* GetResourceBulkData() const;
	/** 
	 * @return size of resource memory
	 */
	DWORD GetResourceBulkDataSize() const;
	/**
	 * Free memory after it has been used to initialize RHI resource 
	 */
	void Discard();

	// FTexture2DResourceMem interface

	/**
	 * @param MipIdx index for mip to retrieve
	 * @return ptr to the offset in bulk memory for the given mip
	 */
	void* GetMipData(INT MipIdx);
	/**
	 * @return total number of mips stored in this resource
	 */
	INT	GetNumMips();
	/** 
	 * @return width of texture stored in this resource
	 */
	INT GetSizeX();
	/** 
	 * @return height of texture stored in this resource
	 */
	INT GetSizeY();
	/** 
	 * @return Size of the texture in bytes.
	 */
	INT GetTextureSize()
	{
		return TextureSize;
	}
	/**
	 * Whether the resource memory is properly allocated or not.
	 **/
	virtual UBOOL IsValid()
	{
		return XeIsValidTextureData(BaseAddress);
	}

private:
	/** Use init constructor */
	FXeTexture2DResourceMem() {}
	/** 
	 * Calculate size needed to store this resource and allocate texture memory for it
	 */
	void AllocateTextureMemory();
	/**
	 * Free the memory from the texture pool
	 */
	void FreeTextureMemory();

	/** width of texture */
	INT SizeX;
	/** height of texture */
	INT SizeY;
	/** total number of mips of texture */
	INT NumMips;
	/** format of texture */
	EPixelFormat Format;
	/** ptr to the allocated memory for all mips */
	void* BaseAddress;
	/** size of the allocated memory for all mips */
	DWORD TextureSize;
	/** ETextureCreateFlags bit flags */
	DWORD TexCreateFlags;
	/** D3D texture header to describe the texture memory */
	IDirect3DTexture9 TextureHeader;
};


#endif
