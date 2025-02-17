/*=============================================================================
	ScenePostProcessing.cpp: Scene post processing implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "SceneFilterRendering.h"

/** Encapsulates the gamma correction pixel shader. */
class FGammaCorrectionPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FGammaCorrectionPixelShader,Global);

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
	}

	/** Default constructor. */
	FGammaCorrectionPixelShader() {}

public:

	FShaderResourceParameter SceneTextureParameter;
	FShaderParameter InverseGammaParameter;
	FShaderParameter ColorScaleParameter;
	FShaderParameter OverlayColorParameter;

	/** Initialization constructor. */
	FGammaCorrectionPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
		SceneTextureParameter.Bind(Initializer.ParameterMap,TEXT("SceneColorTexture"));
		InverseGammaParameter.Bind(Initializer.ParameterMap,TEXT("InverseGamma"));
		ColorScaleParameter.Bind(Initializer.ParameterMap,TEXT("ColorScale"));
		OverlayColorParameter.Bind(Initializer.ParameterMap,TEXT("OverlayColor"));
	}

	// FShader interface.
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << SceneTextureParameter << InverseGammaParameter << ColorScaleParameter << OverlayColorParameter;
		return bShaderHasOutdatedParameters;
	}
};

IMPLEMENT_SHADER_TYPE(,FGammaCorrectionPixelShader,TEXT("GammaCorrectionPixelShader"),TEXT("Main"),SF_Pixel,VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD,0);

/** Encapsulates the gamma correction vertex shader. */
class FGammaCorrectionVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FGammaCorrectionVertexShader,Global);

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
	}

	/** Default constructor. */
	FGammaCorrectionVertexShader() {}

public:

	/** Initialization constructor. */
	FGammaCorrectionVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
	}
};

IMPLEMENT_SHADER_TYPE(,FGammaCorrectionVertexShader,TEXT("GammaCorrectionVertexShader"),TEXT("Main"),SF_Vertex,0,0);

extern TGlobalResource<FFilterVertexDeclaration> GFilterVertexDeclaration;

FGlobalBoundShaderState FSceneRenderer::PostProcessBoundShaderState;

/**
* Finish rendering a view, writing the contents to ViewFamily.RenderTarget.
* @param View - The view to process.
*/
void FSceneRenderer::FinishRenderViewTarget(const FViewInfo* View)
{
	// If the bUseLDRSceneColor flag is set then that means the final post-processing shader has already renderered to
	// the view's render target and that one of the post-processing shaders has performed the gamma correction,
	// unless the scene needs an upscale in which case LDR scene color needs to be copied to the view's render target.
	if( View->bUseLDRSceneColor 
		&& !GSystemSettings.NeedsUpscale()
		// Also skip the final copy to View.RenderTarget if disabled by the view family
		|| !View->Family->bResolveScene )
	{
		return;
	}

	//if the shader complexity viewmode is enabled, use that to render to the view's rendertarget
	if (View->Family->ShowFlags & SHOW_ShaderComplexity)
	{
		RenderShaderComplexity(View);
		return;
	}

	// Set the view family's render target/viewport.
	RHISetRenderTarget(ViewFamily.RenderTarget->GetRenderTargetSurface(),FSurfaceRHIRef());	

	// Deferred the clear until here so the garbage left in the non rendered regions by the post process effects do not show up
	if( ViewFamily.bDeferClear )
	{
		RHIClear(  TRUE, FLinearColor::Black, FALSE, 0.0f, FALSE, 0 );
		ViewFamily.bDeferClear = FALSE;
	}

	// turn off culling and blending
	RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
	RHISetBlendState(TStaticBlendState<>::GetRHI());
		
	// turn off depth reads/writes
	RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());

	TShaderMapRef<FGammaCorrectionVertexShader> VertexShader(GetGlobalShaderMap());
	TShaderMapRef<FGammaCorrectionPixelShader> PixelShader(GetGlobalShaderMap());

	SetGlobalBoundShaderState( PostProcessBoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FFilterVertex));

	FLOAT InvDisplayGamma = 1.0f / ViewFamily.RenderTarget->GetDisplayGamma();

	if (GSystemSettings.NeedsUpscale() && View->bUseLDRSceneColor)
	{
		//don't gamma correct if we are copying from LDR scene color, since the PP effect that copied it there will have already gamma corrected
		InvDisplayGamma = 1.0f;
	}

	SetPixelShaderValue(
		PixelShader->GetPixelShader(),
		PixelShader->InverseGammaParameter,
		InvDisplayGamma
		);
	SetPixelShaderValue(PixelShader->GetPixelShader(),PixelShader->ColorScaleParameter,View->ColorScale);
	SetPixelShaderValue(PixelShader->GetPixelShader(),PixelShader->OverlayColorParameter,View->OverlayColor);

	if (GSystemSettings.NeedsUpscale())
	{
		INT UnscaledViewX = 0;
		INT UnscaledViewY = 0;
		UINT UnscaledViewSizeX = 0;
		UINT UnscaledViewSizeY = 0;

		//convert view constraints to their unscaled versions
		GSystemSettings.UnScaleScreenCoords(
			UnscaledViewX, UnscaledViewY, 
			UnscaledViewSizeX, UnscaledViewSizeY, 
			View->X, View->Y, 
			View->SizeX, View->SizeY);

		RHISetViewport( 0, 0, 0.0f, ViewFamily.RenderTarget->GetSizeX(), ViewFamily.RenderTarget->GetSizeY(), 1.0f);

		if (View->bUseLDRSceneColor)
		{
			//a PP effect has already copied to LDR scene color, so read from that
			SetTextureParameter(
				PixelShader->GetPixelShader(),
				PixelShader->SceneTextureParameter,
				//use linear filtering to get a smoother upsample
				TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
				GSceneRenderTargets.GetSceneColorLDRTexture()
				);
		}
		else
		{
			FSamplerStateRHIParamRef SceneTextureSamplerState = TStaticSamplerState<>::GetRHI();
			// Use linear filtering if supported on floating point scene color, or if we are running SM2 in which case scene color is fixed point.
			if (GSupportsFPFiltering || !CanBlendWithFPRenderTarget(GRHIShaderPlatform))
			{
				SceneTextureSamplerState = TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
			}

			//in this case we are copying from HDR scene color to the view's render target
			SetTextureParameter(
				PixelShader->GetPixelShader(),
				PixelShader->SceneTextureParameter,
				SceneTextureSamplerState,
				GSceneRenderTargets.GetSceneColorTexture()
				);
		}
		

		// Draw a quad mapping scene color to the view's render target
		DrawDenormalizedQuad(
			UnscaledViewX,UnscaledViewY,
			UnscaledViewSizeX,UnscaledViewSizeY,
			View->RenderTargetX,View->RenderTargetY,
			View->RenderTargetSizeX,View->RenderTargetSizeY,
			ViewFamily.RenderTarget->GetSizeX(),ViewFamily.RenderTarget->GetSizeY(),
			GSceneRenderTargets.GetBufferSizeX(),GSceneRenderTargets.GetBufferSizeY()
			);
	}
	else
	{
		SetTextureParameter(
			PixelShader->GetPixelShader(),
			PixelShader->SceneTextureParameter,
			TStaticSamplerState<>::GetRHI(),
			GSceneRenderTargets.GetSceneColorTexture()
			);

		// Draw a quad mapping scene color to the view's render target
		DrawDenormalizedQuad(
			View->X,View->Y,
			View->SizeX,View->SizeY,
			View->RenderTargetX,View->RenderTargetY,
			View->RenderTargetSizeX,View->RenderTargetSizeY,
			ViewFamily.RenderTarget->GetSizeX(),ViewFamily.RenderTarget->GetSizeY(),
			GSceneRenderTargets.GetBufferSizeX(),GSceneRenderTargets.GetBufferSizeY()
			);
	}
}
