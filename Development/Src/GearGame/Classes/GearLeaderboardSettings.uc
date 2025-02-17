/**
 * Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This class holds the game modes and leaderboard time line filters
 */
class GearLeaderboardSettings extends GearLeaderboardSettingsBase;

defaultproperties
{
	LocalizedSettings(LF_GameMode)=(Id=LF_GameMode,ValueIndex=GMF_Warzone)
	LocalizedSettingsMappings(LF_GameMode)=(Id=LF_GameMode,Name="GameMode",ValueMappings=((Id=GMF_Warzone),(Id=GMF_Execution),(Id=GMF_Guardian),(Id=GMF_Annex),(Id=GMF_Koth),(Id=GMF_Meatflag),(Id=GMF_Wingman)))

	LocalizedSettings(LF_TimePeriod)=(Id=LF_TimePeriod,ValueIndex=TF_AllTime)
	LocalizedSettingsMappings(LF_TimePeriod)=(Id=LF_TimePeriod,Name="Time Period",ValueMappings=((Id=TF_Weekly),(Id=TF_Monthly),(Id=TF_AllTime)))

	LocalizedSettings(LF_PlayerFilterType)=(Id=LF_PlayerFilterType,ValueIndex=PF_Friends)
	LocalizedSettingsMappings(LF_PlayerFilterType)=(Id=LF_PlayerFilterType,Name="Player Filter",ValueMappings=((Id=PF_Player),(Id=PF_CenteredOnPlayer),(Id=PF_Friends),(Id=PF_TopRankings)))
}
