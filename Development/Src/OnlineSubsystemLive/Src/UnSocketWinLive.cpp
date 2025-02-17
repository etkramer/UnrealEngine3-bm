/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "OnlineSubsystemLive.h"
#include "UnSocketWinLive.h"

#if WITH_UE3_NETWORKING

#if WITH_PANORAMA

FSocketSubsystemWindowsLive SocketSubsystem;
FSocketSubsystemWindows FallbackSocketSubsystem;

/** Starts up the socket subsystem */
void appSocketInit(void)
{
	UBOOL bUseSecureConnections = FALSE;
	if (GConfig != NULL)
	{
		// Figure out if secure connection is enabled or not
		GConfig->GetBool(SOCKET_API,TEXT("bUseSecureConnections"),bUseSecureConnections,GEngineIni);
	}
#if FINAL_RELEASE || SHIPPING_PC_GAME
	// Always use the secure libs in final release
	bUseSecureConnections = TRUE;
#endif
	// Always use the fallback if in the editor or if secure connections are off,
	// otherwise use the secure layer
	if (!GIsEditor && bUseSecureConnections)
	{
		GSocketSubsystem = &SocketSubsystem;
	}
	else
	{
		GSocketSubsystem = &FallbackSocketSubsystem;
		debugf(NAME_Init,TEXT("Using the non-secure socket subsystem"));
	}
	extern UBOOL GIsUsingParorama;
	// Skip socket initialization if we need the secure lib and Live hasn't
	// been initialized yet
	if ((GSocketSubsystem == &SocketSubsystem && GIsUsingParorama) ||
		(GSocketSubsystem == &FallbackSocketSubsystem))
	{
		FString Error;
		if (GSocketSubsystem->Initialize(Error) == FALSE)
		{
			debugf(NAME_Init,TEXT("Failed to initialize socket subsystem: (%s)"),*Error);
		}
	}
}

#endif

/**
 * Closes the socket
 *
 * @param TRUE if it closes without errors, FALSE otherwise
 */
UBOOL FSocketWinLive::Close(void)
{
	return XSocketClose(Socket) == 0;
}

/**
 * Binds a socket to a network byte ordered address
 *
 * @param Addr the address to bind to
 *
 * @return TRUE if successful, FALSE otherwise
 */
UBOOL FSocketWinLive::Bind(const FInternetIpAddr& Addr)
{
	return XSocketBind(Socket,Addr,sizeof(SOCKADDR_IN)) == 0;
}

/**
 * Connects a socket to a network byte ordered address
 *
 * @param Addr the address to connect to
 *
 * @return TRUE if successful, FALSE otherwise
 */
UBOOL FSocketWinLive::Connect(const FInternetIpAddr& Addr)
{
	INT Return = XSocketConnect(Socket,Addr,sizeof(SOCKADDR_IN));
	return Return == 0 || Return == WSAEWOULDBLOCK;
}

/**
 * Places the socket into a state to listen for incoming connections
 *
 * @param MaxBacklog the number of connections to queue before refusing them
 *
 * @return TRUE if successful, FALSE otherwise
 */
UBOOL FSocketWinLive::Listen(INT MaxBacklog)
{
	return XSocketListen(Socket,MaxBacklog) == 0;
}

/**
 * Queries the socket to determine if there is a pending connection
 *
 * @param bHasPendingConnection out parameter indicating whether a connection is pending or not
 *
 * @return TRUE if successful, FALSE otherwise
 */
UBOOL FSocketWinLive::HasPendingConnection(UBOOL& bHasPendingConnection)
{
	UBOOL bHasSucceeded = FALSE;
	bHasPendingConnection = FALSE;
	// Check and return without waiting
	TIMEVAL Time = {0,0};
	fd_set SocketSet;
	// Set up the socket sets we are interested in (just this one)
	FD_ZERO(&SocketSet);
	FD_SET(Socket,&SocketSet);
	// Check the status of the bits. First check for errors
	INT SelectStatus = XSocketSelect(0,NULL,NULL,&SocketSet,&Time);
	if (SelectStatus == 0)
	{
		FD_ZERO(&SocketSet);
		FD_SET(Socket,&SocketSet);
		// Now check to see if it has a pending connection
		SelectStatus = XSocketSelect(0,&SocketSet,NULL,NULL,&Time);
		// One or more means there is a pending connection
		bHasPendingConnection = SelectStatus > 0;
		// Non negative means it worked
		bHasSucceeded = SelectStatus >= 0;
	}
	return bHasSucceeded;
}

/**
 * Accepts a connection that is pending
 *
 * @return The new (heap-allocated) socket, or NULL if unsuccessful.
 */
FSocket* FSocketWinLive::Accept(void)
{
	SOCKET NewSocket = XSocketAccept(Socket,NULL,NULL);
	if ( NewSocket != INVALID_SOCKET )
	{
		return new FSocketWinLive( NewSocket, SocketType );
	}
	return NULL;
}

/**
 * Accepts a connection that is pending
 *
 * @param OutAddr the address of the connection
 *
 * @return TRUE if successful, FALSE otherwise
 */
FSocket* FSocketWinLive::Accept(FInternetIpAddr& OutAddr)
{
	/*
	INT SizeOf = sizeof(SOCKADDR_IN);
	return XSocketAccept(Socket,OutAddr,&SizeOf) == 0;
	*/
	INT SizeOf = sizeof(SOCKADDR_IN);
	SOCKET NewSocket = XSocketAccept(Socket,OutAddr,&SizeOf);
	if ( NewSocket != INVALID_SOCKET )
	{
		return new FSocketWinLive( NewSocket, SocketType );
	}
	return NULL;
}

/**
 * Sends a buffer to a network byte ordered address
 *
 * @param Data the buffer to send
 * @param Count the size of the data to send
 * @param BytesSent out param indicating how much was sent
 * @param Destination the network byte ordered address to send to
 */
UBOOL FSocketWinLive::SendTo(const BYTE* Data,INT Count,INT& BytesSent,
	const FInternetIpAddr& Destination)
{
	// Write the data and see how much was written
	BytesSent = XSocketSendTo(Socket,(const char*)Data,Count,0,Destination,sizeof(SOCKADDR_IN));
	return BytesSent >= 0;
}

/**
 * Sends a buffer on a connected socket
 *
 * @param Data the buffer to send
 * @param Count the size of the data to send
 * @param BytesSent out param indicating how much was sent
 */
UBOOL FSocketWinLive::Send(const BYTE* Data,INT Count,INT& BytesSent)
{
	BytesSent = XSocketSend(Socket,(const char*)Data,Count,0);
	return BytesSent >= 0;
}

/**
 * Reads a chunk of data from the socket. Gathers the source address too
 *
 * @param Data the buffer to read into
 * @param BufferSize the max size of the buffer
 * @param BytesRead out param indicating how many bytes were read from the socket
 * @param Source out param receiving the address of the sender of the data
 */
UBOOL FSocketWinLive::RecvFrom(BYTE* Data,INT BufferSize,INT& BytesRead,
	FInternetIpAddr& Source)
{
	INT Size = sizeof(SOCKADDR_IN);
	// Read into the buffer and set the source address
	BytesRead = XSocketRecvFrom(Socket,(char*)Data,BufferSize,0,Source,&Size);
	return BytesRead >= 0;
}

/**
 * Reads a chunk of data from a connected socket
 *
 * @param Data the buffer to read into
 * @param BufferSize the max size of the buffer
 * @param BytesRead out param indicating how many bytes were read from the socket
 */
UBOOL FSocketWinLive::Recv(BYTE* Data,INT BufferSize,INT& BytesRead)
{
	BytesRead = XSocketRecv(Socket,(char*)Data,BufferSize,0);
	return BytesRead >= 0;
}

/**
 * Determines the connection state of the socket
 */
ESocketConnectionState FSocketWinLive::GetConnectionState(void)
{
	ESocketConnectionState CurrentState = SCS_ConnectionError;
	// Check and return without waiting
	TIMEVAL Time = {0,0};
	fd_set SocketSet;
	// Set up the socket sets we are interested in (just this one)
	FD_ZERO(&SocketSet);
	FD_SET(Socket,&SocketSet);
	// Check the status of the bits. First check for errors
	INT SelectStatus = XSocketSelect(0,NULL,NULL,&SocketSet,&Time);
	if (SelectStatus == 0)
	{
		FD_ZERO(&SocketSet);
		FD_SET(Socket,&SocketSet);
		// Now check to see if it's connected (writable means connected)
		SelectStatus = XSocketSelect(0,NULL,&SocketSet,NULL,&Time);
		if (SelectStatus > 0)
		{
			CurrentState = SCS_Connected;
		}
		// Zero means it is still pending
		if (SelectStatus == 0)
		{
			CurrentState = SCS_NotConnected;
		}
	}
	return CurrentState;
}

/**
 * Reads the address the socket is bound to and returns it
 */
FInternetIpAddr FSocketWinLive::GetAddress(void)
{
	FInternetIpAddr Addr;
	SOCKLEN Size = sizeof(SOCKADDR_IN);
	// Figure out what ip/port we are bound to
	UBOOL bOk = XSocketGetSockName(Socket,Addr,&Size) == 0;
	if (bOk == FALSE)
	{
		debugf(NAME_Error,TEXT("Failed to read address for socket (%d)"),
			GSocketSubsystem->GetSocketError());
	}
	return Addr;
}

/**
 * Sets this socket into non-blocking mode
 *
 * @param bIsNonBlocking whether to enable broadcast or not
 *
 * @return TRUE if successful, FALSE otherwise
 */
UBOOL FSocketWinLive::SetNonBlocking(UBOOL bIsNonBlocking)
{
	DWORD Value = bIsNonBlocking ? TRUE : FALSE;
	return XSocketIOCTLSocket(Socket,FIONBIO,&Value) == 0;
}

/**
 * Sets a socket into broadcast mode (UDP only)
 *
 * @param bAllowBroadcast whether to enable broadcast or not
 *
 * @return TRUE if successful, FALSE otherwise
 */
UBOOL FSocketWinLive::SetBroadcast(UBOOL bAllowBroadcast)
{
	return XSocketSetSockOpt(Socket,SOL_SOCKET,SO_BROADCAST,(char*)&bAllowBroadcast,sizeof(UBOOL)) == 0;
}

/**
 * Sets whether a socket can be bound to an address in use
 *
 * @param bAllowReuse whether to allow reuse or not
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
UBOOL FSocketWinLive::SetReuseAddr(UBOOL bAllowReuse)
{
	return XSocketSetSockOpt(Socket,SOL_SOCKET,SO_REUSEADDR,(char*)&bAllowReuse,sizeof(UBOOL)) == 0;
}

/**
 * Sets whether and how long a socket will linger after closing
 *
 * @param bShouldLinger whether to have the socket remain open for a time period after closing or not
 * @param Timeout the amount of time to linger before closing
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
UBOOL FSocketWinLive::SetLinger(UBOOL bShouldLinger,INT Timeout)
{
	LINGER ling;
	ling.l_onoff = bShouldLinger;
	ling.l_linger = Timeout;
	return XSocketSetSockOpt(Socket,SOL_SOCKET,SO_LINGER,(char*)&ling,sizeof(ling)) == 0;
}

/**
 * Enables error queue support for the socket
 *
 * @param bUseErrorQueue whether to enable error queueing or not
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
UBOOL FSocketWinLive::SetRecvErr(UBOOL bUseErrorQueue)
{
	// Not supported, but return true to avoid spurious log messages
	return TRUE;
}

/**
 * Sets the size of the send buffer to use
 *
 * @param Size the size to change it to
 * @param NewSize the out value returning the size that was set (in case OS can't set that)
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
UBOOL FSocketWinLive::SetSendBufferSize(INT Size,INT& NewSize)
{
	INT SizeSize = sizeof(INT);
	UBOOL bOk = XSocketSetSockOpt(Socket,SOL_SOCKET,SO_SNDBUF,(char*)&Size,sizeof(INT)) == 0;
	// Read the value back in case the size was modified
	XSocketGetSockOpt(Socket,SOL_SOCKET,SO_SNDBUF,(char*)&NewSize,GCC_OPT_INT_CAST &SizeSize);
	return bOk;
}

/**
 * Sets the size of the receive buffer to use
 *
 * @param Size the size to change it to
 * @param NewSize the out value returning the size that was set (in case OS can't set that)
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
UBOOL FSocketWinLive::SetReceiveBufferSize(INT Size,INT& NewSize)
{
	INT SizeSize = sizeof(INT);
	UBOOL bOk = XSocketSetSockOpt(Socket,SOL_SOCKET,SO_RCVBUF,(char*)&Size,sizeof(INT)) == 0;
	// Read the value back in case the size was modified
	XSocketGetSockOpt(Socket,SOL_SOCKET,SO_RCVBUF,(char*)&NewSize,GCC_OPT_INT_CAST &SizeSize);
	return bOk;
}

/**
 * Does Xenon platform initialization of the sockets library
 *
 * @param Error a string that is filled with error information
 *
 * @return TRUE if initialized ok, FALSE otherwise
 */
UBOOL FSocketSubsystemWindowsLive::Initialize(FString& Error)
{
	if (bTriedToInit == FALSE)
	{
		bTriedToInit = TRUE;
		// Determine if we should use VDP or not
#if	!FINAL_RELEASE && !WITH_PANORAMA
		if (GConfig != NULL)
		{
			// Figure out if VDP is enabled or not
			GConfig->GetBool(SOCKET_API,TEXT("bUseVDP"),bUseVDP,GEngineIni);
			// Figure out if secure connection is enabled or not
			GConfig->GetBool(SOCKET_API,TEXT("bUseSecureConnections"),bUseSecureConnections,GEngineIni);
		}
#else
		#pragma message("Forcing VDP packets to on")
		// Force encryption and voice packets being appended without it
		bUseVDP = TRUE;
		bUseSecureConnections = TRUE;
#endif
		XNetStartupParams XNParams;
		appMemzero(&XNParams,sizeof(XNetStartupParams));
		XNParams.cfgSizeOfStruct = sizeof(XNetStartupParams);
#if !WITH_PANORAMA
#if	!FINAL_RELEASE
		// The final library should enforce this, but to be sure...
		if (bUseSecureConnections == FALSE)
		{
			XNParams.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
		}
#else
		#pragma message("Using secure socket layer")
		#pragma message("This means a Xe client cannot talk to a PC that isn't using LSP")
#endif
#endif
		// Don't cause runtime allocations
		XNParams.cfgFlags |= XNET_STARTUP_ALLOCATE_MAX_DGRAM_SOCKETS |
			XNET_STARTUP_ALLOCATE_MAX_STREAM_SOCKETS;
		if (GConfig != NULL)
		{
			// Read the values from the INI file
			INT Value;
			if (GConfig->GetInt(SOCKET_API,TEXT("MaxDgramSockets"),Value,GEngineIni))
			{
				XNParams.cfgSockMaxDgramSockets = (BYTE)Clamp(Value,0,256);
			}
			if (GConfig->GetInt(SOCKET_API,TEXT("MaxStreamSockets"),Value,GEngineIni))
			{
				XNParams.cfgSockMaxStreamSockets = (BYTE)Clamp(Value,0,256);
			}
			if (GConfig->GetInt(SOCKET_API,TEXT("DefaultRecvBufsizeInK"),Value,GEngineIni))
			{
				XNParams.cfgSockDefaultRecvBufsizeInK = (BYTE)Clamp(Value,0,256);
			}
			if (GConfig->GetInt(SOCKET_API,TEXT("DefaultSendBufsizeInK"),Value,GEngineIni))
			{
				XNParams.cfgSockDefaultSendBufsizeInK = (BYTE)Clamp(Value,0,256);
			}
		}
		// Do the Live secure layer preinitialization
		// NOTE: This will fail if XLiveInitialize() fails or wasn't called first
		INT ErrorCode = XNetStartup(&XNParams);
		if (ErrorCode == 0)
		{
			WSADATA WSAData;
			// Init WinSock
			if (WSAStartup(0x0202,&WSAData) == 0)
			{
				// Now init Live
				DWORD LiveInit = XOnlineStartup();
				if (LiveInit == ERROR_SUCCESS)
				{
					GIpDrvInitialized = TRUE;
					debugf(NAME_Init,
						TEXT("%s: version %i.%i (%i.%i), MaxSocks=%i, MaxUdp=%i"),
						SOCKET_API,
						WSAData.wVersion >> 8,WSAData.wVersion & 0xFF,
						WSAData.wHighVersion >> 8,WSAData.wHighVersion & 0xFF,
						WSAData.iMaxSockets,WSAData.iMaxUdpDg);
				}
				else
				{
					Error = FString::Printf(TEXT("XOnlineStartup() failed with 0x%X"),LiveInit);
					debugf(NAME_Init,*Error);
				}
			}
		}
		else
		{
			// Couldn't use fallback so log why
			Error = FString::Printf(TEXT("XNetStartup() failed with 0x%08X (0x%08X)"),
				ErrorCode,
				GetLastErrorCode());
			debugf(NAME_Init,*Error);
		}
	}
	return GIpDrvInitialized;
}

/**
 * Performs Windows Live specific socket subsystem clean up
 */
void FSocketSubsystemWindowsLive::Destroy(void)
{
	// Shut Live down first
	XOnlineCleanup();
#if WITH_PANORAMA
	extern void appPanoramaHookDeviceDestroyed();
	extern void appPanoramaHookUninitialize(void);
	// Destroy rendering related resources
	appPanoramaHookDeviceDestroyed();
	//Uninitialize Panorama
	appPanoramaHookUninitialize();
#endif
	// Then the sockets layer
	WSACleanup();
	// Finally the secure layer
	XNetCleanup();
	GIpDrvInitialized = FALSE;
}

/**
 * Creates a data gram socket. Creates either a UDP or VDP socket based
 * upon the INI configuration
 *
 * @param bForceUDP overrides any platform specific protocol with UDP instead
 *
 * @return the new socket or NULL if failed
 */
FSocket* FSocketSubsystemWindowsLive::CreateDGramSocket(UBOOL bForceUDP)
{
	if (GIpDrvInitialized)
	{
		INT Protocol = bUseVDP == TRUE && bForceUDP == FALSE ? IPPROTO_VDP : IPPROTO_UDP;
		// Create a socket with the configured protocol support
		SOCKET Socket = XSocketCreate(AF_INET,SOCK_DGRAM,Protocol);
		return Socket != INVALID_SOCKET ? new FSocketWinLive(Socket,SOCKTYPE_Datagram) : NULL;
	}
	return NULL;
}

/**
 * Creates a stream (TCP) socket
 *
 * @return the new socket or NULL if failed
 */
FSocket* FSocketSubsystemWindowsLive::CreateStreamSocket(void)
{
	if (GIpDrvInitialized)
	{
		SOCKET Socket = XSocketCreate(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		return Socket != INVALID_SOCKET ? new FSocketWinLive(Socket,SOCKTYPE_Streaming) : NULL;
	}
	return NULL;
}

/**
 * Does a DNS look up of a host name
 *
 * @param HostName the name of the host to look up
 * @param Addr the address to copy the IP address to
 */
INT FSocketSubsystemWindowsLive::GetHostByName(ANSICHAR* HostName,FInternetIpAddr& Addr)
{
	Addr.SetIp(0);
	XNDNS* Xnds = NULL;
	// Kick off a DNS lookup. This is non-blocking
	INT ErrorCode = XNetDnsLookup(HostName,NULL,&Xnds);
	if (ErrorCode == 0 && Xnds != NULL)
	{
		// While we are waiting for the results to come back, sleep to let
		// other threads run
		while (Xnds->iStatus == WSAEINPROGRESS)
		{
			appSleep(0.1f);
		}
		// It's done with the look up, so validate the results
		if (Xnds->iStatus == 0)
		{
			// Make sure it found some entries
			if (Xnds->cina > 0)
			{
				// Copy the address
				Addr.SetIp(Xnds->aina[0]);
			}
			else
			{
				ErrorCode = WSAHOST_NOT_FOUND;
			}
		}
		// Free the dns structure
		XNetDnsRelease(Xnds);
	}
	return ErrorCode;
}

/**
 * Determines the name of the local machine
 *
 * @param HostName the string that receives the data
 *
 * @return TRUE if successful, FALSE otherwise
 */
UBOOL FSocketSubsystemWindowsLive::GetHostName(FString& HostName)
{
	HostName = appComputerName();
	return TRUE;
}

/**
 * Uses the secure libs to look up the host address
 *
 * @param Out the output device to log messages to
 * @param HostAddr the out param receiving the host address
 *
 * @return always TRUE
 */
UBOOL FSocketSubsystemWindowsLive::GetLocalHostAddr(FOutputDevice& Out,
	FInternetIpAddr& HostAddr)
{
	XNADDR XnAddr = {0};
	// Loop until a response comes back
	while (XNetGetTitleXnAddr(&XnAddr) == XNET_GET_XNADDR_PENDING)
	{
		appSleep(0.1f);
	}
	HostAddr.SetIp(XnAddr.ina);
	static UBOOL First = TRUE;
	if (First)
	{
		First = FALSE;
		debugf(NAME_Init, TEXT("%s: I am %s (%s)"), SOCKET_API, appComputerName(), *HostAddr.ToString(TRUE) );
	}
	return TRUE;
}

#endif	//#if WITH_UE3_NETWORKING
