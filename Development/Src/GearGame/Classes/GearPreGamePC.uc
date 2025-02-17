/**
 * This class is used in the pre-game lobbies.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearPreGamePC extends GearMenuPC;

/** Opens a UI scene */
client reliable event UIScene ClientOpenScene( UIScene SceneReference, optional byte OverrideEmulateButtonPress )
{
	SetTimer(1.0,true,nameof(DelayedSceneOpen));
	if (IsPrimaryPlayer())
	{
		return Super.ClientOpenScene(SceneReference,OverrideEmulateButtonPress);
	}
}

/** Delayed so that the client has time to have the PRI replicate */
function DelayedSceneOpen()
{
	local GearPreGameLobbyPRI PRI;

	PRI = GearPreGameLobbyPRI(PlayerReplicationInfo);
	if (PRI != None)
	{
		if (!PRI.bLobbySceneOpened)
		{
			PRI.ServerNotifyLobbySceneOpened();
		}
		else
		{
			SetTimer(0.0,false,nameof(DelayedSceneOpen));
		}
	}
}

/**
 * Used to assign any players on the host to their teams since they don't call ServerSetUniquePlayerId
 */
event InitUniquePlayerId()
{
	local GearPreGameLobbyGame_Base PreGame;

	Super.InitUniquePlayerId();

	PreGame = GearPreGameLobbyGame_Base(WorldInfo.Game);
	// Have the game use the recent players list to update this PC with skill/team
	if (PreGame != None)
	{
		PreGame.UpdateTeamAndSkillData(self);
	}
}

/**
 * Registers the unique id of the player with the server so it can be replicated
 * to all clients.
 *
 * @param UniqueId the buffer that holds the unique id
 * @param bWasInvited whether the player was invited to play or is joining via search
 */
reliable server function ServerSetUniquePlayerId(UniqueNetId UniqueId,bool bWasInvited)
{
	local GearPreGameLobbyGame_Base PreGame;

	Super.ServerSetUniquePlayerId(UniqueId,bWasInvited);

	if (!bPendingDestroy)
	{
		PreGame = GearPreGameLobbyGame_Base(WorldInfo.Game);
		// Have the game use the recent players list to update this PC with skill/team
		if (PreGame != None)
		{
			PreGame.UpdateTeamAndSkillData(self);
		}
		// If this is not a public session then re-register with the party session
		if (OnlineSub != None &&
			OnlineSub.GameInterface != None &&
			OnlineSub.GameInterface.GetGameSettings('Party') != None &&
			!OnlineSub.GameInterface.GetGameSettings('Game').bUsesArbitration)
		{
			OnlineSub.GameInterface.RegisterPlayer('Party',UniqueId,false);
		}
	}
}

/**
 * Displays a message when the physical network is lost during a networked game
 *
 * @param bIsConnected the new connection state
 */
function OnLinkStatusChange(bool bIsConnected)
{
	if (!bIsConnected)
	{
		// Ignore this change if standalone
		if (WorldInfo.NetMode != NM_Standalone)
		{
			ShowMessageBox("<Strings:GearGameUI.MessageBoxErrorStrings.LostLink_Title>",
				"<Strings:GearGameUI.MessageBoxErrorStrings.LostLink_Message>",
				'ConnectionError');
			ReturnToMainMenu();
		}
	}
}

/**
 * Common method for the host to use to set the rich presence string
 *
 * @param StringId the rich presence string to set
 * @param MpType the mp context that we use
 * @param MapNameId the map name id to set
 */
reliable client function ClientSetVersusRichPresenceString(byte StringId,byte MpType,byte MapNameId)
{
	local LocalPlayer LP;
	local array<LocalizedStringSetting> StringSettings;
	local array<SettingsProperty> Properties;

	LP = LocalPlayer(Player);
	if (LP != None &&
		OnlineSub != None &&
		OnlineSub.PlayerInterface != None)
	{
		StringSettings.Length = 2;
		// Set the map name for rich presence
		StringSettings[0].AdvertisementType = ODAT_OnlineService;
		StringSettings[0].Id = CONTEXT_MAPNAME;
		StringSettings[0].ValueIndex = MapNameId;
		// Add the game mode
		StringSettings[1].AdvertisementType = ODAT_OnlineService;
		StringSettings[1].Id = CONTEXT_VERSUSMODES;
		StringSettings[1].ValueIndex = int(MpType);

		OnlineSub.PlayerInterface.SetOnlineStatus(LP.ControllerId,StringId,StringSettings,Properties);
	}
}

/**
 * Tell this player to return to the main menu.  This version takes care of cleaning up the player's online sessions.
 */
simulated function ReturnToMainMenu()
{
	local OnlineGameSettings GameSettings;
	local bool bShouldTravel;

	bShouldTravel = true;
	bIsReturningFromMatch = true;
	bIgnoreNetworkMessages = true;
	if (OnlineSub != None && OnlineSub.GameInterface != None)
	{
		GameSettings = OnlineSub.GameInterface.GetGameSettings('Game');
		if (GameSettings != None && WorldInfo.NetMode == NM_Client)
		{
			// This is a connection loss in the pre-game lobby, so just return to party instead
			if (GameSettings.bUsesArbitration)
			{
				// It's ok to clean up immediately
				OnlineSub.GameInterface.DestroyOnlineGame('Game');
				bShouldTravel = false;
				// Now return to party
				ClientReturnToParty();
			}
			else
			{
				// It's ok to clean up immediately
				OnlineSub.GameInterface.DestroyOnlineGame('Game');
				if (OnlineSub.GameInterface.GetGameSettings('Party') != None)
				{
					OnlineSub.GameInterface.DestroyOnlineGame('Party');
				}
			}
		}
		else
		{
			// It's ok to clean up immediately
			OnlineSub.GameInterface.DestroyOnlineGame('Game');
			if (OnlineSub.GameInterface.GetGameSettings('Party') != None)
			{
				OnlineSub.GameInterface.DestroyOnlineGame('Party');
			}
		}
	}

	if (bShouldTravel)
	{
		ClientTravel("?closed", TRAVEL_Absolute);
	}
}

defaultproperties
{

}
