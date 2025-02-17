/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUIScenePause_Gameover extends GearUIScenePause_Base
	Config(UI);


/**
* Called after this screen object's children have been initialized
* Overloaded to initialized the references to this scene's widgets
*/
event PostInitialize()
{
	local GameUISceneClient GameSceneClient;
	local int SceneIdx;

	// Close the pause scene if it's open
	GameSceneClient = GetSceneClient();
	SceneIdx = GameSceneClient.FindSceneIndexByTag('UI_Game_Pause', GetPlayerOwner());
	if (SceneIdx != INDEX_NONE)
	{
		GameSceneClient.CloseSceneAtIndex(SceneIdx, false);
	}

	Super.PostInitialize();
}

/** Callback when the load last checkpoint button is pressed - Overloaded so we can skip the "are you sure" check */
function bool OnLastCheckpointClicked( UIScreenObject EventObject, int PlayerIndex )
{
	local GearPC MyGearPC;

	if (!IsBlockingInput())
	{
		// Make sure the looping audio stops since we're heading for a load screen
		if (LoopingAudioAC != none)
		{
			LoopingAudioAC.Stop();
			LoopingAudioAC = none;
		}

		if ( PlayerOwner != None && PlayerOwner.Actor != None )
		{
			MyGearPC = GearPC(PlayerOwner.Actor);
			MyGearPC.LoadCheckpoint();
		}
	}

	return true;
}

/**
 * Called when the scene is activated so we can turn the music on and set the time for text fade
 *		Start the post process effect
 */
function OnSceneActivatedCallback( UIScene ActivatedScene, bool bInitialActivation )
{
	local GearPC MyGearPC;

	Super.OnSceneActivatedCallback(ActivatedScene, bInitialActivation);

	if ( PlayerOwner != None && PlayerOwner.Actor != None )
	{
		MyGearPC = GearPC(PlayerOwner.Actor);
		if ( MyGearPC != None )
		{
			MyGearPC.StartPostProcessOverride( eGPP_Gameover );
		}
	}
}

/**
 * Called when the scene is closed so we can stop the music
 *		Stop the post process effect
 */
function OnSceneDeactivatedCallback( UIScene DeactivatedScene )
{
	local GearPC MyGearPC;

	Super.OnSceneDeactivatedCallback(DeactivatedScene);

	if ( PlayerOwner != None && PlayerOwner.Actor != None )
	{
		MyGearPC = GearPC(PlayerOwner.Actor);
		if ( MyGearPC != None )
		{
			MyGearPC.StopPostProcessOverride( eGPP_Gameover );

			if ( MyGearPC.MyGearHUD != None && MyGearPC.MyGearHUD.GameoverUISceneInstance == Self )
			{
				MyGearPC.MyGearHUD.GameoverUISceneInstance = None;
			}
		}
	}
}

/** Overloaded to unpause game */
function bool CloseScene( optional UIScene SceneToClose=Self, optional bool bCloseChildScenes=true, optional bool bForceCloseImmediately )
{
	local GearPC MyGearPC;

	if ( PlayerOwner != None && PlayerOwner.Actor != None )
	{
		MyGearPC = GearPC(PlayerOwner.Actor);
		if ( MyGearPC != None && MyGearPC.MyGearHud != None )
		{
			MyGearPC.MyGearHud.UnPauseGame( SceneToClose );
		}
	}

	return Super.CloseScene( SceneToClose, bCloseChildScenes, bForceCloseImmediately );
}

/** Whether to close the scene through normal input (B and Start) */
function bool CanCloseSceneWithInput()
{
	return false;
}

defaultproperties
{
	TotalInputBlockTime=0.0f
	bExemptFromAutoClose=true
	OnSceneActivated=OnSceneActivatedCallback
	OnSceneDeactivated=OnSceneDeactivatedCallback
	SceneRenderMode=SPLITRENDER_Fullscreen
	LoopingAudioSoundResource=SoundCue'Music_Score_G2B.Music.G2SJ_ObjectiveFailCue'
}
