/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Writes the warzone specific leaderboards
 */
class GearLeaderboardWarzoneWrite extends GearLeaderboardVersusWrite;

defaultproperties
{
	// The leaderboards the properties are written to
	ViewIds=(STATS_VIEW_WARZONEALLTIMEPRIVATE)
	ArbitratedViewIds=(STATS_VIEW_WARZONEDAILY,STATS_VIEW_WARZONEWEEKLY,STATS_VIEW_WARZONEMONTHLY,STATS_VIEW_WARZONEALLTIME)
}