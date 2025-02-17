/*=============================================================================
	SceneRenderTargets.cpp: Scene render target implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"

/*-----------------------------------------------------------------------------
FSceneRenderTargets
-----------------------------------------------------------------------------*/

/** The global render targets used for scene rendering. */
TGlobalResource<FSceneRenderTargets> GSceneRenderTargets;

/** 
 * Dumps information about render target memory usage
 * Must be called on the rendering thread or while the rendering thread is blocked
 * Currently only implemented for xbox
 */
void FSceneRenderTargets::DumpMemoryUsage(FOutputDevice& Ar) const
{
#if XBOX && TRACK_GPU_RESOURCES
	if (IsInitialized())
	{
		INT TotalVirtualSize = 0;
		INT TotalPhysicalSize = 0;
		for( INT RTIdx=0; RTIdx < MAX_SCENE_RENDERTARGETS; RTIdx++ )
		{
			// Ignore unused textures
			if (IsValidRef(RenderTargets[RTIdx].Texture))
			{
				INT OverlappingTextureIndex = -1;
				// Search for an already reported texture sharing the same memory
				for( INT RTIdx2=0; RTIdx2 < RTIdx; RTIdx2++ )
				{
					if (RTIdx != RTIdx2 
						// Check if texture resources are equal
						&& (RenderTargets[RTIdx].Texture == RenderTargets[RTIdx2].Texture 
						// Check if texture resources are different but they share memory through the RHICreateSharedTexture method
						|| IsValidRef(RenderTargets[RTIdx].Texture) && IsValidRef(RenderTargets[RTIdx2].Texture)
						&& RenderTargets[RTIdx].Texture->GetSharedTexture() && RenderTargets[RTIdx2].Texture->GetSharedTexture()
						&& RenderTargets[RTIdx].Texture->GetSharedTexture()->GetSharedMemory()->BaseAddress == RenderTargets[RTIdx2].Texture->GetSharedTexture()->GetSharedMemory()->BaseAddress))
					{
						OverlappingTextureIndex = RTIdx2;
						break;
					}
				}
				
				if (OverlappingTextureIndex >= 0)
				{
					Ar.Logf(TEXT("	RenderTarget %2i %s sharing memory with %s"), 
						RTIdx, 
						*GetRenderTargetName((ESceneRenderTargetTypes)RTIdx), 
						*GetRenderTargetName((ESceneRenderTargetTypes)OverlappingTextureIndex));
				}
				else
				{
					const INT VirtualResourceSize = RenderTargets[RTIdx].Texture->VirtualSize;
					const INT PhysicalResourceSize = RenderTargets[RTIdx].Texture->PhysicalSize;

					TotalVirtualSize += VirtualResourceSize;
					TotalPhysicalSize += PhysicalResourceSize;

					// for RenderTargets that exist print out how many EDRAM tiles they utilize
					INT EDRAMUsage = 0;
					if( IsValidRef(RenderTargets[RTIdx].Surface) == TRUE )
					{
						EDRAMUsage = RenderTargets[RTIdx].Surface.XeSurfaceInfo.GetSize();
					}

					Ar.Logf(TEXT("	RenderTarget %2i %s using %5.2fMb Virtual memory, %5.2fMb Physical memory, %3d EDRAM memory tiles"), 
						RTIdx, 
						*GetRenderTargetName((ESceneRenderTargetTypes)RTIdx), 
						VirtualResourceSize / (1024.0f * 1024.0f), 
						PhysicalResourceSize / (1024.0f * 1024.0f),
						EDRAMUsage
						);
				}
			}
		}

		Ar.Logf(TEXT("Total rendertarget memory: %.2fMb Virtual, %.2fMb Physical"),  
			TotalVirtualSize / (1024.0f * 1024.0f),
			TotalPhysicalSize / (1024.0f * 1024.0f));
	}
#endif
}

void FSceneRenderTargets::Allocate(UINT MinSizeX,UINT MinSizeY)
{
#if CONSOLE
	// force to always use the global screen sizes to avoid reallocating the scene buffers
	MinSizeX = GScreenWidth;
	MinSizeY = GScreenHeight;
#endif

	if(BufferSizeX < MinSizeX || BufferSizeY < MinSizeY)
	{
		// Reinitialize the render targets for the given size.
		SetBufferSize( Max(BufferSizeX,MinSizeX), Max(BufferSizeY,MinSizeY) );

		UpdateRHI();
	}
}

void FSceneRenderTargets::BeginRenderingFilter()
{
	// Set the filter color surface as the render target
	RHISetRenderTarget( GetFilterColorSurface(), FSurfaceRHIRef());
	RHISetViewport(0,0,0.0f,GSceneRenderTargets.GetFilterBufferSizeX(),GSceneRenderTargets.GetFilterBufferSizeY(),1.0f);
}

void FSceneRenderTargets::FinishRenderingFilter()
{
	// Resolve the filter color surface 
	RHICopyToResolveTarget(GetFilterColorSurface(), FALSE, FResolveParams());
}

/**
* Sets the scene color target and restores its contents if necessary
* @param bRestoreContents - if TRUE then copies contents of SceneColorTexture to the SceneColorSurface
*/
void FSceneRenderTargets::BeginRenderingSceneColor(UBOOL bRestoreContents)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingSceneColor"));

	if(bRestoreContents)
	{
		// Initialize the scene color surface to its previously resolved contents.
		RHICopyFromResolveTarget(GetSceneColorSurface());
	}

	// Set the scene color surface as the render target, and the scene depth surface as the depth-stencil target.
	RHISetRenderTarget( GetSceneColorSurface(), GetSceneDepthSurface());
} 

/**
* Called when finished rendering to the scene color surface
* @param bKeepChanges - if TRUE then the SceneColorSurface is resolved to the SceneColorTexture
*/
void FSceneRenderTargets::FinishRenderingSceneColor(UBOOL bKeepChanges, const FResolveParams& ResolveParams)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingSceneColor"));

	if(bKeepChanges)
	{
		// Resolve the scene color surface to the scene color texture.
		RHICopyToResolveTarget(GetSceneColorSurface(), TRUE, ResolveParams);
		bSceneColorTextureIsRaw= FALSE;
	}
}

/**
* Sets the LDR version of the scene color target.
*/
void FSceneRenderTargets::BeginRenderingSceneColorLDR()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingSceneColorLDR"));

	// Set the light attenuation surface as the render target, and the scene depth buffer as the depth-stencil surface.
	RHISetRenderTarget(GetSceneColorLDRSurface(),GetSceneDepthSurface());
}

/**
* Called when finished rendering to the LDR version of the scene color surface.
* @param bKeepChanges - if TRUE then the SceneColorSurface is resolved to the LDR SceneColorTexture
* @param ResolveParams - optional resolve params
*/
void FSceneRenderTargets::FinishRenderingSceneColorLDR(UBOOL bKeepChanges,const FResolveParams& ResolveParams)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingSceneColorLDR"));

	if(bKeepChanges)
	{
		// Resolve the scene color surface to the scene color texture.
		RHICopyToResolveTarget(GetSceneColorLDRSurface(), TRUE, ResolveParams);
	}
}


/**
* Saves a previously rendered scene color target
*/
void FSceneRenderTargets::ResolveSceneColor(const FResolveParams& ResolveParams)
{
    SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("ResolveSceneColor"));
	RHICopyToResolveTarget(GetSceneColorSurface(), TRUE, ResolveParams);
	bSceneColorTextureIsRaw= FALSE;
}

/**
* Sets the raw version of the scene color target.
*/
void FSceneRenderTargets::BeginRenderingSceneColorRaw()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingSceneColorRaw"));

	// Use the raw version of the scene color as the render target, and use the standard scene depth buffer as the depth-stencil surface.
	RHISetRenderTarget( GetSceneColorRawSurface(), GetSceneDepthSurface());
}

/**
 * Saves a previously rendered scene color surface in the raw bit format.
 */
void FSceneRenderTargets::SaveSceneColorRaw(UBOOL bConvertToFixedPoint, const FResolveParams& ResolveParams)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("SaveSceneColorRaw"));

#if XBOX
	if (bConvertToFixedPoint)
	{
		RHICopyToResolveTarget(RenderTargets[SceneColorFixedPoint].Surface, TRUE, ResolveParams);
	}
	else
#endif
	{
		RHICopyToResolveTarget(GetSceneColorRawSurface(), TRUE, ResolveParams);
	}
	bSceneColorTextureIsRaw= TRUE;
}

/**
 * Restores a previously saved raw-scene color surface.
 */
void FSceneRenderTargets::RestoreSceneColorRaw()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("RestoreSceneColorRaw"));

	// Initialize the scene color surface to its previously resolved contents.
	RHICopyFromResolveTargetFast(GetSceneColorRawSurface());

	// Set the scene color surface as the render target, and the scene depth surface as the depth-stencil target.
	RHISetRenderTarget( GetSceneColorSurface(), GetSceneDepthSurface());
}

/**
 * Restores a rectangle from a previously saved raw-scene color surface.
 */
void FSceneRenderTargets::RestoreSceneColorRectRaw(FLOAT X1,FLOAT Y1,FLOAT X2,FLOAT Y2)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("RestoreSceneColorRectRaw"));

	// Initialize the scene color surface to its previously resolved contents.
	RHICopyFromResolveTargetRectFast(GetSceneColorRawSurface(), X1, Y1, X2, Y2);

	// Set the scene color surface as the render target, and the scene depth surface as the depth-stencil target.
	RHISetRenderTarget( GetSceneColorSurface(), GetSceneDepthSurface());
}

/**
 * Sets the HDR scene color scratch pad render target, which is used during the base pass for SM2.
 */
void FSceneRenderTargets::BeginRenderingSceneColorScratch()
{
	RHISetRenderTarget( GetSceneColorScratchSurface(), GetSceneDepthSurface() );
}

void FSceneRenderTargets::BeginRenderingPrePass()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingPrePass"));

	// Set the scene depth surface and a DUMMY buffer as color buffer
	// (as long as it's the same dimension as the depth buffer),
	RHISetRenderTarget( GetLightAttenuationSurface(), GetSceneDepthSurface());

	// Disable color writes since we only want z depths
	RHISetColorWriteEnable( FALSE);
}

void FSceneRenderTargets::FinishRenderingPrePass()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingPrePass"));

	// Re-enable color writes
	RHISetColorWriteEnable( TRUE);
}

void FSceneRenderTargets::BeginRenderingShadowVolumes()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingShadowVolumes"));

	// Make sure we are writing to the same depth stencil buffer as
	// BeginRenderingSceneColor and BeginRenderingLightAttenuation.
	//
	// Note that we're not actually writing anything to the color
	// buffer here. It could be anything with the same dimension.
	RHISetRenderTarget(GetLightAttenuationSurface(),GetSceneDepthSurface());
	RHISetColorWriteEnable(FALSE);
}

void FSceneRenderTargets::FinishRenderingShadowVolumes()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingShadowVolumes"));
	RHISetColorWriteEnable(TRUE);
}

void FSceneRenderTargets::BeginRenderingShadowDepth()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingShadowDepth"));

	if(GSupportsHardwarePCF || GSupportsFetch4)
	{
		// set the shadow z surface as the depth buffer
		// have to bind a color target that is the same size as the depth texture on platforms that support Hardware PCF and Fetch4
		RHISetRenderTarget(GetShadowDepthColorSurface(), GetShadowDepthZSurface());   
		// disable color writes since we only want z depths
		RHISetColorWriteEnable(FALSE);
	}
	else if( GSupportsDepthTextures)
	{
		// set the shadow z surface as the depth buffer
		RHISetRenderTarget(FSurfaceRHIRef(), GetShadowDepthZSurface());   
		// disable color writes since we only want z depths
		RHISetColorWriteEnable(FALSE);
	}
	else
	{
		// Set the shadow color surface as the render target, and the shadow z surface as the depth buffer
		RHISetRenderTarget(GetShadowDepthColorSurface(), GetShadowDepthZSurface());
	}
}

/**
* Called when finished rendering to the subject shadow depths so the surface can be copied to texture
* @param ResolveParams - optional resolve params
*/
void FSceneRenderTargets::FinishRenderingShadowDepth(const FResolveParams& ResolveParams)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingShadowDepth"));

	if( GSupportsDepthTextures || GSupportsHardwarePCF || GSupportsFetch4)
	{
		// Resolve the shadow depth z surface.
		RHICopyToResolveTarget(GetShadowDepthZSurface(), FALSE, ResolveParams);
		// restore color writes
		RHISetColorWriteEnable(TRUE);
	}
	else
	{
		// Resolve the shadow depth color surface.
		RHICopyToResolveTarget(GetShadowDepthColorSurface(), FALSE, ResolveParams);
	}
}

#if SUPPORTS_VSM
void FSceneRenderTargets::BeginRenderingShadowVariance()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingShadowVariance"));
	// Set the shadow variance surface
	RHISetRenderTarget(GetShadowVarianceSurface(), FSurfaceRHIRef());
}

void FSceneRenderTargets::FinishRenderingShadowVariance(const FResolveParams& ResolveParams)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingShadowVariance"));
	// Resolve the shadow variance surface.
	RHICopyToResolveTarget(GetShadowVarianceSurface(), FALSE,ResolveParams);
}

UINT FSceneRenderTargets::GetShadowVarianceTextureResolution() const
{
	return Clamp(GSystemSettings.RenderThreadSettings.MaxShadowResolution,1,GMaxShadowDepthBufferSize);
}
#endif //#if SUPPORTS_VSM

void FSceneRenderTargets::BeginRenderingLightAttenuation()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingLightAttenuation"));

	// Set the light attenuation surface as the render target, and the scene depth buffer as the depth-stencil surface.
	RHISetRenderTarget(GetLightAttenuationSurface(),GetSceneDepthSurface());
}

void FSceneRenderTargets::FinishRenderingLightAttenuation()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingLightAttenuation"));

	// Resolve the light attenuation surface.
	RHICopyToResolveTarget(GetLightAttenuationSurface(), FALSE, FResolveParams());
}

void FSceneRenderTargets::BeginRenderingAmbientOcclusion(UBOOL bUseDownsizedDepthBuffer)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingAmbientOcclusion"));
	if (bUseDownsizedDepthBuffer)
	{
		RHISetRenderTarget(GetAmbientOcclusionSurface(),GetSmallDepthSurface());
	}
	else
	{
		RHISetRenderTarget(GetAmbientOcclusionSurface(),FSurfaceRHIRef());
	}
}

void FSceneRenderTargets::FinishRenderingAmbientOcclusion(const FResolveParams& ResolveParams)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingAmbientOcclusion"));
	RHICopyToResolveTarget(GetAmbientOcclusionSurface(), FALSE, ResolveParams);
}

void FSceneRenderTargets::BeginRenderingAOHistory(UBOOL bUseDownsizedDepthBuffer)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingAOHistory"));
	if (bUseDownsizedDepthBuffer)
	{
		RHISetRenderTarget(GetAOHistorySurface(),GetSmallDepthSurface());
	}
	else
	{
		RHISetRenderTarget(GetAOHistorySurface(),FSurfaceRHIRef());
	}
}

void FSceneRenderTargets::FinishRenderingAOHistory(const FResolveParams& ResolveParams)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingAOHistory"));
	RHICopyToResolveTarget(GetAOHistorySurface(), FALSE, ResolveParams);
}

void FSceneRenderTargets::BeginRenderingDistortionAccumulation()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingDistortionAccumulation"));

	// use RGBA8 light target for accumulating distortion offsets	
	// R = positive X offset
	// G = positive Y offset
	// B = negative X offset
	// A = negative Y offset

	RHISetRenderTarget(GetLightAttenuationSurface(),GetSceneDepthSurface());
}

void FSceneRenderTargets::FinishRenderingDistortionAccumulation(const FResolveParams& ResolveParams)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingDistortionAccumulation"));

	RHICopyToResolveTarget(GetLightAttenuationSurface(), FALSE, ResolveParams);
}

void FSceneRenderTargets::BeginRenderingDistortionDepth()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingDistortionDepth"));
}

void FSceneRenderTargets::FinishRenderingDistortionDepth()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingDistortionDepth"));
}

/** Starts rendering to the velocity buffer. */
void FSceneRenderTargets::BeginRenderingVelocities()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingVelocities"));

#if XBOX
	// Set the motion blur velocity buffer as the render target, and the small depth surface as the depth-stencil target.
	RHISetRenderTarget( GetVelocitySurface(), GetSmallDepthSurface() );
#else
	RHISetRenderTarget( GetVelocitySurface(), GetSceneDepthSurface() );
#endif
}

/** Stops rendering to the velocity buffer. */
void FSceneRenderTargets::FinishRenderingVelocities()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingVelocities"));

	// Resolve the velocity buffer to a texture, so it can be read later.
	RHICopyToResolveTarget(GetVelocitySurface(), FALSE, FResolveParams());
}

void FSceneRenderTargets::BeginRenderingFogFrontfacesIntegralAccumulation()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingFogFrontfacesIntegralAccumulation"));
	RHISetRenderTarget(GetFogFrontfacesIntegralAccumulationSurface(),FSurfaceRHIRef());
}

void FSceneRenderTargets::FinishRenderingFogFrontfacesIntegralAccumulation()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingFogFrontfacesIntegralAccumulation"));

	RHICopyToResolveTarget(GetFogFrontfacesIntegralAccumulationSurface(), FALSE, FResolveParams());
}

void FSceneRenderTargets::BeginRenderingFogBackfacesIntegralAccumulation()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingFogBackfacesIntegralAccumulation"));
	RHISetRenderTarget(GetFogBackfacesIntegralAccumulationSurface(),FSurfaceRHIRef());
}

void FSceneRenderTargets::FinishRenderingFogBackfacesIntegralAccumulation()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingFogBackfacesIntegralAccumulation"));

	RHICopyToResolveTarget(GetFogBackfacesIntegralAccumulationSurface(), FALSE, FResolveParams());
}

void FSceneRenderTargets::ResolveSceneDepthTexture()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("ResolveSceneDepthTexture"));

	if(GSupportsDepthTextures)
	{
		// Resolve the scene depth surface.
		RHICopyToResolveTarget(GetSceneDepthSurface(), TRUE, FResolveParams());
	}
}

/** Updates the quarter-sized depth buffer with the current contents of the scene depth texture. */
void FSceneRenderTargets::UpdateSmallDepthSurface()
{
	RHISetRenderTarget( NULL, GetSmallDepthSurface() );
	RHIRestoreColorDepth( NULL, GSceneRenderTargets.GetSceneDepthTexture() );
}

void FSceneRenderTargets::BeginRenderingHitProxies()
{
	RHISetRenderTarget(GetHitProxySurface(),GetSceneDepthSurface());
}

void FSceneRenderTargets::FinishRenderingHitProxies()
{
	RHICopyToResolveTarget(GetHitProxySurface(), FALSE, FResolveParams());
}

void FSceneRenderTargets::BeginRenderingFogBuffer()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingFogBuffer"));

	RHISetMRTRenderTarget(GetFogBufferSurface(),1);
}

void FSceneRenderTargets::FinishRenderingFogBuffer(const FResolveParams& ResolveParams)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingFogBuffer"));

	RHISetMRTRenderTarget(NULL,1);
	RHICopyToResolveTarget(GetFogBufferSurface(), FALSE, ResolveParams);
}

UINT Aling(UINT Size, UINT Alignment)
{
	return (Size + Alignment - 1) & ~(Alignment - 1);
}

void FSceneRenderTargets::SetBufferSize( const UINT InBufferSizeX, const UINT InBufferSizeY )
{
	BufferSizeX = InBufferSizeX;
	BufferSizeY = InBufferSizeY;
	FilterDownsampleFactor = 4;
	FilterBufferSizeX = BufferSizeX / FilterDownsampleFactor + 2;
	FilterBufferSizeY = BufferSizeY / FilterDownsampleFactor + 2;

	FogAccumulationDownsampleFactor = 2;
	FogAccumulationBufferSizeX = Max< UINT >( 1, BufferSizeX / FogAccumulationDownsampleFactor );
	FogAccumulationBufferSizeY = Max< UINT >( 1, BufferSizeY / FogAccumulationDownsampleFactor );

	SetAmbientOcclusionDownsampleFactor(AmbientOcclusionDownsampleFactor);
}

void FSceneRenderTargets::InitDynamicRHI()
{
	if(BufferSizeX > 0 && BufferSizeY > 0)
	{
		// Don't use MSAA in the editor, for performance and to avoid dealing with MSAA hit proxy rendering.
		const DWORD MultiSampleFlag = TargetSurfCreate_Multisample; 

		SceneColorBufferFormat = PF_FloatRGB;
		if(GSupportsDepthTextures)
		{
			// Create a texture to store the resolved scene depth, and a render-targetable surface to hold the unresolved scene depth.
			RenderTargets[SceneDepthZ].Texture = RHICreateTexture2D(BufferSizeX,BufferSizeY,PF_DepthStencil,1,TexCreate_ResolveTargetable|TexCreate_DepthStencil,NULL);
			RenderTargets[SceneDepthZ].Surface = RHICreateTargetableSurface(
				BufferSizeX,BufferSizeY,PF_DepthStencil,RenderTargets[SceneDepthZ].Texture,MultiSampleFlag,TEXT("DefaultDepth"));
		}
		else
		{
			// Create a surface to store the unresolved scene depth.
			RenderTargets[SceneDepthZ].Surface = RHICreateTargetableSurface(
				BufferSizeX,BufferSizeY,PF_DepthStencil,FTexture2DRHIRef(),TargetSurfCreate_Dedicated|MultiSampleFlag,TEXT("DefaultDepth"));
			// Allocate an alpha channel in the scene color texture to store the resolved scene depth.
			SceneColorBufferFormat = PF_FloatRGBA; 
		}

#if XBOX && !USE_NULL_RHI
		// Create a quarter-sized version of the scene depth.
		RenderTargets[SmallDepthZ].Surface = RHICreateTargetableSurface(BufferSizeX / SmallColorDepthDownsampleFactor,BufferSizeY / SmallColorDepthDownsampleFactor,PF_DepthStencil,FTexture2DRHIRef(),TargetSurfCreate_Dedicated,TEXT("SmallDepth"));
		bDownsizedDepthSupported = TRUE;
#endif

		if (!CanBlendWithFPRenderTarget(GRHIShaderPlatform))
		{
			// In SM2, create a scratch pad floating point render target, 
			// which will be used to render scene color and depth during the same pass (the World DPG's Base Pass) without requiring MRT support.
			// @todo: use MRT where supported to render scene color and depth without needing the scratch render target.
			RenderTargets[SceneColorScratch].Texture = RHICreateTexture2D(BufferSizeX,BufferSizeY,PF_FloatRGBA,1,TexCreate_ResolveTargetable,NULL);
			RenderTargets[SceneColorScratch].Surface = RHICreateTargetableSurface(
				BufferSizeX,BufferSizeY,PF_FloatRGBA,RenderTargets[SceneColorScratch].Texture,MultiSampleFlag,TEXT("SceneColorScratch"));

			// Use LDR scene color in SM2.  The alpha channel will be used to store a luminance scale for post process effects.
			SceneColorBufferFormat = PF_A8R8G8B8;
		}

#if XBOX && !USE_NULL_RHI
		// Create a texture to store the resolved scene colors, and a dedicated render-targetable surface to hold the unresolved scene colors.
		extern SIZE_T XeCalculateTextureBytes(DWORD SizeX,DWORD SizeY,DWORD SizeZ,BYTE Format);
		const SIZE_T ExpandedSceneColorSize = XeCalculateTextureBytes(BufferSizeX,BufferSizeY,1,SceneColorBufferFormat);
		const SIZE_T RawSceneColorSize = XeCalculateTextureBytes(BufferSizeX,BufferSizeY,1,PF_A2B10G10R10);
		const SIZE_T SharedSceneColorSize = Max(ExpandedSceneColorSize,RawSceneColorSize);
		FSharedMemoryResourceRHIRef MemoryBuffer = RHICreateSharedMemory(SharedSceneColorSize);

		RenderTargets[SceneColor].Texture = RHICreateSharedTexture2D(BufferSizeX,BufferSizeY,SceneColorBufferFormat,1,MemoryBuffer,TexCreate_ResolveTargetable);
		RenderTargets[SceneColor].Surface = RHICreateTargetableSurface(
			BufferSizeX,BufferSizeY,SceneColorBufferFormat,RenderTargets[SceneColor].Texture,TargetSurfCreate_Dedicated|MultiSampleFlag,TEXT("DefaultColor"));

		// Create a version of the scene color textures that represent the raw bits (i.e. that can do the resolves without any format conversion)
		RenderTargets[SceneColorRaw].Texture = RHICreateSharedTexture2D(BufferSizeX,BufferSizeY,PF_A2B10G10R10,1,MemoryBuffer,TexCreate_ResolveTargetable);
		RenderTargets[SceneColorRaw].Surface = RHICreateTargetableSurface(
			BufferSizeX,BufferSizeY,PF_A2B10G10R10,RenderTargets[SceneColorRaw].Texture,TargetSurfCreate_Dedicated|MultiSampleFlag,TEXT("DefaultColorRaw"));

		// Create a version of the scene color textures that is converted to fixed point.
		// This version has banding in the darks so it is only used when the banding will not be noticeable.
		RenderTargets[SceneColorFixedPoint].Texture = RenderTargets[SceneColorRaw].Texture;
		RenderTargets[SceneColorFixedPoint].Surface = RHICreateTargetableSurface(
			BufferSizeX,BufferSizeY,PF_FloatRGB,RenderTargets[SceneColorRaw].Texture,TargetSurfCreate_Dedicated|MultiSampleFlag,TEXT("DefaultColorFixedPoint"));
#else
		// Create a texture to store the resolved scene colors, and a dedicated render-targetable surface to hold the unresolved scene colors.
		RenderTargets[SceneColor].Texture = RHICreateTexture2D(BufferSizeX,BufferSizeY,SceneColorBufferFormat,1,TexCreate_ResolveTargetable,NULL);
		RenderTargets[SceneColor].Surface = RHICreateTargetableSurface(
			BufferSizeX,BufferSizeY,SceneColorBufferFormat,RenderTargets[SceneColor].Texture,TargetSurfCreate_Dedicated|MultiSampleFlag,TEXT("DefaultColor"));
		RenderTargets[SceneColorRaw].Texture = RenderTargets[SceneColor].Texture;
		RenderTargets[SceneColorRaw].Surface = RenderTargets[SceneColor].Surface;

		RenderTargets[SceneColorFixedPoint].Texture = NULL;
		RenderTargets[SceneColorFixedPoint].Surface = NULL;
#endif

		// make sure our flag noting the scene color texture is use is setup
		bSceneColorTextureIsRaw= FALSE;

#if SUPPORTS_VSM
		// We need a 2-channel format to support VSM, and a dedicated surface to work with the filtering
		const EPixelFormat ShadowVarianceFmt=PF_G16R16F_FILTER;
		RenderTargets[ShadowVariance].Texture = RHICreateTexture2D(GetShadowVarianceTextureResolution(),GetShadowVarianceTextureResolution(), ShadowVarianceFmt,1,TexCreate_ResolveTargetable,NULL);
		RenderTargets[ShadowVariance].Surface = RHICreateTargetableSurface(
			GetShadowVarianceTextureResolution(),
			GetShadowVarianceTextureResolution(), 
			ShadowVarianceFmt,RenderTargets[ShadowVariance].Texture,
			TargetSurfCreate_Dedicated,TEXT("ShadowVariance")
			);
#endif //#if SUPPORTS_VSM

		const EPixelFormat LightAttenuationBufferFormat = PF_A8R8G8B8;
#if XBOX && !USE_NULL_RHI
		const SIZE_T LightAttenuationSize = XeCalculateTextureBytes(BufferSizeX,BufferSizeY,1,LightAttenuationBufferFormat);
		// Create a shared memory buffer for light attenuation so we can share it with other render target textures
		// Since light attenuation uses have a very short lifetime
		LightAttenuationMemoryBuffer = RHICreateSharedMemory(LightAttenuationSize);

		RenderTargets[LightAttenuation].Texture = RHICreateSharedTexture2D(BufferSizeX,BufferSizeY,LightAttenuationBufferFormat,1,LightAttenuationMemoryBuffer,TexCreate_ResolveTargetable);
		RenderTargets[LightAttenuation].Surface = RHICreateTargetableSurface(
			BufferSizeX,BufferSizeY,LightAttenuationBufferFormat,RenderTargets[LightAttenuation].Texture,MultiSampleFlag,TEXT("LightAttenuation"));
#else
		// Create a texture to store the resolved light attenuation values, and a render-targetable surface to hold the unresolved light attenuation values.
		RenderTargets[LightAttenuation].Texture = RHICreateTexture2D(BufferSizeX,BufferSizeY,LightAttenuationBufferFormat,1,TexCreate_ResolveTargetable,NULL);
		RenderTargets[LightAttenuation].Surface = RHICreateTargetableSurface(
			BufferSizeX,BufferSizeY,LightAttenuationBufferFormat,RenderTargets[LightAttenuation].Texture,MultiSampleFlag,TEXT("LightAttenuation"));
#endif

		// Create the filter targetable texture and surface.
#if XBOX && !USE_NULL_RHI
		// use 16 bit float targets along with expanded float texture format for better precision
		// Share memory with the light attenuation buffer since uses of the filter texture don't overlap with uses of the light attenuation buffer
		RenderTargets[FilterColor].Texture = RHICreateSharedTexture2D(FilterBufferSizeX,FilterBufferSizeY,PF_FloatRGBA,1,LightAttenuationMemoryBuffer,TexCreate_ResolveTargetable);
		RenderTargets[FilterColor].Surface = RHICreateTargetableSurface(
			FilterBufferSizeX,FilterBufferSizeY,PF_FloatRGBA,RenderTargets[FilterColor].Texture,TargetSurfCreate_Dedicated|MultiSampleFlag,TEXT("FilterColor"));
#else
		RenderTargets[FilterColor].Texture = RHICreateTexture2D(FilterBufferSizeX,FilterBufferSizeY,PF_A16B16G16R16,1,TexCreate_ResolveTargetable,NULL);
		RenderTargets[FilterColor].Surface = RHICreateTargetableSurface(
			FilterBufferSizeX,FilterBufferSizeY,PF_A16B16G16R16,RenderTargets[FilterColor].Texture,TargetSurfCreate_Dedicated|MultiSampleFlag,TEXT("FilterColor"));
#endif

		AllocateAmbientOcclusionBuffers();

		// Reuse the light attenuation texture/surface for hit proxies.
		RenderTargets[HitProxy] = RenderTargets[LightAttenuation];

		// Set up velocity buffer / quarter size scene color shared texture
		const DWORD VelocityBufferFormat	= PF_G16R16;
#if XBOX && !USE_NULL_RHI
		VelocityBufferSizeX	= BufferSizeX / 2;
		VelocityBufferSizeY	= BufferSizeY / 2;
		const DWORD DownsampledBufferFormat = PF_A2B10G10R10;
		extern SIZE_T XeCalculateTextureBytes(DWORD SizeX,DWORD SizeY,DWORD SizeZ,BYTE Format);
		SIZE_T VelocityBufferSize	= GSystemSettings.RenderThreadSettings.bAllowMotionBlur ? XeCalculateTextureBytes(VelocityBufferSizeX,VelocityBufferSizeY,1,VelocityBufferFormat) : 0;
		SIZE_T QuarterBufferSize	= XeCalculateTextureBytes(BufferSizeX / SmallColorDepthDownsampleFactor,BufferSizeY / SmallColorDepthDownsampleFactor,1,DownsampledBufferFormat);
		SIZE_T SharedVelocitySize	= Max(VelocityBufferSize,QuarterBufferSize);

		// Overlay the velocity buffer and the quarter screen resolve buffer (used by the low quality postprocess filter)
		FSharedMemoryResourceRHIRef VelocityMemoryBuffer = RHICreateSharedMemory(SharedVelocitySize);

		// Create a shared texture to store a 2x downsampled render target
		extern IDirect3DSurface9* GD3DBackBufferResolveSource;
		RenderTargets[QuarterSizeSceneColor].Texture = RHICreateSharedTexture2D(BufferSizeX / SmallColorDepthDownsampleFactor,BufferSizeY / SmallColorDepthDownsampleFactor,DownsampledBufferFormat,1,VelocityMemoryBuffer,TexCreate_ResolveTargetable);
		RenderTargets[QuarterSizeSceneColor].Surface = GD3DBackBufferResolveSource;

		// Create a small render target overlapping with the velocity buffer
		// This will be used when rendering height fog during ambient occlusion downsampling
		// 8 bits per channel for storing the fog factors of 4 neighboring pixels
		DWORD FogBufferFormat = PF_A8R8G8B8;
		RenderTargets[FogBuffer].Texture = RHICreateSharedTexture2D(BufferSizeX/2,BufferSizeY/2,FogBufferFormat,1,VelocityMemoryBuffer,TexCreate_ResolveTargetable);
		RenderTargets[FogBuffer].Surface = RHICreateTargetableSurface(BufferSizeX/2,BufferSizeY/2,FogBufferFormat,RenderTargets[FogBuffer].Texture,MultiSampleFlag,TEXT("FogBuffer"));
#else
		VelocityBufferSizeX	= BufferSizeX;
		VelocityBufferSizeY	= BufferSizeY;
#endif

		// Is motion blur allowed?
		if ( GSystemSettings.RenderThreadSettings.bAllowMotionBlur )
		{
			// Create a texture to store the resolved velocity 2d-vectors, and a render-targetable surface to hold them.
#if XBOX && !USE_NULL_RHI
			RenderTargets[VelocityBuffer].Texture = RHICreateSharedTexture2D(VelocityBufferSizeX,VelocityBufferSizeY,VelocityBufferFormat,1,VelocityMemoryBuffer,TexCreate_ResolveTargetable);
#else
			RenderTargets[VelocityBuffer].Texture = RHICreateTexture2D(VelocityBufferSizeX,VelocityBufferSizeY,VelocityBufferFormat,1,TexCreate_ResolveTargetable,NULL);
#endif
			RenderTargets[VelocityBuffer].Surface = RHICreateTargetableSurface(
				VelocityBufferSizeX,VelocityBufferSizeY,VelocityBufferFormat,RenderTargets[VelocityBuffer].Texture,MultiSampleFlag,TEXT("VelocityBuffer"));
		}

		// Are dynamic shadows allowed?
		if ( GSystemSettings.RenderThreadSettings.bAllowDynamicShadows )
		{
			#if !PS3
				if ( !GSupportsDepthTextures && !IsValidRef(RenderTargets[ShadowDepthColor].Surface) )
				{
					//create the shadow depth color surface
					//platforms with GSupportsDepthTextures don't need a depth color target
					//platforms with GSupportsHardwarePCF still need a color target due to API restrictions (except PS3)
					RenderTargets[ShadowDepthColor].Texture = RHICreateTexture2D(GetShadowDepthTextureResolution(),GetShadowDepthTextureResolution(),PF_R32F,1,TexCreate_ResolveTargetable,NULL);
					RenderTargets[ShadowDepthColor].Surface = RHICreateTargetableSurface(
						GetShadowDepthTextureResolution(),GetShadowDepthTextureResolution(),PF_R32F,RenderTargets[ShadowDepthColor].Texture,0,TEXT("ShadowDepthRT"));
				}
			#endif

			//create the shadow depth texture and/or surface
			if (GSupportsHardwarePCF)
			{
				// Create a depth texture, used to sample PCF values
				RenderTargets[ShadowDepthZ].Texture = RHICreateTexture2D(
					GetShadowDepthTextureResolution(),GetShadowDepthTextureResolution(),PF_FilteredShadowDepth,1,TexCreate_DepthStencil,NULL);
    
				// Don't create a dedicated surface
				RenderTargets[ShadowDepthZ].Surface = RHICreateTargetableSurface(
					GetShadowDepthTextureResolution(),
					GetShadowDepthTextureResolution(),
					PF_FilteredShadowDepth,
					RenderTargets[ShadowDepthZ].Texture,
					0,
					TEXT("ShadowDepthZ")
					);
			}
			else if (GSupportsFetch4)
			{
				// Create a D24 depth stencil texture for use with Fetch4 shadows
				RenderTargets[ShadowDepthZ].Texture = RHICreateTexture2D(
					GetShadowDepthTextureResolution(),GetShadowDepthTextureResolution(),PF_D24,1,TexCreate_DepthStencil,NULL);
    
				// Don't create a dedicated surface
				RenderTargets[ShadowDepthZ].Surface = RHICreateTargetableSurface(
					GetShadowDepthTextureResolution(),
					GetShadowDepthTextureResolution(),
					PF_D24,
					RenderTargets[ShadowDepthZ].Texture,
					0,
					TEXT("ShadowDepthZ")
					);
			}
			else
			{
				if( GSupportsDepthTextures )
				{
					// Create a texture to store the resolved shadow depth
#if XBOX && !USE_NULL_RHI
					// Overlap the shadow depth texture with the light attenuation buffer, since their lifetimes don't overlap
					RenderTargets[ShadowDepthZ].Texture = RHICreateSharedTexture2D(
						GetShadowDepthTextureResolution(),GetShadowDepthTextureResolution(),PF_ShadowDepth,1,LightAttenuationMemoryBuffer,TexCreate_ResolveTargetable);
#else
					RenderTargets[ShadowDepthZ].Texture = RHICreateTexture2D(
						GetShadowDepthTextureResolution(),GetShadowDepthTextureResolution(),PF_ShadowDepth,1,TexCreate_ResolveTargetable,NULL);
#endif
				}
    
				// Create a dedicated depth-stencil target surface for shadow depth rendering.
				RenderTargets[ShadowDepthZ].Surface = RHICreateTargetableSurface(
					GetShadowDepthTextureResolution(),
					GetShadowDepthTextureResolution(),
					PF_ShadowDepth,
					RenderTargets[ShadowDepthZ].Texture,
					TargetSurfCreate_Dedicated,
					TEXT("ShadowDepthZ")
					);
			}
		}
		
		// Are fog volumes allowed?
		if ( GSystemSettings.RenderThreadSettings.bAllowFogVolumes )
		{
#if !XBOX
			if (CanBlendWithFPRenderTarget(GRHIShaderPlatform))
			{
				//FogFrontfacesIntegralAccumulation texture and surface not used
				//allocate the highest precision render target with blending and filtering
				//@todo dw - ATI x1x00 cards don't filter this format!
				RenderTargets[FogBackfacesIntegralAccumulation].Texture = RHICreateTexture2D(FogAccumulationBufferSizeX,FogAccumulationBufferSizeY,SceneColorBufferFormat,1,TexCreate_ResolveTargetable,NULL);
				RenderTargets[FogBackfacesIntegralAccumulation].Surface = RHICreateTargetableSurface(
					FogAccumulationBufferSizeX,FogAccumulationBufferSizeY,SceneColorBufferFormat,RenderTargets[FogBackfacesIntegralAccumulation].Texture,0,TEXT("FogBackfacesIntegralAccumulationRT"));
			}
			else
#endif
		    {
				//have to use a low precision format that supports blending and filtering since the only fp format Xenon can blend to (7e3) doesn't have enough precision
#if XBOX && !USE_NULL_RHI
				// Overlap with the light attenuation buffer since both the light attenuation texture and the fog integral textures have short lifetimes that don't overlap
				RenderTargets[FogFrontfacesIntegralAccumulation].Texture = RHICreateSharedTexture2D(FogAccumulationBufferSizeX,FogAccumulationBufferSizeY,PF_A8R8G8B8,1,LightAttenuationMemoryBuffer,TexCreate_ResolveTargetable);
#else
				RenderTargets[FogFrontfacesIntegralAccumulation].Texture = RHICreateTexture2D(FogAccumulationBufferSizeX,FogAccumulationBufferSizeY,PF_A8R8G8B8,1,TexCreate_ResolveTargetable,NULL);
#endif
			    RenderTargets[FogFrontfacesIntegralAccumulation].Surface = RHICreateTargetableSurface(
				    FogAccumulationBufferSizeX,FogAccumulationBufferSizeY,PF_A8R8G8B8,RenderTargets[FogFrontfacesIntegralAccumulation].Texture,0,TEXT("FogFrontfacesIntegralAccumulationRT"));
    
#if XBOX && !USE_NULL_RHI
				// Overlap with the velocity buffer since both the velocity texture and the fog integral textures have short lifetimes that don't overlap
				RenderTargets[FogBackfacesIntegralAccumulation].Texture = RHICreateSharedTexture2D(FogAccumulationBufferSizeX,FogAccumulationBufferSizeY,PF_A8R8G8B8,1,VelocityMemoryBuffer,TexCreate_ResolveTargetable);
#else
				RenderTargets[FogBackfacesIntegralAccumulation].Texture = RHICreateTexture2D(FogAccumulationBufferSizeX,FogAccumulationBufferSizeY,PF_A8R8G8B8,1,TexCreate_ResolveTargetable,NULL);
#endif
			    RenderTargets[FogBackfacesIntegralAccumulation].Surface = RHICreateTargetableSurface(
				    FogAccumulationBufferSizeX,FogAccumulationBufferSizeY,PF_A8R8G8B8,RenderTargets[FogBackfacesIntegralAccumulation].Texture,0,TEXT("FogBackfacesIntegralAccumulationRT"));
		    }
	    }
	}
}

void FSceneRenderTargets::ReleaseDynamicRHI()
{
	// make sure no scene render targets and textures are in use before releasing them
	RHISetRenderTarget(FSurfaceRHIRef(),FSurfaceRHIRef());

	for( INT RTIdx=0; RTIdx < MAX_SCENE_RENDERTARGETS; RTIdx++ )
	{
		RenderTargets[RTIdx].Texture.SafeRelease();
		RenderTargets[RTIdx].Surface.SafeRelease();
	}
}

UINT FSceneRenderTargets::GetShadowDepthTextureResolution() const
{
	return Clamp(GSystemSettings.RenderThreadSettings.MaxShadowResolution,1,GMaxShadowDepthBufferSize);
}

void FSceneRenderTargets::SetAmbientOcclusionDownsampleFactor(INT NewDownsampleFactor)
{
	check(IsInRenderingThread());
	AmbientOcclusionBufferSizeX = Max< UINT >( 1, BufferSizeX / NewDownsampleFactor );
	AmbientOcclusionBufferSizeY = Max< UINT >( 1, BufferSizeY / NewDownsampleFactor );

	if (NewDownsampleFactor != AmbientOcclusionDownsampleFactor)
	{
		AmbientOcclusionDownsampleFactor = NewDownsampleFactor;
		if (IsInitialized())
		{
			RenderTargets[AmbientOcclusion].Texture.SafeRelease();
			RenderTargets[AmbientOcclusion].Surface.SafeRelease();
			RenderTargets[AOHistory].Texture.SafeRelease();
			RenderTargets[AOHistory].Surface.SafeRelease();
		}
		AllocateAmbientOcclusionBuffers();
	}
}

void FSceneRenderTargets::AllocateAmbientOcclusionBuffers()
{
	// Allocate render targets needed for ambient occlusion calculations if allowed
	//@todo - actually check if there is an ambient occlusion effect that is going to use these before allocating,
	// also only allocate the history buffers if they are going to be used.
	if ( GSystemSettings.RenderThreadSettings.bAllowAmbientOcclusion 
		&& AmbientOcclusionBufferSizeX > 0
		&& AmbientOcclusionBufferSizeY > 0 )
	{
		const EPixelFormat AmbientOcclusionFormat = PF_G16R16F;
		// @todo: use fixed point on ATI cards for filtering support
		const EPixelFormat AOHistoryFormat = PF_G16R16F_FILTER;

		// Create a dedicated render target for ambient occlusion, since it will be filtered in several passes.
#if XBOX && !USE_NULL_RHI
		// Overlap with the light attenuation buffer since their lifetimes don't overlap
		RenderTargets[AmbientOcclusion].Texture = RHICreateSharedTexture2D(AmbientOcclusionBufferSizeX,AmbientOcclusionBufferSizeY,AmbientOcclusionFormat,1,LightAttenuationMemoryBuffer,TexCreate_ResolveTargetable);
#else
		RenderTargets[AmbientOcclusion].Texture = RHICreateTexture2D(AmbientOcclusionBufferSizeX,AmbientOcclusionBufferSizeY,AmbientOcclusionFormat,1,TexCreate_ResolveTargetable,NULL);
#endif
		RenderTargets[AmbientOcclusion].Surface = RHICreateTargetableSurface(
			AmbientOcclusionBufferSizeX,AmbientOcclusionBufferSizeY,AmbientOcclusionFormat,RenderTargets[AmbientOcclusion].Texture,TargetSurfCreate_Dedicated,TEXT("AmbientOcclusion"));

		// Create a dedicated render target for ambient occlusion history, since we will be reading from the history and writing to it in the same draw call.
		RenderTargets[AOHistory].Texture = RHICreateTexture2D(AmbientOcclusionBufferSizeX,AmbientOcclusionBufferSizeY,AOHistoryFormat,1,TexCreate_ResolveTargetable,NULL);
		RenderTargets[AOHistory].Surface = RHICreateTargetableSurface(
			AmbientOcclusionBufferSizeX,AmbientOcclusionBufferSizeY,AOHistoryFormat,RenderTargets[AOHistory].Texture,TargetSurfCreate_Dedicated,TEXT("AOHistory"));

		bAOHistoryNeedsCleared = TRUE;
	}
}

/** Returns a string matching the given ESceneRenderTargetTypes */
FString FSceneRenderTargets::GetRenderTargetName(ESceneRenderTargetTypes RTEnum) const
{
	FString RenderTargetName;
#define RTENUMNAME(x) case x: RenderTargetName = TEXT(#x); break;
	switch(RTEnum)
	{
		RTENUMNAME(FilterColor)
		RTENUMNAME(SceneColor)
		RTENUMNAME(SceneColorRaw)
		RTENUMNAME(SceneColorFixedPoint)
		RTENUMNAME(SceneColorScratch)
		RTENUMNAME(SceneDepthZ)
		RTENUMNAME(SmallDepthZ)
		RTENUMNAME(ShadowDepthZ)
		RTENUMNAME(ShadowDepthColor)
		RTENUMNAME(ShadowVariance)
		RTENUMNAME(LightAttenuation)
		RTENUMNAME(AmbientOcclusion)
		RTENUMNAME(AOHistory)
		RTENUMNAME(VelocityBuffer)
		RTENUMNAME(QuarterSizeSceneColor)
		RTENUMNAME(FogFrontfacesIntegralAccumulation)
		RTENUMNAME(FogBackfacesIntegralAccumulation)
		RTENUMNAME(HitProxy)
		RTENUMNAME(FogBuffer)
		default: RenderTargetName = FString::Printf(TEXT("%08X"),(INT)RTEnum);
	}
#undef RTENUMNAME
	return RenderTargetName;
}

/*-----------------------------------------------------------------------------
FSceneTextureShaderParameters
-----------------------------------------------------------------------------*/

//
void FSceneTextureShaderParameters::Bind(const FShaderParameterMap& ParameterMap)
{
	// only used if Material has an expression that requires SceneColorTexture
	SceneColorTextureParameter.Bind(ParameterMap,TEXT("SceneColorTexture"),TRUE);
	// only used if Material has an expression that requires SceneDepthTexture
	SceneDepthTextureParameter.Bind(ParameterMap,TEXT("SceneDepthTexture"),TRUE);
	// only used if Material has an expression that requires SceneDepthTexture
	SceneDepthCalcParameter.Bind(ParameterMap,TEXT("MinZ_MaxZRatio"),TRUE);
	// only used if Material has an expression that requires ScreenPosition biasing
	ScreenPositionScaleBiasParameter.Bind(ParameterMap,TEXT("ScreenPositionScaleBias"),TRUE);
}

//
void FSceneTextureShaderParameters::Set(const FSceneView* View,FShader* PixelShader, ESamplerFilter ColorFilter, const FTexture2DRHIRef& DesiredSceneColorTexture) const
{
	if (SceneColorTextureParameter.IsBound() == TRUE)
	{
		FSamplerStateRHIRef Filter;
		switch ( ColorFilter )
		{
			case SF_Bilinear:
				Filter = TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
				break;
			case SF_Trilinear:
				Filter = TStaticSamplerState<SF_Trilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
				break;
			case SF_AnisotropicPoint:
				Filter = TStaticSamplerState<SF_AnisotropicPoint,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
				break;
			case SF_AnisotropicLinear:
				Filter = TStaticSamplerState<SF_AnisotropicLinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
				break;
			case SF_Point:
			default:
				Filter = TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
				break;
		}

		SetTextureParameter(
			PixelShader->GetPixelShader(),
			SceneColorTextureParameter,
			Filter,
			View->bUseLDRSceneColor ? GSceneRenderTargets.GetSceneColorLDRTexture() : DesiredSceneColorTexture
			);
	}
	if (SceneDepthTextureParameter.IsBound())
	{
		if (GSupportsDepthTextures && IsValidRef(GSceneRenderTargets.GetSceneDepthTexture()))
		{
			// Bind the zbuffer as a texture if depth textures are supported
			SetTextureParameter(
				PixelShader->GetPixelShader(),
				SceneDepthTextureParameter,
				TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
				GSceneRenderTargets.GetSceneDepthTexture()
				);
		}
		else if (GRHIShaderPlatform == SP_PCD3D_SM2 && IsValidRef(GSceneRenderTargets.GetSceneColorScratchTexture()))
		{
			// Bind the scene color scratch surface in SM2, whose alpha contains scene depth from the base pass.
			SetTextureParameter(
				PixelShader->GetPixelShader(),
				SceneDepthTextureParameter,
				TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
				GSceneRenderTargets.GetSceneColorScratchTexture()
				);
		}
	}

	RHISetViewPixelParameters( View, PixelShader->GetPixelShader(), &SceneDepthCalcParameter, &ScreenPositionScaleBiasParameter );
}

void FSceneTextureShaderParameters::Set(const FSceneView* View,FShader* PixelShader, ESamplerFilter ColorFilter/*=SF_Point*/) const
{
	Set(View, PixelShader,ColorFilter, GSceneRenderTargets.GetSceneColorTexture());
}

//
FArchive& operator<<(FArchive& Ar,FSceneTextureShaderParameters& Parameters)
{
	Ar << Parameters.SceneColorTextureParameter;
	Ar << Parameters.SceneDepthTextureParameter;
	Ar << Parameters.SceneDepthCalcParameter;
	Ar << Parameters.ScreenPositionScaleBiasParameter;
	return Ar;
}

/*-----------------------------------------------------------------------------
FSceneRenderTargetProxy
-----------------------------------------------------------------------------*/

/**
* Constructor
*/
FSceneRenderTargetProxy::FSceneRenderTargetProxy()
:	SizeX(0)
,	SizeY(0)
{	
}

/**
* Set SizeX and SizeY of proxy and re-allocate scene targets as needed
*
* @param InSizeX - scene render target width requested
* @param InSizeY - scene render target height requested
*/
void FSceneRenderTargetProxy::SetSizes(UINT InSizeX,UINT InSizeY)
{
	SizeX = InSizeX;
	SizeY = InSizeY;

	if( IsInRenderingThread() )
	{
		GSceneRenderTargets.Allocate(SizeX,SizeY);
	}
	else
	{
		struct FRenderTargetSizeParams
		{
			UINT SizeX;
			UINT SizeY;
		};
		FRenderTargetSizeParams RenderTargetSizeParams = {SizeX,SizeY};
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			RenderTargetAllocProxyCommand,
			FRenderTargetSizeParams,Parameters,RenderTargetSizeParams,
		{
			GSceneRenderTargets.Allocate(Parameters.SizeX, Parameters.SizeY);
		});
	}
}

/**
* @return RHI surface for setting the render target
*/
const FSurfaceRHIRef& FSceneRenderTargetProxy::GetRenderTargetSurface() const
{
	return GSceneRenderTargets.GetSceneColorSurface();
}

/**
* @return width of the scene render target this proxy will render to
*/
UINT FSceneRenderTargetProxy::GetSizeX() const
{
	return SizeX;
}

/**
* @return height of the scene render target this proxy will render to
*/
UINT FSceneRenderTargetProxy::GetSizeY() const
{
	return SizeY;
}

/**
* @return gamma this render target should be rendered with
*/
FLOAT FSceneRenderTargetProxy::GetDisplayGamma() const
{
	return 1.0f;
}

/**
* @return RHI surface for setting the render target
*/
const FSurfaceRHIRef& FSceneDepthTargetProxy::GetDepthTargetSurface() const
{
	return GSceneRenderTargets.GetSceneDepthSurface();
}

