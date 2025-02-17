/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This class holds the data used in reading/writing online profile settings.
 * Online profile settings are stored by an external service.
 */
class OnlineProfileSettings extends Object
	dependson(Settings)
	native;

/** Enum indicating who owns a given online profile proprety */
enum EOnlineProfilePropertyOwner
{
	/** No owner assigned */
	OPPO_None,
	/** Owned by the online service */
	OPPO_OnlineService,
	/** Owned by the game in question */
	OPPO_Game
};

/**
 * Structure used to hold the information for a given profile setting
 */
struct native OnlineProfileSetting
{
	/** Which party owns the data (online service vs game) */
	var EOnlineProfilePropertyOwner Owner;
	/** The profile setting comprised of unique id and union of held types */
	var SettingsProperty ProfileSetting;

	structcpptext
	{
		/** Does nothing (no init version) */
		FOnlineProfileSetting(void)
		{
		}

		/**
		 * Zeroes members
		 */
		FOnlineProfileSetting(EEventParm) :
			Owner(0),
			ProfileSetting(EC_EventParm)
		{
		}

		/**
		 * Copy constructor. Copies the other into this object
		 *
		 * @param Other the other structure to copy
		 */
		FOnlineProfileSetting(const FOnlineProfileSetting& Other) :
			Owner(0),
			ProfileSetting(EC_EventParm)
		{
			Owner = Other.Owner;
			ProfileSetting = Other.ProfileSetting;
		}

		/**
		 * Assignment operator. Copies the other into this object
		 *
		 * @param Other the other structure to copy
		 */
		FOnlineProfileSetting& operator=(const FOnlineProfileSetting& Other)
		{
			if (&Other != this)
			{
				Owner = Other.Owner;
				ProfileSetting = Other.ProfileSetting;
			}
			return *this;
		}
	}
};

/**
 * Enum of profile setting IDs
 */
enum EProfileSettingID
{
	PSI_Unknown,
	// These are all read only
	PSI_ControllerVibration,
	PSI_YInversion,
	PSI_GamerCred,
	PSI_GamerRep,
	PSI_VoiceMuted,
	PSI_VoiceThruSpeakers,
	PSI_VoiceVolume,
	PSI_GamerPictureKey,
	PSI_GamerMotto,
	PSI_GamerTitlesPlayed,
	PSI_GamerAchievementsEarned,
	PSI_GameDifficulty,
	PSI_ControllerSensitivity,
	PSI_PreferredColor1,
	PSI_PreferredColor2,
	PSI_AutoAim,
	PSI_AutoCenter,
	PSI_MovementControl,
	PSI_RaceTransmission,
	PSI_RaceCameraLocation,
	PSI_RaceBrakeControl,
	PSI_RaceAcceleratorControl,
	PSI_GameCredEarned,
	PSI_GameAchievementsEarned,
	PSI_EndLiveIds,
	// Non-Live value that is used to invalidate a stored profile when the versions mismatch
	PSI_ProfileVersionNum,
	// Tracks how many times the profile has been saved
	PSI_ProfileSaveCount
	// Add new profile settings ids here
};

/** Used to determine if the read profile is the proper version or not */
var const int VersionNumber;

/**
 * Holds the list of profile settings to read from the service.
 * NOTE: Only used for a read request and populated by the subclass
 */
var array<int> ProfileSettingIds;

/**
 * Holds the set of profile settings that are either returned from a read or
 * to be written out
 */
var array<OnlineProfileSetting> ProfileSettings;

/**
 * These are the settings to use when no setting has been specified yet for
 * a given id. These values should be used by subclasses to fill in per game
 * default settings
 */
var array<OnlineProfileSetting> DefaultSettings;

/** Mappings for owner information */
var const array<IdToStringMapping> OwnerMappings;

/** Holds the set of mappings from native format to human readable format */
var array<SettingsPropertyPropertyMetaData> ProfileMappings;

/** Enum indicating the current async action happening on this profile */
enum EOnlineProfileAsyncState
{
	OPAS_None,
	OPAS_Read,
	OPAS_Write
};

/** Indicates the state of the profile (whether an async action is happening and what type) */
var const EOnlineProfileAsyncState AsyncState;

/**
 * Enum of difficulty profile values stored by the online service
 * Used with Profile ID PSI_GameDifficulty
 */
enum EProfileDifficultyOptions
{
    PDO_Normal,
    PDO_Easy,
    PDO_Hard,
	// Only add to this list
};

/**
 * Enum of controller sensitivity profile values stored by the online service
 * Used with Profile ID PSI_ControllerSensitivity
 */
enum EProfileControllerSensitivityOptions
{
    PCSO_Medium,
    PCSO_Low,
    PCSO_High,
	// Only add to this list
};

/**
 * Enum of team color preferences stored by the online service
 * Used with Profile ID PSI_PreferredColor1 & PSI_PreferredColor2
 */
enum EProfilePreferredColorOptions
{
    PPCO_None,
    PPCO_Black,
    PPCO_White,
    PPCO_Yellow,
    PPCO_Orange,
    PPCO_Pink,
    PPCO_Red,
    PPCO_Purple,
    PPCO_Blue,
    PPCO_Green,
    PPCO_Brown,
    PPCO_Silver,
	// Only add to this list
};

/**
 * Enum of auto aim preferences stored by the online service
 * Used with Profile ID PSI_AutoAim
 */
enum EProfileAutoAimOptions
{
    PAAO_Off,
    PAAO_On
};

/**
 * Enum of auto center preferences stored by the online service
 * Used with Profile ID PSI_AutoCenter
 */
enum EProfileAutoCenterOptions
{
    PACO_Off,
    PACO_On
};

/**
 * Enum of movement stick preferences stored by the online service
 * Used with Profile ID PSI_MovementControl
 */
enum EProfileMovementControlOptions
{
    PMCO_L_Thumbstick,
    PMCO_R_Thumbstick
};

/**
 * Enum of player's car transmission preferences stored by the online service
 * Used with Profile ID PSI_RaceTransmission
 */
enum EProfileRaceTransmissionOptions
{
    PRTO_Auto,
    PRTO_Manual
};

/**
 * Enum of player's race camera preferences stored by the online service
 * Used with Profile ID PSI_RaceCameraLocation
 */
enum EProfileRaceCameraLocationOptions
{
    PRCLO_Behind,
    PRCLO_Front,
    PRCLO_Inside
};

/**
 * Enum of player's race brake control preferences stored by the online service
 * Used with Profile ID PSI_RaceCameraLocation
 */
enum EProfileRaceBrakeControlOptions
{
    PRBCO_Trigger,
    PRBCO_Button
};

/**
 * Enum of player's race gas control preferences stored by the online service
 * Used with Profile ID PSI_RaceAcceleratorControl
 */
enum EProfileRaceAcceleratorControlOptions
{
    PRACO_Trigger,
    PRACO_Button
};

/**
 * Enum of player's Y axis invert preferences stored by the online service
 * Used with Profile ID PSI_YInversion
 */
enum EProfileYInversionOptions
{
    PYIO_Off,
    PYIO_On
};


/**
* Enum of player's X axis invert preferences stored by the online service
* Used with Profile ID PSI_YInversion
*/
enum EProfileXInversionOptions
{
	PXIO_Off,
	PXIO_On
};


/**
 * Enum of player's vibration preferences stored by the online service
 * Used with Profile ID PSI_ControllerVibration
 */
enum EProfileControllerVibrationToggleOptions
{
    PCVTO_Off,
	PCVTO_IgnoreThis,
	PCVTO_IgnoreThis2,
    PCVTO_On
};

/**
 * Enum of player's voice through speakers preference stored by the online service
 * Used with Profile ID PSI_VoiceThruSpeakers
 */
enum EProfileVoiceThruSpeakersOptions
{
    PVTSO_Off,
    PVTSO_On,
    PVTSO_Both
};

/**
 * Notification that the value of a ProfileSetting in this object has been updated.
 *
 * @param	SettingName		the name of the setting that was changed.
 */
delegate NotifySettingValueUpdated( name SettingName );

/**
 * Searches the profile setting array for the matching string setting name and returns the id
 *
 * @param ProfileSettingName the name of the profile setting being searched for
 * @param ProfileSettingId the id of the context that matches the name
 *
 * @return true if the seting was found, false otherwise
 */
native function bool GetProfileSettingId(name ProfileSettingName,out int ProfileSettingId);

/**
 * Finds the human readable name for the profile setting
 *
 * @param ProfileSettingId the id to look up in the mappings table
 *
 * @return the name of the string setting that matches the id or NAME_None if not found
 */
native function name GetProfileSettingName(int ProfileSettingId);

/**
 * Finds the localized column header text for the profile setting
 *
 * @param ProfileSettingId the id to look up in the mappings table
 *
 * @return the string to use as the list column header for the profile setting that matches the id, or an empty string if not found.
 */
native function string GetProfileSettingColumnHeader( int ProfileSettingId );

/**
 * Finds the index of an OnlineProfileSetting struct given its settings id.
 *
 * @param	ProfileSettingId	the id of the struct to search for
 *
 * @return	the index into the ProfileSettings array for the struct with the matching id.
 */
native final function int FindProfileSettingIndex( int ProfileSettingId ) const;

/**
 * Finds the index of SettingsPropertyPropertyMetaData struct, given its settings id.
 *
 * @param	ProfileSettingId	the id of the struct to search for
 *
 * @return	the index into the ProfileMappings array for the struct with the matching id.
 */
native final function int FindProfileMappingIndex( int ProfileSettingId ) const;

/**
 * Finds the index of SettingsPropertyPropertyMetaData struct, given its settings name.
 *
 * @param	ProfileSettingId	the id of the struct to search for
 *
 * @return	the index into the ProfileMappings array for the struct with the matching name.
 */
native final function int FindProfileMappingIndexByName( name ProfileSettingName ) const;

/**
 * Determines if the setting is id mapped or not
 *
 * @param ProfileSettingId the id to look up in the mappings table
 *
 * @return TRUE if the setting is id mapped, FALSE if it is a raw value
 */
native function bool IsProfileSettingIdMapped(int ProfileSettingId);

/**
 * Finds the human readable name for a profile setting's value. Searches the
 * profile settings mappings for the specifc profile setting and then searches
 * the set of values for the specific value index and returns that value's
 * human readable name
 *
 * @param ProfileSettingId the id to look up in the mappings table
 * @param Value the out param that gets the value copied to it
 * @param ValueMapID optional parameter that allows you to select a specific index in the ValueMappings instead
 * of automatically using the currently set index (if -1 is passed in, which is the default, it means to just
 * use the set index
 *
 * @return true if found, false otherwise
 */
native function bool GetProfileSettingValue(int ProfileSettingId,out string Value,optional int ValueMapID = -1);

/**
 * Finds the human readable name for a profile setting's value. Searches the
 * profile settings mappings for the specifc profile setting and then searches
 * the set of values for the specific value index and returns that value's
 * human readable name
 *
 * @param ProfileSettingId the id to look up in the mappings table
 *
 * @return the name of the value or NAME_None if not value mapped
 */
native function name GetProfileSettingValueName(int ProfileSettingId);

/**
 * Searches the profile settings mappings for the specifc profile setting and
 * then adds all of the possible values to the out parameter
 *
 * @param ProfileSettingId the id to look up in the mappings table
 * @param Values the out param that gets the list of values copied to it
 *
 * @return true if found and value mapped, false otherwise
 */
native function bool GetProfileSettingValues(int ProfileSettingId,out array<name> Values);

/**
 * Finds the human readable name for a profile setting's value. Searches the
 * profile settings mappings for the specifc profile setting and then searches
 * the set of values for the specific value index and returns that value's
 * human readable name
 *
 * @param ProfileSettingName the name of the profile setting to find the string value of
 * @param Value the out param that gets the value copied to it
 *
 * @return true if found, false otherwise
 */
native function bool GetProfileSettingValueByName(name ProfileSettingName,out string Value);

/**
 * Searches for the profile setting by name and sets the value index to the
 * value contained in the profile setting meta data
 *
 * @param ProfileSettingName the name of the profile setting to find
 * @param NewValue the string value to use
 *
 * @return true if the profile setting was found and the value was set, false otherwise
 */
native function bool SetProfileSettingValueByName(name ProfileSettingName,const out string NewValue);

/**
 * Searches for the profile setting by name and sets the value index to the
 * value contained in the profile setting meta data
 *
 * @param ProfileSettingName the name of the profile setting to set the string value of
 * @param NewValue the string value to use
 *
 * @return true if the profile setting was found and the value was set, false otherwise
 */
native function bool SetProfileSettingValue(int ProfileSettingId,const out string NewValue);

/**
 * Searches for the profile setting by id and gets the value index
 *
 * @param ProfileSettingId the id of the profile setting to return
 * @param ValueId the out value of the id
 * @param ListIndex the out value of the index where that value lies in the ValueMappings list
 *
 * @return true if the profile setting was found and id mapped, false otherwise
 */
native function bool GetProfileSettingValueId(int ProfileSettingId,out int ValueId,optional out int ListIndex);

/**
 * Searches for the profile setting by id and gets the value index
 *
 * @param ProfileSettingId the id of the profile setting to return
 * @param Value the out value of the setting
 *
 * @return true if the profile setting was found and not id mapped, false otherwise
 */
native function bool GetProfileSettingValueInt(int ProfileSettingId,out int Value);

/**
 * Searches for the profile setting by id and gets the value index
 *
 * @param ProfileSettingId the id of the profile setting to return
 * @param Value the out value of the setting
 *
 * @return true if the profile setting was found and not id mapped, false otherwise
 */
native function bool GetProfileSettingValueFloat(int ProfileSettingId,out float Value);

/**
 * Searches for the profile setting by id and sets the value
 *
 * @param ProfileSettingId the id of the profile setting to return
 * @param Value the new value
 *
 * @return true if the profile setting was found and id mapped, false otherwise
 */
native function bool SetProfileSettingValueId(int ProfileSettingId,int Value);

/**
 * Searches for the profile setting by id and sets the value
 *
 * @param ProfileSettingId the id of the profile setting to return
 * @param Value the new value
 *
 * @return true if the profile setting was found and not id mapped, false otherwise
 */
native function bool SetProfileSettingValueInt(int ProfileSettingId,int Value);

/**
 * Searches for the profile setting by id and sets the value
 *
 * @param ProfileSettingId the id of the profile setting to return
 * @param Value the new value
 *
 * @return true if the profile setting was found and not id mapped, false otherwise
 */
native function bool SetProfileSettingValueFloat(int ProfileSettingId,float Value);

/**
 * Searches for the profile setting by id and gets the default value index
 *
 * @param ProfileSettingId the id of the profile setting to return
 * @param DefaultId the out value of the default id
 * @param ListIndex the out value of the index where that value lies in the ValueMappings list
 *
 * @return true if the profile setting was found and retrieved the default id, false otherwise
 */
native function bool GetProfileSettingDefaultId(int ProfileSettingId,out int DefaultId, out int ListIndex);

/**
 * Searches for the profile setting by id and gets the default value int
 *
 * @param ProfileSettingId the id of the profile setting to return the default of
 * @param Value the out value of the default setting
 *
 * @return true if the profile setting was found and retrieved the default int, false otherwise
 */
native function bool GetProfileSettingDefaultInt(int ProfileSettingId,out int DefaultInt);

/**
 * Searches for the profile setting by id and gets the default value float
 *
 * @param ProfileSettingId the id of the profile setting to return the default of
 * @param Value the out value of the default setting
 *
 * @return true if the profile setting was found and retrieved the default float, false otherwise
 */
native function bool GetProfileSettingDefaultFloat(int ProfileSettingId,out float DefaultFloat);

/**
 * Sets all of the profile settings to their default values
 */
native noexportheader event SetToDefaults();

/**
 * Adds the version id to the read ids if it is not present
 */
native function AppendVersionToReadIds();

/**
 * Adds the version number to the read data if not present
 */
native function AppendVersionToSettings();

/** Returns the version number that was found in the profile read results */
native function int GetVersionNumber();

/** Sets the version number to the class default */
native function SetDefaultVersionNumber();

/**
 * Determines the mapping type for the specified property
 *
 * @param ProfileId the ID to get the mapping type for
 * @param OutType the out var the value is placed in
 *
 * @return TRUE if found, FALSE otherwise
 */
native function bool GetProfileSettingMappingType(int ProfileId,out EPropertyValueMappingType OutType);

/**
 * Determines the min and max values of a property that is clamped to a range
 *
 * @param ProfileId the ID to get the mapping type for
 * @param OutMinValue the out var the min value is placed in
 * @param OutMaxValue the out var the max value is placed in
 * @param RangeIncrement the amount the range can be adjusted by the UI in any single update
 * @param bFormatAsInt whether the range's value should be treated as an int.
 *
 * @return TRUE if found and is a range property, FALSE otherwise
 */
native function bool GetProfileSettingRange(int ProfileId,out float OutMinValue,out float OutMaxValue,out float RangeIncrement,out byte bFormatAsInt);

/**
 * Sets the value of a ranged property, clamping to the min/max values
 *
 * @param ProfileId the ID of the property to set
 * @param NewValue the new value to apply to the
 *
 * @return TRUE if found and is a range property, FALSE otherwise
 */
native function bool SetRangedProfileSettingValue(int ProfileId,float NewValue);

/**
 * Gets the value of a ranged property
 *
 * @param ProfileId the ID to get the value of
 * @param OutValue the out var that receives the value
 *
 * @return TRUE if found and is a range property, FALSE otherwise
 */
native function bool GetRangedProfileSettingValue(int ProfileId,out float OutValue);

/**
 * Hooks to allow child classes to dynamically adjust available profile settings or mappings based on e.g. ini values.
 */
event ModifyAvailableProfileSettings();

cpptext
{
protected:
	/**
	 * Sets all of the profile settings to their default values
	 */
    virtual void SetToDefaults();

public:
	/**
	 * Finds the specified profile setting
	 *
	 * @param SettingId to search for
	 *
	 * @return pointer to the setting or NULL if not found
	 */
	FORCEINLINE FOnlineProfileSetting* FindSetting(INT SettingId)
	{
		for (INT ProfileIndex = 0; ProfileIndex < ProfileSettings.Num(); ++ProfileIndex)
		{
			FOnlineProfileSetting& Setting = ProfileSettings(ProfileIndex);
			if (Setting.ProfileSetting.PropertyId == SettingId)
			{
				return &Setting;
			}
		}
		return NULL;
	}

	/**
	 * Finds the specified property's meta data
	 *
	 * @param PropertyId id of the property to search the meta data for
	 *
	 * @return pointer to the property meta data or NULL if not found
	 */
	FORCEINLINE FSettingsPropertyPropertyMetaData* FindProfileSettingMetaData(INT ProfileId)
	{
		for (INT MetaDataIndex = 0; MetaDataIndex < ProfileMappings.Num(); MetaDataIndex++)
		{
			FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(MetaDataIndex);
			if (MetaData.Id == ProfileId)
			{
				return &MetaData;
			}
		}
		return NULL;
	}

	/**
	 * Searches for the profile setting by id and sets the value
	 *
	 * @param ProfileSettingId the id of the profile setting to return
	 * @param Value the new value of the setting
	 *
	 * @return true if the profile setting was found and not id mapped, false otherwise
	 */
	template<typename TYPE>
	FORCEINLINE UBOOL SetProfileSettingTypedValue(INT ProfileSettingId,TYPE Value)
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
							Setting.ProfileSetting.Data.SetData(Value);
							if ( DELEGATE_IS_SET(NotifySettingValueUpdated) )
							{
								delegateNotifySettingValueUpdated(GetProfileSettingName(ProfileSettingId));
							}
							return TRUE;
						}
						else
						{
							warnf(TEXT("SetProfileSettingValue(%d) did not find a valid MappingType" ),
								ProfileSettingId);
							return FALSE;
						}
					}
				}
			}
		}
		return FALSE;
	}

	/** Finalize the clean up process */
	virtual void FinishDestroy(void);
}

defaultproperties
{
	// This must be set by subclasses
	VersionNumber=-1
	// UI readable versions of the owners
	OwnerMappings(0)=(Id=OPPO_None)
	OwnerMappings(1)=(Id=OPPO_OnlineService)
	OwnerMappings(2)=(Id=OPPO_Game)
	// Meta data for displaying in the UI
	ProfileMappings(0)=(Id=PSI_ControllerVibration,Name="Controller Vibration",MappingType=PVMT_IdMapped,ValueMappings=((Id=PCVTO_On),(Id=PCVTO_Off)))
	ProfileMappings(1)=(Id=PSI_YInversion,Name="Invert Y",MappingType=PVMT_IdMapped,ValueMappings=((Id=PYIO_Off),(Id=PYIO_On)))
	ProfileMappings(2)=(Id=PSI_VoiceMuted,Name="Mute Voice",MappingType=PVMT_IdMapped,ValueMappings=((Id=0),(Id=1)))
	ProfileMappings(3)=(Id=PSI_VoiceThruSpeakers,Name="Voice Via Speakers",MappingType=PVMT_IdMapped,ValueMappings=((Id=PVTSO_Off),(Id=PVTSO_On),(Id=PVTSO_Both)))
	ProfileMappings(4)=(Id=PSI_VoiceVolume,Name="Voice Volume",MappingType=PVMT_RawValue)
	ProfileMappings(5)=(Id=PSI_GameDifficulty,Name="Difficulty Level",MappingType=PVMT_IdMapped,ValueMappings=((Id=PDO_Normal),(Id=PDO_Easy),(Id=PDO_Hard)))
	ProfileMappings(6)=(Id=PSI_ControllerSensitivity,Name="Controller Sensitivity",MappingType=PVMT_IdMapped,ValueMappings=((Id=PCSO_Medium),(Id=PCSO_Low),(Id=PCSO_High)))
	ProfileMappings(7)=(Id=PSI_PreferredColor1,Name="First Preferred Color",MappingType=PVMT_IdMapped,ValueMappings=((Id=PPCO_None),(Id=PPCO_Black),(Id=PPCO_White),(Id=PPCO_Yellow),(Id=PPCO_Orange),(Id=PPCO_Pink),(Id=PPCO_Red),(Id=PPCO_Purple),(Id=PPCO_Blue),(Id=PPCO_Green),(Id=PPCO_Brown),(Id=PPCO_Silver)))
	ProfileMappings(8)=(Id=PSI_PreferredColor2,Name="Second Preferred Color",MappingType=PVMT_IdMapped,ValueMappings=((Id=PPCO_None),(Id=PPCO_Black),(Id=PPCO_White),(Id=PPCO_Yellow),(Id=PPCO_Orange),(Id=PPCO_Pink),(Id=PPCO_Red),(Id=PPCO_Purple),(Id=PPCO_Blue),(Id=PPCO_Green),(Id=PPCO_Brown),(Id=PPCO_Silver)))
	ProfileMappings(9)=(Id=PSI_AutoAim,Name="Auto Aim",MappingType=PVMT_IdMapped,ValueMappings=((Id=PAAO_Off),(Id=PAAO_On)))
	ProfileMappings(10)=(Id=PSI_AutoCenter,Name="Auto Center",MappingType=PVMT_IdMapped,ValueMappings=((Id=PACO_Off),(Id=PACO_On)))
	ProfileMappings(11)=(Id=PSI_MovementControl,Name="Movement Control",MappingType=PVMT_IdMapped,ValueMappings=((Id=PMCO_L_Thumbstick),(Id=PMCO_R_Thumbstick)))
	ProfileMappings(12)=(Id=PSI_RaceTransmission,Name="Transmission Preference",MappingType=PVMT_IdMapped,ValueMappings=((Id=PRTO_Auto),(Id=PRTO_Manual)))
	ProfileMappings(13)=(Id=PSI_RaceCameraLocation,Name="Race Camera Preference",MappingType=PVMT_IdMapped,ValueMappings=((Id=PRCLO_Behind),(Id=PRCLO_Front),(Id=PRCLO_Inside)))
	ProfileMappings(14)=(Id=PSI_RaceBrakeControl,Name="Brake Preference",MappingType=PVMT_IdMapped,ValueMappings=((Id=PRBCO_Trigger),(Id=PRBCO_Button)))
	ProfileMappings(15)=(Id=PSI_RaceAcceleratorControl,Name="Accelerator Preference",MappingType=PVMT_IdMapped,ValueMappings=((Id=PRACO_Trigger),(Id=PRACO_Button)))
}
