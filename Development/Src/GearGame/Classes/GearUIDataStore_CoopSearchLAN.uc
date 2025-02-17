/**
 * Game search data store for Gears 2.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUIDataStore_CoopSearchLAN extends GearUIDataStore_GameSearch;

DefaultProperties
{
	Tag=LANCoopSearch
	ActiveSearchIndex=0
	GameSearchCfgList.Empty

	GameSearchCfgList.Add((SearchName="LANGameSearchParty",GameSearchClass=class'GearGame.GearCoopLANGameSearch',DefaultGameSettingsClass=class'GearGame.GearCoopGameSettings',SearchResultsProviderClass=class'Engine.UIDataProvider_Settings'))
}
