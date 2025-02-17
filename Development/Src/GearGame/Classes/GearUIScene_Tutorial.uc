/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearUIScene_Tutorial extends GearUIScene_Base;


/** Delegate called when the opening animation is finished */
delegate OnTutorialOpenAnimationComplete();

/** Handler for the completion of this scene's opening animation */
function OnOpenAnimationComplete( UIScreenObject Sender, name AnimName, int TrackTypeMask )
{
	Super.OnOpenAnimationComplete(Sender, AnimName, TrackTypeMask);
	OnTutorialOpenAnimationComplete();
}

DefaultProperties
{
	SceneStackPriority=GEAR_SCENE_PRIORITY_TUTORIAL
	bNeverFocus=true
	bCaptureMatchedInput=false
	bFlushPlayerInput=false
	bRenderParentScenes=true
	bSaveSceneValuesOnClose=false
	SceneInputMode=INPUTMODE_None
	SceneRenderMode=SPLITRENDER_PlayerOwner
}
