/**
 * Gears2-specific version of UIGameInfoSummary.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearGameInfoSummary extends UIGameInfoSummary;

/**
 * URL options (key or key/value pairs) which must be appended to the URL to access this gametype (i.e. UseExecutionRules, etc.)
 */
var	config	array<string>	AdditionalOptions;

/** ID of the game type this is in EGearMPTypes of GearTypes.uc */
var config EGearMPTypes	MPGameMode;

/** The icon representing this game mode in the UI */
var config string IconPathName;

DefaultProperties
{

}
