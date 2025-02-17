/*=============================================================================
	XeD3DResources.h: base XeD3D resource definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "XeD3DDrvPrivate.h"

#if USE_XeD3D_RHI

/**
 * Destructor
 */
FXeGPUResource::~FXeGPUResource()
{
	FRefCountedObject::~FRefCountedObject();		
#if TRACK_GPU_RESOURCES
	ResourceLink.Unlink();
#endif
	// free resource that was manually allocated
	
	if( BaseAddress )
	{
		AddUnusedXeResource( Resource, BaseAddress, bIsTexture, bUsesResourceArray, bIsShared, SecondaryBaseAddress );
	}
	// free resource that was created by device
	else if( Resource )
	{
		Resource->Release();
	}
}

void DeleteUnusedXeSharedResources();

/**
 * Container class for Xenon GPU resources that are to be deleted the next frame.
 */
struct FXeUnusedResource
{
	/** D3D resource */
	IDirect3DResource9* Resource;
	/** address of manually allocated resource */
	void* BaseAddress;
	/** address of secondary manually allocated resource */
	void* SecondaryBaseAddress;
	/** TRUE if resource is a texture */
	BITFIELD bIsTexture:1;
	/** TRUE if resource can be shared */
	BITFIELD bIsShared:1;
	/** TRUE if BaseAddress of this resource comes from a TResourceArray */ 
	BITFIELD bUsesResourceArray:1;
};

/*
 * Two buffers of orphaned resources that will be deleted the next frame.
 * One buffer is for the current frame, the other is for the previous frame.
 */
static TArray<FXeUnusedResource> GUnusedXeResources[2];
	
/* Index into GDeferredXeResources that specifies which buffer is for the current frame. */
static UINT GUnusedXeResourcesCurrentFrame = 0;

/**
 * Deletes an Xbox GPU resource.
 *
 * @param Resource				- D3D struct to be deleted.
 * @param BaseAddress			- Physical memory to be deleted.
 * @param bIsTexture			- TRUE if resource is a texture.
 * @param bUsesResourceArray	- TRUE if BaseAddress of this resource comes from a TResourceArray
 * @param bIsShared				- TRUE if resource can use shared memory
 * @param SecondaryBaseAddress	- Address of secondary manually allocated resource
 */
void DeleteXeResource( IDirect3DResource9* Resource, void* BaseAddress, UBOOL bIsTexture, UBOOL bUsesResourceArray, UBOOL bIsShared, void* SecondaryBaseAddress  )
{
	// Free the GPU memory (physical memory).
	if( bIsTexture )
	{
		if ( SecondaryBaseAddress )
		{
			// Using deferred in-place reallocation (shrinking)?
			if ( bIsShared == FALSE )
			{
				// Shrink it now.
				UBOOL bSuccess = XeReallocateTextureMemory( BaseAddress, SecondaryBaseAddress );
				check(bSuccess);
			}
			// Texture with "shared" mip-map memory?
			else
			{
				XeFreeTextureMemory( SecondaryBaseAddress, FALSE );
				XeFreeTextureMemory( BaseAddress, bIsShared );
			}
		}
		else
		{
			XeFreeTextureMemory( BaseAddress, bIsShared );
		}
	}
	// let resource arrays handle their own allocated memory
	else if( !bUsesResourceArray )
	{
		appPhysicalFree( BaseAddress );
	}

	// Free the resource header (virtual memory).
	delete Resource;
}

/*
 * Adds an unused Xenon GPU resource to a buffer so that it can be deleted properly at a later time.
 *
 * @param Resource				- D3D struct to be deleted.
 * @param BaseAddress			- Physical memory to be deleted.
 * @param bIsTexture			- TRUE if resource is a texture.
 * @param bUsesResourceArray	- TRUE if BaseAddress of this resource comes from a TResourceArray
 * @param bIsShared				- TRUE if resource can use shared memory
 * @param SecondaryBaseAddress	- Address of secondary manually allocated resource
 */
void AddUnusedXeResource( IDirect3DResource9* Resource, void* BaseAddress, UBOOL bIsTexture, UBOOL bUsesResourceArray, UBOOL bIsShared, void* SecondaryBaseAddress  )
{
	// Is the resource still being used by D3D or the GPU?
	if ( Resource == NULL || Resource->IsSet(GDirect3DDevice) || Resource->IsBusy() )
	{
		// Queue up to delete it later.
		TArray<FXeUnusedResource>& ResourceArray = GUnusedXeResources[GUnusedXeResourcesCurrentFrame];
		INT Index = ResourceArray.Add( 1 );
		ResourceArray(Index).Resource = Resource;
		ResourceArray(Index).BaseAddress = BaseAddress;
		ResourceArray(Index).SecondaryBaseAddress = SecondaryBaseAddress;
		ResourceArray(Index).bIsTexture = bIsTexture;
		ResourceArray(Index).bIsShared = bIsShared;
		ResourceArray(Index).bUsesResourceArray = bUsesResourceArray;
	}
	else
	{
		// Delete it right away.
		DeleteXeResource(Resource, BaseAddress, bIsTexture, bUsesResourceArray, bIsShared, SecondaryBaseAddress);
	}
}

/*
 * Deletes all resources that were added during the previous frame. It also swaps the two buffers.
 * This function should be called once at the end of each frame.
 */
void DeleteUnusedXeResources()
{
	TArray<FXeUnusedResource>& ResourceArray = GUnusedXeResources[1 - GUnusedXeResourcesCurrentFrame];
	for ( INT Index=0; Index < ResourceArray.Num(); ++Index )
	{
		FXeUnusedResource& Resource = ResourceArray( Index );

		// Block CPU if necessary.
		if (Resource.Resource != NULL)
		{
			Resource.Resource->BlockUntilNotBusy();
		}

		DeleteXeResource(Resource.Resource, Resource.BaseAddress, Resource.bIsTexture, Resource.bUsesResourceArray, Resource.bIsShared, Resource.SecondaryBaseAddress);
	}

	ResourceArray.Reset();

	// Swap the current frame.
	GUnusedXeResourcesCurrentFrame = 1 - GUnusedXeResourcesCurrentFrame;

	DeleteUnusedXeSharedResources();
}


/**
 * Container class for Xenon shared GPU resources that are to be deleted.
 */
struct FXeUnusedSharedResource
{
	/** D3D resources that share this resource (memory) */
	TArray<IDirect3DResource9*> Resources;
	/** address of manually allocated resource */
	void* BaseAddress;
};

/*
 * A buffer of orphaned shared resources that will be deleted when they are no longer in use.
 */
static TArray<FXeUnusedSharedResource> GUnusedXeSharedResources;

/*
 * Adds an unused Xenon shared GPU resource to a buffer so that it can be deleted properly at a later time.
 *
 * @param Resource		- Array of D3D structs to be deleted.
 * @param BaseAddress	- Physical memory to be deleted.
 */

void AddUnusedXeSharedResource( TArray<IDirect3DResource9*> & Resources, void* BaseAddress )
{
	// Find an empty slot in the unused resource list

	INT UnusedResourceIndex = 0;
	INT NumUnusedResources = GUnusedXeSharedResources.Num();		

	while ( UnusedResourceIndex < NumUnusedResources )
	{
		if ( GUnusedXeSharedResources( UnusedResourceIndex ).BaseAddress == NULL )
		{
			break;
		}
		++UnusedResourceIndex;
	}

	if ( UnusedResourceIndex == NumUnusedResources )
	{
		UnusedResourceIndex = GUnusedXeSharedResources.Add( 1 );
	}

	// Empty slot found or created

	GUnusedXeSharedResources( UnusedResourceIndex ).Resources = Resources;
	GUnusedXeSharedResources( UnusedResourceIndex ).BaseAddress = BaseAddress;
}

/*
 * Deletes all pending resources that are no longer busy.
 * This function should be called once at the end of each frame.
 */
void DeleteUnusedXeSharedResources()
{
	INT NumUnusedResources = GUnusedXeSharedResources.Num();
	INT NumRemainingUnusedResources = 0;

	for ( INT UnusedResourceIndex=0; UnusedResourceIndex < NumUnusedResources; ++UnusedResourceIndex )
	{
		FXeUnusedSharedResource& UnusedResource = GUnusedXeSharedResources( UnusedResourceIndex );

		if ( UnusedResource.BaseAddress == NULL )
		{
			continue;
		}

		// Remove any of the dependent resources that are no longer busy
		INT NumResources = UnusedResource.Resources.Num();
		INT NumRemainingResources = 0;

		for ( INT ResourceIndex=0; ResourceIndex < NumResources; ++ResourceIndex )
		{
			D3DResource* Resource = UnusedResource.Resources( ResourceIndex );
			if ( Resource == NULL )
			{
				continue;
			}

			// We own a single reference count on the resources, so once the reference count
			// goes to 1 and the resource is no longer busy we know that it is no longer
			// using the shared memory and we can remove it from our list of resources to
			// check.

			// If this assert fires then there is a mismatch in reference counts.
			check(Resource->ReferenceCount > 0);

			if ( (Resource->ReferenceCount == 1) && !Resource->IsBusy() )
			{
				delete Resource; // We own the resource so we have to delete it ourselves.		              
				UnusedResource.Resources( ResourceIndex ) = NULL;
			}
			else
			{
				++NumRemainingResources;
			} 
		}

		// If all of the resources are no longer busy we can release the shared memory
		if ( NumRemainingResources == 0 )
		{
			// Free the memory.
			appPhysicalFree( UnusedResource.BaseAddress );
			
			UnusedResource.Resources.Reset();
			UnusedResource.BaseAddress = NULL;
		}
		else
		{
			++NumRemainingUnusedResources;
		}
	}

	if ( NumRemainingUnusedResources == 0 )
	{
		GUnusedXeSharedResources.Reset();
	}
}


/**
 * Dumps GPU resource information if tracking is enabled.
 *
 * @param	bSummaryOnly	Whether to only print summary
 */
void DumpXeGPUResources( UBOOL bSummaryOnly, FOutputDevice& Ar )
{
#if TRACK_GPU_RESOURCES
	// Helper structure to keep track of resource type -> memory mapping
	struct FResourceSummary
	{
		INT TotalPhysicalSize;
		INT	TotalVirtualSize;
		INT ResourceCount;

		FResourceSummary()
		:	TotalPhysicalSize( 0 )
		,	TotalVirtualSize( 0 )
		,	ResourceCount( 0 )
		{}
	
		FResourceSummary( INT InPhysicalSize, INT InVirtualSize)
		:	TotalPhysicalSize( InPhysicalSize )
		,	TotalVirtualSize( InVirtualSize )
		,	ResourceCount( 1 )
		{}
	};
	// Map from resource type to memory usage.
	TMap<FString,FResourceSummary> TypeToSummaryMap;

	Ar.Logf( TEXT("GPU resource dump:") );
	if( !bSummaryOnly )
	{
		Ar.Logf( TEXT(",Resource Type,Physical Size,Virtual Size") );
	}

	TLinkedList<FXeGPUResource*>*& ResourceList = FXeGPUResource::GetResourceList();
	for( TLinkedList<FXeGPUResource*>::TIterator ResourceIt(ResourceList); ResourceIt; ResourceIt.Next() )
	{
		const FXeGPUResource* Resource = (*ResourceIt);

		// Look for existing summary for resource type.
		FResourceSummary* Summary = TypeToSummaryMap.Find( Resource->GetTypeName() );
		
		// Create new one if not found.
		if( !Summary )
		{
			TypeToSummaryMap.Set( Resource->GetTypeName(), FResourceSummary(Resource->PhysicalSize,Resource->VirtualSize) );
		}
		else
		{
			Summary->TotalPhysicalSize	+= Resource->PhysicalSize;
			Summary->TotalVirtualSize	+= Resource->VirtualSize;
			Summary->ResourceCount		++;
		}

		// Log in CSV style if we want more than the summary.
		if( !bSummaryOnly )
		{
			Ar.Logf( TEXT(",%s,%i,%i"), Resource->GetTypeName(), Resource->PhysicalSize, Resource->VirtualSize );
		}
	}

	for( TMap<FString,FResourceSummary>::TConstIterator It(TypeToSummaryMap); It; ++It )
	{
		const FString& ResourceName				= It.Key();
		const FResourceSummary& ResourceInfo	= It.Value();
	
		Ar.Logf( TEXT("%20s %8i resources consumed %8i KByte physical memory and %8i KByte virtual memory"), 
			*ResourceName,
			ResourceInfo.ResourceCount,
			ResourceInfo.TotalPhysicalSize / 1024,
			ResourceInfo.TotalVirtualSize / 1024 );
	}
#endif
}

#endif // USE_XeD3D_RHI






