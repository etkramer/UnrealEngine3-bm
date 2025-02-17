/**
 * Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This class holds the game modes and leaderboard time line filters
 */
class GearLeaderboardSettingsBase extends Settings
	// Use the playlist ini since it is downloaded all the time
	config(Playlist);

`include(GearOnlineConstants.uci)

/** The types of game modes that can have leaderboards read from */
enum EGameModeFilters
{
	GMF_Warzone,
	GMF_Execution,
	GMF_Guardian,
	GMF_Wingman,
	GMF_Annex,
	GMF_Koth,
	GMF_Meatflag,
	GMF_Horde
};

/**
 * This list is the set of different filters that can be used when selecting which leaderboard to read
 */
enum ELeaderboardFilters
{
	LF_GameMode,
	LF_TimePeriod,
	LF_PlayerFilterType,
	// Only valid for Horde
	LF_MapType
};

/**
 * This list is the set of time periods that can be queried
 */
enum ETimeFilters
{
	TF_Weekly,
	TF_Monthly,
	TF_AllTime
};

/**
 * This is the set of players that should be returned by the query
 */
enum EPlayerFilters
{
	PF_Player,
	PF_CenteredOnPlayer,
	PF_Friends,
	PF_TopRankings
};

/** The enum that exactly matches the context values in the xlast */
enum EMapContexts
{
	CONTEXT_MAPNAME_NOMAP,
	CONTEXT_MAPNAME_MAP1,
	CONTEXT_MAPNAME_MAP2,
	CONTEXT_MAPNAME_MAP3,
	CONTEXT_MAPNAME_MAP4,
	CONTEXT_MAPNAME_MAP5,
	CONTEXT_MAPNAME_MAP6,
	CONTEXT_MAPNAME_MAP7,
	CONTEXT_MAPNAME_MAP8,
	CONTEXT_MAPNAME_MAP9,
	CONTEXT_MAPNAME_MAP10,
	CONTEXT_MAPNAME_MAP11,
	CONTEXT_MAPNAME_MAP12,
	CONTEXT_MAPNAME_MAP13,
	CONTEXT_MAPNAME_MAP14,
	CONTEXT_MAPNAME_MAP15,
	CONTEXT_MAPNAME_MAP16,
	CONTEXT_MAPNAME_MAP17,
	CONTEXT_MAPNAME_MAP18,
	CONTEXT_MAPNAME_MAP19,
	CONTEXT_MAPNAME_MAP20,
	CONTEXT_MAPNAME_MAP21,
	CONTEXT_MAPNAME_MAP22,
	CONTEXT_MAPNAME_MAP23,
	CONTEXT_MAPNAME_MAP24
};

/** Maps the map name to it's corresponding context value in the xlast file */
struct MapNameToContextMapping
{
	/** The map name being mapped */
	var name MapName;
	/** The corresponding xlast context value */
	var int ContextValue;
};

/** Holds the context values to use for a given map */
var config array<MapNameToContextMapping> MapNamesToContextValues;

/**
 * Determines the localized string id to use for a given map name
 * NOTE: These must match the xlast layouts
 *
 * @param MapName the map we are converting to a index
 */
static function int GetMapIdFromName(name MapName)
{
	local int SearchIndex;

	// Search the mappings array for the corresponding context value
	for (SearchIndex = 0; SearchIndex < default.MapNamesToContextValues.Length; SearchIndex++)
	{
		if (MapName == default.MapNamesToContextValues[SearchIndex].MapName)
		{
			return default.MapNamesToContextValues[SearchIndex].ContextValue;
		}
	}
	// If it's not found, return an error value
	return 0;
}

/**
 * Determines the localized string id to use for a given map name
 * NOTE: These must match the xlast layouts
 *
 * @param MapId the id of the map we are converting to a name
 */
static function name GetMapNameFromId(int MapId)
{
	local int SearchIndex;

	// Search the mappings array for the corresponding context value
	for (SearchIndex = 0; SearchIndex < default.MapNamesToContextValues.Length; SearchIndex++)
	{
		if (MapId == default.MapNamesToContextValues[SearchIndex].ContextValue)
		{
			return default.MapNamesToContextValues[SearchIndex].MapName;
		}
	}
	return 'Unknown';
}

/**
 * Determines the view id to read from/write to for a given map name
 * NOTE: These must match the xlast layouts
 *
 * @param MapName the map we are converting to a index
 */
static function int GetViewIdFromMapName(name MapName)
{
	//#define STATS_VIEW_HORDEMAP1                        30
	// Horde leaderboards start at view id 30
	return 29 + GetMapIdFromName(MapName);
}

/**
 * Determines the view id to read from/write to for a given map id
 * NOTE: These must match the xlast layouts
 *
 * @param MapName the map we are converting to a index
 */
static function int GetViewIdFromMapId(int MapId)
{
	//#define STATS_VIEW_HORDEMAP1                        30
	// Horde leaderboards start at view id 30
	return 29 + MapId;
}
