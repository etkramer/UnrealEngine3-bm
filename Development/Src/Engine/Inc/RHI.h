/*=============================================================================
	RHI.h: Render Hardware Interface definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __RHI_H__
#define __RHI_H__

#include "UnBuild.h"

/*
Platform independent RHI definitions.
The use of non-member functions to operate on resource references allows two different approaches to RHI implementation:
- Resource references are to RHI data structures, which directly contain the platform resource data.
- Resource references are to the platform's HAL representation of the resource.  This eliminates a layer of indirection for platforms such as
	D3D where the RHI doesn't directly store resource data.
*/

//
// RHI globals.
//

/** True if the render hardware has been initialized. */
extern UBOOL GIsRHIInitialized;

//
// RHI capabilities.
//

/** Maximum number of miplevels in a texture. */
enum { MAX_TEXTURE_MIP_COUNT = 13 };

/** The maximum number of vertex elements which can be used by a vertex declaration. */
enum { MaxVertexElementCount = 16 };

/** The alignment in bytes between elements of array shader parameters. */
enum { ShaderArrayElementAlignBytes = 16 };

/** The maximum number of mip-maps that a texture can contain. 	*/
extern	INT		GMaxTextureMipCount;
/** The minimum number of mip-maps that always remain resident.	*/
extern 	INT		GMinTextureResidentMipCount;

/** TRUE if PF_DepthStencil textures can be created and sampled. */
extern UBOOL GSupportsDepthTextures;

/** 
* TRUE if PF_DepthStencil textures can be created and sampled to obtain PCF values. 
* This is different from GSupportsDepthTextures in three ways:
*	-results of sampling are PCF values, not depths
*	-a color target must be bound with the depth stencil texture even if never written to or read from,
*		due to API restrictions
*	-a dedicated resolve surface may or may not be necessary
*/
extern UBOOL GSupportsHardwarePCF;

/** TRUE if D24 textures can be created and sampled, retrieving 4 neighboring texels in a single lookup. */
extern UBOOL GSupportsFetch4;

/**
* TRUE if floating point filtering is supported
*/
extern UBOOL GSupportsFPFiltering;

/** Can we handle quad primitives? */
extern UBOOL GSupportsQuads;

/** Are we using an inverted depth buffer? Viewport MinZ/MaxZ reversed */
extern UBOOL GUsesInvertedZ;

/** The offset from the upper left corner of a pixel to the position which is used to sample textures for fragments of that pixel. */
extern FLOAT GPixelCenterOffset;

/** The maximum size to allow for the shadow depth buffer. */
extern INT GMaxShadowDepthBufferSize;

/** Bias exponent used to apply to shader color output when rendering to the scene color surface */
extern INT GCurrentColorExpBias;

/** Toggle for MSAA tiling support */
extern UBOOL GUseTilingCode;

/** Enables the use of untiled MSAA for horizontal split screen */
extern UBOOL GUseMSAASplitScreen;

/**
 *	The size to check against for Draw*UP call vertex counts.
 *	If greater than this value, the draw call will not occur.
 */
extern INT GDrawUPVertexCheckCount;
/**
 *	The size to check against for Draw*UP call index counts.
 *	If greater than this value, the draw call will not occur.
 */
extern INT GDrawUPIndexCheckCount;

/** TRUE if the rendering hardware supports vertex instancing. */
extern UBOOL GSupportsVertexInstancing;

/** TRUE if the rendering hardware can emulate vertex instancing. */
extern UBOOL GSupportsEmulatedVertexInstancing;

/** If FALSE code needs to patch up vertex declaration. */
extern UBOOL GVertexElementsCanShareStreamOffset;

/** if TRUE, two paired textures can share a base address, overlapping their mip memory. */
extern UBOOL GTexturesCanShareMipMemory;

/** TRUE for each VET that is supported. One-to-one mapping with EVertexElementType */
extern class FVertexElementTypeSupportInfo GVertexElementTypeSupport;

/** Controls whether to use emission data from alpha when modulating shadows instead of rendering a mask */
extern UBOOL GModShadowsWithAlphaEmissiveBit;

//
// Common RHI definitions.
//

enum ESamplerFilter
{
	SF_Point,
	SF_Bilinear,
	SF_Trilinear,
	SF_AnisotropicPoint,
	SF_AnisotropicLinear,
};

enum ESamplerAddressMode
{
	AM_Wrap,
	AM_Clamp,
	AM_Mirror
};

enum ESamplerMipMapLODBias
{
	MIPBIAS_None,
	MIPBIAS_Get4
};

enum ERasterizerFillMode
{
	FM_Point,
	FM_Wireframe,
	FM_Solid
};

enum ERasterizerCullMode
{
	CM_None,
	CM_CW,
	CM_CCW
};

enum EColorWriteMask
{
	CW_RED		= 0x01,
	CW_GREEN	= 0x02,
	CW_BLUE		= 0x04,
	CW_ALPHA	= 0x08,

	CW_RGB		= 0x07,
	CW_RGBA		= 0x0f,
};

enum ECompareFunction
{
	CF_Less,
	CF_LessEqual,
	CF_Greater,
	CF_GreaterEqual,
	CF_Equal,
	CF_NotEqual,
	CF_Never,
	CF_Always
};

enum EStencilOp
{
	SO_Keep,
	SO_Zero,
	SO_Replace,
	SO_SaturatedIncrement,
	SO_SaturatedDecrement,
	SO_Invert,
	SO_Increment,
	SO_Decrement
};

enum EBlendOperation
{
	BO_Add,
	BO_Subtract,
	BO_Min,
	BO_Max
};

enum EBlendFactor
{
	BF_Zero,
	BF_One,
	BF_SourceColor,
	BF_InverseSourceColor,
	BF_SourceAlpha,
	BF_InverseSourceAlpha,
	BF_DestAlpha,
	BF_InverseDestAlpha,
	BF_DestColor,
	BF_InverseDestColor
};

enum EVertexElementType
{
	VET_None,
	VET_Float1,
	VET_Float2,
	VET_Float3,
	VET_Float4,
	VET_PackedNormal,	// FPackedNormal
	VET_UByte4,
	VET_UByte4N,
	VET_Color,
	VET_Short2,
	VET_Short2N,		// 16 bit word normalized to (value/32767.0,value/32767.0,0,0,1)
	VET_Half2,			// 16 bit float using 1 bit sign, 5 bit exponent, 10 bit mantissa 
	VET_MAX
};

enum EVertexElementUsage
{
	VEU_Position,
	VEU_TextureCoordinate,
	VEU_BlendWeight,
	VEU_BlendIndices,
	VEU_Normal,
	VEU_Tangent,
	VEU_Binormal,
	VEU_Color
};

enum ECubeFace
{
	CubeFace_PosX=0,
	CubeFace_NegX,
	CubeFace_PosY,
	CubeFace_NegY,
	CubeFace_PosZ,
	CubeFace_NegZ,
	CubeFace_MAX
};

/** Info for supporting the vertex element types */
class FVertexElementTypeSupportInfo
{
public:
	FVertexElementTypeSupportInfo() { for(INT i=0; i<VET_MAX; i++) ElementCaps[i]=TRUE; }
	FORCEINLINE UBOOL IsSupported(EVertexElementType ElementType) { return ElementCaps[ElementType]; }
	FORCEINLINE void SetSupported(EVertexElementType ElementType,UBOOL bIsSupported) { ElementCaps[ElementType]=bIsSupported; }
private:
	/** cap bit set for each VET. One-to-one mapping based on EVertexElementType */
	UBOOL ElementCaps[VET_MAX];
};

struct FVertexElement
{
	BYTE StreamIndex;
	BYTE Offset;
	BYTE Type;
	BYTE Usage;
	BYTE UsageIndex;
	UBOOL bUseInstanceIndex;
	UINT NumVerticesPerInstance;

	FVertexElement() {}
	FVertexElement(BYTE InStreamIndex,BYTE InOffset,BYTE InType,BYTE InUsage,BYTE InUsageIndex,UBOOL bInUseInstanceIndex = FALSE,UINT InNumVerticesPerInstance = 0):
		StreamIndex(InStreamIndex),
		Offset(InOffset),
		Type(InType),
		Usage(InUsage),
		UsageIndex(InUsageIndex),
		bUseInstanceIndex(bInUseInstanceIndex),
		NumVerticesPerInstance(InNumVerticesPerInstance)
	{}
	/**
	* Suppress the compiler generated assignment operator so that padding won't be copied.
	* This is necessary to get expected results for code that zeros, assigns and then CRC's the whole struct.
	*/
	void operator=(const FVertexElement& Other)
	{
		StreamIndex = Other.StreamIndex;
		Offset = Other.Offset;
		Type = Other.Type;
		Usage = Other.Usage;
		UsageIndex = Other.UsageIndex;
		bUseInstanceIndex = Other.bUseInstanceIndex;
		NumVerticesPerInstance = Other.NumVerticesPerInstance;
	}
};

typedef TStaticArray<FVertexElement,MaxVertexElementCount> FVertexDeclarationElementList;

struct FSamplerStateInitializerRHI
{
	ESamplerFilter Filter;
	ESamplerAddressMode AddressU;
	ESamplerAddressMode AddressV;
	ESamplerAddressMode AddressW;
	ESamplerMipMapLODBias MipBias;
};
struct FRasterizerStateInitializerRHI
{
	ERasterizerFillMode FillMode;
	ERasterizerCullMode CullMode;
	FLOAT DepthBias;
	FLOAT SlopeScaleDepthBias;
};
struct FDepthStateInitializerRHI
{
	UBOOL bEnableDepthWrite;
	ECompareFunction DepthTest;
};

struct FStencilStateInitializerRHI
{
	FStencilStateInitializerRHI(
		UBOOL bInEnableFrontFaceStencil = FALSE,
		ECompareFunction InFrontFaceStencilTest = CF_Always,
		EStencilOp InFrontFaceStencilFailStencilOp = SO_Keep,
		EStencilOp InFrontFaceDepthFailStencilOp = SO_Keep,
		EStencilOp InFrontFacePassStencilOp = SO_Keep,
		UBOOL bInEnableBackFaceStencil = FALSE,
		ECompareFunction InBackFaceStencilTest = CF_Always,
		EStencilOp InBackFaceStencilFailStencilOp = SO_Keep,
		EStencilOp InBackFaceDepthFailStencilOp = SO_Keep,
		EStencilOp InBackFacePassStencilOp = SO_Keep,
		DWORD InStencilReadMask = 0xFFFFFFFF,
		DWORD InStencilWriteMask = 0xFFFFFFFF,
		DWORD InStencilRef = 0) :
		bEnableFrontFaceStencil(bInEnableFrontFaceStencil),
		FrontFaceStencilTest(InFrontFaceStencilTest),
		FrontFaceStencilFailStencilOp(InFrontFaceStencilFailStencilOp),
		FrontFaceDepthFailStencilOp(InFrontFaceDepthFailStencilOp),
		FrontFacePassStencilOp(InFrontFacePassStencilOp),
		bEnableBackFaceStencil(bInEnableBackFaceStencil),
		BackFaceStencilTest(InBackFaceStencilTest),
		BackFaceStencilFailStencilOp(InBackFaceStencilFailStencilOp),
		BackFaceDepthFailStencilOp(InBackFaceDepthFailStencilOp),
		BackFacePassStencilOp(InBackFacePassStencilOp),
		StencilReadMask(InStencilReadMask),
		StencilWriteMask(InStencilWriteMask),
		StencilRef(InStencilRef)
	{
	}

	UBOOL bEnableFrontFaceStencil;
	ECompareFunction FrontFaceStencilTest;
	EStencilOp FrontFaceStencilFailStencilOp;
	EStencilOp FrontFaceDepthFailStencilOp;
	EStencilOp FrontFacePassStencilOp;
	UBOOL bEnableBackFaceStencil;
	ECompareFunction BackFaceStencilTest;
	EStencilOp BackFaceStencilFailStencilOp;
	EStencilOp BackFaceDepthFailStencilOp;
	EStencilOp BackFacePassStencilOp;
	DWORD StencilReadMask;
	DWORD StencilWriteMask;
	DWORD StencilRef;
};
struct FBlendStateInitializerRHI
{
	EBlendOperation ColorBlendOperation;
	EBlendFactor ColorSourceBlendFactor;
	EBlendFactor ColorDestBlendFactor;
	EBlendOperation AlphaBlendOperation;
	EBlendFactor AlphaSourceBlendFactor;
	EBlendFactor AlphaDestBlendFactor;
	ECompareFunction AlphaTest;
	BYTE AlphaRef;
};

enum EPrimitiveType
{
	PT_TriangleList = 0,
	PT_TriangleStrip = 1,
	PT_LineList = 2,
	PT_QuadList = 3,
	PT_TessellatedQuadPatch = 4,	// Xbox-specific

	PT_NumBits = 3
};

enum EParticleEmitterType
{
	PET_None = 0,		// Not a particle emitter
	PET_Sprite = 1,		// Sprite particle emitter
	PET_SubUV = 2,		// SubUV particle emitter
	PET_Mesh = 3,		// Mesh emitter

	PET_NumBits = 2
};

// Pixel shader constants that are reserved by the Engine.
enum EPixelShaderRegisters
{
	PSR_ColorBiasFactor = 0,			// Factor applied to the color output from the pixelshader
	PSR_ScreenPositionScaleBias = 1,	// Converts projection-space XY coordinates to texture-space UV coordinates
	PSR_MinZ_MaxZ_Ratio = 2,			// Converts device Z values to clip-space W values
};

// Vertex shader constants that are reserved by the Engine.
enum EVertexShaderRegister
{
	VSR_ViewProjMatrix = 0,		// View-projection matrix, transforming from World space to Projection space
	VSR_ViewOrigin = 4,			// World space position of the view's origin (camera position)
};

/**
 *	Resource usage flags - for vertex and index buffers.
 */
enum EResourceUsageFlag
{
	RUF_Static = 0,			// The resource will be created, filled, and never repacked.
	RUF_Dynamic = 1,		// The resource will be repacked in-frequently.
	RUF_Volatile = 2		// The resource will be repacked EVERY frame.
};

/**
 *	Tessellation mode, to be used with RHISetTessellationMode.
 */
enum ETessellationMode
{
	TESS_Discrete = 0,
	TESS_Continuous = 1,
	TESS_PerEdge = 2,
};

/**
 *	Screen Resoultion
 */
struct FScreenResolutionRHI
{
	DWORD	Width;
	DWORD	Height;
	DWORD	RefreshRate;
};

typedef TArray<FScreenResolutionRHI>	FScreenResolutionArray;

/** A macro that calls the provided macro with the name and parent's name of each RHI resource type. */
#define ENUM_RHI_RESOURCE_TYPES(EnumerationMacro) \
	EnumerationMacro(SamplerState,None) \
	EnumerationMacro(RasterizerState,None) \
	EnumerationMacro(DepthState,None) \
	EnumerationMacro(StencilState,None) \
	EnumerationMacro(BlendState,None) \
	EnumerationMacro(VertexDeclaration,None) \
	EnumerationMacro(VertexShader,None) \
	EnumerationMacro(PixelShader,None) \
	EnumerationMacro(BoundShaderState,None) \
	EnumerationMacro(IndexBuffer,None) \
	EnumerationMacro(VertexBuffer,None) \
	EnumerationMacro(Surface,None) \
	EnumerationMacro(Texture,None) \
	EnumerationMacro(Texture2D,Texture) \
	EnumerationMacro(TextureCube,Texture) \
	EnumerationMacro(SharedTexture2D,Texture2D) \
	EnumerationMacro(SharedMemoryResource,None) \
	EnumerationMacro(OcclusionQuery,None) \
	EnumerationMacro(Viewport,None)

/** An enumeration of the different RHI reference types. */
enum ERHIResourceTypes
{
	RRT_None,

#define DECLARE_RESOURCETYPE_ENUM(Type,ParentType) RRT_##Type,
	ENUM_RHI_RESOURCE_TYPES(DECLARE_RESOURCETYPE_ENUM)
#undef DECLARE_RESOURCETYPE_ENUM
};

/** Flags used for texture creation */
enum ETextureCreateFlags
{
	// Texture is encoded in sRGB gamma space
	TexCreate_SRGB					= 1<<0,
	// Texture can be used as a resolve target (normally not stored in the texture pool)
	TexCreate_ResolveTargetable		= 1<<1,
	// Texture is a depth stencil format that can be sampled
	TexCreate_DepthStencil			= 1<<2,
	// Texture will be created without a packed miptail
	TexCreate_NoMipTail				= 1<<3,
	// Texture will be created with an un-tiled format
	TexCreate_NoTiling				= 1<<4,
	// Texture that for a resolve target will only be written to/resolved once
	TexCreate_WriteOnce				= 1<<5,
	// Texture that may be updated every frame
	TexCreate_Dynamic				= 1<<6,
	// Texture that didn't go through the offline cooker (normally not stored in the texture pool)
	TexCreate_Uncooked				= 1<<7,
	// Allow silent texture creation failure
	TexCreate_AllowFailure			= 1<<8,
	// Disable automatic defragmentation if the initial texture memory allocation fails.
	TexCreate_DisableAutoDefrag		= 1<<9,
};

/** Flags used for targetable surface creation */
enum ETargetableSurfaceCreateFlags
{
	// Without this the surface may simply be an alias for the texture's memory. Note that you must still resolve the surface.
    TargetSurfCreate_Dedicated		= 1<<0,
	// Surface must support being read from by RHIReadSurfaceData.
	TargetSurfCreate_Readable		= 1<<1,
	// Surface will be only written to one time, in case that helps the platform
	TargetSurfCreate_WriteOnce		= 1<<2,
	// Surface will be created as multisampled.  This implies TargetSurfCreate_Dedicated, unless multisampling is disabled.
	TargetSurfCreate_Multisample	= 1<<3,
};

// Forward-declaration.
struct FResolveParams;

//
// Platform-specific RHI types.
//
#if !CONSOLE || USE_NULL_RHI
	// Use dynamically bound RHIs on PCs and when using the null RHI.
	#define USE_DYNAMIC_RHI 1
	#include "DynamicRHI.h"
#elif XBOX
	// Use the statically bound XeD3D RHI on Xenon
	// #define a wrapper define that will make XeD3D files still compile even when using Null RHI
	#define USE_XeD3D_RHI 1
	#include "XeD3DDrv.h"
#elif PS3
	// Use statically bound PS3 RHI
    #define USE_PS3_RHI 1
	#include "PS3RHI.h"
#else
	// Fall back to the null RHI
	#undef USE_NULL_RHI
	#define USE_NULL_RHI 1
#endif

struct FResolveParams
{
	/** used to specify face when resolving to a cube map texture */
	ECubeFace CubeFace;
	/** resolve RECT bounded by [X1,Y1]..[X2,Y2]. Or -1 for fullscreen */
	INT X1,Y1,X2,Y2;
	/** Texture to resolve to. If NULL, it will resolve to the texture associated with the rendertarget. */
	FTexture2DRHIParamRef ResolveTarget;
	/** constructor */
	FResolveParams(INT InX1=-1, INT InY1=-1, INT InX2=-1, INT InY2=-1, ECubeFace InCubeFace=CubeFace_PosX, FTexture2DRHIParamRef InResolveTarget=NULL)
		:	CubeFace(InCubeFace)
		,	X1(InX1), Y1(InY1), X2(InX2), Y2(InY2)
		,	ResolveTarget(InResolveTarget)
	{}
};

/** specifies a texture and region to copy */
struct FCopyTextureRegion2D
{
	/** source texture to lock and copy from */
	FTexture2DRHIParamRef SrcTexture;
	/** horizontal texel offset for copy region */
	INT OffsetX;
	/** vertical texel offset for copy region */
	INT OffsetY;
	/** horizontal texel size for copy region */
	INT SizeX;
	/** vertical texel size for copy region */
	INT SizeY;	
	/** Starting mip index. This is treated as the base level (default is 0) */
	INT FirstMipIdx;
	/** constructor */
	FCopyTextureRegion2D(FTexture2DRHIParamRef InSrcTexture,INT InOffsetX=-1,INT InOffsetY=-1,INT InSizeX=-1,INT InSizeY=-1,INT InFirstMipIdx=0)
		:	SrcTexture(InSrcTexture)
		,	OffsetX(InOffsetX)
		,	OffsetY(InOffsetY)
		,	SizeX(InSizeX)
		,	SizeY(InSizeY)
		,	FirstMipIdx(InFirstMipIdx)
	{}
};

// Define the statically bound RHI methods with the RHI name prefix.
#define DEFINE_RHIMETHOD(Type,Name,ParameterTypesAndNames,ParameterNames,ReturnStatement,NullImplementation) extern Type RHI##Name ParameterTypesAndNames
#include "RHIMethods.h"
#undef DEFINE_RHIMETHOD

/** Initializes the RHI. */
extern void RHIInit();

/** Shuts down the RHI. */
extern void RHIExit();

/**
 * The scene rendering stats.
 */
enum ESceneRenderingStats
{
	STAT_BeginOcclusionTestsTime = STAT_SceneRenderingFirstStat,
	STAT_OcclusionResultTime,
	STAT_InitViewsTime,
	STAT_DynamicShadowSetupTime,
	STAT_TranslucencySetupTime,
	STAT_TotalGPUFrameTime,
	STAT_TotalSceneRenderingTime,
	STAT_SceneCaptureRenderingTime,

	STAT_DepthDrawTime,
	STAT_BasePassDrawTime,
	STAT_LightingDrawTime,
	STAT_ProjectedShadowDrawTime,
	STAT_ModulatedShadowDrawTime,
	STAT_LightFunctionDrawTime,
	STAT_ShadowVolumeDrawTime,
	STAT_TranslucencyDrawTime,
	STAT_VelocityDrawTime,

	STAT_ProjectedShadows,
	STAT_CulledPrimitives,
	STAT_OccludedPrimitives,
	STAT_OcclusionQueries,
	STAT_VisibleStaticMeshElements,
	STAT_VisibleDynamicPrimitives,
	STAT_DrawEvents,
	STAT_SceneLights,
};

#endif // !__RHI_H__

