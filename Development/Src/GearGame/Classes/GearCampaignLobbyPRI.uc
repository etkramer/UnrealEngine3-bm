/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearCampaignLobbyPRI extends GearPRI;


/** The difficulty the player is configured with */
var EDifficultyLevel CampDifficulty;


replication
{
	if (bNetDirty)
		CampDifficulty;
}

defaultproperties
{
	SessionName="Party"
}
