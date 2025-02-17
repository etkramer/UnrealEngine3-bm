/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Base class common to both Horde and versus game types
 */
class GearLeaderboardBase extends OnlineStatsRead;

`include(GearOnlineConstants.uci)

/**
 * Used to copy stats to this leaderboard object
 *
 * @param PRI the replication info to process
 */
function UpdateFromPRI(GearPRI PRI);

