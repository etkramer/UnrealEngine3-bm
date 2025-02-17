/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This class holds the game modes and leaderboard time line filters
 */
class UTLeaderboardSettings extends Settings
	native;

enum ELeaderboardFilters
{
	LF_GameMode,
	LF_MatchType,
	LF_PlayerFilterType
};

enum EMatchTypeSettings
{
	MTS_Ranked,
	MTS_Player
};

enum EPlayerFilterSettings
{
	PFS_Player,
	PFS_CenteredOnPlayer,
	PFS_Friends,
	PFS_TopRankings
};

/**
 * Moves to the next value index for the specified setting, wraps around to the first element if it hits zero.
 *
 * @param SettingId	SettingId to move to the next value.
 */
function MoveToNextSettingValue(int SettingId)
{
	local int CurrentValueIndex;
	local int TotalValues;

	if(GetStringSettingValue(SettingId, CurrentValueIndex))
	{
		TotalValues = LocalizedSettingsMappings[SettingId].ValueMappings.length;
		SetStringSettingValue(SettingId, (CurrentValueIndex+1)%TotalValues, false);
	}
}

defaultproperties
{
	LocalizedSettings(LF_GameMode)=(Id=LF_GameMode,ValueIndex=CONTEXT_GAME_MODE_DM)
	LocalizedSettingsMappings(LF_GameMode)=(Id=LF_GameMode,Name="GameMode",ValueMappings=((Id=CONTEXT_GAME_MODE_DM), (Id=CONTEXT_GAME_MODE_TDM), (Id=CONTEXT_GAME_MODE_DUEL), (Id=CONTEXT_GAME_MODE_CTF), (Id=CONTEXT_GAME_MODE_VCTF), (Id=CONTEXT_GAME_MODE_WAR)))

	LocalizedSettings(LF_MatchType)=(Id=LF_MatchType,ValueIndex=MTS_Ranked)
	LocalizedSettingsMappings(LF_MatchType)=(Id=LF_MatchType,Name="MatchType",ValueMappings=((Id=MTS_Ranked),(Id=MTS_Player)))

	LocalizedSettings(LF_PlayerFilterType)=(Id=LF_PlayerFilterType,ValueIndex=PFS_Friends)
	LocalizedSettingsMappings(LF_PlayerFilterType)=(Id=LF_PlayerFilterType,Name="PlayerFilter",ValueMappings=((Id=PFS_Player),(Id=PFS_CenteredOnPlayer),(Id=PFS_Friends),(Id=PFS_TopRankings)))
}
