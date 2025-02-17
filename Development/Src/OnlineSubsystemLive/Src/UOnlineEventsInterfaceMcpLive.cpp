/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "OnlineSubsystemLive.h"

#include "HTTPDownload.h"

/**
 * Sends the hardware data to the server for statistics aggregation
 *
 * @param UniqueId the unique id for the player
 * @param PlayerNick the player's nick name
 *
 * @return true if the async task was started successfully, false otherwise
 */
UBOOL UOnlineEventsInterfaceMcpLive::UploadHardwareData(FUniqueNetId Id,const FString& Nickname)
{
#if WITH_UE3_NETWORKING
	// Build the XML string allowing per platform specialization
	FString XmlPayload = FString::Printf(TEXT("<Hardware TitleId=\"%d\" UniqueId=\"%s\" Nickname=\"%s\" PlatformId=\"%d\">\r\n"),
		appGetTitleId(),
		*FormatAsString(Id),
		*Nickname,
		(DWORD)appGetPlatformType());
	// Add the language/region settings
	XmlPayload += FString::Printf(TEXT("\t<Region Id=\"%d\" LangId=\"%d\"/>\r\n"),
		XGetLanguage(),
		XC_GAME_REGION_REGION(XGetGameRegion()));
	// Read the video settings so we can add those
	XVIDEO_MODE VideoMode;
	XGetVideoMode(&VideoMode);
	XmlPayload += FString::Printf(TEXT("\t<Video Width=\"%d\" Height=\"%d\" Interlaced=\"%s\" WideScreen=\"%s\" HiDef=\"%s\" RefreshRate=\"%f\" VideoStd=\"%s\"/>\r\n"),
		VideoMode.dwDisplayWidth,
		VideoMode.dwDisplayHeight,
		BoolToString(VideoMode.fIsInterlaced),
		BoolToString(VideoMode.fIsWideScreen),
		BoolToString(VideoMode.fIsHiDef),
		VideoMode.RefreshRate,
		VideoStdToString(VideoMode.VideoStandard));
	// Now figure out what storage is available
	XmlPayload += TEXT("\t<Devices>\r\n");
	XDEVICE_DATA DeviceData;
	// There is no decent way to enumerate the devices so brute force device ID checks
	for (DWORD DeviceId = 1; DeviceId < 128; DeviceId++)
	{
		// Try to read the data for this id
		if (XContentGetDeviceData(DeviceId,&DeviceData) == ERROR_SUCCESS)
		{
			// The id was valid, so dump its info
			XmlPayload += FString::Printf(TEXT("\t\t<Device ID=\"%d\" Type=\"%s\" Size=\"%I64d\"/>\r\n"),
				DeviceId,
				DeviceTypeToString(DeviceData.DeviceType),
				DeviceData.ulDeviceBytes);
		}
	}
	XmlPayload += TEXT("\t</Devices>\r\n");
	// Add network information
	XmlPayload += FString::Printf(TEXT("\t<Network NAT=\"%s\"/>\r\n"),
		NatTypeToString(XOnlineGetNatType()));
	// Close the tag
	XmlPayload += TEXT("</Hardware>\r\n");
	// Now POST the data and have a response parsed
	return UploadPayload(EUT_HardwareData,XmlPayload);
#endif
	return FALSE;
}

/**
 * Appends the players to the XML data for matchmaking information
 *
 * @param XmlPayload the string being appended to
 * @param Players the group of players to add skill information for
 */
void UOnlineEventsInterfaceMcpLive::AppendPlayerMatchmakingData(FString& XmlPayload,const TArray<FPlayerReservation>& Players)
{
	XmlPayload += TEXT("\t<Players>\r\n");
	// Add all of the players to the payload
	for (INT Index = 0; Index < Players.Num(); Index++)
	{
		XmlPayload += FString::Printf(TEXT("\t\t<Player UniqueId=\"%s\" Skill=\"%d\" Mu=\"%f\" Sigma=\"%f\"/>\r\n"),
			*FormatAsString(Players(Index).NetId),
			Players(Index).Skill,
			Players(Index).Mu,
			Players(Index).Sigma);
	}
	XmlPayload += TEXT("\t</Players>\r\n");
}

/**
 * Adds the XML information for a particular match
 *
 * @param XmlPayload the string that is appended to
 * @parma HostSettings the match that is being added
 */
void UOnlineEventsInterfaceMcpLive::AppendMatchData(FString& XmlPayload,UOnlineGameSettings* HostSettings)
{
	DOUBLE HostMu = 3.0;
	DOUBLE HostSigma = 1.0;
	// Count how many players are consuming public & private slots
	DOUBLE HostCount = HostSettings->NumPublicConnections +
		HostSettings->NumPrivateConnections -
		HostSettings->NumOpenPublicConnections -
		HostSettings->NumOpenPrivateConnections;
	// Search through the returned settings for the Mu & Sigma values
	FSettingsProperty* PropMu = HostSettings->FindProperty(X_PROPERTY_GAMER_MU);
	if (PropMu != NULL)
	{
		FSettingsProperty* PropSigma = HostSettings->FindProperty(X_PROPERTY_GAMER_SIGMA);
		if (PropSigma != NULL)
		{
			PropMu->Data.GetData(HostMu);
			PropSigma->Data.GetData(HostSigma);
		}
	}
	// Now add the data
	XmlPayload += FString::Printf(TEXT("\t\t<Match Ping=\"%d\" Mu=\"%f\" Sigma=\"%f\" NumSkills=\"%d\" Quality=\"%f\"/>\r\n"),
		HostSettings->PingInMs,
		HostMu,
		HostSigma,
		HostCount,
		HostSettings->MatchQuality);
}

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
UBOOL UOnlineEventsInterfaceMcpLive::UploadMatchmakingSearchData(FUniqueNetId Id,UOnlineGameSearch* Search,INT SelectedIndex,const TArray<FPlayerReservation>& Players)
{
#if WITH_UE3_NETWORKING
	if (Search)
	{
		DOUBLE Mu = 3.0;
		DOUBLE Sigma = 1.0;
		// Use the API to aggregate the skill information
		XSessionCalculateSkill(Search->ManualSkillOverride.Mus.Num(),
			Search->ManualSkillOverride.Mus.GetTypedData(),
			Search->ManualSkillOverride.Sigmas.GetTypedData(),
			&Mu,
			&Sigma);
		// Grab the game we chose and use it's nonce as the key
		UOnlineGameSettings* GameSettings = Search->Results(SelectedIndex).GameSettings;
		// Build the XML string allowing per platform specialization
		FString XmlPayload = FString::Printf(TEXT("<Matchmaking TitleId=\"%d\" PlatformId=\"%d\" SessId=\"%I64d\" SearchUniqueId=\"%s\" SearchMu=\"%f\" SearchSigma=\"%f\" NumSkills=\"%d\">\r\n"),
			appGetTitleId(),
			(DWORD)appGetPlatformType(),
			GameSettings->ServerNonce,
			*FormatAsString(Id),
			Mu,
			Sigma,
			Search->ManualSkillOverride.Mus.Num());
		// Indicate which match we chose
		XmlPayload += FString::Printf(TEXT("\t<Matches SelectedMatch=\"%d\">\r\n"),SelectedIndex);
		// Add each of the sessions returned and mark the selected one
		for (INT Index = 0; Index < Search->Results.Num(); Index++)
		{
			AppendMatchData(XmlPayload,Search->Results(Index).GameSettings);
		}
		XmlPayload += FString::Printf(TEXT("\t</Matches>\r\n"));
		// Append the team information
		AppendPlayerMatchmakingData(XmlPayload,Players);
		// Close the tag
		XmlPayload += TEXT("</Matchmaking>\r\n");
		// Now POST the data and have a response parsed
		return UploadPayload(EUT_MatchmakingData,XmlPayload);
	}
#endif
	return FALSE;
}

/**
 * Uploads information about the match session that is being played
 *
 * @param GameSettings the game settings object that was used
 * @param Players the group of players that were used to register the session
 *
 * @return true if the async task was started successfully, false otherwise
 */
UBOOL UOnlineEventsInterfaceMcpLive::UploadMatchmakingSessionData(UOnlineGameSettings* GameSettings,const TArray<FPlayerReservation>& Players)
{
#if WITH_UE3_NETWORKING
	if (GameSettings)
	{
		// Build the XML string allowing per platform specialization
		FString XmlPayload = FString::Printf(TEXT("<Matchmaking TitleId=\"%d\" PlatformId=\"%d\" SessId=\"%I64d\" MatchUniqueId=\"%s\">\r\n"),
			appGetTitleId(),
			(DWORD)appGetPlatformType(),
			GameSettings->ServerNonce,
			*FormatAsString(GameSettings->OwningPlayerId));
		// Append the team information
		AppendPlayerMatchmakingData(XmlPayload,Players);
		// Close the tag
		XmlPayload += TEXT("</Matchmaking>\r\n");
		// Now POST the data and have a response parsed
		return UploadPayload(EUT_MatchmakingData,XmlPayload);
	}
#endif
	return FALSE;
}
