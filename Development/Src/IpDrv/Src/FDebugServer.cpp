/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * This file contains the implementation of the debug server.
 */

#include "UnIpDrv.h"
#include "FDebugServer.h"
#include "DebugServerDefs.h"

#if WITH_UE3_NETWORKING
#if !SHIPPING_PC_GAME

/*------------------------------ FDebugServer::FClientConnection ----------------------------------*/

/** Creates a listen socket and listens for a connect. */
void FDebugServer::FClientConnection::BeginListening()
{
	if ( bIsStreamingConnection )
	{
		// Streaming connection.
		check( ListenSocket == NULL );
		check( SenderSocket == NULL );
		check( ConnectionState == ClientState_Initialized );

		ListenAddr.SetPort(DefaultDebugChannelListenPort);
		ListenAddr.SetIp(getlocalbindaddr(*GWarn));

		ListenSocket = GSocketSubsystem->CreateStreamSocket();
		if ( ListenSocket == NULL )
		{
			// Socket creation failed; close the connection.
			DestroySockets();
		}
		else
		{
			// Socket was created; listen for connections.
			ListenSocket->SetReuseAddr();
			ListenSocket->SetNonBlocking( TRUE );
			ListenSocket->SetRecvErr();
			if ( !ListenSocket->Bind( ListenAddr ) )
			{
				// Error encountered; close the connection.
				DestroySockets();
			}
			else
			{
				const UBOOL bListenResult = ListenSocket->Listen( 1 );
				if ( bListenResult )
				{
					ConnectStartTime = appSeconds();
					ConnectionState = ClientState_Connecting;
				}
			}
		}
	}
	else
	{
		// Datagram connection
		SenderSocket = GSocketSubsystem->CreateDGramSocket(TRUE);
		if ( SenderSocket == NULL )
		{
			// Socket creation failed; close the connection.
			DestroySockets();
		}
		else
		{
			// Socket created successfully; go straight to 'connected' state.
			ConnectionState = ClientState_Connected;
		}
	}
}

/**
 * Monitors the socket connection, moving to a connected state if the connection
 * request was answered, or closing if the request timed out or an error occurred.
 */
void FDebugServer::FClientConnection::Connecting()
{
	if ( bIsStreamingConnection )
	{
		check( ListenSocket != NULL );
		check( SenderSocket == NULL );
		check( ConnectionState == ClientState_Connecting );

		SenderSocket = ListenSocket->Accept();
		if ( SenderSocket )
		{
			// A client connection has been accepted, so close the listen socket.
			DestroyListenSocket();

			// Connection established.
			ConnectionState = ClientState_Connected;
		}
		else if ( appSeconds() - ConnectStartTime > 10. )
		{
			// Timeout after 10 seconds...
			DestroySockets();
		}
	}
}

/** Send data along the connection. */
UBOOL FDebugServer::FClientConnection::Send(const BYTE* Data,INT Count,INT& BytesSent)
{
	UBOOL bResult = FALSE;
	if ( IsConnected() )
	{
		check( SenderSocket );
		if ( bIsStreamingConnection )
		{
			bResult = SenderSocket->Send( Data, Count, BytesSent );
		}
		else
		{
			bResult = SenderSocket->SendTo( Data, Count, BytesSent, Address.GetSocketAddress() );
		}
	}
#if 0//!PS3
	if ( bResult )
	{
		// Log sent data to file.
		static FILE* DebugFile = NULL;
		char* TempBuf = new char[Count+1];
		appMemcpy( TempBuf, Data, Count );
		TempBuf[Count] = 0;
		const errno_t Result = fopen_s(&DebugFile, "DebugServer Sent Log.txt", !DebugFile ? "w" : "a");
		if ( Result == 0 && DebugFile != NULL )
		{
			fwrite( Data, 1, Count, DebugFile );
			fclose( DebugFile );
		}
		delete[] TempBuf;
	}
#endif
	return bResult;
}

/** Sends any data queued on the connection. */
void FDebugServer::FClientConnection::SendQueuedData()
{
	if ( IsConnected() )
	{
		for ( INT MsgIndex = 0 ; MsgIndex < QueuedData.Num() ; ++MsgIndex )
		{
			INT BytesSent = 0;
			if ( Send( QueuedData(MsgIndex).Data.GetTypedData(), QueuedData(MsgIndex).Data.Num(), BytesSent ) )
			{
				QueuedData.Remove(MsgIndex);
				--MsgIndex;
			}
			else
			{
				// If the send fails, stop trying to send queued data,
				// to ensure that queued data arrives in order.
				return;
			}
		}
	}
}

/*------------------------------ FDebugServer::FListenerRunnable ----------------------------------*/

UBOOL FDebugServer::FListenerRunnable::OutputDeviceAdded = FALSE;

/**
 * Copies the specified values to use for the listener thread
 *
 * @param InPort the port to listen for client requests on
 * @param InTrafficPort the port that will send on
 * @param InClients the list that clients will be added to/removed from
 */
FDebugServer::FListenerRunnable::FListenerRunnable(
	INT InPort,
	INT InTrafficPort,
	FClientList* InClients,
	FDebugServer* InDebugServer) :
	ListenPort(InPort),
	TrafficPort(InTrafficPort),
	TimeToDie(NULL),
	ClientList(InClients),
	ListenSocket(NULL),
	DebugServer(InDebugServer)
{
}

/**
 * Binds to the specified port
 *
 * @return True if initialization was successful, false otherwise
 */
UBOOL FDebugServer::FListenerRunnable::Init()
{
	// Create the event used to tell our threads to bail
	TimeToDie = GSynchronizeFactory->CreateSynchEvent(TRUE);
	UBOOL bOk = TimeToDie != NULL;
	if (bOk)
	{
		ListenAddr.SetPort(ListenPort);
		ListenAddr.SetIp(getlocalbindaddr(*GWarn));
		// Now create and set up our sockets (force UDP even when speciliazed
		// protocols exist)
		ListenSocket = GSocketSubsystem->CreateDGramSocket(TRUE);
		if (ListenSocket != NULL)
		{
			ListenSocket->SetReuseAddr();
			ListenSocket->SetNonBlocking();
			ListenSocket->SetRecvErr();
			// Bind to our listen port
			if (ListenSocket->Bind(ListenAddr))
			{
				INT SizeSet = 0;
				// Make the send buffer large so we don't overflow it
				bOk = ListenSocket->SetSendBufferSize(0x20000,SizeSet);
			}
		}
	}
	return bOk;
}

/**
 * Sends an announcement message on start up and periodically thereafter
 *
 * @return The exit code of the runnable object
 */
DWORD FDebugServer::FListenerRunnable::Run()
{
	BYTE PacketData[512];
	// Check every 1/2 second for a client request, while the death event
	// isn't signaled
	while (TimeToDie->Wait(500) == FALSE)
	{
		// Default to no data being read
		INT BytesRead = 0;
		if (ListenSocket != NULL)
		{
			FInternetIpAddr SockAddr;
			// Read from the socket and process if some was read
			ListenSocket->RecvFrom(PacketData,512,BytesRead,SockAddr);
			if (BytesRead > 0)
			{
				// Process this data
				ProcessPacket(SockAddr,PacketData,BytesRead);
			}
		}
	}
	return 0;
}

/**
 * Receives data from a given call to Poll(). For the beacon, we
 * just send an announcement directly to the source address
 *
 * @param SrcAddr the source address of the request
 * @param Data the packet data
 * @param Count the number of bytes in the packet
 */
void FDebugServer::FListenerRunnable::ProcessPacket(FIpAddr SrcAddr,
	BYTE* Data,INT Count)
{
	// Check for a "server announce" request
	if (Count == 2 && Data[0] == 'S' && Data[1] == 'A')
	{
		debugfSuppressed(NAME_DevStats,TEXT("Sending server announce to %s"),
			*SrcAddr.ToString(TRUE));
		// Build the packet containing the information the client needs
		FNboSerializeToBuffer QueryResponsePacket(DEFAULT_MAX_UDP_PACKET_SIZE);
		// Format is SR, computer name, game name, game type, platform type
		QueryResponsePacket << 'S' << 'R' << appComputerName() << GGameName <<
			(BYTE)appGetGameType() << (BYTE)appGetPlatformType() <<
			TrafficPort;
// @todo joeg - add a "requires password" option
		// Respond on the next port from what we are listening to
		SrcAddr.Port = ListenPort + 1;
		INT BytesSent;
		// Send our info to the client
		ListenSocket->SendTo(QueryResponsePacket,QueryResponsePacket.GetByteCount(),
			BytesSent,SrcAddr.GetSocketAddress());
	}

	// Check for a client connect request
	else if (Count == 2 && Data[0] == 'C' && Data[1] == 'C')
	{
		debugfSuppressed(NAME_DevStats,TEXT("Received client connect from %s"),
			*SrcAddr.ToString(TRUE));
		
		// Make sure debug output was added to log
		if (!OutputDeviceAdded)
		{
			// See if output device was added - it depends on whether the below conditions were met
			// (as specified in UnMisc.cpp::appInit(...), line 3919 as of June 20 2007)
			OutputDeviceAdded = CONSOLE || appIsDebuggerPresent();

			// If output device wasn't added, do it now
			if (!OutputDeviceAdded)
			{
				GLog->AddOutputDevice( new FOutputDeviceDebug() );
				OutputDeviceAdded = TRUE;
			}
		}

		// Decide whether or not to stream based on the platform.
		UE3::EPlatformType CurrentPlatform = appGetPlatformType();
		const UBOOL bStreamingConnection = (CurrentPlatform == UE3::PLATFORM_Windows);

		// Add this address to our update list.
		if (ClientList->AddClient(SrcAddr,bStreamingConnection))
		{
			// Notify listener
			OnClientConnected(SrcAddr);
		}
	}

	// Check for a client disconnect
	else if (Count == 2 && Data[0] == 'C' && Data[1] == 'D')
	{
		debugfSuppressed(NAME_DevStats,TEXT("Received client disconnect from %s"),
			*SrcAddr.ToString(TRUE));
		// Remove this address from our update list
		ClientList->RemoveClient(SrcAddr);
		// Notify listener
		OnClientDisconnected(SrcAddr);
	}

	// Check for client text being sent to us
	else if (Count >= 6 && Data[0] == 'C' && Data[1] == 'T' )
	{
		//@TODO - Do correct platform byteswapping
		INT Len = (DWORD(Data[2]) << 24) | (DWORD(Data[3]) << 16) | (DWORD(Data[4]) << 8) | DWORD(Data[5]);
		DebugServer->OnReceiveText(SrcAddr, (ANSICHAR*)(Data+6), Len );
	}

	else
	{
		// Log the unknown request type
		debugfSuppressed(NAME_DevStats,TEXT("Unknown request of size %d from %s"),Count,
			*SrcAddr.ToString(TRUE));
	}
}

void FDebugServer::OnReceiveText(const FIpAddr& SrcAddr, const ANSICHAR* Characters, INT Length)
{
	FScopeLock ScopeLock(CommandSync);
	if ( NumCommands < MAX_COMMANDS )
	{
		// Truncate if necessary
		if ( Length > (MAX_COMMANDLENGTH - 1) )
		{
			Length = MAX_COMMANDLENGTH - 1;
		}
		appStrncpy( Commands[NumCommands], ANSI_TO_TCHAR(Characters), Length + 1 );
		Commands[NumCommands][ Length ] = 0;	
		NumCommands++;
	}
}

/*------------------------------ FDebugServer::FSenderRunnable ----------------------------------*/

/**
 * Copies the specified values to use for the send thread
 *
 * @param InSync the critical section to use
 * @param InClients the list that clients will be added to/removed from
 */
FDebugServer::FSenderRunnable::FSenderRunnable(
	FCriticalSection* InSync,FClientList* InClients,FMultiThreadedRingBuffer* InLogBuffer) :
	bIsTimeToDie(FALSE),
	WorkEvent(NULL),
	AccessSync(InSync),
	ClientList(InClients),
	LogBuffer(InLogBuffer)
{
}

/**
 * Binds to the specified port
 *
 * @return True if initialization was successful, false otherwise
 */
UBOOL FDebugServer::FSenderRunnable::Init(void)
{
	// Create the event used to tell our threads to bail
	WorkEvent = GSynchronizeFactory->CreateSynchEvent();
	return WorkEvent != NULL;
}

/**
 * Sends an announcement message on start up and periodically thereafter
 *
 * @return The exit code of the runnable object
 */
DWORD FDebugServer::FSenderRunnable::Run(void)
{
	do
	{
		// Wait for there to be work to do
		WorkEvent->Wait();

		// Do the server specific job
		Tick();
	}
	while (bIsTimeToDie == FALSE);
	return 0;
}

void FDebugServer::FSenderRunnable::Tick()
{
	// See if there's new log data to be sent.
	FMultiThreadedRingBuffer::BufferData Data;
	Data.Buffer = NULL;
	Data.Size = 0;
	while (LogBuffer->Peek(Data))
	{
		// Send log to all connected clients.
		FClientConnection* Client = ClientList->BeginIterator();
		while (Client)
		{
			Client->PrimeConnection();
			if ( Client->IsConnected() )
			{
				UBOOL bShouldEnqueueData = TRUE;

				// Try to send the data if there's nothing enqueued.
				if ( Client->QueuedData.Num() == 0 )
				{
					INT BytesSent = 0;
					// If the data didn't send, possibly because there's no buffer space available within the transport system,
					// enqueue the data on the client connection so that it can be transmitted later.
					bShouldEnqueueData = ( Client->Send( Data.Buffer, Data.Size, BytesSent ) == FALSE );
				}

				if ( bShouldEnqueueData )
				{
					Client->EnqueueData(Data.Buffer, Data.Size);
				}
			}
			Client = ClientList->GetNext();
		}
		ClientList->EndIterator();

		// Remove log data from buffer.
		LogBuffer->Pop(Data);
	}

	// Try to send queued data.
	FClientConnection* Client = ClientList->BeginIterator();
	while (Client)
	{
		Client->SendQueuedData();
		Client = ClientList->GetNext();
	}
	ClientList->EndIterator();
}

/**
 * Cleans up any allocated resources
 */
void FDebugServer::FSenderRunnable::Exit(void)
{
	GSynchronizeFactory->Destroy(WorkEvent);
	WorkEvent = NULL;
	GSynchronizeFactory->Destroy(AccessSync);
	AccessSync = NULL;
}

void FDebugServer::FSenderRunnable::WakeUp()
{
	WorkEvent->Trigger();
}

/*------------------------------ FDebugServer ----------------------------------*/

FDebugServer::FDebugServer() :
	ListenerThread(NULL),
	ListenerRunnable(NULL),
	ClientList(NULL),
	SenderThread(NULL),
	SenderRunnable(NULL),
	LogBuffer(NULL),
	NumCommands(0),
	CommandSync(NULL)
{
}

/**
 * Initializes the threads that handle the network layer
 */
UBOOL FDebugServer::Init(INT ListenPort, INT TrafficPort, INT BufferSize, INT InMaxUDPPacketSize)
{
	// None of this code will work if encrypted packets are required so skip in this case
	if (GSocketSubsystem->RequiresEncryptedPackets())
	{
		return FALSE;
	}
	// Decide on max packet size based on platform.
	UE3::EPlatformType CurrentPlatform = appGetPlatformType();
	const UBOOL bStreamingConnection = (CurrentPlatform == UE3::PLATFORM_Windows);

	if ( bStreamingConnection )
	{
		MaxUDPPacketSize = InMaxUDPPacketSize;
	}
	else
	{
		MaxUDPPacketSize = BufferSize = DEFAULT_BUFFER_SIZE*4;
	}
	// Create cyclic buffer for messages that are to be sent
	LogBuffer = new FMultiThreadedRingBuffer(BufferSize, MaxUDPPacketSize);

	// Create command synchronizer
	CommandSync = GSynchronizeFactory->CreateCriticalSection();

	// Validate that the listen / response(equal to listen + 1) / traffic ports don't collide
	if (ListenPort + 1 == TrafficPort)
	{
		TrafficPort++;
		debugf(TEXT("FDebugServer::Init(): Port Collision: Changed traffic port to %d"),TrafficPort);
	}

	// Create the object that will manage our client connections
	ClientList = ::new FClientList(TrafficPort);

	// Create the listener object
	ListenerRunnable = CreateListenerRunnable(ListenPort, TrafficPort, ClientList);
	// Now create the thread that will do the work
	ListenerThread = GThreadFactory->CreateThread(ListenerRunnable,TEXT("DebugServerListener"),FALSE,FALSE,
		8 * 1024);
	if (ListenerThread != NULL)
	{
#if XBOX
		// See UnXenon.h
		//ListenerThread->SetProcessorAffinity(STATS_LISTENER_HWTHREAD);
#endif
		// Created externally so that it can be locked before the sender thread has
		// gone through its initialization
		FCriticalSection* AccessSync = GSynchronizeFactory->CreateCriticalSection();
		if (AccessSync != NULL)
		{
			// Now create the sender thread
			SenderRunnable = CreateSenderRunnable(AccessSync,ClientList);
			// Now create the thread that will do the work
			SenderThread = GThreadFactory->CreateThread(SenderRunnable,TEXT("DebugServerSender"),FALSE,FALSE,
				12 * 1024);
			if (SenderThread != NULL)
			{
#if XBOX
				// See UnXenon.h
				SenderThread->SetProcessorAffinity(STATS_SENDER_HWTHREAD);
#endif
			}
			else
			{
				debugf(NAME_Error,TEXT("Failed to create FDebugServer send thread"));
			}
		}
		else
		{
			debugf(NAME_Error,TEXT("Failed to create FDebugServer send thread"));
		}
	}
	else
	{
		debugf(NAME_Error,TEXT("Failed to create FDebugServer listener thread"));
	}
	return ListenerThread != NULL && SenderThread != NULL;
}

/**
 * Shuts down the network threads
 */
void FDebugServer::Destroy(void)
{
	// Tell the threads to stop running
	if (ListenerRunnable != NULL)
	{
		ListenerRunnable->Stop();
		ListenerThread->WaitForCompletion();
		GThreadFactory->Destroy(ListenerThread);
		delete ListenerRunnable;
		ListenerRunnable = NULL;
		ListenerThread = NULL;
	}
	if (SenderRunnable != NULL)
	{
		SenderRunnable->Stop();
		SenderThread->WaitForCompletion();
		GThreadFactory->Destroy(SenderThread);
		delete SenderRunnable;
		SenderRunnable = NULL;
		SenderThread = NULL;
	}
	// Destroy synch objects
	if (CommandSync != NULL)
	{
		GSynchronizeFactory->Destroy(CommandSync);
		CommandSync = NULL;
	}
	// Delete the clients class
	delete ClientList;
}

void FDebugServer::Tick()
{
	if (CommandSync)
	{
		// Execute all received commands
		FScopeLock ScopeLock(CommandSync);
		for ( INT Index=0; Index < NumCommands; ++Index )
		{
			new(GEngine->DeferredCommands) FString(Commands[Index]);
		}
		NumCommands = 0;

		// Wake up sender
		if (SenderRunnable)
		{
			SenderRunnable->WakeUp();
		}
	}
}

FDebugServer::FSenderRunnable* FDebugServer::CreateSenderRunnable(FCriticalSection* InSync, FClientList* InClients)
{
	return new FDebugServer::FSenderRunnable(InSync, InClients, LogBuffer);
}

FDebugServer::FListenerRunnable* FDebugServer::CreateListenerRunnable(INT InPort, INT InTrafficPort, FClientList* InClients)
{
	return new FDebugServer::FListenerRunnable(InPort, InTrafficPort, InClients, this);
}

void FDebugServer::SendText(TextChannel Channel, const ANSICHAR* Text)
{
	// Do not send text if there's no client connected
	if (!ClientList || ClientList->GetConnectionCount() == 0)
	{
		return;
	}

	// Handle case when packet size is greater than max allowed UDP packet size
	const INT PacketHeaderSize =
		2							// Message header padding
		+ sizeof(ANSICHAR) * 2		// Message header
		+ 4							// Channel enum
		+ sizeof(INT);				// TextLength
	const INT TextLength = strlen(Text);
	const INT PacketSize = PacketHeaderSize + TextLength;

	const INT TextPieceLength = MaxUDPPacketSize - PacketHeaderSize;

	INT NumMessages = TextLength / TextPieceLength;
	if (NumMessages * TextPieceLength < TextLength)
	{
		NumMessages++;
	}

	for (INT i = 0; i < NumMessages; i++)
	{
		const INT CurrentPieceLength = (i < NumMessages - 1) ?
			TextPieceLength :
			(TextLength - (NumMessages - 1) * TextPieceLength);
		SendText(Channel, Text + i * TextPieceLength, CurrentPieceLength);
	}
}

void FDebugServer::SendText(TextChannel Channel, const ANSICHAR* Text, const INT Length)
{
	static UBOOL bRescursiveGuard = FALSE;
	if ( bRescursiveGuard )
	{
		return;
	}
	bRescursiveGuard = TRUE;
	// Create packet
	FNboSerializeToBuffer Packet(MaxUDPPacketSize);
	const ANSICHAR* MessageHeader = ToString(EDebugServerMessageType_ServerTransmission);
	// DB: changed this to explicit cast to char because the int-cast version was calling through to the car version!
	//Packet << MessageHeader[0] << MessageHeader[1] << (INT)Channel;
	Packet << MessageHeader[0] << MessageHeader[1] << (char)Channel;
	Packet.AddString(Text, Length);

	// Attempt to obtain space in logs ring buffer for our packet
	INT AttemptsCounter = 10;
	FMultiThreadedRingBuffer::BufferData Data;
	while (!LogBuffer->BeginPush(Data, Packet.GetByteCount()))
	{
		AttemptsCounter--;
		if (AttemptsCounter == 0)
		{
			bRescursiveGuard = FALSE;
			return; // Drop the message
		}

		appSleep(0.001f);
	}

	// Copy the data to ring buffer
	memcpy(Data.Buffer, Packet.GetRawBuffer(0), Packet.GetByteCount());

	// Finalize operation
	LogBuffer->EndPush(Data);

	// Let's awake the sender thread
	if (SenderRunnable)
	{
		SenderRunnable->WakeUp();
	}
	bRescursiveGuard = FALSE;
}

#endif // !SHIPPING_PC_GAME
#endif // WITH_UE3_NETWORKING
