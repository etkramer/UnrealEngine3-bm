/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTUIScene_Hud extends UTUIScene
	native(UI);


/****************************************************************************************
								 Support Functions
****************************************************************************************/

/**
 * Returns the UTHUD associated with this scene
 */
native function UTHUD GetPlayerHud();

defaultproperties
{
	bDisplayCursor=false
	bPauseGameWhileActive=false
	SceneInputMode=INPUTMODE_None
	bRenderParentScenes=true
	bCloseOnLevelChange=true
	bExemptFromAutoClose=true

}

/*
Things to be ported

- Progress Messages

*/
