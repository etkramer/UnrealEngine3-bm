/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "SceneFilterRendering.h"
#include "AmbientOcclusionRendering.h"

/** Controls whether ambient occlusion is rendered or not.  @Todo: make this a show flag. */
UBOOL GRenderAmbientOcclusion = TRUE;

/** 
 * If TRUE, calculated ambient occlusion will be combined with scene color, 
 * otherwise scene color will be overwritten by the AO factor for visualization purposes. 
 */
UBOOL GAOCombineWithSceneColor = TRUE;

/** 
 * If TRUE, ambient occlusion and fog are simultaneously rendered as an optimization
 */
#if XBOX
UBOOL GAOCombineWithHeightFog = TRUE;
#else
UBOOL GAOCombineWithHeightFog = FALSE;
#endif

/** The minimum projected screen radius for a primitive to be drawn in the ambient occlusion masking passes, as a fraction of half the horizontal screen width. */
static const FLOAT MinScreenRadiusForAOMasking = 0.10f;
static const FLOAT MinScreenRadiusForAOMaskingSquared = Square(MinScreenRadiusForAOMasking);

/** Whether AO passes should take advantage of a depth buffer or do depth testing manually */
static UBOOL UseDepthBufferForAO(const FDownsampleDimensions& DownsampleDimensions)
{
	return GSceneRenderTargets.IsDownsizedDepthSupported() && DownsampleDimensions.Factor == GSceneRenderTargets.GetSmallColorDepthDownsampleFactor();
}

/*-----------------------------------------------------------------------------
	FDownsampleDimensions
-----------------------------------------------------------------------------*/

FDownsampleDimensions::FDownsampleDimensions(const FViewInfo& View)
{
	Factor = GSceneRenderTargets.GetAmbientOcclusionDownsampleFactor();
	TargetX = View.RenderTargetX / Factor;
	TargetY = View.RenderTargetY / Factor;
	TargetSizeX = View.RenderTargetSizeX / Factor;
	TargetSizeY = View.RenderTargetSizeY / Factor;
	// Round off odd view sizes
	ViewSizeX = appFloor(View.SizeX / Factor);
	ViewSizeY = appFloor(View.SizeY / Factor);
}

/*-----------------------------------------------------------------------------
	FAmbientOcclusionParams
-----------------------------------------------------------------------------*/

void FAmbientOcclusionParams::Bind(const FShaderParameterMap& ParameterMap)
{
	AmbientOcclusionTextureParameter.Bind(ParameterMap,TEXT("AmbientOcclusionTexture"), TRUE);
	AOHistoryTextureParameter.Bind(ParameterMap,TEXT("AOHistoryTexture"), TRUE);
	AOScreenPositionScaleBiasParameter.Bind(ParameterMap,TEXT("AOScreenPositionScaleBias"), TRUE);
	ScreenEdgeLimitsParameter.Bind(ParameterMap,TEXT("ScreenEdgeLimits"), TRUE);
}

void FAmbientOcclusionParams::Set(const FDownsampleDimensions& DownsampleDimensions, FShader* PixelShader, ESamplerFilter AOFilter = SF_Point)
{
	// Transform from NDC [-1, 1] to screen space so that positions can be used as texture coordinates to lookup into ambient occlusion buffers.
	// This handles the view size being smaller than the buffer size, and applies a half pixel offset on required platforms.
	// Scale in xy, Bias in zw.
	AOScreenPositionScaleBias = FVector4(
		DownsampleDimensions.ViewSizeX / GSceneRenderTargets.GetAmbientOcclusionBufferSizeX() / +2.0f,
		DownsampleDimensions.ViewSizeY / GSceneRenderTargets.GetAmbientOcclusionBufferSizeY() / -2.0f,
		(DownsampleDimensions.ViewSizeX / 2.0f + GPixelCenterOffset + DownsampleDimensions.TargetX) / GSceneRenderTargets.GetAmbientOcclusionBufferSizeX(),
		(DownsampleDimensions.ViewSizeY / 2.0f + GPixelCenterOffset + DownsampleDimensions.TargetY) / GSceneRenderTargets.GetAmbientOcclusionBufferSizeY());

	SetPixelShaderValue(PixelShader->GetPixelShader(), AOScreenPositionScaleBiasParameter, AOScreenPositionScaleBias);

	if (ScreenEdgeLimitsParameter.IsBound())
	{
		// Find the edges of the viewport in screenspace, used to identify new pixels along the edges of the viewport so their history can be discarded.
		const FVector2D ClipToScreenScale = FVector2D(AOScreenPositionScaleBias.X, AOScreenPositionScaleBias.Y);
		const FVector2D ClipToScreenBias = FVector2D(AOScreenPositionScaleBias.Z, AOScreenPositionScaleBias.W);
		const FVector2D ScreenSpaceMin = FVector2D(-1.0f, 1.0f) * ClipToScreenScale + ClipToScreenBias;
		const FVector2D ScreenSpaceMax = FVector2D(1.0f, -1.0f) * ClipToScreenScale + ClipToScreenBias;

		SetPixelShaderValue(PixelShader->GetPixelShader(), ScreenEdgeLimitsParameter, FVector4(ScreenSpaceMin.X, ScreenSpaceMin.Y, ScreenSpaceMax.X, ScreenSpaceMax.Y));
	}
	
	if (AmbientOcclusionTextureParameter.IsBound())
	{
		FSamplerStateRHIRef Filter;
		if (AOFilter == SF_Bilinear)
		{
			Filter = TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
		}
		else
		{
			Filter = TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
		}
		SetTextureParameter(
			PixelShader->GetPixelShader(),
			AmbientOcclusionTextureParameter,
			Filter,
			GSceneRenderTargets.GetAmbientOcclusionTexture()
			);
	}

	SetTextureParameter(
		PixelShader->GetPixelShader(),
		AOHistoryTextureParameter,
		TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
		GSceneRenderTargets.GetAOHistoryTexture()
		);
}

FArchive& operator<<(FArchive& Ar,FAmbientOcclusionParams& Parameters)
{
	Ar << Parameters.AmbientOcclusionTextureParameter;
	Ar << Parameters.AOHistoryTextureParameter;
	Ar << Parameters.AOScreenPositionScaleBiasParameter;
	Ar << Parameters.ScreenEdgeLimitsParameter;
	return Ar;
}

/*-----------------------------------------------------------------------------
	FDownsampleDepthVertexShader
-----------------------------------------------------------------------------*/

class FDownsampleDepthVertexShader : public FShader
{
	DECLARE_SHADER_TYPE(FDownsampleDepthVertexShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) 
	{ 
		return Platform != SP_PCD3D_SM2; 
	}

	FDownsampleDepthVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
		HalfSceneColorTexelSizeParameter.Bind(Initializer.ParameterMap,TEXT("HalfSceneColorTexelSize"), TRUE);
	}

	void SetParameters(const FViewInfo& View)
	{
		const FVector2D HalfSceneColorTexelSize = FVector2D(
			0.5f / (FLOAT)GSceneRenderTargets.GetBufferSizeX(), 
			0.5f / (FLOAT)GSceneRenderTargets.GetBufferSizeY());

		SetVertexShaderValue(GetVertexShader(), HalfSceneColorTexelSizeParameter, HalfSceneColorTexelSize);
	}

	FDownsampleDepthVertexShader() {}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << HalfSceneColorTexelSizeParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderParameter HalfSceneColorTexelSizeParameter;
};

IMPLEMENT_SHADER_TYPE(,FDownsampleDepthVertexShader,TEXT("AmbientOcclusionShader"),TEXT("DownsampleDepthVertexMain"),SF_Vertex,0,0);

/*-----------------------------------------------------------------------------
	FDownsampleDepthPixelShader
-----------------------------------------------------------------------------*/

class FDownsampleDepthPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FDownsampleDepthPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) 
	{ 
		return Platform != SP_PCD3D_SM2; 
	}

	FDownsampleDepthPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
		SceneTextureParameters.Bind(Initializer.ParameterMap);
	}

	FDownsampleDepthPixelShader() {}

	void SetParameters(const FViewInfo& View)
	{
		SceneTextureParameters.Set(&View, this);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << SceneTextureParameters;
		return bShaderHasOutdatedParameters;
	}

private:
	FSceneTextureShaderParameters SceneTextureParameters;
};

IMPLEMENT_SHADER_TYPE(,FDownsampleDepthPixelShader,TEXT("AmbientOcclusionShader"),TEXT("DownsampleDepthPixelMain"),SF_Pixel,0,0);

FGlobalBoundShaderState DepthDownsampleBoundShaderState;
FGlobalBoundShaderState DistanceMaskBoundShaderState;

extern TGlobalResource<FFilterVertexDeclaration> GFilterVertexDeclaration;

/** Downsamples the scene's fogging and ambient occlusion depth. */
extern UBOOL RenderQuarterDownsampledDepthAndFog(const FScene* Scene, const FViewInfo& View, UINT DPGIndex, const FDownsampleDimensions& DownsampleDimensions);

/**
 * Down-samples depth to the occlusion buffer.
 */
UBOOL DownsampleDepth(const FScene* Scene, UINT InDepthPriorityGroup, const FViewInfo& View, const FDownsampleDimensions& DownsampleDimensions, UBOOL bUsingHistoryBuffer, FLOAT MaxOcclusionDepth)
{
	UBOOL bFogRendered = FALSE;

	// Clear the render target storing depth for ambient occlusion calculations when we're in the editor and rendering to a subset of the render target.
	// This avoids a band of occlusion on editor viewports which are reading depth values outside of the view.
	//@todo: Need to handle this in game, both when the resolution has changed to be smaller (in which case render targets are not re-allocated)
	// and in split screen, since each view will read depth values from the neighboring view.
	if ( GIsEditor 
		&& (DownsampleDimensions.TargetSizeX < (INT)GSceneRenderTargets.GetAmbientOcclusionBufferSizeX()
		|| DownsampleDimensions.TargetSizeY < (INT)GSceneRenderTargets.GetAmbientOcclusionBufferSizeY()))
	{
		GSceneRenderTargets.BeginRenderingAmbientOcclusion();
		// Depth is stored in the g channel, clear to max half so that no occlusion can be added from samples landing outside the viewport.
		const FLinearColor ClearDepthColor(0,65504.0f,0,0);

		// Set the viewport to the current view in the occlusion buffer.
		RHISetViewport(DownsampleDimensions.TargetX, DownsampleDimensions.TargetY, 0.0f, 
			DownsampleDimensions.TargetX + DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetY + DownsampleDimensions.TargetSizeY, 1.0f);				

		FBatchedElements BatchedElements;
		INT V00 = BatchedElements.AddVertex(FVector4(-1,-1,0,1),FVector2D(0,0),ClearDepthColor,FHitProxyId());
		INT V10 = BatchedElements.AddVertex(FVector4(1,-1,0,1),FVector2D(1,0),ClearDepthColor,FHitProxyId());
		INT V01 = BatchedElements.AddVertex(FVector4(-1,1,0,1),FVector2D(0,1),ClearDepthColor,FHitProxyId());
		INT V11 = BatchedElements.AddVertex(FVector4(1,1,0,1),FVector2D(1,1),ClearDepthColor,FHitProxyId());

		// No alpha blending, no depth tests or writes, no backface culling.
		RHISetBlendState(TStaticBlendState<>::GetRHI());
		RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
		RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());

		// Draw a quad using the generated vertices.
		BatchedElements.AddTriangle(V00,V10,V11,GWhiteTexture,BLEND_Opaque);
		BatchedElements.AddTriangle(V00,V11,V01,GWhiteTexture,BLEND_Opaque);
		BatchedElements.Draw(
			FMatrix::Identity,
			GSceneRenderTargets.GetAmbientOcclusionBufferSizeX(),
			GSceneRenderTargets.GetAmbientOcclusionBufferSizeY(),
			FALSE
			);

		GSceneRenderTargets.FinishRenderingAmbientOcclusion(
			FResolveParams(
				DownsampleDimensions.TargetX,
				DownsampleDimensions.TargetY, 
				DownsampleDimensions.TargetX + DownsampleDimensions.TargetSizeX,
				DownsampleDimensions.TargetY + DownsampleDimensions.TargetSizeY
				)
			);
	}

	// Downsample depths
	// Bind the occlusion buffer as a render target.
	GSceneRenderTargets.BeginRenderingAmbientOcclusion();

	const UBOOL bUseDepthBufferForAO = UseDepthBufferForAO(DownsampleDimensions);

	// try to downsample depth and fog together if possible
	if(	GSceneRenderTargets.GetAmbientOcclusionDownsampleFactor() == 2
		&& View.Family->ShowFlags & SHOW_Fog 
		&& GAOCombineWithHeightFog 
		&& GAOCombineWithSceneColor
		&& RenderQuarterDownsampledDepthAndFog(Scene, View, InDepthPriorityGroup, DownsampleDimensions))
	{
		bFogRendered = TRUE;
	}
	else
	{
		SCOPED_DRAW_EVENT(OcclusionEventView)(DEC_SCENE_ITEMS,TEXT("DownsampleDepth"));
		// we have to downsample depth ourselves as it was not done in the fog pass
		RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
		// Disable depth test and writes
		RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
		RHISetBlendState(TStaticBlendState<>::GetRHI());

		// Set the viewport to the current view in the occlusion buffer.
		RHISetViewport(DownsampleDimensions.TargetX, DownsampleDimensions.TargetY, 0.0f, 
			DownsampleDimensions.TargetX + DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetY + DownsampleDimensions.TargetSizeY, 1.0f);				

		TShaderMapRef<FDownsampleDepthVertexShader> VertexShader(GetGlobalShaderMap());
		TShaderMapRef<FDownsampleDepthPixelShader> PixelShader(GetGlobalShaderMap());

		SetGlobalBoundShaderState(DepthDownsampleBoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FFilterVertex));

		VertexShader->SetParameters(View);
		PixelShader->SetParameters(View);

		// Draw a full-view quad whose texture coordinates are setup to read from the current view in scene color,
		// and whose positions are setup to render to the entire viewport in the occlusion buffer.
		DrawDenormalizedQuad(
			0, 0,
			DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetSizeY,
			View.RenderTargetX, View.RenderTargetY,
			View.RenderTargetSizeX, View.RenderTargetSizeY,
			DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetSizeY,
			GSceneRenderTargets.GetBufferSizeX(), GSceneRenderTargets.GetBufferSizeY()
			);
	}

	FLOAT DepthBias = 0.0f;
	// Sets up a mask in the R channel of the AO buffer for any primitives that don't allow generating ambient occlusion.
	// If a depth buffer is supported for this downsample factor, setup a stencil mask so these pixels will not be operated on later.
	{
		SCOPED_DRAW_EVENT(RemoveOcclusionEventView)(DEC_SCENE_ITEMS,TEXT("MaskNonOccluders"));
		// Only affect the R channel, G has depth from the previous pass that we don't want to alter.
		RHISetColorWriteMask(CW_RED);
		if (bUseDepthBufferForAO)
		{
			GSceneRenderTargets.BeginRenderingAmbientOcclusion(TRUE);
			// Using the quarter sized depth buffer
			// Z writes off, depth tests on
			RHISetDepthState(TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());
			// Enable Hi Stencil writes with a cull condition of stencil == 1
			RHIBeginHiStencilRecord(TRUE, 1);
			// Clear stencil in case it is dirty
			RHIClear(FALSE,FLinearColor(0,0,0,0),FALSE,0,TRUE,0);
			// Set stencil to one.
			RHISetStencilState(TStaticStencilState<
				TRUE,CF_Always,SO_Keep,SO_Keep,SO_Replace,
				FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
				0xff,0xff,1
			>::GetRHI());
			// Use a depth bias as the positions generated from this pass will not exactly match up with the depths in the downsampled depth buffer.
			DepthBias = -0.0001f;
		}
		else
		{
			RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
		}
		RHISetBlendState(TStaticBlendState<>::GetRHI());

		FAOMaskDrawingPolicy::DownsampleDimensions = DownsampleDimensions;
		TDynamicPrimitiveDrawer<FAOMaskDrawingPolicyFactory> Drawer(&View, InDepthPriorityGroup, FAOMaskDrawingPolicyFactory::ContextType((ESceneDepthPriorityGroup)InDepthPriorityGroup, DepthBias), TRUE);
		TDynamicPrimitiveDrawer<FAOMaskDrawingPolicyFactory> ForegroundDPGDrawer(&View, SDPG_Foreground, FAOMaskDrawingPolicyFactory::ContextType(SDPG_Foreground, DepthBias), TRUE);
		for(INT PrimitiveIndex = 0; PrimitiveIndex < View.VisibleDynamicPrimitives.Num(); PrimitiveIndex++)
		{
			const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleDynamicPrimitives(PrimitiveIndex);
			const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);
			const UBOOL bVisible = View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id);

			// Only render if visible.
			if(bVisible 
				// Only render movable objects.
				&& !PrimitiveSceneInfo->bStaticShadowing 
				// Skip translucent objects as they don't occlude anything.
				&& PrimitiveViewRelevance.bOpaqueRelevance)
			{
				const FLOAT LODFactorDistanceSquared = (PrimitiveSceneInfo->Bounds.Origin - View.ViewOrigin).SizeSquared() * Square(View.LODDistanceFactor);
				// If we have a depth-stencil buffer, setup a mask for any foreground primitives here so they will not be operated on in the following passes.
				if (PrimitiveViewRelevance.GetDPG(SDPG_Foreground) && bUseDepthBufferForAO)
				{
					ForegroundDPGDrawer.SetPrimitive(PrimitiveSceneInfo);
					PrimitiveSceneInfo->Proxy->DrawDynamicElements(
						&ForegroundDPGDrawer,
						&View,
						SDPG_Foreground
						);
				}
				// Only render primitives that don't allow ambient occlusion.
				else if (!PrimitiveSceneInfo->bAllowAmbientOcclusion
					// Only render if primitive is relevant to the current DPG.
					 && PrimitiveViewRelevance.GetDPG(InDepthPriorityGroup)
					// Skip primitives that take up a small amount of screenspace.
					&& Square(PrimitiveSceneInfo->Bounds.SphereRadius) > MinScreenRadiusForAOMaskingSquared * LODFactorDistanceSquared)
				{
					Drawer.SetPrimitive(PrimitiveSceneInfo);
					PrimitiveSceneInfo->Proxy->DrawDynamicElements(
						&Drawer,
						&View,
						InDepthPriorityGroup
						);
				}
			}
		}

		if (bUseDepthBufferForAO)
		{		
			// No occlusion will be visible past MaxOcclusionDepth so we setup a stencil mask to avoid operating on those pixels.
			RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
			RHISetDepthState(TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());
			RHISetBlendState(TStaticBlendState<>::GetRHI());

			TShaderMapRef<FOneColorVertexShader> VertexShader(GetGlobalShaderMap());
			TShaderMapRef<FOneColorPixelShader> PixelShader(GetGlobalShaderMap());

			SetGlobalBoundShaderState(DistanceMaskBoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FFilterVertex));
			SetPixelShaderValue(PixelShader->GetPixelShader(),PixelShader->ColorParameter,FLinearColor(1.0f, 0.0f, 0.0f, 0.0f));

			FVector ViewSpaceMaxDistance(0.0f, 0.0f, MaxOcclusionDepth);
			FVector4 ClipSpaceMaxDistance = View.ProjectionMatrix.TransformFVector(ViewSpaceMaxDistance);
			// Draw a full-view quad whose Z is at MaxOcclusionDepth
			// Any pixels of the quad that pass the z-test will write a mask to stencil and Hi stencil
			DrawDenormalizedQuad(
				0, 0,
				DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetSizeY,
				DownsampleDimensions.TargetX, DownsampleDimensions.TargetY,
				DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetSizeY,
				DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetSizeY,
				GSceneRenderTargets.GetAmbientOcclusionBufferSizeX(), GSceneRenderTargets.GetAmbientOcclusionBufferSizeY(),
				ClipSpaceMaxDistance.Z / ClipSpaceMaxDistance.W
				);
	
			// Disable Hi Stencil
			RHIEndHiStencil();
		}
		// Restore color writes to all channels
		RHISetColorWriteMask(CW_RGBA);
	}

	GSceneRenderTargets.FinishRenderingAmbientOcclusion(
		FResolveParams(
			DownsampleDimensions.TargetX,
			DownsampleDimensions.TargetY, 
			DownsampleDimensions.TargetX + DownsampleDimensions.TargetSizeX,
			DownsampleDimensions.TargetY + DownsampleDimensions.TargetSizeY
			)
		);

	return bFogRendered;
}

/*-----------------------------------------------------------------------------
	FAmbientOcclusionVertexShader
-----------------------------------------------------------------------------*/

class FAmbientOcclusionVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FAmbientOcclusionVertexShader,Global);
public:
	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return Platform != SP_PCD3D_SM2; 
	}

	/** Default constructor. */
	FAmbientOcclusionVertexShader() {}

	/** Initialization constructor. */
	FAmbientOcclusionVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		ScreenToViewParameter.Bind(Initializer.ParameterMap,TEXT("ScreenToView"), TRUE);
	}

	// FShader interface.
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << ScreenToViewParameter;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(const FViewInfo& View)
	{
		FMatrix ScreenToView = FMatrix(
			FPlane(1,0,0,0),
			FPlane(0,1,0,0),
			FPlane(0,0,(1.0f - Z_PRECISION),1),
			FPlane(0,0,-View.NearClippingDistance * (1.0f - Z_PRECISION),0)
			) *
			View.ProjectionMatrix.Inverse() *
			FTranslationMatrix(-(FVector)View.ViewOrigin);

		SetVertexShaderValue(GetVertexShader(),ScreenToViewParameter,ScreenToView);
	}

private:
	FShaderParameter ScreenToViewParameter;
};

/** Policy for rendering all qualities */
class FDefaultQualityAO
{
public:
	static const UINT QualityIndex = 0;
	static const UINT NumSamples = 8;
	static FVector4 SampleOffsets[NumSamples];
	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}
};

/** Samples in a unit sphere, at cube corners. */
FVector4 FDefaultQualityAO::SampleOffsets[FDefaultQualityAO::NumSamples] = 
{
	FVector(1.0f, 1.0f, 1.0f).UnsafeNormal(), FVector(1.0f, 1.0f, -1.0f).UnsafeNormal(), FVector(1.0f, -1.0f, 1.0f).UnsafeNormal(), FVector(-1.0f, 1.0f, 1.0f).UnsafeNormal(),
	FVector(1.0f, -1.0f, -1.0f).UnsafeNormal(), FVector(-1.0f, -1.0f, 1.0f).UnsafeNormal(), FVector(-1.0f, 1.0f, -1.0f).UnsafeNormal(), FVector(-1.0f, -1.0f, -1.0f).UnsafeNormal(),
};

/*-----------------------------------------------------------------------------
	TAmbientOcclusionPixelShader
-----------------------------------------------------------------------------*/

template<class QualityPolicy, UBOOL bSupportArbitraryProjection>
class TAmbientOcclusionPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(TAmbientOcclusionPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		// NumSamples must be a multiple of 4
		checkSlow(QualityPolicy::NumSamples % 4 == 0);

		return QualityPolicy::ShouldCache(Platform) && Platform != SP_PCD3D_SM2;
	}

	/** Default constructor. */
	TAmbientOcclusionPixelShader() 
	{
	}

	/** Initialization constructor. */
	TAmbientOcclusionPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
		SampleOffsetsParameter.Bind(Initializer.ParameterMap,TEXT("OcclusionSampleOffsets"), TRUE);
		RandomNormalTextureParameter.Bind(Initializer.ParameterMap,TEXT("RandomNormalTexture"), TRUE);
		ProjectionScaleParameter.Bind(Initializer.ParameterMap,TEXT("ProjectionScale"), TRUE);
		ProjectionMatrixParameter.Bind(Initializer.ParameterMap,TEXT("ProjectionMatrix"), TRUE);
		NoiseScaleParameter.Bind(Initializer.ParameterMap,TEXT("NoiseScale"), TRUE);
		AOParams.Bind(Initializer.ParameterMap);
		OcclusionCalcParameters.Bind(Initializer.ParameterMap,TEXT("OcclusionCalcParameters"), TRUE);
		HaloDistanceScaleParameter.Bind(Initializer.ParameterMap,TEXT("HaloDistanceScale"), TRUE);
		OcclusionRemapParameters.Bind(Initializer.ParameterMap,TEXT("OcclusionRemapParameters"), TRUE);
		OcclusionFadeoutParameters.Bind(Initializer.ParameterMap,TEXT("OcclusionFadeoutParameters"), TRUE);
		MaxRadiusTransformParameter.Bind(Initializer.ParameterMap,TEXT("MaxRadiusTransform"), TRUE);
	}

	// FShader interface.
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << SampleOffsetsParameter;
		Ar << RandomNormalTextureParameter;
		Ar << ProjectionScaleParameter;
		Ar << ProjectionMatrixParameter;
		Ar << NoiseScaleParameter;
		Ar << AOParams;
		Ar << OcclusionCalcParameters;
		Ar << HaloDistanceScaleParameter;
		Ar << OcclusionRemapParameters;
		Ar << OcclusionFadeoutParameters;
		Ar << MaxRadiusTransformParameter;
		return bShaderHasOutdatedParameters;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("NUM_OCCLUSION_SAMPLES"), *FString::Printf(TEXT("%u"), QualityPolicy::NumSamples));
		OutEnvironment.Definitions.Set(TEXT("AO_QUALITY"), *FString::Printf(TEXT("%u"), QualityPolicy::QualityIndex));
		OutEnvironment.Definitions.Set(TEXT("ARBITRARY_PROJECTION"), bSupportArbitraryProjection ? TEXT("1") : TEXT("0"));
	}

	void SetParameters(const FViewInfo& View, const FDownsampleDimensions& DownsampleDimensions, const FAmbientOcclusionSettings& AOSettings)
	{
		AOParams.Set(DownsampleDimensions, this);

		SetPixelShaderValues(GetPixelShader(), SampleOffsetsParameter, QualityPolicy::SampleOffsets, QualityPolicy::NumSamples);

		SetTextureParameter(
			GetPixelShader(),
			RandomNormalTextureParameter,
			TStaticSamplerState<SF_Point,AM_Wrap,AM_Wrap,AM_Wrap>::GetRHI(),
			GEngine->RandomNormalTexture->Resource->TextureRHI
			);

		if (bSupportArbitraryProjection)
		{
			// Arbitrary projections are a slow path, but must be supported for tiled screenshots
			SetPixelShaderValue(GetPixelShader(), ProjectionMatrixParameter, View.ProjectionMatrix);
		}
		else
		{
			// If bSupportArbitraryProjection=FALSE the projection matrix cannot be off-center and must only scale x and y.
			checkSlow(Abs(View.ProjectionMatrix.M[3][0]) < KINDA_SMALL_NUMBER 
				&& Abs(View.ProjectionMatrix.M[3][1]) < KINDA_SMALL_NUMBER
				&& Abs(View.ProjectionMatrix.M[2][0]) < KINDA_SMALL_NUMBER
				&& Abs(View.ProjectionMatrix.M[2][1]) < KINDA_SMALL_NUMBER);

			// Combining two scales into one parameter, Projection matrix scaling of x and y and scaling from clip to screen space.
			const FVector2D ProjectionScale = FVector2D(View.ProjectionMatrix.M[0][0], View.ProjectionMatrix.M[1][1])
				* FVector2D(AOParams.AOScreenPositionScaleBias.X, AOParams.AOScreenPositionScaleBias.Y);
			SetPixelShaderValue(GetPixelShader(), ProjectionScaleParameter, ProjectionScale);
		}

		// Maps one pixel of the occlusion buffer to one texel of the random normal texture.
		const FVector4 NoiseScale = FVector4(
			GSceneRenderTargets.GetAmbientOcclusionBufferSizeX() / (FLOAT)GEngine->RandomNormalTexture->SizeX, 
			GSceneRenderTargets.GetAmbientOcclusionBufferSizeY() / (FLOAT)GEngine->RandomNormalTexture->SizeY,
			0.0f,
			0.0f);

		SetPixelShaderValue(GetPixelShader(), NoiseScaleParameter, NoiseScale);

		// Set occlusion heuristic tweakables
		SetPixelShaderValue(
			GetPixelShader(), 
			OcclusionCalcParameters, 
			FVector4(AOSettings.OcclusionRadius, AOSettings.OcclusionAttenuation, AOSettings.HaloDistanceThreshold, AOSettings.HaloOcclusion));

		// Set HaloDistanceScale
		SetPixelShaderValue(
			GetPixelShader(), 
			HaloDistanceScaleParameter, 
			AOSettings.HaloDistanceScale);

		// Set contrast and brightness tweakables
		SetPixelShaderValue(
			GetPixelShader(), 
			OcclusionRemapParameters, 
			FVector4(AOSettings.OcclusionPower, AOSettings.OcclusionScale, AOSettings.OcclusionBias, AOSettings.MinOcclusion));

		const FLOAT FadeoutRange = 1.0f / (AOSettings.OcclusionFadeoutMaxDistance - AOSettings.OcclusionFadeoutMinDistance);
		SetPixelShaderValue(
			GetPixelShader(), 
			OcclusionFadeoutParameters, 
			FVector4(AOSettings.OcclusionFadeoutMinDistance, FadeoutRange, 0.0f, 0.0f));

		// Maximum screenspace radius allowed, used to limit texture cache thrashing.
		const FLOAT MaxScreenSpaceRadius = 70.0f / GSceneRenderTargets.GetBufferSizeX();
		// Transform screenspace radius into a viewspace horizontal vector.  Multiplication by view space z is done in the pixel shader.
		const FLOAT MaxRadiusTransform = MaxScreenSpaceRadius / (View.ProjectionMatrix.M[0][0] * AOParams.AOScreenPositionScaleBias.X);
		SetPixelShaderValue(GetPixelShader(), MaxRadiusTransformParameter, MaxRadiusTransform);
	}

private:
	FShaderParameter SampleOffsetsParameter;
	FShaderResourceParameter RandomNormalTextureParameter;
	FShaderParameter ProjectionScaleParameter;
	FShaderParameter ProjectionMatrixParameter;
	FShaderParameter NoiseScaleParameter;
	FAmbientOcclusionParams AOParams;
	FShaderParameter OcclusionCalcParameters;
	FShaderParameter HaloDistanceScaleParameter;
	FShaderParameter OcclusionRemapParameters;
	FShaderParameter OcclusionFadeoutParameters;
	FShaderParameter MaxRadiusTransformParameter;
};

#define IMPLEMENT_AMBIENTOCCLUSION_SHADER_TYPE(QualityPolicy,bSupportArbitraryProjection,EntryFunctionName,MinPackageVersion,MinLicenseePackageVersion) \
	typedef TAmbientOcclusionPixelShader<QualityPolicy,bSupportArbitraryProjection> TAmbientOcclusionPixelShader##QualityPolicy##bSupportArbitraryProjection; \
	IMPLEMENT_SHADER_TYPE( \
	template<>, \
	TAmbientOcclusionPixelShader##QualityPolicy##bSupportArbitraryProjection, \
	TEXT("AmbientOcclusionShader"), \
	EntryFunctionName, \
	SF_Pixel, \
	MinPackageVersion, \
	MinLicenseePackageVersion \
	);

IMPLEMENT_AMBIENTOCCLUSION_SHADER_TYPE(FDefaultQualityAO,TRUE,TEXT("OcclusionPixelMain"),0,0);
IMPLEMENT_AMBIENTOCCLUSION_SHADER_TYPE(FDefaultQualityAO,FALSE,TEXT("OcclusionPixelMain"),0,0);

IMPLEMENT_SHADER_TYPE(,FAmbientOcclusionVertexShader,TEXT("AmbientOcclusionShader"),TEXT("OcclusionVertexMain"),SF_Vertex,0,0);

FGlobalBoundShaderState DefaultQualityArbitraryProjectionAOBoundShaderState;
FGlobalBoundShaderState DefaultQualityScaleProjectionAOBoundShaderState;

/** 
 * Calculates an occlusion value for each pixel dependent only on scene depth.
 */
void RenderOcclusion(const FViewInfo& View, const FDownsampleDimensions& DownsampleDimensions, const FAmbientOcclusionSettings& AOSettings)
{
	SCOPED_DRAW_EVENT(OcclusionEventView)(DEC_SCENE_ITEMS,TEXT("Occlusion"));

	const UBOOL bUseDepthBufferForAO = UseDepthBufferForAO(DownsampleDimensions);

	GSceneRenderTargets.BeginRenderingAmbientOcclusion(bUseDepthBufferForAO);

	RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
	RHISetBlendState(TStaticBlendState<>::GetRHI());
	// Disable depth test and writes
	RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());

	if (bUseDepthBufferForAO)
	{
		// Pass if 0
		RHISetStencilState(TStaticStencilState<
			TRUE,CF_Equal,SO_Keep,SO_Keep,SO_Keep,
			FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
			0xff,0xff,0
		>::GetRHI());

		// Use Hi Stencil from masking passes
		RHIBeginHiStencilPlayback(TRUE);
	}

	RHISetViewport(DownsampleDimensions.TargetX, DownsampleDimensions.TargetY, 0.0f, 
		DownsampleDimensions.TargetX + DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetY + DownsampleDimensions.TargetSizeY, 1.0f);				

	TShaderMapRef<FAmbientOcclusionVertexShader> VertexShader(GetGlobalShaderMap());
	VertexShader->SetParameters(View);

	if (GIsTiledScreenshot)
	{
		// Handle arbitrary projection matrices with tiled screenshots, since they need to zoom and translate within the same view to get different tiles
		TShaderMapRef<TAmbientOcclusionPixelShader<FDefaultQualityAO,TRUE> > PixelShader(GetGlobalShaderMap());
		PixelShader->SetParameters(View, DownsampleDimensions, AOSettings);
		SetGlobalBoundShaderState(DefaultQualityArbitraryProjectionAOBoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FFilterVertex));
	}
	else
	{
		// Assume the projection matrix only scales x and y in the general case
		TShaderMapRef<TAmbientOcclusionPixelShader<FDefaultQualityAO,FALSE> > PixelShader(GetGlobalShaderMap());
		PixelShader->SetParameters(View, DownsampleDimensions, AOSettings);
		SetGlobalBoundShaderState(DefaultQualityScaleProjectionAOBoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FFilterVertex));
	}

	DrawDenormalizedQuad(
		0, 0,
		DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetSizeY,
		DownsampleDimensions.TargetX, DownsampleDimensions.TargetY,
		DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetSizeY,
		DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetSizeY,
		GSceneRenderTargets.GetAmbientOcclusionBufferSizeX(), GSceneRenderTargets.GetAmbientOcclusionBufferSizeY()
		);

	if (bUseDepthBufferForAO)
	{
		// Disable Hi Stencil
		RHIEndHiStencil();
	}

	GSceneRenderTargets.FinishRenderingAmbientOcclusion(
		FResolveParams(
			DownsampleDimensions.TargetX,
			DownsampleDimensions.TargetY, 
			DownsampleDimensions.TargetX + DownsampleDimensions.TargetSizeX,
			DownsampleDimensions.TargetY + DownsampleDimensions.TargetSizeY
			)
		);
}

/*-----------------------------------------------------------------------------
	FEdgePreservingFilterVertexShader
-----------------------------------------------------------------------------*/

class FEdgePreservingFilterVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FEdgePreservingFilterVertexShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return Platform != SP_PCD3D_SM2; 
	}

	/** Default constructor. */
	FEdgePreservingFilterVertexShader() {}

	/** Initialization constructor. */
	FEdgePreservingFilterVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
	}

	/** Serializer */
	virtual UBOOL Serialize(FArchive& Ar)
	{
		return FShader::Serialize(Ar);
	}
};

/*-----------------------------------------------------------------------------
	TEdgePreservingFilterPixelShader
-----------------------------------------------------------------------------*/

template<UINT NumSamples>
class TEdgePreservingFilterPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(TEdgePreservingFilterPixelShader,Global);
public:

	/** The number of 4D constant registers used to hold the packed 2D sample offsets. */
	enum { NumSampleChunks = (NumSamples + 1) / 2 };

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return Platform != SP_PCD3D_SM2; 
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("NUM_FILTER_SAMPLES"), *FString::Printf(TEXT("%u"), NumSamples));
	}

	/** Default constructor. */
	TEdgePreservingFilterPixelShader() {}

	/** Initialization constructor. */
	TEdgePreservingFilterPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		AOParams.Bind(Initializer.ParameterMap);
		SampleOffsetsParameter.Bind(Initializer.ParameterMap,TEXT("FilterSampleOffsets"), TRUE);
		FilterParameters.Bind(Initializer.ParameterMap,TEXT("FilterParameters"), TRUE);
	}

	/** Serializer */
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << AOParams;
		Ar << SampleOffsetsParameter;
		Ar << FilterParameters;
		return bShaderHasOutdatedParameters;
	}

	/** Sets shader parameter values */
	void SetParameters(
		const FViewInfo& View, 
		const FDownsampleDimensions& DownsampleDimensions, 
		const FAmbientOcclusionSettings& AOSettings,
		UBOOL bHorizontal, 
		const FVector2D* SampleOffsets)
	{
		AOParams.Set(DownsampleDimensions, this);
		
		// Pack float2 offsets into float4's
		FVector4 PackedSampleOffsets[NumSampleChunks];
		for(INT SampleIndex = 0; SampleIndex < NumSamples; SampleIndex += 2)
		{
			PackedSampleOffsets[SampleIndex / 2].X = SampleOffsets[SampleIndex + 0].X;
			PackedSampleOffsets[SampleIndex / 2].Y = SampleOffsets[SampleIndex + 0].Y;
			if(SampleIndex + 1 < NumSamples)
			{
				PackedSampleOffsets[SampleIndex / 2].Z = SampleOffsets[SampleIndex + 1].X;
				PackedSampleOffsets[SampleIndex / 2].W = SampleOffsets[SampleIndex + 1].Y;
			}
		}
		SetPixelShaderValues(GetPixelShader(),SampleOffsetsParameter,PackedSampleOffsets,NumSampleChunks);

		// Calculate the screen space radius of the input view space distance
		FLOAT ScreenSpaceRadius = 0.0f;
		if (bHorizontal)
		{
			ScreenSpaceRadius = AOSettings.FilterDistanceScale 
				* View.ScreenPositionScaleBias.X 
				* View.ProjectionMatrix.M[0][0] 
				* GSceneRenderTargets.GetBufferSizeX() 
				/ (.5f * NumSamples);
		}
		else
		{
			ScreenSpaceRadius = AOSettings.FilterDistanceScale 
				* Abs(View.ScreenPositionScaleBias.Y) 
				* View.ProjectionMatrix.M[1][1] 
				* GSceneRenderTargets.GetBufferSizeY() 
				/ (.5f * NumSamples);
		}

		SetPixelShaderValue(GetPixelShader(), FilterParameters, FVector4(AOSettings.EdgeDistanceThreshold, AOSettings.EdgeDistanceScale, ScreenSpaceRadius, 0.0f));
	}

private:

	FAmbientOcclusionParams AOParams;
	FShaderParameter SampleOffsetsParameter;
	FShaderParameter FilterParameters;
};

IMPLEMENT_SHADER_TYPE(,FEdgePreservingFilterVertexShader,TEXT("AmbientOcclusionShader"),TEXT("FilterVertexMain"),SF_Vertex,0,0);

/** A macro for implementing a specific permutation of the edge filter pixel shader. */
#define IMPLEMENT_EDGEFILTER_SHADER_TYPE(NumSamples) \
IMPLEMENT_SHADER_TYPE(template<>,TEdgePreservingFilterPixelShader<NumSamples>,TEXT("AmbientOcclusionShader"),TEXT("FilterPixelMain"),SF_Pixel,0,0);

const INT MinFilterSamples = 2;
const INT MaxFilterSamples = 30;

// Implement supported filter kernel sizes, bounded by MinFilterSamples and MaxFilterSamples.
IMPLEMENT_EDGEFILTER_SHADER_TYPE(2);
IMPLEMENT_EDGEFILTER_SHADER_TYPE(4);
IMPLEMENT_EDGEFILTER_SHADER_TYPE(6);
IMPLEMENT_EDGEFILTER_SHADER_TYPE(8);
IMPLEMENT_EDGEFILTER_SHADER_TYPE(10);
IMPLEMENT_EDGEFILTER_SHADER_TYPE(12);
IMPLEMENT_EDGEFILTER_SHADER_TYPE(14);
IMPLEMENT_EDGEFILTER_SHADER_TYPE(16);
IMPLEMENT_EDGEFILTER_SHADER_TYPE(18);
IMPLEMENT_EDGEFILTER_SHADER_TYPE(20);
IMPLEMENT_EDGEFILTER_SHADER_TYPE(22);
IMPLEMENT_EDGEFILTER_SHADER_TYPE(24);
IMPLEMENT_EDGEFILTER_SHADER_TYPE(26);
IMPLEMENT_EDGEFILTER_SHADER_TYPE(28);
IMPLEMENT_EDGEFILTER_SHADER_TYPE(30);

/** 
 * Filter the occlusion values to reduce noise, preserving boundaries between objects.
 */
void EdgePreservingFilter(const FViewInfo& View, const FDownsampleDimensions& DownsampleDimensions, UBOOL bHorizontal, const FAmbientOcclusionSettings& AOSettings)
{
	// Calculate how many samples are needed based on the filter size and downsample factor.
	INT NumFilterSamples = AOSettings.FilterSize / GSceneRenderTargets.GetAmbientOcclusionDownsampleFactor();
	// Clamp the number of samples to the range supported by implemented filter shaders, round to nearest even number of samples.
	NumFilterSamples = Min(NumFilterSamples - NumFilterSamples % 2, MaxFilterSamples);

	if (NumFilterSamples < MinFilterSamples)
	{
		return;
	}

	const UBOOL bUseDepthBufferForAO = UseDepthBufferForAO(DownsampleDimensions);
	SCOPED_DRAW_EVENT(FilterEventView)(DEC_SCENE_ITEMS,TEXT("EdgePreservingFilter"));

	GSceneRenderTargets.BeginRenderingAmbientOcclusion(bUseDepthBufferForAO);

	RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
	RHISetBlendState(TStaticBlendState<>::GetRHI());
	RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());

	if (bUseDepthBufferForAO)
	{
		// Pass if zero
		RHISetStencilState(TStaticStencilState<
			TRUE,CF_Equal,SO_Keep,SO_Keep,SO_Keep,
			FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
			0xff,0xff,0
		>::GetRHI());

		// Use the Hi Stencil mask setup earlier
		// Don't flush Hi Stencil since it was flushed during the Occlusion pass
		RHIBeginHiStencilPlayback(FALSE);
	}

	// Setup offsets to sample each horizontal or vertical texel, skipping over the current texel.
	FVector2D SampleOffsets[MaxFilterSamples];
	for (INT i = 0; i < NumFilterSamples; i++)
	{
		INT Offset = i - NumFilterSamples / 2;
		// Skip the current texel, which has an offset of 0.
		if (Offset >= 0)
		{
			Offset++;
		}

		if (bHorizontal)
		{
			SampleOffsets[i] = FVector2D(Offset, 0.0f);
		}
		else
		{
			SampleOffsets[i] = FVector2D(0.0f, Offset);
		}
	}

	const FVector2D AmbientOcclusionTexelSize = FVector2D(
		1.0f / (FLOAT)GSceneRenderTargets.GetAmbientOcclusionBufferSizeX(), 
		1.0f / (FLOAT)GSceneRenderTargets.GetAmbientOcclusionBufferSizeY());

	// Convert texel offsets into texture coordinate offsets.
	for (INT i = 0; i < NumFilterSamples; i++)
	{
		SampleOffsets[i] *= AmbientOcclusionTexelSize;
	}

	RHISetViewport(DownsampleDimensions.TargetX, DownsampleDimensions.TargetY, 0.0f, 
		DownsampleDimensions.TargetX + DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetY + DownsampleDimensions.TargetSizeY, 1.0f);				

	TShaderMapRef<FEdgePreservingFilterVertexShader> VertexShader(GetGlobalShaderMap());

	// A macro to handle setting the filter shader for a specific number of samples.
#define SET_FILTER_SHADER_TYPE(NumSamples) \
	case NumSamples: \
	{ \
		TShaderMapRef<TEdgePreservingFilterPixelShader<NumSamples> > PixelShader(GetGlobalShaderMap()); \
		PixelShader->SetParameters(View, DownsampleDimensions, AOSettings, bHorizontal, SampleOffsets); \
		{ \
			static FGlobalBoundShaderState FilterBoundShaderState[2]; \
			SetGlobalBoundShaderState(FilterBoundShaderState[bHorizontal], GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FFilterVertex)); \
		} \
		break; \
	};

	// Set the appropriate filter shader for the given number of samples.
	switch(NumFilterSamples)
	{
		SET_FILTER_SHADER_TYPE(2);
		SET_FILTER_SHADER_TYPE(4);
		SET_FILTER_SHADER_TYPE(6);
		SET_FILTER_SHADER_TYPE(8);
		SET_FILTER_SHADER_TYPE(10);
		SET_FILTER_SHADER_TYPE(12);
		SET_FILTER_SHADER_TYPE(14);
		SET_FILTER_SHADER_TYPE(16);
		SET_FILTER_SHADER_TYPE(18);
		SET_FILTER_SHADER_TYPE(20);
		SET_FILTER_SHADER_TYPE(22);
		SET_FILTER_SHADER_TYPE(24);
		SET_FILTER_SHADER_TYPE(26);
		SET_FILTER_SHADER_TYPE(28);
		SET_FILTER_SHADER_TYPE(30);
		default:
			appErrorf(TEXT("Invalid number of filter samples: %u"), NumFilterSamples);
	}

#undef SET_FILTER_SHADER_TYPE

	DrawDenormalizedQuad(
		0, 0,
		DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetSizeY,
		DownsampleDimensions.TargetX, DownsampleDimensions.TargetY,
		DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetSizeY,
		DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetSizeY,
		GSceneRenderTargets.GetAmbientOcclusionBufferSizeX(), GSceneRenderTargets.GetAmbientOcclusionBufferSizeY()
		);
	
	if (bUseDepthBufferForAO)
	{
		RHIEndHiStencil();
	}

	GSceneRenderTargets.FinishRenderingAmbientOcclusion(
		FResolveParams(
			DownsampleDimensions.TargetX,
			DownsampleDimensions.TargetY, 
			DownsampleDimensions.TargetX + DownsampleDimensions.TargetSizeX,
			DownsampleDimensions.TargetY + DownsampleDimensions.TargetSizeY
			)
		);
}

/*-----------------------------------------------------------------------------
	FHistoryUpdateVertexShader
-----------------------------------------------------------------------------*/

class FHistoryUpdateVertexShader : public FShader
{
	DECLARE_SHADER_TYPE(FHistoryUpdateVertexShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) 
	{ 
		return Platform != SP_PCD3D_SM2; 
	}

	FHistoryUpdateVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
		ScreenToWorldOffsetParameter.Bind(Initializer.ParameterMap,TEXT("ScreenToWorldOffset"), TRUE);
	}

	FHistoryUpdateVertexShader() {}

	void SetParameters(const FViewInfo& View)
	{
		// Remove translation to the world origin to avoid floating point precision issues far from the origin.
		FMatrix InvViewRotationProjMatrix = View.InvProjectionMatrix * View.ViewMatrix.RemoveTranslation().Inverse();
		FMatrix ScreenToWorldOffset = FMatrix(
			FPlane(1,0,0,0),
			FPlane(0,1,0,0),
			FPlane(0,0,(1.0f - Z_PRECISION),1),
			FPlane(0,0,-View.NearClippingDistance * (1.0f - Z_PRECISION),0)
			) *
			InvViewRotationProjMatrix;

		SetVertexShaderValue(GetVertexShader(), ScreenToWorldOffsetParameter, ScreenToWorldOffset);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << ScreenToWorldOffsetParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderParameter ScreenToWorldOffsetParameter;
};

IMPLEMENT_SHADER_TYPE(,FHistoryUpdateVertexShader,TEXT("AmbientOcclusionShader"),TEXT("HistoryUpdateVertexMain"),SF_Vertex,0,0);

/*-----------------------------------------------------------------------------
	FStaticHistoryUpdatePixelShader
-----------------------------------------------------------------------------*/

class FStaticHistoryUpdatePixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FStaticHistoryUpdatePixelShader,Global);
public:

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		//new(OutEnvironment.CompilerFlags) ECompilerFlags(CFLAG_Debug);
	}

	static UBOOL ShouldCache(EShaderPlatform Platform) 
	{ 
		return Platform != SP_PCD3D_SM2; 
	}

	FStaticHistoryUpdatePixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
		AOParams.Bind(Initializer.ParameterMap);
		PrevViewProjMatrixParameter.Bind(Initializer.ParameterMap,TEXT("PrevViewProjMatrix"), TRUE);
		HistoryConvergenceRatesParameter.Bind(Initializer.ParameterMap,TEXT("HistoryConvergenceRates"), TRUE);
	}

	FStaticHistoryUpdatePixelShader() 
	{
	}

	void SetParameters(
		const FViewInfo& View, 
		const FDownsampleDimensions& DownsampleDimensions, 
		const FAmbientOcclusionSettings& AOSettings, 
		FLOAT OcclusionConvergenceRate,
		FLOAT WeightConvergenceRate)
	{
		AOParams.Set(DownsampleDimensions, this);

		// Instead of finding the world space position of the current pixel, calculate the world space position offset by the camera position, 
		// then translate by the difference between last frame's camera position and this frame's camera position,
		// then apply the rest of the transforms.  This effectively avoids precision issues near the extents of large levels whose world space position is very large.
		FVector ViewOriginDelta = View.ViewOrigin - View.PrevViewOrigin;
		SetPixelShaderValue(GetPixelShader(), PrevViewProjMatrixParameter, FTranslationMatrix(ViewOriginDelta) * View.PrevViewRotationProjMatrix);

		SetPixelShaderValue(GetPixelShader(), HistoryConvergenceRatesParameter, FVector2D(OcclusionConvergenceRate, WeightConvergenceRate));
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << AOParams;
		Ar << PrevViewProjMatrixParameter;
		Ar << HistoryConvergenceRatesParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FAmbientOcclusionParams AOParams;
	FShaderParameter PrevViewProjMatrixParameter;
	FShaderParameter HistoryConvergenceRatesParameter;
};

IMPLEMENT_SHADER_TYPE(,FStaticHistoryUpdatePixelShader,TEXT("AmbientOcclusionShader"),TEXT("StaticHistoryUpdatePixelMain"),SF_Pixel,0,0);

FGlobalBoundShaderState HistoryUpdateBoundShaderState;


/*-----------------------------------------------------------------------------
	FMeshMaskVertexShader
-----------------------------------------------------------------------------*/

template<UBOOL bSupportPrevWorldPosition>
class TAOMeshVertexShader : public FShader
{
	DECLARE_SHADER_TYPE(TAOMeshVertexShader,MeshMaterial);
public:
	static UBOOL ShouldCache(EShaderPlatform Platform, const FMaterial* Material, const FVertexFactoryType* VertexFactoryType)
	{
		if (bSupportPrevWorldPosition)
		{
			// Only compile for vertex factories which support a precise previous world position
			if (VertexFactoryType->SupportsPrecisePrevWorldPos())
			{
				return Material->IsSpecialEngineMaterial() && Platform != SP_PCD3D_SM2;
			}
			return FALSE;
		}
		else
		{
			// Only compile for the default material
			return Material->IsSpecialEngineMaterial() && Platform != SP_PCD3D_SM2;
		}
	}

	TAOMeshVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FShader(Initializer),
		VertexFactoryParameters(Initializer.VertexFactoryType, Initializer.ParameterMap)
	{
		PrevViewProjectionMatrixParameter.Bind(Initializer.ParameterMap,TEXT("PrevViewProjectionMatrix"), TRUE);
		PreviousLocalToWorldParameter.Bind(Initializer.ParameterMap,TEXT("PreviousLocalToWorld"), TRUE);
	}

	TAOMeshVertexShader() {}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		bShaderHasOutdatedParameters |= Ar << VertexFactoryParameters;
		Ar << PrevViewProjectionMatrixParameter;
		Ar << PreviousLocalToWorldParameter;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(const FVertexFactory* VertexFactory, const FMaterialRenderProxy* MaterialRenderProxy, const FViewInfo& View)
	{
		VertexFactoryParameters.Set(this, VertexFactory, View);
		SetVertexShaderValue(
			GetVertexShader(), 
			PrevViewProjectionMatrixParameter,
			FTranslationMatrix(-View.PreViewTranslation) * View.PrevViewProjMatrix
			);
	}

	void SetMesh(const FMeshElement& Mesh, const FSceneView& View, const FMatrix& PreviousLocalToWorld)
	{
		VertexFactoryParameters.SetMesh(this, Mesh, View);
		SetVertexShaderValue(
			GetVertexShader(),
			PreviousLocalToWorldParameter,
			PreviousLocalToWorld.ConcatTranslation(View.PreViewTranslation)
			);
	}

private:

	FVertexFactoryParameterRef VertexFactoryParameters;
	FShaderParameter PrevViewProjectionMatrixParameter;
	FShaderParameter PreviousLocalToWorldParameter;
};

IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TAOMeshVertexShader<TRUE>,TEXT("AmbientOcclusionMeshShaders"),TEXT("MeshHistoryUpdateVertexMain"),SF_Vertex,0,0);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TAOMeshVertexShader<FALSE>,TEXT("AmbientOcclusionMeshShaders"),TEXT("MeshAOMaskVertexMain"),SF_Vertex,0,0);

/*-----------------------------------------------------------------------------
	TAOMaskPixelShader
-----------------------------------------------------------------------------*/

template<EAOMaskType MaskType>
class TAOMaskPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(TAOMaskPixelShader,Global);
public:
	static UBOOL ShouldCache(EShaderPlatform Platform) 
	{ 
		return Platform != SP_PCD3D_SM2; 
	}

	TAOMaskPixelShader() {}

	TAOMaskPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FShader(Initializer)
	{
		AOParams.Bind(Initializer.ParameterMap);
		HistoryConvergenceRatesParameter.Bind(Initializer.ParameterMap,TEXT("HistoryConvergenceRates"), TRUE);
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		const UBOOL bUseManualDepthTest = MaskType == AO_HistoryMaskManualDepthTest || MaskType == AO_HistoryUpdateManualDepthTest;
		OutEnvironment.Definitions.Set(TEXT("USE_MANUAL_DEPTH_TEST"), bUseManualDepthTest ? TEXT("1") : TEXT("0"));
	}

	void SetParameters(const FDownsampleDimensions& DownsampleDimensions, FLOAT OcclusionConvergenceRate, FLOAT WeightConvergenceRate)
	{
		AOParams.Set(DownsampleDimensions, this);
		SetPixelShaderValue(GetPixelShader(), HistoryConvergenceRatesParameter, FVector2D(OcclusionConvergenceRate, WeightConvergenceRate));
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << AOParams;
		Ar << HistoryConvergenceRatesParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FAmbientOcclusionParams AOParams;
	FShaderParameter HistoryConvergenceRatesParameter;
};

IMPLEMENT_SHADER_TYPE(template<>,TAOMaskPixelShader<AO_OcclusionMask>,TEXT("AmbientOcclusionShader"),TEXT("OcclusionMaskPixelMain"),SF_Pixel,0,0);
IMPLEMENT_SHADER_TYPE(template<>,TAOMaskPixelShader<AO_HistoryMask>,TEXT("AmbientOcclusionShader"),TEXT("HistoryMaskPixelMain"),SF_Pixel,0,0);
IMPLEMENT_SHADER_TYPE(template<>,TAOMaskPixelShader<AO_HistoryUpdate>,TEXT("AmbientOcclusionShader"),TEXT("DynamicHistoryUpdatePixelMain"),SF_Pixel,0,0);
IMPLEMENT_SHADER_TYPE(template<>,TAOMaskPixelShader<AO_HistoryMaskManualDepthTest>,TEXT("AmbientOcclusionShader"),TEXT("HistoryMaskPixelMain"),SF_Pixel,0,0);
IMPLEMENT_SHADER_TYPE(template<>,TAOMaskPixelShader<AO_HistoryUpdateManualDepthTest>,TEXT("AmbientOcclusionShader"),TEXT("DynamicHistoryUpdatePixelMain"),SF_Pixel,0,0);

FAOMaskDrawingPolicy::FAOMaskDrawingPolicy(
	const FVertexFactory* InVertexFactory,
	const FMaterialRenderProxy* InMaterialRenderProxy,
	UBOOL bAllowAmbientOcclusion,
	FLOAT DepthBias
	) :	
	FMeshDrawingPolicy(InVertexFactory, InMaterialRenderProxy, FALSE, FALSE, DepthBias),
	HistoryUpdateVertexShader(NULL),
	MaskVertexShader(NULL),
	OcclusionMaskPixelShader(NULL),
	HistoryMaskPixelShader(NULL),
	HistoryUpdatePixelShader(NULL),
	HistoryMaskManualDepthTestPixelShader(NULL),
	HistoryUpdateManualDepthTestPixelShader(NULL)
{
	const FMaterial* MaterialResource = InMaterialRenderProxy->GetMaterial();

	if (bAllowAmbientOcclusion)
	{
		// If ambient occlusion is allowed on this mesh, it is being rendered because it moved since last frame.
		// If the mesh's vertex factory supports calculating last frame's world position accurately, 
		// we can use it to update the history even though the mesh has moved.
		if (InVertexFactory->GetType()->SupportsPrecisePrevWorldPos())
		{
			
			HistoryUpdateVertexShader = MaterialResource->GetShader<TAOMeshVertexShader<TRUE> >(InVertexFactory->GetType());
			if (UseDepthBufferForAO(DownsampleDimensions))
			{
				MaskType = AO_HistoryUpdate;
				HistoryUpdatePixelShader = GetGlobalShaderMap()->GetShader<TAOMaskPixelShader<AO_HistoryUpdate> >();
			}
			else
			{
				// Depth testing will be disabled so we need to do manual depth testing in the shader
				MaskType = AO_HistoryUpdateManualDepthTest;
				HistoryUpdateManualDepthTestPixelShader = GetGlobalShaderMap()->GetShader<TAOMaskPixelShader<AO_HistoryUpdateManualDepthTest> >();
			}
		}
		else
		{
			MaskVertexShader = MaterialResource->GetShader<TAOMeshVertexShader<FALSE> >(InVertexFactory->GetType());
			// The mesh's VF doesn't support a precise prev world pos, the best we can do is discard the history.
			if (UseDepthBufferForAO(DownsampleDimensions))
			{
				MaskType = AO_HistoryMask;
				HistoryMaskPixelShader = GetGlobalShaderMap()->GetShader<TAOMaskPixelShader<AO_HistoryMask> >();
			}
			else
			{
				// Depth testing will be disabled so we need to do manual depth testing in the shader
				MaskType = AO_HistoryMaskManualDepthTest;
				HistoryMaskManualDepthTestPixelShader = GetGlobalShaderMap()->GetShader<TAOMaskPixelShader<AO_HistoryMaskManualDepthTest> >();
			}
		}
	}
	else
	{
		// Ambient occlusion is not allowed on this mesh, so write out a mask to the current render target.
		// If the current render target is the occlusion buffer, the mask will keep any occlusion from being contributed.
		// Otherwise the current render target is the history buffer, and the mask will discard the history.
		MaskType = AO_OcclusionMask;
		MaskVertexShader = MaterialResource->GetShader<TAOMeshVertexShader<FALSE> >(InVertexFactory->GetType());
		OcclusionMaskPixelShader = GetGlobalShaderMap()->GetShader<TAOMaskPixelShader<AO_OcclusionMask> >();
	}
}

void FAOMaskDrawingPolicy::DrawShared(const FViewInfo& View, FBoundShaderStateRHIRef ShaderState) const
{
	RHISetBoundShaderState(ShaderState);

	if (MaskType == AO_HistoryUpdate)
	{
		HistoryUpdateVertexShader->SetParameters(VertexFactory, MaterialRenderProxy, View);
		HistoryUpdatePixelShader->SetParameters(DownsampleDimensions, OcclusionConvergenceRate, WeightConvergenceRate);
	}
	else if (MaskType == AO_HistoryUpdateManualDepthTest)
	{
		HistoryUpdateVertexShader->SetParameters(VertexFactory, MaterialRenderProxy, View);
		HistoryUpdateManualDepthTestPixelShader->SetParameters(DownsampleDimensions, OcclusionConvergenceRate, WeightConvergenceRate);
	}
	else
	{
		MaskVertexShader->SetParameters(VertexFactory, MaterialRenderProxy, View);
		if (MaskType == AO_OcclusionMask)
		{
			OcclusionMaskPixelShader->SetParameters(DownsampleDimensions, 0.0f, 0.0f);
		}
		else if (MaskType == AO_HistoryMask)
		{
			HistoryMaskPixelShader->SetParameters(DownsampleDimensions, 0.0f, 0.0f);
		}
		else if (MaskType == AO_HistoryMaskManualDepthTest)
		{
			HistoryMaskManualDepthTestPixelShader->SetParameters(DownsampleDimensions, 0.0f, 0.0f);
		}
	}

	// Set the shared mesh resources.
	FMeshDrawingPolicy::DrawShared(&View);
}

void FAOMaskDrawingPolicy::SetMeshRenderState(
	const FViewInfo& View,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	const ElementDataType& ElementData
	) const
{
	if (MaskType == AO_HistoryUpdate || MaskType == AO_HistoryUpdateManualDepthTest)
	{
		const FMotionBlurInfo* MotionBlurInfo = NULL;
		const UBOOL bSuccess = FScene::GetPrimitiveMotionBlurInfo(PrimitiveSceneInfo, MotionBlurInfo);
		check(bSuccess);
		// The vertex shader needs PreviousLocalToWorld to calculate last frame's world position
		HistoryUpdateVertexShader->SetMesh(Mesh, View, MotionBlurInfo->PreviousLocalToWorld);
	}
	else
	{
		MaskVertexShader->SetMesh(Mesh, View, Mesh.LocalToWorld);
	}

	FMeshDrawingPolicy::SetMeshRenderState(View, PrimitiveSceneInfo, Mesh, bBackFace, ElementData);
}

/** 
 * Create bound shader state using the vertex decl from the mesh draw policy
 * as well as the shaders needed to draw the mesh
 * @param DynamicStride - optional stride for dynamic vertex data
 * @return new bound shader state object
 */
FBoundShaderStateRHIRef FAOMaskDrawingPolicy::CreateBoundShaderState(DWORD DynamicStride)
{
	FVertexDeclarationRHIRef VertexDeclaration;
	DWORD StreamStrides[MaxVertexElementCount];

	FMeshDrawingPolicy::GetVertexDeclarationInfo(VertexDeclaration, StreamStrides);
	if (DynamicStride)
	{
		StreamStrides[0] = DynamicStride;
	}

	FShader* PixelShader = NULL;
	FShader* VertexShader = NULL;

	if (MaskType == AO_HistoryUpdate)
	{
		VertexShader = HistoryUpdateVertexShader;
		PixelShader = HistoryUpdatePixelShader;
	}
	else if (MaskType == AO_HistoryUpdateManualDepthTest)
	{
		VertexShader = HistoryUpdateVertexShader;
		PixelShader = HistoryUpdateManualDepthTestPixelShader;
	}
	else
	{
		VertexShader = MaskVertexShader;
		if (MaskType == AO_OcclusionMask)
		{
			PixelShader = OcclusionMaskPixelShader;
		}
		else if (MaskType == AO_HistoryMask)
		{
			PixelShader = HistoryMaskPixelShader;
		}
		else if (MaskType == AO_HistoryMaskManualDepthTest)
		{
			PixelShader = HistoryMaskManualDepthTestPixelShader;
		}
	}

	return RHICreateBoundShaderState(VertexDeclaration, StreamStrides, VertexShader->GetVertexShader(), PixelShader->GetPixelShader());	
}

INT Compare(const FAOMaskDrawingPolicy& A, const FAOMaskDrawingPolicy& B)
{
	COMPAREDRAWINGPOLICYMEMBERS(MaskType);
	// No need to compare pixel shaders as they are determined by MaskType
	COMPAREDRAWINGPOLICYMEMBERS(MaskVertexShader);
	COMPAREDRAWINGPOLICYMEMBERS(HistoryUpdateVertexShader);
	COMPAREDRAWINGPOLICYMEMBERS(VertexFactory);
	return 0;
}

/** Indicates whether the primitive has moved since last frame. */
UBOOL FAOMaskDrawingPolicy::HasMoved(const FPrimitiveSceneInfo* PrimitiveSceneInfo)
{
	const FMatrix* CheckMatrix = NULL;
	const FMotionBlurInfo* MBInfo;
	if ( FScene::GetPrimitiveMotionBlurInfo(PrimitiveSceneInfo, MBInfo) )
	{
		CheckMatrix = &(MBInfo->PreviousLocalToWorld);
	}

	// Check if the primitive has moved based on a comparison of LocalToWorld's
	// This only handles movement that changes the LocalToWorld matrix
	if (CheckMatrix && !PrimitiveSceneInfo->Component->LocalToWorld.Equals(*CheckMatrix, 0.0001f))
	{
		return TRUE;
	}

	// Either the primitive hasn't moved or there wasn't enough information to decide.
	return FALSE;
}

FDownsampleDimensions FAOMaskDrawingPolicy::DownsampleDimensions;
FLOAT FAOMaskDrawingPolicy::OcclusionConvergenceRate;
FLOAT FAOMaskDrawingPolicy::WeightConvergenceRate;

UBOOL FAOMaskDrawingPolicyFactory::DrawDynamicMesh(
	const FViewInfo& View,
	ContextType DrawingContext,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	UBOOL bPreFog,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	FHitProxyId HitProxyId
	)
{
	const FMaterialRenderProxy* MaterialRenderProxy = Mesh.MaterialRenderProxy;
	const FMaterial* Material = MaterialRenderProxy->GetMaterial();
	EBlendMode BlendMode = Material->GetBlendMode();
	// Only add primitives with opaque or masked materials, unless they have a decal material.
	if ((BlendMode == BLEND_Opaque || BlendMode == BLEND_Masked) && !Material->IsDecalMaterial())
	{
		// Mask out occlusion and history for any occluder in the foreground DPG.
		const UBOOL bAllowAmbientOcclusion = PrimitiveSceneInfo->bAllowAmbientOcclusion && DrawingContext.DPG != SDPG_Foreground;
		// Override with the default material.
		// Masked and two-sided materials are not handled separately, but the artifacts are normally not severe.
		FAOMaskDrawingPolicy DrawingPolicy(Mesh.VertexFactory, GEngine->DefaultMaterial->GetRenderProxy(FALSE), bAllowAmbientOcclusion, DrawingContext.DepthBias);
		DrawingPolicy.DrawShared(View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
		DrawingPolicy.SetMeshRenderState(View, PrimitiveSceneInfo, Mesh, bBackFace, FMeshDrawingPolicy::ElementDataType());
		DrawingPolicy.DrawMesh(Mesh);
		return TRUE;
	}
	return FALSE;
}

UBOOL FAOMaskDrawingPolicyFactory::IsMaterialIgnored(const FMaterialRenderProxy* MaterialRenderProxy)
{
	// Ignore primitives with translucent materials
	return IsTranslucentBlendMode(MaterialRenderProxy->GetMaterial()->GetBlendMode());
}

/**
 * Updates the occlusion history buffer
 */
void HistoryUpdate(const FScene* Scene, UINT InDepthPriorityGroup, const FViewInfo& View, const FDownsampleDimensions& DownsampleDimensions, const FAmbientOcclusionSettings& AOSettings)
{
	if (GSceneRenderTargets.bAOHistoryNeedsCleared )
	{
		// Clear the entire history buffer to ensure the contents are valid. (no NAN/INFs)
		// This can't be done in InitDynamicRHI since that is called on some platforms (PS3) before the shader cache has been initialized,
		// and shaders are required for clearing the AO history format.
		GSceneRenderTargets.BeginRenderingAOHistory();
		// Set the viewport to the current view in the occlusion history buffer.
		RHISetViewport(0, 0, 0.0f, GSceneRenderTargets.GetAmbientOcclusionBufferSizeX(), GSceneRenderTargets.GetAmbientOcclusionBufferSizeY(), 1.0f);				
		RHIClear(TRUE, FLinearColor::Black, FALSE, 0, FALSE, 0);
		GSceneRenderTargets.FinishRenderingAOHistory(FResolveParams());
		GSceneRenderTargets.bAOHistoryNeedsCleared = FALSE;
	}

	//@todo - would like to have access to avg framerate, but that is compiled out in final release
	const FLOAT CurrentFPS = 1.0f / Max(View.Family->DeltaWorldTime, 0.0001f);
	// Calculate framerate dependent convergence rate using the approximation that a weight of W will converge in 1 / (1 - W) frames.
	FLOAT OcclusionConvergenceRate = Clamp(1.0f - 1.0f / (CurrentFPS * AOSettings.HistoryOcclusionConvergenceTime), 0.0f, .9999f);
	// Weight converges linearly
	FLOAT WeightConvergenceRate = Clamp(View.Family->DeltaWorldTime / AOSettings.HistoryWeightConvergenceTime, 0.0001f, 1.0f);
	if (View.bPrevTransformsReset)
	{
		// Discard the occlusion history on frames when the previous transforms have been reset
		// This will avoid occlusion in the wrong places after a camera cut
		OcclusionConvergenceRate = 0.0f;
		WeightConvergenceRate = 0.0f;
	}

	const UBOOL bUseDepthBufferForAO = UseDepthBufferForAO(DownsampleDimensions);
	if (bUseDepthBufferForAO)
	{
		// Bind the ambient occlusion history buffer as a render target.
		GSceneRenderTargets.BeginRenderingAOHistory(TRUE);
		// Clear to unoccluded (R channel of 1.0f) and fully converged (G channel of 0.0f)
		// This is necessary because we are using a stencil mask that keeps all pixels from being touched
		RHIClear(TRUE, FLinearColor(1.0f, 0.0f, 0.0f, 0.0f), FALSE, 0, FALSE, 0);
		// Pass if zero
		RHISetStencilState(TStaticStencilState<
			TRUE,CF_Equal,SO_Keep,SO_Keep,SO_Keep,
			FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
			0xff,0xff,0
		>::GetRHI());
		// Use the Hi Stencil mask setup earlier
		// Don't flush Hi Stencil since it was flushed during the Occlusion pass
		RHIBeginHiStencilPlayback(FALSE);
		RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
		// Disable depth test and writes
		RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
		RHISetBlendState(TStaticBlendState<>::GetRHI());
	}
	else
	{
		// Bind the ambient occlusion history buffer as a render target.
		GSceneRenderTargets.BeginRenderingAOHistory(FALSE);
	}

	// Draw a full screen quad to update the occlusion history.
	// This pass assumes that the world space position this frame is the same as the world space position last frame,
	// so it is only correct for static primitives.  Movable primitives will streak occlusion and need to be handled separately.
	{
		SCOPED_DRAW_EVENT(OcclusionEventView)(DEC_SCENE_ITEMS,TEXT("StaticHistoryUpdate"));

		TShaderMapRef<FHistoryUpdateVertexShader> VertexShader(GetGlobalShaderMap());
		TShaderMapRef<FStaticHistoryUpdatePixelShader> PixelShader(GetGlobalShaderMap());

		SetGlobalBoundShaderState(HistoryUpdateBoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FFilterVertex));

		// Set the viewport to the current view in the occlusion history buffer.
		RHISetViewport(DownsampleDimensions.TargetX, DownsampleDimensions.TargetY, 0.0f, 
			DownsampleDimensions.TargetX + DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetY + DownsampleDimensions.TargetSizeY, 1.0f);				

		VertexShader->SetParameters(View);
		PixelShader->SetParameters(View, DownsampleDimensions, AOSettings, OcclusionConvergenceRate, WeightConvergenceRate);

		DrawDenormalizedQuad(
			0, 0,
			DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetSizeY,
			DownsampleDimensions.TargetX, DownsampleDimensions.TargetY,
			DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetSizeY,
			DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetSizeY,
			GSceneRenderTargets.GetAmbientOcclusionBufferSizeX(), GSceneRenderTargets.GetAmbientOcclusionBufferSizeY()
			);
	}
	
	// Render each visible moving primitive, and handle the history appropriately.
	// Depth testing must be handled manually, since the AO History render target is potentially smaller than the depth buffer.
	{
		SCOPED_DRAW_EVENT(OcclusionEventView2)(DEC_SCENE_ITEMS,TEXT("DynamicHistoryUpdate"));
		// Allocate more GPR's for vertex shaders now since movable meshes are usually vertex ALU heavy (skeletal meshes)
		RHISetShaderRegisterAllocation(96, 32);

		FLOAT DepthBias = 0.0f;
		if (bUseDepthBufferForAO)
		{
			// Using the quarter sized depth buffer
			// Z writes off, depth tests on
			RHISetDepthState(TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());
			// Use a depth bias as the positions calculated here will be slightly different from the ones in the downsampled depth buffer.
			DepthBias = -0.0001f;
		}
		else
		{
			RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
		}

		FAOMaskDrawingPolicy::DownsampleDimensions = DownsampleDimensions;
		FAOMaskDrawingPolicy::OcclusionConvergenceRate = OcclusionConvergenceRate;
		FAOMaskDrawingPolicy::WeightConvergenceRate = WeightConvergenceRate;

		TDynamicPrimitiveDrawer<FAOMaskDrawingPolicyFactory> CurrentDPGDrawer(&View, InDepthPriorityGroup, FAOMaskDrawingPolicyFactory::ContextType((ESceneDepthPriorityGroup)InDepthPriorityGroup, DepthBias), TRUE);
		TDynamicPrimitiveDrawer<FAOMaskDrawingPolicyFactory> ForegroundDPGDrawer(&View, SDPG_Foreground, FAOMaskDrawingPolicyFactory::ContextType(SDPG_Foreground, DepthBias), TRUE);
		for(INT PrimitiveIndex = 0; PrimitiveIndex < View.VisibleDynamicPrimitives.Num(); PrimitiveIndex++)
		{
			const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleDynamicPrimitives(PrimitiveIndex);
			const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);
			// Only render if visible.
			if(View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id) 
				// Used to determine whether object is movable or not.
				&& !PrimitiveSceneInfo->bStaticShadowing 
				// Skip translucent objects as they don't support velocities and in the case of particles have a significant CPU overhead.
				&& PrimitiveViewRelevance.bOpaqueRelevance)
			{
				const FLOAT LODFactorDistanceSquared = (PrimitiveSceneInfo->Bounds.Origin - View.ViewOrigin).SizeSquared() * Square(View.LODDistanceFactor);
				// If the primitive has bAllowAmbientOcclusion set to false, we must mask out the history, 
				// Unless this was already done in the DownsampleDepth pass by setting up a stencil mask.
				const UBOOL bMaskHistory = !PrimitiveSceneInfo->bAllowAmbientOcclusion && !bUseDepthBufferForAO;

				// Only render if primitive is relevant to the current DPG.
				const UBOOL bShouldRenderInCurrentDPG = PrimitiveViewRelevance.GetDPG(InDepthPriorityGroup) 
					// Cull primitives that cover a small area of the screen, their occlusion streaking won't be noticeable.
					&& Square(PrimitiveSceneInfo->Bounds.SphereRadius) > MinScreenRadiusForAOMaskingSquared * LODFactorDistanceSquared
					// Only render if occlusion is not allowed on this primitive or the primitive has moved since last frame.
					&& (bMaskHistory || FAOMaskDrawingPolicy::HasMoved(PrimitiveSceneInfo));

				// Always render foreground primitives as they need to mask out the history,
				// Unless this was already done in the DownsampleDepth pass by setting up a stencil mask.
				if (PrimitiveViewRelevance.GetDPG(SDPG_Foreground) && !bUseDepthBufferForAO)
				{
					ForegroundDPGDrawer.SetPrimitive(PrimitiveSceneInfo);
					PrimitiveSceneInfo->Proxy->DrawDynamicElements(
						&ForegroundDPGDrawer,
						&View,
						SDPG_Foreground
						);
				}
				else if (bShouldRenderInCurrentDPG)
				{
					CurrentDPGDrawer.SetPrimitive(PrimitiveSceneInfo);
					PrimitiveSceneInfo->Proxy->DrawDynamicElements(
						&CurrentDPGDrawer,
						&View,
						InDepthPriorityGroup
						);
				}
			}
		}
		RHISetShaderRegisterAllocation(32, 96);
	}

	if (bUseDepthBufferForAO)
	{
		RHIEndHiStencil();
	}

	GSceneRenderTargets.FinishRenderingAOHistory(
		FResolveParams(
			DownsampleDimensions.TargetX,
			DownsampleDimensions.TargetY, 
			DownsampleDimensions.TargetX + DownsampleDimensions.TargetSizeX,
			DownsampleDimensions.TargetY + DownsampleDimensions.TargetSizeY
			)
		);
}

/*-----------------------------------------------------------------------------
	TAOApplyPixelShader
-----------------------------------------------------------------------------*/
enum EAOApplyMode
{
	AOApply_Normal,
	AOApply_ReadFromAOHistory,
	AOApply_ApplyFog,
	AOApply_ApplyFogReadFromHistory,
	AOApply_Max
};

template<EAOApplyMode ApplyMode>
class TAOApplyPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(TAOApplyPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) 
	{ 
		const UBOOL bPlatformSupported = Platform != SP_PCD3D_SM2;
		// Only compile combined fog versions on xbox
		const UBOOL bCombinationSupported = Platform == SP_XBOXD3D || ApplyMode == AOApply_Normal || ApplyMode == AOApply_ReadFromAOHistory;
		return bPlatformSupported && bCombinationSupported; 
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		const UBOOL bApplyFromAOHistory = ApplyMode == AOApply_ReadFromAOHistory || ApplyMode == AOApply_ApplyFogReadFromHistory;
		OutEnvironment.Definitions.Set(TEXT("APPLY_FROM_AOHISTORY"), bApplyFromAOHistory ? TEXT("1") : TEXT("0"));
	}

	TAOApplyPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
	FShader(Initializer)
	{
		AOParams.Bind(Initializer.ParameterMap);
		FogTextureParameter.Bind(Initializer.ParameterMap,TEXT("FogFactorTexture"), TRUE);
		FogColorParameter.Bind(Initializer.ParameterMap,TEXT("FogInScattering"), TRUE);
		TargetSizeParameter.Bind(Initializer.ParameterMap,TEXT("TargetSize"), TRUE);
		OcclusionColorParameter.Bind(Initializer.ParameterMap,TEXT("OcclusionColor"), TRUE);
		InvEncodePowerParameter.Bind(Initializer.ParameterMap,TEXT("InvEncodePower"), TRUE);
	}
	TAOApplyPixelShader() {}

	void SetParameters(const FViewInfo& View, const FDownsampleDimensions& DownsampleDimensions, FLinearColor OcclusionColor, FLinearColor FogColor, UBOOL bIncreaseNearPrecision)
	{
		AOParams.Set(DownsampleDimensions, this);

		SetPixelShaderValue(GetPixelShader(), FogColorParameter, FogColor);

		SetPixelShaderValue(
			GetPixelShader(), 
			TargetSizeParameter, 
			FVector2D(GSceneRenderTargets.GetBufferSizeX(),GSceneRenderTargets.GetBufferSizeY())
			);

		SetTextureParameter(
			GetPixelShader(),
			FogTextureParameter,
			TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
			GSceneRenderTargets.GetFogBufferTexture()
			);

		SetPixelShaderValue(GetPixelShader(), OcclusionColorParameter, OcclusionColor);

		SetPixelShaderValue(GetPixelShader(), InvEncodePowerParameter, bIncreaseNearPrecision ? 1.0f / 8.0f : 1.0f);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << AOParams;
		Ar << OcclusionColorParameter;
		Ar << FogColorParameter;
		Ar << TargetSizeParameter;
		Ar << InvEncodePowerParameter;
		Ar << FogTextureParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FAmbientOcclusionParams AOParams;
	FShaderParameter OcclusionColorParameter;
	FShaderParameter FogColorParameter;
	FShaderParameter TargetSizeParameter;
	FShaderParameter InvEncodePowerParameter;
	FShaderResourceParameter FogTextureParameter;
};

IMPLEMENT_SHADER_TYPE(template<>,TAOApplyPixelShader<AOApply_Normal>,TEXT("AmbientOcclusionShader"),TEXT("AOApplyMain"),SF_Pixel,0,0);
IMPLEMENT_SHADER_TYPE(template<>,TAOApplyPixelShader<AOApply_ReadFromAOHistory>,TEXT("AmbientOcclusionShader"),TEXT("AOApplyMain"),SF_Pixel,0,0);
IMPLEMENT_SHADER_TYPE(template<>,TAOApplyPixelShader<AOApply_ApplyFog>,TEXT("AmbientOcclusionShader"),TEXT("AOAndFogApplyMain"),SF_Pixel,0,0);
IMPLEMENT_SHADER_TYPE(template<>,TAOApplyPixelShader<AOApply_ApplyFogReadFromHistory>,TEXT("AmbientOcclusionShader"),TEXT("AOAndFogApplyMain"),SF_Pixel,0,0);

FGlobalBoundShaderState AOApplyBoundShaderState[AOApply_Max];

/** 
 * Applies the calculated occlusion value to scene color.
 */
void AmbientOcclusionApply(const FScene* Scene, const FViewInfo& View, const FDownsampleDimensions& DownsampleDimensions, FLinearColor OcclusionColor, UBOOL bReadFromHistoryBuffer, UBOOL bApplyFog)
{
	SCOPED_DRAW_EVENT(OcclusionEventView)(DEC_SCENE_ITEMS,TEXT("AmbientOcclusionApply"));
	// Render to the scene color buffer.
	GSceneRenderTargets.BeginRenderingSceneColor();

	if (GRenderAmbientOcclusion && !GAOCombineWithSceneColor)
	{
		// Rendering AO but not scene color, so overwrite previous scene color calculations.
		// This is useful as a visualization of the occlusion factor.
		RHISetBlendState(TStaticBlendState<BO_Add,BF_SourceAlpha,BF_Zero>::GetRHI());
		OcclusionColor = FLinearColor::White;
	}
	else if (GRenderAmbientOcclusion && GAOCombineWithSceneColor && !bApplyFog)
	{
		// Lerp between the occlusion color and existing scene color (which only contains lighting) based on the calculated occlusion factor.
		RHISetBlendState(TStaticBlendState<BO_Add,BF_InverseSourceAlpha,BF_SourceAlpha>::GetRHI());
	}
	else if (GRenderAmbientOcclusion && GAOCombineWithSceneColor && bApplyFog)
	{
		// Filter ambient occlusion through fog
		RHISetBlendState(TStaticBlendState<BO_Add,BF_One,BF_SourceAlpha>::GetRHI());
	}
	else
	{
		check(FALSE);
	}

	RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
	// Disable depth test and writes
	RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
	// Preserve destination scene color alpha, which contains scene depth on some platforms.
	RHISetColorWriteMask(CW_RGB);
	RHISetStencilState(TStaticStencilState<>::GetRHI());

	// Set the viewport to the current view
	RHISetViewport(View.RenderTargetX, View.RenderTargetY, 0.0f, View.RenderTargetX + View.RenderTargetSizeX, View.RenderTargetY + View.RenderTargetSizeY, 1.0f);				

	TShaderMapRef<FAmbientOcclusionVertexShader> VertexShader(GetGlobalShaderMap());
	VertexShader->SetParameters(View);

	if (bReadFromHistoryBuffer)
	{
		if (bApplyFog)
		{
			TShaderMapRef<TAOApplyPixelShader<AOApply_ApplyFogReadFromHistory> > PixelShader(GetGlobalShaderMap());
			PixelShader->SetParameters(View, DownsampleDimensions, OcclusionColor, View.FogInScattering[0], Scene->Fogs(0).bIncreaseNearPrecision);
			SetGlobalBoundShaderState(AOApplyBoundShaderState[AOApply_ApplyFogReadFromHistory], GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FFilterVertex));
		}
		else
		{
			TShaderMapRef<TAOApplyPixelShader<AOApply_ReadFromAOHistory> > PixelShader(GetGlobalShaderMap());
			PixelShader->SetParameters(View, DownsampleDimensions, OcclusionColor, FLinearColor::Black, FALSE);
			SetGlobalBoundShaderState(AOApplyBoundShaderState[AOApply_ReadFromAOHistory], GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FFilterVertex));
		}
	}
	else
	{
		if (bApplyFog)
		{
			TShaderMapRef<TAOApplyPixelShader<AOApply_ApplyFog> > PixelShader(GetGlobalShaderMap());
			PixelShader->SetParameters(View, DownsampleDimensions, OcclusionColor, View.FogInScattering[0], Scene->Fogs(0).bIncreaseNearPrecision);
			SetGlobalBoundShaderState(AOApplyBoundShaderState[AOApply_ApplyFog], GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FFilterVertex));
		}
		else
		{
			TShaderMapRef<TAOApplyPixelShader<AOApply_Normal> > PixelShader(GetGlobalShaderMap());
			PixelShader->SetParameters(View, DownsampleDimensions, OcclusionColor, FLinearColor::Black, FALSE);
			SetGlobalBoundShaderState(AOApplyBoundShaderState[AOApply_Normal], GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FFilterVertex));
		}
	}

	// Draw a full-view quad, with texture coordinates setup to read from the view in the occlusion buffer.
	DrawDenormalizedQuad(
		0, 0,
		View.RenderTargetSizeX, View.RenderTargetSizeY,
		DownsampleDimensions.TargetX, DownsampleDimensions.TargetY,
		DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetSizeY,
		View.RenderTargetSizeX, View.RenderTargetSizeY,
		GSceneRenderTargets.GetAmbientOcclusionBufferSizeX(), GSceneRenderTargets.GetAmbientOcclusionBufferSizeY()
		);

	RHISetColorWriteMask(CW_RGBA);

	GSceneRenderTargets.FinishRenderingSceneColor(!(GRenderAmbientOcclusion && GAOCombineWithSceneColor));
}

FAmbientOcclusionSettings::FAmbientOcclusionSettings(const UAmbientOcclusionEffect* InEffect) :
	OcclusionColor(InEffect->OcclusionColor),
	OcclusionPower(InEffect->OcclusionPower),
	OcclusionScale(InEffect->OcclusionScale),
	OcclusionBias(InEffect->OcclusionBias),
	MinOcclusion(InEffect->MinOcclusion),
	OcclusionRadius(InEffect->OcclusionRadius),
	OcclusionAttenuation(InEffect->OcclusionAttenuation),
	OcclusionQuality((EAmbientOcclusionQuality)InEffect->OcclusionQuality),
	OcclusionFadeoutMinDistance(InEffect->OcclusionFadeoutMinDistance),
	OcclusionFadeoutMaxDistance(InEffect->OcclusionFadeoutMaxDistance),
	HaloDistanceThreshold(InEffect->HaloDistanceThreshold),
	HaloDistanceScale(InEffect->HaloDistanceScale),
	HaloOcclusion(InEffect->HaloOcclusion),
	EdgeDistanceThreshold(InEffect->EdgeDistanceThreshold),
	EdgeDistanceScale(InEffect->EdgeDistanceScale),
	FilterDistanceScale(InEffect->FilterDistanceScale),
	FilterSize(InEffect->FilterSize),
	HistoryOcclusionConvergenceTime(InEffect->HistoryConvergenceTime),
	HistoryWeightConvergenceTime(InEffect->HistoryWeightConvergenceTime)
{
}

/*-----------------------------------------------------------------------------
	FAmbientOcclusionSceneProxy
-----------------------------------------------------------------------------*/

class FAmbientOcclusionSceneProxy : public FPostProcessSceneProxy
{
public:
	/** 
	 * Initialization constructor. 
	 * @param InEffect - effect to mirror in this proxy
	 */
	FAmbientOcclusionSceneProxy(const UAmbientOcclusionEffect* InEffect,const FPostProcessSettings* WorldSettings) :
		FPostProcessSceneProxy(InEffect),
		AOSettings(InEffect)
	{
	}

	UBOOL UseHistorySmoothing(const FViewInfo& View) const
	{
		// Only use the history when realtime update is on, since the algorithm requires continuous frames
		return View.Family->bRealtimeUpdate
			// Disable when using the lowest quality for speed
			&& AOSettings.OcclusionQuality != AO_Low 
			// Disable the history buffer if smoothing won't be noticeable
			&& AOSettings.HistoryOcclusionConvergenceTime > 0.01f
			// Need bilinear filtering for resampling the floating point history buffer
			&& GSupportsFPFiltering
			// Disable the history for tiled screenshots, frames will not be spatially coherent
			&& !GIsTiledScreenshot;
	}

	/**
	 * Render the post process effect
	 * Called by the rendering thread during scene rendering
	 * @param InDepthPriorityGroup - scene DPG currently being rendered
	 * @param View - current view
	 * @param CanvasTransform - same canvas transform used to render the scene
	 * @return TRUE if anything was rendered
	 */
	UBOOL Render(const FScene* Scene, UINT InDepthPriorityGroup, FViewInfo& View, const FMatrix& CanvasTransform)
	{
		if (!GRenderAmbientOcclusion)
		{
			return FALSE;
		}

		// Currently unsupported on SM2
		if (GRHIShaderPlatform == SP_PCD3D_SM2
			|| !GSystemSettings.RenderThreadSettings.bAllowAmbientOcclusion)
		{
			return FALSE;
		}

		// Should be enforced by the post process effect
		check(InDepthPriorityGroup == SDPG_World);

		// Disable downsampling on the high quality setting
		INT NewDownsampleFactor = AOSettings.OcclusionQuality == AO_High ? 1 : 2;
#if XBOX
		// AO buffers won't fit in EDRAM at the same time as scene color and depth unless they are downsampled.
		NewDownsampleFactor = 2;
#endif

		GSceneRenderTargets.SetAmbientOcclusionDownsampleFactor(NewDownsampleFactor);

		SCOPED_DRAW_EVENT(AmbientOcclusionEvent)(DEC_SCENE_ITEMS,TEXT("AmbientOcclusion"));

		FDownsampleDimensions DownsampleDimensions(View);

		const UBOOL bUseHistoryBuffer = UseHistorySmoothing(View);

		// Downsample depth and store it in the occlusion buffer, which reduces texture lookups required during the filter passes.
		const UBOOL bApplyFog = DownsampleDepth(Scene, InDepthPriorityGroup, View, DownsampleDimensions, bUseHistoryBuffer, AOSettings.OcclusionFadeoutMaxDistance);

		// Mark one layer height fog as already rendered for this view
		View.bOneLayerHeightFogRenderedInAO = bApplyFog;

		// Render the occlusion factor
		RenderOcclusion(View, DownsampleDimensions, AOSettings);

		// Filter the occlusion factor to reduce spatial noise
		EdgePreservingFilter(View, DownsampleDimensions, TRUE, AOSettings);
		EdgePreservingFilter(View, DownsampleDimensions, FALSE, AOSettings);

		if (bUseHistoryBuffer)
		{
			// Lerp between the new occlusion value and the running history to reduce temporal noise
			HistoryUpdate(Scene, InDepthPriorityGroup, View, DownsampleDimensions, AOSettings);
		}

		// Apply to scene color
		AmbientOcclusionApply(Scene, View, DownsampleDimensions, AOSettings.OcclusionColor, bUseHistoryBuffer, bApplyFog);

		// Scene color needs to be resolved before being read again.
		return TRUE;
	}

	/**
	 * Tells FSceneRenderer whether to store the previous frame's transforms.
	 */
	virtual UBOOL RequiresPreviousTransforms(const FViewInfo& View) const
	{
		return UseHistorySmoothing(View);
	}

private:
	/** Mirrored properties. */
	FAmbientOcclusionSettings AOSettings;
};

IMPLEMENT_CLASS(UAmbientOcclusionEffect);

/**
 * Creates a proxy to represent the render info for a post process effect
 * @param WorldSettings - The world's post process settings for the view.
 * @return The proxy object.
 */
FPostProcessSceneProxy* UAmbientOcclusionEffect::CreateSceneProxy(const FPostProcessSettings* WorldSettings)
{
	if (!WorldSettings || WorldSettings->bAllowAmbientOcclusion)
	{
		return new FAmbientOcclusionSceneProxy(this,WorldSettings);
	}
	else
	{
		return NULL;
	}
}

/**
 * @param View - current view
 * @return TRUE if the effect should be rendered
 */
UBOOL UAmbientOcclusionEffect::IsShown(const FSceneView* View) const
{
	UBOOL bIsShown = View->Family->bAllowAmbientOcclusion && UPostProcessEffect::IsShown(View);
	if (IsInGameThread())
	{
		bIsShown = bIsShown && GSystemSettings.bAllowAmbientOcclusion;
	}
	else
	{
		bIsShown = bIsShown && GSystemSettings.RenderThreadSettings.bAllowAmbientOcclusion;
	}
	return bIsShown;
}

void UAmbientOcclusionEffect::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	// Maximum world space threshold allowed
	const FLOAT WorldThresholdMax = 5000.0f;

	// Clamp settings to valid ranges.
	SceneDPG = SDPG_World;
	OcclusionPower = Clamp(OcclusionPower, 0.0001f, 50.0f);
	MinOcclusion = Clamp(MinOcclusion, 0.0f, 1.0f);
	OcclusionRadius = Max(0.0f, OcclusionRadius);
	OcclusionAttenuation = Clamp(OcclusionAttenuation, 1.0f, WorldThresholdMax);
	EdgeDistanceThreshold = Clamp(EdgeDistanceThreshold, 0.0f, WorldThresholdMax);
	EdgeDistanceScale = Clamp(EdgeDistanceScale, 0.0f, 1.0f);
	HaloDistanceThreshold = Clamp(HaloDistanceThreshold, 0.0f, WorldThresholdMax);
	HaloOcclusion = Clamp(HaloOcclusion, 0.0f, 100.0f);
	HaloDistanceScale = Clamp(HaloDistanceScale, 0.0f, 1.0f);
	OcclusionFadeoutMinDistance = Clamp(OcclusionFadeoutMinDistance, 0.0f, OcclusionFadeoutMaxDistance);
	OcclusionFadeoutMaxDistance = Clamp(OcclusionFadeoutMaxDistance, OcclusionFadeoutMinDistance, (FLOAT)HALF_WORLD_MAX);
	FilterDistanceScale = Clamp(FilterDistanceScale, 1.0f, WorldThresholdMax);
	FilterSize = Clamp<INT>(FilterSize, 0, 60);
	HistoryConvergenceTime = Clamp(HistoryConvergenceTime, 0.0f, 30.0f);
	HistoryWeightConvergenceTime = Clamp(HistoryWeightConvergenceTime, 0.0001f, 30.0f);
}	
