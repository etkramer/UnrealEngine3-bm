/*=============================================================================
	DOFAndBloomEffect.cpp: Combined depth of field and bloom effect implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "SceneFilterRendering.h"
#include "DOFAndBloomEffect.h"

IMPLEMENT_CLASS(UDOFAndBloomEffect);

/*-----------------------------------------------------------------------------
FDOFShaderParameters
-----------------------------------------------------------------------------*/

/** Initialization constructor. */
FDOFShaderParameters::FDOFShaderParameters(const FShaderParameterMap& ParameterMap)
{
	PackedParameters.Bind(ParameterMap,TEXT("PackedParameters"),TRUE);
	MinMaxBlurClampParameter.Bind(ParameterMap,TEXT("MinMaxBlurClamp"),TRUE);
}

/** Set the dof shader parameter values. */
void FDOFShaderParameters::Set(FShader* PixelShader,FLOAT FocusDistance,FLOAT FocusInnerRadius,FLOAT FalloffExponent,FLOAT MaxNearBlurAmount,FLOAT MaxFarBlurAmount)
{
	SetPixelShaderValue(
		PixelShader->GetPixelShader(),
		PackedParameters,
		FVector4(
		FocusDistance,
		1.0f / FocusInnerRadius,
		FalloffExponent,
		0.0f
		)
		);
	SetPixelShaderValue(PixelShader->GetPixelShader(),MinMaxBlurClampParameter,FVector4(MaxNearBlurAmount,MaxFarBlurAmount,0,0));
}

/** Serializer. */
FArchive& operator<<(FArchive& Ar,FDOFShaderParameters& P)
{
	Ar << P.PackedParameters;
	Ar << P.MinMaxBlurClampParameter;
	return Ar;
}

const UINT NumFPFilterSamples = 4;
const UINT HalfNumFPFilterSamples = (NumFPFilterSamples + 1) / 2;

//shaders to use for the fast version, which only takes 4 samples and uses linear floating point filtering
IMPLEMENT_SHADER_TYPE(template<>,TDOFAndBloomGatherPixelShader<NumFPFilterSamples>,TEXT("DOFAndBloomGatherPixelShader"),TEXT("Main"),SF_Pixel,VER_DOFBLOOMGATHER_SCENEMULTIPLIER,0);
IMPLEMENT_SHADER_TYPE(template<>,TDOFAndBloomGatherVertexShader<NumFPFilterSamples>,TEXT("DOFAndBloomGatherVertexShader"),TEXT("Main"),SF_Vertex,0,0);

//shaders to use for the high quality version, which takes 16 point samples and has minimal aliasing
IMPLEMENT_SHADER_TYPE(template<>,TDOFAndBloomGatherPixelShader<MAX_FILTER_SAMPLES>,TEXT("DOFAndBloomGatherPixelShader"),TEXT("Main"),SF_Pixel,VER_DOFBLOOMGATHER_SCENEMULTIPLIER,0);
IMPLEMENT_SHADER_TYPE(template<>,TDOFAndBloomGatherVertexShader<MAX_FILTER_SAMPLES>,TEXT("DOFAndBloomGatherVertexShader"),TEXT("Main"),SF_Vertex,0,0);

//pixel shader to use on the sm2 fallback
IMPLEMENT_SHADER_TYPE(,FDOFAndBloomGatherFallbackPixelShader,TEXT("DOFAndBloomGatherPixelShader"),TEXT("SM2Main"),SF_Pixel,0,0);

/*-----------------------------------------------------------------------------
FDOFAndBloomBlendPixelShader
-----------------------------------------------------------------------------*/

/** Encapsulates the DOF and bloom blend pixel shader. */
UBOOL FDOFAndBloomBlendPixelShader::ShouldCache(EShaderPlatform Platform)
{
	return TRUE;
}

void FDOFAndBloomBlendPixelShader::ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
{
}

/** Initialization constructor. */
FDOFAndBloomBlendPixelShader::FDOFAndBloomBlendPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
:	FGlobalShader(Initializer)
,	DOFParameters(Initializer.ParameterMap)
{
	SceneTextureParameters.Bind(Initializer.ParameterMap);
	BlurredImageParameter.Bind(Initializer.ParameterMap,TEXT("BlurredImage"),TRUE);
}

UBOOL FDOFAndBloomBlendPixelShader::Serialize(FArchive& Ar)
{
	UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
	Ar << DOFParameters << SceneTextureParameters << BlurredImageParameter;
	return bShaderHasOutdatedParameters;
}

IMPLEMENT_SHADER_TYPE(,FDOFAndBloomBlendPixelShader,TEXT("DOFAndBloomBlendPixelShader"),TEXT("Main"),SF_Pixel,VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD,0);

/*-----------------------------------------------------------------------------
FDOFAndBloomBlendVertexShader
-----------------------------------------------------------------------------*/

UBOOL FDOFAndBloomBlendVertexShader::ShouldCache(EShaderPlatform Platform)
{
	return TRUE;
}

void FDOFAndBloomBlendVertexShader::ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
{
}

/** Initialization constructor. */
FDOFAndBloomBlendVertexShader::FDOFAndBloomBlendVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
:	FGlobalShader(Initializer)
{
	SceneCoordinateScaleBiasParameter.Bind(Initializer.ParameterMap,TEXT("SceneCoordinateScaleBias"),TRUE);
}

// FShader interface.
UBOOL FDOFAndBloomBlendVertexShader::Serialize(FArchive& Ar)
{
	UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
	Ar << SceneCoordinateScaleBiasParameter;
	return bShaderHasOutdatedParameters;
}

IMPLEMENT_SHADER_TYPE(,FDOFAndBloomBlendVertexShader,TEXT("DOFAndBloomBlendVertexShader"),TEXT("Main"),SF_Vertex,VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD,0);

/*-----------------------------------------------------------------------------
FDOFAndBloomPostProcessSceneProxy
-----------------------------------------------------------------------------*/

extern TGlobalResource<FFilterVertexDeclaration> GFilterVertexDeclaration;

/** 
* Initialization constructor. 
* @param InEffect - DOF post process effect to mirror in this proxy
*/
FDOFAndBloomPostProcessSceneProxy::FDOFAndBloomPostProcessSceneProxy(const UDOFAndBloomEffect* InEffect,const FPostProcessSettings* WorldSettings)
:	FPostProcessSceneProxy(InEffect)
,	FalloffExponent(WorldSettings ? WorldSettings->DOF_FalloffExponent : InEffect->FalloffExponent)
,	BlurKernelSize(WorldSettings ? WorldSettings->DOF_BlurKernelSize : InEffect->BlurKernelSize)
,	MaxNearBlurAmount(WorldSettings ? WorldSettings->DOF_MaxNearBlurAmount : InEffect->MaxNearBlurAmount)
,	MaxFarBlurAmount(WorldSettings ? WorldSettings->DOF_MaxFarBlurAmount : InEffect->MaxFarBlurAmount)
,	ModulateBlurColor(WorldSettings ? WorldSettings->DOF_ModulateBlurColor : InEffect->ModulateBlurColor)
,	FocusType(WorldSettings ? WorldSettings->DOF_FocusType : InEffect->FocusType)
,	FocusInnerRadius(WorldSettings ? WorldSettings->DOF_FocusInnerRadius : InEffect->FocusInnerRadius)
,	FocusDistance(WorldSettings ? WorldSettings->DOF_FocusDistance : InEffect->FocusDistance)
,	FocusPosition(WorldSettings ? WorldSettings->DOF_FocusPosition : InEffect->FocusPosition)
,	BloomScale(WorldSettings ? WorldSettings->Bloom_Scale : InEffect->BloomScale)
{
	if(WorldSettings && !WorldSettings->bEnableDOF)
	{
		MaxNearBlurAmount = MaxFarBlurAmount = 0.0f;
	}

	if(WorldSettings && !WorldSettings->bEnableBloom)
	{
		BloomScale = 0.0;
	}
}

/**
* Renders the gather pass which downsamples scene color and depth, then uses those to 
* calculate bloom color and unfocused DOF color which are stored in the filter buffer.
*/
void FDOFAndBloomPostProcessSceneProxy::RenderDOFAndBloomGatherPass(FViewInfo& View)
{
	const UINT BufferSizeX = GSceneRenderTargets.GetBufferSizeX();
	const UINT BufferSizeY = GSceneRenderTargets.GetBufferSizeY();
	const UINT FilterBufferSizeX = GSceneRenderTargets.GetFilterBufferSizeX();
	const UINT FilterBufferSizeY = GSceneRenderTargets.GetFilterBufferSizeY();
	const UINT FilterDownsampleFactor = GSceneRenderTargets.GetFilterDownsampleFactor();
	const UINT DownsampledSizeX = View.RenderTargetSizeX / FilterDownsampleFactor;
	const UINT DownsampledSizeY = View.RenderTargetSizeY / FilterDownsampleFactor;

	// No depth tests, no backface culling.
	RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
	RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
	RHISetBlendState(TStaticBlendState<>::GetRHI());

	// Render to the downsampled filter buffer.
	GSceneRenderTargets.BeginRenderingFilter();

#if PS3
	const UINT NumFilterSamplesUsed = (MAX_FILTER_SAMPLES + 1) / 2;
	//use the fast fp filtering version
	const UBOOL bUseHighQualityBloom = FALSE;
#elif XBOX
	const UINT NumFilterSamplesUsed = 1;
	const UBOOL bUseHighQualityBloom = GSystemSettings.bUseHighQualityBloom;
#else
	const UINT NumFilterSamplesUsed = (MAX_FILTER_SAMPLES + 1) / 2;
	//use the slow (but high quality) 16 point sample version if specified by the scalability options
	//or floating point filtering is not supported, unless this the current platform is SM2. (in which case scene color won't be fp)
	const UBOOL bUseHighQualityBloom = (GSystemSettings.bUseHighQualityBloom || !GSupportsFPFiltering) && GRHIShaderPlatform != SP_PCD3D_SM2;
#endif

	FLOAT SceneMultiplier = 1.0f;

	if (bUseHighQualityBloom)
	{
		//Setup the downsamples so that all 16 texels in scene color that map to the currently rendered pixel are sampled.
		FVector2D DownsampleOffsets[MAX_FILTER_SAMPLES];

		for(UINT SampleY = 0;SampleY < FilterDownsampleFactor;SampleY++)
		{
			for(UINT SampleX = 0;SampleX < FilterDownsampleFactor;SampleX++)
			{
				const INT SampleIndex = SampleY * FilterDownsampleFactor + SampleX;
				DownsampleOffsets[SampleIndex] = FVector2D(
					( (FLOAT)SampleX - ( 0.5f * FilterDownsampleFactor ) ) / (FLOAT)BufferSizeX,
					( (FLOAT)SampleY - ( 0.5f * FilterDownsampleFactor ) ) / (FLOAT)BufferSizeY
					);
			}
		}

		// Pack the downsamples.
		FVector4 PackedDownsampleOffsets[(MAX_FILTER_SAMPLES + 1) / 2];
		for(INT ChunkIndex = 0;ChunkIndex < (MAX_FILTER_SAMPLES + 1) / 2;ChunkIndex++)
		{
			PackedDownsampleOffsets[ChunkIndex].X = DownsampleOffsets[ChunkIndex * 2 + 0].X;
			PackedDownsampleOffsets[ChunkIndex].Y = DownsampleOffsets[ChunkIndex * 2 + 0].Y;
			PackedDownsampleOffsets[ChunkIndex].Z = DownsampleOffsets[ChunkIndex * 2 + 1].Y;
			PackedDownsampleOffsets[ChunkIndex].W = DownsampleOffsets[ChunkIndex * 2 + 1].X;
		}

		// Set the gather vertex shader.
		TShaderMapRef<TDOFAndBloomGatherVertexShader<MAX_FILTER_SAMPLES> > GatherVertexShader(GetGlobalShaderMap());

		SetVertexShaderValues(GatherVertexShader->GetVertexShader(),GatherVertexShader->SampleOffsetsParameter,PackedDownsampleOffsets,NumFilterSamplesUsed);

		TShaderMapRef<TDOFAndBloomGatherPixelShader<MAX_FILTER_SAMPLES> > GatherPixelShader(GetGlobalShaderMap());
		RHIReduceTextureCachePenalty( GatherPixelShader->GetPixelShader() );
		GatherPixelShader->DOFParameters.Set( *GatherPixelShader,FocusDistance,FocusInnerRadius,FalloffExponent,MaxNearBlurAmount,MaxFarBlurAmount );
		GatherPixelShader->SceneTextureParameters.Set(&View,*GatherPixelShader);
		SetPixelShaderValue(GatherPixelShader->GetPixelShader(),GatherPixelShader->BloomScaleParameter,BloomScale);
		SetPixelShaderValue(GatherPixelShader->GetPixelShader(),GatherPixelShader->SceneMultiplierParameter,SceneMultiplier);
		SetGlobalBoundShaderState( HighQualityGatherBoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *GatherVertexShader, *GatherPixelShader, sizeof(FFilterVertex) );
	}
	else
	{
		FVector4 PackedDownsampleOffsets[HalfNumFPFilterSamples];

#if XBOX && !USE_NULL_RHI
		// Downsample via resolve
		// Resolve with an exponent bias of +3 (on top of the already existing +3), for a total of +6
		// This means that the gather shader will have to multiply scene color by 8 compensate
		FSurfaceRHIRef resolveRef = FSurfaceRHIRef(GSceneRenderTargets.GetQuarterSizeSceneColorTexture(),GSceneRenderTargets.GetQuarterSizeSceneColorSurface(),FXeSurfaceInfo(0,0,6,0));
		FResolveParams resParams = FResolveParams(0,0,BufferSizeX,BufferSizeY);
		RHICopyToResolveTarget(resolveRef, FALSE, resParams);
		SceneMultiplier = 8.0f;
#endif

#if XBOX
		// XBOX does the sample offset during tfetch2D, so the vertex shader only needs the center offset
		PackedDownsampleOffsets[0] = FVector4(
			-GPixelCenterOffset / (FLOAT)BufferSizeX,
			-GPixelCenterOffset / (FLOAT)BufferSizeY);
#else
		// Setup the downsamples so that linear filtering will let us get the average of a 2x2 block with a single sample
		FVector2D DownsampleOffsets[NumFPFilterSamples];

		for(UINT SampleY = 0;SampleY < HalfNumFPFilterSamples;SampleY++)
		{
			for(UINT SampleX = 0;SampleX < HalfNumFPFilterSamples;SampleX++)
			{
				const INT SampleIndex = SampleY * HalfNumFPFilterSamples + SampleX;

				DownsampleOffsets[SampleIndex] = FVector2D(
					( (FLOAT)SampleX * 2.0f - 1.0f ) / (FLOAT)GSceneRenderTargets.GetBufferSizeX(),
					( (FLOAT)SampleY * 2.0f - 1.0f ) / (FLOAT)GSceneRenderTargets.GetBufferSizeY()
					);
			}
		}

		// Pack the downsamples.
		for(INT ChunkIndex = 0; ChunkIndex < HalfNumFPFilterSamples; ChunkIndex++)
		{
			PackedDownsampleOffsets[ChunkIndex].X = DownsampleOffsets[ChunkIndex * 2 + 0].X;
			PackedDownsampleOffsets[ChunkIndex].Y = DownsampleOffsets[ChunkIndex * 2 + 0].Y;
			PackedDownsampleOffsets[ChunkIndex].Z = DownsampleOffsets[ChunkIndex * 2 + 1].Y;
			PackedDownsampleOffsets[ChunkIndex].W = DownsampleOffsets[ChunkIndex * 2 + 1].X;
		}
#endif //#if XBOX

		// Set the gather vertex shader.
		TShaderMapRef<TDOFAndBloomGatherVertexShader<NumFPFilterSamples> > GatherVertexShader(GetGlobalShaderMap());
		SetVertexShaderValues(GatherVertexShader->GetVertexShader(),GatherVertexShader->SampleOffsetsParameter,PackedDownsampleOffsets,HalfNumFPFilterSamples);

		if (GRHIShaderPlatform == SP_PCD3D_SM2)
		{
			// Set the gather pixel shader.
			TShaderMapRef<FDOFAndBloomGatherFallbackPixelShader> GatherPixelShader(GetGlobalShaderMap());
			GatherPixelShader->DOFParameters.Set( *GatherPixelShader,FocusDistance,FocusInnerRadius,FalloffExponent,MaxNearBlurAmount,MaxFarBlurAmount );

			//enable linear filtering on scene color
			GatherPixelShader->SceneTextureParameters.Set(&View,*GatherPixelShader, SF_Bilinear);
			SetPixelShaderValue(GatherPixelShader->GetPixelShader(),GatherPixelShader->BloomScaleParameter,BloomScale);

			SetGlobalBoundShaderState( FastGatherBoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *GatherVertexShader, *GatherPixelShader, sizeof(FFilterVertex) );
		}
		else
		{
			// Set the gather pixel shader.
			TShaderMapRef<TDOFAndBloomGatherPixelShader<NumFPFilterSamples> > GatherPixelShader(GetGlobalShaderMap());
			RHIReduceTextureCachePenalty( GatherPixelShader->GetPixelShader() );
			GatherPixelShader->DOFParameters.Set( *GatherPixelShader,FocusDistance,FocusInnerRadius,FalloffExponent,MaxNearBlurAmount,MaxFarBlurAmount );

			//enable linear filtering on scene color
			GatherPixelShader->SceneTextureParameters.Set(&View,*GatherPixelShader, SF_Bilinear);
			SetPixelShaderValue(GatherPixelShader->GetPixelShader(),GatherPixelShader->BloomScaleParameter,BloomScale);
			SetPixelShaderValue(GatherPixelShader->GetPixelShader(),GatherPixelShader->SceneMultiplierParameter,SceneMultiplier);

			SetGlobalBoundShaderState( FastGatherBoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *GatherVertexShader, *GatherPixelShader, sizeof(FFilterVertex) );

#if XBOX
			// The low-quality version use the quarter-size resolve texture instead of normal scene color.
			SetTextureParameter(
				GatherPixelShader->GetPixelShader(),
				GatherPixelShader->SmallSceneColorTextureParameter,
				TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
				GSceneRenderTargets.GetQuarterSizeSceneColorTexture()
				);
#endif
		}
	}

	//@TODO - We shouldn't have to clear since we fill the entire buffer anyway. :(
#if !PS3
	// Clear the buffer to black.
	RHIClear(TRUE,FLinearColor(0,0,0,0),FALSE,0,FALSE,0);
#endif

	// Draw a quad mapping the view's scene colors/depths to the downsampled filter buffer.
	DrawDenormalizedQuad(
		1,1,
		DownsampledSizeX,DownsampledSizeY,
		View.RenderTargetX,View.RenderTargetY,
		View.RenderTargetSizeX,View.RenderTargetSizeY,
		FilterBufferSizeX,FilterBufferSizeY,
		BufferSizeX,BufferSizeY
		);

	// Resolve the filter buffer.
	GSceneRenderTargets.FinishRenderingFilter();
}


/**
* Render the post process effect
* Called by the rendering thread during scene rendering
* @param InDepthPriorityGroup - scene DPG currently being rendered
* @param View - current view
* @param CanvasTransform - same canvas transform used to render the scene
* @return TRUE if anything was rendered
*/
UBOOL FDOFAndBloomPostProcessSceneProxy::Render(const FScene* Scene, UINT InDepthPriorityGroup,FViewInfo& View,const FMatrix& CanvasTransform)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("DOFAndBloom"));

	const UINT BufferSizeX = GSceneRenderTargets.GetBufferSizeX();
	const UINT BufferSizeY = GSceneRenderTargets.GetBufferSizeY();
	const UINT FilterBufferSizeX = GSceneRenderTargets.GetFilterBufferSizeX();
	const UINT FilterBufferSizeY = GSceneRenderTargets.GetFilterBufferSizeY();
	const UINT FilterDownsampleFactor = GSceneRenderTargets.GetFilterDownsampleFactor();
	const UINT DownsampledSizeX = View.RenderTargetSizeX / FilterDownsampleFactor;
	const UINT DownsampledSizeY = View.RenderTargetSizeY / FilterDownsampleFactor;

	RenderDOFAndBloomGatherPass( View);

	// Blur the filter buffer.
	GaussianBlurFilterBuffer( DownsampledSizeX,DownsampledSizeY,BlurKernelSize);

	// apply mask if available
	if( View.PostProcessMask && 
		View.PostProcessMask->ShouldRender() )
	{
		View.PostProcessMask->BeginStencilMask();
	}

	// Render to the scene color buffer.
	GSceneRenderTargets.BeginRenderingSceneColor();

	// Set the blend vertex shader.
	TShaderMapRef<FDOFAndBloomBlendVertexShader> BlendVertexShader(GetGlobalShaderMap());
	SetVertexShaderValue(
		BlendVertexShader->GetVertexShader(),
		BlendVertexShader->SceneCoordinateScaleBiasParameter,
		FVector4(
		+0.5f,
		-0.5f,
		+0.5f + GPixelCenterOffset / BufferSizeY,
		+0.5f + GPixelCenterOffset / BufferSizeX
		)
		);

	// Set the blend pixel shader.
	TShaderMapRef<FDOFAndBloomBlendPixelShader> BlendPixelShader(GetGlobalShaderMap());
	BlendPixelShader->DOFParameters.Set( *BlendPixelShader,FocusDistance,FocusInnerRadius,FalloffExponent,MaxNearBlurAmount,MaxFarBlurAmount);
	BlendPixelShader->SceneTextureParameters.Set(&View,*BlendPixelShader);

	SetTextureParameter(
		BlendPixelShader->GetPixelShader(),
		BlendPixelShader->BlurredImageParameter,
		TStaticSamplerState<SF_Bilinear>::GetRHI(),
		GSceneRenderTargets.GetFilterColorTexture()
		);

	SetGlobalBoundShaderState( BlendBoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *BlendVertexShader, *BlendPixelShader, sizeof(FFilterVertex));

	// Draw a quad mapping the blurred pixels in the filter buffer to the scene color buffer.
	DrawDenormalizedQuad(
		View.RenderTargetX,View.RenderTargetY,
		View.RenderTargetSizeX,View.RenderTargetSizeY,
		1,1,
		DownsampledSizeX,DownsampledSizeY,
		BufferSizeX,BufferSizeY,
		FilterBufferSizeX,FilterBufferSizeY
		);

	// restore mask if available
	if( View.PostProcessMask && 
		View.PostProcessMask->ShouldRender() )
	{
		View.PostProcessMask->EndStencilMask();
	}

	// Resolve the scene color buffer.
	GSceneRenderTargets.FinishRenderingSceneColor();

	return TRUE;
}

FGlobalBoundShaderState FDOFAndBloomPostProcessSceneProxy::HighQualityGatherBoundShaderState;
FGlobalBoundShaderState FDOFAndBloomPostProcessSceneProxy::FastGatherBoundShaderState;
FGlobalBoundShaderState FDOFAndBloomPostProcessSceneProxy::BlendBoundShaderState;

/**
 * Creates a proxy to represent the render info for a post process effect
 * @param WorldSettings - The world's post process settings for the view.
 * @return The proxy object.
 */
FPostProcessSceneProxy* UDOFAndBloomEffect::CreateSceneProxy(const FPostProcessSettings* WorldSettings)
{
	if(!WorldSettings || WorldSettings->bEnableDOF || WorldSettings->bEnableBloom)
	{
		return new FDOFAndBloomPostProcessSceneProxy(this,WorldSettings);
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
UBOOL UDOFAndBloomEffect::IsShown(const FSceneView* View) const
{
	return (GSystemSettings.bAllowBloom || GSystemSettings.bAllowDepthOfField) && Super::IsShown( View );
}

