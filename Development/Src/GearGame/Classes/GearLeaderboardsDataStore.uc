/**
 * Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This class holds the game modes and leaderboard time line filters
 */
class GearLeaderboardsDataStore extends GearLeaderboardsDataStoreBase;

/**
 * Gears specific function that figures out what type of search to do
 */
function SetStatsReadInfo()
{
	local int ObjectIndex;
	local int GameModeId;
	local int PlayerFilterType;
	local int TimePeriodId;
	local int HordeMap;
	local GearLeaderboardHorde HordeStats;

	// Figure out which set of leaderboards to use by gamemode
	LeaderboardSettings.GetStringSettingValue(LF_GameMode,GameModeId);

	// Read the time period
	LeaderboardSettings.GetStringSettingValue(LF_TimePeriod,TimePeriodId);

	switch (GameModeId)
	{
		case GMF_Horde:
			ObjectIndex = 2;
			// Read which map
			LeaderboardSettings.GetStringSettingValue(LF_MapType,HordeMap);
			HordeStats = GearLeaderboardHorde(StatsReadObjects[ObjectIndex]);
			// Update the view to read the correct leaderboard for this map
			HordeStats.ViewId = GearLeaderboardSettingsBase(LeaderboardSettings).GetViewIdFromMapId(HordeMap);
			// Force friends only, all time since that is all that makes sense
			LeaderboardSettings.SetStringSettingValue(LF_TimePeriod,TF_AllTime,false);
			LeaderboardSettings.SetStringSettingValue(LF_PlayerFilterType,PF_Friends);
			break;

		case GMF_Warzone:
			// Now choose based off of the time period
			switch (TimePeriodId)
			{
				case TF_Weekly:
					ObjectIndex = STATS_VIEW_WARZONEWEEKLY;
					break;
				case TF_Monthly:
					ObjectIndex = STATS_VIEW_WARZONEMONTHLY;
					break;
				case TF_AllTime:
					ObjectIndex = STATS_VIEW_WARZONEALLTIME;
					break;
			}
			break;

		case GMF_Execution:
			ObjectIndex = STATS_VIEW_EXECUTIONWEEKLY + TimePeriodId;
			break;

		case GMF_Guardian:
			ObjectIndex = STATS_VIEW_GUARDIANWEEKLY + TimePeriodId;
			break;

		case GMF_Meatflag:
			ObjectIndex = STATS_VIEW_MEATFLAGWEEKLY + TimePeriodId;
			break;

		case GMF_Annex:
			ObjectIndex = STATS_VIEW_ANNEXWEEKLY + TimePeriodId;
			break;

		case GMF_Koth:
			ObjectIndex = STATS_VIEW_KOTHWEEKLY + TimePeriodId;
			break;

		case GMF_Wingman:
			ObjectIndex = STATS_VIEW_WINGMANWEEKLY + TimePeriodId;
			break;
	}

	// Choose the read object based upon which filter they selected
	StatsRead = StatsReadObjects[ObjectIndex];

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
	LeaderboardSettingsClass=class'GearGame.GearLeaderboardSettings'

// Horde stats
	StatsReadClasses(2)=class'GearGame.GearLeaderboardHorde'

// Warzone stats
	StatsReadClasses(STATS_VIEW_WARZONEWEEKLY)=class'GearGame.GearLeaderboardWarzoneWeekly'
	StatsReadClasses(STATS_VIEW_WARZONEMONTHLY)=class'GearGame.GearLeaderboardWarzoneMonthly'
	StatsReadClasses(STATS_VIEW_WARZONEALLTIME)=class'GearGame.GearLeaderboardWarzoneAllTime'

// Execution stats
	StatsReadClasses(STATS_VIEW_EXECUTIONWEEKLY)=class'GearGame.GearLeaderboardExecutionWeekly'
	StatsReadClasses(STATS_VIEW_EXECUTIONMONTHLY)=class'GearGame.GearLeaderboardExecutionMonthly'
	StatsReadClasses(STATS_VIEW_EXECUTIONALLTIME)=class'GearGame.GearLeaderboardExecutionAllTime'

// Guardian stats
	StatsReadClasses(STATS_VIEW_GUARDIANWEEKLY)=class'GearGame.GearLeaderboardGuardianWeekly'
	StatsReadClasses(STATS_VIEW_GUARDIANMONTHLY)=class'GearGame.GearLeaderboardGuardianMonthly'
	StatsReadClasses(STATS_VIEW_GUARDIANALLTIME)=class'GearGame.GearLeaderboardGuardianAllTime'

// Annex stats
	StatsReadClasses(STATS_VIEW_ANNEXWEEKLY)=class'GearGame.GearLeaderboardAnnexWeekly'
	StatsReadClasses(STATS_VIEW_ANNEXMONTHLY)=class'GearGame.GearLeaderboardAnnexMonthly'
	StatsReadClasses(STATS_VIEW_ANNEXALLTIME)=class'GearGame.GearLeaderboardAnnexAllTime'

// Koth stats
	StatsReadClasses(STATS_VIEW_KOTHWEEKLY)=class'GearGame.GearLeaderboardKothWeekly'
	StatsReadClasses(STATS_VIEW_KOTHMONTHLY)=class'GearGame.GearLeaderboardKothMonthly'
	StatsReadClasses(STATS_VIEW_KOTHALLTIME)=class'GearGame.GearLeaderboardKothAllTime'

// Meatflag stats
	StatsReadClasses(STATS_VIEW_MEATFLAGWEEKLY)=class'GearGame.GearLeaderboardMeatflagWeekly'
	StatsReadClasses(STATS_VIEW_MEATFLAGMONTHLY)=class'GearGame.GearLeaderboardMeatflagMonthly'
	StatsReadClasses(STATS_VIEW_MEATFLAGALLTIME)=class'GearGame.GearLeaderboardMeatflagAllTime'

// Wingman stats
	StatsReadClasses(STATS_VIEW_WINGMANWEEKLY)=class'GearGame.GearLeaderboardWingmanWeekly'
	StatsReadClasses(STATS_VIEW_WINGMANMONTHLY)=class'GearGame.GearLeaderboardWingmanMonthly'
	StatsReadClasses(STATS_VIEW_WINGMANALLTIME)=class'GearGame.GearLeaderboardWingmanAllTime'

	Tag=GearLeaderboardsPublic
}
