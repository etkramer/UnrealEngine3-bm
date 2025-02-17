/**
 * Game settings data store for Gears 2.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUIDataStore_GameSettings extends UIDataStore_OnlineGameSettings;

`include(GearOnlineConstants.uci)

/**
 * Sets the index into the list of game settings to use
 *
 * @param	ContextId	the context id of the game settings object to use.  Should be one of the
 *						CONTEXT_VERSUSMODES values.
 */
event SetCurrentByContextId(int ContextId)
{
	local int GameSettingsIndex, GameTypeValueId;
	local OnlineGameSettings GameSettings;

	for ( GameSettingsIndex = 0; GameSettingsIndex < GameSettingsCfgList.Length; GameSettingsIndex++ )
	{
		GameSettings = GameSettingsCfgList[GameSettingsIndex].GameSettings;
		if ( GameSettings != None )
		{
			if (GameSettings.GetStringSettingValue(CONTEXT_VERSUSMODES, GameTypeValueId)
			&&	ContextId == GameTypeValueId)
			{
				SetCurrentByIndex(GameSettingsIndex);
				return;
			}
		}
	}

	`log("No game settings objects found with a CONTEXT_VERSUSMODES value of" @ ContextId);
}

event OnlineGameSettings GetGameSettingsByContextId( int ContextId )
{
	local int GameSettingsIndex, GameTypeValueId;
	local OnlineGameSettings GameSettings, Result;

	for ( GameSettingsIndex = 0; GameSettingsIndex < GameSettingsCfgList.Length; GameSettingsIndex++ )
	{
		GameSettings = GameSettingsCfgList[GameSettingsIndex].GameSettings;
		if ( GameSettings != None )
		{
			if (GameSettings.GetStringSettingValue(CONTEXT_VERSUSMODES, GameTypeValueId)
			&&	ContextId == GameTypeValueId)
			{
				Result = GameSettings;
				break;
			}
		}
	}

	`log("No game settings objects found with a CONTEXT_VERSUSMODES value of" @ ContextId);
	return Result;
}

defaultproperties
{
	GameSettingsCfgList.Empty

	GameSettingsCfgList.Add((SettingsName="GameSettingsAnnex",GameSettingsClass=class'GearGame.GearAnnexSettings'))
	GameSettingsCfgList.Add((SettingsName="GameSettingsAssassination",GameSettingsClass=class'GearGame.GearAssassinationSettings'))
	GameSettingsCfgList.Add((SettingsName="GameSettingsExecution",GameSettingsClass=class'GearGame.GearExecutionSettings'))
	GameSettingsCfgList.Add((SettingsName="GameSettingsHorde",GameSettingsClass=class'GearGame.GearHordeSettings'))
	GameSettingsCfgList.Add((SettingsName="GameSettingsKOTH",GameSettingsClass=class'GearGame.GearKOTHSettings'))
	GameSettingsCfgList.Add((SettingsName="GameSettingsMeatflag",GameSettingsClass=class'GearGame.GearMeatflagSettings'))
	GameSettingsCfgList.Add((SettingsName="GameSettingsWarzone",GameSettingsClass=class'GearGame.GearWarzoneSettings'))
	GameSettingsCfgList.Add((SettingsName="GameSettingsWingman",GameSettingsClass=class'GearGame.GearWingmanSettings'))
}
