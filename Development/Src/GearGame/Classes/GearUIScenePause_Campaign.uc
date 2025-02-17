/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUIScenePause_Campaign extends GearUIScenePause_Base;


/**
 * Called just before the scene perform its first update.  This is first time it's guaranteed to be safe to call functions
 * which require a valid viewport, such as SetPosition/GetPosition
 */
function InitialSceneUpdate()
{
	local UILabel lblTitle;
	local UIImage imgTitleBar;
	local WorldInfo WI;

	imgTitleBar = UIImage(FindChild('imgTitleBar', true));
	lblTitle = UILabel(imgTitleBar.FindChild('lblTitle',true));

	WI = GetWorldInfo();
	if ( WI != None )
	{
		if ( WI.Pauser != None )
		{
			imgTitleBar.SetVisibility(true);
			lblTitle.SetDataStoreBinding("<Strings:GearGame.Generic.Paused>");
		}
		else
		{
			imgTitleBar.SetVisibility(false);
		}
	}
}

/** Callback when the user confirms leaving - overloaded to leave breadcrumb for the campaign lobby */
function bool OnReturnToLobbyConfirmed( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	if ( SelectedInputAlias == 'GenericAccept' )
	{
		// Leave breadcrumb for the campaign lobby to know we are coming back from the game
		SetTransitionValue("ExitGame", "Yes");
	}

	return Super.OnReturnToLobbyConfirmed(Sender, SelectedInputAlias, PlayerIndex);
}

/** @return true if the return to lobby button can be used in this pause menu */
function bool CanReturnToLobby()
{
	local OnlineSubsystem OnlineSub;
	local OnlineGameSettings GameSettings;
	local WorldInfo WI;
	local int NumLocals;
	local GearPC PC;
	local LocalPlayer LP;

	WI = GetWorldInfo();
	// If there are 2 local players, then this is splitscreen
	foreach WI.LocalPlayerControllers(class'GearPC', PC)
	{
		NumLocals++;
	}
	// Also verify that a Live coop session is still logged in
	if (NumLocals == 1)
	{
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None && OnlineSub.GameInterface != None)
		{
			GameSettings = OnlineSub.GameInterface.GetGameSettings('Party');
			// If this is a Live match
			if (!GameSettings.bIsLanMatch)
			{
				// Then make sure they are signed in
				foreach WI.LocalPlayerControllers(class'GearPC', PC)
				{
					LP = LocalPlayer(PC.Player);
					if (LP != None)
					{
						if (OnlineSub.PlayerInterface.GetLoginStatus(LP.ControllerId) < LS_LoggedIn)
						{
							// Not logged in, so force a quit to main menu
							return false;
						}
					}
				}
			}
		}
	}
	// Handle both networked and splitscreen games
	return WI.NetMode == NM_ListenServer || NumLocals == 2;
}

defaultproperties
{
	LoopingAudioSoundResource=SoundCue'Interface_Audio.Menu.MenuPauseLoopCue'
	OnInitialSceneUpdate=InitialSceneUpdate
}

