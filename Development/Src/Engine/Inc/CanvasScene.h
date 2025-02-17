/*=============================================================================
	CanvasScene.h: Canvas scene rendering classes
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __CANVASSCENE_H__
#define __CANVASSCENE_H__

/**
* A simple scene setup for rendering primitives on the canvas. 
* Uses an FScene internally
*/
class FCanvasScene : public FSceneInterface
{
public:

	// FCanvasScene

	/**
	* Constructor
	*/
	FCanvasScene();	

	/**
	* Get the number of primitive components that have been added to this scene
	* @return number of primitives
	*/
	INT GetNumPrimitives();
	/**
	* Get the number of light components that have been added to this scene
	* @return number of lights
	*/
	INT GetNumLights();
	/** 
	* Set the Camera location for view matrix calcs 
	*/
	void SetViewLocaton(const FVector& InCameraOffset);
	/** 
	* Set the Field of view for projection calcs 
	*/
	void SetFOV(FLOAT InFOV);
	/**
	* Return the scene to be used for rendering
	*/
	virtual FSceneInterface* GetRenderScene()
	{
		return Scene;
	}
	/**
	* Generate the views for rendering this scene
	*
	* @param ViewFamily - new views are added to this view family
	* @param Offset - screen offset scale for this view [0,1]
	* @param Size - screen size scale for this view [0,1]
	* @param Viewport - the viewport that will be used to render the views
	* @param InPostProcessChain - optional post process chain to be used for the view
	* @param InPostProcessSettings - optional post process settings to be used for the view
	*/
	virtual void CalcSceneView(
		FSceneViewFamily* ViewFamily, 
		const FVector2D& Offset, 
		const FVector2D& Size, 
		const FViewport* Viewport,
		const UPostProcessChain* InPostProcessChain,
		const FPostProcessSettings* InPostProcessSettings
		);

	/**
	* Detach all components that were added to the scene
	*/
	virtual void DetachAllComponents();

	// FSceneInterface interface

	/** 
	* Adds a new primitive component to the scene
	* 
	* @param Primitive - primitive component to add
	*/
	virtual void AddPrimitive(UPrimitiveComponent* Primitive);
	/** 
	* Removes a primitive component from the scene
	* 
	* @param Primitive - primitive component to remove
	*/
	virtual void RemovePrimitive(UPrimitiveComponent* Primitive);
	/** 
	* Updates the transform of a primitive which has already been added to the scene. 
	* 
	* @param Primitive - primitive component to update
	*/
	virtual void UpdatePrimitiveTransform(UPrimitiveComponent* Primitive);
	/** 
	* Adds a new light component to the scene
	* 
	* @param Light - light component to add
	*/
	virtual void AddLight(ULightComponent* Light);
	/** 
	* Removes a light component from the scene
	* 
	* @param Light - light component to remove
	*/
	virtual void RemoveLight(ULightComponent* Light);
	/** 
	* Updates the transform of a light which has already been added to the scene. 
	*
	* @param Light - light component to update
	*/
	virtual void UpdateLightTransform(ULightComponent* Light);
	/** 
	 * Updates the color and brightness of a light which has already been added to the scene. 
	 *
	 * @param Light - light component to update
	 */
	virtual void UpdateLightColorAndBrightness(ULightComponent* Light);
	/**
	* Release this scene and remove it from the rendering thread
	*/
	virtual void Release();
	/**
	* Retrieves the lights interacting with the passed in primitive and adds them to the out array.
	*
	* @param	Primitive				Primitive to retrieve interacting lights for
	* @param	RelevantLights	[out]	Array of lights interacting with primitive
	*/
	virtual void GetRelevantLights( UPrimitiveComponent* Primitive, TArray<const ULightComponent*>* RelevantLights ) const;
	/** 
	* Indicates if sounds in this should be allowed to play. 
	* 
	* @return TRUE if audio playback is allowed
	*/
	virtual UBOOL AllowAudioPlayback();
	/**
	* Indicates if hit proxies should be processed by this scene
	*
	* @return TRUE if hit proxies should be rendered in this scene.
	*/
	virtual UBOOL RequiresHitProxies() const;
	/**
	* Get the optional UWorld that is associated with this scene
	* 
	* @return UWorld instance used by this scene
	*/
	virtual class UWorld* GetWorld() const;

	// FSceneInterface - not implemented

	virtual void AddSceneCapture(USceneCaptureComponent* CaptureComponent) {}
	virtual void RemoveSceneCapture(USceneCaptureComponent* CaptureComponent) {}
	virtual void AddFluidSurface(UFluidSurfaceComponent* FluidComponent) {}
	virtual void RemoveFluidSurface(UFluidSurfaceComponent* FluidComponent) {}
	virtual const TArray<UFluidSurfaceComponent*>* GetFluidSurfaces() { return NULL; }
	virtual void AddHeightFog(class UHeightFogComponent* FogComponent) {}
	virtual void RemoveHeightFog(class UHeightFogComponent* FogComponent) {}
	virtual void AddFogVolume(const UPrimitiveComponent* MeshComponent) {}
	virtual void AddFogVolume(const UFogVolumeDensityComponent* FogVolumeComponent, const UPrimitiveComponent* MeshComponent) {}
	virtual void RemoveFogVolume(const UPrimitiveComponent* MeshComponent) {}
	virtual void AddWindSource(class UWindDirectionalSourceComponent* WindComponent) {}
	virtual void RemoveWindSource(class UWindDirectionalSourceComponent* WindComponent) {}
	virtual void AddLightEnvironment(const class ULightEnvironmentComponent* LightEnvironment) {}
	virtual void RemoveLightEnvironment(const class ULightEnvironmentComponent* LightEnvironment) {}
	virtual const TArray<class FWindSourceSceneProxy*>& GetWindSources_RenderThread() const
	{
		static TArray<class FWindSourceSceneProxy*> NullWindSources;
		return NullWindSources;
	}

private:

	/** Internally allocated scene used for rendering */
	FSceneInterface* Scene;	
	/** List of primitives added to this scene */
	TArray<UPrimitiveComponent*> Primitives;
	/** List of lights added to this scene */
	TArray<ULightComponent*> Lights;
	/** Camera location for view matrix calcs */
	FVector ViewLocation;
	/** Field of view for projection calcs */
	FLOAT fFOV;
};

#endif // __CANVASSCENE_H__

