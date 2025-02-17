/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Holds the base configuration settings for a party lobby
 * NOTE: This class will normally be code generated
 */
class GearPartyGameSettings extends OnlineGameSettings;

`include(GearOnlineConstants.uci)

defaultproperties
{
	bUsesArbitration=false

	// This number will change to 10 when a private party
	NumPublicConnections=5

	// Game mode for the party (const)
	LocalizedSettings(0)=(Id=CONTEXT_GAME_MODE,ValueIndex=CONTEXT_GAME_MODE_PARTY,AdvertisementType=ODAT_OnlineServiceAndQoS)

// Partial party matchmaking values
	LocalizedSettings(1)=(Id=CONTEXT_PARTYSLOT_1,ValueIndex=CONTEXT_PARTYSLOT_BLOCKED,AdvertisementType=ODAT_OnlineService)
	LocalizedSettingsMappings(1)=(Id=CONTEXT_PARTYSLOT_1,Name="PartySlot_1",ValueMappings=((Id=CONTEXT_PARTYSLOT_BLOCKED),(Id=CONTEXT_PARTYSLOT_WANTS),(Id=CONTEXT_PARTYSLOT_NEEDS),(Id=CONTEXT_PARTYSLOT_ANY,bIsWildcard=true)))

	LocalizedSettings(2)=(Id=CONTEXT_PARTYSLOT_2,ValueIndex=CONTEXT_PARTYSLOT_BLOCKED,AdvertisementType=ODAT_OnlineService)
	LocalizedSettingsMappings(2)=(Id=CONTEXT_PARTYSLOT_2,Name="PartySlot_2",ValueMappings=((Id=CONTEXT_PARTYSLOT_BLOCKED),(Id=CONTEXT_PARTYSLOT_WANTS),(Id=CONTEXT_PARTYSLOT_NEEDS),(Id=CONTEXT_PARTYSLOT_ANY,bIsWildcard=true)))

	LocalizedSettings(3)=(Id=CONTEXT_PARTYSLOT_3,ValueIndex=CONTEXT_PARTYSLOT_BLOCKED,AdvertisementType=ODAT_OnlineService)
	LocalizedSettingsMappings(3)=(Id=CONTEXT_PARTYSLOT_3,Name="PartySlot_3",ValueMappings=((Id=CONTEXT_PARTYSLOT_BLOCKED),(Id=CONTEXT_PARTYSLOT_WANTS),(Id=CONTEXT_PARTYSLOT_NEEDS),(Id=CONTEXT_PARTYSLOT_ANY,bIsWildcard=true)))
// End partial party matchmaking values

	// Whether the party is Public (anyone can join) or not
	Properties(0)=(PropertyId=PROPERTY_ISPUBLICPARTY,Data=(Type=SDT_Int32,Value1=1),AdvertisementType=ODAT_OnlineService)
	PropertyMappings(0)=(Id=PROPERTY_ISPUBLICPARTY,Name="IsPublicParty")
	// The playlist id for this match
	Properties(1)=(PropertyId=PROPERTY_PLAYLISTID,Data=(Type=SDT_Int32,Value1=0),AdvertisementType=ODAT_OnlineService)
	PropertyMappings(1)=(Id=PROPERTY_PLAYLISTID,Name="PlaylistId")
}
