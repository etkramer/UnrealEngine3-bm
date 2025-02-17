/*=============================================================================
	D3D10Drv.h: Public D3D RHI definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_D3D10DRV
#define _INC_D3D10DRV

// D3D headers.
#pragma pack(push,8)
#define D3D_OVERLOADS 1
// This depends on the environment variable DXSDK_DIR being setup by the DXSDK.
// All projects that use D3D must add DXSDK_DIR/Inc to their include paths
// All projects that link with D3D must add DXSDK_DIR/Lib/x(86|64) to their linker paths
#include <d3d10.h>
#include <d3dx10.h>
#undef DrawText
#pragma pack(pop)

/**
 * Make sure we are compiling against the DXSDK we are expecting to.
 */
const INT REQUIRED_D3DX10_SDK_VERSION = 35;
checkAtCompileTime(D3DX10_SDK_VERSION == REQUIRED_D3DX10_SDK_VERSION, D3DX10_SDK_VERSION_DoesNotMatchRequiredVersion);


/** This is a macro that casts a dynamically bound RHI reference to the appropriate D3D type. */
#define DYNAMIC_CAST_D3D10RESOURCE(Type,Name) \
	FD3D10##Type* Name = (FD3D10##Type*)Name##RHI;

// D3D RHI public headers.
#include "D3D10Util.h"
#include "D3D10State.h"
#include "D3D10Resources.h"
#include "D3D10RenderTarget.h"
#include "D3D10Viewport.h"
#include "D3D10ConstantBuffer.h"
#include "D3D10CachedStates.h"

/** A D3D event query resource. */
class FD3D10EventQuery : public FRenderResource
{
public:

	/** Initialization constructor. */
	FD3D10EventQuery(class FD3D10DynamicRHI* InD3DRHI):
		D3DRHI(InD3DRHI)
	{
	}

	/** Issues an event for the query to poll. */
	void IssueEvent();

	/** Waits for the event query to finish. */
	void WaitForCompletion();

	// FRenderResource interface.
	virtual void InitDynamicRHI();
	virtual void ReleaseDynamicRHI();

private:
	FD3D10DynamicRHI* D3DRHI;
	TRefCountPtr<ID3D10Query> Query;
};

/** The interface which is implemented by the dynamically bound RHI. */
class FD3D10DynamicRHI : public FDynamicRHI
{
public:

	friend class FD3D10Viewport;

	/** Global D3D10 lock list */
	TMap<FD3D10LockedKey,FD3D10LockedData> OutstandingLocks;


	/** Initialization constructor. */
	FD3D10DynamicRHI(IDXGIFactory* InDXGIFactory);

	/** Destructor */
	~FD3D10DynamicRHI();

	/** If it hasn't been initialized yet, initializes the D3D device. */
	void InitD3DDevice();

	/**
	 * Reads a D3D query's data into the provided buffer.
	 * @param Query - The D3D query to read data from.
	 * @param Data - The buffer to read the data into.
	 * @param DataSize - The size of the buffer.
	 * @param bWait - If TRUE, it will wait for the query to finish.
	 * @return TRUE if the query finished.
	 */
	UBOOL GetQueryData(ID3D10Query* Query,void* Data,SIZE_T DataSize,UBOOL bWait);

	// The RHI methods are defined as virtual functions in URenderHardwareInterface.
	#define DEFINE_RHIMETHOD(Type,Name,ParameterTypesAndNames,ParameterNames,ReturnStatement,NullImplementation) virtual Type Name ParameterTypesAndNames
	#include "RHIMethods.h"
	#undef DEFINE_RHIMETHOD

	// Reference counting API for the different resource types.
	#define IMPLEMENT_DYNAMICRHI_REFCOUNTING_FORTYPE(Type,ParentType) \
		virtual void AddResourceRef(TDynamicRHIResource<RRT_##Type>* ReferenceRHI) \
		{ \
			DYNAMIC_CAST_D3D10RESOURCE(Type,Reference); \
			Reference->AddRef(); \
		} \
		virtual void RemoveResourceRef(TDynamicRHIResource<RRT_##Type>* ReferenceRHI) \
		{ \
			DYNAMIC_CAST_D3D10RESOURCE(Type,Reference); \
			Reference->Release(); \
		} \
		virtual DWORD GetRefCount(TDynamicRHIResource<RRT_##Type>* ReferenceRHI) \
		{ \
			DYNAMIC_CAST_D3D10RESOURCE(Type,Reference); \
			Reference->AddRef(); \
			return Reference->Release(); \
		}

	ENUM_RHI_RESOURCE_TYPES(IMPLEMENT_DYNAMICRHI_REFCOUNTING_FORTYPE);

	#undef IMPLEMENT_DYNAMICRHI_REFCOUNTING_FORTYPE

	// Accessors.
	ID3D10Device* GetDevice() const
	{
		return Direct3DDevice;
	}
	IDXGIFactory* GetFactory() const
	{
		return DXGIFactory;
	}

private:
		
	/** The global D3D interface. */
	TRefCountPtr<IDXGIFactory> DXGIFactory;

	/** The global D3D device. */
	TRefCountPtr<ID3D10Device> Direct3DDevice;

	/** A list of all viewport RHIs that have been created. */
	TArray<FD3D10Viewport*> Viewports;

	/** The viewport which is currently being drawn. */
	TRefCountPtr<FD3D10Viewport> DrawingViewport;

	/** True if the device being used has been removed. */
	UBOOL bDeviceRemoved;

	/** The width of the D3D device's back buffer. */
	UINT DeviceSizeX;

	/** The height of the D3D device's back buffer. */
	UINT DeviceSizeY;

	/** The window handle associated with the D3D device. */
	HWND DeviceWindow;

	/** True if the D3D device is in fullscreen mode. */
	UBOOL bIsFullscreenDevice;

	/** True if the device supports MSAA on the necessary formats. */
	UBOOL bMSAAIsSupported;

	/** True if the currently set render target is multisampled. */
	UBOOL bCurrentRenderTargetIsMultisample;

	/** An event used to track the GPU's progress. */
	FD3D10EventQuery FrameSyncEvent;

	/** Keep references to currently bound resources to ensure they're not deleted before a draw call is made. */
	TArray< TRefCountPtr<IUnknown> > PipelineResources;

	/** If a vertex stream with instance data is set, this tracks how many instances it contains. */
	UINT PendingNumInstances;

	// Tracks the currently set state blocks.
	D3D10_DEPTH_STENCIL_DESC CurrentDepthState;
	D3D10_DEPTH_STENCIL_DESC CurrentStencilState;
	D3D10_RASTERIZER_DESC CurrentRasterizerState;
	D3D10_BLEND_DESC CurrentBlendState;
	UINT CurrentStencilRef;
	UBOOL CurrentScissorEnable;
	UINT8 CurrentColorWriteEnable;
	FLinearColor CurrentBlendFactor;

	void *PendingDrawPrimitiveUPVertexData;
	UINT PendingNumVertices;
	UINT PendingVertexDataStride;
	void *StaticData;
	UINT StaticDataSize;

	void *PendingDrawPrimitiveUPIndexData;
	UINT PendingPrimitiveType;
	UINT PendingNumPrimitives;
	UINT PendingMinVertexIndex;
	UINT PendingIndexDataStride;

	// For now have 1 4mb buffer and fill it sequentially.
	enum { NumUserDataBuffers = 1 };
	enum { UserDataBufferSize = 4*1024*1024 };
	TRefCountPtr<ID3D10Buffer> DynamicVertexBufferArray[NumUserDataBuffers];
	TRefCountPtr<ID3D10Buffer> DynamicIndexBufferArray[NumUserDataBuffers];
	UINT CurrentDynamicVB;
	UINT CurrentDynamicIB;
	UINT CurrentVBOffset;
	UINT CurrentIBOffset;

	TMap<FSamplerKey,TRefCountPtr<ID3D10SamplerState> > CachedSamplers;
	TMap<FDepthStencilKey,TRefCountPtr<ID3D10DepthStencilState> > CachedDepthStencilStates;
	TMap<FRasterizerKey,TRefCountPtr<ID3D10RasterizerState> > CachedRasterizerStates;
	TMap<FBlendKey,TRefCountPtr<ID3D10BlendState> > CachedBlendStates;

	/** A list of all D3D constant buffers RHIs that have been created. */
	TArray<TRefCountPtr<FD3D10ConstantBuffer> > VSConstantBuffers;
	TArray<TRefCountPtr<FD3D10ConstantBuffer> > PSConstantBuffers;

	/** A history of the most recently used bound shader states, used to keep transient bound shader states from being recreated for each use. */
	TGlobalResource< TBoundShaderStateHistory<1024> > BoundShaderStateHistory;

	void* CreateVertexDataBuffer(UINT Size);

	void ReleaseDynamicVBandIBBuffers();

	FD3D10Texture2D* CreateD3D10Texture(UINT SizeX,UINT SizeY,UBOOL CubeTexture,BYTE Format,UINT NumMips,DWORD Flags);

	/** Initializes the constant buffers.  Called once at RHI initialization time. */
	void InitConstantBuffers();

	void ReleaseCachedD3D10States();

	/**
	 * Returns an appropriate D3D10 buffer from an array of buffers.  It also makes sure the buffer is of the proper size.
	 * @param Count The number of objects in the buffer
	 * @param Stride The stride of each object
	 * @param Binding Which type of binding (VB or IB) this buffer will need
	 */
	ID3D10Buffer* EnsureBufferSize(UINT Count, UINT Stride, D3D10_BIND_FLAG Binding);

	/**
	 * Fills a D3D10 buffer with the input data.  Returns the D3D10 buffer with the data.
	 * @param Count The number of objects in the buffer
	 * @param Stride The stride of each object
	 * @param Data The actual buffer data
	 * @param Binding Which type of binding (VB or IB) this buffer will need
	 */
	ID3D10Buffer* FillD3D10Buffer(UINT Count, UINT Stride, const void* Data, D3D10_BIND_FLAG Binding, UINT& OutOffset);

	/**
	 * Gets
	 */
	ID3D10SamplerState* GetCachedSamplerState( FSamplerStateRHIParamRef SamplerState );

	ID3D10DepthStencilState* GetCachedDepthStencilState( const D3D10_DEPTH_STENCIL_DESC& DepthState, const D3D10_DEPTH_STENCIL_DESC& StencilState );

	ID3D10RasterizerState* GetCachedRasterizerState( const D3D10_RASTERIZER_DESC& RasterizerState, UBOOL bScissorEnabled, UBOOL bMultisampleEnable );

	ID3D10BlendState* GetCachedBlendState( const D3D10_BLEND_DESC& BlendState, UINT8 EnabledStateValue );

	void CommitVertexAndPixelShaderConstants();

	/**
	 * Cleanup the D3D device.
	 * This function must be called from the main game thread.
	 */
	void CleanupD3DDevice();
};

#endif
