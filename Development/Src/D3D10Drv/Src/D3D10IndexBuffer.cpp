/*=============================================================================
	D3D10IndexBuffer.cpp: D3D Index buffer RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D10DrvPrivate.h"

EXTERN_C const GUID GUID_INDEX_FORMAT = { 0xf47ee061, 0x65f, 0x4737, { 0xaf, 0x94, 0x0, 0x62, 0x37, 0xcb, 0x60, 0x3c } };

FIndexBufferRHIRef FD3D10DynamicRHI::CreateIndexBuffer(UINT Stride,UINT Size,FResourceArrayInterface* ResourceArray,EResourceUsageFlag InUsage)
{
	// Explicitly check that the size is nonzero before allowing CreateIndexBuffer to opaquely fail.
	check(Size > 0);

	// Describe the index buffer.
	D3D10_BUFFER_DESC Desc;
	Desc.ByteWidth = Size;
	Desc.Usage = (InUsage != RUF_Static) ? D3D10_USAGE_DYNAMIC : D3D10_USAGE_DEFAULT;
	Desc.BindFlags = D3D10_BIND_INDEX_BUFFER;
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

	TRefCountPtr<FD3D10IndexBuffer> IndexBuffer;
	VERIFYD3D10RESULT(Direct3DDevice->CreateBuffer(&Desc,pInitData,(ID3D10Buffer**)IndexBuffer.GetInitReference()));

	DXGI_FORMAT Format = (Stride == sizeof(WORD) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
	IndexBuffer->SetPrivateData(GUID_INDEX_FORMAT,sizeof(DXGI_FORMAT),&Format);

	if(ResourceArray)
	{
		// Discard the resource array's contents.
		ResourceArray->Discard();
	}

	return IndexBuffer.GetReference();
}

FIndexBufferRHIRef FD3D10DynamicRHI::CreateInstancedIndexBuffer(UINT Stride,UINT Size,EResourceUsageFlag InUsage,UINT& OutNumInstances)
{
	// PC never needs extra instances in the index buffer
	OutNumInstances = 1;
	return CreateIndexBuffer(Stride,Size,0,InUsage);
}

void* FD3D10DynamicRHI::LockIndexBuffer(FIndexBufferRHIParamRef IndexBufferRHI,UINT Offset,UINT Size)
{
	DYNAMIC_CAST_D3D10RESOURCE(IndexBuffer,IndexBuffer);

	D3D10_BUFFER_DESC Desc;
	IndexBuffer->GetDesc(&Desc);
	void* Data = NULL;

	if(Desc.Usage == D3D10_USAGE_DEFAULT)
	{
		FD3D10LockedKey LockedKey(IndexBuffer);
		FD3D10LockedData LockedData;
		LockedData.Data = (BYTE*)appMalloc(Desc.ByteWidth);
		LockedData.Pitch = Desc.ByteWidth;
		Data = LockedData.Data;

		OutstandingLocks.Set(LockedKey,LockedData);
	}
	else
	{
		VERIFYD3D10RESULT(IndexBuffer->Map(D3D10_MAP_WRITE_DISCARD,0,&Data));
	}

	// Return the offset pointer
	return (void*)((BYTE*)Data + Offset);
}

void FD3D10DynamicRHI::UnlockIndexBuffer(FIndexBufferRHIParamRef IndexBufferRHI)
{
	DYNAMIC_CAST_D3D10RESOURCE(IndexBuffer,IndexBuffer);

	D3D10_BUFFER_DESC Desc;
	IndexBuffer->GetDesc(&Desc);

	if(Desc.Usage == D3D10_USAGE_DEFAULT)
	{
		// Find the lock
		FD3D10LockedKey LockedKey(IndexBuffer);
		FD3D10LockedData* LockedData = OutstandingLocks.Find(LockedKey);

		// Update the buffer
		Direct3DDevice->UpdateSubresource(IndexBuffer,LockedKey.Subresource,NULL,LockedData->Data,LockedData->Pitch,0);
		appFree(LockedData->Data);

		// remove the FD3D10LockedData from the lock list
		OutstandingLocks.Remove(LockedKey);
	}
	else
	{
		IndexBuffer->Unmap();
	}
}
