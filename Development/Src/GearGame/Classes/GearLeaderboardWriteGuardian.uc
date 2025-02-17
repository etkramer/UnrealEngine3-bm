/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Writes the Guardian specific leaderboards
 */
class GearLeaderboardWriteGuardian extends GearLeaderboardVersusWrite;

defaultproperties
{
	// The leaderboards the properties are written to
	ViewIds=(STATS_VIEW_GUARDIANALLTIMEPRIVATE)
	ArbitratedViewIds=(STATS_VIEW_GUARDIANDAILY,STATS_VIEW_GUARDIANWEEKLY,STATS_VIEW_GUARDIANMONTHLY,STATS_VIEW_GUARDIANALLTIME)
}