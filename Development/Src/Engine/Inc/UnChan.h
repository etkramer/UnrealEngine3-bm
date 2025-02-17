/*=============================================================================
	UnChan.h: Unreal datachannel class.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Constant for all buffers that are reading from the network
#define MAX_STRING_SERIALIZE_SIZE	2048

/*-----------------------------------------------------------------------------
	UChannel base class.
-----------------------------------------------------------------------------*/
//
// Base class of communication channels.
//
class UChannel : public UObject
{
	DECLARE_ABSTRACT_CLASS(UChannel,UObject,CLASS_Transient|CLASS_Intrinsic,Engine);

	// Variables.
	UNetConnection*	Connection;		// Owner connection.
	BITFIELD		OpenAcked:1;		// Whether open has been acknowledged.
	BITFIELD		Closing:1;		// State of the channel.
	BITFIELD		OpenTemporary:1;	// Opened temporarily.
	BITFIELD		Broken:1;			// Has encountered errors and is ignoring subsequent packets.
	BITFIELD		bTornOff:1;		// Actor associated with this channel was torn off
	INT             ChIndex;		// Index of this channel.
	INT				OpenedLocally;	// Whether channel was opened locally or by remote.
	INT				OpenPacketId;	// Packet the spawn message was sent in.
	EChannelType	ChType;			// Type of this channel.
	INT				NumInRec;		// Number of packets in InRec.
	INT				NumOutRec;		// Number of packets in OutRec.
	INT				NegotiatedVer;	// Negotiated version of engine = Min(client version, server version).
	FInBunch*		InRec;			// Incoming data with queued dependencies.
	FOutBunch*		OutRec;			// Outgoing reliable unacked data.

	// Statics.
	static UClass* ChannelClasses[CHTYPE_MAX];
	static UBOOL IsKnownChannelType( INT Type );

	// Constructor.
	UChannel();
	virtual void BeginDestroy();

	// UChannel interface.
	virtual void Init( UNetConnection* InConnection, INT InChIndex, UBOOL InOpenedLocally );
	virtual void SetClosingFlag();
	virtual void Close();
	virtual FString Describe();
#if WITH_UE3_NETWORKING
	virtual void ReceivedBunch( FInBunch& Bunch ) PURE_VIRTUAL(UChannel::ReceivedBunch,);
	virtual void ReceivedNak( INT NakPacketId );
#endif	//#if WITH_UE3_NETWORKING
	virtual void Tick();

	// General channel functions.
#if WITH_UE3_NETWORKING
	void ReceivedAcks();
	UBOOL ReceivedSequencedBunch( FInBunch& Bunch );
	void ReceivedRawBunch( FInBunch& Bunch );
	INT SendBunch( FOutBunch* Bunch, UBOOL Merge );
#endif	//#if WITH_UE3_NETWORKING
	INT IsNetReady( UBOOL Saturate );
	void AssertInSequenced();
	INT MaxSendBytes();

	/** cleans up channel if it hasn't already been */
	void ConditionalCleanUp()
	{
		if (!HasAnyFlags(RF_PendingKill))
		{
			MarkPendingKill();
			CleanUp();
		}
	}
protected:
	/** cleans up channel structures and NULLs references to the channel */
	virtual void CleanUp();
};

/*-----------------------------------------------------------------------------
	UControlChannel base class.
-----------------------------------------------------------------------------*/

//
// A channel for exchanging text.
//
class UControlChannel : public UChannel, public FOutputDevice
{
	DECLARE_CLASS(UControlChannel,UChannel,CLASS_Transient,Engine);
	/**
	 * Used to interrogate the first packet received to determine endianess
	 * of the sending client
	 */
	UBOOL bNeedsEndianInspection;

	/** provies an extra buffer beyond RELIABLE_BUFFER for control channel messages
	 * as we must be able to guarantee delivery for them
	 * because they include package map updates and other info critical to client/server synchronization
	 */
	TArray<FString> QueuedMessages;

	/** maximum size of additional buffer
	 * if this is exceeded as well, we kill the connection
	 */
	enum { MAX_QUEUED_CONTROL_MESSAGES = 256 };

	/**
	 * Inspects the packet for endianess information. Validates this information
	 * against what the client sent. If anything seems wrong, the connection is
	 * closed
	 *
	 * @param Bunch the packet to inspect
	 *
	 * @return TRUE if the packet is good, FALSE otherwise. FALSE means shutdown
	 */
	UBOOL CheckEndianess(FInBunch& Bunch);

	/** adds the given string to the QueuedMessages list. Closes the connection if MAX_QUEUED_CONTROL_MESSAGES is exceeded */
	void QueueMessage(const TCHAR* Data);

	// Constructor.
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues()
	{
		ChannelClasses[CHTYPE_Control]      = GetClass();
		ChType								= CHTYPE_Control;
	}
	UControlChannel();
	void Init( UNetConnection* InConnection, INT InChIndex, UBOOL InOpenedLocally );

	// UChannel interface.
#if WITH_UE3_NETWORKING
	void ReceivedBunch( FInBunch& Bunch );
#endif	//#if WITH_UE3_NETWORKING
	virtual void Tick();

	// FArchive interface.
	void Serialize( const TCHAR* Data, EName Event );

	// UControlChannel interface.
	FString Describe();
};

/*-----------------------------------------------------------------------------
	UActorChannel.
-----------------------------------------------------------------------------*/

//
// A channel for exchanging actor properties.
//
class UActorChannel : public UChannel
{
	DECLARE_CLASS(UActorChannel,UChannel,CLASS_Transient,Engine);
	
	// Variables.
	UWorld*	World;			// World this actor channel is associated with.
	AActor* Actor;			// Actor this corresponds to.
	UClass* ActorClass;		// Class of the actor.
	DOUBLE	RelevantTime;	// Last time this actor was relevant to client.
	DOUBLE	LastUpdateTime;	// Last time this actor was replicated.
	DOUBLE	LastFullUpdateTime;	// Last time this actor was fully replicated.
	BITFIELD SpawnAcked:1;	    // Whether spawn has been acknowledged.
	BITFIELD ActorDirty:1;		// Whether actor is dirty
	BITFIELD bActorMustStayDirty:1; // ActorDirty may not be cleared at end of this tick
	BITFIELD bActorStillInitial:1;	// Not all properties sent while bNetInitial, so still bNetInitial next tick
	BITFIELD bIsReplicatingActor:1; // true when in this channel's ReplicateActor() to avoid recursion as that can cause invalid data to be sent
	/** whether we should NULL references to this channel's Actor in other channels' Recent data when this channel is closed
	 * set to false in cases where the Actor can't become relevant again (e.g. destruction) as it's unnecessary in that case
	 */
	BITFIELD bClearRecentActorRefs:1;
	TArray<BYTE> Recent;	// Most recently sent values.
	TArray<BYTE> RepEval;	// Evaluated replication conditions.
	TArray<INT>  Dirty;     // Properties that are dirty and need resending.
	TArray<FPropertyRetirement> Retirement; // Property retransmission.
	/** list of replicated actor properties for ActorClass
	 * this is used to NULL Recent's references to Actors that lose relevancy
	 */
	TArray<UObjectProperty*> ReplicatedActorProperties;

	// Constructor.
	void StaticConstructor();

	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	*/
	void InitializeIntrinsicPropertyValues()
	{
		ChannelClasses[CHTYPE_Actor]        = GetClass();
		ChType = CHTYPE_Actor;
		bClearRecentActorRefs = TRUE;
	}
	UActorChannel();
	void Init( UNetConnection* InConnection, INT InChIndex, UBOOL InOpenedLocally );

	// UChannel interface.
	virtual void SetClosingFlag();
#if WITH_UE3_NETWORKING
	virtual void ReceivedBunch( FInBunch& Bunch );
	virtual void ReceivedNak( INT NakPacketId );
#endif	//#if WITH_UE3_NETWORKING
	virtual void Close();

	// UActorChannel interface and accessors.
	AActor* GetActor() {return Actor;}
	FString Describe();
	void ReplicateActor();
	void SetChannelActor( AActor* InActor );

protected:
	/** cleans up channel structures and NULLs references to the channel */
	virtual void CleanUp();
};

/*-----------------------------------------------------------------------------
	File transfer channel.
-----------------------------------------------------------------------------*/

//
// A channel for exchanging binary files.
//
class UFileChannel : public UChannel
{
	DECLARE_CLASS(UFileChannel,UChannel,CLASS_Transient,Engine);

	// Receive Variables.
	UChannelDownload*	Download;		 // UDownload when receiving.

	// Send Variables.
	FArchive*			SendFileAr;		 // File being sent.
	TCHAR				SrcFilename[256];// Filename being sent.
	INT					PackageIndex;	 // Index of package in map.
	INT					SentData;		 // Number of bytes sent.

	// Constructor.
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues()
	{
		ChannelClasses[CHTYPE_File]        = GetClass();
		ChType = CHTYPE_File;
	}
	UFileChannel();
	void Init( UNetConnection* InConnection, INT InChIndex, UBOOL InOpenedLocally );

	// UChannel interface.
#if WITH_UE3_NETWORKING
	virtual void ReceivedBunch( FInBunch& Bunch );
#endif	//#if WITH_UE3_NETWORKING

	// UFileChannel interface.
	FString Describe();
	void Tick();

protected:
	/** cleans up channel structures and NULLs references to the channel */
	virtual void CleanUp();
};

/**
 * This channel is responsible for passing voice data as part of our network data
 */
class UVoiceChannel : public UChannel
{
	DECLARE_CLASS(UVoiceChannel,UChannel,CLASS_Transient | CLASS_Intrinsic,Engine);

	/** The set of voice packets for this channel */
	FVoicePacketList VoicePackets;

	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues()
	{
		ChannelClasses[CHTYPE_Voice] = GetClass();
		ChType = CHTYPE_Voice;
	}

	/**
	 * Adds the voice packet to the list to send for this channel
	 *
	 * @param VoicePacket the voice packet to send
	 */
	void AddVoicePacket(FVoicePacket* VoicePacket);

// UObject interface

	/**
	 * Tracks how much memory is being used by this object (no persistence)
	 *
	 * @param Ar the archive to serialize against
	 */
	virtual void Serialize(FArchive& Ar);

// UChannel interface
protected:
	/** Cleans up any voice data remaining in the queue */
	virtual void CleanUp(void);

#if WITH_UE3_NETWORKING
	/**
	 * Processes the in bound bunch to extract the voice data
	 *
	 * @param Bunch the voice data to process
	 */
	virtual void ReceivedBunch(FInBunch& Bunch);
#endif	//#if WITH_UE3_NETWORKING

#if !XBOX && !WITH_PANORAMA
	/**
	 * Performs any per tick update of the VoIP state
	 */
	virtual void Tick(void);
#endif

	/** Human readable information about the channel */
	virtual FString Describe(void)
	{
		return FString(TEXT("VoIP: ")) + UChannel::Describe();
	}
};

