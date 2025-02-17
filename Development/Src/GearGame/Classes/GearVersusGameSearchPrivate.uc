/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Used to find matches that a party can participate in
 */
class GearVersusGameSearchPrivate extends GearVersusGameSearch;

defaultproperties
{
	// The class to use when creating
	GameSettingsClass=class'GearGame.GearVersusGameSettingsPrivate'

	// Private matches aren't ranked
	bUsesArbitration=false
}
