/**
 * Base class for all PlayerController classes used by the menus to enable special functionality.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearMenuPC extends GearPC;

`include(GearOnlineConstants.uci)


simulated event PostBeginPlay()
{
	Super.PostBeginPlay();

	if( GearGame(WorldInfo.Game) != None && GearGame(WorldInfo.Game).bDoLocScreenShotTest == TRUE )
	{
		SetTimer( 30.0f, FALSE, nameof(Automation_PressStart) );
	}
}

simulated function Automation_PressStart()
{
	Sentinel_PressStartKeyAtStartMenu();

	SetTimer( 2.0f, FALSE, nameof(Sentinel_DoBugitWithLang) );
}

reliable client event ClientSetViewTarget( Actor A, optional ViewTargetTransitionParams TransitionParams )
{
	//@fixme - mini hack, ignore viewtarget changes for menu pc's since we do it locally
	if (CameraActor(ViewTarget) == None)
	{
		Super.ClientSetViewTarget(A,TransitionParams);
	}
}

function SetViewTarget( Actor NewViewTarget, optional ViewTargetTransitionParams TransitionParams  )
{
	//@fixme - mini hack, ignore viewtarget changes for menu pc's since we do it locally
	if (CameraActor(ViewTarget) == None)
	{
		Super.SetViewTarget(NewViewTarget,TransitionParams);
	}
}

/**
 * Calls a function on the client to open the lobby scene.  Called by either the gameinfo or the game replication info when the
 * player connects.
 */
function OpenLobbyScene( UIScene LobbySceneResource )
{
	if ( LobbySceneResource != None && IsPrimaryPlayer() )
	{
		ClientOpenScene(LobbySceneResource);
	}
}


reliable client function ClientPartyLobbyClosed()
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local UIMessageBoxBase InstancedScene;

	ButtonAliases.AddItem('GenericAccept');

	GameSceneClient = class'UIRoot'.static.GetSceneClient();
	GameSceneClient.ShowUIMessage('PartyLobbyClosed', "<Strings:GearGameUI.MessageBoxStrings.PartyDisbanded_Title>",
		"<Strings:GearGameUI.MessageBoxStrings.PartyDisbanded_Message>", "", ButtonAliases, None, LocalPlayer(Player), InstancedScene);

	InstancedScene.bCloseOnLevelChange = false;
	InstancedScene.bExemptFromAutoClose = true;
	InstancedScene.SceneStackPriority++;
	ReturnToMainMenu();
}

/**
 * Called from client to server to indicate that the client has received the correct gametype and is now ready for the gametype's options.
 * Sends the server's values for all gametype options to this client.
 */
reliable server function ServerAcknowledgeGameTypeUpdate()
{
	local int i, SettingId, PropertyValueId;
	local GearUIDataStore_GameSettings SettingsDS;
	local OnlineGameSettings GameSettings;

	SettingsDS = GearUIDataStore_GameSettings(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameSettings'.default.Tag, None, LocalPlayer(Player)));

	// show that the function was called
	if ( SettingsDS != None )
	{
		GameSettings = SettingsDS.GetCurrentGameSettings();
		if ( GameSettings != None )
		{
			// for any issues arising from problems in the settings object itself
			// first, figure out which game settings object to use
			for ( i = 0; i < GameSettings.LocalizedSettings.Length; i++ )
			{
				// show what we're sending
				SettingId = GameSettings.LocalizedSettings[i].Id;
				`log("  CurrentSettings[" $ i $ "]:" @ `showvar(SettingId,Id) @ "(" $ GameSettings.GetStringSettingName(SettingId) $ ")" @ `showvar(GameSettings.LocalizedSettings[i].ValueIndex,ValueId),,'RON_DEBUG');
				if ( SettingId == class'GearVersusGameSettings'.const.CONTEXT_VERSUSMODES )
				{
					// skip the gametype option as that one is sent first by ClientUpdateGameSettingValue
					continue;
				}

				// send the value to the client
				ClientUpdateGameSettingValue( SettingId, GameSettings.LocalizedSettings[i].ValueIndex );
			}

			for ( i = 0; i < GameSettings.Properties.Length; i++ )
			{
				SettingId = GameSettings.Properties[i].PropertyId;
				GameSettings.GetPropertyValueId(SettingId, PropertyValueId);

				// send the value to the client
				ClientUpdateGamePropertyValue(SettingId,PropertyValueId);
			}
		}
		else
		{
			`warn(`location@"(" $ `showobj(Self) $ "): Couldn't find current game settings object!");
		}
	}
	else
	{
		`warn(`location@"(" $ `showobj(Self) $ "): Unable to resolve the online game settings data store!");
	}
}

/**
 * Receives an updated value from the match host for one of the current game's online game settings' LocalizedSettings elements.  Applies
 * that value to the client's local copy of the current game's online settings object.
 *
 * @param	SettingId	the id for the Setting.LocalizedSetting element that is being updated.
 * @param	ValueId		the id for the new value of the setting
 */
reliable client function ClientUpdateGameSettingValue( int SettingId, int ValueId )
{
	local GearUIDataStore_GameSettings SettingsDS;
	local OnlineGameSettings GameSettings;

	SettingsDS = GearUIDataStore_GameSettings(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameSettings'.default.Tag, None, None));
	if ( SettingsDS != None )
	{
		// special case when replicating the game type - we need to let the server know when we've received the updated gametype so that
		// the server can send the remainder of the settings.
		if ( SettingId == class'GearVersusGameSettings'.const.CONTEXT_VERSUSMODES )
		{
			SettingsDS.SetCurrentByContextId(ValueId);

			GameSettings = SettingsDS.GetCurrentGameSettings();
			if ( GameSettings != None )
			{
				GameSettings.SetStringSettingValue(SettingId, ValueId, false);
				NotifyLobbyOfGameSettingValueChange(SettingId, ValueId);
			}
			else
			{
				`warn(`location@"(" $ `showobj(Self) $ "): Couldn't find current game settings object!");
			}

			// let the server know we're ready to receive this new gametype's settings
			ServerAcknowledgeGameTypeUpdate();
		}
		else
		{
			GameSettings = SettingsDS.GetCurrentGameSettings();
			if ( GameSettings != None )
			{
				GameSettings.SetStringSettingValue(SettingId, ValueId, false);
				NotifyLobbyOfGameSettingValueChange(SettingId, ValueId);
			}
			else
			{
				`warn(`location@"(" $ `showobj(Self) $ "): Couldn't find current game settings object!");
			}
		}
	}
	else
	{
		`warn(`location@"(" $ `showobj(Self) $ "): Unable to resolve the online game settings data store!");
	}
}

/**
 * Receives an updated value from the match host for one of the current game's online game settings' Properties elements.  Applies
 * that value to the client's local copy of the current game's online settings object.
 *
 * @param	PropertyId	the id for the Setting.Properties element that is being updated.
 * @param	ValueId		the id for the new value of the setting
 */
reliable client function ClientUpdateGamePropertyValue( int PropertyId, int ValueId )
{
	local GearUIDataStore_GameSettings SettingsDS;
	local OnlineGameSettings GameSettings;

	SettingsDS = GearUIDataStore_GameSettings(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameSettings'.default.Tag, None, None));
	if ( SettingsDS != None )
	{
		GameSettings = SettingsDS.GetCurrentGameSettings();
		if ( GameSettings != None )
		{
			GameSettings.SetPropertyValueId(PropertyId, ValueId);
			NotifyLobbyOfGamePropertyValueChange(PropertyId, ValueId);
		}
		else
		{
			`warn(`location@"(" $ `showobj(Self) $ "): Couldn't find current game settings object!");
		}
	}
	else
	{
		`warn(`location@"(" $ `showobj(Self) $ "): Unable to resolve the online game settings data store!");
	}
}

/** Notifies the lobby scene that a game setting value has changed */
function NotifyLobbyOfGameSettingValueChange( int SettingId, int ValueId )
{
	local GameUISceneClient GameSceneClient;
	local GearUISceneFELobby_Base Scene;

	GameSceneClient = class'UIRoot'.static.GetSceneClient();
	if ( GameSceneClient != None )
	{
		foreach GameSceneClient.AllActiveScenes(class'GearUISceneFELobby_Base', Scene, true)
		{
			Scene.OnGameSettingValueChange(SettingId, ValueId);
		}
	}
}

/** Notifies the lobby scene that a game setting property has changed */
function NotifyLobbyOfGamePropertyValueChange( int SettingId, int ValueId )
{
	local GameUISceneClient GameSceneClient;
	local GearUISceneFELobby_Base LobbyScene;

	GameSceneClient = class'UIRoot'.static.GetSceneClient();
	if ( GameSceneClient != None )
	{
		foreach GameSceneClient.AllActiveScenes(class'GearUISceneFELobby_Base', LobbyScene, true)
		{
			LobbyScene.OnGamePropertyValueChange(SettingId, ValueId);
		}
	}
}

/**
 * Sets the rich presence to the default value (in the menu)
 */
reliable client function ClientSetOnlineStatus()
{
	SetRichPresenceString(CONTEXT_PRESENCE_MENUPRESENCE);
}

/* === GearPC interface === */

/** @return whether we want the spectator UI scene to be displayed */
function bool WantsSpectatorUI()
{
	// never show the spectator UI in the front end.
	return false;
}

