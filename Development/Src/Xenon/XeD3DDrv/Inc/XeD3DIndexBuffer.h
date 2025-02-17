/*=============================================================================
	XeD3DIndexBuffer.h: XeD3D index buffer RHI definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if USE_XeD3D_RHI

/** index buffer resource */
class FXeIndexBuffer : public FXeGPUResource
{
public:

	DWORD NumInstances;

	/** default constructor */
	FXeIndexBuffer(EResourceUsageFlag InUsageFlag,DWORD InNumInstances)
	:	FXeGPUResource(new IDirect3DIndexBuffer9, InUsageFlag)
	,	NumInstances(InNumInstances)
	{
	}

	/** constructor initialized w/ existing D3D resource */
	FXeIndexBuffer(IDirect3DIndexBuffer9* InResource, EResourceUsageFlag InUsageFlag,DWORD InNumInstances)
	:	FXeGPUResource(InResource, InUsageFlag)
	,	NumInstances(InNumInstances)
	{
	}

	/** 
	 * Sets D3D header and allocates memory for the resource 
	 *
	 * @param Stride - size of the index value type.
	 * @param Size - total size of the index buffer in bytes.
	 * @param ResourceArray - optional interface to source data array.
	 */
	void AllocIndexBuffer(UINT Stride, UINT Size, FResourceArrayInterface* ResourceArray);

	/**
	 * @return Resource type name.
	 */
	virtual TCHAR* GetTypeName() const
	{
		return TEXT("IndexBuffer");
	}
};

/*-----------------------------------------------------------------------------
RHI index buffer type
-----------------------------------------------------------------------------*/
typedef TXeGPUResourceRef<IDirect3DIndexBuffer9,FXeIndexBuffer> FIndexBufferRHIRef;
typedef FXeIndexBuffer* FIndexBufferRHIParamRef;

#endif

