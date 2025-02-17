/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class GearTeamPlayerStart extends PlayerStart;

/** Team index of the players who can spawn at this playerstart. */
var() int TeamIndex;

/** The first round that a player can spawn from this playerstart. */
var() int FirstRoundForSpawning;

/** The last round that a player can spawn from this playerstart.  If left alone only the FirstRoundForSpawning value will be used. */
var() int LastRoundForSpawning;

defaultproperties
{
	FirstRoundForSpawning=-1
	LastRoundForSpawning=-1
}
