/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Writes the execution specific leaderboards
 */
class GearLeaderboardWriteExecution extends GearLeaderboardVersusWrite;

defaultproperties
{
	// The leaderboards the properties are written to
	ViewIds=(STATS_VIEW_EXECUTIONALLTIMEPRIVATE)
	ArbitratedViewIds=(STATS_VIEW_EXECUTIONDAILY,STATS_VIEW_EXECUTIONWEEKLY,STATS_VIEW_EXECUTIONMONTHLY,STATS_VIEW_EXECUTIONALLTIME)
}