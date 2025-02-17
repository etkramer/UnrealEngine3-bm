/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Used to find matches that a party can participate in
 */
class GearVersusGameSearch extends OnlineGameSearch
	config(Game)
	native(Online);

`include(GearOnlineConstants.uci)

/** The target ping we think would be a great match */
var config int TargetPing;

/** The max ping we will accept before discarding the match */
var config int MaxPing;

/** The max match quality that is used in determining a sessions overall quality */
var config float MaxMatchQuality;

/** Sorts the results using our target match quality and target ping */
native event SortSearchResults();

defaultproperties
{
	// Which server side query to execute
	Query=(ValueIndex=SESSION_MATCH_QUERY_VERSUS)

	// The class to use for any search results. Important for party matches!
	GameSettingsClass=class'GearGame.GearVersusGameSettings'

	// All versus searches are ranked
	bUsesArbitration=true

	// We are only searching for versus sessions
	LocalizedSettings(0)=(Id=CONTEXT_GAME_MODE,ValueIndex=CONTEXT_GAME_MODE_VERSUS,AdvertisementType=ODAT_OnlineService)

	// We only search based off of playlist id
	Properties(0)=(PropertyId=PROPERTY_PLAYLISTID,Data=(Type=SDT_Int32,Value1=0),AdvertisementType=ODAT_OnlineService)
	PropertyMappings(0)=(Id=PROPERTY_PLAYLISTID,Name="PlaylistId")

	MaxSearchResults=5
}
