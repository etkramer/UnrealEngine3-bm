/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#ifndef __UNNETLIVE_H__
#define __UNNETLIVE_H__

#if WITH_UE3_NETWORKING

// Windows needs this, whereas the 360 doesn't
#if !CONSOLE
	#include "PreWindowsApi.h"
		#define WIN32_LEAN_AND_MEAN
		//NOTE: If you get an error here, make sure the G4WLive directories are in your additional includes/libs
		//NOTE: Tools->Options->Projects and Solutions->VC++ Directories
		//NOTE: $(GFWLSDK_DIR)include
		//NOTE: $(GFWLSDK_DIR)lib\x86
		#include <WinLive.h>
		#pragma warning(push) // Mirror what Windows.h does before including these
			#pragma warning(disable:4001)
			#pragma warning(disable:4201)
			#pragma warning(disable:4214)
			#include <MMSystem.h>
			#include <dsound.h>
			#include <shlobj.h>
		#pragma warning(pop)
	#include "PostWindowsApi.h"
#endif
// Both platforms need this for Voice
#pragma pack(push,8)
	#include <xhv.h>
#pragma pack(pop)

/**
 * Live specific IP net driver implementation
 */
class UIpNetDriverLive :
	public UTcpNetDriver
{
	DECLARE_CLASS(UIpNetDriverLive,UTcpNetDriver,CLASS_Config|CLASS_Transient,OnlineSubsystemLive)

#if STATS
	/** Tracks the overhead percentage of in bound bytes */
	DWORD InPercentOverhead;
	/** Tracks the overhead percentage of out bound bytes */
	DWORD OutPercentOverhead;
#endif

private:
	/**
	 * Queues any local voice packets for replication
	 */
	virtual void TickFlush(void);

	/** Used to disable auto downloading */
	virtual UBOOL InitConnect( FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error )
	{
		AllowDownloads = FALSE;
		return Super::InitConnect(InNotify,ConnectURL,Error);
	}

	/** Used to disable auto downloading */
	virtual UBOOL InitListen( FNetworkNotify* InNotify, FURL& ListenURL, FString& Error )
	{
		AllowDownloads = FALSE;
		return Super::InitListen(InNotify,ListenURL,Error);
	}
};

// The size of the VDP buffer
#define VDP_BUFFER_SIZE 512

// The max size we can have for sending
#define XE_MAX_PACKET_SIZE (VDP_BUFFER_SIZE - 2)

/**
 * Live specific IP connection implementation
 */
class UIpConnectionLive :
	public UTcpipConnection
{
	DECLARE_CLASS(UIpConnectionLive,UTcpipConnection,CLASS_Config|CLASS_Transient,OnlineSubsystemLive)

private:
	/** So we don't have to cast to get to the ReplicateVoicePacket() method */
	UIpNetDriverLive* XeNetDriver;
	/** Whether this connection is sending voice data as part of game data packets */
	UBOOL bUseVDP;
	/** Buffer used to merge game and voice data before sending */
	BYTE Buffer[VDP_BUFFER_SIZE];
	/** Whether we are running as a server or not */
	UBOOL bIsServer;
	/** TRUE if the socket has detected a critical error */
	UBOOL bHasSocketError;

	/**
	 * Emits object references for GC
	 */
	virtual void StaticConstructor(void);

	/**
	 * Initializes a connection with the passed in settings
	 *
	 * @param InDriver the net driver associated with this connection
	 * @param InSocket the socket associated with this connection
	 * @param InRemoteAddr the remote address for this connection
	 * @param InState the connection state to start with for this connection
	 * @param InOpenedLocally whether the connection was a client/server
	 * @param InURL the URL to init with
	 * @param InMaxPacket the max packet size that will be used for sending
	 * @param InPacketOverhead the packet overhead for this connection type
	 */
	virtual void InitConnection(UNetDriver* InDriver,FSocket* InSocket,
		const FInternetIpAddr& InRemoteAddr,EConnectionState InState,
		UBOOL InOpenedLocally,const FURL& InURL,INT InMaxPacket = 0,
		INT InPacketOverhead = 0);

	/**
	 * Sends a byte stream to the remote endpoint using the underlying socket.
	 * To minimize badwidth being consumed by encryption overhead, the voice
	 * data is appended to the gamedata before sending
	 *
	 * @param Data the byte stream to send
	 * @param Count the length of the stream to send
	 */
	virtual void LowLevelSend(void* Data,INT Count);

	/**
	 * Attempts to pack all of the voice packets in the voice channel into the buffer.
	 *
	 * @param WriteAt the point in the buffer to write voice data
	 * @param SpaceAvail the amount of space left in the merge buffer
	 * @param OutBytesMerged the amount of data added to the buffer
	 *
	 * @return TRUE if the merging included all replicated packets, FALSE otherwise (not enough space)
	 */
	UBOOL MergeVoicePackets(BYTE* WriteAt,DWORD SpaceAvail,WORD& OutBytesMerged);

	/**
	 * Processes the byte stream for VDP merged packet support. Parses out all
	 * of the voice packets and then forwards the game data to the base class
	 * for handling
	 *
	 * @param Data the data to process
	 * @param Count the size of the data buffer to process
	 */
	virtual void ReceivedRawPacket(void* Data,INT Count);
};

#endif	//#if WITH_UE3_NETWORKING

#endif
