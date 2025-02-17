/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Used to find parties that are publicly joinable
 */
class GearPartyGameSearch extends OnlineGameSearch
	config(Game)
	native(Online);

`include(GearOnlineConstants.uci)

/** The max ping we will accept before discarding the party session */
var config int MaxPing;

/** Sorts the results by ping since skill doesn't matter here */
native event SortSearchResults();

defaultproperties
{
	// Which server side query to execute
	Query=(ValueIndex=SESSION_MATCH_QUERY_PARTY)
	GameSettingsClass=class'GearGame.GearPartyGameSettings'

	// We are only searching for party sessions
	LocalizedSettings(0)=(Id=CONTEXT_GAME_MODE,ValueIndex=CONTEXT_GAME_MODE_PARTY,AdvertisementType=ODAT_OnlineService)

// Partial party matchmaking values
	LocalizedSettings(1)=(Id=CONTEXT_PARTYSLOT_1,ValueIndex=CONTEXT_PARTYSLOT_ANY,AdvertisementType=ODAT_OnlineService)
	LocalizedSettingsMappings(1)=(Id=CONTEXT_PARTYSLOT_1,Name="PartySlot_1",ValueMappings=((Id=CONTEXT_PARTYSLOT_BLOCKED),(Id=CONTEXT_PARTYSLOT_WANTS),(Id=CONTEXT_PARTYSLOT_NEEDS),(Id=CONTEXT_PARTYSLOT_ANY,bIsWildcard=true)))

	LocalizedSettings(2)=(Id=CONTEXT_PARTYSLOT_2,ValueIndex=CONTEXT_PARTYSLOT_ANY,AdvertisementType=ODAT_OnlineService)
	LocalizedSettingsMappings(2)=(Id=CONTEXT_PARTYSLOT_2,Name="PartySlot_2",ValueMappings=((Id=CONTEXT_PARTYSLOT_BLOCKED),(Id=CONTEXT_PARTYSLOT_WANTS),(Id=CONTEXT_PARTYSLOT_NEEDS),(Id=CONTEXT_PARTYSLOT_ANY,bIsWildcard=true)))

	LocalizedSettings(3)=(Id=CONTEXT_PARTYSLOT_3,ValueIndex=CONTEXT_PARTYSLOT_ANY,AdvertisementType=ODAT_OnlineService)
	LocalizedSettingsMappings(3)=(Id=CONTEXT_PARTYSLOT_3,Name="PartySlot_3",ValueMappings=((Id=CONTEXT_PARTYSLOT_BLOCKED),(Id=CONTEXT_PARTYSLOT_WANTS),(Id=CONTEXT_PARTYSLOT_NEEDS),(Id=CONTEXT_PARTYSLOT_ANY,bIsWildcard=true)))
// End partial party matchmaking values

	// The playlist id for this match
	Properties(0)=(PropertyId=PROPERTY_PLAYLISTID,Data=(Type=SDT_Int32,Value1=0),AdvertisementType=ODAT_OnlineService)
	PropertyMappings(0)=(Id=PROPERTY_PLAYLISTID,Name="PlaylistId")

	MaxSearchResults=5
}
