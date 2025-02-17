/*=============================================================================
	D3D10ConstantBuffer.h: Public D3D Constant Buffer definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 * A D3D constant buffer
 */
#define MAX_CONSTANT_BUFFER_SLOTS 4
//#define MAX_CONSTANT_BUFFER_SIZE (1536)
#define MAX_CONSTANT_BUFFER_SIZE (2560)
#define MIN_CONSTANT_BUFFER_SIZE (5*4*4)
#define BONE_CONSTANT_BUFFER_SIZE (75*4*4*3)

#define VS_OFFSET_CONSTANT_BUFFER 1
#define PS_OFFSET_CONSTANT_BUFFER 2
#define VS_BONE_CONSTANT_BUFFER 3

class FD3D10ConstantBuffer : public FRenderResource, public FRefCountedObject
{
public:

	FD3D10ConstantBuffer(FD3D10DynamicRHI* InD3DRHI,UINT InSize = 0,UINT SubBuffers = 1);
	~FD3D10ConstantBuffer();

	// FRenderResource interface.
	virtual void InitDynamicRHI();
	virtual void ReleaseDynamicRHI();

	void UpdateConstant(const BYTE* Data, WORD Offset, WORD Size);
	UBOOL CommitConstantsToDevice();
	ID3D10Buffer* GetConstantBuffer();

private:
	FD3D10DynamicRHI* D3DRHI;
	UINT	MaxSize;
	UBOOL	IsDirty;
	BYTE*	ShadowData;
	TRefCountPtr<ID3D10Buffer>** PoolBuffers;
	UINT	CurrentSubBuffer;
	UINT*	CurrentPoolBuffer;
	UINT	NumSubBuffers;
	UINT	MaxUpdateSize;
};
