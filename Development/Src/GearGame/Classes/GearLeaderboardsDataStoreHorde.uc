/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This class holds the game modes and leaderboard time line filters
 */
class GearLeaderboardsDataStoreHorde extends GearLeaderboardsDataStoreBase;

/**
 * Gears specific function that figures out what type of search to do
 */
function SetStatsReadInfo()
{
	local int PlayerFilterType;
	local int HordeMap;
	local GearLeaderboardHorde HordeStats;

	// Read which map
	LeaderboardSettings.GetStringSettingValue(LF_MapType,HordeMap);
	HordeStats = GearLeaderboardHorde(StatsReadObjects[0]);
	// Update the view to read the correct leaderboard for this map
	HordeStats.ViewId = GearLeaderboardSettingsBase(LeaderboardSettings).GetViewIdFromMapId(HordeMap);
	// All time since that is all that makes sense
	LeaderboardSettings.SetStringSettingValue(LF_TimePeriod,TF_AllTime,false);
	LeaderboardSettings.SetStringSettingValue(LF_GameMode,GMF_Horde,false);

	// It's horde, all the time
	StatsRead = StatsReadObjects[0];

	// Read the set of players they want to view
	LeaderboardSettings.GetStringSettingValue(LF_PlayerFilterType,PlayerFilterType);
	switch (PlayerFilterType)
	{
		case PF_Player:
			CurrentReadType = SFT_Player;
			break;
		case PF_CenteredOnPlayer:
			CurrentReadType = SFT_CenteredOnPlayer;
			break;
		case PF_Friends:
			CurrentReadType = SFT_Friends;
			break;
		case PF_TopRankings:
			CurrentReadType = SFT_TopRankings;
			break;
	}
}

defaultproperties
{
	LeaderboardSettingsClass=class'GearGame.GearLeaderboardSettingsHorde'

// Horde stats
	StatsReadClasses(0)=class'GearGame.GearLeaderboardHorde'

	Tag=GearLeaderboardsHorde
}
