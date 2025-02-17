/**
 * Provides data for a UT3 game mode.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTUIDataProvider_GameModeInfo extends UTUIResourceDataProvider
	native(UI)
	PerObjectConfig;

cpptext
{
	/** @return 	TRUE if this data provider represents the campaign gametype */
	virtual UBOOL IsFiltered();
}

/** full path to the game class */
var config string GameMode;

/** The map that this game mode defaults to */
var config string DefaultMap;

/** Settings class to use for this game mode. */
var config string GameSettingsClass;

/** Search class to use for this game mode. */
var config string GameSearchClass;

/** Option set to use for the game mode. */
var config string OptionSet;

/** Friendly displayable name to the player. */
var config localized string FriendlyName;

/** Localized description of the game mode */
var config localized string Description;

/** Markup text used to display the preview image for the game mode. */
var config string PreviewImageMarkup;

/** Prefixes of the maps this gametype supports. */
var config string Prefixes;

/** Image that the icon of this game mode is on. */
var config string IconImage;

/** Whether or not this a campaign game mode. */
var config bool bIsCampaign;

/** UV Coordinates for the icon. */
var config float IconU;
var config float IconV;
var config float IconUL;
var config float IconVL;

defaultproperties
{
	bSearchAllInis=true
}
