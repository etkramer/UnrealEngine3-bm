/**
 * This class displays more detailed information about a single player's leaderboard stats.  It is opened from the main leaderboards scene in
 * the War Journal.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUISceneWJ_LeaderboardDetails extends GearUISceneFE_PlayerStatBase;

/** reference to the leaderboards data store  */
var	transient	GearLeaderboardsDataStoreBase	LeaderboardsDataStore;

var	transient	OnlineStatsRead				CurrentLeaderboard;

var	transient	int							LeaderboardRow;


/* == Delegates == */

/* == Natives == */

/* == Events == */

/* == UnrealScript == */
/**
 * Links this scene to the leaderboard row specified.
 *
 * @param	InLeaderboardsDataStore the datastore to bind to
 * @param	inLeaderboardIndex	the index into the leaderboards data store for the player to retrieve stats details for.
 */
function SetLeaderboardIndex( GearLeaderboardsDataStoreBase InLeaderboardsDataStore, int LeaderboardIndex )
{
	// Find the gears leaderboard so we can tell it to refresh
	LeaderboardsDataStore = InLeaderboardsDataStore;
	CurrentLeaderboard = LeaderboardsDataStore.StatsRead;

	LeaderboardRow = LeaderboardIndex;
	PlayerNetworkId = CurrentLeaderboard.Rows[LeaderboardRow].PlayerId;

	SetGametype();

	InitializePlayerData();
}

/**
 * Converts the gametype id from the EGameModeFilters enum to the EGearMPTypes enum.
 */
function SetGametype()
{
	local int GameModeId;

	GameModeId = INDEX_NONE;

	// Figure out which set of leaderboards to use by gamemode
	LeaderboardsDataStore.LeaderboardSettings.GetStringSettingValue(LF_GameMode,GameModeId);
	switch (GameModeId)
	{
		case GMF_Horde:
			GameType = eGEARMP_CombatTrials;
			break;

		case GMF_Warzone:
			GameType = eGEARMP_Warzone;
			break;

		case GMF_Execution:
			GameType = eGEARMP_Execution;
			break;

		case GMF_Guardian:
			GameType = eGEARMP_KTL;
			break;

		case GMF_Meatflag:
			GameType = eGEARMP_CTM;
			break;

		case GMF_Annex:
			GameType = eGEARMP_Annex;
			break;

		case GMF_Koth:
			GameType = eGEARMP_KOTH;
			break;

		case GMF_Wingman:
			GameType = eGEARMP_Wingman;
			break;

		default:
			`log(`location @ "unhandled game mode ID retrieved from leaderboard settings (" $ LeaderboardsDataStore.LeaderboardSettings $ "):" @ GetEnum(enum'EGameModeFilters',GameModeId));
			break;
	}
}

/* === GearUISceneFE_PlayerStatBase interface === */
/**
 * Assigns delegates in important child widgets to functions in this scene class.
 */
function SetupCallbacks()
{
	Super.SetupCallbacks();
	ButtonBar.SetButtonCallback('CloseWindow', OnBackPress);
}

/** Returns the string of the name of the player using the netId as the key */
function String GetPlayerName()
{
	if ( CurrentLeaderboard != None )
	{
		return CurrentLeaderboard.Rows[LeaderboardRow].NickName;
	}

	return Super.GetPlayerName();
}

/** Returns the string of the rank of the player using the netId as the key */
function String GetPlayerRank()
{
	return " ";
}

/** Returns the string of the leaderboard rank of the player, using the netId as the key */
function String GetPlayerLeaderRank()
{
	local int Rank;

	if ( CurrentLeaderboard != None )
	{
		Rank = CurrentLeaderboard.GetRankForPlayer(PlayerNetworkId);
		return Rank == 0 ? " " : string(Rank);
	}

	return Super.GetPlayerLeaderRank();
}

/** Returns the string of the stat for the player using the netId as the key */
function String GetPlayerStatValue(bool bIsLifetime,int StatId)
{
	local int Value;
	local float FloatValue;

	if (CurrentLeaderboard != None)
	{
		switch (StatId)
		{
			case STATS_COLUMN_KILLDEATH_RATIO:
			case STATS_COLUMN_POINTSPERMATCH_RATIO:
			case STATS_COLUMN_POINTSPERROUND_RATIO:

				// Get the specified column from the stats code (last match)
				if (CurrentLeaderboard.GetFloatStatValueForPlayer(PlayerNetworkId,StatId,FloatValue))
				{
					if (FloatValue != 0)
					{
						// Snip off everything except one digit past the decimel
						return TruncateFloatString(string(FloatValue));
					}
					return "--";
				}
				break;

			default:
				// Get the specified column from the stats code (last match)
				if (CurrentLeaderboard.GetIntStatValueForPlayer(PlayerNetworkId,StatId,Value))
				{
					return string(Value);
				}
				break;
		}
	}

	return "";
}

/**
 * Returns the string of the stat at index StatIdx
 *
 * @param StatIdx - the index in the PlayerData.StatsData array
 * @param bIsLifetime - whether the stat is for the entire lifetime of playing this gametype or just the last time you played
 */
function String GetStatString(int StatIdx, bool bIsLifetime)
{
	// we always want the lifetime stat
	if ( bIsLifetime )
	{
		return Super.GetStatString(StatIdx, true);
	}

	return "";
}

DefaultProperties
{
	// create an spacer between the generic stats and the game-specific stats
	bRequiresNetwork=true
	bRequiresOnlineService=true

	// GearUISceneWJ_Base
	SceneSkin=UISkin'UI_Art_WarJournal.UI_WarJournal'
	bRequiresProfile=true

	// GearUISceneWJ_PopupBase
	bRenderParentScenes=true
}
