/**
 * Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Settings used to find an online coop game
 * NOTE: This class will normally be code generated
 */
class GearCoopGameSearch extends OnlineGameSearch
	native(Online);

`include(GearOnlineConstants.uci)

/** Sorts the results using ping */
native event SortSearchResults();

defaultproperties
{
	// Which server side query to execute
	Query=(ValueIndex=SESSION_MATCH_QUERY_COOP)

	// The class to use for any search results
	GameSettingsClass=class'GearGame.GearCoopGameSettings'

	// We are only searching for versus sessions
	LocalizedSettings(0)=(Id=CONTEXT_GAME_MODE,ValueIndex=CONTEXT_GAME_MODE_COOP,AdvertisementType=ODAT_OnlineService)

	// Chapter number property
	PropertyMappings(0)=(Id=PROPERTY_CHAPTERNUM,Name="ChapterNum")
	Properties(0)=(PropertyId=PROPERTY_CHAPTERNUM,Data=(Type=SDT_Int32,Value1=1),AdvertisementType=ODAT_OnlineService)

	PropertyMappings(1)=(Id=PROPERTY_ACTNUM,Name="ActNum")
	Properties(1)=(PropertyId=PROPERTY_ACTNUM,Data=(Type=SDT_Int32,Value1=1),AdvertisementType=ODAT_OnlineService)

	MaxSearchResults=5
}
