/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Tab page for a user's HUD settings.
 */

class UTUITabPage_HUDSettings extends UTUITabPage_Options
	placeable;


/** Post initialization event - Setup widget delegates.*/
event PostInitialize()
{
	Super.PostInitialize();

	// Set the button tab caption.
	SetDataStoreBinding("<Strings:UTGameUI.Settings.HUD>");
}

defaultproperties
{
	bAllowResetToDefaults=true
}

