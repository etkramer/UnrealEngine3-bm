/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This class is used to create a network accessible beacon for responding
 * to reservation requests for party matches. It handles all tracking of
 * who has reserved space, how much space is available, and how many parties
 * can reserve space.
 */
class PartyBeaconHost extends PartyBeacon
	native;

/** Holds the information for a client and whether they've timed out */
struct native ClientBeaconConnection
{
	/** The unique id of the party leader for this connection */
	var UniqueNetId PartyLeader;
	/** How long it's been since the last heartbeat */
	var float ElapsedHeartbeatTime;
	/** The socket this client is communicating on */
	var native transient pointer Socket{FSocket};
};

/** The object that is used to send/receive data with the remote host/client */
var array<ClientBeaconConnection> Clients;

/** The number of teams that these players will be divided amongst */
var const int NumTeams;

/** The number of players required for a full team */
var const int NumPlayersPerTeam;

/** The number of players that are allowed to reserve space */
var const int NumReservations;

/** The number of slots that have been consumed by parties in total (saves having to iterate and sum) */
var const int NumConsumedReservations;

/** The list of accepted reservations */
var array<PartyReservation> Reservations;

/** The online session name that players will register with */
var name OnlineSessionName;

/** The number of connections to allow before refusing them */
var config int ConnectionBacklog;

/** Used to keep a list of available teams that are randomly assigned out to parties */
var const array<int> PotentialTeamChoices;

/** The team the host (owner of the beacon) is assigned to when random teams are chosen */
var const int ReservedHostTeamNum;

cpptext
{
	/**
	 * Ticks the network layer to see if there are any requests or responses to requests
	 *
	 * @param DeltaTime the amount of time that has elapsed since the last tick
	 */
	virtual void Tick(FLOAT DeltaTime);

	/** Accepts any pending connections and adds them to our queue */
	void AcceptConnections(void);

	/**
	 * Reads the socket and processes any data from it
	 *
	 * @param ClientConn the client connection that sent the packet
	 *
	 * @return TRUE if the socket is ok, FALSE if it is in error
	 */
	UBOOL ReadClientData(FClientBeaconConnection& ClientConn);

	/**
	 * Processes a packet that was received from a client
	 *
	 * @param Packet the packet that the client sent
	 * @param PacketSize the size of the packet to process
	 * @param ClientConn the client connection that sent the packet
	 */
	void ProcessRequest(BYTE* Packet,INT PacketSize,FClientBeaconConnection& ClientConn);

	/**
	 * Processes a reservation packet that was received from a client
	 *
	 * @param FromBuffer the packet serializer to read from
	 * @param ClientConn the client connection that sent the packet
	 */
	void ProcessReservationRequest(FNboSerializeFromBuffer& FromBuffer,FClientBeaconConnection& ClientConn);

	/**
	 * Processes a cancellation packet that was received from a client
	 *
	 * @param FromBuffer the packet serializer to read from
	 * @param ClientConn the client connection that sent the packet
	 */
	void ProcessCancellationRequest(FNboSerializeFromBuffer& FromBuffer,FClientBeaconConnection& ClientConn);

	/**
	 * Sends a client the specified response code
	 *
	 * @param Result the result being sent to the client
	 * @param ClientSocket the client socket to send the response on
	 */
	void SendReservationResponse(EPartyReservationResult Result,FSocket* ClientSocket);

	/**
	 * Tells clients that a reservation update has occured and sends them the current
	 * number of remaining reservations so they can update their UI
	 */
	void SendReservationUpdates(void);

	/**
	 * Sends a client acknowledgement that it got the message
	 *
	 * @param ClientSocket the client socket to send the response on
	 */
	void SendCancellationResponse(FSocket* ClientSocket);

	/**
	 * Initializes the team array so that random choices can be made from it
	 * Also initializes the host's team number (random from range)
	 */
	void InitTeamArray(void);

	/** @return TRUE if there are teams available, FALSE otherwise */
	inline UBOOL AreTeamsAvailable(void)
	{
		return NumTeams == 1 || PotentialTeamChoices.Num();
	}

	/**
	 * Removes the specified party leader (and party) from the arrays and notifies
	 * any connected clients of the change in status
	 *
	 * @param PartyLeader the leader of the party to remove
	 * @param ClientConn the client connection that sent the packet
	 */
	void CancelPartyReservation(FUniqueNetId& PartyLeader,FClientBeaconConnection& ClientConn);
}

/**
 * Creates a listening host beacon with the specified number of parties, players, and
 * the session name that remote parties will be registered under
 *
 * @param InNumTeams the number of teams that are expected to join
 * @param InNumPlayersPerTeam the number of players that are allowed to be on each team
 * @param InNumReservations the total number of players to allow to join (if different than team * players)
 * @param InSessionName the name of the session to add the players to when a reservation occurs
 *
 * @return true if the beacon was created successfully, false otherwise
 */
native function bool InitHostBeacon(int InNumTeams,int InNumPlayersPerTeam,int InNumReservations,name InSessionName);

/**
 * Called by the beacon when a reservation occurs or is cancelled so that UI can be updated, etc.
 */
delegate OnReservationChange();

/**
 * Called by the beacon when all of the available reservations have been filled
 */
delegate OnReservationsFull();

/**
 * Called by the beacon when a client cancels a reservation
 *
 * @param PartyLeader the party leader that is cancelling the reservation
 */
delegate OnClientCancellationReceived(UniqueNetId PartyLeader);

/**
 * Stops listening for clients and releases any allocated memory
 */
native event DestroyBeacon();

/**
 * Tells all of the clients to go to a specific session (contained in platform
 * specific info). Used to route clients that aren't in the same party to one destination.
 *
 * @param SessionName the name of the session to register
 * @param SearchClass the search that should be populated with the session
 * @param PlatformSpecificInfo the binary data to place in the platform specific areas
 */
native function TellClientsToTravel(name SessionName,class<OnlineGameSearch> SearchClass,byte PlatformSpecificInfo[68]);

/**
 * Tells all of the clients that the host is ready for them to travel to the host connection
 */
native function TellClientsHostIsReady();

/**
 * Tells all of the clients that the host has cancelled the matchmaking beacon and that they
 * need to find a different host
 */
native function TellClientsHostHasCancelled();

/**
 * Registers all of the parties as part of the session that this beacon is associated with
 */
event RegisterPartyMembers()
{
	local int Index;
	local int PartyIndex;
	local OnlineSubsystem OnlineSub;
	local OnlineRecentPlayersList PlayersList;
	local array<UniqueNetId> Members;
	local PlayerReservation PlayerRes;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None &&
		OnlineSub.GameInterface != None)
	{
		// Iterate through the parties adding the players from each to the session
		for (PartyIndex = 0; PartyIndex < Reservations.Length; PartyIndex++)
		{
			for (Index = 0; Index < Reservations[PartyIndex].PartyMembers.Length; Index++)
			{
				PlayerRes = Reservations[PartyIndex].PartyMembers[Index];
				OnlineSub.GameInterface.RegisterPlayer(OnlineSessionName,PlayerRes.NetId,false);
				Members.AddItem(PlayerRes.NetId);
			}
			// Add the remote party members to the recent players list if available
			PlayersList = OnlineRecentPlayersList(OnlineSub.GetNamedInterface('RecentPlayersList'));
			if (PlayersList != None)
			{
				PlayersList.AddPartyToRecentParties(Reservations[PartyIndex].PartyLeader,Members);
			}
		}
	}
}

/**
 * Unregisters each of the party members at the specified reservation with the session
 */
event UnregisterPartyMembers()
{
	local int Index;
	local int PartyIndex;
	local OnlineSubsystem OnlineSub;
	local PlayerReservation PlayerRes;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None &&
		OnlineSub.GameInterface != None)
	{
		// Iterate through the parties removing the players from each from the session
		for (PartyIndex = 0; PartyIndex < Reservations.Length; PartyIndex++)
		{
			for (Index = 0; Index < Reservations[PartyIndex].PartyMembers.Length; Index++)
			{
				PlayerRes = Reservations[PartyIndex].PartyMembers[Index];
				OnlineSub.GameInterface.UnregisterPlayer(OnlineSessionName,PlayerRes.NetId);
			}
		}
	}
}

/**
 * Unregisters each of the party members that have the specified party leader
 *
 * @param PartyLeader the leader to search for in the reservation array
 */
event UnregisterParty(UniqueNetId PartyLeader)
{
	local int PlayerIndex;
	local int PartyIndex;
	local OnlineSubsystem OnlineSub;
	local PlayerReservation PlayerRes;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None &&
		OnlineSub.GameInterface != None)
	{
		// Iterate through the parties removing the players from each from the session
		for (PartyIndex = 0; PartyIndex < Reservations.Length; PartyIndex++)
		{
			// If this is the reservation in question, remove the members from the session
			if (Reservations[PartyIndex].PartyLeader == PartyLeader)
			{
				for (PlayerIndex = 0; PlayerIndex < Reservations[PartyIndex].PartyMembers.Length; PlayerIndex++)
				{
					PlayerRes = Reservations[PartyIndex].PartyMembers[PlayerIndex];
					OnlineSub.GameInterface.UnregisterPlayer(OnlineSessionName,PlayerRes.NetId);
				}
			}
		}
	}
}

/**
 * Appends the skills from all reservations to the search object so that they can
 * be included in the search information
 *
 * @param Search the search object to update
 */
native function AppendReservationSkillsToSearch(OnlineGameSearch Search);

/**
 * Gathers all the unique ids for players that have reservations
 */
function GetPlayers(out array<UniqueNetId> Players)
{
	local int PlayerIndex;
	local int PartyIndex;
	local PlayerReservation PlayerRes;

	// Iterate through the parties adding the players from each to the out array
	for (PartyIndex = 0; PartyIndex < Reservations.Length; PartyIndex++)
	{
		for (PlayerIndex = 0; PlayerIndex < Reservations[PartyIndex].PartyMembers.Length; PlayerIndex++)
		{
			PlayerRes = Reservations[PartyIndex].PartyMembers[PlayerIndex];
			Players.AddItem(PlayerRes.NetId);
		}
	}
}

/**
 * @return Uses the array that was created during initialization to determine the new team number
 */
native function int GetTeamAssignment();

`if(`notdefined(FINAL_RELEASE))
/**
 * Logs the reservation information for this beacon
 */
function DumpReservations()
{
	local int PartyIndex;
	local int MemberIndex;
	local UniqueNetId NetId;
	local PlayerReservation PlayerRes;

	`Log("Session that reservations are for: "$OnlineSessionName);
	`Log("Number of teams: "$NumTeams);
	`Log("Number players per team: "$NumPlayersPerTeam);
	`Log("Number reservations available: "$NumReservations);
	`Log("Number of party reservations: "$Reservations.Length);
	`Log("Reserved host team: "$ReservedHostTeamNum);
	// Log each party that has a reservation
	for (PartyIndex = 0; PartyIndex < Reservations.Length; PartyIndex++)
	{
		NetId = Reservations[PartyIndex].PartyLeader;
		`Log("  Party leader: "$class'OnlineSubsystem'.static.UniqueNetIdToString(NetId));
		`Log("  Party team: "$Reservations[PartyIndex].TeamNum);
		`Log("  Party size: "$Reservations[PartyIndex].PartyMembers.Length);
		// Log each member of the party
		for (MemberIndex = 0; MemberIndex < Reservations[PartyIndex].PartyMembers.Length; MemberIndex++)
		{
			PlayerRes = Reservations[PartyIndex].PartyMembers[MemberIndex];
			`Log("  Party member: "$class'OnlineSubsystem'.static.UniqueNetIdToString(PlayerRes.NetId));
		}
	}
}
`endif
