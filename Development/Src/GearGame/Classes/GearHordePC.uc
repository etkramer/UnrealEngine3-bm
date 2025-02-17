/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Specialized player controller to only write stats when a high score is hit
 */
class GearHordePC extends GearPC
	dependson(GearTeamInfo);

`include(GearOnlineConstants.uci)

/**
 * If the player beats their high score that high score and wave is written
 *
 * @param OnlineStatsWriteClass the stats class to write with
 */
reliable client function ClientWriteLeaderboardStats(class<OnlineStatsWrite> OnlineStatsWriteClass)
{
	local GearHordePRI HordePRI;
	local GearRecentPlayersList PlayersList;
	local GearTeamInfo GearTeam;

	HordePRI = GearHordePRI(PlayerReplicationInfo);
	GearTeam = GearTeamInfo(PlayerReplicationInfo.Team);
	// Don't write scores if this is not a new best or there was an error reading it
	if (HordePRI != None &&
		GearTeam != None &&
		// Negative high scores means error reading previous score
		HordePRI.HighScore >= 0 &&
		GearTeam.TotalScore > HordePRI.HighScore)
	{
		Super.ClientWriteLeaderboardStats(OnlineStatsWriteClass);
		// Mark the new highest score, so continuing doesn't erase scores
		HordePRI.HighScore = GearTeam.TotalScore;
	}
	// Get the recent player data for the last match stats, so we can clear it
	PlayersList = GearRecentPlayersList(OnlineSub.GetNamedInterface('RecentPlayersList'));
	if (PlayersList != None)
	{
		PlayersList.ClearRecentMatchResults();
	}
}

/**
 * Displays the high score for this player
 */
function ShowHighScore()
{
	local string HighScoreString;
	local GearHordePRI HordePRI;

	HordePRI = GearHordePRI(PlayerReplicationInfo);
//@todo RobM -- Loc me
	HighScoreString = "Highest Score "$HordePRI.HighScore;
	MyGearHud.StartDrawingDramaticText(HighScoreString,"",5.0,1.0,1.0);
}

/**
 * Sets the presence to in a party gathering players
 */
reliable client function ClientSetOnlineStatus()
{
	// Should only be called from PostLogin() so default to wave one
	ClientSetHordePresence(1);
}

/**
 * Called whenever StartNextWave() is called to update the rich presence
 *
 * @param Wave the wave that we are in
 */
reliable client function ClientSetHordePresence(int Wave)
{
	local LocalPlayer LP;
	local array<LocalizedStringSetting> StringSettings;
	local array<SettingsProperty> Properties;
	local SettingsProperty Prop;

	LP = LocalPlayer(Player);
	if (LP != None &&
		OnlineSub != None &&
		OnlineSub.PlayerInterface != None)
	{
		StringSettings.Length = 1;
		// Set the map name for rich presence
		StringSettings[0].AdvertisementType = ODAT_OnlineService;
		StringSettings[0].Id = CONTEXT_MAPNAME;
		StringSettings[0].ValueIndex = class'GearLeaderboardSettings'.static.GetMapIdFromName(name(WorldInfo.GetMapName(true)));
		// Add the wave as a property
		Prop.PropertyId = PROPERTY_WAVE;
		Prop.AdvertisementType = ODAT_OnlineService;
		class'Settings'.static.SetSettingsDataInt(Prop.Data,Wave);
		Properties.AddItem(Prop);

		OnlineSub.PlayerInterface.SetOnlineStatus(LP.ControllerId,CONTEXT_PRESENCE_HORDEPRESENCE,StringSettings,Properties);
	}
}

/**
 * total hack - returns false in horde so that we don't pause the game in horde when the host's gamepad is disconnected
 */
function bool AllowPauseForControllerRemovalHack()
{
	return false;
}

/**
 * login changed handler.  Usually returns the player to the title screen.
 *
 * @param	NewStatus		The new login state of the player.
 */
function OnLoginChanged(ELoginStatus NewStatus)
{
	local OnlineGameSettings PartySettings;
	local LocalPlayer LP;
	local UniqueNetId NetId;

	if ( OnlineSub != None && OnlineSub.GameInterface != None )
	{
		PartySettings = OnlineSub.GameInterface.GetGameSettings('Party');
		if ( PartySettings != None && !PartySettings.bIsLanMatch )
		{
			if ( OnlineSub.PlayerInterface != None )
			{
				LP = LocalPlayer(Player);
				OnlineSub.PlayerInterface.GetUniquePlayerId(LP.ControllerId, NetId);
				if ( PlayerReplicationInfo.UniqueId != NetId )
				{
					`log(`location @ "player" @ LP @ "(" $ LP.ControllerId $ ")" @ "signed into another profile without signing out; returning to main menu.");
					class'GearUIScene_Base'.static.ProcessIllegalPlayerLoginStatusChange(LP.ControllerId, true);
					return;
				}
			}
		}
	}

	Super.OnLoginChanged(NewStatus);
}

