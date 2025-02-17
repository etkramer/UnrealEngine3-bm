/*=============================================================================
	D3DVertexBuffer.cpp: D3D vertex buffer RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "XeD3DDrvPrivate.h"

#if USE_XeD3D_RHI

/** 
* Sets D3D header and allocates memory for the resource 
*
* @param Size - total size of the vertex buffer in bytes
* @param ResourceArray - optional interface to source data array
*/
void FXeVertexBuffer::AllocVertexBuffer(UINT Size, FResourceArrayInterface* ResourceArray)
{
	check(Resource);

	// set the vertex buffer header for the driver	
	XGSetVertexBufferHeader( Size, 0, 0, 0, (IDirect3DVertexBuffer9*)Resource );
	
	if( ResourceArray )
	{
		// use existing resource array memory directly
		BaseAddress = (void*)ResourceArray->GetResourceData();
		check(BaseAddress);
		check(Size == ResourceArray->GetResourceDataSize());
#if TRACK_GPU_RESOURCES
		PhysicalSize = Size;
#endif
		bUsesResourceArray = TRUE;
	}
	else
	{
		// allocate memory for vertex data
		INT AlignedSize = Align(Size,D3DVERTEXBUFFER_ALIGNMENT);
#if TRACK_GPU_RESOURCES
		PhysicalSize = AlignedSize;
#endif
		BaseAddress = appPhysicalAlloc( AlignedSize, CACHE_WriteCombine );
	}
	
#if TRACK_GPU_RESOURCES
	VirtualSize = sizeof(IDirect3DVertexBuffer9);
#endif

	// set data address for this resource
	XGOffsetResourceAddress( Resource, BaseAddress );
}

FVertexBufferRHIRef RHICreateVertexBuffer(UINT Size,FResourceArrayInterface* ResourceArray,EResourceUsageFlag InUsage)
{
	// create the vertex buffer resource
	FVertexBufferRHIRef VertexBuffer( new FXeVertexBuffer(InUsage) );
	// manually allocate resource and set header
	VertexBuffer->AllocVertexBuffer( Size, ResourceArray );

	return FVertexBufferRHIRef(VertexBuffer);

}

void* RHILockVertexBuffer(FVertexBufferRHIParamRef VertexBuffer,UINT Offset,UINT Size,UBOOL bReadOnlyInsteadOfWriteOnly)
{
	void* Data = NULL;
	IDirect3DVertexBuffer9* D3DVertexBufferPtr = (IDirect3DVertexBuffer9*) VertexBuffer->Resource;

	// Read the vertex buffer description.
	D3DVERTEXBUFFER_DESC VertexBufferDesc;
	VERIFYD3DRESULT(D3DVertexBufferPtr->GetDesc(&VertexBufferDesc));

	if ((VertexBuffer->UsageFlag != RUF_Static) && 
		(Offset == 0) && 
		(VertexBuffer->bHasBeenLocked != 0))
	{
		// Free the memory
		if (VertexBuffer->BaseAddress)
		{
			AddUnusedXeResource(NULL, VertexBuffer->BaseAddress, VertexBuffer->bIsTexture, VertexBuffer->bUsesResourceArray);
		}

		// Re-allocate the buffer
		VertexBuffer->AllocVertexBuffer(VertexBufferDesc.Size, NULL);
	}

	// Lock the whole vertex buffer.
	const DWORD LockFlags = bReadOnlyInsteadOfWriteOnly ? D3DLOCK_READONLY : 0;
	VERIFYD3DRESULT(D3DVertexBufferPtr->Lock(0,VertexBufferDesc.Size,&Data,LockFlags));

	// Set the flag indicating this has been touched
	VertexBuffer->bHasBeenLocked = 1;

	// Return a pointer to the desired offset in the buffer.
	return (BYTE*)Data + Offset;
}

void RHIUnlockVertexBuffer(FVertexBufferRHIParamRef VertexBuffer)
{
	IDirect3DVertexBuffer9* D3DVertexBufferPtr = (IDirect3DVertexBuffer9*) VertexBuffer->Resource;
	VERIFYD3DRESULT(D3DVertexBufferPtr->Unlock());
}

/**
 * Checks if a vertex buffer is still in use by the GPU.
 * @param VertexBuffer - the RHI texture resource to check
 * @return TRUE if the texture is still in use by the GPU, otherwise FALSE
 */
UBOOL RHIIsBusyVertexBuffer(FVertexBufferRHIParamRef VertexBuffer)
{
	check( VertexBuffer );
	IDirect3DVertexBuffer9* D3DVertexBufferPtr = (IDirect3DVertexBuffer9*) VertexBuffer->Resource;
	return D3DVertexBufferPtr && (D3DVertexBufferPtr->IsSet(GDirect3DDevice) || D3DVertexBufferPtr->IsBusy());
}

#endif
