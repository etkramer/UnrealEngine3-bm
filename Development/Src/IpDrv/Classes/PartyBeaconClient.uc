/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This class is used to connect to a network beacon in order to request
 * a reservation for their party with that host
 */
class PartyBeaconClient extends PartyBeacon
	native;

/** Holds a reference to the data that is used to reach the potential host */
var const OnlineGameSearchResult HostPendingRequest;

/** The request to send once the socket is established */
var PartyReservation PendingRequest;

/** Used to drive the client state machine */
enum EPartyBeaconClientState
{
	// Inactive or unknown
	PBCS_None,
	// A connection request is outstanding with host
	PBCS_Connecting,
	// Connected to the host and is ready to send
	PBCS_Connected,
	// Failed to establish a connection
	PBCS_ConnectionFailed,
	// Client has sent to the host and is awaiting for replies
	PBCS_AwaitingResponse,
	// The client has closed the connection
	PBCS_Closed
};

/** The state of the client beacon */
var EPartyBeaconClientState ClientBeaconState;

/** Indicates how long the client should wait for a response before timing out and trying a new server */
var config float ReservationRequestTimeout;

/** Used to track how long we've been waiting for a response */
var float ReservationRequestElapsedTime;

cpptext
{
	/**
	 * Ticks the network layer to see if there are any requests or responses to requests
	 *
	 * @param DeltaTime the amount of time that has elapsed since the last tick
	 */
	virtual void Tick(FLOAT DeltaTime);

	/**
	 * Creates a beacon that will send requests to remote hosts
	 *
	 * @param Addr the address that we are connecting to (needs to be resolved)
	 *
	 * @return true if the beacon was created successfully, false otherwise
	 */
	UBOOL InitClientBeacon(const FInternetIpAddr& Addr);

	/**
	 * Once the socket has been established, it sends the pending request to the host
	 */
	void SendReservationRequest(void);

	/**
	 * Processes a packet that was received from the host indicating success or
	 * failure for our reservation
	 *
	 * @param Packet the packet that the host sent
	 * @param PacketSize the size of the packet to process
	 */
	void ProcessHostResponse(BYTE* Packet,INT PacketSize);

	/**
	 * Processes a reservation response packet that was received from the host
	 *
	 * @param FromBuffer the packet serializer to read from
	 */
	void ProcessReservationResponse(FNboSerializeFromBuffer& FromBuffer);
	
	/**
	 * Processes a cancellation response packet that was received from the host
	 *
	 * @param FromBuffer the packet serializer to read from
	 */
	void ProcessCancellationResponse(FNboSerializeFromBuffer& FromBuffer);

	/**
	 * Processes a reservation count update packet that was received from the host
	 *
	 * @param FromBuffer the packet serializer to read from
	 */
	void ProcessReservationCountUpdate(FNboSerializeFromBuffer& FromBuffer);

	/**
	 * Processes a heartbeat update, sends a heartbeat back, and clears the timer
	 */
	void ProcessHeartbeat(void);

	/**
	 * Notifies the delegates that the host is ready to play
	 */
	void ProcessHostIsReady(void);

	/**
	 * Processes a travel request packet that was received from the host
	 *
	 * @param FromBuffer the packet serializer to read from
	 */
	void ProcessTravelRequest(FNboSerializeFromBuffer& FromBuffer);

	/**
	 * Performs platform specific resolution of the address
	 *
	 * @param DesiredHost the host to resolve the IP address for
	 * @param Addr out param having it's address set
	 *
	 * @return true if the address could be resolved, false otherwise
	 */
	virtual UBOOL ResolveAddress(const FOnlineGameSearchResult& DesiredHost,FInternetIpAddr& Addr);

	/**
	 * Allows for per platform registration of secure keys, so that a secure connection
	 * can be opened and used for sending/receiving data.
	 *
	 * @param DesiredHost the host that is being registered
	 */
	virtual UBOOL RegisterAddress(const FOnlineGameSearchResult& DesiredHost)
	{
		return TRUE;
	}

	/**
	 * Allows for per platform unregistration of secure keys, which breaks the link between
	 * a client and server. This also releases any memory associated with the keys.
	 *
	 * @param DesiredHost the host that is being registered
	 */
	virtual UBOOL UnregisterAddress(const FOnlineGameSearchResult& DesiredHost)
	{
		return TRUE;
	}

	/** Unregisters the address and zeros the members involved to prevent multiple releases */
	void CleanupAddress(void);

	/**
	 * Handles checking for the transition from connecting to connected (socket established)
	 */
	void CheckConnectionStatus(void);

	/**
	 * Checks the socket for a response from the and processes if present
	 */
	void ReadResponse(void);

	/** Common routine for cancelling matchmaking */
	inline void ProcessHostCancelled(void)
	{
		CleanupAddress();
		delegateOnHostHasCancelled();
	}
}

/**
 * Called by the beacon when a reservation request has been responded to by the destination host
 *
 * @param ReservationResult whether there was space allocated for the party or not
 */
delegate OnReservationRequestComplete(EPartyReservationResult ReservationResult);

/**
 * Called by the beacon when a cancellation request has been acknowledged by the destination host
 */
delegate OnCancellationRequestComplete();

/**
 * Called by the beacon when the host sends a reservation count update packet so
 * that any UI can be updated
 *
 * @param ReservationRemaining the number of reservations that are still available
 */
delegate OnReservationCountUpdated(int ReservationRemaining);

/**
 * Called by the beacon when the host sends a request for all clients to travel to
 * the destination included in the packet
 *
 * @param SessionName the name of the session to register
 * @param SearchClass the search that should be populated with the session
 * @param PlatformSpecificInfo the binary data to place in the platform specific areas
 */
delegate OnTravelRequestReceived(name SessionName,class<OnlineGameSearch> SearchClass,byte PlatformSpecificInfo[68]);

/**
 * Called by the beacon when the host sends the "ready" packet, so the client
 * can connect to the host to start the match
 */
delegate OnHostIsReady();

/**
 * Called by the beacon when the host sends the "cancellation" packet, so the client
 * can return to finding a new host
 */
delegate OnHostHasCancelled();

/**
 * Sends a request to the remote host to allow the specified members to reserve space
 * in the host's session. Note this request is async and the results will be sent via
 * the delegate
 *
 * @param DesiredHost the server that the connection will be made to
 * @param RequestingPartyLeader the leader of this party that will be joining
 * @param Players the list of players that want to reserve space
 *
 * @return true if the request able to be sent, false if it failed to send
 */
native function bool RequestReservation(const out OnlineGameSearchResult DesiredHost,UniqueNetId RequestingPartyLeader,const out array<PlayerReservation> Players);

/**
 * Sends a cancellation message to the remote host so that it knows there is more
 * space available
 *
 * @param CancellingPartyLeader the leader of this party that wants to cancel
 *
 * @return true if the request able to be sent, false if it failed to send
 */
native function bool CancelReservation(UniqueNetId CancellingPartyLeader);

/**
 * Stops listening for requests/responses and releases any allocated memory
 */
native event DestroyBeacon();
