/*=============================================================================
	CanvasScene.cpp: Canvas scene rendering
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "CanvasScene.h"

/**
* Constructor
*/
FCanvasScene::FCanvasScene()
:	ViewLocation(-200,0,0)
,	fFOV(90)
{
	// Create the scene for attaching the canvas components. This scene doesnt have a world.
	Scene = AllocateScene( NULL, FALSE, FALSE );
}

/**
* Get the number of primitive components that have been added to this scene
* @return number of primitives
*/
INT FCanvasScene::GetNumPrimitives()
{
	return Primitives.Num();
}

/**
* Get the number of light components that have been added to this scene
* @return number of lights
*/
INT FCanvasScene::GetNumLights()
{
	return Lights.Num();
}

/** 
* Set the Camera location for view matrix calcs 
*/
void FCanvasScene::SetViewLocaton(const FVector& InViewLocation)
{
	ViewLocation = InViewLocation;
}
/** 
* Set the Field of view for projection calcs 
*/
void FCanvasScene::SetFOV(FLOAT InFOV)
{
	fFOV = InFOV;
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
void FCanvasScene::CalcSceneView(FSceneViewFamily* ViewFamily, 
						   const FVector2D& Offset, 
						   const FVector2D& Size, 
						   const FViewport* Viewport,
						   const UPostProcessChain* InPostProcessChain,
						   const FPostProcessSettings* InPostProcessSettings
						   )
{
	check(ViewFamily);
	check(Viewport);

	// Compute the view's screen rectangle.
	INT X = appTrunc(Offset.X * Viewport->GetSizeX());
	INT Y = appTrunc(Offset.Y * Viewport->GetSizeY());
	UINT SizeX = appTrunc(Size.X * Viewport->GetSizeX());
	UINT SizeY = appTrunc(Size.Y * Viewport->GetSizeY());

	UINT UnscaledSizeX = SizeX;
	UINT UnscaledSizeY = SizeY;

	// Take screen percentage option into account if percentage != 100.
	GSystemSettings.ScaleScreenCoords(X,Y,SizeX,SizeY);
	
	// use the same transform that's used by the canvas
	FMatrix ViewMatrix( FCanvas::CalcViewMatrix(UnscaledSizeX,UnscaledSizeY,fFOV) );
	FMatrix ProjectionMatrix( FCanvas::CalcProjectionMatrix(SizeX,SizeY,fFOV,NEAR_CLIPPING_PLANE) );

	// add the new view to the scene
	FSceneView* View = new FSceneView(
		ViewFamily,
		NULL,
		-1,
		NULL,
		NULL,
		NULL,
		InPostProcessChain,
		InPostProcessSettings,
		NULL,
		NULL,
		X,
		Y,
		SizeX,
		SizeY,
		ViewMatrix,
		ProjectionMatrix,
		FLinearColor::Black,
		FLinearColor(0,0,0,0),
		FLinearColor::White,
		TSet<UPrimitiveComponent*>()
		);
	ViewFamily->Views.AddItem(View);
}

/**
* Detach all components that were added to the scene
*/
void FCanvasScene::DetachAllComponents()
{
	// make a copy in case the component removes itself from our array.
	TArray<UPrimitiveComponent*> PrimCopy = Primitives;

	// Remove all the attached prims
	for( INT Idx=0; Idx < PrimCopy.Num(); Idx++ )
	{
		PrimCopy(Idx)->ConditionalDetach();
	}
	// make a copy in case the component removes itself from our array.
	TArray<ULightComponent*> LightsCopy = Lights;

	// Remove all the attached lights
	for( INT Idx=0; Idx < LightsCopy.Num(); Idx++ )
	{
		LightsCopy(Idx)->ConditionalDetach();
	}

	// empty lists
	Primitives.Empty();
	Lights.Empty();
}

/** 
* Adds a new primitive component to the scene
* 
* @param Primitive - primitive component to add
*/
void FCanvasScene::AddPrimitive(UPrimitiveComponent* Primitive)
{
	check(Primitive);	

	// all canvas scene primitives should be in the foreground DPG
	Primitive->DepthPriorityGroup = GIsEditor ? SDPG_UnrealEdForeground : SDPG_Foreground;

	Primitives.AddUniqueItem(Primitive);
	Scene->AddPrimitive(Primitive);
}

/** 
* Removes a primitive component from the scene
* 
* @param Primitive - primitive component to remove
*/
void FCanvasScene::RemovePrimitive(UPrimitiveComponent* Primitive)
{
	check(Primitive);
	Primitives.RemoveItem(Primitive);
	Scene->RemovePrimitive(Primitive);
}

/** 
* Updates the transform of a primitive which has already been added to the scene. 
* 
* @param Primitive - primitive component to update
*/
void FCanvasScene::UpdatePrimitiveTransform(UPrimitiveComponent* Primitive)
{
	check(Primitive);
	Scene->UpdatePrimitiveTransform(Primitive);
}

/** 
* Adds a new light component to the scene
* 
* @param Light - light component to add
*/
void FCanvasScene::AddLight(ULightComponent* Light)
{
	check(Light);
	Lights.AddUniqueItem(Light);
	Scene->AddLight(Light);
}

/** 
* Removes a light component from the scene
* 
* @param Light - light component to remove
*/
void FCanvasScene::RemoveLight(ULightComponent* Light)
{
	check(Light);
	Lights.RemoveItem(Light);
	Scene->RemoveLight(Light);
}

/** 
* Updates the transform of a light which has already been added to the scene. 
*
* @param Light - light component to update
*/
void FCanvasScene::UpdateLightTransform(ULightComponent* Light)
{
	check(Light);
	Scene->UpdateLightTransform(Light);
}

/** 
 * Updates the color and brightness of a light which has already been added to the scene. 
 *
 * @param Light - light component to update
 */
void FCanvasScene::UpdateLightColorAndBrightness(ULightComponent* Light)
{
	check(Light);
	Scene->UpdateLightColorAndBrightness(Light);
}

/**
* Release this scene and remove it from the rendering thread
*/
void FCanvasScene::Release()
{
	Primitives.Empty();
	Lights.Empty();
	Scene->Release();
}

/**
* Retrieves the lights interacting with the passed in primitive and adds them to the out array.
*
* @param	Primitive				Primitive to retrieve interacting lights for
* @param	RelevantLights	[out]	Array of lights interacting with primitive
*/
void FCanvasScene::GetRelevantLights( UPrimitiveComponent* Primitive, TArray<const ULightComponent*>* RelevantLights ) const
{
	Scene->GetRelevantLights(Primitive,RelevantLights);
}

/** 
* Indicates if sounds in this should be allowed to play. 
* 
* @return TRUE if audio playback is allowed
*/
UBOOL FCanvasScene::AllowAudioPlayback()
{
	return Scene->AllowAudioPlayback();
}

/**
* Indicates if hit proxies should be processed by this scene
*
* @return TRUE if hit proxies should be rendered in this scene.
*/
UBOOL FCanvasScene::RequiresHitProxies() const
{
	return Scene->RequiresHitProxies();
}

/**
* Get the optional UWorld that is associated with this scene
* 
* @return UWorld instance used by this scene
*/
UWorld* FCanvasScene::GetWorld() const
{
	return Scene->GetWorld();
}


