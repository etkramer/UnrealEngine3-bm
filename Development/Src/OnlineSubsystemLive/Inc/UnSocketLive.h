/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#ifndef __UNSOCKETLIVE_H__
#define __UNSOCKETLIVE_H__

// Include the base class for our xenon subsystem
#include "UnSocketWin.h"

#if WITH_UE3_NETWORKING

/**
 * Xenon specific socket subsystem implementation
 */
class FSocketSubsystemLive :
	public FSocketSubsystemWindows
{
	/** Whether to create dgram sockets using VDP or not */
	UBOOL bUseVDP;
	/** Whether to enable the secure libs or not */
	UBOOL bUseSecureConnections;
	/** Whether to use the LSP enumeration or not */
	UBOOL bUseLspEnumerate;
	/** The LSP service id provided by the Live team */
	INT ServiceId;

public:
	/** Zeroes members */
	FSocketSubsystemLive(void) :
		FSocketSubsystemWindows(),
		bUseVDP(TRUE),
		bUseSecureConnections(FALSE),
		bUseLspEnumerate(FALSE),
		ServiceId(0)
	{
	}

	/**
	 * Does Xenon platform initialization of the sockets library
	 *
	 * @param Error a string that is filled with error information
	 *
	 * @return TRUE if initialized ok, FALSE otherwise
	 */
	virtual UBOOL Initialize(FString& Error);
	/**
	 * Performs Xenon & Live specific socket subsystem clean up
	 */
	virtual void Destroy(void);
	/**
	 * Creates a data gram socket
	 *
	 * @param bForceUDP overrides any platform specific protocol with UDP instead
	 *
	 * @return the new socket or NULL if failed
	 */
	virtual FSocket* CreateDGramSocket(UBOOL bForceUDP = FALSE);
	/**
	 * Does a DNS look up of a host name
	 *
	 * @param HostName the name of the host to look up
	 * @param Addr the address to copy the IP address to
	 */
	virtual INT GetHostByName(ANSICHAR* HostName,FInternetIpAddr& Addr);
	/**
	 * Creates a platform specific async hostname resolution object
	 *
	 * @param HostName the name of the host to look up
	 *
	 * @return the resolve info to query for the address
	 */
	virtual FResolveInfo* GetHostByName(ANSICHAR* HostName);
	/**
	 * Live requires chat data (voice, text, etc.) to be placed into
	 * packets after game data (unencrypted). Use the VDP setting to
	 * control this
	 */
	virtual UBOOL RequiresChatDataBeSeparate(void)
	{
		return bUseVDP;
	}
	/**
	 * Some platforms require packets be encrypted. This function tells the
	 * net connection whether this is required for this platform
	 */
	virtual UBOOL RequiresEncryptedPackets(void)
	{
		return bUseSecureConnections;
	}
	/**
	 * Determines the name of the Xenon
	 *
	 * @param HostName the string that receives the data
	 *
	 * @return TRUE if successful, FALSE otherwise
	 */
	virtual UBOOL GetHostName(FString& HostName);
	/**
	 * Uses the secure libs to look up the host address
	 *
	 * @param Out the output device to log messages to
	 * @param HostAddr the out param receiving the host address
	 *
	 * @return always TRUE
	 */
	virtual UBOOL GetLocalHostAddr(FOutputDevice& Out,FInternetIpAddr& HostAddr);
};

#endif	//#if WITH_UE3_NETWORKING

#endif
