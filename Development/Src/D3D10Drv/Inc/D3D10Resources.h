/*=============================================================================
	D3D10Resources.h: D3D resource RHI definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "BoundShaderStateCache.h"

/**
 * Combined shader state and vertex definition for rendering geometry. 
 * Each unique instance consists of a vertex decl, vertex shader, and pixel shader.
 */
class FD3D10BoundShaderState :
	public FRefCountedObject,
	public TDynamicRHIResource<RRT_BoundShaderState>
{
public:

	FCachedBoundShaderStateLink CacheLink;

	TRefCountPtr<ID3D10InputLayout> InputLayout;
	TRefCountPtr<ID3D10VertexShader> VertexShader;
	TRefCountPtr<ID3D10PixelShader> PixelShader;

	/** Initialization constructor. */
	FD3D10BoundShaderState(
		FVertexDeclarationRHIParamRef InVertexDeclarationRHI,
		DWORD* InStreamStrides,
		FVertexShaderRHIParamRef InVertexShaderRHI,
		FPixelShaderRHIParamRef InPixelShaderRHI,
		ID3D10Device* Direct3DDevice
		);
};

// {F47EE061-065F-4737-AF94-006237CB603C}
DEFINE_GUID(GUID_INDEX_FORMAT,0xf47ee061, 0x65f, 0x4737, 0xaf, 0x94, 0x0, 0x62, 0x37, 0xcb, 0x60, 0x3c);

/** This represents a vertex declaration that hasn't been combined with a specific shader to create a bound shader. */
class FD3D10VertexDeclaration : public FRefCountedObject, public TDynamicRHIResource<RRT_VertexDeclaration>
{
public:

	/** Elements of the vertex declaration. */
	TStaticArray<D3D10_INPUT_ELEMENT_DESC,MaxVertexElementCount> VertexElements;

	/** Initialization constructor. */
	FD3D10VertexDeclaration(const FVertexDeclarationElementList& InElements);
};

/** This represents a vertex shader that hasn't been combined with a specific declaration to create a bound shader. */
class FD3D10VertexShader : public FRefCountedObject, public TDynamicRHIResource<RRT_VertexShader>
{
public:

	/** The vertex shader resource. */
	TRefCountPtr<ID3D10VertexShader> Resource;

	/** The vertex shader's bytecode. */
	TArray<BYTE> Code;

	/** Initialization constructor. */
	FD3D10VertexShader(ID3D10VertexShader* InResource,const TArray<BYTE>& InCode):
		Resource(InResource),
		Code(InCode)
	{}
};

// Textures.
template<ERHIResourceTypes ResourceTypeEnum>
class TD3D10Texture : public FRefCountedObject, public TDynamicRHIResource<ResourceTypeEnum>
{
public:

	/** The D3D texture resource.  Note that a Texture2D may also be an array of homogenous Texture2Ds in D3D10 forming a cubemap. */
	TRefCountPtr<ID3D10Texture2D> Resource;

	/** The view that is used to access the texture from a shader. */
	TRefCountPtr<ID3D10ShaderResourceView> View;

	/** The width of the texture. */
	const UINT SizeX;

	/** The height of texture. */
	const UINT SizeY;

	/** The number of mip-maps in the texture. */
	const UINT NumMips;

	/** The texture's format. */
	EPixelFormat Format;

	/** Whether the texture is a cube-map. */
	const BITFIELD bCubemap : 1;

	/** Initialization constructor. */
	TD3D10Texture(
		class FD3D10DynamicRHI* InD3DRHI,
		ID3D10Texture2D* InResource,
		ID3D10ShaderResourceView* InView,
		UINT InSizeX,
		UINT InSizeY,
		UINT InNumMips,
		EPixelFormat InFormat,
		UBOOL bInCubemap
		)
		: D3DRHI(InD3DRHI)
		, Resource(InResource)
		, View(InView)
		, SizeX(InSizeX)
		, SizeY(InSizeY)
		, NumMips(InNumMips)
		, Format(InFormat)
		, bCubemap(bInCubemap)
	{
	}

	/**
	 * Locks one of the texture's mip-maps.
	 * @return A pointer to the specified texture data.
	 */
	void* Lock(UINT MipIndex,UINT ArrayIndex,UBOOL bIsDataBeingWrittenTo,UINT& DestStride);

	/** Unlocks a previously locked mip-map. */
	void Unlock(UINT MipIndex,UINT ArrayIndex);

private:

	/** The D3D10 RHI that created this texture. */
	FD3D10DynamicRHI* D3DRHI;
};

typedef TD3D10Texture<RRT_Texture>			FD3D10Texture;
typedef TD3D10Texture<RRT_Texture2D>		FD3D10Texture2D;
typedef TD3D10Texture<RRT_TextureCube>		FD3D10TextureCube;
typedef TD3D10Texture<RRT_SharedTexture2D>	FD3D10SharedTexture2D;

/** This represents a vertex shader that hasn't been combined with a specific declaration to create a bound shader. */
class FD3D10OcclusionQuery : public FRefCountedObject, public TDynamicRHIResource<RRT_OcclusionQuery>
{
public:

	/** The query resource. */
	TRefCountPtr<ID3D10Query> Resource;

	/** The cached query result. */
	QWORD Result;

	/** TRUE if the query's result is cached. */
	UBOOL bResultIsCached : 1;

	/** Initialization constructor. */
	FD3D10OcclusionQuery(ID3D10Query* InResource):
		Resource(InResource),
		bResultIsCached(FALSE)
	{}
};

// Resources that directly map to D3D resources.
template<typename D3DResourceType,ERHIResourceTypes ResourceTypeEnum> class TD3DResource : public D3DResourceType, public TDynamicRHIResource<ResourceTypeEnum> {};
typedef TD3DResource<ID3D10PixelShader,RRT_PixelShader>				FD3D10PixelShader;
typedef TD3DResource<ID3D10Buffer,RRT_IndexBuffer>				FD3D10IndexBuffer;
typedef TD3DResource<ID3D10Buffer,RRT_VertexBuffer>			FD3D10VertexBuffer;
typedef TD3DResource<FRefCountedObject,RRT_SharedMemoryResource>	FD3D10SharedMemoryResource;

// Release of dynamic buffers used as a workaround for DIPUP and DUP
void ReleaseDynamicVBandIBBuffers();
