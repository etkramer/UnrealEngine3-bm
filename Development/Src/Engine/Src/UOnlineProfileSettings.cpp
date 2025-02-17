/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "EnginePrivate.h"

IMPLEMENT_CLASS(UOnlineProfileSettings);

/**
 * Searches the profile setting array for the matching string setting name and returns the id
 *
 * @param ProfileSettingName the name of the profile setting being searched for
 * @param ProfileSettingId the id of the context that matches the name
 *
 * @return true if the seting was found, false otherwise
 */
UBOOL UOnlineProfileSettings::GetProfileSettingId(FName ProfileSettingName,
	INT& ProfileSettingId)
{
	// Search for the profile setting name in the mappings and return its id
	for (INT Index = 0; Index < ProfileMappings.Num(); Index++)
	{
		const FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(Index);
		if (MetaData.Name == ProfileSettingName)
		{
			ProfileSettingId = MetaData.Id;
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Finds the human readable name for the profile setting
 *
 * @param ProfileSettingId the id to look up in the mappings table
 *
 * @return the name of the string setting that matches the id or NAME_None if not found
 */
FName UOnlineProfileSettings::GetProfileSettingName(INT ProfileSettingId)
{
	// Search for the string setting id in the mappings and return its name
	for (INT Index = 0; Index < ProfileMappings.Num(); Index++)
	{
		const FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(Index);
		// If this is the ID we are looking for
		if (MetaData.Id == ProfileSettingId)
		{
			return MetaData.Name;
		}
	}
	return NAME_None;
}

/**
 * Finds the localized column header text for the profile setting
 *
 * @param ProfileSettingId the id to look up in the mappings table
 *
 * @return the string to use as the list column header for the profile setting that matches the id, or an empty string if not found.
 */
FString UOnlineProfileSettings::GetProfileSettingColumnHeader( INT ProfileSettingId )
{
	FString Result;
	// Search for the string setting id in the mappings and return its localized column header
	for (INT Index = 0; Index < ProfileMappings.Num(); Index++)
	{
		const FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(Index);
		// If this is the ID we are looking for
		if (MetaData.Id == ProfileSettingId)
		{
			Result = MetaData.ColumnHeaderText;
			break;
		}
	}
	return Result;
}

/**
 * Finds the index of an OnlineProfileSetting struct given its settings id.
 *
 * @param	ProfileSettingId	the id of the struct to search for
 *
 * @return	the index into the ProfileSettings array for the struct with the matching id.
 */
INT UOnlineProfileSettings::FindProfileSettingIndex( INT ProfileSettingId ) const
{
	INT Result = INDEX_NONE;

	for (INT SettingsIndex = 0; SettingsIndex < ProfileSettings.Num(); SettingsIndex++)
	{
		const FOnlineProfileSetting& Setting = ProfileSettings(SettingsIndex);
		if (Setting.ProfileSetting.PropertyId == ProfileSettingId)
		{
			Result = SettingsIndex;
			break;
		}
	}

	return Result;
}

/**
 * Finds the index of SettingsPropertyPropertyMetaData struct, given its settings id. 
 *
 * @param	ProfileSettingId	the id of the struct to search for
 *
 * @return	the index into the ProfileMappings array for the struct with the matching id.
 */
INT UOnlineProfileSettings::FindProfileMappingIndex( INT ProfileSettingId ) const
{
	INT Result = INDEX_NONE;

	// Search for the string setting id in the mappings
	for (INT MappingIndex = 0; MappingIndex < ProfileMappings.Num(); MappingIndex++)
	{
		const FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(MappingIndex);
		if (MetaData.Id == ProfileSettingId)
		{
			Result = MappingIndex;
			break;
		}
	}

	return Result;
}

/**
 * Finds the index of SettingsPropertyPropertyMetaData struct, given its settings name. 
 *
 * @param	ProfileSettingId	the id of the struct to search for
 *
 * @return	the index into the ProfileMappings array for the struct with the matching name.
 */
INT UOnlineProfileSettings::FindProfileMappingIndexByName( FName ProfileSettingName ) const
{
	INT Result = INDEX_NONE;

	// Search for the string setting id in the mappings
	for (INT MappingIndex = 0; MappingIndex < ProfileMappings.Num(); MappingIndex++)
	{
		const FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(MappingIndex);
		if (MetaData.Name == ProfileSettingName)
		{
			Result = MappingIndex;
			break;
		}
	}

	return Result;
}

/**
 * Determines if the setting is id mapped or not
 *
 * @param ProfileSettingId the id to look up in the mappings table
 *
 * @return TRUE if the setting is id mapped, FALSE if it is a raw value
 */
UBOOL UOnlineProfileSettings::IsProfileSettingIdMapped(INT ProfileSettingId)
{
	// Search for the string setting id in the mappings and return its name
	for (INT Index = 0; Index < ProfileMappings.Num(); Index++)
	{
		const FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(Index);
		// If this is the ID we are looking for
		if (MetaData.Id == ProfileSettingId)
		{
			return MetaData.MappingType == PVMT_IdMapped;
		}
	}
	return FALSE;
}

/**
 * Finds the human readable name for a profile setting's value. Searches the
 * profile settings mappings for the specifc profile setting and then searches
 * the set of values for the specific value index and returns that value's
 * human readable name
 *
 * @param Value the out param that gets the value copied to it
 * @param ValueMapID optional parameter that allows you to select a specific index in the ValueMappings instead
 * of automatically using the currently set index (if -1 is passed in, which is the default, it means to just
 * use the set index
 *
 * @return true if found, false otherwise
 */
UBOOL UOnlineProfileSettings::GetProfileSettingValue(INT ProfileSettingId,FString& Value,INT ValueMapID)
{
	// Search for the profile setting id in the mappings
	for (INT Index = 0; Index < ProfileMappings.Num(); Index++)
	{
		const FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(Index);
		if (MetaData.Id == ProfileSettingId)
		{
			// Find the profile setting that matches this id
			for (INT Index2 = 0; Index2 < ProfileSettings.Num(); Index2++)
			{
				const FOnlineProfileSetting& Setting = ProfileSettings(Index2);
				if (Setting.ProfileSetting.PropertyId == ProfileSettingId)
				{
					// If this is ID mapped, then find the ID
					if (MetaData.MappingType == PVMT_IdMapped)
					{
						INT ValueIndex;
						// Use ValueMapID if one was passed in, else use the current value in the setting
						if ( ValueMapID >= 0 )
						{
							ValueIndex = ValueMapID;
						}
						else
						{
							// Read the index so we can find its name
							Setting.ProfileSetting.Data.GetData(ValueIndex);
						}
						// Now search for the value index mapping
						for (INT Index3 = 0; Index3 < MetaData.ValueMappings.Num(); Index3++)
						{
							const FIdToStringMapping& Mapping = MetaData.ValueMappings(Index3);
							if (Mapping.Id == ValueIndex)
							{
								Value = Mapping.Name.ToString();
								return TRUE;
							}
						}
					}
					else
					{
						// This item should be treated as a string
						Value = Setting.ProfileSetting.Data.ToString();
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}

/**
 * Finds the human readable name for a profile setting's value. Searches the
 * profile settings mappings for the specifc profile setting and then searches
 * the set of values for the specific value index and returns that value's
 * human readable name
 *
 * @param ProfileSettingId the id to find the value's name
 *
 * @return the name of the value or NAME_None if not value mapped
 */
FName UOnlineProfileSettings::GetProfileSettingValueName(INT ProfileSettingId)
{
	// Search for the profile setting id in the mappings
	for (INT Index = 0; Index < ProfileMappings.Num(); Index++)
	{
		const FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(Index);
		if (MetaData.Id == ProfileSettingId)
		{
			// Find the profile setting that matches this id
			for (INT Index2 = 0; Index2 < ProfileSettings.Num(); Index2++)
			{
				const FOnlineProfileSetting& Setting = ProfileSettings(Index2);
				if (Setting.ProfileSetting.PropertyId == ProfileSettingId)
				{
					// If this is ID mapped, then find the ID
					if (MetaData.MappingType == PVMT_IdMapped)
					{
						INT ValueIndex;
						// Read the index so we can find its name
						Setting.ProfileSetting.Data.GetData(ValueIndex);
						// Now search for the value index mapping
						for (INT Index3 = 0; Index3 < MetaData.ValueMappings.Num(); Index3++)
						{
							const FIdToStringMapping& Mapping = MetaData.ValueMappings(Index3);
							if (Mapping.Id == ValueIndex)
							{
								return Mapping.Name;
							}
						}
					}
					else
					{
						return NAME_None;
					}
				}
			}
		}
	}
	return NAME_None;
}

/**
 * Searches the profile settings mappings for the specifc profile setting and
 * then adds all of the possible values to the out parameter
 *
 * @param ProfileSettingId the id to look up in the mappings table
 * @param Values the out param that gets the list of values copied to it
 *
 * @return true if found and value mapped, false otherwise
 */
UBOOL UOnlineProfileSettings::GetProfileSettingValues(INT ProfileSettingId,TArray<FName>& Values)
{
	// Search for the profile setting id in the mappings
	for (INT Index = 0; Index < ProfileMappings.Num(); Index++)
	{
		const FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(Index);
		if (MetaData.Id == ProfileSettingId)
		{
			// If this is ID mapped, then find the ID
			if (MetaData.MappingType == PVMT_IdMapped)
			{
				// Add each name mapped value to the array
				for (INT Index2 = 0; Index2 < MetaData.ValueMappings.Num(); Index2++)
				{
					Values.AddItem(MetaData.ValueMappings(Index2).Name);
				}
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}
	return FALSE;
}

/**
 * Finds the human readable value for a profile setting. Searches the
 * profile settings mappings for the specific profile setting and then searches
 * the set of values for the specific value index and returns that value's
 * human readable name
 *
 * @param ProfileSettingName the name of the profile setting to find the string value of
 * @param Value the out param that gets the value copied to it
 *
 * @return true if found, false otherwise
 */
UBOOL UOnlineProfileSettings::GetProfileSettingValueByName(FName ProfileSettingName,
	FString& Value)
{
	INT ProfileSettingId;
	// See if this is a known string setting
	if (GetProfileSettingId(ProfileSettingName,ProfileSettingId))
	{
		return GetProfileSettingValue(ProfileSettingId,Value);
	}
	return FALSE;
}

/**
 * Searches for the profile setting by name and sets the value index to the
 * value contained in the profile setting meta data
 *
 * @param ProfileSettingName the name of the profile setting to find
 * @param NewValue the string value to use
 *
 * @return true if the profile setting was found and the value was set, false otherwise
 */
UBOOL UOnlineProfileSettings::SetProfileSettingValueByName(FName ProfileSettingName,
	const FString& NewValue)
{
	INT ProfileSettingId;
	// See if this is a known string setting
	if (GetProfileSettingId(ProfileSettingName,ProfileSettingId))
	{
		return SetProfileSettingValue(ProfileSettingId,NewValue);
	}
	return FALSE;
}

/**
 * Searches for the profile setting by name and sets the value index to the
 * value contained in the profile setting meta data
 *
 * @param ProfileSettingName the name of the profile setting to set the string value of
 * @param NewValue the string value to use
 *
 * @return true if the profile setting was found and the value was set, false otherwise
 */
UBOOL UOnlineProfileSettings::SetProfileSettingValue(INT ProfileSettingId,
	const FString& NewValue)
{
	// Search for the profile setting id in the mappings
	for (INT Index = 0; Index < ProfileMappings.Num(); Index++)
	{
		const FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(Index);
		if (MetaData.Id == ProfileSettingId)
		{
			// Find the profile setting that matches this id
			for (INT Index2 = 0; Index2 < ProfileSettings.Num(); Index2++)
			{
				FOnlineProfileSetting& Setting = ProfileSettings(Index2);
				if (Setting.ProfileSetting.PropertyId == ProfileSettingId)
				{
					// If this is ID mapped, then find the ID
					if (MetaData.MappingType == PVMT_IdMapped)
					{
						FName ValueName(*NewValue);
						// Now search for the value index mapping
						for (INT Index3 = 0; Index3 < MetaData.ValueMappings.Num(); Index3++)
						{
							const FIdToStringMapping& Mapping = MetaData.ValueMappings(Index3);
							if (Mapping.Name == ValueName)
							{
								// Set the value based off the ID for this mapping
								const UBOOL bValueChanged = Setting.ProfileSetting.Data != Mapping.Id;
								Setting.ProfileSetting.Data.SetData(Mapping.Id);
								if ( bValueChanged && DELEGATE_IS_SET(NotifySettingValueUpdated) )
								{
									delegateNotifySettingValueUpdated(MetaData.Name);
								}
								return TRUE;
							}
						}
					}
					else
					{
						// This item should be treated as a string
						const UBOOL bValueChanged = Setting.ProfileSetting.Data != NewValue;
						Setting.ProfileSetting.Data.FromString(NewValue);
						if ( bValueChanged && DELEGATE_IS_SET(NotifySettingValueUpdated) )
						{
							delegateNotifySettingValueUpdated(MetaData.Name);
						}
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}

/**
 * Searches for the profile setting by id and gets the value index
 *
 * @param ProfileSettingId the id of the profile setting to return
 * @param ValueId the out value of the id
 * @param ListIndex the out value of the index where that value lies in the ValueMappings list
 *
 * @return true if the profile setting was found and id mapped, false otherwise
 */
UBOOL UOnlineProfileSettings::GetProfileSettingValueId(INT ProfileSettingId,INT& ValueId,INT* ListIndex)
{
	// Search for the profile setting id in the mappings
	for (INT Index = 0; Index < ProfileMappings.Num(); Index++)
	{
		const FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(Index);
		if (MetaData.Id == ProfileSettingId)
		{
			// Find the profile setting that matches this id
			for (INT Index2 = 0; Index2 < ProfileSettings.Num(); Index2++)
			{
				FOnlineProfileSetting& Setting = ProfileSettings(Index2);
				if (Setting.ProfileSetting.PropertyId == ProfileSettingId)
				{
					// If this is ID mapped, then read the ID
					if (MetaData.MappingType == PVMT_IdMapped)
					{
						Setting.ProfileSetting.Data.GetData(ValueId);

						// If a ListIndex is requested, look for it in the ValueMappings array
						if ( ListIndex )
						{
							// Find that value in the ValueMappings list
							for (INT MapIdx = 0; MapIdx < MetaData.ValueMappings.Num(); MapIdx++)
							{
								if (MetaData.ValueMappings(MapIdx).Id == ValueId)
								{
									*ListIndex = MapIdx;
									break;
								}
							}
						}
						return TRUE;
					}
					else
					{
						warnf( TEXT( "GetProfileSettingValueId did not find a valid MappingType.  ProfileSettingId: %d ValueId: %d" ), ProfileSettingId, ValueId );
						return FALSE;
					}
				}
			}
		}
	}
	return FALSE;
}

/**
 * Searches for the profile setting by id and gets the value index
 *
 * @param ProfileSettingId the id of the profile setting to return
 * @param Value the out value of the setting
 *
 * @return true if the profile setting was found and not id mapped, false otherwise
 */
UBOOL UOnlineProfileSettings::GetProfileSettingValueInt(INT ProfileSettingId,INT& Value)
{
	// Search for the profile setting id in the mappings
	for (INT Index = 0; Index < ProfileMappings.Num(); Index++)
	{
		const FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(Index);
		if (MetaData.Id == ProfileSettingId)
		{
			// Find the profile setting that matches this id
			for (INT Index2 = 0; Index2 < ProfileSettings.Num(); Index2++)
			{
				FOnlineProfileSetting& Setting = ProfileSettings(Index2);
				if (Setting.ProfileSetting.PropertyId == ProfileSettingId)
				{
					// If this is a raw value, then read it
					if (MetaData.MappingType == PVMT_RawValue)
					{
						Setting.ProfileSetting.Data.GetData(Value);
						return TRUE;
					}
					else
					{
						warnf( TEXT( "GetProfileSettingValueInt did not find a valid MappingType.  ProfileSettingId: %d Value: %d" ), ProfileSettingId, Value );
						return FALSE;
					}
				}
			}
		}
	}
	return FALSE;
}

/**
 * Searches for the profile setting by id and gets the value index
 *
 * @param ProfileSettingId the id of the profile setting to return
 * @param Value the out value of the setting
 *
 * @return true if the profile setting was found and not id mapped, false otherwise
 */
UBOOL UOnlineProfileSettings::GetProfileSettingValueFloat(INT ProfileSettingId,FLOAT& Value)
{
	// Search for the profile setting id in the mappings
	for (INT Index = 0; Index < ProfileMappings.Num(); Index++)
	{
		const FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(Index);
		if (MetaData.Id == ProfileSettingId)
		{
			// Find the profile setting that matches this id
			for (INT Index2 = 0; Index2 < ProfileSettings.Num(); Index2++)
			{
				FOnlineProfileSetting& Setting = ProfileSettings(Index2);
				if (Setting.ProfileSetting.PropertyId == ProfileSettingId)
				{
					// If this is a raw value, then read it
					if (MetaData.MappingType == PVMT_RawValue)
					{
						Setting.ProfileSetting.Data.GetData(Value);
						return TRUE;
					}
					else
					{
						warnf( TEXT( "GetProfileSettingValueFloat did not find a valid MappingType.  ProfileSettingId: %d ValueId: %d" ), ProfileSettingId, Value );
						return FALSE;
					}
				}
			}
		}
	}
	return FALSE;
}


/**
 * Searches for the profile setting by id and sets the value
 *
 * @param ProfileSettingId the id of the profile setting to return
 * @param Value the new value of the setting
 *
 * @return true if the profile setting was found and not id mapped, false otherwise
 */
UBOOL UOnlineProfileSettings::SetProfileSettingValueId(INT ProfileSettingId,INT Value)
{
	// Search for the profile setting id in the mappings
	for (INT Index = 0; Index < ProfileMappings.Num(); Index++)
	{
		const FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(Index);
		if (MetaData.Id == ProfileSettingId)
		{
			// Find the profile setting that matches this id
			for (INT Index2 = 0; Index2 < ProfileSettings.Num(); Index2++)
			{
				FOnlineProfileSetting& Setting = ProfileSettings(Index2);
				if (Setting.ProfileSetting.PropertyId == ProfileSettingId)
				{
					// If this is a raw value, then read it
					if (MetaData.MappingType == PVMT_IdMapped)
					{
						const UBOOL bValueChanged = Setting.ProfileSetting.Data != Value;
						Setting.ProfileSetting.Data.SetData(Value);
						if ( bValueChanged && DELEGATE_IS_SET(NotifySettingValueUpdated) )
						{
							delegateNotifySettingValueUpdated(MetaData.Name);
						}
						return TRUE;
					}
					else
					{
						warnf( TEXT( "SetProfileSettingValueId did not find a valid MappingType.  ProfileSettingId: %d Value: %d" ), ProfileSettingId, Value );
						return FALSE;
					}
				}
			}
		}
	}
	return FALSE;
}

/**
 * Searches for the profile setting by id and sets the value
 *
 * @param ProfileSettingId the id of the profile setting to return
 * @param Value the new value of the setting
 *
 * @return true if the profile setting was found and not id mapped, false otherwise
 */
UBOOL UOnlineProfileSettings::SetProfileSettingValueInt(INT ProfileSettingId,INT Value)
{
	return SetProfileSettingTypedValue<INT>(ProfileSettingId,Value);
}

/**
 * Searches for the profile setting by id and sets the value
 *
 * @param ProfileSettingId the id of the profile setting to return
 * @param Value the new value of the setting
 *
 * @return true if the profile setting was found and not id mapped, false otherwise
 */
UBOOL UOnlineProfileSettings::SetProfileSettingValueFloat(INT ProfileSettingId,FLOAT Value)
{
	return SetProfileSettingTypedValue<FLOAT>(ProfileSettingId,Value);
}

/**
 * Sets all of the profile settings to their default values
 */
void UOnlineProfileSettings::SetToDefaults(void)
{
	ProfileSettings.Empty();
	// Add all of the defaults for this user
	for (INT Index = 0; Index < DefaultSettings.Num(); Index++)
	{
		// Copy the default into that user's settings
		INT AddIndex = ProfileSettings.AddZeroed();
		// Deep copy the data
		ProfileSettings(AddIndex) = DefaultSettings(Index);
	}
	// Now append the version
	AppendVersionToSettings();

	if ( DELEGATE_IS_SET(NotifySettingValueUpdated) )
	{
		delegateNotifySettingValueUpdated(GetProfileSettingName(NAME_None));
	}
}

/**
 * Searches for the profile setting by id and gets the default value index
 *
 * @param ProfileSettingId the id of the profile setting to return
 * @param DefaultId the out value of the default id
 * @param ListIndex the out value of the index where that value lies in the ValueMappings list
 *
 * @return true if the profile setting was found and retrieved the default id, false otherwise
 */
UBOOL UOnlineProfileSettings::GetProfileSettingDefaultId(INT ProfileSettingId, INT& DefaultId, INT& ListIndex)
{
	// Search for the profile setting id in the mappings
	for (INT Index = 0; Index < ProfileMappings.Num(); Index++)
	{
		const FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(Index);
		if (MetaData.Id == ProfileSettingId)
		{
			// Find the default setting that matches this id
			for (INT Index2 = 0; Index2 < DefaultSettings.Num(); Index2++)
			{
				FOnlineProfileSetting& Setting = DefaultSettings(Index2);
				if (Setting.ProfileSetting.PropertyId == ProfileSettingId)
				{
					// If this is ID mapped, then read the ID
					if (MetaData.MappingType == PVMT_IdMapped)
					{
						Setting.ProfileSetting.Data.GetData(DefaultId);

						// Found the default value, now find that value in the ValueMappings list
						for (INT MapIdx = 0; MapIdx < MetaData.ValueMappings.Num(); MapIdx++)
						{
							if (MetaData.ValueMappings(MapIdx).Id == DefaultId)
							{
								ListIndex = MapIdx;
								break;
							}
						}
						return TRUE;
					}
					else
					{
						warnf( TEXT( "GetProfileSettingDefaultId did not find a valid MappingType.  ProfileSettingId: %d DefaultId: %d" ), ProfileSettingId, DefaultId );
						return FALSE;
					}
				}
			}
		}
	}
	return FALSE;
}

/**
 * Searches for the profile setting by id and gets the default value int
 *
 * @param ProfileSettingId the id of the profile setting to return the default of
 * @param Value the out value of the default setting
 *
 * @return true if the profile setting was found and retrieved the default int, false otherwise
 */
UBOOL UOnlineProfileSettings::GetProfileSettingDefaultInt(INT ProfileSettingId, INT& DefaultInt)
{
	// Search for the profile setting id in the mappings
	for (INT Index = 0; Index < ProfileMappings.Num(); Index++)
	{
		const FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(Index);
		if (MetaData.Id == ProfileSettingId)
		{
			// Find the default setting that matches this id
			for (INT Index2 = 0; Index2 < DefaultSettings.Num(); Index2++)
			{
				FOnlineProfileSetting& Setting = DefaultSettings(Index2);
				if (Setting.ProfileSetting.PropertyId == ProfileSettingId)
				{
					// If this is a raw value, then read it
					if (MetaData.MappingType == PVMT_RawValue)
					{
						Setting.ProfileSetting.Data.GetData(DefaultInt);
						return TRUE;
					}
					else
					{
						warnf( TEXT( "GetProfileSettingDefaultInt did not find a valid MappingType.  ProfileSettingId: %d DefaultInt: %d" ), ProfileSettingId, DefaultInt );
						return FALSE;
					}
				}
			}
		}
	}
	return FALSE;
}

/**
 * Searches for the profile setting by id and gets the default value float
 *
 * @param ProfileSettingId the id of the profile setting to return the default of
 * @param Value the out value of the default setting
 *
 * @return true if the profile setting was found and retrieved the default float, false otherwise
 */
UBOOL UOnlineProfileSettings::GetProfileSettingDefaultFloat(INT ProfileSettingId, FLOAT& DefaultFloat)
{
	// Search for the profile setting id in the mappings
	for (INT Index = 0; Index < ProfileMappings.Num(); Index++)
	{
		const FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(Index);
		if (MetaData.Id == ProfileSettingId)
		{
			// Find the default setting that matches this id
			for (INT Index2 = 0; Index2 < DefaultSettings.Num(); Index2++)
			{
				FOnlineProfileSetting& Setting = DefaultSettings(Index2);
				if (Setting.ProfileSetting.PropertyId == ProfileSettingId)
				{
					// If this is a raw value, then read it
					if (MetaData.MappingType == PVMT_RawValue)
					{
						Setting.ProfileSetting.Data.GetData(DefaultFloat);
						return TRUE;
					}
					else
					{
						warnf( TEXT( "GetProfileSettingDefaultFloat did not find a valid MappingType.  ProfileSettingId: %d DefaultInt: %d" ), ProfileSettingId, DefaultFloat );
						return FALSE;
					}
				}
			}
		}
	}
	return FALSE;
}

/**
 * Adds the version id to the read ids if it is not present
 */
void UOnlineProfileSettings::AppendVersionToReadIds(void)
{
	UBOOL bFound = FALSE;
	// Search the read ids for the version number
	for (INT Index = 0; Index < ProfileSettingIds.Num(); Index++)
	{
		if (ProfileSettingIds(Index) == PSI_ProfileVersionNum)
		{
			bFound = TRUE;
			break;
		}
	}
	// Add the version id if missing
	if (bFound == FALSE)
	{
		ProfileSettingIds.AddItem(PSI_ProfileVersionNum);
	}
}

/**
 * Adds the version number to the read data if not present
 */
void UOnlineProfileSettings::AppendVersionToSettings(void)
{
	UBOOL bFound = FALSE;
	// Search the read ids for the version number
	for (INT Index = 0; Index < ProfileSettings.Num(); Index++)
	{
		FOnlineProfileSetting& Setting = ProfileSettings(Index);
		if (Setting.ProfileSetting.PropertyId == PSI_ProfileVersionNum)
		{
			const UBOOL bValueChanged = Setting.ProfileSetting.Data != VersionNumber;
			Setting.ProfileSetting.Data.SetData(VersionNumber);
			if ( bValueChanged && DELEGATE_IS_SET(NotifySettingValueUpdated) )
			{
				delegateNotifySettingValueUpdated(GetProfileSettingName(PSI_ProfileVersionNum));
			}
			bFound = TRUE;
			break;
		}
	}
	// Add the version number if missing
	if (bFound == FALSE)
	{
		INT AddIndex = ProfileSettings.AddZeroed();
		FOnlineProfileSetting& Setting = ProfileSettings(AddIndex);
		// Copy over the data
		Setting.Owner = OPPO_Game;
		Setting.ProfileSetting.PropertyId = PSI_ProfileVersionNum;
		Setting.ProfileSetting.Data.SetData(VersionNumber);

		if ( DELEGATE_IS_SET(NotifySettingValueUpdated) )
		{
			delegateNotifySettingValueUpdated(GetProfileSettingName(PSI_ProfileVersionNum));
		}
	}
}

/** Returns the version number that was found in the profile read results */
INT UOnlineProfileSettings::GetVersionNumber(void)
{
	// Set to an invalid number
	INT VerNum = -1;
	// Search the read ids for the version number
	for (INT Index = 0; Index < ProfileSettings.Num(); Index++)
	{
		const FOnlineProfileSetting& Setting = ProfileSettings(Index);
		if (Setting.ProfileSetting.PropertyId == PSI_ProfileVersionNum)
		{
			// Read whatever came back from the provider
			Setting.ProfileSetting.Data.GetData(VerNum);
			break;
		}
	}
	return VerNum;
}

/** Sets the version number to the class default */
void UOnlineProfileSettings::SetDefaultVersionNumber(void)
{
	// Search the read ids for the version number
	for (INT Index = 0; Index < ProfileSettings.Num(); Index++)
	{
		FOnlineProfileSetting& Setting = ProfileSettings(Index);
		if (Setting.ProfileSetting.PropertyId == PSI_ProfileVersionNum)
		{
			// Set it with the default
			Setting.ProfileSetting.Data.SetData(VersionNumber);
			if ( DELEGATE_IS_SET(NotifySettingValueUpdated) )
			{
				delegateNotifySettingValueUpdated(GetProfileSettingName(PSI_ProfileVersionNum));
			}
			break;
		}
	}
}

/**
 * Determines the mapping type for the specified property
 *
 * @param ProfileId the ID to get the mapping type for
 * @param OutType the out var the value is placed in
 *
 * @return TRUE if found, FALSE otherwise
 */
UBOOL UOnlineProfileSettings::GetProfileSettingMappingType(INT ProfileId,BYTE& OutType)
{
	// Check for the property mapping
	FSettingsPropertyPropertyMetaData* MetaData = FindProfileSettingMetaData(ProfileId);
	if (MetaData != NULL)
	{
		OutType = MetaData->MappingType;
		return TRUE;
	}
	return FALSE;
}

/**
 * Determines the min and max values of a property that is clamped to a range
 *
 * @param ProfileId the ID to get the mapping type for
 * @param OutMinValue the out var the min value is placed in
 * @param OutMaxValue the out var the max value is placed in
 * @param OutRangeIncrement the amount the range can be adjusted by the UI in any single update
 * @param bFormatAsInt whether the range's value should be treated as an int.
 *
 * @return TRUE if found and is a range property, FALSE otherwise
 */
UBOOL UOnlineProfileSettings::GetProfileSettingRange(INT ProfileId,FLOAT& OutMinValue,FLOAT& OutMaxValue,FLOAT& OutRangeIncrement,BYTE& bOutFormatAsInt)
{
	UBOOL bFound = FALSE;
	// Check for the property mapping
	FSettingsPropertyPropertyMetaData* MetaData = FindProfileSettingMetaData(ProfileId);
	FOnlineProfileSetting* Setting = FindSetting(ProfileId);
	// And make sure it's a ranged property
	if (MetaData != NULL && MetaData->MappingType == PVMT_Ranged && Setting != NULL)
	{
		OutRangeIncrement = MetaData->RangeIncrement;
		OutMinValue = MetaData->MinVal;
		OutMaxValue = MetaData->MaxVal;
		bOutFormatAsInt = Setting->ProfileSetting.Data.Type == SDT_Int32;
		bFound = TRUE;
	}
	return bFound;
}

/**
 * Sets the value of a ranged property, clamping to the min/max values
 *
 * @param ProfileId the ID of the property to set
 * @param NewValue the new value to apply to the 
 *
 * @return TRUE if found and is a range property, FALSE otherwise
 */
UBOOL UOnlineProfileSettings::SetRangedProfileSettingValue(INT ProfileId,FLOAT NewValue)
{
	UBOOL bFound = FALSE, bValueChanged = FALSE;
	FLOAT MinValue, MaxValue, Increment;
	BYTE bIntRange;
	// Read the min/max for the property id so we can clamp
	if (GetProfileSettingRange(ProfileId,MinValue,MaxValue,Increment,bIntRange))
	{
		// Clamp within the valid range
		NewValue = Clamp<FLOAT>(NewValue,MinValue,MaxValue);
		if ( bIntRange )
		{
			NewValue = appTrunc(NewValue);
		}
		// Get the profile setting that we are going to set
		FOnlineProfileSetting* ProfileSetting = FindSetting(ProfileId);
		check(ProfileSetting && "Missing profile setting that has a meta data entry");
		// Now set the value
		switch (ProfileSetting->ProfileSetting.Data.Type)
		{
			case SDT_Int32:
			{
				INT Value = appTrunc(NewValue);

				bValueChanged = ProfileSetting->ProfileSetting.Data != Value;
				ProfileSetting->ProfileSetting.Data.SetData(Value);
				bFound = TRUE;
				break;
			}
			case SDT_Float:
			{
				bValueChanged = ProfileSetting->ProfileSetting.Data != NewValue;
				ProfileSetting->ProfileSetting.Data.SetData(NewValue);
				bFound = TRUE;
				break;
			}
		}
	}

	if ( bFound && bValueChanged && DELEGATE_IS_SET(NotifySettingValueUpdated) )
	{
		delegateNotifySettingValueUpdated(GetProfileSettingName(ProfileId));
	}

	return bFound;
}

/**
 * Gets the value of a ranged property
 *
 * @param ProfileId the ID to get the value of
 * @param OutValue the out var that receives the value
 *
 * @return TRUE if found and is a range property, FALSE otherwise
 */
UBOOL UOnlineProfileSettings::GetRangedProfileSettingValue(INT ProfileId,FLOAT& OutValue)
{
	// Get the profile setting that we are going to read
	FOnlineProfileSetting* ProfileSetting = FindSetting(ProfileId);
	if (ProfileSetting)
	{
		// Now set the value
		switch (ProfileSetting->ProfileSetting.Data.Type)
		{
			case SDT_Int32:
			{
				INT Value;
				ProfileSetting->ProfileSetting.Data.GetData(Value);
				OutValue = Value;
				break;
			}
			case SDT_Float:
			{
				ProfileSetting->ProfileSetting.Data.GetData(OutValue);
				break;
			}
			default:
			{
				ProfileSetting = NULL;
				break;
			}
		}
	}
	return ProfileSetting != NULL;
}

/** Finalize the clean up process */
void UOnlineProfileSettings::FinishDestroy(void)
{
	ProfileSettings.Empty();
	Super::FinishDestroy();
}
