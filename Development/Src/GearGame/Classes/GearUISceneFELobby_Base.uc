/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFELobby_Base extends GearUISceneFrontEnd_Base
	abstract
	dependson(GearUISceneFE_LobbyPlayerOptions)
	ClassRedirect(GearUISceneFEParty_Base)
	Config(inherit);


/** Opens the player options scene and sets up the variable needed for this instance */
function OpenPlayerOptionsScene( int PlayerIndex, GearPRI SelectedPRI, array<EGearPlayerOptions> Options, bool bIsLiveEnabled )
{
	local GearPC PC;
	local UIScene SceneInstance, SceneResource;

	PC = GetGearPlayerOwner(PlayerIndex);
	if ( PC != None && PC.PlayerReplicationInfo != None && SelectedPRI != None )
	{
		SceneResource = UIScene(FindObject("UI_Scenes_Lobby.PlayerOptions", class'UIScene'));
		if ( SceneResource != None )
		{
			SceneInstance = OpenScene(SceneResource, GetPlayerOwner(PlayerIndex));
			if ( SceneInstance != None )
			{
				GearUISceneFE_LobbyPlayerOptions(SceneInstance).SetupOptionsVariables( GearPRI(PC.PlayerReplicationInfo), SelectedPRI, Options, bIsLiveEnabled );
			}
		}
	}
}

/**
 * Determines whether any collapsing selection lists are expanded in the specified object list.
 *
 * @param	ObjectList	the list to check for open selection lists
 * @param	ExpandedList	if an expanded collapsing selection list is found, receives the reference to that list
 *
 * @return	TRUE if an expanded collapsing selection list was found in the object list.
 */
function bool IsEditingSelectionList( GearUIObjectList ObjectList, optional out GearUICollapsingSelectionList ExpandedList )
{
	local int i;
	local bool bResult;
	local array<UIObject> ObjectListChildren;

	ObjectListChildren = ObjectList.GetChildren(false);
	for ( i = 0; i < ObjectListChildren.Length; i++ )
	{
		ExpandedList = GearUICollapsingSelectionList(ObjectListChildren[i]);
		if ( ExpandedList != None && (ExpandedList.IsExpanded() || ExpandedList.IsExpanding()) )
		{
			bResult = true;
			break;
		}
	}

	return bResult;
}

/**
 * Wrapper for grabbing the game settings object for the currently selected gametype.
 */
final function OnlineGameSettings GetCurrentGameSettings()
{
	local GearUIDataStore_GameSettings GameSettingsDS;
	local OnlineGameSettings Result;

	GameSettingsDS = GetGameSettingsDataStore();
	if ( GameSettingsDS != None )
	{
		Result = GameSettingsDS.GetCurrentGameSettings();
	}
	return Result;
}

/**
 * Wrapper for retrieving a game setting object based on the setting id
 *
 * @param	ContextId	the id of the gametype to retrieve the settings object for. should match a value in the
 *						CONTEXT_VERSUSMODES list of values.
 */
final function OnlineGameSettings GetGameSettingsByContextId( int ContextId )
{
	local GearUIDataStore_GameSettings GameSettingsDS;
	local OnlineGameSettings Result;

	GameSettingsDS = GetGameSettingsDataStore();
	if ( GameSettingsDS != None )
	{
		Result = GameSettingsDS.GetGameSettingsByContextId(ContextId);
	}

	return Result;
}

/**
 * Wrapper for getting a reference to the current GearPartyGame gameinfo.
 */
final function GearPartyGame_Base GetPartyGameInfo()
{
	return GearPartyGame_Base(GetCurrentGameInfo());
}

/**
 * Wrapper for getting a reference to the current GearCampaignLobbyGame_Base gameinfo.
 */
final function GearCampaignLobbyGame_Base GetCampGameInfo()
{
	return GearCampaignLobbyGame_Base(GetCurrentGameInfo());
}

/**
 * Determines whether the specified player is the leader of the party.
 *
 * @param	PlayerIndex		the index of the player that generated this event
 */
function bool IsPartyLeader( int PlayerIndex )
{
	local GearPC GearPO;
	local bool bResult;
	local OnlineGameSettings PartySettings;
	local OnlineSubsystem OnlineSub;

	// the current party session should be the most authoritative, so try that one first
	GearPO = GetGearPlayerOwner(PlayerIndex);
	if ( GearPO != None )
	{
		bResult = GearPO.IsPartyLeader();

		// If false, we have to check for a local party
		if ( !bResult )
		{
			// First see that there is a proper profile and that the playerindex is 0 (which is guaranteed to be the player driving the menus)
			if ( GearPO.ProfileSettings != None && PlayerIndex == 0 )
			{
				// If there is a game object we are on the server
				if ( GetWorldInfo().Game != None )
				{
					OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
					if ( OnlineSub != None && OnlineSub.GameInterface != None )
					{
						// If there is NOT a party settings object we SHOULD be the leader of a local party
						// If not there is an issue
						PartySettings = OnlineSub.GameInterface.GetGameSettings('Party');
						if ( PartySettings == None )
						{
							bResult = true;
						}
					}
				}
			}
		}
	}
	return bResult;
}

/** Send the server all local players' DLCFlag values */
function UpdateLocalPlayersDLCValues()
{
	local GearPC OwnerPC;

	OwnerPC = GetGearPlayerOwner(GetBestPlayerIndex());
	OwnerPC.UpdateLocalPlayersDLCValues();
}

/** Called by GearMenuPC to notify the client when a game setting value has changed */
function OnGameSettingValueChange( int SettingId, int ValueId );

/** Called by GearMenuPC to notify the client when a game property value has changed */
function OnGamePropertyValueChange( int SettingId, int ValueId );


defaultproperties
{
	bAllowPlayerJoin=false
	bAllowSigninChanges=false
	SceneInputMode=INPUTMODE_Selective


	//@todo ronp animation - temporary until I've made sure that all closing animations are hooked up properly in the lobbies
	SceneAnimation_Close=""
}

