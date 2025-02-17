/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Reads the skill for a player
 */
class GearLeaderboardVersusSkill extends OnlineStatsRead
	native(Online);

`include(GearOnlineConstants.uci)

/**
 * Copies the results of the stats read to the search object
 *
 * @param Search the search object to populate with skill data
 */
native function CopySkillDataToSearch(OnlineGameSearch Search);

/**
 * Gets the skill value for the player specified
 *
 * @param PlayerId the unique id of the player to look up
 *
 * @return the skill value for that player
 */
native function int GetSkillForPlayer(UniqueNetId PlayerId);

/**
 * Populates the player reservation structure with the skill data contained herein
 *
 * @param PlayerId the player to read the skill data for
 * @param PlayerRes the reservation that is populated with the skill data
 */
native function FillPlayerResevation(UniqueNetId PlayerId,out PlayerReservation PlayerRes);

defaultproperties
{
	ViewId=STATS_VIEW_SKILL_RANKED_VERSUS

	ColumnIds=(STATS_COLUMN_SKILL,STATS_COLUMN_GAMESPLAYED,STATS_COLUMN_MU,STATS_COLUMN_SIGMA)

	// The metadata for the columns
	ColumnMappings(0)=(Id=STATS_COLUMN_SKILL,Name="Skill")
	ColumnMappings(1)=(Id=STATS_COLUMN_GAMESPLAYED,Name="GamesPlayed")
	ColumnMappings(2)=(Id=STATS_COLUMN_MU,Name="Mu")
	ColumnMappings(3)=(Id=STATS_COLUMN_SIGMA,Name="Sigma")
}
