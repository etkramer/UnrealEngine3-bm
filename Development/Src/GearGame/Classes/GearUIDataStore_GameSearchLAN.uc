/**
 * Game search data store for Gears 2.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUIDataStore_GameSearchLAN extends GearUIDataStore_GameSearch;

DefaultProperties
{
	Tag=LANGameSearch
	ActiveSearchIndex=0
	GameSearchCfgList.Empty

	GameSearchCfgList.Add((SearchName="LANGameSearchParty",GameSearchClass=class'GearGame.GearLANPartySearch',DefaultGameSettingsClass=class'GearGame.GearPartyGameSettings',SearchResultsProviderClass=class'Engine.UIDataProvider_Settings'))
}
