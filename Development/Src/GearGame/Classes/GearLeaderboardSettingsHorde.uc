/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This class holds the game modes and leaderboard time line filters
 */
class GearLeaderboardSettingsHorde extends GearLeaderboardSettingsBase;

/**
 * Searches the localized string setting array for the matching id and sets the selected value
 * to the next (or prev) item in the list, wrapping if required
 *
 * @param StringSettingId the string setting to find the value of
 * @param Direction the direction to move in the list (1 forward, -1 backward)
 * @param bShouldWrap if true out of bound access wraps around, false clamps to min/max
 *
 * @return true if found, false otherwise
 */
function bool IncrementStringSettingValue(int StringSettingId,int Direction,bool bShouldWrap)
{
	local int Index;
	local int NewValueIndex;
	local int MaxVal;
	local int MinVal;

	if (StringSettingId == LF_MapType)
	{
		// Max needs to be the size of maps array
		MaxVal = class'GearLeaderboardSettings'.default.MapNamesToContextValues.Length;
		MinVal = CONTEXT_MAPNAME_MAP1;
		// Search for the value and return it if found
		for (Index = 0; Index < LocalizedSettings.Length; Index++)
		{
			if (LocalizedSettings[Index].Id == StringSettingId)
			{
				// Determine the new value based upon direction
				NewValueIndex = LocalizedSettings[Index].ValueIndex + Direction;
				// Handle out of bounds cases and either clamp or wrap
				if (NewValueIndex < MinVal || NewValueIndex > MaxVal)
				{
					if (bShouldWrap)
					{
						// Wrap to top if we went negative
						if (NewValueIndex < MinVal)
						{
							NewValueIndex = MaxVal;
						}
						// Otherwise wrap to the bottom
						else
						{
							NewValueIndex = MinVal;
						}
					}
					else
					{
						Clamp(NewValueIndex,MinVal,MaxVal);
					}
				}
				// Finally assign the new index
				LocalizedSettings[Index].ValueIndex = NewValueIndex;
				NotifySettingValueUpdated(GetStringSettingName(StringSettingId));
				return true;
			}
		}
		return false;
	}
	return Super.IncrementStringSettingValue(StringSettingId,Direction,bShouldWrap);
}

defaultproperties
{
	LocalizedSettings(LF_GameMode)=(Id=LF_GameMode,ValueIndex=GMF_Horde)
	LocalizedSettingsMappings(LF_GameMode)=(Id=LF_GameMode,Name="GameMode",ValueMappings=((Id=GMF_Horde)))

	LocalizedSettings(LF_TimePeriod)=(Id=LF_TimePeriod,ValueIndex=TF_AllTime)
	LocalizedSettingsMappings(LF_TimePeriod)=(Id=LF_TimePeriod,Name="Time Period",ValueMappings=((Id=TF_AllTime)))

	LocalizedSettings(LF_PlayerFilterType)=(Id=LF_PlayerFilterType,ValueIndex=PF_Friends)
	LocalizedSettingsMappings(LF_PlayerFilterType)=(Id=LF_PlayerFilterType,Name="Player Filter",ValueMappings=((Id=PF_Player),(Id=PF_CenteredOnPlayer),(Id=PF_Friends),(Id=PF_TopRankings)))

	LocalizedSettings(LF_MapType)=(Id=LF_MapType,ValueIndex=CONTEXT_MAPNAME_MAP1)
	LocalizedSettingsMappings(LF_MapType)=(Id=LF_MapType,Name="HordeMap",ValueMappings=((Id=CONTEXT_MAPNAME_MAP1),(Id=CONTEXT_MAPNAME_MAP2),(Id=CONTEXT_MAPNAME_MAP3),(Id=CONTEXT_MAPNAME_MAP4),(Id=CONTEXT_MAPNAME_MAP5),(Id=CONTEXT_MAPNAME_MAP6),(Id=CONTEXT_MAPNAME_MAP7),(Id=CONTEXT_MAPNAME_MAP8),(Id=CONTEXT_MAPNAME_MAP9),(Id=CONTEXT_MAPNAME_MAP10),(Id=CONTEXT_MAPNAME_MAP11),(Id=CONTEXT_MAPNAME_MAP12),(Id=CONTEXT_MAPNAME_MAP13),(Id=CONTEXT_MAPNAME_MAP14),(Id=CONTEXT_MAPNAME_MAP15),(Id=CONTEXT_MAPNAME_MAP16),(Id=CONTEXT_MAPNAME_MAP17),(Id=CONTEXT_MAPNAME_MAP18),(Id=CONTEXT_MAPNAME_MAP19),(Id=CONTEXT_MAPNAME_MAP20),(Id=CONTEXT_MAPNAME_MAP21),(Id=CONTEXT_MAPNAME_MAP22),(Id=CONTEXT_MAPNAME_MAP23),(Id=CONTEXT_MAPNAME_MAP24)))
}
