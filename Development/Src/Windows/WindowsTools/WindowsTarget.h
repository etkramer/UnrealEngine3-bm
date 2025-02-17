/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#ifndef _WINDOWSTARGET_H_
#define _WINDOWSTARGET_H_

#include "Common.h"
#include "..\..\IpDrv\Inc\GameType.h"
#include "Socket.h"
#include "RefCountedTarget.h"

enum EOverlappedEventType
{
	OVT_Recv,
	OVT_Send,
};

#define OVERLAPPED_BUF_SIZE	2048 //2kb

class CWindowsTarget;

struct OverlappedEventArgs : OVERLAPPED 
{
	EOverlappedEventType EventType;
	FReferenceCountPtr<CWindowsTarget> Owner;
	WSABUF WSABuffer;
	char Buffer[OVERLAPPED_BUF_SIZE];

	OverlappedEventArgs()
	{
		ZeroMemory(this, sizeof(OVERLAPPED));
		WSABuffer.buf = Buffer;
		WSABuffer.len = OVERLAPPED_BUF_SIZE;
	}

	OverlappedEventArgs(EOverlappedEventType EType) : EventType(EType)
	{
		ZeroMemory(this, sizeof(OVERLAPPED));
		WSABuffer.buf = Buffer;
		WSABuffer.len = OVERLAPPED_BUF_SIZE;
	}
};

/** Available platform types; mirror of UnFile.h */
enum EPlatformType
{
	EPlatformType_Unknown = 0x00,
	EPlatformType_Windows = 0x01,
	EPlatformType_Xenon = 0x04,
	EPlatformType_PS3 = 0x08,
	EPlatformType_Linux = 0x10,
	EPlatformType_MacOSX = 0x20,
	EPlatformType_Max
};

/// Representation of a single UE3 instance running on PC
class CWindowsTarget : public FRefCountedTarget<CWindowsTarget>
{
public:
	/** UDP client used to connect to this target. */
	FSocket UDPClient;
	/** TCP client that listens to messages from the game. */
	FSocket TCPClient;

	/** User friendly name of the target. */
	wstring Name;
	/** Computer name. */
	wstring ComputerName;
	/** Game name. */
	wstring GameName;
	/** Configuration name. */
	wstring Configuration;
	/** Game type id. */
	EGameType GameType;
	/** Game type name. */
	wstring GameTypeName;
	/** Platform id. */
	EPlatformType PlatformType;
	/** Port the server listens on. */
	int ListenPortNo;

	/** System time of target registration ('server response' received). */
	__time64_t TimeRegistered;

	char *PartialPacketBuffer;
	int PartialPacketBufferSize;

	/** The remote address of the target */
	sockaddr_in RemoteAddress;

	/** True if something is currently connected to this target. */
	bool bConnected;

	/** The callback for TTY notifications. */
	TTYEventCallbackPtr TxtCallback;

	/** Constructor. */
	CWindowsTarget();	

	virtual ~CWindowsTarget();

	/** Make the name user friendly. */
	void UpdateName();

	/** Sends TTY text on the callback if it is valid. */
	void SendTTY(const wchar_t *Txt);
};

#endif
