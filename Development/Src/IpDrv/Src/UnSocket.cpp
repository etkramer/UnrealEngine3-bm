/*============================================================================
	UnSocket.cpp: Common interface for WinSock and BSD sockets.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
============================================================================*/

#include "UnIpDrv.h"

#if WITH_UE3_NETWORKING

/** The global socket subsystem pointer */
FSocketSubsystem* GSocketSubsystem = NULL;

//
// Class for creating a background thread to resolve a host.
//
class FResolveInfoAsync :
	public FResolveInfo,
	public FQueuedWork
{
	// Variables.
	FInternetIpAddr		Addr;
	ANSICHAR	HostName[256];
	/** Error code returned by GetHostByName. */
	INT			ErrorCode;
	UBOOL		bIsDone;
	/** Tells the worker thread whether it should abandon it's work or not */
	INT bShouldAbandon;

public:
	/**
	 * Copies the host name for async resolution
	 *
	 * @param InHostName the host name to resolve
	 */
	FResolveInfoAsync(const ANSICHAR* InHostName) :
		ErrorCode(SE_NO_ERROR),
		bIsDone(FALSE),
		bShouldAbandon(FALSE)
	{
		appStrncpyANSI(HostName,InHostName,256);
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

//FQueuedWork interface

	/**
	 * Resolves the specified host name
	 */
	virtual void DoWork(void)
	{
		Addr.SetIp(0);
		INT AttemptCount = 0;
		// Make up to 3 attempts to resolve it
		do 
		{
			ErrorCode = GSocketSubsystem->GetHostByName(HostName,Addr);
			if (ErrorCode != SE_NO_ERROR)
			{
				if (ErrorCode == SE_HOST_NOT_FOUND || ErrorCode == SE_NO_DATA || ErrorCode == SE_ETIMEDOUT)
				{
					// Force a failure
					AttemptCount = 3;
				}
			}
		}
		while (ErrorCode != SE_NO_ERROR && AttemptCount < 3 && bShouldAbandon == FALSE);
	}

	/**
	 * Tells the thread to quit trying to resolve
	 */
	virtual void Abandon(void)
	{
		appInterlockedExchange(&bShouldAbandon,TRUE);
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
 * Determines if the result code is considered an error or not
 *
 * @param ResultCode the return code to check
 *
 * @return TRUE if the code is an error code, FALSE otherwise
 */
UBOOL FSocketSubsystem::IsSocketError(INT ResultCode)
{
	if (ResultCode == -1)
	{
		ResultCode = GetLastErrorCode();
	}
	return ResultCode != SE_NO_ERROR;
}

/**
 * Checks the host name cache for an existing entry (faster than resolving again)
 *
 * @param HostName the host name to search for
 * @param Addr the out param that the IP will be copied to
 *
 * @return TRUE if the host was found, FALSE otherwise
 */
UBOOL FSocketSubsystem::GetHostByNameFromCache(ANSICHAR* HostName,FInternetIpAddr& Addr)
{
	// Lock for thread safety
	FScopeLock sl(&HostNameCacheSync);
	// Now search for the entry
	FInternetIpAddr* FoundAddr = HostNameCache.Find(FString(HostName));
	if (FoundAddr)
	{
		Addr = *FoundAddr;
	}
	return FoundAddr != NULL;
}

/**
 * Stores the ip address with the matching host name
 *
 * @param HostName the host name to search for
 * @param Addr the out param that the IP will be copied to
 */
void FSocketSubsystem::AddHostNameToCache(ANSICHAR* HostName,const FInternetIpAddr& Addr)
{
	// Lock for thread safety
	FScopeLock sl(&HostNameCacheSync);
	HostNameCache.Set(FString(HostName),Addr);
}

/**
 * Creates a platform specific async hostname resolution object
 *
 * @param HostName the name of the host to look up
 *
 * @return the resolve info to query for the address
 */
FResolveInfo* FSocketSubsystem::GetHostByName(ANSICHAR* HostName)
{
	FResolveInfo* Result = NULL;
	FInternetIpAddr Addr;
	// See if we have it cached or not
	if (GetHostByNameFromCache(HostName,Addr))
	{
		Result = new FResolveInfoCached(Addr);
	}
	else
	{
		// Create an async resolve info
		FResolveInfoAsync* AsyncResolve = new FResolveInfoAsync(HostName);
		AsyncResolve->StartAsyncTask();
		Result = AsyncResolve;
	}
	return Result;
}

//
// FSocketData functions
//

FString FSocketData::GetString( UBOOL bAppendPort )
{
	return Addr.ToString(bAppendPort);
}

void FSocketData::UpdateFromSocket(void)
{
	if (Socket != NULL)
	{
		Addr = Socket->GetAddress();
		Addr.GetPort(Port);
	}
}

//
// FIpAddr functions
//

/**
 * Constructs an ip address from an internet ip address
 *
 * @param InternetIpAddr the ip address to get host order info from
 */
FIpAddr::FIpAddr(const FInternetIpAddr& SockAddr)
{
	const FIpAddr New = SockAddr.GetAddress();
	Addr = New.Addr;
	Port = New.Port;
}

/**
 * Converts this address into string form. Optionally including the port info
 *
 * @param bShowPort whether to append the port number or not
 *
 * @return A new string object with the ip address data in it
 */
FString FIpAddr::ToString( UBOOL bAppendPort ) const
{
	// Get the individual bytes
	const INT A = (Addr >> 24) & 0xFF;
	const INT B = (Addr >> 16) & 0xFF;
	const INT C = (Addr >> 8) & 0xFF;
	const INT D = Addr & 0xFF;
	if (bAppendPort)
	{
		return FString::Printf(TEXT("%i.%i.%i.%i:%i"),A,B,C,D,Port);
	}
	else
	{
		return FString::Printf(TEXT("%i.%i.%i.%i"),A,B,C,D);
	}
}

/**
 * Builds an internet address from this host ip address object
 */
FInternetIpAddr FIpAddr::GetSocketAddress(void) const
{
	FInternetIpAddr Result;
	Result.SetIp(Addr);
	Result.SetPort(Port);
	return Result;
}

#endif	//#if WITH_UE3_NETWORKING
