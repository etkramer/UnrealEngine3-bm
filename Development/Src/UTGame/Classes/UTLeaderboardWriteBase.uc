
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/** The class that handles the writing of UT leaderboard stats */
/** After calling CopyAllStats() this class should have all relevant */
/** stats ready to be given to the online stat writing system */

class UTLeaderboardWriteBase extends OnlineStatsWrite
    dependson(UTPlayerReplicationInfo)
    native;

`include(UTOnlineConstants.uci)

/** Maps a stat property value to a stat string */
struct native StatMappingEntry
{
	/** Human readable form of the Id */
	var const name StatName;

	/** Property Id for the given string */
	var const int Id;
};

var const array<int> PureViewIds;

/** Array of mappings from PRI stat names to Online Stat properties */
var protected array<StatMappingEntry> StatNameToStatIdMapping;


native function PrintDebugInformation(OnlineSubsystem OnlineSubsystem);

/** Given a PRI Stat name, return the PropertyId and PropertyDataType */
native function bool GetPropertyIdFromStatType(name StatName, out int StatId, out ESettingsDataType StatType);

/** Set an int stat value to the online properties array given a PRI stat name and its value */
native function bool SetIntStatFromMapping(Name StatName, int StatValue);

/** Set an float stat value to the online properties array given a PRI stat name and its value */
native function bool SetFloatStatFromMapping(name StatName, float StatValue);

/** Tell the stats writing that we are a pure server to switch where we record stats */
native function SetPureServerMode(const bool bIsPureServer);

//Copies all relevant PRI game stats into the Properties struct of the OnlineStatsWrite
//There can be many more stats in the PRI than what is in the Properties table (on Xbox for example)
//If the Properties table does not contain the entry, the data is not written
function CopyAllStats(UTPlayerReplicationInfo PRI);

defaultproperties
{
	// Sort the leaderboard by this property
	RatingId=PROPERTY_LEADERBOARDRATING

	// Views being written to depending on type of match (ranked or player)
	ViewIds=(STATS_VIEW_DM_PLAYER_ALLTIME)
	PureViewIds=(STATS_VIEW_DM_RANKED_ALLTIME)
	ArbitratedViewIds=(STATS_VIEW_DM_RANKED_ALLTIME)
}
