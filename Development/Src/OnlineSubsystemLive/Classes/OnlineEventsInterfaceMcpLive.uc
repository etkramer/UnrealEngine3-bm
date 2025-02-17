/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Provides an in game gameplay events/stats upload mechanism via the MCP backend
 */
class OnlineEventsInterfaceMcpLive extends OnlineEventsInterfaceMcp
	native;

cpptext
{
	/**
	 * Converts the net id to a string with the one way hash of it
	 *
	 * @param Id the net id to convert
	 *
	 * @return the string form of the id
	 */
	virtual FString FormatAsString(const FUniqueNetId& Id)
	{
		XUID ZeroId = 0;
		// Don't SHA bot's ids
		if ((XUID&)Id != ZeroId)
		{
			return FString::Printf(TEXT("%s"),*appHashUniqueNetId(Id));
		}
		return TEXT("0");
	}

	/**
	 * Returns a true/false string for the bool
	 *
	 * @param bBool the bool being converted
	 *
	 * @return true/false string
	 */
	FORCEINLINE const TCHAR* BoolToString(UBOOL bBool)
	{
		return bBool ? TEXT("true") : TEXT("false");
	}

	/**
	 * Returns a string for the video standard
	 *
	 * @param VideoStd the video standard to convert
	 *
	 * @return either NTSC, NTSC-J, or PAL
	 */
	FORCEINLINE const TCHAR* VideoStdToString(DWORD VideoStd)
	{
#if CONSOLE
		switch (VideoStd)
		{
			case XC_VIDEO_STANDARD_NTSC_J:
			{
				return TEXT("NTSC-J");
			}
			case XC_VIDEO_STANDARD_PAL_I:
			{
				return TEXT("PAL");
			}
		}
		return TEXT("NTSC");
#else
		return TEXT("VGA");
#endif
	}

	/**
	 * Returns a string for the device type
	 *
	 * @param DeviceType the device type to convert
	 *
	 * @return either HD, MU, or Unknown
	 */
	FORCEINLINE const TCHAR* DeviceTypeToString(DWORD DeviceType)
	{
#if CONSOLE
		switch (DeviceType)
		{
			case XCONTENTDEVICETYPE_HDD:
			{
				return TEXT("HD");
			}
			case XCONTENTDEVICETYPE_MU:
			{
				return TEXT("MU");
			}
		}
#endif
		return TEXT("UNKNOWN");
	}

	/**
	 * Returns a string for the NAT type
	 *
	 * @param NatType the NAT type to convert
	 *
	 * @return either OPEN, STRICT, or MODERATE
	 */
	FORCEINLINE const TCHAR* NatTypeToString(DWORD NatType)
	{
		switch (NatType)
		{
			case XONLINE_NAT_OPEN:
			{
				return TEXT("OPEN");
			}
			case XONLINE_NAT_MODERATE:
			{
				return TEXT("MODERATE");
			}
		}
		return TEXT("STRICT");
	}

	/**
	 * Adds the XML information for a particular match
	 *
	 * @param XmlPayload the string that is appended to
	 * @parma HostSettings the match that is being added
	 */
	void AppendMatchData(FString& XmlPayload,UOnlineGameSettings* HostSettings);

	/**
	 * Appends the players to the XML data for matchmaking information
	 *
	 * @param XmlPayload the string being appended to
	 * @param Players the group of players to add skill information for
	 */
	void AppendPlayerMatchmakingData(FString& XmlPayload,const TArray<FPlayerReservation>& Players);
}

/**
 * Sends the hardware data to the server for statistics aggregation
 *
 * @param UniqueId the unique id for the player
 * @param PlayerNick the player's nick name
 *
 * @return true if the async task was started successfully, false otherwise
 */
native function bool UploadHardwareData(UniqueNetId UniqueId,string PlayerNick);

/**
 * Uploads information about the matching search
 *
 * @param UniqueId the unique id for the player uploading
 * @param Search the search object that was used
 * @param SelectedIndex the match that was chosen
 * @param Players the group of players that were used to make the search
 *
 * @return true if the async task was started successfully, false otherwise
 */
native function bool UploadMatchmakingSearchData(UniqueNetId UniqueId,OnlineGameSearch Search,int SelectedIndex,const out array<PlayerReservation> Players);

/**
 * Uploads information about the match session that is being played
 *
 * @param GameSettings the game settings object that was used
 * @param Players the group of players that were used to register the session
 *
 * @return true if the async task was started successfully, false otherwise
 */
native function bool UploadMatchmakingSessionData(OnlineGameSettings GameSettings,const out array<PlayerReservation> Players);
