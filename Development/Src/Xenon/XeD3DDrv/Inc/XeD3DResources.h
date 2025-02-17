/*=============================================================================
	XeD3DResources.h: base XeD3D resource definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if USE_XeD3D_RHI

#define TRACK_GPU_RESOURCES !FINAL_RELEASE

/*
 * Adds an unused Xenon GPU resource to a buffer so that it can be deleted properly at a later time.
 *
 * @param Resource		- D3D struct to be deleted.
 * @param BaseAddress	- Physical memory to be deleted.
 * @param bIsTexture	- TRUE if resource is a texture.
 * @param bUsesResourceArray - TRUE if BaseAddress of this resource comes from a TResourceArray
 * @param bIsShared		- TRUE if resource might use shared memory
 * @param SecondaryBaseAddress - Address of secondary manually allocated resource
*/
void AddUnusedXeResource( IDirect3DResource9* Resource, void* BaseAddress, UBOOL bIsTexture, UBOOL bUsesResourceArray, UBOOL bIsShared = FALSE, void* SecondaryBaseAddress = NULL);

/*
 * Adds an unused Xenon shared GPU resource to a buffer so that it can be deleted properly at a later time.
 *
 * @param Resource		- Array of D3D structs to be deleted.
 * @param BaseAddress	- Physical memory to be deleted.
 */
void AddUnusedXeSharedResource( TArray<IDirect3DResource9*> & Resources, void* BaseAddress );

/*
 * Deletes all resources that became unused during the previous frame. It also swaps the two buffers.
 * This function should be called once at the end of each frame.
 */
void DeleteUnusedXeResources();


/**
 * The base class of XeRHI resources.
 */
class FXeResource : public virtual FRefCountedObject
{
};

/**
 * The base class of XeRHI GPU resources.
 */
class FXeGPUResource : public FXeResource
{
public:
	/** default constructor */
	FXeGPUResource(EResourceUsageFlag InUsageFlag)
	:	Resource(NULL)
	,	BaseAddress(NULL)
	,	SecondaryBaseAddress(NULL)
	,	bIsTexture(FALSE)
	,   bIsShared(FALSE)
	,	bUsesResourceArray(FALSE)
	,	bHasBeenLocked(FALSE)
	,	UsageFlag(InUsageFlag)
#if TRACK_GPU_RESOURCES
	,	VirtualSize(0)
	,	PhysicalSize(0)
	,	ResourceLink(NULL)
#endif
	{
#if TRACK_GPU_RESOURCES
		ResourceLink = TLinkedList<FXeGPUResource*>(this);
		ResourceLink.Link( GetResourceList() );
#endif
	}

	/** constructor initialized w/ existing D3D resource */
	FXeGPUResource(IDirect3DResource9* InResource, EResourceUsageFlag InUsageFlag)
	:	Resource(InResource)
	,	BaseAddress(NULL)
	,	SecondaryBaseAddress(NULL)
	,	bIsTexture(FALSE)
	,   bIsShared(FALSE)
	,	bUsesResourceArray(FALSE)
	,	bHasBeenLocked(FALSE)
	,	UsageFlag(InUsageFlag)
#if TRACK_GPU_RESOURCES
	,	VirtualSize(0)
	,	PhysicalSize(0)
	,	ResourceLink(NULL)
#endif
	{
#if TRACK_GPU_RESOURCES
		ResourceLink = TLinkedList<FXeGPUResource*>(this);
		ResourceLink.Link( GetResourceList() );
#endif
	}

	/** destructor */
	virtual ~FXeGPUResource();

	/**
	 * @return Resource type name.
	 */
	virtual TCHAR* GetTypeName() const = 0;

#if TRACK_GPU_RESOURCES
	/**
	 * @return reference to the global list of GPU resources.
	 */
	static TLinkedList<FXeGPUResource*>*& GetResourceList()
	{
		static TLinkedList<FXeGPUResource*>* FirstResourceLink = NULL;
		return FirstResourceLink;
	}
#endif

	/** D3D resource */
	IDirect3DResource9* Resource;
	/** address of manually allocated resource */
	void* BaseAddress;
	/** address of secondary manually allocated resource */
	void* SecondaryBaseAddress;
	/** TRUE if resource is a texture */
	BITFIELD bIsTexture:1;
	/** TRUE if resource is using shared memory */
	BITFIELD bIsShared:1;
	/** TRUE if BaseAddress of this resource comes from a TResourceArray */ 
	BITFIELD bUsesResourceArray:1;
	/** 
	 *	FALSE if the data has not ever been locked.
	 *	Used to identify when dynamic resources should be reallocated.
	 */ 
	BITFIELD bHasBeenLocked:1;
	/** The EResourceUsageFlag... */ 
	BITFIELD UsageFlag:2;
#if TRACK_GPU_RESOURCES
	INT	VirtualSize;
	INT PhysicalSize;
	TLinkedList<FXeGPUResource*> ResourceLink;
#endif
};


/**
 * A shared memory resources used by any of the other shared GPU resources.
 */
class FXeSharedMemoryResource : public FXeResource
{
public:
	/** default constructor */
	FXeSharedMemoryResource()
	:	BaseAddress(NULL)
    ,   Size(0)
	{

	}

    /** constructor that allocates memory */
    FXeSharedMemoryResource(UINT InSize)
    {
        BaseAddress = appPhysicalAlloc(InSize, CACHE_WriteCombine);
        Size = BaseAddress ? InSize : 0;
    }

    /** destructor */
    virtual ~FXeSharedMemoryResource()
    {
	    FRefCountedObject::~FRefCountedObject();

        // queue up the memory chunk to be freed
        AddUnusedXeSharedResource( Resources, BaseAddress );
    }

    /** add a resource to be tracked along with this memory */
    void AddResource(IDirect3DResource9 *Resource)
    {
        INT Index = Resources.Add( 1 );
        Resources(Index) = Resource;
    }

    /** resources that use this shared block of memory */
    TArray<IDirect3DResource9*> Resources;

    /** address of manually allocated memory */
    void* BaseAddress;

    /** size of the allocated memory */
    UINT  Size;
};

typedef TRefCountPtr<FXeSharedMemoryResource> FSharedMemoryResourceRHIRef;
typedef FXeSharedMemoryResource* FSharedMemoryResourceRHIParamRef;


#endif
