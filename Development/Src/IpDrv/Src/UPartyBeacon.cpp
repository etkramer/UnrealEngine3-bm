/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "UnIpDrv.h"

IMPLEMENT_CLASS(UPartyBeacon);
IMPLEMENT_CLASS(UPartyBeaconHost);
IMPLEMENT_CLASS(UPartyBeaconClient);

#define WANTS_BEACON_DEBUGGING 1

#if WANTS_BEACON_DEBUGGING
	#define debugfBeacon debugf
#else
	#define debugfBeacon debugfSuppressed
#endif

/**
 * Ticks the network layer to see if there are any requests or responses to requests
 *
 * @param DeltaTime the amount of time that has elapsed since the last tick
 */
void UPartyBeacon::Tick(FLOAT DeltaTime)
{
#if WITH_UE3_NETWORKING
	// Only tick if we have a socket present
	if (Socket)
	{
		// See if we need to clean up
		if (bWantsDeferredDestroy)
		{
			eventDestroyBeacon();
		}
	}
#endif
}

/**
 * Stops listening for requests/responses and releases any allocated memory
 */
void UPartyBeacon::DestroyBeacon(void)
{
#if WITH_UE3_NETWORKING
	if (Socket)
	{
		// Don't delete if this is during a tick or we'll crash
		if (bIsInTick == FALSE)
		{
			GSocketSubsystem->DestroySocket(Socket);
			Socket = NULL;
			// Clear the deletion flag so we don't incorrectly delete
			bWantsDeferredDestroy = FALSE;
			debugf(NAME_DevOnline,TEXT("Beacon (%s) destroy complete"),*BeaconName.ToString());
		}
		else
		{
			bWantsDeferredDestroy = TRUE;
			debugf(NAME_DevOnline,TEXT("Deferring beacon (%s) destroy until end of tick"),*BeaconName.ToString());
		}
	}
#endif
}


/**
 * Sends a heartbeat packet to the specified socket
 *
 * @param Socket the socket to send the data on
 *
 * @return TRUE if it sent ok, FALSE if there was an error
 */
UBOOL UPartyBeacon::SendHeartbeat(FSocket* Socket)
{
	if (Socket)
	{
		BYTE Heartbeat = RPT_Heartbeat;
		INT BytesSent;
		// Send the message indicating the party that is cancelling
		UBOOL bDidSendOk = Socket->Send(&Heartbeat,1,BytesSent);
		if (bDidSendOk == FALSE)
		{
			debugf(NAME_Error,
				TEXT("Beacon (%s) failed to send heartbeat packet with (%s)"),
				*BeaconName.ToString(),
				GSocketSubsystem->GetSocketError());
		}
		return bDidSendOk;
	}
	return FALSE;
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
UBOOL UPartyBeaconHost::InitHostBeacon(INT InNumTeams,INT InNumPlayersPerTeam,INT InNumReservations,FName InSessionName)
{
#if WITH_UE3_NETWORKING
	// Make sure we allow at least one client to be queued
	ConnectionBacklog = Max(1,ConnectionBacklog);
	FInternetIpAddr ListenAddr;
	// Now the listen address
	ListenAddr.SetPort(PartyBeaconPort);
	ListenAddr.SetIp(getlocalbindaddr(*GWarn));
	// Now create and set up our TCP socket
	Socket = GSocketSubsystem->CreateStreamSocket();
	if (Socket != NULL)
	{
		// Set basic state
		Socket->SetReuseAddr();
		Socket->SetNonBlocking();
		// Bind to our listen port so we can respond to client connections
		if (Socket->Bind(ListenAddr))
		{
			// Start listening for client connections
			if (Socket->Listen(ConnectionBacklog))
			{
				// It worked so copy the settings and set our party beacon type
				NumTeams = InNumTeams;
				NumPlayersPerTeam = InNumPlayersPerTeam;
				NumReservations = InNumReservations;
				NumConsumedReservations = 0;
				OnlineSessionName = InSessionName;
				// Initialize the random teams
				InitTeamArray();
				debugf(NAME_DevOnline,
					TEXT("Created party beacon (%s) on port (%d) for session (%s)"),
					*BeaconName.ToString(),
					PartyBeaconPort,
					*OnlineSessionName.ToString());
				return TRUE;
			}
			else
			{
				debugf(NAME_DevOnline,
					TEXT("Beacon (%s) failed to Listen(%d) on the socket for clients"),
					*BeaconName.ToString(),
					ConnectionBacklog);
			}
		}
		else
		{
			debugf(NAME_DevOnline,
				TEXT("Beacon (%s) failed to bind listen socket to addr (%s) for party beacon"),
				*BeaconName.ToString(),
				*ListenAddr.ToString(TRUE));
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Failed to create listen socket for lan beacon (%s)"),
			*BeaconName.ToString());
	}
#endif
	return FALSE;
}

/**
 * Initializes the team array so that random choices can be made from it
 * Also initializes the host's team number (random from range)
 */
void UPartyBeaconHost::InitTeamArray(void)
{
	if (NumTeams > 1)
	{
		PotentialTeamChoices.Empty(NumTeams);
		// Fill the array in ascending order, which we will pick from at random
		for (INT Count = 0; Count < NumTeams; Count++)
		{
			PotentialTeamChoices.AddItem(Count);
		}
		// Grab one for the host and remove it from the available choices
		INT HostIndex = appTrunc(appFrand() * NumTeams);
		ReservedHostTeamNum = PotentialTeamChoices(HostIndex);
		PotentialTeamChoices.Remove(HostIndex);
	}
	else
	{
		// Only one team, so choose team 0 for everything
		NumTeams = 1;
		ReservedHostTeamNum = 0;
	}
	debugf(NAME_DevOnline,
		TEXT("Beacon (%s) team count (%d), host team (%d)"),
		*BeaconName.ToString(),
		NumTeams,
		ReservedHostTeamNum);
}

/**
 * @return Uses the array that was created during initialization to determine the new team number
 */
INT UPartyBeaconHost::GetTeamAssignment(void)
{
	if (NumTeams > 1)
	{
		// Grab one from our list of choices and then remove it from further use
		INT TeamIndex = appTrunc(appFrand() * PotentialTeamChoices.Num());
		INT NewTeam = PotentialTeamChoices(TeamIndex);
		PotentialTeamChoices.Remove(TeamIndex);
		return NewTeam;
	}
	return 0;
}

/** Accepts any pending connections and adds them to our queue */
void UPartyBeaconHost::AcceptConnections(void)
{
	FSocket* ClientSocket = NULL;
	do
	{
		// See what clients have connected and add them for processing
		ClientSocket = Socket->Accept();
		if (ClientSocket)
		{
			// Add to the list for reading
			INT AddIndex = Clients.AddZeroed();
			FClientBeaconConnection& ClientConn = Clients(AddIndex);
			ClientConn.Socket = ClientSocket;
			debugf(NAME_DevOnline,
				TEXT("Beacon (%s) new client connection from (%s)"),
				*BeaconName.ToString(),
				*ClientSocket->GetAddress().ToString(TRUE));
		}
		else
		{
			INT SocketError = GSocketSubsystem->GetLastErrorCode();
			if (SocketError != SE_EWOULDBLOCK)
			{
				debugf(NAME_DevOnline,
					TEXT("Beacon (%s) failed to accept a connection due to: %s"),
					*BeaconName.ToString(),
					GSocketSubsystem->GetSocketError());
			}
		}
	}
	while (ClientSocket);
}

/**
 * Reads the socket and processes any data from it
 *
 * @param ClientConn the client connection that sent the packet
 *
 * @return TRUE if the socket is ok, FALSE if it is in error
 */
UBOOL UPartyBeaconHost::ReadClientData(FClientBeaconConnection& ClientConn)
{
	UBOOL bShouldRead = TRUE;
	BYTE PacketData[512];
	// Read each pending packet and pass it out for processing
	while (bShouldRead)
	{
		INT BytesRead;
		// Read from the socket and handle errors if present
		if (ClientConn.Socket->Recv(PacketData,512,BytesRead))
		{
			if (BytesRead > 0)
			{
				ClientConn.ElapsedHeartbeatTime = 0.f;
				// Process the packet
				ProcessRequest(PacketData,BytesRead,ClientConn);
			}
			else
			{
				bShouldRead = FALSE;
			}
		}
		else
		{
			// Check for an error other than would block, which isn't really an error
			INT ErrorCode = GSocketSubsystem->GetLastErrorCode();
			if (ErrorCode != SE_EWOULDBLOCK)
			{
				// Cancel this reservation, since they are gone
				CancelPartyReservation(ClientConn.PartyLeader,ClientConn);
				// Log and remove this connection
				debugf(NAME_DevOnline,
					TEXT("Beacon (%s) closing socket to (%s) with error (%s)"),
					*BeaconName.ToString(),
					*ClientConn.Socket->GetAddress().ToString(TRUE),
					GSocketSubsystem->GetSocketError(ErrorCode));
				return FALSE;
			}
			bShouldRead = FALSE;
		}
	}
	return TRUE;
}

/**
 * Ticks the network layer to see if there are any requests or responses to requests
 *
 * @param DeltaTime the amount of time that has elapsed since the last tick
 */
void UPartyBeaconHost::Tick(FLOAT DeltaTime)
{
#if WITH_UE3_NETWORKING
	// Only tick if we have a socket present
	if (Socket && bShouldTick && bWantsDeferredDestroy == FALSE)
	{
		// Use this to guard against deleting the socket while in use
		bIsInTick = TRUE;
		// Add any new connections since the last tick
		AcceptConnections();
		// Skip ticking if no clients are connected
		if (Clients.Num())
		{
			// Determine if it's time to send a heartbeat
			ElapsedHeartbeatTime += DeltaTime;
			UBOOL bNeedsHeartbeat = (ElapsedHeartbeatTime > (HeartbeatTimeout * 0.5f));
			// Check each client in the list for a packet
			for (INT Index = 0; Index < Clients.Num(); Index++)
			{
				UBOOL bHadError = FALSE;
				// Grab the connection we are checking for data on
				FClientBeaconConnection& ClientConn = Clients(Index);
				// Mark how long since we've gotten something from the client
				ClientConn.ElapsedHeartbeatTime += DeltaTime;
				// Read the client data processing any packets
				if (ReadClientData(ClientConn))
				{
					// Send this client the heartbeat request
					if (bNeedsHeartbeat)
					{
						SendHeartbeat(ClientConn.Socket);
						ElapsedHeartbeatTime = 0.f;
					}
					// Check for a client going beyond our heartbeat timeout
					if (ClientConn.ElapsedHeartbeatTime > HeartbeatTimeout)
					{
						debugf(NAME_DevOnline,
							TEXT("Client timed out. Leader 0x%016I64X"),
							(QWORD&)ClientConn.PartyLeader);
						bHadError = bShouldTick && bWantsDeferredDestroy == FALSE;
					}
				}
				else
				{
					debugf(NAME_DevOnline,
						TEXT("Reading from client failed. Leader 0x%016I64X"),
						(QWORD&)ClientConn.PartyLeader);
					bHadError = bShouldTick && bWantsDeferredDestroy == FALSE;
				}
				if (bHadError)
				{
					// Cancel this client
					CancelPartyReservation(ClientConn.PartyLeader,ClientConn);
					// Now clean up that socket
					GSocketSubsystem->DestroySocket(ClientConn.Socket);
					Clients.Remove(Index);
					Index--;
				}
			}
		}
		// No longer a danger, so release the sentinel on the destruction
		bIsInTick = FALSE;
	}
	Super::Tick(DeltaTime);
#endif
}

/**
 * Processes a packet that was received from a potential client when in host mode
 *
 * @param Packet the packet that the client sent
 * @param PacketSize the size of the packet to process
 * @param ClientConn the client connection that sent the packet
 */
void UPartyBeaconHost::ProcessRequest(BYTE* Packet,INT PacketSize,FClientBeaconConnection& ClientConn)
{
	// Use our packet serializer to read from the raw buffer
	FNboSerializeFromBuffer FromBuffer(Packet,PacketSize);
	BYTE PacketType = RPT_UnknownPacketType;
	FromBuffer >> PacketType;
	// Route the call to the proper handler
	switch (PacketType)
	{
		case RPT_ClientReservationRequest:
		{
			ProcessReservationRequest(FromBuffer,ClientConn);
			break;
		}
		case RPT_ClientCancellationRequest:
		{
			ProcessCancellationRequest(FromBuffer,ClientConn);
			break;
		}
		case RPT_Heartbeat:
		{
			break;
		}
		default:
		{
			debugf(NAME_DevOnline,
				TEXT("Beacon (%s) unknown packet type received from client (%d)"),
				*BeaconName.ToString(),
				(DWORD)PacketType);
			break;
		}
	}
}

/**
 * Processes a reservation packet that was received from a client
 *
 * @param FromBuffer the packet serializer to read from
 * @param ClientConn the client connection that sent the packet
 */
void UPartyBeaconHost::ProcessReservationRequest(FNboSerializeFromBuffer& FromBuffer,FClientBeaconConnection& ClientConn)
{
	debugf(NAME_DevOnline,
		TEXT("Beacon (%s) received reservation request from (%s)"),
		*BeaconName.ToString(),
		*ClientConn.Socket->GetAddress().ToString(TRUE));
	// See if we have space or not
	if (NumConsumedReservations < NumReservations)
	{
		FUniqueNetId PartyLeader;
		// Serialize the leader from the buffer
		FromBuffer >> PartyLeader;
		INT PartySize = 0;
		// Now read the number of party members that were sent over
		FromBuffer >> PartySize;
		// Validate that the party size matches our max team size expectations
		if (PartySize <= NumPlayersPerTeam &&
			// And that there is enough space to hold this group
			(PartySize + NumConsumedReservations) <= NumReservations &&
			// And that there are teams left for them to reserve
			AreTeamsAvailable())
		{
			// The size is fine, so create a reservation entry
			INT AddIndex = Reservations.AddZeroed();
			FPartyReservation& Reservation = Reservations(AddIndex);
			// Put this party on a team randomly chosen from the set available
			Reservation.TeamNum = GetTeamAssignment();
			// Copy over the information
			Reservation.PartyLeader = PartyLeader;
			debugf(NAME_DevOnline,
				TEXT("Beacon (%s) added reservation: Leader 0x%016I64X, Team (%d)"),
				*BeaconName.ToString(),
				(QWORD&)PartyLeader,
				Reservation.TeamNum);
			// Create all of the members
			Reservation.PartyMembers.AddZeroed(PartySize);
			// Now serialize each member
			for (INT Count = 0; Count < PartySize; Count++)
			{
				FPlayerReservation& PlayerRes = Reservation.PartyMembers(Count);
				FromBuffer >> PlayerRes.NetId
					>> PlayerRes.Skill
					>> PlayerRes.Mu
					>> PlayerRes.Sigma;
				debugf(NAME_DevOnline,
					TEXT("Player 0x%016I64X, Skill (%d), Mu (%f), Sigma (%f)"),
					(QWORD&)PlayerRes.NetId,
					PlayerRes.Skill,
					PlayerRes.Mu,
					PlayerRes.Sigma);
			}
			// Update the reservation count before sending the response
			NumConsumedReservations += PartySize;
			// Copy the party leader to this connection so that timeouts can remove the party
			ClientConn.PartyLeader = PartyLeader;
			// Send a happy response
			SendReservationResponse(PRR_ReservationAccepted,ClientConn.Socket);
			// Tell any UI and/or clients that there has been a change in the reservation state
			SendReservationUpdates();
			// Tell the owner that we've received a reservation so the UI can be updated
			delegateOnReservationChange();
			// If we've hit our limit, fire the delegate so the host can do the
			// next step in getting parties together
			if (NumConsumedReservations == NumReservations)
			{
				delegateOnReservationsFull();
			}
		}
		else
		{
			// Send an invalid party size response
			SendReservationResponse(PRR_IncorrectPlayerCount,ClientConn.Socket);
		}
	}
	else
	{
		// Send a session full response
		SendReservationResponse(PRR_PartyLimitReached,ClientConn.Socket);
	}
}

/**
 * Processes a cancellation packet that was received from a client
 *
 * @param FromBuffer the packet serializer to read from
 * @param ClientConn the client connection that sent the packet
 */
void UPartyBeaconHost::ProcessCancellationRequest(FNboSerializeFromBuffer& FromBuffer,FClientBeaconConnection& ClientConn)
{
	debugf(NAME_DevOnline,
		TEXT("Beacon (%s) received cancellation request from (%s)"),
		*BeaconName.ToString(),
		*ClientConn.Socket->GetAddress().ToString(TRUE));
	FUniqueNetId PartyLeader;
	// Serialize the leader from the buffer
	FromBuffer >> PartyLeader;
	// Remove them from the various arrays
	CancelPartyReservation(PartyLeader,ClientConn);
}

/**
 * Removes the specified party leader (and party) from the arrays and notifies
 * any connected clients of the change in status
 *
 * @param PartyLeader the leader of the party to remove
 * @param ClientConn the client connection that sent the packet
 */
void UPartyBeaconHost::CancelPartyReservation(FUniqueNetId& PartyLeader,FClientBeaconConnection& ClientConn)
{
	INT PartyIndex;
	INT PartySize = 0;
	INT TeamNum = -1;
	// Find the party so they can be removed
	for (PartyIndex = 0; PartyIndex < Reservations.Num(); PartyIndex++)
	{
		FPartyReservation& Reservation = Reservations(PartyIndex);
		if (Reservation.PartyLeader == PartyLeader)
		{
			TeamNum = Reservation.TeamNum;
			PartySize = Reservation.PartyMembers.Num();
			break;
		}
	}
	// Whether we found the leader or not
	if (Reservations.IsValidIndex(PartyIndex))
	{
		// If there are multiple teams supported, then don't leak the team assignment
		if (NumTeams > 1)
		{
			// Add it back to the available list, so a new team gets it
			PotentialTeamChoices.AddItem(TeamNum);
		}
		// Notify the code that it has lost players
		delegateOnClientCancellationReceived(PartyLeader);
		// Remove the party members from the session
		eventUnregisterParty(PartyLeader);
		// Send an ack of the packet
		SendCancellationResponse(ClientConn.Socket);
		NumConsumedReservations -= PartySize;
		// Tell any UI and/or clients that there has been a change in the reservation state
		SendReservationUpdates();
		// Tell the owner that we've received a reservation so the UI can be updated
		delegateOnReservationChange();
		// Zero the party leader, so his socket timeout doesn't cause another cancel
		(QWORD&)ClientConn.PartyLeader = 0;
		// Remove the reservation, now that we are no longer accessing it
		Reservations.Remove(PartyIndex);
	}
}

/**
 * Sends a client the specified response code
 *
 * @param Result the result being sent to the client
 * @param ClientSocket the client socket to send the response on
 */
void UPartyBeaconHost::SendReservationResponse(EPartyReservationResult Result,FSocket* ClientSocket)
{
	check(ClientSocket);
	INT NumRemaining = NumReservations - NumConsumedReservations;
	debugf(NAME_DevOnline,
		TEXT("Beacon (%s) sending host response (%s),(%d) to client (%s)"),
		*BeaconName.ToString(),
		PartyReservationResultToString(Result),
		NumRemaining,
		*ClientSocket->GetAddress().ToString(TRUE));
	FNboSerializeToBuffer ToBuffer(64);
	// Packet format is <Type><Result><NumRemaining>
	ToBuffer << (BYTE)RPT_HostReservationResponse
		<< (BYTE)Result
		<< NumRemaining;
	INT BytesSent;
	UBOOL bDidSendOk = ClientSocket->Send(ToBuffer.GetRawBuffer(0),ToBuffer.GetByteCount(),BytesSent);
	if (bDidSendOk == FALSE)
	{
		debugf(NAME_Error,
			TEXT("Beacon (%s) failed to send reservation response to client with (%s)"),
			*BeaconName.ToString(),
			GSocketSubsystem->GetSocketError());
	}
}

/**
 * Sends a client acknowledgement that it got the message
 *
 * @param ClientSocket the client socket to send the response on
 */
void UPartyBeaconHost::SendCancellationResponse(FSocket* ClientSocket)
{
	check(ClientSocket);
	INT NumRemaining = NumReservations - NumConsumedReservations;
	debugf(NAME_DevOnline,
		TEXT("Beacon (%s) sending cancellation acknowledgement to client (%s)"),
		*BeaconName.ToString(),
		*ClientSocket->GetAddress().ToString(TRUE));
	// Packet format is <RPT_HostCancellationResponse>
	BYTE Packet = RPT_HostCancellationResponse;
	INT BytesSent;
	UBOOL bDidSendOk = ClientSocket->Send(&Packet,1,BytesSent);
	if (bDidSendOk == FALSE)
	{
		debugf(NAME_Error,
			TEXT("Beacon (%s) failed to send cancellation response to client with (%s)"),
			*BeaconName.ToString(),
			GSocketSubsystem->GetSocketError());
	}
}

/**
 * Tells clients that a reservation update has occured and sends them the current
 * number of remaining reservations so they can update their UI
 */
void UPartyBeaconHost::SendReservationUpdates(void)
{
	INT NumRemaining = NumReservations - NumConsumedReservations;
	debugf(NAME_DevOnline,
		TEXT("Beacon (%s) sending reservation count update to clients (%d)"),
		*BeaconName.ToString(),
		NumRemaining);
	// Build the packet that we are sending
	FNboSerializeToBuffer ToBuffer(64);
	// Packet format is <Type><NumRemaining>
	ToBuffer << (BYTE)RPT_HostReservationCountUpdate
		<< NumRemaining;
	INT BytesSent;
	// Iterate through and send to each client
	for (INT SocketIndex = 0; SocketIndex < Clients.Num(); SocketIndex++)
	{
		FClientBeaconConnection& ClientConn = Clients(SocketIndex);
		if ((QWORD&)ClientConn.PartyLeader != (QWORD)0)
		{
			FSocket* ClientSocket = ClientConn.Socket;
			check(ClientSocket);
			UBOOL bDidSendOk = ClientSocket->Send(ToBuffer.GetRawBuffer(0),ToBuffer.GetByteCount(),BytesSent);
			if (bDidSendOk == FALSE)
			{
				debugf(NAME_Error,
					TEXT("Beacon (%s) failed to send reservation update to client with (%s)"),
					*BeaconName.ToString(),
					GSocketSubsystem->GetSocketError());
			}
		}
	}
}

/**
 * Tells all of the clients to go to a specific session (contained in platform specific info)
 *
 * @param SessionName the name of the session to register
 * @param SearchClass the search that should be populated with the session
 * @param PlatformSpecificInfo the binary data to place in the platform specific areas
 */
void UPartyBeaconHost::TellClientsToTravel(FName SessionName,UClass* SearchClass,BYTE* PlatformSpecificInfo)
{
	debugf(NAME_DevOnline,TEXT("Beacon (%s) sending travel information to clients"),*BeaconName.ToString());
	const FString& SessionNameStr = SessionName.ToString();
	const FString& ClassName = SearchClass->GetPathName();
	// Build the packet that we are sending
	FNboSerializeToBuffer ToBuffer(512);
	// Packet format is <Type><SessionNameLen><SessionName><ClassNameLen><ClassName><SecureSessionInfo>
	ToBuffer << (BYTE)RPT_HostTravelRequest
		<< SessionNameStr
		<< ClassName;
	// Copy the buffer over in raw form (it is already in NBO)
	ToBuffer.WriteBinary(PlatformSpecificInfo,68);
	INT BytesSent;
	// Iterate through and send to each client
	for (INT SocketIndex = 0; SocketIndex < Clients.Num(); SocketIndex++)
	{
		FClientBeaconConnection& ClientConn = Clients(SocketIndex);
		// Don't send to a party leader that we didn't accept
		if ((QWORD&)ClientConn.PartyLeader != (QWORD)0)
		{
			FSocket* ClientSocket = ClientConn.Socket;
			check(ClientSocket);
			UBOOL bDidSendOk = ClientSocket->Send(ToBuffer.GetRawBuffer(0),ToBuffer.GetByteCount(),BytesSent);
			if (bDidSendOk == FALSE)
			{
				debugf(NAME_Error,
					TEXT("Beacon (%s) failed to send travel request to client with (%s)"),
					*BeaconName.ToString(),
					GSocketSubsystem->GetSocketError());
			}
		}
	}
	bShouldTick = FALSE;
}

/**
 * Tells all of the clients that the host is ready for them to travel to
 */
void UPartyBeaconHost::TellClientsHostIsReady(void)
{
	debugf(NAME_DevOnline,
		TEXT("Beacon (%s) sending host is ready message to clients"),
		*BeaconName.ToString());
	BYTE Buffer = RPT_HostIsReady;
	INT BytesSent;
	// Iterate through and send to each client
	for (INT SocketIndex = 0; SocketIndex < Clients.Num(); SocketIndex++)
	{
		FClientBeaconConnection& ClientConn = Clients(SocketIndex);
		if ((QWORD&)ClientConn.PartyLeader != (QWORD)0)
		{
			FSocket* ClientSocket = ClientConn.Socket;
			check(ClientSocket);
			UBOOL bDidSendOk = ClientSocket->Send(&Buffer,1,BytesSent);
			if (bDidSendOk == FALSE)
			{
				debugf(NAME_Error,
					TEXT("Beacon (%s) failed to notify client that host is ready with (%s)"),
					*BeaconName.ToString(),
					GSocketSubsystem->GetSocketError());
			}
		}
	}
	bShouldTick = FALSE;
}

/**
 * Tells all of the clients that the host has cancelled the matchmaking beacon and that they
 * need to find a different host
 */
void UPartyBeaconHost::TellClientsHostHasCancelled(void)
{
	debugf(NAME_DevOnline,
		TEXT("Beacon (%s) sending host has cancelled message to clients"),
		*BeaconName.ToString());
	BYTE Buffer = RPT_HostHasCancelled;
	INT BytesSent;
	// Iterate through and send to each client
	for (INT SocketIndex = 0; SocketIndex < Clients.Num(); SocketIndex++)
	{
		FClientBeaconConnection& ClientConn = Clients(SocketIndex);
		if ((QWORD&)ClientConn.PartyLeader != (QWORD)0)
		{
			FSocket* ClientSocket = ClientConn.Socket;
			check(ClientSocket);
			UBOOL bDidSendOk = ClientSocket->Send(&Buffer,1,BytesSent);
			if (bDidSendOk == FALSE)
			{
				debugf(NAME_Error,
					TEXT("Beacon (%s) failed to notify client that host is cancelling with (%s)"),
					*BeaconName.ToString(),
					GSocketSubsystem->GetSocketError());
			}
		}
	}
	bShouldTick = FALSE;
}

/**
 * Appends the skills from all reservations to the search object so that they can
 * be included in the search information
 *
 * @param Search the search object to update
 */
void UPartyBeaconHost::AppendReservationSkillsToSearch(UOnlineGameSearch* Search)
{
	if (Search != NULL)
	{
		// Add each reservations players to the skill information
		for (INT PartyIndex = 0; PartyIndex < Reservations.Num(); PartyIndex++)
		{
			const FPartyReservation& Reservation = Reservations(PartyIndex);
			for (INT PlayerIndex = 0; PlayerIndex < Reservation.PartyMembers.Num(); PlayerIndex++)
			{
				const FPlayerReservation& PlayerRes = Reservation.PartyMembers(PlayerIndex);
				// Copy the player & their skill information over
				Search->ManualSkillOverride.Players.AddItem(PlayerRes.NetId);
				Search->ManualSkillOverride.Mus.AddItem(PlayerRes.Mu);
				Search->ManualSkillOverride.Sigmas.AddItem(PlayerRes.Sigma);
			}
		}
	}
}

/**
 * Cleans up any client connections and then routes to the base class
 */
void UPartyBeaconHost::DestroyBeacon(void)
{
#if WITH_UE3_NETWORKING
	if (Socket)
	{
		// Don't delete if this is during a tick or we'll crash
		if (bIsInTick == FALSE)
		{
			// Destroy each socket in the list and then empty it
			for (INT Index = 0; Index < Clients.Num(); Index++)
			{
				GSocketSubsystem->DestroySocket(Clients(Index).Socket);
			}
			Clients.Empty();
		}
	}
	Super::DestroyBeacon();
#endif
}

/**
 * Performs platform specific resolution of the address
 *
 * @param DesiredHost the host to resolve the IP address for
 * @param Addr out param having it's address set
 *
 * @return true if the address could be resolved, false otherwise
 */
UBOOL UPartyBeaconClient::ResolveAddress(const FOnlineGameSearchResult& DesiredHost,FInternetIpAddr& Addr)
{
#if WITH_UE3_NETWORKING
	// Use the session information to build the address
	FSessionInfo* SessionInfo = (FSessionInfo*)DesiredHost.PlatformData;
	if (SessionInfo != NULL)
	{
		// Copy the destination IP
		Addr = SessionInfo->HostAddr;
		// Set to the configured port rather than what's in the address
		Addr.SetPort(PartyBeaconPort);
		return TRUE;
	}
#endif
	return FALSE;
}

/**
 * Creates a beacon that will send requests to remote hosts
 *
 * @param Addr the address that we are connecting to (needs to be resolved)
 *
 * @return true if the beacon was created successfully, false otherwise
 */
UBOOL UPartyBeaconClient::InitClientBeacon(const FInternetIpAddr& Addr)
{
#if WITH_UE3_NETWORKING
	// Now create and set up our TCP socket
	Socket = GSocketSubsystem->CreateStreamSocket();
	if (Socket != NULL)
	{
		// Set basic state
		Socket->SetReuseAddr();
		Socket->SetNonBlocking();
		// Connect to the remote host so we can send the request
		if (Socket->Connect(Addr))
		{
			ClientBeaconState = PBCS_Connecting;
			return TRUE;
		}
	}
	// Failed to create the connection
	ClientBeaconState = PBCS_ConnectionFailed;
#endif
	return FALSE;
}

/**
 * Sends a request to the remote host to allow the specified members to reserve space
 * in the host's session. Note this request is async and the results will be sent via
 * the delegate
 *
 * @param DesiredHost the server that the connection will be made to
 * @param RequestingPartyLeader the leader of this party that will be joining
 * @param Players the list of players that want to reserve space
 *
 * @return true if the request async task started ok, false if it failed to send
 */
UBOOL UPartyBeaconClient::RequestReservation(const FOnlineGameSearchResult& DesiredHost,FUniqueNetId RequestingPartyLeader,const TArray<FPlayerReservation>& Players)
{
	UBOOL bWasStarted = FALSE;
#if WITH_UE3_NETWORKING
	// Register the secure keys so we can decrypt and communicate
	if (RegisterAddress(DesiredHost))
	{
		FInternetIpAddr SendTo;
		// Make sure we can resolve where we are sending to
		if (ResolveAddress(DesiredHost,SendTo))
		{
			HostPendingRequest = DesiredHost;
			// Copy the party information
			PendingRequest.PartyLeader = RequestingPartyLeader;
			PendingRequest.PartyMembers = Players;
			InitClientBeacon(SendTo);
			// At this point we have a socket to the server and we'll check in tick when the
			// connection is fully established
			bWasStarted = TRUE;
			// Reset the request timeout
			ReservationRequestElapsedTime = 0.f;
		}
		else
		{
			debugf(NAME_DevOnline,
				TEXT("Beacon (%s) RequestReservation() failed to resolve party host address"),
				*BeaconName.ToString());
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Beacon (%s) RequestReservation() failed to register the party host address"),
			*BeaconName.ToString());
	}
#endif
	// Fire the delegate so the code can process the next items
	if (bWasStarted == FALSE)
	{
		DestroyBeacon();
	}
	return bWasStarted;
}

/**
 * Sends a cancellation message to the remote host so that it knows there is more
 * space available
 *
 * @param CancellingPartyLeader the leader of this party that wants to cancel
 *
 * @return true if the request able to be sent, false if it failed to send
 */
UBOOL UPartyBeaconClient::CancelReservation(FUniqueNetId CancellingPartyLeader)
{
	bShouldTick = FALSE;
	if (Socket != NULL)
	{
		// Create a buffer and serialize the request in
		FNboSerializeToBuffer ToBuffer(64);
		ToBuffer << (BYTE)RPT_ClientCancellationRequest
			<< CancellingPartyLeader;
		INT BytesSent;
		// Send the message indicating the party that is cancelling
		UBOOL bDidSendOk = Socket->Send(ToBuffer.GetRawBuffer(0),ToBuffer.GetByteCount(),BytesSent);
		if (bDidSendOk == FALSE)
		{
			debugf(NAME_Error,
				TEXT("Beacon (%s) failed to send cancel reservation to host with (%s)"),
				*BeaconName.ToString(),
				GSocketSubsystem->GetSocketError());
		}
		return bDidSendOk;
	}
	return FALSE;
}

/**
 * Once the socket has been established, it sends the pending request to the host
 */
void UPartyBeaconClient::SendReservationRequest(void)
{
	// Create a buffer and serialize the request in
	FNboSerializeToBuffer ToBuffer(512);
	// Packet format is <Type><PartyLeader><PartySize><PartyMember1><PartyMemberN>...
	ToBuffer << (BYTE)RPT_ClientReservationRequest;
	// Serialize the leader to the buffer
	ToBuffer << PendingRequest.PartyLeader;
	DWORD PartySize = PendingRequest.PartyMembers.Num();
	// Write the number players in the party
	ToBuffer << PartySize;
	// Now serialize each member
	for (INT Index = 0; Index < PendingRequest.PartyMembers.Num(); Index++)
	{
		const FPlayerReservation& PlayerRes = PendingRequest.PartyMembers(Index);
		ToBuffer << PlayerRes.NetId
			<< PlayerRes.Skill
			<< PlayerRes.Mu
			<< PlayerRes.Sigma;
	}
	INT BytesSent;
	// Now send to the destination host
	if (Socket->Send(ToBuffer.GetRawBuffer(0),ToBuffer.GetByteCount(),BytesSent))
	{
		ClientBeaconState = PBCS_AwaitingResponse;
		debugf(NAME_DevOnline,
			TEXT("Beacon (%s) sent party reservation request with (%d) players to (%s)"),
			*BeaconName.ToString(),
			PartySize,
			*Socket->GetAddress().ToString(TRUE));
	}
	else
	{
		// Failed to send, so mark the request as failed
		ClientBeaconState = PBCS_ConnectionFailed;
		debugf(NAME_DevOnline,
			TEXT("Beacon (%s) SendRequest() failed to send the packet to the host (%s) with error code (%s)"),
			*BeaconName.ToString(),
			*Socket->GetAddress().ToString(TRUE),
			GSocketSubsystem->GetSocketError());
	}
}

/**
 * Processes a packet that was received from the host indicating success or
 * failure for our reservation
 *
 * @param Packet the packet that the host sent
 * @param PacketSize the size of the packet to process
 */
void UPartyBeaconClient::ProcessHostResponse(BYTE* Packet,INT PacketSize)
{
	// Use our packet serializer to read from the raw buffer
	FNboSerializeFromBuffer FromBuffer(Packet,PacketSize);
	// Work through the stream until we've consumed it all
	do
	{
		BYTE PacketType = RPT_UnknownPacketType;
		FromBuffer >> PacketType;
		// Don't process a packet if we are at the end
		if (FromBuffer.HasOverflow() == FALSE)
		{
			// Route the call to the proper handler
			switch (PacketType)
			{
				case RPT_HostReservationResponse:
				{
					ProcessReservationResponse(FromBuffer);
					break;
				}
				case RPT_HostCancellationResponse:
				{
					ProcessCancellationResponse(FromBuffer);
					break;
				}
				case RPT_HostReservationCountUpdate:
				{
					ProcessReservationCountUpdate(FromBuffer);
					break;
				}
				case RPT_HostTravelRequest:
				{
					ProcessTravelRequest(FromBuffer);
					break;
				}
				case RPT_HostIsReady:
				{
					ProcessHostIsReady();
					break;
				}
				case RPT_HostHasCancelled:
				{
					ProcessHostCancelled();
					break;
				}
				case RPT_Heartbeat:
				{
					// Respond to this so they know we are alive
					ProcessHeartbeat();
					break;
				}
				default:
				{
					debugf(NAME_DevOnline,
						TEXT("Beacon (%s) unknown packet type received from host (%d)"),
						*BeaconName.ToString(),
						(DWORD)PacketType);
					break;
				}
			}
		}
	}
	while (FromBuffer.HasOverflow() == FALSE);
}

/**
 * Notifies the delegates that the host is ready to play
 */
void UPartyBeaconClient::ProcessHostIsReady(void)
{
	bShouldTick = FALSE;
	CleanupAddress();
	delegateOnHostIsReady();
}

/**
 * Processes a reservation response packet that was received from the host
 *
 * @param FromBuffer the packet serializer to read from
 */
void UPartyBeaconClient::ProcessReservationResponse(FNboSerializeFromBuffer& FromBuffer)
{
	BYTE Result = PRR_GeneralError;
	FromBuffer >> Result;
	INT ReservationRemaining = 0;
	FromBuffer >> ReservationRemaining;
	debugf(NAME_DevOnline,
		TEXT("Beacon (%s) host (%s) response was (%s) with %d reservations remaining"),
		*BeaconName.ToString(),
		*Socket->GetAddress().ToString(TRUE),
		PartyReservationResultToString((EPartyReservationResult)Result),
		ReservationRemaining);
	// Tell the game code the result
	delegateOnReservationRequestComplete((EPartyReservationResult)Result);
}

/**
 * Processes a cancellation response packet that was received from the host
 *
 * @param FromBuffer the packet serializer to read from
 */
void UPartyBeaconClient::ProcessCancellationResponse(FNboSerializeFromBuffer& FromBuffer)
{
	bShouldTick = FALSE;
	CleanupAddress();
	debugf(NAME_DevOnline,
		TEXT("Beacon (%s) host ack-ed the cancellation request"),
		*BeaconName.ToString());
	delegateOnCancellationRequestComplete();
}

/**
 * Processes a reservation count update packet that was received from the host
 *
 * @param FromBuffer the packet serializer to read from
 */
void UPartyBeaconClient::ProcessReservationCountUpdate(FNboSerializeFromBuffer& FromBuffer)
{
	INT ReservationsRemaining = 0;
	FromBuffer >> ReservationsRemaining;
	debugf(NAME_DevOnline,
		TEXT("Beacon (%s) host (%s) reservations remaining is (%d)"),
		*BeaconName.ToString(),
		*Socket->GetAddress().ToString(TRUE),
		ReservationsRemaining);
	// Notify the client of the change
	delegateOnReservationCountUpdated(ReservationsRemaining);
}

/**
 * Processes a travel request packet that was received from the host
 *
 * @param FromBuffer the packet serializer to read from
 */
void UPartyBeaconClient::ProcessTravelRequest(FNboSerializeFromBuffer& FromBuffer)
{
	bShouldTick = FALSE;
	FString SessionNameStr;
	FString ClassNameStr;
	BYTE DestinationInfo[68];
	// Read the two strings first
	FromBuffer >> SessionNameStr >> ClassNameStr;
	// Now copy the buffer data in
	FromBuffer.ReadBinary(DestinationInfo,68);
	debugf(NAME_DevOnline,
		TEXT("Beacon (%s) host (%s) sent travel request for (%s),(%s)"),
		*BeaconName.ToString(),
		*Socket->GetAddress().ToString(TRUE),
		*SessionNameStr,
		*ClassNameStr);
	FName SessionName(*SessionNameStr,0);
	UClass* SearchClass = FindObject<UClass>(NULL,*ClassNameStr);
	CleanupAddress();
	// Fire the delegate so the client can process
	delegateOnTravelRequestReceived(SessionName,SearchClass,DestinationInfo);
}

/**
 * Ticks the network layer to see if there are any requests or responses to requests
 *
 * @param DeltaTime the amount of time that has elapsed since the last tick
 */
void UPartyBeaconClient::Tick(FLOAT DeltaTime)
{
#if WITH_UE3_NETWORKING
	// Only tick if we have a socket present
	if (Socket && bShouldTick && bWantsDeferredDestroy == FALSE)
	{
		// Use this to guard against deleting the socket while in use
		bIsInTick = TRUE;
		switch (ClientBeaconState)
		{
			case PBCS_Connecting:
			{
				CheckConnectionStatus();
				ReservationRequestElapsedTime += DeltaTime;
				// Check for a timeout or an error
				if (ReservationRequestElapsedTime > ReservationRequestTimeout ||
					ClientBeaconState == PBCS_ConnectionFailed)
				{
					debugf(NAME_DevOnline,
						TEXT("Beacon (%s) timeout waiting for host to accept our connection"),
						*BeaconName.ToString());
					ProcessHostCancelled();
				}
				break;
			}
			case PBCS_Connected:
			{
				// Send the request to the host and transition to waiting for a response
				SendReservationRequest();
				ReservationRequestElapsedTime += DeltaTime;
				// Check for a timeout or an error
				if (ReservationRequestElapsedTime > ReservationRequestTimeout ||
					ClientBeaconState == PBCS_ConnectionFailed)
				{
					debugf(NAME_DevOnline,
						TEXT("Beacon (%s) timeout waiting for host to respond to our reservation request"),
						*BeaconName.ToString());
					ProcessHostCancelled();
				}
				break;
			}
			case PBCS_AwaitingResponse:
			{
				// Increment the time since we've last seen a heartbeat
				ElapsedHeartbeatTime += DeltaTime;
				// Read and process any host packets
				ReadResponse();
				// Make sure we haven't processed a packet saying to travel
				if (bShouldTick && bWantsDeferredDestroy == FALSE)
				{
					// Check to see if we've lost the host
					if (ElapsedHeartbeatTime > HeartbeatTimeout ||
						ClientBeaconState == PBCS_ConnectionFailed)
					{
						ProcessHostCancelled();
					}
				}
				break;
			}
		}
		// No longer a danger, so release the sentinel on the destruction
		bIsInTick = FALSE;
	}
	Super::Tick(DeltaTime);
#endif
}

/**
 * Handles checking for the transition from connecting to connected (socket established)
 */
void UPartyBeaconClient::CheckConnectionStatus(void)
{
	ESocketConnectionState State = Socket->GetConnectionState();
	if (State == SCS_Connected)
	{
		ClientBeaconState = PBCS_Connected;
	}
	else if (State == SCS_ConnectionError)
	{
		INT SocketErrorCode = GSocketSubsystem->GetLastErrorCode();
		// Continue to allow the session to be established for EWOULDBLOCK (not an error)
		if (SocketErrorCode != SE_EWOULDBLOCK)
		{
			debugf(NAME_DevOnline,
				TEXT("Beacon (%s) error connecting to host (%s) with error (%s)"),
				*BeaconName.ToString(),
				*Socket->GetAddress().ToString(TRUE),
				GSocketSubsystem->GetSocketError());
			ClientBeaconState = PBCS_ConnectionFailed;
		}
	}
}

/**
 * Checks the socket for a response from the and processes if present
 */
void UPartyBeaconClient::ReadResponse(void)
{
	UBOOL bShouldRead = TRUE;
	BYTE PacketData[512];
	// Read each pending packet and pass it out for processing
	while (bShouldRead && bShouldTick && bWantsDeferredDestroy == FALSE)
	{
		INT BytesRead = 0;
		// Read the response from the socket
		if (Socket->Recv(PacketData,512,BytesRead))
		{
			if (BytesRead > 0)
			{
				ProcessHostResponse(PacketData,BytesRead);
			}
			else
			{
				bShouldRead = FALSE;
			}
		}
		else
		{
			// Check for an error other than would block, which isn't really an error
			INT ErrorCode = GSocketSubsystem->GetLastErrorCode();
			if (ErrorCode != SE_EWOULDBLOCK)
			{
				debugf(NAME_DevOnline,
					TEXT("Beacon (%s) socket error (%s) detected trying to read host response from (%s)"),
					*BeaconName.ToString(),
					GSocketSubsystem->GetSocketError(ErrorCode),
					*Socket->GetAddress().ToString(TRUE));
				ClientBeaconState = PBCS_ConnectionFailed;
			}
			bShouldRead = FALSE;
		}
	}
}

/**
 * Stops listening for requests/responses and releases any allocated memory
 */
void UPartyBeaconClient::DestroyBeacon(void)
{
	if (bIsInTick == FALSE)
	{
		CleanupAddress();
	}
	// Clean up the socket now too
	Super::DestroyBeacon();
}

/** Unregisters the address and zeros the members involved to prevent multiple releases */
void UPartyBeaconClient::CleanupAddress(void)
{
	// Release any memory associated with communicating with the host
	UnregisterAddress(HostPendingRequest);
	// Zero since this was a shallow copy
	HostPendingRequest.GameSettings = NULL;
	HostPendingRequest.PlatformData = NULL;
	ClientBeaconState = PBCS_Closed;
}

/**
 * Processes a heartbeat update, sends a heartbeat back, and clears the timer
 */
void UPartyBeaconClient::ProcessHeartbeat(void)
{
	ElapsedHeartbeatTime = 0.f;
	if (Socket != NULL)
	{
		// Notify the host that we received their heartbeat
		SendHeartbeat(Socket);
	}
}
