/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Tab page for a user's general stats.
 */
class UTUITabPage_StatsGeneral extends UTUITabPage_StatsPage
	placeable;

/** Post initialization event - Setup widget delegates.*/
event PostInitialize()
{
	Super.PostInitialize();

	// Set the button tab caption.
	SetDataStoreBinding("<Strings:UTGameUI.Stats.General>");

	RefreshingLabel.SetVisibility(false);
}

defaultproperties
{

}