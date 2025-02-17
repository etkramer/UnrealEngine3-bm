/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearHordePRI extends GearPRI;

/** Highest score the player has gotten in Horde */
var int HighScore;

replication
{
	// sent to everyone
	if (bNetDirty)
		HighScore;
}
