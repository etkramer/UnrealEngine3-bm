/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "UnIpDrv.h"

#include "HTTPDownload.h"

IMPLEMENT_CLASS(UOnlineEventsInterfaceMcp);

#define GAMEPLAY_EVENTS_XML_VER 3
const INT GAMEPLAY_EVENTS_BINARY_VER = 1003;

/** @note these must match the COM component used by MCP */
#define COMPRESSED_VER 1
#define UNCOMPRESSED_HEADER_SIZE (1 + 3)
#define COMPRESSED_HEADER_SIZE (1 + 3 + 4)
#define BINARY_STATS_FLAG 0x02
#define BYTE_SWAP_BINARY_UPLOADS 1

/**
 * Ticks any http requests that are in flight
 *
 * @param DeltaTime the amount of time that has passed since the last tick
 */
void UOnlineEventsInterfaceMcp::Tick(FLOAT DeltaTime)
{
	// Tick any objects in the list and remove if they are done
	for (INT HttpIndex = 0; HttpIndex < HttpPostObjects.Num(); HttpIndex++)
	{
		FHttpDownloadString* HttpPoster = HttpPostObjects(HttpIndex);
		HttpPoster->Tick(DeltaTime);
		// See if we are done and remove the item if so
		if (HttpPoster->GetHttpState() == HTTP_Closed ||
			HttpPoster->GetHttpState() == HTTP_Error)
		{
#if _DEBUG
			FString Response;
			HttpPoster->GetString(Response);
			debugf(NAME_DevOnline,TEXT("Event upload response:\r\n%s"),*Response);
#endif
			delete HttpPoster;
			HttpPostObjects.Remove(HttpIndex);
			HttpIndex--;
		}
	}
}

/**
 * Sends the profile data to the server for statistics aggregation
 *
 * @param Id the unique id of the player
 * @param Nickname the nickname of the player
 * @param ProfileSettings the profile object that is being sent
 *
 * @return true if the async task was started successfully, false otherwise
 */
UBOOL UOnlineEventsInterfaceMcp::UploadProfileData(FUniqueNetId Id,const FString& Nickname,UOnlineProfileSettings* ProfileSettings)
{
#if WITH_UE3_NETWORKING
	if (ProfileSettings)
	{
		// Build the XML string allowing per platform specialization
		FString XmlPayload = FString::Printf(TEXT("<Profile TitleId=\"%d\" UniqueId=\"%s\" Name=\"%s\" PlatformId=\"%d\">\r\n"),
			appGetTitleId(),
			*FormatAsString(Id),
			*Nickname,
			(DWORD)appGetPlatformType());
		// Now add the profile data
		ToXml(XmlPayload,ProfileSettings->ProfileSettings,1);
		// Close the tag
		XmlPayload += TEXT("</Profile>\r\n");
		// Now POST the data and have a response parsed
		return UploadPayload(EUT_ProfileData,XmlPayload);
	}
#endif
	return FALSE;
}

/**
 * Method for POST-ing text data
 *
 * @param UploadType the type of upload that is happening
 * @param Payload the data to send
 *
 * @return TRUE if the send started successfully, FALSE otherwise
 */
UBOOL UOnlineEventsInterfaceMcp::UploadPayload(BYTE UploadType,const FString& Payload)
{
	TArray<BYTE> UncompressedBuffer;
	INT UncompressedBufferSize = Payload.Len();
	// Copy the data into the uncompressed buffer in ANSI char form
	UncompressedBuffer.Empty(UncompressedBufferSize);
	UncompressedBuffer.Add(UncompressedBufferSize);
	appMemcpy(UncompressedBuffer.GetTypedData(),(const BYTE*)TCHAR_TO_ANSI(*Payload),UncompressedBufferSize);

	return UploadFinalPayload(true, UploadType, UncompressedBuffer);
}

/**
 * Method for POST-ing binary data.
 *
 * @param UploadType the type of upload that is happening
 * @param Payload the data to send
 *
 * @return TRUE if the send started successfully, FALSE otherwise
 */
UBOOL UOnlineEventsInterfaceMcp::UploadBinaryPayload(BYTE UploadType, const TArray<BYTE>& UncompressedBuffer)
{
	return UploadFinalPayload(false, UploadType, UncompressedBuffer);
}
/**
* Common method for POST-ing a payload to an URL (determined by upload type)
*
* @param UploadType the type of upload that is happening
* @param Payload the data to send
*
* @return TRUE if the send started successfully, FALSE otherwise
*/
UBOOL UOnlineEventsInterfaceMcp::UploadFinalPayload(UBOOL bWasText, BYTE UploadType, const TArray<BYTE>& UncompressedBuffer)
{
	DWORD Result = E_FAIL;
#if WITH_UE3_NETWORKING
	// Find the upload configuration
	FEventUploadConfig* UploadConfig = FindUploadConfig(UploadType);
	// Validate the entry was configured properly
	if (UploadConfig &&
		UploadConfig->UploadUrl.Len())
	{
		// Build an url from the string
		FURL Url(NULL,*UploadConfig->UploadUrl,TRAVEL_Absolute);
		FResolveInfo* ResolveInfo = NULL;
		// See if we need to resolve this string
		UBOOL bIsValidIp = FInternetIpAddr::IsValidIp(*Url.Host);
		if (bIsValidIp == FALSE)
		{
			// Allocate a platform specific resolver and pass that in
			ResolveInfo = GSocketSubsystem->GetHostByName(TCHAR_TO_ANSI(*Url.Host));
		}
		// Create a new posting object
		FHttpDownloadString* HttpPoster = new FHttpDownloadString(FALSE,
			UploadConfig->TimeOut,
			FString(),
			ResolveInfo,
			HRT_Post);
		// Determine whether to send as text or compressed
		if (UploadConfig->bUseCompression)
		{
			TArray<BYTE> CompressedBuffer;
			const INT UncompressedBufferSize = UncompressedBuffer.Num();
			INT CompressedBufferSize = UncompressedBufferSize + COMPRESSED_HEADER_SIZE;
			// Now size the buffer leaving space for the header
			CompressedBuffer.Empty(CompressedBufferSize);
			CompressedBuffer.Add(CompressedBufferSize);
			// Now compress the bufer into our destination skipping the header
			verify(appCompressMemory(ECompressionFlags(COMPRESS_ZLIB | COMPRESS_BiasSpeed),
				&CompressedBuffer(COMPRESSED_HEADER_SIZE),
				CompressedBufferSize,
				(void*)UncompressedBuffer.GetTypedData(),
				UncompressedBufferSize));
			// Finally, write the header information into the buffer
			CompressedBuffer(0) = 0x4D;	// M
			CompressedBuffer(1) = 0x43;	// C
			CompressedBuffer(2) = 0x50; // P
			CompressedBuffer(3) = COMPRESSED_VER;
			if ( !bWasText )
			{
				CompressedBuffer(3) |= BINARY_STATS_FLAG;	// Turn on the Binary Data flag
			}
			CompressedBuffer(4) = (UncompressedBufferSize & 0xFF000000) >> 24;
			CompressedBuffer(5) = (UncompressedBufferSize & 0x00FF0000) >> 16;
			CompressedBuffer(6) = (UncompressedBufferSize & 0x0000FF00) >> 8;
			CompressedBuffer(7) = UncompressedBufferSize & 0xFF;
			// Copy the payload into the object for sending
			HttpPoster->CopyPayload(CompressedBuffer.GetTypedData(),CompressedBufferSize + COMPRESSED_HEADER_SIZE);
		}
		else
		{
			TArray<BYTE> FinalBuffer;
			const INT UncompressedBufferSize = UncompressedBuffer.Num();
			INT FinalBufferSize = UncompressedBufferSize + UNCOMPRESSED_HEADER_SIZE;
			// Now size the buffer leaving space for the header
			FinalBuffer.Empty(FinalBufferSize);
			FinalBuffer.Add(FinalBufferSize);
			FinalBuffer(0) = 0x4D;	// M
			FinalBuffer(1) = 0x43;	// C
			FinalBuffer(2) = 0x50;  // P
			FinalBuffer(3) = (bWasText) ? 0 : BINARY_STATS_FLAG;
			appMemcpy(&FinalBuffer(4), &UncompressedBuffer(0), UncompressedBufferSize);
			HttpPoster->CopyPayload(FinalBuffer.GetTypedData(),FinalBufferSize);
		}

		// Create the object that will upload the data and download the response
		INT AddIndex = HttpPostObjects.AddItem(HttpPoster);
		// Start the download task
		HttpPoster->DownloadUrl(Url);
		Result = ERROR_IO_PENDING;
	}
	else
	{
		debugf(NAME_Error,TEXT("No URL configured for upload type (%d)"),(DWORD)UploadType);
	}
#endif
	return Result == ERROR_SUCCESS || Result == ERROR_IO_PENDING;
}

/**
 * SimpleStream - This is a utility class that allows us to serialize raw data out to a TArray.
 */
class FSimpleStream : public TArray<BYTE>
{
public:
	
	/**
	 * Constructor
	 */
	FSimpleStream(INT InitialSize = 0):TArray<BYTE>(InitialSize),Offset(0){};
	
	
	/**
	 * @Returns the current offset in to the stream
	 */
	INT Tell() const
	{
		return Offset;
	}

	/**
	 * Seeks to a given position in the streem 
	 *
	 * @Param Pos		The position to seek to
	 */
	void Seek(DWORD Pos) 
	{
		Offset = Pos;
	}

	/**
	 * Serializes bytes in to the stream
	 *
	 * @Param Data			Pointer to byte(s) to be added
	 * @Param ByteCount		# of bytes to add
	 */
	void SerializeBytes( const BYTE* Data, INT ByteCount )
	{
		INT NumBytesToAdd = Offset + ByteCount - Num();
		if( NumBytesToAdd > 0 )
		{
			Add( NumBytesToAdd );
		}
		if( ByteCount )
		{
			appMemcpy( (void*)(GetTypedData() + Offset), Data, ByteCount );
			Offset+=ByteCount;
		}
	}

	/**
	 * The base serialize function.  Originally, this used special casing to manage the functional overload but
	 * that seems to have issues with the PS3.  The solution was to just add 4 normal overloads for handling strings.
	 * @param	Data	The date to serialize
	 */
	template <typename T> 
	void Serialize(const T& Data)
	{
		// only statnded 2 and 4 byte data types are valid. 
		// write custom Serialize routines for anything else
		check(sizeof(T) == 4 || sizeof(T) == 2 || sizeof(T) == 1);
#if BYTE_SWAP_BINARY_UPLOADS
		// only one of the following branches will be compiled in by the template
		if (sizeof(T) == 4)
		{
			// alias the input data as an unsigned long. Note that this is not a cast.
			const DWORD InputBits = *((unsigned long*)&Data);
			// byte swap to generate the output
			const DWORD SwappedBits = BYTESWAP_ORDER32(InputBits);
			// serialize the swapped data
			SerializeBytes((const BYTE*)&SwappedBits, 4);
		}
		else if (sizeof(T) == 2)
		{
			// alias the input data as an unsigned short. Note that this is not a cast.
			const SWORD InputBits = *((unsigned short*)&Data);
			// byte swap to generate the output
			const SWORD SwappedBits = BYTESWAP_ORDER16(InputBits);
			// serialize the swapped data
			SerializeBytes((const BYTE*)&SwappedBits, 2);
		}
		else
		{
			SerializeBytes((const BYTE*)&Data, sizeof(T));
		}
#else
		SerializeBytes((const BYTE*)&Data, sizeof(T));
#endif
	}

	/**
	 * Serializes a string
	 *
	 * @Param	Data	The string to serialize
	 */
	void Serialize(const FString& Data)
	{
		TArray<BYTE> AnsiBuffer;
		INT AnsiBufferSize = Data.Len();
		// Copy the data into the buffer in ANSI char form
		AnsiBuffer.Empty(AnsiBufferSize);
		AnsiBuffer.AddZeroed(AnsiBufferSize);
		appMemcpy(AnsiBuffer.GetTypedData(),(const BYTE*)TCHAR_TO_ANSI(*Data),AnsiBufferSize);
		// write the string length
		Serialize(AnsiBufferSize);
		// write the string characters (note that the terminating NULL is not included)
		if (AnsiBufferSize > 0)
		{
			SerializeBytes((const BYTE*)AnsiBuffer.GetTypedData(), AnsiBufferSize);
		}
	}

	/**
	 * Serialize a string
	 */
	void Serialize(FString& Data)
	{
		Serialize((const FString&) Data);
	}

	/**
	 * Serialize a string
	 *
	 * @Param	Data	The string to serialize
	 */
	void Serialize(const FStringNoInit& Data)
	{
		Serialize((const FString&) Data);
	}

	/**
	 * Serialize a string
	 */
	void Serialize(FStringNoInit& Data)
	{
		Serialize((const FString&) Data);
	}

protected:
	/**
	 * The offset we are currently at
	 */
	INT Offset;
};

/**
 * Sends the data contained within the gameplay events object to the online server for statistics
 *
 * @param Events the object that has the set of events in it
 *
 * @return true if the async send started ok, false otherwise
 */
UBOOL UOnlineEventsInterfaceMcp::UploadGameplayEventsData(UOnlineGameplayEvents* Events)
{
#if WITH_UE3_NETWORKING
	if (Events)
	{
		//@STATS
		if (bBinaryStats)
		{
			// make an archive to serialize into
			FSimpleStream StreamWriter(1024);
			// make space for stream contents
			INT PackageSizeOffset = 0;
			INT CurrentPosition = 0;
			// build the header that contains the payload data
			DWORD AppTitleID = appGetTitleId();
			StreamWriter.Serialize(GEngineVersion);
			StreamWriter.Serialize(AppTitleID);
			StreamWriter.Serialize(Events->GameplaySessionID.String());
			StreamWriter.Serialize(appGetPlatformType());
			StreamWriter.Serialize(appGetLanguageExt());
			// write a placeholder for the size of the package
			PackageSizeOffset = StreamWriter.Tell();
			StreamWriter.Serialize(PackageSizeOffset);
			// write each player entry
			const INT NumPlayers = Events->PlayerList.Num();
			StreamWriter.Serialize(NumPlayers);
			for (INT Index = 0; Index < NumPlayers; Index++)
			{
				FPlayerInformation& Player = Events->PlayerList(Index);
				if (Player.bIsBot)
				{
					const INT Bot = 1;
					StreamWriter.Serialize(FormatAsString(Events->PlayerList(Index).UniqueId));
					StreamWriter.Serialize(Player.ControllerName);
					StreamWriter.Serialize(Bot);
				}
				else
				{
					const INT Bot = 0;
					StreamWriter.Serialize(FormatAsString(Events->PlayerList(Index).UniqueId));
					StreamWriter.Serialize(Player.PlayerName);
					StreamWriter.Serialize(Bot);
				}
			}
			// write each event name
			const INT NumEventNames = Events->EventNames.Num();
			StreamWriter.Serialize(NumEventNames);
			for (INT Index = 0; Index < NumEventNames; Index++)
			{
				FString ActualName = Events->EventNames(Index).ToString();
				StreamWriter.Serialize(ActualName);
			}
			// write each event description
			const INT NumEventDescriptions = Events->EventDescList.Num();
			StreamWriter.Serialize(NumEventDescriptions);
			for (INT Index = 0; Index < NumEventDescriptions; Index++)
			{
				StreamWriter.Serialize(Events->EventDescList(Index));
			}
			// write each player event and inject all associated Gameplay events
			const INT NumPlayerEvents = Events->PlayerEvents.Num();
			const INT NumGameplayEvents = Events->GameplayEvents.Num();
			INT LowestGameplayEventIndex = 0;
			StreamWriter.Serialize(NumPlayerEvents);
			for (INT PlayerEventIndex = 0; PlayerEventIndex < NumPlayerEvents; ++PlayerEventIndex)
			{
				const FPlayerEvent &PlayerEvt = Events->PlayerEvents(PlayerEventIndex);
				const INT ThisPlayerIdx = PlayerEvt.PlayerIndexAndYaw >> 16;
				StreamWriter.Serialize(PlayerEvt.EventTime);
				StreamWriter.Serialize(PlayerEvt.EventLocation.X);
				StreamWriter.Serialize(PlayerEvt.EventLocation.Y);
				StreamWriter.Serialize(PlayerEvt.EventLocation.Z);
				WORD PlayerIdx = PlayerEvt.PlayerIndexAndYaw >> 16;
				WORD Yaw = PlayerEvt.PlayerIndexAndYaw & 0xffff;
				WORD Pitch = PlayerEvt.PlayerPitchAndRoll >> 16;
				WORD Roll = PlayerEvt.PlayerPitchAndRoll & 0xffff;
				StreamWriter.Serialize(PlayerIdx);
				StreamWriter.Serialize(Yaw);
				StreamWriter.Serialize(Pitch);
				StreamWriter.Serialize(Roll);
				// write a placeholder for the gameplay event count
				INT TotalGameplayEvents = 0;
				INT TotalGameplayEventsOffset = StreamWriter.Tell();
				StreamWriter.Serialize(TotalGameplayEvents);
				// find all associated events
				for (INT GameplayEventIndex = LowestGameplayEventIndex; GameplayEventIndex < NumGameplayEvents; ++GameplayEventIndex)
				{
					const FGameplayEvent& GameplayEvt = Events->GameplayEvents(GameplayEventIndex);
					const INT ParentPlayerEventIndex = GameplayEvt.PlayerEventAndTarget >> 16;
					if (ParentPlayerEventIndex == PlayerEventIndex)
					{
						BYTE EventNameIndex = ((GameplayEvt.EventNameAndDesc >> 16) & 0xff);
						WORD EventDescIndex = GameplayEvt.EventNameAndDesc & 0xffff;
						WORD TargetIndex = GameplayEvt.PlayerEventAndTarget & 0xffff;
						StreamWriter.Serialize(EventNameIndex);
						StreamWriter.Serialize(EventDescIndex);
						StreamWriter.Serialize(TargetIndex);
						++TotalGameplayEvents;
						if (GameplayEventIndex == LowestGameplayEventIndex)
						{
							++LowestGameplayEventIndex;
						}
					}
				}
				// jump back and write the TotalGameplayEvents
				CurrentPosition = StreamWriter.Tell();
				StreamWriter.Seek(TotalGameplayEventsOffset);
				StreamWriter.Serialize(TotalGameplayEvents);
				StreamWriter.Seek(CurrentPosition);
			}
			// go back and write the size of the entire package
			CurrentPosition = StreamWriter.Tell();
			INT PackageSize = CurrentPosition - PackageSizeOffset;
			StreamWriter.Seek(PackageSizeOffset);
			StreamWriter.Serialize(PackageSize);
			StreamWriter.Seek(CurrentPosition);
			debugf(NAME_DevOnline,TEXT("Uploading Binary Stats (%i Bytes)\r\n"), StreamWriter.Num());
			// Now POST the data and have a response parsed
			return UploadBinaryPayload(EUT_GenericStats, StreamWriter);
		}
		else
		{
			// Build the outer XML wrapper that indicates the type of payload
			FString XmlPayload = FString::Printf(TEXT("<GameplaySession Ver=\"%d\" TitleId=\"%d\" SessId=\"%s\" PlatformId=\"%d\" Localization=\"%s\">\r\n"),
				GEngineVersion,
				appGetTitleId(),
				*Events->GameplaySessionID.String(),
				(DWORD)appGetPlatformType(),
				*appGetLanguageExt());
			XmlPayload += TEXT("\t<Players>\r\n");
			// Now add the player information for the session
			for (INT Index = 0; Index < Events->PlayerList.Num(); Index++)
			{
				if (Events->PlayerList(Index).bIsBot)
				{
					XmlPayload += FString::Printf(TEXT("\t\t<Player Name=\"%s\" UniqueId=\"\" AI=\"1\"/>\r\n"),
						*Events->PlayerList(Index).ControllerName);
				}
				else
				{
					XmlPayload += FString::Printf(TEXT("\t\t<Player Name=\"%s\" UniqueId=\"%s\" AI=\"0\"/>\r\n"),
						*Events->PlayerList(Index).PlayerName,
						*FormatAsString(Events->PlayerList(Index).UniqueId),
						*Events->PlayerList(Index).ControllerName);
				}
			}
			XmlPayload += TEXT("\t</Players>\r\n");
			// Add the data that doesn't change (event names and descriptions)
			XmlPayload += TEXT("\t<EventNames>\r\n");
			for (INT Index = 0; Index < Events->EventNames.Num(); Index++)
			{
				XmlPayload += FString::Printf(TEXT("\t\t<Name>%s</Name>\r\n"),
					*Events->EventNames(Index).ToString());
			}
			XmlPayload += TEXT("\t</EventNames>\r\n");
			XmlPayload += TEXT("\t<EventDescs>\r\n");
			for (INT Index = 0; Index < Events->EventDescList.Num(); Index++)
			{
				XmlPayload += FString::Printf(TEXT("\t\t<Name>%s</Name>\r\n"),
					*Events->EventDescList(Index));
			}
			XmlPayload += TEXT("\t</EventDescs>\r\n");
			XmlPayload += TEXT("\t<PlayerEvents>\r\n");
			// Finally, add the events that make up the session
			const INT NumPlayerEvents = Events->PlayerEvents.Num();
			const INT NumGameplayEvents = Events->GameplayEvents.Num();
			INT LowestGameplayEventIndex = 0;
			for (INT Index = 0; Index < Events->PlayerEvents.Num(); Index++)
			{
				const FPlayerEvent &PlayerEvt = Events->PlayerEvents(Index);
				const INT PlayerIdx = PlayerEvt.PlayerIndexAndYaw >> 16;
				const INT Yaw = PlayerEvt.PlayerIndexAndYaw && 0xffff;
				const INT Pitch = PlayerEvt.PlayerPitchAndRoll >> 16;
				const INT Roll = PlayerEvt.PlayerPitchAndRoll && 0xffff;
				// Build the entry and append the gameplay events for this timestamped period
				XmlPayload += FString::Printf(TEXT("\t\t<PlayerEvent X=\"%d\" Y=\"%d\" Z=\"%d\" Yaw=\"%d\" Pitch=\"%d\" Roll=\"%d\" PlayerIdx=\"%d\" Time=\"%.1f\">\r\n"),
					appTrunc(PlayerEvt.EventLocation.X),
					appTrunc(PlayerEvt.EventLocation.Y),
					appTrunc(PlayerEvt.EventLocation.Z),
					Yaw,
					Pitch,
					Roll,
					PlayerIdx,
					PlayerEvt.EventTime);
				// find all associated events
				for (INT GameplayEventIndex = LowestGameplayEventIndex; GameplayEventIndex < NumGameplayEvents; ++GameplayEventIndex)
				{
					const FGameplayEvent& GameplayEvt = Events->GameplayEvents(GameplayEventIndex);
					const INT ParentPlayerEventIndex = GameplayEvt.PlayerEventAndTarget >> 16;
					if (ParentPlayerEventIndex == Index)
					{
						BYTE EventNameIndex = ((GameplayEvt.EventNameAndDesc >> 16) & 0xff);
						WORD EventDescIndex = GameplayEvt.EventNameAndDesc & 0xffff;
						WORD TargetIndex = GameplayEvt.PlayerEventAndTarget & 0xffff;
						XmlPayload += FString::Printf(TEXT("\t\t\t<Event Name=\"%d\" Desc=\"%d\" TargetIdx=\"%d\"/>\r\n"),
						EventNameIndex,
						EventDescIndex,
						TargetIndex);
						if (GameplayEventIndex == LowestGameplayEventIndex)
						{
							++LowestGameplayEventIndex;
						}
					}
				}
				XmlPayload += TEXT("\t\t</PlayerEvent>\r\n");
			}
			XmlPayload += TEXT("\t</PlayerEvents>\r\n");
			// Close the XML so it is well formed
			XmlPayload += TEXT("</GameplaySession>\r\n");
			// Now POST the data and have a response parsed
			appSaveStringToFile(*XmlPayload,TEXT("\\Session.xml"));
			return UploadPayload(EUT_GenericStats,XmlPayload);
		}
	}
#endif
	return FALSE;
}
