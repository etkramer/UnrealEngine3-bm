//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================

#include "UTEditor.h"
#include "UTGameUIClasses.h"

IMPLEMENT_CLASS(UUTEditorUISceneClient);

/*=========================================================================================
  UUTEditorUISceneClient - We have our own scene client so that we can provide Tick/Prerender
  passes.  We also use this object as a repository for all UTFontPool objects
  ========================================================================================= */

/**
 * Render all the active scenes
 */
void UUTEditorUISceneClient::RenderScenes( FCanvas* Canvas )
{
	// get player field of view
	FLOAT FOV = GEngine->GamePlayers(0)->Actor->eventGetFOVAngle();

	// Set 3D Projection
	Canvas->SetBaseTransform(FCanvas::CalcBaseTransform3D(RenderViewport->GetSizeX(),
		RenderViewport->GetSizeY(),FOV,NEAR_CLIPPING_PLANE));
	{
		Super::RenderScenes(Canvas);
	}
	// Restore 2D Projection
	Canvas->SetBaseTransform(FCanvas::CalcBaseTransform2D(RenderViewport->GetSizeX(),
		RenderViewport->GetSizeY()));
}

/**
 * We override the Render_Scene so that we can provide a PreRender pass
 * @See UGameUISceneClient for more details
 */
void UUTEditorUISceneClient::Render_Scene(FCanvas* Canvas, UUIScene *Scene, EUIPostProcessGroup UIPostProcessGroup)
{
	if( UIPostProcessGroup != UIPostProcess_None )
	{
		return;
	}

	// UTUIScene's support a pre-render pass.  If this is a UTUIScene, start that pass and clock it

	UUTUIScene* UTScene = Cast<UUTUIScene>(Scene);
	if ( UTScene )
	{
		UTScene->PreRender(Canvas);
	}

	// Render the scene

	Super::Render_Scene(Canvas,Scene,UIPostProcessGroup);
}
