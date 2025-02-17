/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "WindowsTarget.h"
#include "StringUtils.h"

///////////////////////CWindowsTarget/////////////////////////////////

CWindowsTarget::CWindowsTarget() : PartialPacketBuffer(NULL), PartialPacketBufferSize(0), bConnected(false), TxtCallback(NULL)
{
	ZeroMemory(&RemoteAddress, sizeof(RemoteAddress));

	UDPClient.SetAttributes(SF_IPv4, ST_Datagram, SP_UDP);
	TCPClient.SetAttributes(SF_IPv4, ST_Stream, SP_TCP);
}

CWindowsTarget::~CWindowsTarget()
{
	if(PartialPacketBuffer)
	{
		delete [] PartialPacketBuffer;
		PartialPacketBuffer = NULL;
	}
}

/** Make the name user friendly. */
void CWindowsTarget::UpdateName()
{
	const unsigned int Address = UDPClient.GetIP();
	Name = ComputerName + L" (" + ToWString( inet_ntoa(*(in_addr*) &Address) ) + L")";
}

void CWindowsTarget::SendTTY(const wchar_t *Txt)
{
	if(TxtCallback)
	{
		TxtCallback(Txt);
	}
}