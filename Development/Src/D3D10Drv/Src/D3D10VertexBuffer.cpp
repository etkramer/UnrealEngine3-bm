/*=============================================================================
	D3D10VertexBuffer.cpp: D3D vertex buffer RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D10DrvPrivate.h"

FVertexBufferRHIRef FD3D10DynamicRHI::CreateVertexBuffer(UINT Size,FResourceArrayInterface* ResourceArray,EResourceUsageFlag InUsage)
{
	// Explicitly check that the size is nonzero before allowing CreateVertexBuffer to opaquely fail.
	check(Size > 0);

	D3D10_BUFFER_DESC Desc;
	Desc.ByteWidth = Size;
	Desc.Usage = (InUsage != RUF_Static) ? D3D10_USAGE_DYNAMIC : D3D10_USAGE_DEFAULT;
	Desc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	Desc.CPUAccessFlags = (InUsage != RUF_Static) ? D3D10_CPU_ACCESS_WRITE : 0;
	Desc.MiscFlags = 0;

	// If a resource array was provided for the resource, create the resource pre-populated
	D3D10_SUBRESOURCE_DATA InitData;
	D3D10_SUBRESOURCE_DATA* pInitData = NULL;
	if(ResourceArray)
	{
		check(Size == ResourceArray->GetResourceDataSize());
		InitData.pSysMem = ResourceArray->GetResourceData();
		InitData.SysMemPitch = Size;
		InitData.SysMemSlicePitch = 0;
		pInitData = &InitData;
	}

	TRefCountPtr<FD3D10VertexBuffer> VertexBuffer;
	VERIFYD3D10RESULT(Direct3DDevice->CreateBuffer(&Desc,pInitData,(ID3D10Buffer**)VertexBuffer.GetInitReference()));

	if(ResourceArray)
	{
		// Discard the resource array's contents.
		ResourceArray->Discard();
	}

	return VertexBuffer.GetReference();
}

void* FD3D10DynamicRHI::LockVertexBuffer(FVertexBufferRHIParamRef VertexBufferRHI,UINT Offset,UINT Size,UBOOL bReadOnlyInsteadOfWriteOnly)
{
	DYNAMIC_CAST_D3D10RESOURCE(VertexBuffer,VertexBuffer);

	// Determine whether the vertex buffer is dynamic or not.
	D3D10_BUFFER_DESC Desc;
	VertexBuffer->GetDesc(&Desc);
	const UBOOL bIsDynamic = (Desc.Usage == D3D10_USAGE_DYNAMIC);

	FD3D10LockedKey LockedKey(VertexBuffer);
	FD3D10LockedData LockedData;

	if(bIsDynamic)
	{
		check(!bReadOnlyInsteadOfWriteOnly);

		// If the buffer is dynamic, map its memory for writing.
		VERIFYD3D10RESULT(VertexBuffer->Map(D3D10_MAP_WRITE_DISCARD,0,(void**)&LockedData.Data));
		LockedData.Pitch = Size;
	}
	else
	{
		if(bReadOnlyInsteadOfWriteOnly)
		{
			// If the static buffer is being locked for reading, create a staging buffer.
			D3D10_BUFFER_DESC StagingBufferDesc;
			StagingBufferDesc.ByteWidth = Size;
			StagingBufferDesc.Usage = D3D10_USAGE_STAGING;
			StagingBufferDesc.BindFlags = 0;
			StagingBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
			StagingBufferDesc.MiscFlags = 0;
			TRefCountPtr<ID3D10Buffer> StagingVertexBuffer;
			VERIFYD3D10RESULT(Direct3DDevice->CreateBuffer(&StagingBufferDesc,NULL,StagingVertexBuffer.GetInitReference()));
			LockedData.StagingResource = StagingVertexBuffer;

			// Copy the contents of the vertex buffer to the staging buffer.
			Direct3DDevice->CopyResource(StagingVertexBuffer,VertexBuffer);

			// Map the staging buffer's memory for reading.
			VERIFYD3D10RESULT(StagingVertexBuffer->Map(D3D10_MAP_READ,0,(void**)&LockedData.Data));
			LockedData.Pitch = Size;
		}
		else
		{
			// If the static buffer is being locked for writing, allocate memory for the contents to be written to.
			LockedData.Data = (BYTE*)appMalloc(Desc.ByteWidth);
			LockedData.Pitch = Desc.ByteWidth;
		}
	}

	// Add the lock to the lock map.
	OutstandingLocks.Set(LockedKey,LockedData);

	// Return the offset pointer
	return (void*)((BYTE*)LockedData.Data + Offset);
}

void FD3D10DynamicRHI::UnlockVertexBuffer(FVertexBufferRHIParamRef VertexBufferRHI)
{
	DYNAMIC_CAST_D3D10RESOURCE(VertexBuffer,VertexBuffer);

	// Determine whether the vertex buffer is dynamic or not.
	D3D10_BUFFER_DESC Desc;
	VertexBuffer->GetDesc(&Desc);
	const UBOOL bIsDynamic = (Desc.Usage == D3D10_USAGE_DYNAMIC);

	// Find the outstanding lock for this VB.
	FD3D10LockedKey LockedKey(VertexBuffer);
	FD3D10LockedData* LockedData = OutstandingLocks.Find(LockedKey);
	check(LockedData);

	if(bIsDynamic)
	{
		// If the VB is dynamic, its memory was mapped directly; unmap it.
		VertexBuffer->Unmap();
	}
	else
	{
		// If the static VB lock involved a staging resource, it was locked for writing.
		if(LockedData->StagingResource)
		{
			// Unmap the staging buffer's memory.
			ID3D10Buffer* StagingBuffer = (ID3D10Buffer*)LockedData->StagingResource.GetReference();
			StagingBuffer->Unmap();
		}
		else 
		{
			// Copy the contents of the temporary memory buffer allocated for writing into the VB.
			Direct3DDevice->UpdateSubresource(VertexBuffer,LockedKey.Subresource,NULL,LockedData->Data,LockedData->Pitch,0);

			// Free the temporary memory buffer.
			appFree(LockedData->Data);
		}
	}

	// Remove the FD3D10LockedData from the lock map.
	// If the lock involved a staging resource, this releases it.
	OutstandingLocks.Remove(LockedKey);
}

/**
 * Checks if a vertex buffer is still in use by the GPU.
 * @param VertexBuffer - the RHI texture resource to check
 * @return TRUE if the texture is still in use by the GPU, otherwise FALSE
 */
UBOOL FD3D10DynamicRHI::IsBusyVertexBuffer(FVertexBufferRHIParamRef VertexBuffer)
{
	//@TODO: Implement somehow! (could perhaps use D3D10_MAP_FLAG_DO_NOT_WAIT)
	return FALSE;
}
