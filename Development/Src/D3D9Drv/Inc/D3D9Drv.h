/*=============================================================================
	D3D9Drv.h: Public D3D RHI definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_D3D9DRV
#define _INC_D3D9DRV

// D3D headers.
#pragma pack(push,8)
#define D3D_OVERLOADS 1
// This depends on the environment variable DXSDK_DIR being setup by the DXSDK.
// All projects that use D3D must add DXSDK_DIR/Inc to their include paths
// All projects that link with D3D must add DXSDK_DIR/Lib/x(86|64) to their linker paths
#include <d3d9.h>
#include <d3dx9.h>
#undef DrawText
#pragma pack(pop)

/**
 * Make sure we are compiling against the DXSDK we are expecting to.
 */
const INT REQUIRED_D3DX_SDK_VERSION = 35;
checkAtCompileTime(D3DX_SDK_VERSION == REQUIRED_D3DX_SDK_VERSION, D3DX_SDK_VERSION_DoesNotMatchRequiredVersion);


/** This is a macro that casts a dynamically bound RHI reference to the appropriate D3D type. */
#define DYNAMIC_CAST_D3D9RESOURCE(Type,Name) \
	FD3D9##Type* Name = (FD3D9##Type*)Name##RHI;

// D3D RHI public headers.
#include "D3D9Util.h"
#include "D3D9State.h"
#include "D3D9Resources.h"
#include "D3D9RenderTarget.h"
#include "D3D9Viewport.h"
#include "D3D9MeshUtils.h"

/** A D3D event query resource. */
class FD3D9EventQuery : public FRenderResource
{
public:

	/** Initialization constructor. */
	FD3D9EventQuery(FD3D9DynamicRHI* InD3DRHI):
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
	FD3D9DynamicRHI* D3DRHI;
	TRefCountPtr<IDirect3DQuery9> Query;
};

/** Reuses vertex declarations that are identical. */
class FD3D9VertexDeclarationCache
{
public:

	/** Key used to map a set of vertex element definitions to an IDirect3DVertexDeclaration9 resource */
	class FKey
	{
	public:
		/** Initialization constructor. */
		FKey(const FVertexDeclarationElementList& InElements);

		/**
		* @return TRUE if the decls are the same
		* @param Other - instance to compare against
		*/
		UBOOL operator == (const FKey &Other) const;

		// Accessors.
		D3DVERTEXELEMENT9* GetVertexElements() 
		{ 
			return &VertexElements(0);
		}
		const D3DVERTEXELEMENT9* GetVertexElements() const
		{ 
			return &VertexElements(0);
		}

		/** @return hash value for this type */
		friend DWORD GetTypeHash(const FKey &Key)
		{
			return Key.Hash;
		}

	private:
		/** array of D3D vertex elements */
		TStaticArray<D3DVERTEXELEMENT9,MaxVertexElementCount + 1> VertexElements;
		/** hash value based on vertex elements */
		DWORD Hash;
	};

	/** Initialization constructor. */
	FD3D9VertexDeclarationCache(FD3D9DynamicRHI* InD3DRHI):
		D3DRHI(InD3DRHI)
	{}

	/**
	 * Get a vertex declaration
	 * Tries to find the decl within the set. Creates a new one if it doesn't exist
	 * @param Declaration - key containing the vertex elements
	 * @return D3D vertex decl object
	 */
	FD3D9VertexDeclaration* GetVertexDeclaration(const FKey& Declaration);

private:

	FD3D9DynamicRHI* D3DRHI;

	/** maps key consisting of vertex elements to the D3D decl */
	TMap<FKey, TRefCountPtr<FD3D9VertexDeclaration> > VertexDeclarationMap;
};

/** The interface which is implemented by the dynamically bound RHI. */
class FD3D9DynamicRHI : public FDynamicRHI
{
public:

	friend class FD3D9Viewport;

	/** Initialization constructor. */
	FD3D9DynamicRHI();

	/** Destructor. */
	~FD3D9DynamicRHI();

	/** Reinitializes the D3D device upon a viewport change. */
	void UpdateD3DDeviceFromViewports();

	/**
	 * Reads a D3D query's data into the provided buffer.
	 * @param Query - The D3D query to read data from.
	 * @param Data - The buffer to read the data into.
	 * @param DataSize - The size of the buffer.
	 * @param bWait - If TRUE, it will wait for the query to finish.
	 * @return TRUE if the query finished.
	 */
	UBOOL GetQueryData(IDirect3DQuery9* Query,void* Data,SIZE_T DataSize,UBOOL bWait);

	// The RHI methods are defined as virtual functions in URenderHardwareInterface.
	#define DEFINE_RHIMETHOD(Type,Name,ParameterTypesAndNames,ParameterNames,ReturnStatement,NullImplementation) virtual Type Name ParameterTypesAndNames
	#include "RHIMethods.h"
	#undef DEFINE_RHIMETHOD

	// Reference counting API for the different resource types.
	#define IMPLEMENT_DYNAMICRHI_REFCOUNTING_FORTYPE(Type,ParentType) \
		virtual void AddResourceRef(TDynamicRHIResource<RRT_##Type>* ReferenceRHI) \
		{ \
			DYNAMIC_CAST_D3D9RESOURCE(Type,Reference); \
			Reference->AddRef(); \
		} \
		virtual void RemoveResourceRef(TDynamicRHIResource<RRT_##Type>* ReferenceRHI) \
		{ \
			DYNAMIC_CAST_D3D9RESOURCE(Type,Reference); \
			Reference->Release(); \
		} \
		virtual DWORD GetRefCount(TDynamicRHIResource<RRT_##Type>* ReferenceRHI) \
		{ \
			DYNAMIC_CAST_D3D9RESOURCE(Type,Reference); \
			Reference->AddRef(); \
			return Reference->Release(); \
		}

	ENUM_RHI_RESOURCE_TYPES(IMPLEMENT_DYNAMICRHI_REFCOUNTING_FORTYPE);

	#undef IMPLEMENT_DYNAMICRHI_REFCOUNTING_FORTYPE

	// Accessors.
	IDirect3DDevice9* GetDevice() const
	{
		return Direct3DDevice;
	}

private:

	/** The number of bytes in each shader register. */
	static const UINT NumBytesPerShaderRegister = sizeof(FLOAT) * 4;

	/** The global D3D interface. */
	TRefCountPtr<IDirect3D9> Direct3D;

	/** The global D3D device. */
	TRefCountPtr<IDirect3DDevice9> Direct3DDevice;

	/** The global D3D device's back buffer. */
	TRefCountPtr<FD3D9Surface> BackBuffer;

	/** A list of all viewport RHIs that have been created. */
	TArray<FD3D9Viewport*> Viewports;

	/** The viewport which is currently being drawn. */
	TRefCountPtr<FD3D9Viewport> DrawingViewport;

	/** True if the application has lost D3D device access. */
	UBOOL bDeviceLost;

	/** The width of the D3D device's back buffer. */
	UINT DeviceSizeX;

	/** The height of the D3D device's back buffer. */
	UINT DeviceSizeY;

	/** The window handle associated with the D3D device. */
	HWND DeviceWindow;

	/** True if the D3D device is in fullscreen mode. */
	UBOOL bIsFullscreenDevice;

	/** The capabilities of the D3D device. */
	D3DCAPS9 DeviceCaps;

	/** The number of active vertex streams. */
	INT MaxActiveVertexStreamIndex;

	/** The largest viewport size the application is expecting to need for the time being, or zero for
		no preference.  This is used as a hint to the RHI to reduce redundant device resets when viewports
		are created or destroyed (typically in the editor.)  These values can change at any point, but
		they're usually configured before a viewport is created or destroyed. */
	UINT LargestExpectedViewportWidth;
	UINT LargestExpectedViewportHeight;

	/** Indicates support for Nvidia's Depth Bounds Test through a driver hack in D3D. */
	UBOOL bDepthBoundsHackSupported;

	/** An event used to track the GPU's progress. */
	FD3D9EventQuery FrameSyncEvent;

	/** The vertex declarations that have been created for this device. */
	FD3D9VertexDeclarationCache VertexDeclarationCache;

	/** Information about a vertex stream that's been bound to the D3D device. */
	struct FD3D9Stream
	{
		FD3D9Stream()
		:	Stride(0)
		{}
		FVertexBufferRHIParamRef VertexBuffer;
		UINT Stride;
		UINT NumVerticesPerInstance;
	};

	enum { NumVertexStreams = 16 };

	UINT PendingNumInstances;
	UINT UpdateStreamForInstancingMask;
	FD3D9Stream PendingStreams[NumVertexStreams];

	// Information about pending BeginDraw[Indexed]PrimitiveUP calls.
	UBOOL PendingBegunDrawPrimitiveUP;
	TArray<BYTE> PendingDrawPrimitiveUPVertexData;
	UINT PendingNumVertices;
	UINT PendingVertexDataStride;
	TArray<BYTE> PendingDrawPrimitiveUPIndexData;
	UINT PendingPrimitiveType;
	UINT PendingNumPrimitives;
	UINT PendingMinVertexIndex;
	UINT PendingIndexDataStride;

	/** A history of the most recently used bound shader states, used to keep transient bound shader states from being recreated for each use. */
	TGlobalResource< TBoundShaderStateHistory<1024> > BoundShaderStateHistory;

	/**
	 * Cleanup the D3D device.
	 * This function must be called from the main game thread.
	 */
	void CleanupD3DDevice();
	
	/** Resets the active vertex streams. */
	void ResetVertexStreams();
};

#endif

