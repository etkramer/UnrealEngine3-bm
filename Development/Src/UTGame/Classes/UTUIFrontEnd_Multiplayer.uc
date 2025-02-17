/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Multiplayer Scene for UT3.
 */

class UTUIFrontEnd_Multiplayer extends UTUIFrontEnd_BasicMenu
	dependson(UTUIScene_MessageBox);

const MULTIPLAYER_OPTION_QUICKMATCH = 0;
const MULTIPLAYER_OPTION_JOINGAME = 1;
const MULTIPLAYER_OPTION_HOSTGAME = 2;

/** Reference to the quick match scene. */
var string	QuickMatchScene;

/** Reference to the host game scene. */
var string	HostScene;

/** Reference to the join game scene. */
var string	JoinScene;

/** Reference to the character selection scene. */
var string	CharacterSelectionScene;

/** Reference to the settings panels scene. */
var string	SettingsPanelsScene;

/** indicates that the player's profile should be saved */
var	transient	bool	bProfileNeedsSaving;

/** Whether or not we have already displayed the new player message box to the user. */
var transient	bool bDisplayedNewPlayerMessageBox;

/** @return bool Returns whether or not this user has saved character data. */
function bool HasSavedCharacterData()
{
	local bool bHaveLoadedCharData;
	local string CharacterDataStr;

	bHaveLoadedCharData = false;


	if( GetDataStoreStringValue("<OnlinePlayerData:ProfileData.CustomCharData>", CharacterDataStr, Self, GetPlayerOwner())
	&&	Len(CharacterDataStr) > 0 )
	{
		bHaveLoadedCharData = true;
	}

	return bHaveLoadedCharData;
}

/** Called when the screen has finished showing. */
function OnMainRegion_Show_UIAnimEnd( UIScreenObject AnimTarget, name AnimName, int TrackTypeMask )
{
	Super.OnMainRegion_Show_UIAnimEnd(AnimTarget, AnimName, TrackTypeMask);

	if ( AnimName == 'SceneShowInitial' )
	{
		if ( CheckLoginAndError(GetPlayerControllerId(GetPlayerIndex()), true, ,"<Strings:UTGameUI.Errors.OnlineRequiredForInternet_Message>") )
		{
			PerformMultiplayerChecks();
		}
		else
		{
			GetSceneClient().bKillRestoreMenuProgression = true;
		}
	}
}

/** Displays the login interface using the online subsystem. */
function bool ShowLoginUI(optional bool bOnlineOnly=false)
{
	local bool bResult;
	local int PlayerIndex;

	bResult = Super.ShowLoginUI(bOnlineOnly);

	if ( bResult )
	{
		// while waiting for the login to complete, don't allow the player to go anywhere
		PlayerIndex = GetPlayerIndex();
//		ButtonBar.Buttons[0].DisableWidget(PlayerIndex);
		ButtonBar.Buttons[1].DisableWidget(PlayerIndex);
	}

	return bResult;
}

/**
 * Executes a action based on the currently selected menu item.
 */
function OnSelectItem(int PlayerIndex=GetPlayerIndex())
{
	local int SelectedItem, ControllerId;

	if ( ButtonBar.Buttons[1].IsEnabled(PlayerIndex) )
	{
		SelectedItem = MenuList.GetCurrentItem();
		ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
		switch(SelectedItem)
		{
		case MULTIPLAYER_OPTION_QUICKMATCH:
			if ( CheckLinkConnectionAndError() && CheckLoginAndError(ControllerId, true) )
			{
				OpenSceneByName(QuickMatchScene);
			}
			break;

		case MULTIPLAYER_OPTION_HOSTGAME:
			if ( CheckLinkConnectionAndError() )
			{
				OpenSceneByName(HostScene);
			}
			break;

		case MULTIPLAYER_OPTION_JOINGAME:
			if ( CheckLinkConnectionAndError() )
			{
				OpenSceneByName(Joinscene);
			}
			break;
		}
	}
}

/** Callback for when the user wants to back out of this screen. */
function OnBack()
{
	if ( ButtonBar.Buttons[0].IsEnabled(GetPlayerIndex()) )
	{
		Super.OnBack();
	}
}

event SceneDeactivated()
{
	local OnlinePlayerInterface PlayerInt;

	Super.SceneDeactivated();

	// make sure that we completely unsubscribed from all login change notifications (this can happen if the player closes
	// this menu immediately after signing in, before the subsystem has fired the notification)
	PlayerInt = GetPlayerInterface();

	if(PlayerInt != none)
	{
		PlayerInt.ClearLoginChangeDelegate(OnLoginUI_LoginChange, GetPlayerOwner().ControllerId);
		PlayerInt.ClearLoginFailedDelegate(GetPlayerOwner().ControllerId, OnLoginUI_LoginFailed);
	}

	UTGameUISceneClient(GetSceneClient()).bDimScreen=false;
}

/**
 * Checks various conditions required to play online and displays the relevant messages to the user if any online features
 * will be disabled.
 */
function PerformMultiplayerChecks()
{
	local UTProfileSettings Profile;
	local UTUIScene_MessageBox MessageBoxReference;
	local int CurrentValue, ControllerId;
	local string ChatRestrictionValue;
	local OnlinePlayerInterface PlayerInt;

	Profile = GetPlayerProfile();
	if ( Profile != None )
	{
		if(!IsConsole()
		&&(!Profile.GetProfileSettingValueInt(class'UTProfileSettings'.const.UTPID_FirstTimeMultiplayer, CurrentValue) || CurrentValue!=1) )
		{
			CurrentValue = 1;
			Profile.SetProfileSettingValueInt(class'UTProfileSettings'.const.UTPID_FirstTimeMultiplayer, CurrentValue);
			MarkProfileDirty(true);

			MessageBoxReference = GetMessageBoxScene();
			MessageBoxReference.DisplayAcceptBox("<Strings:UTGameUI.MessageBox.FirstTimeMultiplayer_Message>",
				"<Strings:UTGameUI.MessageBox.FirstTimeMultiplayer_Title>", OnFirstTimeMultiplayer_Confirm);
		}
		else
		{
			if (!GetDataStoreStringValue("<Registry:PerformedChatRestrictionCheck>", ChatRestrictionValue, Self, GetPlayerOwner())
			||	ChatRestrictionValue != "1")
			{
				ChatRestrictionValue = "1";
				SetDataStoreStringValue("<Registry:PerformedChatRestrictionCheck>", ChatRestrictionValue, Self, GetPlayerOwner());

				ControllerId = GetPlayerOwner().ControllerId;
				PlayerInt = GetPlayerInterface();
				if ( IsLoggedIn(ControllerId, true) && PlayerInt != None && PlayerInt.CanCommunicate(ControllerId) == FPL_Disabled )
				{
					GetMessageBoxScene().DisplayAcceptBox("<Strings:UTGameUI.Errors.CommunicationRequired_Message>", "<Strings:UTGameUI.Errors.CommunicationRequired_Title>",
						OnChatRestrictionConfirm);
				}
				else
				{
					OnChatRestrictionConfirm(None, 0, GetPlayerIndex());
				}
			}
			// See if the current player hasn't setup a character yet.
			else if ( !bDisplayedNewPlayerMessageBox )
			{
				bDisplayedNewPlayerMessageBox=true;
				DisplayNewPlayerMessageBox();
			}
		}
	}
}

/** Callback for when the login changes after showing the login UI. */
function OnLoginUI_LoginChange()
{
	local int PlayerIndex;

	// re-enable the buttons
	PlayerIndex = GetPlayerIndex();

	ButtonBar.Buttons[0].EnableWidget(PlayerIndex);
	ButtonBar.Buttons[1].EnableWidget(PlayerIndex);

	Super.OnLoginUI_LoginChange();

	// regenerate the list of available options - some options are shown depending on our login status
	MenuList.RefreshSubscriberValue();
	PerformMultiplayerChecks();
}

/**
 * Delegate used in notifying the UI/game that the manual login failed after showing the login UI.
 *
 * @param LocalUserNum the controller number of the associated user
 * @param ErrorCode the async error code that occurred
 */
function OnLoginUI_LoginFailed(byte LocalUserNum,EOnlineServerConnectionStatus ErrorCode)
{
	local int PlayerIndex;

	// re-enable the buttons
	PlayerIndex = GetPlayerIndex();

	ButtonBar.Buttons[0].EnableWidget(PlayerIndex);
	ButtonBar.Buttons[1].EnableWidget(PlayerIndex);

	Super.OnLoginUI_LoginFailed(LocalUserNum, ErrorCode);

	PerformMultiplayerChecks();
}

/**
 * Called when the user dismisses the "you can't chat online" message box
 */
function OnChatRestrictionConfirm(UTUIScene_MessageBox MessageBox, int SelectedOption, int PlayerIndex)
{
	if(bDisplayedNewPlayerMessageBox==false)
	{
		bDisplayedNewPlayerMessageBox=true;
		DisplayNewPlayerMessageBox();
	}
}

/**
 * Callback when the user dismisses the firsttime multiplayer message box.
 */
function OnFirstTimeMultiplayer_Confirm(UTUIScene_MessageBox MessageBox, int SelectedOption, int PlayerIndex)
{
	OpenSceneByName(SettingsPanelsScene, false, OnSettingsSceneOpened);
}

/** Callback for when the settings scene has opened. */
function OnSettingsSceneOpened(UIScene OpenedScene, bool bInitialActivation)
{
	local UTUIFrontEnd_SettingsPanels PanelSceneInst;

	PanelSceneInst = UTUIFrontEnd_SettingsPanels(OpenedScene);

	PanelSceneInst.OnMarkProfileDirty = MarkProfileDirty;
	PanelSceneInst.TabControl.ActivatePage(PanelSceneInst.NetworkTab, GetBestPlayerIndex());
	PanelSceneInst.OnSceneDeactivated = SettingsSceneClosed;
}
function SettingsSceneClosed( UIScene DeactivatedScene )
{
	if ( bProfileNeedsSaving )
	{
		MarkProfileDirty(false);
		UTGameUISceneClient(GetSceneClient()).ShowSaveProfileScene(GetUTPlayerOwner());
	}

	PerformMultiplayerChecks();
}

/**
 * Called when the profile has been modified and needs to be saved.
 */
function MarkProfileDirty( optional bool bDirty=true )
{
	bProfileNeedsSaving = bDirty;
}

/** Displays a messagebox if the player doesn't have a custom character set. */
function DisplayNewPlayerMessageBox()
{
	local UTUIScene_MessageBox MessageBoxReference;
	local array<string> MessageBoxOptions;
	local array<PotentialOptionKeys> PotentialOptionKeyMappings;

	if( !HasSavedCharacterData() )
	{
		if ( GetWorldInfo().IsDemoBuild() )	// In demo builds just generate a random character.
		{
			OnFirstTimeCharacter_Confirm(None,0,GetPlayerIndex());
		}
		else
		{
			// Pop up a message box asking the user if they want to edit their character or randomly generate one.
			MessageBoxReference = GetMessageBoxScene();

			if(MessageBoxReference != none)
			{
				MessageBoxOptions.AddItem("<Strings:UTGameUI.CharacterCustomization.RandomlyGenerate>");
				MessageBoxOptions.AddItem("<Strings:UTGameUI.CharacterCustomization.CreateCharacter>");
				MessageBoxOptions.AddItem("<Strings:UTGameUI.Generic.Cancel>");

				PotentialOptionKeyMappings.length = 3;
				PotentialOptionKeyMappings[0].Keys.AddItem('XboxTypeS_X');
				PotentialOptionKeyMappings[1].Keys.AddItem('XboxTypeS_A');
				PotentialOptionKeyMappings[2].Keys.AddItem('XboxTypeS_B');
				PotentialOptionKeyMappings[2].Keys.AddItem('Escape');

				MessageBoxReference.SetPotentialOptionKeyMappings(PotentialOptionKeyMappings);
				MessageBoxReference.SetPotentialOptions(MessageBoxOptions);
				MessageBoxReference.Display("<Strings:UTGameUI.MessageBox.FirstTimeCharacter_Message>", "<Strings:UTGameUI.MessageBox.FirstTimeCharacter_Title>", OnFirstTimeCharacter_Confirm);
			}
		}
	}
}

/**
 * Callback when the user dismisses the firsttime character message box.
 */

function OnFirstTimeCharacter_Confirm(UTUIScene_MessageBox MessageBox, int SelectedOption, int PlayerIndex)
{
	/*	@TODO - figure out new character selection system
	local CustomCharData CustomData;
	local string DataString;

	switch ( SelectedOption )
	{
	case 0:	// Randomly generate a character
		CustomData = class'UTCustomChar_Data'.static.MakeRandomCharData();
		DataString = class'UTCustomChar_Data'.static.CharDataToString(CustomData);
		if(SetDataStoreStringValue("<OnlinePlayerData:ProfileData.CustomCharData>", DataString, Self, GetPlayerOwner()))
		{
			UTGameUISceneClient(GetSceneClient()).ShowSaveProfileScene(GetUTPlayerOwner());
			MarkProfileDirty(false);
		}
		break;

	case 1:	// Open the character selection scene
		OpenSceneByName(CharacterSelectionScene);
		bDisplayedNewPlayerMessageBox=false;	// Need to recheck for a customized character after the user has closed the player customization screen.
		break;
	case 2:	// Close this scene
		CloseScene(self);
		break;
	}
	*/
}

defaultproperties
{
	QuickMatchScene="UI_Scenes_ChrisBLayout.Scenes.QuickMatch"
	JoinScene="UI_Scenes_FrontEnd.Scenes.JoinGame"
	HostScene="UI_Scenes_ChrisBLayout.Scenes.HostGame"
	SettingsPanelsScene="UI_Scenes_ChrisBLayout.Scenes.SettingsPanels"
	CharacterSelectionScene="UI_Scenes_ChrisBLayout.Scenes.CharacterFaction"
	bMenuLevelRestoresScene=true
	bRequiresNetwork=true
}
