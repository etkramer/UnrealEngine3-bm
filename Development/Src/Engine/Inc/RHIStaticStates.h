/*=============================================================================
	RHIStaticStates.h: RHI static state template definition.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 * The base class of the static RHI state classes.
 */
template<typename InitializerType,typename RHIRefType>
class TStaticStateRHI
{
public:

	static RHIRefType GetRHI()
	{
		static FStaticStateResource Resource;
		return Resource.StateRHI;
	};

private:

	/** A resource which manages the RHI resource. */
	class FStaticStateResource : public FRenderResource
	{
	public:

		RHIRefType StateRHI;

		FStaticStateResource()
		{
			InitResource();
		}

		// FRenderResource interface.
		virtual void InitRHI()
		{
			StateRHI = InitializerType::CreateRHI();
		}
		virtual void ReleaseRHI()
		{
			StateRHI.SafeRelease();
		}

		~FStaticStateResource()
		{
			ReleaseResource();
		}
	};
};

/**
 * A static RHI sampler state resource.
 * TStaticSamplerStateRHI<...>::GetStaticState() will return a FSamplerStateRHIRef to a sampler state with the desired settings.
 * Should only be used from the rendering thread.
 */
template<ESamplerFilter Filter=SF_Point,ESamplerAddressMode AddressU=AM_Clamp,ESamplerAddressMode AddressV=AM_Clamp,ESamplerAddressMode AddressW=AM_Clamp, ESamplerMipMapLODBias MipBias=MIPBIAS_None>
class TStaticSamplerState : public TStaticStateRHI<TStaticSamplerState<Filter,AddressU,AddressV,AddressW,MipBias>,FSamplerStateRHIRef>
{
public:
	static FSamplerStateRHIRef CreateRHI()
	{
		FSamplerStateInitializerRHI Initializer = { Filter, AddressU, AddressV, AddressW, MipBias };
		return RHICreateSamplerState(Initializer);
	}
};

/**
 * A static RHI rasterizer state resource.
 * TStaticRasterizerStateRHI<...>::GetStaticState() will return a FRasterizerStateRHIRef to a rasterizer state with the desired
 * settings.
 * Should only be used from the rendering thread.
 */
template<ERasterizerFillMode FillMode=FM_Solid,ERasterizerCullMode CullMode=CM_None>
class TStaticRasterizerState : public TStaticStateRHI<TStaticRasterizerState<FillMode,CullMode>,FRasterizerStateRHIRef>
{
public:
	static FRasterizerStateRHIRef CreateRHI()
	{
		FRasterizerStateInitializerRHI Initializer = { FillMode, CullMode, 0, 0 };
		return RHICreateRasterizerState(Initializer);
	}
};

/**
 * A static RHI depth state resource.
 * TStaticDepthStateRHI<...>::GetStaticState() will return a FDepthStateRHIRef to a depth state with the desired
 * settings.
 * Should only be used from the rendering thread.
 */
template<
	UBOOL bEnableDepthWrite=TRUE,
	ECompareFunction DepthTest=CF_LessEqual
	>
class TStaticDepthState : public TStaticStateRHI<
	TStaticDepthState<
		bEnableDepthWrite,
		DepthTest
		>,
	FDepthStateRHIRef
	>
{
public:
	static FDepthStateRHIRef CreateRHI()
	{
		FDepthStateInitializerRHI Initializer =
		{
			bEnableDepthWrite,
			DepthTest
		};
		return RHICreateDepthState(Initializer);
	}
};

/**
 * A static RHI stencil state resource.
 * TStaticStencilStateRHI<...>::GetStaticState() will return a FStencilStateRHIRef to a stencil state with the desired
 * settings.
 * Should only be used from the rendering thread.
 */
template<
	UBOOL bEnableFrontFaceStencil = FALSE,
	ECompareFunction FrontFaceStencilTest = CF_Always,
	EStencilOp FrontFaceStencilFailStencilOp = SO_Keep,
	EStencilOp FrontFaceDepthFailStencilOp = SO_Keep,
	EStencilOp FrontFacePassStencilOp = SO_Keep,
	UBOOL bEnableBackFaceStencil = FALSE,
	ECompareFunction BackFaceStencilTest = CF_Always,
	EStencilOp BackFaceStencilFailStencilOp = SO_Keep,
	EStencilOp BackFaceDepthFailStencilOp = SO_Keep,
	EStencilOp BackFacePassStencilOp = SO_Keep,
	DWORD StencilReadMask = 0xFFFFFFFF,
	DWORD StencilWriteMask = 0xFFFFFFFF,
	DWORD StencilRef = 0
	>
class TStaticStencilState : public TStaticStateRHI<
	TStaticStencilState<
		bEnableFrontFaceStencil,
		FrontFaceStencilTest,
		FrontFaceStencilFailStencilOp,
		FrontFaceDepthFailStencilOp,
		FrontFacePassStencilOp,
		bEnableBackFaceStencil,
		BackFaceStencilTest,
		BackFaceStencilFailStencilOp,
		BackFaceDepthFailStencilOp,
		BackFacePassStencilOp,
		StencilReadMask,
		StencilWriteMask,
		StencilRef
		>,
	FStencilStateRHIRef
	>
{
public:
	static FStencilStateRHIRef CreateRHI()
	{
		FStencilStateInitializerRHI Initializer(
			bEnableFrontFaceStencil,
			FrontFaceStencilTest,
			FrontFaceStencilFailStencilOp,
			FrontFaceDepthFailStencilOp,
			FrontFacePassStencilOp,
			bEnableBackFaceStencil,
			BackFaceStencilTest,
			BackFaceStencilFailStencilOp,
			BackFaceDepthFailStencilOp,
			BackFacePassStencilOp,
			StencilReadMask,
			StencilWriteMask,
			StencilRef);

		return RHICreateStencilState(Initializer);
	}
};

/**
 * A static RHI blend state resource.
 * TStaticBlendStateRHI<...>::GetStaticState() will return a FBlendStateRHIRef to a blend state with the desired settings.
 * Should only be used from the rendering thread.
 */
template<
	EBlendOperation ColorBlendOp = BO_Add,
	EBlendFactor ColorSrcBlend = BF_One,
	EBlendFactor ColorDestBlend = BF_Zero,
	EBlendOperation AlphaBlendOp = BO_Add,
	EBlendFactor AlphaSrcBlend = BF_One,
	EBlendFactor AlphaDestBlend = BF_Zero,
	ECompareFunction AlphaTest = CF_Always,
	BYTE AlphaRef = 255
	>
class TStaticBlendState : public TStaticStateRHI<
	TStaticBlendState<ColorBlendOp,ColorSrcBlend,ColorDestBlend,AlphaBlendOp,AlphaSrcBlend,AlphaDestBlend,AlphaTest,AlphaRef>,
	FBlendStateRHIRef
	>
{
public:
	static FBlendStateRHIRef CreateRHI()
	{
		FBlendStateInitializerRHI Initializer =
		{
			ColorBlendOp,
			ColorSrcBlend,
			ColorDestBlend,
			AlphaBlendOp,
			AlphaSrcBlend,
			AlphaDestBlend,
			AlphaTest,
			AlphaRef
		};
		return RHICreateBlendState(Initializer);
	}
};
