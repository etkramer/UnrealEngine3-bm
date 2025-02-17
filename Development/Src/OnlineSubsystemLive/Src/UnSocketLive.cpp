/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "OnlineSubsystemLive.h"

#if WITH_UE3_NETWORKING

FSocketSubsystemLive SocketSubsystem;

#if !WITH_PANORAMA
/** Starts up the socket subsystem */
void appSocketInit(void)
{
	GSocketSubsystem = &SocketSubsystem;
	FString Error;
	if (GSocketSubsystem->Initialize(Error) == FALSE)
	{
		debugf(NAME_Init,TEXT("Failed to initialize socket subsystem: (%s)"),*Error);
	}
}
#endif

/**
 * LSP specific resolve info object. Resolves a host by service ID
 */
class FResolveInfoLsp :
	public FResolveInfo,
	public FQueuedWork
{
	/** The host name to resolve */
	ANSICHAR HostName[256];
	/** Caches the service id for secure address resolution */
	DWORD ServiceId;
	/** The handle of the enumeration task */
	HANDLE LspEnumerateHandle;
	/** The buffer that is used to read into */
	BYTE* EnumBuffer;
	/** Whether the async task is done or not */
	UBOOL bIsDone;
	/** The result of the host resolve */
	INT ErrorCode;
	/** The cached ip address */
	FInternetIpAddr Addr;
	/** Read 16 ips at a time */
	static const DWORD NumLspsPerEnum = 16;

public:
	/**
	 * Forwards the host name to the base class and zeros the handle and buffer
	 *
	 * @param InHostName the name of the host being resolved
	 * @param InServiceId the service id of the LSP in question
	 */
	FResolveInfoLsp(const ANSICHAR* InHostName,DWORD InServiceId) :
		ServiceId(InServiceId),
		LspEnumerateHandle(NULL),
		EnumBuffer(NULL),
		bIsDone(FALSE),
		ErrorCode(SE_NO_ERROR)
	{
		appStrncpyANSI(HostName,InHostName,256);
	}

	/** Cleans up any resources */
	virtual ~FResolveInfoLsp(void)
	{
		if (LspEnumerateHandle)
		{
			XCloseHandle(LspEnumerateHandle);
		}
		delete [] EnumBuffer;
	}

	/**
	 * Resolves the specified host name using the XTitleServerCreateEnumerator
	 */
	virtual void DoWork(void)
	{
		Addr.SetIp(0);
		DWORD BufferSize = 0;
		// Ask for how large the buffer will be
	    DWORD Result = XTitleServerCreateEnumerator(HostName,
			NumLspsPerEnum,
			&BufferSize,
			&LspEnumerateHandle);
		debugf(NAME_DevOnline,
			TEXT("XTitleServerCreateEnumerator(%s,%d) returned 0x%08X"),
			ANSI_TO_TCHAR(HostName),
			BufferSize,
			Result);
		if (Result == ERROR_SUCCESS)
		{
			DWORD ItemsReturned = NumLspsPerEnum;
			EnumBuffer = new BYTE[BufferSize];
			// Have it read the next (only) item in the list
			Result = XEnumerate(LspEnumerateHandle,
				EnumBuffer,
				BufferSize,
				&ItemsReturned,
				NULL);
			debugf(NAME_DevOnline,
				TEXT("XEnumerate(%s,%d,%d) returned 0x%08X"),
				ANSI_TO_TCHAR(HostName),
				BufferSize,
				ItemsReturned,
				Result);
			if (Result == ERROR_SUCCESS)
			{
				if (ItemsReturned > 0)
				{
					DWORD Rand;
					XNetRandom((BYTE*)&Rand,sizeof(DWORD));
					// Choose a random one to use as our server ip
					DWORD ServerIndex = Rand % ItemsReturned;
					debugf(NAME_DevOnline,TEXT("Choosing random SG %d out of %d"),ServerIndex,ItemsReturned);
					XTITLE_SERVER_INFO* ServerInfo = (XTITLE_SERVER_INFO*)EnumBuffer;
					// Copy the address
					Addr.SetIp(ServerInfo[ServerIndex].inaServer);
					// Use the secure address if required 
					if (GSocketSubsystem->RequiresEncryptedPackets())
					{
						IN_ADDR SecureAddr;
						appMemzero(&SecureAddr,sizeof(IN_ADDR));
						// Convert the address from the non-secure to secure
						Result = XNetServerToInAddr(ServerInfo[ServerIndex].inaServer,ServiceId,&SecureAddr);
						debugf(NAME_DevOnline,
							TEXT("XNetServerToInAddr(%s,%d) returned 0x%08X"),
							*Addr.ToString(FALSE),
							ServiceId,
							Result);
						Addr.SetIp(SecureAddr);
					}
					else
					{
						Result = ERROR_SUCCESS;
					}
					debugf(NAME_DevOnline,TEXT("Resolved (%s) to %s"),ANSI_TO_TCHAR(HostName),*Addr.ToString(FALSE));
				}
				else
				{
					Result = E_FAIL;
					debugf(NAME_DevOnline,TEXT("XEnumerate() returned %d items"),ItemsReturned);
				}
			}
			else
			{
				// Failed to resolve
				Result = E_FAIL;
			}
			// Clean up the handles/memory
			XCloseHandle(LspEnumerateHandle);
			LspEnumerateHandle = NULL;
			delete [] EnumBuffer;
			EnumBuffer = NULL;
		}
		else
		{
			// Failed to resolve
			Result = E_FAIL;
		}
		WSASetLastError(SE_HOST_NOT_FOUND);
		ErrorCode = Result == ERROR_SUCCESS ? SE_NO_ERROR : SE_HOST_NOT_FOUND;
	}

	/**
	 * Tells the consumer thread that the resolve is done
	 */ 
	virtual void Dispose(void)
	{
		if (ErrorCode == SE_NO_ERROR)
		{
			// Cache for reuse
			GSocketSubsystem->AddHostNameToCache(HostName,Addr);
		}
		// Atomicly update our "done" flag
		appInterlockedExchange((INT*)&bIsDone,TRUE);
	}

	/**
	 * Tells the thread to quit trying to resolve
	 */
	virtual void Abandon(void)
	{
	}

	/**
	 * Start the async work and perform it synchronously if no thread pool is available
	 */
	void StartAsyncTask(void)
	{
		if (GThreadPool != NULL)
		{
			// Queue this to our worker thread(s) for resolving
			GThreadPool->AddQueuedWork(this);
		}
		else
		{
			DoWork();
			Dispose();
		}
	}

// FResolveInfo interface

	/**
	 * Whether the async process has completed or not
	 *
	 * @return true if it completed successfully, false otherwise
	 */
	virtual UBOOL IsComplete(void) const
	{
		return bIsDone;
	}

	/**
	 * The error that occured when trying to resolve
	 *
	 * @return error code from the operation
	 */
	virtual INT GetErrorCode(void) const
	{
		return ErrorCode;
	}

	/**
	 * Returns a copy of the resolved address
	 *
	 * @return the resolved IP address
	 */
	virtual FInternetIpAddr GetResolvedAddress(void) const
	{
		return Addr;
	}
};

/**
 * Does Xenon platform initialization of the sockets library
 *
 * @param Error a string that is filled with error information
 *
 * @return TRUE if initialized ok, FALSE otherwise
 */
UBOOL FSocketSubsystemLive::Initialize(FString& Error)
{
	if (bTriedToInit == FALSE)
	{
		bTriedToInit = TRUE;
		// Determine if we should use VDP or not
#if	!FINAL_RELEASE || FINAL_RELEASE_DEBUGCONSOLE
		if (GConfig != NULL)
		{
			// Figure out if VDP is enabled or not
			GConfig->GetBool(SOCKET_API,TEXT("bUseVDP"),bUseVDP,GEngineIni);
			// Figure out if secure connection is enabled or not
			GConfig->GetBool(SOCKET_API,TEXT("bUseSecureConnections"),bUseSecureConnections,GEngineIni);
			// Figure out if LSP enumeration is enabled or not
			GConfig->GetBool(SOCKET_API,TEXT("bUseLspEnumerate"),bUseLspEnumerate,GEngineIni);
			// Allow disabling secure connections via commandline override.
			if( ParseParam(appCmdLine(), TEXT("DEVCON")) )
			{
				bUseSecureConnections = FALSE;
			}
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
#if	(!FINAL_RELEASE && !WITH_PANORAMA) || FINAL_RELEASE_DEBUGCONSOLE // The final library should enforce this, but to be sure...
		if (bUseSecureConnections == FALSE)
		{
			XNParams.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
		}
#else
		#pragma message("Using secure socket layer")
		#pragma message("This means a Live client cannot talk to a non-Live server/client that isn't using LSP")
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
			// Read the service id to use for LSP access
			GConfig->GetInt(SOCKET_API,TEXT("ServiceId"),ServiceId,GEngineIni);
		}
		// Do the Xenon preinitialization
		INT ErrorCode = XNetStartup(&XNParams);
		if (ErrorCode == 0)
		{
			WSADATA WSAData;
			// Init WinSock
			if (WSAStartup(0x0101,&WSAData) == 0)
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
			Error = FString::Printf(TEXT("XNetStartup() failed with 0x%X"),GetLastError());
			debugf(NAME_Init,*Error);
		}
	}
	return GIpDrvInitialized;
}

/**
 * Performs Xenon & Live specific socket subsystem clean up
 */
void FSocketSubsystemLive::Destroy(void)
{
	// Shut Live down first
	XOnlineCleanup();
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
FSocket* FSocketSubsystemLive::CreateDGramSocket(UBOOL bForceUDP)
{
	INT Protocol = bUseVDP == TRUE && bForceUDP == FALSE ? IPPROTO_VDP : IPPROTO_UDP;
	// Create a socket with the configured protocol support
	SOCKET Socket = socket(AF_INET,SOCK_DGRAM,Protocol);
	return Socket != INVALID_SOCKET ? new FSocketWin(Socket,SOCKTYPE_Datagram) : NULL;
}

/**
 * Does a DNS look up of a host name
 *
 * @param HostName the name of the host to look up
 * @param Addr the address to copy the IP address to
 */
INT FSocketSubsystemLive::GetHostByName(ANSICHAR* HostName,FInternetIpAddr& Addr)
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
		else
		{
			ErrorCode = Xnds->iStatus;
		}
		// Free the dns structure
		XNetDnsRelease(Xnds);
	}
	return ErrorCode;
}

/**
 * Creates a platform specific async hostname resolution object
 *
 * @param HostName the name of the host to look up
 *
 * @return the resolve info to query for the address
 */
FResolveInfo* FSocketSubsystemLive::GetHostByName(ANSICHAR* HostName)
{
#if !FINAL_RELEASE
	if (bUseLspEnumerate)
#endif
	{
		FInternetIpAddr Addr;
		// See if we have it cached or not
		if (GetHostByNameFromCache(HostName,Addr))
		{
			return new FResolveInfoCached(Addr);
		}
		else
		{
			FResolveInfoLsp* AsyncResolve = new FResolveInfoLsp(HostName,ServiceId);
			AsyncResolve->StartAsyncTask();
			return AsyncResolve;
		}
	}
	return FSocketSubsystem::GetHostByName(HostName);
}

/**
 * Determines the name of the local machine
 *
 * @param HostName the string that receives the data
 *
 * @return TRUE if successful, FALSE otherwise
 */
UBOOL FSocketSubsystemLive::GetHostName(FString& HostName)
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
UBOOL FSocketSubsystemLive::GetLocalHostAddr(FOutputDevice& Out,
	FInternetIpAddr& HostAddr)
{
	XNADDR XnAddr;
	appMemzero(&XnAddr,sizeof(XNADDR));
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
