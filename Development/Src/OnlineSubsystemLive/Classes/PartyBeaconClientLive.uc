/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This is the Live specific version of the party beacon. It handles registering &
 * unregistering of secure keys so that communication with a potential host is
 * possible
 */
class PartyBeaconClientLive extends PartyBeaconClient
	native;

cpptext
{
	/**
	 * Performs platform specific resolution of the address
	 *
	 * @param DesiredHost the host to resolve the IP address for
	 * @param Addr out param having it's address set
	 *
	 * @return true if the address could be resolved, false otherwise
	 */
	virtual UBOOL ResolveAddress(const FOnlineGameSearchResult& DesiredHost,FInternetIpAddr& Addr)
	{
		// Use the session information to build the address
		XSESSION_INFO* SessionInfo = (XSESSION_INFO*)DesiredHost.PlatformData;
		if (SessionInfo != NULL)
		{
			// Figure out if we need to do the secure IP handling or not
			if (GSocketSubsystem->RequiresEncryptedPackets())
			{
				in_addr InAddr;
				DWORD Result = XNetXnAddrToInAddr(&SessionInfo->hostAddress,&SessionInfo->sessionID,&InAddr);
				debugf(NAME_DevOnline,
					TEXT("Beacon (%s) XNetXnAddrToInAddr() returned 0x%08X"),
					*BeaconName.ToString(),
					Result);
				// Try to decode the secure address so we can connect to it
				if (Result == 0)
				{
					Addr.SetIp(InAddr);
					// Set to the configured port rather than what's in the address
					Addr.SetPort(PartyBeaconPort);
					return TRUE;
				}
				else
				{
					debugf(NAME_DevOnline,TEXT("Failed to resolve secure IP"));
				}
			}
			else
			{
				// Don't use the encrypted/decrypted form of the IP when it's not required
				Addr.SetIp(SessionInfo->hostAddress.ina);
				// Set to the configured port rather than what's in the address
				Addr.SetPort(PartyBeaconPort);
				return TRUE;
			}
		}
		return FALSE;
	}

	/**
	 * Allows for per platform registration of secure keys, so that a secure connection
	 * can be opened and used for sending/receiving data.
	 *
	 * @param DesiredHost the host that is being registered
	 */
	virtual UBOOL RegisterAddress(const FOnlineGameSearchResult& DesiredHost)
	{
		if (GSocketSubsystem->RequiresEncryptedPackets())
		{
			// Grab the secure keys from the session info
			XSESSION_INFO* SessionInfo = (XSESSION_INFO*)DesiredHost.PlatformData;
			if (SessionInfo != NULL)
			{
				// Now register them with the net layer so we can send packets
				DWORD Result = XNetRegisterKey(&SessionInfo->sessionID,&SessionInfo->keyExchangeKey);
				debugf(NAME_DevOnline,
					TEXT("Beacon (%s) XNetRegisterKey() returned 0x%08X"),
					*BeaconName.ToString(),
					Result);
				return Result == 0;
			}
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Allows for per platform unregistration of secure keys, which breaks the link between
	 * a client and server. This also releases any memory associated with the keys.
	 *
	 * @param DesiredHost the host that is being registered
	 */
	virtual UBOOL UnregisterAddress(const FOnlineGameSearchResult& DesiredHost)
	{
		if (GSocketSubsystem->RequiresEncryptedPackets())
		{
			// Grab the secure keys from the session info
			XSESSION_INFO* SessionInfo = (XSESSION_INFO*)DesiredHost.PlatformData;
			if (SessionInfo != NULL)
			{
				// Unregister the keys so we don't leak
				DWORD Result = XNetUnregisterKey(&SessionInfo->sessionID);
				debugf(NAME_DevOnline,
					TEXT("Beacon (%s) XNetUnregisterKey() returned 0x%08X"),
					*BeaconName.ToString(),
					Result);
				return Result == 0;
			}
			return FALSE;
		}
		return TRUE;
	}
}
