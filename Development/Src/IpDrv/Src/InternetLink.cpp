/*=============================================================================
	InternetLink.cpp: Unreal Internet Connection Superclass
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Includes.
-----------------------------------------------------------------------------*/

#include "UnIpDrv.h"

#if WITH_UE3_NETWORKING

/*-----------------------------------------------------------------------------
	Defines.
-----------------------------------------------------------------------------*/

#define FTCPLINK_MAX_SEND_BYTES 4096

/*-----------------------------------------------------------------------------
	FInternetLink
-----------------------------------------------------------------------------*/

UBOOL FInternetLink::ThrottleSend = 0;
UBOOL FInternetLink::ThrottleReceive = 0;
INT FInternetLink::BandwidthSendBudget = 0;
INT FInternetLink::BandwidthReceiveBudget = 0;

void FInternetLink::ThrottleBandwidth( INT SendBudget, INT ReceiveBudget )
{
	ThrottleSend = SendBudget != 0;
	ThrottleReceive = ReceiveBudget != 0;

	// If we didn't spend all our sent bandwidth last timeframe, we don't get it back again.
	BandwidthSendBudget = SendBudget;

	// If we received more than our budget last timeframe, reduce this timeframe's budget accordingly.
    BandwidthReceiveBudget = Min<INT>( BandwidthReceiveBudget + ReceiveBudget, ReceiveBudget );
}

/*-----------------------------------------------------------------------------
	FUdpLink
-----------------------------------------------------------------------------*/

FUdpLink::FUdpLink()
:	ExternalSocket(0)
,	StatBytesSent(0)
,	StatBytesReceived(0)
{
	SocketData.Socket = GSocketSubsystem ? GSocketSubsystem->CreateDGramSocket(TRUE) : NULL;
	if (SocketData.Socket != NULL)
	{
		SocketData.Socket->SetReuseAddr();
		SocketData.Socket->SetNonBlocking();
		SocketData.Socket->SetRecvErr();
	}
}

FUdpLink::FUdpLink(const FSocketData &InSocketData)
:	FInternetLink(InSocketData)
,	ExternalSocket(1)
,	StatBytesSent(0)
,	StatBytesReceived(0)
{
}

FUdpLink::~FUdpLink()
{
	if( !ExternalSocket )
	{
//		warnf(TEXT("Closing UDP socket %d"), SocketData.Port);
		GSocketSubsystem->DestroySocket(SocketData.Socket);
		SocketData.Socket = NULL;
	}
}

UBOOL FUdpLink::BindPort(INT InPort)
{
	if (SocketData.Socket == NULL)
	{
		warnf(TEXT("FUdpLink::BindPort: Socket was not created"));
		return FALSE;
	}
	
	SocketData.Port = InPort;
	SocketData.Addr.SetPort(InPort);
	SocketData.Addr.SetIp(getlocalbindaddr( *GWarn ));

	if (SocketData.Socket == NULL)
	{
		warnf(TEXT("FUdpLink::BindPort: Socket was not created"));
		return FALSE;
	}

	if (SocketData.Socket->SetBroadcast() == FALSE)
	{
		warnf(TEXT("FUdpLink::BindPort: setsockopt failed"));
		return FALSE;
	}

	if (SocketData.Socket->Bind(SocketData.Addr) == FALSE)
	{
		warnf(TEXT("FUdpLink::BindPort: setsockopt failed"));
		return FALSE;
	}

	// if 0, read the address we bound from the socket.
	if( InPort == 0 )
	{
		SocketData.UpdateFromSocket();
	}

//	warnf(TEXT("UDP: bound to local port %d"), SocketData.Port);

	return TRUE;
}

INT FUdpLink::SendTo( FIpAddr Destination, BYTE* Data, INT Count )
{
	UBOOL bOk = FALSE;
	INT BytesSent = 0;

	if (SocketData.Socket == NULL)
	{
		return FALSE;
	}

	bOk = SocketData.Socket->SendTo(Data,Count,BytesSent,Destination.GetSocketAddress());
	if (bOk == FALSE)
	{
		warnf(TEXT("SendTo: %s returned %d: %d"), *Destination.ToString(TRUE),
			GSocketSubsystem->GetLastErrorCode(),
			GSocketSubsystem->GetSocketError());
	}
	StatBytesSent += BytesSent;
	return bOk;
}

void FUdpLink::Poll()
{
	BYTE Buffer[4096];
	FInternetIpAddr SockAddr;

	if (SocketData.Socket == NULL)
	{
		return;
	}
	for(;;)
	{
		INT BytesRead = 0;
		UBOOL bOk = SocketData.Socket->RecvFrom(Buffer,sizeof(Buffer),BytesRead,SockAddr);
		if( bOk == FALSE )
		{
			if( GSocketSubsystem->GetLastErrorCode() == SE_EWOULDBLOCK )
				break;
			else
			// SE_ECONNRESET means we got an ICMP unreachable, and should continue calling recv()
			if( GSocketSubsystem->GetLastErrorCode() != SE_ECONNRESET )
			{
#if PS3
				if (GSocketSubsystem->GetLastErrorCode() == SE_ETIMEDOUT) // ETIMEDOUT, apparently an improper error
				{
					// ignore this error
				}
				else
#endif
				{
					warnf(TEXT("RecvFrom returned SOCKET_ERROR %d [socket = %d:%d]"),
						GSocketSubsystem->GetLastErrorCode(), SocketData.Socket, SocketData.Port );
				}
				break;
			}
		}
		else
		if( BytesRead > 0 )
		{
			StatBytesReceived += BytesRead;
			const FIpAddr& Addr = SockAddr.GetAddress();
			OnReceivedData(Addr,Buffer,BytesRead);
		}
		else
			break;
	}
}

#endif	//#if WITH_UE3_NETWORKING
