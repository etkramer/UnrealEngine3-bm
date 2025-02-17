/*=============================================================================
	D3D10ConstantBuffer.cpp: D3D Constant buffer RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D10DrvPrivate.h"

// SHANOND: Looks like UpdateSubresource is going to be the way to update these CBs.
//			The driver writers are trying to optimize for UpdateSubresource and this
//			will also avoid any driver renaming issues we may hit with map_discard.

//#define _USE_MAP_DISCARD_ //Undef this to use updatesubresource
#if defined(_USE_MAP_DISCARD_)
#define MAX_POOL_BUFFERS 384 // use more constant buffers for driver renaming
#else
#define MAX_POOL_BUFFERS 1 // use update subresource and plop it into the command stream
#endif

FD3D10ConstantBuffer::FD3D10ConstantBuffer(FD3D10DynamicRHI* InD3DRHI,UINT InSize,UINT SubBuffers) : 
	D3DRHI(InD3DRHI),
	MaxSize(InSize),
	IsDirty(FALSE),
	ShadowData(NULL),
	PoolBuffers(NULL),
	CurrentSubBuffer(0),
	CurrentPoolBuffer(NULL),
	NumSubBuffers(SubBuffers),
	MaxUpdateSize(0)
{
	InitResource();
}

FD3D10ConstantBuffer::~FD3D10ConstantBuffer()
{
	ReleaseResource();
}

/**
* Creates a constant buffer on the device
*/
void FD3D10ConstantBuffer::InitDynamicRHI()
{
	TRefCountPtr<ID3D10Buffer> CBuffer = NULL;
	D3D10_BUFFER_DESC BufferDesc;
	BufferDesc.ByteWidth = MaxSize;

#if defined(_USE_MAP_DISCARD_)
	BufferDesc.Usage = D3D10_USAGE_DYNAMIC;
	BufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
#else
	BufferDesc.Usage = D3D10_USAGE_DEFAULT;
	BufferDesc.CPUAccessFlags = 0;
#endif
	BufferDesc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
	BufferDesc.MiscFlags = 0;

	PoolBuffers = new TRefCountPtr<ID3D10Buffer>*[NumSubBuffers];
	CurrentPoolBuffer = new UINT[NumSubBuffers];
	appMemzero(CurrentPoolBuffer,sizeof(UINT)*NumSubBuffers);
	CurrentSubBuffer = 0;
	for(UINT s = 0;s < NumSubBuffers;s++)
	{
		PoolBuffers[s] = new TRefCountPtr<ID3D10Buffer>[MAX_POOL_BUFFERS];
		appMemzero(PoolBuffers[s],sizeof(TRefCountPtr<ID3D10Buffer>)*MAX_POOL_BUFFERS);
		for(UINT i = 0;i < MAX_POOL_BUFFERS;i++)
		{
			VERIFYD3D10RESULT(D3DRHI->GetDevice()->CreateBuffer(&BufferDesc, NULL, PoolBuffers[s][i].GetInitReference()));
		}

		BufferDesc.ByteWidth = BufferDesc.ByteWidth / 2;
	}

	ShadowData = new BYTE[MaxSize];
	appMemzero(ShadowData,MaxSize);
}

void FD3D10ConstantBuffer::ReleaseDynamicRHI()
{
	if(ShadowData)
		delete []ShadowData;

	if( CurrentPoolBuffer )
		delete []CurrentPoolBuffer;

	if(PoolBuffers)
	{
		for(UINT s = 0;s < NumSubBuffers;s++)
		{
			delete []PoolBuffers[s];
		}

		delete []PoolBuffers;
	}
}

/**
* Updates a variable in the constant buffer.
* @param Data - The data to copy into the constant buffer
* @param Offset - The offset in the constant buffer to place the data at
* @param Size - The size of the data being copied
*/
void FD3D10ConstantBuffer::UpdateConstant(const BYTE* Data, WORD Offset, WORD InSize)
{
	IsDirty = TRUE;
	appMemcpy(ShadowData+Offset, Data, InSize);
	MaxUpdateSize = Max( (UINT)(Offset + InSize), MaxUpdateSize );
}

/**
* Unlocks the constant buffer so the data can be transmitted to the device
*/
UBOOL FD3D10ConstantBuffer::CommitConstantsToDevice()
{
	if(IsDirty)
	{
		SCOPE_CYCLE_COUNTER(STAT_D3D10ConstantBufferUpdateTime);

		UINT Size = MaxSize;
		CurrentSubBuffer = 0;

		while( Size >= MaxUpdateSize && CurrentSubBuffer < NumSubBuffers )
		{
			CurrentSubBuffer ++;
			Size /= 2;
		}

		CurrentSubBuffer --;
		Size *= 2;

		CurrentPoolBuffer[CurrentSubBuffer] ++;
		if(CurrentPoolBuffer[CurrentSubBuffer] >= MAX_POOL_BUFFERS)
			CurrentPoolBuffer[CurrentSubBuffer] = 0;

		ID3D10Buffer* Buffer = PoolBuffers[CurrentSubBuffer][CurrentPoolBuffer[CurrentSubBuffer]];

#if defined(_USE_MAP_DISCARD_)
		BYTE* MappedData = NULL;
		VERIFYD3D10RESULT(Buffer->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**)&MappedData));
		appMemcpy(MappedData,ShadowData,Size);
		Buffer->Unmap();
#else
		D3DRHI->GetDevice()->UpdateSubresource(Buffer,0,NULL,(void*)ShadowData,Size,Size);
#endif

		IsDirty = FALSE;
		MaxUpdateSize = 0;

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
* Get the current pool buffer
*/
ID3D10Buffer* FD3D10ConstantBuffer::GetConstantBuffer()
{
	return PoolBuffers[CurrentSubBuffer][CurrentPoolBuffer[CurrentSubBuffer]];
}

void FD3D10DynamicRHI::InitConstantBuffers()
{
	// Allocate vertex shader constant buffers.
	VSConstantBuffers.Empty(MAX_CONSTANT_BUFFER_SLOTS);
	for(INT BufferIndex = 0;BufferIndex < MAX_CONSTANT_BUFFER_SLOTS;BufferIndex++)
	{
		UINT Size;
		UINT SubBuffers;
		if(BufferIndex == 0)
		{
			Size = MAX_CONSTANT_BUFFER_SIZE;
			SubBuffers = 6;
		}
		else if(BufferIndex == VS_BONE_CONSTANT_BUFFER)
		{
			Size = BONE_CONSTANT_BUFFER_SIZE;
			SubBuffers = 1;
		}
		else
		{
			Size = MIN_CONSTANT_BUFFER_SIZE;
			SubBuffers = 1;
		}
		VSConstantBuffers.AddItem(new FD3D10ConstantBuffer(this,Size,SubBuffers));
	}

	// Allocate pixel shader constant buffers.
	PSConstantBuffers.Empty(MAX_CONSTANT_BUFFER_SLOTS);
	for(INT BufferIndex = 0;BufferIndex < MAX_CONSTANT_BUFFER_SLOTS;BufferIndex++)
	{
		const UINT Size = BufferIndex == 0 ? MAX_CONSTANT_BUFFER_SIZE : MIN_CONSTANT_BUFFER_SIZE;
		PSConstantBuffers.AddItem(new FD3D10ConstantBuffer(this,Size));
	}
}
