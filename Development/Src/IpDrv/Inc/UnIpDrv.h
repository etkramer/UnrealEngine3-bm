/*=============================================================================
	IpDrvPrivate.h: Unreal TCP/IP driver.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef UNIPDRV_H
#define UNIPDRV_H

// include for all definitions
#include "Engine.h"

#if WITH_UE3_NETWORKING

#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS 0
#endif

#if _MSC_VER
	#pragma warning( disable : 4201 )
#endif

// Set the socket api name depending on platform
#if _MSC_VER
#if XBOX || WITH_PANORAMA
	#define SOCKET_API TEXT("LiveSock")
#else
	#define SOCKET_API TEXT("WinSock")
#endif
#elif PS3
	#define SOCKET_API TEXT("PS3Sockets")
#else
	#define SOCKET_API TEXT("Sockets")
#endif

// BSD socket includes.
#if !_MSC_VER
	#include <stdio.h>
	#include <unistd.h>
	#include <sys/types.h>
#if PS3
	#include <common/include/sdk_version.h>
	#include <sys/ansi.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <netex/errno.h>
	#include <netex/net.h>
	#include <sys/time.h>
	#include <sys/select.h>
	#include <cell/sysmodule.h>
	#include <np.h>
#else
	#include <errno.h>
	#include <fcntl.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <sys/uio.h>
	#include <sys/ioctl.h>
	#include <sys/time.h>
	#include <pthread.h>
#endif
	// Handle glibc < 2.1.3
	#ifndef MSG_NOSIGNAL
	#define MSG_NOSIGNAL 0x4000
	#endif
#endif	// !_MSC_VER


/*-----------------------------------------------------------------------------
	Includes..
-----------------------------------------------------------------------------*/

#include "UnNet.h"
// Base socket interfaces
#include "UnSocket.h"
// Platform specific implementations
#if PS3
#include "UnSocketPS3.h"
#elif PLATFORM_UNIX
#include "UnSocketBSD.h"
#else
#include "UnSocketWin.h"
#endif

/*-----------------------------------------------------------------------------
	Definitions.
-----------------------------------------------------------------------------*/

// Globals.
extern UBOOL GIpDrvInitialized;

#if !SHIPPING_PC_GAME
/** Network communication between UE3 executable and the "Unreal Console" tool. */
extern class FDebugServer*				GDebugChannel;
#endif

/*-----------------------------------------------------------------------------
	More Includes.
-----------------------------------------------------------------------------*/

#include "InternetLink.h"
#include "FRemotePropagator.h"
#include "FDebugServer.h"
#include "DebugServerDefs.h"

/*-----------------------------------------------------------------------------
	Bind to next available port.
-----------------------------------------------------------------------------*/

//
// Bind to next available port.
//
inline INT bindnextport( FSocket* Socket, FInternetIpAddr& Addr, INT portcount, INT portinc )
{
	for( INT i=0; i<portcount; i++ )
	{
		if( Socket->Bind(Addr) == TRUE )
		{
			if (Addr.GetPort() != 0)
			{
				return Addr.GetPort();
			}
			else
			{
				return Socket->GetPortNo();
			}
		}
		if( Addr.GetPort() == 0 )
			break;
		Addr.SetPort(Addr.GetPort() + portinc);
	}
	return 0;
}

//
// Get local IP to bind to
//
inline FInternetIpAddr getlocalbindaddr( FOutputDevice& Out )
{
	FInternetIpAddr BindAddr;
	// If we can bind to all addresses, return 0.0.0.0
	if (GSocketSubsystem->GetLocalHostAddr(Out,BindAddr) == TRUE)
	{
		BindAddr.SetAnyAddress();
	}
	return BindAddr;

}


/*-----------------------------------------------------------------------------
	Public includes.
-----------------------------------------------------------------------------*/

#include "VoiceInterface.h"
// Common code shared across all platforms for the online subsystem
#include "OnlineSubsystemUtilities.h"

#include "IpDrvClasses.h"

#include "UnTcpNetDriver.h"

// Per platform driver/connection implementations
#if PS3
#endif

#include "UnStatsNotifyProviders_UDP.h"

#endif	//#if WITH_UE3_NETWORKING

#endif // UNIPDRV_H

