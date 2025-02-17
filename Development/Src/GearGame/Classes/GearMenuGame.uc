/**
 * This class is the game info for the main menu
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearMenuGame extends GearGame
	config(Game)
	dependson(GearProfileSettings,GearTypes);

`include(GearOnlineConstants.uci)

/** Cached pointer to the playlist manager */
var OnlinePlaylistManager PlaylistMan;

/** The party search that is being used */
var GearPartyGameSearch PartySearch;

/** The ping to use as the max acceptable */
var config int PingThreshold;

/** Holds the nat type for this console */
var ENATType NatType;

/** The most restrictive setting that a player can have and still host a versus match */
var config ENATType CanHostVersusNatType;

/** The most restrictive setting that a player can have and still host a partial party */
var config ENATType CanHostPartyNatType;

/** The most restrictive setting that a player can have and still host a campaign */
var config ENATType CanHostCampaignNatType;

/** The most restrictive setting that a player can have and still invite to the party */
var config ENATType CanInviteNatType;

/** The coop game settings that is being used */
var GearCoopGameSettings CoopGameSettings;

/** set when coming from the endgame to show the credits screen first */
var bool bShowCreditsAtStartup;

/** The campaign lobby mode (if we are in the campaign lobby) */
var EGearCampaignLobbyMode CampaignLobbyMode;


/**
 * Whether the console is allowed to host due to NAT configuration and/or the
 * number of players locally (no splitscreen hosts)
 *
 * @return true if the NAT is ok for hosting, false otherwise
 */
function bool CanHostVersusMatch()
{
	local int Index;
	local int Count;

	// Count the number of logged in players and block hosting if splitscreen
	for (Index = 0; Index < 4; Index++)
	{
		if (OnlineSub.PlayerInterface.GetLoginStatus(Index) != LS_NotLoggedIn)
		{
			Count++;
		}
	}
	return Count == 1 && NatType <= CanHostVersusNatType;
}

/**
 * Whether the console is allowed to partial party host due to NAT configuration
 *
 * @return true if the NAT is ok for hosting, false otherwise
 */
function bool CanHostPartyMatch()
{
	local int Index;
	local int Count;

	// Count the number of logged in players and block hosting if splitscreen
	for (Index = 0; Index < 4; Index++)
	{
		if (OnlineSub.PlayerInterface.GetLoginStatus(Index) != LS_NotLoggedIn)
		{
			Count++;
		}
	}
	return Count == 1 && NatType <= CanHostPartyNatType;
}

/**
 * Whether the console is allowed to partial party host due to NAT configuration
 *
 * @return true if the NAT is ok for hosting, false otherwise
 */
function bool IsNatOkForParty()
{
	return NatType <= CanHostPartyNatType;
}

/**
 * Whether the console is allowed to host due to NAT configuration
 *
 * @return true if the NAT is ok for hosting, false otherwise
 */
function bool CanHostCampaign()
{
	return NatType <= CanHostCampaignNatType;
}

/**
 * Whether the console is allowed to invite due to NAT configuration
 *
 * @return true if the NAT is ok for hosting, false otherwise
 */
function bool CanInviteToMatch()
{
	return NatType <= CanInviteNatType;
}

/** Stubbed out so we don't spawn a player in the menus */
function RestartPlayer(Controller NewPlayer);

/** Get the profile of the host */
function GearProfileSettings GetProfile(int ControllerId)
{
	local LocalPlayer LP;
	local GearPC GearPlayerOwner;
	local int PlayerIndex;

	PlayerIndex = class'UIInteraction'.static.GetPlayerIndex(ControllerId);
	LP = GetPlayerOwner(PlayerIndex);
	GearPlayerOwner = GearPC(LP.Actor);
	return GearPlayerOwner.ProfileSettings;
}

/**
 * Wrapper for checking whether the currently selected gametype is wingman
 */
function bool IsWingmanGametype()
{
	local LocalPlayer LP;
	local GearPC GearPlayerOwner;
	local GearProfileSettings Profile;
	local int ValueContextId;
	local bool bResult;

	LP = GetPlayerOwner();
	GearPlayerOwner = GearPC(LP.Actor);

	Profile = GearPlayerOwner.ProfileSettings;
	if ( Profile.GetProfileSettingValueId(Profile.const.VERSUS_GAMETYPE, ValueContextId) )
	{
		bResult = ValueContextId == eGEARMP_Wingman;
	}

	return bResult;
}

/**
 * Wrapper for checking whether the currently selected gametype is horde
 */
function bool IsHordeGametype()
{
	local LocalPlayer LP;
	local GearPC GearPlayerOwner;
	local GearProfileSettings Profile;
	local int ValueContextId;
	local bool bResult;

	LP = GetPlayerOwner();
	GearPlayerOwner = GearPC(LP.Actor);

	Profile = GearPlayerOwner.ProfileSettings;
	if ( Profile.GetProfileSettingValueId(Profile.const.VERSUS_GAMETYPE, ValueContextId) )
	{
		bResult = ValueContextId == eGEARMP_CombatTrials;
	}

	return bResult;
}

/**
 * Wrapper for getting a reference to the LocalPlayer object for an active player.
 */
function LocalPlayer GetPlayerOwner( int PlayerIndex=0 )
{
	local UIInteraction UIController;
	local LocalPlayer Result;

	UIController = class'UIRoot'.static.GetCurrentUIController();
	if ( PlayerIndex >= 0 && PlayerIndex < UIController.GamePlayers.Length )
	{
		Result = UIController.GamePlayers[PlayerIndex];
	}

	return Result;
}

/** Returns the GameViewportClient of the playerowner */
function GameViewPortClient GetPlayerOwnerViewportClient( int PlayerIndex=0 )
{
	local LocalPlayer LP;

	LP = GetPlayerOwner(PlayerIndex);
	if ( LP != None )
	{
		return LP.ViewportClient;
	}
	return None;
}

/**
 * Changes the invite type. NOTE: Does not publish to Live
 *
 * @param Coop the settings object for this coop campaign
 * @param InviteType the JIP mode to set the game ot
 * @param whether to set the profile with the new InviteType value or not
 */
function ChangeCoopInviteSetting(GearCoopGameSettings Coop, EGearCoopInviteType InviteType)
{
	if (Coop != None)
	{
		// Whether anyone can join via presence
		Coop.bAllowJoinViaPresence = InviteType == eGCIT_Public;
		// Whether only friends can join via presence
		Coop.bAllowJoinViaPresenceFriendsOnly = InviteType == eGCIT_FriendsOnly;
		// Just use public connections since the search is controlled by the IsPublicParty flag
		Coop.NumPublicConnections = 2;
		// If we aren't public, then set the property to filter out searches
		Coop.SetIntProperty(class'GearCoopGameSettings'.const.PROPERTY_ISPUBLICPARTY,InviteType <= eGCIT_FriendsOnly ? 0 : 1);

		MaxPlayers = Coop.NumPublicConnections + Coop.NumPrivateConnections;
	}
	else
	{
		`Warn("ChangeCoopInviteSetting -> Missing the coop game settings to update!");
	}
}

/** Changes the Chapter/Act settings for the specified object. NOTE: Does not publish to Live */
function ChangeChapterSettings(EChapterPoint Chapter)
{
	local int ActId, NormalizedChapter;
	local GearCampaignActData ActData;

	// Set the act and chapter
	ActData = class'GearUIDataStore_GameResource'.static.GetActDataProviderUsingChapter(Chapter);
	if ( ActData != None )
	{
		ActId = ActData.ActType;
	}
	CoopGameSettings.SetIntProperty(PROPERTY_ACTNUM, ActId+1);
	NormalizedChapter = class'GearUIDataStore_GameResource'.static.GetActChapterProviderIndexFromChapterId(Chapter);
	// Sanity check
	if (NormalizedChapter == INDEX_NONE)
	{
		NormalizedChapter = 0;
	}
	CoopGameSettings.SetIntProperty(PROPERTY_CHAPTERNUM, NormalizedChapter+1);
}

/**
 * Initialize the coop settings from the profile
 *
 * @param bIsRecreating - set to true if we are just recreating the coop session for the lobby (as opposed to the initialization of the lobby)
 */
function bool InitializeCoopSettingsFromProfile(int ControllerIndex,bool bIsLAN,EChapterPoint Chapter,bool bIsRecreating)
{
	local GearProfileSettings Profile;
	local int CampMode, InviteType;
	local GameViewPortClient VPClient;

	if ( OnlineSub != None &&
		 OnlineSub.PlayerInterface != None )
	{
		// Get the profile for the player
		Profile = GetProfile(ControllerIndex);
		// See if we can attempt a network game
		VPClient = GetPlayerOwnerViewportClient(class'UIInteraction'.static.GetPlayerIndex(ControllerIndex));
		if (Profile != None &&
			VPClient.UIController.HasLinkConnection())
		{
			// See if they have access to Live to determine which mode to default them to
			VPClient = GetPlayerOwnerViewportClient(class'UIInteraction'.static.GetPlayerIndex(ControllerIndex));
			// If there is no Live service, they are not allowed to play online, or the fail the NAT check default to LAN
			if (bIsLAN ||
				VPClient.UIController.GetLowestLoginStatusOfControllers() != LS_LoggedIn ||
				!VPClient.UIController.CanAllPlayOnline() ||
				(CampaignLobbyMode == eGCLOBBYMODE_Host && !CanHostCampaign()))
			{
				bIsLAN = true;
			}

			// Initialize the campaign mode and set the profile settings
			// NOTE: Don't force the lobby to private when recreating (this overrides the lobby functionality)
			if (!bIsRecreating || bIsLAN)
			{
				CampMode = bIsLAN ? eGCM_SystemLink : eGCM_LivePrivate;
				Profile.SetProfileSettingValueId(class'GearProfileSettings'.const.CAMP_MODE, CampMode);
			}
			else
			{
				Profile.GetProfileSettingValueId(class'GearProfileSettings'.const.CAMP_MODE, CampMode);
			}

			// Set whether this is a LAN party or not
			if (CoopGameSettings != None)
			{
				CoopGameSettings.bIsLanMatch = bIsLAN;
			}

			// Get the invite policy
			if (CampaignLobbyMode == eGCLOBBYMODE_SoloPath)
			{
				InviteType = eGCIT_InviteRequired;
			}
			else if (CampMode == eGCM_LivePrivate)
			{
				InviteType = eGCIT_FriendsOnly;
			}
			else if (CampMode == eGCM_LivePublic ||
					 CampMode == eGCM_SystemLink)
			{
				InviteType = eGCIT_Public;
			}

			// Change the Invite type session data
			ChangeCoopInviteSetting(CoopGameSettings, EGearCoopInviteType(InviteType));
			// Set the settings object with the campaign mode
			CoopGameSettings.SetStringSettingValue( class'GearProfileSettings'.const.CAMP_MODE, CampMode, false );
			// Change the chapter in the session data
			ChangeChapterSettings(Chapter);

			return true;
		}
		else
		{
			`log("Could not create an online game because we didn't pass the networking checks!");
		}
	}
	else
	{
		`log("Cannot begin search for coop - no" @ (OnlineSub != None ? "OnlinePlayerInterface" : "OnlineSubsystem") @ "found!");
	}

	return false;
}

/**
 * Looks at the player's profile and recreates a campaign with the last used settings
 *
 * @param ControllerIndex the index of the player creating the session
 * @param bIsLAN whether to create a LAN game or not
 *
 * @return whether an attempt to create an online campaign lobby happened
 */
function bool RecreateCampaign(int ControllerIndex, bool bIsLAN, EChapterPoint Chapter)
{
	if (CampaignLobbyMode == eGCLOBBYMODE_Host)
	{
		// Create the object and change it's settings to match the profile preference
		CoopGameSettings = new class'GearCoopGameSettings';

		// Initialize the campaign for a LIVE session using the profile settings
		if (InitializeCoopSettingsFromProfile(ControllerIndex, bIsLAN, Chapter,true))
		{
			GameInterface.AddCreateOnlineGameCompleteDelegate(OnRecreateCampaignComplete);
			// Create an online game
			if (GameInterface.CreateOnlineGame(ControllerIndex,'Party',CoopGameSettings))
			{
				OpenUpdatingPartyScene();
				return true;
			}
			else
			{
				GameInterface.ClearCreateOnlineGameCompleteDelegate(OnRecreateCampaignComplete);
			}
		}
	}
	return false;
}

/**
 * Called when the campaign recreate finishes publishing with Live
 *
 * @param SessionName the name of the session this event is for
 * @param bWasSuccessful whether the create worked or not
 */
function OnRecreateCampaignComplete(name SessionName,bool bWasSuccessful)
{
	if (SessionName == 'Party')
	{
		GameInterface.ClearCreateOnlineGameCompleteDelegate(OnRecreateCampaignComplete);
		CloseUpdatingPartyScene();
	}

	if (!bWasSuccessful)
	{
		// TODO: robm show error message about network being down
	}
}

/** Changes the party type for the specified object. NOTE: Does not publish to Live */
function ChangePartySettings( GearPartyGameSettings PartySettings, int PartyType )
{
	local OnlineGameSettings CurrentGameSettings;

	if ( PartySettings != None )
	{
		// Sets the invite settings based off whether you can invite or not
		if (CanInviteToMatch())
		{
			// Whether anyone can join via presence
			PartySettings.bAllowJoinViaPresence = PartyType == eGVPT_Public;
			// Whether only friends can join via presence
			PartySettings.bAllowJoinViaPresenceFriendsOnly = PartyType == eGVPT_FriendsOnly;
		}
		else
		{
			// This NAT doesn't allow invites
			PartySettings.bAllowInvites = false;
			PartySettings.bAllowJoinViaPresence = false;
			PartySettings.bAllowJoinViaPresenceFriendsOnly = false;
		}

		// Copy to the settings for propagating to clients
		CurrentGameSettings = GetCurrentGameSettings();
		CurrentGameSettings.SetStringSettingValue( class'GearProfileSettings'.const.VERSUS_PARTY_TYPE, PartyType, false );
	}
	else
	{
		`Warn("ChangePartySettings -> Missing the party game settings to update!");
	}
}

/** Changes the Match Mode for the specified object (public, private). NOTE: Does not publish to Live */
function ChangeMatchModeSettings( GearPartyGameSettings PartySettings, int MatchMode, GearProfileSettings Profile )
{
	local OnlineGameSettings CurrentGameSettings;
	local int PlaylistValue, TeamSize, TeamCount;

	// Switch on the new Match Mode
	switch ( MatchMode )
	{
		// Public Match
		case eGVMT_Official:
			// Get the number of players for the UI
			OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
			if ( OnlineSub != None )
			{
				if ( PlaylistMan != None )
				{
					if ( Profile != None )
					{
						PlaylistValue = Profile.GetPlaylistID();
						PlaylistMan.GetTeamInfoFromPlaylist( PlaylistValue, TeamSize, TeamCount );
						GearPartyGRI(WorldInfo.GRI).TeamSize = TeamSize;
					}
				}
			}

			// Set the number of players for the gameinfo
			MaxPlayers = 10;

			// Set the number of players for the party and mark the party as public for matchmaking
			if ( PartySettings != None )
			{
				PartySettings.NumPublicConnections = MaxPlayers;

				// Determine whether this should show up in matchmaking or not
				PartySettings.SetIntProperty( PROPERTY_ISPUBLICPARTY, 1 );
			}
		break;

		// Custom and SystemLink
		case eGVMT_Custom:
		case eGVMT_SystemLink:
			// Set the number of players for the UI
			GearPartyGRI(WorldInfo.GRI).TeamSize = IsHordeGametype() ? 5 : 10;;

			// Set the number of players for the gameinfo
			MaxPlayers = 10;

			// Set the number of player for the party and mark the party as private for no matchmaking
			if ( PartySettings != None )
			{
				PartySettings.NumPublicConnections = MaxPlayers;

				// Determine whether this should show up in matchmaking or not
				PartySettings.SetIntProperty( PROPERTY_ISPUBLICPARTY, (MatchMode == eGVMT_SystemLink) ? 1 : 0 );
			}
		break;

		// Local Match
		case eGVMT_Local:
			// Set the number of players for the UI
			GearPartyGRI(WorldInfo.GRI).TeamSize = 2;

			// Set the number of players for the gameinfo
			MaxPlayers = 2;
		break;
	}

	// Copy to the settings for propagating to clients
	CurrentGameSettings = GetCurrentGameSettings();
	CurrentGameSettings.SetStringSettingValue( class'GearProfileSettings'.const.VERSUS_MATCH_MODE, MatchMode, false );
}

/**
 * Initialize the party settings from the profile
 *
 * @param Party - the party settings to use
 * @param ControllerIndex - the controller index of the player who initiated this call
 * @param bIsLAN - whether we are in system link or not
 * @param bIsRecreating - whether this was called from recreating the party settings or not
 */
function bool InitializePartySettingsFromProfile( GearPartyGameSettings Party, int ControllerIndex, bool bIsLAN, optional bool bIsRecreating )
{
	local GearProfileSettings Profile;
	local int MatchMode, PartyType;
	local GameViewPortClient VPClient;
	local bool bHordeIsDefault;
	if ( OnlineSub != None &&
		 OnlineSub.PlayerInterface != None )
	{
		// Get the profile for the player
		Profile = GetProfile(ControllerIndex);
		if (Profile != None)
		{
			bHordeIsDefault = IsHordeGametype();

			// First get the value from the profile
			if (!Profile.GetProfileSettingValueId(class'GearProfileSettings'.const.VERSUS_MATCH_MODE,MatchMode))
			{
				MatchMode = bHordeIsDefault ? eGVMT_Custom : eGVMT_Official;
			}
			// See if we should default to custom match
			// Don't do it if we are recreating the party (switching match mode inside the lobby)
			if (!bIsRecreating &&
				// If the last saved gametype was horde and the last match mode was NOT public OR
				((bHordeIsDefault && MatchMode != eGVMT_Official) ||
				// Any guest accounts are logged in
				 class'UIInteraction'.static.GetNumGuestsLoggedIn() > 0))
			{
				// Default to custom
				MatchMode = eGVMT_Custom;
				Profile.SetProfileSettingValueId(class'GearProfileSettings'.const.VERSUS_MATCH_MODE,MatchMode);
			}

			// If this is a LAN
			if ( bIsLAN )
			{
				// Make sure we have a network connection
				if ( !class'UIInteraction'.static.HasLinkConnection() )
				{
					// bail
					return false;
				}
				if ( Party != None )
				{
					Party.bAllowJoinInProgress = false;
				}
			}
			else
			{
				VPClient = GetPlayerOwnerViewportClient( class'UIInteraction'.static.GetPlayerIndex(ControllerIndex) );
				// If  we have a connection, all local players are logged into live and can play online
				if ( VPClient != None &&
					 VPClient.UIController != None &&
				     VPClient.UIController.HasLinkConnection() &&
					 VPClient.UIController.GetLowestLoginStatusOfControllers() == LS_LoggedIn &&
					 VPClient.UIController.CanAllPlayOnline() )
				{
					// We're logged into LIVE, so if the matchmode is NOT a LIVE match set it to Public
					// NOTE: It's ok to force to public here because if this was Horde or there were any guest accounts,
					// the code above would've already made the matchmode be custom
					if ( MatchMode != eGVMT_Official && MatchMode != eGVMT_Custom && !bIsRecreating )
					{
						MatchMode = eGVMT_Official;
						Profile.SetProfileSettingValueId(class'GearProfileSettings'.const.VERSUS_MATCH_MODE,MatchMode);
					}
				}
				// Didn't pass all of the LIVE checks so bail (this will result in a local player party being started
				else
				{
					return false;
				}
			}

			// Set whether this is a LAN party or not
			if ( Party != None )
			{
				Party.bIsLanMatch = bIsLAN;
			}

			// Change the Match Mode session data
			ChangeMatchModeSettings( Party, MatchMode, Profile );

			// Get the invite policy
			if ( !Profile.GetProfileSettingValueId(class'GearProfileSettings'.const.VERSUS_PARTY_TYPE,PartyType) )
			{
				PartyType = eGVPT_FriendsOnly;
			}

			// Change the Party Mode session data
			ChangePartySettings( Party, PartyType );

			return true;
		}
	}
	else
	{
		`log("Cannot begin search for party - no" @ (OnlineSub != None ? "OnlinePlayerInterface" : "OnlineSubsystem") @ "found!");
	}

	return false;
}

/**
 * Looks at the player's profile and creates a party with the last used settings
 *
 * @param ControllerIndex the index of the player creating the session
 */
function CreateParty(int ControllerIndex)
{
	local GearProfileSettings Profile;
	local GearPartyGameSettings Party;

	// Get the profile for the player
	Profile = GetProfile(ControllerIndex);
	if ( Profile != None )
	{
		// Create the object and change it's settings to match the profile preference
		Party = new class'GearPartyGameSettings';

		// Initialize the party for a LIVE session using the profile settings
		if ( InitializePartySettingsFromProfile(Party, ControllerIndex, false) )
		{
			GameInterface.AddCreateOnlineGameCompleteDelegate(OnCreatePartyComplete);
			// Publish a new party up on Live
			GameInterface.CreateOnlineGame(ControllerIndex,'Party',Party);
		}
		// Failed in initializing a LIVE party so create a local one
		else
		{
			CreateLocalParty( Profile );
		}
	}
}

/**
 * Called when the party create finishes publishing with Live
 *
 * @param SessionName the name of the session this event is for
 * @param bWasSuccessful whether the create worked or not
 */
function OnCreatePartyComplete(name SessionName,bool bWasSuccessful)
{
	local GearProfileSettings Profile;
	local LocalPlayer LP;

	if (SessionName == 'Party')
	{
		GameInterface.ClearCreateOnlineGameCompleteDelegate(OnCreatePartyComplete);

		LP = GetPlayerOwner();
		if (bWasSuccessful)
		{
			if ( LP != None && LP.Actor != None )
			{
				// Go ahead and load the party lobby
				//class'GearUIScene_Base'.static.ShowLoadingMovie(LP, true);
				LP.Actor.ClientTravel("GearStart?listen?game=GearGameContent.GearPartyGame?", TRAVEL_Absolute);
			}
		}
		else
		{
			Profile = GearPC(LP.Actor).ProfileSettings;
			if ( Profile != None )
			{
				// Failed in creating the party so create a local one
				CreateLocalParty( Profile );
			}
		}
	}
}

/** Opens or restarts the updating scene (used for blocking input during async tasks) */
function OpenUpdatingPartyScene()
{
	local GearUISceneFE_Updating SceneInstance;

	SceneInstance = class'GearUIScene_Base'.static.OpenUpdatingScene();
	if (SceneInstance != None)
	{
		SceneInstance.InitializeUpdatingScene("UpdatingTitle", "LiveStateDesc", 0.5f);
	}
}

/** Begins the process of closing the updating scene (there is a min time the scene must be open) */
function CloseUpdatingPartyScene()
{
	class'GearUIScene_Base'.static.CloseUpdatingScene();
}

/**
 * Looks at the player's profile and recreates a party with the last used settings
 *
 * @param ControllerIndex the index of the player creating the session
 * @param bIsLAN whether to create a LAN party or not
 */
function RecreateParty(int ControllerIndex, bool bIsLAN)
{
	local GearProfileSettings Profile;
	local GearPartyGameSettings Party;

	// Get the profile for the player
	Profile = GetProfile(ControllerIndex);
	if ( Profile != None )
	{
		// Create the object and change it's settings to match the profile preference
		Party = new class'GearPartyGameSettings';

		// Initialize the party for a LIVE session using the profile settings
		if ( InitializePartySettingsFromProfile(Party, ControllerIndex, bIsLAN, true) )
		{
			GameInterface.AddCreateOnlineGameCompleteDelegate(OnRecreatePartyComplete);
			// Publish a new party up on Live
			if (GameInterface.CreateOnlineGame(ControllerIndex,'Party',Party))
			{
				OpenUpdatingPartyScene();
			}
		}
	}
}

/**
 * Called when the party recreate finishes publishing with Live
 *
 * @param SessionName the name of the session this event is for
 * @param bWasSuccessful whether the create worked or not
 */
function OnRecreatePartyComplete(name SessionName,bool bWasSuccessful)
{
	if (SessionName == 'Party')
	{
		GameInterface.ClearCreateOnlineGameCompleteDelegate(OnRecreatePartyComplete);
		CloseUpdatingPartyScene();
	}
}

/**
 * Creates a local party
 */
function CreateLocalParty( GearProfileSettings Profile, optional bool bIsTraining )
{
	local LocalPlayer LP;
	local String URL;

	Profile.SetProfileSettingValueId( class'GearProfileSettings'.const.VERSUS_MATCH_MODE, eGVMT_Local );

	LP = GetPlayerOwner();
	URL = "GearStart?listen?game=GearGameContent.GearPartyGame?";
	if (bIsTraining)
	{
		URL $= "bIsTraining?";
	}
	//class'GearUIScene_Base'.static.ShowLoadingMovie(LP, true);
	LP.Actor.ClientTravel(URL, TRAVEL_Absolute);
}

/**
 * Looks at the player's profile and creates a campaign with the last used settings
 *
 * @param ControllerIndex the index of the player creating the session
 */
function ContinueCampaign(int ControllerIndex)
{
}

/**
 * Delegate fired when a create request has completed
 *
 * @param SessionName the name of the session this callback is for
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnCreateGameComplete(name SessionName,bool bWasSuccessful);

/**
 * Returns the campaign mode to start a solo campaign with via network permissions
 *
 * @param ControllerIndex the index of the player to return the campaign mode for
 *
 * @return - Either the enum value of the Private Live mode, or -1 for local
 */
function int GetSoloCampaignMode(int ControllerIndex)
{
	local GameViewPortClient VPClient;

	VPClient = GetPlayerOwnerViewportClient(class'UIInteraction'.static.GetPlayerIndex(ControllerIndex));
	if (VPClient != none &&
		VPClient.UIController != none &&
		VPClient.UIController.HasLinkConnection() &&
		VPClient.UIController.GetLowestLoginStatusOfControllers() == LS_LoggedIn &&
		VPClient.UIController.CanAllPlayOnline())
	{
		return eGCM_LivePrivate;
	}
	return -1;
}

/**
 * Looks at the player's profile and creates a campaign with the last used settings
 *
 * @param ControllerIndex the index of the player creating the session
 * @param CoopSettings the settings to use when creating the Live session
 * @param bCreateLobby whether this function will create the lobby or the actual game
 * @param bIsLanCampaign whether this is to be created as a LAN campaign or not
 *
 * @param Whether an attempt to create an online game was made
 */
function bool CreateCampaign(int ControllerIndex,GearCoopGameSettings CoopSettings,bool bCreateLobby,bool bIsLANCampaign,EChapterPoint Chapter)
{
	local GearProfileSettings Profile;
	local int CampMode;

	// Get the profile for the player
	Profile = GetProfile(ControllerIndex);
	if (Profile != None)
	{
		// Create the object if one was not passed in
		if (CoopSettings == None)
		{
			CoopGameSettings = new class'GearCoopGameSettings';
		}
		else
		{
			CoopGameSettings = CoopSettings;
		}

		// If this is happening via the solo path see if we should make it a LivePrivate match or just a local one
		if (CampaignLobbyMode == eGCLOBBYMODE_SoloPath)
		{
			CampMode = GetSoloCampaignMode(ControllerIndex);
		}
		// Check for whether to create a network game or not
		if (CampaignLobbyMode == eGCLOBBYMODE_Host ||
			(CampaignLobbyMode == eGCLOBBYMODE_SoloPath && CampMode == eGCM_LivePrivate))
		{
			// Initialize the campaign for a LIVE session using the profile settings
			if (InitializeCoopSettingsFromProfile(ControllerIndex, bIsLANCampaign,Chapter,false))
			{
				GameInterface.AddCreateOnlineGameCompleteDelegate(bCreateLobby ? OnCreateCampaignLobbyComplete : OnCreateCampaignComplete);
				GameInterface.CreateOnlineGame(ControllerIndex,'Party',CoopGameSettings);
				return true;
			}
		}
		else if (CampaignLobbyMode == eGCLOBBYMODE_Split ||
			     (CampaignLobbyMode == eGCLOBBYMODE_Join && bCreateLobby) ||
				 CampaignLobbyMode == eGCLOBBYMODE_SoloPath)
		{
			CreateLocalCampaign(ControllerIndex, Profile, CoopGameSettings, bCreateLobby, Chapter);
			return true;
		}
	}
	return false;
}

/** Creates a local campaign */
function CreateLocalCampaign(int ControllerIndex,GearProfileSettings Profile,GearCoopGameSettings CoopSettings,bool bCreateLobby,EChapterPoint Chapter)
{
	local LocalPlayer LP;
	local string URL;
	local bool bUseCheckpoint;
	local int SelectedContext;
	local GameViewPortClient VPClient;
	local bool bIsLAN;
	local int CampMode;

	if (bCreateLobby)
	{
		if (CampaignLobbyMode == eGCLOBBYMODE_Join)
		{
			// See if they have access to Live to determine which mode to default them to
			VPClient = GetPlayerOwnerViewportClient(class'UIInteraction'.static.GetPlayerIndex(ControllerIndex));
			// If there is no Live service default to LAN
			if (VPClient.UIController.GetLowestLoginStatusOfControllers() != LS_LoggedIn ||
				!VPClient.UIController.CanAllPlayOnline())
			{
				bIsLAN = true;
			}
			// Set the proper default campaign mode for a player looking to join
			CampMode = bIsLAN ? eGCM_SystemLink : eGCM_LivePublic;
			Profile.SetProfileSettingValueId(class'GearProfileSettings'.const.CAMP_MODE, CampMode);
		}

		URL = BuildCampaignURL(true);
		// Let join still apply ?Listen since we need to go to main menu with connection loss
		if (CampaignLobbyMode != eGCLOBBYMODE_Split)
		{
			 URL $= "?listen";
		}
		`log("Launching local campaign lobby with URL '"$URL$"'");
		LP = GetPlayerOwner();
		LP.Actor.ClientTravel(URL, TRAVEL_Absolute);
	}
	else
	{
		// Figure out whether they are using a checkpoint
		Profile.GetProfileSettingValueId(Profile.const.CampCheckpointUsage,SelectedContext);
		bUseCheckpoint = (SelectedContext == eGEARCHECKPOINT_UseLast);

		// Set the chapter they are going to
		ChangeChapterSettings(Chapter);

		// Build the URL for launching the game
		URL = BuildCampaignURL(false, Chapter, bUseCheckpoint) $ "?listen";
		`log("Launching local campaign game with URL '"$URL$"'");
		LP = GetPlayerOwner();
		LP.Actor.ClientTravel(URL, TRAVEL_Absolute);
	}
}

/**
 * Called when the campaign create finishes publishing with Live
 *
 * @param SessionName the name of the session this event is for
 * @param bWasSuccessful whether the create worked or not
 */
function OnCreateCampaignLobbyComplete(name SessionName,bool bWasSuccessful)
{
	local string URL;
	local LocalPlayer LP;

	if (SessionName == 'Party')
	{
		// Clear the delegate
		if (GameInterface != None)
		{
			GameInterface.ClearCreateOnlineGameCompleteDelegate(OnCreateCampaignLobbyComplete);
		}

		if (bWasSuccessful)
		{
			// Build the URL depending on the slot that we chose
			URL = BuildCampaignURL(true) $ "?listen";
			`Log("Launching LIVE campaign lobby with URL '"$URL$"'");
			LP = GetPlayerOwner();
			LP.Actor.ClientTravel(URL, TRAVEL_Absolute);
		}
		else
		{
			// TODO: robm show error message about network being down
		}
	}
}

/**
 * Called when the campaign create finishes publishing with Live
 *
 * @param SessionName the name of the session this event is for
 * @param bWasSuccessful whether the create worked or not
 */
function OnCreateCampaignComplete(name SessionName,bool bWasSuccessful)
{
	local string URL;
	local bool bUseCheckpoint;
	local int SelectedContext, Act, NormalizedChapter;
	local EChapterPoint Chapter;
	local GearProfileSettings Profile;
	local int ControllerId;

	if (SessionName == 'Party')
	{
		// Clear the delegate
		if (GameInterface != None)
		{
			GameInterface.ClearCreateOnlineGameCompleteDelegate(OnCreateCampaignComplete);
		}

		if (bWasSuccessful)
		{
			// Get the profile for the player
			ControllerId = class'UIInteraction'.static.GetPlayerControllerId(0);
			Profile = GetProfile(ControllerId);

			if (Profile != None)
			{
				// Figure out whether they are using a checkpoint
				Profile.GetProfileSettingValueId(Profile.const.CampCheckpointUsage,SelectedContext);
				bUseCheckpoint = (SelectedContext == eGEARCHECKPOINT_UseLast);
			}

			// Figure out the chapter they are going to
			CoopGameSettings.GetIntProperty(CoopGameSettings.PROPERTY_ACTNUM, Act);
			CoopGameSettings.GetIntProperty(CoopGameSettings.PROPERTY_CHAPTERNUM, NormalizedChapter);
			Chapter = class'GearUIDataStore_GameResource'.static.GetChapterTypeFromNormalizedChapterInAct(EGearAct(Act-1), NormalizedChapter-1);

			// Build the URL depending on the slot that we chose
			URL = BuildCampaignURL(false, Chapter, bUseCheckpoint) $ "?listen";
			`Log("Launching campaign with URL '"$URL$"'");
			class'GearUIScene_Base'.static.ShowLoadingMovie(true);
			WorldInfo.ServerTravel(URL,true);
		}
		else
		{
			// TODO: robm show error message about network being down
		}
	}
}

/**
 * Builds the campaign URL depending on whether we are going to the lobby or
 * continuing a campaign and going straight to the game
 */
function string BuildCampaignURL(bool bLaunchLobby, optional EChapterPoint Chapter, optional bool bUseCheckpoint)
{
	local string URL;

	if ( bLaunchLobby )
	{
		URL = "GearStart?game=GearGameContent.GearCampaignLobbyGame";
	}
	else
	{
		URL = "GearGame_P?game=geargamecontent.GearGameSP" $ "?chapter=" $ int(Chapter);
		if ( bUseCheckpoint )
		{
			URL $= "?loadcheckpoint";
		}
	}

	return URL;
}

/**
 * @return	a reference to the gear-specific static game resource data store.
 */
static final function GearUIDataStore_GameResource GetGameResourceDataStore()
{
	return class'GearUIScene_Base'.static.GetGameResourceDataStore();
}

/**
 * Unsubscribes this object from all online delegates it supports.  Called at level transition to make sure no dangling
 * references are left hanging around.
 */
function ClearOnlineDelegates()
{
	// on console, we specifically do not check for null GameInterface so that we know if this method can't do its thing
	if ( class'UIRoot'.static.IsConsole()
	||	(OnlineSub != None && GameInterface != None) )
	{
		GameInterface.ClearCreateOnlineGameCompleteDelegate(OnCreatePartyComplete);
		GameInterface.ClearCreateOnlineGameCompleteDelegate(OnCreateCampaignComplete);
		GameInterface.ClearCreateOnlineGameCompleteDelegate(OnCreateCampaignLobbyComplete);
	}
}

/**
 * Tells all party members to return to the main menu.
 */
function DisbandParty()
{
	local GearMenuPC MenuPC, LocalPC;

	foreach WorldInfo.AllControllers(class'GearMenuPC',MenuPC)
	{
		if ( !MenuPC.IsLocalPlayerController() )
		{
			MenuPC.ClientPartyLobbyClosed();
		}
		else
		{
			LocalPC = MenuPC;
		}
	}

	LocalPC.ClientShowLoadingMovie(true);
	SetTimer(1.0,false,nameof(DelayedDestroyThenTravel));
}

/** Clean up and travel after notifying clients */
function DelayedDestroyThenTravel()
{
	local PlayerController MenuPC;

	GameInterface.DestroyOnlineGame('Party');

	foreach WorldInfo.AllControllers(class'PlayerController',MenuPC)
	{
		if (MenuPC.IsLocalPlayerController())
		{
			MenuPC.ClientTravel("?closed",TRAVEL_Absolute);
			break;
		}
	}
}

/* === GameInfo interface === */
/**
 * Initializes the party system object for manipulation by the lobby
 */
event InitGame(string Options, out string ErrorMessage)
{
	local GearRecentPlayersList PlayersList;

	bShowCreditsAtStartup = HasOption(Options, "credits");

	Super.InitGame(Options, ErrorMessage);

	if (OnlineSub != None)
	{
		// Grab the nat type so we can use it to help control matchmaking
		if (OnlineSub.SystemInterface != None)
		{
			NatType = OnlineSub.SystemInterface.GetNATType();
			`Log("CanHostVersusMatch() = "$CanHostVersusMatch());
			`Log("CanHostPartyMatch() = "$CanHostPartyMatch());
			`Log("CanInviteToMatch() = "$CanInviteToMatch());
		}
		else
		{
			NatType = NAT_Open;
		}
		// Read the playlist information
		PlaylistMan = OnlinePlaylistManager(OnlineSub.GetNamedInterface('PlaylistManager'));
		if (PlaylistMan != None)
		{
			PlaylistMan.DownloadPlaylist();
		}
		if (Class == class'GearMenuGame')
		{
			PlayersList = GearRecentPlayersList(OnlineSub.GetNamedInterface('RecentPlayersList'));
			if (PlayersList != None)
			{
				// Clear the last match results so that going back to mp doesn't confuse people
				PlayersList.ClearRecentMatchResults();
			}
		}
	}
}

/**
 * If we are traveling back from the game to show credits, change the location
 * to upper right
 */
function SetNetworkNotificationLocation()
{
	if (Class == class'GearMenuGame')
	{
		if (OnlineSub != None &&
			OnlineSub.SystemInterface != None)
		{
			OnlineSub.SystemInterface.SetNetworkNotificationPosition(NNP_TopRight);
		}
	}
	else
	{
		Super.SetNetworkNotificationLocation();
	}
}

/**
 * do nothing!
 */
function StartOnlineGame()
{
}

/**
 * @return	TRUE if the player is allowed to pause the game.
 */
function bool AllowPausing( optional PlayerController PC )
{
	return bPauseable;
}

/*
 * Return whether a team change is allowed.
 */
function bool ChangeTeam(Controller Other, int N, bool bNewTeam)
{
	return true;
}

auto State PendingMatch
{
Begin:
	// don't do anything for menu games
}

defaultproperties
{
	// No spectating
	MaxSpectatorsAllowed=0

	PlayerControllerClass=class'GearGame.GearMenuPC'
	// Don't allow pausing of the game by the host
	bPauseable=false

	HUDType=class'GearGame.GearMenuHUD'
}
