/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Holds the base configuration settings for an online coop game
 * NOTE: This class will normally be code generated
 */
class GearCoopGameSettings extends OnlineGameSettings;

`include(GearOnlineConstants.uci)

defaultproperties
{
	bAllowJoinInProgress=true
	bAllowInvites=true
	bUsesPresence=true
	bAllowJoinViaPresence=true

	// Enough space for 2 players
	NumPublicConnections=2

	// Coop game mode (const)
	LocalizedSettings(0)=(Id=CONTEXT_GAME_MODE,ValueIndex=CONTEXT_GAME_MODE_COOP,AdvertisementType=ODAT_OnlineServiceAndQoS)

	// Invite setting
	LocalizedSettings(1)=(Id=CONTEXT_COOP_INVITE,ValueIndex=eGCIT_FriendsOnly,AdvertisementType=ODAT_DontAdvertise)
	LocalizedSettingsMappings(1)=(Id=CONTEXT_COOP_INVITE,Name="CoopInvite",ValueMappings=((Id=eGCIT_InviteRequired),(Id=eGCIT_FriendsOnly),(Id=eGCIT_Public)))

	// Act number property
	PropertyMappings(0)=(Id=PROPERTY_ACTNUM,Name="ActNum")
	Properties(0)=(PropertyId=PROPERTY_ACTNUM,Data=(Type=SDT_Int32,Value1=1),AdvertisementType=ODAT_OnlineService)

	// Chapter number property
	PropertyMappings(1)=(Id=PROPERTY_CHAPTERNUM,Name="ChapterNum")
	Properties(1)=(PropertyId=PROPERTY_CHAPTERNUM,Data=(Type=SDT_Int32,Value1=1),AdvertisementType=ODAT_OnlineService)

	// Whether the party is Public (anyone can join) or not
	Properties(2)=(PropertyId=PROPERTY_ISPUBLICPARTY,Data=(Type=SDT_Int32,Value1=0),AdvertisementType=ODAT_OnlineService)
	PropertyMappings(2)=(Id=PROPERTY_ISPUBLICPARTY,Name="IsPublicParty")

	LocalizedSettings(2)=(Id=CampCheckpointUsage,ValueIndex=eGEARCHECKPOINT_UseLast,AdvertisementType=ODAT_DontAdvertise)
	LocalizedSettingsMappings(2)=(Id=CampCheckpointUsage,Name="RemoteCheckpointUsage",ValueMappings=((Id=eGEARCHECKPOINT_UseLast),(Id=eGEARCHECKPOINT_Restart)))

	LocalizedSettings(3)=(Id=CAMP_MODE,ValueIndex=eGCM_LivePrivate,AdvertisementType=ODAT_DontAdvertise)
	LocalizedSettingsMappings(3)=(Id=CAMP_MODE,Name="RemoteCampMode",ValueMappings=((Id=eGCM_LivePublic),(Id=eGCM_LivePrivate),(Id=eGCM_SystemLink)))
}
