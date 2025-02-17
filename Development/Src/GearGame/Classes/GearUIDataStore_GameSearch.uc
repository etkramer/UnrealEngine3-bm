/**
 * Game search data store for Gears 2.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUIDataStore_GameSearch extends UIDataStore_OnlineGameSearch;

DefaultProperties
{
	ActiveSearchIndex=0
	GameSearchCfgList.Empty

	GameSearchCfgList.Add((SearchName="GameSearchParty",GameSearchClass=class'GearGame.GearPartyGameSearch',DefaultGameSettingsClass=class'GearGame.GearPartyGameSettings',SearchResultsProviderClass=class'Engine.UIDataProvider_Settings'))
	GameSearchCfgList.Add((SearchName="GameSearchVersus",GameSearchClass=class'GearGame.GearVersusGameSearch',DefaultGameSettingsClass=class'GearGame.GearVersusGameSettings',SearchResultsProviderClass=class'Engine.UIDataProvider_Settings'))
}
