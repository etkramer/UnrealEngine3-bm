/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Base class for all leaderboard writes. Each specific leadeboard will override
 */
class GearLeaderboardWriteBase extends OnlineStatsWrite
	abstract;

`include(GearOnlineConstants.uci)

/**
 * Copies the values from the PRI
 *
 * @param PRI the replication info to process
 * @param RoundsPlayed the total number of rounds for the match
 * @param RoundsWon how many rounds this PRI won
 * @param RoundsLost how many rounds this PRI lost
 */
function UpdateFromPRI(GearPRI PRI,int RoundsPlayed,int RoundsWon,int RoundsLost);

/**
 * Sets the stat to the specified value
 *
 * @param Value the number of to set the stat to
 */
function SetPoints(int Value)
{
	SetIntStat(PROPERTY_POINTS,Value);
	// Make sure to create a non-zero rating value
	if (Value == 0)
	{
		Value = -1;
	}
	SetIntStat(PROPERTY_POINTSRATED,Value);
}

defaultproperties
{
	RatingId=PROPERTY_POINTSRATED
}
