/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPartyPRI extends GearPRI;

/** Whether the player is a guest of another player or not */
var bool bIsGuest;

replication
{
	if (bNetDirty)
		bIsGuest;
}

simulated function bool ShouldBroadCastWelcomeMessage(optional bool bExiting)
{
	return FALSE;
}

defaultproperties
{
	SessionName="Party"
}
