/*=============================================================================
UberPostProcessEffect.cpp: combines DOF, bloom and material coloring.

Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "SceneFilterRendering.h"
#include "DOFAndBloomEffect.h"

IMPLEMENT_CLASS(UUberPostProcessEffect);

/**
* Called when properties change.
*/
void UUberPostProcessEffect::PostEditChange(UProperty* PropertyThatChanged)
{
	// clamp saturation to 0..1
	SceneDesaturation = Clamp(SceneDesaturation, 0.f, 1.f);

	// UberPostProcessEffect should only ever exists in the SDPG_PostProcess scene
	SceneDPG=SDPG_PostProcess;

	Super::PostEditChange(PropertyThatChanged);
}	

/**
* Called after this instance has been serialized.
*/
void UUberPostProcessEffect::PostLoad()
{
	Super::PostLoad();

	// UberPostProcessEffect should only ever exists in the SDPG_PostProcess scene
	SceneDPG=SDPG_PostProcess;

	// clamp desaturation to 0..1 (fixup for old data)
	SceneDesaturation = Clamp(SceneDesaturation, 0.f, 1.f);
}



/*-----------------------------------------------------------------------------
FMaterialShaderParameters
-----------------------------------------------------------------------------*/

/** Encapsulates the Material parameters. */
class FMaterialShaderParameters
{
public:

	/** Default constructor. */
	FMaterialShaderParameters() {}

	/** Initialization constructor. */
	FMaterialShaderParameters(const FShaderParameterMap& ParameterMap)
	{
		SceneShadowsAndDesaturation.Bind(ParameterMap,TEXT("SceneShadowsAndDesaturation"),TRUE);
		SceneInverseHighLights.Bind(ParameterMap,TEXT("SceneInverseHighLights"),TRUE);
		SceneMidTones.Bind(ParameterMap,TEXT("SceneMidTones"),TRUE);
		SceneScaledLuminanceWeights.Bind(ParameterMap,TEXT("SceneScaledLuminanceWeights"),TRUE);
	}

	/** Set the material shader parameter values. */
	void Set(FShader* PixelShader, FVector4 const& Shadows, FVector4 const& HighLights, FVector4 const& MidTones, FLOAT Desaturation)
	{
		// SceneInverseHighlights

		FVector4 InvHighLights;

		InvHighLights.X = 1.f / HighLights.X;
		InvHighLights.Y = 1.f / HighLights.Y;
		InvHighLights.Z = 1.f / HighLights.Z;
		InvHighLights.W = 0.f; // Unused

		SetPixelShaderValue(
			PixelShader->GetPixelShader(),
			SceneInverseHighLights,
			InvHighLights
			);

		// SceneShadowsAndDesaturation

		FVector4 ShadowsAndDesaturation;

		ShadowsAndDesaturation.X = Shadows.X;
		ShadowsAndDesaturation.Y = Shadows.Y;
		ShadowsAndDesaturation.Z = Shadows.Z;
		ShadowsAndDesaturation.W = (1.f - Desaturation);

		SetPixelShaderValue(
			PixelShader->GetPixelShader(),
			SceneShadowsAndDesaturation,
			ShadowsAndDesaturation
			);

		// MaterialPower

		SetPixelShaderValue(
			PixelShader->GetPixelShader(),
			SceneMidTones,
			MidTones
			);

		// SceneScaledLuminanceWeights

		FVector4 ScaledLuminanceWeights;

		ScaledLuminanceWeights.X = 0.30000001f * Desaturation;
		ScaledLuminanceWeights.Y = 0.58999997f * Desaturation;
		ScaledLuminanceWeights.Z = 0.11000000f * Desaturation;
		ScaledLuminanceWeights.W = 0.f; // Unused

		SetPixelShaderValue(
			PixelShader->GetPixelShader(),
			SceneScaledLuminanceWeights,
			ScaledLuminanceWeights
			);
	}

	/** Serializer. */
	friend FArchive& operator<<(FArchive& Ar,FMaterialShaderParameters& P)
	{
		Ar << P.SceneShadowsAndDesaturation;
		Ar << P.SceneInverseHighLights;
		Ar << P.SceneMidTones;
		Ar << P.SceneScaledLuminanceWeights;
		return Ar;
	}

private:
	FShaderParameter SceneShadowsAndDesaturation;
	FShaderParameter SceneInverseHighLights;
	FShaderParameter SceneMidTones;
	FShaderParameter SceneScaledLuminanceWeights;
};


/*-----------------------------------------------------------------------------
FMGammaShaderParameters
-----------------------------------------------------------------------------*/

/** Encapsulates the gamma correction parameters. */
class FGammaShaderParameters
{
public:

	/** Default constructor. */
	FGammaShaderParameters() {}

	/** Initialization constructor. */
	FGammaShaderParameters(const FShaderParameterMap& ParameterMap)
	{
		GammaColorScaleAndInverse.Bind(ParameterMap,TEXT("GammaColorScaleAndInverse"),TRUE);
		GammaOverlayColor.Bind(ParameterMap,TEXT("GammaOverlayColor"),TRUE);
	}

	/** Set the material shader parameter values. */
	void Set(FShader* PixelShader, FLOAT DisplayGamma, FLinearColor const& ColorScale, FLinearColor const& ColorOverlay)
	{
		// GammaColorScaleAndInverse

		FLOAT InvDisplayGamma = 1.f / Max<FLOAT>(DisplayGamma,KINDA_SMALL_NUMBER);
		FLOAT OneMinusOverlayBlend = 1.f - ColorOverlay.A;

		FVector4 ColorScaleAndInverse;

		ColorScaleAndInverse.X = Max<FLOAT>(ColorScale.R * OneMinusOverlayBlend, KINDA_SMALL_NUMBER);
		ColorScaleAndInverse.Y = Max<FLOAT>(ColorScale.G * OneMinusOverlayBlend, KINDA_SMALL_NUMBER);
		ColorScaleAndInverse.Z = Max<FLOAT>(ColorScale.B * OneMinusOverlayBlend, KINDA_SMALL_NUMBER);
		ColorScaleAndInverse.W = InvDisplayGamma;

		SetPixelShaderValue(
			PixelShader->GetPixelShader(),
			GammaColorScaleAndInverse,
			ColorScaleAndInverse
			);

		// GammaOverlayColor

		FVector4 OverlayColor;

		OverlayColor.X = ColorOverlay.R * ColorOverlay.A / ColorScaleAndInverse.X; // Note that the divide by the ColorScale is just done to work around some weirdness in the HLSL compiler
		OverlayColor.Y = ColorOverlay.G * ColorOverlay.A / ColorScaleAndInverse.Y; // (see UberPostProcessBlendPixelShader.usf)
		OverlayColor.Z = ColorOverlay.B * ColorOverlay.A / ColorScaleAndInverse.Z; 
		OverlayColor.W = 0.f; // Unused

		SetPixelShaderValue(
			PixelShader->GetPixelShader(),
			GammaOverlayColor,
			OverlayColor
			);
	}

	/** Serializer. */
	friend FArchive& operator<<(FArchive& Ar,FGammaShaderParameters& P)
	{
		Ar << P.GammaColorScaleAndInverse;
		Ar << P.GammaOverlayColor;
		return Ar;
	}

private:
	FShaderParameter GammaColorScaleAndInverse;
	FShaderParameter GammaOverlayColor;
};

/*-----------------------------------------------------------------------------
FMGammaShaderParameters
-----------------------------------------------------------------------------*/

/** Encapsulates the gamma correction parameters. */
class FMotionBlurShaderParameters
{
public:

	/** Default constructor. */
	FMotionBlurShaderParameters() {}

	/** Initialization constructor. */
	FMotionBlurShaderParameters(const FShaderParameterMap& ParameterMap)
	{
		VelocityBuffer.Bind(ParameterMap, TEXT("VelocityBuffer"), TRUE );
		ScreenToWorldParameter.Bind(ParameterMap, TEXT("ScreenToWorld"), TRUE );
		PrevViewProjParameter.Bind(ParameterMap, TEXT("PrevViewProjMatrix"), TRUE );
		StaticVelocityParameters.Bind(ParameterMap, TEXT("StaticVelocityParameters"), TRUE );
		DynamicVelocityParameters.Bind(ParameterMap, TEXT("DynamicVelocityParameters"), TRUE );
		RenderTargetClampParameter.Bind(ParameterMap, TEXT("RenderTargetClampParameter"), TRUE );
		MotionBlurMaskScaleParameter.Bind(ParameterMap, TEXT("MotionBlurMaskScaleAndBias"), TRUE );
	}

	/** Set the material shader parameter values. */
	void Set(FShader* PixelShader, const FViewInfo& View, FLOAT BlurAmount, FLOAT MaxVelocity)
	{
		// Calculate the maximum velocities (MAX_PIXELVELOCITY is per 30 fps frame).
		const FLOAT SizeX = View.SizeX;
		const FLOAT SizeY = View.SizeY;
		FLOAT VelocityX = MAX_PIXELVELOCITY * MaxVelocity;
		FLOAT VelocityY = MAX_PIXELVELOCITY * MaxVelocity * SizeY / SizeX;

		const FSceneViewState* ViewState = (FSceneViewState*)View.State;
		BlurAmount *= ViewState ? ViewState->MotionBlurTimeScale : 1.0f;

		// Converts projection space velocity to texel space [0,1].
		FVector4 StaticVelocity( 0.5f*BlurAmount, -0.5f*BlurAmount, VelocityX, VelocityY );
		SetPixelShaderValue( PixelShader->GetPixelShader(), StaticVelocityParameters, StaticVelocity );

		// Scale values from the biased velocity buffer [-1,+1] to texel space [-MaxVelocity,+MaxVelocity].
		FVector4 DynamicVelocity( VelocityX, -VelocityY, 0.0f, 0.0f );
		SetPixelShaderValue( PixelShader->GetPixelShader(), DynamicVelocityParameters, DynamicVelocity );

		// Calculate and set the ScreenToWorld matrix.
		FMatrix ScreenToWorld = FMatrix(
				FPlane(1,0,0,0),
				FPlane(0,1,0,0),
				FPlane(0,0,(1.0f - Z_PRECISION),1),
				FPlane(0,0,-View.NearClippingDistance * (1.0f - Z_PRECISION),0) ) * View.InvViewProjectionMatrix;

		ScreenToWorld.M[0][3] = 0.f; // Note that we reset the column here because in the shader we only used
		ScreenToWorld.M[1][3] = 0.f; // the x, y, and z components of the matrix multiplication result and we
		ScreenToWorld.M[2][3] = 0.f; // set the w component to 1 before multiplying by the PrevViewProjMatrix.
		ScreenToWorld.M[3][3] = 1.f;

		FMatrix CombinedMatrix = ScreenToWorld * View.PrevViewProjMatrix;
		SetPixelShaderValue( PixelShader->GetPixelShader(), PrevViewProjParameter, CombinedMatrix );

		// Set clamp values that help us avoid sampling from outside viewport region
		const FVector4 RenderTargetClampValues(
			(View.RenderTargetX - GPixelCenterOffset) / (FLOAT)GSceneRenderTargets.GetBufferSizeX(),
			(View.RenderTargetY - GPixelCenterOffset) / (FLOAT)GSceneRenderTargets.GetBufferSizeY(),
			(View.RenderTargetX + View.RenderTargetSizeX - GPixelCenterOffset) / (FLOAT)GSceneRenderTargets.GetBufferSizeX(),
			(View.RenderTargetY + View.RenderTargetSizeY - GPixelCenterOffset) / (FLOAT)GSceneRenderTargets.GetBufferSizeY());
		SetPixelShaderValue( PixelShader->GetPixelShader(), RenderTargetClampParameter, RenderTargetClampValues );

		SetTextureParameter( PixelShader->GetPixelShader(), VelocityBuffer, TStaticSamplerState<SF_Point>::GetRHI(), GSceneRenderTargets.GetVelocityTexture() );

#if USE_XeD3D_RHI
		// When the scene color texture is not Raw, we need to scale it by SCENE_COLOR_BIAS_FACTOR_EXP to re-normalize the range:
		FLOAT SceneColorScale = GSceneRenderTargets.bSceneColorTextureIsRaw ? 1.0f : appPow(2.0f,GSceneRenderTargets.GetSceneColorSurface().XeSurfaceInfo.GetColorExpBias());
		FVector4 ScaleAndBias( 3.0f * SceneColorScale, -2.0f, 0.0f, 0.0f );
		SetPixelShaderValue( PixelShader->GetPixelShader(), MotionBlurMaskScaleParameter, ScaleAndBias);
#endif
	}

	/** Serializer. */
	friend FArchive& operator<<(FArchive& Ar,FMotionBlurShaderParameters& P)
	{
		Ar << P.VelocityBuffer;
		Ar << P.ScreenToWorldParameter;
		Ar << P.PrevViewProjParameter;
		Ar << P.StaticVelocityParameters;
		Ar << P.DynamicVelocityParameters;
		Ar << P.RenderTargetClampParameter;
		Ar << P.MotionBlurMaskScaleParameter;
		return Ar;
	}

private:
	FShaderResourceParameter VelocityBuffer;
	FShaderParameter ScreenToWorldParameter;
	FShaderParameter PrevViewProjParameter;
	FShaderParameter StaticVelocityParameters;		// = { 0.5f, -0.5f, 16.0f/1280.0f, 16.0f/720.0f }
	FShaderParameter DynamicVelocityParameters;		// = { 2.0f*16.0f/1280.0f, -2.0f*16.0f/720.0f, -16.0f/1280.0f, 16.0f/720.0f }
	FShaderParameter RenderTargetClampParameter;	//!< Viewport region in render target = { MinUV.x, MinUV.y, MaxUV.x, MaxUV.y }
	FShaderParameter MotionBlurMaskScaleParameter;	// XY used for ScaleAndBias: { 3*SCENE_COLOR_BIAS_FACTOR, -2, 0, 0 }
};

/*-----------------------------------------------------------------------------
FUberPostProcessBlendPixelShader
-----------------------------------------------------------------------------*/

/** Encapsulates the blend pixel shader. */
template<UBOOL bMotionBlur>
class FUberPostProcessBlendPixelShader : public FDOFAndBloomBlendPixelShader
{
	DECLARE_SHADER_TYPE(FUberPostProcessBlendPixelShader,Global);

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("MOTION_BLUR"),bMotionBlur ? TEXT("1") : TEXT("0"));
	}

	/** Default constructor. */
	FUberPostProcessBlendPixelShader() {}

public:
	FMaterialShaderParameters     MaterialParameters;
	FGammaShaderParameters        GammaParameters;
	FMotionBlurShaderParameters	  MotionBlurParameters;

	/** Initialization constructor. */
	FUberPostProcessBlendPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FDOFAndBloomBlendPixelShader(Initializer)
		,	MaterialParameters(Initializer.ParameterMap)
		,	GammaParameters(Initializer.ParameterMap)
		,	MotionBlurParameters(Initializer.ParameterMap)
	{
	}

	// FShader interface.
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FDOFAndBloomBlendPixelShader::Serialize(Ar);
		Ar << MaterialParameters << GammaParameters << MotionBlurParameters;
		return bShaderHasOutdatedParameters;
	}
};

IMPLEMENT_SHADER_TYPE(template<>,FUberPostProcessBlendPixelShader<TRUE>,TEXT("UberPostProcessBlendPixelShader"),TEXT("Main"),SF_Pixel,VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD,0);
IMPLEMENT_SHADER_TYPE(template<>,FUberPostProcessBlendPixelShader<FALSE>,TEXT("UberPostProcessBlendPixelShader"),TEXT("Main"),SF_Pixel,VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD,0);


/*-----------------------------------------------------------------------------
FUberPostProcessVertexShader
-----------------------------------------------------------------------------*/

/** Encapsulates the UberPostProcess vertex shader. */
class FUberPostProcessVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FUberPostProcessVertexShader,Global);

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
	}

	/** Default constructor. */
	FUberPostProcessVertexShader() {}
	
	FUberPostProcessVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
		SceneCoordinateScaleBiasParameter.Bind(Initializer.ParameterMap,TEXT("SceneCoordinateScaleBias"),TRUE);
	}

	UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << SceneCoordinateScaleBiasParameter;
		return bShaderHasOutdatedParameters;
	}

public:
	FShaderParameter SceneCoordinateScaleBiasParameter;
};

IMPLEMENT_SHADER_TYPE(,FUberPostProcessVertexShader,TEXT("UberPostProcessVertexShader"),TEXT("Main"),SF_Vertex,0,0);


/*-----------------------------------------------------------------------------
FUberPostProcessSceneProxy
-----------------------------------------------------------------------------*/

extern TGlobalResource<FFilterVertexDeclaration> GFilterVertexDeclaration;

template<UBOOL bMotionBlur>
class FUberPostProcessSceneProxy : public FDOFAndBloomPostProcessSceneProxy
{
public:
	/** 
	* Initialization constructor. 
	* @param InEffect - Uber post process effect to mirror in this proxy
	*/
	FUberPostProcessSceneProxy(const UUberPostProcessEffect* InEffect,const FPostProcessSettings* WorldSettings)
		:	FDOFAndBloomPostProcessSceneProxy(InEffect, WorldSettings)
		,	SceneShadows(WorldSettings ? WorldSettings->Scene_Shadows : InEffect->SceneShadows)
		,	SceneHighLights(WorldSettings ? WorldSettings->Scene_HighLights : InEffect->SceneHighLights)
		,	SceneMidTones(WorldSettings ? WorldSettings->Scene_MidTones : InEffect->SceneMidTones)
		,	SceneDesaturation(WorldSettings ? WorldSettings->Scene_Desaturation : InEffect->SceneDesaturation)
		,	MaxVelocity(WorldSettings ? WorldSettings->MotionBlur_MaxVelocity : InEffect->MaxVelocity)
		,	MotionBlurAmount(WorldSettings ? WorldSettings->MotionBlur_Amount : InEffect->MotionBlurAmount)
		,	bFullMotionBlur(WorldSettings ? WorldSettings->MotionBlur_FullMotionBlur : InEffect->FullMotionBlur)
		,	CameraRotationThreshold(WorldSettings ? WorldSettings->MotionBlur_CameraRotationThreshold : InEffect->CameraRotationThreshold)
		,	CameraTranslationThreshold(WorldSettings ? WorldSettings->MotionBlur_CameraTranslationThreshold : InEffect->CameraTranslationThreshold)
	{
		extern INT GMotionBlurFullMotionBlur;
		bFullMotionBlur = GMotionBlurFullMotionBlur < 0 ? bFullMotionBlur : (GMotionBlurFullMotionBlur > 0);
		if(WorldSettings && !WorldSettings->bEnableSceneEffect)
		{
			SceneShadows = FVector(0.f, 0.f, 0.f);
			SceneHighLights = FVector(1.f, 1.f, 1.f);
			SceneMidTones = FVector(1.f, 1.f, 1.f);
			SceneDesaturation = 0.f;
		}
	}

	/**
	* Render the post process effect
	* Called by the rendering thread during scene rendering
	* @param InDepthPriorityGroup - scene DPG currently being rendered
	* @param View - current view
	* @param CanvasTransform - same canvas transform used to render the scene
	* @return TRUE if anything was rendered
	*/
	UBOOL Render(const FScene* Scene, UINT InDepthPriorityGroup,FViewInfo& View,const FMatrix& CanvasTransform)
	{
		SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("UberPostProcessing%s"),bMotionBlur ? TEXT(" with MotionBlur") : TEXT(""));

		check(SDPG_PostProcess==InDepthPriorityGroup);
		check(FALSE==View.bUseLDRSceneColor);

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

		// The combined (uber) post-processing shader does depth of field, bloom, material colorization and gamma correction all in one
		// pass.
		//
		// This means it takes an HDR (64-bit) input and produces a low dynamic range (32-bit) output.  If it is the final post processing
		// shader in the post processing chain then it can render directly into the view's render target.  Otherwise it has to render into
		// a 32-bit render target.
		//
		// Note: Any post-processing shader that follows the uber shader needs to be able to handle an LDR input.  The shader can can check
		//       for this by looking at the bUseLDRSceneColor flag in the FViewInfo structure.  Also the final shader following the uber
		//       shader needs to write into the view's render target (or needs to write the result out to a 64-bit render-target).

		FLOAT DisplayGamma = View.Family->RenderTarget->GetDisplayGamma();

		UBOOL bAllowPostProcessMask = TRUE;
		if( View.Family->bResolveScene && FinalEffectInGroup && !GSystemSettings.NeedsUpscale() )  
		{
			// disable masking when rendering to final viewport RT;
			bAllowPostProcessMask = FALSE;
		}

		// apply mask if available
		if( bAllowPostProcessMask &&
			View.PostProcessMask && 
			View.PostProcessMask->ShouldRender() )
		{
			View.PostProcessMask->BeginStencilMask();
		}

		// if the scene wasn't meant to be resolved to LDR then continue rendering to HDR
		if( !View.Family->bResolveScene )			
		{
			// Using 64-bit (HDR) surface
			GSceneRenderTargets.BeginRenderingSceneColor();
			// disable gamma correction
			DisplayGamma = 1.0f;
		}
		else
		{
			//if this is the final effect then render directly to the view's render target
			//unless an upscale is needed, in which case render to LDR scene color
			if (FinalEffectInGroup && !GSystemSettings.NeedsUpscale())
			{
				RHISetRenderTarget(View.Family->RenderTarget->GetRenderTargetSurface(), FSurfaceRHIRef()); // Render to the final render target
			}
			else
			{
				GSceneRenderTargets.BeginRenderingSceneColorLDR();
			}
		}

		// Set the blend vertex shader.
		TShaderMapRef<FUberPostProcessVertexShader> BlendVertexShader(GetGlobalShaderMap());

		// Set the blend pixel shader.
		TShaderMapRef<FUberPostProcessBlendPixelShader<bMotionBlur> > BlendPixelShader(GetGlobalShaderMap());

		// Set the DOF and bloom parameters
		BlendPixelShader->DOFParameters.Set(
			*BlendPixelShader,
			FocusDistance,
			FocusInnerRadius,
			FalloffExponent,
			MaxNearBlurAmount,
			MaxFarBlurAmount
			);

		// Set the material colorization parameters
		BlendPixelShader->MaterialParameters.Set(
			*BlendPixelShader,
			SceneShadows,
			SceneHighLights,
			SceneMidTones,
			// ensure that desat values out of range get clamped (this can happen in the editor currently)
			Clamp(SceneDesaturation, 0.f, 1.f)
			);

		// Set the gamma correction parameters
		BlendPixelShader->GammaParameters.Set(
			*BlendPixelShader,
			DisplayGamma,
			View.ColorScale,
			View.OverlayColor
			);
			
		if(bMotionBlur)
		{
			RHISetShaderRegisterAllocation(16, 112);

			BlendPixelShader->MotionBlurParameters.Set(
				*BlendPixelShader,
				View,
				MotionBlurAmount, 
				MaxVelocity 
				);
		}

		// Setup the scene texture
		BlendPixelShader->SceneTextureParameters.Set(&View,*BlendPixelShader);

		// Setup the pre-filtered DOF/Bloom texture
		SetTextureParameter(
			BlendPixelShader->GetPixelShader(),
			BlendPixelShader->BlurredImageParameter,
			TStaticSamplerState<SF_Bilinear>::GetRHI(),
			GSceneRenderTargets.GetFilterColorTexture()
			);

		SetGlobalBoundShaderState( UberBlendBoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *BlendVertexShader, *BlendPixelShader, sizeof(FFilterVertex));

		if( FinalEffectInGroup
			&& View.Family->bResolveScene 
			&& !GSystemSettings.NeedsUpscale() )
		{
			// We need to adjust the UV coordinate calculation for the scene color texture when rendering directly to
			// the view's render target (to support the editor).

			UINT TargetSizeX = View.Family->RenderTarget->GetSizeX();
			UINT TargetSizeY = View.Family->RenderTarget->GetSizeY();

			FLOAT ScaleX = TargetSizeX / static_cast<float>(BufferSizeX);
			FLOAT ScaleY = TargetSizeY / static_cast<float>(BufferSizeY);

			FLOAT UVScaleX  = +0.5f * ScaleX;
			FLOAT UVScaleY  = -0.5f * ScaleY;

			// we are not going to sample RenderTarget. the pixel size should be taken from the sampled buffer
			FLOAT UVOffsetX = +0.5f * ScaleX + GPixelCenterOffset / static_cast<float>(BufferSizeX);
			FLOAT UVOffsetY = +0.5f * ScaleY + GPixelCenterOffset / static_cast<float>(BufferSizeY);

			// For split-screen and 4:3 views we also need to take into account the viewport correctly; however, the
			// DOFAndBloomBlendVertex shader computes the UV coordinates for the SceneColor texture directly from the
			// screen coordinates that are used to render and since the view-port may not be located at 0,0 we need
			// to adjust for that by modifying the UV offset and scale.

			// we are not going to sample RenderTarget. the pixel size should be taken from the sampled buffer
			UVOffsetX -= (View.X - View.RenderTargetX ) / BufferSizeX;
			UVOffsetY -= (View.Y - View.RenderTargetY ) / BufferSizeY;

			SetVertexShaderValue(
				BlendVertexShader->GetVertexShader(),
				BlendVertexShader->SceneCoordinateScaleBiasParameter,
				FVector4(
					UVScaleX,
					UVScaleY,
					UVOffsetY,
					UVOffsetX
					)
				);

			// Draw a quad mapping the blurred pixels in the filter buffer to the scene color buffer.
			DrawDenormalizedQuad(
				View.X, View.Y, View.SizeX, View.SizeY,
				1,1, DownsampledSizeX,DownsampledSizeY,
				TargetSizeX, TargetSizeY,
				FilterBufferSizeX, FilterBufferSizeY);

			// restore mask if available
			if( bAllowPostProcessMask &&
				View.PostProcessMask && 
				View.PostProcessMask->ShouldRender() )
			{
				View.PostProcessMask->EndStencilMask();
			}
		}
		else
		{
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

			// Draw a quad mapping the blurred pixels in the filter buffer to the scene color buffer.
			DrawDenormalizedQuad(
				View.RenderTargetX,View.RenderTargetY, View.RenderTargetSizeX,View.RenderTargetSizeY,
				1,1, DownsampledSizeX,DownsampledSizeY,
				BufferSizeX, BufferSizeY,
				FilterBufferSizeX, FilterBufferSizeY);

			// restore mask if available
			if( bAllowPostProcessMask &&
				View.PostProcessMask && 
				View.PostProcessMask->ShouldRender() )
			{
				View.PostProcessMask->EndStencilMask();
			}

			FResolveParams ResolveParams;
			ResolveParams.X1 = View.RenderTargetX;
			ResolveParams.Y1 = View.RenderTargetY;
			ResolveParams.X2 = View.RenderTargetX + View.RenderTargetSizeX;
			ResolveParams.Y2 = View.RenderTargetY + View.RenderTargetSizeY;

			if( View.Family->bResolveScene )
			{
				// Resolve the scene color LDR buffer.
				GSceneRenderTargets.FinishRenderingSceneColorLDR(TRUE,ResolveParams);
			}
			else
			{
				// Resolve the scene color HDR buffer.
				GSceneRenderTargets.FinishRenderingSceneColor(TRUE);
			}
		}

		if( View.Family->bResolveScene )
		{
			// Indicate that from now on the scene color is in an LDR surface.
			View.bUseLDRSceneColor = TRUE; 
		}

		return TRUE;
	}
	/**
	 * Informs FSceneRenderer what to do during pre-pass.
	 * @param MotionBlurParameters	- The parameters for the motion blur effect are returned in this struct.
	 * @return Motion blur needs to have velocities written during pre-pass.
	 */
	virtual UBOOL RequiresVelocities( FMotionBlurParameters &MotionBlurParameters ) const
	{
		if ( bMotionBlur )
		{
			MotionBlurParameters.VelocityScale			= MotionBlurAmount;
			MotionBlurParameters.MaxVelocity			= MaxVelocity;
			MotionBlurParameters.bFullMotionBlur		= bFullMotionBlur;
			MotionBlurParameters.RotationThreshold		= CameraRotationThreshold;
			MotionBlurParameters.TranslationThreshold	= CameraTranslationThreshold;
		}
		return bMotionBlur;
	}

	/**
	 * Tells FSceneRenderer whether to store the previous frame's transforms.
	 */
	virtual UBOOL RequiresPreviousTransforms(const FViewInfo& View) const
	{
		return bMotionBlur;
	}

protected:
	/** mirrored material properties (see UberPostProcessEffect.uc) */
	FVector			SceneShadows;
	FVector			SceneHighLights;
	FVector			SceneMidTones;
	FLOAT			SceneDesaturation;
	/** Mirrored properties. See DOFBloomMotionBlurEffect.uc for descriptions */
    FLOAT MaxVelocity;
    FLOAT MotionBlurAmount;
	UBOOL bFullMotionBlur;
	FLOAT CameraRotationThreshold;
	FLOAT CameraTranslationThreshold;

	/** bound shader state for the blend pass */
	static FGlobalBoundShaderState UberBlendBoundShaderState;
};

template<UBOOL bMotionBlur> FGlobalBoundShaderState FUberPostProcessSceneProxy<bMotionBlur>::UberBlendBoundShaderState;

/**
 * Creates a proxy to represent the render info for a post process effect
 * @param WorldSettings - The world's post process settings for the view.
 * @return The proxy object.
 */
FPostProcessSceneProxy* UUberPostProcessEffect::CreateSceneProxy(const FPostProcessSettings* WorldSettings)
{
#if XBOX
	// Disable motion blur for tiled screenshots; the way motion blur is handled is
	// incompatible with tiled rendering and a single view.
	// Probably we could workaround with multiple views if it is necessery.
	extern UBOOL GIsTiledScreenshot;
	extern INT GGameScreenshotCounter;

	if ( (WorldSettings == NULL || WorldSettings->bEnableMotionBlur) && GSystemSettings.bAllowMotionBlur && !GIsTiledScreenshot && (GGameScreenshotCounter == 0) )
	{
		return new FUberPostProcessSceneProxy<TRUE>(this,WorldSettings);
	}
	else
#endif
	{
		return new FUberPostProcessSceneProxy<FALSE>(this,WorldSettings);
	}
}
