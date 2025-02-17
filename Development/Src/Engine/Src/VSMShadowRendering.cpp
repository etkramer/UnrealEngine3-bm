/*=============================================================================
	VSMShadowRendering.cpp: VSM (Variance Shadow Map) rendering implementation
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "UnTextureLayout.h"
#include "SceneFilterRendering.h"

#if SUPPORTS_VSM

#define VSM_FILTER_SIZE 5

/** number of filter passes to apply to the variance texture */
INT GNumVSMFilterPasses = 1;

/*-----------------------------------------------------------------------------
FVSMShadowProjectionPixelShader
-----------------------------------------------------------------------------*/

// these don't really need to be exposed as shader params but easier to tweak for now
/** VSM tweak epsilon */
FLOAT FVSMProjectionPixelShader::VSMEpsilon = 0.0001f;
/** VSM tweak exponent */
FLOAT FVSMProjectionPixelShader::VSMExponent = 3.2f; 

/** 
* Constructor - binds all shader params
* @param Initializer - init data from shader compiler
*/
FVSMProjectionPixelShader::FVSMProjectionPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
:	FShader(Initializer)
{
    SceneTextureParams.Bind(Initializer.ParameterMap);
	ScreenToShadowMatrixParameter.Bind(Initializer.ParameterMap,TEXT("ScreenToShadowMatrix"));
	ShadowVarianceTextureParameter.Bind(Initializer.ParameterMap,TEXT("ShadowVarianceTexture"));
	ShadowDepthTextureParam.Bind(Initializer.ParameterMap,TEXT("ShadowDepthTexture"),TRUE);
	VSMEpsilonParam.Bind(Initializer.ParameterMap,TEXT("VSMEpsilon"));
	VSMExponentParam.Bind(Initializer.ParameterMap,TEXT("VSMExponent"));
}

/**
* @param Platform - hardware platform
* @return TRUE if this shader should be cached
*/
UBOOL FVSMProjectionPixelShader::ShouldCache(EShaderPlatform Platform)
{
	return TRUE;
}

/**
* Sets the current pixel shader params
* @param View - current view
* @param ShadowInfo - projected shadow info for a single light
*/
void FVSMProjectionPixelShader::SetParameters(const FSceneView& View, const FProjectedShadowInfo* ShadowInfo)
{
	// set params needed for scene color/depth sampling
	SceneTextureParams.Set(&View,this);

	// Set the transform from screen coordinates to shadow depth texture coordinates.
	const FLOAT InvBufferResolution = 1.0f / (FLOAT)GSceneRenderTargets.GetShadowVarianceTextureResolution();
	const FLOAT ShadowResolutionFraction = 0.5f * (FLOAT)ShadowInfo->Resolution * InvBufferResolution;
	FMatrix	ScreenToShadow = FMatrix(
			FPlane(1,0,0,0),
			FPlane(0,1,0,0),
			FPlane(0,0,View.ProjectionMatrix.M[2][2],1),
			FPlane(0,0,View.ProjectionMatrix.M[3][2],0)) *
		View.InvViewProjectionMatrix *
		ShadowInfo->SubjectAndReceiverMatrix *
		FMatrix(
			FPlane(ShadowResolutionFraction,0,							0,									0),
			FPlane(0,						-ShadowResolutionFraction,	0,									0),
			FPlane(0,						0,							1.0f / ShadowInfo->MaxSubjectDepth,	0),
			FPlane(	(ShadowInfo->X + SHADOW_BORDER + GPixelCenterOffset) * InvBufferResolution + ShadowResolutionFraction,
					(ShadowInfo->Y + SHADOW_BORDER + GPixelCenterOffset) * InvBufferResolution + ShadowResolutionFraction,
					0,
					1)
		);
	SetPixelShaderValue(GetPixelShader(),ScreenToShadowMatrixParameter,ScreenToShadow);

	//Use linear filtering to take advantage of floating point filtering for VSM
	SetTextureParameter(
		GetPixelShader(),
		ShadowVarianceTextureParameter,
		TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
		GSceneRenderTargets.GetShadowVarianceTexture()
		);

	// set VSM tweakables
	SetPixelShaderValue(GetPixelShader(),VSMEpsilonParam,VSMEpsilon);
	SetPixelShaderValue(GetPixelShader(),VSMExponentParam,VSMExponent);
}

/**
* Serialize shader parameters for this shader
* @param Ar - archive to serialize with
*/
UBOOL FVSMProjectionPixelShader::Serialize(FArchive& Ar)
{
	UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
	Ar << SceneTextureParams;
	Ar << ScreenToShadowMatrixParameter;
	Ar << ShadowVarianceTextureParameter;
	Ar << ShadowDepthTextureParam;
	Ar << VSMEpsilonParam;
	Ar << VSMExponentParam;
	return bShaderHasOutdatedParameters;
}

IMPLEMENT_SHADER_TYPE(,FVSMProjectionPixelShader,TEXT("VSMProjectionPixelShader"),TEXT("Main"),SF_Pixel,0,0);

/*-----------------------------------------------------------------------------
FVSMModProjectionPixelShader
-----------------------------------------------------------------------------*/

/**
* Constructor - binds all shader params
* @param Initializer - init data from shader compiler
*/
FVSMModProjectionPixelShader::FVSMModProjectionPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
:	FVSMProjectionPixelShader(Initializer)
{
	ShadowModulateColorParam.Bind(Initializer.ParameterMap,TEXT("ShadowModulateColor"));
	ScreenToWorldParam.Bind(Initializer.ParameterMap,TEXT("ScreenToWorld"),TRUE);
}

/**
* Sets the current pixel shader params
* @param View - current view
* @param ShadowInfo - projected shadow info for a single light
*/
void FVSMModProjectionPixelShader::SetParameters(const FSceneView& View, const FProjectedShadowInfo* ShadowInfo)
{
	FVSMProjectionPixelShader::SetParameters(View,ShadowInfo);		
	const FLightSceneInfo* LightSceneInfo = ShadowInfo->LightSceneInfo;
	// color to modulate shadowed areas on screen
	SetPixelShaderValue(
		GetPixelShader(),
		ShadowModulateColorParam,
		Lerp(FLinearColor::White,LightSceneInfo->ModShadowColor,ShadowInfo->FadeAlpha)
		);
	// screen space to world space transform
	FMatrix ScreenToWorld = FMatrix(
		FPlane(1,0,0,0),
		FPlane(0,1,0,0),
		FPlane(0,0,(1.0f - Z_PRECISION),1),
		FPlane(0,0,-View.NearClippingDistance * (1.0f - Z_PRECISION),0)
		) * 
		View.InvViewProjectionMatrix;
	SetPixelShaderValue( GetPixelShader(), ScreenToWorldParam, ScreenToWorld );
}

/**
* Serialize the parameters for this shader
* @param Ar - archive to serialize to
*/
UBOOL FVSMModProjectionPixelShader::Serialize(FArchive& Ar)
{
	UBOOL bShaderHasOutdatedParameters = FVSMProjectionPixelShader::Serialize(Ar);
	Ar << ShadowModulateColorParam;
	Ar << ScreenToWorldParam;
	return bShaderHasOutdatedParameters;
}

/*-----------------------------------------------------------------------------
FVSMDepthGatherPixelShader
-----------------------------------------------------------------------------*/
#if 0
/** 
* Renders shadow variance moments using shadow depth texture values
*/
class FVSMDepthGatherPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FVSMDepthGatherPixelShader,Global);
public:

	/** 
	* Constructor. 
	*/
	FVSMDepthGatherPixelShader() {}

	/**
	* Constructor - binds all shader params
	* @param Initializer - init data from shader compiler
	*/
	FVSMDepthGatherPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FShader(Initializer)
	{		
		ShadowDepthTextureParam.Bind(Initializer.ParameterMap,TEXT("ShadowDepthTexture"));
	}

	// FShader interface.

	/**
	* Serialize the parameters for this shader
	* @param Ar - archive to serialize to
	*/
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << ShadowDepthTextureParam;
		return bShaderHasOutdatedParameters;
	}

	/**
	* @param Platform - hardware platform
	* @return TRUE if this shader should be cached
	*/
	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	FShaderResourceParameter ShadowDepthTextureParam;
};
IMPLEMENT_SHADER_TYPE(,FVSMDepthGatherPixelShader,TEXT("VSMDepthGatherPixelShader"),TEXT("Main"),SF_Pixel,0,0);
#endif

/*-----------------------------------------------------------------------------
FVSMDepthGatherVertexShader
-----------------------------------------------------------------------------*/
#if 0
/** Renders shadow variance moments using shadow depth texture values */
class FVSMDepthGatherVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FVSMDepthGatherVertexShader,Global);
public:

	/** 
	* Constructor. 
	*/
	FVSMDepthGatherVertexShader() {}

	/**
	* Constructor - binds all shader params
	* @param Initializer - init data from shader compiler
	*/
	FVSMDepthGatherVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FShader(Initializer)
	{		
	}

	// FShader interface.

	/**
	* Serialize the parameters for this shader
	* @param Ar - archive to serialize to
	*/
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		return bShaderHasOutdatedParameters;
	}

	/**
	* @param Platform - hardware platform
	* @return TRUE if this shader should be cached
	*/
	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}
};
IMPLEMENT_SHADER_TYPE(,FVSMDepthGatherVertexShader,TEXT("VSMDepthGatherVertexShader"),TEXT("Main"),SF_Vertex,0,0);
#endif

/*-----------------------------------------------------------------------------
TVSMFilteredDepthGatherPixelShader
-----------------------------------------------------------------------------*/

/** 
* Renders shadow variance moments using shadow depth texture values
* And also filters the results
* It should be used with the TFilterVertexShader
*/
template<UINT NumSamples>
class TVSMFilterDepthGatherPixelShader : public TFilterPixelShader<NumSamples>
{
	DECLARE_SHADER_TYPE(TVSMFilterDepthGatherPixelShader,Global);
public:
	
	/** 
	* Constructor. 
	*/
	TVSMFilterDepthGatherPixelShader() {}

	/**
	* Constructor - binds all shader params
	* @param Initializer - init data from shader compiler
	*/
	TVSMFilterDepthGatherPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	TFilterPixelShader<NumSamples>(Initializer)
	{		
		MinOffsetParam.Bind(Initializer.ParameterMap,TEXT("MinOffset"));
		MaxOffsetParam.Bind(Initializer.ParameterMap,TEXT("MaxOffset"));
	}

	/**
	* Sets the current pixel shader params
	* @param SamplerStateRHI - sampler for source filter texture
	* @param TextureRHI - source filter texture 
	* @param SampleWeights - UV offsets for filter sampling
	* @param MinOffset - min UV offset clamp
	* @param MaxOffset - max UV offset clamp
	*/
	void SetParameters(
		FSamplerStateRHIParamRef SamplerStateRHI,
		FTextureRHIParamRef TextureRHI,
		const FLinearColor* SampleWeights,
		const FVector4& MinOffset,
		const FVector4& MaxOffset )
	{
		TFilterPixelShader<NumSamples>::SetParameters(SamplerStateRHI,TextureRHI,SampleWeights);
		SetPixelShaderValue( TFilterPixelShader<NumSamples>::GetPixelShader(), MinOffsetParam, MinOffset );
		SetPixelShaderValue( TFilterPixelShader<NumSamples>::GetPixelShader(), MaxOffsetParam, MaxOffset );
	}

	// FShader interface.

	/**
	* Serialize the parameters for this shader
	* @param Ar - archive to serialize to
	*/
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = TFilterPixelShader<NumSamples>::Serialize(Ar);
		Ar << MinOffsetParam;
		Ar << MaxOffsetParam;
		return bShaderHasOutdatedParameters;
	}

	/**
	* @param Platform - hardware platform
	* @return TRUE if this shader should be cached
	*/
	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	/**
	* Add any defines required by the shader - defines NUM_SAMPLES
	* @param OutEnvironment - shader environment to modify
	*/
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		TFilterPixelShader<NumSamples>::ModifyCompilationEnvironment(Platform, OutEnvironment);
	}

	/** min clamp xy|wz since we don't rely on empty border texels */
	FShaderParameter MinOffsetParam;
	/** max clamp xy|wz since we don't rely on empty border texels */
	FShaderParameter MaxOffsetParam;

};
IMPLEMENT_SHADER_TYPE(template<>,TVSMFilterDepthGatherPixelShader<VSM_FILTER_SIZE>,TEXT("VSMFilterDepthGatherPixelShader"),TEXT("Main"),SF_Pixel,0,0);

/*-----------------------------------------------------------------------------
VSM Rendering ...
-----------------------------------------------------------------------------*/

/** Vertex declaration for the light function fullscreen 2D quad. */
TGlobalResource<FFilterVertexDeclaration> GVSMFilterVertexDeclaration;

/** 
* Helper for setting the VSM filter vs/ps shaders and their parameters. 
* Also sets the bound shader state for them.
* @param SampleOffsets - A pointer to an array of NumSamples UV offsets
* @param SampleWeights - A pointer to an array of NumSamples 4-vector weights
* @param MinOffset - min UV offset clamp
* @param MaxOffset - max UV offset clamp
*/
void SetVSMFilterDepthGatherShaders(
					  const FVector2D* SampleOffsets,
					  const FLinearColor* SampleWeights,
					  const FVector4& MinOffset,
					  const FVector4& MaxOffset
					  )
{
	// shadow depth source texture
	const FTexture2DRHIRef& ShadowMapDepthTexture = GSupportsDepthTextures ? GSceneRenderTargets.GetShadowDepthZTexture() : GSceneRenderTargets.GetShadowDepthColorTexture();

	// find the filter vertex shader
	TShaderMapRef<TFilterVertexShader<VSM_FILTER_SIZE> > VSMFilterVertexShader(GetGlobalShaderMap());
	// find the vsm gather filter pixel shader
	TShaderMapRef<TVSMFilterDepthGatherPixelShader<VSM_FILTER_SIZE> > VSMFilterGatherPixelShader(GetGlobalShaderMap());
	// set the shader params
	VSMFilterVertexShader->SetParameters(SampleOffsets);
	VSMFilterGatherPixelShader->SetParameters(
		TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
		ShadowMapDepthTexture,
		SampleWeights,
		MinOffset,
		MaxOffset
		);

	// create the bound shader state if it doesn't exist
	static FBoundShaderStateRHIRef VSMFilterDepthGatherBoundShaderState;
	if( !IsValidRef(VSMFilterDepthGatherBoundShaderState) )
	{
		DWORD Strides[MaxVertexElementCount];
		appMemzero(Strides, sizeof(Strides));
		Strides[0] = sizeof(FFilterVertex);
		VSMFilterDepthGatherBoundShaderState = RHICreateBoundShaderState(
			GVSMFilterVertexDeclaration.VertexDeclarationRHI, 
			Strides, 
			VSMFilterVertexShader->GetVertexShader(), 
			VSMFilterGatherPixelShader->GetPixelShader()
			);
	}
	// set the bound shader state
	RHISetBoundShaderState( VSMFilterDepthGatherBoundShaderState);
}

/**
* Evaluates a normal distribution PDF at given X.
* @param X - The X to evaluate the PDF at.
* @param Mean - The normal distribution's mean.
* @param Variance - The normal distribution's variance.
* @return The value of the normal distribution at X.
*/
static FLOAT NormalDistribution(FLOAT X,FLOAT Mean,FLOAT Variance)
{
	const FLOAT StandardDeviation = appSqrt(Variance);
	return appExp(-Square(X - Mean) / (2.0f * Variance)) / (StandardDeviation * appSqrt(2.0f * (FLOAT)PI));
}

/**
* Renders shadow variance moments using shadow depth values and filters them.
* @param ShadowInfo - shadow projection info for offset/resolution
* @param Direction - 2d vector for sample offset direction
*/
static void ApplyVSMGatherBlurStep(const FProjectedShadowInfo& ShadowInfo, FVector2D Direction)
{
	// direction offset in texels
	FLOAT SrcTexelSize = 1.0f / GSceneRenderTargets.GetShadowDepthTextureResolution();
	FVector2D TexelDirection = Direction * SrcTexelSize;
	// calc blur offsets/weights
	FVector2D BlurOffsets[VSM_FILTER_SIZE];
	FLinearColor BlurWeights[VSM_FILTER_SIZE];

	if( VSM_FILTER_SIZE==1 )
	{
		BlurOffsets[0] = FVector2D(0.f,0.f);
		BlurWeights[0] = FLinearColor::White;
	}
	else
	{
		FLOAT ClampedKernelRadius = (FLOAT)(VSM_FILTER_SIZE - 1);
		INT IntegerKernelRadius = VSM_FILTER_SIZE - 1;
		UINT NumSamples = 0;
		FLOAT WeightSum = 0.0f;
		for(INT SampleIndex = -IntegerKernelRadius;SampleIndex <= IntegerKernelRadius;SampleIndex += 2)
		{
			FLOAT Weight0 = NormalDistribution(SampleIndex,0,ClampedKernelRadius);
			FLOAT Weight1 = NormalDistribution(SampleIndex + 1,0,ClampedKernelRadius);
			FLOAT TotalWeight = Weight0 + Weight1;
			BlurOffsets[NumSamples] = TexelDirection * (SampleIndex + Weight1 / TotalWeight);
			BlurWeights[NumSamples] = FLinearColor::White * TotalWeight;
			WeightSum += TotalWeight;
			NumSamples++;
		}
		// Normalize blur weights.
		for(UINT SampleIndex = 0;SampleIndex < NumSamples;SampleIndex++)
		{
			BlurWeights[SampleIndex] /= WeightSum;
		}
	}

	// Rendering to the entire shadow viewport (including border pixels) to avoid clearing the variance buffer
	UINT	X = ShadowInfo.X,
			Y = ShadowInfo.Y,
			SizeX = SHADOW_BORDER*2 + ShadowInfo.Resolution,
			SizeY = SHADOW_BORDER*2 + ShadowInfo.Resolution;

	// Clamp UV offsets to the border texels for this shadow's area
	FVector4 MinOffset(
		X * SrcTexelSize,
		Y * SrcTexelSize, 
		Y * SrcTexelSize, 
		X * SrcTexelSize
		);
	FVector4 MaxOffset( 
		(X+SizeX-1) * SrcTexelSize,
		(Y+SizeY-1) * SrcTexelSize,
		(Y+SizeY-1) * SrcTexelSize,
		(X+SizeX-1) * SrcTexelSize
		);
	
    // set shaders and constants
	SetVSMFilterDepthGatherShaders(
		BlurOffsets,
		BlurWeights,
		MinOffset,
		MaxOffset
		);	
	DrawDenormalizedQuad(
		X, Y,
		SizeX, SizeY,
		X, Y,
		SizeX, SizeY,
		GSceneRenderTargets.GetShadowVarianceTextureResolution(),GSceneRenderTargets.GetShadowVarianceTextureResolution(),
		GSceneRenderTargets.GetShadowDepthTextureResolution(),GSceneRenderTargets.GetShadowDepthTextureResolution()
		);
}

/**
* Filters the existing VSM variance texture.
* @param ShadowInfo - shadow projection info for offset/resolution
* @param Direction - 2d vector for sample offset direction
*/
static void ApplyVSMBlurStep(const FProjectedShadowInfo& ShadowInfo, FVector2D Direction)
{
	// direction offset in texels
	FVector2D TexelDirection = Direction / GSceneRenderTargets.GetShadowVarianceTextureResolution();
	// calc blur offsets/weights
	FLOAT ClampedKernelRadius = (FLOAT)(VSM_FILTER_SIZE - 1);
	INT IntegerKernelRadius = VSM_FILTER_SIZE - 1;
	FVector2D BlurOffsets[MAX_FILTER_SAMPLES];
	FLinearColor BlurWeights[MAX_FILTER_SAMPLES];
	UINT NumSamples = 0;
	FLOAT WeightSum = 0.0f;
	for(INT SampleIndex = -IntegerKernelRadius;SampleIndex <= IntegerKernelRadius;SampleIndex += 2)
	{
		FLOAT Weight0 = NormalDistribution(SampleIndex,0,ClampedKernelRadius);
		FLOAT Weight1 = NormalDistribution(SampleIndex + 1,0,ClampedKernelRadius);
		FLOAT TotalWeight = Weight0 + Weight1;
		BlurOffsets[NumSamples] = TexelDirection * (SampleIndex + Weight1 / TotalWeight);
		BlurWeights[NumSamples] = FLinearColor::White * TotalWeight;
		WeightSum += TotalWeight;
		NumSamples++;
	}
	// Normalize blur weights.
	for(UINT SampleIndex = 0;SampleIndex < NumSamples;SampleIndex++)
	{
		BlurWeights[SampleIndex] /= WeightSum;
	}
	// set shaders and constants
	SetFilterShaders(
		TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
		GSceneRenderTargets.GetShadowVarianceTexture(),
		BlurOffsets,
		BlurWeights,
		NumSamples
		);

	// Only render within shadow resolution and shadow border texels to avoid sampling from edges
	UINT	X = ShadowInfo.X+SHADOW_BORDER,
			Y = ShadowInfo.Y+SHADOW_BORDER,
			SizeX = ShadowInfo.Resolution,
			SizeY = ShadowInfo.Resolution;
	DrawDenormalizedQuad(
		X,Y,
		SizeX,SizeY,
		X,Y,
		SizeX,SizeY,
		GSceneRenderTargets.GetShadowVarianceTextureResolution(),GSceneRenderTargets.GetShadowVarianceTextureResolution(),
		GSceneRenderTargets.GetShadowVarianceTextureResolution(),GSceneRenderTargets.GetShadowVarianceTextureResolution()
		);
}

/**
* Renders the shadow subject variance by sampling the existing shadow depth texture
* @param SceneRenderer - current scene rendering info
* @param Shadows - array of projected shadow infos
* @param DPGIndex - DPG level rendering the shadow projection pass
*/
void RenderShadowVariance(const class FSceneRenderer* SceneRenderer, const TArray<FProjectedShadowInfo*>& Shadows, UINT DPGIndex )
{
	// No depth tests, no backface culling, enable color writes
	RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
	RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
	RHISetBlendState(TStaticBlendState<>::GetRHI());
	RHISetColorWriteEnable(TRUE);

	FResolveParams ResolveParams;
	ResolveParams.X1 = 0;
	ResolveParams.Y1 = 0;
	UBOOL bResolveParamsInit=FALSE;
	// calc the max RECT needed for resolving the variance surface	
	for( INT ShadowIdx=0; ShadowIdx < Shadows.Num(); ShadowIdx++ )
	{
		const FProjectedShadowInfo* ShadowInfo = Shadows(ShadowIdx);			
		if( ShadowInfo->bAllocated )
		{
			// init values
			if( !bResolveParamsInit )
			{
				ResolveParams.X2 = ShadowInfo->X + ShadowInfo->Resolution + SHADOW_BORDER*2;
				ResolveParams.Y2 = ShadowInfo->Y + ShadowInfo->Resolution + SHADOW_BORDER*2;
				bResolveParamsInit=TRUE;
			}
			// keep track of max extents
			else 
			{
				ResolveParams.X2 = Max<UINT>(ShadowInfo->X + ShadowInfo->Resolution + SHADOW_BORDER*2,ResolveParams.X2);
				ResolveParams.Y2 = Max<UINT>(ShadowInfo->Y + ShadowInfo->Resolution + SHADOW_BORDER*2,ResolveParams.Y2);
			}
		}		
	}	

	// first blur/gather pass will do a horizontal blur
	FVector2D BlurDirection(1.0f,0);
	GSceneRenderTargets.BeginRenderingShadowVariance();
	if( GIsEditor || 
		// hack to render foreground items' shadows in the world (need to clear entire variance texture)
		DPGIndex == SDPG_Foreground )
	{
		RHIClear(TRUE,FLinearColor::White,FALSE,0.f,FALSE,0);
		ResolveParams = FResolveParams();
	}	
	for( INT ShadowIdx=0; ShadowIdx < Shadows.Num(); ShadowIdx++ )
	{
		const FProjectedShadowInfo* ShadowInfo = Shadows(ShadowIdx);			
		if( ShadowInfo->bAllocated )
		{
			ApplyVSMGatherBlurStep(*ShadowInfo,BlurDirection);
		}		
	}
	GSceneRenderTargets.FinishRenderingShadowVariance(ResolveParams);

	for( INT Pass=0; Pass < GNumVSMFilterPasses; Pass++ )
	{
		GSceneRenderTargets.BeginRenderingShadowVariance();
		// alternate between horzontal/vertical blurring between passes
		BlurDirection = (Pass%2) ? FVector2D(1.0f,0) : FVector2D(0,1.0f);
		for( INT ShadowIdx=0; ShadowIdx < Shadows.Num(); ShadowIdx++ )
		{
			const FProjectedShadowInfo* ShadowInfo = Shadows(ShadowIdx);
			if( ShadowInfo->bAllocated )
			{
				ApplyVSMBlurStep(*ShadowInfo,BlurDirection);
			}
		}
		GSceneRenderTargets.FinishRenderingShadowVariance(ResolveParams);
	}
}

#else

// Suppress linker warning "warning LNK4221: no public symbols found; archive member will be inaccessible"
INT VSMShadowRenderingLinkerHelper;

#endif // SUPPORTS_VSM



