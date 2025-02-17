/*=============================================================================
	D3DIndexBuffer.cpp: D3D Index buffer RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "XeD3DDrvPrivate.h"

#if USE_XeD3D_RHI

/** 
* Sets D3D header and allocates memory for the resource 
*
* @param Stride - size of the index value type
* @param Size - total size of the index buffer in bytes
* @param ResourceArray - optional interface to source data array
*/
void FXeIndexBuffer::AllocIndexBuffer(UINT Stride, UINT Size, FResourceArrayInterface* ResourceArray)
{
	check(Resource);

	// format based on stride size
	D3DFORMAT Format = (Stride == sizeof(WORD) ? D3DFMT_INDEX16 : D3DFMT_INDEX32);
	// set the index buffer header for the driver	
	XGSetIndexBufferHeader( Size, 0, Format, D3DPOOL_DEFAULT, 0, (IDirect3DIndexBuffer9*)Resource );

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
		INT AlignedSize = Align(Size,D3DINDEXBUFFER_ALIGNMENT);
#if TRACK_GPU_RESOURCES
		PhysicalSize = AlignedSize;
#endif
		BaseAddress = appPhysicalAlloc( AlignedSize, CACHE_WriteCombine );
	}
	
#if TRACK_GPU_RESOURCES
	VirtualSize = sizeof(IDirect3DIndexBuffer9);
#endif

	// set data address for this resource
	XGOffsetResourceAddress( Resource, BaseAddress );
}

FIndexBufferRHIRef RHICreateIndexBuffer(UINT Stride,UINT Size,FResourceArrayInterface* ResourceArray,EResourceUsageFlag InUsage)
{
	// Dynamic needs to be rethought!
	// We need some way to communicate the dynamicness to Lock/Unlock, or not allow dynamic VBs like this.

	// create the index buffer resource
	FIndexBufferRHIRef IndexBuffer( new FXeIndexBuffer(InUsage,1) );
	// manually allocate resource and set header
	IndexBuffer->AllocIndexBuffer( Stride, Size, ResourceArray );	

	return IndexBuffer;
}

FIndexBufferRHIRef RHICreateInstancedIndexBuffer(UINT Stride,UINT Size,EResourceUsageFlag InUsage,UINT& OutNumInstances)
{
	// If the index buffer is smaller than 2KB, replicate it to allow drawing multiple instances with one draw command.
	const UINT MinInstancingIndexBufferSize = 2048;
	OutNumInstances = 1;
	if (Size < MinInstancingIndexBufferSize)
	{
		OutNumInstances = (MinInstancingIndexBufferSize * 2) / Size;
		Size *= OutNumInstances;
	}

	// create the index buffer resource
	FIndexBufferRHIRef IndexBuffer( new FXeIndexBuffer(InUsage,OutNumInstances) );
	// manually allocate resource and set header
	IndexBuffer->AllocIndexBuffer( Stride, Size, 0 );	

	return IndexBuffer;
}

void* RHILockIndexBuffer(FIndexBufferRHIParamRef IndexBuffer,UINT Offset,UINT Size)
{
	void* Data = NULL;
	check(Offset>=0);	

	if ((IndexBuffer->UsageFlag != RUF_Static) &&
		(IndexBuffer->bUsesResourceArray == FALSE) &&
		(Offset == 0) && 
		(IndexBuffer->bHasBeenLocked != 0))
	{
		// Read the index buffer description to get the size...
		D3DINDEXBUFFER_DESC IndexBufferDesc;
		VERIFYD3DRESULT(((IDirect3DIndexBuffer9*)(IndexBuffer->Resource))->GetDesc(&IndexBufferDesc));

		// Free the memory
		if (IndexBuffer->BaseAddress)
		{
			AddUnusedXeResource(NULL, IndexBuffer->BaseAddress, IndexBuffer->bIsTexture, IndexBuffer->bUsesResourceArray);
		}

		// Re-allocate the buffer
		IndexBuffer->AllocIndexBuffer(
			(IndexBufferDesc.Format == D3DFMT_INDEX16) ? sizeof(WORD) : sizeof(DWORD), 
			IndexBufferDesc.Size, NULL);
	}

	IDirect3DIndexBuffer9* D3DIndexBufferPtr = (IDirect3DIndexBuffer9*) IndexBuffer->Resource;
	// range locking not supported on Xe so offset=0,size=0
	VERIFYD3DRESULT(D3DIndexBufferPtr->Lock(0,0,&Data,0));

	// Set the flag indicating this has been touched
	IndexBuffer->bHasBeenLocked = 1;

	// if an offset>0 was specified then just return the position into the buffer
	return (BYTE*)Data + Offset;
}

void RHIUnlockIndexBuffer(FIndexBufferRHIParamRef IndexBuffer)
{
	IDirect3DIndexBuffer9* D3DIndexBufferPtr = (IDirect3DIndexBuffer9*) IndexBuffer->Resource;
	VERIFYD3DRESULT(D3DIndexBufferPtr->Unlock());
}


#endif
