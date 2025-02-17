/*=============================================================================
	TextureRenderTarget.cpp: UTextureRenderTarget implementation
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

/*-----------------------------------------------------------------------------
UTextureRenderTarget
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(UTextureRenderTarget);

/**
 * Access the render target resource for this texture target object
 * @return pointer to resource or NULL if not initialized
 */
FTextureRenderTargetResource* UTextureRenderTarget::GetRenderTargetResource()
{
	FTextureRenderTargetResource* Result = NULL;
	if( Resource &&
		Resource->IsInitialized() )
	{
		Result = (FTextureRenderTargetResource*)Resource;
	}
	return Result;
}

/**
 * No mip data used so compression is not needed
 */
void UTextureRenderTarget::Compress()
{
	// don't do anything
}

/**
 * @return newly created FTextureRenderTarget2DResource
 */
FTextureResource* UTextureRenderTarget::CreateResource()
{
	return NULL;
}

/**
 * @return EMaterialValueType for this resource
 */
EMaterialValueType UTextureRenderTarget::GetMaterialType()
{
	return MCT_Texture;
}

/*-----------------------------------------------------------------------------
	FTextureRenderTargetResource
-----------------------------------------------------------------------------*/

/** 
 * Return true if a render target of the given format is allowed
 * for creation
 */
UBOOL FTextureRenderTargetResource::IsSupportedFormat( EPixelFormat Format )
{
	switch( Format )
	{
	case PF_A8R8G8B8:
		return true;
	default:
		return false;
	}
}

/*-----------------------------------------------------------------------------
FDeferredUpdateResource
-----------------------------------------------------------------------------*/

/** 
* Resources can be added to this list if they need a deferred update during scene rendering.
* @return global list of resource that need to be updated. 
*/
TLinkedList<FDeferredUpdateResource*>*& FDeferredUpdateResource::GetUpdateList()
{		
	static TLinkedList<FDeferredUpdateResource*>* FirstUpdateLink = NULL;
	return FirstUpdateLink;
}

/**
* Iterate over the global list of resources that need to
* be updated and call UpdateResource on each one.
*/
void FDeferredUpdateResource::UpdateResources()
{
	if( bNeedsUpdate )
	{
		TLinkedList<FDeferredUpdateResource*>*& UpdateList = FDeferredUpdateResource::GetUpdateList();
		for( TLinkedList<FDeferredUpdateResource*>::TIterator ResourceIt(UpdateList);ResourceIt;ResourceIt.Next() )
		{
			FDeferredUpdateResource* RTResource = *ResourceIt;
			if( RTResource )
			{
				// update each resource
				RTResource->UpdateResource();
				if( RTResource->bOnlyUpdateOnce )
				{
					// remove from list if only a single update was requested
					RTResource->RemoveFromDeferredUpdateList();
				}
			}
		}
		// since the updates should only occur once globally
		// then we need to reset this before rendering any viewports
		bNeedsUpdate = FALSE;
	}
}

/**
* Add this resource to deferred update list
* @param OnlyUpdateOnce - flag this resource for a single update if TRUE
*/
void FDeferredUpdateResource::AddToDeferredUpdateList( UBOOL OnlyUpdateOnce )
{
	UBOOL bExists=FALSE;
	TLinkedList<FDeferredUpdateResource*>*& UpdateList = FDeferredUpdateResource::GetUpdateList();
	for( TLinkedList<FDeferredUpdateResource*>::TIterator ResourceIt(UpdateList);ResourceIt;ResourceIt.Next() )
	{
		if( (*ResourceIt) == this )
		{
			bExists=TRUE;
			break;
		}
	}
	if( !bExists )
	{
		UpdateListLink = TLinkedList<FDeferredUpdateResource*>(this);
		UpdateListLink.Link(UpdateList);
	}
	bOnlyUpdateOnce=OnlyUpdateOnce;
}

/**
* Remove this resource from deferred update list
*/
void FDeferredUpdateResource::RemoveFromDeferredUpdateList()
{
	UpdateListLink.Unlink();
}
