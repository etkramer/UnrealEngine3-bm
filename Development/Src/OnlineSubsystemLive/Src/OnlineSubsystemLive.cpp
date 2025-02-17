/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "OnlineSubsystemLive.h"
#include "VoiceInterfaceCommon.h"

IMPLEMENT_CLASS(UOnlineEventsInterfaceMcpLive);
IMPLEMENT_CLASS(UPartyBeaconClientLive);

#if WITH_UE3_NETWORKING

#if WITH_PANORAMA
	#pragma message("Linking Games for Windows Live")
	//NOTE: If you get an error here, make sure the G4WLive directories are in your additional includes/libs
	#pragma comment(lib, "XLive.lib")
#endif

// Change this value anytime the QoS packet format changes
#define QOS_PACKET_VERSION (BYTE)3

/** Static buffer to use when writing stats */
static XSESSION_VIEW_PROPERTIES Views[MAX_VIEWS];
/** Static buffer to use when writing stats */
static XUSER_PROPERTY Stats[MAX_STATS];

IMPLEMENT_CLASS(UOnlineSubsystemLive);

/**
 * Converts the gamestate into a human readable string
 *
 * @param GameState the game state to convert to a string
 *
 * @return a string representation of the game state
 */
inline FString GetOnlineGameStateString(BYTE GameState)
{
#if !FINAL_RELEASE && !SHIPPING_PC_GAME
	if (GameState < OGS_MAX)
	{
		UEnum* OnlineGameStateEnum = FindObject<UEnum>(ANY_PACKAGE,TEXT("EOnlineGameState"),TRUE);
		if (OnlineGameStateEnum != NULL)
		{
			return OnlineGameStateEnum->GetEnum(GameState).ToString();
		}
	}
#endif
	return TEXT("Unknown");
}

#if _DEBUG
/**
 * Logs the set of contexts and properties for debugging purposes
 *
 * @param GameSearch the game to log the information for
 */
void DumpContextsAndProperties(UOnlineGameSearch* GameSearch)
{
	debugf(NAME_DevOnline,TEXT("Search contexts:"));
	// Iterate through all contexts and log them
	for (INT Index = 0; Index < GameSearch->LocalizedSettings.Num(); Index++)
	{
		const FLocalizedStringSetting& Context = GameSearch->LocalizedSettings(Index);
		// Check for wildcard status
		if (GameSearch->IsWildcardStringSetting(Context.Id) == FALSE)
		{
			debugf(NAME_DevOnline,TEXT("0x%08X = %d"),Context.Id,Context.ValueIndex);
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("Using wildcard for 0x%08X"),Context.Id);
		}
	}
	debugf(NAME_DevOnline,TEXT("Search properties:"));
	// Iterate through all properties and log them
	for (INT Index = 0; Index < GameSearch->Properties.Num(); Index++)
	{
		const FSettingsProperty& Property = GameSearch->Properties(Index);
		debugf(NAME_DevOnline,TEXT("0x%08X (%d) = %s"),Property.PropertyId,
			Property.Data.Type,*Property.Data.ToString());
	}
}
#endif

/**
 * Given an SettingsData object determines the size we'll be sending
 *
 * @param SettingsData the object to inspect
 */
static inline DWORD GetSettingsDataSize(const FSettingsData& SettingsData)
{
	DWORD SizeOfData = sizeof(DWORD);
	// Figure out if we have a type that isn't the default size
	switch (SettingsData.Type)
	{
		case SDT_Int64:
		case SDT_Double:
		{
			SizeOfData = sizeof(DOUBLE);
			break;
		}
		case SDT_Blob:
		{
			// Read the setting that is set by Value1
			SizeOfData = (DWORD)SettingsData.Value1;
			break;
		}
		case SDT_String:
		{
			// The null terminator needs to be counted too
			SizeOfData = ((DWORD)SettingsData.Value1 + 1) * sizeof(TCHAR);
			break;
		}
	}
	return SizeOfData;
}

/**
 * Given an SettingsData object determines the data pointer to give to Live
 *
 * @param SettingsData the object to inspect
 */
static inline const void* GetSettingsDataPointer(const FSettingsData& SettingsData)
{
	const void* DataPointer = NULL;
	// Determine where to get the pointer from
	switch (SettingsData.Type)
	{
		case SDT_Float:
		case SDT_Int32:
		case SDT_Int64:
		case SDT_Double:
		{
			DataPointer = &SettingsData.Value1;
			break;
		}
		case SDT_Blob:
		case SDT_String:
		{
			DataPointer = SettingsData.Value2;
			break;
		}
	}
	return DataPointer;
}

/**
 * Copies the data from the Live structure to the Epic structure
 *
 * @param Data the Epic structure that is the destination
 * @param XData the Live strucuture that is the source
 */
static inline void CopyXDataToSettingsData(FSettingsData& Data,const XUSER_DATA& XData)
{
	// Copy based upon data type
	switch (XData.type)
	{
		case XUSER_DATA_TYPE_FLOAT:
		{
			Data.SetData(XData.fData);
			break;
		}
		case XUSER_DATA_TYPE_INT32:
		{
			Data.SetData((INT)XData.nData);
			break;
		}
		case XUSER_DATA_TYPE_INT64:
		{
			Data.SetData((QWORD)XData.i64Data);
			break;
		}
		case XUSER_DATA_TYPE_DOUBLE:
		{
			Data.SetData(XData.dblData);
			break;
		}
		case XUSER_DATA_TYPE_BINARY:
		{
			// Deep copy the data
			Data.SetData(XData.binary.cbData,XData.binary.pbData);
			break;
		}
		case XUSER_DATA_TYPE_UNICODE:
		{
			Data.SetData(XData.string.pwszData);
			break;
		}
		case XUSER_DATA_TYPE_DATETIME:
		{
			Data.SetData(XData.ftData.dwLowDateTime,XData.ftData.dwHighDateTime);
			break;
		}
	}
}

/** Global mapping of Unreal enum values to Live values */
const DWORD LiveProfileSettingIDs[] =
{
	// Live read only settings
	XPROFILE_OPTION_CONTROLLER_VIBRATION,
	XPROFILE_GAMER_YAXIS_INVERSION,
	XPROFILE_GAMERCARD_CRED,
	XPROFILE_GAMERCARD_REP,
	XPROFILE_OPTION_VOICE_MUTED,
	XPROFILE_OPTION_VOICE_THRU_SPEAKERS,
	XPROFILE_OPTION_VOICE_VOLUME,
	XPROFILE_GAMERCARD_PICTURE_KEY,
	XPROFILE_GAMERCARD_TITLES_PLAYED,
	XPROFILE_GAMERCARD_MOTTO,
	XPROFILE_GAMERCARD_ACHIEVEMENTS_EARNED,
	XPROFILE_GAMER_DIFFICULTY,
	XPROFILE_GAMER_CONTROL_SENSITIVITY,
	XPROFILE_GAMER_PREFERRED_COLOR_FIRST,
	XPROFILE_GAMER_PREFERRED_COLOR_SECOND,
	XPROFILE_GAMER_ACTION_AUTO_AIM,
	XPROFILE_GAMER_ACTION_AUTO_CENTER,
	XPROFILE_GAMER_ACTION_MOVEMENT_CONTROL,
	XPROFILE_GAMER_RACE_TRANSMISSION,
	XPROFILE_GAMER_RACE_CAMERA_LOCATION,
	XPROFILE_GAMER_RACE_BRAKE_CONTROL,
	XPROFILE_GAMER_RACE_ACCELERATOR_CONTROL,
	XPROFILE_GAMERCARD_TITLE_CRED_EARNED,
	XPROFILE_GAMERCARD_TITLE_ACHIEVEMENTS_EARNED
};

/**
 * Determines if the specified ID is outside the range of standard Live IDs
 *
 * @param Id the id in question
 *
 * @return TRUE of the ID is game specific, FALSE otherwise
 */
FORCEINLINE UBOOL IsProfileSettingIdGameOnly(DWORD Id)
{
	return Id >= PSI_EndLiveIds;
}

/**
 * Converts an EProfileSettingID enum to the Live equivalent
 *
 * @param EnumValue the value to convert to a Live value
 *
 * @return the Live specific value for the specified enum
 */
inline DWORD ConvertToLiveValue(EProfileSettingID EnumValue)
{
	check(ARRAY_COUNT(LiveProfileSettingIDs) == PSI_EndLiveIds - 1 &&
		"Live profile mapping array isn't in synch");
	check(EnumValue > PSI_Unknown && EnumValue < PSI_EndLiveIds);
	return LiveProfileSettingIDs[EnumValue - 1];
}

/**
 * Converts a Live profile id to an EProfileSettingID enum value
 *
 * @param LiveValue the Live specific value to convert
 *
 * @return the Unreal enum value representing that Live profile id
 */
EProfileSettingID ConvertFromLiveValue(DWORD LiveValue)
{
	BYTE EnumValue = PSI_Unknown;
	// Figure out which enum value to use
	switch (LiveValue)
	{
		case XPROFILE_OPTION_CONTROLLER_VIBRATION:
		{
			EnumValue = 1;
			break;
		}
		case XPROFILE_GAMER_YAXIS_INVERSION:
		{
			EnumValue = 2;
			break;
		}
		case XPROFILE_GAMERCARD_CRED:
		{
			EnumValue = 3;
			break;
		}
		case XPROFILE_GAMERCARD_REP:
		{
			EnumValue = 4;
			break;
		}
		case XPROFILE_OPTION_VOICE_MUTED:
		{
			EnumValue = 5;
			break;
		}
		case XPROFILE_OPTION_VOICE_THRU_SPEAKERS:
		{
			EnumValue = 6;
			break;
		}
		case XPROFILE_OPTION_VOICE_VOLUME:
		{
			EnumValue = 7;
			break;
		}
		case XPROFILE_GAMERCARD_PICTURE_KEY:
		{
			EnumValue = 8;
			break;
		}
		case XPROFILE_GAMERCARD_TITLES_PLAYED:
		{
			EnumValue = 9;
			break;
		}
		case XPROFILE_GAMERCARD_MOTTO:
		{
			EnumValue = 10;
			break;
		}
		case XPROFILE_GAMERCARD_ACHIEVEMENTS_EARNED:
		{
			EnumValue = 11;
			break;
		}
		case XPROFILE_GAMER_DIFFICULTY:
		{
			EnumValue = 12;
			break;
		}
		case XPROFILE_GAMER_CONTROL_SENSITIVITY:
		{
			EnumValue = 13;
			break;
		}
		case XPROFILE_GAMER_PREFERRED_COLOR_FIRST:
		{
			EnumValue = 14;
			break;
		}
		case XPROFILE_GAMER_PREFERRED_COLOR_SECOND:
		{
			EnumValue = 15;
			break;
		}
		case XPROFILE_GAMER_ACTION_AUTO_AIM:
		{
			EnumValue = 16;
			break;
		}
		case XPROFILE_GAMER_ACTION_AUTO_CENTER:
		{
			EnumValue = 17;
			break;
		}
		case XPROFILE_GAMER_ACTION_MOVEMENT_CONTROL:
		{
			EnumValue = 18;
			break;
		}
		case XPROFILE_GAMER_RACE_TRANSMISSION:
		{
			EnumValue = 19;
			break;
		}
		case XPROFILE_GAMER_RACE_CAMERA_LOCATION:
		{
			EnumValue = 20;
			break;
		}
		case XPROFILE_GAMER_RACE_BRAKE_CONTROL:
		{
			EnumValue = 21;
			break;
		}
		case XPROFILE_GAMER_RACE_ACCELERATOR_CONTROL:
		{
			EnumValue = 22;
			break;
		}
		case XPROFILE_GAMERCARD_TITLE_CRED_EARNED:
		{
			EnumValue = 23;
			break;
		}
		case XPROFILE_GAMERCARD_TITLE_ACHIEVEMENTS_EARNED:
		{
			EnumValue = 24;
			break;
		}
	};
	return (EProfileSettingID)EnumValue;
}

/**
 * Converts the Unreal enum values into an array of Live values
 *
 * @param ProfileIds the Unreal values to convert
 * @param DestIds an out array that gets the converted data
 */
static inline void BuildLiveProfileReadIDs(const TArray<DWORD>& ProfileIds,
	DWORD* DestIds)
{
	// Loop through using the helper to convert
	for (INT Index = 0; Index < ProfileIds.Num(); Index++)
	{
		DestIds[Index] = ConvertToLiveValue((EProfileSettingID)ProfileIds(Index));
	}
}

#if !FINAL_RELEASE
/**
 * Validates that the specified write stats object has the proper number
 * of views and stats per view
 *
 * @param WriteStats the object to validate
 *
 * @return TRUE if acceptable, FALSE otherwise
 */
UBOOL IsValidStatsWrite(UOnlineStatsWrite* WriteStats)
{
	// Validate the number of views
	if (WriteStats->ViewIds.Num() > 0 && WriteStats->ViewIds.Num() <= 5)
	{
		return WriteStats->Properties.Num() >= 0 &&
			WriteStats->Properties.Num() < 64;
	}
	return FALSE;
}
#endif

/**
 * Finds the player controller associated with the specified index
 *
 * @param Index the id of the user to find
 *
 * @return the player controller for that id
 */
inline APlayerController* GetPlayerControllerFromUserIndex(INT Index)
{
	// Find the local player that has the same controller id as the index
	for (FPlayerIterator It(GEngine); It; ++It)
	{
		ULocalPlayer* Player = *It;
		if (Player->ControllerId == Index)
		{
			// The actor is the corresponding player controller
			return Player->Actor;
		}
	}
	return NULL;
}

/**
 * Copies the properties we are interested in from the source object to the destination
 *
 * @param Dest the target to copy to
 * @param Src the object to copy from
 */
inline void CopyGameSettings(UOnlineGameSettings* Dest,UOnlineGameSettings* Src)
{
	if (Dest && Src)
	{
		// Copy the session size information
		Dest->NumPublicConnections = Src->NumPublicConnections;
		Dest->NumPrivateConnections = Src->NumPrivateConnections;
		// Copy the flags that will be set on the session
		Dest->bUsesStats = Src->bUsesStats;
		Dest->bAllowJoinInProgress = Src->bAllowJoinInProgress;
		Dest->bAllowInvites = Src->bAllowInvites;
		Dest->bUsesPresence = Src->bUsesPresence;
		Dest->bAllowJoinViaPresence = Src->bAllowJoinViaPresence;
		Dest->bUsesArbitration = Src->bUsesArbitration;
		// Update the properties/contexts
		Dest->UpdateStringSettings(Src->LocalizedSettings);
		Dest->UpdateProperties(Src->Properties);
	}
}

/**
 * @return TRUE if this is the server, FALSE otherwise
 */
inline UBOOL IsServer(void)
{
	return GWorld &&
		GWorld->GetWorldInfo() &&
		GWorld->GetWorldInfo()->NetMode < NM_Client;
}

/**
 * Routes the call to the function on the subsystem for parsing search results
 *
 * @param LiveSubsystem the object to make the final call on
 */
UBOOL FLiveAsyncTaskSearch::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	UBOOL bDelete = FALSE;
	DWORD Result = GetCompletionCode();
	if (Result == ERROR_SUCCESS)
	{
		// See if we are just waiting for live to send back matchmaking
		if (bIsWaitingForLive == TRUE)
		{
			bIsWaitingForLive = FALSE;
			// Parse the Live results
			LiveSubsystem->ParseSearchResults(LiveSubsystem->GameSearch,
				*(FLiveAsyncTaskDataSearch*)TaskData);
			// Kick off QoS searches for the servers that were returned
			if (LiveSubsystem->CheckServersQoS((FLiveAsyncTaskDataSearch*)TaskData) == FALSE)
			{
				// QoS failed so don't wait
				bDelete = TRUE;
			}
		}
		// We are waiting on QoS results
		else
		{
			FLiveAsyncTaskDataSearch* AsyncData = (FLiveAsyncTaskDataSearch*)TaskData;
			// Make sure we have data and then check it for completion
			XNQOS* QosData = *AsyncData->GetXNQOS();
			if (QosData != NULL)
			{
				// Check if all results are back
				if (QosData->cxnqosPending == 0)
				{
					// Have the subsystem update its search results data
					LiveSubsystem->ParseQoSResults(QosData);
					bDelete = TRUE;
				}
			}
			else
			{
				debugfLiveSlow(NAME_DevOnline,TEXT("NULL XNQOS pointer, aborting QoS code"));
				// Something is messed up
				bDelete = TRUE;
			}
		}
	}
	else
	{
		// Stuff is broked
		debugf(NAME_DevOnline,TEXT("XSessionSearch() completed with error 0x%08X"),Result);
		bDelete = TRUE;
	}
	// Mark the search as complete
	if (bDelete == TRUE && LiveSubsystem->GameSearch)
	{
		LiveSubsystem->GameSearch->bIsSearchInProgress = FALSE;
	}
	return bDelete;
}

/**
 * Coalesces the game settings data into one buffer instead of 3
 */
void FLiveAsyncTaskDataReadProfileSettings::CoalesceGameSettings(void)
{
	PXUSER_READ_PROFILE_SETTING_RESULT ReadResults = GetGameSettingsBuffer();
	// Copy each binary buffer into the working buffer
	for (DWORD Index = 0; Index < ReadResults->dwSettingsLen; Index++)
	{
		XUSER_PROFILE_SETTING& LiveSetting = ReadResults->pSettings[Index];
		// Don't bother copying data for no value settings and the data
		// should only be binary. Ignore otherwise
		if (LiveSetting.source != XSOURCE_NO_VALUE &&
			LiveSetting.data.type == XUSER_DATA_TYPE_BINARY)
		{
			// Figure out how much data to copy
			appMemcpy(&WorkingBuffer[WorkingBufferUsed],
				LiveSetting.data.binary.pbData,
				LiveSetting.data.binary.cbData);
			// Increment our offset for the next copy
			WorkingBufferUsed += LiveSetting.data.binary.cbData;
		}
	}
}

/**
 * Reads the online profile settings from the buffer into the specified array
 *
 * @param Settings the array to populate from the game settings
 */
void FLiveAsyncTaskReadProfileSettings::SerializeGameSettings(TArray<FOnlineProfileSetting>& Settings)
{
	FLiveAsyncTaskDataReadProfileSettings* Data =(FLiveAsyncTaskDataReadProfileSettings*)TaskData;
	// Don't bother if the buffer wasn't there
	if (Data->GetWorkingBufferSize() > 0)
	{
		FProfileSettingsReader Reader(FALSE,Data->GetWorkingBuffer(),Data->GetWorkingBufferSize());
		// Serialize the profile from that array
		if (Reader.SerializeFromBuffer(Settings) == FALSE)
		{
			// Empty the array if it failed to read
			Settings.Empty();
		}
	}
}

/**
 * Routes the call to the function on the subsystem for parsing the results
 *
 * @param LiveSubsystem the object to make the final call on
 */
UBOOL FLiveAsyncTaskReadProfileSettings::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	UBOOL bDone = FALSE;
	FLiveAsyncTaskDataReadProfileSettings* Data = (FLiveAsyncTaskDataReadProfileSettings*)TaskData;
	DWORD Result = GetCompletionCode();
	if (Result == ERROR_SUCCESS)
	{
		// Figure out what our next step is
		if (Data->GetCurrentAction() == FLiveAsyncTaskDataReadProfileSettings::ReadingGameSettings)
		{
			// Build one big buffer out of the returned data
			Data->CoalesceGameSettings();
			TArray<FOnlineProfileSetting> Settings;
			// Serialize from the buffer
			SerializeGameSettings(Settings);
			TArray<DWORD> MissingSettingIds;
			// Get the Ids that the game is interested in
			DWORD* Ids = Data->GetIds();
			DWORD NumIds = Data->GetIdsCount();
			// For each ID that we need to read
			for (DWORD IdIndex = 0; IdIndex < NumIds; IdIndex++)
			{
				DWORD SettingId = Ids[IdIndex];
				UBOOL bFound = FALSE;
				// Search the resulting array for the data
				for (INT Index = 0; Index < Settings.Num(); Index++)
				{
					const FOnlineProfileSetting& Setting = Settings(Index);
					// If found, copy the data from the array to the profile data
					if (Setting.ProfileSetting.PropertyId == SettingId)
					{
						// Place the data in the user's profile results array
						LiveSubsystem->AppendProfileSetting(Data->GetUserIndex(),Setting);
						bFound = TRUE;
						break;
					}
				}
				// The requested ID wasn't in the game settings list, so add the
				// ID to the list we need to read from Live
				if (bFound == FALSE)
				{
					MissingSettingIds.AddItem(SettingId);
				}
			}
			// If there are IDs we need to read from Live and/or the game defaults
			if (MissingSettingIds.Num() > 0)
			{
				// Fill any game specific settings that aren't Live aware from the defaults
				LiveSubsystem->ProcessProfileDefaults(Data->GetUserIndex(),MissingSettingIds);
				// The game defaults may have fulfilled the remaining ids
				if (MissingSettingIds.Num() > 0)
				{
					check(MissingSettingIds.Num() <= (INT)Data->GetIdsCount());
					// Map the unreal IDs to Live ids
					BuildLiveProfileReadIDs(MissingSettingIds,Data->GetIds());
					// Allocate a buffer for the ones we need to read from Live
					Data->AllocateBuffer(MissingSettingIds.Num());
					appMemzero(&Overlapped,sizeof(XOVERLAPPED));
					// Need to indicate the buffer size
					DWORD SizeNeeded = Data->GetIdsSize();
					// Kick off a new read with just the missing ids
					DWORD Return = XUserReadProfileSettings(0,
						Data->GetUserIndex(),
						MissingSettingIds.Num(),
						Data->GetIds(),
						&SizeNeeded,
						Data->GetProfileBuffer(),
						&Overlapped);
					if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
					{
						bDone = FALSE;
						// Mark this task as still processing
						Data->SetCurrentAction(FLiveAsyncTaskDataReadProfileSettings::ReadingLiveSettings);
					}
					else
					{
						debugf(NAME_DevOnline,TEXT("Failed to read Live IDs 0x%08X"),Return);
						bDone = TRUE;
					}
				}
				else
				{
					// All requested profile settings were met
					bDone = TRUE;
				}
			}
			else
			{
				// All requested profile settings were met
				bDone = TRUE;
			}
		}
		else
		{
			// Append anything that comes back
			LiveSubsystem->ParseReadProfileResults(Data->GetUserIndex(),
				Data->GetProfileBuffer());
			bDone = TRUE;
		}
	}
	else
	{
		bDone = TRUE;
		debugf(NAME_DevOnline,TEXT("Profile read failed with 0x%08X"),Result);
		// Set the profile to the defaults in this case
		if (LiveSubsystem->ProfileCache[Data->GetUserIndex()].Profile != NULL)
		{
			LiveSubsystem->ProfileCache[Data->GetUserIndex()].Profile->eventSetToDefaults();
		}
	}
	if (bDone)
	{
		// In case this gets cleared while in progress
		if (LiveSubsystem->ProfileCache[Data->GetUserIndex()].Profile != NULL)
		{
			INT ReadVersion = LiveSubsystem->ProfileCache[Data->GetUserIndex()].Profile->GetVersionNumber();
			// Check the version number and reset to defaults if they don't match
			if (LiveSubsystem->ProfileCache[Data->GetUserIndex()].Profile->VersionNumber != ReadVersion)
			{
				debugfLiveSlow(NAME_DevOnline,
					TEXT("Detected profile version mismatch (%d != %d), setting to defaults"),
					LiveSubsystem->ProfileCache[Data->GetUserIndex()].Profile->VersionNumber,
					ReadVersion);
				LiveSubsystem->ProfileCache[Data->GetUserIndex()].Profile->eventSetToDefaults();
			}
			// Done with the reading, so mark the async state as done
			LiveSubsystem->ProfileCache[Data->GetUserIndex()].Profile->AsyncState = OPAS_None;
		}
	}
#if DEBUG_PROFILE_DATA
	if (LiveSubsystem->ProfileCache[Data->GetUserIndex()].Profile != NULL)
	{
		DumpProfile(LiveSubsystem->ProfileCache[Data->GetUserIndex()].Profile);
	}
#endif
	return bDone;
}

/**
 * Iterates the list of delegates and fires those notifications
 *
 * @param Object the object that the notifications are going to be issued on
 */
void FLiveAsyncTaskReadProfileSettings::TriggerDelegates(UObject* Object)
{
	check(Object);
	// Only fire off the events if there are some registered
	if (ScriptDelegates != NULL)
	{
		BYTE UserIndex = (BYTE)((FLiveAsyncTaskDataReadProfileSettings*)TaskData)->GetUserIndex();
		// Pass in the data that indicates whether the call worked or not
		OnlineSubsystemLive_eventOnReadProfileSettingsComplete_Parms Parms(EC_EventParm);
		Parms.bWasSuccessful = (GetCompletionCode() == 0) ? FIRST_BITFIELD : 0;
		Parms.LocalUserNum = UserIndex;
		// Use the common method to do the work
		TriggerOnlineDelegates(Object,*ScriptDelegates,&Parms);
	}
}

/**
 * Clears the active profile write reference since it is no longer needed
 *
 * @param LiveSubsystem the object to make the final call on
 */
UBOOL FLiveAsyncTaskWriteProfileSettings::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	FLiveAsyncTaskDataWriteProfileSettings* Data = (FLiveAsyncTaskDataWriteProfileSettings*)TaskData;
	// In case this gets cleared while in progress
	if (LiveSubsystem->ProfileCache[Data->GetUserIndex()].Profile != NULL)
	{
		// Done with the writing, so mark the async state
		LiveSubsystem->ProfileCache[Data->GetUserIndex()].Profile->AsyncState = OPAS_None;
	}
	return TRUE;
}

/**
 * Iterates the list of delegates and fires those notifications
 *
 * @param Object the object that the notifications are going to be issued on
 */
void FLiveAsyncTaskWriteProfileSettings::TriggerDelegates(UObject* Object)
{
	check(Object);
	// Only fire off the events if there are some registered
	if (ScriptDelegates != NULL)
	{
		BYTE UserIndex = (BYTE)((FLiveAsyncTaskDataWriteProfileSettings*)TaskData)->GetUserIndex();
		// Pass in the data that indicates whether the call worked or not
		OnlineSubsystemLive_eventOnWriteProfileSettingsComplete_Parms Parms(EC_EventParm);
		Parms.bWasSuccessful = (GetCompletionCode() == 0) ? FIRST_BITFIELD : 0;
		Parms.LocalUserNum = UserIndex;
		// Use the common method to do the work
		TriggerOnlineDelegates(Object,*ScriptDelegates,&Parms);
	}
}

/**
 * Changes the state of the game session to the one specified at construction
 *
 * @param LiveSubsystem the object to make the final call on
 */
UBOOL FLiveAsyncTaskSessionStateChange::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	FNamedSession* Session = LiveSubsystem->GetNamedSession(SessionName);
	if (Session && Session->GameSettings)
	{
		Session->GameSettings->GameState = StateToTransitionTo;
	}
	return TRUE;
}

/**
 * Checks the arbitration flag and issues a session size change if arbitrated
 *
 * @param LiveSubsystem the object to make the final call on
 *
 * @return TRUE if this task should be cleaned up, FALSE to keep it
 */
UBOOL FLiveAsyncTaskStartSession::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	if (bUsesArbitration)
	{
		// Kick off another task to shrink the session size
		LiveSubsystem->ShrinkToArbitratedRegistrantSize(LiveSubsystem->GetNamedSession(SessionName));
	}
	return FLiveAsyncTaskSessionStateChange::ProcessAsyncResults(LiveSubsystem);
}

/**
 * Changes the state of the game session to pending and registers all of the
 * local players
 *
 * @param LiveSubsystem the object to make the final call on
 */
UBOOL FLiveAsyncTaskCreateSession::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	if (bIsCreate)
	{
		// Forward to the subsystem for completion
		LiveSubsystem->FinishCreateOnlineGame(HostingPlayerNum,
			SessionName,
			XGetOverlappedExtendedError(&Overlapped),
			bIsFromInvite);
	}
	else
	{
		// Forward to the subsystem for completion
		LiveSubsystem->FinishJoinOnlineGame(HostingPlayerNum,
			SessionName,
			XGetOverlappedExtendedError(&Overlapped),
			bIsFromInvite);
	}
	return TRUE;
}

/**
 * Routes the call to the function on the subsystem for parsing friends
 * results. Also, continues searching as needed until there are no more
 * friends to read
 *
 * @param LiveSubsystem the object to make the final call on
 *
 * @return TRUE if this task should be cleaned up, FALSE to keep it
 */
UBOOL FLiveAsyncTaskReadFriends::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	FLiveAsyncTaskDataEnumeration* FriendsData = (FLiveAsyncTaskDataEnumeration*)TaskData;
	// Figure out if we are at the end of the list or not
	DWORD Result = XGetOverlappedExtendedError(&Overlapped);
	// If the task completed ok, then parse the results and start another async task
	if (Result == ERROR_SUCCESS)
	{
		DWORD ItemsEnumerated = 0;
		XGetOverlappedResult(&Overlapped,&ItemsEnumerated,FALSE);
		// Figure out which items we were reading
		if (ReadState == ReadingFriendsXuids)
		{
			PXONLINE_FRIEND Friends = (PXONLINE_FRIEND)FriendsData->GetBuffer();
			// Build a list of XUIDs for requesting the presence information
			for (DWORD Index = 0; Index < ItemsEnumerated; Index++)
			{
				PresenceList.AddItem(Friends[Index].xuid);
			}
			// Add the data to the friends cache
			LiveSubsystem->ParseFriendsResults(FriendsData->GetPlayerIndex(),
				(PXONLINE_FRIEND)FriendsData->GetBuffer(),ItemsEnumerated);
			// Zero between uses or results will be incorrect
			appMemzero(&Overlapped,sizeof(XOVERLAPPED));
			// Do another async friends read
			Result = XEnumerate(FriendsData->GetHandle(),
				FriendsData->GetBuffer(),
				FriendsData->GetBufferSize(),
				0,
				&Overlapped);
		}
		else
		{
			// Add the data to the friends cache
			LiveSubsystem->ParseFriendsResults(FriendsData->GetPlayerIndex(),
				(PXONLINE_PRESENCE)FriendsData->GetBuffer(),ItemsEnumerated);
			// Zero between uses or results will be incorrect
			appMemzero(&Overlapped,sizeof(XOVERLAPPED));
			// Do another presence read
			Result = XEnumerate(PresenceHandle,
				FriendsData->GetBuffer(),
				FriendsData->GetBufferSize(),
				0,
				&Overlapped);
		}
	}
	else
	{
		// Check for "no more files"
		if (Result == 0x80070012)
		{
			if (ReadState == ReadingFriendsPresence)
			{
				// Done reading, need to mark the cache as finished
				LiveSubsystem->FriendsCache[FriendsData->GetPlayerIndex()].ReadState = OERS_Done;
			}
			// Done enumerating friends, now read presence
			else
			{
				// Tell the presence code who we care about
				Result = XPresenceSubscribe(FriendsData->GetPlayerIndex(),
					PresenceList.Num(),
					PresenceList.GetTypedData());
				debugf(NAME_DevOnline,
					TEXT("XPresenceSubscribe(%d,%d,ptr) return 0x%08X"),
					FriendsData->GetPlayerIndex(),
					PresenceList.Num(),
					Result);
				DWORD BufferSize = 0;
				// Create a new enumeration for presence info
				Result = XPresenceCreateEnumerator(FriendsData->GetPlayerIndex(),
					PresenceList.Num(),
					PresenceList.GetTypedData(),
					0,
					MAX_FRIENDS,
					&BufferSize,
					&PresenceHandle);
				debugf(NAME_DevOnline,
					TEXT("XPresenceCreateEnumerator(%d,%d,ptr,0,%d,%d,handle) return 0x%08X"),
					FriendsData->GetPlayerIndex(),
					PresenceList.Num(),
					MAX_FRIENDS,
					BufferSize,
					Result);
				check(BufferSize < FriendsData->GetBufferSize());
				if (Result == ERROR_SUCCESS)
				{
					// Zero between uses or results will be incorrect
					appMemzero(&Overlapped,sizeof(XOVERLAPPED));
					// Have the enumeration start reading data
					Result = XEnumerate(PresenceHandle,
						FriendsData->GetBuffer(),
						FriendsData->GetBufferSize(),
						0,
						&Overlapped);
				}
				// Switch the read state so that we know when to end
				ReadState = ReadingFriendsPresence;
			}
		}
		else
		{
			// Mark it as in error
			LiveSubsystem->FriendsCache[FriendsData->GetPlayerIndex()].ReadState = OERS_Failed;
		}
	}
	// When this is true, there is no more data left to read
	return Result != ERROR_SUCCESS && Result != ERROR_IO_PENDING;
}

/**
 * Routes the call to the function on the subsystem for parsing content
 * results. Also, continues searching as needed until there are no more
 * content to read
 *
 * @param LiveSubsystem the object to make the final call on
 *
 * @return TRUE if this task should be cleaned up, FALSE to keep it
 */
UBOOL FLiveAsyncTaskReadContent::ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem)
{
//@todo  joeg -- Determine if Panorama is going to support this and then re-hook up
#if CONSOLE
	FLiveAsyncTaskContent* ContentTaskData = (FLiveAsyncTaskContent*)TaskData;
	
	// what is our current state?
	DWORD Result = XGetOverlappedExtendedError(&Overlapped);

	// Zero between uses or results will be incorrect
	appMemzero(&Overlapped,sizeof(XOVERLAPPED));

	// look to see how our enumeration is progressing
	if (TaskMode == CTM_Enumerate)
	{
		// If the task completed ok, then parse the results and start another async task to open the content
		if (Result == ERROR_SUCCESS)
		{
// @todo josh: support iterating over more than one item, right now FLiveAsyncTaskDataEnumeration only supports 1 read
			// process a piece of content found by the enumerator
			XCONTENT_DATA* Content = (XCONTENT_DATA*)ContentTaskData->GetBuffer();

			// create a virtual drive that the content will be mapped to (number is unimportant, just needs to be unique)
			static INT ContentNum = 0;
			FString ContentDrive = FString::Printf(TEXT("DLC%d"), ContentNum++);

			// open up the package (make sure it's openable)
			DWORD Return = XContentCreate(ContentTaskData->GetPlayerIndex(), TCHAR_TO_ANSI(*ContentDrive), 
				Content, XCONTENTFLAG_OPENEXISTING, NULL, NULL, &Overlapped);

			if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
			{
				// remember the data for the async task
				ContentTaskData->SetContentDrive(ContentDrive);
				ContentTaskData->SetFriendlyName(Content->szDisplayName);

				// switch modes
				TaskMode = CTM_Create;
			}
			// if we failed to open the content, then kick off another enumeration
			else
			{
				// Do another async content read
				Result = XEnumerate(ContentTaskData->GetHandle(), ContentTaskData->GetBuffer(), sizeof(XCONTENT_DATA), 0, &Overlapped);
			}
		}
		else
		{
			// Done reading, need to mark the cache as finished
			LiveSubsystem->ContentCache[ContentTaskData->GetPlayerIndex()].ReadState = OERS_Done;
		}
	}
	// 
	else
	{
		// If the task completed ok, then parse the results and start another enumeration
		if (Result == ERROR_SUCCESS)
		{
			// get the cache to fill out
			FContentListCache& Cache = LiveSubsystem->ContentCache[ContentTaskData->GetPlayerIndex()];
			// add a new empty content structure
			INT Index = Cache.Content.AddZeroed(1);

			// find the newly added content
			FOnlineContent& NewContent = Cache.Content(Index);
			// friendly name is the displayable for the content (not a map name)
			NewContent.FriendlyName = ContentTaskData->GetFriendlyName();
			// remember the virtual drive for the content (so we can close it later)
		// @todo josh: Close the content when the content goes away (user logs out)
			NewContent.ContentPath = ContentTaskData->GetContentDrive();

			// find all the packages in the content
			appFindFilesInDirectory(NewContent.ContentPackages, *(ContentTaskData->GetContentDrive() + TEXT(":\\")), TRUE, FALSE);

			// find all the non-packages in the content
			appFindFilesInDirectory(NewContent.ContentFiles, *(ContentTaskData->GetContentDrive() + TEXT(":\\")), FALSE, TRUE);

			debugfLiveSlow(TEXT("Using Downloaded Content Package '%s'"), *NewContent.FriendlyName);

			// let the system process the content
			LiveSubsystem->ProcessDownloadedContent(NewContent);

			// Do another async content read
			DWORD Return = XEnumerate(ContentTaskData->GetHandle(), ContentTaskData->GetBuffer(), sizeof(XCONTENT_DATA), 0, &Overlapped);

			if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
			{
				// switch modes
				TaskMode = CTM_Enumerate;
			}
			// if this failed, we need to stop
			else
			{
				// Done reading, need to mark the cache as finished
				LiveSubsystem->ContentCache[ContentTaskData->GetPlayerIndex()].ReadState = OERS_Done;
			}
		}
	}

	// When this is true, there is no more data left to read
	return LiveSubsystem->ContentCache[ContentTaskData->GetPlayerIndex()].ReadState == OERS_Done;
#else
	return TRUE;
#endif
}

#if !WITH_PANORAMA
/**
 * Copies the download query results into the per user storage on the subsystem
 *
 * @param LiveSubsystem the object to make the final call on
 *
 * @return TRUE if this task should be cleaned up, FALSE to keep it
 */
UBOOL FLiveAsyncTaskQueryDownloads::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	FLiveAsyncTaskDataQueryDownloads* Data = (FLiveAsyncTaskDataQueryDownloads*)TaskData;
	// Copy to our cached data for this user
	LiveSubsystem->ContentCache[Data->GetUserIndex()].NewDownloadCount = Data->GetQuery()->dwNewOffers;
	LiveSubsystem->ContentCache[Data->GetUserIndex()].TotalDownloadCount = Data->GetQuery()->dwTotalOffers;
	return TRUE;
}
#endif

/**
 * Copies the resulting string into subsytem buffer. Optionally, will start
 * another async task to validate the string if requested
 *
 * @param LiveSubsystem the object to make the final call on
 *
 * @return TRUE if this task should be cleaned up, FALSE to keep it
 */
UBOOL FLiveAsyncTaskKeyboard::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	FLiveAsyncTaskDataKeyboard* KeyboardData = (FLiveAsyncTaskDataKeyboard*)TaskData;
	UBOOL bShouldCleanup = TRUE;
	// Determine if we are validating or just processing the keyboard results
	if (bIsValidating == FALSE)
	{
		// Keyboard results are back
		if (KeyboardData->NeedsStringValidation() &&
			XGetOverlappedExtendedError(&Overlapped) != ERROR_CANCELLED)
		{
			appMemzero(&Overlapped,sizeof(XOVERLAPPED));
			// Set up the string input data
			TCHAR* String = *KeyboardData;
			STRING_DATA* StringData = *KeyboardData;
			StringData->wStringSize = appStrlen(String);
			StringData->pszString = String;
			// Kick off the validation as an async task
			DWORD Return = XStringVerify(0,
//@todo joeg -- figure out what the different strings are based upon language
				"en-us",
				1,
				StringData,
				sizeof(STRING_VERIFY_RESPONSE) + sizeof(HRESULT),
				*KeyboardData,
				&Overlapped);
			if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
			{
				bShouldCleanup = FALSE;
				bIsValidating = TRUE;
			}
			else
			{
				debugf(NAME_DevOnline,
					TEXT("Failed to validate string (%s) with 0x%08X"),
					String,
					Return);
			}
		}
		else
		{
			// Copy the data into our subsystem buffer
			LiveSubsystem->KeyboardInputResults = *KeyboardData;
			// Determine if the user canceled input or not
			LiveSubsystem->bWasKeyboardInputCanceled = XGetOverlappedExtendedError(&Overlapped) == ERROR_CANCELLED;
		}
	}
	else
	{
		// Validation is complete, so copy the string if ok otherwise zero it
		STRING_VERIFY_RESPONSE* Response = *KeyboardData;
		if (Response->pStringResult[0] == S_OK)
		{
			// String is ok, so copy
			LiveSubsystem->KeyboardInputResults = *KeyboardData;
		}
		else
		{
			// String was a bad word so empty
			LiveSubsystem->KeyboardInputResults = TEXT("");
		}
	}
	return bShouldCleanup;
}

/**
 * Tells the Live subsystem to parse the results of the stats read
 *
 * @param LiveSubsystem the object to make the final call on
 *
 * @return TRUE if this task should be cleaned up, FALSE to keep it
 */
UBOOL FLiveAsyncTaskReadStats::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	UBOOL bShouldDelete = TRUE;
	DWORD Result = GetCompletionCode();
	debugfLiveSlow(NAME_DevOnline,TEXT("XUserReadStats() returned 0x%08X"),Result);
	if (Result == ERROR_SUCCESS)
	{
		LiveSubsystem->ParseStatsReadResults(GetReadResults());
		// Update the player buffers and see if there are more to read
		UpdatePlayersToRead();
		if (NumToRead > 0)
		{
			appMemzero(&Overlapped,sizeof(XOVERLAPPED));
			bShouldDelete = FALSE;
			// Kick off another async read
			DWORD Return = XUserReadStats(TitleId,
				NumToRead,
				GetPlayers(),
				1,
				GetSpecs(),
				&BufferSize,
				GetReadResults(),
				&Overlapped);
			debugfLiveSlow(NAME_DevOnline,
				TEXT("Paged XUserReadStats(0,%d,Players,1,Specs,%d,Buffer,Overlapped) returned 0x%08X"),
				NumToRead,BufferSize,Return);
			if (Return != ERROR_SUCCESS && Return != ERROR_IO_PENDING)
			{
				bShouldDelete = TRUE;
			}
		}
	}
	// If we are done processing, zero the read state
	if (bShouldDelete)
	{
		LiveSubsystem->CurrentStatsRead->eventOnReadComplete();
		LiveSubsystem->CurrentStatsRead = NULL;
	}
	return bShouldDelete;
}

/**
 * Tells the Live subsystem to parse the results of the stats read
 *
 * @param LiveSubsystem the object to make the final call on
 *
 * @return TRUE if this task should be cleaned up, FALSE to keep it
 */
UBOOL FLiveAsyncTaskReadStatsByRank::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	DWORD Result = GetCompletionCode();
	if (Result == ERROR_SUCCESS)
	{
		LiveSubsystem->ParseStatsReadResults(GetReadResults());
		LiveSubsystem->CurrentStatsRead->eventOnReadComplete();
	}
	LiveSubsystem->CurrentStatsRead = NULL;
	debugfLiveSlow(NAME_DevOnline,TEXT("XEnumerate() returned 0x%08X"),Result);
	return TRUE;
}

/**
 * Tells the Live subsystem to continue the async game invite join
 *
 * @param LiveSubsystem the object to make the final call on
 *
 * @return TRUE if this task should be cleaned up, FALSE to keep it
 */
UBOOL FLiveAsyncTaskJoinGameInvite::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	DWORD Result = GetCompletionCode();
	if (Result == ERROR_SUCCESS)
	{
		if (State == WaitingForSearch)
		{
			// Need to create a search object to append the results to
			UOnlineGameSearch* Search = ConstructObject<UOnlineGameSearch>(UOnlineGameSearch::StaticClass());
			// Store the search so it can be used on accept
			LiveSubsystem->InviteCache[UserNum].InviteSearch = Search;
			// We need to parse the search results
			LiveSubsystem->ParseSearchResults(Search,GetResults());
			// Get the game object so we can mark it as being from an invite
			InviteSettings = Search->Results.Num() > 0 ? Search->Results(0).GameSettings : NULL;
			if (InviteSettings != NULL)
			{
				InviteSettings->bWasFromInvite = TRUE;
				// Request QoS so we can get the list play server's data
				RequestQoS(Search);
			}
			else
			{
				// Can't join this server since the search didn't return it
				State = InviteReady;
			}
		}
		else if (State == QueryingQos)
		{
			if (QosData != NULL)
			{
				// Check if all results are back
				if (QosData->cxnqosPending == 0)
				{
					// Move the QoS data to the settings object
					ParseQoS(LiveSubsystem->InviteCache[UserNum].InviteSearch);
				}
			}
		}
		else
		{
			InviteSettings = NULL;
			State = InviteReady;
		}
	}
	else
	{
		InviteSettings = NULL;
		State = InviteReady;
	}
	if (State == InviteReady)
	{
		// Fire off the delegate with the results (possibly NULL if not found)
		OnlineSubsystemLive_eventOnGameInviteAccepted_Parms Parms(EC_EventParm);
		Parms.InviteSettings = InviteSettings;
		// Use the helper method to fire the delegates
		TriggerOnlineDelegates(LiveSubsystem,*ScriptDelegates,&Parms);
		// Don't fire automatically since we manually fired it off
		ScriptDelegates = NULL;
	}
	return State == InviteReady;
}

/**
 * Reads the qos data for the server that was sending the invite
 *
 * @param Search the game search to update
 */
void FLiveAsyncTaskJoinGameInvite::RequestQoS(UOnlineGameSearch* Search)
{
	XSESSION_INFO* SessInfo = (XSESSION_INFO*)Search->Results(0).PlatformData;
	debugf(NAME_DevOnline,TEXT("Requesting QoS for 0x%016I64X"),(QWORD&)SessInfo->sessionID);
	ServerAddrs[0] = &SessInfo->hostAddress;
	ServerKids[0] = &SessInfo->sessionID;
	ServerKeys[0] = &SessInfo->keyExchangeKey;
	// Kick off the QoS set of queries
	DWORD Return = XNetQosLookup(1,
		(const XNADDR**)ServerAddrs,
		(const XNKID**)ServerKids,
		(const XNKEY**)ServerKeys,
		// We skip all gateway services
		0,0,0,
		// 1 probe is fine since accurate ping doesn't matter
		1,
		64 * 1024,
		// Flags are unsupported and we'll poll
		0,NULL,
		// The out parameter that holds the data
		&QosData);
	debugf(NAME_DevOnline,
		TEXT("Invite: XNetQosLookup(1,Addrs,Kids,Keys,0,0,0,1,64K,0,NULL,Data) returned 0x%08X"),
		Return);
	State = QueryingQos;
}

/**
 * Parses the qos data that came back from the server
 *
 * @param Search the game search to update
 */
void FLiveAsyncTaskJoinGameInvite::ParseQoS(UOnlineGameSearch* Search)
{
	// Iterate through the results
	if (QosData->cxnqos == 1)
	{
		// Read the custom data if present
		if (QosData->axnqosinfo[0].cbData > 0 &&
			QosData->axnqosinfo[0].pbData != NULL)
		{
			// Create a packet reader to read the data out
			FNboSerializeFromBufferXe Packet(QosData->axnqosinfo[0].pbData,
				QosData->axnqosinfo[0].cbData);
			BYTE QosPacketVersion = 0;
			Packet >> QosPacketVersion;
			// Verify the packet version
			if (QosPacketVersion == QOS_PACKET_VERSION)
			{
				// Read the XUID and the server nonce
				Packet >> InviteSettings->OwningPlayerId;
				Packet >> InviteSettings->ServerNonce;
				Packet >> InviteSettings->BuildUniqueId;
				INT NumProps = 0;
				// Read how many props are in the buffer
				Packet >> NumProps;
				InviteSettings->Properties.Empty(NumProps);
				for (INT PropIndex = 0; PropIndex < NumProps; PropIndex++)
				{
					INT AddAt = InviteSettings->Properties.AddZeroed();
					Packet >> InviteSettings->Properties(AddAt);
				}
				INT NumContexts = 0;
				// Read how many contexts are in the buffer
				Packet >> NumContexts;
				InviteSettings->LocalizedSettings.Empty(NumContexts);
				for (INT ContextIndex = 0; ContextIndex < NumContexts; ContextIndex++)
				{
					INT AddAt = InviteSettings->LocalizedSettings.AddZeroed();
					Packet >> InviteSettings->LocalizedSettings(AddAt);
				}
				// Set the ping that the QoS estimated
				InviteSettings->PingInMs = QosData->axnqosinfo[0].wRttMedInMsecs;
				debugfLiveSlow(NAME_DevOnline,TEXT("QoS for invite is %d"),InviteSettings->PingInMs);
			}
			else
			{
				debugf(NAME_DevOnline,TEXT("Failed to get QoS data for invite"));
				InviteSettings = NULL;
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("Failed to get QoS data for invite"));
			InviteSettings = NULL;
		}
	}
	State = InviteReady;
}

/**
 * Parses the arbitration results and stores them in the arbitration list
 *
 * @param LiveSubsystem the object to make the final call on
 *
 * @return TRUE if this task should be cleaned up, FALSE to keep it
 */
UBOOL FLiveAsyncTaskArbitrationRegistration::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	DWORD Result = XGetOverlappedExtendedError(&Overlapped);
	if (Result == ERROR_SUCCESS)
	{
		// Forward the call to the subsystem
		LiveSubsystem->ParseArbitrationResults(SessionName,GetResults());
	}
	return TRUE;
}

/**
 * Checks to see if the match is arbitrated and shrinks it by one if it is
 *
 * @param LiveSubsystem the object to make the final call on
 *
 * @return TRUE if this task should be cleaned up, FALSE to keep it
 */
UBOOL FLiveAsyncPlayer::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	FNamedSession* Session = LiveSubsystem->GetNamedSession(SessionName);
	// Shrink the session size by one if using arbitration
	if (Session &&
		Session->GameSettings &&
		Session->GameSettings->bUsesArbitration &&
		Session->GameSettings->GameState >= OGS_InProgress)
	{
		Session->GameSettings->NumPublicConnections--;
		Session->GameSettings->NumPrivateConnections = 0;
		LiveSubsystem->ModifySession(Session,NULL);
	}
	return TRUE;
}

/**
 * Checks to see if the join worked. If this was an invite it may need to
 * try private and then public.
 *
 * @param LiveSubsystem the object to make the final call on
 *
 * @return TRUE if this task should be cleaned up, FALSE to keep it
 */
UBOOL FLiveAsyncRegisterPlayer::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	DWORD Result = XGetOverlappedExtendedError(&Overlapped);
	// If the task failed and we tried private first, then try public
	if (Result != ERROR_SUCCESS && bPrivateInvite == TRUE && bIsSecondTry == FALSE)
	{
		debugfLiveSlow(NAME_DevOnline,TEXT("Private invite failed with 0x%08X. Trying public"),Result);
		bIsSecondTry = TRUE;
		bPrivateInvite = FALSE;
		appMemzero(&Overlapped,sizeof(XOVERLAPPED));
		// Grab the session information by name
		FNamedSession* Session = LiveSubsystem->GetNamedSession(SessionName);
		check(Session);
		FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
		check(SessionInfo);
		// Kick off the async join request
		Result = XSessionJoinRemote(SessionInfo->Handle,
			1,
			GetXuids(),
			GetPrivateInvites(),
			&Overlapped);
		debugf(NAME_DevOnline,TEXT("XSessionJoinRemote(0x%016I64X) returned 0x%08X"),
			*GetXuids(),Result);
		return FALSE;
	}
	return TRUE;
}

/**
 * Marks the skill in progress flag as false
 *
 * @param LiveSubsystem the object to make the final call on
 *
 * @return TRUE if this task should be cleaned up, FALSE to keep it
 */
UBOOL FLiveAsyncUpdateSessionSkill::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	// Grab the session information by name
	FNamedSession* Session = LiveSubsystem->GetNamedSession(SessionName);
	check(Session && Session->GameSettings);
	Session->GameSettings->bHasSkillUpdateInProgress = FALSE;
	return TRUE;
}

/**
 * After getting the list of files that are to be downloaded, it downloads
 * and merges each INI file in the list
 *
 * @param LiveSubsystem the object to make the final call on
 *
 * @return TRUE if this task should be cleaned up, FALSE to keep it
 */
UBOOL FLiveAsyncTMSRead::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	UBOOL bShouldDelete = FALSE;
	// Merge the file results if the read was successful
	if (NextFileToRead >= 0)
	{
		// Add a TMS file to the list and copy the data
		INT AddIndex = FilesRead.AddZeroed();
		// Make sure the read completed ok, otherwise mark the entry as failed
		if (XGetOverlappedExtendedError(&Overlapped) == ERROR_SUCCESS)
		{
			FString FileName = FilesReturned->pItems[NextFileToRead].pwszPathName;
			// Strip off the title specific directories to get just the INI name
			INT Index = FileName.InStr(TEXT("/"),TRUE);
			if (Index != -1)
			{
				FileName = FileName.Right(FileName.Len() - 1 - Index);
			}
			INT FileSize = FilesReturned->pItems[NextFileToRead].dwInstalledSize;
			// Copy the data into the cached buffer
			FilesRead(AddIndex).Data.AddZeroed(FileSize);
			appMemcpy(FilesRead(AddIndex).Data.GetTypedData(),FileBuffer,FileSize);
			debugfLiveSlow(NAME_DevOnline,TEXT("Read TMS file '%s' of size (%d)"),*FileName,FileSize);
		}
		else
		{
			FilesRead(AddIndex).AsyncState = OERS_Failed;
			debugf(NAME_DevOnline,
				TEXT("Failed to read TMS file '%s'"),
				FilesReturned->pItems[NextFileToRead].pwszPathName);
		}
		// Pass in the data that indicates whether the call worked or not
		OnlineSubsystemLive_eventOnReadTitleFileComplete_Parms Parms(EC_EventParm);
		Parms.bWasSuccessful = FilesRead(AddIndex).AsyncState == OERS_Done ? FIRST_BITFIELD : 0;
		Parms.Filename = FilesRead(AddIndex).Filename;
		// Use the common method to do the work
		TriggerOnlineDelegates(LiveSubsystem,*ScriptDelegates,&Parms);
	}
	// Move to the next file to read in the enumeration buffer
	NextFileToRead++;
	// If there are more files to read, kick off the next read
	if (NextFileToRead < (INT)FilesReturned->dwNumItemsReturned)
	{
		// Don't allocate a new buffer if the old one is large enough
		if (FileBufferSize < FilesReturned->pItems[NextFileToRead].dwInstalledSize)
		{
			// Free the previous buffer
			if (FileBuffer != NULL)
			{
				delete [] FileBuffer;
				FileBuffer = NULL;
				FileBufferSize = 0;
			}
			FileBufferSize = FilesReturned->pItems[NextFileToRead].dwInstalledSize;
			// Allocate the buffer needed to download the file
			FileBuffer = new BYTE[FileBufferSize];
		}
		// Clear our overlapped so it can be reused
		appMemzero(&Overlapped,sizeof(XOVERLAPPED));
		// Kick off a download of the file to the buffer
		DWORD Result = XStorageDownloadToMemory(0,
			FilesReturned->pItems[NextFileToRead].pwszPathName,
			FileBufferSize,
			FileBuffer,
			sizeof(XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS),
			&FileDownloadResults,
			&Overlapped);
		debugfLiveSlow(NAME_DevOnline,
			TEXT("XStorageDownloadToMemory(0,\"%s\",%d,Buffer,%d,DLResults,Overlapped) returned 0x%08X"),
			FilesReturned->pItems[NextFileToRead].pwszPathName,
			FileBufferSize,
			sizeof(XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS),
			Result);
		if (Result != ERROR_SUCCESS && Result != ERROR_IO_PENDING)
		{
			// Add a TMS file to the list and mark as being in error
			INT AddIndex = FilesRead.AddZeroed();
			FilesRead(AddIndex).AsyncState = OERS_Failed;
			bShouldDelete = TRUE;
		}
	}
	else
	{
		if (FilesReturned->dwNumItemsReturned == 0)
		{
			// Failed to read any files, so indicate a failure
			OnlineSubsystemLive_eventOnReadTitleFileComplete_Parms Parms(EC_EventParm);
			Parms.bWasSuccessful = 0;
			Parms.Filename = TEXT("");
			// Use the common method to do the work
			TriggerOnlineDelegates(LiveSubsystem,*ScriptDelegates,&Parms);
		}
		bShouldDelete = TRUE;
	}
	return bShouldDelete;
}

/**
 * Parses the read results and continues the read if needed
 *
 * @param LiveSubsystem the object to add the data to
 *
 * @return TRUE if this task should be cleaned up, FALSE to keep it
 */
UBOOL FLiveAsyncTaskReadAchievements::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	FLiveAsyncTaskDataReadAchievements* Data = (FLiveAsyncTaskDataReadAchievements*)TaskData;
	FCachedAchievements& Cached = LiveSubsystem->GetCachedAchievements(Data->GetPlayerIndex(),
		Data->GetTitleId());
	// Figure out if we are at the end of the list or not
	DWORD Result = XGetOverlappedExtendedError(&Overlapped);
	// If the task completed ok, then parse the results and start another enumeration fetch
	if (Result == ERROR_SUCCESS)
	{
		XACHIEVEMENT_DETAILS* Details = Data->GetDetailsBuffer();
		DWORD ItemsEnumerated = 0;
		XGetOverlappedResult(&Overlapped,&ItemsEnumerated,FALSE);
		// Add each achievement that was enumerated
		for (DWORD AchIndex = 0; AchIndex < ItemsEnumerated; AchIndex++)
		{
			INT AddIndex = Cached.Achievements.AddZeroed();
			// Add the new details to the list
			FAchievementDetails& AchDetails = Cached.Achievements(AddIndex);
			AchDetails.Id = Details[AchIndex].dwId;
			AchDetails.AchievementName = Details[AchIndex].pwszLabel;
			AchDetails.Description = Details[AchIndex].pwszDescription;
			AchDetails.HowTo = Details[AchIndex].pwszUnachieved;
			AchDetails.GamerPoints = Details[AchIndex].dwCred;
			// Check the flags to see how it was earned (or not)
			if (!(Details[AchIndex].dwFlags & XACHIEVEMENT_DETAILS_SHOWUNACHIEVED))
			{
				AchDetails.bIsSecret = TRUE;
			}
			if (Details[AchIndex].dwFlags & XACHIEVEMENT_DETAILS_ACHIEVED_ONLINE)
			{
				AchDetails.bWasAchievedOnline = TRUE;
			}
			if ((Details[AchIndex].dwFlags & XACHIEVEMENT_DETAILS_ACHIEVED) && !AchDetails.bWasAchievedOnline)
			{
				AchDetails.bWasAchievedOffline = TRUE;
			}
		}
		// Zero between uses or results will be incorrect
		appMemzero(&Overlapped,sizeof(XOVERLAPPED));
		// Have it read the next item in the list
		Result = XEnumerate(Data->GetHandle(),
			Data->GetBuffer(),
			Data->GetBufferSize(),
			0,
			&Overlapped);
	}
	else
	{
		// Done reading, need to mark as finished
		Cached.ReadState = OERS_Done;
	}
	// When this is true, there is no more data left to read
	return Result != ERROR_SUCCESS && Result != ERROR_IO_PENDING;
}

/**
 * Iterates the list of delegates and fires those notifications
 *
 * @param Object the object that the notifications are going to be issued on
 */
void FLiveAsyncTaskReadAchievements::TriggerDelegates(UObject* Object)
{
	check(Object);
	// Only fire off the events if there are some registered
	if (ScriptDelegates != NULL)
	{
		OnlineSubsystemLive_eventOnReadAchievementsComplete_Parms Parms(EC_EventParm);
		FLiveAsyncTaskDataReadAchievements* Data = (FLiveAsyncTaskDataReadAchievements*)TaskData;
		Parms.TitleId = Data->GetTitleId();
		// Use the common method to do the work
		TriggerOnlineDelegates(Object,*ScriptDelegates,&Parms);
	}
}

/**
 * If the skill read completes successfully, it then triggers the requested search
 * If it fails, it uses the search delegates to notify the game code
 *
 * @param LiveSubsystem the object to make the final call on
 *
 * @return TRUE if this task should be cleaned up, FALSE to keep it
 */
UBOOL FLiveAsyncReadPlayerSkillForSearch::ProcessAsyncResults(UOnlineSubsystemLive* LiveSubsystem)
{
	XUSER_STATS_READ_RESULTS* ReadResults = GetReadBuffer();
	INT PlayerCount = Search->ManualSkillOverride.Players.Num();
	DWORD Result = XGetOverlappedExtendedError(&Overlapped);
	// If the skill read completed ok, then parse the results and kick of the
	// session search with them
	if (Result == ERROR_SUCCESS &&
		// Make sure we have a skill entry for each player
		ReadResults->pViews->dwNumRows == PlayerCount)
	{
		// Preallocate our space for exactly the number we need (no slack)
		Search->ManualSkillOverride.Mus.Empty(PlayerCount);
		Search->ManualSkillOverride.Sigmas.Empty(PlayerCount);
		// Each row represents a player
		for (INT Index = 0; Index < PlayerCount; Index++)
		{
			// The Mu is in the first column
			DOUBLE Mu = ReadResults->pViews[0].pRows[Index].pColumns[0].Value.dblData;
			// The Sigma is in the second column
			DOUBLE Sigma = ReadResults->pViews[0].pRows[Index].pColumns[1].Value.dblData;
			// Make sure the player isn't new to the leaderboard
			if (Mu == 0.0 && Sigma == 0.0)
			{
				// Default to middle of the range with 100% uncertainty
				Mu = 3.0;
				Sigma = 1.0;
			}
			Search->ManualSkillOverride.Mus.AddItem(Mu);
			Search->ManualSkillOverride.Sigmas.AddItem(Sigma);
		}
		// Now that we have the skill data, kick of the search
		LiveSubsystem->FindOnlineGames(LocalUserNum,Search);
	}
	else
	{
		// Set the delegates so they'll notify the game code
		ScriptDelegates = &LiveSubsystem->FindOnlineGamesCompleteDelegates;
	}
	return TRUE;
}

/**
 * Live specific initialization. Sets all the interfaces to point to this
 * object as it implements them all
 *
 * @return always returns TRUE
 */
UBOOL UOnlineSubsystemLive::Init(void)
{
	Super::Init();
	// Set the player interface to be the same as the object
	eventSetPlayerInterface(this);
	// Set the Live specific player interface to be the same as the object
	eventSetPlayerInterfaceEx(this);
	// Set the system interface to be the same as the object
	eventSetSystemInterface(this);
	// Set the game interface to be the same as the object
	eventSetGameInterface(this);
	// Set the content interface to be the same as the object
	eventSetContentInterface(this);
	// Set the stats reading/writing interface
	eventSetStatsInterface(this);
	// Create the voice engine and if successful register the interface
	VoiceEngine = appCreateVoiceInterface(MaxLocalTalkers,MaxRemoteTalkers,
		bIsUsingSpeechRecognition);
	// Set the voice interface to this object
	eventSetVoiceInterface(this);
	if (bShouldUseMcp)
	{
		UOnlineNewsInterfaceMcp* NewsObject = ConstructObject<UOnlineNewsInterfaceMcp>(UOnlineNewsInterfaceMcp::StaticClass(),this);
		eventSetNewsInterface(NewsObject);
	}
	// Check each controller for a logged in player, DLC, etc.
	InitLoginState();
	// Register the notifications we are interested in
	NotificationHandle = XNotifyCreateListener(XNOTIFY_SYSTEM | XNOTIFY_LIVE | XNOTIFY_FRIENDS);
	if (NotificationHandle == NULL)
	{
		debugf(NAME_DevOnline,TEXT("Failed to create Live notification listener"));
	}
	// Use the unique build id to prevent incompatible builds from colliding
	LanGameUniqueId = GetBuildUniqueId();
	// Tell presence how many we want
	XPresenceInitialize(MAX_FRIENDS);
	// Set the default toast location
	SetNetworkNotificationPosition(CurrentNotificationPosition);
	// Set the default log level
	SetDebugSpewLevel(DebugLogLevel);
#if WITH_PANORAMA
	return NotificationHandle != NULL && InitG4WLive();
#else
	return NotificationHandle != NULL;
#endif
}

/**
 * Initializes the various sign in state and DLC state
 */
void UOnlineSubsystemLive::InitLoginState(void)
{
#if CONSOLE
	XINPUT_CAPABILITIES InputCaps;
	appMemzero(&InputCaps,sizeof(XINPUT_CAPABILITIES));
#endif
	appMemzero(LastXuids,sizeof(FXuidPair) * 4);
	// Iterate controller indices and update sign in state & DLC
	for (INT UserIndex = 0; UserIndex < 4; UserIndex++)
	{
		// Wipe their profile cache
		ProfileCache[UserIndex].Profile = NULL;
		// Cache the last logged in xuid for signin comparison
		if (XUserGetXUID(UserIndex,&(XUID&)LastXuids[UserIndex].OnlineXuid) == ERROR_SUCCESS)
		{
			XUSER_SIGNIN_INFO SigninInfo = {0};
			// Cache the offline xuid to detect sign in changes
			if (XUserGetSigninInfo(UserIndex,
				XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY,
				&SigninInfo) == ERROR_SUCCESS)
			{
				(XUID&)LastXuids[UserIndex].OfflineXuid = SigninInfo.xuid;
			}
		}
		// See if they are signed in
		UBOOL bIsSignedIn = XUserGetSigninState(UserIndex) != eXUserSigninState_NotSignedIn;
		// Update the last sign in mask with their sign in state
		LastSignInMask |= ((bIsSignedIn) << UserIndex);
		if (bIsSignedIn)
		{
			ReadContentList(UserIndex);
		}
#if CONSOLE
		// Init the base controller connection state
		if (XInputGetCapabilities(UserIndex,XINPUT_FLAG_GAMEPAD,&InputCaps) == ERROR_SUCCESS)
		{
			LastInputDeviceConnectedMask |= 1 << UserIndex;
		}
#endif
	}
}

#if WITH_PANORAMA
/**
 * Initializes the G4W Live specific features
 *
 * @return TRUE if successful, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::InitG4WLive(void)
{
	HRESULT hr = E_FAIL;
	extern UBOOL GIsUsingParorama;
	// Don't try to set these if it failed to init (deadlocks)
	if (GIsUsingParorama)
	{
#if !SHIPPING_PC_GAME
		XLiveSetDebugLevel(XLIVE_DEBUG_LEVEL_INFO,NULL);
#else
		XLiveSetDebugLevel(XLIVE_DEBUG_LEVEL_OFF,NULL);
#endif
		// Set the system link port so Live doesn't block it
		INT ErrorCode = XNetSetSystemLinkPort(XSocketHTONS((WORD)LiveSystemLinkPort));
		if (ErrorCode != 0)
		{
			debugf(NAME_DevOnline,
				TEXT("Unable to set LAN/Systemlink port (%d) (error = %d), so LAN matches are disabled"),
				LanAnnouncePort,
				ErrorCode);
		}
	}
	return SUCCEEDED(hr);
}

/**
 * Checks the results of the signin operation and shuts down the server if
 * it fails
 *
 * @param LiveSubsystem the object to make the final call on
 *
 * @return TRUE if this task should be cleaned up, FALSE to keep it
 */
UBOOL FLiveAsyncTaskSignin::ProcessAsyncResults(UOnlineSubsystemLive*)
{
	DWORD Result = GetCompletionCode();
	if (Result != ERROR_SUCCESS)
	{
		// If this is a dedicated server, bail out
		if (GIsServer && GIsUCC)
		{
			debugf(NAME_DevOnline,
				TEXT("Unable to sign in using the specified credentials (error 0x%08X), exiting"),
				Result);
			appRequestExit(0);
		}
	}
	return TRUE;
}
#endif

/**
 * Called from the engine shutdown code to allow the Live to cleanup. Also, this
 * version blocks until all async tasks are complete before returning.
 */
void UOnlineSubsystemLive::Exit(void)
{
	// While there are any outstanding tasks, block so that file writes, etc. complete
	while (AsyncTasks.Num())
	{
		TickAsyncTasks();
		appSleep(0.1f);
	}
	// Close our notification handle
	if (NotificationHandle)
	{
		XCloseHandle(NotificationHandle);
		NotificationHandle = NULL;
	}
#if WITH_PANORAMA
	// Tell Panorama to shut down
	XLiveUnInitialize();
#endif
}

/**
 * Checks for new Live notifications and passes them out to registered delegates
 *
 * @param DeltaTime the amount of time that has passed since the last tick
 */
void UOnlineSubsystemLive::Tick(FLOAT DeltaTime)
{
	// Tick sign in notifications if pending
	if (bIsCountingDownSigninNotification)
	{
		SigninCountDownCounter -= DeltaTime;
		if (SigninCountDownCounter <= 0.f)
		{
			ProcessSignInNotification(NULL);
		}
	}
	DWORD Notification = 0;
	ULONG_PTR Data = NULL;
	// Check Live for notification events
	while (XNotifyGetNext(NotificationHandle,0,&Notification,&Data))
	{
		// Now process the event
		ProcessNotification(Notification,Data);
	}
	// Process any invites that occured at start up and are pending
	TickDelayedInvites();
	// Now tick any outstanding async tasks
	TickAsyncTasks();
	// Tick any tasks needed for LAN support
	TickLanTasks(DeltaTime);
	// Tick voice processing
	TickVoice(DeltaTime);
}

/**
 * Processes a notification that was returned during polling
 *
 * @param Notification the notification event that was fired
 * @param Data the notification specifc data
 */
void UOnlineSubsystemLive::ProcessNotification(DWORD Notification,ULONG_PTR Data)
{
	switch (Notification)
	{
		case XN_SYS_SIGNINCHANGED:
		{
			// Second notification should override the first according to the XDK FAQ
			if (bIsCountingDownSigninNotification)
			{
				ProcessSignInNotification(Data);
			}
			else
			{
				// Start the ticking of the count down timer to work around an XDK bug
				bIsCountingDownSigninNotification = TRUE;
				SigninCountDownCounter = SigninCountDownDelay;
			}
			break;
		}
		case XN_SYS_MUTELISTCHANGED:
		{
			TriggerOnlineDelegates(this,MutingChangeDelegates,NULL);
			// Have the voice code re-evaluate its mute settings
			ProcessMuteChangeNotification();
			break;
		}
		case XN_FRIENDS_FRIEND_ADDED:
		case XN_FRIENDS_FRIEND_REMOVED:
		case XN_FRIENDS_PRESENCE_CHANGED:
		{
			// Per user notification of friends change
			TriggerOnlineDelegates(this,FriendsCache[(DWORD)Data].FriendsChangeDelegates,NULL);
			break;
		}
		case XN_SYS_UI:
		{
			// Data will be non-zero if opening
			UBOOL bIsOpening = Data != 0;
			ProcessExternalUINotification(bIsOpening);
#if WITH_PANORAMA
			extern UBOOL GIsPanoramaUIOpen;
			// Allow the guide to pause input processing
			GIsPanoramaUIOpen = bIsOpening;
#endif
			break;
		}
		case XN_SYS_INPUTDEVICESCHANGED:
		{
			ProcessControllerNotification();
			break;
		}
		case XN_SYS_STORAGEDEVICESCHANGED:
		{
			// Notify any registered delegates
			TriggerOnlineDelegates(this,StorageDeviceChangeDelegates,NULL);
			break;
		}
		case XN_LIVE_LINK_STATE_CHANGED:
		{
			// Data will be non-zero if connected
			UBOOL bIsConnected = Data != 0;
			// Notify registered delegates
			ProcessLinkStateNotification(bIsConnected);
			break;
		}
		case XN_LIVE_CONTENT_INSTALLED:
		{
			// remove all downloaded content, it will be added again below
			FlushAllDownloadedContent(4);
			// Build the last sign in mask and refresh content
			for (INT Index = 0; Index < 4; Index++)
			{
				// See if they are signed in
				if (XUserGetSigninState(Index) != eXUserSigninState_NotSignedIn)
				{
					ReadContentList(Index);
				}
			}
			break;
		}
		case XN_LIVE_INVITE_ACCEPTED:
		{
			// Accept the invite for the specified user
			ProcessGameInvite((DWORD)Data);
			break;
		}
		case XN_LIVE_CONNECTIONCHANGED:
		{
			// Fire off any events needed for this notification
			ProcessConnectionStatusNotification((HRESULT)Data);
			break;
		}
		case XN_SYS_PROFILESETTINGCHANGED:
		{
			ProcessProfileDataNotification(Data);
			break;
		}
#if WITH_PANORAMA
		case XN_SYS_XLIVETITLEUPDATE:
		case XN_SYS_XLIVESYSTEMUPDATE:
		{
			// Handle G4WLive telling us about downloaded updates
			ProcessLiveTitleUpdate();
			break;
		}
#endif
	}
}

/**
 * Handles any sign in change processing (firing delegates, etc)
 *
 * @param Data the mask of changed sign ins
 */
void UOnlineSubsystemLive::ProcessSignInNotification(ULONG_PTR Data)
{
	// Disable the ticking of delayed sign ins
	bIsCountingDownSigninNotification = FALSE;
	// Notify each subscriber
	TriggerOnlineDelegates(this,AllLoginDelegates.Delegates,NULL);
	// remove all downloaded content, it will be added again below
	FlushAllDownloadedContent(4);
	// Loop through the valid bits and send notifications
	for (DWORD Index = 0; Index < 4; Index++)
	{
		DWORD SignInState = XUserGetSigninState(Index);
		// Clear and get the currently logged in xuid
		XUID CurrentXuid = 0;
		XUserGetXUID(Index,&CurrentXuid);
		// Is the user signed in? or were they signed in?
		if ((SignInState != eXUserSigninState_NotSignedIn && !(LastSignInMask & (1 << Index))) ||
			((SignInState == eXUserSigninState_NotSignedIn && (LastSignInMask & (1 << Index)))) ||
			// If the sign in changes without a sign out notification, fire the event
			(CurrentXuid != (XUID&)LastXuids[Index].OnlineXuid && CurrentXuid != (XUID&)LastXuids[Index].OfflineXuid))
		{
			debugfLiveSlow(NAME_DevOnline,TEXT("Discarding cached profile for user %d"),Index);
			// Zero the cached profile so we don't use the wrong profile
			ProfileCache[Index].Profile = NULL;
			// Clear any cached achievement data
			ClearCachedAchievements(Index);
			// Fire the delegate for each registered delegate
			TriggerOnlineDelegates(this,PlayerLoginDelegates[Index].Delegates,NULL);
		}
	}
	LastSignInMask = 0;
	appMemzero(LastXuids,sizeof(FXuidPair) * 4);
	// Build the last sign in mask and refresh content
	for (INT Index = 0; Index < 4; Index++)
	{
		// Cache the last logged in xuid for signin comparison
		if (XUserGetXUID(Index,&(XUID&)LastXuids[Index].OnlineXuid) == ERROR_SUCCESS)
		{
			XUSER_SIGNIN_INFO SigninInfo = {0};
			// Cache the offline xuid to detect sign in changes
			if (XUserGetSigninInfo(Index,
				XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY,
				&SigninInfo) == ERROR_SUCCESS)
			{
				(XUID&)LastXuids[Index].OfflineXuid = SigninInfo.xuid;
			}
		}
		// See if they are signed in
		UBOOL bIsSignedIn = XUserGetSigninState(Index) != eXUserSigninState_NotSignedIn;
		// Update the last sign in mask with their sign in state
		LastSignInMask |= ((bIsSignedIn) << Index);
		if (bIsSignedIn)
		{
			// Read content for signed in users
			ReadContentList(Index);
		}
	}
	// Update voice's registered talkers
	UpdateVoiceFromLoginChange();
	bIsInSignInUI = FALSE;
}

/**
 * Handles notifying interested parties when a signin is cancelled
 */
void UOnlineSubsystemLive::ProcessSignInCancelledNotification(void)
{
	// Notify each subscriber of the user canceling
	TriggerOnlineDelegates(this,LoginCancelledDelegates,NULL);
	bIsInSignInUI = FALSE;
}

/**
 * Searches the PRI array for the specified player
 *
 * @param User the user to find
 *
 * @return TRUE if found, FALSE otherwise
 */
inline UBOOL IsUserInSession(XUID User)
{
	UBOOL bIsInSession = FALSE;
	if (GWorld)
	{
		AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
		if (WorldInfo)
		{
			AGameReplicationInfo* GameRep = WorldInfo->GRI;
			if (GameRep)
			{
				// Search through the set of players and see if they are in our game
				for (INT Index = 0; Index < GameRep->PRIArray.Num(); Index++)
				{
					APlayerReplicationInfo* PlayerRep = GameRep->PRIArray(Index);
					if (PlayerRep)
					{
						if ((XUID&)PlayerRep->UniqueId == User)
						{
							bIsInSession = TRUE;
							break;
						}
					}
				}
			}
		}
	}
	return bIsInSession;
}

/**
 * Handles accepting a game invite for the specified user
 *
 * @param UserNum the user that accepted the game invite
 */
void UOnlineSubsystemLive::ProcessGameInvite(DWORD UserNum)
{
	UBOOL bHasInviteDelegates = FALSE;
	// Make sure there are registered delegates before trying to handle it
	for (DWORD UserIndex = 0; UserIndex < 4; UserIndex++)
	{
		if (InviteCache[UserNum].InviteDelegates.Num())
		{
			bHasInviteDelegates = TRUE;
			break;
		}
	}
	if (bHasInviteDelegates)
	{
		// Clear the delayed bit
		DelayedInviteUserMask &= (~(1 << UserNum)) & 0xF;
		// This code assumes XNKID is 8 bytes
		check(sizeof(QWORD) == sizeof(XNKID));
		XINVITE_INFO* Info;
		// Allocate space on demand
		if (InviteCache[UserNum].InviteData == NULL)
		{
			InviteCache[UserNum].InviteData = new XINVITE_INFO;
		}
		// If for some reason the data didn't get cleaned up, do so now
		if (InviteCache[UserNum].InviteSearch != NULL &&
			InviteCache[UserNum].InviteSearch->Results.Num() > 0)
		{
			// Clean up the invite data
			delete (XSESSION_INFO*)InviteCache[UserNum].InviteSearch->Results(0).PlatformData;
			InviteCache[UserNum].InviteSearch->Results(0).PlatformData = NULL;
			InviteCache[UserNum].InviteSearch = NULL;
		}
		// Get the buffer to use and clear the previous contents
		Info = InviteCache[UserNum].InviteData;
		appMemzero(Info,sizeof(XINVITE_INFO));
		// Ask Live for the game details (session info)
		DWORD Return = XInviteGetAcceptedInfo(UserNum,Info);
		debugf(NAME_DevOnline,TEXT("XInviteGetAcceptedInfo(%d,Data) returned 0x%08X"),
			UserNum,Return);
		if (Return == ERROR_SUCCESS)
		{
			HandleJoinBySessionId(UserNum,(QWORD&)Info->xuidInviter,(QWORD&)Info->hostInfo.sessionID);
		}
	}
	else
	{
		// None are present to handle the invite, so set the delayed bit
		DelayedInviteUserMask |= 1 << UserNum;
	}
}

/**
 * Common method for joining a session by session id
 *
 * @param UserNum the user that is performing the search
 * @param Inviter the user that is sending the invite (or following)
 * @param SessionId the session id to join (from invite or friend presence)
 */
UBOOL UOnlineSubsystemLive::HandleJoinBySessionId(DWORD UserNum,QWORD Inviter,QWORD SessionId)
{
	DWORD Return = ERROR_SUCCESS;
	// Don't trigger an invite notification if we are in this session
	if (Sessions.Num() == 0 ||
		(IsInSession(SessionId) == FALSE && IsUserInSession(Inviter) == FALSE))
	{
		DWORD Size = 0;
		// Kick off an async search of the game by its session id. This is because
		// we need the gamesettings to understand what options to set
		//
		// First read the size needed to hold the results
		Return = XSessionSearchByID((XNKID&)SessionId,UserNum,&Size,NULL,NULL);
		if (Return == ERROR_INSUFFICIENT_BUFFER && Size > 0)
		{
			DWORD InviteDelegateIndex = UserNum;
			APlayerController* PC = GetPlayerControllerFromUserIndex(InviteDelegateIndex);
			if (PC == NULL)
			{
				// The player that accepted the invite hasn't created a PC, so notify the first valid one
				for (DWORD PlayerIndex = 0; PlayerIndex < 4; PlayerIndex++)
				{
					PC = GetPlayerControllerFromUserIndex(PlayerIndex);
					if (PC != NULL)
					{
						InviteDelegateIndex = PlayerIndex;
						break;
					}
				}
			}
			check(InviteDelegateIndex >= 0 && InviteDelegateIndex < 4); 
			// Create the async task with the proper data size
			FLiveAsyncTaskJoinGameInvite* AsyncTask = new FLiveAsyncTaskJoinGameInvite(UserNum,
				Size,&InviteCache[InviteDelegateIndex].InviteDelegates);
			// Now kick off the task
			Return = XSessionSearchByID((XNKID&)SessionId,
				UserNum,
				&Size,
				AsyncTask->GetResults(),
				*AsyncTask);
			debugf(NAME_DevOnline,TEXT("XSessionSearchByID(SessId,%d,%d,Data,Overlapped) returned 0x%08X"),
				UserNum,
				Size,
				Return);
			if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
			{
				AsyncTasks.AddItem(AsyncTask);
			}
			else
			{
				// Don't leak the task/data
				delete AsyncTask;
			}
		}
		else
		{
			debugf(NAME_DevOnline,
				TEXT("Couldn't determine the size needed for searching for the game invite/jip information 0x%08X"),
				Return);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Ignoring a game invite/jip to a session we're already in"));
		Return = E_FAIL;
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Handles external UI change notifications
 *
 * @param bIsOpening whether the UI is opening or closing
 */
void UOnlineSubsystemLive::ProcessExternalUINotification(UBOOL bIsOpening)
{
    OnlineSubsystemLive_eventOnExternalUIChange_Parms Parms(EC_EventParm);
    Parms.bIsOpening = bIsOpening ? FIRST_BITFIELD : 0;
	// Notify of the UI changes
	TriggerOnlineDelegates(this,ExternalUIChangeDelegates,&Parms);
	// Handle user cancelling a signin request
	if (bIsOpening == FALSE && bIsInSignInUI == TRUE)
	{
		ProcessSignInCancelledNotification();
	}
}

/**
 * Handles controller connection state changes
 */
void UOnlineSubsystemLive::ProcessControllerNotification(void)
{
#if CONSOLE
	XINPUT_CAPABILITIES InputCaps;
	appMemzero(&InputCaps,sizeof(XINPUT_CAPABILITIES));
	// Default to none connected
	INT CurrentMask = 0;
	// Iterate the controllers cheching their state
	for (INT ControllerIndex = 0; ControllerIndex < 4; ControllerIndex++)
	{
		INT ControllerMask = 1 << ControllerIndex;
		// See if this controller is connected or not
		if (XInputGetCapabilities(ControllerIndex,XINPUT_FLAG_GAMEPAD,&InputCaps) == ERROR_SUCCESS)
		{
			CurrentMask |= ControllerMask;
		}
		// Only fire the event if the connection status has changed
		if ((CurrentMask & ControllerMask) != (LastInputDeviceConnectedMask & ControllerMask))
		{
			OnlineSubsystemLive_eventOnControllerChange_Parms Parms(EC_EventParm);
			Parms.ControllerId = ControllerIndex;
			// If the current mask and the controller mask match, the controller was inserted
			// otherwise it was removed
			Parms.bIsConnected = (CurrentMask & ControllerMask) ? FIRST_BITFIELD : 0;
			TriggerOnlineDelegates(this,ControllerChangeDelegates,&Parms);
		}
	}
	// Set to the new state mask
	LastInputDeviceConnectedMask = CurrentMask;
#endif
}

/**
 * Handles notifying interested parties when the Live connection status
 * has changed
 *
 * @param Status the type of change that has happened
 */
void UOnlineSubsystemLive::ProcessConnectionStatusNotification(HRESULT Status)
{
	EOnlineServerConnectionStatus ConnectionStatus;
	// Map the Live code to ours
	switch (Status)
	{
		case XONLINE_S_LOGON_CONNECTION_ESTABLISHED:
			ConnectionStatus = OSCS_Connected;
			break;
		case XONLINE_S_LOGON_DISCONNECTED:
			ConnectionStatus = OSCS_NotConnected;
			break;
		case XONLINE_E_LOGON_NO_NETWORK_CONNECTION:
			ConnectionStatus = OSCS_NoNetworkConnection;
			break;
		case XONLINE_E_LOGON_CANNOT_ACCESS_SERVICE:
			ConnectionStatus = OSCS_ServiceUnavailable;
			break;
		case XONLINE_E_LOGON_UPDATE_REQUIRED:
		case XONLINE_E_LOGON_FLASH_UPDATE_REQUIRED:
			ConnectionStatus = OSCS_UpdateRequired;
			break;
		case XONLINE_E_LOGON_SERVERS_TOO_BUSY:
			ConnectionStatus = OSCS_ServersTooBusy;
			break;
		case XONLINE_E_LOGON_KICKED_BY_DUPLICATE_LOGON:
			ConnectionStatus = OSCS_DuplicateLoginDetected;
			break;
		case XONLINE_E_LOGON_INVALID_USER:
			ConnectionStatus = OSCS_InvalidUser;
			break;
		default:
			ConnectionStatus = OSCS_ConnectionDropped;
			break;
	}
    OnlineSubsystemLive_eventOnConnectionStatusChange_Parms Parms(EC_EventParm);
    Parms.ConnectionStatus = ConnectionStatus;
	TriggerOnlineDelegates(this,ConnectionStatusChangeDelegates,&Parms);
}

/**
 * Handles notifying interested parties when the link state changes
 *
 * @param bIsConnected whether the link has a connection or not
 */
void UOnlineSubsystemLive::ProcessLinkStateNotification(UBOOL bIsConnected)
{
    OnlineSubsystemLive_eventOnLinkStatusChange_Parms Parms(EC_EventParm);
    Parms.bIsConnected = bIsConnected ? FIRST_BITFIELD : 0;
	TriggerOnlineDelegates(this,LinkStatusChangeDelegates,&Parms);
}

/**
 * Handles notifying interested parties when the player changes profile data
 *
 * @param ChangeStatus bit flags indicating which user just changed status
 */
void UOnlineSubsystemLive::ProcessProfileDataNotification(DWORD ChangeStatus)
{
	// Check each user for a change
	for (DWORD Index = 0; Index < 4; Index++)
	{
		if ((1 << Index) & ChangeStatus)
		{
			// Notify this delegate of the change
			TriggerOnlineDelegates(this,ProfileCache[Index].ProfileDataChangedDelegates,NULL);
		}
	}
}

#if WITH_PANORAMA
/**
 * Handles the process of installing a game or system update
 */
void UOnlineSubsystemLive::ProcessLiveTitleUpdate(void)
{
	XLIVEUPDATE_INFORMATION LiveUpdate = {0};
	LiveUpdate.cbSize = sizeof(XLIVEUPDATE_INFORMATION);
	// Ask Live what was updated
	HRESULT hr = XLiveGetUpdateInformation(&LiveUpdate);
	debugf(NAME_DevOnline,TEXT("XLiveGetUpdateInformation() returned 0x%08X"),hr);
	if (SUCCEEDED(hr))
	{
		// System updates are handled by Live, we handle app updates
		if (LiveUpdate.bSystemUpdate)
		{
			TCHAR ExePath[MAX_PATH + 1];
			// Ask the OS the path to the exe
			if (GetModuleFileName(NULL,ExePath,MAX_PATH) > 0)
			{
				FString RelaunchCmdLine;
				// Set the path and append the command line args
				RelaunchCmdLine = FString::Printf(TEXT("\"%s %s\""),ExePath,appCmdLine());
				hr = XLiveUpdateSystem(*RelaunchCmdLine);
				debugf(NAME_DevOnline,TEXT("XLiveUpdateSystem(%s) returned 0x%08X"),*RelaunchCmdLine,hr);
				// Shut things down
				appRequestExit(FALSE); 
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("Game update path is %s"),LiveUpdate.szUpdateDownloadPath);
			TCHAR FolderPath[MAX_PATH + 1];
			// Convert the CSILD path to a real path
			if (SUCCEEDED(SHGetFolderPath(NULL,CSIDL_LOCAL_APPDATA,NULL,NULL,FolderPath)))
			{
				// Append the path from the update
				FString LaunchPath(FolderPath);
				LaunchPath *= LiveUpdate.szUpdateDownloadPath;
				TArray<FString> Files;
				// Search for MSI files or EXE files
				GFileManager->FindFiles(Files,TEXT("*.msi"),TRUE,FALSE);
				if (Files.Num() == 0)
				{
					// Find exes instead
					GFileManager->FindFiles(Files,TEXT("*.exe"),TRUE,FALSE);
					if (Files.Num() == 0)
					{
						debugf(NAME_DevOnline,TEXT("No update EXE or MSI files were found"));
					}
				}
				// For each returned installer, launch it
				for (INT Index = 0; Index < Files.Num(); Index++)
				{
					FString FullPath(LaunchPath * Files(Index));
					debugf(NAME_DevOnline,TEXT("Launching update installer (%s)"),*FullPath);
					appLaunchURL(*FullPath);
					// Shut things down
					appRequestExit(FALSE);
				}
			}
		}
	}
}
#endif

/**
 * Iterates the list of outstanding tasks checking for their completion
 * status. Upon completion, it fires off the corresponding delegate
 * notification
 */
void UOnlineSubsystemLive::TickAsyncTasks(void)
{
	// Check each task for completion
	for (INT Index = 0; Index < AsyncTasks.Num(); Index++)
	{
		if (AsyncTasks(Index)->HasTaskCompleted())
		{
			// Perform any task specific finalization of data before having
			// the script delegate fired off
			if (AsyncTasks(Index)->ProcessAsyncResults(this) == TRUE)
			{
				// Have the task fire off its delegates on our object
				AsyncTasks(Index)->TriggerDelegates(this);
#if !FINAL_RELEASE && !SHIPPING_PC_GAME
				AsyncTasks(Index)->LogResults();
#endif
				// Free the memory and remove it from our list
				delete AsyncTasks(Index);
				AsyncTasks.Remove(Index);
				Index--;
			}
		}
	}
}

/**
 * Ticks voice subsystem for reading/submitting any voice data
 *
 * @param DeltaTime the time since the last tick
 */
void UOnlineSubsystemLive::TickVoice(FLOAT DeltaTime)
{
	if (VoiceEngine != NULL)
	{
		// Allow the voice engine to do some stuff ahead of time
		VoiceEngine->Tick(DeltaTime);
		// If we aren't using VDP or aren't in a networked match, no need to update
		// networked voice
		if (GSocketSubsystem->RequiresChatDataBeSeparate() &&
			Sessions.Num())
		{
			// Queue local packets for sending via the network
			ProcessLocalVoicePackets();
			// Submit queued packets to XHV
			ProcessRemoteVoicePackets();
			// Fire off any talking notifications for hud display
			ProcessTalkingDelegates();
		}
	}
	// Check the speech recognition engine for pending notifications
	ProcessSpeechRecognitionDelegates();
}

/**
 * Reads any data that is currently queued in XHV
 */
void UOnlineSubsystemLive::ProcessLocalVoicePackets(void)
{
	if (VoiceEngine != NULL)
	{
		// Read the data from any local talkers
		DWORD DataReadyFlags = VoiceEngine->GetVoiceDataReadyFlags();
		// Skip processing if there is no data from a local talker
		if (DataReadyFlags)
		{
			// Process each talker with a bit set
			for (DWORD Index = 0; DataReadyFlags; Index++, DataReadyFlags >>= 1)
			{
				// Talkers needing processing will always be in lsb due to shifts
				if (DataReadyFlags & 1)
				{
					// Mark the person as talking
					LocalTalkers[Index].bWasTalking = TRUE;
					DWORD SpaceAvail = MAX_VOICE_DATA_SIZE - GVoiceData.LocalPackets[Index].Length;
					// Figure out if there is space for this packet
					if (SpaceAvail > 0)
					{
						DWORD NumPacketsCopied = 0;
						// Figure out where to append the data
						BYTE* BufferStart = GVoiceData.LocalPackets[Index].Buffer;
						BufferStart += GVoiceData.LocalPackets[Index].Length;
						// Copy the sender info
						XUserGetXUID(Index,(XUID*)&GVoiceData.LocalPackets[Index].Sender);
						// Process this user
						HRESULT hr = VoiceEngine->ReadLocalVoiceData(Index,
							BufferStart,
							&SpaceAvail);
						if (SUCCEEDED(hr))
						{
							if (LocalTalkers[Index].bHasNetworkedVoice)
							{
								// Update the length based on what it copied
								GVoiceData.LocalPackets[Index].Length += SpaceAvail;
							}
							else
							{
								// Zero out the data since it isn't to be sent via the network
								GVoiceData.LocalPackets[Index].Length = 0;
							}
						}
					}
					else
					{
						debugfLiveSlow(NAME_DevOnline,TEXT("Dropping voice data due to network layer not processing fast enough"));
						// Buffer overflow, so drop previous data
						GVoiceData.LocalPackets[Index].Length = 0;
					}
				}
			}
		}
	}
}

/**
 * Submits network packets to XHV for playback
 */
void UOnlineSubsystemLive::ProcessRemoteVoicePackets(void)
{
	// Now process all pending packets from the server
	for (INT Index = 0; Index < GVoiceData.RemotePackets.Num(); Index++)
	{
		FVoicePacket* VoicePacket = GVoiceData.RemotePackets(Index);
		if (VoicePacket != NULL)
		{
			// Skip local submission of voice if dedicated server or no voice
			if (VoiceEngine != NULL)
			{
				// Get the size since it is an in/out param
				DWORD PacketSize = VoicePacket->Length;
				// Submit this packet to the XHV engine
				HRESULT hr = VoiceEngine->SubmitRemoteVoiceData(VoicePacket->Sender,
					VoicePacket->Buffer,
					&PacketSize);
				if (FAILED(hr))
				{
					debugf(NAME_DevOnline,
						TEXT("SubmitRemoteVoiceData(0x%016I64X) failed with 0x%08X"),
						(QWORD&)VoicePacket->Sender,
						hr);
				}
			}
			// Skip all delegate handling if none are registered
			if (TalkingDelegates.Num() > 0)
			{
				// Find the remote talker and mark them as talking
				for (INT Index2 = 0; Index2 < RemoteTalkers.Num(); Index2++)
				{
					FLiveRemoteTalker& Talker = RemoteTalkers(Index2);
					// Compare the xuids
					if (Talker.TalkerId == VoicePacket->Sender)
					{
						Talker.bWasTalking = TRUE;
					}
				}
			}
			VoicePacket->DecRef();
		}
	}
	// Zero the list without causing a free/realloc
	GVoiceData.RemotePackets.Reset();
}

/**
 * Processes any talking delegates that need to be fired off
 */
void UOnlineSubsystemLive::ProcessTalkingDelegates(void)
{
	// Skip all delegate handling if none are registered
	if (TalkingDelegates.Num() > 0)
	{
		// Fire off any talker notification delegates for local talkers
		for (DWORD Index = 0; Index < 4; Index++)
		{
			// Only check players with voice
			if (LocalTalkers[Index].bHasVoice && LocalTalkers[Index].bWasTalking)
			{
				OnlineSubsystemLive_eventOnPlayerTalking_Parms Parms(EC_EventParm);
				// Read the XUID from Live
				DWORD Return = XUserGetXUID(Index,(XUID*)&Parms.Player);
				if (Return == ERROR_SUCCESS)
				{
					TriggerOnlineDelegates(this,TalkingDelegates,&Parms);
				}
				// Clear the flag so it only activates when needed
				LocalTalkers[Index].bWasTalking = FALSE;
			}
		}
		// Now check all remote talkers
		for (INT Index = 0; Index < RemoteTalkers.Num(); Index++)
		{
			FLiveRemoteTalker& Talker = RemoteTalkers(Index);
			// Make sure to not fire off the delegate if this talker is muted
			if (Talker.bWasTalking && Talker.IsLocallyMuted() == FALSE)
			{
				OnlineSubsystemLive_eventOnPlayerTalking_Parms Parms(EC_EventParm);
				Parms.Player = Talker.TalkerId;
				TriggerOnlineDelegates(this,TalkingDelegates,&Parms);
				// Clear the flag so it only activates when needed
				Talker.bWasTalking = FALSE;
			}
		}
	}
}

/**
 * Processes any speech recognition delegates that need to be fired off
 */
void UOnlineSubsystemLive::ProcessSpeechRecognitionDelegates(void)
{
	// Skip all delegate handling if we aren't using speech recognition
	if (bIsUsingSpeechRecognition && VoiceEngine != NULL)
	{
		// Fire off any talker notification delegates for local talkers
		for (DWORD Index = 0; Index < 4; Index++)
		{
			if (VoiceEngine->HasRecognitionCompleted(Index))
			{
				TriggerOnlineDelegates(this,PerUserDelegates[Index].SpeechRecognitionDelegates,NULL);
			}
		}
	}
}

/**
 * Processes a system link packet. For a host, responds to discovery
 * requests. For a client, parses the discovery response and places
 * the resultant data in the current search's search results array
 *
 * @param PacketData the packet data to parse
 * @param PacketLength the amount of data that was received
 */
void UOnlineSubsystemLive::ProcessLanPacket(BYTE* PacketData,INT PacketLength)
{
	// Check our mode to determine the type of allowed packets
	if (LanBeaconState == LANB_Hosting)
	{
		QWORD ClientNonce;
		// We can only accept Server Query packets
		if (IsValidLanQueryPacket(PacketData,PacketLength,ClientNonce))
		{
			// Iterate through all registered sessions and respond for each LAN match
			for (INT SessionIndex = 0; SessionIndex < Sessions.Num(); SessionIndex++)
			{
				FNamedSession* Session = &Sessions(SessionIndex);
				// Don't respond to queries when the match is full or it's not a lan match
				if (Session &&
					Session->GameSettings &&
					Session->GameSettings->bIsLanMatch &&
					Session->GameSettings->NumOpenPublicConnections > 0)
				{
					FNboSerializeToBufferXe Packet;
					// Add the supported version
					Packet << LAN_BEACON_PACKET_VERSION
						// Platform information
						<< (BYTE)appGetPlatformType()
						// Game id to prevent cross game lan packets
						<< LanGameUniqueId
						// Add the packet type
						<< LAN_SERVER_RESPONSE1 << LAN_SERVER_RESPONSE2
						// Append the client nonce as a QWORD
						<< ClientNonce;
					FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
					// Write host info (host addr, session id, and key)
					Packet << SessionInfo->XSessionInfo.hostAddress
						<< SessionInfo->XSessionInfo.sessionID
						<< SessionInfo->XSessionInfo.keyExchangeKey;
					// Now append per game settings
					AppendGameSettingsToPacket(Packet,Session->GameSettings);
					// Broadcast this response so the client can see us
					if (LanBeacon->BroadcastPacket(Packet,Packet.GetByteCount()) == FALSE)
					{
						debugfLiveSlow(NAME_DevOnline,TEXT("Failed to send response packet %d"),
							GSocketSubsystem->GetLastErrorCode());
					}
				}
			}
		}
	}
	else if (LanBeaconState == LANB_Searching)
	{
		// We can only accept Server Response packets
		if (IsValidLanResponsePacket(PacketData,PacketLength))
		{
			// Create an object that we'll copy the data to
			UOnlineGameSettings* NewServer = ConstructObject<UOnlineGameSettings>(
				GameSearch->GameSettingsClass);
			if (NewServer != NULL)
			{
				// Add space in the search results array
				INT NewSearch = GameSearch->Results.Add();
				FOnlineGameSearchResult& Result = GameSearch->Results(NewSearch);
				// Link the settings to this result
				Result.GameSettings = NewServer;
				// Strip off the type and nonce since it's been validated
				FNboSerializeFromBufferXe Packet(&PacketData[LAN_BEACON_PACKET_HEADER_SIZE],
					PacketLength - LAN_BEACON_PACKET_HEADER_SIZE);
				// Allocate and read the session data
				XSESSION_INFO* SessInfo = new XSESSION_INFO;
				// Read the connection data
				Packet >> SessInfo->hostAddress
					>> SessInfo->sessionID
					>> SessInfo->keyExchangeKey;
				// Store this in the results
				Result.PlatformData = SessInfo;
				// Read any per object data using the server object
				ReadGameSettingsFromPacket(Packet,NewServer);
				// Allow game code to sort the servers
				GameSearch->eventSortSearchResults();
				// Let any registered consumers know the data has changed
				FAsyncTaskDelegateResults Results(S_OK);
				TriggerOnlineDelegates(this,FindOnlineGamesCompleteDelegates,&Results);
			}
			else
			{
				debugfLiveSlow(NAME_DevOnline,TEXT("Failed to create new online game settings object"));
			}
		}
	}
}

/**
 * Displays the Xbox Guide to perform the login
 *
 * @param bShowOnlineOnly whether to only display online enabled profiles or not
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
UBOOL UOnlineSubsystemLive::ShowLoginUI(UBOOL bShowOnlineOnly)
{
	// We allow 1, 2, or 4 on Xe. Should be 1 for Windows Live
	if (NumLogins != 1 && NumLogins != 2 && NumLogins != 4)
	{
		NumLogins = 1;
	}
	DWORD Result = XShowSigninUI(NumLogins,bShowOnlineOnly ? XSSUI_FLAGS_SHOWONLYONLINEENABLED : 0);
	if (Result == ERROR_SUCCESS)
	{
		bIsInSignInUI = TRUE;
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("XShowSigninUI(%d,0) failed with 0x%08X"),NumLogins,Result);
	}
	return Result == ERROR_SUCCESS;
}

/**
 * Logs the player into the online service. If this fails, it generates a
 * OnLoginFailed notification
 *
 * @param LocalUserNum the controller number of the associated user
 * @param LoginName the unique identifier for the player
 * @param Password the password for this account
 * @param ignored
 *
 * @return true if the async call started ok, false otherwise
 */
UBOOL UOnlineSubsystemLive::Login(BYTE LocalUserNum,const FString& LoginName,const FString& Password,UBOOL)
{
	HRESULT Return = E_NOTIMPL;
#if WITH_PANORAMA
	// Create a simple async task
	FLiveAsyncTaskSignin* AsyncTask = new FLiveAsyncTaskSignin();
	// Now try to sign them in
	Return = XLiveSignin((LPWSTR)*LoginName,
		(LPWSTR)*Password,
		XLSIGNIN_FLAG_ALLOWTITLEUPDATES | XLSIGNIN_FLAG_ALLOWSYSTEMUPDATES,
		*AsyncTask);
	debugfLiveSlow(NAME_DevOnline,TEXT("XLiveSignin(%s,...) returned 0x%08X"),*LoginName,Return);
	if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
	{
		AsyncTasks.AddItem(AsyncTask);
	}
	else
	{
		// Don't leak the task/data
		delete AsyncTask;
	}
#endif
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Logs the player into the online service using parameters passed on the
 * command line. Expects -Login=<UserName> -Password=<password>. If either
 * are missing, the function returns false and doesn't start the login
 * process
 *
 * @return true if the async call started ok, false otherwise
 */
UBOOL UOnlineSubsystemLive::AutoLogin(void)
{
	UBOOL bReturn = FALSE;
	FString LiveId;
	// Check to see if they specified a login
	if (Parse(appCmdLine(),TEXT("-Login="),LiveId))
	{
		FString LivePassword;
		// Make sure there is a password too
		if (Parse(appCmdLine(),TEXT("-Password="),LivePassword))
		{
			bReturn = Login(0,LiveId,LivePassword);
		}
	}
	return bReturn;
}

/**
 * Signs the player out of the online service
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::Logout(BYTE LocalUserNum)
{
	HRESULT Return = E_NOTIMPL;
#if WITH_PANORAMA
	// Create a simple async task
	FLiveAsyncTaskSignout* AsyncTask = new FLiveAsyncTaskSignout();
	// Sign the player out
	Return = XLiveSignout(*AsyncTask);
	debugfLiveSlow(NAME_DevOnline,TEXT("XLiveSignout() returned 0x%08X"),Return);
	if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
	{
		AsyncTasks.AddItem(AsyncTask);
	}
	else
	{
		// Don't leak the task/data
		delete AsyncTask;
	}
#endif
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Fetches the login status for a given player from Live
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return the enum value of their status
 */
BYTE UOnlineSubsystemLive::GetLoginStatus(BYTE LocalUserNum)
{
	ELoginStatus Status = LS_NotLoggedIn;
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Ask Live for the login status
		XUSER_SIGNIN_STATE State = XUserGetSigninState(LocalUserNum);
		if (State == eXUserSigninState_SignedInToLive)
		{
			Status = LS_LoggedIn;
		}
		else if (State == eXUserSigninState_SignedInLocally)
		{
			Status = LS_UsingLocalProfile;
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to GetLoginStatus()"),
			(DWORD)LocalUserNum);
	}
	return Status;
}

/**
 * Determines whether the specified user is a guest login or not
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return true if a guest, false otherwise
 */
UBOOL UOnlineSubsystemLive::IsGuestLogin(BYTE LocalUserNum)
{
	UBOOL bIsGuest = FALSE;
	// Validate the user number
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		XUSER_SIGNIN_INFO SigninInfo;
		// Ask live for the signin flags that indicate a guest or not
		if (XUserGetSigninInfo(LocalUserNum,0,&SigninInfo) == ERROR_SUCCESS)
		{
			bIsGuest = (SigninInfo.dwInfoFlags & XUSER_INFO_FLAG_GUEST) ? TRUE : FALSE;
		}
	}
	return bIsGuest;
}

/**
 * Determines whether the specified user is a local (non-online) login or not
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return true if a local profile, false otherwise
 */
UBOOL UOnlineSubsystemLive::IsLocalLogin(BYTE LocalUserNum)
{
	UBOOL bIsLocal = FALSE;
	// Validate the user number
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		XUSER_SIGNIN_INFO SigninInfo;
		// Ask live for the signin flags that indicate a local profile or not
		if (XUserGetSigninInfo(LocalUserNum,0,&SigninInfo) == ERROR_SUCCESS)
		{
			bIsLocal = (SigninInfo.dwInfoFlags & XUSER_INFO_FLAG_LIVE_ENABLED) ? FALSE : TRUE;
		}
	}
	return bIsLocal;
}

/**
 * Gets the platform specific unique id for the specified player
 *
 * @param LocalUserNum the controller number of the associated user
 * @param UniqueId the byte array that will receive the id
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::GetUniquePlayerId(BYTE LocalUserNum,FUniqueNetId& UniqueId)
{
	check(sizeof(XUID) == 8);
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Read the XUID from Live
		Result = XUserGetXUID(LocalUserNum,(XUID*)&UniqueId);
		if (Result != ERROR_SUCCESS)
		{
			debugf(NAME_DevOnline,TEXT("XUserGetXUID(%d) failed 0x%08X"),LocalUserNum,Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to GetUniquePlayerId()"),
			(DWORD)LocalUserNum);
	}
	return Result == ERROR_SUCCESS;
}

/**
 * Reads the player's nick name from the online service
 *
 * @param UniqueId the unique id of the player being queried
 *
 * @return a string containing the players nick name
 */
FString UOnlineSubsystemLive::GetPlayerNickname(BYTE LocalUserNum)
{
	ANSICHAR Buffer[32];
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Read the gamertag from Live
		Result = XUserGetName(LocalUserNum,Buffer,sizeof(Buffer));
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to GetPlayerNickname()"),
			(DWORD)LocalUserNum);
	}
	if (Result != ERROR_SUCCESS)
	{
		debugf(NAME_DevOnline,TEXT("XUserGetName(%d) failed 0x%08X"),LocalUserNum,Result);
		Buffer[0] = '\0';
	}
	return FString(ANSI_TO_TCHAR(Buffer));
}

/**
 * Determines whether the player is allowed to play online
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return the Privilege level that is enabled
 */
BYTE UOnlineSubsystemLive::CanPlayOnline(BYTE LocalUserNum)
{
	// Default to enabled for non-Live accounts
	EFeaturePrivilegeLevel Priv = FPL_Enabled;
	BOOL bCan;
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Check for the priveledge
		Result = XUserCheckPrivilege(LocalUserNum,XPRIVILEGE_MULTIPLAYER_SESSIONS,
			&bCan);
		if (Result == ERROR_SUCCESS)
		{
			Priv = bCan == TRUE ? FPL_Enabled : FPL_Disabled;
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("XUserCheckPrivilege(%d,XPRIVILEGE_MULTIPLAYER_SESSIONS) failed with 0x%08X"),
				LocalUserNum,Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to CanPlayOnline()"),
			(DWORD)LocalUserNum);
		// Force it off because this is a bogus player
		Priv = FPL_Disabled;
	}
	return Priv;
}

/**
 * Determines whether the player is allowed to use voice or text chat online
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return the Privilege level that is enabled
 */
BYTE UOnlineSubsystemLive::CanCommunicate(BYTE LocalUserNum)
{
	// Default to enabled for non-Live accounts
	EFeaturePrivilegeLevel Priv = FPL_Enabled;
	BOOL bCan;
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Check for the priveledge
		Result = XUserCheckPrivilege(LocalUserNum,XPRIVILEGE_COMMUNICATIONS,
			&bCan);
		if (Result == ERROR_SUCCESS)
		{
			if (bCan == TRUE)
			{
				// Universally ok
				Priv = FPL_Enabled;
			}
			else
			{
				// Not valid for everyone so check for friends only
				Result = XUserCheckPrivilege(LocalUserNum,
					XPRIVILEGE_COMMUNICATIONS_FRIENDS_ONLY,&bCan);
				if (Result == ERROR_SUCCESS)
				{
					// Can only do this with friends or not at all
					Priv = bCan == TRUE ? FPL_EnabledFriendsOnly : FPL_Disabled;
				}
				else
				{
					debugf(NAME_DevOnline,TEXT("XUserCheckPrivilege(%d,XPRIVILEGE_COMMUNICATIONS_FRIENDS_ONLY) failed with 0x%08X"),
						LocalUserNum,Result);
				}
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("XUserCheckPrivilege(%d,XPRIVILEGE_COMMUNICATIONS) failed with 0x%08X"),
				LocalUserNum,Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to CanCommunicate()"),
			(DWORD)LocalUserNum);
		// Force it off because this is a bogus player
		Priv = FPL_Disabled;
	}
	return Priv;
}

/**
 * Determines whether the player is allowed to download user created content
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return the Privilege level that is enabled
 */
BYTE UOnlineSubsystemLive::CanDownloadUserContent(BYTE LocalUserNum)
{
	// Default to enabled for non-Live accounts
	EFeaturePrivilegeLevel Priv = FPL_Enabled;
	BOOL bCan;
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Check for the priveledge
		Result = XUserCheckPrivilege(LocalUserNum,
			XPRIVILEGE_USER_CREATED_CONTENT,&bCan);
		if (Result == ERROR_SUCCESS)
		{
			if (bCan == TRUE)
			{
				// Universally ok
				Priv = FPL_Enabled;
			}
			else
			{
				// Not valid for everyone so check for friends only
				Result = XUserCheckPrivilege(LocalUserNum,
					XPRIVILEGE_USER_CREATED_CONTENT_FRIENDS_ONLY,&bCan);
				if (Result == ERROR_SUCCESS)
				{
					// Can only do this with friends or not at all
					Priv = bCan == TRUE ? FPL_EnabledFriendsOnly : FPL_Disabled;
				}
				else
				{
					debugf(NAME_DevOnline,TEXT("XUserCheckPrivilege(%d,XPRIVILEGE_USER_CREATED_CONTENT_FRIENDS_ONLY) failed with 0x%08X"),
						LocalUserNum,Result);
				}
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("XUserCheckPrivilege(%d,XPRIVILEGE_USER_CREATED_CONTENT) failed with 0x%08X"),
				LocalUserNum,Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to CanDownloadUserContent()"),
			(DWORD)LocalUserNum);
		// Force it off because this is a bogus player
		Priv = FPL_Disabled;
	}
	return Priv;
}

/**
 * Determines whether the player is allowed to view other people's player profile
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return the Privilege level that is enabled
 */
BYTE UOnlineSubsystemLive::CanViewPlayerProfiles(BYTE LocalUserNum)
{
	// Default to enabled for non-Live accounts
	EFeaturePrivilegeLevel Priv = FPL_Enabled;
	BOOL bCan;
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Check for the priveledge
		Result = XUserCheckPrivilege(LocalUserNum,XPRIVILEGE_PROFILE_VIEWING,
			&bCan);
		if (Result == ERROR_SUCCESS)
		{
			if (bCan == TRUE)
			{
				// Universally ok
				Priv = FPL_Enabled;
			}
			else
			{
				// Not valid for everyone so check for friends only
				Result = XUserCheckPrivilege(LocalUserNum,
					XPRIVILEGE_PROFILE_VIEWING_FRIENDS_ONLY,&bCan);
				if (Result == ERROR_SUCCESS)
				{
					// Can only do this with friends or not at all
					Priv = bCan == TRUE ? FPL_EnabledFriendsOnly : FPL_Disabled;
				}
				else
				{
					debugf(NAME_DevOnline,TEXT("XUserCheckPrivilege(%d,XPRIVILEGE_PROFILE_VIEWING_FRIENDS_ONLY) failed with 0x%08X"),
						LocalUserNum,Result);
				}
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("XUserCheckPrivilege(%d,XPRIVILEGE_PROFILE_VIEWING) failed with 0x%08X"),
				LocalUserNum,Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to CanViewPlayerProfiles()"),
			(DWORD)LocalUserNum);
		// Force it off because this is a bogus player
		Priv = FPL_Disabled;
	}
	return Priv;
}

/**
 * Determines whether the player is allowed to have their online presence
 * information shown to remote clients
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return the Privilege level that is enabled
 */
BYTE UOnlineSubsystemLive::CanShowPresenceInformation(BYTE LocalUserNum)
{
	// Default to enabled for non-Live accounts
	EFeaturePrivilegeLevel Priv = FPL_Enabled;
	BOOL bCan;
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Check for the priveledge
		Result = XUserCheckPrivilege(LocalUserNum,XPRIVILEGE_PRESENCE,
			&bCan);
		if (Result == ERROR_SUCCESS)
		{
			if (bCan == TRUE)
			{
				// Universally ok
				Priv = FPL_Enabled;
			}
			else
			{
				// Not valid for everyone so check for friends only
				Result = XUserCheckPrivilege(LocalUserNum,
					XPRIVILEGE_PRESENCE_FRIENDS_ONLY,&bCan);
				if (Result == ERROR_SUCCESS)
				{
					// Can only do this with friends or not at all
					Priv = bCan == TRUE ? FPL_EnabledFriendsOnly : FPL_Disabled;
				}
				else
				{
					debugf(NAME_DevOnline,TEXT("XUserCheckPrivilege(%d,XPRIVILEGE_PRESENCE_FRIENDS_ONLY) failed with 0x%08X"),
						LocalUserNum,Result);
				}
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("XUserCheckPrivilege(%d,XPRIVILEGE_PRESENCE) failed with 0x%08X"),
				LocalUserNum,Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to CanShowPresenceInformation()"),
			(DWORD)LocalUserNum);
		// Force it off because this is a bogus player
		Priv = FPL_Disabled;
	}
	return Priv;
}

/**
 * Determines whether the player is allowed to purchase downloadable content
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return the Privilege level that is enabled
 */
BYTE UOnlineSubsystemLive::CanPurchaseContent(BYTE LocalUserNum)
{
	// Default to enabled for non-Live accounts
	EFeaturePrivilegeLevel Priv = FPL_Enabled;
	BOOL bCan;
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Check for the priveledge
		Result = XUserCheckPrivilege(LocalUserNum,XPRIVILEGE_PURCHASE_CONTENT,
			&bCan);
		if (Result == ERROR_SUCCESS)
		{
			Priv = bCan == TRUE ? FPL_Enabled : FPL_Disabled;
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("XUserCheckPrivilege(%d,XPRIVILEGE_PURCHASE_CONTENT) failed with 0x%08X"),
				LocalUserNum,Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to CanPurchaseContent()"),
			(DWORD)LocalUserNum);
		// Force it off because this is a bogus player
		Priv = FPL_Disabled;
	}
	return Priv;
}

/**
 * Checks that a unique player id is part of the specified user's friends list
 *
 * @param LocalUserNum the controller number of the associated user
 * @param UniqueId the id of the player being checked
 *
 * @return TRUE if a member of their friends list, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::IsFriend(BYTE LocalUserNum,FUniqueNetId UniqueId)
{
	check(sizeof(XUID) == 8);
	BOOL bIsFriend = FALSE;
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Ask Live if the local user is a friend of the specified player
		Result = XUserAreUsersFriends(LocalUserNum,(XUID*)&UniqueId,1,
			&bIsFriend,NULL);
		if (Result != ERROR_SUCCESS)
		{
			debugf(NAME_DevOnline,TEXT("XUserAreUsersFriends(%d,(0x%016I64X)) failed with 0x%08X"),
				LocalUserNum,UniqueId.Uid,Result);
			bIsFriend = FALSE;
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to IsFriend()"),
			(DWORD)LocalUserNum);
	}
	return bIsFriend;
}

/**
 * Checks that whether a group of player ids are among the specified player's
 * friends
 *
 * @param LocalUserNum the controller number of the associated user
 * @param Query an array of players to check for being included on the friends list
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::AreAnyFriends(BYTE LocalUserNum,TArray<FFriendsQuery>& Query)
{
	check(sizeof(XUID) == 8);
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Perform the check for each query
		for (INT Index = 0; Index < Query.Num(); Index++)
		{
			BOOL bIsFriend;
			// Ask Live if the local user is a friend of the specified player
			Result = XUserAreUsersFriends(LocalUserNum,(XUID*)&Query(Index).UniqueId,1,
				&bIsFriend,NULL);
			if (Result != ERROR_SUCCESS)
			{
				Query(Index).bIsFriend = bIsFriend;
			}
			else
			{
				debugf(NAME_DevOnline,TEXT("XUserAreUsersFriends(%d,(0x%016I64X)) failed with 0x%08X"),
					LocalUserNum,Query(Index).UniqueId.Uid,Result);
				// Failure means no friendship
				Query(Index).bIsFriend = FALSE;
				break;
			}
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to AreAnyFriends()"),
			(DWORD)LocalUserNum);
	}
	return Result == ERROR_SUCCESS;
}

/**
 * Checks that a unique player id is on the specified user's mute list
 *
 * @param LocalUserNum the controller number of the associated user
 * @param UniqueId the id of the player being checked
 *
 * @return TRUE if a member of their friends list, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::IsMuted(BYTE LocalUserNum,FUniqueNetId UniqueId)
{
	check(sizeof(XUID) == 8);
	BOOL bIsMuted = FALSE;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Ask Live if the local user has muted the specified player
		XUserMuteListQuery(LocalUserNum,(XUID&)UniqueId,&bIsMuted);
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to IsMuted()"),
			(DWORD)LocalUserNum);
	}
	return bIsMuted;
}

/**
 * Displays the Xbox Guide Friends UI
 *
 * @param LocalUserNum the controller number of the user where are showing the friends for
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
UBOOL UOnlineSubsystemLive::ShowFriendsUI(BYTE LocalUserNum)
{
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Show the friends UI for the specified controller num
		Result = XShowFriendsUI(LocalUserNum);
		if (Result != ERROR_SUCCESS)
		{
			debugf(NAME_DevOnline,TEXT("XShowFriendsUI(%d) failed with 0x%08X"),
				LocalUserNum,Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to ShowFriendsUI()"),
			(DWORD)LocalUserNum);
	}
	return Result == ERROR_SUCCESS;
}

/**
 * Displays the Xbox Guide Friends Request UI
 *
 * @param LocalUserNum the controller number of the user where are showing the friends for
 * @param UniqueId the id of the player being invited (null or 0 to have UI pick)
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
UBOOL UOnlineSubsystemLive::ShowFriendsInviteUI(BYTE LocalUserNum,FUniqueNetId UniqueId)
{
	check(sizeof(XUID) == 8);
	// Figure out whether to use a specific XUID or not
	XUID RequestedId = (XUID&)UniqueId;
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Show the friends UI for the specified controller num and player
		Result = XShowFriendRequestUI(LocalUserNum,RequestedId);
		if (Result != ERROR_SUCCESS)
		{
			BYTE* Xuid = (BYTE*)&RequestedId;
			debugf(NAME_DevOnline,
				TEXT("XShowFriendsRequestUI(%d,) failed with 0x%08X"),
				LocalUserNum,
				Xuid,
				Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to ShowFriendsInviteUI()"),
			(DWORD)LocalUserNum);
	}
	return Result == ERROR_SUCCESS;
}

/**
 * Displays the UI that allows a player to give feedback on another player
 *
 * @param LocalUserNum the controller number of the associated user
 * @param UniqueId the id of the player having feedback given for
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
UBOOL UOnlineSubsystemLive::ShowFeedbackUI(BYTE LocalUserNum,FUniqueNetId UniqueId)
{
	check(sizeof(XUID) == 8);
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Show the live guide ui for player review
		Result = XShowPlayerReviewUI(LocalUserNum,(XUID&)UniqueId);
		if (Result != ERROR_SUCCESS)
		{
			BYTE* Xuid = (BYTE*)&UniqueId;
			debugf(NAME_DevOnline,
				TEXT("XShowPlayerReviewUI(%d,0x%016I64X) failed with 0x%08X"),
				LocalUserNum,
				Xuid,
				Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to ShowFeedbackUI()"),
			(DWORD)LocalUserNum);
	}
	return Result == ERROR_SUCCESS;
}

/**
 * Displays the gamer card UI for the specified player
 *
 * @param LocalUserNum the controller number of the associated user
 * @param UniqueId the id of the player to show the gamer card of
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
UBOOL UOnlineSubsystemLive::ShowGamerCardUI(BYTE LocalUserNum,FUniqueNetId UniqueId)
{
	check(sizeof(XUID) == 8);
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Show the live guide ui for gamer cards
		Result = XShowGamerCardUI(LocalUserNum,(XUID&)UniqueId);
		if (Result != ERROR_SUCCESS)
		{
			BYTE* Xuid = (BYTE*)&UniqueId;
			debugf(NAME_DevOnline,
				TEXT("XShowGamerCardUI(%d,0x%016I64X) failed with 0x%08X"),
				LocalUserNum,
				Xuid,
				Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to ShowGamerCardUI()"),
			(DWORD)LocalUserNum);
	}
	return Result == ERROR_SUCCESS;
}

/**
 * Displays the messages UI for a player
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
UBOOL UOnlineSubsystemLive::ShowMessagesUI(BYTE LocalUserNum)
{
	// Show the live guide ui for player messages
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		Result = XShowMessagesUI(LocalUserNum);
		if (Result != ERROR_SUCCESS)
		{
			debugf(NAME_DevOnline,
				TEXT("XShowMessagesUI(%d) failed with 0x%08X"),
				LocalUserNum,
				Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to ShowMessagesUI()"),
			(DWORD)LocalUserNum);
	}
	return Result == ERROR_SUCCESS;
}

/**
 * Displays the achievements UI for a player
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
UBOOL UOnlineSubsystemLive::ShowAchievementsUI(BYTE LocalUserNum)
{
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Show the live guide ui for player achievements
		Result = XShowAchievementsUI(LocalUserNum);
		if (Result != ERROR_SUCCESS)
		{
			debugf(NAME_DevOnline,TEXT("XShowAchievementsUI(%d) failed with 0x%08X"),
				LocalUserNum,Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to ShowAchievementsUI()"),
			(DWORD)LocalUserNum);
	}
	return Result == ERROR_SUCCESS;
}

/**
 * Displays the Live Guide
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
UBOOL UOnlineSubsystemLive::ShowGuideUI()
{
#if WITH_PANORAMA
	DWORD Result = XShowGuideUI( 0 );
	if (Result != ERROR_SUCCESS)
	{
		debugf(NAME_DevOnline,TEXT("XShowGuideUI(0) failed with 0x%08X"),Result);
	}
	return Result == ERROR_SUCCESS;
#else
	return FALSE;
#endif
}

/**
 * Displays the achievements UI for a player
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
UBOOL UOnlineSubsystemLive::ShowPlayersUI(BYTE LocalUserNum)
{
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Show the live guide ui for the player list
		Result = XShowPlayersUI(LocalUserNum);
		if (Result != ERROR_SUCCESS)
		{
			debugf(NAME_DevOnline,TEXT("XShowPlayersUI(%d) failed with 0x%08X"),
				LocalUserNum,Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to ShowPlayersUI()"),
			(DWORD)LocalUserNum);
	}
	return Result == ERROR_SUCCESS;
}

/**
 * Displays the UI that shows the keyboard for inputing text
 *
 * @param LocalUserNum the controller number of the associated user
 * @param TitleText the title to display to the user
 * @param DescriptionText the text telling the user what to input
 * @param bIsPassword whether the item being entered is a password or not
 * @param bShouldValidate whether to apply the string validation API after input or not
 * @param DefaultText the default string to display
 * @param MaxResultLength the maximum length string expected to be filled in
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
UBOOL UOnlineSubsystemLive::ShowKeyboardUI(BYTE LocalUserNum,
	const FString& TitleText,const FString& DescriptionText,UBOOL bIsPassword,
	UBOOL bShouldValidate,const FString& DefaultText,INT MaxResultLength)
{
	DWORD Return = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		if (MaxResultLength > 0)
		{
			DWORD Flags = VKBD_HIGHLIGHT_TEXT;
#if !WITH_PANORAMA
			// Determine which keyboard features based upon dash language
			switch (XGetLanguage())
			{
				case XC_LANGUAGE_JAPANESE:
				{
					Flags |= VKBD_JAPANESE_FULL;
					break;
				}
				case XC_LANGUAGE_KOREAN:
				{
					Flags |= VKBD_KOREAN_FULL;
					break;
				}
				case XC_LANGUAGE_TCHINESE:
				{
					Flags |= VKBD_TCH_FULL;
					break;
				}
				default:
				{
					Flags |= VKBD_LATIN_FULL;
					break;
				}
			}
#endif
			// Allow for password entry if requested
			if (bIsPassword)
			{
				Flags |= VKBD_LATIN_PASSWORD;
			}
			// Allocate an async task to hold the data while in process
			FLiveAsyncTaskDataKeyboard* AsyncData = new FLiveAsyncTaskDataKeyboard(TitleText,
				DefaultText,DescriptionText,MaxResultLength,bShouldValidate);
			FLiveAsyncTaskKeyboard* AsyncTask = new FLiveAsyncTaskKeyboard(&KeyboardInputDelegates,AsyncData);
			// Show the live guide ui for inputing text
			Return = XShowKeyboardUI(LocalUserNum,
				Flags,
				AsyncData->GetDefaultText(),
				AsyncData->GetTitleText(),
				AsyncData->GetDescriptionText(),
				*AsyncData,
				MaxResultLength,
				*AsyncTask);
			if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
			{
				AsyncTasks.AddItem(AsyncTask);
			}
			else
			{
				// Just trigger the delegate as having failed
				FAsyncTaskDelegateResults Results(Return);
				TriggerOnlineDelegates(this,KeyboardInputDelegates,&Results);
				// Don't leak the task/data
				delete AsyncTask;
				debugf(NAME_DevOnline,TEXT("XShowKeyboardUI(%d,%d,'%s','%s','%s',data,data,data) failed with 0x%08X"),
					LocalUserNum,Flags,*DefaultText,*TitleText,*DescriptionText,Return);
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("Invalid MaxResultLength"));
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to ShowKeyboardUI()"),
			(DWORD)LocalUserNum);
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Determines if the ethernet link is connected or not
 */
UBOOL UOnlineSubsystemLive::HasLinkConnection(void)
{
	return (XNetGetEthernetLinkStatus() & XNET_ETHERNET_LINK_ACTIVE) != 0;
}

/**
 * Sets a new position for the network notification icons/images
 *
 * @param NewPos the new location to use
 */
void UOnlineSubsystemLive::SetNetworkNotificationPosition(BYTE NewPos)
{
	CurrentNotificationPosition = NewPos;
	// Map our enum to Live's
	switch (CurrentNotificationPosition)
	{
		case NNP_TopLeft:
		{
			XNotifyPositionUI(XNOTIFYUI_POS_TOPLEFT);
			break;
		}
		case NNP_TopCenter:
		{
			XNotifyPositionUI(XNOTIFYUI_POS_TOPCENTER);
			break;
		}
		case NNP_TopRight:
		{
			XNotifyPositionUI(XNOTIFYUI_POS_TOPRIGHT);
			break;
		}
		case NNP_CenterLeft:
		{
			XNotifyPositionUI(XNOTIFYUI_POS_CENTERLEFT);
			break;
		}
		case NNP_Center:
		{
			XNotifyPositionUI(XNOTIFYUI_POS_CENTER);
			break;
		}
		case NNP_CenterRight:
		{
			XNotifyPositionUI(XNOTIFYUI_POS_CENTERRIGHT);
			break;
		}
		case NNP_BottomLeft:
		{
			XNotifyPositionUI(XNOTIFYUI_POS_BOTTOMLEFT);
			break;
		}
		case NNP_BottomCenter:
		{
			XNotifyPositionUI(XNOTIFYUI_POS_BOTTOMCENTER);
			break;
		}
		case NNP_BottomRight:
		{
			XNotifyPositionUI(XNOTIFYUI_POS_BOTTOMRIGHT);
			break;
		}
	}
}

/**
 * Determines if the specified controller is connected or not
 *
 * @param ControllerId the controller to query
 *
 * @return true if connected, false otherwise
 */
UBOOL UOnlineSubsystemLive::IsControllerConnected(INT ControllerId)
{
	return (LastInputDeviceConnectedMask & (1 << ControllerId)) ? TRUE : FALSE;
}

/**
 * Determines the NAT type the player is using
 */
BYTE UOnlineSubsystemLive::GetNATType(void)
{
	ENATType NatType = NAT_Unknown;
	// Ask the system for it
	switch (XOnlineGetNatType())
	{
		case XONLINE_NAT_OPEN:
			NatType = NAT_Open;
			break;

		case XONLINE_NAT_MODERATE:
			NatType = NAT_Moderate;
			break;

		case XONLINE_NAT_STRICT:
			NatType = NAT_Strict;
			break;
	}
	return NatType;
}

/**
 * Creates the session flags value from the game settings object. First looks
 * for the standard settings and then checks for Live specific settings
 *
 * @param InSettings the game settings of the new session
 *
 * @return the flags needed to set up the session
 */
DWORD UOnlineSubsystemLive::BuildSessionFlags(UOnlineGameSettings* InSettings)
{
	DWORD Flags = 0;
	// Base setting is that we are using the peer network (secure with
	// parental controls)
	Flags |= XSESSION_CREATE_HOST | XSESSION_CREATE_USES_PEER_NETWORK;
	// The flag checks below are only for normal Live (player/ranked) sessions
	if (InSettings->bIsLanMatch == FALSE)
	{
		// Whether to advertise the server or not
		if (InSettings->bShouldAdvertise == TRUE)
		{
			Flags |= XSESSION_CREATE_USES_MATCHMAKING;
		}
		// Whether to require arbitration or not
		if (InSettings->bUsesArbitration == TRUE)
		{
			Flags |= XSESSION_CREATE_USES_ARBITRATION;
		}
		// Whether to use stats or not
		if (InSettings->bUsesStats == TRUE)
		{
			Flags |= XSESSION_CREATE_USES_STATS;
		}
		// Check all of the flags that rely on presence information
		if (InSettings->bUsesPresence)
		{
			Flags |= XSESSION_CREATE_USES_PRESENCE;
#if CONSOLE
			// Check for the friends only flag
			if (InSettings->bAllowJoinViaPresenceFriendsOnly)
			{
				Flags |= XSESSION_CREATE_JOIN_VIA_PRESENCE_FRIENDS_ONLY;
			}
			else
#endif
			// Whether to allow join via presence information or not. NOTE: Friends only overrides
			if (InSettings->bAllowJoinViaPresence == FALSE)
			{
				Flags |= XSESSION_CREATE_JOIN_VIA_PRESENCE_DISABLED;
			}
			// Whether to allow invites or not
			if (InSettings->bAllowInvites == FALSE)
			{
				Flags |= XSESSION_CREATE_INVITES_DISABLED;
			}
			// Whether to allow join in progress or not
			if (InSettings->bAllowJoinInProgress == FALSE)
			{
				Flags |= XSESSION_CREATE_JOIN_IN_PROGRESS_DISABLED;
			}
		}
		else
		{
			// Can't join in progress if uses presence is disabled
			Flags |= XSESSION_CREATE_JOIN_IN_PROGRESS_DISABLED;
		}
	}
	return Flags;
}

/**
 * Sets the list contexts for the player
 *
 * @param PlayerNum the index of the player hosting the match
 * @param Contexts the list of contexts to set
 */
void UOnlineSubsystemLive::SetContexts(BYTE PlayerNum,const TArray<FLocalizedStringSetting>& Contexts)
{
	// Iterate through all contexts and set them
	for (INT Index = 0; Index < Contexts.Num(); Index++)
	{
		const FLocalizedStringSetting& Context = Contexts(Index);
		// Only publish fields that are meant to be advertised
		if (Context.AdvertisementType == ODAT_OnlineService ||
			Context.AdvertisementType == ODAT_OnlineServiceAndQoS)
		{
			// Set the context data
			DWORD Result = XUserSetContextEx(PlayerNum,Context.Id,Context.ValueIndex,NULL);
			// Log it for debug purposes
			debugf(NAME_DevOnline,TEXT("XUserSetContextEx(%d,0x%08X,%d,NULL) returned 0x%08X"),
				PlayerNum,Context.Id,Context.ValueIndex,Result);
		}
	}
}

/**
 * Sets the list properties for the player
 *
 * @param PlayerNum the index of the player hosting the match
 * @param Properties the list of properties to set
 */
void UOnlineSubsystemLive::SetProperties(BYTE PlayerNum,const TArray<FSettingsProperty>& Properties)
{
	// Iterate through all properties and set those too
	for (INT Index = 0; Index < Properties.Num(); Index++)
	{
		const FSettingsProperty& Property = Properties(Index);
		// Only publish fields that are meant to be advertised
		if (Property.AdvertisementType == ODAT_OnlineService ||
			Property.AdvertisementType == ODAT_OnlineServiceAndQoS)
		{
			// Get the size of data that we'll be sending
			DWORD SizeOfData = GetSettingsDataSize(Property.Data);
			// Get the pointer to the data we are sending
			const void* DataPointer = GetSettingsDataPointer(Property.Data);
			// Set the context data
			DWORD Result = XUserSetPropertyEx(PlayerNum,Property.PropertyId,SizeOfData,DataPointer,NULL);
#if !FINAL_RELEASE
			// Log it for debug purposes
			FString StringVal = Property.Data.ToString();
			debugf(NAME_DevOnline,TEXT("XUserSetPropertyEx(%d,0x%08X,%d,%s,NULL) returned 0x%08X"),
				PlayerNum,Property.PropertyId,SizeOfData,*StringVal,Result);
#endif
		}
	}
}

/**
 * Sets the contexts and properties for this game settings object
 *
 * @param PlayerNum the index of the player performing the search/hosting the match
 * @param GameSettings the game settings of the new session
 *
 * @return TRUE if successful, FALSE otherwise
 */
void UOnlineSubsystemLive::SetContextsAndProperties(BYTE PlayerNum,
	UOnlineGameSettings* InSettings)
{
	DWORD GameType = X_CONTEXT_GAME_TYPE_STANDARD;
	// Add arbitration flag if requested
	if (InSettings->bUsesArbitration == TRUE)
	{
		GameType = X_CONTEXT_GAME_TYPE_RANKED;
	}
	// Set the game type (standard or ranked)
	DWORD Result = XUserSetContextEx(PlayerNum,X_CONTEXT_GAME_TYPE,GameType,NULL);
	debugf(NAME_DevOnline,TEXT("XUserSetContextEx(%d,X_CONTEXT_GAME_TYPE,%d,NULL) returned 0x%08X"),
		PlayerNum,GameType,Result);
	// Use the common methods for setting the lists of contexts & properties
	SetContexts(PlayerNum,InSettings->LocalizedSettings);
	SetProperties(PlayerNum,InSettings->Properties);
}

/**
 * Creates an online game based upon the settings object specified.
 *
 * @param HostingPlayerNum the index of the player hosting the match
 * @param SessionName the name to associate with this setting
 * @param GameSettings the settings to use for the new game session
 *
 * @return true if successful creating the session, false otherwsie
 */
UBOOL UOnlineSubsystemLive::CreateOnlineGame(BYTE HostingPlayerNum,FName SessionName,
	UOnlineGameSettings* NewGameSettings)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	FNamedSession* Session = GetNamedSession(SessionName);
	// Don't set if we already have a session going
	if (Session == NULL)
	{
		// Add a named session and set it's game settings
		Session = AddNamedSession(SessionName,NewGameSettings);
		Session->SessionInfo = new FSecureSessionInfo();
		// Init the game settings counts so the host can use them later
		Session->GameSettings->NumOpenPrivateConnections = Session->GameSettings->NumPrivateConnections;
		Session->GameSettings->NumOpenPublicConnections = Session->GameSettings->NumPublicConnections;
		// Read the XUID of the owning player for gamertag and gamercard support
		XUserGetXUID(HostingPlayerNum,(XUID*)&Session->GameSettings->OwningPlayerId);
		ANSICHAR Buffer[32];
		// Read the name of the owning player
		XUserGetName(HostingPlayerNum,Buffer,sizeof(Buffer));
		Session->GameSettings->OwningPlayerName = Buffer;
		// Register via Live
		Return = CreateLiveGame(HostingPlayerNum,Session);
		// If we were unable to create the game, clean up
		if (Return != ERROR_IO_PENDING && Return != ERROR_SUCCESS && Session)
		{
			// Clean up the session info so we don't get into a confused state
			RemoveNamedSession(SessionName);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Cannot create session '%s': session already exists."), *SessionName.ToString());
	}
	if (Return != ERROR_IO_PENDING)
	{
		// Just trigger the delegate as having failed
		FAsyncTaskDelegateResultsNamedSession Results(SessionName,Return);
		TriggerOnlineDelegates(this,CreateOnlineGameCompleteDelegates,&Results);
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Finishes creating the online game, including creating lan beacons and/or
 * list play sessions
 *
 * @param HostingPlayerNum the player starting the session
 * @param SessionName the name of the session that is being created
 * @param CreateResult the result code from the async create operation
 * @param bIsFromInvite whether this is from an invite or not
 */
void UOnlineSubsystemLive::FinishCreateOnlineGame(DWORD HostingPlayerNum,FName SessionName,DWORD CreateResult,UBOOL bIsFromInvite)
{
	// Get the session from the name (can't be missing since this is a finish process)
	FNamedSession* Session = GetNamedSession(SessionName);
	check(Session);
#if WITH_PANORAMA
	// Whether the delegate should fire if it failed (ignored by list play as there are more steps)
	UBOOL bShouldFireOnError = Session->GameSettings == NULL;
#endif
	// If the task completed ok, then continue the create process
	if (CreateResult == ERROR_SUCCESS)
	{
		FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
		// Register the host with QoS
		RegisterQoS(Session);
		// Determine if we are registering a Live session or system link
		if (Session->GameSettings->bIsLanMatch)
		{
			// Initialize the lan game's lan beacon for queries
			CreateResult = CreateLanGame(HostingPlayerNum,Session);
		}
		// Set the game state as pending (not started)
		Session->GameSettings->GameState = OGS_Pending;
		// Register all local folks as participants/talkers
		RegisterLocalPlayers(Session,bIsFromInvite);
	}
	else
	{
		// Clean up partial create
		RemoveNamedSession(SessionName);
	}
	// As long as there isn't an async task outstanding, fire the events
	if (CreateResult != ERROR_IO_PENDING)
	{
#if WITH_PANORAMA
		if (bShouldFireOnError)
#endif
		{
			// Just trigger the delegate as having failed
			FAsyncTaskDelegateResultsNamedSession Results(SessionName,CreateResult);
			TriggerOnlineDelegates(this,CreateOnlineGameCompleteDelegates,&Results);
		}
	}
}

/**
 * Tells the QoS thread to start its listening process. Builds the packet
 * of custom data to send back to clients in the query.
 *
 * @param Session the named session information that is being registered
 *
 * @return The success/error code of the operation
 */
DWORD UOnlineSubsystemLive::RegisterQoS(FNamedSession* Session)
{
	check(Session && Session->GameSettings && Session->SessionInfo);
	DWORD Return = ERROR_SUCCESS;
	// Grab the data from the session for less typing
	FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
	UOnlineGameSettings* GameSettings = Session->GameSettings;
	// Skip if the game isn't being advertised
	if (GameSettings->bShouldAdvertise)
	{
		// Copy over the Nonce for replicating to clients
		GameSettings->ServerNonce = SessionInfo->Nonce;
		// Set the build unique id for replication to clients
		GameSettings->BuildUniqueId = GetBuildUniqueId();
		// Build our custom QoS packet
		FNboSerializeToBufferXe Packet;
		Packet << QOS_PACKET_VERSION;
		Packet << GameSettings->OwningPlayerId;
		Packet << GameSettings->ServerNonce;
		Packet << GameSettings->BuildUniqueId;
		TArray<FSettingsProperty> Props;
		// Get the properties that are to be advertised via QoS
		GameSettings->GetQoSAdvertisedProperties(Props);
		// Append any custom properties to be exposed via QoS
		INT Num = Props.Num();
		Packet << Num;
		for (INT Index = 0; Index < Props.Num(); Index++)
		{
			Packet << Props(Index);
		}
		TArray<FLocalizedStringSetting> Contexts;
		// Get the contexts that are to be advertised via QoS
		GameSettings->GetQoSAdvertisedStringSettings(Contexts);
		// Append any custom contexts to be exposed via QoS
		Num = Contexts.Num();
		Packet << Num;
		for (INT Index = 0; Index < Contexts.Num(); Index++)
		{
			Packet << Contexts(Index);
		}
		DWORD QoSFlags = XNET_QOS_LISTEN_ENABLE | XNET_QOS_LISTEN_SET_DATA;
		// Determine the size of the packet data
		DWORD QoSPacketLen = Packet.GetByteCount();
		if (QoSPacketLen > ARRAY_COUNT(QoSPacket))
		{
			QoSPacketLen = 0;
			QoSFlags = XNET_QOS_LISTEN_ENABLE;
			debugfLiveSlow(NAME_DevOnline,TEXT("QoS packet too large, discarding it"));
		}
		// Copy the data into our persistent buffer since QoS will access it async
		appMemcpy(QoSPacket,(BYTE*)Packet,QoSPacketLen);
		// Register with the QoS listener
		Return = XNetQosListen(&SessionInfo->XSessionInfo.sessionID,
			QoSPacket,
			QoSPacketLen,
			// Uses the default (16kbps)
			0,
			QoSFlags);
		debugfLiveSlow(NAME_DevOnline,
			TEXT("XNetQosListen(Key,Data,%d,0,XNET_QOS_LISTEN_ENABLE | XNET_QOS_LISTEN_SET_DATA) returned 0x%08X"),
			QoSPacketLen,Return);
	}
	return Return;
}

/**
 * Tells the QoS to respond with a "go away" packet and includes our custom
 * data. Prevents bandwidth from going to QoS probes
 *
 * @param Session the named session info for the session
 *
 * @return The success/error code of the operation
 */
DWORD UOnlineSubsystemLive::DisableQoS(FNamedSession* Session)
{
	DWORD Return = ERROR_SUCCESS;
	check(Session && Session->SessionInfo);
	// Skip if the game isn't being advertised
	if (Session->GameSettings->bShouldAdvertise)
	{
		FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
		// Stops the QoS from responding
		Return = XNetQosListen(&SessionInfo->XSessionInfo.sessionID,
			NULL,
			0,
			0,
			XNET_QOS_LISTEN_DISABLE);
		debugfLiveSlow(NAME_DevOnline,
			TEXT("XNetQosListen(Key,NULL,0,0,XNET_QOS_LISTEN_DISABLE) returned 0x%08X"),
			Return);
	}
	return Return;
}

/**
 * Tells the QoS thread to stop its listening process
 *
 * @param Session the named session info for the session
 *
 * @return The success/error code of the operation
 */
DWORD UOnlineSubsystemLive::UnregisterQoS(FNamedSession* Session)
{
	DWORD Return = ERROR_SUCCESS;
	check(Session && Session->SessionInfo);
	// Skip if the game isn't being advertised
	if (Session->GameSettings->bShouldAdvertise)
	{
		FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
		// Unregister with the QoS listener and releases any memory underneath
		Return = XNetQosListen(&SessionInfo->XSessionInfo.sessionID,
			NULL,
			0,
			0,
			XNET_QOS_LISTEN_RELEASE | XNET_QOS_LISTEN_SET_DATA);
		debugfLiveSlow(NAME_DevOnline,
			TEXT("XNetQosListen(Key,NULL,0,0,XNET_QOS_LISTEN_RELEASE | XNET_QOS_LISTEN_SET_DATA) returned 0x%08X"),
			Return);
	}
	return Return;
}

/**
 * Kicks off the list of returned servers' QoS queries
 *
 * @param AsyncData the object that holds the async QoS data
 *
 * @return TRUE if the call worked and the results should be polled for,
 *		   FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::CheckServersQoS(FLiveAsyncTaskDataSearch* AsyncData)
{
	UBOOL bOk = FALSE;
	if (GameSearch != NULL)
	{
		check(AsyncData);
		// Figure out how many servers we need to ping
		DWORD NumServers = (DWORD)GameSearch->Results.Num();
		if (NumServers > 0)
		{
			// The QoS arrays are hardcoded to 50
			check(NumServers < 50);
			// Get the pointers to the arrays used in the QoS calls
			XNADDR** ServerAddrs = AsyncData->GetXNADDRs();
			XNKID** ServerKids = AsyncData->GetXNKIDs();
			XNKEY** ServerKeys = AsyncData->GetXNKEYs();
			// Loop through the results and build the arrays needed for our queries
			for (INT Index = 0; Index < GameSearch->Results.Num(); Index++)
			{
				const FOnlineGameSearchResult& Game = GameSearch->Results(Index);
				const XSESSION_INFO* XSessInfo = (XSESSION_INFO*)Game.PlatformData;
				// Copy the addr, id, and key
				ServerAddrs[Index] = (XNADDR*)&XSessInfo->hostAddress;
				ServerKids[Index] = (XNKID*)&XSessInfo->sessionID;
				ServerKeys[Index] = (XNKEY*)&XSessInfo->keyExchangeKey;
			}
			// Kick off the QoS set of queries
			DWORD Return = XNetQosLookup(NumServers,
				(const XNADDR**)ServerAddrs,
				(const XNKID**)ServerKids,
				(const XNKEY**)ServerKeys,
				// We skip all gateway services
				0,0,0,
				// 8 probes are recommended
				8,
				64 * 1024,
				// Flags are unsupported and we'll poll
				0,NULL,
				// The out parameter that holds the data
				AsyncData->GetXNQOS());
			debugfLiveSlow(NAME_DevOnline,
				TEXT("XNetQosLookup(%d,Addrs,Kids,Keys,0,0,0,8,64K,0,NULL,Data) returned 0x%08X"),
				NumServers,
				Return);
			bOk = Return == ERROR_SUCCESS;
		}
	}
	return bOk;
}

/**
 * Parses the results from the QoS queries and places those results in the
 * corresponding search results info
 *
 * @param QosData the data to parse the results of
 */
void UOnlineSubsystemLive::ParseQoSResults(XNQOS* QosData)
{
	check(QosData);
	check(GameSearch);
	// If these don't match, we don't know who the data belongs to
	if (GameSearch->Results.Num() == QosData->cxnqos)
	{
		// Iterate through the results
		for (DWORD Index = 0; Index < QosData->cxnqos; Index++)
		{
			// Get the game settings object to add data to
			UOnlineGameSettings* ServerSettings = GameSearch->Results(Index).GameSettings;
			// Read the custom data if present
			if (QosData->axnqosinfo[Index].cbData > 0 &&
				QosData->axnqosinfo[Index].pbData != NULL)
			{
				// Create a packet reader to read the data out
				FNboSerializeFromBufferXe Packet(QosData->axnqosinfo[Index].pbData,
					QosData->axnqosinfo[Index].cbData);
				BYTE QosPacketVersion = 0;
				Packet >> QosPacketVersion;
				// Verify the packet version
				if (QosPacketVersion == QOS_PACKET_VERSION)
				{
					// Read the XUID and the server nonce
					Packet >> ServerSettings->OwningPlayerId;
					Packet >> ServerSettings->ServerNonce;
					Packet >> ServerSettings->BuildUniqueId;
					INT NumProps = 0;
					// Read how many props are in the buffer
					Packet >> NumProps;
					for (INT PropIndex = 0; PropIndex < NumProps; PropIndex++)
					{
						INT AddAt = ServerSettings->Properties.AddZeroed();
						Packet >> ServerSettings->Properties(AddAt);
					}
					INT NumContexts = 0;
					// Read how many contexts are in the buffer
					Packet >> NumContexts;
					for (INT ContextIndex = 0; ContextIndex < NumContexts; ContextIndex++)
					{
						INT AddAt = ServerSettings->LocalizedSettings.AddZeroed();
						Packet >> ServerSettings->LocalizedSettings(AddAt);
					}
					// Set the ping that the QoS estimated
					ServerSettings->PingInMs = QosData->axnqosinfo[Index].wRttMedInMsecs;
					debugfLiveSlow(NAME_DevOnline,TEXT("QoS for %s is %d"),
						*ServerSettings->OwningPlayerName,ServerSettings->PingInMs);
				}
				else
				{
					debugfLiveSlow(NAME_DevOnline,TEXT("Skipping QoS packet due to version mismatch"));
				}
			}
		}
		// Make a second pass through the search results and pull out any
		// that had partial QoS data. This can't be done during QoS parsing
		// since the indices need to match then
		for (INT Index = 0; Index < GameSearch->Results.Num(); Index++)
		{
			FOnlineGameSearchResult& SearchResult = GameSearch->Results(Index);
			// If any of the fields are missing, remove this item from the list
			if (SearchResult.GameSettings->ServerNonce == 0 ||
				SearchResult.GameSettings->OwningPlayerName.Len() == 0 ||
				(XUID&)SearchResult.GameSettings->OwningPlayerId == 0 ||
				SearchResult.GameSettings->BuildUniqueId != GetBuildUniqueId())
			{
				debugfLiveSlow(NAME_DevOnline,TEXT("Removing server with malformed QoS data at index %d"),Index);
				// Log incompatible builds if present
				if (SearchResult.GameSettings->BuildUniqueId != GetBuildUniqueId())
				{
					debugfLiveSlow(NAME_DevOnline,
						TEXT("Removed incompatible builds: GameSettings->BuildUniqueId = %d, GetBuildUniqueId() = %d"),
						SearchResult.GameSettings->BuildUniqueId,
						GetBuildUniqueId());
				}
				// Free the data
				delete (XSESSION_INFO*)SearchResult.PlatformData;
				// And then remove from the list
				GameSearch->Results.Remove(Index);
				Index--;
			}
		}
		// Allow game code to sort the servers
		GameSearch->eventSortSearchResults();
	}
	else
	{
		debugfLiveSlow(NAME_Warning,TEXT("QoS data for servers doesn't match up, skipping"));
	}
}

/**
 * Creates a new Live enabled game for the requesting player using the
 * settings specified in the game settings object
 *
 * @param HostingPlayerNum the player hosting the game
 * @param Session the named session for this live match
 *
 * @return The result from the Live APIs
 */
DWORD UOnlineSubsystemLive::CreateLiveGame(BYTE HostingPlayerNum,FNamedSession* Session)
{
	check(Session && Session->GameSettings && Session->SessionInfo);
	UOnlineGameSettings* GameSettings = Session->GameSettings;
	FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
	debugf(NAME_DevOnline,TEXT("Creating a %s match"),GameSettings->bIsLanMatch ? TEXT("System Link") : TEXT("Live"));
	// For each local player, force them to use the same props/contexts
	for (DWORD Index = 0; Index < 4; Index++)
	{
		// Ignore non-Live enabled profiles
		if (XUserGetSigninState(Index) != eXUserSigninState_NotSignedIn)
		{
			// Register all of the context/property information for the session
			SetContextsAndProperties(Index,GameSettings);
		}
	}
	// Get the flags for the session
	DWORD Flags = BuildSessionFlags(GameSettings);
	// Create a new async task for handling the creation
	FOnlineAsyncTaskLive* AsyncTask = new FLiveAsyncTaskCreateSession(Session->SessionName,HostingPlayerNum);
	// Now create the session
	DWORD Return = XSessionCreate(Flags,
		HostingPlayerNum,
		GameSettings->NumPublicConnections,
		GameSettings->NumPrivateConnections,
		&SessionInfo->Nonce,
		&SessionInfo->XSessionInfo,
		*AsyncTask,
		&SessionInfo->Handle);
	debugf(NAME_DevOnline,TEXT("XSessionCreate(%d,%d,%d,%d,Nonce,SessInfo,Data,OutHandle) returned 0x%08X"),
		Flags,(DWORD)HostingPlayerNum,GameSettings->NumPublicConnections,
		GameSettings->NumPrivateConnections,Return);
	if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
	{
		// Add the task for tracking since the call worked
		AsyncTasks.AddItem(AsyncTask);
	}
	else
	{
		// Don't leak the task in this case
		delete AsyncTask;
	}
	return Return;
}

/**
 * Creates a new system link enabled game. Registers the keys/nonce needed
 * for secure communication
 *
 * @param HostingPlayerNum the player hosting the game
 * @param Session the named session for this lan game
 *
 * @return The result code from the nonce/key APIs
 */
DWORD UOnlineSubsystemLive::CreateLanGame(BYTE HostingPlayerNum,FNamedSession* Session)
{
	check(Session && Session->GameSettings && Session->SessionInfo);
	DWORD Return = ERROR_SUCCESS;
	// Don't create a system link beacon if advertising is off
	if (Session->GameSettings->bShouldAdvertise == TRUE)
	{
		// Bind a socket for system link activity
		LanBeacon = new FLanBeacon();
		if (LanBeacon->Init(LanAnnouncePort))
		{
			// We successfully created everything so mark the socket as
			// needing polling
			LanBeaconState = LANB_Hosting;
			debugfLiveSlow(NAME_DevOnline,TEXT("Listening for beacon requestes on %d"),
				LanAnnouncePort);
		}
		else
		{
			debugfLiveSlow(NAME_Error,TEXT("Failed to init to system link beacon %d"),
				GSocketSubsystem->GetSocketError());
			Return = XNET_CONNECT_STATUS_LOST;
		}
	}
	return Return;
}

/**
 * Updates the localized settings/properties for the game in question. Updates
 * the QoS packet if needed (starting & restarting QoS).
 *
 * @param SessionName the session that is being updated
 * @param UpdatedGameSettings the settings to use for the new game session
 * @param bShouldRefreshOnlineData whether to submit the data to the backend or not
 *
 * @return true if successful creating the session, false otherwsie
 */
UBOOL UOnlineSubsystemLive::UpdateOnlineGame(FName SessionName,UOnlineGameSettings* UpdatedGameSettings,UBOOL bShouldRefreshOnlineData)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	// Find the session in question
	FNamedSession* Session = GetNamedSession(SessionName);
	if (Session && Session->GameSettings && UpdatedGameSettings)
	{
		// If they specified a different object copy over the settings to the
		// existing one
		if (UpdatedGameSettings != Session->GameSettings)
		{
			CopyGameSettings(Session->GameSettings,UpdatedGameSettings);
		}
		// Don't update QoS data when not the host
		if (IsServer())
		{
			// Determine if this is a lan match or Live to see if we need to change QoS
			if (Session->GameSettings->bIsLanMatch == FALSE)
			{
				// Unregister our QoS packet data so that it can be updated
				UnregisterQoS(Session);
				// Now reregister with the new info
				Return = RegisterQoS(Session);
			}
		}
		// Now update all of the props/contexts for all players
		for (DWORD Index = 0; Index < 4; Index++)
		{
			// Ignore non-Live enabled profiles
			if (XUserGetSigninState(Index) != eXUserSigninState_NotSignedIn)
			{
				// Register all of the context/property information for the session
				SetContextsAndProperties(Index,Session->GameSettings);
			}
		}
		// If requested, update the session information
		if (bShouldRefreshOnlineData)
		{
			Return = ModifySession(Session,&UpdateOnlineGameCompleteDelegates);
		}
		else
		{
			Return = ERROR_SUCCESS;
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Can't update (%s) game settings with a NULL object"),
			*SessionName.ToString());
	}
	if (Return != ERROR_IO_PENDING)
	{
		// Just trigger the delegate as having failed
		FAsyncTaskDelegateResultsNamedSession Results(SessionName,Return);
		TriggerOnlineDelegates(this,UpdateOnlineGameCompleteDelegates,&Results);
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Destroys the current online game
 *
 * @param SessionName the session that is being destroyed
 *
 * @return true if successful destroying the session, false otherwsie
 */
UBOOL UOnlineSubsystemLive::DestroyOnlineGame(FName SessionName)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	// Find the session in question
	FNamedSession* Session = GetNamedSession(SessionName);
	// Don't shut down if it isn't valid
	if (Session && Session->GameSettings && Session->SessionInfo)
	{
		// Shutdown the lan beacon if needed
		if (Session->GameSettings->bIsLanMatch &&
			Session->GameSettings->bShouldAdvertise)
		{
			// Tear down the system link beacon
			StopLanBeacon();
		}
		// Unregister our QoS packet data and stop handling the queries
		UnregisterQoS(Session);
		Return = DestroyLiveGame(Session);
		// The session info is no longer needed
		RemoveNamedSession(Session->SessionName);
		// Only unregister everything if we no longer have sessions
		if (Sessions.Num() == 0)
		{
			// Stop all local talkers (avoids a debug runtime warning)
			UnregisterLocalTalkers();
			// Stop all remote voice before ending the session
			RemoveAllRemoteTalkers();
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Can't destroy a null online session (%s)"),
			*SessionName.ToString());
	}
	if (Return != ERROR_IO_PENDING)
	{
		// Just trigger the delegate as having failed
		FAsyncTaskDelegateResultsNamedSession Results(SessionName,Return);
		TriggerOnlineDelegates(this,DestroyOnlineGameCompleteDelegates,&Results);
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Terminates a Live session
 *
 * @param Session the named session that is being destroyed
 *
 * @return The result from the Live APIs
 */
DWORD UOnlineSubsystemLive::DestroyLiveGame(FNamedSession* Session)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
	// Create a new async task for handling the deletion
	FOnlineAsyncTaskLive* AsyncTask = new FLiveAsyncDestroySession(Session->SessionName,
		SessionInfo->Handle,
		&DestroyOnlineGameCompleteDelegates);
	// Shutdown the session asynchronously
	Return = XSessionDelete(SessionInfo->Handle,*AsyncTask);
	debugf(NAME_DevOnline,
		TEXT("XSessionDelete() '%s' returned 0x%08X"),
		*Session->SessionName.ToString(),
		Return);
	if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
	{
		// Add the task to the list to be ticked later
		AsyncTasks.AddItem(AsyncTask);
	}
	else
	{
		// Manually clean up
		CloseHandle(SessionInfo->Handle);
		// Don't leak the task
		delete AsyncTask;
	}
	return Return;
}

/**
 * Allocates the space/structure needed for holding the search results plus
 * any resources needed for async support
 *
 * @param SearchingPlayerNum the index of the player searching for the match
 * @param QueryNum the unique id of the query to be run
 * @param MaxSearchResults the maximum number of search results we want
 * @param NumBytes the out param indicating the size that was allocated
 *
 * @return The data allocated for the search (space plus overlapped)
 */
FLiveAsyncTaskDataSearch* UOnlineSubsystemLive::AllocateSearch(BYTE SearchingPlayerNum,
	DWORD QueryNum,DWORD MaxSearchResults,DWORD& NumBytes)
{
	// Use the search code to determine the size buffer we need
    DWORD Return = XSessionSearch(QueryNum,SearchingPlayerNum,MaxSearchResults,
        0,0,NULL,NULL,&NumBytes,NULL,NULL);
	FLiveAsyncTaskDataSearch* Data = NULL;
	// Only allocate the buffer if the call worked ok
	if (Return == ERROR_INSUFFICIENT_BUFFER && NumBytes > 0)
	{
		Data = new FLiveAsyncTaskDataSearch(NumBytes);
	}
	return Data;
}

/**
 * Builds a Live/system link search query and sends it off to be processed. Uses
 * the search settings passed in to populate the query.
 *
 * @param SearchingPlayerNum the index of the player searching for the match
 * @param SearchSettings the game settings that we are interested in
 *
 * @return TRUE if the search was started successfully, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::FindOnlineGames(BYTE SearchingPlayerNum,
	UOnlineGameSearch* SearchSettings)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	// Verify that we have valid search settings
	if (SearchSettings != NULL)
	{
		// Don't start another while in progress or multiple entries for the
		// same server will show up in the server list
		if (SearchSettings->bIsSearchInProgress == FALSE)
		{
			// Free up previous results
			FreeSearchResults();
			// Check for Live or Systemlink
			if (SearchSettings->bIsLanQuery == FALSE)
			{
				// If they have manually requested a skill search and the data
				// has been read from Live, then do the search with that skill data
				// or if they have not requested a skill override (both arrays are empty)
				if (SearchSettings->ManualSkillOverride.Players.Num() == SearchSettings->ManualSkillOverride.Mus.Num())
				{
					Return = FindLiveGames(SearchingPlayerNum,SearchSettings);
				}
				else
				{
					// Perform the skill leaderboard read async
					Return = ReadSkillForSearch(SearchingPlayerNum,SearchSettings);
				}
			}
			else
			{
				// Copy the search pointer so we can keep it around
				GameSearch = SearchSettings;
				Return = FindLanGames();
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("Ignoring game search request while one is pending"));
			Return = ERROR_IO_PENDING;
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Can't search with null criteria"));
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Copies the Epic structures into the Live equivalent
 *
 * @param DestProps the destination properties
 * @param SourceProps the source properties
 */
void UOnlineSubsystemLive::CopyPropertiesForSearch(PXUSER_PROPERTY DestProps,
	const TArray<FSettingsProperty>& SourceProps)
{
	appMemzero(DestProps,sizeof(XUSER_PROPERTY) * SourceProps.Num());
	// Loop through the properties and copy them over
	for (INT Index = 0; Index < SourceProps.Num(); Index++)
	{
		const FSettingsProperty& Prop = SourceProps(Index);
		// Copy property id and type
		DestProps[Index].dwPropertyId = Prop.PropertyId;
		// Now copy the held data (no strings or blobs as they aren't supported)
		switch (Prop.Data.Type)
		{
			case SDT_Float:
			{
				Prop.Data.GetData(DestProps[Index].value.fData);
				DestProps[Index].value.type = XUSER_DATA_TYPE_FLOAT;
				break;
			}
			case SDT_Int32:
			{
				Prop.Data.GetData((INT&)DestProps[Index].value.nData);
				DestProps[Index].value.type = XUSER_DATA_TYPE_INT32;
				break;
			}
			case SDT_Int64:
			{
				Prop.Data.GetData((QWORD&)DestProps[Index].value.i64Data);
				DestProps[Index].value.type = XUSER_DATA_TYPE_INT64;
				break;
			}
			case SDT_Double:
			{
				Prop.Data.GetData(DestProps[Index].value.dblData);
				DestProps[Index].value.type = XUSER_DATA_TYPE_DOUBLE;
				break;
			}
			case SDT_Blob:
			case SDT_String:
			{
				DestProps[Index].value.type = 0;
				debugfLiveSlow(NAME_DevOnline,
					TEXT("Ignoring property (%d) for search as blobs/strings aren't supported by Live"),
					Prop.PropertyId);
				break;
			}
			case SDT_DateTime:
			{
				DestProps[Index].value.ftData.dwLowDateTime = Prop.Data.Value1;
				DestProps[Index].value.ftData.dwHighDateTime = (DWORD)Prop.Data.Value2;
				DestProps[Index].value.type = XUSER_DATA_TYPE_DATETIME;
				break;
			}
		}
	}
}

/**
 * Copies the Epic structures into the Live equivalent
 *
 * @param Search the object to use when determining
 * @param DestContexts the destination contexts
 * @param SourceContexts the source contexts
 *
 * @return the number of items copied (handles skipping for wildcards)
 */
DWORD UOnlineSubsystemLive::CopyContextsForSearch(UOnlineGameSearch* Search,
	PXUSER_CONTEXT DestContexts,
	const TArray<FLocalizedStringSetting>& SourceContexts)
{
	DWORD Count = 0;
	// Iterate through the source contexts and copy any that aren't wildcards
	for (INT Index = 0; Index < SourceContexts.Num(); Index++)
	{
		const FLocalizedStringSetting& Setting = SourceContexts(Index);
		// Don't copy if the item is meant to use wildcard matching
		if (Search->IsWildcardStringSetting(Setting.Id) == FALSE)
		{
			DestContexts[Count].dwContextId = Setting.Id;
			DestContexts[Count].dwValue = Setting.ValueIndex;
			Count++;
		}
	}
	return Count;
}

/**
 * Builds a Live game query and submits it to Live for processing
 *
 * @param SearchingPlayerNum the player searching for games
 * @param SearchSettings the settings that the player is interested in
 *
 * @return The result from the Live APIs
 */
DWORD UOnlineSubsystemLive::FindLiveGames(BYTE SearchingPlayerNum,UOnlineGameSearch* SearchSettings)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	// Get to our Live specific settings so we can check arbitration & max results
	DWORD MaxSearchResults = Clamp(SearchSettings->MaxSearchResults,0,XSESSION_SEARCH_MAX_RETURNS);
	DWORD QueryId = SearchSettings->Query.ValueIndex;
	DWORD NumResultBytes = 0;
	// Now allocate the search data bucket
	FLiveAsyncTaskDataSearch* SearchData = AllocateSearch(SearchingPlayerNum,
		QueryId,MaxSearchResults,NumResultBytes);
	if (SearchData != NULL)
	{
		// Figure out the game type we want to search for
		DWORD GameType = X_CONTEXT_GAME_TYPE_STANDARD;
		if (SearchSettings->bUsesArbitration == TRUE)
		{
			GameType = X_CONTEXT_GAME_TYPE_RANKED;
		}
		// Append the required contexts if missing
		SearchSettings->SetStringSettingValue(X_CONTEXT_GAME_TYPE,
			GameType,TRUE);
		// Append the skill properties to override the searching
		AppendSkillProperties(SearchSettings);
		// Allocate space to hold the properties array
		PXUSER_PROPERTY Properties = SearchData->AllocateProperties(SearchSettings->Properties.Num());
		// Copy property data over
		CopyPropertiesForSearch(Properties,SearchSettings->Properties);
		// Allocate space to hold the contexts array
		PXUSER_CONTEXT Contexts = SearchData->AllocateContexts(SearchSettings->LocalizedSettings.Num());
		// Copy contexts data over
		DWORD NumContexts = CopyContextsForSearch(SearchSettings,Contexts,SearchSettings->LocalizedSettings);
#ifdef _DEBUG
		// Log properties and contexts
		DumpContextsAndProperties(SearchSettings);
#endif
		DWORD NumProperties = SearchSettings->Properties.Num();
		// Create a new async task for handling the async
		FOnlineAsyncTaskLive* AsyncTask = new FLiveAsyncTaskSearch(&FindOnlineGamesCompleteDelegates,SearchData);
		// Kick off the async search
		Return = XSessionSearch(QueryId,
			SearchingPlayerNum,
			MaxSearchResults,
			NumProperties,
			NumContexts,
#if WITH_PANORAMA // G4WLive doesn't handle non-NULL values when zero elements are passed in
			NumProperties ? Properties : NULL,
			NumContexts ? Contexts : NULL,
#else
			Properties,
			Contexts,
#endif
			&NumResultBytes,
			*SearchData,
			*AsyncTask);
		debugf(NAME_DevOnline,TEXT("XSessionSearch(%d,%d,%d,%d,%d,data,data,%d,data,data) returned 0x%08X"),
			QueryId,(DWORD)SearchingPlayerNum,MaxSearchResults,NumProperties,NumContexts,NumResultBytes,Return);
		if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
		{
			// Mark the search in progress
			SearchSettings->bIsSearchInProgress = TRUE;
			// Add the task to the list to be ticked later
			AsyncTasks.AddItem(AsyncTask);
			// Copy the search pointer so we can keep it around
			GameSearch = SearchSettings;
		}
		else
		{
			// Just trigger the delegate as having failed
			FAsyncTaskDelegateResults Results(Return);
			TriggerOnlineDelegates(this,FindOnlineGamesCompleteDelegates,&Results);
			// Don't leak the task
			delete AsyncTask;
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Failed to allocate space for the online game search"));
	}
	return Return;
}

/**
 * Reads the contexts and properties from the Live search data and populates the
 * game settings object with them
 *
 * @param SearchResult the data that was returned from Live
 * @param GameSettings the game settings that we are setting the data on
 */
void UOnlineSubsystemLive::ParseContextsAndProperties(
	XSESSION_SEARCHRESULT& SearchResult,UOnlineGameSettings* GameSettings)
{
	UOnlineGameSettings* DefaultSettings = GameSettings->GetClass()->GetDefaultObject<UOnlineGameSettings>();
	check(DefaultSettings);
	// Clear any settings that were in the defualts
	GameSettings->LocalizedSettings.Empty();
	GameSettings->Properties.Empty();
	// Check the number of contexts
	if (SearchResult.cContexts > 0)
	{
		// Pre add them so memory isn't copied
		GameSettings->LocalizedSettings.AddZeroed(SearchResult.cContexts);
		// Iterate through the contexts and add them to the GameSettings
		for (INT Index = 0; Index < (INT)SearchResult.cContexts; Index++)
		{
			FLocalizedStringSetting& Context = GameSettings->LocalizedSettings(Index);
			Context.Id = SearchResult.pContexts[Index].dwContextId;
			Context.ValueIndex = SearchResult.pContexts[Index].dwValue;
			// Look at the default object so we can determine how this is to be advertised
			FLocalizedStringSetting* DefaultContextSetting = DefaultSettings->FindStringSetting(Context.Id);
			if (DefaultContextSetting != NULL)
			{
				Context.AdvertisementType = DefaultContextSetting->AdvertisementType;
			}
			else
			{
				debugfLiveSlow(NAME_DevOnline,TEXT("Added non-advertised string setting %d with value %d"),
					Context.Id,Context.ValueIndex);
			}
		}
	}
	// And now the number of properties
	if (SearchResult.cProperties > 0)
	{
		// Pre add them so memory isn't copied
		GameSettings->Properties.AddZeroed(SearchResult.cProperties);
		// Iterate through the properties and add them to the GameSettings
		for (INT Index = 0; Index < (INT)SearchResult.cProperties; Index++)
		{
			FSettingsProperty& Property = GameSettings->Properties(Index);
			Property.PropertyId = SearchResult.pProperties[Index].dwPropertyId;
			// Copy the data over (may require allocs for strings)
			CopyXDataToSettingsData(Property.Data,SearchResult.pProperties[Index].value);
			// Look at the default object so we can determine how this is to be advertised
			FSettingsProperty* DefaultPropertySetting = DefaultSettings->FindProperty(Property.PropertyId);
			if (DefaultPropertySetting != NULL)
			{
				Property.AdvertisementType = DefaultPropertySetting->AdvertisementType;
			}
			else
			{
				debugfLiveSlow(NAME_DevOnline,TEXT("Adding non-advertised property 0x%08X (%d) = %s"),
					Property.PropertyId,Property.Data.Type,*Property.Data.ToString());
			}
			// Copy the hostname into the OwningPlayerName field if this is it
			if (SearchResult.pProperties[Index].dwPropertyId == X_PROPERTY_GAMER_HOSTNAME)
			{
				GameSettings->OwningPlayerName = SearchResult.pProperties[Index].value.string.pwszData;
			}
		}
	}
}

/**
 * Parses the search results into something the game play code can handle
 *
 * @param Search the Unreal search object
 * @param SearchResults the buffer filled by Live
 */
void UOnlineSubsystemLive::ParseSearchResults(UOnlineGameSearch* Search,
	PXSESSION_SEARCHRESULT_HEADER SearchResults)
{
	if (Search != NULL)
	{
		DOUBLE Mu, Sigma, PlayerCount;
		GetLocalSkills(Mu,Sigma,PlayerCount);
		check(SearchResults != NULL);
		// Loop through the results copying the info over
		for (DWORD Index = 0; Index < SearchResults->dwSearchResults; Index++)
		{
			// Matchmaking should never return full servers, but just in case
			if (SearchResults->pResults[Index].dwOpenPrivateSlots > 0 ||
				SearchResults->pResults[Index].dwOpenPublicSlots > 0)
			{
				// Create an object that we'll copy the data to
				UOnlineGameSettings* NewServer = ConstructObject<UOnlineGameSettings>(
					Search->GameSettingsClass);
				if (NewServer != NULL)
				{
					// Add space in the search results array
					INT NewSearch = Search->Results.Add();
					FOnlineGameSearchResult& Result = Search->Results(NewSearch);
					// Whether arbitration is used or not comes from the search
					NewServer->bUsesArbitration = Search->bUsesArbitration;
					// Now copy the data
					Result.GameSettings = NewServer;
					NewServer->NumOpenPrivateConnections = SearchResults->pResults[Index].dwOpenPrivateSlots;
					NewServer->NumOpenPublicConnections = SearchResults->pResults[Index].dwOpenPublicSlots;
					// Determine the total slots for the match (used + open)
					NewServer->NumPrivateConnections = SearchResults->pResults[Index].dwOpenPrivateSlots +
						SearchResults->pResults[Index].dwFilledPrivateSlots;
					NewServer->NumPublicConnections = SearchResults->pResults[Index].dwOpenPublicSlots +
						SearchResults->pResults[Index].dwFilledPublicSlots;
					// Read the various contexts and properties from the search
					ParseContextsAndProperties(SearchResults->pResults[Index],NewServer);
					// Allocate and copy the Live specific data
					XSESSION_INFO* SessInfo = new XSESSION_INFO;
					appMemcpy(SessInfo,&SearchResults->pResults[Index].info,sizeof(XSESSION_INFO));
					// Determine the match quality for this search result
					CalculateMatchQuality(Mu,Sigma,PlayerCount,NewServer);
					// Store this in the results and mark them as needin proper clean ups
					Result.PlatformData = SessInfo;
				}
			}
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("No search object to store results on!"));
	}
}

/**
 * Cancels the current search in progress if possible for that search type
 *
 * @return true if successful searching for sessions, false otherwise
 */
UBOOL UOnlineSubsystemLive::CancelFindOnlineGames(void)
{
	DWORD Return = E_FAIL;
	if (GameSearch != NULL &&
		GameSearch->bIsSearchInProgress)
	{
		// Make sure it's the right type
		if (GameSearch->bIsLanQuery)
		{
			if (GameSearch->bIsLanQuery)
			{
				Return = ERROR_SUCCESS;
				StopLanBeacon();
				GameSearch->bIsSearchInProgress = FALSE;
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("Can't cancel a Player/Ranked search only LAN/List Play"));
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Can't cancel a search that isn't in progress"));
	}
	if (Return != ERROR_IO_PENDING)
	{
		FAsyncTaskDelegateResults Results(Return);
		TriggerOnlineDelegates(this,CancelFindOnlineGamesCompleteDelegates,&Results);
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}


/**
 * Serializes the platform specific data into the provided buffer for the specified search result
 *
 * @param DesiredGame the game to copy the platform specific data for
 * @param PlatformSpecificInfo the buffer to fill with the platform specific information
 *
 * @return true if successful serializing the data, false otherwise
 */
UBOOL UOnlineSubsystemLive::ReadPlatformSpecificSessionInfo(const FOnlineGameSearchResult& DesiredGame,BYTE* PlatformSpecificInfo)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	if (DesiredGame.GameSettings && DesiredGame.PlatformData)
	{
		FNboSerializeToBufferXe Buffer;
		XSESSION_INFO* SessionInfo = (XSESSION_INFO*)DesiredGame.PlatformData;
		// Write host info (host addr, session id, and key)
		Buffer << SessionInfo->hostAddress
			<< SessionInfo->sessionID
			<< SessionInfo->keyExchangeKey
			// Read the nonce to the game settings
			<< DesiredGame.GameSettings->ServerNonce;
		if (Buffer.GetByteCount() <= 68)
		{
			// Copy the built up data
			appMemcpy(PlatformSpecificInfo,Buffer.GetRawBuffer(0),Buffer.GetByteCount());
			Return = ERROR_SUCCESS;
		}
		else
		{
			debugf(NAME_DevOnline,
				TEXT("Platform data is larger (%d) than the supplied buffer (64)"),
				Buffer.GetByteCount());
		}
	}
	return Return == ERROR_SUCCESS;
}

/**
 * Serializes the platform specific data into the provided buffer for the specified settings object.
 * NOTE: This can only be done for a session that is bound to the online system
 *
 * @param GameSettings the game to copy the platform specific data for
 * @param PlatformSpecificInfo the buffer to fill with the platform specific information
 *
 * @return true if successful reading the data for the session, false otherwise
 */
UBOOL UOnlineSubsystemLive::ReadPlatformSpecificSessionInfoBySessionName(FName SessionName,BYTE* PlatformSpecificInfo)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	// Look up the session by name and copy the session data from it
	FNamedSession* Session = GetNamedSession(SessionName);
	if (Session &&
		Session->SessionInfo)
	{
		FNboSerializeToBufferXe Buffer;
		FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
		// Write host info (host addr, session id, and key)
		Buffer << SessionInfo->XSessionInfo.hostAddress
			<< SessionInfo->XSessionInfo.sessionID
			<< SessionInfo->XSessionInfo.keyExchangeKey
			// Read the nonce to the game settings
			<< Session->GameSettings->ServerNonce;
		if (Buffer.GetByteCount() <= 68)
		{
			// Copy the built up data
			appMemcpy(PlatformSpecificInfo,Buffer.GetRawBuffer(0),Buffer.GetByteCount());
			Return = ERROR_SUCCESS;
		}
		else
		{
			debugf(NAME_DevOnline,
				TEXT("Platform data is larger (%d) than the supplied buffer (64)"),
				Buffer.GetByteCount());
		}
	}
	return Return == ERROR_SUCCESS;
}

/**
 * Creates a search result out of the platform specific data and adds that to the specified search object
 *
 * @param SearchingPlayerNum the index of the player searching for a match
 * @param SearchSettings the desired search to bind the session to
 * @param PlatformSpecificInfo the platform specific information to convert to a server object
 *
 * @return true if successful searching for sessions, false otherwise
 */
UBOOL UOnlineSubsystemLive::BindPlatformSpecificSessionToSearch(BYTE SearchingPlayerNum,UOnlineGameSearch* SearchSettings,BYTE* PlatformSpecificInfo)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	// Verify that we have valid search settings
	if (SearchSettings != NULL)
	{
		// Don't start another while in progress or multiple entries for the
		// same server will show up in the server list
		if (GameSearch == NULL ||
			GameSearch->bIsSearchInProgress == FALSE)
		{
			// Free up previous results
			FreeSearchResults();
			// Copy the search pointer so we can keep it around
			GameSearch = SearchSettings;
			// Create a new server and assign it the session key info
			UOnlineGameSettings* NewServer = ConstructObject<UOnlineGameSettings>(
				SearchSettings->GameSettingsClass);
			if (NewServer != NULL)
			{
				// Add space in the search results array
				INT NewSearch = SearchSettings->Results.Add();
				FOnlineGameSearchResult& Result = SearchSettings->Results(NewSearch);
				// Whether arbitration is used or not comes from the search
				NewServer->bUsesArbitration = SearchSettings->bUsesArbitration;
				// Now copy the data
				Result.GameSettings = NewServer;
				// Allocate and read the Live secure key info
				XSESSION_INFO* SessInfo = new XSESSION_INFO;
				// Use our reader class to read from the buffer
				FNboSerializeFromBufferXe Buffer(PlatformSpecificInfo,68);
				// Read the connection data
				Buffer >> SessInfo->hostAddress
					>> SessInfo->sessionID
					>> SessInfo->keyExchangeKey
				// Add the nonce to the game settings
					>> NewServer->ServerNonce;
				// Store this in the results so they can join it
				Result.PlatformData = SessInfo;
				Return = ERROR_SUCCESS;
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("Ignoring bind to game search request while a search is pending"));
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Can't bind to a search that is null"));
	}
	return Return == ERROR_SUCCESS;
}

/**
 * Cleans up the Live specific session data contained in the search results
 *
 * @param Search the object to free the previous results from
 *
 * @return TRUE if it could, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::FreeSearchResults(UOnlineGameSearch* Search)
{
	UBOOL bDidFree = FALSE;
	// If they didn't pass on object in, they meant for us to use the current one
	if (Search == NULL)
	{
		Search = GameSearch;
	}
	if (Search != NULL)
	{
		if (Search->bIsSearchInProgress == FALSE)
		{
			// Loop through the results freeing the session info pointers
			for (INT Index = 0; Index < Search->Results.Num(); Index++)
			{
				FOnlineGameSearchResult& Result = Search->Results(Index);
				if (Result.PlatformData != NULL)
				{
					// Free the data and clear the leak detection flag
					delete (XSESSION_INFO*)Result.PlatformData;
				}
			}
			Search->Results.Empty();
			bDidFree = TRUE;
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("Can't free search results while the search is in progress"));
		}
	}
	return bDidFree;
}

/**
 * Joins the game specified. This creates the session, decodes the IP address,
 * then kicks off the connection process
 *
 * @param PlayerNum the index of the player searching for a match
 * @param SessionName the name of the session that is being joined
 * @param DesiredGame the desired game to join
 *
 * @return true if successful destroying the session, false otherwsie
 */
UBOOL UOnlineSubsystemLive::JoinOnlineGame(BYTE PlayerNum,FName SessionName,
	const FOnlineGameSearchResult& DesiredGame)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	FNamedSession* Session = GetNamedSession(SessionName);
	// Don't join a session if already in one or hosting one
	if (Session == NULL)
	{
		// Add a named session and set it's game settings
		Session = AddNamedSession(SessionName,DesiredGame.GameSettings);
		FSecureSessionInfo* SessionInfo = new FSecureSessionInfo();
		Session->SessionInfo = SessionInfo;
		// Copy the session info over
		appMemcpy(&SessionInfo->XSessionInfo,DesiredGame.PlatformData,
			sizeof(XSESSION_INFO));
		// The session nonce needs to come from the game settings when joining
		SessionInfo->Nonce = Session->GameSettings->ServerNonce;
		// Fill in Live specific data
		Return = JoinLiveGame(PlayerNum,Session,Session->GameSettings->bWasFromInvite);
		if (Return != ERROR_SUCCESS && Return != ERROR_IO_PENDING)
		{
			// Clean up the session info so we don't get into a confused state
			RemoveNamedSession(SessionName);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Session (%s) already exists, can't join twice"),*SessionName.ToString());
	}
	if (Return != ERROR_IO_PENDING)
	{
		// Just trigger the delegate as having failed
		FAsyncTaskDelegateResultsNamedSession Results(SessionName,Return);
		TriggerOnlineDelegates(this,JoinOnlineGameCompleteDelegates,&Results);
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Joins a Live game by creating the session without hosting it
 *
 * @param PlayerNum the player joining the game
 * @param Session the session the join is being processed on
 * @param bIsFromInvite whether this join is from invite or search
 *
 * @return The result from the Live APIs
 */
DWORD UOnlineSubsystemLive::JoinLiveGame(BYTE PlayerNum,FNamedSession* Session,
	UBOOL bIsFromInvite)
{
	FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
	debugf(NAME_DevOnline,TEXT("Joining a Live match 0x%016I64X"),(QWORD&)SessionInfo->XSessionInfo.sessionID);
	// Register all of the context/property information for the session
	SetContextsAndProperties(PlayerNum,Session->GameSettings);
	// Get the flags for the session
	DWORD Flags = BuildSessionFlags(Session->GameSettings);
	// Strip off the hosting flag if specified
	Flags &= ~XSESSION_CREATE_HOST;
	// Create a new async task for handling the creation/joining
	FOnlineAsyncTaskLive* AsyncTask = new FLiveAsyncTaskCreateSession(Session->SessionName,
		PlayerNum,
		FALSE,
		bIsFromInvite);
	// Now create the session so we can decode the IP address
	DWORD Return = XSessionCreate(Flags,
		PlayerNum,
		Session->GameSettings->NumPublicConnections,
		Session->GameSettings->NumPrivateConnections,
		&SessionInfo->Nonce,
		&SessionInfo->XSessionInfo,
		*AsyncTask,
		&SessionInfo->Handle);
	debugf(NAME_DevOnline,TEXT("XSessionCreate(%d,%d,%d,%d,0x%016I64X,SessInfo,Data,OutHandle) (join request) returned 0x%08X"),
		Flags,
		(DWORD)PlayerNum,
		Session->GameSettings->NumPublicConnections,
		Session->GameSettings->NumPrivateConnections,
		Session->GameSettings->ServerNonce,
		Return);
	if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
	{
		// Add the task for tracking since the call worked
		AsyncTasks.AddItem(AsyncTask);
	}
	else
	{
		// Don't leak the task
		delete AsyncTask;
	}
	return Return;
}

/**
 * Finishes creating the online game, including creating lan beacons and/or
 * list play sessions
 *
 * @param HostingPlayerNum the player starting the session
 * @param SessionName the name of the session that is being joined
 * @param JoinResult the result code from the async create operation
 * @param bIsFromInvite whether this is from an invite or not
 */
void UOnlineSubsystemLive::FinishJoinOnlineGame(DWORD HostingPlayerNum,FName SessionName,DWORD JoinResult,UBOOL bIsFromInvite)
{
	// If the task completed ok, then continue the create process
	if (JoinResult == ERROR_SUCCESS)
	{
		// Find the session they are referring to
		FNamedSession* Session = GetNamedSession(SessionName);
		if (Session && Session->GameSettings && Session->SessionInfo)
		{
			FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
			// Set the game state as pending (not started)
			Session->GameSettings->GameState = OGS_Pending;
			// Register all local folks as participants/talkers
			RegisterLocalPlayers(Session,bIsFromInvite);
		}
		else
		{
			debugf(NAME_DevOnline,
				TEXT("Session (%s) was missing to complete join, failing"),
				*SessionName.ToString());
			JoinResult = E_FAIL;
		}
	}
	else
	{
		// Clean up partial create/join
		RemoveNamedSession(SessionName);
	}
	// As long as there isn't an async task outstanding, fire the events
	if (JoinResult != ERROR_IO_PENDING)
	{
		// Just trigger the delegate as having failed
		FAsyncTaskDelegateResultsNamedSession Results(SessionName,JoinResult);
		TriggerOnlineDelegates(this,JoinOnlineGameCompleteDelegates,&Results);
	}
}

/**
 * Returns the platform specific connection information for joining the match.
 * Call this function from the delegate of join completion
 *
 * @param SessionName the name of the session to resolve
 * @param ConnectInfo the out var containing the platform specific connection information
 *
 * @return true if the call was successful, false otherwise
 */
UBOOL UOnlineSubsystemLive::GetResolvedConnectString(FName SessionName,FString& ConnectInfo)
{
	UBOOL bOk = FALSE;
	// Find the session they are referring to
	FNamedSession* Session = GetNamedSession(SessionName);
	if (Session != NULL)
	{
		FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
		if (SessionInfo != NULL)
		{
			FInternetIpAddr IpAddr;
			// Figure out if we need to do the secure IP handling or not
			if (GSocketSubsystem->RequiresEncryptedPackets())
			{
				in_addr Addr;
				// Try to decode the secure address so we can connect to it
				if (XNetXnAddrToInAddr(&SessionInfo->XSessionInfo.hostAddress,
					&SessionInfo->XSessionInfo.sessionID,
					&Addr) == 0)
				{
					// Always use the secure layer in final release
					IpAddr.SetIp(Addr);
					bOk = TRUE;
				}
				else
				{
					debugf(NAME_DevOnline,
						TEXT("Failed to decrypt target host IP for session (%s)"),
						*SessionName.ToString());
				}
			}
			else
			{
				bOk = TRUE;
				// Don't use the encrypted/decrypted form of the IP when it's not required
				IpAddr.SetIp(SessionInfo->XSessionInfo.hostAddress.ina);
			}
			// Copy the destination IP
			ConnectInfo = IpAddr.ToString(FALSE);
		}
		else
		{
			debugf(NAME_DevOnline,
				TEXT("Can't decrypt a NULL session's IP for session (%s)"),
				*SessionName.ToString());
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Unknown session name (%s) specified to GetResolvedConnectString()"),
			*SessionName.ToString());
	}
	return bOk;
}

/**
 * Registers a player with the online service as being part of the online game
 *
 * @param NewPlayer the player to register with the online service
 * @param bWasInvited whether to use private or public slots first
 *
 * @return true if the call succeeds, false otherwise
 */
UBOOL UOnlineSubsystemLive::RegisterPlayer(FName SessionName,FUniqueNetId UniquePlayerId,UBOOL bWasInvited)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	// Find the session they are referring to
	FNamedSession* Session = GetNamedSession(SessionName);
	// Don't try to join a non-existant game
	if (Session && Session->GameSettings && Session->SessionInfo)
	{
		INT RegistrantIndex = INDEX_NONE;
		FOnlineRegistrant Registrant(UniquePlayerId);
		// See if this is a new player or not
		if (Session->Registrants.FindItem(Registrant,RegistrantIndex) == FALSE)
		{
			// Add the player as a registrant for this session
			Session->Registrants.AddItem(Registrant);
			// Determine if this player is really remote or not
			if (IsLocalPlayer(UniquePlayerId) == FALSE)
			{
				FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
				// Create a new async task for handling the async notification
				FLiveAsyncRegisterPlayer* AsyncTask = new FLiveAsyncRegisterPlayer(SessionName,
					(XUID&)UniquePlayerId,
					// Treat invite or a private match the same
					bWasInvited || Session->GameSettings->NumPublicConnections == 0,
					&RegisterPlayerCompleteDelegates);
				// Kick off the async join request
				Return = XSessionJoinRemote(SessionInfo->Handle,
					1,
					AsyncTask->GetXuids(),
					AsyncTask->GetPrivateInvites(),
					*AsyncTask);
				debugf(NAME_DevOnline,
					TEXT("XSessionJoinRemote(0x%016I64X) for '%s' returned 0x%08X"),
					(XUID&)UniquePlayerId,
					*SessionName.ToString(),
					Return);
				// Only queue the task up if the call worked
				if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
				{
					// Register this player for voice
					RegisterRemoteTalker(UniquePlayerId);
					// Add the async task to be ticked
					AsyncTasks.AddItem(AsyncTask);
				}
				else
				{
					delete AsyncTask;
				}
			}
			else
			{
				// This is a local player. In case their PRI came last during replication, reprocess muting
				ProcessMuteChangeNotification();
				Return = ERROR_SUCCESS;
			}
		}
		else
		{
			// Determine if this player is really remote or not
			if (IsLocalPlayer(UniquePlayerId) == FALSE)
			{
				// Re-register this player for voice in case they were removed from one session,
				// but are still in another
				RegisterRemoteTalker(UniquePlayerId);
			}
			else
			{
				// This is a local player. In case their PRI came last during replication, reprocess muting
				ProcessMuteChangeNotification();
			}
			debugf(NAME_DevOnline,
				TEXT("Skipping register since player is already registered at index (%d)"),
				RegistrantIndex);
			Return = ERROR_SUCCESS;
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("No game present to join for session (%s)"),
			*SessionName.ToString());
	}
	if (Return != ERROR_IO_PENDING)
	{
		FAsyncTaskDelegateResultsNamedSession Results(SessionName,Return);
		TriggerOnlineDelegates(this,RegisterPlayerCompleteDelegates,&Results);
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Unregisters a player with the online service as being part of the online game
 *
 * @param SessionName the name of the session the player is leaving
 * @param UniquePlayerId the player to unregister with the online service
 *
 * @return true if the call succeeds, false otherwise
 */
UBOOL UOnlineSubsystemLive::UnregisterPlayer(FName SessionName,FUniqueNetId UniquePlayerId)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	// Find the session they are referring to
	FNamedSession* Session = GetNamedSession(SessionName);
	// Don't try to leave a non-existant game
	if (Session && Session->GameSettings && Session->SessionInfo)
	{
		INT RegistrantIndex = INDEX_NONE;
		FOnlineRegistrant Registrant(UniquePlayerId);
		// See if this is a new player or not
		if (Session->Registrants.FindItem(Registrant,RegistrantIndex))
		{
			// Remove the player from the list
			Session->Registrants.Remove(RegistrantIndex);
			// Now remove from Live if they are a remote player
			if (IsLocalPlayer(UniquePlayerId) == FALSE)
			{
				FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
				// Create a new async task for handling the async notification
				FLiveAsyncPlayer* AsyncTask = new FLiveAsyncPlayer(SessionName,
					(XUID&)UniquePlayerId,
					&UnregisterPlayerCompleteDelegates,
					TEXT("XSessionLeaveRemote()"));
				// Kick off the async join request
				Return = XSessionLeaveRemote(SessionInfo->Handle,
					1,
					AsyncTask->GetXuids(),
					*AsyncTask);
				debugf(NAME_DevOnline,
					TEXT("XSessionLeaveRemote(0x%016I64X) '%s' returned 0x%08X"),
					(QWORD&)UniquePlayerId,
					*SessionName.ToString(),
					Return);
				// Only queue the task up if the call worked
				if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
				{
					// Remove this player from the voice list
					UnregisterRemoteTalker(UniquePlayerId);
					// Add the async task to be ticked
					AsyncTasks.AddItem(AsyncTask);
				}
				else
				{
					delete AsyncTask;
				}
			}
			else
			{
				Return = ERROR_SUCCESS;
			}
		}
		else
		{
			debugf(NAME_DevOnline,
				TEXT("Player 0x%016I64X is not part of session (%s)"),
				(QWORD&)UniquePlayerId,
				*SessionName.ToString());
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("No game present to leave for session (%s)"),
			*SessionName.ToString());
	}
	if (Return != ERROR_IO_PENDING)
	{
		FAsyncTaskDelegateResultsNamedSession Results(SessionName,Return);
		TriggerOnlineDelegates(this,UnregisterPlayerCompleteDelegates,&Results);
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Updates the current session's skill rating using the list of players' skills
 *
 * @param SessionName the name of the session that is being updated
 * @param Players the set of players to use in the skill calculation
 *
 * @return true if the update succeeded, false otherwise
 */
UBOOL UOnlineSubsystemLive::RecalculateSkillRating(FName SessionName,const TArray<FUniqueNetId>& Players)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	if (Players.Num())
	{
		FNamedSession* Session = GetNamedSession(SessionName);
		// Skip for LAN
		if (Session &&
			Session->GameSettings &&
			Session->GameSettings->bIsLanMatch == FALSE &&
			// Don't try to modify if you aren't the server
			IsServer())
		{
			// Skip if the game isn't pending or in progress
			if (Session->GameSettings->GameState < OGS_Ending)
			{
				// Skip if an update is outstanding
				if (Session->GameSettings->bHasSkillUpdateInProgress == FALSE)
				{
					FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
					FLiveAsyncUpdateSessionSkill* AsyncTask = new FLiveAsyncUpdateSessionSkill(SessionName,Players);
					// Tell Live to update the skill for the server with the following people
					Return = XSessionModifySkill(SessionInfo->Handle,
						AsyncTask->GetCount(),
						AsyncTask->GetXuids(),
						*AsyncTask);
					debugfLiveSlow(NAME_DevOnline,TEXT("XSessionModifySkill() '%s' returned 0x%08X"),
						*SessionName.ToString(),
						Return);
					if (Return == ERROR_IO_PENDING)
					{
						// Add the async task to be ticked
						AsyncTasks.AddItem(AsyncTask);
						// Indicate the async task is running
						Session->GameSettings->bHasSkillUpdateInProgress = TRUE;
					}
					else
					{
						delete AsyncTask;
					}
				}
				else
				{
					debugf(NAME_DevOnline,
						TEXT("A skill update is already being processed for (%s), ignoring request"),
						*SessionName.ToString());
					Return = ERROR_SUCCESS;
				}
			}
			else
			{
				debugf(NAME_DevOnline,
					TEXT("Skipping a skill update for a session (%s) that is ending/ed"),
					*SessionName.ToString());
				Return = ERROR_SUCCESS;
			}
		}
		else
		{
			Return = ERROR_SUCCESS;
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Can't update skill for a session (%s) with no players"),
			*SessionName.ToString());
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Reads the online profile settings for a given user from Live using an async task.
 * First, the game settings are read. If there is data in those, then the settings
 * are pulled from there. If not, the settings come from Live, followed by the
 * class defaults.
 *
 * @param LocalUserNum the user that we are reading the data for
 * @param ProfileSettings the object to copy the results to and contains the list of items to read
 *
 * @return true if the call succeeds, false otherwise
 */
UBOOL UOnlineSubsystemLive::ReadProfileSettings(BYTE LocalUserNum,
	UOnlineProfileSettings* ProfileSettings)
{
 	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	// Only read if we don't have a profile for this player
	if (ProfileCache[LocalUserNum].Profile == NULL)
	{
		if (ProfileSettings != NULL)
		{
			ProfileCache[LocalUserNum].Profile = ProfileSettings;
			ProfileSettings->AsyncState = OPAS_Read;
			// Clear the previous set of results
			ProfileSettings->ProfileSettings.Empty();
			// Make sure the version number is requested
			ProfileSettings->AppendVersionToReadIds();
			// If they are not logged in, give them all the defaults
			if (XUserGetSigninState(LocalUserNum) != eXUserSigninState_NotSignedIn)
			{
				DWORD NumIds = ProfileSettings->ProfileSettingIds.Num();
				DWORD* ProfileIds = (DWORD*)ProfileSettings->ProfileSettingIds.GetData();
				// Create the read buffer
				FLiveAsyncTaskDataReadProfileSettings* ReadData = new FLiveAsyncTaskDataReadProfileSettings(LocalUserNum,NumIds);
				// Copy the IDs for later use when inspecting the game settings blobs
				appMemcpy(ReadData->GetIds(),ProfileIds,sizeof(DWORD) * NumIds);
				// Create a new async task for handling the async notification
				FOnlineAsyncTaskLive* AsyncTask = new FLiveAsyncTaskReadProfileSettings(
					&ProfileCache[LocalUserNum].ReadDelegates,ReadData);
				// Tell Live the size of our buffer
				DWORD SizeNeeded = ReadData->GetGameSettingsSize();
				// Start by reading the game settings fields
				Return = XUserReadProfileSettings(0,
					LocalUserNum,
					ReadData->GetGameSettingsIdsCount(),
					ReadData->GetGameSettingsIds(),
					&SizeNeeded,
					ReadData->GetGameSettingsBuffer(),
					*AsyncTask);
				if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
				{
					// Queue the async task for ticking
					AsyncTasks.AddItem(AsyncTask);
				}
				else
				{
					// Just trigger the delegate as having failed
					OnlineSubsystemLive_eventOnReadProfileSettingsComplete_Parms Results(EC_EventParm);
					Results.LocalUserNum = LocalUserNum;
					Results.bWasSuccessful = FALSE;
					TriggerOnlineDelegates(this,ProfileCache[LocalUserNum].ReadDelegates,&Results);
					delete AsyncTask;
					ProfileCache[LocalUserNum].Profile = NULL;
				}
				debugfLiveSlow(NAME_DevOnline,TEXT("XUserReadProfileSettings(0,%d,3,GameSettingsIds,%d,data,data) returned 0x%08X"),
					LocalUserNum,SizeNeeded,Return);
			}
			else
			{
				debugfLiveSlow(NAME_DevOnline,TEXT("User (%d) not logged in, using defaults"),
					(DWORD)LocalUserNum);
				// Use the defaults for this player
				ProfileSettings->eventSetToDefaults();
				ProfileSettings->AsyncState = OPAS_None;
				// Just trigger the delegate as having failed
				OnlineSubsystemLive_eventOnReadProfileSettingsComplete_Parms Results(EC_EventParm);
				Results.LocalUserNum = LocalUserNum;
				Results.bWasSuccessful = FIRST_BITFIELD;
				TriggerOnlineDelegates(this,ProfileCache[LocalUserNum].ReadDelegates,&Results);
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("Can't specify a null profile settings object"));
		}
	}
	// Make sure the profile isn't already being read, since this is going to
	// complete immediately
	else if (ProfileCache[LocalUserNum].Profile->AsyncState != OPAS_Read)
	{
		debugfLiveSlow(NAME_DevOnline,TEXT("Using cached profile data instead of reading"));
		// If the specified read isn't the same as the cached object, copy the
		// data from the cache
		if (ProfileCache[LocalUserNum].Profile != ProfileSettings)
		{
			ProfileSettings->ProfileSettings = ProfileCache[LocalUserNum].Profile->ProfileSettings;
			ProfileCache[LocalUserNum].Profile = ProfileSettings;
		}
		// Just trigger the read delegate as being done
		// Send the notification of completion
		OnlineSubsystemLive_eventOnReadProfileSettingsComplete_Parms Results(EC_EventParm);
		Results.LocalUserNum = LocalUserNum;
		Results.bWasSuccessful = FIRST_BITFIELD;
		TriggerOnlineDelegates(this,ProfileCache[LocalUserNum].ReadDelegates,&Results);
		Return = ERROR_SUCCESS;
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Profile read for player (%d) is already in progress"),LocalUserNum);
		// Just trigger the read delegate as failed
		OnlineSubsystemLive_eventOnReadProfileSettingsComplete_Parms Results(EC_EventParm);
		Results.LocalUserNum = LocalUserNum;
		Results.bWasSuccessful = FALSE;
		TriggerOnlineDelegates(this,ProfileCache[LocalUserNum].ReadDelegates,&Results);
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Parses the read profile results into something the game play code can handle
 *
 * @param PlayerNum the number of the user being processed
 * @param ReadResults the buffer filled by Live
 */
void UOnlineSubsystemLive::ParseReadProfileResults(BYTE PlayerNum,PXUSER_READ_PROFILE_SETTING_RESULT ReadResults)
{
	check(PlayerNum >=0 && PlayerNum < 4);
	UOnlineProfileSettings* ProfileRead = ProfileCache[PlayerNum].Profile;
	if (ProfileRead != NULL)
	{
		// Make sure the profile settings have a version number
		ProfileRead->SetDefaultVersionNumber();
		check(ReadResults != NULL);
		// Loop through the results copying the info over
		for (DWORD Index = 0; Index < ReadResults->dwSettingsLen; Index++)
		{
			XUSER_PROFILE_SETTING& LiveSetting = ReadResults->pSettings[Index];
			// Convert to our property id
			INT PropId = ConvertFromLiveValue(LiveSetting.dwSettingId);
			INT UpdateIndex = INDEX_NONE;
			// Search the settings for the property so we can replace if needed
			for (INT FindIndex = 0; FindIndex < ProfileRead->ProfileSettings.Num(); FindIndex++)
			{
				if (ProfileRead->ProfileSettings(FindIndex).ProfileSetting.PropertyId == PropId)
				{
					UpdateIndex = FindIndex;
					break;
				}
			}
			// Add if not already in the settings
			if (UpdateIndex == INDEX_NONE)
			{
				UpdateIndex = ProfileRead->ProfileSettings.AddZeroed();
			}
			// Now update the setting
			FOnlineProfileSetting& Setting = ProfileRead->ProfileSettings(UpdateIndex);
			// Copy the source and id is set to game since you can't write to Live ones
			Setting.Owner = OPPO_Game;
			Setting.ProfileSetting.PropertyId = PropId;
			// Don't bother copying data for no value settings
			if (LiveSetting.source != XSOURCE_NO_VALUE)
			{
				// Use the helper to copy the data over
				CopyXDataToSettingsData(Setting.ProfileSetting.Data,LiveSetting.data);
			}
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Skipping read profile results parsing due to no read object"));
	}
}

/**
 * Copies Unreal data to Live structures for the Live property writes
 *
 * @param Profile the profile object to copy the data from
 * @param LiveData the Live data structures to copy the data to
 */
void UOnlineSubsystemLive::CopyLiveProfileSettings(UOnlineProfileSettings* Profile,
	PXUSER_PROFILE_SETTING LiveData)
{
	check(Profile && LiveData);
	// Make a copy of the data for each setting
	for (INT Index = 0; Index < Profile->ProfileSettings.Num(); Index++)
	{
		FOnlineProfileSetting& Setting = Profile->ProfileSettings(Index);
		// Copy the data
		LiveData[Index].dwSettingId = ConvertToLiveValue((EProfileSettingID)Setting.ProfileSetting.PropertyId);
		LiveData[Index].source = (XUSER_PROFILE_SOURCE)Setting.Owner;
		// Shallow copy requires the data to live throughout the duration of the call
		LiveData[Index].data.type = Setting.ProfileSetting.Data.Type;
		LiveData[Index].data.binary.cbData = Setting.ProfileSetting.Data.Value1;
		LiveData[Index].data.binary.pbData = (BYTE*)Setting.ProfileSetting.Data.Value2;
	}
}

/**
 * Determines whether the specified settings should come from the game
 * default settings. If so, the defaults are copied into the players
 * profile results and removed from the settings list
 *
 * @param PlayerNum the id of the player
 * @param SettingsIds the set of ids to filter against the game defaults
 */
void UOnlineSubsystemLive::ProcessProfileDefaults(BYTE PlayerNum,TArray<DWORD>& SettingsIds)
{
	check(PlayerNum >=0 && PlayerNum < 4);
	check(ProfileCache[PlayerNum].Profile);
	// Copy the current settings so that setting the defaults doesn't clobber them
	TArray<FOnlineProfileSetting> Copy = ProfileCache[PlayerNum].Profile->ProfileSettings; 
	// Tell the profile to replace it's defaults
	ProfileCache[PlayerNum].Profile->eventSetToDefaults();
	TArray<FOnlineProfileSetting>& Settings = ProfileCache[PlayerNum].Profile->ProfileSettings;
	// Now reapply the copied settings
	for (INT Index = 0; Index < Copy.Num(); Index++)
	{
		UBOOL bFound = FALSE;
		const FOnlineProfileSetting& CopiedSetting = Copy(Index);
		// Search the profile settings and replace the setting with copied one
		for (INT Index2 = 0; Index2 < Settings.Num(); Index2++)
		{
			if (Settings(Index2).ProfileSetting.PropertyId == CopiedSetting.ProfileSetting.PropertyId)
			{
				Settings(Index2) = CopiedSetting;
				bFound = TRUE;
				break;
			}
		}
		// Add if it wasn't in the defaults
		if (bFound == FALSE)
		{
			Settings.AddItem(CopiedSetting);
		}
	}
	// Now remove the IDs that the defaults set from the missing list
	for (INT Index = 0; Index < Settings.Num(); Index++)
	{
		INT FoundIdIndex = INDEX_NONE;
		// Search and remove if found because it isn't missing then
		if (SettingsIds.FindItem(Settings(Index).ProfileSetting.PropertyId,FoundIdIndex) &&
			FoundIdIndex != INDEX_NONE &&
			Settings(Index).Owner != OPPO_OnlineService)
		{
			SettingsIds.Remove(FoundIdIndex);
		}
	}
}

/**
 * Adds one setting to the users profile results
 *
 * @param Profile the profile object to copy the data from
 * @param LiveData the Live data structures to copy the data to
 */
void UOnlineSubsystemLive::AppendProfileSetting(BYTE PlayerNum,const FOnlineProfileSetting& Setting)
{
	check(PlayerNum >=0 && PlayerNum < 4);
	check(ProfileCache[PlayerNum].Profile);
	INT AddIndex = ProfileCache[PlayerNum].Profile->ProfileSettings.AddZeroed();
	// Deep copy the data
	ProfileCache[PlayerNum].Profile->ProfileSettings(AddIndex) = Setting;
}

/**
 * Writes the online profile settings for a given user Live using an async task
 *
 * @param LocalUserNum the user that we are writing the data for
 * @param ProfileSettings the list of settings to write out
 *
 * @return true if the call succeeds, false otherwise
 */
UBOOL UOnlineSubsystemLive::WriteProfileSettings(BYTE LocalUserNum,
	UOnlineProfileSettings* ProfileSettings)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
#if WITH_PANORAMA
	// G4WLive does not allow writing of profile data with user created content present
	if (appHasAnyUserCreatedContentLoaded() == FALSE)
#endif
	{
	// Don't allow a write if there is a task already in progress
	if (ProfileCache[LocalUserNum].Profile == NULL ||
		ProfileCache[LocalUserNum].Profile->AsyncState == OPAS_None)
	{
		if (ProfileSettings != NULL)
		{
			// Mark this as a write in progress
			ProfileSettings->AsyncState = OPAS_Write;
			// Make sure the profile settings have a version number
			ProfileSettings->AppendVersionToSettings();
			// Cache to make sure GC doesn't collect this while we are waiting
			// for the Live task to complete
			ProfileCache[LocalUserNum].Profile = ProfileSettings;
			// Skip the write if the user isn't signed in
			if (XUserGetSigninState(LocalUserNum) != eXUserSigninState_NotSignedIn)
			{
				// Used to write the profile settings into a blob
				FProfileSettingsWriter Writer(FALSE);
				if (Writer.SerializeToBuffer(ProfileSettings->ProfileSettings))
				{
					// Create the write buffer
					FLiveAsyncTaskDataWriteProfileSettings* WriteData =
						new FLiveAsyncTaskDataWriteProfileSettings(LocalUserNum,Writer.GetFinalBuffer(),Writer.GetFinalBufferLength());
					// Create a new async task to hold the data during the lifetime of
					// the call. It will be freed once the call is complete.
					FLiveAsyncTaskWriteProfileSettings* AsyncTask = new FLiveAsyncTaskWriteProfileSettings(
						&ProfileCache[LocalUserNum].WriteDelegates,WriteData);
					// Call a second time to fill in the data
					Return = XUserWriteProfileSettings(LocalUserNum,
						3,
						*WriteData,
						*AsyncTask);
					debugfLiveSlow(NAME_DevOnline,TEXT("XUserWriteProfileSettings(%d,3,data,data) returned 0x%08X"),
						LocalUserNum,Return);
					if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
					{
						// Queue the async task for ticking
						AsyncTasks.AddItem(AsyncTask);
					}
					else
					{
						// Send the notification of error completion
						OnlineSubsystemLive_eventOnWriteProfileSettingsComplete_Parms Results(EC_EventParm);
						Results.LocalUserNum = LocalUserNum;
						Results.bWasSuccessful = FALSE;
						TriggerOnlineDelegates(this,ProfileCache[LocalUserNum].WriteDelegates,&Results);
						delete AsyncTask;
					}
				}
				else
				{
					debugf(NAME_DevOnline,TEXT("Failed to compress buffer for profile settings. Write aborted"));
				}
			}
			else
			{
				Return = ERROR_SUCCESS;
				// Remove the write state so that subsequent writes work
				ProfileCache[LocalUserNum].Profile->AsyncState = OPAS_None;
				// Send the notification of completion
				OnlineSubsystemLive_eventOnWriteProfileSettingsComplete_Parms Results(EC_EventParm);
				Results.LocalUserNum = LocalUserNum;
				Results.bWasSuccessful = FIRST_BITFIELD;
				TriggerOnlineDelegates(this,ProfileCache[LocalUserNum].WriteDelegates,&Results);
				debugfLiveSlow(NAME_DevOnline,TEXT("Skipping profile write for non-signed in user. Caching in profile cache"));
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("Can't write a null profile settings object"));
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Can't write profile as an async profile task is already in progress for player (%d)"),
			LocalUserNum);
	}
	}
	if (Return != ERROR_SUCCESS && Return != ERROR_IO_PENDING)
	{
		// Remove the write state so that subsequent writes work
		ProfileCache[LocalUserNum].Profile->AsyncState = OPAS_None;
		// Send the notification of error completion
		OnlineSubsystemLive_eventOnWriteProfileSettingsComplete_Parms Results(EC_EventParm);
		Results.LocalUserNum = LocalUserNum;
		Results.bWasSuccessful = FALSE;
		TriggerOnlineDelegates(this,ProfileCache[LocalUserNum].WriteDelegates,&Results);
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Sets a rich presence information to use for the specified player
 *
 * @param LocalUserNum the controller number of the associated user
 * @param PresenceMode the rich presence mode to use
 * @param Contexts the list of contexts to set
 * @param Properties the list of properties to set for the contexts
 */
void UOnlineSubsystemLive::SetOnlineStatus(BYTE LocalUserNum,INT PresenceMode,
	const TArray<FLocalizedStringSetting>& Contexts,
	const TArray<FSettingsProperty>& Properties)
{
	// Set all of the contexts/properties before setting the presence mode
	SetContexts(LocalUserNum,Contexts);
	SetProperties(LocalUserNum,Properties);
	debugf(NAME_DevOnline,TEXT("XUserSetContext(%d,X_CONTEXT_PRESENCE,%d)"),
		LocalUserNum,PresenceMode);
	// Update the presence mode
	XUserSetContext(LocalUserNum,X_CONTEXT_PRESENCE,PresenceMode);
}

/**
 * Displays the invite ui
 *
 * @param LocalUserNum the local user sending the invite
 * @param InviteText the string to prefill the UI with
 */
UBOOL UOnlineSubsystemLive::ShowInviteUI(BYTE LocalUserNum,const FString& InviteText)
{
	DWORD Result = E_FAIL;
	// Validate the user index
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Show the game invite UI for the specified controller num
		Result = XShowGameInviteUI(LocalUserNum,NULL,0,
			InviteText.Len() > 0 ? *InviteText : NULL);
		if (Result != ERROR_SUCCESS)
		{
			debugf(NAME_DevOnline,TEXT("XShowInviteUI(%d) failed with 0x%08X"),
				LocalUserNum,Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to ShowInviteUI()"),
			(DWORD)LocalUserNum);
	}
	return Result == ERROR_SUCCESS;
}

/**
 * Displays the marketplace UI for content
 *
 * @param LocalUserNum the local user viewing available content
 * @param CategoryMask the bitmask to use to filter content by type
 * @param OfferId a specific offer that you want shown
 */
UBOOL UOnlineSubsystemLive::ShowContentMarketplaceUI(BYTE LocalUserNum,INT CategoryMask,INT OfferId)
{
	DWORD Result = E_FAIL;
//@todo  joeg -- Determine if Panorama is going to support this and then re-hook up
#if CONSOLE
	// Validate the user index
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		QWORD FinalOfferId = 0;
		if (OfferId)
		{
			FinalOfferId |= ((QWORD)appGetTitleId() << 32);
			FinalOfferId |= OfferId;
		}
		// Show the marketplace for content
		Result = XShowMarketplaceUI(LocalUserNum,
			OfferId != 0 ? XSHOWMARKETPLACEUI_ENTRYPOINT_CONTENTITEM : XSHOWMARKETPLACEUI_ENTRYPOINT_CONTENTLIST,
			FinalOfferId,
			(DWORD)CategoryMask);
		if (Result != ERROR_SUCCESS)
		{
			debugf(NAME_DevOnline,TEXT("XShowMarketplaceUI(%d,%s,0x%016I64X,0x%08X) failed with 0x%08X"),
				LocalUserNum,
				OfferId != 0 ? TEXT("XSHOWMARKETPLACEUI_ENTRYPOINT_CONTENTITEM") : TEXT("XSHOWMARKETPLACEUI_ENTRYPOINT_CONTENTLIST"),
				FinalOfferId,
				CategoryMask,
				Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to ShowContentMarketplaceUI()"),
			(DWORD)LocalUserNum);
	}
#endif
	return Result == ERROR_SUCCESS;
}

/**
 * Displays the marketplace UI for memberships
 *
 * @param LocalUserNum the local user viewing available memberships
 */
UBOOL UOnlineSubsystemLive::ShowMembershipMarketplaceUI(BYTE LocalUserNum)
{
	DWORD Result = E_FAIL;
//@todo  joeg -- Determine if Panorama is going to support this and then re-hook up
#if CONSOLE
	// Validate the user index
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Show the marketplace for memberships
		Result = XShowMarketplaceUI(LocalUserNum,
			XSHOWMARKETPLACEUI_ENTRYPOINT_MEMBERSHIPLIST,0,(DWORD)-1);
		if (Result != ERROR_SUCCESS)
		{
			debugf(NAME_DevOnline,TEXT("XShowMarketplaceUI(%d,XSHOWMARKETPLACEUI_ENTRYPOINT_MEMBERSHIPLIST,0,-1) failed with 0x%08X"),
				LocalUserNum,Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to ShowMembershipMarketplaceUI()"),
			(DWORD)LocalUserNum);
	}
#endif
	return Result == ERROR_SUCCESS;
}

/**
 * Displays the UI that allows the user to choose which device to save content to
 *
 * @param LocalUserNum the controller number of the associated user
 * @param SizeNeeded the size of the data to be saved in bytes
 * @param bForceShowUI true to always show the UI, false to only show the
 *		  UI if there are multiple valid choices
 * @param bManageStorage whether to allow the user to manage their storage or not
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
UBOOL UOnlineSubsystemLive::ShowDeviceSelectionUI(BYTE LocalUserNum,INT SizeNeeded,UBOOL bForceShowUI,UBOOL bManageStorage)
{
	DWORD Return = E_FAIL;
//@todo  joeg -- Determine if Panorama is going to support this and then re-hook up
#if CONSOLE
	// Validate the user index
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		ULARGE_INTEGER BytesNeeded;
		BytesNeeded.HighPart = 0;
		BytesNeeded.LowPart = SizeNeeded;
		// Allocate an async task for deferred calling of the delegates
		FOnlineAsyncTaskLive* AsyncTask = new FOnlineAsyncTaskLive(&DeviceCache[LocalUserNum].DeviceSelectionDelegates,NULL,TEXT("XShowDeviceSelectorUI()"));
		ULARGE_INTEGER ContentSize = {0,0};
		ContentSize.QuadPart = XContentCalculateSize(BytesNeeded.QuadPart,1);
		DWORD Flags = bForceShowUI ? XCONTENTFLAG_FORCE_SHOW_UI : XCONTENTFLAG_NONE;
		Flags |= bManageStorage ? XCONTENTFLAG_MANAGESTORAGE : 0;
		// Show the live guide for selecting a device
		Return = XShowDeviceSelectorUI(LocalUserNum,
			XCONTENTTYPE_SAVEDGAME,
			Flags,
			ContentSize,
			(PXCONTENTDEVICEID)&DeviceCache[LocalUserNum].DeviceID,
			*AsyncTask);
		if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
		{
			AsyncTasks.AddItem(AsyncTask);
		}
		else
		{
			// Just trigger the delegate as having failed
			FAsyncTaskDelegateResults Results(Return);
			TriggerOnlineDelegates(this,DeviceCache[LocalUserNum].DeviceSelectionDelegates,&Results);
			// Don't leak the task/data
			delete AsyncTask;
			debugf(NAME_DevOnline,
				TEXT("XShowDeviceSelectorUI(%d,XCONTENTTYPE_SAVEDGAME,%d,%d,data,data) failed with 0x%08X"),
				LocalUserNum,
				Flags,
				Return);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified ShowDeviceSelectionUI()"),
			(DWORD)LocalUserNum);
	}
#endif
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Fetches the results of the device selection
 *
 * @param LocalUserNum the controller number of the associated user
 * @param DeviceName out param that gets a copy of the string
 *
 * @return the ID of the device that was selected
 */
INT UOnlineSubsystemLive::GetDeviceSelectionResults(BYTE LocalUserNum,FString& DeviceName)
{
	DeviceName.Empty();
	DWORD DeviceId = 0;
//@todo  joeg -- Determine if Panorama is going to support this and then re-hook up
#if CONSOLE
	// Validate the user index
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Zero means they haven't selected a device
		if (DeviceCache[LocalUserNum].DeviceID > 0)
		{
			// Live asserts if you call with a non-signed in player
			if (XUserGetSigninState(LocalUserNum) != eXUserSigninState_NotSignedIn)
			{
				XDEVICE_DATA DeviceData;
				appMemzero(&DeviceData,sizeof(XDEVICE_DATA));
				// Fetch the data, so we can get the friendly name
				DWORD Return = XContentGetDeviceData(DeviceCache[LocalUserNum].DeviceID,&DeviceData);
				if (Return == ERROR_SUCCESS)
				{
					DeviceName = DeviceData.wszFriendlyName;
					DeviceId = DeviceCache[LocalUserNum].DeviceID;
				}
			}
			else
			{
				debugf(NAME_DevOnline,TEXT("User (%d) is not signed in, returning zero as an error"),
					(DWORD)LocalUserNum);
				DeviceCache[LocalUserNum].DeviceID = 0;
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("User (%d) has not selected a device yet, returning zero as an error"),
				(DWORD)LocalUserNum);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified GetDeviceSelectionResults()"),
			(DWORD)LocalUserNum);
	}
#endif
	return DeviceId;
}

/**
 * Checks the device id to determine if it is still valid (could be removed)
 *
 * @param DeviceId the device to check
 *
 * @return TRUE if valid, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::IsDeviceValid(INT DeviceId,INT SizeNeeded)
{
//@todo  joeg -- Determine if Panorama is going to support this and then re-hook up
#if CONSOLE
	// Live asserts for a device id of zero
	if (DeviceId > 0)
	{
		DWORD Result = XContentGetDeviceState(DeviceId,NULL);
		if (Result == ERROR_SUCCESS && SizeNeeded > 0)
		{
			XDEVICE_DATA DeviceData;
			// Check the space available for this device
			Result = XContentGetDeviceData(DeviceId,&DeviceData);
			if (Result == ERROR_SUCCESS)
			{
				// Compare the size too
				return DeviceData.ulDeviceFreeBytes >= (QWORD)SizeNeeded;
			}
		}
		return Result == ERROR_SUCCESS;
	}
#endif
	return FALSE;
}

/**
 * Unlocks the specified achievement for the specified user
 *
 * @param LocalUserNum the controller number of the associated user
 * @param AchievementId the id of the achievement to unlock
 *
 * @return TRUE if the call worked, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::UnlockAchievement(BYTE LocalUserNum,INT AchievementId)
{
	DWORD Return = E_FAIL;
	// Validate the user index
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
#if WITH_PANORAMA
		// G4WLive does not allow achievements when user created content is present
		if (appHasAnyUserCreatedContentLoaded() == FALSE)
#endif
		{
			FLiveAsyncTaskDataWriteAchievement* AsyncTaskData = new FLiveAsyncTaskDataWriteAchievement(LocalUserNum,AchievementId);
			// Create a new async task to hold the data
			FOnlineAsyncTaskLive* AsyncTask = new FOnlineAsyncTaskLive(&PerUserDelegates[LocalUserNum].AchievementDelegates,
				AsyncTaskData,TEXT("XUserWriteAchievements()"));
			// Write the achievement to Live
			Return = XUserWriteAchievements(1,
				AsyncTaskData->GetAchievement(),
				*AsyncTask);
			debugfLiveSlow(NAME_DevOnline,TEXT("XUserWriteAchievements(%d,%d) returned 0x%08X"),
				(DWORD)LocalUserNum,AchievementId,Return);
			// Clean up the task if it didn't succeed
			if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
			{
				// Clear any cached achievement data for this title so the sort order is correct
				ClearCachedAchievements(LocalUserNum,0);
				// Queue the async task for ticking
				AsyncTasks.AddItem(AsyncTask);
			}
			else
			{
				// Just trigger the delegate as having failed
				FAsyncTaskDelegateResults Results(Return);
				TriggerOnlineDelegates(this,PerUserDelegates[LocalUserNum].AchievementDelegates,&Results);
				delete AsyncTask;
			}
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to UnlockAchievement()"),
			(DWORD)LocalUserNum);
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Unlocks a gamer picture for the local user
 *
 * @param LocalUserNum the user to unlock the picture for
 * @param PictureId the id of the picture to unlock
 *
 * @return TRUE if the call worked, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::UnlockGamerPicture(BYTE LocalUserNum,INT PictureId)
{
	DWORD Return = E_FAIL;
	// Validate the user index
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Create a new async task to hold the data
		FOnlineAsyncTaskLive* AsyncTask = new FOnlineAsyncTaskLive(NULL,NULL,TEXT("XUserAwardGamerPicture()"));
		// Unlock the picture with Live
		Return = XUserAwardGamerPicture(LocalUserNum,
			PictureId,
			0,
			*AsyncTask);
		debugfLiveSlow(NAME_DevOnline,TEXT("XUserAwardGamerPicture(%d,%d,0,Overlapped) returned 0x%08X"),
			(DWORD)LocalUserNum,PictureId,Return);
		// Clean up the task if it didn't succeed
		if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
		{
			// Queue the async task for ticking
			AsyncTasks.AddItem(AsyncTask);
		}
		else
		{
			delete AsyncTask;
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to UnlockGamerPicture()"),
			(DWORD)LocalUserNum);
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Tells Live that the session is in progress. Matches with join in progress
 * disabled will no longer show up in the search results.
 *
 * @return TRUE if the call succeeds, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::StartOnlineGame(FName SessionName)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	// Grab the session information by name
	FNamedSession* Session = GetNamedSession(SessionName);
	if (Session && Session->GameSettings && Session->SessionInfo)
	{
		// Lan matches don't report starting to Live
		if (Session->GameSettings->bIsLanMatch == FALSE)
		{
			// Can't start a match multiple times
			if (Session->GameSettings->GameState == OGS_Pending ||
				Session->GameSettings->GameState == OGS_Ended)
			{
				FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
#if WITH_PANORAMA
				// List play servers won't have a session handle so ignore the calls
				if (SessionInfo->Handle)
#endif
				{
					// Stop handling QoS queries if we aren't join in progress
					if (Session->GameSettings->bAllowJoinInProgress == FALSE ||
						Session->GameSettings->bUsesArbitration == TRUE)
					{
						DisableQoS(Session);
					}
					// Allocate the object that will hold the data throughout the async lifetime
					FOnlineAsyncTaskLive* AsyncTask = new FLiveAsyncTaskStartSession(SessionName,
						Session->GameSettings->bUsesArbitration,
						&StartOnlineGameCompleteDelegates,
						TEXT("XSessionStart()"));
					// Do an async start request
					Return = XSessionStart(SessionInfo->Handle,
						0,
						*AsyncTask);
					debugf(NAME_DevOnline,
						TEXT("XSessionStart() '%s' returned 0x%08X"),
						*SessionName.ToString(),
						Return);
					// Clean up the task if it didn't succeed
					if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
					{
						Session->GameSettings->GameState = OGS_Starting;
						// Queue the async task for ticking
						AsyncTasks.AddItem(AsyncTask);
					}
					else
					{
						delete AsyncTask;
					}
				}
#if WITH_PANORAMA
				else
				{
					// Make sure list play sessions work ok
					Return = S_OK;
				}
#endif
			}
			else
			{
				debugf(NAME_DevOnline,
					TEXT("Can't start an online game for session (%s) in state %s"),
					*SessionName.ToString(),
					*GetOnlineGameStateString(Session->GameSettings->GameState));
			}
		}
		else
		{
			// If this lan match has join in progress disabled, shut down the beacon
			if (Session->GameSettings->bAllowJoinInProgress == FALSE)
			{
				StopLanBeacon();
			}
			Return = ERROR_SUCCESS;
			Session->GameSettings->GameState = OGS_InProgress;
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Can't start an online game for session (%s) that hasn't been created"),
			*SessionName.ToString());
	}
	if (Return != ERROR_IO_PENDING)
	{
		// Just trigger the delegate as having failed
		FAsyncTaskDelegateResultsNamedSession Results(SessionName,Return);
		TriggerOnlineDelegates(this,StartOnlineGameCompleteDelegates,&Results);
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Tells Live that the session has ended
 *
 * @param SessionName the name of the session to end
 *
 * @return TRUE if the call succeeds, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::EndOnlineGame(FName SessionName)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	// Grab the session information by name
	FNamedSession* Session = GetNamedSession(SessionName);
	if (Session && Session->GameSettings && Session->SessionInfo)
	{
		// Lan matches don't report ending to Live
		if (Session->GameSettings->bIsLanMatch == FALSE)
		{
			// Can't end a match that isn't in progress
			if (Session->GameSettings->GameState == OGS_InProgress)
			{
				Session->GameSettings->GameState = OGS_Ending;
				FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
				// Task that will change the state
				FOnlineAsyncTaskLive* AsyncTask = new FLiveAsyncTaskSessionStateChange(SessionName,
					OGS_Ended,
					&EndOnlineGameCompleteDelegates,
					TEXT("XSessionEnd()"));
				// Do an async end request
				Return = XSessionEnd(SessionInfo->Handle,*AsyncTask);
				debugf(NAME_DevOnline,
					TEXT("XSessionEnd() '%s' returned 0x%08X"),
					*SessionName.ToString(),
					Return);
				// Clean up the task if it didn't succeed
				if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
				{
					// Queue the async task for ticking
					AsyncTasks.AddItem(AsyncTask);
				}
				else
				{
					delete AsyncTask;
				}
			}
			else
			{
				debugf(NAME_DevOnline,
					TEXT("Can't end an online game for session (%s) in state %s"),
					*SessionName.ToString(),
					*GetOnlineGameStateString(Session->GameSettings->GameState));
			}
		}
		else
		{
			// If the session should be advertised and the lan beacon was destroyed, recreate
			if (Session->GameSettings->bShouldAdvertise &&
				LanBeacon == NULL)
			{
				// Recreate the beacon
				Return = CreateLanGame(0,Session);
			}
			else
			{
				Return = ERROR_SUCCESS;
			}
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Can't end an online game for session (%s) that hasn't been created"),
			*SessionName.ToString());
	}
	if (Return != ERROR_IO_PENDING)
	{
		// Just trigger the delegate as having failed
		FAsyncTaskDelegateResultsNamedSession Results(SessionName,Return);
		TriggerOnlineDelegates(this,EndOnlineGameCompleteDelegates,&Results);
		if (Session && Session->GameSettings)
		{
			Session->GameSettings->GameState = OGS_Ended;
		}
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Writes the specified set of scores to the skill tables
 *
 * @param SessionName the name of the session the player scores are for
 * @param LeaderboardId the leaderboard to write the score information to
 * @param PlayerScores the list of scores to write out
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::WriteOnlinePlayerScores(FName SessionName,INT LeaderboardId,const TArray<FOnlinePlayerScore>& PlayerScores)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	// Grab the session information by name
	FNamedSession* Session = GetNamedSession(SessionName);
	// Can only write skill data as part of a session
	if (Session && Session->GameSettings && Session->SessionInfo)
	{
		FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
		// Perform an async write for each player involved
		for (INT Index = 0; Index < PlayerScores.Num(); Index++)
		{
			XSESSION_VIEW_PROPERTIES Views[1];
			XUSER_PROPERTY Stats[2];
			// Build the skill view
			Views[0].dwViewId = LeaderboardId;
			Views[0].dwNumProperties = 2;
			Views[0].pProperties = Stats;
			// Now build the score info
			Stats[0].dwPropertyId = X_PROPERTY_RELATIVE_SCORE;
			Stats[0].value.nData = PlayerScores(Index).Score;
			Stats[0].value.type = XUSER_DATA_TYPE_INT32;
			// And finally the team info
			Stats[1].dwPropertyId = X_PROPERTY_SESSION_TEAM;
			Stats[1].value.nData = PlayerScores(Index).TeamID;
			Stats[1].value.type = XUSER_DATA_TYPE_INT32;
			// Kick off the async write
			Return = XSessionWriteStats(SessionInfo->Handle,
				(XUID&)PlayerScores(Index).PlayerID,
				1,
				Views,
				NULL);
			debugf(NAME_DevOnline,TEXT("TrueSkill write for '%s' with ViewId (0x%08X) (Player = 0x%016I64X, Team = %d, Score = %d) returned 0x%08X"),
				*SessionName.ToString(),
				LeaderboardId,
				(QWORD&)PlayerScores(Index).PlayerID,
				PlayerScores(Index).TeamID,
				PlayerScores(Index).Score,
				Return);
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Can't write TrueSkill for session (%s) when not in a game"),
			*SessionName.ToString());
	}
	return Return == ERROR_SUCCESS;
}

/**
 * Parses the arbitration results into something the game play code can handle
 *
 * @param SessionName the session that arbitration happened for
 * @param ArbitrationResults the buffer filled by Live
 */
void UOnlineSubsystemLive::ParseArbitrationResults(FName SessionName,PXSESSION_REGISTRATION_RESULTS ArbitrationResults)
{
	// Grab the session information by name
	FNamedSession* Session = GetNamedSession(SessionName);
	if (Session)
	{
		// Dump out arbitration data if configured
		if (bShouldLogArbitrationData)
		{
			debugf(NAME_DevOnline,TEXT("Aribitration registration results %d"),
				(DWORD)ArbitrationResults->wNumRegistrants);
		}
		// Iterate through the list and set each entry
		for (DWORD Index = 0; Index < ArbitrationResults->wNumRegistrants; Index++)
		{
			const XSESSION_REGISTRANT& LiveEntry = ArbitrationResults->rgRegistrants[Index];
			// Add a new item for each player listed
			for (DWORD PlayerIndex = 0; PlayerIndex < LiveEntry.bNumUsers; PlayerIndex++)
			{
				INT AddAtIndex = Session->ArbitrationRegistrants.AddZeroed();
				FOnlineArbitrationRegistrant& Entry = Session->ArbitrationRegistrants(AddAtIndex);
				// Copy the data over
				Entry.MachineId = LiveEntry.qwMachineID;
				(XUID&)Entry.PlayerNetId = LiveEntry.rgUsers[PlayerIndex];
				Entry.Trustworthiness = LiveEntry.bTrustworthiness;
				// Dump out arbitration data if configured
				if (bShouldLogArbitrationData)
				{
					debugf(NAME_DevOnline,TEXT("MachineId = 0x%016I64X, PlayerId = 0x%016I64X, Trustworthiness = %d"),
						Entry.MachineId,(XUID&)Entry.PlayerNetId,Entry.Trustworthiness);
				}
			}
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Couldn't find session (%s) to add arbitration results to"),
			*SessionName.ToString());
	}
}

/**
 * Tells the game to register with the underlying arbitration server if available
 *
 * @param SessionName the name of the session to start arbitration for
 *
 * @return TRUE if the async task start up succeeded, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::RegisterForArbitration(FName SessionName)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	// Grab the session information by name
	FNamedSession* Session = GetNamedSession(SessionName);
	// Can't register for arbitration without the host information
	if (Session && Session->GameSettings && Session->SessionInfo)
	{
		// Lan matches don't use arbitration
		if (Session->GameSettings->bIsLanMatch == FALSE)
		{
			// Verify that the game is meant to use arbitration
			if (Session->GameSettings->bUsesArbitration == TRUE)
			{
				// Make sure the game state is pending as registering after that is silly
				if (Session->GameSettings->GameState == OGS_Pending)
				{
					FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
					DWORD BufferSize = 0;
					// Determine the amount of space needed for arbitration
					Return = XSessionArbitrationRegister(SessionInfo->Handle,
						0,
						Session->GameSettings->ServerNonce,
						&BufferSize,
						NULL,
						NULL);
					if (Return == ERROR_INSUFFICIENT_BUFFER && BufferSize > 0)
					{
						// Async task to parse the results
						FLiveAsyncTaskArbitrationRegistration* AsyncTask = new FLiveAsyncTaskArbitrationRegistration(SessionName,
							BufferSize,
							&ArbitrationRegistrationCompleteDelegates);
						// Now kick off the async task to do the registration
						Return = XSessionArbitrationRegister(SessionInfo->Handle,
							0,
							Session->GameSettings->ServerNonce,
							&BufferSize,
							AsyncTask->GetResults(),
							*AsyncTask);
						debugf(NAME_DevOnline,
							TEXT("XSessionArbitrationRegister(0x%016I64X) '%s' returned 0x%08X"),
							Session->GameSettings->ServerNonce,
							*SessionName.ToString(),
							Return);
						// Clean up the task if it didn't succeed
						if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
						{
							// Queue the async task for ticking
							AsyncTasks.AddItem(AsyncTask);
						}
						else
						{
							delete AsyncTask;
						}
					}
					else
					{
						debugf(NAME_DevOnline,
							TEXT("Failed to determine buffer size for arbitration for session (%s) with 0x%08X"),
							*SessionName.ToString(),
							Return);
					}
				}
				else
				{
					debugf(NAME_DevOnline,
						TEXT("Can't register for arbitration in session (%s) when the game is not pending"),
						*SessionName.ToString());
				}
			}
			else
			{
				debugf(NAME_DevOnline,
					TEXT("Can't register for arbitration for session (%s) on non-arbitrated games"),
					*SessionName.ToString());
			}
		}
		else
		{
			debugf(NAME_DevOnline,
				TEXT("LAN matches don't use arbitration for session (%s), ignoring call"),
				*SessionName.ToString());
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Can't register for arbitration for session (%s) with a non-existant game"),
			*SessionName.ToString());
	}
	// If there is an error, fire the delegate indicating the error
	if (Return != ERROR_SUCCESS && Return != ERROR_IO_PENDING)
	{
		FAsyncTaskDelegateResultsNamedSession Results(SessionName,Return);
		TriggerOnlineDelegates(this,ArbitrationRegistrationCompleteDelegates,&Results);
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Tells the online subsystem to accept the game invite that is currently pending
 *
 * @param LocalUserNum the local user accepting the invite
 * @param SessionName the name of the session the invite will be part of
 *
 * @return TRUE if the invite was accepted ok, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::AcceptGameInvite(BYTE LocalUserNum,FName SessionName)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	// Fail if we don't have an invite pending for this user
	if (InviteCache[LocalUserNum].InviteData != NULL)
	{
		FInviteData& Invite = InviteCache[LocalUserNum];
		// And now we can join the session
		if (JoinOnlineGame(LocalUserNum,SessionName,Invite.InviteSearch->Results(0)) == TRUE)
		{
			Return = ERROR_SUCCESS;
		}
		else
		{
			debugf(NAME_DevOnline,
				TEXT("Failed to join the invite game in session (%s), aborting"),
				*SessionName.ToString());
		}
		// Clean up the invite data
		delete (XSESSION_INFO*)Invite.InviteSearch->Results(0).PlatformData;
		Invite.InviteSearch->Results(0).PlatformData = NULL;
		delete Invite.InviteData;
		// Zero out so we know this invite has been handled
		Invite.InviteData = NULL;
		// Zero the search so it can be GCed
		Invite.InviteSearch = NULL;
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Registers all local players with the current session
 *
 * @param Session the session that they are registering in
 * @param bIsFromInvite whether this is from an invite or from searching
 */
void UOnlineSubsystemLive::RegisterLocalPlayers(FNamedSession* Session,UBOOL bIsFromInvite)
{
	check(Session && Session->SessionInfo);
	FLiveAsyncTaskDataRegisterLocalPlayers* AsyncTaskData = new FLiveAsyncTaskDataRegisterLocalPlayers();
	// Loop through the 4 available players and register them if they
	// are valid
	for (DWORD Index = 0; Index < 4; Index++)
	{
		// Ignore non-Live enabled profiles
		if (XUserGetSigninState(Index) != eXUserSigninState_NotSignedIn)
		{
			AsyncTaskData->AddPlayer(Index);
			// Register the local player as a local talker
			RegisterLocalTalker(Index);
			// Add the local player's XUID to the session
			XUID Xuid;
			XUserGetXUID(0,&Xuid);
			FOnlineRegistrant Registrant;
			Registrant.PlayerNetId.Uid = Xuid;
			Session->Registrants.AddItem(Registrant);
		}
	}
	DWORD PlayerCount = AsyncTaskData->GetCount();
	// This should never happen outside of testing, but check anyway
	if (PlayerCount > 0)
	{
		// If this match is an invite match or there were never any public slots (private only)
		// then join as private
		if (bIsFromInvite || Session->GameSettings->NumPublicConnections == 0)
		{
			// Adjust the number of private slots based upon invites and space
			DWORD NumPrivateToUse = Min<DWORD>(PlayerCount,Session->GameSettings->NumOpenPrivateConnections);
			debugfLiveSlow(NAME_DevOnline,TEXT("Using %d private slots"),NumPrivateToUse);
			AsyncTaskData->SetPrivateSlotsUsed(NumPrivateToUse);
		}
		FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
		// Create a new async task to hold the data
		FOnlineAsyncTaskLive* AsyncTask = new FOnlineAsyncTaskLive(NULL,AsyncTaskData,TEXT("XSessionJoinLocal()"));
		// Now register them as a group, asynchronously
		DWORD Return = XSessionJoinLocal(SessionInfo->Handle,
			AsyncTaskData->GetCount(),
			AsyncTaskData->GetPlayers(),
			AsyncTaskData->GetPrivateSlots(),
			*AsyncTask);
		debugf(NAME_DevOnline,
			TEXT("XSessionJoinLocal() '%s' returned 0x%08X"),
			*Session->SessionName.ToString(),
			Return);
		// Clean up the task if it didn't succeed
		if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
		{
			// Queue the async task for ticking
			AsyncTasks.AddItem(AsyncTask);
		}
		else
		{
			delete AsyncTask;
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("No locally signed in players to join game in session (%s)"),
			*Session->SessionName.ToString());
		delete AsyncTaskData;
	}
}

/**
 * Starts an async task that retrieves the list of friends for the player from the
 * online service. The list can be retrieved in whole or in part.
 *
 * @param LocalUserNum the user to read the friends list of
 * @param Count the number of friends to read or zero for all
 * @param StartingAt the index of the friends list to start at (for pulling partial lists)
 *
 * @return true if the read request was issued successfully, false otherwise
 */
UBOOL UOnlineSubsystemLive::ReadFriendsList(BYTE LocalUserNum,INT Count,INT StartingAt)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	UBOOL bFireDelegate = FALSE;
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Throw out the old friends list
		FriendsCache[LocalUserNum].Friends.Empty();
		FriendsCache[LocalUserNum].ReadState = OERS_NotStarted;
		DWORD NumPerRead = MAX_FRIENDS;
		HANDLE Handle;
		DWORD SizeNeeded;
		// Create a new enumerator for the friends list
		Return = XFriendsCreateEnumerator(LocalUserNum,
			StartingAt,
			NumPerRead,
			&SizeNeeded,
			&Handle);
		debugfLiveSlow(NAME_DevOnline,
			TEXT("XFriendsCreateEnumerator(%d,%d,%d,out,out) returned 0x%08X"),
			LocalUserNum,
			StartingAt,
			Count,
			Return);
		if (Return == ERROR_SUCCESS)
		{
			// Create the async data object that holds the buffers, etc.
			FLiveAsyncTaskDataEnumeration* AsyncTaskData = new FLiveAsyncTaskDataEnumeration(LocalUserNum,Handle,SizeNeeded,Count);
			// Create the async task object
			FLiveAsyncTaskReadFriends* AsyncTask = new FLiveAsyncTaskReadFriends(&FriendsCache[LocalUserNum].ReadFriendsDelegates,AsyncTaskData);
			// Start the async read
			Return = XEnumerate(Handle,
				AsyncTaskData->GetBuffer(),
				SizeNeeded,
				0,
				*AsyncTask);
			if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
			{
				// Mark this as being read
				FriendsCache[LocalUserNum].ReadState = OERS_InProgress;
				AsyncTasks.AddItem(AsyncTask);
			}
			else
			{
				bFireDelegate = TRUE;
				// Delete the async task
				delete AsyncTask;
			}
		}
		// Friends list might be empty
		if (Return == ERROR_NO_MORE_FILES)
		{
			bFireDelegate = TRUE;
			Return = ERROR_SUCCESS;
			// Set it to done, since there is nothing to read
			FriendsCache[LocalUserNum].ReadState = OERS_Done;
		}
		else if (Return != ERROR_SUCCESS && Return != ERROR_IO_PENDING)
		{
			bFireDelegate = TRUE;
			FriendsCache[LocalUserNum].ReadState = OERS_Failed;
		}
	}
	else
	{
		bFireDelegate = TRUE;
		debugf(NAME_DevOnline,
			TEXT("Invalid user specified in StartNetworkedVoice(%d)"),
			(DWORD)LocalUserNum);
	}
	// Fire off the delegate if needed
	if (bFireDelegate)
	{
		FAsyncTaskDelegateResults Results(Return);
		TriggerOnlineDelegates(this,FriendsCache[LocalUserNum].ReadFriendsDelegates,&Results);
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Copies the list of friends for the player previously retrieved from the online
 * service. The list can be retrieved in whole or in part.
 *
 * @param LocalUserNum the user to read the friends list of
 * @param Friends the out array that receives the copied data
 * @param Count the number of friends to read or zero for all
 * @param StartingAt the index of the friends list to start at (for pulling partial lists)
 *
 * @return OERS_Done if the read has completed, otherwise one of the other states
 */
BYTE UOnlineSubsystemLive::GetFriendsList(BYTE LocalUserNum,
	TArray<FOnlineFriend>& Friends,INT Count,INT StartingAt)
{
	BYTE Return = OERS_Failed;
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Check to see if the last read request has completed
		Return = FriendsCache[LocalUserNum].ReadState;
		if (Return == OERS_Done)
		{
			// See if they requested all of the data
			if (Count == 0)
			{
				Count = FriendsCache[LocalUserNum].Friends.Num();
			}
			// Presize the out array
			INT AmountToAdd = Min(Count,FriendsCache[LocalUserNum].Friends.Num() - StartingAt);
			Friends.Empty(AmountToAdd);
			Friends.AddZeroed(AmountToAdd);
			// Copy the data from the starting point to the number desired
			for (INT Index = 0;
				Index < Count && (Index + StartingAt) < FriendsCache[LocalUserNum].Friends.Num();
				Index++)
			{
				Friends(Index) = FriendsCache[LocalUserNum].Friends(Index + StartingAt);
			}
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Invalid user specified in GetFriendsList(%d)"),
			(DWORD)LocalUserNum);
	}
	return Return;
}

/**
 * Parses the friends results into something the game play code can handle
 *
 * @param PlayerIndex the index of the player that this read was for
 * @param LiveFriends the buffer filled by Live
 * @param NumReturned the number of friends returned by live
 */
void UOnlineSubsystemLive::ParseFriendsResults(DWORD PlayerIndex,PXONLINE_PRESENCE LiveFriends,DWORD NumReturned)
{
	check(PlayerIndex >= 0 && PlayerIndex < 4);
	DWORD TitleId = appGetTitleId();
	// Iterate through them all, adding them
	for (DWORD Count = 0; Count < NumReturned; Count++)
	{
		// Skip the update if there is no title id because that means it wasn't read
		if (LiveFriends[Count].dwTitleID != 0)
		{
			// Find the friend being updated
			for (INT FriendIndex = 0; FriendIndex < FriendsCache[PlayerIndex].Friends.Num(); FriendIndex++)
			{
				// Grab access to the friend
				FOnlineFriend& Friend = FriendsCache[PlayerIndex].Friends(FriendIndex);
				// If this is the one we need to update, then do so
				if (Friend.UniqueId.Uid == LiveFriends[Count].xuid)
				{
					// Copy the session info so we can follow them
					Friend.SessionId = (QWORD&)LiveFriends[Count].sessionID;
					// Copy the presence string
					Friend.PresenceInfo = LiveFriends[Count].wszRichPresence;
					// Set booleans based off of Live state
					Friend.bHasVoiceSupport = LiveFriends[Count].dwState & XONLINE_FRIENDSTATE_FLAG_VOICE ? TRUE : FALSE;
					Friend.bIsJoinable = (LiveFriends[Count].dwState & XONLINE_FRIENDSTATE_FLAG_JOINABLE ||
						LiveFriends[Count].dwState & XONLINE_FRIENDSTATE_FLAG_JOINABLE_FRIENDS_ONLY) ? TRUE : FALSE;
					Friend.bIsOnline = LiveFriends[Count].dwState & XONLINE_FRIENDSTATE_FLAG_ONLINE ? TRUE : FALSE;
					// Update the friend state with the online info and state flags
					if (Friend.bIsOnline)
					{
						if (XOnlineIsUserAway(LiveFriends[Count].dwState))
						{
							Friend.FriendState = OFS_Away;
						}
						else if (XOnlineIsUserBusy(LiveFriends[Count].dwState))
						{
							Friend.FriendState = OFS_Busy;
						}
						else
						{
							Friend.FriendState = OFS_Online;
						}
					}
					else
					{
						Friend.FriendState = OFS_Offline;
					}
					Friend.bIsPlaying = LiveFriends[Count].dwState & XONLINE_FRIENDSTATE_FLAG_PLAYING ? TRUE : FALSE;
					// Check that the title id is the one we expect
					Friend.bIsPlayingThisGame = LiveFriends[Count].dwTitleID == TitleId;
				}
			}
		}
	}
	/** Helper class that sorts friends by online in this game, online, and then offline */
	class FriendSorter
	{
	public:
		static inline INT Compare(const FOnlineFriend& A,const FOnlineFriend& B)
		{
			// Sort by our game first so they are at the top
			INT InGameSort = (INT)B.bIsPlayingThisGame - (INT)A.bIsPlayingThisGame;
			if (InGameSort == 0)
			{
				// Now sort by whether they are online
				INT OnlineSort = (INT)B.bIsOnline - (INT)A.bIsOnline;
				if (OnlineSort == 0)
				{
					// Finally sort by name
					INT NameSort = appStrcmp(*A.NickName,*B.NickName);
					return NameSort;
				}
				return OnlineSort;
			}
			return InGameSort;
		}
	};
	// Now sort the friends
	Sort<FOnlineFriend,FriendSorter>(FriendsCache[PlayerIndex].Friends.GetTypedData(),FriendsCache[PlayerIndex].Friends.Num());
}

/**
 * Parses the friends results into something the game play code can handle
 *
 * @param PlayerIndex the index of the player that this read was for
 * @param LiveFriends the buffer filled by Live
 * @param NumReturned the number of friends returned by live
 */
void UOnlineSubsystemLive::ParseFriendsResults(DWORD PlayerIndex,PXONLINE_FRIEND LiveFriends,DWORD NumReturned)
{
	check(PlayerIndex >= 0 && PlayerIndex < 4);
	DWORD TitleId = appGetTitleId();
	// Iterate through them all, adding them
	for (DWORD Count = 0; Count < NumReturned; Count++)
	{
		// Figure out where we are appending our friends to
		INT Offset = FriendsCache[PlayerIndex].Friends.AddZeroed(1);
		// Get the friend we just added
		FOnlineFriend& Friend = FriendsCache[PlayerIndex].Friends(Offset);
		// Copy the session info so we can follow them
		Friend.SessionId = (QWORD&)LiveFriends[Count].sessionID;
		// Copy the name
		Friend.NickName = LiveFriends[Count].szGamertag;
		// Copy the presence string
		Friend.PresenceInfo = LiveFriends[Count].wszRichPresence;
		// Copy the XUID
		Friend.UniqueId.Uid = LiveFriends[Count].xuid;
		// Set booleans based off of Live state
		Friend.bHasVoiceSupport = LiveFriends[Count].dwFriendState & XONLINE_FRIENDSTATE_FLAG_VOICE ? TRUE : FALSE;
		Friend.bIsJoinable = (LiveFriends[Count].dwFriendState & XONLINE_FRIENDSTATE_FLAG_JOINABLE ||
			LiveFriends[Count].dwFriendState & XONLINE_FRIENDSTATE_FLAG_JOINABLE_FRIENDS_ONLY) ? TRUE : FALSE;
		Friend.bIsOnline = LiveFriends[Count].dwFriendState & XONLINE_FRIENDSTATE_FLAG_ONLINE ? TRUE : FALSE;
		// Update the friend state with the online info and state flags
		if (Friend.bIsOnline)
		{
			if (XOnlineIsUserAway(LiveFriends[Count].dwFriendState))
			{
				Friend.FriendState = OFS_Away;
			}
			else if (XOnlineIsUserBusy(LiveFriends[Count].dwFriendState))
			{
				Friend.FriendState = OFS_Busy;
			}
			else
			{
				Friend.FriendState = OFS_Online;
			}
		}
		else
		{
			Friend.FriendState = OFS_Offline;
		}
		Friend.bIsPlaying = LiveFriends[Count].dwFriendState & XONLINE_FRIENDSTATE_FLAG_PLAYING ? TRUE : FALSE;
		// Check that the title id is the one we expect
		Friend.bIsPlayingThisGame = LiveFriends[Count].dwTitleID == TitleId;
		// Set invite status
		Friend.bHaveInvited = LiveFriends[Count].dwFriendState & XONLINE_FRIENDSTATE_FLAG_SENTINVITE ? TRUE : FALSE;
		Friend.bHasInvitedYou = LiveFriends[Count].dwFriendState & XONLINE_FRIENDSTATE_FLAG_RECEIVEDINVITE ? TRUE : FALSE;
	}
}

/**
 * Starts an async task that retrieves the list of downloaded content for the player.
 *
 * @param LocalUserNum The user to read the content list of
 *
 * @return true if the read request was issued successfully, false otherwise
 */
UBOOL UOnlineSubsystemLive::ReadContentList(BYTE LocalUserNum)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	UBOOL bFireDelegate = FALSE;
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
//@todo  joeg -- Determine if Panorama is going to support this and then re-hook up
#if CONSOLE
		// Throw out the old friends list
		ContentCache[LocalUserNum].Content.Empty();
		ContentCache[LocalUserNum].ReadState = OERS_NotStarted;
		// if the user is logging in, search for any DLC
		DWORD SizeNeeded;
		HANDLE Handle;
		// return 1 at a time per XEnumerate call
		DWORD NumToRead = 1;
		// start looking for this user's downloadable content
		Return = XContentCreateEnumerator(LocalUserNum, XCONTENTDEVICE_ANY, XCONTENTTYPE_MARKETPLACE, 
			0, NumToRead, &SizeNeeded, &Handle);
		// make sure we succeeded
		if (Return == ERROR_SUCCESS)
		{
			// Create the async data object that holds the buffers, etc (using 0 for number to retrieve for all)
			FLiveAsyncTaskContent* AsyncTaskData = new FLiveAsyncTaskContent(LocalUserNum, Handle, SizeNeeded, 0);
			// Create the async task object
			FLiveAsyncTaskReadContent* AsyncTask = new FLiveAsyncTaskReadContent(&ContentCache[LocalUserNum].ReadCompleteDelegates, AsyncTaskData);
			// Start the async read
			Return = XEnumerate(Handle, AsyncTaskData->GetBuffer(), SizeNeeded, 0, *AsyncTask);
			if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
			{
				// Mark this as being read
				ContentCache[LocalUserNum].ReadState = OERS_InProgress;
				AsyncTasks.AddItem(AsyncTask);
			}
			else
			{
				bFireDelegate = TRUE;
				// Delete the async task
				delete AsyncTask;
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("XContentCreateEnumerator(%d, XCONTENTDEVICE_ANY, XCONTENTTYPE_MARKETPLACE, 0, 1, &BufferSize, &EnumerateHandle) failed with 0x%08X"),
				LocalUserNum, Return);
		}
		// Friends list might be empty
		if (Return == ERROR_NO_MORE_FILES)
		{
			bFireDelegate = TRUE;
			Return = ERROR_SUCCESS;
			// Set it to done, since there is nothing to read
			ContentCache[LocalUserNum].ReadState = OERS_Done;
		}
		else if (Return != ERROR_SUCCESS && Return != ERROR_IO_PENDING)
		{
			bFireDelegate = TRUE;
			ContentCache[LocalUserNum].ReadState = OERS_Failed;
		}
	}
	else
	{
		bFireDelegate = TRUE;
		debugf(NAME_DevOnline,
			TEXT("Invalid user specified in ReadContentList(%d)"),
			(DWORD)LocalUserNum);
	}
	// Fire off the delegate if needed
	if (bFireDelegate)
	{
		FAsyncTaskDelegateResults Results(Return);
		TriggerOnlineDelegates(this,ContentCache[LocalUserNum].ReadCompleteDelegates,&Results);
	}
#endif
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Retrieve the list of content the given user has downloaded or otherwise retrieved
 * to the local console.
 *
 * @param LocalUserNum The user to read the content list of
 * @param ContentList The out array that receives the list of all content
 */
BYTE UOnlineSubsystemLive::GetContentList(BYTE LocalUserNum, TArray<FOnlineContent>& ContentList)
{
	BYTE Return = OERS_Failed;
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Check to see if the last read request has completed
		Return = ContentCache[LocalUserNum].ReadState;
		if (Return == OERS_Done)
		{
			ContentList = ContentCache[LocalUserNum].Content;
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Invalid user specified in GetContentList(%d)"),
			(DWORD)LocalUserNum);
	}
	return Return;
}

/**
 * Asks the online system for the number of new and total content downloads
 *
 * @param LocalUserNum the user to check the content download availability for
 * @param CategoryMask the bitmask to use to filter content by type
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::QueryAvailableDownloads(BYTE LocalUserNum,INT CategoryMask)
{
//@todo  joeg -- Determine if Panorama is going to support this and then re-hook up
#if CONSOLE
	DWORD Return = E_INVALIDARG;
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Create the async task and data objects
		FLiveAsyncTaskDataQueryDownloads* AsyncData = new FLiveAsyncTaskDataQueryDownloads(LocalUserNum);
		FLiveAsyncTaskQueryDownloads* AsyncTask = new FLiveAsyncTaskQueryDownloads(&ContentCache[LocalUserNum].QueryDownloadsDelegates,AsyncData);
		// Do an async query for the content counts
		Return = XContentGetMarketplaceCounts(LocalUserNum,
			(DWORD)CategoryMask,
			sizeof(XOFFERING_CONTENTAVAILABLE_RESULT),
			AsyncData->GetQuery(),
			*AsyncTask);
		debugfLiveSlow(NAME_DevOnline,TEXT("XContentGetMarketplaceCounts(%d,0x%08X) returned 0x%08X"),
			(DWORD)LocalUserNum,
			CategoryMask,
			Return);
		// Clean up the task if it didn't succeed
		if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
		{
			// Queue the async task for ticking
			AsyncTasks.AddItem(AsyncTask);
		}
		else
		{
			// Just trigger the delegate as having failed
			FAsyncTaskDelegateResults Results(Return);
			TriggerOnlineDelegates(this,ContentCache[LocalUserNum].QueryDownloadsDelegates,&Results);
			delete AsyncTask;
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Invalid user specified in QueryAvailableDownloads(%d)"),
			(DWORD)LocalUserNum);
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
#else
	return FALSE;
#endif
}

/**
 * Registers the user as a talker
 *
 * @param LocalUserNum the local player index that is a talker
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::RegisterLocalTalker(BYTE LocalUserNum)
{
	DWORD Return = E_FAIL;
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Get at the local talker's cached data
		FLocalTalker& Talker = LocalTalkers[LocalUserNum];
		// Make local user capable of sending voice data
		StartNetworkedVoice(LocalUserNum);
		// Don't register non-Live enabled accounts
		if (XUserGetSigninState(LocalUserNum) != eXUserSigninState_NotSignedIn &&
			// Or register talkers when voice is disabled
			VoiceEngine != NULL)
		{
			if (Talker.bIsRegistered == FALSE)
			{
				// Register the talker locally
				Return = VoiceEngine->RegisterLocalTalker(LocalUserNum);
				debugfLiveSlow(NAME_DevOnline,TEXT("RegisterLocalTalker(%d) returned 0x%08X"),
					LocalUserNum,Return);
				if (Return == S_OK)
				{
					Talker.bHasVoice = TRUE;
					Talker.bIsRegistered = TRUE;
					// Kick off the processing mode
					Return = VoiceEngine->StartLocalVoiceProcessing(LocalUserNum);
					debugfLiveSlow(NAME_DevOnline,TEXT("StartLocalProcessing(%d) returned 0x%08X"),
						(DWORD)LocalUserNum,Return);
				}
			}
			else
			{
				// Just say yes, we registered fine
				Return = S_OK;
			}
			APlayerController* PlayerController = GetPlayerControllerFromUserIndex(LocalUserNum);
			if (PlayerController != NULL)
			{
				// Update the muting information for this local talker
				UpdateMuteListForLocalTalker(LocalUserNum,PlayerController);
			}
			else
			{
				debugf(NAME_DevOnline,TEXT("Can't update mute list for %d due to:"),LocalUserNum);
				debugf(NAME_DevOnline,TEXT("Failed to find player controller for %d"),LocalUserNum);
			}
		}
		else
		{
			// Not properly logged in, so skip voice for them
			Talker.bHasVoice = FALSE;
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Invalid user specified in RegisterLocalTalker(%d)"),
			(DWORD)LocalUserNum);
	}
	return Return == S_OK;
}

/**
 * Unregisters the user as a talker
 *
 * @param LocalUserNum the local player index to be removed
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::UnregisterLocalTalker(BYTE LocalUserNum)
{
	DWORD Return = S_OK;
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Get at the local talker's cached data
		FLocalTalker& Talker = LocalTalkers[LocalUserNum];
		// Skip the unregistration if not registered
		if (Talker.bHasVoice == TRUE &&
			// Or when voice is disabled
			VoiceEngine != NULL)
		{
			// Remove them from XHV too
			Return = VoiceEngine->UnregisterLocalTalker(LocalUserNum);
			debugfLiveSlow(NAME_DevOnline,TEXT("UnregisterLocalTalker(%d) returned 0x%08X"),
				LocalUserNum,Return);
			Talker.bHasVoice = FALSE;
			Talker.bIsRegistered = FALSE;
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Invalid user specified in UnregisterLocalTalker(%d)"),
			(DWORD)LocalUserNum);
	}
	return Return == S_OK;
}

/**
 * Registers a remote player as a talker
 *
 * @param UniqueId the unique id of the remote player that is a talker
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::RegisterRemoteTalker(FUniqueNetId UniqueId)
{
	DWORD Return = E_FAIL;
	// Skip this if the session isn't active
	if (Sessions.Num() &&
		// Or when voice is disabled
		VoiceEngine != NULL)
	{
		// See if this talker has already been registered or not
		FLiveRemoteTalker* Talker = FindRemoteTalker(UniqueId);
		if (Talker == NULL)
		{
			// Add a new talker to our list
			INT AddIndex = RemoteTalkers.AddZeroed();
			Talker = &RemoteTalkers(AddIndex);
			// Copy the XUID
			(XUID&)Talker->TalkerId = (XUID&)UniqueId;
			// Register the remote talker locally
			Return = VoiceEngine->RegisterRemoteTalker(UniqueId);
			debugfLiveSlow(NAME_DevOnline,TEXT("RegisterRemoteTalker(0x%016I64X) returned 0x%08X"),
				(XUID&)UniqueId,Return);
		}
		else
		{
			debugfLiveSlow(NAME_DevOnline,TEXT("Remote talker 0x%016I64X is being re-registered"),(XUID&)UniqueId);
			Return = S_OK;
		}
		// Update muting all of the local talkers with this remote talker
		ProcessMuteChangeNotification();
		// Now start processing the remote voices
		Return = VoiceEngine->StartRemoteVoiceProcessing(UniqueId);
		debugfLiveSlow(NAME_DevOnline,TEXT("StartRemoteVoiceProcessing(0x%016I64X) returned 0x%08X"),
			(XUID&)UniqueId,Return);
	}
	return Return == S_OK;
}

/**
 * Unregisters a remote player as a talker
 *
 * @param UniqueId the unique id of the remote player to be removed
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::UnregisterRemoteTalker(FUniqueNetId UniqueId)
{
	DWORD Return = E_FAIL;
	// Skip this if the session isn't active
	if (Sessions.Num() &&
		// Or when voice is disabled
		VoiceEngine != NULL)
	{
		// Make sure the talker is valid
		if (FindRemoteTalker(UniqueId) != NULL)
		{
			// Find them in the talkers array and remove them
			for (INT Index = 0; Index < RemoteTalkers.Num(); Index++)
			{
				const FLiveRemoteTalker& Talker = RemoteTalkers(Index);
				// Is this the remote talker?
				if ((XUID&)Talker.TalkerId == (XUID&)UniqueId)
				{
					RemoteTalkers.Remove(Index);
					break;
				}
			}
			// Remove them from XHV too
			Return = VoiceEngine->UnregisterRemoteTalker(UniqueId);
			debugfLiveSlow(NAME_DevOnline,TEXT("UnregisterRemoteTalker(0x%016I64X) returned 0x%08X"),
				(XUID&)UniqueId,Return);
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("Unknown remote talker specified to UnregisterRemoteTalker()"));
		}
	}
	return Return == S_OK;
}

/**
 * Determines if the specified player is actively talking into the mic
 *
 * @param LocalUserNum the local player index being queried
 *
 * @return TRUE if the player is talking, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::IsLocalPlayerTalking(BYTE LocalUserNum)
{
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		return VoiceEngine != NULL && VoiceEngine->IsLocalPlayerTalking(LocalUserNum);
	}
	return FALSE;
}

/**
 * Determines if the specified remote player is actively talking into the mic
 * NOTE: Network latencies will make this not 100% accurate
 *
 * @param UniqueId the unique id of the remote player being queried
 *
 * @return TRUE if the player is talking, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::IsRemotePlayerTalking(FUniqueNetId UniqueId)
{
	return Sessions.Num() && VoiceEngine != NULL && VoiceEngine->IsRemotePlayerTalking(UniqueId);
}

/**
 * Determines if the specified player has a headset connected
 *
 * @param LocalUserNum the local player index being queried
 *
 * @return TRUE if the player has a headset plugged in, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::IsHeadsetPresent(BYTE LocalUserNum)
{
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		return VoiceEngine != NULL && VoiceEngine->IsHeadsetPresent(LocalUserNum);
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Invalid user specified in IsHeadsetPresent(%d)"),
			(DWORD)LocalUserNum);
	}
	return FALSE;
}

/**
 * Sets the relative priority for a remote talker. 0 is highest
 *
 * @param LocalUserNum the user that controls the relative priority
 * @param UniqueId the remote talker that is having their priority changed for
 * @param Priority the relative priority to use (0 highest, < 0 is muted)
 *
 * @return TRUE if the function succeeds, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::SetRemoteTalkerPriority(BYTE LocalUserNum,FUniqueNetId UniqueId,INT Priority)
{
	DWORD Return = E_INVALIDARG;
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Skip this if the session isn't active
		if (Sessions.Num() &&
			// Or if voice is disabled
			VoiceEngine != NULL)
		{
			// Find the remote talker to modify
			FLiveRemoteTalker* Talker = FindRemoteTalker(UniqueId);
			if (Talker != NULL)
			{
				// Cache the old and set the new current priority
				Talker->LocalPriorities[LocalUserNum].LastPriority = Talker->LocalPriorities[LocalUserNum].CurrentPriority;
				Talker->LocalPriorities[LocalUserNum].CurrentPriority = Priority;
				Return = VoiceEngine->SetPlaybackPriority(LocalUserNum,UniqueId,
					(XHV_PLAYBACK_PRIORITY)Priority);
				debugfLiveSlow(NAME_DevOnline,TEXT("SetPlaybackPriority(%d,0x%016I64X,%d) return 0x%08X"),
					LocalUserNum,(XUID&)UniqueId,Priority,Return);
			}
			else
			{
				debugf(NAME_DevOnline,
					TEXT("Unknown remote talker (0x%016I64X) specified to SetRemoteTalkerPriority()"),
					(QWORD&)UniqueId);
			}
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Invalid user specified in SetRemoteTalkerPriority(%d)"),
			(DWORD)LocalUserNum);
	}
	return Return == S_OK;
}

/**
 * Mutes a remote talker for the specified local player. NOTE: This is separate
 * from the user's permanent online mute list
 *
 * @param LocalUserNum the user that is muting the remote talker
 * @param UniqueId the remote talker that is being muted
 *
 * @return TRUE if the function succeeds, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::MuteRemoteTalker(BYTE LocalUserNum,FUniqueNetId UniqueId)
{
	DWORD Return = E_INVALIDARG;
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Skip this if the session isn't active
		if (Sessions.Num() &&
			// Or if voice is disabled
			VoiceEngine != NULL)
		{
			// Find the specified talker
			FLiveRemoteTalker* Talker = FindRemoteTalker(UniqueId);
			if (Talker != NULL)
			{
				// This is the talker in question, so cache the last priority
				// and change the current to muted
				Talker->LocalPriorities[LocalUserNum].LastPriority = Talker->LocalPriorities[LocalUserNum].CurrentPriority;
				Talker->LocalPriorities[LocalUserNum].CurrentPriority = XHV_PLAYBACK_PRIORITY_NEVER;
				// Set their priority to never
				Return = VoiceEngine->SetPlaybackPriority(LocalUserNum,UniqueId,XHV_PLAYBACK_PRIORITY_NEVER);
				debugfLiveSlow(NAME_DevOnline,TEXT("SetPlaybackPriority(%d,0x%016I64X,-1) return 0x%08X"),
					LocalUserNum,(XUID&)UniqueId,Return);
			}
			else
			{
				debugf(NAME_DevOnline,TEXT("Unknown remote talker specified to MuteRemoteTalker()"));
			}
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Invalid user specified in MuteRemoteTalker(%d)"),
			(DWORD)LocalUserNum);
	}
	return Return == S_OK;
}

/**
 * Allows a remote talker to talk to the specified local player. NOTE: This call
 * will fail for remote talkers on the user's permanent online mute list
 *
 * @param LocalUserNum the user that is allowing the remote talker to talk
 * @param UniqueId the remote talker that is being restored to talking
 *
 * @return TRUE if the function succeeds, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::UnmuteRemoteTalker(BYTE LocalUserNum,FUniqueNetId UniqueId)
{
	DWORD Return = E_INVALIDARG;
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Skip this if the session isn't active
		if (Sessions.Num() &&
			// Or if voice is disabled
			VoiceEngine != NULL)
		{
			// Find the specified talker
			FLiveRemoteTalker* Talker = FindRemoteTalker(UniqueId);
			if (Talker != NULL)
			{
				INT bIsMuted = FALSE;
				// Verify that this talker isn't on the mute list
				XUserMuteListQuery(LocalUserNum,(XUID&)UniqueId,&bIsMuted);
				// Only restore their priority if they aren't muted
				if (bIsMuted == FALSE)
				{
					Talker->LocalPriorities[LocalUserNum].LastPriority = Talker->LocalPriorities[LocalUserNum].CurrentPriority;
					Talker->LocalPriorities[LocalUserNum].CurrentPriority = XHV_PLAYBACK_PRIORITY_MAX;
					// Don't unmute if any player on this console has them muted
					if (Talker->IsLocallyMuted() == FALSE)
					{
						// Set their priority to unmuted
						Return = VoiceEngine->SetPlaybackPriority(LocalUserNum,UniqueId,
							Talker->LocalPriorities[LocalUserNum].CurrentPriority);
						debugfLiveSlow(NAME_DevOnline,TEXT("SetPlaybackPriority(%d,0x%016I64X,%d) returned 0x%08X"),
							LocalUserNum,(XUID&)UniqueId,
							Talker->LocalPriorities[LocalUserNum].CurrentPriority,Return);
					}
				}
			}
			else
			{
				debugf(NAME_DevOnline,TEXT("Unknown remote talker specified to UnmuteRemoteTalker()"));
			}
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Invalid user specified in UnmuteRemoteTalker(%d)"),
			(DWORD)LocalUserNum);
	}
	return Return == S_OK;
}

/**
 * Tells the voice layer that networked processing of the voice data is allowed
 * for the specified player. This allows for push-to-talk style voice communication
 *
 * @param LocalUserNum the local user to allow network transimission for
 */
void UOnlineSubsystemLive::StartNetworkedVoice(BYTE LocalUserNum)
{
	// Validate the range of the entry
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		LocalTalkers[LocalUserNum].bHasNetworkedVoice = TRUE;
		debugfLiveSlow(NAME_DevOnline,TEXT("Starting networked voice for %d"),LocalUserNum);
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid user specified in StartNetworkedVoice(%d)"),
			(DWORD)LocalUserNum);
	}
}

/**
 * Tells the voice layer to stop processing networked voice support for the
 * specified player. This allows for push-to-talk style voice communication
 *
 * @param LocalUserNum the local user to disallow network transimission for
 */
void UOnlineSubsystemLive::StopNetworkedVoice(BYTE LocalUserNum)
{
	// Validate the range of the entry
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		LocalTalkers[LocalUserNum].bHasNetworkedVoice = FALSE;
		debugfLiveSlow(NAME_DevOnline,TEXT("Stopping networked voice for %d"),LocalUserNum);
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid user specified in StopNetworkedVoice(%d)"),
			(DWORD)LocalUserNum);
	}
}

/**
 * Tells the voice system to start tracking voice data for speech recognition
 *
 * @param LocalUserNum the local user to recognize voice data for
 *
 * @return true upon success, false otherwise
 */
UBOOL UOnlineSubsystemLive::StartSpeechRecognition(BYTE LocalUserNum)
{
	HRESULT Return = E_FAIL;
	if (bIsUsingSpeechRecognition && VoiceEngine != NULL)
	{
		Return = VoiceEngine->StartSpeechRecognition(LocalUserNum);
		debugfLiveSlow(NAME_DevOnline,TEXT("StartSpeechRecognition(%d) returned 0x%08X"),
			LocalUserNum,Return);
	}
	return SUCCEEDED(Return);
}

/**
 * Tells the voice system to stop tracking voice data for speech recognition
 *
 * @param LocalUserNum the local user to recognize voice data for
 *
 * @return true upon success, false otherwise
 */
UBOOL UOnlineSubsystemLive::StopSpeechRecognition(BYTE LocalUserNum)
{
	HRESULT Return = E_FAIL;
	if (bIsUsingSpeechRecognition && VoiceEngine != NULL)
	{
		Return = VoiceEngine->StopSpeechRecognition(LocalUserNum);
		debugfLiveSlow(NAME_DevOnline,TEXT("StopSpeechRecognition(%d) returned 0x%08X"),
			LocalUserNum,Return);
	}
	return SUCCEEDED(Return);
}

/**
 * Gets the results of the voice recognition
 *
 * @param LocalUserNum the local user to read the results of
 * @param Words the set of words that were recognized by the voice analyzer
 *
 * @return true upon success, false otherwise
 */
UBOOL UOnlineSubsystemLive::GetRecognitionResults(BYTE LocalUserNum,TArray<FSpeechRecognizedWord>& Words)
{
	HRESULT Return = E_FAIL;
	if (bIsUsingSpeechRecognition && VoiceEngine != NULL)
	{
		Return = VoiceEngine->GetRecognitionResults(LocalUserNum,Words);
		debugfLiveSlow(NAME_DevOnline,TEXT("GetRecognitionResults(%d,Array) returned 0x%08X"),
			LocalUserNum,Return);
	}
	return SUCCEEDED(Return);
}

/**
 * Changes the vocabulary id that is currently being used
 *
 * @param LocalUserNum the local user that is making the change
 * @param VocabularyId the new id to use
 *
 * @return true if successful, false otherwise
 */
UBOOL UOnlineSubsystemLive::SelectVocabulary(BYTE LocalUserNum,INT VocabularyId)
{
	HRESULT Return = E_FAIL;
	if (bIsUsingSpeechRecognition && VoiceEngine != NULL)
	{
		Return = VoiceEngine->SelectVocabulary(LocalUserNum,VocabularyId);
		debugfLiveSlow(NAME_DevOnline,TEXT("SelectVocabulary(%d,%d) returned 0x%08X"),
			LocalUserNum,VocabularyId,Return);
	}
	return SUCCEEDED(Return);
}

/**
 * Changes the object that is in use to the one specified
 *
 * @param LocalUserNum the local user that is making the change
 * @param SpeechRecogObj the new object use
 *
 * @param true if successful, false otherwise
 */
UBOOL UOnlineSubsystemLive::SetSpeechRecognitionObject(BYTE LocalUserNum,USpeechRecognition* SpeechRecogObj)
{
	HRESULT Return = E_FAIL;
	if (bIsUsingSpeechRecognition && VoiceEngine != NULL)
	{
		Return = VoiceEngine->SetRecognitionObject(LocalUserNum,SpeechRecogObj);
		debugfLiveSlow(NAME_DevOnline,TEXT("SetRecognitionObject(%d,%s) returned 0x%08X"),
			LocalUserNum,SpeechRecogObj ? *SpeechRecogObj->GetName() : TEXT("NULL"),Return);
	}
	return SUCCEEDED(Return);
}

/**
 * Re-evaluates the muting list for all local talkers
 */
void UOnlineSubsystemLive::ProcessMuteChangeNotification(void)
{
	// Nothing to update if there isn't an active session
	if (Sessions.Num() && VoiceEngine != NULL)
	{
		// For each local user with voice
		for (INT Index = 0; Index < 4; Index++)
		{
			APlayerController* PlayerController = GetPlayerControllerFromUserIndex(Index);
			// If there is a player controller, we can mute/unmute people
			if (LocalTalkers[Index].bHasVoice && PlayerController != NULL)
			{
				// Use the common method of checking muting
				UpdateMuteListForLocalTalker(Index,PlayerController);
			}
		}
	}
}

/**
 * Figures out which remote talkers need to be muted for a given local talker
 *
 * @param TalkerIndex the talker that needs the mute list checked for
 * @param PlayerController the player controller associated with this talker
 */
void UOnlineSubsystemLive::UpdateMuteListForLocalTalker(INT TalkerIndex,
	APlayerController* PlayerController)
{
	// For each registered remote talker
	for (INT RemoteIndex = 0; RemoteIndex < RemoteTalkers.Num(); RemoteIndex++)
	{
		FLiveRemoteTalker& Talker = RemoteTalkers(RemoteIndex);
		INT bIsMuted = FALSE;
		// Is the remote talker on this local player's mute list?
		XUserMuteListQuery(TalkerIndex,(XUID&)Talker.TalkerId,&bIsMuted);
		// Figure out which priority to use now
		if (bIsMuted == FALSE)
		{
			// If they were previously muted, set them to zero priority
			if (Talker.LocalPriorities[TalkerIndex].CurrentPriority == XHV_PLAYBACK_PRIORITY_NEVER)
			{
				Talker.LocalPriorities[TalkerIndex].LastPriority = Talker.LocalPriorities[TalkerIndex].CurrentPriority;
				Talker.LocalPriorities[TalkerIndex].CurrentPriority = XHV_PLAYBACK_PRIORITY_MAX;
				// Unmute on the server
				PlayerController->eventServerUnmutePlayer(Talker.TalkerId);
			}
			else
			{
				// Use their current priority without changes
			}
		}
		else
		{
			// Mute this remote talker
			Talker.LocalPriorities[TalkerIndex].LastPriority = Talker.LocalPriorities[TalkerIndex].CurrentPriority;
			Talker.LocalPriorities[TalkerIndex].CurrentPriority = XHV_PLAYBACK_PRIORITY_NEVER;
			// Mute on the server
			PlayerController->eventServerMutePlayer(Talker.TalkerId);
		}
		// The ServerUn/MutePlayer() functions will perform the muting based
		// upon gameplay settings and other player's mute list
	}
}

/**
 * Registers/unregisters local talkers based upon login changes
 */
void UOnlineSubsystemLive::UpdateVoiceFromLoginChange(void)
{
	// Nothing to update if there isn't an active session
	if (Sessions.Num())
	{
		// Check each user index for a sign in change
		for (INT Index = 0; Index < 4; Index++)
		{
			XUSER_SIGNIN_STATE SignInState = XUserGetSigninState(Index);
			// Was this player registered last time around but is no longer signed in
			if (LocalTalkers[Index].bHasVoice == TRUE &&
				SignInState == eXUserSigninState_NotSignedIn)
			{
				UnregisterLocalTalker(Index);
			}
			// Was this player not registered, but now is logged in
			else if (LocalTalkers[Index].bHasVoice == FALSE &&
				SignInState != eXUserSigninState_NotSignedIn)
			{
				RegisterLocalTalker(Index);
			}
			else
			{
				// Logged in and registered, so do nothing
			}
		}
	}
}

/**
 * Iterates the current remote talker list unregistering them with XHV
 * and our internal state
 */
void UOnlineSubsystemLive::RemoveAllRemoteTalkers(void)
{
	debugfLiveSlow(NAME_DevOnline,TEXT("Removing all remote talkers"));
	if (VoiceEngine != NULL)
	{
		// Work backwards through array removing the talkers
		for (INT Index = RemoteTalkers.Num() - 1; Index >= 0; Index--)
		{
			const FLiveRemoteTalker& Talker = RemoteTalkers(Index);
			// Remove them from XHV
			DWORD Return = VoiceEngine->UnregisterRemoteTalker(Talker.TalkerId);
			debugfLiveSlow(NAME_DevOnline,TEXT("UnregisterRemoteTalker(0x%016I64X) returned 0x%08X"),
				(XUID&)Talker.TalkerId,Return);
		}
	}
	// Empty the array now that they are all unregistered
	RemoteTalkers.Empty();
}

/**
 * Registers all signed in local talkers
 */
void UOnlineSubsystemLive::RegisterLocalTalkers(void)
{
	debugfLiveSlow(NAME_DevOnline,TEXT("Registering all local talkers"));
	// Loop through the 4 available players and register them
	for (DWORD Index = 0; Index < 4; Index++)
	{
		// Register the local player as a local talker
		RegisterLocalTalker(Index);
	}
}

/**
 * Unregisters all signed in local talkers
 */
void UOnlineSubsystemLive::UnregisterLocalTalkers(void)
{
	debugfLiveSlow(NAME_DevOnline,TEXT("Unregistering all local talkers"));
	// Loop through the 4 available players and unregister them
	for (DWORD Index = 0; Index < 4; Index++)
	{
		// Unregister the local player as a local talker
		UnregisterLocalTalker(Index);
	}
}

/**
 * Parses the read results and copies them to the stats read object
 *
 * @param ReadResults the data to add to the stats object
 */
void UOnlineSubsystemLive::ParseStatsReadResults(XUSER_STATS_READ_RESULTS* ReadResults)
{
	check(CurrentStatsRead && ReadResults);
	// Copy over the view's info
	CurrentStatsRead->TotalRowsInView = ReadResults->pViews->dwTotalViewRows;
	CurrentStatsRead->ViewId = ReadResults->pViews->dwViewId;
	// Now copy each row that was returned
	for (DWORD RowIndex = 0; RowIndex < ReadResults->pViews->dwNumRows; RowIndex++)
	{
		INT NewIndex = CurrentStatsRead->Rows.AddZeroed();
		FOnlineStatsRow& Row = CurrentStatsRead->Rows(NewIndex);
		const XUSER_STATS_ROW& XRow = ReadResults->pViews->pRows[RowIndex];
		// Copy the row data over
		Row.NickName = XRow.szGamertag;
		(XUID&)Row.PlayerID = XRow.xuid;
		// See if they are ranked on the leaderboard or not
		if (XRow.dwRank > 0)
		{
			Row.Rank.SetData((INT)XRow.dwRank);
		}
		else
		{
			Row.Rank.SetData(TEXT("--"));
		}
		// Now allocate our columns
		Row.Columns.Empty(XRow.dwNumColumns);
		Row.Columns.AddZeroed(XRow.dwNumColumns);
		// And copy the columns
		for (DWORD ColIndex = 0; ColIndex < XRow.dwNumColumns; ColIndex++)
		{
			FOnlineStatsColumn& Col = Row.Columns(ColIndex);
			const XUSER_STATS_COLUMN& XCol = XRow.pColumns[ColIndex];
			// Copy the column id and the data object
			Col.ColumnNo = XCol.wColumnId;
			CopyXDataToSettingsData(Col.StatValue,XCol.Value);
			// Handle Live sending "empty" values when they should be zeroed
			if (Col.StatValue.Type == SDT_Empty)
			{
				Col.StatValue.SetData(TEXT("--"));
			}
		}
	}
}

/**
 * Builds the data that we want to read into the Live specific format. Live
 * uses WORDs which script doesn't support, so we can't map directly to it
 *
 * @param DestSpecs the destination stat specs to fill in
 * @param ViewId the view id that is to be used
 * @param Columns the columns that are being requested
 */
void UOnlineSubsystemLive::BuildStatsSpecs(XUSER_STATS_SPEC* DestSpecs,INT ViewId,
	const TArrayNoInit<INT>& Columns)
{
	debugfLiveSlow(NAME_DevOnline,TEXT("ViewId = %d, NumCols = %d"),ViewId,Columns.Num());
	// Copy the view data over
	DestSpecs->dwViewId = ViewId;
	DestSpecs->dwNumColumnIds = Columns.Num();
	// Iterate through the columns and copy those over
	// NOTE: These are different types so we can't just memcpy
	for (INT Index = 0; Index < Columns.Num(); Index++)
	{
		DestSpecs->rgwColumnIds[Index] = (WORD)Columns(Index);
		if (bShouldLogStatsData)
		{
			debugfLiveSlow(NAME_DevOnline,TEXT("rgwColumnIds[%d] = %d"),Index,Columns(Index));
		}
	}
}

/**
 * Reads a set of stats for the specified list of players
 *
 * @param Players the array of unique ids to read stats for
 * @param StatsRead holds the definitions of the tables to read the data from and
 *		  results are copied into the specified object
 *
 * @return TRUE if the call is successful, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::ReadOnlineStats(const TArray<FUniqueNetId>& Players,
	UOnlineStatsRead* StatsRead)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	if (CurrentStatsRead == NULL)
	{
		if (StatsRead != NULL)
		{
			CurrentStatsRead = StatsRead;
			// Clear previous results
			CurrentStatsRead->Rows.Empty();
			// Validate that players were specified
			if (Players.Num() > 0)
			{
				// Create an async task to read the stats and kick that off
				FLiveAsyncTaskReadStats* AsyncTask = new FLiveAsyncTaskReadStats(StatsRead->TitleId,
					Players,
					&ReadOnlineStatsCompleteDelegates);
				// Get the read specs so they can be populated
				XUSER_STATS_SPEC* Specs = AsyncTask->GetSpecs();
				// Fill in the Live data
				BuildStatsSpecs(Specs,StatsRead->ViewId,StatsRead->ColumnIds);
				// Copy the player info
				XUID* XPlayers = AsyncTask->GetPlayers();
				DWORD NumPlayers = AsyncTask->GetPlayerCount();
				DWORD BufferSize = 0;
				// First time through figure out how much memory to allocate for search results
				Return = XUserReadStats(StatsRead->TitleId,
					NumPlayers,
					XPlayers,
					1,
					Specs,
					&BufferSize,
					NULL,
					NULL);
				if (Return == ERROR_INSUFFICIENT_BUFFER && BufferSize > 0)
				{
					// Allocate the results buffer
					AsyncTask->AllocateResults(BufferSize);
					// Now kick off the async read
					Return = XUserReadStats(StatsRead->TitleId,
						NumPlayers,
						XPlayers,
						1,
						Specs,
						&BufferSize,
						AsyncTask->GetReadResults(),
						*AsyncTask);
					debugf(NAME_DevOnline,
						TEXT("XUserReadStats(0,%d,Players,1,Specs,%d,Buffer,Overlapped) returned 0x%08X"),
						NumPlayers,BufferSize,Return);
					if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
					{
						AsyncTasks.AddItem(AsyncTask);
					}
					else
					{
						// Don't leak the task/data
						delete AsyncTask;
					}
				}
				else
				{
					// Don't leak the task/data
					delete AsyncTask;
					debugf(NAME_DevOnline,
						TEXT("Failed to determine buffer size needed for stats read 0x%08X"),
						Return);
				}
			}
			else
			{
				debugf(NAME_DevOnline,TEXT("Can't read stats for zero players"));
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("Can't perform a stats read with a null object"));
		}
		// Fire the delegate upon error
		if (Return != ERROR_SUCCESS && Return != ERROR_IO_PENDING)
		{
			CurrentStatsRead = NULL;
			// Just trigger the delegate as having failed
			FAsyncTaskDelegateResults Results(Return);
			TriggerOnlineDelegates(this,ReadOnlineStatsCompleteDelegates,&Results);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Can't perform a stats read while one is in progress"));
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Reads a player's stats and all of that player's friends stats for the
 * specified set of stat views. This allows you to easily compare a player's
 * stats to their friends.
 *
 * @param LocalUserNum the local player having their stats and friend's stats read for
 * @param StatsRead holds the definitions of the tables to read the data from and
 *		  results are copied into the specified object
 *
 * @return TRUE if the call is successful, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::ReadOnlineStatsForFriends(BYTE LocalUserNum,
	UOnlineStatsRead* StatsRead)
{
	if (CurrentStatsRead == NULL)
	{
		if (StatsRead != NULL)
		{
			// Validate that it won't be outside our friends cache
			if (LocalUserNum >= 0 && LocalUserNum <= 4)
			{
				const FFriendsListCache& Cache = FriendsCache[LocalUserNum];
				TArray<FUniqueNetId> Players;
				// Allocate space for all of the friends plus the player
				Players.AddZeroed(Cache.Friends.Num() + 1);
				// Copy the player into the first
				XUserGetXUID(LocalUserNum,(XUID*)&Players(0));
				// Iterate through the friends list and add them to the list
				for (INT Index = 0; Index < Cache.Friends.Num(); Index++)
				{
					Players(Index + 1) = Cache.Friends(Index).UniqueId;
				}
				// Now use the common method to read the stats
				return ReadOnlineStats(Players,StatsRead);
			}
			else
			{
				debugf(NAME_DevOnline,TEXT("Invalid player index specified %d"),
					(DWORD)LocalUserNum);
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("Can't perform a stats read with a null object"));
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Can't perform a stats read while one is in progress"));
	}
	return FALSE;
}

/**
 * Reads stats by ranking. This grabs the rows starting at StartIndex through
 * NumToRead and places them in the StatsRead object.
 *
 * @param StatsRead holds the definitions of the tables to read the data from and
 *		  results are copied into the specified object
 * @param StartIndex the starting rank to begin reads at (1 for top)
 * @param NumToRead the number of rows to read (clamped at 100 underneath)
 *
 * @return TRUE if the call is successful, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::ReadOnlineStatsByRank(UOnlineStatsRead* StatsRead,
	INT StartIndex,INT NumToRead)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	if (CurrentStatsRead == NULL)
	{
		if (StatsRead != NULL)
		{
			CurrentStatsRead = StatsRead;
			// Clear previous results
			CurrentStatsRead->Rows.Empty();
			FLiveAsyncTaskReadStatsByRank* AsyncTask = new FLiveAsyncTaskReadStatsByRank(&ReadOnlineStatsCompleteDelegates);
			DWORD BufferSize = 0;
			HANDLE hEnumerate = NULL;
			// Get the read specs so they can be populated
			XUSER_STATS_SPEC* Specs = AsyncTask->GetSpecs();
			// Fill in the Live data
			BuildStatsSpecs(Specs,StatsRead->ViewId,StatsRead->ColumnIds);
			// Figure out how much space is needed
			Return = XUserCreateStatsEnumeratorByRank(StatsRead->TitleId,
				StartIndex,
				NumToRead,
				1,
				Specs,
				&BufferSize,
				&hEnumerate);
			debugf(NAME_DevOnline,
				TEXT("XUserCreateStatsEnumeratorByRank(0,%d,%d,1,Specs,OutSize,OutHandle) returned 0x%08X"),
				StartIndex,NumToRead,Return);
			if (Return == ERROR_SUCCESS)
			{
				AsyncTask->Init(hEnumerate,BufferSize);
				// Start the async enumeration
				Return = XEnumerate(hEnumerate,
					AsyncTask->GetReadResults(),
					BufferSize,
					NULL,
					*AsyncTask);
				debugf(NAME_DevOnline,
					TEXT("XEnumerate(hEnumerate,Data,%d,Data,Overlapped) returned 0x%08X"),
					BufferSize,Return);
				if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
				{
					AsyncTasks.AddItem(AsyncTask);
				}
				else
				{
					// Don't leak the task/data
					delete AsyncTask;
				}
			}
			else
			{
				delete AsyncTask;
				debugf(NAME_DevOnline,
					TEXT("Failed to determine buffer size needed for stats enumeration 0x%08X"),
					Return);
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("Can't perform a stats read with a null object"));
		}
		// Fire the delegate upon error
		if (Return != ERROR_SUCCESS && Return != ERROR_IO_PENDING)
		{
			CurrentStatsRead = NULL;
			// Just trigger the delegate as having failed
			FAsyncTaskDelegateResults Results(Return);
			TriggerOnlineDelegates(this,ReadOnlineStatsCompleteDelegates,&Results);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Can't perform a stats read while one is in progress"));
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Reads stats by ranking centered around a player. This grabs a set of rows
 * above and below the player's current rank
 *
 * @param LocalUserNum the local player having their stats being centered upon
 * @param StatsRead holds the definitions of the tables to read the data from and
 *		  results are copied into the specified object
 * @param NumRows the number of rows to read above and below the player's rank
 *
 * @return TRUE if the call is successful, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::ReadOnlineStatsByRankAroundPlayer(BYTE LocalUserNum,
	UOnlineStatsRead* StatsRead,INT NumRows)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	if (CurrentStatsRead == NULL)
	{
		if (StatsRead != NULL)
		{
			CurrentStatsRead = StatsRead;
			// Validate the index
			if (LocalUserNum >= 0 && LocalUserNum <= 4)
			{
				XUID Player;
				// Get the XUID of the player in question
				XUserGetXUID(LocalUserNum,&Player);
				FLiveAsyncTaskReadStatsByRank* AsyncTask = new FLiveAsyncTaskReadStatsByRank(&ReadOnlineStatsCompleteDelegates);
				DWORD BufferSize = 0;
				HANDLE hEnumerate = NULL;
				// Get the read specs so they can be populated
				XUSER_STATS_SPEC* Specs = AsyncTask->GetSpecs();
				// Fill in the Live data
				BuildStatsSpecs(Specs,StatsRead->ViewId,StatsRead->ColumnIds);
				// Figure out how much space is needed
				Return = XUserCreateStatsEnumeratorByXuid(StatsRead->TitleId,
					Player,
					NumRows,
					1,
					Specs,
					&BufferSize,
					&hEnumerate);
				debugf(NAME_DevOnline,
					TEXT("XUserCreateStatsEnumeratorByXuid(0,%d,%d,1,Specs,OutSize,OutHandle) returned 0x%08X"),
					(DWORD)LocalUserNum,NumRows,Return);
				if (Return == ERROR_SUCCESS)
				{
					AsyncTask->Init(hEnumerate,BufferSize);
					// Start the async enumeration
					Return = XEnumerate(hEnumerate,
						AsyncTask->GetReadResults(),
						BufferSize,
						NULL,
						*AsyncTask);
					debugf(NAME_DevOnline,
						TEXT("XEnumerate(hEnumerate,Data,%d,Data,Overlapped) returned 0x%08X"),
						BufferSize,Return);
					if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
					{
						AsyncTasks.AddItem(AsyncTask);
					}
					else
					{
						// Don't leak the task/data
						delete AsyncTask;
					}
				}
				else
				{
					delete AsyncTask;
					debugf(NAME_DevOnline,
						TEXT("Failed to determine buffer size needed for stats enumeration 0x%08X"),
						Return);
				}
			}
			else
			{
				debugf(NAME_DevOnline,TEXT("Invalid player index specified %d"),
					(DWORD)LocalUserNum);
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("Can't perform a stats read with a null object"));
		}
		// Fire the delegate upon error
		if (Return != ERROR_SUCCESS && Return != ERROR_IO_PENDING)
		{
			CurrentStatsRead = NULL;
			// Just trigger the delegate as having failed
			FAsyncTaskDelegateResults Results(Return);
			TriggerOnlineDelegates(this,ReadOnlineStatsCompleteDelegates,&Results);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Can't perform a stats read while one is in progress"));
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Copies the stats data from our Epic form to something Live can handle
 *
 * @param Stats the destination buffer the stats are written to
 * @param Properties the Epic structures that need copying over
 * @param RatingId the id to set as the rating for this leaderboard set
 */
void UOnlineSubsystemLive::CopyStatsToProperties(XUSER_PROPERTY* Stats,
	const TArray<FSettingsProperty>& Properties,const DWORD RatingId)
{
	check(Properties.Num() < 64);
	// Copy each Epic property into the Live form
	for (INT Index = 0; Index < Properties.Num(); Index++)
	{
		const FSettingsProperty& Property = Properties(Index);
		// Assign the values
		Stats[Index].dwPropertyId = Property.PropertyId;
		// Do per type init
		switch (Property.Data.Type)
		{
			case SDT_Int32:
			{
				// Determine if this property needs to be promoted
				if (Property.PropertyId != RatingId)
				{
					Property.Data.GetData((INT&)Stats[Index].value.nData);
					Stats[Index].value.type = XUSER_DATA_TYPE_INT32;
				}
				else
				{
					INT Value;
					// Promote this value from Int32 to 64
					Property.Data.GetData(Value);
					Stats[Index].value.i64Data = (QWORD)Value;
					Stats[Index].value.type = XUSER_DATA_TYPE_INT64;
				}
				break;
			}
			case SDT_Float:
			{
				FLOAT Convert = 0.f;
				// Read it as a float, but report as a double
				Property.Data.GetData(Convert);
				Stats[Index].value.dblData = Convert;
				Stats[Index].value.type = XUSER_DATA_TYPE_DOUBLE;
				break;
			}
			case SDT_Double:
			{
				Property.Data.GetData(Stats[Index].value.dblData);
				Stats[Index].value.type = XUSER_DATA_TYPE_DOUBLE;
				break;
			}
			case SDT_Int64:
			{
				Property.Data.GetData((QWORD&)Stats[Index].value.i64Data);
				Stats[Index].value.type = XUSER_DATA_TYPE_INT64;
				break;
			}
			default:
			{
				Stats[Index].value.type = 0;
				debugf(NAME_DevOnline,
					TEXT("Ignoring stat type %d at index %d as it is unsupported by Live"),
					Property.Data.Type,Index);
				break;
			}
		}
		if (bShouldLogStatsData)
		{
			debugfLiveSlow(NAME_DevOnline,TEXT("Writing stat (%d) of type (%d) with value %s"),
				Property.PropertyId,Property.Data.Type,*Property.Data.ToString());
		}
	}
}

/**
 * Cleans up any platform specific allocated data contained in the stats data
 *
 * @param StatsRead the object to handle per platform clean up on
 */
void UOnlineSubsystemLive::FreeStats(UOnlineStatsRead* StatsRead)
{
	check(StatsRead);
	// We just empty these without any special platform data...yet
	StatsRead->Rows.Empty();
}

/**
 * Writes out the stats contained within the stats write object to the online
 * subsystem's cache of stats data. Note the new data replaces the old. It does
 * not write the data to the permanent storage until a FlushOnlineStats() call
 * or a session ends. Stats cannot be written without a session or the write
 * request is ignored. No more than 5 stats views can be written to at a time
 * or the write request is ignored.
 *
 * @param SessionName the name of the session the stats are for
 * @param Player the player to write stats for
 * @param StatsWrite the object containing the information to write
 *
 * @return TRUE if the call is successful, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::WriteOnlineStats(FName SessionName,FUniqueNetId Player,
	UOnlineStatsWrite* StatsWrite)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	// Grab the session information by name
	FNamedSession* Session = GetNamedSession(SessionName);
	// Can only write stats data as part of a session
	if (Session && Session->GameSettings && Session->SessionInfo)
	{
		if (StatsWrite != NULL)
		{
#if !FINAL_RELEASE
			// Validate the number of views and the number of stats per view
			if (IsValidStatsWrite(StatsWrite))
#endif
			{
				if (bShouldLogStatsData)
				{
					debugfLiveSlow(NAME_DevOnline,
						TEXT("Writing stats object type %s for session (%s)"),
						*StatsWrite->GetClass()->GetName(),
						*SessionName.ToString());
				}
				// The live specific buffers to use
				appMemzero(Views,sizeof(XSESSION_VIEW_PROPERTIES) * MAX_VIEWS);
				appMemzero(Stats,sizeof(XUSER_PROPERTY) * MAX_STATS);
				// Copy stats properties to the Live data
				CopyStatsToProperties(Stats,StatsWrite->Properties,StatsWrite->RatingId);
				// Get the number of views/stats involved
				DWORD StatsCount = StatsWrite->Properties.Num();
				// Switch upon the arbitration setting which set of view ids to use
				DWORD ViewCount = Session->GameSettings->bUsesArbitration ?
					StatsWrite->ArbitratedViewIds.Num() :
					StatsWrite->ViewIds.Num();
				INT* ViewIds = Session->GameSettings->bUsesArbitration ?
					(INT*)StatsWrite->ArbitratedViewIds.GetData() :
					(INT*)StatsWrite->ViewIds.GetData();
				// Initialize the view data for each view involved
				for (DWORD Index = 0; Index < ViewCount; Index++)
				{
					Views[Index].dwViewId = ViewIds[Index];
					Views[Index].dwNumProperties = StatsCount;
					Views[Index].pProperties = Stats;
					if (bShouldLogStatsData)
					{
						debugfLiveSlow(NAME_DevOnline,TEXT("ViewId = %d, NumProps = %d"),ViewIds[Index],StatsCount);
					}
				}
				FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
				// Write the data to the leaderboard
				Return = XSessionWriteStats(SessionInfo->Handle,
					(XUID&)Player,
					ViewCount,
					Views,
					NULL);
				debugf(NAME_DevOnline,
					TEXT("XSessionWriteStats() '%s' return 0x%08X"),
					*SessionName.ToString(),
					Return);
			}
#if !FINAL_RELEASE
			else
			{
				debugf(NAME_DevOnline,
					TEXT("Invalid stats write object specified for session (%s)"),
					*SessionName.ToString());
			}
#endif
		}
		else
		{
			debugf(NAME_DevOnline,
				TEXT("Can't write stats for session (%s) using a null object"),
				*SessionName.ToString());
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Can't write stats for session (%s) without a Live game in progress"),
			*SessionName.ToString());
	}
	return Return == ERROR_SUCCESS;
}

/**
 * Commits any changes in the online stats cache to the permanent storage
 *
 * @param SessionName the name of the session flushing stats
 *
 * @return TRUE if the call is successful, FALSE otherwise
 */
UBOOL UOnlineSubsystemLive::FlushOnlineStats(FName SessionName)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	// Grab the session information by name
	FNamedSession* Session = GetNamedSession(SessionName);
	// Error if there isn't a session going
	if (Session && Session->GameSettings && Session->SessionInfo)
	{
		FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
		// Allocate an async task for deferred calling of the delegates
		FOnlineAsyncTaskLiveNamedSession* AsyncTask = new FOnlineAsyncTaskLiveNamedSession(SessionName,
			&FlushOnlineStatsDelegates,
			NULL,
			TEXT("XSessionFlushStats()"));
		// Show the live guide ui for inputing text
		Return = XSessionFlushStats(SessionInfo->Handle,*AsyncTask);
		debugf(NAME_DevOnline,
			TEXT("XSessionFlushStats() '%s' return 0x%08X"),
			*SessionName.ToString(),
			Return);
		if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
		{
			AsyncTasks.AddItem(AsyncTask);
		}
		else
		{
			// Just trigger the delegate as having failed
			FAsyncTaskDelegateResultsNamedSession Results(SessionName,Return);
			TriggerOnlineDelegates(this,FlushOnlineStatsDelegates,&Results);
			// Don't leak the task/data
			delete AsyncTask;
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Can't flush stats for session (%s) without a Live game in progress"),
			*SessionName.ToString());
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Updates the flags and number of public/private slots that are available
 *
 * @param Session the session to modify/update
 * @param ScriptDelegates the set of delegates to fire when the modify complete
 */
DWORD UOnlineSubsystemLive::ModifySession(FNamedSession* Session,TArray<FScriptDelegate>* ScriptDelegates)
{
	check(Session && Session->GameSettings && Session->SessionInfo);
	// We must pass the same flags in (zero doesn't mean please don't change stuff)
	DWORD SessionFlags = BuildSessionFlags(Session->GameSettings);
	if (IsServer() == FALSE)
	{
		// Strip off the hosting flag if specified
		SessionFlags &= ~XSESSION_CREATE_HOST;
	}
	FSecureSessionInfo* SessionInfo = GetSessionInfo(Session);
	// Allocate the object that will hold the data throughout the async lifetime
	FOnlineAsyncTaskLiveNamedSession* AsyncTask = new FOnlineAsyncTaskLiveNamedSession(Session->SessionName,
		ScriptDelegates,
		NULL,
		TEXT("XSessionModify()"));
	// Tell Live to shrink to the number of public/private specified
	DWORD Result = XSessionModify(SessionInfo->Handle,
		SessionFlags,
		Session->GameSettings->NumPublicConnections,
		Session->GameSettings->NumPrivateConnections,
		*AsyncTask);
	debugfLiveSlow(NAME_DevOnline,
		TEXT("Updating session (%s) flags and size to public = %d, private = %d XSessionModify() returned 0x%08X"),
		*Session->SessionName.ToString(),
		Session->GameSettings->NumPublicConnections,
		Session->GameSettings->NumPrivateConnections,
		Result);
	// Clean up the task if it didn't succeed
	if (Result == ERROR_IO_PENDING)
	{
		// Queue the async task for ticking
		AsyncTasks.AddItem(AsyncTask);
	}
	else
	{
		delete AsyncTask;
		if (ScriptDelegates)
		{
			FAsyncTaskDelegateResultsNamedSession Results(Session->SessionName,Result);
			TriggerOnlineDelegates(this,*ScriptDelegates,&Results);
		}
	}
	return Result;
}

/**
 * Shrinks the session to the number of arbitrated registrants
 *
 * @param Session the session to modify/update
 */
void UOnlineSubsystemLive::ShrinkToArbitratedRegistrantSize(FNamedSession* Session)
{
	// Don't try to modify if you aren't the server
	if (IsServer() && Session && Session->GameSettings)
	{
		// Update the number of public slots on the game settings
		Session->GameSettings->NumPublicConnections = Session->ArbitrationRegistrants.Num();
		Session->GameSettings->NumOpenPublicConnections = Session->ArbitrationRegistrants.Num() - 1;
		Session->GameSettings->NumPrivateConnections = 0;
		Session->GameSettings->NumOpenPrivateConnections = 0;
		// Flush the data up to Live
		ModifySession(Session,NULL);
	}
}

/**
 * Determines if the packet header is valid or not
 *
 * @param Packet the packet data to check
 * @param Length the size of the packet buffer
 *
 * @return true if the header is valid, false otherwise
 */
UBOOL UOnlineSubsystemLive::IsValidLanQueryPacket(const BYTE* Packet,
	DWORD Length,QWORD& ClientNonce)
{
	ClientNonce = 0;
	UBOOL bIsValid = FALSE;
	// Serialize out the data if the packet is the right size
	if (Length == LAN_BEACON_PACKET_HEADER_SIZE)
	{
		FNboSerializeFromBuffer PacketReader(Packet,Length);
		BYTE Version = 0;
		PacketReader >> Version;
		// Do the versions match?
		if (Version == LAN_BEACON_PACKET_VERSION)
		{
			BYTE Platform = 255;
			PacketReader >> Platform;
			// Can we communicate with this platform?
			if (Platform & LanPacketPlatformMask)
			{
				INT GameId = -1;
				PacketReader >> GameId;
				// Is this our game?
				if (GameId == LanGameUniqueId)
				{
					BYTE SQ1 = 0;
					PacketReader >> SQ1;
					BYTE SQ2 = 0;
					PacketReader >> SQ2;
					// Is this a server query?
					bIsValid = (SQ1 == LAN_SERVER_QUERY1 && SQ2 == LAN_SERVER_QUERY2);
					// Read the client nonce as the outvalue
					PacketReader >> ClientNonce;
				}
			}
		}
	}
	return bIsValid;
}

/**
 * Determines if the packet header is valid or not
 *
 * @param Packet the packet data to check
 * @param Length the size of the packet buffer
 *
 * @return true if the header is valid, false otherwise
 */
UBOOL UOnlineSubsystemLive::IsValidLanResponsePacket(const BYTE* Packet,DWORD Length)
{
	UBOOL bIsValid = FALSE;
	// Serialize out the data if the packet is the right size
	if (Length > LAN_BEACON_PACKET_HEADER_SIZE)
	{
		FNboSerializeFromBuffer PacketReader(Packet,Length);
		BYTE Version = 0;
		PacketReader >> Version;
		// Do the versions match?
		if (Version == LAN_BEACON_PACKET_VERSION)
		{
			BYTE Platform = 255;
			PacketReader >> Platform;
			// Can we communicate with this platform?
			if (Platform & LanPacketPlatformMask)
			{
				INT GameId = -1;
				PacketReader >> GameId;
				// Is this our game?
				if (GameId == LanGameUniqueId)
				{
					BYTE SQ1 = 0;
					PacketReader >> SQ1;
					BYTE SQ2 = 0;
					PacketReader >> SQ2;
					// Is this a server response?
					if (SQ1 == LAN_SERVER_RESPONSE1 && SQ2 == LAN_SERVER_RESPONSE2)
					{
						QWORD Nonce = 0;
						PacketReader >> Nonce;
						bIsValid = Nonce == (QWORD&)LanNonce;
					}
				}
			}
		}
	}
	return bIsValid;
}

/**
 * Ticks any lan beacon background tasks
 *
 * @param DeltaTime the time since the last tick
 */
void UOnlineSubsystemLive::TickLanTasks(FLOAT DeltaTime)
{
	if (LanBeaconState > LANB_NotUsingLanBeacon && LanBeacon != NULL)
	{
		BYTE PacketData[512];
		UBOOL bShouldRead = TRUE;
		// Read each pending packet and pass it out for processing
		while (bShouldRead)
		{
			INT NumRead = LanBeacon->ReceivePacket(PacketData,512);
			if (NumRead > 0)
			{
				// Hand this packet off to child classes for processing
				ProcessLanPacket(PacketData,NumRead);
				// Reset the timeout since a packet came in
				LanQueryTimeLeft = LanQueryTimeout;
			}
			else
			{
				if (LanBeaconState == LANB_Searching)
				{
					// Decrement the amount of time remaining
					LanQueryTimeLeft -= DeltaTime;
					// Check for a timeout on the search packet
					if (LanQueryTimeLeft <= 0.f)
					{
						// Stop future timeouts since we aren't searching any more
						StopLanBeacon();
						if (GameSearch != NULL)
						{
							GameSearch->bIsSearchInProgress = FALSE;
						}
						// Trigger the delegate so the UI knows we didn't find any
						FAsyncTaskDelegateResults Results(S_OK);
						TriggerOnlineDelegates(this,FindOnlineGamesCompleteDelegates,&Results);
					}
				}
				bShouldRead = FALSE;
			}
		}
	}
}

/**
 * Adds the game settings data to the packet that is sent by the host
 * in reponse to a server query
 *
 * @param Packet the writer object that will encode the data
 * @param GameSettings the game settings to add to the packet
 */
void UOnlineSubsystemLive::AppendGameSettingsToPacket(FNboSerializeToBuffer& Packet,
	UOnlineGameSettings* GameSettings)
{
#if DEBUG_LAN_BEACON
	debugf(NAME_DevOnline,TEXT("Sending game settings to client"));
#endif
	// Members of the game settings class
	Packet << GameSettings->NumOpenPublicConnections
		<< GameSettings->NumOpenPrivateConnections
		<< GameSettings->NumPublicConnections
		<< GameSettings->NumPrivateConnections
		<< (BYTE)GameSettings->bShouldAdvertise
		<< (BYTE)GameSettings->bIsLanMatch
		<< (BYTE)GameSettings->bUsesStats
		<< (BYTE)GameSettings->bAllowJoinInProgress
		<< (BYTE)GameSettings->bAllowInvites
		<< (BYTE)GameSettings->bUsesPresence
		<< (BYTE)GameSettings->bAllowJoinViaPresence
		<< (BYTE)GameSettings->bUsesArbitration;
	// Write the player id so we can show gamercard
	Packet << GameSettings->OwningPlayerId;
	Packet << GameSettings->OwningPlayerName;
#if DEBUG_SYSLINK
	QWORD Uid = (QWORD&)GameSettings->OwningPlayerId.Uid;
	debugf(NAME_DevOnline,TEXT("%s 0x%016I64X"),*GameSettings->OwningPlayerName,Uid);
#endif
	// Now add the contexts and properties from the settings class
	// First, add the number contexts involved
	INT Num = GameSettings->LocalizedSettings.Num();
	Packet << Num;
	// Now add each context individually
	for (INT Index = 0; Index < GameSettings->LocalizedSettings.Num(); Index++)
	{
		Packet << GameSettings->LocalizedSettings(Index);
#if DEBUG_LAN_BEACON
		debugf(NAME_DevOnline,*BuildContextString(GameSettings,GameSettings->LocalizedSettings(Index)));
#endif
	}
	// Next, add the number of properties involved
	Num = GameSettings->Properties.Num();
	Packet << Num;
	// Now add each property
	for (INT Index = 0; Index < GameSettings->Properties.Num(); Index++)
	{
		Packet << GameSettings->Properties(Index);
#if DEBUG_LAN_BEACON
		debugf(NAME_DevOnline,*BuildPropertyString(GameSettings,GameSettings->Properties(Index)));
#endif
	}
}

/**
 * Reads the game settings data from the packet and applies it to the
 * specified object
 *
 * @param Packet the reader object that will read the data
 * @param GameSettings the game settings to copy the data to
 */
void UOnlineSubsystemLive::ReadGameSettingsFromPacket(FNboSerializeFromBuffer& Packet,
	UOnlineGameSettings* GameSettings)
{
#if DEBUG_LAN_BEACON
	debugf(NAME_DevOnline,TEXT("Reading game settings from server"));
#endif
	// Members of the game settings class
	Packet >> GameSettings->NumOpenPublicConnections
		>> GameSettings->NumOpenPrivateConnections
		>> GameSettings->NumPublicConnections
		>> GameSettings->NumPrivateConnections;
	BYTE Read = FALSE;
	// Read all the bools as bytes
	Packet >> Read;
	GameSettings->bShouldAdvertise = Read == TRUE;
	Packet >> Read;
	GameSettings->bIsLanMatch = Read == TRUE;
	Packet >> Read;
	GameSettings->bUsesStats = Read == TRUE;
	Packet >> Read;
	GameSettings->bAllowJoinInProgress = Read == TRUE;
	Packet >> Read;
	GameSettings->bAllowInvites = Read == TRUE;
	Packet >> Read;
	GameSettings->bUsesPresence = Read == TRUE;
	Packet >> Read;
	GameSettings->bAllowJoinViaPresence = Read == TRUE;
	Packet >> Read;
	GameSettings->bUsesArbitration = Read == TRUE;
	// Read the owning player id
	Packet >> GameSettings->OwningPlayerId;
	// Read the owning player name
	Packet >> GameSettings->OwningPlayerName;
#if DEBUG_LAN_BEACON
	QWORD Uid = (QWORD&)GameSettings->OwningPlayerId.Uid;
	debugf(NAME_DevOnline,TEXT("%s 0x%016I64X"),*GameSettings->OwningPlayerName,Uid);
#endif
	// Now read the contexts and properties from the settings class
	INT NumContexts = 0;
	// First, read the number contexts involved, so we can presize the array
	Packet >> NumContexts;
	if (Packet.HasOverflow() == FALSE)
	{
		GameSettings->LocalizedSettings.Empty(NumContexts);
		GameSettings->LocalizedSettings.AddZeroed(NumContexts);
	}
	// Now read each context individually
	for (INT Index = 0;
		Index < GameSettings->LocalizedSettings.Num() && Packet.HasOverflow() == FALSE;
		Index++)
	{
		Packet >> GameSettings->LocalizedSettings(Index);
#if DEBUG_LAN_BEACON
		debugf(NAME_DevOnline,*BuildContextString(GameSettings,GameSettings->LocalizedSettings(Index)));
#endif
	}
	INT NumProps = 0;
	// Next, read the number of properties involved for array presizing
	Packet >> NumProps;
	if (Packet.HasOverflow() == FALSE)
	{
		GameSettings->Properties.Empty(NumProps);
		GameSettings->Properties.AddZeroed(NumProps);
	}
	// Now read each property from the packet
	for (INT Index = 0;
		Index < GameSettings->Properties.Num() && Packet.HasOverflow() == FALSE;
		Index++)
	{
		Packet >> GameSettings->Properties(Index);
#if DEBUG_LAN_BEACON
		debugf(NAME_DevOnline,*BuildPropertyString(GameSettings,GameSettings->Properties(Index)));
#endif
	}
	// If there was an overflow, treat the string settings/properties as broken
	if (Packet.HasOverflow())
	{
		GameSettings->LocalizedSettings.Empty();
		GameSettings->Properties.Empty();
		debugf(NAME_DevOnline,TEXT("Packet overflow detected in ReadGameSettingsFromPacket()"));
	}
}

/**
 * Builds a LAN query and broadcasts it
 *
 * @return an error/success code
 */
DWORD UOnlineSubsystemLive::FindLanGames(void)
{
	DWORD Return = S_OK;
	// Recreate the unique identifier for this client
	XNetRandom(LanNonce,8);
	// Create the lan beacon if we don't already have one
	if (LanBeacon == NULL)
	{
		LanBeacon = new FLanBeacon();
		if (LanBeacon->Init(LanAnnouncePort) == FALSE)
		{
			debugf(NAME_DevOnline,TEXT("Failed to create socket for lan announce port %d"),
				GSocketSubsystem->GetSocketError());
			Return = E_FAIL;
		}
	}
	// If we have a socket and a nonce, broadcast a discovery packet
	if (LanBeacon && Return == S_OK)
	{
		QWORD Nonce = *(QWORD*)LanNonce;
		FNboSerializeToBuffer Packet(LAN_BEACON_MAX_PACKET_SIZE);
		// Build the discovery packet
		Packet << LAN_BEACON_PACKET_VERSION
			// Platform information
			<< (BYTE)appGetPlatformType()
			// Game id to prevent cross game lan packets
			<< LanGameUniqueId
			// Identify the packet type
			<< LAN_SERVER_QUERY1 << LAN_SERVER_QUERY2
			// Append the nonce as a QWORD
			<< Nonce;
		// Now kick off our broadcast which hosts will respond to
		if (LanBeacon->BroadcastPacket(Packet,Packet.GetByteCount()))
		{
			debugfLiveSlow(NAME_DevOnline,TEXT("Sent query packet..."));
			// We need to poll for the return packets
			LanBeaconState = LANB_Searching;
			// Set the timestamp for timing out a search
			LanQueryTimeLeft = LanQueryTimeout;
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("Failed to send discovery broadcast %d"),
				GSocketSubsystem->GetSocketError());
			Return = E_FAIL;
		}
	}
	if (Return != S_OK)
	{
		delete LanBeacon;
		LanBeacon = NULL;
		LanBeaconState = LANB_NotUsingLanBeacon;
	}
	return Return;
}

/**
 * Sends a message to a friend
 *
 * @param LocalUserNum the user that is sending the message
 * @param Friend the player to send the message to
 * @param Message the message to display to the recipient
 *
 * @return true if successful, false otherwise
 */
UBOOL UOnlineSubsystemLive::SendMessageToFriend(BYTE LocalUserNum,FUniqueNetId Friend,const FString& Message)
{
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Show the UI with the message in it
		return XShowMessageComposeUI(LocalUserNum,(XUID*)&Friend,1,*Message);
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid or not logged in player specified (%d)"),(DWORD)LocalUserNum);
	}
	return FALSE;
}

/**
 * Sends an invitation to play in the player's current session
 *
 * @param LocalUserNum the user that is sending the invite
 * @param Friend the player to send the invite to
 * @param Text the message to accompany the invite
 *
 * @return true if successful, false otherwise
 */
UBOOL UOnlineSubsystemLive::SendGameInviteToFriend(BYTE LocalUserNum,FUniqueNetId Friend,const FString& Text)
{
	TArray<FUniqueNetId> Friends;
	Friends.AddItem(Friend);
	// Use the group method to do the send
	return SendGameInviteToFriends(LocalUserNum,Friends,Text);
}

/**
 * Sends invitations to play in the player's current session
 *
 * @param LocalUserNum the user that is sending the invite
 * @param Friends the player to send the invite to
 * @param Text the message to accompany the invite
 *
 * @return true if successful, false otherwise
 */
UBOOL UOnlineSubsystemLive::SendGameInviteToFriends(BYTE LocalUserNum,const TArray<FUniqueNetId>& Friends,const FString& Text)
{
	DWORD Return = E_FAIL;
	if (LocalUserNum >=0 && LocalUserNum < 4)
	{
		UBOOL bHasInvitableSession = FALSE;
		// Make sure that there is an invitable session present
		for (INT Index = 0; Index < Sessions.Num(); Index++)
		{
			const FNamedSession& Session = Sessions(Index);
			// Make sure the session is invitable and not full
			if (Session.GameSettings != NULL &&
				Session.GameSettings->bAllowInvites &&
				(Session.GameSettings->NumPublicConnections + Session.GameSettings->NumPrivateConnections) > Session.Registrants.Num())
			{
				bHasInvitableSession = TRUE;
				break;
			}
		}
		if (bHasInvitableSession)
		{
			// Create an async task for logging the process
			FLiveAsyncTaskInviteToGame* AsyncTask = new FLiveAsyncTaskInviteToGame(Friends,Text);
			// Send the invites async
			Return = XInviteSend(LocalUserNum,
				AsyncTask->GetInviteeCount(),
				AsyncTask->GetInvitees(),
				AsyncTask->GetMessage(),
				*AsyncTask);
			debugf(NAME_DevOnline,TEXT("XInviteSend(%d,%d,,%s,) returned 0x%08X"),
				(DWORD)LocalUserNum,
				AsyncTask->GetInviteeCount(),
				AsyncTask->GetMessage(),
				Return);
			if (Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING)
			{
				// Queue the async task for ticking
				AsyncTasks.AddItem(AsyncTask);
			}
			else
			{
				delete AsyncTask;
			}
		}
		else
		{
			debugf(NAME_DevOnline,TEXT("No invitable session present. Not sending an invite"));
		}
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Attempts to join a friend's game session (join in progress)
 *
 * @param LocalUserNum the user that is sending the invite
 * @param FriendId the player to follow into the match
 *
 * @return true if successful, false otherwise
 */
UBOOL UOnlineSubsystemLive::JoinFriendGame(BYTE LocalUserNum,FUniqueNetId FriendId)
{
	if (LocalUserNum >=0 && LocalUserNum < 4)
	{
		if (IsUserInSession((QWORD&)FriendId) == FALSE)
		{
			QWORD SessionId = 0;
			const TArray<FOnlineFriend>& Friends = FriendsCache[LocalUserNum].Friends;
			// Find the friend in the cached list and search for that session id
			for (INT FriendIndex = 0; FriendIndex < Friends.Num(); FriendIndex++)
			{
				const FOnlineFriend& Friend = Friends(FriendIndex);
				if (Friend.UniqueId == FriendId)
				{
					// This is the session that we are going to try to join
					SessionId = Friend.SessionId;
					break;
				}
			}
			if (SessionId != 0)
			{
				// This code assumes XNKID is 8 bytes
				check(sizeof(QWORD) == sizeof(XNKID));
				XINVITE_INFO* Info;
				// Allocate space on demand
				if (InviteCache[LocalUserNum].InviteData == NULL)
				{
					InviteCache[LocalUserNum].InviteData = new XINVITE_INFO;
				}
				// If for some reason the data didn't get cleaned up, do so now
				if (InviteCache[LocalUserNum].InviteSearch != NULL &&
					InviteCache[LocalUserNum].InviteSearch->Results.Num() > 0)
				{
					// Clean up the invite data
					delete (XSESSION_INFO*)InviteCache[LocalUserNum].InviteSearch->Results(0).PlatformData;
					InviteCache[LocalUserNum].InviteSearch->Results(0).PlatformData = NULL;
					InviteCache[LocalUserNum].InviteSearch = NULL;
				}
				// Get the buffer to use and clear the previous contents
				Info = InviteCache[LocalUserNum].InviteData;
				appMemzero(Info,sizeof(XINVITE_INFO));
				// Fill in the invite data manually
				Info->dwTitleID = appGetTitleId();
				Info->fFromGameInvite = TRUE;
				Info->xuidInviter = FriendId.Uid;
				// Now use the join by id code
				return HandleJoinBySessionId(LocalUserNum,FriendId.Uid,SessionId);
			}
			else
			{
				debugf(NAME_DevOnline,TEXT("Friend 0x%016I64X was not in a joinable session"),FriendId.Uid);
			}
		}
		else
		{
			debugf(NAME_DevOnline,
				TEXT("Player (%d) is already in the session with friend 0x%016I64X"),
				LocalUserNum,
				FriendId.Uid);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to JoinFriendGame()"),
			(DWORD)LocalUserNum);
	}
	return FALSE;
}

/**
 * Starts an asynchronous read of the specified file from the network platform's
 * title specific file store
 *
 * @param FileToRead the name of the file to read
 *
 * @return true if the calls starts successfully, false otherwise
 */
UBOOL UOnlineSubsystemLive::ReadTitleFile(const FString& FileToRead)
{
	if (XUserGetSigninState(0) == eXUserSigninState_SignedInToLive)
	{
		WCHAR ServerPath[XONLINE_MAX_PATHNAME_LENGTH];
		ServerPath[0] = L'\0';
		DWORD ServerPathLen = XONLINE_MAX_PATHNAME_LENGTH;
		// Get the name to send to Live for finding files
		DWORD Result = XStorageBuildServerPath(0,
			XSTORAGE_FACILITY_PER_TITLE,
			NULL,
			0,
			*FileToRead,
			ServerPath,
			&ServerPathLen);
		debugfLiveSlow(NAME_DevLive,TEXT("XStorageBuildServerPath(%s) returned 0x%08X with path %s"),
			*FileToRead,
			Result,
			ServerPath);
		if (Result == ERROR_SUCCESS)
		{
			// Create the async task that will hold the enumeration data
			FLiveAsyncTMSRead* AsyncTask = new FLiveAsyncTMSRead(TitleManagedFiles,&ReadTitleFileCompleteDelegates);
			// Start the file list enumeration
			Result = XStorageEnumerate(0,
				ServerPath,
				0,
				MAX_TITLE_MANAGED_STORAGE_FILES,
				AsyncTask->GetAllocatedSize(),
				AsyncTask->GetEnumerationResults(),
				*AsyncTask);
			if (Result == ERROR_SUCCESS || Result == ERROR_IO_PENDING)
			{
				// Queue the async task for ticking
				AsyncTasks.AddItem(AsyncTask);
			}
			else
			{
				debugf(NAME_DevOnline,
					TEXT("Failed to enumerate TMS file(s) (%s). XStorageEnumerate() returned 0x%08X"),
					*FileToRead,
					Result);
				delete AsyncTask;
			}
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Not logged into Live. Can't download TMS data"));
	}
	return TRUE;
}

/**
 * Copies the file data into the specified buffer for the specified file
 *
 * @param FileToRead the name of the file to read
 * @param FileContents the out buffer to copy the data into
 *
 * @return true if the data was copied, false otherwise
 */
UBOOL UOnlineSubsystemLive::GetTitleFileContents(const FString& FileName,TArray<BYTE>& FileContents)
{
	// Search for the specified file and return the raw data
	for (INT FileIndex = 0; FileIndex < TitleManagedFiles.Num(); FileIndex++)
	{
		FTitleFile& TitleFile = TitleManagedFiles(FileIndex);
		if (TitleFile.Filename == FileName)
		{
			FileContents = TitleFile.Data;
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Starts an async read for the achievement list
 *
 * @param LocalUserNum the controller number of the associated user
 * @param TitleId the title id of the game the achievements are to be read for
 * @param bShouldReadText whether to fetch the text strings or not
 * @param bShouldReadImages whether to fetch the image data or not
 *
 * @return TRUE if the task starts, FALSE if it failed
 */
UBOOL UOnlineSubsystemLive::ReadAchievements(BYTE LocalUserNum,INT TitleId,UBOOL bShouldReadText,UBOOL bShouldReadImages)
{
	DWORD Return = XONLINE_E_SESSION_WRONG_STATE;
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		XUID Xuid = INVALID_XUID;
		// Get the struct that we'll fill
		FCachedAchievements& Cached = GetCachedAchievements(LocalUserNum,TitleId);
		// Ignore multiple reads on the same achievement list
		if (Cached.ReadState == OERS_NotStarted)
		{
			HANDLE Handle = NULL;
			DWORD SizeNeeded = 0;
			DWORD Flags = 0;
			// Set the flags on what to read based upon the flags passed in
			if (bShouldReadText)
			{
				Flags |= XACHIEVEMENT_DETAILS_LABEL | XACHIEVEMENT_DETAILS_DESCRIPTION | XACHIEVEMENT_DETAILS_UNACHIEVED;
			}
			// Create a new enumerator for reading the achievements list
			Return = XUserCreateAchievementEnumerator((DWORD)TitleId,
				LocalUserNum,
				Xuid,
				Flags,
				0,
				60,
				&SizeNeeded,
				&Handle);
			debugfLiveSlow(NAME_DevOnline,
				TEXT("XUserCreateAchievementEnumerator(0x%08X,%d,0x%016I64X,0,1,%d,out) returned 0x%08X"),
				TitleId,
				(DWORD)LocalUserNum,
				Xuid,
				SizeNeeded,
				Return);
			if (Return == ERROR_SUCCESS)
			{
				// Create the async data object that holds the buffers, etc.
				FLiveAsyncTaskDataReadAchievements* AsyncTaskData = new FLiveAsyncTaskDataReadAchievements(TitleId,LocalUserNum,Handle,SizeNeeded,0);
				// Create the async task object
				FLiveAsyncTaskReadAchievements* AsyncTask = new FLiveAsyncTaskReadAchievements(
					&PerUserDelegates[LocalUserNum].AchievementReadDelegates,
					AsyncTaskData);
				// Start the async read
				Return = XEnumerate(Handle,
					AsyncTaskData->GetBuffer(),
					SizeNeeded,
					0,
					*AsyncTask);
				if (Return == ERROR_IO_PENDING)
				{
					// Mark this as being read
					Cached.ReadState = OERS_InProgress;
					AsyncTasks.AddItem(AsyncTask);
				}
				else
				{
					// Delete the async task
					delete AsyncTask;
				}
			}
		}
		// If it has already been done, indicate success
		else if (Cached.ReadState == OERS_Done)
		{
			Return = ERROR_SUCCESS;
		}
		// If one is in progress, indicate that it is still outstanding
		else if (Cached.ReadState == OERS_InProgress)
		{
			Return = ERROR_IO_PENDING;
		}
		if (Return != ERROR_SUCCESS && Return != ERROR_IO_PENDING)
		{
			Cached.ReadState = OERS_Failed;
		}
	}
	else
	{
		debugf(NAME_DevOnline,
			TEXT("Invalid user specified in ReadAchievements(%d)"),
			(DWORD)LocalUserNum);
	}
	// Fire off the delegate if needed
	if (Return != ERROR_IO_PENDING)
	{
		OnlineSubsystemLive_eventOnReadAchievementsComplete_Parms Parms(EC_EventParm);
		Parms.TitleId = TitleId;
		// Use the common method to do the work
		TriggerOnlineDelegates(this,PerUserDelegates[LocalUserNum].AchievementReadDelegates,&Parms);
	}
	return Return == ERROR_SUCCESS || Return == ERROR_IO_PENDING;
}

/**
 * Copies the list of achievements for the specified player and title id
 *
 * @param LocalUserNum the user to read the friends list of
 * @param Achievements the out array that receives the copied data
 * @param TitleId the title id of the game that these were read for
 *
 * @return OERS_Done if the read has completed, otherwise one of the other states
 */
BYTE UOnlineSubsystemLive::GetAchievements(BYTE LocalUserNum,TArray<FAchievementDetails>& Achievements,INT TitleId)
{
	FCachedAchievements& Cached = GetCachedAchievements(LocalUserNum,TitleId);
	Achievements.Reset();
	Achievements = Cached.Achievements;
	return Cached.ReadState;
}

/**
 * Shows a custom players UI for the specified list of players
 *
 * @param LocalUserNum the controller number of the associated user
 * @param Players the list of players to show in the custom UI
 * @param Title the title to use for the UI
 * @param Description the text to show at the top of the UI
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
UBOOL UOnlineSubsystemLive::ShowCustomPlayersUI(BYTE LocalUserNum,const TArray<FUniqueNetId>& Players,const FString& Title,const FString& Description)
{
	DWORD Result = E_FAIL;
	// Validate the user index passed in
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Copy the various data so that it exists throughout the async call
		FLiveAsyncTaskCustomPlayersList* AsyncTask = new FLiveAsyncTaskCustomPlayersList(Players,Title,Description);
		// Show the live guide ui for the player list
		Result = XShowCustomPlayerListUI(LocalUserNum,
			0,
			AsyncTask->GetTitle(),
			AsyncTask->GetDescription(),
			NULL,
			0,
			AsyncTask->GetPlayers(),
			AsyncTask->GetPlayerCount(),
			NULL,
			NULL,
			NULL,
			*AsyncTask);
		if (Result == ERROR_SUCCESS || Result == ERROR_IO_PENDING)
		{
			AsyncTasks.AddItem(AsyncTask);
		}
		else
		{
			delete AsyncTask;
			debugf(NAME_DevOnline,TEXT("XShowCustomPlayersUI(%d,%d,%s,%s) failed with 0x%08X"),
				(DWORD)LocalUserNum,
				Players.Num(),
				*Title,
				*Description,
				Result);
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("Invalid player index (%d) specified to ShowPlayersUI()"),
			(DWORD)LocalUserNum);
	}
	return Result == ERROR_SUCCESS || Result == ERROR_IO_PENDING;
}

/**
 * Kicks off an async read of the skill information for the list of players in the
 * search object
 *
 * @param SearchingPlayerNum the player executing the search
 * @param SearchSettings the object that has the list of players to use in the read
 *
 * @return the error/success code from the skill search
 */
DWORD UOnlineSubsystemLive::ReadSkillForSearch(BYTE SearchingPlayerNum,UOnlineGameSearch* SearchSettings)
{
	check(SearchSettings);
	// Create an async task to read the stats and kick that off
	FLiveAsyncReadPlayerSkillForSearch* AsyncTask = new FLiveAsyncReadPlayerSkillForSearch(SearchingPlayerNum,SearchSettings);
	// Set the vars passed into the read so we can inspect them when debugging
	DWORD NumPlayers = SearchSettings->ManualSkillOverride.Players.Num();
	XUID* XPlayers = (XUID*)SearchSettings->ManualSkillOverride.Players.GetData();
	DWORD BufferSize = 0;
	// First time through figure out how much memory to allocate for search results
	DWORD Return = XUserReadStats(0,
		NumPlayers,
		XPlayers,
		1,
		AsyncTask->GetSpecs(),
		&BufferSize,
		NULL,
		NULL);
	if (Return == ERROR_INSUFFICIENT_BUFFER && BufferSize > 0)
	{
		// Allocate the results buffer
		AsyncTask->AllocateSpace(BufferSize);
		// Now kick off the async skill leaderboard read
		Return = XUserReadStats(0,
			NumPlayers,
			XPlayers,
			1,
			AsyncTask->GetSpecs(),
			&BufferSize,
			AsyncTask->GetReadBuffer(),
			*AsyncTask);
		debugfLiveSlow(NAME_DevOnline,
			TEXT("XUserReadStats(0,%d,Players,1,Specs,%d,Buffer,Overlapped) for skill read returned 0x%08X"),
			NumPlayers,BufferSize,Return);
		if (Return == ERROR_IO_PENDING)
		{
			AsyncTasks.AddItem(AsyncTask);
		}
	}
	if (Return != ERROR_IO_PENDING)
	{
		// Just trigger the delegate as having failed
		FAsyncTaskDelegateResults Results(Return);
		TriggerOnlineDelegates(this,FindOnlineGamesCompleteDelegates,&Results);
		// Don't leak the task
		delete AsyncTask;
	}
	return Return;
}

/**
 * Returns the skill for the last search if the search manually specified a search value
 * otherwise it uses the default skill rating
 *
 * @param OutMu has the skill rating set
 * @param OutSigma has the skill certainty set
 * @param OutCount has the number of contributing players in it
 */
void UOnlineSubsystemLive::GetLocalSkills(DOUBLE& OutMu,DOUBLE& OutSigma,DOUBLE& OutCount)
{
	// Set to the defaults (middle of the curve, completely uncertain)
	OutMu = 3.0;
	OutSigma = 1.0;
	OutCount = 1.0;
	// Check for a game search overriding the values
	if (GameSearch && GameSearch->ManualSkillOverride.Mus.Num())
	{
		OutCount = GameSearch->ManualSkillOverride.Mus.Num();
		// Use the API to aggregate the skill information
		XSessionCalculateSkill(GameSearch->ManualSkillOverride.Mus.Num(),
			GameSearch->ManualSkillOverride.Mus.GetTypedData(),
			GameSearch->ManualSkillOverride.Sigmas.GetTypedData(),
			&OutMu,
			&OutSigma);
	}
	debugfLiveSlow(NAME_DevOnline,
		TEXT("Local skills: Mu (%f), Sigma (%f), Count (%f), SkillLow (%d), SkillHigh (%d)"),
		OutMu,
		OutSigma,
		OutCount,
		CalculateConservativeSkill(OutMu,OutSigma),
		CalculateOptimisticSkill(OutMu,OutSigma));
}

/**
 * Determines how good of a skill match this session is for the local players
 *
 * @param Mu the skill rating of the local player(s)
 * @param Sigma the certainty of that rating
 * @param PlayerCount the number of players contributing to the skill
 * @param GameSettings the game session to calculate the match quality for
 */
void UOnlineSubsystemLive::CalculateMatchQuality(DOUBLE Mu,DOUBLE Sigma,DOUBLE PlayerCount,UOnlineGameSettings* HostSettings)
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
	// Now use the two sets of skill information to determine the match quality
	HostSettings->MatchQuality = CalculateHostQuality(Mu,Sigma,PlayerCount,HostMu,HostSigma,HostCount);
	debugfLiveSlow(NAME_DevOnline,
		TEXT("Match quality for host %s with skill (Mu (%f), Sigma (%f), Count (%f), SkillLow (%d), SkillHigh (%d)) is %f"),
		*HostSettings->OwningPlayerName,
		HostMu,
		HostSigma,
		HostCount,
		CalculateConservativeSkill(HostMu,HostSigma),
		CalculateOptimisticSkill(HostMu,HostSigma),
		HostSettings->MatchQuality);
}

/**
 * Takes the manual skill override data and places that in the properties array
 *
 * @param SearchSettings the search object to update
 */
void UOnlineSubsystemLive::AppendSkillProperties(UOnlineGameSearch* SearchSettings)
{
	if (SearchSettings->ManualSkillOverride.Players.Num())
	{
		DOUBLE Mu = 3.0;
		DOUBLE Sigma = 1.0;
		// Use the API to aggregate the skill information
		XSessionCalculateSkill(SearchSettings->ManualSkillOverride.Mus.Num(),
			SearchSettings->ManualSkillOverride.Mus.GetTypedData(),
			SearchSettings->ManualSkillOverride.Sigmas.GetTypedData(),
			&Mu,
			&Sigma);
		debugfLiveSlow(NAME_DevOnline,
			TEXT("Searching for match using skill of Mu (%f), Sigma (%f), SkillLow (%d), SkillHigh (%d)"),
			Mu,
			Sigma,
			CalculateConservativeSkill(Mu,Sigma),
			CalculateOptimisticSkill(Mu,Sigma));
		// Set the Mu property and add if not present
		FSettingsProperty* PropMu = SearchSettings->FindProperty(X_PROPERTY_GAMER_MU);
		if (PropMu == NULL)
		{
			INT AddIndex = SearchSettings->Properties.AddZeroed();
			PropMu = &SearchSettings->Properties(AddIndex);
			PropMu->PropertyId = X_PROPERTY_GAMER_MU;
		}
		PropMu->Data.SetData(Mu);
		// Set the Sigma property and add if not present
		FSettingsProperty* PropSigma = SearchSettings->FindProperty(X_PROPERTY_GAMER_SIGMA);
		if (PropSigma == NULL)
		{
			INT AddIndex = SearchSettings->Properties.AddZeroed();
			PropSigma = &SearchSettings->Properties(AddIndex);
			PropSigma->PropertyId = X_PROPERTY_GAMER_SIGMA;
		}
		PropSigma->Data.SetData(Sigma);
	}
	else
	{
		// Remove the special search fields if present
		for (INT PropertyIndex = 0; PropertyIndex < SearchSettings->Properties.Num(); PropertyIndex++)
		{
			FSettingsProperty& Property = SearchSettings->Properties(PropertyIndex);
			if (Property.PropertyId == X_PROPERTY_GAMER_MU ||
				Property.PropertyId == X_PROPERTY_GAMER_SIGMA)
			{
				SearchSettings->Properties.Remove(PropertyIndex);
				PropertyIndex--;
			}
		}
	}
}

/**
 * Enumerates the sessions that are set and call XSessionGetDetails() on them to
 * log Live's view of the session information
 */
void UOnlineSubsystemLive::DumpLiveSessionState(void)
{
	debugf(NAME_ScriptLog,TEXT(""));
	debugf(NAME_ScriptLog,TEXT("Live's online session state"));
	debugf(NAME_ScriptLog,TEXT("-------------------------------------------------------------"));
	debugf(NAME_ScriptLog,TEXT(""));
	debugf(NAME_ScriptLog,TEXT("Number of sessions: %d"),Sessions.Num());
	// Iterate through the sessions listing the session info that Live has
	for (INT Index = 0; Index < Sessions.Num(); Index++)
	{
		debugf(NAME_ScriptLog,TEXT("  Session: %s"),*Sessions(Index).SessionName.ToString());
		FSecureSessionInfo* SessionInfo = GetSessionInfo(&Sessions(Index));
		if (SessionInfo != NULL)
		{
			debugf(NAME_ScriptLog,TEXT("    Handle: 0x%08X"),SessionInfo->Handle);
			// Local details plus space for up to 32 players
			struct FLiveSessionDetails
			{
				XSESSION_LOCAL_DETAILS LocalDetails;
				XSESSION_MEMBER Members[32];

				/** Inits Live structures */
				inline FLiveSessionDetails(void)
				{
					// Zero this out
					appMemzero(this,sizeof(FLiveSessionDetails));
					// Point to the member buffer
					LocalDetails.pSessionMembers = Members;
				}
			};
			DWORD BufferSize = sizeof(FLiveSessionDetails);
			FLiveSessionDetails LSD;
			// Ask Live for the details, which will block and be slow
			DWORD Result = XSessionGetDetails(SessionInfo->Handle,
				&BufferSize,
				&LSD.LocalDetails,
				NULL);
			if (Result == ERROR_SUCCESS)
			{
				debugf(NAME_ScriptLog,TEXT("    GameType: 0x%08X"),LSD.LocalDetails.dwGameType);
				debugf(NAME_ScriptLog,TEXT("    GameMode: 0x%08X"),LSD.LocalDetails.dwGameMode);
				debugf(NAME_ScriptLog,TEXT("    MaxPublic: %d"),LSD.LocalDetails.dwMaxPublicSlots);
				debugf(NAME_ScriptLog,TEXT("    AvailPublic: %d"),LSD.LocalDetails.dwAvailablePublicSlots);
				debugf(NAME_ScriptLog,TEXT("    MaxPrivate: %d"),LSD.LocalDetails.dwMaxPrivateSlots);
				debugf(NAME_ScriptLog,TEXT("    AvailPrivate: %d"),LSD.LocalDetails.dwAvailablePrivateSlots);
				debugf(NAME_ScriptLog,TEXT("    NumLocalRegistrants: %d"),LSD.LocalDetails.dwActualMemberCount);
				debugf(NAME_ScriptLog,TEXT("    Nonce: 0x%016I64X"),LSD.LocalDetails.qwNonce);
				debugf(NAME_ScriptLog,TEXT("    SessionId: 0x%016I64X"),(QWORD&)LSD.LocalDetails.sessionInfo.sessionID);
				debugf(NAME_ScriptLog,TEXT("    ArbitrationId: 0x%016I64X"),(QWORD&)LSD.LocalDetails.xnkidArbitration);
				// Build the flags string
				debugf(NAME_ScriptLog,TEXT("    Flags:"));
				if (LSD.LocalDetails.dwFlags & XSESSION_CREATE_HOST)
				{
					debugf(NAME_ScriptLog,TEXT("           XSESSION_CREATE_HOST |"));
				}
				if (LSD.LocalDetails.dwFlags & XSESSION_CREATE_USES_PEER_NETWORK)
				{
					debugf(NAME_ScriptLog,TEXT("           XSESSION_CREATE_USES_PEER_NETWORK |"));
				}
				if (LSD.LocalDetails.dwFlags & XSESSION_CREATE_USES_MATCHMAKING)
				{
					debugf(NAME_ScriptLog,TEXT("           XSESSION_CREATE_USES_MATCHMAKING |"));
				}
				if (LSD.LocalDetails.dwFlags & XSESSION_CREATE_USES_ARBITRATION)
				{
					debugf(NAME_ScriptLog,TEXT("           XSESSION_CREATE_USES_ARBITRATION |"));
				}
				if (LSD.LocalDetails.dwFlags & XSESSION_CREATE_USES_STATS)
				{
					debugf(NAME_ScriptLog,TEXT("           XSESSION_CREATE_USES_STATS |"));
				}
				if (LSD.LocalDetails.dwFlags & XSESSION_CREATE_USES_PRESENCE)
				{
					debugf(NAME_ScriptLog,TEXT("           XSESSION_CREATE_USES_PRESENCE |"));
				}
				if (LSD.LocalDetails.dwFlags & XSESSION_CREATE_JOIN_VIA_PRESENCE_FRIENDS_ONLY)
				{
					debugf(NAME_ScriptLog,TEXT("           XSESSION_CREATE_JOIN_VIA_PRESENCE_FRIENDS_ONLY |"));
				}
				if (LSD.LocalDetails.dwFlags & XSESSION_CREATE_JOIN_VIA_PRESENCE_DISABLED)
				{
					debugf(NAME_ScriptLog,TEXT("           XSESSION_CREATE_JOIN_VIA_PRESENCE_DISABLED |"));
				}
				if (LSD.LocalDetails.dwFlags & XSESSION_CREATE_INVITES_DISABLED)
				{
					debugf(NAME_ScriptLog,TEXT("           XSESSION_CREATE_INVITES_DISABLED |"));
				}
				if (LSD.LocalDetails.dwFlags & XSESSION_CREATE_JOIN_IN_PROGRESS_DISABLED)
				{
					debugf(NAME_ScriptLog,TEXT("           XSESSION_CREATE_JOIN_IN_PROGRESS_DISABLED |"));
				}
				// Log the Live version of the state
				switch (LSD.LocalDetails.eState)
				{
					case XSESSION_STATE_LOBBY:
					{
						debugf(NAME_ScriptLog,TEXT("    State: XSESSION_STATE_LOBBY"));
						break;
					}
					case XSESSION_STATE_REGISTRATION:
					{
						debugf(NAME_ScriptLog,TEXT("    State: XSESSION_STATE_REGISTRATION"));
						break;
					}
					case XSESSION_STATE_INGAME:
					{
						debugf(NAME_ScriptLog,TEXT("    State: XSESSION_STATE_INGAME"));
						break;
					}
					case XSESSION_STATE_REPORTING:
					{
						debugf(NAME_ScriptLog,TEXT("    State: XSESSION_STATE_REPORTING"));
						break;
					}
					case XSESSION_STATE_DELETED:
					{
						debugf(NAME_ScriptLog,TEXT("    State: XSESSION_STATE_DELETED"));
						break;
					}
				}
				debugf(NAME_ScriptLog,TEXT("    Number of players: %d"),LSD.LocalDetails.dwReturnedMemberCount);
				// List each player in the session
				for (DWORD PlayerIndex = 0; PlayerIndex < LSD.LocalDetails.dwReturnedMemberCount; PlayerIndex++)
				{
					debugf(NAME_ScriptLog,TEXT("      Player: 0x%016I64X"),LSD.Members[PlayerIndex].xuidOnline);
					debugf(NAME_ScriptLog,TEXT("        Type: %s"),LSD.Members[PlayerIndex].dwUserIndex == XUSER_INDEX_NONE ? TEXT("Remote") : TEXT("Local"));
					debugf(NAME_ScriptLog,TEXT("        Private?: %s"),LSD.Members[PlayerIndex].dwFlags & XSESSION_MEMBER_FLAGS_PRIVATE_SLOT ? TEXT("True") : TEXT("False"));
					debugf(NAME_ScriptLog,TEXT("        Zombie?: %s"),LSD.Members[PlayerIndex].dwFlags & XSESSION_MEMBER_FLAGS_ZOMBIE ? TEXT("True") : TEXT("False"));
				}
			}
			else
			{
				debugf(NAME_ScriptLog,TEXT("Failed to read session information with result 0x%08X"),Result);
			}
		}
		debugf(NAME_ScriptLog,TEXT(""));
	}
}

/**
 * Logs the list of players that are registered for voice
 */
void UOnlineSubsystemLive::DumpVoiceRegistration(void)
{
	debugf(NAME_ScriptLog,TEXT("Voice registrants:"));
	// Iterate through local talkers
	for (INT Index = 0; Index < 4; Index++)
	{
		if (LocalTalkers[Index].bHasVoice)
		{
			XUID Xuid;
			XUserGetXUID(Index,&Xuid);
			debugf(NAME_ScriptLog,TEXT("    Player: 0x%016I64X"),Xuid);
			debugf(NAME_ScriptLog,TEXT("        Type: Local"));
			debugf(NAME_ScriptLog,TEXT("        bHasVoice: %s"),LocalTalkers[Index].bHasVoice ? TEXT("True") : TEXT("False"));
			debugf(NAME_ScriptLog,TEXT("        bHasNetworkedVoice: %s"),LocalTalkers[Index].bHasNetworkedVoice ? TEXT("True") : TEXT("False"));
		}
	}
	// Iterate through the remote talkers listing them
	for (INT Index = 0; Index < RemoteTalkers.Num(); Index++)
	{
		FLiveRemoteTalker& Talker = RemoteTalkers(Index);
		debugf(NAME_ScriptLog,TEXT("    Player: 0x%016I64X"),Talker.TalkerId.Uid);
		debugf(NAME_ScriptLog,TEXT("        Type: Remote"));
		debugf(NAME_ScriptLog,TEXT("        bHasVoice: True"));
		debugf(NAME_ScriptLog,TEXT("        IsLocallyMuted: %s"),Talker.IsLocallyMuted() ? TEXT("True") : TEXT("False"));
		// Log out who has muted this player
		if (Talker.IsLocallyMuted())
		{
			for (INT Index = 0; Index < 4; Index++)
			{
				if (Talker.LocalPriorities[Index].CurrentPriority == XHV_PLAYBACK_PRIORITY_NEVER)
				{
					debugf(NAME_ScriptLog,TEXT("          MutedBy: %d"),Index);
				}
			}
		}
	}
	debugf(NAME_ScriptLog,TEXT(""));
}

/**
 * Sets the debug output level for the platform specific API (if applicable)
 *
 * @param DebugSpewLevel the level to set
 */
void UOnlineSubsystemLive::SetDebugSpewLevel(INT DebugSpewLevel)
{
#if _DEBUG
	debugf(NAME_DevOnline,TEXT("Setting debug spew to %d"),DebugSpewLevel);
	XDebugSetSystemOutputLevel(HXAMAPP_XGI,DebugSpewLevel);
#endif
}

#endif	//#if WITH_UE3_NETWORKING
