/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Writes the koth specific leaderboards
 */
class GearLeaderboardWriteKoth extends GearLeaderboardWriteAnnex;

/**
 * Copies the values from the PRI
 *
 * @param PRI the replication info to process
 * @param RoundsPlayed the total number of rounds for the match
 * @param RoundsWon how many rounds this PRI won
 * @param RoundsLost how many rounds this PRI lost
 */
function UpdateFromPRI(GearPRI PRI,int RoundsPlayed,int RoundsWon,int RoundsLost)
{
	Super.UpdateFromPRI(PRI,RoundsPlayed,RoundsWon,RoundsLost);

	SetIntStat(PROPERTY_RINGPOINTS,PRI.Score_GameSpecific3);
}

defaultproperties
{
	// The leaderboards the properties are written to
	ViewIds=(STATS_VIEW_KOTHALLTIMEPRIVATE)
	ArbitratedViewIds=(STATS_VIEW_KOTHDAILY,STATS_VIEW_KOTHWEEKLY,STATS_VIEW_KOTHMONTHLY,STATS_VIEW_KOTHALLTIME)

	// These are the stats we are collecting
	Properties=((PropertyId=PROPERTY_RINGPOINTS,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_RINGBREAK,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_RINGCAPTURE,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_MATCHESPLAYED,Data=(Type=SDT_Int32,Value1=1)),(PropertyId=PROPERTY_MATCHESWON,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_MATCHESLOST,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_ROUNDSPLAYED,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_ROUNDSWON,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_ROUNDSLOST,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_ROUNDSDRAW,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_KILLS,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_DEATHS,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_REVIVES,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_TAKEDOWNS,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_POINTS,Data=(Type=SDT_Int32,Value1=0)),(PropertyId=PROPERTY_POINTSRATED,Data=(Type=SDT_Int32,Value1=0)))
}