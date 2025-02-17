/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * NOTE: This class will normally be code generated, but the tool hasn't
 * been written yet
 */
class GearLeaderboardHorde extends GearLeaderboardBase;

/**
 * Gets the high score for the player specified
 *
 * @param PlayerId the unique id of the player to look up
 *
 * @return the value of the points for that player
 */
function int GetHighScoreForPlayer(UniqueNetId PlayerId)
{
	local int HighScore;
	GetIntStatValueForPlayer(PlayerId,STATS_COLUMN_HORDE_POINTS,HighScore);
	return HighScore;
}

/**
 * Used to copy stats to this leaderboard object
 *
 * @param PRI the replication info to process
 */
function UpdateFromPRI(GearPRI PRI)
{
	AddPlayer(PRI.PlayerName,PRI.UniqueId);
	SetIntStatValueForPlayer(PRI.UniqueId,STATS_COLUMN_HORDE_WAVE,GearGRI(PRI.WorldInfo.GRI).RoundCount);
	SetIntStatValueForPlayer(PRI.UniqueId,STATS_COLUMN_HORDE_POINTS,PRI.Score);

	// Set the leaderboard based off of map name
	UpdateViewIdFromName(PRI.WorldInfo.GetMapName(true));
}

/**
 * Updates the view ids being written to based off of the map that the match was in
 *
 * @param MapName the name of the map to look up
 */
function UpdateViewIdFromName(string MapName)
{
	ViewId = class'GearLeaderboardSettings'.static.GetViewIdFromMapName(name(MapName));
	`Log("Using Horde view id of "$ViewId);
}

defaultproperties
{
	ViewId=30

	SortColumnId=STATS_COLUMN_HORDE_POINTS

	ColumnIds=(STATS_COLUMN_HORDE_WAVE,STATS_COLUMN_HORDE_POINTS)

	// The metadata for the columns
	ColumnMappings(0)=(Id=STATS_COLUMN_HORDE_WAVE,Name="Wave")
	ColumnMappings(1)=(Id=STATS_COLUMN_HORDE_POINTS,Name="Points")
}
