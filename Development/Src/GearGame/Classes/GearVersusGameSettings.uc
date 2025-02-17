/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Holds the base configuration settings for TDM match.
 * NOTE: This class will normally be code generated
 */
class GearVersusGameSettings extends OnlineGameSettings
	native(Online);

/** The overall session quality determined using our heuristic */
var float OverallMatchQuality;

`include(GearOnlineConstants.uci)

defaultproperties
{
	// These are only joinable via matchmaking, no invites or jip
	bAllowJoinInProgress=false
	bAllowInvites=false
	bUsesPresence=false
	bAllowJoinViaPresence=false

	// All matches are ranked by default
	bUsesArbitration=true

	// Enough space for 2 teams of 5
	NumPublicConnections=10

	/************************************************************************/
	/* NOTE: VERY IMPORTANT!!! IF THIS ARRAY IS CHANGED YOU MUST MAKE SURE	*/
	/*       DERIVED CLASSES ARE ADJUSTED ACCORDINGLY!!!!					*/
	/************************************************************************/
	// Versus game mode (const)
	LocalizedSettings(0)=(Id=CONTEXT_GAME_MODE,ValueIndex=CONTEXT_GAME_MODE_VERSUS,AdvertisementType=ODAT_OnlineService)
	LocalizedSettingsMappings(0)=(Id=CONTEXT_GAME_MODE)

	// The playlist id for this match
	Properties(0)=(PropertyId=PROPERTY_PLAYLISTID,Data=(Type=SDT_Int32,Value1=0),AdvertisementType=ODAT_OnlineService)
	PropertyMappings(0)=(Id=PROPERTY_PLAYLISTID,Name="PlaylistId")

	// The sub-game mode (const and set in every derived class, only put here for clarity)
//@todo joeg/robm -- Are these needed any more?
	LocalizedSettings(1)=(Id=CONTEXT_VERSUSMODES,ValueIndex=eGEARMP_Warzone,AdvertisementType=ODAT_DontAdvertise)
	LocalizedSettingsMappings(1)=(Id=CONTEXT_VERSUSMODES,Name="GameType",ValueMappings=((Id=eGEARMP_Warzone),(Id=eGEARMP_CTM),(Id=eGEARMP_Wingman),(Id=eGEARMP_CombatTrials),(Id=eGEARMP_Execution),(Id=eGEARMP_KTL),(Id=eGEARMP_Annex),(Id=eGEARMP_KOTH)))

	// Common settings for all game modes that are only configurable in private matches
	// even though we don't use this one, don't comment it out as then its Id will be the same as CONTEXT_VERSUSMODES
	LocalizedSettings(2)=(Id=CONTEXT_MAPNAME,ValueIndex=CONTEXT_MAPNAME_FLOOD,AdvertisementType=ODAT_DontAdvertise)
	LocalizedSettingsMappings(2)=(Id=CONTEXT_MAPNAME)

	LocalizedSettings(3)=(Id=CONTEXT_FRIENDLYFIRE,ValueIndex=CONTEXT_FRIENDLYFIRE_NO,AdvertisementType=ODAT_DontAdvertise)
	LocalizedSettingsMappings(3)=(Id=CONTEXT_FRIENDLYFIRE,Name="FriendlyFire",ValueMappings=((Id=CONTEXT_FRIENDLYFIRE_NO),(Id=CONTEXT_FRIENDLYFIRE_YES)))

	LocalizedSettings(4)=(Id=CONTEXT_ROUNDTIME,ValueIndex=CONTEXT_ROUNDTIME_FIVE,AdvertisementType=ODAT_DontAdvertise)
	LocalizedSettingsMappings(4)=(Id=CONTEXT_ROUNDTIME,Name="RoundTime",ValueMappings=((Id=CONTEXT_ROUNDTIME_ONE),(Id=CONTEXT_ROUNDTIME_TWO),(Id=CONTEXT_ROUNDTIME_THREE),(Id=CONTEXT_ROUNDTIME_FOUR),(Id=CONTEXT_ROUNDTIME_FIVE),(Id=CONTEXT_ROUNDTIME_SIX),(Id=CONTEXT_ROUNDTIME_SEVEN),(Id=CONTEXT_ROUNDTIME_EIGHT),(Id=CONTEXT_ROUNDTIME_NINE),(Id=CONTEXT_ROUNDTIME_TEN)))

	LocalizedSettings(5)=(Id=CONTEXT_BLEEDOUTTIME,ValueIndex=CONTEXT_BLEEDOUTTIME_FIFTEEN,AdvertisementType=ODAT_DontAdvertise)
	LocalizedSettingsMappings(5)=(Id=CONTEXT_BLEEDOUTTIME,Name="Bleedout",ValueMappings=((Id=CONTEXT_BLEEDOUTTIME_FIVE),(Id=CONTEXT_BLEEDOUTTIME_TEN),(Id=CONTEXT_BLEEDOUTTIME_FIFTEEN),(Id=CONTEXT_BLEEDOUTTIME_TWENTY),(Id=CONTEXT_BLEEDOUTTIME_THIRTY),(Id=CONTEXT_BLEEDOUTTIME_SIXTY)))

	LocalizedSettings(6)=(Id=CONTEXT_WEAPONSWAP,ValueIndex=CONTEXT_WEAPONSWAP_CYCLE,AdvertisementType=ODAT_DontAdvertise)
	LocalizedSettingsMappings(6)=(Id=CONTEXT_WEAPONSWAP,Name="WeaponSwap",ValueMappings=((Id=CONTEXT_WEAPONSWAP_CYCLE),(Id=CONTEXT_WEAPONSWAP_NORMAL),(Id=CONTEXT_WEAPONSWAP_CUSTOM)))

	LocalizedSettings(7)=(Id=CONTEXT_ROUNDSTOWIN,ValueIndex=CONTEXT_ROUNDSTOWIN_FIVE,AdvertisementType=ODAT_DontAdvertise)
	LocalizedSettingsMappings(7)=(Id=CONTEXT_ROUNDSTOWIN,Name="RoundsToWin",ValueMappings=((Id=CONTEXT_ROUNDSTOWIN_ONE),(Id=CONTEXT_ROUNDSTOWIN_TWO),(Id=CONTEXT_ROUNDSTOWIN_THREE),(Id=CONTEXT_ROUNDSTOWIN_FOUR),(Id=CONTEXT_ROUNDSTOWIN_FIVE),(Id=CONTEXT_ROUNDSTOWIN_SIX),(Id=CONTEXT_ROUNDSTOWIN_SEVEN),(Id=CONTEXT_ROUNDSTOWIN_EIGHT),(Id=CONTEXT_ROUNDSTOWIN_NINE),(Id=CONTEXT_ROUNDSTOWIN_TEN),(Id=CONTEXT_ROUNDSTOWIN_ELEVEN),(Id=CONTEXT_ROUNDSTOWIN_TWELVE),(Id=CONTEXT_ROUNDSTOWIN_THIRTEEN),(Id=CONTEXT_ROUNDSTOWIN_FOURTEEN),(Id=CONTEXT_ROUNDSTOWIN_FIFTEEN)))

	LocalizedSettings(8)=(Id=CONTEXT_BOTDIFFICULTY,ValueIndex=CONTEXT_AIDIFFICULTY_NORMAL,AdvertisementType=ODAT_DontAdvertise)
	LocalizedSettingsMappings(8)=(Id=CONTEXT_BOTDIFFICULTY,Name="BotDifficulty",ValueMappings=((Id=CONTEXT_AIDIFFICULTY_CASUAL),(Id=CONTEXT_AIDIFFICULTY_NORMAL),(Id=CONTEXT_AIDIFFICULTY_HARDCORE),(Id=CONTEXT_AIDIFFICULTY_INSANE)))

	LocalizedSettings(9)=(Id=CONTEXT_NUMBOTS,ValueIndex=CONTEXT_NUMBOTS_0,AdvertisementType=ODAT_DontAdvertise)
	LocalizedSettingsMappings(9)=(Id=CONTEXT_NUMBOTS,Name="NumBots",ValueMappings=((Id=CONTEXT_NUMBOTS_0),(Id=CONTEXT_NUMBOTS_1),(Id=CONTEXT_NUMBOTS_2),(Id=CONTEXT_NUMBOTS_3),(Id=CONTEXT_NUMBOTS_4),(Id=CONTEXT_NUMBOTS_5),(Id=CONTEXT_NUMBOTS_6),(Id=CONTEXT_NUMBOTS_7),(Id=CONTEXT_NUMBOTS_8),(Id=CONTEXT_NUMBOTS_9)))

	LocalizedSettings(10)=(Id=MAP_SELECTION_MODE,ValueIndex=eGEARMAPSELECT_VOTE,AdvertisementType=ODAT_DontAdvertise)
	LocalizedSettingsMappings(10)=(Id=MAP_SELECTION_MODE,Name="MapSelectionMode",ValueMappings=((Id=eGEARMAPSELECT_VOTE),(Id=eGEARMAPSELECT_HOSTSELECT)))

	LocalizedSettings(11)=(Id=VERSUS_MATCH_MODE,ValueIndex=eGVMT_Official,AdvertisementType=ODAT_DontAdvertise)
	LocalizedSettingsMappings(11)=(Id=VERSUS_MATCH_MODE,Name="RemoteMatchMode",ValueMappings=((Id=eGVMT_Official),(Id=eGVMT_Custom),(Id=eGVMT_SystemLink),(Id=eGVMT_Local)))

	LocalizedSettings(12)=(Id=VERSUS_PARTY_TYPE,ValueIndex=eGVPT_FriendsOnly,AdvertisementType=ODAT_DontAdvertise)
	LocalizedSettingsMappings(12)=(Id=VERSUS_PARTY_TYPE,Name="RemotePartyType",ValueMappings=((Id=eGVPT_InviteRequired),(Id=eGVPT_FriendsOnly),(Id=eGVPT_Public)))
}

