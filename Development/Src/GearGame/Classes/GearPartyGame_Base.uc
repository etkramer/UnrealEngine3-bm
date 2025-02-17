/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * This class is the game info for the party lobby. It's used to build a party,
 * transition to matches, and handle post match reporting, etc.
 *
 * Here is the sequence of events for find party member to fill a team:
 *
 *	Party leader searches for a match that needs the exact number of party members to start
 *	If no matches are available
 *		(see Any search flow)
 *	Otherwise
 *		It searches through the list for a match that meets the ping requirements
 *		If there are no matches (due to being full or too high of a ping)
 *			(see Any search flow)
 *		Then it uses a party beacon to request a reservation
 *		If the reservation is available
 *			It registers all of the party members with that host
 *			Waits for response from host
 *				Upon full party and a destination, tells all party members to travel
 *		Otherwise
 *			It adds the failed session to a list and goes back to the ping check
 *
 *	Any search flow
 *		Party leader searches for any match that can hold the number party members
 *		If no matches are available
 *			(see Create flow)
 *		Otherwise
 *			It searches through the list for a match that meets the ping requirements
 *			If there are no matches (due to being full or too high of a ping)
 *				(see Create flow)
 *			Then it uses a party beacon to request a reservation
 *			If the reservation is available
 *				It registers all of the party members with that host
 *				Waits for response from host
 *					Upon full party and a destination, tells all party members to travel
 *			Otherwise
 *				It adds the failed session to a list and goes back to the ping check
 *
 *	Create flow
 *		Registers the session with Live
 *		Creates a party beacon with the number of reservations it can accept
 *		When all reservations are full
 *			Travels to the pre-game lobby
 *
 * Here is the sequence of events for opponent (party vs. party) matching:
 *
 *	Party leader does a search for available matches
 *	If no matches are available
 *		Creates one (see Create flow)
 *	Otherwise
 *		It searches through the list for a match that meets the ping requirements
 *		If there are no matches (due to being full or too high of a ping)
 *			Creates one (see Create flow)
 *		Then it uses a party beacon to request a reservation
 *		If the reservation is available
 *			It tells all party members to join that session (pre-game lobby)
 *		Otherwise
 *			It adds the failed session to a list and goes back to the ping check
 *
 *	Create flow
 *		Registers the session with Live
 *		Creates a party beacon with the number of reservations it can accept
 *		When all reservations are full
 *			Travels to the pre-game lobby
 */
class GearPartyGame_Base extends GearMenuGame
	dependson(GearPartyGRI);

var	transient UIScene LobbySceneResource;

/** Holds the party game settings that are registered with Live */
var transient GearPartyGameSettings PartySettings;

/** The controller index that is used for searching/creating vs matches */
var int OwningControllerIndex;

/** The current best session to check for reservation space */
var int BestSessionIndex;

/** Contains the list of reservations we tried to make but were full */
var array<int> ReservationsWithNoSpace;

/** Contains the list of reservations we tried and either failed or canceled on us */
var array<UniqueNetId> SessionsToIgnore;

/** The beacon that is used to control reservations */
var PartyBeaconHost HostVsReservationBeacon;

/** The beacon that is used to control reservations */
var PartyBeaconClient ClientVsReservationBeacon;

/** The beacon to use so that it can be platform specific */
var config string ClientPartyBeaconClassName;

/** The instance of the beacon class once loaded */
var class<PartyBeaconClient> ClientPartyBeaconClass;

/** The beacon that is used to control partial party reservations */
var PartyBeaconHost HostPartyReservationBeacon;

/** The beacon that is used to control partial party reservations */
var PartyBeaconClient ClientPartyReservationBeacon;

/** Holds the player skill information for all players in the party */
var GearLeaderboardVersusSkill PartySkill;

/** The search object to use when finding potential matches */
var GearVersusGameSearch VsSearch;

/** The matchmaking state of the party */
var EMatchmakingState MatchmakingState;

/** Whether to enable partial party matching or not */
var config bool bIsUsingPartialPartyMatching;

/** The search object to use when finding parties to fill in */
var GearPartyGameSearch PartialPartySearch;

/** Whether matchmaking has started or not */
var bool bIsMatchmaking;

/** Whether the host is traveling to a match or not */
var bool bIsTraveling;

/** The size of the team in this playlist */
var int PlaylistTeamSize;

/** The number of teams in this playlist */
var int PlaylistTeamCount;

/** Holds either the VsSearch or the PartialPartySearch object when selecting a session */
var OnlineGameSearch SessionSearch;

/** The max ping that we want to allow when searching for sessions */
var int MaxPing;

/** Whether to post matchmaking stats to MCP or not */
var config bool bWantsMatchmakingUploads;

/** Whether there is an async create pending that is blocking canceling */
var bool bHasCreateOutstanding;

/** Whether there is the need for a delete or not */
var bool bHasDeleteRequested;

/** The number of searches that have happened while searching for a team to join */
var int PartialPartySearchCount;

/** The maximum number of times to search before deciding to host */
var config int MaxPartialPartySearchCount;

/** The amount of time a partial party host can remain unjoined before tearing down and searching again */
var config float MaxIdleHostTime;

/** Opens or restarts the updating scene (used for blocking input during async tasks) */
function OpenCogSpin()
{
	local GearUISceneFE_Updating SceneInstance;

	SceneInstance = class'GearUIScene_Base'.static.OpenUpdatingScene();
	if (SceneInstance != None)
	{
		SceneInstance.InitializeUpdatingScene("UpdatingTitle", "LiveStateDesc", 0.5f);
	}
}

/** Begins the process of closing the updating scene (there is a min time the scene must be open) */
function CloseCogSpin()
{
	class'GearUIScene_Base'.static.CloseUpdatingScene();
}

/**
 * If this is a private match, it goes straight to creating a game
 * If it is public, then it reads the skill for matchmaking
 *
 * @param ControllerIndex the index of the player creating the session
 */
function bool StartPartyMatchmaking(int ControllerIndex)
{
	local int MatchMode;
	local GearProfileSettings Profile;
	local bool bNeedsFullPartyMatching;

	// No remote parties are connected currently
	GearPartyGRI(GameReplicationInfo).PartySlotsFilled = 0;
	// Discard our bad session cache
	SessionsToIgnore.Length = 0;

	if (OnlineSub != None)
	{
		OwningControllerIndex = ControllerIndex;

		// Get the profile for the player
		Profile = GearProfileSettings(OnlineSub.PlayerInterface.GetProfileSettings(OwningControllerIndex));
		if (Profile != None)
		{
			// Get the match mode and handle each differently
			Profile.GetProfileSettingValueId(class'GearProfileSettings'.const.VERSUS_MATCH_MODE,MatchMode);

			// Have to be in a party for Public, Custom, and System Link
			if (PartySettings != None)
			{
				switch (MatchMode)
				{
					case eGVMT_Official:
						if (PlaylistMan != None)
						{
							// Find out how big the teams are and how many for the selected playlist id
							PlaylistMan.GetTeamInfoFromPlaylist(GearPartyGRI(GameReplicationInfo).PlaylistId,PlaylistTeamSize,PlaylistTeamCount);
							// If our party matches the team size or partial matching is off, do full party matching
							bNeedsFullPartyMatching = (PlaylistTeamSize == GameReplicationInfo.PRIArray.Length) || (bIsUsingPartialPartyMatching == false);
							if (bNeedsFullPartyMatching)
							{
								// Check team size and see if we need to do partial party matchmaking
								GotoState('PartyMatchmaking');
							}
							else
							{
								// Find other parties to fill out our team
								GotoState('PartialPartyMatchmaking');
							}
						}
						// Set presence indicating that we are matchmaking
						SetClientsRichPresenceStrings(CONTEXT_PRESENCE_PARTYSEARCHPRESENCE);
						break;

					case eGVMT_SystemLink:
						GotoState('SystemLink');
						break;

					case eGVMT_Custom:
						GotoState('PrivateMatch');
						break;
				}
			}
			// No PartySettings so see if this is a local match
			else if ( MatchMode == eGVMT_Local )
			{
				GotoState('LocalMatch');
			}
		}
	}

	return false;
}

/**
 * Reads the skill information for all party members in order to perform searches that
 * take into account the skill of the party as a whole when finding matches
 */
function ReadPartySkillData()
{
	local array<UniqueNetId> Players;
	local int Index;
	local UniqueNetId ZeroId;

	// Allocate the read object if not present
	if (PartySkill == None)
	{
		PartySkill = new class'GearLeaderboardVersusSkill';
	}
	if (OnlineSub != None &&
		OnlineSub.StatsInterface != None)
	{
		// If someone left or joined since the last search or it's empty, refresh the data
		if (PartySkill.Rows.Length != WorldInfo.GRI.PRIArray.Length)
		{
			// Iterate through the PRIs adding them to the list to read
			for (Index = 0; Index < WorldInfo.GRI.PRIArray.Length; Index++)
			{
				if (WorldInfo.GRI.PRIArray[Index].UniqueId != ZeroId)
				{
					Players.AddItem(WorldInfo.GRI.PRIArray[Index].UniqueId);
				}
			}
			// Need to kick off the read
			OnlineSub.StatsInterface.AddReadOnlineStatsCompleteDelegate(OnReadPartySkillComplete);
			OnlineSub.StatsInterface.ReadOnlineStats(Players,PartySkill);
		}
		else
		{
			// Data is already present, so reuse
			OnReadPartySkillComplete(true);
		}
	}
	else
	{
		OnReadPartySkillComplete(false);
	}
}

/**
 * Called once the skill read for the party has completed
 *
 * @param bWasSuccessful whether the stats read was able to complete successfully
 */
function OnReadPartySkillComplete(bool bWasSuccessful)
{
	local int Index;
	local GearPRI PRI;
	local int OldPlayerSkill;

	OnlineSub.StatsInterface.ClearReadOnlineStatsCompleteDelegate(OnReadPartySkillComplete);
	if (bWasSuccessful)
	{
		// Iterate through the PRIs adding them to the list to read
		for (Index = 0; Index < WorldInfo.GRI.PRIArray.Length; Index++)
		{
			PRI = GearPRI(WorldInfo.GRI.PRIArray[Index]);
			if (PRI != None)
			{
				OldPlayerSkill = PRI.PlayerSkill;
				// Get the skill and assign it to the PRI, so it flows to all clients
				PRI.PlayerSkill = PartySkill.GetSkillForPlayer(PRI.UniqueId);
				// Don't tell the client to save, if the value is the same
				if (OldPlayerSkill != PRI.PlayerSkill)
				{
					// Tell the client to save the new skill value
					GearPC(PRI.Owner).ClientSetPlayerSkill(PRI.PlayerSkill);
				}
			}
		}
	}
}

/**
 * Changes what the game's next action is and calls the write handlers
 *
 * @param NewState the new state to use
 */
function ProcessMatchmakingStateChange(EMatchmakingState NewState)
{
	// Set the new state and replicate that information
	MatchmakingState = NewState;
	GearPartyGRI(GameReplicationInfo).MatchmakingState = NewState;

	switch (NewState)
	{
		case MMS_NotMatchmaking:
			bIsMatchmaking = false;
			break;

		case MMS_CancelingMatchmaking:
			ProcessCancelPartyMatchmaking();
			break;

		case MMS_HostCancelingMatchmaking:
			ProcessHostCancelPartyMatchmaking();
			break;

		case MMS_ReadingSkills:
			// Read the skills for this session
			ReadPartySkillData();
			break;

		case MMS_FindingBestParty:
			// Do a search that finds a set of exact matches of our party size
			FindBestPartialParty();
			break;

		case MMS_FindingAnyParty:
			// Do a search that finds a set of matches that can hold our party size
			FindAnyPartialParty();
			break;

		case MMS_FindingOpposingParty:
			// Take the set of people and find a match for them to play
			FindOpponents();
			break;

		case MMS_WaitingForOpposingParty:
		case MMS_ConnectingToOpposingParty:
			// These don't require an action and just update the UI state
			break;

		default:
			break;
	}
}

/**
 * End the session and then restore the invite flags
 */
function CleanupPartyMatchmaking()
{
	OpenCogSpin();
	GameInterface.AddEndOnlineGameCompleteDelegate(OnEndOnlineGameComplete);
	GameInterface.EndOnlineGame('Party');
}

/**
 * Called when ending the party has completed
 *
 * @param bWasSuccessful true if it completed ok, false otherwise
 */
function OnEndOnlineGameComplete(name SessionName,bool bWasSuccessful)
{
	local OnlineGameSettings CurrentSettings;
	local int PartyType;
	local bool bAllowInvites;
	local bool bAllowJoinViaPresence;
	local bool bAllowJoinViaPresenceFriendsOnly;

	GameInterface.ClearEndOnlineGameCompleteDelegate(OnEndOnlineGameComplete);
	// Update the invite settings and then go to not matchmaking
	CurrentSettings = GetCurrentGameSettings();
	// Change the invite flags and update
	if (CanInviteToMatch())
	{
		if (CurrentSettings.GetStringSettingValue(class'GearProfileSettings'.const.VERSUS_PARTY_TYPE,PartyType))
		{
			switch (PartyType)
			{
				case eGVPT_InviteRequired:
					bAllowInvites = true;
					bAllowJoinViaPresence = false;
					bAllowJoinViaPresenceFriendsOnly = false;
					break;
				case eGVPT_FriendsOnly:
					bAllowInvites = true;
					bAllowJoinViaPresence = false;
					bAllowJoinViaPresenceFriendsOnly = true;
					break;
				default:
					bAllowInvites = true;
					bAllowJoinViaPresence = true;
					bAllowJoinViaPresenceFriendsOnly = true;
					break;
			}
		}
	}
	else
	{
		bAllowInvites = false;
		bAllowJoinViaPresence = false;
		bAllowJoinViaPresenceFriendsOnly = false;
	}
	// Tell clients to update their game state to match
	TellClientsToSetPartyInviteFlags(bAllowInvites,bAllowJoinViaPresence,bAllowJoinViaPresenceFriendsOnly);

	SetTimer(3.0,false,nameof(DelayEndUpdateComplete));
}

/**
 * Now transition to being able to start again
 */
function DelayEndUpdateComplete()
{
	CloseCogSpin();
	ProcessMatchmakingStateChange(MMS_NotMatchmaking);
	// Set presence indicating that we available for joining
	SetClientsRichPresenceStrings(CONTEXT_PRESENCE_PARTYWAITPRESENCE);
}

/**
 * Called when the party leader wants to give up on matchmaking
 */
function CancelPartyMatchmaking()
{
	// Can't cancel once the connect starts
	if (MatchmakingState != MMS_ConnectingToOpposingParty)
	{
		ProcessMatchmakingStateChange(MMS_CancelingMatchmaking);
	}
}

/**
 * Does a search that finds a set of parties that need the exact number of players
 * that are in this party to make their party a whole team
 */
function FindBestPartialParty();

/**
 * Does a search that finds a set of parties that can hold the number of players
 * that are in this party to make their party closer to being a whole team
 */
function FindAnyPartialParty();

/**
 * Does a search that finds a set of whole teams that are ready to play
 */
function FindOpponents()
{
	// Set the playlist that we are searching for
	VsSearch.SetIntProperty(PROPERTY_PLAYLISTID,GearPartyGRI(GameReplicationInfo).PlaylistId);

	GameInterface.AddFindOnlineGamesCompleteDelegate(OnFindOpponentsComplete);
	// Now start the search
	GameInterface.FindOnlineGames(OwningControllerIndex,VsSearch);
}

/**
 * Either creates a new versus session or finds a good session and attempts to join
 */
function OnFindOpponentsComplete(bool bWasSuccessful)
{
	GameInterface.ClearFindOnlineGamesCompleteDelegate(OnFindOpponentsComplete);
	if (bWasSuccessful)
	{
		if (MatchMakingState > MMS_CancelingMatchmaking)
		{
			if (VsSearch.Results.Length > 0)
			{
				// Determine the best session to join
				StartSessionMatching();
			}
			else
			{
				if (CanHostVersusMatch())
				{
					// Auto register a new session if none are found
					CreatePublicVersusMatch();
				}
				else
				{
					// Can't host so start searching again
					SetTimer(3.0,false,nameof(FindOpponents));
				}
			}
		}
	}
	else
	{
		//@todo robm -- Add UI error display here
	}
}

/**
 * Builds a list of players that are in this session locally or including remote
 * partial parties
 *
 * @param Members the set of people that is considered to be part of this team
 */
function BuildTeamList(out array<PlayerReservation> Members);

/**
 * Transitions the party session to "in progress"
 */
function bool StartPartyGame()
{
	if (PartySettings != None)
	{
		GameInterface.AddStartOnlineGameCompleteDelegate(OnStartPartyComplete);
		return GameInterface.StartOnlineGame('Party');
	}
	return false;
}

/**
 * Party has moved from gathering to started so search for a match
 *
 * @param SessionName the name of the session the callback is for
 * @param bWasSuccessful whether the create worked or not
 */
function OnStartPartyComplete(name SessionName,bool bWasSuccessful)
{
	GameInterface.ClearStartOnlineGameCompleteDelegate(OnStartPartyComplete);

	// Tell clients to update to match our settings
	TellClientsToSetPartyInviteFlags(false,false,false);
}

/**
 * Creates a party beacon for reserving space, resets all of the
 * session searching vars and starts the reservation search process
 */
function StartSessionMatching()
{
	// Operate on the versus search space
	SessionSearch = VsSearch;
	MaxPing = VsSearch.MaxPing;
	// We are able to start trying to reserve space so start that process
	ResetBestSessionVars();
	ContinueSessionMatching();
}

/**
 * Resets the session searching vars to their default state so that we can search
 * a new list of possible search candidates
 */
function ResetBestSessionVars()
{
	// Zero so that we start evaluating from the beginning
	BestSessionIndex = 0;
	// Clear the previous list of reservations that we attempted
	ReservationsWithNoSpace.Length = 0;
}

/**
 * Search results are sorted by ping (and/or skill) so start from the front and work
 * through the list attempting to negotiate each one
 */
function ChooseBestSession()
{
	local int SessionIndex;

	// Start searching from where we left off
	for (SessionIndex = BestSessionIndex; SessionIndex < SessionSearch.Results.Length; SessionIndex++)
	{
		// Does this session match our max ping value?
		if (SessionSearch.Results[SessionIndex].GameSettings.PingInMs < MaxPing &&
			// Don't choose a session that is in this list since we already know it is full
			ReservationsWithNoSpace.Find(SessionIndex) == INDEX_NONE &&
			// Don't choose a session that cancelled/timedout on us
			SessionsToIgnore.Find('Uid',SessionSearch.Results[SessionIndex].GameSettings.OwningPlayerId.Uid) == INDEX_NONE)
		{
			// Found the match that we want
			BestSessionIndex = SessionIndex;
			return;
		}
	}
	BestSessionIndex = -1;
}

/**
 * Called to find the next best session to attempt to join. Called from both an initial
 * search and from a failed reservation request. When there are no good matches left,
 * it calls the create path
 */
function ContinueSessionMatching()
{
	ChooseBestSession();
	// If we have a valid choice, attempt to connect to it
	if (ReservationsWithNoSpace.Length < SessionSearch.Results.Length &&
		(BestSessionIndex >= 0 && BestSessionIndex < SessionSearch.Results.Length))
	{
		// Try to reserve space in the session
		RequestVersusReservation();
	}
	else
	{
		if (CanHostVersusMatch())
		{
			// We didn't find any sessions in that search, so register one
			CreatePublicVersusMatch();
		}
		else
		{
			// Can't host so start searching again
			SetTimer(3.0,false,nameof(FindOpponents));
		}
	}
}

/**
 * Cleans up any memory and/or sockets used by the searching process
 */
function EndSessionMatching()
{
	SessionSearch = None;
	// No longer needed, so free the memory
	GameInterface.FreeSearchResults();
	// Destroy the beacon too
	if (ClientVsReservationBeacon != None)
	{
		ClientVsReservationBeacon.DestroyBeacon();
		ClientVsReservationBeacon = None;
	}
}

/**
 * Uses the party beacon to request a reservation for the selected session
 */
function RequestVersusReservation()
{
	local array<PlayerReservation> Party;
	local UniqueNetId PartyLeader;
	local OnlineGameSearchResult SessionToCheck;

	// Determine which players to send to the remote host
	BuildTeamList(Party);
	PartyLeader = PartySettings.OwningPlayerId;
	// Grab the search object so we can pass the session information to the beacon
	SessionToCheck = VsSearch.Results[BestSessionIndex];

	// Create a new beacon each time to make sure there is no left over state
	ClientVsReservationBeacon = new ClientPartyBeaconClass;
	ClientVsReservationBeacon.BeaconName = 'ClientVs';

	`Log("Attempting to reserve space in session owned by "$SessionToCheck.GameSettings.OwningPlayerName$" at index "$BestSessionIndex);
	// Send out a reservation request
	if (ClientVsReservationBeacon.RequestReservation(SessionToCheck,PartyLeader,Party))
	{
		// Set the callback so we know when it's done
		ClientVsReservationBeacon.OnReservationRequestComplete = OnVsReservationRequestComplete;
		ClientVsReservationBeacon.OnHostIsReady = OnVsHostIsReady;
		ClientVsReservationBeacon.OnHostHasCancelled = OnVsHostHasCancelled;
	}
	else
	{
		`Log("Failed to send reservation request for BestSessionIndex("$BestSessionIndex$")");
		// Add this one to the failed list
		ReservationsWithNoSpace.AddItem(BestSessionIndex);
		// Find the next best match
		ContinueSessionMatching();
	}
}

/**
 * Put a message up, close down the beacon, and then transition after a while
 */
function OnVsHostHasCancelled()
{
	local OnlineGameSearchResult SessionToCheck;

	ClientVsReservationBeacon.DestroyBeacon();

	`Log("Failed to send reservation request for BestSessionIndex("$BestSessionIndex$")");
	// Grab the last session we connected to so we can make sure to avoid them later
	SessionToCheck = VsSearch.Results[BestSessionIndex];
	SessionsToIgnore.AddItem(SessionToCheck.GameSettings.OwningPlayerId);

	// Find the next best match
	ContinueSessionMatching();
}

/**
 * Called by the beacon when a reservation request has been responded to by the destination host
 *
 * @param ReservationResult whether there was space allocated for the party or not
 */
function OnVsReservationRequestComplete(EPartyReservationResult ReservationResult)
{
	// Clear the delegate since it will be reset if needed
	ClientVsReservationBeacon.OnReservationRequestComplete = None;

	if (ReservationResult != PRR_ReservationAccepted)
	{
		`Log("Reservation request for index ("$BestSessionIndex$") rejected with "$ReservationResult);
		// Add this one to the failed list
		ReservationsWithNoSpace.AddItem(BestSessionIndex);
		// Find the next best match
		ContinueSessionMatching();
	}
}

/**
 * Called once all the teams are filled in on the host
 */
function OnVsHostIsReady()
{
	// We are transitioning so block the cancel
	ProcessMatchmakingStateChange(MMS_ConnectingToOpposingParty);
	// Join the reserved session after letting the host prepare
	SetTimer(1.0,false,nameof(JoinReservedSession));
}

/**
 * Once a session has been reserved, this code notifies the clients, pauses, and
 * joins the session
 */
function JoinReservedSession()
{
	local OnlineGameSearchResult SessionToJoin;
	local byte PlatformInfo[68];

	// Grab the session that was successfully reserved
	SessionToJoin = VsSearch.Results[BestSessionIndex];

	// Get the information needed to travel to this destination
	if (GameInterface.ReadPlatformSpecificSessionInfo(SessionToJoin,PlatformInfo))
	{
		bIsTraveling = true;
		// Have everyone travel to this destination
		TellClientsToTravelToSession('Game',VsSearch.Class,PlatformInfo);

		GameInterface.AddJoinOnlineGameCompleteDelegate(OnJoinVersusMatchComplete);
		// Now join this Session
		GameInterface.JoinOnlineGame(OwningControllerIndex,'Game',SessionToJoin);

		// Kick off the upload of our matchmaking data
		UploadSearchMatckmakingStats();
	}
	else
	{
		//@todo robm -- Add UI error display here
	}
}

/**
 * Uploads the matchmaking stats for the search side of things
 */
function UploadSearchMatckmakingStats()
{
	local array<PlayerReservation> Party;
	local OnlineEventsInterfaceMcp Uploader;

	if (bWantsMatchmakingUploads)
	{
		BuildTeamList(Party);
		if (OnlineSub != None)
		{
			// Ask for the interface by name and cast to our well known type
			Uploader = OnlineEventsInterfaceMcp(OnlineSub.GetNamedInterface('McpUpload'));
			if (Uploader != None)
			{
				Uploader.UploadMatchmakingSearchData(PartySettings.OwningPlayerId,VsSearch,BestSessionIndex,Party);
			}
		}
	}
}

/**
 * Uploads the matchmaking stats for the search side of things
 */
function UploadHostMatchmakingStats()
{
	local array<PlayerReservation> Players;
	local OnlineEventsInterfaceMcp Uploader;
	local int PRIIndex;
	local UniqueNetId ZeroId;
	local PlayerReservation CurrentPlayer;
	local int PartyIndex;
	local int PlayerIndex;

	if (bWantsMatchmakingUploads &&
		OnlineSub != None)
	{
		// Add all of the net ids for the party (same team as host)
		for (PRIIndex = 0; PRIIndex < GameReplicationInfo.PRIArray.Length; PRIIndex++)
		{
			if (GameReplicationInfo.PRIArray[PRIIndex].UniqueId != ZeroId)
			{
				CurrentPlayer.NetId = GameReplicationInfo.PRIArray[PRIIndex].UniqueId;
				if (PartySkill != None)
				{
					PartySkill.FillPlayerResevation(CurrentPlayer.NetId,CurrentPlayer);
				}
				Players.AddItem(CurrentPlayer);
			}
		}
		// Append any partial party players and use the host team as their team
		if (HostPartyReservationBeacon != None)
		{
			// Iterate through the parties adding their information
			for (PartyIndex = 0; PartyIndex < HostPartyReservationBeacon.Reservations.Length; PartyIndex++)
			{
				for (PlayerIndex = 0; PlayerIndex < HostPartyReservationBeacon.Reservations[PartyIndex].PartyMembers.Length; PlayerIndex++)
				{
					CurrentPlayer = HostPartyReservationBeacon.Reservations[PartyIndex].PartyMembers[PlayerIndex];
					Players.AddItem(CurrentPlayer);
				}
			}
		}
		// Append any remote party players
		if (HostVsReservationBeacon != None)
		{
			// Iterate through the parties adding their information
			for (PartyIndex = 0; PartyIndex < HostVsReservationBeacon.Reservations.Length; PartyIndex++)
			{
				for (PlayerIndex = 0; PlayerIndex < HostVsReservationBeacon.Reservations[PartyIndex].PartyMembers.Length; PlayerIndex++)
				{
					CurrentPlayer = HostVsReservationBeacon.Reservations[PartyIndex].PartyMembers[PlayerIndex];
					Players.AddItem(CurrentPlayer);
				}
			}
		}
		// Ask for the interface by name and cast to our well known type
		Uploader = OnlineEventsInterfaceMcp(OnlineSub.GetNamedInterface('McpUpload'));
		if (Uploader != None)
		{
			Uploader.UploadMatchmakingSessionData(GameInterface.GetGameSettings('Game'),Players);
		}
	}
}

/**
 * Has the pary host travel to the session that was just joined
 *
 * @param SessionName the name of the session this event is for
 * @param bWasSuccessful whether the join completed successfully or not
 */
function OnJoinVersusMatchComplete(name SessionName,bool bWasSuccessful)
{
	GameInterface.ClearJoinOnlineGameCompleteDelegate(OnJoinVersusMatchComplete);

	if (SessionName == 'Game')
	{
		if (bWasSuccessful)
		{
			//@todo joeg -- Make the time config
			// Make sure replication has enough time to deliver the join
			SetTimer(1.0,false,nameof(FinishJoinTravel));
			EndSessionMatching();
		}
	}
}

/** Has the host travel to the new session after giving time for clients to get the data */
function FinishJoinTravel()
{
	local string URL;

	// We are joining so grab the connect string to use
	if (GameInterface.GetResolvedConnectString('Game',URL))
	{
		class'GearUIScene_Base'.static.ShowLoadingMovie(true);

		// Make sure all beacons are gone
		DestroyBeacons();

		`Log("Resulting url for 'Game' is ("$URL$")");
		// Trigger a console command to connect to this url
		ConsoleCommand("start " $ URL);
	}
}

/**
 * Used when no acceptable matches are found
 */
function bool CreatePublicVersusMatch()
{
	local GearVersusGameSettings VsSettings;
	local int PlaylistId;

	// Clean up anything left over from searching
	EndSessionMatching();

	if (OnlineSub != None &&
		GameInterface != None)
	{
		VsSettings = new class'GearVersusGameSettings';
		// Set the size of the session to match the max size of players we expect
		VsSettings.NumPublicConnections = PlaylistTeamSize * PlaylistTeamCount;
		// Advertise using the selected playlist id
		PlaylistId = GearGRI(GameReplicationInfo).PlaylistId;
		VsSettings.SetIntProperty(PROPERTY_PLAYLISTID,PlaylistId);

		GameInterface.AddCreateOnlineGameCompleteDelegate(OnCreatePublicVersusMatchComplete);
		if (GameInterface.CreateOnlineGame(OwningControllerIndex,'Game',VsSettings))
		{
			bHasCreateOutstanding = true;
			return true;
		}
	}

	return false;
}

/**
 *  Makes the server travel to the new map
 *
 * @param SessionName the name of the session the callback is for
 * @param bWasSuccessful whether the create worked or not
 */
function OnCreatePublicVersusMatchComplete(name SessionName,bool bWasSuccessful)
{
	GameInterface.ClearCreateOnlineGameCompleteDelegate(OnCreatePublicVersusMatchComplete);

	if (bWasSuccessful)
	{
		if (MatchMakingState > MMS_CancelingMatchmaking)
		{
			// Add our remote party members to the session
			RegisterPartyMembers();
			if (PlaylistTeamCount > 1)
			{
				// Create a party beacon for sending reservation requests
				HostVsReservationBeacon = new class'PartyBeaconHost';
				HostVsReservationBeacon.BeaconName = 'HostVs';
				// Start listening for client reservations
				if (HostVsReservationBeacon.InitHostBeacon(PlaylistTeamCount,PlaylistTeamSize,(PlaylistTeamCount - 1) * PlaylistTeamSize,'Game'))
				{
					HostVsReservationBeacon.OnReservationsFull = OnVsReservationsFull;
				}
				else
				{
					//@todo joeg -- plzfixkthxbye
				}
			}
			else
			{
				// Playing a one team match so just travel
				FinishReservationFull();
			}
		}
	}
	bHasCreateOutstanding = false;
}

/**
 * Registers the party members with the public versus match to speed the removal
 * of this server once another party joins
 */
function RegisterPartyMembers()
{
	local PlayerController PC;
	local array<UniqueNetId> Players;
	local UniqueNetId ZeroId;

	if (GameInterface != None)
	{
		foreach WorldInfo.AllControllers(class'PlayerController',PC)
		{
			if (PC.PlayerReplicationInfo.UniqueId != ZeroId)
			{
				// Locally controlled PCs are added during the create so skip them
				if (!PC.IsLocalPlayerController())
				{
					GameInterface.RegisterPlayer('Game',PC.PlayerReplicationInfo.UniqueId,false);
				}
				// Recalc skill including all players
				Players.AddItem(PC.PlayerReplicationInfo.UniqueId);
			}
		}
		// Register partial party members if there are any
		if (HostPartyReservationBeacon != None)
		{
			HostPartyReservationBeacon.RegisterPartyMembers();
			// Get the XUIDs so we can recalc skill with them
			HostPartyReservationBeacon.GetPlayers(Players);
`if(`notdefined(FINAL_RELEASE))
			`Log("");
			`Log("Party Reservations:");
			HostPartyReservationBeacon.DumpReservations();
			`Log("");
			`Log("");
`endif
		}
		// Now update the skill for the session
		GameInterface.RecalculateSkillRating('Game',Players);
	}
}

/** Called by the party beacon when all the needed parties have joined */
function OnVsReservationsFull()
{
`if(`notdefined(FINAL_RELEASE))
	`Log("");
	`Log("Vs Reservations:");
	HostVsReservationBeacon.DumpReservations();
	`Log("");
	`Log("");
`endif
	// Remove the delegate since it's not needed any more
	HostVsReservationBeacon.OnReservationsFull = None;
	// Tell all clients that it is time to travel
	HostVsReservationBeacon.TellClientsHostIsReady();
	// Delay to allow clean up to happen outside of the delegate
	SetTimer(0.1,false,nameof(FinishReservationFull));
}

/** Cleans up the beacon and travels to the pre-game lobby */
function FinishReservationFull()
{
	local byte PlatformInfo[68];

	if (GameInterface.ReadPlatformSpecificSessionInfoBySessionName('Game',PlatformInfo))
	{
		// show the loading movie.
		class'GearUIScene_Base'.static.ShowLoadingMovie(true);

		bIsTraveling = true;
		// Now set the travel URL and don't notify clients
		WorldInfo.ServerTravel("gearstart?game=GearGameContent.GearPreGameLobbyGame?listen",true,true);

		// Have everyone travel to this destination
		TellClientsToTravelToSession('Game',class'GearVersusGameSearch',PlatformInfo);

		// Build the current players with all their data for the pregame
		BuildCurrentPlayerList();
		// Clean up the beacons
		SetTimer(1.0,false,nameof(DestroyBeacons));

		// Upload the matchmaking stats from the host side
		UploadHostMatchmakingStats();
	}
	else
	{
		//@fixme robm/joeg - Show ui and cleanup state
	}
}

/** Cleans up the beacons involved with matchmaking */
function DestroyBeacons()
{
	if (HostVsReservationBeacon != None)
	{
		// Shut down the beacon if still running
		HostVsReservationBeacon.DestroyBeacon();
		HostVsReservationBeacon = None;
	}
	if (HostPartyReservationBeacon != None)
	{
		// Shut down the beacon if still running
		HostPartyReservationBeacon.DestroyBeacon();
		HostPartyReservationBeacon = None;
	}
}

/**
 * Used when creating a private match
 */
function bool CreatePrivateVersusMatch()
{
	local GearVersusGameSettings VsSettings;
	local OnlineGameSettings CurrentGameSettings;

	if (OnlineSub != None &&
		GameInterface != None)
	{
		CurrentGameSettings = GetCurrentGameSettings(true);

		// Create a non-arbitrated version of the object for the pre-game to use
		VsSettings = new class'GearVersusGameSettingsPrivate'(CurrentGameSettings);
		VsSettings.bUsesArbitration = false;
		VsSettings.NumPublicConnections = PartySettings.NumPublicConnections;
		VsSettings.NumPrivateConnections = PartySettings.NumPrivateConnections;

		GameInterface.AddCreateOnlineGameCompleteDelegate(OnCreatePrivateVersusMatchComplete);
		return GameInterface.CreateOnlineGame(OwningControllerIndex,'Game',VsSettings);
	}

	return false;
}

/**
 *  Makes the server travel to the new map
 *
 * @param SessionName the name of the session the callback is for
 * @param bWasSuccessful whether the create worked or not
 */
function OnCreatePrivateVersusMatchComplete(name SessionName,bool bWasSuccessful)
{
	local byte PlatformInfo[68];

	GameInterface.ClearCreateOnlineGameCompleteDelegate(OnCreatePrivateVersusMatchComplete);

	if (bWasSuccessful)
	{
		if (GameInterface.ReadPlatformSpecificSessionInfoBySessionName('Game',PlatformInfo))
		{
			// Add our remote party members to the session
			RegisterPartyMembers();

			// show the loading movie.
			class'GearUIScene_Base'.static.ShowLoadingMovie(true);

			bIsTraveling = true;
			// Now set the travel URL and don't notify clients
			WorldInfo.ServerTravel("gearstart?game=GearGameContent.GearPreGameLobbyGame?listen",true,true);

			// Have everyone travel to this destination
			TellClientsToTravelToSession('Game',class'GearVersusGameSearchPrivate',PlatformInfo);
		}
	}
	else
	{
		//@fixme robm/joeg - Show ui and cleanup state
	}
}

/**
 * Tells the clients to start their game and then reads the skill data
 */
function StartParty()
{
	local PlayerController PC;

	// Block players from joining
	bIsMatchmaking = true;

	// Tell clients to mark their game as started
	foreach WorldInfo.AllControllers(class'PlayerController',PC)
	{
		// The game will do this post skill read locally, so just do remote clients
		if (!PC.IsLocalPlayerController())
		{
			PC.ClientStartOnlineGame();
		}
	}
	// Read the skills and then start our session when done
	ProcessMatchmakingStateChange(MMS_ReadingSkills);
}

/**
 * Build the current players list with all of the local, networked, and remote party information
 */
function BuildCurrentPlayerList()
{
	local int PRIIndex;
	local UniqueNetId ZeroId;
	local CurrentPlayerMet CurrentPlayer;
	local int HostTeamNum;
	local array<CurrentPlayerMet> Players;
	local OnlineRecentPlayersList PlayersList;

	if (HostVsReservationBeacon != None)
	{
		HostTeamNum = HostVsReservationBeacon.ReservedHostTeamNum;
	}
	// Add all of the net ids for the party (same team as host)
	for (PRIIndex = 0; PRIIndex < GameReplicationInfo.PRIArray.Length; PRIIndex++)
	{
		if (GameReplicationInfo.PRIArray[PRIIndex].UniqueId != ZeroId)
		{
			CurrentPlayer.NetId = GameReplicationInfo.PRIArray[PRIIndex].UniqueId;
			CurrentPlayer.Skill = GameReplicationInfo.PRIArray[PRIIndex].PlayerSkill;
			CurrentPlayer.TeamNum = HostTeamNum;
			Players.AddItem(CurrentPlayer);
		}
	}
	// Append any partial party players and use the host team as their team
	if (HostPartyReservationBeacon != None)
	{
		CopyCurrentPlayersFromBeacon(HostPartyReservationBeacon,Players,HostTeamNum);
	}
	// Append any remote party players
	if (HostVsReservationBeacon != None)
	{
		CopyCurrentPlayersFromBeacon(HostVsReservationBeacon,Players);
	}
	// Now cache this information in the recent player list since that persists
	PlayersList = OnlineRecentPlayersList(OnlineSub.GetNamedInterface('RecentPlayersList'));
	if (PlayersList != None)
	{
		PlayersList.SetCurrentPlayersList(Players);
	}
}

/**
 * Copies the player information from the beacon into the array
 *
 * @param Beacon the beacon to read the players from
 * @param Players the array that is appended to
 * @param HostTeamOverride the team to use if they are on the host
 */
function CopyCurrentPlayersFromBeacon(PartyBeaconHost Beacon,out array<CurrentPlayerMet> Players,optional int HostTeamOverride = -1)
{
	local int PartyIndex;
	local int PlayerIndex;
	local PlayerReservation PlayerRes;
	local CurrentPlayerMet CurrentPlayer;

	// Iterate through the parties adding their information
	for (PartyIndex = 0; PartyIndex < Beacon.Reservations.Length; PartyIndex++)
	{
		for (PlayerIndex = 0; PlayerIndex < Beacon.Reservations[PartyIndex].PartyMembers.Length; PlayerIndex++)
		{
			PlayerRes = Beacon.Reservations[PartyIndex].PartyMembers[PlayerIndex];
			// Copy the data over to the other struct
			CurrentPlayer.NetId = PlayerRes.NetId;
			CurrentPlayer.Skill = PlayerRes.Skill;
			CurrentPlayer.TeamNum = HostTeamOverride == -1 ? Beacon.Reservations[PartyIndex].TeamNum : HostTeamOverride;
			Players.AddItem(CurrentPlayer);
		}
	}
}

/** Overriden in the state specific code to stop the matchmaking process */
function ProcessCancelPartyMatchmaking();

/** Overriden in the state specific code to reset the process to searching */
function ProcessHostCancelPartyMatchmaking();

/**
 * Uses the cancel on all of party beacons that are present
 */
function CancelPartyBeacons()
{
	if (HostPartyReservationBeacon != None)
	{
		// Tell all clients that we are cancelling
		HostPartyReservationBeacon.TellClientsHostHasCancelled();
		// Clear all delegates and set a timer to destroy the beacon
		HostPartyReservationBeacon.OnReservationChange = None;
		HostPartyReservationBeacon.OnReservationsFull = None;
		HostPartyReservationBeacon.OnClientCancellationReceived = None;
		SetTimer(3.0,false,nameof(OnHostPartyCancellationRequestComplete));
	}
	else if (ClientPartyReservationBeacon != None)
	{
		// Tell any party host that we are cancelling
		ClientPartyReservationBeacon.CancelReservation(PartySettings.OwningPlayerId);
		// Clear all delegates except the acknowledgement since that's all we care about
		ClientPartyReservationBeacon.OnReservationRequestComplete = None;
		ClientPartyReservationBeacon.OnReservationCountUpdated = None;
		ClientPartyReservationBeacon.OnTravelRequestReceived = None;
		ClientPartyReservationBeacon.OnCancellationRequestComplete = OnPartyCancellationRequestComplete;
		// If the host doesn't respond in 3 seconds, bail
		SetTimer(3.0,false,nameof(OnPartyCancellationRequestComplete));
	}
}

/**
 * Uses the cancel on all of versus beacons that are present
 */
function CancelVersusBeacons()
{
	// Notify versus ones too
	if (HostVsReservationBeacon != None)
	{
		// Tell all clients that we are cancelling
		HostVsReservationBeacon.TellClientsHostHasCancelled();
		// Clear all delegates and set a timer to destroy the beacon
		HostVsReservationBeacon.OnReservationChange = None;
		HostVsReservationBeacon.OnReservationsFull = None;
		HostVsReservationBeacon.OnClientCancellationReceived = None;
		SetTimer(3.0,false,nameof(OnHostVsCancellationRequestComplete));
	}
	else if (ClientVsReservationBeacon != None)
	{
		// Tell any party host that we are cancelling
		ClientVsReservationBeacon.CancelReservation(PartySettings.OwningPlayerId);
		// Clear all delegates except the acknowledgement since that's all we care about
		ClientVsReservationBeacon.OnReservationRequestComplete = None;
		ClientVsReservationBeacon.OnReservationCountUpdated = None;
		ClientVsReservationBeacon.OnTravelRequestReceived = None;
		ClientVsReservationBeacon.OnCancellationRequestComplete = OnVsCancellationRequestComplete;
		// If the host doesn't respond in 3 seconds, bail
		SetTimer(3.0,false,nameof(OnVsCancellationRequestComplete));
	}
}

/**
 * Timer used to delay clean up of messages until clients have had time to process them
 */
function OnHostPartyCancellationRequestComplete()
{
	HostPartyReservationBeacon.DestroyBeacon();
	HostPartyReservationBeacon = None;
}

/**
 * Called when the partial party host acknowledges our request
 */
function OnPartyCancellationRequestComplete()
{
	SetTimer(0.0,false,nameof(OnPartyCancellationRequestComplete));
	// Safe to delete the beacon now
	ClientPartyReservationBeacon.DestroyBeacon();
	ClientPartyReservationBeacon = None;
}

/**
 * Timer used to delay clean up of messages until clients have had time to process them
 */
function OnHostVsCancellationRequestComplete()
{
	HostVsReservationBeacon.DestroyBeacon();
	HostVsReservationBeacon = None;
}

/**
 * Called when the partial party host acknowledges our request
 */
function OnVsCancellationRequestComplete()
{
	SetTimer(0.0,false,nameof(OnVsCancellationRequestComplete));
	if (ClientVsReservationBeacon != None)
	{
		// Safe to delete the beacon now
		ClientVsReservationBeacon.DestroyBeacon();
		ClientVsReservationBeacon = None;
	}
}

/**
 * The state that handles full party matchmaking
 */
state PartyMatchmaking
{
	/**
	 * Does a search that finds a set of whole teams that are ready to play
	 */
	function FindOpponents()
	{
		local UniqueNetId ZeroId;
		local int PRIIndex;

		// Don't touch these if previously filled by the skill read
		if (VsSearch.ManualSkillOverride.Players.Length == 0)
		{
			// Set the skill leaderboard to read from
			VsSearch.ManualSkillOverride.LeaderboardId = STATS_VIEW_SKILL_RANKED_VERSUS;
			// Iterate all PRIs and add those players to the search for skill reasons
			for (PRIIndex = 0; PRIIndex < GameReplicationInfo.PRIArray.Length; PRIIndex++)
			{
				if (GameReplicationInfo.PRIArray[PRIIndex].UniqueId != ZeroId)
				{
					VsSearch.ManualSkillOverride.Players.AddItem(GameReplicationInfo.PRIArray[PRIIndex].UniqueId);
				}
			}
		}
		// Use the base function to search
		Global.FindOpponents();
	}

	/**
	 * Called once the skill read for the party has completed. It is then copied to the
	 * search object so that the search can reuse the resulting data
	 *
	 * @param bWasSuccessful whether the stats read was able to complete successfully
	 */
	function OnReadPartySkillComplete(bool bWasSuccessful)
	{
		Super.OnReadPartySkillComplete(bWasSuccessful);

		if (MatchMakingState > MMS_CancelingMatchmaking)
		{
			if (bWasSuccessful)
			{
				// Copy the skill information to the search object
				PartySkill.CopySkillDataToSearch(VsSearch);
			}
			StartPartyGame();
		}
	}

	/**
	 * Party has moved from gathering to started so search for a match
	 *
	 * @param SessionName the name of the session the callback is for
	 * @param bWasSuccessful whether the create worked or not
	 */
	function OnStartPartyComplete(name SessionName,bool bWasSuccessful)
	{
		Super.OnStartPartyComplete(SessionName,bWasSuccessful);
		if (PlaylistTeamCount > 1)
		{
			// Now move to our next step
			ProcessMatchmakingStateChange(MMS_FindingOpposingParty);
		}
		else
		{
			CreatePublicVersusMatch();
		}
	}

	/**
	 * Called when the party leader wants to give up on matchmaking
	 */
	function ProcessCancelPartyMatchmaking()
	{
		CancelVersusBeacons();
		// Clear this timer if it was present
		SetTimer(0.0,false,nameof(FindOpponents));
		// Clear all online delegates that are in process
		GameInterface.ClearFindOnlineGamesCompleteDelegate(OnFindOpponentsComplete);
		OnlineSub.StatsInterface.ClearReadOnlineStatsCompleteDelegate(OnReadPartySkillComplete);
		// Queue the deleting in an async way
		SetTimer(3.0,false,nameof(DelayedGameSessionDelete));
	}

	/** Delays while a create is in progress */
	function DelayedGameSessionDelete()
	{
		if (!bHasCreateOutstanding)
		{
			// Destroy the game if it was created
			if (GameInterface.GetGameSettings('Game') != None)
			{
				GameInterface.DestroyOnlineGame('Game');
			}
			// Delay for a bit and then transition to not matchmaking
			SetTimer(3.0,false,nameof(OnCancelMatchmakingDelayComplete));
		}
		else
		{
			// Retry in a few seconds
			SetTimer(3.0,false,nameof(DelayedGameSessionDelete));
		}
	}

	/**
	 * Delayed transition to not matchmaking
	 */
	function OnCancelMatchmakingDelayComplete()
	{
		CleanupPartyMatchmaking();
	}

	/**
	 * Builds the team list for the party mode in question. The full party mode
	 * just uses the PRI array. The partial uses PRI and remote players
	 *
	 * @param Members the array that is being populated
	 */
	function BuildTeamList(out array<PlayerReservation> Members)
	{
		local int PRIIndex;
		local UniqueNetId ZeroId;
		local PlayerReservation PlayerRes;

		// Add all of the net ids for the party
		for (PRIIndex = 0; PRIIndex < GameReplicationInfo.PRIArray.Length; PRIIndex++)
		{
			if (GameReplicationInfo.PRIArray[PRIIndex].UniqueId != ZeroId)
			{
				PlayerRes.NetId = GameReplicationInfo.PRIArray[PRIIndex].UniqueId;
				// Get the skill information for this player
				if (PartySkill != None)
				{
					PartySkill.FillPlayerResevation(PlayerRes.NetId,PlayerRes);
				}
				Members.AddItem(PlayerRes);
			}
		}
	}

Begin:
	// Start the party matchmaking
	StartParty();
}

/**
 * The state that handles partial party matchmaking. It finds other parties that need
 * more players before being able to play and has them merge to find an opponent.
 */
state PartialPartyMatchmaking
{
	/**
	 * Does a search that finds a set of parties that need the exact number of players
	 * that are in this party to make their party a whole team
	 */
	function FindBestPartialParty()
	{
		// Keep incrementing this, so that we will start rolling the dice
		PartialPartySearchCount++;

		// Set the search to any match that can take the party
		SetPartialPartySearch(CONTEXT_PARTYSLOT_NEEDS);

		GameInterface.AddFindOnlineGamesCompleteDelegate(OnFindBestPartialPartyComplete);
		// Search for the exact match
		GameInterface.FindOnlineGames(OwningControllerIndex,PartialPartySearch);
	}

	/**
	 * When the search in complete, it starts the process of checking reservations
	 *
	 * @param bWasSuccessful whether the search worked or not
	 */
	function OnFindBestPartialPartyComplete(bool bWasSuccessful)
	{
		GameInterface.ClearFindOnlineGamesCompleteDelegate(OnFindBestPartialPartyComplete);
		// If the search fails or returns no results, then move to searching any match
		if (bWasSuccessful && PartialPartySearch.Results.Length > 0)
		{
			// Search for parties to fill
			StartPartialPartyMatching();
		}
		else
		{
			// Delay and then search again for sessions
			SetTimer(1.5,false,nameof(DelayedFindAnyPartialParty));
		}
	}

	/**
	 * Changes the searching state when called
	 */
	function DelayedFindAnyPartialParty()
	{
		if (MatchmakingState != MMS_CancelingMatchmaking)
		{
			ProcessMatchmakingStateChange(MMS_FindingAnyParty);
		}
	}

	/**
	 * Does a search that finds a set of parties that can hold the number of players
	 * that are in this party to make their party closer to being a whole team
	 */
	function FindAnyPartialParty()
	{
		// Set the search to any match that can take the party
		SetPartialPartySearch(CONTEXT_PARTYSLOT_WANTS);

		GameInterface.AddFindOnlineGamesCompleteDelegate(OnFindAnyPartialPartyComplete);
		// Now start the search
		GameInterface.FindOnlineGames(OwningControllerIndex,PartialPartySearch);
	}

	/**
	 * When the search in complete, it starts the process of checking reservations
	 *
	 * @param bWasSuccessful whether the search worked or not
	 */
	function OnFindAnyPartialPartyComplete(bool bWasSuccessful)
	{
		GameInterface.ClearFindOnlineGamesCompleteDelegate(OnFindAnyPartialPartyComplete);
		// If the search fails or returns no results, then move to searching any match
		if (bWasSuccessful && PartialPartySearch.Results.Length > 0)
		{
			// Search for parties to fill
			StartPartialPartyMatching();
		}
		else
		{
			// If our NAT is ok and we have searched enough (plus random), then host
			if (CanHostPartyMatch() && ShouldSwitchToPartialPartyHost())
			{
				// We need to register ourselves as the gatherer of parties
				SwitchToPartialPartyLeader();
			}
			// Otherwise, we need to continue to fill in matches
			else
			{
				// Delay and then search again for sessions
				SetTimer(3.0,false,nameof(DelayedFindBestPartialParty));
			}
		}
	}

	/**
	 * Changes state to finding best party when found
	 */
	function DelayedFindBestPartialParty()
	{
		if (MatchmakingState != MMS_CancelingMatchmaking)
		{
			ProcessMatchmakingStateChange(MMS_FindingBestParty);
		}
	}

	/**
	 * Does a search that finds a set of whole teams that are ready to play
	 */
	function FindOpponents()
	{
		// Add the local party skill information to the search
		PartySkill.CopySkillDataToSearch(VsSearch);
		// Now append the remote player (partial party) skill information to the search
		HostPartyReservationBeacon.AppendReservationSkillsToSearch(VsSearch);

		// Use the base function to search
		Global.FindOpponents();
	}

	/**
	 * Called once the skill read for the party has completed. It is then copied to the
	 * search object so that the search can reuse the resulting data
	 *
	 * @param bWasSuccessful whether the stats read was able to complete successfully
	 */
	function OnReadPartySkillComplete(bool bWasSuccessful)
	{
		Super.OnReadPartySkillComplete(bWasSuccessful);

		if (MatchMakingState > MMS_CancelingMatchmaking)
		{
			if (bWasSuccessful)
			{
				// Copy the skill information to the search object
				PartySkill.CopySkillDataToSearch(VsSearch);
			}
			StartPartyGame();
		}
	}

	/**
	 * Party has moved from gathering to started so search for players to fill our match
	 *
	 * @param SessionName the name of the session the callback is for
	 * @param bWasSuccessful whether the create worked or not
	 */
	function OnStartPartyComplete(name SessionName,bool bWasSuccessful)
	{
		Super.OnStartPartyComplete(SessionName,bWasSuccessful);
		// Now move to our next step
		ProcessMatchmakingStateChange(MMS_FindingBestParty);
	}

	/**
	 * Called when the partial party leader cancelled and we need to start searching again
	 */
	function ProcessHostCancelPartyMatchmaking()
	{
		if (ClientPartyReservationBeacon != None)
		{
			ClientPartyReservationBeacon.DestroyBeacon();
			ClientPartyReservationBeacon = None;
		}
		SetTimer(6.0,false,nameof(OnHostCancelPartyMatchmakingDelayComplete));
	}

	/**
	 * Lets the user see the host cancel message before trying to find a new one
	 */
	function OnHostCancelPartyMatchmakingDelayComplete()
	{
		ProcessMatchmakingStateChange(MMS_FindingBestParty);
	}

	/**
	 * Called when the party leader wants to give up on matchmaking
	 */
	function ProcessCancelPartyMatchmaking()
	{
		// Clear the party beacons
		CancelPartyBeacons();
		CancelVersusBeacons();
		// Clear these timers if present
		SetTimer(0.0,false,nameof(FindOpponents));
		SetTimer(0.0,false,nameof(DelayedFindBestPartialParty));
		SetTimer(0.0,false,nameof(DelayedFindAnyPartialParty));
		SetTimer(0.0,false,nameof(HostIdleTimer));
		// Clear all online delegates that are in process
		GameInterface.ClearFindOnlineGamesCompleteDelegate(OnFindBestPartialPartyComplete);
		GameInterface.ClearFindOnlineGamesCompleteDelegate(OnFindAnyPartialPartyComplete);
		GameInterface.ClearFindOnlineGamesCompleteDelegate(OnFindOpponentsComplete);
		OnlineSub.StatsInterface.ClearReadOnlineStatsCompleteDelegate(OnReadPartySkillComplete);
		// Reset the party back to where we want it
		ClearNeedsWants();
		// Queue the deleting in an async way
		SetTimer(3.0,false,nameof(DelayedGameSessionDelete));
	}

	/** Delays while a create is in progress */
	function DelayedGameSessionDelete()
	{
		if (!bHasCreateOutstanding)
		{
			// Destroy the game if it was created
			if (GameInterface.GetGameSettings('Game') != None)
			{
				GameInterface.DestroyOnlineGame('Game');
			}
			// Delay for a bit and then transition to not matchmaking
			SetTimer(3.0,false,nameof(OnCancelMatchmakingDelayComplete));
		}
		else
		{
			// Retry in a few seconds
			SetTimer(3.0,false,nameof(DelayedGameSessionDelete));
		}
	}

	/**
	 * Delayed transition to not matchmaking
	 */
	function OnCancelMatchmakingDelayComplete()
	{
		CleanupPartyMatchmaking();
	}

	/**
	 * Resets the partial party matching state
	 */
	function ClearNeedsWants()
	{
		// Set the playlist that we are searching for
		PartySettings.SetIntProperty(PROPERTY_PLAYLISTID,GearPartyGRI(GameReplicationInfo).PlaylistId);

		// Set these to not joinable
		PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_1,CONTEXT_PARTYSLOT_BLOCKED);
		PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_2,CONTEXT_PARTYSLOT_BLOCKED);
		PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_3,CONTEXT_PARTYSLOT_BLOCKED);
		PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_4,CONTEXT_PARTYSLOT_BLOCKED);

		// Update Live to match this
		GameInterface.UpdateOnlineGame('Party',PartySettings,true);
	}

	/**
	 * Builds the team list for the party mode in question. The full party mode
	 * just uses the PRI array. The partial uses PRI and remote players
	 *
	 * @param Members the array that is being populated
	 */
	function BuildTeamList(out array<PlayerReservation> Members)
	{
		local int PRIIndex;
		local int ReservationIndex;
		local int PartyMemberIndex;
		local UniqueNetId ZeroId;
		local PlayerReservation PlayerRes;

		// Add all of the net ids for the party
		for (PRIIndex = 0; PRIIndex < GameReplicationInfo.PRIArray.Length; PRIIndex++)
		{
			if (GameReplicationInfo.PRIArray[PRIIndex].UniqueId != ZeroId)
			{
				PlayerRes.NetId = GameReplicationInfo.PRIArray[PRIIndex].UniqueId;
				// Get the skill information for this player
				if (PartySkill != None)
				{
					PartySkill.FillPlayerResevation(PlayerRes.NetId,PlayerRes);
				}
				Members.AddItem(PlayerRes);
			}
		}
		// Append any players that have a partial party reservation with us to our list
		// so that they are counted as part of our party
		if (HostPartyReservationBeacon != None)
		{
			// For each party that has made a reservation
			for (ReservationIndex = 0;
				ReservationIndex < HostPartyReservationBeacon.Reservations.Length;
				ReservationIndex++)
			{
				// Add each of that parties members to the list
				for (PartyMemberIndex = 0;
					PartyMemberIndex < HostPartyReservationBeacon.Reservations[ReservationIndex].PartyMembers.Length;
					PartyMemberIndex++)
				{
					Members.AddItem(HostPartyReservationBeacon.Reservations[ReservationIndex].PartyMembers[PartyMemberIndex]);
				}
			}
		}
	}

	/**
	 * Sets up the searching based on the number of players and whether we are
	 * looking for a need or a want
	 *
	 * @param NeedOrWant whether we are searching for exact fit or can hold
	 */
	function SetPartialPartySearch(int NeedOrWant)
	{
		local int NeedSize;

		// Set the playlist that we are searching for
		PartialPartySearch.SetIntProperty(PROPERTY_PLAYLISTID,GearPartyGRI(GameReplicationInfo).PlaylistId);

		// Reset these to the defaults
		PartialPartySearch.SetStringSettingValue(CONTEXT_PARTYSLOT_1,CONTEXT_PARTYSLOT_ANY);
		PartialPartySearch.SetStringSettingValue(CONTEXT_PARTYSLOT_2,CONTEXT_PARTYSLOT_ANY);
		PartialPartySearch.SetStringSettingValue(CONTEXT_PARTYSLOT_3,CONTEXT_PARTYSLOT_ANY);
		PartialPartySearch.SetStringSettingValue(CONTEXT_PARTYSLOT_4,CONTEXT_PARTYSLOT_ANY);

		NeedSize = GameReplicationInfo.PRIArray.Length;
		// Search for a match that needs the number of players that we have
		switch (NeedSize)
		{
			case 4:
				PartialPartySearch.SetStringSettingValue(CONTEXT_PARTYSLOT_4,NeedOrWant);
				break;
			case 3:
				PartialPartySearch.SetStringSettingValue(CONTEXT_PARTYSLOT_3,NeedOrWant);
				break;
			case 2:
				PartialPartySearch.SetStringSettingValue(CONTEXT_PARTYSLOT_2,NeedOrWant);
				break;
			case 1:
				PartialPartySearch.SetStringSettingValue(CONTEXT_PARTYSLOT_1,NeedOrWant);
				break;
		}
	}

	/**
	 * Updates what we have advertised in terms of partial party matching. Includes counts
	 * from remote parties that will fill in our party
	 */
	function UpdateNeedWantsStatus()
	{
		local int NeedSize;

		// Our needs are based off of playlist size, our party, and externally bound parties
		NeedSize = PlaylistTeamSize - GameReplicationInfo.PRIArray.Length - HostPartyReservationBeacon.NumConsumedReservations;

		// Set the playlist that we are searching for
		PartySettings.SetIntProperty(PROPERTY_PLAYLISTID,GearPartyGRI(GameReplicationInfo).PlaylistId);

		// Set these to not joinable
		PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_1,CONTEXT_PARTYSLOT_BLOCKED);
		PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_2,CONTEXT_PARTYSLOT_BLOCKED);
		PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_3,CONTEXT_PARTYSLOT_BLOCKED);
		PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_4,CONTEXT_PARTYSLOT_BLOCKED);

		// Set the size for our optimal and acceptable partial parties
		switch (NeedSize)
		{
			case 4:
				PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_4,CONTEXT_PARTYSLOT_NEEDS);
				PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_3,CONTEXT_PARTYSLOT_WANTS);
				PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_2,CONTEXT_PARTYSLOT_WANTS);
				PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_1,CONTEXT_PARTYSLOT_WANTS);
				break;
			case 3:
				PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_3,CONTEXT_PARTYSLOT_NEEDS);
				PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_2,CONTEXT_PARTYSLOT_WANTS);
				PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_1,CONTEXT_PARTYSLOT_WANTS);
				break;
			case 2:
				PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_2,CONTEXT_PARTYSLOT_NEEDS);
				PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_1,CONTEXT_PARTYSLOT_WANTS);
				break;
			case 1:
				PartySettings.SetStringSettingValue(CONTEXT_PARTYSLOT_1,CONTEXT_PARTYSLOT_NEEDS);
				break;
		}
		// Now update the settings so that players can find them
		GameInterface.UpdateOnlineGame('Party',PartySettings,true);
	}

	/**
	 * Changes the options on the party so that we appear in searches from other
	 * partial parties.
	 */
	function SwitchToPartialPartyLeader()
	{
		local int NeedSize;

		NeedSize = PlaylistTeamSize - GameReplicationInfo.PRIArray.Length;

		// Create the host beacon so parties can find us
		HostPartyReservationBeacon = new class'PartyBeaconHost';
		HostPartyReservationBeacon.BeaconName = 'HostParty';
		// Change the port so that vs & party beacons can coexist
		HostPartyReservationBeacon.PartyBeaconPort++;
		// Start listening for client reservations
		if (HostPartyReservationBeacon.InitHostBeacon(1,NeedSize,NeedSize,'Game'))
		{
			HostPartyReservationBeacon.OnReservationsFull = OnPartyReservationsFull;
			HostPartyReservationBeacon.OnReservationChange = OnPartyReservationChange;
			HostPartyReservationBeacon.OnClientCancellationReceived = OnPartyClientCancellationReceived;
		}

		// Set the online state to match what we are looking for
		UpdateNeedWantsStatus();

		// Set the timer used to restart searching if nobody joins us
		SetTimer(MaxIdleHostTime,false,nameof(HostIdleTimer));
	}

	/** Called when we need to destroy the current host session and start searching again */
	function HostIdleTimer()
	{
		if (MatchmakingState != MMS_CancelingMatchmaking)
		{
			// Reset the number of times we've searched
			PartialPartySearchCount = 0;
			// Stop the beacon so no one joins while we tear down
			HostPartyReservationBeacon.DestroyBeacon();
			HostPartyReservationBeacon = None;
			// Change our advertised state
			ClearNeedsWants();
			// Now delay and start searching
			SetTimer(3.0,false,nameof(DelayedFindBestPartialParty));
		}
	}

	/**
	 * Once the partial party has a full team, we can start seaching for opponents
	 */
	function OnPartyReservationsFull()
	{
		`Log("Reservation full time to search again");
		SetTimer(0.0,false,nameof(HostIdleTimer));
		ProcessMatchmakingStateChange(MMS_FindingOpposingParty);
	}

	/**
	 * Updates the GRI with the number of slots consumed by remote parties
	 */
	function OnPartyReservationChange()
	{
		if (HostPartyReservationBeacon.NumConsumedReservations > 0)
		{
			// Clear our timer that causes us to start searching since we have a client
			SetTimer(0.0,false,nameof(HostIdleTimer));
		}
		else
		{
			// Reset the timer since we don't have any connections now
			SetTimer(MaxIdleHostTime,false,nameof(HostIdleTimer));
		}
		// Replicate our filled count
		GearPartyGRI(GameReplicationInfo).PartySlotsFilled = HostPartyReservationBeacon.NumConsumedReservations;
		// Keep the party not searchable until we can clean up properly
		if (!bHasCreateOutstanding && !bHasDeleteRequested)
		{
			// Update what we can accept
			UpdateNeedWantsStatus();
		}
	}

	/**
	 * Called by the beacon when a client cancels a reservation
	 *
	 * @param PartyLeader the party leader that is cancelling the reservation
	 */
	function OnPartyClientCancellationReceived(UniqueNetId PartyLeader)
	{
		local OnlineGameSettings GameSettings;

		CancelVersusBeacons();
		// Clean up any find opponents search
		GameInterface.ClearFindOnlineGamesCompleteDelegate(OnFindOpponentsComplete);
		if (!bHasCreateOutstanding)
		{
			GameSettings = GameInterface.GetGameSettings('Game');
			if (GameSettings != None &&
				!GameSettings.bHasSkillUpdateInProgress)
			{
				`Log("Deleting immediately since create has completed");
				// Clean up any game object and wait for full again
				GameInterface.DestroyOnlineGame('Game');
			}
			else
			{
				`Log("Using deferred delete because skill update is in progress");
				bHasDeleteRequested = true;
				SetTimer(3.0,false,nameof(DeferredDeleteDueToCancel));
			}
		}
		else
		{
			`Log("Using deferred delete since a create is outstanding");
			bHasDeleteRequested = true;
		}
		// Don't really change state since we aren't recreating beacons
		MatchmakingState = MMS_FindingAnyParty;
		GearPartyGRI(GameReplicationInfo).MatchmakingState = MatchmakingState;
	}

	/** Timer to allow deletion after skill updates */
	function DeferredDeleteDueToCancel()
	{
		`Log("Deleting game session due to timed deferred delete request");
		bHasDeleteRequested = false;
		// A client caused us to need to start searching while the create was happening, so destroy this and update our beacon
		if (GameInterface.GetGameSettings('Game') != None)
		{
			GameInterface.DestroyOnlineGame('Game');
		}
		// Re-advertise our state so we can accept clients
		UpdateNeedWantsStatus();
	}

	/**
	 *  Makes the server travel to the new map
	 *
	 * @param SessionName the name of the session the callback is for
	 * @param bWasSuccessful whether the create worked or not
	 */
	function OnCreatePublicVersusMatchComplete(name SessionName,bool bWasSuccessful)
	{
		if (bHasDeleteRequested)
		{
			`Log("Deleting game session due to deferred delete request");
			bHasDeleteRequested = false;
			// A client caused us to need to start searching while the create was happening, so destroy this and update our beacon
			if (GameInterface.GetGameSettings('Game') != None)
			{
				GameInterface.DestroyOnlineGame('Game');
			}
			// Re-advertise our state so we can accept clients
			UpdateNeedWantsStatus();
		}
		else
		{
			`Log("Completing the create process since no cancellation happened");
			// Let the base code handle this
			Super.OnCreatePublicVersusMatchComplete(SessionName,bWasSuccessful);
		}
	}

	/**
	 * Creates a party beacon for reserving space in someone else's party,
	 * resets all of the session searching vars and starts the reservation
	 * search process
	 */
	function StartPartialPartyMatching()
	{
		// Operate on the party search space
		SessionSearch = PartialPartySearch;
		MaxPing = PartialPartySearch.MaxPing;
		// We are able to start trying to reserve space so start that process
		ResetBestSessionVars();
		ContinuePartialPartyMatching();
	}

	/**
	 * Called to find the next best session to reserve space in
	 */
	function ContinuePartialPartyMatching()
	{
		ChooseBestSession();
		// If we have a valid choice, attempt to connect to it
		if (ReservationsWithNoSpace.Length < SessionSearch.Results.Length &&
			(BestSessionIndex >= 0 && BestSessionIndex < SessionSearch.Results.Length))
		{
			// Try to reserve space in the session
			RequestPartialPartyReservation();
		}
		else
		{
			// If our NAT is ok and we have searched enough (plus random), then host
			if (CanHostPartyMatch() && ShouldSwitchToPartialPartyHost())
			{
				// We need to register ourselves as the gatherer of parties
				SwitchToPartialPartyLeader();
			}
			// Otherwise, we need to continue to fill in matches
			else
			{
				// Delay and then search again for sessions
				if (MatchmakingState == MMS_FindingBestParty)
				{
					SetTimer(1.5,false,nameof(DelayedFindAnyPartialParty));
				}
				else
				{
					SetTimer(3.0,false,nameof(DelayedFindBestPartialParty));
				}
			}
		}
	}

	/**
	 * Cleans up any memory and/or sockets used by the searching process
	 */
	function EndPartialPartyMatching()
	{
		SessionSearch = None;
		// No longer needed, so free the memory
		GameInterface.FreeSearchResults();
		// Destroy the beacon too
		if (ClientPartyReservationBeacon != None)
		{
			ClientPartyReservationBeacon.DestroyBeacon();
			ClientPartyReservationBeacon = None;
		}
	}

	/**
	 * Uses the party beacon to request a reservation for the selected session
	 */
	function RequestPartialPartyReservation()
	{
		local array<PlayerReservation> Party;
		local UniqueNetId PartyLeader;
		local OnlineGameSearchResult SessionToCheck;

		// Create a party beacon for sending reservation requests
		ClientPartyReservationBeacon = new ClientPartyBeaconClass;
		ClientPartyReservationBeacon.BeaconName = 'ClientParty';
		// Prevent vs beacons and party beacons from colliding
		ClientPartyReservationBeacon.PartyBeaconPort++;
		// Determine which players to send to the remote host
		BuildTeamList(Party);
		PartyLeader = PartySettings.OwningPlayerId;
		// Grab the search object so we can pass the session information to the beacon
		SessionToCheck = PartialPartySearch.Results[BestSessionIndex];

		`Log("Attempting to reserve space in session owned by "$SessionToCheck.GameSettings.OwningPlayerName$" at index "$BestSessionIndex);
		// Send out a reservation request
		if (ClientPartyReservationBeacon.RequestReservation(SessionToCheck,PartyLeader,Party))
		{
			// Set the callback so we know when it's done
			ClientPartyReservationBeacon.OnReservationRequestComplete = OnPartialPartyReservationRequestComplete;
			ClientPartyReservationBeacon.OnTravelRequestReceived = OnPartialPartyTravelRequestReceived;
			ClientPartyReservationBeacon.OnReservationCountUpdated = OnPartialPartyReservationCountUpdated;
			ClientPartyReservationBeacon.OnHostHasCancelled = OnPartyHostHasCancelled;
		}
		else
		{
			`Log("Failed to send reservation request for BestSessionIndex("$BestSessionIndex$")");
			// Add this one to the failed list
			ReservationsWithNoSpace.AddItem(BestSessionIndex);
			// Find the next best match
			ContinuePartialPartyMatching();
		}
	}

	/**
	 * Put a message up, close down the beacon, and then transition after a while
	 */
	function OnPartyHostHasCancelled()
	{
		local OnlineGameSearchResult SessionToCheck;

		// Grab the last session we connected to so we can make sure to avoid them later
		SessionToCheck = PartialPartySearch.Results[BestSessionIndex];
		SessionsToIgnore.AddItem(SessionToCheck.GameSettings.OwningPlayerId);

		GearPartyGRI(GameReplicationInfo).PartySlotsFilled = 0;
		ProcessMatchmakingStateChange(MMS_HostCancelingMatchmaking);
	}

	/**
	 * Called by the beacon when the host sends a reservation count update packet so
	 * that any UI can be updated
	 *
	 * @param ReservationRemaining the number of reservations that are still available
	 */
	function OnPartialPartyReservationCountUpdated(int ReservationRemaining)
	{
		GearPartyGRI(GameReplicationInfo).PartySlotsFilled = PlaylistTeamSize - GameReplicationInfo.PRIArray.Length - ReservationRemaining;
	}

	/**
	 * Called by the beacon when a reservation request has been responded to by the destination host
	 *
	 * @param ReservationResult whether there was space allocated for the party or not
	 */
	function OnPartialPartyReservationRequestComplete(EPartyReservationResult ReservationResult)
	{
		// Clear the delegate since it will be reset if needed
		ClientPartyReservationBeacon.OnReservationRequestComplete = None;

		if (ReservationResult == PRR_ReservationAccepted)
		{
			// Change the feedback text to say we are searching for opponents without actually searching
			ProcessMatchmakingStateChange(MMS_WaitingForOpposingParty);
		}
		else
		{
			`Log("Partial party reservation request for index ("$BestSessionIndex$") rejected with "$ReservationResult);
			// Create a party beacon for sending reservation requests
			ClientPartyReservationBeacon.DestroyBeacon();
			// Add this one to the failed list
			ReservationsWithNoSpace.AddItem(BestSessionIndex);
			// Find the next best match
			ContinuePartialPartyMatching();
		}
	}

	/**
	 * Forwards the call to travel to all clients
	 *
	 * @param SessionName the name of the session to register
	 * @param SearchClass the search that should be populated with the session
	 * @param PlatformSpecificInfo the binary data to place in the platform specific areas
	 */
	function OnPartialPartyTravelRequestReceived(name SessionName,class<OnlineGameSearch> SearchClass,byte PlatformSpecificInfo[68])
	{
		local GearPartyPC PC;

		bIsTraveling = true;
		// We are transitioning so block the cancel
		ProcessMatchmakingStateChange(MMS_ConnectingToOpposingParty);

		// Tell any party members connected to us to go to the destination we found
		TellClientsToTravelToSession(SessionName,SearchClass,PlatformSpecificInfo);

		// Since the above doesn't handle local, explicitly have us travel the same way
		foreach LocalPlayerControllers(class'GearPartyPC',PC)
		{
			PC.ClientTravelToSession(SessionName,SearchClass,PlatformSpecificInfo);
			break;
		}

		// Clean up the beacon for later
		ClientPartyReservationBeacon.DestroyBeacon();
		ClientPartyReservationBeacon = None;
	}

	/** Cleans up the beacon and travels to the pre-game lobby */
	function FinishReservationFull()
	{
		local byte PlatformInfo[68];

		// We are transitioning so block the cancel
		ProcessMatchmakingStateChange(MMS_ConnectingToOpposingParty);

		if (GameInterface.ReadPlatformSpecificSessionInfoBySessionName('Game',PlatformInfo))
		{
			// show the loading movie.
			class'GearUIScene_Base'.static.ShowLoadingMovie(true);

			bIsTraveling = true;
			// Now set the travel URL and don't notify clients
			WorldInfo.ServerTravel("gearstart?game=GearGameContent.GearPreGameLobbyGame?listen",true,true);

			// Have everyone travel to this destination
			TellClientsToTravelToSession('Game',class'GearVersusGameSearch',PlatformInfo);
			// Tell our remote partial parties to travel
			HostPartyReservationBeacon.TellClientsToTravel('Game',class'GearVersusGameSearch',PlatformInfo);

			// Add all the players to the recent player list for the pregame
			BuildCurrentPlayerList();
			// Clean up the beacons
			SetTimer(1.0,false,nameof(DestroyBeacons));

			// Kick off the upload of our matchmaking data
			UploadHostMatchmakingStats();
		}
		else
		{
			//@fixme robm/joeg - Show ui and cleanup state
		}
	}

	/**
	 * Once a session has been reserved, this code notifies the clients, pauses, and
	 * joins the session
	 */
	function JoinReservedSession()
	{
		local OnlineGameSearchResult SessionToJoin;
		local byte PlatformInfo[68];

		// Grab the session that was successfully reserved
		SessionToJoin = VsSearch.Results[BestSessionIndex];

		// Get the information needed to travel to this destination
		if (GameInterface.ReadPlatformSpecificSessionInfo(SessionToJoin,PlatformInfo))
		{
			bIsTraveling = true;
			// Tell our remote partial parties to travel
			HostPartyReservationBeacon.TellClientsToTravel('Game',VsSearch.Class,PlatformInfo);
			// Have everyone in our party travel to this destination
			TellClientsToTravelToSession('Game',VsSearch.Class,PlatformInfo);

			GameInterface.AddJoinOnlineGameCompleteDelegate(OnJoinVersusMatchComplete);
			// Now join this Session
			GameInterface.JoinOnlineGame(OwningControllerIndex,'Game',SessionToJoin);

			// Kick off the upload of our matchmaking data
			UploadSearchMatckmakingStats();
		}
		else
		{
			//@todo robm -- Add UI error display here
		}
	}

	/**
	 * After searching N number of times, it uses a random number based upon the
	 * number of people in the party
	 *
	 * @return true if the party should transition to hosting
	 */
	function bool ShouldSwitchToPartialPartyHost()
	{
		local float PercentChance;
		local float DiceRoll;

		// If we've passed the max number of times, then roll the dice for hosting
		if (PartialPartySearchCount >= MaxPartialPartySearchCount)
		{
			PercentChance = float(GameReplicationInfo.PRIArray.Length) / float(PlaylistTeamSize);
			DiceRoll = FRand();
			return DiceRoll < PercentChance;
		}
		return false;
	}

Begin:
	// Reset the number of times we've searched
	PartialPartySearchCount = 0;
	// Start the party matchmaking
	StartParty();
}

/** Does the work for BuildCurrentPlayerList for all custom games (Private, SystemLink, Local) */
function BuildCustomMatchCurrentPlayerList()
{
	local int PRIIndex;
	local UniqueNetId ZeroId;
	local CurrentPlayerMet CurrentPlayer;
	local array<CurrentPlayerMet> Players;
	local OnlineRecentPlayersList PlayersList;
	local int CurrentTeamNum;
	local OnlineGameSettings CurrentSettings;
	local int VersusMode, MatchMode;
	local int MaxTeams;
	local int TotalPlayers;
	local bool bIsLocalMatch;

	CurrentSettings = GetCurrentGameSettings();
	MaxTeams = 1;

	// Figure out the number of teams to use from the game type
	if (CurrentSettings.GetStringSettingValue(class'GearVersusGameSettings'.const.CONTEXT_VERSUSMODES,VersusMode))
	{
		switch (VersusMode)
		{
		case eGEARMP_CombatTrials:
			MaxTeams = 1;
			break;
		case eGEARMP_Wingman:
			// set the number of teams so that we divide up the players as evenly as possible
			CurrentSettings.GetStringSettingValue(class'GearVersusGameSettings'.const.CONTEXT_NUMBOTS, TotalPlayers);
			TotalPlayers += NumPlayers;
			if (TotalPlayers <= 5)
			{
				MaxTeams = 5;
			}
			else
			{
				MaxTeams = TotalPlayers / 2;
				if (TotalPlayers % 2 == 1)
				{
					MaxTeams++;
				}
				MaxTeams = Clamp(MaxTeams, 2, 5);
			}
			break;
		default:
			MaxTeams = 2;
			break;
		}
	}

	// See if this is a local match
	if (CurrentSettings.GetStringSettingValue(class'GearProfileSettings'.const.VERSUS_MATCH_MODE,MatchMode))
	{
		bIsLocalMatch = (MatchMode == eGVMT_Local);
	}

	// Only randomize teams if more than one player is present
	if (GameReplicationInfo.PRIArray.Length > 1)
	{
		// Start with a random team index and then round robin from there
		CurrentTeamNum = FRand() * MaxTeams;
	}
	else
	{
		CurrentTeamNum = 0;
	}

	// Just walk the PRIArray adding players and assigning teams
	for (PRIIndex = 0; PRIIndex < GameReplicationInfo.PRIArray.Length; PRIIndex++)
	{
		if (GameReplicationInfo.PRIArray[PRIIndex].UniqueId != ZeroId || bIsLocalMatch)
		{
			CurrentPlayer.NetId = GameReplicationInfo.PRIArray[PRIIndex].UniqueId;
			CurrentPlayer.Skill = GameReplicationInfo.PRIArray[PRIIndex].PlayerSkill;
			CurrentPlayer.TeamNum = CurrentTeamNum;
			Players.AddItem(CurrentPlayer);
			// Move to the next team to even things out
			CurrentTeamNum = (CurrentTeamNum + 1) % MaxTeams;
		}
	}
	// Now cache this information in the recent player list since that persists
	PlayersList = OnlineRecentPlayersList(OnlineSub.GetNamedInterface('RecentPlayersList'));
	if (PlayersList != None)
	{
		PlayersList.SetCurrentPlayersList(Players);
	}
}

/**
 * Handles all of the private and systemlink match logic for creating, starting games
 */
state PrivateMatch
{
	/**
	 * Called once the skill read for the party has completed. It is then copied to the
	 * search object so that the search can reuse the resulting data
	 *
	 * @param bWasSuccessful whether the stats read was able to complete successfully
	 */
	function OnReadPartySkillComplete(bool bWasSuccessful)
	{
		Super.OnReadPartySkillComplete(bWasSuccessful);

		if (bWasSuccessful)
		{
			// Copy the skill information to the search object
			PartySkill.CopySkillDataToSearch(VsSearch);
		}
		StartPartyGame();
	}

	/**
	 * Build the current players list with all of the local, networked, and remote party information
	 */
	function BuildCurrentPlayerList()
	{
		BuildCustomMatchCurrentPlayerList();
	}

	/**
	 * Party has moved from gathering to started so search for players to fill our match
	 *
	 * @param SessionName the name of the session the callback is for
	 * @param bWasSuccessful whether the create worked or not
	 */
	function OnStartPartyComplete(name SessionName,bool bWasSuccessful)
	{
		Super.OnStartPartyComplete(SessionName,bWasSuccessful);
		// Make sure the playerlist contains everyone in the party
		BuildCurrentPlayerList();
		// Now transition to playing some fun stuff
		CreatePrivateVersusMatch();
	}

Begin:
	// Start the party matchmaking
	StartParty();
}

/**
 * Handles all of the local match logic for creating, starting games
 */
state LocalMatch
{
Begin:
	BuildCustomMatchCurrentPlayerList();
	bUseSeamlessTravel = true;
	ConsoleCommand("start gearstart?game=GearGameContent.GearPreGameLobbyGame?listen?");
}

/**
 * Handles the starting of a system link match
 */
state SystemLink
{
	/**
	 * Kicks off the system link match
	 */
	function StartSystemLinkMatch()
	{
		local PlayerController PC;

		// Set the teams and players involved
		BuildCustomMatchCurrentPlayerList();
		// Tell clients to mark their game as started
		foreach WorldInfo.AllControllers(class'PlayerController',PC)
		{
			// The game will do this post skill read locally, so just do remote clients
			if (!PC.IsLocalPlayerController())
			{
				PC.ClientStartOnlineGame();
			}
		}
		// Start it locally so the beacon stops advertising
		GameInterface.StartOnlineGame('Party');
		bIsTraveling = true;
		// And now travel to the match
		WorldInfo.ServerTravel("gearstart?game=GearGameContent.GearPreGameLobbyGame");
	}

Begin:
	StartSystemLinkMatch();
}


/** Verifies that the party beacon was cleaned up so a socket doesn't leak */
event Destroyed()
{
	if (HostVsReservationBeacon != None)
	{
		HostVsReservationBeacon.DestroyBeacon();
		HostVsReservationBeacon = None;
	}
	if (ClientVsReservationBeacon != None)
	{
		ClientVsReservationBeacon.DestroyBeacon();
		ClientVsReservationBeacon = None;
	}
	if (HostPartyReservationBeacon != None)
	{
		HostPartyReservationBeacon.DestroyBeacon();
		HostPartyReservationBeacon = None;
	}
	if (ClientPartyReservationBeacon != None)
	{
		ClientPartyReservationBeacon.DestroyBeacon();
		ClientPartyReservationBeacon = None;
	}
	Super.Destroyed();
}

/**
 * Calculates updated values for the party session's dynamic values (open slots, etc.) and begins the update task.
 */
function UpdatePartySettings()
{
	GameInterface.AddUpdateOnlineGameCompleteDelegate(OnUpdateComplete);
	if (GameInterface.UpdateOnlineGame('Party',PartySettings,true))
	{
		OpenUpdatingPartyScene();
	}
	else
	{
		GameInterface.ClearUpdateOnlineGameCompleteDelegate(OnUpdateComplete);
	}
}

/**
 * Called when the update of the session state has completed
 *
 * @param SessionName the name of the session this event is for
 * @param bWasSuccessful whether the update completed ok or not
 */
function OnUpdateComplete(name SessionName,bool bWasSuccessful)
{
	GameInterface.ClearUpdateOnlineGameCompleteDelegate(OnUpdateComplete);
	if (SessionName == 'Party')
	{
		CloseUpdatingPartyScene();
	}
}

/** Closes the party session so we can join a SystemLink or Local game */
function bool DestroyParty()
{
	GameInterface.AddDestroyOnlineGameCompleteDelegate(OnDestroyComplete);
	if (GameInterface.DestroyOnlineGame('Party'))
	{
		PartySettings = None;
		OpenUpdatingPartyScene();
	}
	else
	{
		GameInterface.ClearDestroyOnlineGameCompleteDelegate(OnDestroyComplete);
	}
	return true;
}

/**
 * Called when the party recreate finishes publishing with Live
 *
 * @param SessionName the name of the session this event is for
 * @param bWasSuccessful whether the create worked or not
 */
function OnRecreatePartyComplete(name SessionName,bool bWasSuccessful)
{
	if (SessionName == 'Party' && bWasSuccessful)
	{
		PartySettings = GearPartyGameSettings(GameInterface.GetGameSettings('Party'));
		SetPresenceByMode();
		// Update so that it's advertised correctly
		UpdateGameSettingsCounts();
	}

	Super.OnRecreatePartyComplete(SessionName, bWasSuccessful);
}

/**
 * Sets the rich presence string handling which mode we are playing
 */
function SetPresenceByMode()
{
	local int MatchMode;
	local GearProfileSettings Profile;

	// Get the profile for the player so we can look at match mode
	Profile = GearProfileSettings(OnlineSub.PlayerInterface.GetProfileSettings(LobbySceneResource.GetBestControllerId()));
	if (Profile != None)
	{
		Profile.GetProfileSettingValueId(class'GearProfileSettings'.const.VERSUS_MATCH_MODE,MatchMode);
		switch (MatchMode)
		{
			case eGVMT_Official:
				SetClientsRichPresenceStrings(CONTEXT_PRESENCE_PARTYWAITPRESENCE);
				break;

			case eGVMT_SystemLink:
				SetClientsRichPresenceStrings(CONTEXT_PRESENCE_SYSTEMLINKPARTYPRESENCE);
				break;

			default:
				SetClientsRichPresenceStrings(CONTEXT_PRESENCE_PRIVATEPARTYPRESENCE);
				break;
		}
	}
}

/**
 * Delegate fired when a destroying an online game has completed
 *
 * @param SessionName the name of the session this callback is for
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
function OnDestroyComplete(name SessionName,bool bWasSuccessful)
{
	GameInterface.ClearDestroyOnlineGameCompleteDelegate(OnDestroyComplete);
	CloseUpdatingPartyScene();
}

/**
 * Unsubscribes this object from all online delegates it supports.  Called at level transition to make sure no dangling
 * references are left hanging around.
 */
function ClearOnlineDelegates()
{
	Super.ClearOnlineDelegates();

	// on console, we specifically do not check for null GameInterface so that we know if this method can't do its thing
	if ( class'UIRoot'.static.IsConsole()
		||	(OnlineSub != None && GameInterface != None) )
	{
		GameInterface.ClearUpdateOnlineGameCompleteDelegate(OnUpdateComplete);
		GameInterface.ClearStartOnlineGameCompleteDelegate(OnStartPartyComplete);
		GameInterface.ClearFindOnlineGamesCompleteDelegate(OnFindOpponentsComplete);
		GameInterface.ClearJoinOnlineGameCompleteDelegate(OnJoinVersusMatchComplete);
	}
}

/**
 * Converts a character type enum value string into its byte value.
 *
 * @param	TeamIndex			determines which enum is used for converting the string value.
 * @param	EnumValueString		a string representing a value from either the ECogMPCharacter or the ELocustMPCharacter
 *								enums, depending on the specified TeamIndex.
 *
 * @return	the actual byte value of the specified enum value string
 */
static function byte ConvertCharacterValueStringToByte( int TeamIndex, string EnumValueString )
{
	local int i, MaxCharacters;
	local byte Result;

	MaxCharacters = (TeamIndex % 2) == 0 ? CMPC_MAX : LMPC_MAX;
	for ( i = 0; i < MaxCharacters; i++ )
	{
		if ( EnumValueString == string(GetEnum(class'GearUIDataStore_GameResource'.static.GetTeamCharacterProviderIdType(TeamIndex),i)) )
		{
			Result = i;
			break;
		}
	}

	return Result;
}

/**
 * Retrieves the ProfileId value for a GearGameCharacterSummary data provider.
 *
 * @param	CharacterProviderIndex	the index [into the GameResource data store's list of character data providers; the owning player's
 *									current team will determine which of the data store's lists of character providers will be used.
 *
 * @return	the value of the specified character data provider's ProfileId; will be one of the values of either the
 *			ECogMPCharacter or ELocustMPCharacter enums, depending on the player's team.
 */
static function byte GetCharacterIdFromProviderIndex( int TeamIndex, int CharacterProviderIndex )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue CharacterValue;
	local byte Result;

	GameResourceDS = GetGameResourceDataStore();
	if ( GameResourceDS != None )
	{
		if ( GameResourceDS.GetProviderFieldValue(GameResourceDS.GetTeamCharacterProviderTag(TeamIndex), 'ProfileId', CharacterProviderIndex, CharacterValue) )
		{
			Result = ConvertCharacterValueStringToByte(TeamIndex, CharacterValue.StringValue);
		}
		else
		{
			Result = 0;	// ??
		}
	}

	return Result;
}

/* === GearGame interface === */
/**
 * Accessor method for applying a user-generated character class change.  This version takes the index into the
 * actual array of character data providers, and should only be used internally.  For communicating with this class
 * SetPlayerClassFromPlayerSelection should be used, and the value passed should be an ID, not an index.
 */
protected function ApplyPlayerRequestedCharacterSwitch( GearPRI WPRI, int CharacterProviderIndex )
{
	local GearPartyPRI PartyPRI;

	PartyPRI = GearPartyPRI(WPRI);
	if ( PartyPRI != None && CharacterProviderIndex != INDEX_NONE )
	{
		PartyPRI.SelectedCharacterProfileId = GetCharacterIdFromProviderIndex(PartyPRI.Team.TeamIndex, CharacterProviderIndex);
		PartyPRI.bForceNetUpdate = true;
	}
}

/* === GameInfo interface === */

/**
 * Hook for overriding the gametype.
 *
 * @param	MapName		the name of the map that was just loaded
 * @param	Options		the options that were passed on the URL.
 * @param	Portal		name of teleporter to spawn at
 */
static event class<GameInfo> SetGameType(string MapName, string Options, string Portal)
{
	return Super(GameInfo).SetGameType(MapName, Options, Portal);
}

/** Returns the index of the team that should receive the next available player */
function int GetForcedTeam(Controller Other, int Team)
{
	// always team 0 in party lobby
	return 0;
}

/**
 * Everyone in the party lobby can talk with each other, so reset to talking
 *
 * @param PC the playercontroller that is ready for updates
 */
function UpdateGameplayMuteList(PlayerController PC)
{
	local GearPC OtherPC;

	foreach WorldInfo.AllControllers(class'GearPC',OtherPC)
	{
		if (OtherPC != PC)
		{
			// Clear any muting that was left over from a previous match
			PC.GameplayUnmutePlayer(OtherPC.PlayerReplicationInfo.UniqueId);
			OtherPC.GameplayUnmutePlayer(PC.PlayerReplicationInfo.UniqueId);
		}
	}

	Super.UpdateGameplayMuteList(PC);
}

/**
 * Initializes the party system object for manipulation by the lobby
 */
event InitGame(string Options, out string ErrorMessage)
{
	Super.InitGame(Options, ErrorMessage);

	// Load our custom beacon class
	ClientPartyBeaconClass = class<PartyBeaconClient>(DynamicLoadObject(ClientPartyBeaconClassName,class'class'));

	if (GameInterface != None)
	{
		PartySettings = GearPartyGameSettings(GameInterface.GetGameSettings('Party'));
		// Create a single reused search object so that we re-use skill data
		VsSearch = new class'GearVersusGameSearch';
		// Create the party search object we'll use when doing partial party matching
		PartialPartySearch = new class'GearPartyGameSearch';

		// If this is training (came from the training menu) we set the number of bots
		if (PartySettings != None &&
			HasOption(Options, "bIsTraining"))
		{
			PartySettings.SetStringSettingValue(PartySettings.const.CONTEXT_NUMBOTS, 9);
		}
	}

	// If we are returning (not pregame lobby), reenable invites
	if (GearPreGameLobbyGame_Base(self) == None && PartySettings.GameState == OGS_InProgress)
	{
		CleanupPartyMatchmaking();
		// Destroy the game if it is still around
		if (GameInterface.GetGameSettings('Game') != None)
		{
			GameInterface.DestroyOnlineGame('Game');
		}
	}
}

event PreBeginPlay()
{
	Super.PreBeginPlay();

	// Override the arbitration flag based off of the party settings
	if ( PartySettings != None )
	{
		bUsingArbitration = PartySettings.bUsesArbitration;
		MaxPlayers = PartySettings.NumPublicConnections;
	}
}

/**
 * Checks to see if matchmaking is in progress or not and blocks joining
 *
 * @param Options URL options for the match
 * @param Address the IP address of the client connecting
 * @param ErrorMessage an out string that receives the error message
 */
event PreLogin(string Options,string Address,out string ErrorMessage)
{
	Super.PreLogin(Options,Address,ErrorMessage);

	if (bIsMatchmaking)
	{
		ErrorMessage = "GearGameUI.MessageBoxErrorStrings.PartyIsMatchmaking";
		return;
	}

	if (GameReplicationInfo != None && len(ErrorMessage) == 0)
	{
		// Increment the number of slots that should show "Connecting..."
		GearPartyGRI(GameReplicationInfo).ConnectingPlayerCount++;
	}
}

/**
 * Called after a successful login. This is the first place it is safe to call replicated functions on the PlayerController.
 *
 * @param	NewPlayer	the PlayerController that just successfully connected and logged in.
 */
event PostLogin( PlayerController NewPlayer )
{
	Super.PostLogin(NewPlayer);

	HandlePostLogin(NewPlayer);

	// Don't update counts for local players
	if (!NewPlayer.IsLocalPlayerController())
	{
		// Decrement the number of slots that should show "Connecting..."
		GearPartyGRI(GameReplicationInfo).ConnectingPlayerCount--;
	}
}

/**
 * Called after player completes seamless travel
 *
 * @param C the controller that just completed seamless travel
 */
event HandleSeamlessTravelPlayer(out Controller C)
{
	local PlayerController PC;

	if (C.IsA('GearAI'))
	{
		C.Destroy();
	}
	else
	{
		Super.HandleSeamlessTravelPlayer(C);

		PC = PlayerController(C);
		if (PC != None)
		{
			HandlePostLogin(PC);
		}
	}
}

/**
 * Performs the party game post login code. Opens UI, etc.
 *
 * @param NewPlayer the player that just joined the game
 */
function HandlePostLogin(PlayerController NewPlayer)
{
	local GearUIDataStore_GameSettings SettingsDS;
	local GearPartyPC PartyPC;
	local OnlineGameSettings CurrentSettings;
	local int GametypeContextId;

	if ( !NewPlayer.IsLocalPlayerController() && NewPlayer.IsPrimaryPlayer() )
	{
		PartyPC = GearPartyPC(NewPlayer);
		if ( PartyPC != None )
		{
			// tell the player to open the lobby scene, if they are ready to.
			PartyPC.OpenLobbyScene(LobbySceneResource);

			SettingsDS = GearUIDataStore_GameSettings(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameSettings'.default.Tag, None, GetPlayerOwner(0)));
			if ( SettingsDS != None )
			{
				CurrentSettings = SettingsDS.GetCurrentGameSettings();
				if ( CurrentSettings != None )
				{
					// initialize this pri's game settings objects
					CurrentSettings.GetStringSettingValue(class'GearVersusGameSettings'.const.CONTEXT_VERSUSMODES, GametypeContextId);
					PartyPC.ClientUpdateGameSettingValue(class'GearVersusGameSettings'.const.CONTEXT_VERSUSMODES, GametypeContextId);
				}
			}

			// Set the invite settings to match ours
			PartyPC.ClientSetPartyInviteFlags(PartySettings.bAllowInvites,PartySettings.bAllowJoinViaPresence,PartySettings.bAllowJoinViaPresenceFriendsOnly);
		}
	}
	// Refresh presence for all players
	SetPresenceByMode();
}

/**
 * Handles a player leaving the session. Cancels matchmaking when a player leaves if it
 * was in progress, since the data is no longer accurate
 *
 * @param Exiting the player leaving the session
 */
function Logout(Controller Exiting)
{
	local GearPartyPC PC;

	// Don't cancel if they logged out because we told them to travel
	if (bIsMatchmaking && !bIsTraveling && !Exiting.IsLocalPlayerController())
	{
		PC = GearPartyPC(Exiting);
		if (PC != None)
		{
			PC.ServerCancelMatchmaking();
		}
	}

	Super.Logout(Exiting);
}

/** Called when a connection closes before getting to PostLogin() */
event NotifyPendingConnectionLost()
{
	local GearPartyGRI GRI;

	GRI = GearPartyGRI(GameReplicationInfo);
	if (GRI != None && GRI.ConnectingPlayerCount > 0)
	{
		GRI.ConnectingPlayerCount--;
	}
}

defaultproperties
{
	OwningControllerIndex=INDEX_NONE

	// Specify the party specific PC for voice and recent player's list to work
	PlayerControllerClass=class'GearGame.GearPartyPC'

	// Use the party game PRI so that voice and recent players works in the party
	PlayerReplicationInfoClass=class'GearGame.GearPartyPRI'

	// Replicates matchmaking states to clients
	GameReplicationInfoClass=class'GearGame.GearPartyGRI'
}

