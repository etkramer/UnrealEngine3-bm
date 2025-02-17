/*=============================================================================
	UnMotionBlurEffect.cpp: Motion blur post process effect implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "SceneFilterRendering.h"

IMPLEMENT_CLASS(UMotionBlurEffect);
IMPLEMENT_CLASS(UDOFBloomMotionBlurEffect);


//=============================================================================
/** Encapsulates the Motion Blur post-process vertex shader. */
class FMotionBlurVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FMotionBlurVertexShader,Global);
public:
	/** Initialization constructor. */
	FMotionBlurVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
	}

private:
	/** Default constructor. */
	FMotionBlurVertexShader() {}

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
	}
};

IMPLEMENT_SHADER_TYPE(,FMotionBlurVertexShader,TEXT("MotionBlurShader"),TEXT("MainVertexShader"),SF_Vertex,VER_MOTIONBLURSHADER_RECOMPILE_VER2,0);


//=============================================================================
/** Encapsulates the Motion Blur post-process pixel shader. */
class FMotionBlurPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FMotionBlurPixelShader,Global);
public:
	/** Initialization constructor. */
	FMotionBlurPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
		SceneTextureParameters.Bind(Initializer.ParameterMap);
		VelocityBuffer.Bind(Initializer.ParameterMap, TEXT("VelocityBuffer"), TRUE );
		ScreenToWorldParameter.Bind(Initializer.ParameterMap, TEXT("ScreenToWorld"), TRUE );
		PrevViewProjParameter.Bind(Initializer.ParameterMap, TEXT("PrevViewProjMatrix"), TRUE );
		StaticVelocityParameters.Bind(Initializer.ParameterMap, TEXT("StaticVelocityParameters"), TRUE );
		DynamicVelocityParameters.Bind(Initializer.ParameterMap, TEXT("DynamicVelocityParameters"), TRUE );
		RenderTargetClampParameter.Bind(Initializer.ParameterMap, TEXT("RenderTargetClampParameter"), TRUE );
		MotionBlurMaskScaleParameter.Bind(Initializer.ParameterMap, TEXT("MotionBlurMaskScaleAndBias"), TRUE );
	}

	// FShader interface.
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << SceneTextureParameters;
		Ar << VelocityBuffer;
		Ar << ScreenToWorldParameter;
		Ar << PrevViewProjParameter;
		Ar << StaticVelocityParameters;
		Ar << DynamicVelocityParameters;
		Ar << RenderTargetClampParameter;
		Ar << MotionBlurMaskScaleParameter;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters( const FViewInfo& View, FLOAT BlurAmount, FLOAT MaxVelocity )
	{
		const FSceneViewState* ViewState = (FSceneViewState*)View.State;

		// Setup parameters to lookup color and depth buffers.  Note that if we are operating in LDR space we use a 32-bit variant of the scene color texture.
#if XBOX
		if (View.bUseLDRSceneColor)
		{
			SceneTextureParameters.Set( &View, this, SF_Bilinear);
		}
		else
		{
			// The floating point Xbox render target format doesn't support filtering without using the _EXPAND version.
			SceneTextureParameters.Set( &View, this, SF_Point); 
		}
#else
		SceneTextureParameters.Set( &View, this, SF_Bilinear);
#endif

		SetTextureParameter( GetPixelShader(), VelocityBuffer, TStaticSamplerState<SF_Point>::GetRHI(), GSceneRenderTargets.GetVelocityTexture() );

		// Calculate the maximum velocities (MAX_PIXELVELOCITY is per 30 fps frame).
		const FLOAT SizeX = View.SizeX;
		const FLOAT SizeY = View.SizeY;
		FLOAT VelocityX = MAX_PIXELVELOCITY * MaxVelocity;
		FLOAT VelocityY = MAX_PIXELVELOCITY * MaxVelocity * SizeY / SizeX;
		BlurAmount *= ViewState->MotionBlurTimeScale;

		// Converts projection space velocity to texel space [0,1].
		FVector4 StaticVelocity( 0.5f*BlurAmount, -0.5f*BlurAmount, VelocityX, VelocityY );
		SetPixelShaderValue( GetPixelShader(), StaticVelocityParameters, StaticVelocity );

		// Scale values from the biased velocity buffer [-1,+1] to texel space [-MaxVelocity,+MaxVelocity].
		FVector4 DynamicVelocity( VelocityX, -VelocityY, 0.0f, 0.0f );
		SetPixelShaderValue( GetPixelShader(), DynamicVelocityParameters, DynamicVelocity );

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
		SetPixelShaderValue( GetPixelShader(), PrevViewProjParameter, CombinedMatrix );

		// Set clamp values that help us avoid sampling from outside viewport region
		const FVector4 RenderTargetClampValues(
			(View.RenderTargetX - GPixelCenterOffset) / (FLOAT)GSceneRenderTargets.GetBufferSizeX(),
			(View.RenderTargetY - GPixelCenterOffset) / (FLOAT)GSceneRenderTargets.GetBufferSizeY(),
			(View.RenderTargetX + View.RenderTargetSizeX - GPixelCenterOffset) / (FLOAT)GSceneRenderTargets.GetBufferSizeX(),
			(View.RenderTargetY + View.RenderTargetSizeY - GPixelCenterOffset) / (FLOAT)GSceneRenderTargets.GetBufferSizeY());
		SetPixelShaderValue( GetPixelShader(), RenderTargetClampParameter, RenderTargetClampValues );

#if USE_XeD3D_RHI
		// When the scene color texture is not Raw, we need to scale it by SCENE_COLOR_BIAS_FACTOR_EXP to re-normalize the range:
		FLOAT SceneColorScale = GSceneRenderTargets.bSceneColorTextureIsRaw ? 1.0f : appPow(2.0f,GSceneRenderTargets.GetSceneColorSurface().XeSurfaceInfo.GetColorExpBias());
		FVector4 ScaleAndBias( 3.0f * SceneColorScale, -2.0f, 0.0f, 0.0f );
		SetPixelShaderValue( GetPixelShader(), MotionBlurMaskScaleParameter, ScaleAndBias);
#endif
	}

	FSceneTextureShaderParameters SceneTextureParameters;
	FShaderResourceParameter VelocityBuffer;
	FShaderParameter ScreenToWorldParameter;
	FShaderParameter PrevViewProjParameter;
	FShaderParameter StaticVelocityParameters;	// = { 0.5f, -0.5f, 16.0f/1280.0f, 16.0f/720.0f };
	FShaderParameter DynamicVelocityParameters;	// = { 2.0f*16.0f/1280.0f, -2.0f*16.0f/720.0f, -16.0f/1280.0f, 16.0f/720.0f };
	FShaderParameter RenderTargetClampParameter; //!< Viewport region in render target = { MinUV.x, MinUV.y, MaxUV.x, MaxUV.y }
	FShaderParameter MotionBlurMaskScaleParameter;	// XY used for ScaleAndBias: { 3*SCENE_COLOR_BIAS_FACTOR, -2, 0, 0 }

protected:
	/** Default constructor. */
	FMotionBlurPixelShader() {}

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
	}
};

//=============================================================================
/** Encapsulates the Motion Blur post-process pixel shader for dynamic velocities. */
class FMotionBlurPixelShaderDynamicVelocitiesOnly : public FMotionBlurPixelShader
{
	DECLARE_SHADER_TYPE(FMotionBlurPixelShaderDynamicVelocitiesOnly,Global);
public:
	/** Initialization constructor. */
	FMotionBlurPixelShaderDynamicVelocitiesOnly(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FMotionBlurPixelShader(Initializer)
	{
	}

private:
	/** Default constructor. */
	FMotionBlurPixelShaderDynamicVelocitiesOnly() {}
};

IMPLEMENT_SHADER_TYPE(,FMotionBlurPixelShader,TEXT("MotionBlurShader"),TEXT("MainPixelShader"),SF_Pixel,VER_BACKGROUNDVELOCITYVALUE,0);
IMPLEMENT_SHADER_TYPE(,FMotionBlurPixelShaderDynamicVelocitiesOnly,TEXT("MotionBlurShader"),TEXT("MainPixelShaderDynamicVelocitiesOnly"),SF_Pixel,VER_BACKGROUNDVELOCITYVALUE,0);


//=============================================================================
/**	FMotionBlurProcessSceneProxy */

class FMotionBlurProcessSceneProxy : public FPostProcessSceneProxy
{
public:
	/** 
	 * Initialization constructor. 
	 * @param InEffect - Motion blur effect to mirror in this proxy
	 */
	FMotionBlurProcessSceneProxy(const UMotionBlurEffect* InEffect,const FPostProcessSettings* WorldSettings)
		:	FPostProcessSceneProxy(InEffect)
		,	MaxVelocity(WorldSettings ? WorldSettings->MotionBlur_MaxVelocity : InEffect->MaxVelocity)
		,	MotionBlurAmount(WorldSettings ? WorldSettings->MotionBlur_Amount : InEffect->MotionBlurAmount)
		,	bFullMotionBlur(WorldSettings ? WorldSettings->MotionBlur_FullMotionBlur : InEffect->FullMotionBlur)
		,	CameraRotationThreshold(WorldSettings ? WorldSettings->MotionBlur_CameraRotationThreshold : InEffect->CameraRotationThreshold)
		,	CameraTranslationThreshold(WorldSettings ? WorldSettings->MotionBlur_CameraTranslationThreshold : InEffect->CameraTranslationThreshold)
	{
		extern INT GMotionBlurFullMotionBlur;
		bFullMotionBlur = GMotionBlurFullMotionBlur < 0 ? bFullMotionBlur : (GMotionBlurFullMotionBlur > 0);
	}

	/**
	 * Render the post process effect
	 * Called by the rendering thread during scene rendering
	 * @param InDepthPriorityGroup - scene DPG currently being rendered
	 * @param View - current view
	 * @param CanvasTransform - same canvas transform used to render the scene
	 * @return TRUE if anything was rendered
	 */
	UBOOL Render(const FScene* Scene, UINT InDepthPriorityGroup,FViewInfo& View, const FMatrix& CanvasTransform)
	{
		SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("MotionBlur"));

		const FSceneViewState* ViewState = (FSceneViewState*)View.State;

		// Check to see if we should render motion blur this frame.
		if ( !ViewState 
			|| !View.bRequiresVelocities 
			|| !GSystemSettings.RenderThreadSettings.bAllowMotionBlur )
		{
			return FALSE;
		}

		/** Vertex declaration for the light function fullscreen 2D quad. */
		extern TGlobalResource<FFilterVertexDeclaration> GFilterVertexDeclaration;

		const UINT SizeX = GSceneRenderTargets.GetBufferSizeX();
		const UINT SizeY = GSceneRenderTargets.GetBufferSizeY();

		// No depth tests, no backface culling.
		RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
		RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
		RHISetBlendState(TStaticBlendState<>::GetRHI());

		// The motion blur shader can operate in any DPG and can come before or after the uber post-processing shader.  This
		// means it must be able to operate on HDR-to-HDR (64-bit) or LDR-to-LDR (32-bit) images.
		//
		// Additionally if the motion blur shader is the final shader in the post-processing chain and it is operating in
		// post gamma-corrected space (LDR/32-bit) then it must render the output to the view's render target.

		UINT TargetSizeX = SizeX;
		UINT TargetSizeY = SizeY;

		if (View.bUseLDRSceneColor) // Using 32-bit (LDR) surface
		{
			if (FinalEffectInGroup) // Final effect in chain, so render to the view's output render target
			{
#if XBOX
				//@hack to fix motion blur sampling from outside of view bounds for the first view in splitscreen
				// only do this for the first view and if there are multiple views
				if( View.Family && 
					View.Family->Views.Num() > 1 &&
					View.RenderTargetY == View.Family->Views(0)->RenderTargetY &&
					(View.RenderTargetY + View.RenderTargetSizeY) <= View.Family->Views(1)->RenderTargetY )
				{
					const INT MOTIONBLUR_BORDER_SIZE = 10;
					// clear a border on the bottom portion of the viewport
					RHISetViewport(
						View.RenderTargetX,
						View.RenderTargetY + View.RenderTargetSizeY,
						0.0f,
						View.RenderTargetX + View.RenderTargetSizeX,
						Min<INT>(View.RenderTargetY + View.RenderTargetSizeY + MOTIONBLUR_BORDER_SIZE,GSceneRenderTargets.GetBufferSizeX()),
						1.0f
						);
					// clear to black
					RHIClear(TRUE,FLinearColor::Black,FALSE,0.0f,FALSE,0);
					// resolve the black border to texture
					FResolveParams ResolveParams;
					ResolveParams.X1 = View.RenderTargetX;
					ResolveParams.Y1 = View.RenderTargetY + View.RenderTargetSizeY;
					ResolveParams.X2 = View.RenderTargetX + View.RenderTargetSizeX;
					ResolveParams.Y2 = Min<INT>(View.RenderTargetY + View.RenderTargetSizeY + MOTIONBLUR_BORDER_SIZE,GSceneRenderTargets.GetBufferSizeX());					
					GSceneRenderTargets.FinishRenderingSceneColorLDR(TRUE,ResolveParams);
				}
#endif

				RHISetRenderTarget(View.Family->RenderTarget->GetRenderTargetSurface(), FSurfaceRHIRef()); // Render to the final render target
				
				TargetSizeX = View.Family->RenderTarget->GetSizeX();
				TargetSizeY = View.Family->RenderTarget->GetSizeY();
			}
			else
			{
				GSceneRenderTargets.BeginRenderingSceneColorLDR();
			}
		}
		else // Using 64-bit (HDR) surface
		{
			GSceneRenderTargets.BeginRenderingSceneColor(); // Start rendering to the scene color buffer.
		}
			
		// Set the motion blur shaders and parameters.
		TShaderMapRef<FMotionBlurVertexShader> VertexShader( GetGlobalShaderMap() );
		TShaderMapRef<FMotionBlurPixelShader> PixelShader( GetGlobalShaderMap() );
		TShaderMapRef<FMotionBlurPixelShaderDynamicVelocitiesOnly> PixelShaderDynamicVelocitiesOnly( GetGlobalShaderMap() );
		static FGlobalBoundShaderState BoundShaderState, BoundShaderStateDynamicVelocitiesOnly;

		if ( bFullMotionBlur )
		{
			SetGlobalBoundShaderState( BoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FFilterVertex));
			PixelShader->SetParameters( View, MotionBlurAmount, MaxVelocity );
		}
		else
		{
			SetGlobalBoundShaderState( BoundShaderStateDynamicVelocitiesOnly, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShaderDynamicVelocitiesOnly, sizeof(FFilterVertex));
			PixelShaderDynamicVelocitiesOnly->SetParameters( View, MotionBlurAmount, MaxVelocity );
		}

		// disable alpha writes in order to preserve scene depth values on PC
		RHISetColorWriteMask( CW_RED|CW_GREEN|CW_BLUE);

		if( View.bUseLDRSceneColor &&
			(View.X > 0 || View.Y > 0 || View.SizeX < TargetSizeX || View.SizeY < TargetSizeY) )
		{
			// Draw the quad.
			DrawDenormalizedQuad(
				View.X, View.Y, View.SizeX, View.SizeY,
				View.RenderTargetX, View.RenderTargetY, View.RenderTargetSizeX, View.RenderTargetSizeY,
				TargetSizeX, TargetSizeY,
				SizeX, SizeY);

		}
		else
		{
			// Draw the quad.
			DrawDenormalizedQuad(
				View.RenderTargetX, View.RenderTargetY, View.RenderTargetSizeX, View.RenderTargetSizeY,
				View.RenderTargetX, View.RenderTargetY, View.RenderTargetSizeX, View.RenderTargetSizeY,
				TargetSizeX, TargetSizeY, SizeX, SizeY );

		}

		// restore color write mask
		RHISetColorWriteMask( CW_RED|CW_GREEN|CW_BLUE|CW_ALPHA);

		// Resolve the scene color buffer.  Note that nothing needs to be done if we are writing directly to the view's render target
		if (View.bUseLDRSceneColor)
		{
			if (!FinalEffectInGroup)
			{
				GSceneRenderTargets.FinishRenderingSceneColorLDR();
			}
		}
		else
		{
			GSceneRenderTargets.FinishRenderingSceneColor();
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
#if XBOX
		// if uberpostprocess is running, the motion blur pass will be skipped
		// and motion blur will be combined with DOF and bloom
		if(GSystemSettings.bAllowDepthOfField || GSystemSettings.bAllowBloom)
		{
			return FALSE;
		}
#endif
		MotionBlurParameters.VelocityScale			= MotionBlurAmount;
		MotionBlurParameters.MaxVelocity			= MaxVelocity;
		MotionBlurParameters.bFullMotionBlur		= bFullMotionBlur;
		MotionBlurParameters.RotationThreshold		= CameraRotationThreshold;
		MotionBlurParameters.TranslationThreshold	= CameraTranslationThreshold;
		return TRUE;
	}

	/**
	 * Tells FSceneRenderer whether to store the previous frame's transforms.
	 */
	virtual UBOOL RequiresPreviousTransforms(const FViewInfo& View) const
	{
		return TRUE;
	}

private:
	/** Mirrored properties. See MotionBlurEffect.uc for descriptions */
    FLOAT MaxVelocity;
    FLOAT MotionBlurAmount;
	UBOOL bFullMotionBlur;
	FLOAT CameraRotationThreshold;
	FLOAT CameraTranslationThreshold;
};

/**
 * Creates a proxy to represent the render info for a post process effect
 * @param WorldSettings - The world's post process settings for the view.
 * @return The proxy object.
 */
FPostProcessSceneProxy* UMotionBlurEffect::CreateSceneProxy(const FPostProcessSettings* WorldSettings)
{
	// Turn off motion blur for tiled screenshots; the way motion blur is handled is
	// incompatible with tiled rendering and a single view.
	// Probably we could workaround with multiple views if it is necessery.
	extern UBOOL GIsTiledScreenshot;
	if ( GIsTiledScreenshot )
	{
		return NULL;
	}

	extern INT GGameScreenshotCounter;
	if ( GGameScreenshotCounter != 0 )
	{
		return NULL;
	}

	if(!WorldSettings || WorldSettings->bEnableMotionBlur)
	{
		return new FMotionBlurProcessSceneProxy(this,WorldSettings);
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
UBOOL UMotionBlurEffect::IsShown(const FSceneView* View) const
{
#if XBOX
	// motion blur is done in uberpostprocesing on XBOX
	// so only render it here if DOF and Bloom will not be rendered
	return GSystemSettings.bAllowMotionBlur && Super::IsShown( View ) && !GSystemSettings.bAllowDepthOfField && !GSystemSettings.bAllowBloom;
#else

	// Disable motion blur when not using realtime update, since it requires continuous frames.
	if( !( View->Family->ShowFlags & SHOW_MotionBlur ) 
		|| !View->Family->bRealtimeUpdate )
	{
		return FALSE;
	}

	// Disable motion blur in the editor viewports, even if "G" mode and realtime updates are on.
	if( GIsEditor && !GIsGame )
	{
		return FALSE;
	}

	if ( IsInGameThread() )
	{
		return GSystemSettings.bAllowMotionBlur && Super::IsShown( View );
	}
	else
	{
		return GSystemSettings.RenderThreadSettings.bAllowMotionBlur && Super::IsShown( View );
	}
#endif
}

/**
 * Creates a proxy to represent the render info for a post process effect
 * @param WorldSettings - The world's post process settings for the view.
 * @return The proxy object.
 */
FPostProcessSceneProxy* UDOFBloomMotionBlurEffect::CreateSceneProxy(const FPostProcessSettings* WorldSettings)
{
	// UDOFBloomMotionBlurEffect is only used as a base class for uberpostprocess and doesn't render itself
	return NULL;
}

/**
 * @param View - current view
 * @return TRUE if the effect should be rendered
 */
UBOOL UDOFBloomMotionBlurEffect::IsShown(const FSceneView* View) const
{
	return (GSystemSettings.bAllowBloom || GSystemSettings.bAllowDepthOfField) && Super::IsShown( View );
}

