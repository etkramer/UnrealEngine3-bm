/**
 * UTUI_DataStores.cpp: Implementation file for all UT3 datastore classes.
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
#include "UTGame.h"
#include "UTGameUIClasses.h"
#include "UTGameUIFrontEndClasses.h"

#include "DownloadableContent.h"

IMPLEMENT_CLASS(UUTUIDataProvider_DemoFile);
IMPLEMENT_CLASS(UUTUIDataProvider_MainMenuItems);
IMPLEMENT_CLASS(UUTUIDataProvider_KeyBinding);
IMPLEMENT_CLASS(UUTUIDataProvider_MultiplayerMenuItem);
IMPLEMENT_CLASS(UUTUIDataProvider_CommunityMenuItem);
IMPLEMENT_CLASS(UUTUIDataProvider_SettingsMenuItem);
IMPLEMENT_CLASS(UUTUIDataProvider_Weapon);
IMPLEMENT_CLASS(UUTOfficialContent);

//////////////////////////////////////////////////////////////////////////
// UUTUIDataStore_StringAliasMap
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataStore_StringAliasMap);

/**
 * Set MappedString to be the localized string using the FieldName as a key
 * Returns the index into the mapped string array of where it was found.
 */
INT UUTUIDataStore_StringAliasMap::GetStringWithFieldName( const FString& FieldName, FString& MappedString )
{
	INT FieldIdx = INDEX_NONE;
	FString FinalFieldName = FieldName;

#if PS3
	// Swap accept and cancel on PS3 if we need to, this is a TRC requirement.
	if(appPS3UseCircleToAccept())
	{
		if(FinalFieldName==TEXT("Accept"))
		{
			FinalFieldName=TEXT("Cancel");
		}
		else if(FinalFieldName==TEXT("Cancel"))
		{
			FinalFieldName=TEXT("Accept");
		}
	}
#endif

	// Try to find platform specific versions first
	FString SetName;
#if XBOX
	SetName = TEXT("360");
#elif PS3
	SetName = TEXT("PS3");
#else
	switch ( FakePlatform )
	{
		case 1: SetName = TEXT("360"); break;
		case 2: SetName = TEXT("PS3"); break;
		default: SetName = TEXT("PC"); break;
	}
#endif
	FieldIdx = FindMappingWithFieldName(FinalFieldName, SetName);

	if(FieldIdx == INDEX_NONE)
	{
		FieldIdx = FindMappingWithFieldName(FinalFieldName);
	}

	if(FieldIdx == INDEX_NONE)
	{
		FieldIdx = FindMappingWithFieldName();
	}

	if(FieldIdx != INDEX_NONE)
	{
		MappedString = MenuInputMapArray(FieldIdx).MappedText;
	}

	return FieldIdx;
}

//////////////////////////////////////////////////////////////////////////
// UUTUIDataStore_StringAliasBindingsMap
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataStore_StringAliasBindingsMap);

//Clear the command to input keybinding cache
void UUTUIDataStore_StringAliasBindingsMap::ClearBoundKeyCache()
{
	CommandToBindNames.Empty();
}

// Given an input command of the form GBA_ return the mapped keybinding string 
// Returns TRUE if it exists, FALSE otherwise
UBOOL UUTUIDataStore_StringAliasBindingsMap::FindMappingInBoundKeyCache(const FString& Command, FString& MappingStr, INT& FieldIndex)
{
	const FName Key(*Command);
	// Does the data already exist
	const FBindCacheElement* CacheElement = CommandToBindNames.Find(Key);
	if (CacheElement != NULL)
	{
		MappingStr = CacheElement->MappingString;
		FieldIndex = CacheElement->FieldIndex;
	}

	return (CacheElement != NULL);
}

//Given a input command of the form GBA_ and its mapping store that in a lookup for future use
void UUTUIDataStore_StringAliasBindingsMap::AddMappingToBoundKeyCache(const FString& Command, const FString& MappingStr, const INT FieldIndex)
{
	const FName Key(*Command);

	// Does the data already exist
	const FBindCacheElement* CacheElement = CommandToBindNames.Find(Key);

	if (CacheElement == NULL)
	{
		// Initialize a new FBindCacheElement.  It contains a FStringNoInit, so it needs to be initialized to zero.
		FBindCacheElement NewElement;
		appMemzero(&NewElement,sizeof(NewElement));

		NewElement.KeyName = Key;
		NewElement.MappingString = MappingStr;
		NewElement.FieldIndex = FieldIndex;
		CommandToBindNames.Set(Key, NewElement);
	}
}

/**
* Set MappedString to be the localized string using the FieldName as a key
* Returns the index into the mapped string array of where it was found.
*/
INT UUTUIDataStore_StringAliasBindingsMap::GetStringWithFieldName( const FString& FieldName, FString& MappedString )
{
	INT StartIndex = UCONST_SABM_FIND_FIRST_BIND;
	INT FieldIndex = INDEX_NONE;

	if (FindMappingInBoundKeyCache(FieldName, MappedString, FieldIndex) == FALSE)
	{
		FieldIndex = GetBoundStringWithFieldName( FieldName, MappedString, &StartIndex );
		AddMappingToBoundKeyCache(FieldName, MappedString, FieldIndex);
	}

	return FieldIndex;
}

/**
 * Called by GetStringWithFieldName() to retreive the string using the input binding system.
 */
INT UUTUIDataStore_StringAliasBindingsMap::GetBoundStringWithFieldName( const FString& FieldName, FString& MappedString, INT* StartIndex/*=NULL*/, FString* BindString/*=NULL*/ )
{
	// String to set MappedString to
	FString LocalizedString = TEXT(" ");

	// Get the index in the MenuInputMapArray using FieldName as the key.
	INT FieldIdx = INDEX_NONE;
	FName KeyName = FName(*FieldName);
	for ( INT Idx = 0; Idx < MenuInputMapArray.Num(); Idx++ )
	{
		if ( KeyName == MenuInputMapArray(Idx).FieldName )
		{
			// Found it
			FieldIdx = Idx;
			break;
		}
	}

	// If we found the entry in our array find the binding and map it to a localized string.
	if ( FieldIdx != INDEX_NONE )
	{
		// Determine how the localized string will need to be mapped.
		// 0 = PC
		// 1 = XBox360
		// 2 = PS3
		INT Platform;

		// FIXME TEMP - for PC development of 360 controls
		Platform = 1;

#if XBOX
		Platform = 1;
#elif PS3
		Platform = 2;
#else
		switch ( FakePlatform )
		{
			case 1: Platform = 1; break;
			case 2: Platform = 2; break;
			default: Platform = 0; break;
		}
#endif

		// Get the player controller.
		ULocalPlayer* LP = GetPlayerOwner();
		AUTPlayerController* UTPC = NULL;
		if ( LP )
		{
			UTPC = Cast<AUTPlayerController>(LP->Actor);
		}

		FString NameSearch = TEXT("");
		INT BindIndex = -1;

		if ( UTPC )
		{
			// Get the bind using the mapped FieldName as the key
			FString KeyCommand = MenuInputMapArray(FieldIdx).FieldName.ToString();
			if ( KeyCommand.Len() > 0 )
			{
				UUTPlayerInput* UTInput = Cast<UUTPlayerInput>(UTPC->PlayerInput);
				if ( UTInput )
				{
					if ( StartIndex && *StartIndex == UCONST_SABM_FIND_FIRST_BIND )
					{
						// Get the game logic specific bind based from the command.
						KeyCommand = UTInput->GetUTBindNameFromCommand( *KeyCommand );
					}
					else
					{
						// Get the bind starting from the back at index StartIndex.
						KeyCommand = UTInput->GetBindNameFromCommand( *KeyCommand, StartIndex );

						// Don't allow controller binds to be shown on PC.
						if ( Platform == 0 )
						{
							while( KeyCommand.StartsWith(TEXT("XBoxTypeS")) && (StartIndex && *StartIndex > -1) )
							{
								(*StartIndex)--;
								KeyCommand = UTInput->GetBindNameFromCommand( *KeyCommand, StartIndex );
							}
						}
					}

					// Set the bind string to the string we found.
					if ( BindString )
					{
						*BindString = KeyCommand;
					}

					// If this is a controller string we have to check the ControllerMapArray for the localized text.
					if ( KeyCommand.StartsWith(TEXT("XBoxTypeS")) )
					{
						// Prefix the mapping with the localized string variable prefix.
						FString SubString = FString::Printf(TEXT("GMS_%s"),*KeyCommand);

						// If this is the Xbox360 or PS3 map it to the localized button strings.
						if ( Platform > 0 )
						{
							FName CommandName = FName(*SubString);
							for ( INT Idx = 0; Idx < ControllerMapArray.Num(); Idx++ )
							{
								if ( CommandName == ControllerMapArray(Idx).KeyName )
								{
									// Found it, now set the correct mapping.
									if ( Platform == 1 )
									{
										SubString = ControllerMapArray(Idx).XBoxMapping;
									}
									else
									{
										SubString = ControllerMapArray(Idx).PS3Mapping;
									}

									// Try and localize it using the ButtonFont section.
									LocalizedString = Localize( TEXT("ButtonFont"), *SubString, TEXT("UTGameUI") );
									break;
								}
							}
						}
						else
						{
							// Try and localize it using the GameMappedStrings section.
							LocalizedString = Localize( TEXT("GameMappedStrings"), *SubString, TEXT("UTGameUI") );
						}
					}
					else
					{
						// Could not find a mapping... if this happens the game is trying to draw the string for a bind that
						// it didn't ensure would exist.
						if ( KeyCommand.Len() <= 0 )
						{
							LocalizedString = TEXT("");
						}
						// Found a bind.
						else
						{
							// Prefix the mapping with the localized string variable prefix.
							FString SubString = FString::Printf(TEXT("GMS_%s"),*KeyCommand);
							// Try and localize it using the GameMappedStrings section.
							LocalizedString = Localize( TEXT("GameMappedStrings"), *SubString, TEXT("UTGameUI") );
						}
					}
				}
			}
		}
	}

	// Set the localized string and return the index.
	MappedString = LocalizedString;
	return FieldIdx;
}

//////////////////////////////////////////////////////////////////////////
// UUTUIResourceDataProvider
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIResourceDataProvider);

/** @return Whether or not we should be filtered out of the list results. */
UBOOL UUTUIResourceDataProvider::IsFiltered()
{
#if !CONSOLE
	return bRemoveOnPC;
#elif XBOX
	return bRemoveOn360;
#elif PS3
	return bRemoveOnPS3;
#else
#error Please define your platform.
#endif
}

//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_OnlineFriends
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_OnlineFriends);

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
 *							do not provide unique UIListElement objects for each element.
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
 *							to a data collection.
 */
UBOOL UUTUIDataProvider_OnlineFriends::GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	UBOOL bResult = FALSE;
	FString TrueMarkup = TEXT("<Strings:UTGAMEUI.Icons.True>");
	FString FalseMarkup = TEXT("<Strings:UTGAMEUI.Icons.False>");

	if (CellTag == FName(TEXT("bIsOnline")))
	{
		out_FieldValue.StringValue = FriendsList(ListIndex).bIsOnline ? TrueMarkup : FalseMarkup;
		bResult=TRUE;
	}
	else if (CellTag == FName(TEXT("bIsPlaying")))
	{
		out_FieldValue.StringValue = FriendsList(ListIndex).bIsPlaying ? TrueMarkup : FalseMarkup;
		bResult=TRUE;
	}
	else if (CellTag == FName(TEXT("bIsPlayingThisGame")))
	{
		out_FieldValue.StringValue = FriendsList(ListIndex).bIsPlayingThisGame ? TrueMarkup : FalseMarkup;
		bResult=TRUE;
	}
	else if (CellTag == FName(TEXT("bIsJoinable")))
	{
		out_FieldValue.StringValue = FriendsList(ListIndex).bIsJoinable ? TrueMarkup : FalseMarkup;
		bResult=TRUE;
	}
	else if (CellTag == FName(TEXT("bHasVoiceSupport")))
	{
		out_FieldValue.StringValue = FriendsList(ListIndex).bHasVoiceSupport ? TrueMarkup : FalseMarkup;
		bResult=TRUE;
	}

	if(bResult==FALSE)
	{
		bResult = Super::GetCellFieldValue(FieldName, CellTag, ListIndex, out_FieldValue, ArrayIndex);
	}
	else
	{
		out_FieldValue.PropertyTag=CellTag;
		out_FieldValue.PropertyType=DATATYPE_Property;
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_OnlineFriendMessages
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_OnlineFriendMessages);


/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
 *							do not provide unique UIListElement objects for each element.
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
 *							to a data collection.
 */
UBOOL UUTUIDataProvider_OnlineFriendMessages::GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	UBOOL bResult = FALSE;
	FString TrueMarkup = TEXT("<Strings:UTGAMEUI.Icons.True>");
	FString FalseMarkup = TEXT("<Strings:UTGAMEUI.Icons.False>");

	if (CellTag == FName(TEXT("bIsFriendInvite")))
	{
		out_FieldValue.StringValue = Messages(ListIndex).bIsFriendInvite ? TrueMarkup : FalseMarkup;
		bResult=TRUE;
	}

	if(bResult==FALSE)
	{
		bResult = Super::GetCellFieldValue(FieldName, CellTag, ListIndex, out_FieldValue, ArrayIndex);
	}
	else
	{
		out_FieldValue.PropertyTag=CellTag;
		out_FieldValue.PropertyType=DATATYPE_Property;
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_SearchResult
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_SearchResult);
/**
 * @return	TRUE if server corresponding to this search result is a pure server.
 */
UBOOL UUTUIDataProvider_SearchResult::IsPureServer()
{
	INT SettingValueIndex = 0;
	if ( Settings != NULL )
	{
		static FName PureServerSettingName = FName(TEXT("PureServer"));
		verify(Settings->GetStringSettingValueByName(PureServerSettingName, SettingValueIndex));
	}
	return SettingValueIndex == 1;
}

/**
 * @return	TRUE if server corresponding to this search result is password protected.
 */
UBOOL UUTUIDataProvider_SearchResult::IsPrivateServer()
{
	INT SettingValueIndex = 0;
	if ( Settings != NULL )
	{
		static FName PrivateServerSettingName = FName(TEXT("LockedServer"));
		verify(Settings->GetStringSettingValueByName(PrivateServerSettingName, SettingValueIndex));
	}
	return SettingValueIndex == 1;
}

/**
 * @return	TRUE if server corresponding to this search result allows players to use keyboard & mouse.
 */
UBOOL UUTUIDataProvider_SearchResult::AllowsKeyboardMouse()
{
	INT SettingValueIndex = 0;
	if ( Settings != NULL )
	{
		static FName KMAllowedSettingName = FName(TEXT("AllowKeyboard"));
		verify(Settings->GetStringSettingValueByName(KMAllowedSettingName, SettingValueIndex));
	}
	return SettingValueIndex == 1;
}

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUTUIDataProvider_SearchResult::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& out_FieldValue,INT ArrayIndex)
{
	UBOOL bResult=FALSE;

	const FName FieldFName(*FieldName);
	if ( FieldFName == ServerFlagsTag )
	{
		out_FieldValue.PropertyTag = PlayerRatioTag;
		out_FieldValue.PropertyType = DATATYPE_Property;

		// since we always want this field value to be displayed using a certain font, use markup to ensure that font is used
		// this way we can just use the same style as the rest of the list or string
		out_FieldValue.StringValue = US + TEXT("<Fonts:") + IconFontPathName + TEXT(">");

		if ( IsPureServer() || !GIsGame )
		{
			out_FieldValue.StringValue += TEXT("0");
		}

		if ( IsPrivateServer() || !GIsGame )
		{
			out_FieldValue.StringValue += TEXT("7");
		}

		if ( AllowsKeyboardMouse() || !GIsGame )
		{
			out_FieldValue.StringValue += TEXT("6");
		}

		// closing data store markup
		out_FieldValue.StringValue += TEXT("<Fonts:/>");

		bResult = TRUE;
	}
	else if ( FieldFName == PlayerRatioTag )
	{
		FUIProviderFieldValue TotalValue(EC_EventParm);
		FUIProviderFieldValue OpenValue(EC_EventParm);
		if (Super::GetFieldValue(TEXT("NumPublicConnections"), TotalValue, ArrayIndex)
		&&	Super::GetFieldValue(TEXT("NumOpenPublicConnections"), OpenValue, ArrayIndex))
		{
			INT OpenNum = appAtoi(*OpenValue.StringValue);
			INT TotalNum = appAtoi(*TotalValue.StringValue);

			out_FieldValue.PropertyTag = PlayerRatioTag;
			out_FieldValue.StringValue = FString::Printf(TEXT("%i/%i"), TotalNum-OpenNum, TotalNum);
			bResult = TRUE;
		}
	}
	else if ( FieldFName == GameModeFriendlyNameTag || FieldFName == TEXT("CustomGameMode") )
	{
		FUIProviderFieldValue GameClassName(EC_EventParm);

		if ( Super::GetFieldValue(TEXT("CustomGameMode"), GameClassName, ArrayIndex) )
		{
			// Try to get the localized game name out of the localization file.
			FString LocalizedGameName;
			LocalizedGameName = GameClassName.StringValue;
			
			// Replace _Content class name with normal class name.
			if(LocalizedGameName.ReplaceInline(TEXT("UTGameContent."),TEXT("UTGame.")))
			{
				LocalizedGameName.ReplaceInline(TEXT("_Content"),TEXT(""));
			}

			INT PeriodPos = LocalizedGameName.InStr(TEXT("."));
			if(PeriodPos > 0 && PeriodPos < LocalizedGameName.Len())
			{	
				FString GamePackageName = LocalizedGameName.Left(PeriodPos);
				FString GameModeName = LocalizedGameName.Mid(PeriodPos + 1);
				FString LocalizedResult = Localize(*GameModeName, TEXT("GameName"), *GamePackageName, NULL, TRUE);
				if ( LocalizedResult.Len() > 0 )
				{
					LocalizedGameName = LocalizedResult;
				}
				else
				{
					LocalizedGameName = GameModeName;
				}
			}

			// Return the string field value.
			out_FieldValue.PropertyTag = GameModeFriendlyNameTag;
			out_FieldValue.StringValue = LocalizedGameName;
			bResult = TRUE;
		}
	}
	else if ( FieldFName == MapNameTag )
	{
		FUIProviderFieldValue ActualMapName(EC_EventParm);
		if ( Super::GetFieldValue(FieldName, ActualMapName, ArrayIndex) )
		{
			const FName MapTag(TEXT("Maps"));

			// same thing for the map
			// for maps however, we'll need to look up this map in the menu items (game resource) data store
			// for the friendly name because there isn't any loc section to look in for a map (no class)
			UDataStoreClient* DSClient = UUIInteraction::GetDataStoreClient();
			if ( DSClient != NULL )
			{
				UUTUIDataStore_MenuItems* ResourceDataStore = Cast<UUTUIDataStore_MenuItems>(DSClient->FindDataStore(TEXT("UTMenuItems")));
				if ( ResourceDataStore != NULL )
				{
					INT ProviderIndex = ResourceDataStore->FindValueInProviderSet(MapTag, TEXT("MapName"), ActualMapName.StringValue);
					if ( ProviderIndex != INDEX_NONE )
					{
						if ( ResourceDataStore->GetValueFromProviderSet(MapTag, TEXT("FriendlyName"), ProviderIndex, out_FieldValue.StringValue) )
						{
							out_FieldValue.PropertyTag = MapNameTag;
							out_FieldValue.PropertyType = DATATYPE_Property;
							bResult = TRUE;
						}
					}
				}
			}

			if ( !bResult && ActualMapName.StringValue.Len() > 0 )
			{
				out_FieldValue = ActualMapName;
				out_FieldValue.PropertyTag = MapNameTag;
				bResult = TRUE;
			}
		}
	}
	else if ( FieldName == TEXT("OwningPlayerName") )
	{
		FUIProviderFieldValue PlayerName(EC_EventParm);
		FString FinalName;

		// Get the player's name first
		if(Super::GetFieldValue(TEXT("OwningPlayerName"), PlayerName, ArrayIndex))
		{
			FinalName = PlayerName.StringValue;

			// See if we should append a server description.
			FUIProviderFieldValue ServerDescription(EC_EventParm);
			if(Super::GetFieldValue(TEXT("ServerDescription"), ServerDescription, ArrayIndex))
			{
				FString DescStr = ServerDescription.StringValue.Trim().TrimTrailing();
				if(DescStr.Len())
				{
					FinalName += TEXT(": ");
					FinalName += DescStr;
				}
			}

			out_FieldValue.PropertyTag = *FieldName;
			out_FieldValue.StringValue = FinalName;
			bResult = TRUE;
		}
	}
	else
	{
		bResult = Super::GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
	}

	return bResult;
}

/**
 * Builds a list of available fields from the array of properties in the
 * game settings object
 *
 * @param OutFields	out value that receives the list of exposed properties
 */
void UUTUIDataProvider_SearchResult::GetSupportedDataFields(TArray<FUIDataProviderField>& OutFields)
{
	Super::GetSupportedDataFields(OutFields);

	OutFields.AddItem(FUIDataProviderField(TEXT("PlayerRatio")));
	OutFields.AddItem(FUIDataProviderField(TEXT("GameModeFriendlyName")));

	// this field displays icons indicating whether the server is pure, locked, allows keyboard/mouse
	new(OutFields) FUIDataProviderField(TEXT("ServerFlags"));
}

/**
 * Gets the list of data fields (and their localized friendly name) for the fields exposed this provider.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	out_CellTags	receives the name/friendly name pairs for all data fields in this provider.
 */
void UUTUIDataProvider_SearchResult::GetElementCellTags( FName FieldName, TMap<FName,FString>& out_CellTags )
{
	Super::GetElementCellTags(FieldName, out_CellTags);

	if ( FieldName == UCONST_UnknownCellDataFieldName )
	{
		const FString SectionName = GetClass()->GetName();

		out_CellTags.Set( TEXT("PlayerRatio"), *Localize(*SectionName, TEXT("PlayerRatio"), TEXT("UTGameUI")) );
		out_CellTags.Set( TEXT("GameModeFriendlyName"), *Localize(*SectionName, TEXT("GameModeFriendlyName"), TEXT("UTGameUI")) );
		out_CellTags.Set( TEXT("ServerFlags"), *Localize(*SectionName, TEXT("ServerFlags"), TEXT("UTGameUI")) );
	}
}


//////////////////////////////////////////////////////////////////////////
// UUTDataStore_GameSearchBase
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTDataStore_GameSearchBase);

/**
 * Loads and creates an instance of the registered filter object
 */
void UUTDataStore_GameSearchBase::InitializeDataStore(void)
{
	Super::InitializeDataStore();

	// Create server details object
	ServerDetailsProvider = ConstructObject<UUTUIDataProvider_ServerDetails>(UUTUIDataProvider_ServerDetails::StaticClass(), this);
}

/**
 * Retrieves the list of currently enabled mutators.
 *
 * @param	MutatorIndices	indices are from the list of UTUIDataProvider_Mutator data providers in the
 *							UTUIDataStore_MenuItems data store which are currently enabled.
 *
 * @return	TRUE if the list of enabled mutators was successfully retrieved.
 */
UBOOL UUTDataStore_GameSearchBase::GetEnabledMutators( TArray<INT>& MutatorIndices )
{
	UBOOL bResult = FALSE;

	MutatorIndices.Empty();

	if ( ServerDetailsProvider != NULL )
	{
		UUIDataProvider_Settings* SearchResults = ServerDetailsProvider->GetSearchResultsProvider();
		if ( SearchResults != NULL )
		{
			// get the bitmask of enabled official mutators
			FUIProviderFieldValue MutatorValue(EC_EventParm);
			if ( SearchResults->GetFieldValue(TEXT("OfficialMutators"), MutatorValue) )
			{
				const INT MutatorBitmask = appAtoi(*MutatorValue.StringValue);
				for ( INT BitIdx = 0; BitIdx < sizeof(INT) * 8; BitIdx++ )
				{
					if ( (MutatorBitmask&(1<<BitIdx)) != 0 )
					{
						MutatorIndices.AddItem(BitIdx);
					}
				}

				bResult = TRUE;
			}

			// now get the list of enabled custom mutators
			MutatorValue.StringValue = TEXT("");
			if ( SearchResults->GetFieldValue(TEXT("CustomMutators"), MutatorValue) && MutatorValue.StringValue.Len() )
			{
				const TCHAR CustomMutatorDelimiter[2] = { 0x1C, 0x00 };
				TArray<FString> CustomMutatorNames;

				const INT MutatorBitmaskBoundary = sizeof(INT) * 8;
				MutatorValue.StringValue.ParseIntoArray(&CustomMutatorNames, CustomMutatorDelimiter, TRUE);
				for ( INT MutIndex = 0; MutIndex < CustomMutatorNames.Num(); MutIndex++ )
				{
					MutatorIndices.AddItem(MutatorBitmaskBoundary + MutIndex);
				}

				bResult = TRUE;
			}

			UDataStoreClient* DSClient = UUIInteraction::GetDataStoreClient();
			if ( DSClient != NULL )
			{
				UUTUIDataStore_MenuItems* ResourceDataStore = Cast<UUTUIDataStore_MenuItems>(DSClient->FindDataStore(TEXT("UTMenuItems")));
				if ( ResourceDataStore != NULL )
				{
					//@todo
					bResult = TRUE;
				}
			}
		}
	}

	return bResult;
}

/**
 * Returns the stats read results as a collection and appends the filter provider
 *
 * @param OutFields	out value that receives the list of exposed properties
 */
void UUTDataStore_GameSearchBase::GetSupportedDataFields(TArray<FUIDataProviderField>& OutFields)
{
	Super::GetSupportedDataFields(OutFields);
	
	// Append the server details provider
	new(OutFields) FUIDataProviderField(FName(TEXT("CurrentServerDetails")),DATATYPE_Provider,ServerDetailsProvider);
	new(OutFields) FUIDataProviderField(TEXT("CurrentServerMutators"), DATATYPE_Collection);
}

/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UUTDataStore_GameSearchBase::GetElementProviderTags()
{
	TArray<FName> Result = Super::GetElementProviderTags();

	Result.AddItem(TEXT("CurrentServerDetails"));
	Result.AddItem(TEXT("CurrentServerMutators"));

	return Result;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UUTDataStore_GameSearchBase::GetElementCount( FName FieldName )
{
	INT Result=0;

	if(FieldName==TEXT("CurrentServerDetails"))
	{
		Result = ServerDetailsProvider->GetElementCount();	
	}
	else if ( FieldName == TEXT("CurrentServerMutators") )
	{
		TArray<INT> Mutators;
		if ( GetEnabledMutators(Mutators) )
		{
			Result = Mutators.Num();
		}
	}
	else
	{
		Result = Super::GetElementCount(FieldName);
	}

	return Result;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UUTDataStore_GameSearchBase::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;
	
	if(FieldName==TEXT("CurrentServerDetails"))
	{
		bResult = ServerDetailsProvider->GetListElements(FieldName, out_Elements);
	}
	else if ( FieldName == TEXT("CurrentServerMutators") )
	{
		bResult = GetEnabledMutators(out_Elements);
	}
	else
	{
		bResult = Super::GetListElements(FieldName, out_Elements);
	}

	return bResult;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	FieldName		the tag of the list element data field that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<class IUIListElementCellProvider> UUTDataStore_GameSearchBase::GetElementCellSchemaProvider( FName FieldName )
{
	if( FieldName==TEXT("CurrentServerDetails") || FieldName == TEXT("CurrentServerMutators") )
	{
		return TScriptInterface<IUIListElementCellProvider>(ServerDetailsProvider);
	}
	else
	{
		return Super::GetElementCellSchemaProvider(FieldName);
	}

	return TScriptInterface<IUIListElementCellProvider>();
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 *
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<class IUIListElementCellProvider> UUTDataStore_GameSearchBase::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	if( FieldName==TEXT("CurrentServerDetails") || FieldName == TEXT("CurrentServerMutators") )
	{
		return TScriptInterface<IUIListElementCellProvider>(ServerDetailsProvider);
	}
	else
	{
		return Super::GetElementCellValueProvider(FieldName, ListIndex);
	}

	return TScriptInterface<IUIListElementCellProvider>();
}

TScriptInterface<class IUIListElementProvider> UUTDataStore_GameSearchBase::ResolveListElementProvider( const FString& PropertyName ) 
{
	if( PropertyName==TEXT("CurrentServerDetails") || PropertyName == TEXT("CurrentServerMutators") )
	{
		return TScriptInterface<IUIListElementProvider> (this);
	}
	
	return Super::ResolveListElementProvider(PropertyName);
}

//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_ServerDetails
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_ServerDetails);

/** @return Returns a reference to the search results provider that is used to generate data for this class. */
UUIDataProvider_Settings* UUTUIDataProvider_ServerDetails::GetSearchResultsProvider()
{
	UUIDataProvider_Settings* ServerDetailsProvider = NULL;

	UUIDataStore_OnlineGameSearch* GameSearchDataStore = Cast<UUIDataStore_OnlineGameSearch>(GetOuter());
	if ( GameSearchDataStore == NULL )
	{
		// Find the game search datastore
		UDataStoreClient* DSClient = UUIInteraction::GetDataStoreClient();
		if( DSClient != NULL )
		{
			GameSearchDataStore = Cast<UUIDataStore_OnlineGameSearch>(DSClient->FindDataStore(TEXT("UTGameSearch")));
		}
	}

	if (GameSearchDataStore != NULL
	&&	GameSearchDataStore->GameSearchCfgList.IsValidIndex(GameSearchDataStore->SelectedIndex)
	&&	GameSearchDataStore->GameSearchCfgList(GameSearchDataStore->SelectedIndex).SearchResults.IsValidIndex(SearchResultsRow))
	{
		// Get the current server details given our search results row
		ServerDetailsProvider = GameSearchDataStore->GameSearchCfgList(GameSearchDataStore->SelectedIndex).SearchResults(SearchResultsRow);
	}

	return ServerDetailsProvider;
}

/**
 * Determines whether the specified field should be included when the user requests to see a list of this server's details.
 */
UBOOL UUTUIDataProvider_ServerDetails::ShouldDisplayField( FName FieldName )
{
	static TLookupMap<FName> FieldsToHide;
	if ( FieldsToHide.Num() == 0 )
	{
		FieldsToHide.AddItem(FName(TEXT("MaxPlayers"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("MinNetPlayers"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("bShouldAdvertise"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("bIsLanMatch"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("bAllowJoinInProgress"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("bAllowInvites"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("bUsesPresence"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("bUsesStats"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("bAllowJoinViaPresence"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("bUsesArbitration"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("bIsListPlay"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("Campaign"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("CustomMapName"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("CustomGameMode"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("CustomMutators"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("GameMode"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("ServerDescription"),FNAME_Find));

		// these are the fields that are shown in the main server browser list
		FieldsToHide.AddItem(FName(TEXT("ServerFlags")));
		FieldsToHide.AddItem(FName(TEXT("OwningPlayerName"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("ServerFlags")));
		FieldsToHide.AddItem(FName(TEXT("PingInMs"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("OfficialMutators"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("PlayerRatio"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("NumOpenPrivateConnections"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("NumOpenPublicConnections"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("NumPublicConnections"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("NumPrivateConnections"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("MapName"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("PureServer"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("LockedServer"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("IsFullServer"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("IsEmptyServer"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("AllowKeyboard"),FNAME_Find));
		FieldsToHide.AddItem(FName(TEXT("IsDedicated"),FNAME_Find));
	}

	return !FieldsToHide.HasKey(FieldName);
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UUTUIDataProvider_ServerDetails::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	if ( FieldName == TEXT("CurrentServerDetails") )
	{
		UUIDataProvider_Settings* ServerDetailsProvider = GetSearchResultsProvider();
		if(ServerDetailsProvider != NULL)
		{
			TArray<FUIDataProviderField> SupportedFields;
			ServerDetailsProvider->GetSupportedDataFields(SupportedFields);

			for ( INT FieldIndex = 0; FieldIndex < SupportedFields.Num(); FieldIndex++ )
			{
				if ( ShouldDisplayField(SupportedFields(FieldIndex).FieldTag) )
				{
					out_Elements.AddItem(FieldIndex);
				}
			}
		}
		bResult = TRUE;
	}

	return bResult;
}

/** Returns the number of elements in the list. */
INT UUTUIDataProvider_ServerDetails::GetElementCount()
{
	INT Result = 0;

	UUIDataProvider_Settings* ServerDetailsProvider = GetSearchResultsProvider();
	if(ServerDetailsProvider != NULL)
	{
		TArray<FUIDataProviderField> SupportedFields;
		ServerDetailsProvider->GetSupportedDataFields(SupportedFields);

		Result = SupportedFields.Num();
		for ( INT FieldIndex = 0; FieldIndex < SupportedFields.Num(); FieldIndex++ )
		{
			if ( !ShouldDisplayField(SupportedFields(FieldIndex).FieldTag) )
			{
				Result--;
			}
		}
	}
	
	return Result;
}

// IUIListElement interface

/**
 * Returns the names of the exposed members in the first entry in the array
 * of search results
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param OutCellTags the columns supported by this row
 */
void UUTUIDataProvider_ServerDetails::GetElementCellTags( FName FieldName, TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Empty();

	if ( FieldName == TEXT("CurrentServerDetails") )
	{
		OutCellTags.Set(TEXT("Key"),*Localize(TEXT("ServerBrowser"), TEXT("Key"), TEXT("UTGameUI")));
		OutCellTags.Set(TEXT("Value"),*Localize(TEXT("ServerBrowser"), TEXT("Value"), TEXT("UTGameUI")));
	}
	else if ( FieldName == TEXT("CurrentServerMutators") )
	{
		OutCellTags.Set(TEXT("CurrentServerMutators"), *Localize(TEXT("ServerBrowser"), TEXT("CurrentServerMutators"), TEXT("UTGameUI")));
	}
	else
	{
		Super::GetElementCellTags(FieldName, OutCellTags);
	}
}


/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the index of the value to fetch
 * @param	OutFieldValue	receives the resolved value for the property specified.
 * @param	ArrayIndex		ignored
 */
UBOOL UUTUIDataProvider_ServerDetails::GetCellFieldValue(FName FieldName, const FName& CellTag,INT ListIndex,FUIProviderFieldValue& OutFieldValue,INT)
{
	UBOOL bResult = FALSE;
	UUIDataProvider_Settings* ServerDetailsProvider = GetSearchResultsProvider();

	if(ServerDetailsProvider != NULL)
	{
		if ( CellTag == TEXT("CurrentServerMutators") )
		{
			UDataStoreClient* DSClient = UUIInteraction::GetDataStoreClient();
			if ( DSClient != NULL )
			{
				UUTUIDataStore_MenuItems* ResourceDataStore = Cast<UUTUIDataStore_MenuItems>(DSClient->FindDataStore(TEXT("UTMenuItems")));
				if ( ResourceDataStore != NULL )
				{
					TArray<UUIResourceDataProvider*> Providers;
					ResourceDataStore->ListElementProviders.MultiFind(TEXT("Mutators"), Providers);

					// we build the list of mutators manually so that we don't filter out mutators which aren't compatible with the selected gametype
					// because here, we want a mutator which should be at a specific location in the array.
					TArray<UUTUIDataProvider_Mutator*> AvailableMutators;
					for ( INT ProviderIndex = 0; ProviderIndex < Providers.Num(); ProviderIndex++ )
					{
						UUTUIDataProvider_Mutator* DataProvider = Cast<UUTUIDataProvider_Mutator>(Providers(ProviderIndex));
						if ( DataProvider && DataProvider->bOfficialMutator )
						{
							AvailableMutators.AddUniqueItem(DataProvider);
						}
					}

					if ( AvailableMutators.IsValidIndex(ListIndex) )
					{
						OutFieldValue.PropertyTag = CellTag;
						OutFieldValue.PropertyType = DATATYPE_Property;
						OutFieldValue.StringValue = AvailableMutators(ListIndex)->FriendlyName;
						bResult = TRUE;
					}
					else if ( ListIndex >= 0 )
					{
						FUIProviderFieldValue MutatorValue(EC_EventParm);
						if ( ServerDetailsProvider->GetFieldValue(TEXT("CustomMutators"), MutatorValue) && MutatorValue.StringValue.Len() )
						{
							const TCHAR CustomMutatorDelimiter[2] = { 0x1C, 0x00 };
							TArray<FString> CustomMutatorNames;

							const INT MutatorBitmaskBoundary = sizeof(INT) * 8;
							const INT CustomMutatorIndex = ListIndex - MutatorBitmaskBoundary;
							MutatorValue.StringValue.ParseIntoArray(&CustomMutatorNames, CustomMutatorDelimiter, TRUE);
							if ( CustomMutatorNames.IsValidIndex(CustomMutatorIndex) )
							{
								OutFieldValue.PropertyTag = CellTag;
								OutFieldValue.PropertyType = DATATYPE_Property;
								OutFieldValue.StringValue = CustomMutatorNames(CustomMutatorIndex);
								bResult = TRUE;
							}
						}
					}
				}
			}
		}
		else
		{
			// Use the supported fields of the datastore for our row data.
			TMap<FName,FString> SupportedFields;
			ServerDetailsProvider->GetElementCellTags(UCONST_UnknownCellDataFieldName, SupportedFields);

			TMap<FName,FString>::TIterator Itor(SupportedFields, FALSE, ListIndex);
			if ( Itor )
			{
				if ( CellTag==TEXT("Key") )
				{
					OutFieldValue.PropertyTag = CellTag;
					OutFieldValue.PropertyType = DATATYPE_Property;

					// set the value to the localized friendly name for this field
					OutFieldValue.StringValue = Itor.Value();
					// but if it doesn't have one, just use the name of the field itself
					if ( OutFieldValue.StringValue.Len() == 0 )
					{
						OutFieldValue.StringValue = Itor.Key().ToString();
					}
					bResult = TRUE;
				}
				else if ( CellTag==TEXT("Value") )
				{
					bResult = ServerDetailsProvider->GetFieldValue(Itor.Key().ToString(), OutFieldValue);
				}
			}
			else
			{
				// if an invalid ListIndex was specified, it means that one of our fields was requested by name ("NumPlayers" instead of e.g. "Key[3]"),
				// so search for the named field and return just the value.
				if ( SupportedFields.HasKey(CellTag) )
				{
					OutFieldValue.PropertyTag = CellTag;
					OutFieldValue.PropertyType = DATATYPE_Property;
					bResult = ServerDetailsProvider->GetFieldValue(CellTag.ToString(), OutFieldValue);
				}
			}
		}
	}

	return bResult;
}

void UUTUIDataProvider_MapInfo::AddChildProviders(TArray<UUTUIResourceDataProvider*>& Providers)
{
	if (MapName != TEXT(""))
	{
		TArray<FString> SectionNames;
		GConfig->GetPerObjectConfigSections(*(appGameConfigDir() + MapName + TEXT(".ini")), TEXT("UTCustomLinkSetup"), SectionNames);
		for (INT i = 0; i < SectionNames.Num(); i++)
		{
			FString CombinedName = MapName + TEXT("_LinkSetup_") + SectionNames(i);
			UUTUIDataProvider_MapInfo* NewProvider = Cast<UUTUIDataProvider_MapInfo>(StaticFindObject(GetClass(), ANY_PACKAGE, *CombinedName, TRUE));
			if (NewProvider == NULL)
			{
				NewProvider = Cast<UUTUIDataProvider_MapInfo>(StaticDuplicateObject(this, this, GetOuter(), *CombinedName));
				if (NewProvider != NULL)
				{
					// append link setup name to map and friendly name
					FString LinkSetupName = SectionNames(i).Left(SectionNames(i).InStr(TEXT(" ")));
					NewProvider->FriendlyName += FString::Printf(TEXT(" (%s)"), *LinkSetupName);
					NewProvider->MapName += FString::Printf(TEXT("?LinkSetup=%s"), *LinkSetupName);
				}
			}

			if (NewProvider != NULL)
			{
				Providers.AddItem(NewProvider);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// UUTUIDataStore_MenuItems
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataStore_MenuItems);

void UUTUIDataStore_MenuItems::GetAllResourceDataProviders(UClass* ProviderClass, TArray<UUTUIResourceDataProvider*>& Providers)
{
	checkSlow(ProviderClass->IsChildOf(UUTUIResourceDataProvider::StaticClass()));

	// Get a list of ini files from disk.
	TArray<FString> ConfigFileNamesFromDisk;
	GFileManager->FindFiles(ConfigFileNamesFromDisk, *(appGameConfigDir() + TEXT("*.ini")), TRUE, FALSE);

	// Get a lsit of ini files in memory.  Things like e.g. DLC that were merged at startup
	// will be in memory but not in the config dir.
	TArray<FFilename> ConfigFileNamesInMemory;
	GConfig->GetConfigFilenames( ConfigFileNamesInMemory );

	// Merge the two lists so that proivders decalred in inis in both lists don't get double-added to the menus.
	TArray<FString> ConfigFileNames;
	for (INT i = 0; i < ConfigFileNamesFromDisk.Num(); i++)	
	{
		ConfigFileNames.AddUniqueItem(ConfigFileNamesFromDisk(i));
	}
	for (INT i = 0; i < ConfigFileNamesInMemory.Num(); i++)	
	{
		// There may be loc files in memory -- we don't want these.
		if ( ConfigFileNamesInMemory(i).GetExtension() == TEXT("ini") )
		{
			ConfigFileNames.AddUniqueItem(ConfigFileNamesInMemory(i).GetCleanFilename());
		}
	}

	for (INT i = 0; i < ConfigFileNames.Num(); i++)
	{
		// ignore default .inis
		if (appStricmp(*ConfigFileNames(i).Left(7), TEXT("Default")) != 0)
		{
			FString FullConfigPath = appGameConfigDir() + ConfigFileNames(i);
			TArray<FString> GameTypeResourceSectionNames;
			if (GConfig->GetPerObjectConfigSections(*FullConfigPath, *ProviderClass->GetName(), GameTypeResourceSectionNames) )
			{
				for (INT SectionIndex = 0; SectionIndex < GameTypeResourceSectionNames.Num(); SectionIndex++)
				{
					INT POCDelimiterPosition = GameTypeResourceSectionNames(SectionIndex).InStr(TEXT(" "));
					// we shouldn't be here if the name was included in the list
					checkSlow(POCDelimiterPosition != INDEX_NONE);

					FName ObjectName = *GameTypeResourceSectionNames(SectionIndex).Left(POCDelimiterPosition);
					if (ObjectName != NAME_None)
					{
						//@note: names must be unique across all .ini files
						UUTUIResourceDataProvider* NewProvider = Cast<UUTUIResourceDataProvider>(StaticFindObject(ProviderClass, ANY_PACKAGE, *ObjectName.ToString(), TRUE));
						if (NewProvider == NULL)
						{
							NewProvider = ConstructObject<UUTUIResourceDataProvider>(ProviderClass, IsTemplate() ? (UObject*)GetTransientPackage() : this, ObjectName);
							if (NewProvider != NULL)
							{
								// load the config and localized values from the current .ini name
								NewProvider->IniName = *FFilename(ConfigFileNames(i)).GetBaseFilename();
								NewProvider->LoadConfig(NULL, *FullConfigPath);
								LoadLocalizedStruct( ProviderClass, *NewProvider->IniName,
													*(ObjectName.ToString() + TEXT(" ") + ProviderClass->GetName()), NULL, NewProvider, (BYTE*)NewProvider );
							}
						}

						if (NewProvider != NULL)
						{
							Providers.AddItem(NewProvider);
							NewProvider->AddChildProviders(Providers);
						}
					}
				}
			}
		}
	}
}

IMPLEMENT_COMPARE_CONSTPOINTER(UUTUIDataProvider_MapInfo,UI_DataStores, {
	INT Result = 0;
	if ( A && B )
	{
		Result = B->bOfficialMap - A->bOfficialMap;
		if ( Result == 0 )
		{
			Result = appStricmp(*A->MapName, *B->MapName);
		}
	}

	return Result;
})

IMPLEMENT_COMPARE_CONSTPOINTER(UUTUIDataProvider_Mutator,UI_DataStores, {
	INT Result = 0;
	if ( A && B )
	{
		Result = B->bOfficialMutator - A->bOfficialMutator;
		if ( Result == 0 )
		{
			Result = appStricmp(*A->ClassName, *B->ClassName);
		}
	}

	return Result;
})

/**
 * Finds or creates the UIResourceDataProvider instances referenced by ElementProviderTypes, and stores the result
 * into the ListElementProvider map.
 */
void UUTUIDataStore_MenuItems::InitializeListElementProviders()
{
	ListElementProviders.Empty();

	// for each configured provider type, retrieve the list of ini sections that contain data for that provider class
	for ( INT ProviderTypeIndex = 0; ProviderTypeIndex < ElementProviderTypes.Num(); ProviderTypeIndex++ )
	{
		FGameResourceDataProvider& ProviderType = ElementProviderTypes(ProviderTypeIndex);

		UClass* ProviderClass = ProviderType.ProviderClass;

#if !CONSOLE
		if (ProviderClass->IsChildOf(UUTUIResourceDataProvider::StaticClass()) && ProviderClass->GetDefaultObject<UUTUIResourceDataProvider>()->bSearchAllInis)
		{
			// search all .ini files for instances to create
			TArray<UUTUIResourceDataProvider*> Providers;
			GetAllResourceDataProviders(ProviderClass, Providers);
			for (INT i = 0; i < Providers.Num(); i++)
			{
				ListElementProviders.Add(ProviderType.ProviderTag, Providers(i));
			}
		}
		else
#endif
		{
			// use default method of only searching the class's .ini file
			TArray<FString> GameTypeResourceSectionNames;
			if ( GConfig->GetPerObjectConfigSections(*ProviderClass->GetConfigName(), *ProviderClass->GetName(), GameTypeResourceSectionNames) )
			{
				for ( INT SectionIndex = 0; SectionIndex < GameTypeResourceSectionNames.Num(); SectionIndex++ )
				{
					INT POCDelimiterPosition = GameTypeResourceSectionNames(SectionIndex).InStr(TEXT(" "));
					// we shouldn't be here if the name was included in the list
					check(POCDelimiterPosition!=INDEX_NONE);

					FName ObjectName = *GameTypeResourceSectionNames(SectionIndex).Left(POCDelimiterPosition);
					if ( ObjectName != NAME_None )
					{
						UUIResourceDataProvider* Provider = Cast<UUIResourceDataProvider>( StaticFindObject(ProviderClass, ANY_PACKAGE, *ObjectName.ToString(), TRUE) );
						if ( Provider == NULL )
						{
							Provider = ConstructObject<UUIResourceDataProvider>(
								ProviderClass,
								this,
								ObjectName
							);
						}

						if ( Provider != NULL )
						{
							ListElementProviders.Add(ProviderType.ProviderTag, Provider);
						}
					}
				}
			}
		}
	}

	// Get a list of demo file names. - @todo: This may need to be generated at runtime when entering the demo playback menu.
	TArray<FString> DemoFileNames;
	FString DemoDir = appGameDir() + TEXT("Demos/");
	GFileManager->FindFiles(DemoFileNames, *(DemoDir+ TEXT("*.demo")), TRUE, FALSE);
	for (INT FileIdx = 0; FileIdx < DemoFileNames.Num(); FileIdx++)
	{
		UUIResourceDataProvider* Provider = ConstructObject<UUIResourceDataProvider>(
			UUTUIDataProvider_DemoFile::StaticClass(),
			this);

		if ( Provider != NULL )
		{
			UUTUIDataProvider_DemoFile* DemoProvider = Cast<UUTUIDataProvider_DemoFile>(Provider);
			DemoProvider->Filename = DemoFileNames(FileIdx);
			ListElementProviders.Add(TEXT("DemoFiles"), Provider);
		}
	}

	// Generate a placeholder entry for the editor.
	if(GIsEditor && !GIsGame)
	{
		UUIResourceDataProvider* Provider = ConstructObject<UUIResourceDataProvider>(
			UUTUIDataProvider_DemoFile::StaticClass(),
			this);

		if ( Provider != NULL )
		{
			UUTUIDataProvider_DemoFile* DemoProvider = Cast<UUTUIDataProvider_DemoFile>(Provider);
			DemoProvider->Filename = TEXT("Placeholder.demo");
			ListElementProviders.Add(TEXT("DemoFiles"), Provider);
		}
	}

	// Generate DropDownWeapons provider set
	TArray<UUTUIResourceDataProvider*> WeaponProviders;
	GetFilteredElementProviders(TEXT("Weapons"), WeaponProviders);

	ListElementProviders.RemoveKey(TEXT("DropDownWeapons"));
	for(INT WeaponIdx=0; WeaponIdx<WeaponProviders.Num(); WeaponIdx++)
	{
		UUTUIDataProvider_Weapon* Provider = Cast<UUTUIDataProvider_Weapon>(WeaponProviders(WeaponIdx));

		if(Provider)
		{
			if(Provider->Flags.InStr(TEXT("HideDropDown"))==INDEX_NONE)
			{
				ListElementProviders.Add(TEXT("DropDownWeapons"), Provider);
			}
		}
	}

	SortRelevantProviders();
}

/**
 * Sorts the list of map and mutator data providers according to whether they're official or not, then alphabetically.
 */
void UUTUIDataStore_MenuItems::SortRelevantProviders()
{

	FName MapsName(TEXT("Maps"));
	// sort the maps
	TArray<UUTUIDataProvider_MapInfo*> MapProviders;
	ListElementProviders.MultiFind(MapsName, (TArray<UUIResourceDataProvider*>&)MapProviders);
	for ( INT ProviderIndex = 0; ProviderIndex < MapProviders.Num(); ProviderIndex++ )
	{
		UUTUIDataProvider_MapInfo* Provider = MapProviders(ProviderIndex);
		Provider->bOfficialMap = Provider->IsOfficialMap();
	}

	Sort<USE_COMPARE_CONSTPOINTER(UUTUIDataProvider_MapInfo,UI_DataStores)>( &MapProviders(0), MapProviders.Num() );

	// now re-add the mutators in reverse order so that calls to MultiFind will return them in the right order
	ListElementProviders.RemoveKey(MapsName);
	for ( INT ProviderIndex = MapProviders.Num() - 1; ProviderIndex >= 0; ProviderIndex-- )
	{
		ListElementProviders.Add(MapsName, MapProviders(ProviderIndex));
	}

	// now sort the mutator providers
	FName MutatorsName(TEXT("Mutators"));
	TArray<UUTUIDataProvider_Mutator*> MutatorProviders;
	ListElementProviders.MultiFind(MutatorsName, (TArray<UUIResourceDataProvider*>&)MutatorProviders);
	for ( INT ProviderIndex = 0; ProviderIndex < MutatorProviders.Num(); ProviderIndex++ )
	{
		UUTUIDataProvider_Mutator* Provider = MutatorProviders(ProviderIndex);
		Provider->bOfficialMutator = Provider->IsOfficialMutator();
	}

	Sort<USE_COMPARE_CONSTPOINTER(UUTUIDataProvider_Mutator,UI_DataStores)>( &MutatorProviders(0), MutatorProviders.Num() );

	// the index [into the MutatorProviders array] will be used as the bitflag for the mutator in the game search results value, so
	// verify that we aren't going to overflow the field
	INT LastOfficialMutatorIndex = MutatorProviders.Num() - 1;
	while ( LastOfficialMutatorIndex >= 0 && !MutatorProviders(LastOfficialMutatorIndex)->bOfficialMutator )
	{
		LastOfficialMutatorIndex--;
	}

	check(LastOfficialMutatorIndex >= 0);
	// sizeof(INT) because we're currently using a 4-byte integer to store the mutator mask
	check(LastOfficialMutatorIndex < sizeof(INT) * 8);

	// now re-add the mutators in reverse order so that calls to MultiFind will return them in the right order
	ListElementProviders.RemoveKey(MutatorsName);
	for ( INT ProviderIndex = MutatorProviders.Num() - 1; ProviderIndex >= 0; ProviderIndex-- )
	{
		ListElementProviders.Add(MutatorsName, MutatorProviders(ProviderIndex));
	}

#if 0
	//@re-enable to verify that non-official mutators are sorted correctly
	for ( INT ProviderIndex = 0; ProviderIndex < MutatorProviders.Num(); ProviderIndex++ )
	{
		UUTUIDataProvider_Mutator* Provider = MutatorProviders(ProviderIndex);
		debugf(TEXT("%i) %s: %s"), ProviderIndex, Provider->bOfficialMutator ? GTrue : GFalse, *Provider->ClassName);
	}
#endif
}

/** 
 * Gets the list of element providers for a fieldname with filtered elements removed.
 *
 * @param FieldName				Fieldname to use to search for providers.
 * @param OutElementProviders	Array to store providers in.
 */
void UUTUIDataStore_MenuItems::GetFilteredElementProviders(FName FieldName, TArray<UUTUIResourceDataProvider*> &OutElementProviders)
{
	OutElementProviders.Empty();

	if(FieldName==TEXT("EnabledMutators") || FieldName==TEXT("AvailableMutators"))
	{
		FieldName = TEXT("Mutators");
	}
	else if(FieldName==TEXT("MapCycle") || FieldName==TEXT("MapsNotInCycle"))
	{
		FieldName = TEXT("Maps");
	}
	else if(FieldName==TEXT("WeaponPriority"))
	{
		FieldName = TEXT("DropDownWeapons");
	}
	else if(FieldName==TEXT("GameModeFilter"))
	{
		FieldName = TEXT("GameModes");

		// Show all game modes for the game mode filter
		TArray<UUIResourceDataProvider*> Providers;
		ListElementProviders.MultiFind(FieldName, Providers);

		for ( INT ProviderIndex = 0; ProviderIndex < Providers.Num(); ProviderIndex++ )
		{
			UUTUIDataProvider_GameModeInfo* DataProvider = Cast<UUTUIDataProvider_GameModeInfo>(Providers(ProviderIndex));
			if(DataProvider)
			{
				OutElementProviders.AddUniqueItem(DataProvider);
			}
		}

		return;
	}
	else if ( FieldName == TEXT("OfficialMutators") )
	{
		TArray<UUIResourceDataProvider*> Providers;
		ListElementProviders.MultiFind(TEXT("Mutators"), Providers);
		for ( INT ProviderIndex = 0; ProviderIndex < Providers.Num(); ProviderIndex++ )
		{
			UUTUIDataProvider_Mutator* DataProvider = Cast<UUTUIDataProvider_Mutator>(Providers(ProviderIndex));
			if ( DataProvider && DataProvider->bOfficialMutator && !DataProvider->IsFiltered() )
			{
				OutElementProviders.AddUniqueItem(DataProvider);
			}
		}

		return;
	}

	TArray<UUIResourceDataProvider*> Providers;
	ListElementProviders.MultiFind(FieldName, Providers);

	for ( INT ProviderIndex = 0; ProviderIndex < Providers.Num(); ProviderIndex++ )
	{
		UUTUIResourceDataProvider* DataProvider = Cast<UUTUIResourceDataProvider>(Providers(ProviderIndex));
		if(DataProvider && DataProvider->IsFiltered() == FALSE)
		{
			OutElementProviders.AddUniqueItem(DataProvider);
		}
	}
}

/* === IUIListElementProvider interface === */
/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UUTUIDataStore_MenuItems::GetElementProviderTags()
{
	TArray<FName> Tags = Super::GetElementProviderTags();
	
	// Loop through all of the simple menus and get tags.
	TArray<UUIResourceDataProvider*> Providers;
	ListElementProviders.MultiFind(TEXT("SimpleMenus"), Providers);

	for(INT MenuIdx=0; MenuIdx<Providers.Num(); MenuIdx++)
	{
		UUTUIDataProvider_SimpleMenu* Menu = Cast<UUTUIDataProvider_SimpleMenu>(Providers(MenuIdx));

		if(Menu)
		{
			Tags.AddItem(Menu->FieldName);
		}
	}

	Tags.AddItem(TEXT("AvailableMutators"));
	Tags.AddItem(TEXT("EnabledMutators"));
	Tags.AddItem(TEXT("OfficialMutators"));
//	not ready for CustomMutators yet
//	Tags.AddItem(TEXT("CustomMutators"));
	Tags.AddItem(TEXT("MapCycle"));
	Tags.AddItem(TEXT("MapsNotInCycle"));
	Tags.AddItem(TEXT("WeaponPriority"));
	Tags.AddItem(TEXT("GameModeFilter"));

	return Tags;
}


/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UUTUIDataStore_MenuItems::GetElementCount( FName FieldName )
{
	INT Result = 0;
	UBOOL bFound = FALSE;

	// Check for simple menu first.
	UUTUIDataProvider_SimpleMenu* Menu = FindSimpleMenu(FieldName);

	if(Menu)
	{
		Result = Menu->Options.Num();
		bFound = TRUE;
	}

	if(bFound==FALSE)
	{
		TArray<UUTUIResourceDataProvider*> Providers;
		GetFilteredElementProviders(FieldName, Providers);

		return Providers.Num();
	}

	return Result;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UUTUIDataStore_MenuItems::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	UUTUIDataProvider_SimpleMenu* Menu = FindSimpleMenu(FieldName);

	if(Menu)
	{
		for(INT OptionIdx=0; OptionIdx<Menu->Options.Num(); OptionIdx++)
		{
			out_Elements.AddItem(OptionIdx);
		}

		bResult = TRUE;
	}

	if(bResult==FALSE)
	{
		TArray<UUIResourceDataProvider*> Providers;

		if(FieldName==TEXT("EnabledMutators"))
		{
			FieldName = TEXT("Mutators");
			ListElementProviders.MultiFind(FieldName, Providers);

			// Use the enabled mutators array to determine which mutators to expose
			out_Elements.Empty();
			for ( INT MutatorIndex = 0; MutatorIndex < EnabledMutators.Num(); MutatorIndex++ )
			{
				INT ProviderIndex = EnabledMutators(MutatorIndex);
				if(Providers.IsValidIndex(ProviderIndex))
				{	
					UUTUIResourceDataProvider* DataProvider = Cast<UUTUIResourceDataProvider>(Providers(ProviderIndex));
					if(DataProvider && DataProvider->IsFiltered() == FALSE)
					{
						out_Elements.AddUniqueItem(ProviderIndex);
					}
				}
			}

			bResult = TRUE;
		}
		else if(FieldName==TEXT("AvailableMutators"))
		{
			FieldName = TEXT("Mutators");
			ListElementProviders.MultiFind(FieldName, Providers);

			// Make sure the provider index isnt in the enabled mutators array.
			out_Elements.Empty();
			for ( INT ProviderIndex = 0; ProviderIndex < Providers.Num(); ProviderIndex++ )
			{
				UUTUIResourceDataProvider* DataProvider = Cast<UUTUIResourceDataProvider>(Providers(ProviderIndex));
				if(DataProvider && DataProvider->IsFiltered() == FALSE && EnabledMutators.ContainsItem(ProviderIndex)==FALSE)
				{
					out_Elements.AddUniqueItem(ProviderIndex);
				}
			}
			bResult = TRUE;
		}
		else if ( FieldName == TEXT("OfficialMutators") )
		{
			ListElementProviders.MultiFind(TEXT("Mutators"), Providers);

			out_Elements.Empty();
			for ( INT ProviderIndex = 0; ProviderIndex < Providers.Num(); ProviderIndex++ )
			{
				UUTUIDataProvider_Mutator* DataProvider = Cast<UUTUIDataProvider_Mutator>(Providers(ProviderIndex));
				if( DataProvider && !DataProvider->IsFiltered() && DataProvider->bOfficialMutator )
				{
					out_Elements.AddUniqueItem(ProviderIndex);
				}
			}
			bResult = TRUE;
		}
		else if(FieldName==TEXT("MapCycle"))
		{
			FieldName = TEXT("Maps");
			ListElementProviders.MultiFind(FieldName, Providers);

			// Use the enabled mutators array to determine which mutators to expose
			out_Elements.Empty();
			for ( INT MapIndex = 0; MapIndex < MapCycle.Num(); MapIndex++ )
			{
				INT ProviderIndex = MapCycle(MapIndex);
				if(Providers.IsValidIndex(ProviderIndex))
				{	
					UUTUIResourceDataProvider* DataProvider = Cast<UUTUIResourceDataProvider>(Providers(ProviderIndex));
					if(DataProvider && DataProvider->IsFiltered() == FALSE)
					{
						out_Elements.AddUniqueItem(ProviderIndex);
					}
				}
			}

			bResult = TRUE;
		}
		else if(FieldName==TEXT("MapsNotInCycle"))
		{
			FieldName = TEXT("Maps");
			ListElementProviders.MultiFind(FieldName, Providers);

			// Make sure the provider index isnt in the enabled mutators array.
			out_Elements.Empty();
			for ( INT ProviderIndex = 0; ProviderIndex < Providers.Num(); ProviderIndex++ )
			{
				UUTUIResourceDataProvider* DataProvider = Cast<UUTUIResourceDataProvider>(Providers(ProviderIndex));
				if(DataProvider && DataProvider->IsFiltered() == FALSE && MapCycle.ContainsItem(ProviderIndex)==FALSE)
				{
					out_Elements.AddUniqueItem(ProviderIndex);
				}
			}
			bResult = TRUE;
		}
		else if(FieldName==TEXT("WeaponPriority"))
		{
			FieldName = TEXT("DropDownWeapons");
			ListElementProviders.MultiFind(FieldName, Providers);

			// Use the weapon priority array to determine the order of weapons.
			out_Elements.Empty();
			for ( INT WeaponIdx = 0; WeaponIdx < WeaponPriority.Num(); WeaponIdx++ )
			{
				INT ProviderIndex = WeaponPriority(WeaponIdx);
				if(Providers.IsValidIndex(ProviderIndex))
				{	
					UUTUIDataProvider_Weapon* DataProvider = Cast<UUTUIDataProvider_Weapon>(Providers(ProviderIndex));
					if(DataProvider && DataProvider->IsFiltered() == FALSE)
					{
						out_Elements.AddUniqueItem(ProviderIndex);
					}
				}
			}

			bResult = TRUE;
		}
		else if(FieldName==TEXT("GameModeFilter"))
		{
			FieldName = TEXT("GameModes");

			ListElementProviders.MultiFind(FieldName, Providers);
			if ( Providers.Num() > 0 )
			{
				out_Elements.Empty();
				for ( INT ProviderIndex = 0; ProviderIndex < Providers.Num(); ProviderIndex++ )
				{
					UUTUIDataProvider_GameModeInfo* DataProvider = Cast<UUTUIDataProvider_GameModeInfo>(Providers(ProviderIndex));
					if(DataProvider && (DataProvider->IsFiltered() == FALSE || DataProvider->bIsCampaign))
					{
						out_Elements.AddUniqueItem(ProviderIndex);
					}
				}
				bResult = TRUE;
			}
		}
		else
		{
			ListElementProviders.MultiFind(FieldName, Providers);
			if ( Providers.Num() > 0 )
			{
				out_Elements.Empty();
				for ( INT ProviderIndex = 0; ProviderIndex < Providers.Num(); ProviderIndex++ )
				{
					UUTUIResourceDataProvider* DataProvider = Cast<UUTUIResourceDataProvider>(Providers(ProviderIndex));
					if(DataProvider && DataProvider->IsFiltered() == FALSE)
					{
						out_Elements.AddUniqueItem(ProviderIndex);
					}
				}
				bResult = TRUE;
			}
			else
			{
				bResult = Super::GetListElements(FieldName, out_Elements);
			}
		}
	}

	return bResult;
}

/** 
 * Attempts to retrieve all providers with the specified provider field name.
 *
 * @param ProviderFieldName		Name of the provider set to search for
 * @param OutProviders			A set of providers with the given name
 * 
 * @return	TRUE if the set was found, FALSE otherwise.
 */
UBOOL UUTUIDataStore_MenuItems::GetProviderSet(FName ProviderFieldName,TArray<class UUTUIResourceDataProvider*>& OutProviders)
{
	OutProviders.Empty();
	
	TArray<UUIResourceDataProvider*> Providers;
	ListElementProviders.MultiFind(ProviderFieldName, Providers);
	for ( INT ProviderIndex = 0; ProviderIndex < Providers.Num(); ProviderIndex++ )
	{
		UUTUIResourceDataProvider* DataProvider = Cast<UUTUIResourceDataProvider>(Providers(ProviderIndex));

		if(DataProvider!=NULL)
		{
			OutProviders.AddUniqueItem(DataProvider);
		}
	}

	return (OutProviders.Num()>0);
}

/* === UIDataProvider interface === */
/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 *
 * @todo - not yet implemented
 */
UBOOL UUTUIDataStore_MenuItems::GetFieldValue( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	UBOOL bResult = FALSE;

	if(FieldName==TEXT("GameModeFilter"))
	{
		bResult = GetCellFieldValue(*FieldName, TEXT("GameModeFilter"), GameModeFilter, out_FieldValue);
	}
	else if(FieldName==TEXT("GameModeFilterClass"))
	{
		TArray<UUIResourceDataProvider*> Providers;
		ListElementProviders.MultiFind(TEXT("GameModes"), Providers);

		if(Providers.IsValidIndex(GameModeFilter))
		{
			UUTUIDataProvider_GameModeInfo* GameModeInfo = Cast<UUTUIDataProvider_GameModeInfo>(Providers(GameModeFilter));
			out_FieldValue.PropertyTag = *FieldName;
			out_FieldValue.PropertyType = DATATYPE_Property;
			out_FieldValue.StringValue = GameModeInfo->GameMode;
			out_FieldValue.ArrayValue.AddItem(GameModeFilter);
			bResult = TRUE;
		}
	}

	if(bResult==FALSE)
	{
		bResult = Super::GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
	}

	return bResult;
}

/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUTUIDataStore_MenuItems::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	TArray<UUIResourceDataProvider*> Providers;

	/*
	out_Fields.Empty();

	for ( INT ProviderIndex = 0; ProviderIndex < ElementProviderTypes.Num(); ProviderIndex++ )
	{
		FGameResourceDataProvider& Provider = ElementProviderTypes(ProviderIndex);

		TArray<UUTUIResourceDataProvider*> ResourceProviders;
		GetFilteredElementProviders(Provider.ProviderTag, ResourceProviders);

		// for each of the game resource providers, add a tag to allow the UI to access the list of providers
		new(out_Fields) FUIDataProviderField( Provider.ProviderTag, (TArray<UUIDataProvider*>&)ResourceProviders );
	}
	*/
	Super::GetSupportedDataFields(out_Fields);

	// Add the enabled/available mutators fields.
	Providers.Empty();
	ListElementProviders.MultiFind(TEXT("Mutators"), Providers);
	out_Fields.AddItem(FUIDataProviderField(TEXT("EnabledMutators"), (TArray<UUIDataProvider*>&)Providers));
	out_Fields.AddItem(FUIDataProviderField(TEXT("AvailableMutators"), (TArray<UUIDataProvider*>&)Providers));

	while ( Providers.Num() > 0 && !Cast<UUTUIDataProvider_Mutator>(Providers.Last())->bOfficialMutator )
	{
		Providers.Pop();
	}
	out_Fields.AddItem(FUIDataProviderField(TEXT("OfficialMutators"), (TArray<UUIDataProvider*>&)Providers));

	// Add the mapcycle fields.
	Providers.Empty();
	ListElementProviders.MultiFind(TEXT("Maps"), Providers);
	out_Fields.AddItem(FUIDataProviderField(TEXT("MapCycle"), (TArray<UUIDataProvider*>&)Providers));
	out_Fields.AddItem(FUIDataProviderField(TEXT("MapsNotInCycle"), (TArray<UUIDataProvider*>&)Providers));

	Providers.Empty();
	ListElementProviders.MultiFind(TEXT("GameModes"), Providers);
	out_Fields.AddItem(FUIDataProviderField(TEXT("GameModeFilter"), (TArray<UUIDataProvider*>&)Providers));
	out_Fields.AddItem(FUIDataProviderField(TEXT("GameModeFilterClass"), DATATYPE_Property));

	Providers.Empty();
	ListElementProviders.MultiFind(TEXT("DropDownWeapons"), Providers);
	out_Fields.AddItem(FUIDataProviderField(TEXT("WeaponPriority"), (TArray<UUIDataProvider*>&)Providers));

	// Loop through all of the simple menus.
	Providers.Empty();
	ListElementProviders.MultiFind(TEXT("SimpleMenus"), Providers);

	for(INT MenuIdx=0; MenuIdx<Providers.Num(); MenuIdx++)
	{
		UUTUIDataProvider_SimpleMenu* Menu = Cast<UUTUIDataProvider_SimpleMenu>(Providers(MenuIdx));

		if(Menu)
		{
			TArray<UUIDataProvider*> Providers;
			Providers.AddItem(Menu);

			FUIDataProviderField NewField(Menu->FieldName,  Providers);
			out_Fields.AddItem(NewField);
		}
	}
}

/**
 * Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	FieldName		the tag of the list element data provider that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<class IUIListElementCellProvider> UUTUIDataStore_MenuItems::GetElementCellSchemaProvider( FName FieldName )
{
	TScriptInterface<class IUIListElementCellProvider> Result;
	UUTUIDataProvider_SimpleMenu* Menu = FindSimpleMenu(FieldName);

	if(Menu)
	{
		Result = Menu;
	}
	else
	{
		// Replace enabled/available mutators with the straight mutators schema.
		if(FieldName==TEXT("EnabledMutators") || FieldName==TEXT("AvailableMutators") || FieldName == TEXT("OfficialMutators") )
		{
			FieldName = TEXT("Mutators");
		}
		else if(FieldName==TEXT("MapCycle") || FieldName==TEXT("MapsNotInCycle"))
		{
			FieldName = TEXT("Maps");
		}
		else if(FieldName==TEXT("WeaponPriority"))
		{
			FieldName = TEXT("DropDownWeapons");
		}
		else if(FieldName==TEXT("GameModeFilter"))
		{
			return this;
		}

		Result = Super::GetElementCellSchemaProvider(FieldName);
	}

	return Result;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 *
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<class IUIListElementCellProvider> UUTUIDataStore_MenuItems::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	TScriptInterface<class IUIListElementCellProvider> Result;
	UUTUIDataProvider_SimpleMenu* Menu = FindSimpleMenu(FieldName);

	if(Menu)
	{
		Result = Menu;
	}
	else
	{
		// Replace enabled/available mutators with the straight mutators schema.
		if(FieldName==TEXT("EnabledMutators") || FieldName==TEXT("AvailableMutators") || FieldName == TEXT("OfficialMutators") )
		{
			FieldName = TEXT("Mutators");
		}
		else if(FieldName==TEXT("MapCycle") || FieldName==TEXT("MapsNotInCycle"))
		{
			FieldName = TEXT("Maps");
		}
		else if(FieldName==TEXT("WeaponPriority"))
		{
			FieldName = TEXT("DropDownWeapons");
		}
		else if(FieldName==TEXT("GameModeFilter"))
		{
			return this;
		}

		Result = Super::GetElementCellValueProvider(FieldName, ListIndex);
	}

	return Result;
}

/**
 * Resolves the value of the data field specified and stores the value specified to the appropriate location for that field.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	FieldValue		the value to store for the property specified.
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUTUIDataStore_MenuItems::SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex )
{
	UBOOL bResult = FALSE;

	if(FieldName==TEXT("GameModeFilter"))
	{
		if ( FieldValue.StringValue.Len() > 0 || FieldValue.ArrayValue.Num() > 0 )
		{
			INT NumItems = GetElementCount(*FieldName);

			for(INT ValueIdx=0; ValueIdx<NumItems; ValueIdx++)
			{
				FUIProviderFieldValue out_FieldValue(EC_EventParm);

				if(GetCellFieldValue(*FieldName, TEXT("GameModeFilter"), ValueIdx, out_FieldValue))
				{
					if ((FieldValue.StringValue.Len() > 0 && FieldValue.StringValue == out_FieldValue.StringValue)
					||	(FieldValue.StringValue.Len() == 0 && FieldValue.ArrayValue.Num() > 0 && FieldValue.ArrayValue.ContainsItem(ValueIdx)))
					{
						GameModeFilter = ValueIdx;
						bResult = TRUE;
						break;
					}
				}
			}
		}
	}

	if(bResult==FALSE)
	{
		bResult = Super::SetFieldValue(FieldName, FieldValue, ArrayIndex);
	}

	return bResult;
}

/**
 * Determines whether a member of a collection should be considered "enabled" by subscribed lists.  Disabled elements will still be displayed in the list
 * but will be drawn using the disabled state.
 *
 * @param	FieldName			the name of the collection data field that CollectionIndex indexes into.
 * @param	CollectionIndex		the index into the data field collection indicated by FieldName to check
 *
 * @return	TRUE if FieldName doesn't correspond to a valid collection data field, CollectionIndex is an invalid index for that collection,
 *			or the item is actually enabled; FALSE only if the item was successfully resolved into a data field value, but should be considered disabled.
 */
UBOOL UUTUIDataStore_MenuItems::IsElementEnabled( FName FieldName, INT CollectionIndex )
{
	return TRUE;
}

/**
 * Attempts to find a simple menu with the field name provided.
 *
 * @param FieldName		Field name of the simple menu.  Defined in UUTUIDataProvider_SimpleMenu::FieldName
 */
UUTUIDataProvider_SimpleMenu* UUTUIDataStore_MenuItems::FindSimpleMenu(FName FieldName)
{
	UUTUIDataProvider_SimpleMenu* Result = NULL;

	TArray<UUIResourceDataProvider*> Providers;
	ListElementProviders.MultiFind(TEXT("SimpleMenus"), Providers);

	for(INT MenuIdx=0; MenuIdx<Providers.Num(); MenuIdx++)
	{
		UUTUIDataProvider_SimpleMenu* Menu = Cast<UUTUIDataProvider_SimpleMenu>(Providers(MenuIdx));

		if(Menu && Menu->FieldName==FieldName)
		{
			Result = Menu;
			break;
		}
	}

	return Result;
}

/** @return Returns the number of providers for a given field name. */
INT UUTUIDataStore_MenuItems::GetProviderCount(FName FieldName)
{
	TArray<UUTUIResourceDataProvider*> Providers;
	GetFilteredElementProviders(FieldName, Providers);

	return Providers.Num();
}




/**
 * Returns the names of the exposed members in the first entry in the array
 * of search results
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param OutCellTags the columns supported by this row
 */
void UUTUIDataStore_MenuItems::GetElementCellTags( FName FieldName, TMap<FName,FString>& out_CellTags )
{
	static UProperty* GameModeFilterProperty = FindField<UProperty>(GetClass(),TEXT("GameModeFilter"));
	checkSlow(GameModeFilterProperty);

	out_CellTags.Set(TEXT("GameModeFilter"), *GameModeFilterProperty->GetFriendlyName(GetClass()));
}

/**
 * Retrieves the field type for the specified cell.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag				the tag for the element cell to get the field type for
 * @param	out_CellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
 *
 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
 */
UBOOL UUTUIDataStore_MenuItems::GetCellFieldType( FName FieldName, const FName& CellTag, BYTE& out_CellFieldType )
{
	return DATATYPE_Property;
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
 *							do not provide unique UIListElement objects for each element.
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
 *							to a data collection.
 */
UBOOL UUTUIDataStore_MenuItems::GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	UBOOL bResult = FALSE;
	TArray<UUIResourceDataProvider*> Providers;
	Providers.Empty();
	FString FieldToFind=CellTag.ToString();
	ListElementProviders.MultiFind(TEXT("GameModes"), Providers);

	if(CellTag==TEXT("GameModeFilter"))
	{
		FieldToFind=TEXT("FriendlyName");
	}
	
	if(Providers.IsValidIndex(ListIndex))
	{
		UUTUIDataProvider_GameModeInfo* GameMode = Cast<UUTUIDataProvider_GameModeInfo>(Providers(ListIndex));
		bResult = GameMode->GetFieldValue(FieldToFind, out_FieldValue);
		if ( bResult && out_FieldValue.StringValue.Len() > 0 )
		{
			out_FieldValue.ArrayValue.AddItem(ListIndex);
		}
	}

	return bResult;
}


/**
 * Attempts to find the index of a provider given a provider field name, a search tag, and a value to match.
 *
 * @return	Returns the index of the provider or INDEX_NONE if the provider wasn't found.
 */
INT UUTUIDataStore_MenuItems::FindValueInProviderSet(FName ProviderFieldName,FName SearchTag,const FString& SearchValue)
{
	INT Result = INDEX_NONE;
	TArray<INT> PossibleItems;
	GetListElements(ProviderFieldName, PossibleItems);

	for(INT ItemIdx=0; ItemIdx<PossibleItems.Num(); ItemIdx++)
	{
		FUIProviderFieldValue OutValue(EC_EventParm);
		TScriptInterface<IUIListElementCellProvider> DataProvider = GetElementCellValueProvider(ProviderFieldName, PossibleItems(ItemIdx));

		if ( DataProvider && DataProvider->GetCellFieldValue(ProviderFieldName,SearchTag, PossibleItems(ItemIdx), OutValue))
		{
			if(SearchValue==OutValue.StringValue)
			{
				Result = PossibleItems(ItemIdx);
				break;
			}
		}
	}

	return Result;
}


/**
 * @return	Returns whether or not the specified provider is filtered or not.
 */
UBOOL UUTUIDataStore_MenuItems::IsProviderFiltered(FName ProviderFieldName, INT ProviderIdx)
{
	UBOOL bResult = TRUE;

	TScriptInterface<IUIListElementCellProvider> DataProvider = GetElementCellValueProvider(ProviderFieldName, ProviderIdx);
	UUTUIResourceDataProvider* UTDataProvider = Cast<UUTUIResourceDataProvider>(DataProvider->GetUObjectInterfaceUIListElementCellProvider());
	if(UTDataProvider)
	{
		bResult = UTDataProvider->IsFiltered();
	}

	return bResult;
}



/**
 * Attempts to find the value of a provider given a provider cell field.
 *
 * @return	Returns true if the value was found, false otherwise.
 */
UBOOL UUTUIDataStore_MenuItems::GetValueFromProviderSet(FName ProviderFieldName,FName SearchTag,INT ListIndex,FString& OutValue)
{
	UBOOL bResult = FALSE;
	FUIProviderFieldValue FieldValue(EC_EventParm);
	TScriptInterface<IUIListElementCellProvider> DataProvider = GetElementCellValueProvider(ProviderFieldName, ListIndex);

	if( DataProvider && DataProvider->GetCellFieldValue(ProviderFieldName,SearchTag, ListIndex, FieldValue))
	{
		OutValue = FieldValue.StringValue;
		bResult = TRUE;
	}

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_SimpleMenu
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_SimpleMenu);

/**
 * Returns the data tag associated with the specified provider.
 *
 * @return	the data field tag associated with the provider specified, or NAME_None if the provider specified is not
 *			contained by this data store.
 */
FName UUTUIDataProvider_SimpleMenu::GetProviderDataTag( UUIDataProvider* Provider )
{
	return FieldName;
}

/**
 * Retrieves the list of tags that can be bound to individual cells in a single list element.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	out_CellTags	receives the list of tag/column headers that can be bound to element cells for the specified property.
 */
void UUTUIDataProvider_SimpleMenu::GetElementCellTags( FName FieldName, TMap<FName,FString>& out_CellTags )
{
	out_CellTags.Set(TEXT("OptionName"),TEXT("OptionName"));
}

/**
 * Retrieves the field type for the specified cell.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag				the tag for the element cell to get the field type for
 * @param	out_CellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
 *
 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
 */
UBOOL UUTUIDataProvider_SimpleMenu::GetCellFieldType( FName FieldName, const FName& CellTag, BYTE& out_CellFieldType )
{
	UBOOL bResult = FALSE;

	if(CellTag==TEXT("OptionName"))
	{
		out_CellFieldType = DATATYPE_Property;
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
 *							do not provide unique UIListElement objects for each element.
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
 *							to a data collection.
 */
UBOOL UUTUIDataProvider_SimpleMenu::GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	UBOOL bResult = FALSE;

	if((CellTag==TEXT("OptionName") || CellTag == FieldName) && Options.IsValidIndex(ListIndex))
	{
		out_FieldValue.PropertyType = DATATYPE_Property;
		out_FieldValue.PropertyTag = CellTag;
		out_FieldValue.StringValue = Options(ListIndex);
		bResult = TRUE;
	}

	return bResult;
}


/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 *
 * @todo - not yet implemented
 */
UBOOL UUTUIDataProvider_SimpleMenu::GetFieldValue( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	UBOOL bResult = FALSE;

	if(FieldName==TEXT("OptionName") && Options.IsValidIndex(ArrayIndex))
	{
		out_FieldValue.PropertyTag = TEXT("OptionName");
		out_FieldValue.StringValue = Options(ArrayIndex);
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 * 						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUTUIDataProvider_SimpleMenu::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	Super::GetSupportedDataFields(out_Fields);

	new(out_Fields) FUIDataProviderField( TEXT("OptionName"), DATATYPE_Property );
}


//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_GameModeInfo
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_GameModeInfo);

/** @return Whether or not we should be filtered out of the list results. */
UBOOL UUTUIDataProvider_GameModeInfo::IsFiltered()
{
	UBOOL bFiltered = FALSE;

#if DEMOVERSION
	if( Prefixes==TEXT("DM") || Prefixes==TEXT("VCTF") )
	{
		bFiltered=FALSE;
	}
	else
	{
		bFiltered=TRUE;
	}
#endif

	// Only show campaign game mode in very specific cases.
	return bIsCampaign || Super::IsFiltered() || bFiltered;
}

//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_MapInfo
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_MapInfo);

/**
 * @return	TRUE if this data provider corresponds to an epic map.
 */
UBOOL UUTUIDataProvider_MapInfo::IsOfficialMap() const
{
	UBOOL bResult = FALSE;

	if ( MapName.Len() > 0 )
	{
		UUTOfficialContent* TOCCDO = UUTOfficialContent::StaticClass()->GetDefaultObject<UUTOfficialContent>();
		
		INT OptionPos = MapName.InStr(TEXT("?"));
		if ( OptionPos != INDEX_NONE )
		{
			bResult = TOCCDO->OfficialMaps.ContainsItem(MapName.Left(OptionPos));
		}
		else
		{
			bResult = TOCCDO->OfficialMaps.ContainsItem(MapName);
		}
	}

	return bResult;
}

/** @return Whether or not we should be filtered out of the list results. */
UBOOL UUTUIDataProvider_MapInfo::IsFiltered()
{
	UBOOL bFiltered = FALSE;


#if DEMOVERSION
	if( MapName==TEXT("DM-HeatRay") || MapName==TEXT("DM-ShangriLa") || MapName==TEXT("VCTF-Suspense") )
	{
		bFiltered=FALSE;
	}
	else
	{
		bFiltered=TRUE;
	}
#endif

	return (Super::IsFiltered() || eventSupportedByCurrentGameMode()==FALSE || bFiltered);
}

//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_Mutator
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_Mutator);

/**
 * @return	TRUE if this data provider corresponds to an epic mutator class.
 */
UBOOL UUTUIDataProvider_Mutator::IsOfficialMutator() const
{
	UBOOL bResult = FALSE;

	if ( ClassName.Len() > 0 )
	{
		FString PackageName, MutatorClass;
		if ( ClassName.Split(TEXT("."), &PackageName, &MutatorClass) )
		{
			// verify that we don't have a malformed string
			if ( PackageName.InStr(TEXT(".")) == INDEX_NONE
			&&	MutatorClass.InStr(TEXT(".")) == INDEX_NONE )
			{
				UUTOfficialContent* TOCCDO = UUTOfficialContent::StaticClass()->GetDefaultObject<UUTOfficialContent>();
				if ( TOCCDO->OfficialPackages.ContainsItem(PackageName) )
				{
					bResult = TRUE;
				}
			}
		}
	}

	return bResult;
}

/** @return Whether or not we should be filtered out of the list results. */
UBOOL UUTUIDataProvider_Mutator::IsFiltered()
{
	return (Super::IsFiltered() || eventSupportsCurrentGameMode()==FALSE);
}

//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_CharacterFaction
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_CharacterFaction);

/** Sets the Faction data for this provider. */
void UUTUIDataProvider_CharacterFaction::SetData(FFactionInfo &InFactionData)
{
	CustomData = InFactionData;
}

// IUIListElement interface

/**
 * Returns the names of the exposed members in the first entry in the array
 * of search results
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param OutCellTags the columns supported by this row
 */
void UUTUIDataProvider_CharacterFaction::GetElementCellTags( FName FieldName, TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Empty();
	OutCellTags.Set(TEXT("FactionID"),TEXT("FactionID"));
	OutCellTags.Set(TEXT("FriendlyName"),TEXT("FriendlyName"));
	OutCellTags.Set(TEXT("Description"),TEXT("Description"));
	OutCellTags.Set(TEXT("PreviewImageMarkup"),TEXT("PreviewImageMarkup"));
}

/**
 * Retrieves the field type for the specified cell.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag				the tag for the element cell to get the field type for
 * @param	OutCellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
 *
 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
 */
UBOOL UUTUIDataProvider_CharacterFaction::GetCellFieldType(FName FieldName, const FName& CellTag,BYTE& OutCellFieldType)
{
	TMap<FName,FString> CellTags;
	GetElementCellTags(FieldName,CellTags);
	UBOOL bResult = FALSE;

	if(CellTags.Find(CellTag) != NULL)
	{
		OutCellFieldType = DATATYPE_Property;
		bResult = TRUE;
	}
	

	return bResult;
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the index of the value to fetch
 * @param	OutFieldValue	receives the resolved value for the property specified.
 * @param	ArrayIndex		ignored
 */
UBOOL UUTUIDataProvider_CharacterFaction::GetCellFieldValue(FName FieldName, const FName& CellTag,INT ListIndex,FUIProviderFieldValue& OutFieldValue,INT)
{
	UBOOL bResult = FALSE;

	if(CellTag==TEXT("FriendlyName"))
	{
		OutFieldValue.PropertyTag = CellTag;
		OutFieldValue.PropertyType = DATATYPE_Property;
		OutFieldValue.StringValue = *Localize(TEXT("FactionData"),*(CustomData.Faction+TEXT("_FriendlyName")),TEXT("UTGameUI"));
		bResult = TRUE;
	}
	else if(CellTag==TEXT("FactionID"))
	{
		OutFieldValue.PropertyTag = CellTag;
		OutFieldValue.PropertyType = DATATYPE_Property;
		OutFieldValue.StringValue = CustomData.Faction;
		bResult = TRUE;
	}
	else if(CellTag==TEXT("Description"))
	{
		OutFieldValue.PropertyTag = CellTag;
		OutFieldValue.PropertyType = DATATYPE_Property;
		OutFieldValue.StringValue = CustomData.Description;
		bResult = TRUE;
	}
	else if(CellTag==TEXT("PreviewImageMarkup"))
	{
		OutFieldValue.PropertyTag = CellTag;
		OutFieldValue.PropertyType = DATATYPE_Property;
		OutFieldValue.StringValue = CustomData.PreviewImageMarkup;
		bResult = TRUE;
	}

	return bResult;
}


/* === UIDataProvider interface === */
/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUTUIDataProvider_CharacterFaction::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	out_Fields.Empty();

	TMap<FName,FString> Elements;
	GetElementCellTags(UCONST_UnknownCellDataFieldName,Elements);

	TArray<FName> ElementTags;
	Elements.GenerateKeyArray(ElementTags);

	for(INT TagIdx=0; TagIdx<ElementTags.Num(); TagIdx++)
	{
		// for each property contained by this ResourceDataProvider, add a provider field to expose them to the UI
		new(out_Fields) FUIDataProviderField( ElementTags(TagIdx), DATATYPE_Property );
	}
}

/**
 * Resolves the value of the data cell specified by FieldName and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUTUIDataProvider_CharacterFaction::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	FName PropertyFName(*FieldName, FNAME_Find);
	if ( PropertyFName != NAME_None )
	{
		bResult = GetCellFieldValue(UCONST_UnknownCellDataFieldName,PropertyFName,INDEX_NONE,out_FieldValue,ArrayIndex);
	}

	return bResult;
}



//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_Character
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_Character);

/** Sets the Faction data for this provider. */
void UUTUIDataProvider_Character::SetData(FCharacterInfo &InData)
{
	CustomData = InData;
}


/** @return Returns whether or not this provider should be filtered, by default it checks the platform flags. */
UBOOL UUTUIDataProvider_Character::IsFiltered()
{
	UBOOL bResult = FALSE;
	INT UnlockMask=0;
	UUTCharInfo* DataDefault = UUTCharInfo::StaticClass()->GetDefaultObject<UUTCharInfo>();

	UDataStoreClient* DataStoreClient = UUIInteraction::GetDataStoreClient();
	if ( DataStoreClient != NULL && GEngine->GamePlayers.Num() > 0 )
	{
		//@todo:  Is there a better way to get the profile value?
		UUIDataStore_OnlinePlayerData* PlayerDatastore = Cast<UUIDataStore_OnlinePlayerData>(DataStoreClient->FindDataStore(TEXT("OnlinePlayerData"), GEngine->GamePlayers(0)));
		if(PlayerDatastore != NULL && PlayerDatastore->ProfileProvider != NULL && PlayerDatastore->ProfileProvider->Profile != NULL)
		{
			if(PlayerDatastore->ProfileProvider->Profile->GetProfileSettingValueInt(UCONST_UTPID_UnlockedCharacters, UnlockMask))
			{	
				if(CustomData.bLocked && DataDefault->CharIsUnlocked(CustomData.CharName, UnlockMask)==FALSE)
				{
					bResult=TRUE;
				}
			}
		}
	}



	return bResult;
}

// IUIListElement interface

/**
 * Returns the names of the exposed members in the first entry in the array
 * of search results
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param OutCellTags the columns supported by this row
 */
void UUTUIDataProvider_Character::GetElementCellTags( FName FieldName, TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Empty();
	OutCellTags.Set(TEXT("CharacterID"),TEXT("CharacterID"));
	OutCellTags.Set(TEXT("FriendlyName"),TEXT("FriendlyName"));
	OutCellTags.Set(TEXT("PreviewImageMarkup"),TEXT("PreviewImageMarkup"));
	OutCellTags.Set(TEXT("Description"),TEXT("Description"));
}

/**
 * Retrieves the field type for the specified cell.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag				the tag for the element cell to get the field type for
 * @param	OutCellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
 *
 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
 */
UBOOL UUTUIDataProvider_Character::GetCellFieldType(FName FieldName, const FName& CellTag,BYTE& OutCellFieldType)
{
	TMap<FName,FString> CellTags;
	GetElementCellTags(FieldName,CellTags);
	UBOOL bResult = FALSE;

	if(CellTags.Find(CellTag) != NULL)
	{
		OutCellFieldType = DATATYPE_Property;
		bResult = TRUE;
	}
	

	return bResult;
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the index of the value to fetch
 * @param	OutFieldValue	receives the resolved value for the property specified.
 * @param	ArrayIndex		ignored
 */
UBOOL UUTUIDataProvider_Character::GetCellFieldValue(FName FieldName, const FName& CellTag,INT ListIndex,FUIProviderFieldValue& OutFieldValue,INT)
{
	UBOOL bResult = FALSE;

	if(CellTag==TEXT("FriendlyName"))
	{
		OutFieldValue.PropertyTag = CellTag;
		OutFieldValue.PropertyType = DATATYPE_Property;
		OutFieldValue.StringValue = CustomData.CharName;
		bResult = TRUE;
	}
	else if(CellTag==TEXT("CharacterID"))
	{
		OutFieldValue.PropertyTag = CellTag;
		OutFieldValue.PropertyType = DATATYPE_Property;
		OutFieldValue.StringValue = CustomData.CharID;
		bResult = TRUE;
	}
	else if(CellTag==TEXT("Description"))
	{
		OutFieldValue.PropertyTag = CellTag;
		OutFieldValue.PropertyType = DATATYPE_Property;
		OutFieldValue.StringValue = CustomData.Description;
		bResult = TRUE;
	}
	else if(CellTag==TEXT("PreviewImageMarkup"))
	{
		OutFieldValue.PropertyTag = CellTag;
		OutFieldValue.PropertyType = DATATYPE_Property;
		OutFieldValue.StringValue = CustomData.PreviewImageMarkup;
		bResult = TRUE;
	}

	return bResult;
}


/* === UIDataProvider interface === */
/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUTUIDataProvider_Character::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	out_Fields.Empty();

	TMap<FName,FString> Elements;
	GetElementCellTags(UCONST_UnknownCellDataFieldName,Elements);

	TArray<FName> ElementTags;
	Elements.GenerateKeyArray(ElementTags);

	for(INT TagIdx=0; TagIdx<ElementTags.Num(); TagIdx++)
	{
		// for each property contained by this ResourceDataProvider, add a provider field to expose them to the UI
		new(out_Fields) FUIDataProviderField( ElementTags(TagIdx), DATATYPE_Property );
	}
}

/**
 * Resolves the value of the data cell specified by FieldName and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUTUIDataProvider_Character::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	FName PropertyFName(*FieldName, FNAME_Find);
	if ( PropertyFName != NAME_None )
	{
		bResult = GetCellFieldValue(UCONST_UnknownCellDataFieldName,PropertyFName,INDEX_NONE,out_FieldValue,ArrayIndex);
	}

	return bResult;
}






//////////////////////////////////////////////////////////////////////////
// UUTUIDataStore_Options
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataStore_Options);


/**
 * Called when this data store is added to the data store manager's list of active data stores.
 *
 * Initializes the ListElementProviders map
 *
 * @param	PlayerOwner		the player that will be associated with this DataStore.  Only relevant if this data store is
 *							associated with a particular player; NULL if this is a global data store.
 */
void UUTUIDataStore_Options::OnRegister( ULocalPlayer* PlayerOwner )
{
	Super::OnRegister(PlayerOwner);

	// Initialize all of the option providers, go backwards because of the way MultiMap appends items.
	TArray<UUIResourceDataProvider*> Providers;
	ListElementProviders.MultiFind(TEXT("OptionSets"), Providers);

	for ( INT ProviderIndex = Providers.Num()-1; ProviderIndex >= 0; ProviderIndex-- )
	{
		UUTUIDataProvider_MenuOption* DataProvider = Cast<UUTUIDataProvider_MenuOption>(Providers(ProviderIndex));
		if(DataProvider)
		{
			for (INT OptionIndex=0;OptionIndex<DataProvider->OptionSet.Num();OptionIndex++)
			{
				OptionProviders.Add(DataProvider->OptionSet(OptionIndex), DataProvider);
			}
		}
	}
}


/** 
 * Gets the list of element providers for a fieldname with filtered elements removed.
 *
 * @param FieldName				Fieldname to use to search for providers.
 * @param OutElementProviders	Array to store providers in.
 */
void UUTUIDataStore_Options::GetFilteredElementProviders(FName FieldName, TArray<UUTUIResourceDataProvider*> &OutElementProviders)
{
	OutElementProviders.Empty();

	TArray<UUTUIResourceDataProvider*> Providers;
	OptionProviders.MultiFind(FieldName, Providers);

	for ( INT ProviderIndex = 0; ProviderIndex < Providers.Num(); ProviderIndex++ )
	{
		UUTUIResourceDataProvider* DataProvider = Providers(ProviderIndex);
		if(DataProvider && DataProvider->IsFiltered() == FALSE)
		{
			OutElementProviders.AddUniqueItem(DataProvider);
		}
	}
}

/* === IUIListElementProvider interface === */
/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UUTUIDataStore_Options::GetElementProviderTags()
{
	TLookupMap<FName> Keys;
	TArray<FName> OptionSets;
	OptionProviders.GetKeys(Keys);

	for(INT OptionIdx=0; OptionIdx<Keys.Num(); OptionIdx++)
	{
		OptionSets.AddUniqueItem(Keys(OptionIdx));
	}

	return OptionSets;
}


/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UUTUIDataStore_Options::GetElementCount( FName FieldName )
{
	TArray<UUTUIResourceDataProvider*> Providers;
	GetFilteredElementProviders(FieldName, Providers);

	return Providers.Num();
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UUTUIDataStore_Options::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	TArray<UUTUIResourceDataProvider*> Providers;
	OptionProviders.MultiFind(FieldName, Providers);

	if ( Providers.Num() > 0 )
	{
		out_Elements.Empty();
		for ( INT ProviderIndex = 0; ProviderIndex < Providers.Num(); ProviderIndex++ )
		{
			UUTUIResourceDataProvider* DataProvider = Providers(ProviderIndex);
			if(DataProvider && DataProvider->IsFiltered() == FALSE)
			{
				out_Elements.AddUniqueItem(ProviderIndex);
			}
		}
		bResult = TRUE;
	}

	return bResult;
}

/* === UIDataProvider interface === */
/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 *
 * @todo - not yet implemented
 */
UBOOL UUTUIDataStore_Options::GetFieldValue( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	return Super::GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
}

/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUTUIDataStore_Options::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	out_Fields.Empty();

	TLookupMap<FName> OptionSets;
	OptionProviders.GetKeys(OptionSets);

	for ( INT ProviderIndex = 0; ProviderIndex < OptionSets.Num(); ProviderIndex++ )
	{
		FName ProviderTag = *OptionSets(ProviderIndex);

		TArray<UUTUIResourceDataProvider*> ResourceProviders;
		OptionProviders.MultiFind(ProviderTag, ResourceProviders);

		// for each of the game resource providers, add a tag to allow the UI to access the list of providers
		new(out_Fields) FUIDataProviderField( ProviderTag, (TArray<UUIDataProvider*>&)ResourceProviders );
	}
}

/**
 * Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	FieldName		the tag of the list element data provider that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<class IUIListElementCellProvider> UUTUIDataStore_Options::GetElementCellSchemaProvider( FName FieldName )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	// search for the provider that has the matching tag
	INT ProviderIndex = FindProviderTypeIndex(TEXT("OptionSets"));
	if ( ProviderIndex != INDEX_NONE )
	{
		FGameResourceDataProvider& Provider = ElementProviderTypes(ProviderIndex);
		if ( Provider.ProviderClass != NULL )
		{
			Result = Provider.ProviderClass->GetDefaultObject<UUIResourceDataProvider>();
		}
	}

	return Result;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 *
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<class IUIListElementCellProvider> UUTUIDataStore_Options::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	TScriptInterface<class IUIListElementCellProvider> Result;
	
	// search for the provider that has the matching tag
	TArray<UUTUIResourceDataProvider*> Providers;
	OptionProviders.MultiFind(FieldName, Providers);

	if(Providers.IsValidIndex(ListIndex))
	{
		Result = Providers(ListIndex);
	}

	return Result;
}

/**
 * Resolves the value of the data field specified and stores the value specified to the appropriate location for that field.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	FieldValue		the value to store for the property specified.
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUTUIDataStore_Options::SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex )
{
	return Super::SetFieldValue(FieldName, FieldValue, ArrayIndex);
}

/**
 * Determines whether a member of a collection should be considered "enabled" by subscribed lists.  Disabled elements will still be displayed in the list
 * but will be drawn using the disabled state.
 *
 * @param	FieldName			the name of the collection data field that CollectionIndex indexes into.
 * @param	CollectionIndex		the index into the data field collection indicated by FieldName to check
 *
 * @return	TRUE if FieldName doesn't correspond to a valid collection data field, CollectionIndex is an invalid index for that collection,
 *			or the item is actually enabled; FALSE only if the item was successfully resolved into a data field value, but should be considered disabled.
 */
UBOOL UUTUIDataStore_Options::IsElementEnabled( FName FieldName, INT CollectionIndex )
{
	return TRUE;
}

/** 
 * Clears all options in the specified set.
 *
 * @param SetName		Set to clear
 */
void UUTUIDataStore_Options::ClearSet(FName SetName)
{
	TArray<UUTUIResourceDataProvider*> Providers;
	OptionProviders.MultiFind(SetName, Providers);

	for(INT ProviderIdx=0; ProviderIdx<Providers.Num(); ProviderIdx++)
	{
		DynamicProviders.RemoveItem(Providers(ProviderIdx));
	}

	// Remove teh key
	OptionProviders.RemoveKey(SetName);
}

/** 
 * Appends N amount of providers to the specified set.
 *
 * @param SetName		Set to append to
 * @param NumOptions	Number of options to append
 */
void UUTUIDataStore_Options::AppendToSet(FName SetName, INT NumOptions)
{
	for(INT AddIdx=0; AddIdx<NumOptions; AddIdx++)
	{
		UUTUIDataProvider_MenuOption* NewProvider = ConstructObject<UUTUIDataProvider_MenuOption>(UUTUIDataProvider_MenuOption::StaticClass(), this);
		OptionProviders.Add(SetName, NewProvider);
		DynamicProviders.AddUniqueItem(NewProvider);
	}
}

/**
 * Retrieves a set of option providers.
 *
 * @param SetName	Set to retreieve
 * 
 * @return Array of dataproviders for all the options in the set.
 */
void UUTUIDataStore_Options::GetSet(FName SetName, TArray<UUTUIResourceDataProvider*> &Providers)
{
	Providers.Empty();
	OptionProviders.MultiFind(SetName, Providers);
}


//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_MenuOption
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_MenuOption);

/** @return 	TRUE if this menu option's configuration isn't compatible with the desired game settings  */
UBOOL UUTUIDataProvider_MenuOption::IsFiltered()
{
	UBOOL bFiltered = Super::IsFiltered();

	if ( !bFiltered )
	{
		UDataStoreClient* DataStoreClient = UUIInteraction::GetDataStoreClient();
		if (DataStoreClient != NULL)
		{
			UUIDataStore_Registry* Registry = Cast<UUIDataStore_Registry>(DataStoreClient->FindDataStore(TEXT("Registry")));
			if(Registry)
			{
				FUIProviderFieldValue OutFieldValue(EC_EventParm);

				if ( Registry->GetDataStoreValue(TEXT("SelectedGameMode"), OutFieldValue ))
				{
					bFiltered = (RequiredGameMode != NAME_None && RequiredGameMode != *OutFieldValue.StringValue);
				}

				if ( !bFiltered && Registry->GetDataStoreValue(TEXT("StandaloneGame"), OutFieldValue) )
				{
								// bOnlineOnly option
					bFiltered = (bOnlineOnly && OutFieldValue.StringValue == TEXT("1"))
								// bOfflineOnly option
							||	(bOfflineOnly && OutFieldValue.StringValue == TEXT("0"));
				}
			}
		}

		// If we are the german version, remove gore option
#if FORCELOWGORE
		if(GetName()==TEXT("GoreLevel"))
		{
			bFiltered=TRUE;
		}
#endif

#if DEMOVERSION
		if(GetName()==TEXT("RecordDemo_DM"))
		{
			bFiltered=TRUE;
		}
#endif

#if PS3
		// If this menu option corresponds to something that is only relevant for keyboard/mouse (i.e. mouse sensitivity, etc.),
		// filter it out if we don't have a keyboard or mouse
		if ( bKeyboardOrMouseOption )
		{
			if(GEngine && GEngine->GamePlayers.Num() && GEngine->GamePlayers(0))
			{
				AUTPlayerController* PC = Cast<AUTPlayerController>(GEngine->GamePlayers(0)->Actor);

				if(PC != NULL)
				{
					if(PC->IsKeyboardAvailable()==FALSE && PC->IsMouseAvailable()==FALSE)
					{
						bFiltered=TRUE;
					}
				}
			}
		}
#endif
	}

	return bFiltered || Super::IsFiltered();
}

//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_MultiplayerMenuItem
//////////////////////////////////////////////////////////////////////////
/** @return 	TRUE if this data provider requires online access but is not able or allowed to play online */
UBOOL UUTUIDataProvider_MultiplayerMenuItem::IsFiltered()
{
	UBOOL bFiltered = Super::IsFiltered();

	if ( !bFiltered && bRequiresOnlineAccess )
	{
		UUIInteraction* InteractionCDO = GetDefaultUIController();
		for ( INT PlayerIndex = 0; PlayerIndex < GEngine->GamePlayers.Num(); PlayerIndex++ )
		{
			ULocalPlayer* LP = GEngine->GamePlayers(PlayerIndex);
			if ( LP != NULL )
			{
				if (!InteractionCDO->eventIsLoggedIn(LP->ControllerId,TRUE)
				||	!InteractionCDO->eventCanPlayOnline(LP->ControllerId) )
				{
					bFiltered = TRUE;
					break;
				}
			}
		}
	}

	return bFiltered;
}

//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_MultiplayerMenuItem
//////////////////////////////////////////////////////////////////////////
/** @return 	TRUE if this data provider requires online access but is not able or allowed to play online */
UBOOL UUTUIDataProvider_SettingsMenuItem::IsFiltered()
{
	UBOOL bFiltered = Super::IsFiltered();

	if ( !bFiltered && bFrontEndOnly )
	{
		bFiltered = !(GWorld && GWorld->GetWorldInfo()->IsMenuLevel());
	}

	return bFiltered;
}

//////////////////////////////////////////////////////////////////////////
// UUTDataStore_StringList
//////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLASS(UUTUIDataStore_StringList);

/**
 * @param FieldName		Name of the String List to find
 * @return the index of a string list
 */

INT UUTUIDataStore_StringList::GetFieldIndex(FName FieldName)
{
	INT Result = INDEX_NONE;
	for (INT i=0;i<StringData.Num();i++)
	{
		if (StringData(i).Tag == FieldName)
		{
			Result = i;
			break;
		}
	}
	return Result;
}

/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UUTUIDataStore_StringList::GetElementProviderTags()
{
	TArray<FName> Result;
	for (INT i=0;i<StringData.Num();i++)
	{
		Result.AddUniqueItem(StringData(i).Tag);
	}
	return Result;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UUTUIDataStore_StringList::GetElementCount( FName FieldName )
{
	INT Result = 0;
	const INT FieldIndex = GetFieldIndex(FieldName);
	if ( StringData.IsValidIndex(FieldIndex) )
	{
		Result = StringData(FieldIndex).Strings.Num();
	}
	return Result;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UUTUIDataStore_StringList::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	const INT FieldIndex = GetFieldIndex(FieldName);
	if ( StringData.IsValidIndex(FieldIndex) )
	{
		for (INT i=0;i<StringData(FieldIndex).Strings.Num();i++)
		{
			out_Elements.AddUniqueItem(i);
		}
		bResult = TRUE;
	}
	return bResult;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	FieldName		the tag of the list element data field that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<class IUIListElementCellProvider> UUTUIDataStore_StringList::GetElementCellSchemaProvider( FName FieldName )
{
	const INT FieldIndex = GetFieldIndex(FieldName);
	if ( StringData.IsValidIndex(FieldIndex) )
	{
		// Create a dataprovider for this string data object if one doesn't already exist.
		if(StringData(FieldIndex).DataProvider==NULL)
		{
			StringData(FieldIndex).DataProvider=ConstructObject<UUTUIDataProvider_StringArray>(UUTUIDataProvider_StringArray::StaticClass());
		}
		StringData(FieldIndex).DataProvider->Strings = StringData(FieldIndex).Strings;
		

		return TScriptInterface<IUIListElementCellProvider>(StringData(FieldIndex).DataProvider);
	}
	return TScriptInterface<IUIListElementCellProvider>();
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 *
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<class IUIListElementCellProvider> UUTUIDataStore_StringList::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	const INT FieldIndex = GetFieldIndex(FieldName);
	if ( StringData.IsValidIndex(FieldIndex) )
	{
		// Create a dataprovider for this string data object if one doesn't already exist.
		if(StringData(FieldIndex).DataProvider==NULL)
		{
			StringData(FieldIndex).DataProvider=ConstructObject<UUTUIDataProvider_StringArray>(UUTUIDataProvider_StringArray::StaticClass());
		}
		StringData(FieldIndex).DataProvider->Strings = StringData(FieldIndex).Strings;

		return TScriptInterface<IUIListElementCellProvider>(StringData(FieldIndex).DataProvider);
	}
	return TScriptInterface<IUIListElementCellProvider>();
}

TScriptInterface<class IUIListElementProvider> UUTUIDataStore_StringList::ResolveListElementProvider( const FString& PropertyName ) 
{
	return TScriptInterface<IUIListElementProvider> (this);
}

/* === UIListElementCellProvider === */

/**
 * Retrieves the list of tags that can be bound to individual cells in a single list element, along with the human-readable,
 * localized string that should be used in the header for each cell tag (in lists which have column headers enabled).
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	out_CellTags	receives the list of tag/column headers that can be bound to element cells for the specified property.
 */
void UUTUIDataStore_StringList::GetElementCellTags( FName FieldName, TMap<FName,FString>& out_CellTags )
{
	out_CellTags.Empty();
	for ( INT DataIndex = 0; DataIndex < StringData.Num(); DataIndex++ )
	{
		FEStringListData& ListData = StringData(DataIndex);
		out_CellTags.Set( ListData.Tag, ListData.ColumnHeaderText.Len() > 0 ? *ListData.ColumnHeaderText : *ListData.Tag.ToString() );			
	}
}

//	virtual void GetElementCellTags( TMap<FName,FString>& out_CellTags )=0;

/**
 * Retrieves the field type for the specified cell.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag				the tag for the element cell to get the field type for
 * @param	out_CellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
 *
 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
 */
UBOOL UUTUIDataStore_StringList::GetCellFieldType( FName FieldName, const FName& CellTag, BYTE& out_CellFieldType )
{
	return DATATYPE_Property;
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
 *							do not provide unique UIListElement objects for each element.
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
 *							to a data collection.
 */
UBOOL UUTUIDataStore_StringList::GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex)
{
	UBOOL bResult = FALSE;

	const INT FieldIndex = GetFieldIndex(CellTag);
	if ( FieldIndex > UCONST_INVALIDFIELD )
	{
		if (FieldIndex < StringData.Num() && ListIndex < StringData(FieldIndex).Strings.Num() )
		{
			out_FieldValue.StringValue = StringData(FieldIndex).Strings(ListIndex);
		}
		else
		{
			out_FieldValue.StringValue = TEXT("");
		}
		out_FieldValue.PropertyType = DATATYPE_Property;
		bResult = TRUE;
	}
	return bResult;
}


/* === UIDataProvider interface === */
/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUTUIDataStore_StringList::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	for (INT i=0;i<StringData.Num();i++)
	{
		new(out_Fields) FUIDataProviderField( StringData(i).Tag, DATATYPE_Collection );			
	}
}

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUTUIDataStore_StringList::GetFieldValue( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	UBOOL bResult = FALSE;

	const INT FieldIndex = GetFieldIndex(FName(*FieldName));

	if ( StringData.IsValidIndex(FieldIndex) )
	{
		FEStringListData& StringListItem = StringData(FieldIndex);

		out_FieldValue.PropertyTag = *FieldName;
		out_FieldValue.PropertyType = DATATYPE_Property;
		out_FieldValue.StringValue = StringListItem.CurrentValue;

		// fill in ArrayValue for lists.
		if ( out_FieldValue.StringValue.Len() > 0 )
		{
			INT ValueIndex = StringListItem.Strings.FindItemIndex(out_FieldValue.StringValue);
			if ( ValueIndex != INDEX_NONE )
			{
				out_FieldValue.ArrayValue.AddItem(ValueIndex);
			}
		}
		bResult = TRUE;
	}
	return bResult;
}

/**
 * Resolves the value of the data field specified and stores the value specified to the appropriate location for that field.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	FieldValue		the value to store for the property specified.
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUTUIDataStore_StringList::SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex )
{
	UBOOL bResult = FALSE;
	const INT FieldIndex = GetFieldIndex(FName(*FieldName));

	if ( StringData.IsValidIndex(FieldIndex) )
	{
		if ( FieldValue.ArrayValue.Num()==0 && FieldValue.StringValue.Len() )
		{
			StringData(FieldIndex).CurrentValue = FieldValue.StringValue;
			bResult = TRUE;
		}
		else if ( FieldValue.ArrayValue.Num() > 0 && StringData(FieldIndex).Strings.IsValidIndex(FieldValue.ArrayValue(0)) )
		{
			StringData(FieldIndex).CurrentValue = StringData(FieldIndex).Strings(FieldValue.ArrayValue(0));
			bResult = TRUE;
		}
	}
	return bResult;
}

/**
 * Adds a new field to the list
 *
 * @param	FieldName		the data field to resolve the value for
 * @param	NewString		The first string to add.
 * @param	bBatchOp		if TRUE, doesn't call RefreshSubscribers()
 * @returns the index of the new field
 */

INT UUTUIDataStore_StringList::AddNewField(FName FieldName, const FString &NewString, UBOOL bBatchOp/*=FALSE*/)
{
	FEStringListData* NewData = new(StringData) FEStringListData(EC_EventParm);

	NewData->Tag = FieldName;
	NewData->CurrentValue = NewString;
	new(NewData->Strings) FString(NewString);

	if ( !bBatchOp )
	{
		eventRefreshSubscribers(FieldName);
	}

	return StringData.Num() - 1;
}


/**
 * Add a string to the list
 *
 * @Param FieldName		The string list to work on
 * @Param NewString		The new string to add
 * @param bBatchOp		if TRUE, doesn't call RefreshSubscribers()
 */

void UUTUIDataStore_StringList::AddStr(FName FieldName, const FString &NewString, UBOOL bBatchOp/*=FALSE*/)
{
	INT FieldIndex = GetFieldIndex(FieldName);
	if ( StringData.IsValidIndex(FieldIndex) )
	{
		StringData(FieldIndex).Strings.AddItem(NewString);
		if ( StringData(FieldIndex).Strings.Num() == 1 )
		{
			StringData(FieldIndex).CurrentValue = NewString;
		}

		if ( !bBatchOp )
		{
			eventRefreshSubscribers(FieldName);
		}
	}
	else	// Create a new list and prime it
	{
		AddNewField(FieldName, NewString, bBatchOp);
	}
}

/**
 * Insert a string in to the list at a given index
 *
 * @Param FieldName		The string list to work on
 * @Param NewString		The new string to add
 * @Param InsertIndex	The index where you wish to insert the string
 * @param bBatchOp		if TRUE, doesn't call RefreshSubscribers()
 */

void UUTUIDataStore_StringList::InsertStr(FName FieldName, const FString &NewString, INT InsertIndex, UBOOL bBatchOp/*=FALSE*/)
{
	INT FieldIndex;
	
	// See if we can find this field.

	FieldIndex = GetFieldIndex(FieldName);

	if ( StringData.IsValidIndex(FieldIndex) )  // Found it, add the string
	{
		// Don't duplicate the strings

		INT StrIndex;
		if ( !StringData(FieldIndex).Strings.FindItem(NewString, StrIndex) )
		{
			StringData(FieldIndex).Strings.InsertItem(NewString, InsertIndex);
		}

		if ( !bBatchOp )
		{
			eventRefreshSubscribers(FieldName);
		}
	}
	else	// Create a new list and prime it
	{
		AddNewField(FieldName, NewString, bBatchOp);
	}
}



/**
 * Remove a string from the list
 *
 * @Param FieldName		The string list to work on
 * @Param StringToRemove 	The string to remove
 * @param bBatchOp		if TRUE, doesn't call RefreshSubscribers()
 */

void UUTUIDataStore_StringList::RemoveStr(FName FieldName, const FString &StringToRemove, UBOOL bBatchOp/*=FALSE*/)
{
	INT FieldIndex = GetFieldIndex(FieldName);
	if ( StringData.IsValidIndex(FieldIndex) )
	{
		StringData(FieldIndex).Strings.RemoveItem(StringToRemove);
	}

	if ( !bBatchOp )
	{
		eventRefreshSubscribers(FieldName);
	}
}

/**
 * Remove a string (or multiple strings) by the index.
 *
 * @Param FieldName		The string list to work on
 * @Param Index			The index to remove
 * @Param Count			<Optional> # of strings to remove
 * @param bBatchOp		if TRUE, doesn't call RefreshSubscribers()
 */

void UUTUIDataStore_StringList::RemoveStrByIndex(FName FieldName, INT Index, INT Count, UBOOL bBatchOp/*=FALSE*/)
{
	INT FieldIndex = GetFieldIndex(FieldName);
	if (StringData.IsValidIndex(FieldIndex)
	&&	StringData(FieldIndex).Strings.IsValidIndex(Index))
	{
		StringData(FieldIndex).Strings.Remove(Index, Count);
	}
	if ( !bBatchOp )
	{
		eventRefreshSubscribers(FieldName);
	}
}

/**
 * Empty a string List
 *
 * @Param FieldName		The string list to work on
 * @param bBatchOp		if TRUE, doesn't call RefreshSubscribers()
 */

void UUTUIDataStore_StringList::Empty(FName FieldName, UBOOL bBatchOp/*=FALSE*/)
{
	INT FieldIndex = GetFieldIndex(FieldName);
	if ( StringData.IsValidIndex(FieldIndex) )
	{
		StringData(FieldIndex).Strings.Empty();
	}

	if ( !bBatchOp )
	{
		eventRefreshSubscribers(FieldName);
	}
}

/**
 * Finds a string in the list
 *
 * @Param FieldName		The string list to add the new string to
 * @Param SearchStr		The string to find
 *
 * @returns the index in the list or INVALIDFIELD
 */

INT UUTUIDataStore_StringList::FindStr(FName FieldName, const FString &SearchString)
{
	INT Result = UCONST_INVALIDFIELD;

	const INT FieldIndex = GetFieldIndex(FieldName);
	if ( StringData.IsValidIndex(FieldIndex) )
	{	
		Result = StringData(FieldIndex).Strings.FindItemIndex(SearchString);
	}

	return Result;
}

/**
 * Returns the a string by the index
 *
 * @Param FieldName		The string list to add the new string to
 * @Param StrIndex		The index of the string to get
 *
 * @returns the string.
 */

FString UUTUIDataStore_StringList::GetStr(FName FieldName, INT StrIndex)
{
	FString Result;

	const INT FieldIndex = GetFieldIndex(FieldName);
	if (StringData.IsValidIndex(FieldIndex)
	&&	StringData(FieldIndex).Strings.IsValidIndex(StrIndex))
	{	
		Result = StringData(FieldIndex).Strings(StrIndex);
	}

	return Result;
}

/**
 * Get a list
 *
 * @Param FieldName		The string list to add the new string to
 * @returns a copy of the list
 */

TArray<FString> UUTUIDataStore_StringList::GetList(FName FieldName)
{
	INT FieldIndex = GetFieldIndex(FieldName);
	if ( StringData.IsValidIndex(FieldIndex) )
	{	
		return StringData(FieldIndex).Strings;		
	}
	else	// Create a new list and prime it
	{
		FieldIndex = AddNewField(FieldName, TEXT("") );
		return StringData(FieldIndex).Strings;
	}

	return TArray<FString>();
}


//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_StringArray
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_StringArray)


/** @return Returns the number of elements(rows) provided. */
INT UUTUIDataProvider_StringArray::GetElementCount()
{
	return Strings.Num();
}

// IUIListElement interface

/**
 * Returns the names of the exposed members in the first entry in the array
 * of search results
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param OutCellTags the columns supported by this row
 */
void UUTUIDataProvider_StringArray::GetElementCellTags( FName FieldName, TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Empty();
	OutCellTags.Set(TEXT("Strings"),TEXT("Strings"));
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the index of the value to fetch
 * @param	OutFieldValue	receives the resolved value for the property specified.
 * @param	ArrayIndex		ignored
 */
UBOOL UUTUIDataProvider_StringArray::GetCellFieldValue(FName FieldName, const FName& CellTag,INT ListIndex,FUIProviderFieldValue& OutFieldValue,INT)
{
	UBOOL bResult = FALSE;
	FOnlineStatsRow StatsRow(EC_EventParm);

	
	if(ListIndex < Strings.Num())
	{
		OutFieldValue.PropertyTag = CellTag;
		OutFieldValue.PropertyType = DATATYPE_Property;
		OutFieldValue.StringValue = Strings(ListIndex);
		bResult = TRUE;
	}

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
// UUTDataStore_OnlineStats
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTLeaderboardSettings);
IMPLEMENT_CLASS(UUTDataStore_OnlineStats);

/**
 * Loads and creates an instance of the registered filter object
 */
void UUTDataStore_OnlineStats::InitializeDataStore(void)
{
	Super::InitializeDataStore();

	// Create settings object
	LeaderboardSettings = ConstructObject<USettings>(LeaderboardSettingsClass);
	if (LeaderboardSettings != NULL)
	{
		SettingsProvider = ConstructObject<UUIDataProvider_Settings>(UUIDataProvider_Settings::StaticClass());
		if (SettingsProvider->BindSettings(LeaderboardSettings) == FALSE)
		{
			debugf(NAME_Error,TEXT("Failed to bind leaderboard filter settings object to %s"),
				*LeaderboardSettings->GetName());
		}
	}
	else
	{
		debugf(NAME_Error,TEXT("Failed to create leaderboard filter settings object %s"),
			*LeaderboardSettingsClass->GetName());
	}

	// Create stats details provider objects
	GeneralProvider = ConstructObject<UUTUIDataProvider_StatsGeneral>(UUTUIDataProvider_StatsGeneral::StaticClass(), this);
	WeaponsProvider = ConstructObject<UUTUIDataProvider_StatsWeapons>(UUTUIDataProvider_StatsWeapons::StaticClass(), this);
	VehiclesProvider = ConstructObject<UUTUIDataProvider_StatsVehicles>(UUTUIDataProvider_StatsVehicles::StaticClass(), this);
	VehicleWeaponsProvider = ConstructObject<UUTUIDataProvider_StatsVehicleWeapons>(UUTUIDataProvider_StatsVehicleWeapons::StaticClass(), this);
	RewardsProvider = ConstructObject<UUTUIDataProvider_StatsRewards>(UUTUIDataProvider_StatsRewards::StaticClass(), this);
}

/**
 * Returns the stats read results as a collection and appends the filter provider
 *
 * @param OutFields	out value that receives the list of exposed properties
 */
void UUTDataStore_OnlineStats::GetSupportedDataFields(TArray<FUIDataProviderField>& OutFields)
{
	Super::GetSupportedDataFields(OutFields);

	// Append our settings provider
	new(OutFields)FUIDataProviderField(FName(TEXT("Filters")),DATATYPE_Provider,SettingsProvider);

	// Append the stats providers
	new(OutFields)FUIDataProviderField(FName(TEXT("GeneralStats")),DATATYPE_Provider,GeneralProvider);
	new(OutFields)FUIDataProviderField(FName(TEXT("WeaponStats")),DATATYPE_Provider,WeaponsProvider);
	new(OutFields)FUIDataProviderField(FName(TEXT("VehicleStats")),DATATYPE_Provider,VehiclesProvider);
	new(OutFields)FUIDataProviderField(FName(TEXT("VehicleWeaponStats")),DATATYPE_Provider,VehicleWeaponsProvider);
	new(OutFields)FUIDataProviderField(FName(TEXT("RewardStats")),DATATYPE_Provider,RewardsProvider);
}

/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UUTDataStore_OnlineStats::GetElementProviderTags()
{
	TArray<FName> Result = Super::GetElementProviderTags();

	Result.AddItem(TEXT("GeneralStats"));
	Result.AddItem(TEXT("WeaponStats"));
	Result.AddItem(TEXT("VehicleStats"));
	Result.AddItem(TEXT("VehicleWeaponStats"));
	Result.AddItem(TEXT("RewardStats"));

	return Result;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UUTDataStore_OnlineStats::GetElementCount( FName FieldName )
{
	INT Result;
	UUTUIDataProvider_StatsElementProvider* StatsProvider = eventGetElementProviderFromName(FieldName);

	if(StatsProvider != NULL)
	{
		Result = StatsProvider->GetElementCount();
	}
	else
	{
		Result = Super::GetElementCount(FieldName);
	}

	return Result;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UUTDataStore_OnlineStats::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;
	UUTUIDataProvider_StatsElementProvider* StatsProvider = eventGetElementProviderFromName(FieldName);

	if(StatsProvider != NULL)
	{
		INT ElementCount = StatsProvider->GetElementCount();
		for(INT ElementIdx=0; ElementIdx<ElementCount; ElementIdx++)
		{
			out_Elements.AddItem(ElementIdx);
		}

		bResult = TRUE;
	}
	else
	{
		bResult = Super::GetListElements(FieldName, out_Elements);
	}

	return bResult;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	FieldName		the tag of the list element data field that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<class IUIListElementCellProvider> UUTDataStore_OnlineStats::GetElementCellSchemaProvider( FName FieldName )
{
	UUTUIDataProvider_StatsElementProvider* StatsProvider = eventGetElementProviderFromName(FieldName);

	if(StatsProvider != NULL)
	{
		return TScriptInterface<IUIListElementCellProvider>(StatsProvider);
	}
	else
	{
		return Super::GetElementCellSchemaProvider(FieldName);
	}

	return TScriptInterface<IUIListElementCellProvider>();
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 *
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<class IUIListElementCellProvider> UUTDataStore_OnlineStats::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	UUTUIDataProvider_StatsElementProvider* StatsProvider = eventGetElementProviderFromName(FieldName);

	if(StatsProvider != NULL)
	{
		return TScriptInterface<IUIListElementCellProvider>(StatsProvider);
	}
	else
	{
		return Super::GetElementCellValueProvider(FieldName, ListIndex);
	}

	return TScriptInterface<IUIListElementCellProvider>();
}

TScriptInterface<class IUIListElementProvider> UUTDataStore_OnlineStats::ResolveListElementProvider( const FString& PropertyName ) 
{
	UUTUIDataProvider_StatsElementProvider* StatsProvider = eventGetElementProviderFromName(*PropertyName);

	if(StatsProvider != NULL)
	{
		return TScriptInterface<IUIListElementProvider> (this);
	}
	else
	{
		// Make a copy because we potentially modify the string
		FString CompareName(PropertyName);
		FString ProviderName;
		// If there is an intervening provider name, snip it off
		if (ParseNextDataTag(CompareName,ProviderName) == FALSE)
		{
			CompareName = ProviderName;
		}
		// Check for the stats results
		if (FName(*CompareName) == StatsReadName)
		{
			return this;
		}
		// See if this is for one of our filters
		return SettingsProvider->ResolveListElementProvider(CompareName);
	}
}


//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_SimpleElementProvider
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_SimpleElementProvider)


/** @return Returns the number of elements(rows) provided. */
INT UUTUIDataProvider_SimpleElementProvider::GetElementCount()
{
	return 0;
}

// IUIListElement interface

/**
 * Returns the names of the exposed members in the first entry in the array
 * of search results
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param OutCellTags the columns supported by this row
 */
void UUTUIDataProvider_SimpleElementProvider::GetElementCellTags( FName FieldName, TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Empty();
}

/**
 * Retrieves the field type for the specified cell.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag				the tag for the element cell to get the field type for
 * @param	OutCellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
 *
 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
 */
UBOOL UUTUIDataProvider_SimpleElementProvider::GetCellFieldType(FName FieldName, const FName& CellTag,BYTE& OutCellFieldType)
{
	TMap<FName,FString> CellTags;
	GetElementCellTags(FieldName,CellTags);
	UBOOL bResult = FALSE;

	if(CellTags.Find(CellTag) != NULL)
	{
		OutCellFieldType = DATATYPE_Property;
		bResult = TRUE;
	}
	

	return bResult;
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the index of the value to fetch
 * @param	OutFieldValue	receives the resolved value for the property specified.
 * @param	ArrayIndex		ignored
 */
UBOOL UUTUIDataProvider_SimpleElementProvider::GetCellFieldValue(FName FieldName, const FName& CellTag,INT ListIndex,FUIProviderFieldValue& OutFieldValue,INT)
{
	return FALSE;
}


/* === UIDataProvider interface === */
/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUTUIDataProvider_SimpleElementProvider::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	out_Fields.Empty();

	TMap<FName,FString> Elements;
	GetElementCellTags(UCONST_UnknownCellDataFieldName,Elements);

	TArray<FName> ElementTags;
	Elements.GenerateKeyArray(ElementTags);

	for(INT TagIdx=0; TagIdx<ElementTags.Num(); TagIdx++)
	{
		// for each property contained by this ResourceDataProvider, add a provider field to expose them to the UI
		new(out_Fields) FUIDataProviderField( ElementTags(TagIdx), DATATYPE_Property );
	}
}

/**
 * Resolves the value of the data cell specified by FieldName and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUTUIDataProvider_SimpleElementProvider::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	FName PropertyFName(*FieldName, FNAME_Find);
	if ( PropertyFName != NAME_None )
	{
		bResult = GetCellFieldValue(UCONST_UnknownCellDataFieldName,PropertyFName,ArrayIndex,out_FieldValue,INDEX_NONE);
	}

	return bResult;
}




/* 
 * Given a string containing some amount of time in seconds, 
 * convert to a string of the form HHHH:MM:SS 
 */
FString ConvertSecondsToFormattedString(const FString& SecondsString)
{
	INT Hours = 0;
	INT Minutes = 0;
	INT SecondsToConvert = Clamp(appAtoi(*SecondsString), 0, 9999 * 3600 + 59 * 60 + 59); //Clamp to 9999:59:59

	if (SecondsToConvert > 0)
	{
		//Slice up the seconds
		Hours = SecondsToConvert / 3600;
		SecondsToConvert = SecondsToConvert % 3600;
		if (SecondsToConvert > 0)
		{
			Minutes = SecondsToConvert / 60;
			SecondsToConvert = SecondsToConvert % 60;
		}
	}
	   
	//debugf(TEXT("Converted %s to %s"), *SecondsString, *FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, SecondsToConvert));
	return FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, SecondsToConvert);
}

//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_StatsWeapons
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_StatsWeapons)


/** @return Returns the number of elements(rows) provided. */
INT UUTUIDataProvider_StatsWeapons::GetElementCount()
{
	return Stats.Num();
}

// IUIListElement interface

/**
 * Returns the names of the exposed members in the first entry in the array
 * of search results
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param OutCellTags the columns supported by this row
 */
void UUTUIDataProvider_StatsWeapons::GetElementCellTags( FName FieldName, TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Empty();

	OutCellTags.Set(TEXT("WeaponName"),*Localize(TEXT("Stats"), TEXT("WeaponName"), TEXT("UTGameUI")));
	OutCellTags.Set(TEXT("Kills"),*Localize(TEXT("Stats"), TEXT("Kills"), TEXT("UTGameUI")));
	OutCellTags.Set(TEXT("Deaths"),*Localize(TEXT("Stats"), TEXT("Deaths"), TEXT("UTGameUI")));
	OutCellTags.Set(TEXT("Suicides"),*Localize(TEXT("Stats"), TEXT("Suicides"), TEXT("UTGameUI")));
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the index of the value to fetch
 * @param	OutFieldValue	receives the resolved value for the property specified.
 * @param	ArrayIndex		ignored
 */
UBOOL UUTUIDataProvider_StatsWeapons::GetCellFieldValue(FName FieldName, const FName& CellTag,INT ListIndex,FUIProviderFieldValue& OutFieldValue,INT)
{
	UBOOL bResult = FALSE;
	FOnlineStatsRow StatsRow(EC_EventParm);

	if(GetStatsRow(StatsRow) && ListIndex < Stats.Num())
	{
		OutFieldValue.PropertyTag = CellTag;
		OutFieldValue.PropertyType = DATATYPE_Property;

		if(CellTag==TEXT("WeaponName"))
		{
			OutFieldValue.StringValue = *Localize(*Stats(ListIndex).WeaponName, TEXT("ItemName"), TEXT("UTGame"));
			bResult = TRUE;
		}
		else if(CellTag==TEXT("Kills"))
		{
			OutFieldValue.StringValue = GetColumnValue(StatsRow, Stats(ListIndex).KillsName);
			bResult = TRUE;
		}
		else if(CellTag==TEXT("Deaths"))
		{
			OutFieldValue.StringValue = GetColumnValue(StatsRow, Stats(ListIndex).DeathsName);
			bResult = TRUE;
		}
		else if(CellTag==TEXT("Suicides"))
		{
			OutFieldValue.StringValue = GetColumnValue(StatsRow, Stats(ListIndex).SuicidesName);
			bResult = TRUE;
		}
	}

	return bResult;
}



//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_StatsVehicles
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_StatsVehicles)


/** @return Returns the number of elements(rows) provided. */
INT UUTUIDataProvider_StatsVehicles::GetElementCount()
{
	return Stats.Num();
}

// IUIListElement interface

/**
 * Returns the names of the exposed members in the first entry in the array
 * of search results
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param OutCellTags the columns supported by this row
 */
void UUTUIDataProvider_StatsVehicles::GetElementCellTags( FName FieldName, TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Empty();

	OutCellTags.Set(TEXT("VehicleName"),*Localize(TEXT("Stats"), TEXT("VehicleName"), TEXT("UTGameUI")));
	OutCellTags.Set(TEXT("DrivingTime"),*Localize(TEXT("Stats"), TEXT("DrivingTime"), TEXT("UTGameUI")));
	OutCellTags.Set(TEXT("VehicleKills"),*Localize(TEXT("Stats"), TEXT("VehicleKills"), TEXT("UTGameUI")));
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the index of the value to fetch
 * @param	OutFieldValue	receives the resolved value for the property specified.
 * @param	ArrayIndex		ignored
 */
UBOOL UUTUIDataProvider_StatsVehicles::GetCellFieldValue(FName FieldName, const FName& CellTag,INT ListIndex,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	UBOOL bResult = FALSE;
	FOnlineStatsRow StatsRow(EC_EventParm);

	if(GetStatsRow(StatsRow) && ListIndex < Stats.Num())
	{
		OutFieldValue.PropertyTag = CellTag;
		OutFieldValue.PropertyType = DATATYPE_Property;

		if(CellTag==TEXT("VehicleName"))
		{
			OutFieldValue.StringValue = *Localize(*Stats(ListIndex).VehicleName, TEXT("VehicleNameString"), TEXT("UTGame"));
			bResult = TRUE;
		}
		else if(CellTag==TEXT("VehicleKills"))
		{
			OutFieldValue.StringValue =  GetColumnValue(StatsRow, Stats(ListIndex).VehicleKillsName);
			bResult = TRUE;
		}
		else if(CellTag==TEXT("DrivingTime"))
		{
			const FString& TimeString = GetColumnValue(StatsRow, Stats(ListIndex).DrivingTimeName);
			OutFieldValue.StringValue = ConvertSecondsToFormattedString(TimeString);
			bResult = TRUE;
		}
	}

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_StatsVehicleWeapons
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_StatsVehicleWeapons)


/** @return Returns the number of elements(rows) provided. */
INT UUTUIDataProvider_StatsVehicleWeapons::GetElementCount()
{
	return Stats.Num();
}

// IUIListElement interface

/**
 * Returns the names of the exposed members in the first entry in the array
 * of search results
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param OutCellTags the columns supported by this row
 */
void UUTUIDataProvider_StatsVehicleWeapons::GetElementCellTags( FName FieldName, TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Empty();

	OutCellTags.Set(TEXT("WeaponName"),*Localize(TEXT("Stats"), TEXT("WeaponName"), TEXT("UTGameUI")));
	OutCellTags.Set(TEXT("Kills"),*Localize(TEXT("Stats"), TEXT("Kills"), TEXT("UTGameUI")));
	OutCellTags.Set(TEXT("Deaths"),*Localize(TEXT("Stats"), TEXT("Deaths"), TEXT("UTGameUI")));
	OutCellTags.Set(TEXT("Suicides"),*Localize(TEXT("Stats"), TEXT("Suicides"), TEXT("UTGameUI")));
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the index of the value to fetch
 * @param	OutFieldValue	receives the resolved value for the property specified.
 * @param	ArrayIndex		ignored
 */
UBOOL UUTUIDataProvider_StatsVehicleWeapons::GetCellFieldValue(FName FieldName, const FName& CellTag,INT ListIndex,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	UBOOL bResult = FALSE;
	FOnlineStatsRow StatsRow(EC_EventParm);

	if(GetStatsRow(StatsRow) && ListIndex < Stats.Num())
	{
		OutFieldValue.PropertyTag = CellTag;
		OutFieldValue.PropertyType = DATATYPE_Property;

		if(CellTag==TEXT("WeaponName"))
		{
			OutFieldValue.StringValue = *Localize(TEXT("VehicleWeapons"), *Stats(ListIndex).WeaponName, TEXT("UTGameUI"));
			bResult = TRUE;
		}
		else if(CellTag==TEXT("Kills"))
		{
			OutFieldValue.StringValue = GetColumnValue(StatsRow, Stats(ListIndex).KillsName);
			bResult = TRUE;
		}
		else if(CellTag==TEXT("Deaths"))
		{
			OutFieldValue.StringValue = GetColumnValue(StatsRow, Stats(ListIndex).DeathsName);
			bResult = TRUE;
		}
		else if(CellTag==TEXT("Suicides"))
		{
			OutFieldValue.StringValue = GetColumnValue(StatsRow, Stats(ListIndex).SuicidesName);
			bResult = TRUE;
		}
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_StatsElementProvider
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_StatsElementProvider);

/**
 * Returns the value of a column given its ID and row
 *
 * @param StatsRow	Row to get the value from.
 * @param ColumnId	ColumnId to search for
 *
 * @return	The string value of the column.
 */
FString UUTUIDataProvider_StatsElementProvider::GetColumnValue(const FOnlineStatsRow &StatsRow, FName ColumnName)
{
	INT ColumnId = GetColumnIdFromStatName(ColumnName);

	FString Result = FString::Printf(TEXT("0"), *ColumnName.ToString(), ColumnId);
	if (ColumnId >= 0)
	{
		for(INT ColumnIdx=0; ColumnIdx<StatsRow.Columns.Num(); ColumnIdx++)
		{
			if(ColumnId==StatsRow.Columns(ColumnIdx).ColumnNo)
			{
				Result = StatsRow.Columns(ColumnIdx).StatValue.ToString();

				if(Result.Len()==0)
				{		
					Result=TEXT("0");
				}

				break;
			}
		}
	}

 	return Result;
}

/** 
 * Returns a reference to the stats row that is used to expose data.
 *
 * @param StatsRow	Object to hold the stats row.
 *
 * @return TRUE if a stats row was found, FALSE otherwise. 
 */
UBOOL UUTUIDataProvider_StatsElementProvider::GetStatsRow(FOnlineStatsRow& OutStatsRow)
{
	UBOOL bResult=FALSE;
	UUTDataStore_OnlineStats* StatsDataStore;
	UDataStoreClient* DataStoreClient = UUIInteraction::GetDataStoreClient();
	if (DataStoreClient != NULL)
	{
		StatsDataStore = Cast<UUTDataStore_OnlineStats>(DataStoreClient->FindDataStore(TEXT("UTLeaderboards")));
		bResult = FALSE;

		if(StatsDataStore != NULL && ReadObject != NULL && ReadObject->Rows.Num()>0)
		{
			INT ReadIdx = (StatsDataStore->DetailedStatsRowIndex < ReadObject->Rows.Num()) ? StatsDataStore->DetailedStatsRowIndex : 0;
			OutStatsRow = ReadObject->Rows(ReadIdx);
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
* Returns the localized name of a column given its ID
*
* @param ColumnId	ColumnId to search for
*
* @return	The localized name of the column.
*/
FString UUTUIDataProvider_StatsElementProvider::GetColumnName(FName StatName)
{
	FString Result = FString::Printf(TEXT(""), *StatName.ToString());

	for(INT ColumnIdx=0; ColumnIdx < ReadObject->ColumnMappings.Num(); ColumnIdx++)
	{
		if(ReadObject->ColumnMappings(ColumnIdx).Name == StatName)
		{
			Result = ReadObject->ColumnMappings(ColumnIdx).ColumnName;
			break;
		}
	}

	return Result;
}

/**
* Generates the column name used by the data provider, based on a view id and a column id
*
* @param StatsRead holds the definitions of the tables to read the data from
* @param ViewId the view to read from
* @param ColumnId the column to read.  If 0, then this is just the view's column (flag indicating if the view has been written to)
* @return the name of the field
*/
INT UUTUIDataProvider_StatsElementProvider::GetColumnIdFromStatName(FName StatName)
{
	INT ColumnId = -1;
	for (INT Index = 0; Index < ReadObject->ColumnMappings.Num(); Index++)
	{
		if(ReadObject->ColumnMappings(Index).Name == StatName)
		{
			ColumnId = ReadObject->ColumnMappings(Index).Id;
			check(ColumnId >= 0);
			break;
		}
	}
	return ColumnId;
}

//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_StatsGeneral
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_StatsGeneral)

/** @return Returns the number of elements(rows) provided. */
INT UUTUIDataProvider_StatsGeneral::GetElementCount()
{
	return Stats.Num();
}

// IUIListElement interface

/**
 * Returns the names of the exposed members in the first entry in the array
 * of search results
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param OutCellTags the columns supported by this row
 */
void UUTUIDataProvider_StatsGeneral::GetElementCellTags( FName FieldName, TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Empty();

	OutCellTags.Set(TEXT("StatName"),*Localize(TEXT("Stats"), TEXT("Statistic"), TEXT("UTGameUI")));
	OutCellTags.Set(TEXT("Value"),*Localize(TEXT("Stats"), TEXT("Value"), TEXT("UTGameUI")));
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the index of the value to fetch
 * @param	OutFieldValue	receives the resolved value for the property specified.
 * @param	ArrayIndex		ignored
 */
UBOOL UUTUIDataProvider_StatsGeneral::GetCellFieldValue(FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue,INT)
{
	UBOOL bResult = FALSE;
	FOnlineStatsRow StatsRow(EC_EventParm);

	if(GetStatsRow(StatsRow) && ListIndex < Stats.Num())
	{
		OutFieldValue.PropertyTag = CellTag;
		OutFieldValue.PropertyType = DATATYPE_Property;

		if(CellTag==TEXT("StatName"))
		{
			OutFieldValue.StringValue = GetColumnName(Stats(ListIndex).StatName);
			bResult = TRUE;
		}
		else if(CellTag==TEXT("Value"))
		{
			const FName& StatName = Stats(ListIndex).StatName;
			OutFieldValue.StringValue = GetColumnValue(StatsRow, StatName);
			if (StatName.ToString().InStr(TEXT("Time"), FALSE, TRUE, INDEX_NONE) >= 0)
			{
				OutFieldValue.StringValue = ConvertSecondsToFormattedString(OutFieldValue.StringValue);
			}
			bResult = TRUE;
		}
	}

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_StatsRewards
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_StatsRewards)


/** @return Returns the number of elements(rows) provided. */
INT UUTUIDataProvider_StatsRewards::GetElementCount()
{
	return Stats.Num();
}


//////////////////////////////////////////////////////////////////////////
// UUTUIDataStore_Content
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataStore_Content);

/**
 * Loads and creates an instance of the registered filter object
 */
void UUTUIDataStore_Content::InitializeDataStore(void)
{
	Super::InitializeDataStore();


	// Create content provider objects
	AvailableContentProvider = ConstructObject<UUTUIDataProvider_AvailableContent>(UUTUIDataProvider_AvailableContent::StaticClass(), this);
	InstalledContentProvider = ConstructObject<UUTUIDataProvider_InstalledContent>(UUTUIDataProvider_InstalledContent::StaticClass(), this);
}

/**
 * Returns the stats read results as a collection and appends the filter provider
 *
 * @param OutFields	out value that receives the list of exposed properties
 */
void UUTUIDataStore_Content::GetSupportedDataFields(TArray<FUIDataProviderField>& OutFields)
{
	Super::GetSupportedDataFields(OutFields);

	new(OutFields)FUIDataProviderField(FName(TEXT("AvailableContent")),DATATYPE_Provider,AvailableContentProvider);
	new(OutFields)FUIDataProviderField(FName(TEXT("InstalledContent")),DATATYPE_Provider,InstalledContentProvider);
}

/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UUTUIDataStore_Content::GetElementProviderTags()
{
	TArray<FName> Result;

	Result.AddItem(TEXT("AvailableContent"));
	Result.AddItem(TEXT("InstalledContent"));

	return Result;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UUTUIDataStore_Content::GetElementCount( FName FieldName )
{
	INT Result=0;
	UUTUIDataProvider_SimpleElementProvider* SimpleProvider = eventGetElementProviderFromName(FieldName);

	if(SimpleProvider != NULL)
	{
		Result = SimpleProvider->GetElementCount();
	}

	return Result;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UUTUIDataStore_Content::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;
	UUTUIDataProvider_SimpleElementProvider* SimpleProvider = eventGetElementProviderFromName(FieldName);

	if(SimpleProvider != NULL)
	{
		INT ElementCount = SimpleProvider->GetElementCount();
		for(INT ElementIdx=0; ElementIdx<ElementCount; ElementIdx++)
		{
			out_Elements.AddItem(ElementIdx);
		}

		bResult = TRUE;
	}

	return bResult;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	FieldName		the tag of the list element data field that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<class IUIListElementCellProvider> UUTUIDataStore_Content::GetElementCellSchemaProvider( FName FieldName )
{
	UUTUIDataProvider_SimpleElementProvider* SimpleProvider = eventGetElementProviderFromName(FieldName);

	if(SimpleProvider != NULL)
	{
		return TScriptInterface<IUIListElementCellProvider>(SimpleProvider);
	}

	return TScriptInterface<IUIListElementCellProvider>();
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 *
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<class IUIListElementCellProvider> UUTUIDataStore_Content::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	UUTUIDataProvider_SimpleElementProvider* SimpleProvider = eventGetElementProviderFromName(FieldName);

	if(SimpleProvider != NULL)
	{
		return TScriptInterface<IUIListElementCellProvider>(SimpleProvider);
	}

	return TScriptInterface<IUIListElementCellProvider>();
}

TScriptInterface<class IUIListElementProvider> UUTUIDataStore_Content::ResolveListElementProvider( const FString& PropertyName ) 
{
	UUTUIDataProvider_SimpleElementProvider* SimpleProvider = eventGetElementProviderFromName(*PropertyName);

	if(SimpleProvider != NULL)
	{
		return TScriptInterface<IUIListElementProvider> (this);
	}

	return TScriptInterface<IUIListElementProvider> ();
}


//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_AvailableContent
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_AvailableContent)

/** Parses a string for downloadable content. */
void UUTUIDataProvider_AvailableContent::ParseContentString(const FString &ContentStr)
{
	FString FileName;
	FString Description;
	TArray<FString> Lines;

	Packages.Empty();
	
	// Each content package is on a new line so split that first
	ContentStr.ParseIntoArray(&Lines, TEXT("\n"),TRUE);
	for(INT LineIdx=0; LineIdx<Lines.Num(); LineIdx++)
	{
		FString LineStr = Lines(LineIdx);
		TArray<FString> Sections;

		// Each content package consists of a set of sections separated by tabs so split on that.
		LineStr.ParseIntoArray(&Sections, TEXT("\t"), FALSE);

		if(Sections.Num()==3)
		{
			FAvailableContentPackage ContentPackage(EC_EventParm);
			ContentPackage.ContentName=Sections(0);
			ContentPackage.ContentFriendlyName=Sections(1);
			ContentPackage.ContentDescription=Sections(2);
			Packages.AddItem(ContentPackage);
		}
	}
}

/** @return Returns the number of elements(rows) provided. */
INT UUTUIDataProvider_AvailableContent::GetElementCount()
{
	return Packages.Num();
}

// IUIListElement interface

/**
 * Returns the names of the exposed members in the first entry in the array
 * of search results
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param OutCellTags the columns supported by this row
 */
void UUTUIDataProvider_AvailableContent::GetElementCellTags( FName FieldName, TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Empty();

	OutCellTags.Set(TEXT("FileName"),*Localize(TEXT("Content"), TEXT("FileName"), TEXT("UTGameUI")));
	OutCellTags.Set(TEXT("FriendlyName"),*Localize(TEXT("Content"), TEXT("Name"), TEXT("UTGameUI")));
	OutCellTags.Set(TEXT("Description"),*Localize(TEXT("Content"), TEXT("Description"), TEXT("UTGameUI")));
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the index of the value to fetch
 * @param	OutFieldValue	receives the resolved value for the property specified.
 * @param	ArrayIndex		ignored
 */
UBOOL UUTUIDataProvider_AvailableContent::GetCellFieldValue(FName FieldName, const FName& CellTag,INT ListIndex,FUIProviderFieldValue& OutFieldValue,INT)
{
	UBOOL bResult = FALSE;
	FOnlineStatsRow StatsRow(EC_EventParm);

	if(ListIndex < Packages.Num())
	{
		OutFieldValue.PropertyTag = CellTag;
		OutFieldValue.PropertyType = DATATYPE_Property;

		if(CellTag==TEXT("FileName"))
		{
			OutFieldValue.StringValue = Packages(ListIndex).ContentName;
			bResult = TRUE;
		}
		else if(CellTag==TEXT("FriendlyName"))
		{
			OutFieldValue.StringValue =  Packages(ListIndex).ContentFriendlyName;
			bResult = TRUE;
		}
		else if(CellTag==TEXT("Description"))
		{
			OutFieldValue.StringValue =  Packages(ListIndex).ContentDescription;
			bResult = TRUE;
		}
	}

	return bResult;
}



//////////////////////////////////////////////////////////////////////////
// UUTUIDataProvider_InstalledContent
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIDataProvider_InstalledContent)


/** @return Returns the number of elements(rows) provided. */
INT UUTUIDataProvider_InstalledContent::GetElementCount()
{
	TArray<FString> Packages = GDownloadableContent->GetDownloadableContentList();
	return Packages.Num();
}

// IUIListElement interface

/**
 * Returns the names of the exposed members in the first entry in the array
 * of search results
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param OutCellTags the columns supported by this row
 */
void UUTUIDataProvider_InstalledContent::GetElementCellTags( FName FieldName, TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Empty();

	OutCellTags.Set(TEXT("FileName"),*Localize(TEXT("Content"), TEXT("FileName"), TEXT("UTGameUI")));
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the index of the value to fetch
 * @param	OutFieldValue	receives the resolved value for the property specified.
 * @param	ArrayIndex		ignored
 */
UBOOL UUTUIDataProvider_InstalledContent::GetCellFieldValue(FName FieldName, const FName& CellTag,INT ListIndex,FUIProviderFieldValue& OutFieldValue,INT)
{
	UBOOL bResult = FALSE;
	FOnlineStatsRow StatsRow(EC_EventParm);
	TArray<FString> Packages = GDownloadableContent->GetDownloadableContentList();

	if(ListIndex < Packages.Num())
	{
		OutFieldValue.PropertyTag = CellTag;
		OutFieldValue.PropertyType = DATATYPE_Property;

		if(CellTag==TEXT("FileName"))
		{
			OutFieldValue.StringValue = Packages(ListIndex);
			bResult = TRUE;
		}
	}

	return bResult;
}









