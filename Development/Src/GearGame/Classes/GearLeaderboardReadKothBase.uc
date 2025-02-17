/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Holds all of the columns for koth
 */
class GearLeaderboardReadKothBase extends GearLeaderboardReadAnnexBase;

/**
 * Used to copy stats to this leaderboard object
 *
 * @param PRI the replication info to process
 */
function UpdateFromPRI(GearPRI PRI)
{
	Super.UpdateFromPRI(PRI);

	SetIntStatValueForPlayer(PRI.UniqueId,STATS_COLUMN_KOTH_RINGPOINTS,PRI.Score_GameSpecific3);
}

defaultproperties
{
	ColumnIds=(STATS_COLUMN_KILLS,STATS_COLUMN_DEATHS,STATS_COLUMN_REVIVES,STATS_COLUMN_TAKEDOWNS,STATS_COLUMN_POINTS,STATS_COLUMN_MATCHESPLAYED,STATS_COLUMN_MATCHESWON,STATS_COLUMN_MATCHESLOST,STATS_COLUMN_ROUNDSPLAYED,STATS_COLUMN_ROUNDSWON,STATS_COLUMN_ROUNDSLOST,STATS_COLUMN_ROUNDSDRAW,STATS_COLUMN_MEATFLAG_MEATFLAGCAPTURES,STATS_COLUMN_ANNEX_CAPTURES,STATS_COLUMN_ANNEX_BREAKS,STATS_COLUMN_KOTH_RINGPOINTS)

	// The metadata for the columns
	ColumnMappings(14)=(Id=STATS_COLUMN_KOTH_RINGPOINTS,Name="RingPoints")
}
