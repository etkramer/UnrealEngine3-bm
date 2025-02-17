/*=============================================================================
	SceneRenderTargets.h: Scene render target definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __SCENERENDERTARGETS_H__
#define __SCENERENDERTARGETS_H__

/**
 * Encapsulates the render targets used for scene rendering.
 */
class FSceneRenderTargets : public FRenderResource
{
public:

	/** Destructor. */
	virtual ~FSceneRenderTargets() {}

	/** 
	 * Dumps information about render target memory usage
	 * Must be called on the rendering thread or while the rendering thread is blocked
	 * Currently only implemented for xbox
	 */
	void DumpMemoryUsage(FOutputDevice& Ar) const;

	/**
	 * Checks that scene render targets are ready for rendering a view family of the given dimensions.
	 * If the allocated render targets are too small, they are reallocated.
	 */
	void Allocate(UINT MinSizeX,UINT MinSizeY);

	void BeginRenderingFilter();
	void FinishRenderingFilter();

	/**
	 * Sets the scene color target and restores its contents if necessary
	 * @param bRestoreContents - if TRUE then copies contents of SceneColorTexture to the SceneColorSurface
	 */
	void BeginRenderingSceneColor(UBOOL bRestoreContents = FALSE);
	/**
	 * Called when finished rendering to the scene color surface
	 * @param bKeepChanges - if TRUE then the SceneColorSurface is resolved to the SceneColorTexture
	 */
	void FinishRenderingSceneColor(UBOOL bKeepChanges = TRUE, const FResolveParams& ResolveParams = FResolveParams());

    /**
     * Resolve a previously rendered scene color surface.
     */
    void ResolveSceneColor(const FResolveParams& ResolveParams = FResolveParams());

    /**
     * Sets the LDR version of the scene color target.
     */
    void BeginRenderingSceneColorLDR();
    /**
     * Called when finished rendering to the LDR version of the scene color surface.
     * @param bKeepChanges - if TRUE then the SceneColorSurface is resolved to the LDR SceneColorTexture
	 * @param ResolveParams - optional resolve params
     */
    void FinishRenderingSceneColorLDR(UBOOL bKeepChanges = TRUE, const FResolveParams& ResolveParams = FResolveParams());

	/**
	 * Sets the raw version of the scene color target.
	 */
	void BeginRenderingSceneColorRaw();

	/**
	 * Saves a previously rendered scene color surface in the raw bit format.
	 */
	void SaveSceneColorRaw(UBOOL bConvertToFixedPoint = FALSE, const FResolveParams& ResolveParams = FResolveParams());

	/**
	 * Restores a previously saved raw-scene color surface.
	 */
	void RestoreSceneColorRaw();

	/**
	 * Restores a portion of a previously saved raw-scene color surface.
	 */
	void RestoreSceneColorRectRaw(FLOAT X1,FLOAT Y1,FLOAT X2,FLOAT Y2);

	/**
	 * Sets the HDR scene color scratch pad render target, which is used during the base pass for SM2.
	 */
	void BeginRenderingSceneColorScratch();

    /** Called to start rendering the depth pre-pass. */
	void BeginRenderingPrePass();

	/** Called when finished rendering the depth pre-pass. */
	void FinishRenderingPrePass();

	void BeginRenderingShadowVolumes();
	void FinishRenderingShadowVolumes();

	void BeginRenderingShadowDepth();
	/**
	* Called when finished rendering to the subject shadow depths so the surface can be copied to texture
	* @param ResolveParams - optional resolve params
	*/
	void FinishRenderingShadowDepth(const FResolveParams& ResolveParams = FResolveParams());

	void BeginRenderingLightAttenuation();
	void FinishRenderingLightAttenuation();

	void BeginRenderingAmbientOcclusion(UBOOL bUseDownsizedDepthBuffer = FALSE);
	void FinishRenderingAmbientOcclusion(const FResolveParams& ResolveParams);

	void BeginRenderingAOHistory(UBOOL bUseDownsizedDepthBuffer = FALSE);
	void FinishRenderingAOHistory(const FResolveParams& ResolveParams);

	void BeginRenderingDistortionAccumulation();
	void FinishRenderingDistortionAccumulation(const FResolveParams& ResolveParams = FResolveParams());

	void BeginRenderingDistortionDepth();
	void FinishRenderingDistortionDepth();

	/** Starts rendering to the velocity buffer. */
	void BeginRenderingVelocities();

	/** Stops rendering to the velocity buffer. */
	void FinishRenderingVelocities();

	void BeginRenderingFogFrontfacesIntegralAccumulation();
	void FinishRenderingFogFrontfacesIntegralAccumulation();

	void BeginRenderingFogBackfacesIntegralAccumulation();
	void FinishRenderingFogBackfacesIntegralAccumulation();

	void ResolveSceneDepthTexture();

	/** Updates the quarter-sized depth buffer with the current contents of the scene depth texture. */
	void UpdateSmallDepthSurface();

	void BeginRenderingHitProxies();
	void FinishRenderingHitProxies();

	void BeginRenderingFogBuffer();
	void FinishRenderingFogBuffer(const FResolveParams& ResolveParams);

	/**
	 * Creates/releases render targets based on the current System Settings
	 */
	void ApplySystemSettings();

	// FRenderResource interface.
	virtual void InitDynamicRHI();
	virtual void ReleaseDynamicRHI();

	// Texture Accessors.
	const FTexture2DRHIRef& GetFilterColorTexture() const { return RenderTargets[FilterColor].Texture; }
	const FTexture2DRHIRef& GetSceneColorRawTexture() const { return RenderTargets[SceneColorRaw].Texture; }
	const FTexture2DRHIRef& GetSceneColorScratchTexture() const { return RenderTargets[SceneColorScratch].Texture; }
	const FTexture2DRHIRef& GetSceneColorTexture() const { return RenderTargets[SceneColor].Texture; }
	const FTexture2DRHIRef& GetSceneColorLDRTexture() const { return RenderTargets[LightAttenuation].Texture; }
	const FTexture2DRHIRef& GetSceneDepthTexture() const { return RenderTargets[SceneDepthZ].Texture; }
	const FTexture2DRHIRef& GetShadowDepthZTexture() const { return RenderTargets[ShadowDepthZ].Texture; }
	const FTexture2DRHIRef& GetShadowDepthColorTexture() const { return RenderTargets[ShadowDepthColor].Texture; }
	const FTexture2DRHIRef& GetLightAttenuationTexture() const { return RenderTargets[LightAttenuation].Texture; }
	const FTexture2DRHIRef& GetAmbientOcclusionTexture() const { return RenderTargets[AmbientOcclusion].Texture; }
	const FTexture2DRHIRef& GetAOHistoryTexture() const { return RenderTargets[AOHistory].Texture; }
	const FTexture2DRHIRef& GetVelocityTexture() const { return RenderTargets[VelocityBuffer].Texture; }
	const FTexture2DRHIRef& GetQuarterSizeSceneColorTexture() const { return RenderTargets[QuarterSizeSceneColor].Texture; }
	const FTexture2DRHIRef& GetFogFrontfacesIntegralAccumulationTexture() const { return RenderTargets[FogFrontfacesIntegralAccumulation].Texture; }	
	const FTexture2DRHIRef& GetFogBackfacesIntegralAccumulationTexture() const { return RenderTargets[FogBackfacesIntegralAccumulation].Texture; }
	const FTexture2DRHIRef& GetHitProxyTexture() const { return RenderTargets[HitProxy].Texture; }
	const FTexture2DRHIRef& GetFogBufferTexture() const { return RenderTargets[FogBuffer].Texture; }
	
	/** 
	* Allows substitution of a 1x1 white texture in place of the light attenuation buffer when it is not needed;
	* this improves shader performance and removes the need for redundant Clears
	*/
	void SetLightAttenuationMode(UBOOL bEnabled) { bLightAttenuationEnabled = bEnabled; }
	const FTextureRHIRef& GetEffectiveLightAttenuationTexture() const 
	{
		if( bLightAttenuationEnabled )
		{
			return *(FTextureRHIRef*)&RenderTargets[LightAttenuation].Texture;
		}
		else
		{
			return GWhiteTexture->TextureRHI;
		}
	}

	UBOOL IsDownsizedDepthSupported() const { return bDownsizedDepthSupported; }

	// Surface Accessors.
	const FSurfaceRHIRef& GetFilterColorSurface() const							{ return RenderTargets[FilterColor].Surface; }
	const FSurfaceRHIRef& GetSceneColorRawSurface() const						{ return RenderTargets[SceneColorRaw].Surface; }
	const FSurfaceRHIRef& GetSceneColorScratchSurface() const					{ return RenderTargets[SceneColorScratch].Surface; }
	const FSurfaceRHIRef& GetSceneColorSurface() const							{ return RenderTargets[SceneColor].Surface; }
	const FSurfaceRHIRef& GetSceneColorLDRSurface() const						{ return RenderTargets[LightAttenuation].Surface; }
	const FSurfaceRHIRef& GetSceneDepthSurface() const							{ return RenderTargets[SceneDepthZ].Surface; }
	const FSurfaceRHIRef& GetSmallDepthSurface() const							{ return RenderTargets[SmallDepthZ].Surface; }
	const FSurfaceRHIRef& GetShadowDepthZSurface() const						{ return RenderTargets[ShadowDepthZ].Surface; }
	const FSurfaceRHIRef& GetShadowDepthColorSurface() const					{ return RenderTargets[ShadowDepthColor].Surface; }
	const FSurfaceRHIRef& GetLightAttenuationSurface() const					{ return RenderTargets[LightAttenuation].Surface; }
	const FSurfaceRHIRef& GetAmbientOcclusionSurface() const					{ return RenderTargets[AmbientOcclusion].Surface; }
	const FSurfaceRHIRef& GetAOHistorySurface() const							{ return RenderTargets[AOHistory].Surface; }
	const FSurfaceRHIRef& GetVelocitySurface() const							{ return RenderTargets[VelocityBuffer].Surface; }
	const FSurfaceRHIRef& GetQuarterSizeSceneColorSurface() const				{ return RenderTargets[QuarterSizeSceneColor].Surface; }
	const FSurfaceRHIRef& GetFogFrontfacesIntegralAccumulationSurface() const	{ return RenderTargets[FogFrontfacesIntegralAccumulation].Surface; }
	const FSurfaceRHIRef& GetFogBackfacesIntegralAccumulationSurface() const	{ return RenderTargets[FogBackfacesIntegralAccumulation].Surface; }
	const FSurfaceRHIRef& GetHitProxySurface() const							{ return RenderTargets[HitProxy].Surface; }
	const FSurfaceRHIRef& GetFogBufferSurface() const							{ return RenderTargets[FogBuffer].Surface; }

	UINT GetShadowDepthTextureResolution() const;	

	UINT GetBufferSizeX() const { return BufferSizeX; }
	UINT GetBufferSizeY() const { return BufferSizeY; }
	void SetBufferSize( const UINT InBufferSizeX, const UINT InBufferSizeY );

	UINT GetSmallColorDepthDownsampleFactor() const { return SmallColorDepthDownsampleFactor; }

	UINT GetFilterDownsampleFactor() const { return FilterDownsampleFactor; }

	UINT GetFilterBufferSizeX() const { return FilterBufferSizeX; }
	UINT GetFilterBufferSizeY() const { return FilterBufferSizeY; }

	UINT GetVelocityBufferSizeX() const { return VelocityBufferSizeX; }
	UINT GetVelocityBufferSizeY() const { return VelocityBufferSizeY; }

	UINT GetFogAccumulationDownsampleFactor() const { return FogAccumulationDownsampleFactor; }
	UINT GetFogAccumulationBufferSizeX() const { return FogAccumulationBufferSizeX; }
	UINT GetFogAccumulationBufferSizeY() const { return FogAccumulationBufferSizeY; }

	UINT GetAmbientOcclusionDownsampleFactor() const { return AmbientOcclusionDownsampleFactor; }
	void SetAmbientOcclusionDownsampleFactor(INT NewDownsampleFactor);
	UINT GetAmbientOcclusionBufferSizeX() const { return AmbientOcclusionBufferSizeX; }
	UINT GetAmbientOcclusionBufferSizeY() const { return AmbientOcclusionBufferSizeY; }

	/** TRUE if the ambient occlusion history render target needs to be cleared before next use. */
	UBOOL bAOHistoryNeedsCleared;

	/** TRUE if the shared surface memory used for SceneColor and SceneColorRaw contains the raw format. */
	UBOOL bSceneColorTextureIsRaw;

#if SUPPORTS_VSM
	void BeginRenderingShadowVariance();
	void FinishRenderingShadowVariance(const FResolveParams& ResolveParams);
	const FTexture2DRHIRef& GetShadowVarianceTexture() const { return RenderTargets[ShadowVariance].Texture; }
	const FSurfaceRHIRef& GetShadowVarianceSurface() const { return RenderTargets[ShadowVariance].Surface; }
	UINT GetShadowVarianceTextureResolution() const;
#endif //#if SUPPORTS_VSM

protected:
	// Constructor.
	FSceneRenderTargets(): 
		bAOHistoryNeedsCleared(FALSE),
		bSceneColorTextureIsRaw(FALSE),
		BufferSizeX(0), 
		BufferSizeY(0), 
		SmallColorDepthDownsampleFactor(2),
		FilterDownsampleFactor(0), 
		FilterBufferSizeX(0), 
		FilterBufferSizeY(0), 
		VelocityBufferSizeX(0),
		VelocityBufferSizeY(0),
		LightAttenuationMemoryBuffer(NULL),
		AmbientOcclusionDownsampleFactor(2),
		SceneColorBufferFormat(PF_FloatRGB),
		bLightAttenuationEnabled(TRUE),
		bDownsizedDepthSupported(FALSE)
		{}

private:

	UINT BufferSizeX;
	UINT BufferSizeY;

	UINT SmallColorDepthDownsampleFactor;

	UINT FilterDownsampleFactor;
	UINT FilterBufferSizeX;
	UINT FilterBufferSizeY;

	UINT VelocityBufferSizeX;
	UINT VelocityBufferSizeY;

	UINT FogAccumulationDownsampleFactor;
	UINT FogAccumulationBufferSizeX;
	UINT FogAccumulationBufferSizeY;

	FSharedMemoryResourceRHIRef LightAttenuationMemoryBuffer;
	UINT AmbientOcclusionDownsampleFactor;
	UINT AmbientOcclusionBufferSizeX;
	UINT AmbientOcclusionBufferSizeY;

	EPixelFormat SceneColorBufferFormat;

	enum ESceneRenderTargetTypes
	{
		// Render target for post process filter colors.
        FilterColor=0,
		// Render target for scene colors.
		SceneColor,
		// Render target for scene colors (resolved as raw-bits).
		SceneColorRaw,
		// Render target for scene colors (converted to fixed point).
		SceneColorFixedPoint,
		// Render target used for scene color and depth in SM2.
		SceneColorScratch,
		// Render target for scene depths.
		SceneDepthZ,
		// Render target for a quarter-sized version of the scene depths.
		SmallDepthZ,
		// Render target for shadow depths.
		ShadowDepthZ,
		// Render target for shadow depths as color.
		ShadowDepthColor,
		// Render target for shadow variance depths
		ShadowVariance,
		// Render target for light attenuation values.
		LightAttenuation,
		// Render target for ambient occlusion calculations.
		AmbientOcclusion,
		// Render target for ambient occlusion history.
		AOHistory,
		// Render target for motion velocity 2D-vectors.
		VelocityBuffer,
		// Render target for scene colors (resolved at quarter size).
		QuarterSizeSceneColor,
		// Render target that accumulates fog volume frontface integrals
		FogFrontfacesIntegralAccumulation,
		// Render target that accumulates fog volume backface integrals
		FogBackfacesIntegralAccumulation,
		// Render target that hit proxy IDs are drawn to
		HitProxy,
		// Render target for downsampled fog factors
		FogBuffer,
		// Max scene RTs available
        MAX_SCENE_RENDERTARGETS
	};

	enum ESceneRenderTargetFlags
	{

	};

	/**
	* Single render target item consists of a render surface and its resolve texture
	*/
	struct FSceneRenderTargetItem
	{
		FSceneRenderTargetItem() 
			: Flags(0) 
		{
		}
		/** texture for resolving to */
		FTexture2DRHIRef Texture;
		/** surface to render to */
		FSurfaceRHIRef Surface;
        /** combination of ESceneRenderTargetFlags */
		DWORD Flags;
	};
	/** static array of all the scene render targets */
	FSceneRenderTargetItem RenderTargets[MAX_SCENE_RENDERTARGETS];	
	/** if TRUE we use the light attenuation buffer otherwise the 1x1 white texture is used */
	BITFIELD bLightAttenuationEnabled : 1;
	/** Whether the render target corresponding to SmallDepthZ is supported. */
	BITFIELD bDownsizedDepthSupported : 1;
	/** Allocates ambient occlusion buffers if allowed */
	void AllocateAmbientOcclusionBuffers();
	/** Returns a string matching the given ESceneRenderTargetTypes */
	FString GetRenderTargetName(ESceneRenderTargetTypes RTEnum) const;
};

/** The global render targets used for scene rendering. */
extern TGlobalResource<FSceneRenderTargets> GSceneRenderTargets;

/**
* Proxy render target that wraps an existing render target RHI resource
*/
class FSceneRenderTargetProxy : public FRenderTarget
{
public:
	/**
	* Constructor
	*/
	FSceneRenderTargetProxy();

	/**
	* Set SizeX and SizeY of proxy and re-allocate scene targets as needed
	*
	* @param InSizeX - scene render target width requested
	* @param InSizeY - scene render target height requested
	*/
	void SetSizes(UINT InSizeX,UINT InSizeY);

	// FRenderTarget interface

	/**
	* @return width of the scene render target this proxy will render to
	*/
	virtual UINT GetSizeX() const;

	/**
	* @return height of the scene render target this proxy will render to
	*/
	virtual UINT GetSizeY() const;	

	/**
	* @return gamma this render target should be rendered with
	*/
	virtual FLOAT GetDisplayGamma() const;

	/**
	* @return RHI surface for setting the render target
	*/
	virtual const FSurfaceRHIRef& GetRenderTargetSurface() const;

private:

	/** scene render target width requested */
	UINT SizeX;
	/** scene render target height requested */
	UINT SizeY;
};

/**
* Proxy render target that wraps an existing scene depth buffer surface RHI resource
*/
class FSceneDepthTargetProxy
{
public:
	/**
	* @return RHI surface for setting the render target
	*/
	const FSurfaceRHIRef& GetDepthTargetSurface() const;
};

#endif
