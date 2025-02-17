/**
 * This player controller class is used for the campaign lobby.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPC_SP extends GearPC;

`include(GearOnlineConstants.uci)

/**
 * Used when a host is telling a client to return to their party host from the
 * current session. It looks for the session named 'Party' and does a travel to
 * it. If it's not available, it just does a "disconnect"
 */
reliable client function ClientReturnToParty()
{
	local string URL;

	bIgnoreNetworkMessages = true;

	// make sure we're not paused
	if ( MyGearHUD != None && MyGearHUD.PauseUISceneInstance != None )
	{
		MyGearHUD.UnPauseGame(MyGearHUD.PauseUISceneInstance);
	}
	else
	{
		SetPause(false);
	}

	// explicit call to show loading movie because this transition occurs via ServerTravel, which doesn't always call ClientTravel
	ClientShowLoadingMovie(true);
	// only do this for the first player in split-screen sessions.
	if (IsPrimaryPlayer())
	{
		if (OnlineSub != None &&
			OnlineSub.GameInterface != None &&
			OnlineSub.PlayerInterface != None)
		{
			// Find the party settings to verify that a party is registered
			if (OnlineSub.GameInterface.GetGameSettings('Party') != None)
			{
				// Now see if we are the party host or not
				if (IsPartyLeader())
				{
					UnregisterRemotePlayers();
					// We are the party host so create the session and listen for returning clients
					URL = GetPartyMapName() $ "?game=" $ GetPartyGameTypeName() $ "?listen";
					// Transition to being the party host without notifying clients and traveling absolute
					WorldInfo.ServerTravel(URL,true,true);
				}
				else
				{
					// We are joining so grab the connect string to use
					if (OnlineSub.GameInterface.GetResolvedConnectString('Party',URL))
					{
						ClientTravel(URL, TRAVEL_Absolute);
					}
				}
			}
			// If we are splitscreen, then return to the lobby
			else if (GetSplitscreenPlayerCount() > 1)
			{
				URL = GetPartyMapName() $ "?game=" $ GetPartyGameTypeName();
				// Transition to being the party host without notifying clients and traveling absolute
				WorldInfo.ServerTravel(URL,true,true);
			}
			else
			{
				ConsoleCommand("disconnect");
			}
		}
		else
		{
			ConsoleCommand("disconnect");
		}
	}
}

/**
 * Returns the party map name for this game
 */
static function string GetPartyMapName()
{
	return "GearStart";
}

/**
 * Returns the party game info name for this game
 */
static function string GetPartyGameTypeName()
{
	return "GearGameContent.GearCampaignLobbyGame";
}

/**
 * Replicated function to use when setting the rich presence string for coop
 *
 * @param PawnClass the pawn this player is playing as
 * @param Chapter the chapter point to convert to act/chapter information
 */
reliable client function ClientSetCoopRichPresence(class<Pawn> PawnClass,EChapterPoint Chapter)
{
	local int ActId;
	local int NormalizedChapter;
	local GearCampaignActData ActData;
	local LocalPlayer LP;
	local array<LocalizedStringSetting> StringSettings;
	local array<SettingsProperty> Properties;
	local SettingsProperty Prop;

	LP = LocalPlayer(Player);
	if (LP != None &&
		OnlineSub != None &&
		OnlineSub.PlayerInterface != None)
	{
		// Get the act this chapter is in
		ActData = class'GearUIDataStore_GameResource'.static.GetActDataProviderUsingChapter(Chapter);
		if (ActData != None)
		{
			ActId = ActData.ActType + 1;
		}
		Prop.PropertyId = PROPERTY_ACTNUM;
		Prop.AdvertisementType = ODAT_OnlineService;
		class'Settings'.static.SetSettingsDataInt(Prop.Data,ActId);
		Properties.AddItem(Prop);
		// Now get the chapter number to display in rich presence
		NormalizedChapter = class'GearUIDataStore_GameResource'.static.GetActChapterProviderIndexFromChapterId(Chapter) + 1;
		Prop.PropertyId = PROPERTY_CHAPTERNUM;
		class'Settings'.static.SetSettingsDataInt(Prop.Data,NormalizedChapter);
		Properties.AddItem(Prop);
		// Finally set the person they are playing as
		StringSettings.Length = 1;
		StringSettings[0].AdvertisementType = ODAT_OnlineService;
		StringSettings[0].Id = CONTEXT_COOPCHARNAME;
		StringSettings[0].ValueIndex = PawnClass == class'GearPawn_COGDom' ? CONTEXT_COOPCHARNAME_DOM : CONTEXT_COOPCHARNAME_MARCUS ;

		OnlineSub.PlayerInterface.SetOnlineStatus(LP.ControllerId,CONTEXT_PRESENCE_COOPPRESENCE,StringSettings,Properties);
	}
}

/**
 * Determines whether sign-in status changes are allowed in the current match.  Not supported when in the front-end.
 *
 * @return	TRUE to allow players to be created or removed in response to login-status changes; FALSE to end the game and
 *			return to main menu if login-status has changed.
 */
function bool IsSigninChangeAllowed()
{
	return true;
}

/**
 * login changed handler.  Usually returns the player to the title screen.
 *
 * @param	NewStatus		The new login state of the player.
 */
function OnLoginChanged(ELoginStatus NewStatus)
{
	local LocalPlayer LP;
	local GameUISceneClient GameSceneClient;
	local bool bReturnToMainMenu;

	if ( IsPrimaryPlayer() )
	{
		LP = LocalPlayer(Player);
		if ( NewStatus == LS_NotLoggedIn )
		{
			`log(`location @ "player signed out - returning to main menu");
			bReturnToMainMenu = true;
		}
		else if ( HasSigninChanged() )
		{
			`log(`location @ "player" @ LP @ "(" $ LP.ControllerId $ ")" @ "signed into another profile without signing out; returning to main menu.");
			bReturnToMainMenu = true;
		}

		if ( bReturnToMainMenu )
		{
			class'GearUIScene_Base'.static.ProcessIllegalPlayerLoginStatusChange(LP.ControllerId, true);
			return;
		}
	}

	// now that we've verified that it wasn't the primary player that signed-out, synchronize players again - this time we allow removal
	GameSceneClient = class'UIInteraction'.static.GetSceneClient();
	if ( GameSceneClient != None )
	{
		GameSceneClient.SynchronizePlayers();
	}
}

