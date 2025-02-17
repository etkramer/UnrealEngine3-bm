/**
 * First scene after the title scene.  Provides options for entering the campaign, party lobby, options, etc.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearUISceneFETran_MainMenu extends GearUISceneFETran_Base
	Config(UI);

`include(Engine/Classes/UIDev.uci)

/** Gears 1 Title Ids */
const GEARS1_TITLEID_XBOX	= 0x4D5307D5;
const GEARS1_TITLEID_PC		= 0x4D530842;

/** references to widgets in the scene */
var		transient		UILabelButton	btnSoloCampaign;
var		transient		UILabelButton	btnCampaign;
var		transient		UILabelButton	btnCreateParty;
var		transient		UILabelButton	btnHorde;
var		transient		UILabelButton	btnWarJournal;
var		transient		UILabelButton	btnTraining;
var		transient		UILabel			lblLatestNews;

var		transient		UILabel			lblPlayerName[2];

/** config values for enabling/disabling the buttons */
var config bool bContinueCampaignDisabled;
var config bool bCampaignDisabled;
var config bool bFindPartyDisabled;
var config bool bCreatePartyDisabled;
var config bool bHordeDisabled;
var config bool bWarJournalDisabled;
var config bool bTrainingDisabled;

/** Cached interface to the news functions */
var OnlineNewsInterface NewsInterface;

/** Cached number of empty memory slots */
var transient int NumEmptySaveSlots;
/** Cached number of used memory slots */
var transient int NumUsedSaveSlots;

var			UIScene	WarJournalScene;

/** The buttonbar that has the DLC button in it */
var transient UICalloutButtonPanel ButtonBar;

/**
 * Indicates that the player has just signed-in and a profile is pending.  Prevents the scene's state from being updated until the
 * player's profile has been loaded.
 */
var	transient	bool	bPendingProfileRefresh;

function SetPendingProfileRefresh( bool bValue )
{
`if(`isdefined(FIXING_SIGNIN_ISSUES))
	`log(`location @ `showvar(bPendingProfileRefresh) @ `showvar(bValue));
	ScriptTrace();
`endif
	bPendingProfileRefresh = bValue;
}

/* == Delegates == */

/* == UnrealScript == */

/* === UIScreenObject interface === */
/**
 * Called after this screen object's children have been initialized.  While the Initialized event is only called when
 * a widget is initialized for the first time, PostInitialize() will be called every time this widget receives a call
 * to Initialize(), even if the widget was already initialized.  Examples would be reparenting a widget.
 */
event PostInitialize()
{
	Super.PostInitialize();

	WarJournalScene = UIScene(DynamicLoadObject("UI_Scenes_WarJournal.TOC.WarJournalMainTOC", class'UIScene'));

	InitializeWidgetReferences();

	SetupCallbacks();
}

/**
 * Assigns all member variables to appropriate child widget from this scene.
 */
function InitializeWidgetReferences()
{
	local int PlayerIndex;

	ButtonBar			= UICalloutButtonPanel(FindChild('btnbarExtra',true));
	btnSoloCampaign		= UILabelButton(FindChild('btnSoloCampaign',true));
	btnCampaign			= UILabelButton(FindChild('btnCampaign',true));
	btnCreateParty		= UILabelButton(FindChild('btnCreateParty',true));
	btnHorde			= UILabelButton(FindChild('btnHorde',true));
	btnWarJournal		= UILabelButton(FindChild('btnWarJournal',true));
	btnTraining			= UILabelButton(FindChild('btnTraining',true));
	lblLatestNews		= UILabel(FindChild('lblLatestNews',true));

	lblPlayerName[0]	= UILabel(FindChild('lblProfile0',true));
	lblPlayerName[1]	= UILabel(FindChild('lblProfile1',true));

	ButtonBar.SetButtonCallback('DLC', OnDLCClicked);
	PlayerIndex = GetPlayerOwnerIndex();
	btnSoloCampaign.SetEnabled( !bCampaignDisabled, PlayerIndex );
	btnCampaign.SetEnabled( !bCampaignDisabled, PlayerIndex );
	btnCreateParty.SetEnabled( !bCreatePartyDisabled, PlayerIndex );
	btnHorde.SetEnabled( !bHordeDisabled, PlayerIndex );
	btnWarJournal.SetEnabled( !bWarJournalDisabled, PlayerIndex );
	btnTraining.SetEnabled( !bTrainingDisabled, PlayerIndex );

	RefreshNavigationIconWidgets();

	// Start reading the current news for the game
	ReadNews();
}

/** Set the initial focus of the buttons when entering the scene */
final function InitializeSceneFocus()
{
	local int PlayerIndex;

	PlayerIndex = GetPlayerOwnerIndex();

	// If the NewCampaign is disabled focus the Multiplayer
	if ( !bCampaignDisabled )
	{
		btnSoloCampaign.SetFocus(None, PlayerIndex);
	}
	else
	{
		btnCreateParty.SetFocus(None, PlayerIndex);
	}
}

/**
 * DLC was clicked
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool OnDLCClicked( UIScreenObject EventObject, int PlayerIndex )
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local int ControllerId;

	if ( PlayerIndex == 0 && ShouldEnableDLC() )
	{
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None)
		{
			PlayerIntEx = OnlineSub.PlayerInterfaceEx;
			if (PlayerIntEx != None)
			{
				ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
				PlayerIntEx.ShowContentMarketplaceUI(ControllerId);
			}
		}
	}

	return true;
}

/**
 * Set navigation markups for special buttons that show (NEW!) icon
 *
 * @fixme ronp - should we show the icon if either player has new items?
 */
function RefreshNavigationIconWidgets()
{
	local int PlayerIndex;
	local GearPC CurrGearPC;

	PlayerIndex = GetPlayerOwnerIndex();
	CurrGearPC = GetGearPlayerOwner(PlayerIndex);

	// See if WarJournal or TrainingGrounds should show (!NEW) icon
	if ( CurrGearPC != None && CurrGearPC.ProfileSettings != None )
	{
		if ( CurrGearPC.ProfileSettings.NavigationPathNeedsViewed(eNAVPATH_MainMenu_WarJournal) )
		{
			AddNavigationIconMarkupToLabelButton( eGEARNAVICON_NewStuff, btnWarJournal );
		}
		else
		{
			ClearNavigationIconMarkupToLabelButton( eGEARNAVICON_NewStuff, btnWarJournal );
		}

		if ( CurrGearPC.ProfileSettings.NavigationPathNeedsViewed(eNAVPATH_MainMenu_Training) )
		{
			AddNavigationIconMarkupToLabelButton( eGEARNAVICON_NewStuff, btnTraining );
		}
		else
		{
			ClearNavigationIconMarkupToLabelButton( eGEARNAVICON_NewStuff, btnTraining );
		}
	}
}

/**
 * Generic, light-weight function for updating the state of various widgets in the scene.
 *
 * @param	ControllerConnectionStatusOverrides		see UpdateProfileLabels.
 */
function UpdateScenePlayerState( optional array<bool> ControllerConnectionStatusOverrides )
{
	Super.UpdateScenePlayerState(ControllerConnectionStatusOverrides);

	// we might be showing the error message box
	UpdateProfileLabels(ControllerConnectionStatusOverrides);

	// update the enabled status of the DLC button
	//ButtonBar.EnableButton('DLC', 0, ShouldEnableDLC(), false);
	RefreshNavigationIconWidgets();
}

/**
 * Updates the player name labels to display the name of the signed in profile and the appropriate gamepad icon.
 *
 * @param	ControllerConnectionStatusOverrides		array indicating the connection status of each gamepad; should always contain
 *													MAX_SUPPORTED_GAMEPADS elements; useful when executing code as a result of a controller
 *													insertion/removal notification, as IsControllerConnected isn't reliable in that case.
 */
private function UpdateProfileLabels( optional array<bool> ControllerConnectionStatusOverrides )
{
	local int Idx, ControllerId[2];
	local array<int> ConnectedGamepads, UnusedGamepads;

	// get the controllerId of the primary player.
	ControllerId[0] = class'UIInteraction'.static.GetPlayerControllerId(0);
	ControllerId[1] = class'UIInteraction'.static.GetPlayerControllerId(1);

	// check the number of connected controllers.
	for ( Idx = 0; Idx < MAX_SUPPORTED_GAMEPADS; Idx++ )
	{
		// don't include the controllerId for the primary player, because we can't take his controller
		if ( Idx < ControllerConnectionStatusOverrides.Length )
		{
			if ( ControllerConnectionStatusOverrides[Idx] )
			{
				ConnectedGamepads.AddItem(Idx);
				if ( ControllerId[0] != Idx && ControllerId[1] != Idx )
				{
					UnusedGamepads.AddItem(Idx);
				}
			}
		}
		else if ( IsGamepadConnected(Idx) )
		{
			ConnectedGamepads.AddItem(Idx);
			if ( ControllerId[0] != Idx && ControllerId[1] != Idx )
			{
				UnusedGamepads.AddItem(Idx);
			}
		}
	}

	// if there is no player in one or both of the slots and we have connected gamepads that aren't being used, populate the slots with
	// the ids of the unused gamepads until we run out
	if ( ControllerId[0] == INDEX_NONE && UnusedGamepads.Length > 0 )
	{
		ControllerId[0] = UnusedGamepads[0];
		UnusedGamepads.Remove(0,1);
	}
	if ( ControllerId[1] == INDEX_NONE && UnusedGamepads.Length > 0 )
	{
		ControllerId[1] = UnusedGamepads[0];
		UnusedGamepads.Remove(0,1);
	}

	UpdateProfileLabel(lblPlayerName[1], 1, ControllerId[1], ConnectedGamepads);
	UpdateProfileLabel(lblPlayerName[0], 0, ControllerId[0], ConnectedGamepads);
}

/**
 * Worker method for UpdateProfileLabels().  Encapsulates the logic for determining what text should be displayed in a label, based on
 * the specified ControllerId and PlayerIndex.
 *
 * @param	ProfileStatusLabel	the label to update
 * @param	PlayerIndex			the index for the player that should be shown in this label, or INDEX_NONE if no profile is signed into
 *								the gamepad specified.
 * @param	ControllerId		the id of the gamepad being used by the player, or INDEX_NONE if there is no gamepad connected for this
 *								label, regardless of whether the player is signed in or not.
 */
protected function UpdateProfileLabel( UILabel ProfileStatusLabel, int PlayerIndex, int ControllerId, const out array<int> ConnectedGamepads )
{
	local LocalPlayer LP;
	local int PlayerCount, PrimaryControllerId;
	local string GamepadIconMarkup, PlayerName;
	local OnlineSubsystem OnlineSub;
	local bool bConnected, bHasProfile;

	if ( ControllerId >= 0 && ControllerId < MAX_SUPPORTED_GAMEPADS )
	{
		bConnected  =  (ConnectedGamepads.Length == 0 && IsGamepadConnected(ControllerId))
					||  ConnectedGamepads.Find(ControllerId) != INDEX_NONE;

		bHasProfile = IsLoggedin(ControllerId);
	}


//`log(`location @ `showobj(ProfileStatusLabel) @ `showvar(PlayerIndex) @ `showvar(ControllerId) @ `showvar(bConnected) @ `showvar(bHasProfile)
//	@ `showvar(class'UIInteraction'.static.GetPlayerCount(),PlayerCount) @ `showvar(class'UIInteraction'.static.GetConnectedGamepadCount(),ConnectedGamepads) @ `showvar(ProfileStatusLabel.PlayerInputMask,InputMask));

	if ( ProfileStatusLabel != None )
	{
		PlayerCount = class'UIInteraction'.static.GetPlayerCount();

		if ( PlayerIndex >= 0 && PlayerIndex < PlayerCount )
		{
			LP = GetPlayerOwner(PlayerIndex);
			OnlineSub = class'GameEngine'.static.GetOnlineSubSystem();
			if ( OnlineSub != None && OnlineSub.PlayerInterface != None )
			{
				PlayerName = OnlineSub.PlayerInterface.GetPlayerNickname(ControllerId);
			}
			else if ( LP != None )
			{
				PlayerName = LP.Actor.PlayerReplicationInfo.PlayerName;
			}
		}

		// means the gamepad is connected - so we're definitely going to show the icon
		GamepadIconMarkup = GetControllerIconString(ControllerId, true) $ " ";
		PrimaryControllerId = class'UIInteraction'.static.GetPlayerControllerId(0);

		if ( PlayerName == "" )
		{
			// if this is not the primary player
			if (PlayerIndex != 0

			// and the gamepad is connected
			&&	bConnected

			// and the primary player is signed into live
			&&	IsLoggedIn(PrimaryControllerId, true)

			// and the primary player is allowed to have guests
			&&	CanPlayOnline(PrimaryControllerId))
			{
				// gamepad is active but not signed-in; display the sign-in prompt
				PlayerName = "<Strings:GearGameUI.FrontEndStrings.GuestSignin>";

				// enable guest sign-in; allow any connected gamepad to press Y to sign-in.
				ProfileStatusLabel.SetInputMask(0xE, true, true);
			}
			else if ( bConnected )
			{
				// if the controller is signed in, we should have a valid PlayerName - don't show the 'not signed in' text
				if ( !bHasProfile )
				{
					PlayerName = "<Strings:GearGameUI.FrontEndStrings.NotSignedInProfileLabel>";
					ProfileStatusLabel.SetInputMask(0, true, true);
				}
			}
			else
			{
				if ( PlayerIndex == 0 )
				{
					PlayerName = "<Color:R=0.45,G=0.45,B=0.45,A=1.0><Strings:GearGameUI.FrontEndStrings.GamepadNotConnected><Color:/>";
				}
				else
				{
					// make the label go away!
					PlayerName = "";
					GamepadIconMarkup = "";
				}
				ProfileStatusLabel.SetInputMask(0, true, true);
			}
		}
		else
		{
			ProfileStatusLabel.SetInputMask(0, true, true);
		}

		ProfileStatusLabel.StringRenderComponent.bReapplyFormatting = true;
		ProfileStatusLabel.SetDataStoreBinding(GamepadIconMarkup $ PlayerName);
	}
}

`define AssignOnClickHandler(child)	if ( `child != None ) { `child.OnClicked = NavButtonClicked; }

/**
 * Assigns delegates in important child widgets to functions in this scene class.
 */
function SetupCallbacks()
{
	`AssignOnClickHandler(btnSoloCampaign);
	`AssignOnClickHandler(btnCampaign);
	`AssignOnClickHandler(btnCreateParty);
	`AssignOnClickHandler(btnHorde);
	`AssignOnClickHandler(btnWarJournal);
	`AssignOnClickHandler(btnTraining);
}

/** Wipe the memory slot cache and check for a storage device */
final function CheckStorageDevice( optional delegate<OnDeviceMemoryCheckComplete> CheckCompleteCallback=OnSceneActivatedDeviceCheckComplete )
{
`if(`isdefined(FIXING_SIGNIN_ISSUES))
ScriptTrace();
`log(`location @ `showvar(PlayerOwner) @ `showvar(PlayerOwner.ControllerId) @ `showvar(GetGearEngine().GetCurrentDeviceID(),CurrentDeviceId));
`endif
	// Clear the cache
	ClearMemorySlotAvailabilityCache();
//`log(`location @ `showvar(CheckCompleteCallback));
	// Ask for a device if there currently isn't a valid one
	CheckForMemoryDevice( CheckCompleteCallback );
}

/** Delegate called when the memory device ID check is complete when the scene is activated */
function OnSceneActivatedDeviceCheckComplete( bool bWasSuccessful )
{
	local int PlayerIndex;

	PlayerIndex = class'UIInteraction'.static.GetPlayerIndex(GetBestControllerId());

	// Clear the delegate from the online subsystem
	ClearDeviceMemoryCheckCompleteDelegate( PlayerIndex, OnSceneActivatedDeviceCheckComplete );

//`log(`location @ `showvar(bWasSuccessful));
	BuildMemorySlotAvailabilityCache();
}

/** Returns the number of empty and used memory slots */
function GetMemorySlotAvailability( out int EmptySlots, out int UsedSlots )
{
	if ( NumEmptySaveSlots == -1 || NumUsedSaveSlots == -1 )
	{
		BuildMemorySlotAvailabilityCache();
	}

	EmptySlots = NumEmptySaveSlots;
	UsedSlots = NumUsedSaveSlots;
}

/** Builds the slot availability cache */
final function BuildMemorySlotAvailabilityCache()
{
	local GearEngine Engine;
	local GearPC CurrGearPC;
	local int Idx, PlayerIndex, DeviceID;
	local CheckpointEnumerationResult EnumResult;

	NumEmptySaveSlots = 0;
	NumUsedSaveSlots = 0;
	PlayerIndex = GetPlayerOwnerIndex();
	CurrGearPC = GetGearPlayerOwner(PlayerIndex);

//	`log(`location @ `showvar(PlayerIndex) @ `showvar(CurrGearPC));
	if ( PlayerIndex == 0 && CurrGearPC != None )
	{
		Engine = GearEngine(CurrGearPC.Player.Outer);
		if ( Engine != None )
		{
			if ( StorageDeviceIsReady(PlayerIndex, DeviceID) )
			{
				// Set the checkpoint system to look at the proper player and device
				Engine.CurrentUserID = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
				Engine.SetCurrentDeviceID(DeviceID);
//`log("00000000" @ `showvar(DeviceID));
				// Find all the checkpoints and increment counters accordingly
				Engine.FindCheckpointData(INDEX_NONE, EnumResult);
				for (Idx = 0; Idx < eGEARCAMPMEMORYSLOT_MAX; Idx++)
				{
					if ( EnumResult.bCheckpointFileExists[Idx] != 0 && EnumResult.bCheckpointFileContainsData[Idx] != 0 )
					{
						NumUsedSaveSlots++;
					}
					else
					{
						NumEmptySaveSlots++;
					}
				}
			}
		}
	}
}

/** Clears the cached memory slot availability variables */
final function ClearMemorySlotAvailabilityCache()
{
	NumEmptySaveSlots = -1;
	NumUsedSaveSlots = -1;
}

/**
 * Handler for the leaderboard list's OnRawInputKey delegate.  In Gears2, the triggers are not linked to the page up / page down input alias,
 * so for lists that need to support this, we must handle it manually.
 *
 * @param	EventParms	information about the input event.
 *
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool OnReceivedInputKey( const out InputEventParameters EventParms )
{
	local int PrimaryControllerId;
	local OnlineSubsystem OnlineSub;
	local bool bResult;

	if ( EventParms.PlayerIndex == 1 && EventParms.InputKeyName == 'XboxTypeS_Y' )
	{
		if ( EventParms.EventType == IE_Released )
		{
			OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
			PrimaryControllerId = class'UIInteraction'.static.GetPlayerControllerId(0);
			if (OnlineSub != None && IsLoggedIn(PrimaryControllerId, true)
			&&	CanPlayOnline(PrimaryControllerId) && !IsLoggedIn(EventParms.ControllerId))
			{
				// Prompt for login
				OnlineSub.PlayerInterface.ShowLoginUI(true);
			}
		}

		bResult = true;
	}

	return bResult;
}

/**
 * Handler for the OnClick delegate for all of the nav buttons in this scene.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool NavButtonClicked(UIScreenObject EventObject, int PlayerIndex)
{
	local bool bResult;
	local UILabelButton Sender;
	local int ControllerId, PlayerCount;

	ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
	PlayerCount = class'UIInteraction'.static.GetPlayerCount();
	Sender = UILabelButton(EventObject);
	switch ( Sender )
	{
	case btnSoloCampaign:
		SetDataStoreStringValue("<Registry:MainMenu_Selection>", string(Sender.WidgetTag), Self, GetPlayerOwner(PlayerIndex));
		// Clear all the navigation values we store to know where we are in the FE
		ClearAllTransitionValues();
		// Leave a breadcrumb of where we came from
		SetTransitionValue("Campaign", "Solo");
		// Attempt a solo campaign
		SoloCampaign(PlayerIndex);
		bResult = true;
		break;
	case btnCampaign:
		SetDataStoreStringValue("<Registry:MainMenu_Selection>", string(Sender.WidgetTag), Self, GetPlayerOwner(PlayerIndex));
		CampaignMenu( PlayerIndex );
		bResult = true;
		break;

	case btnTraining:
		SetDataStoreStringValue("<Registry:MainMenu_Selection>", string(Sender.WidgetTag), Self, GetPlayerOwner(PlayerIndex));
		OpenTraining(PlayerIndex);
		bResult = true;
		break;

	case btnCreateParty:
	case btnHorde:
		SetDataStoreStringValue("<Registry:MainMenu_Selection>", string(Sender.WidgetTag), Self, GetPlayerOwner(PlayerIndex));
		if ( class'UIInteraction'.static.GetLoggedInPlayerCount(false) == PlayerCount || !IsConsole() )
		{
			TransitionToPartyLobby(PlayerIndex, Sender == btnHorde);
		}
		else
		{
			PromptForProfileNotSignedInMP(PlayerIndex, Sender == btnHorde);
		}
		bResult = true;
		break;

	case btnWarJournal:
		SetDataStoreStringValue("<Registry:MainMenu_Selection>", string(Sender.WidgetTag), Self, GetPlayerOwner(PlayerIndex));
		if ( IsLoggedIn(ControllerId, false) || !IsConsole() )
		{
			if ( IsGuestAccount(ControllerId) )
			{
				DisplayErrorMessage("IllegalGuestAction_Message", "IllegalGuestAction_Title", "GenericAccept", GetPlayerOwner(PlayerIndex));
			}
			else
			{
				OpenWarJournal(PlayerIndex);
			}
		}
		else
		{
			DisplayErrorMessage("NeedProfileForWarJournal_Message", "GamerProfileRequiredErrorTitle", "GenericAccept", GetPlayerOwner(PlayerIndex));
		}
		bResult = true;
		break;

	case None:
		`warn(`location$": received OnClick for non-UILabelButton" @ `showobj(EventObject) @ `showvar(PlayerIndex));
		break;

	default:
		`log(`location$": OnClick not processed for" @ `showobj(EventObject) @ `showvar(PlayerIndex));
		bResult = true;
		break;
	}


	return bResult;
}

/** Prompts for continuing into MP without a profile signed in */
final function PromptForProfileNotSignedInMP( int Playerindex, bool bIsHorde )
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	ButtonAliases.AddItem('GenericCancel');
	ButtonAliases.AddItem('AcceptContinueAnyway');

	GameSceneClient = GetSceneClient();
	GameSceneClient.ShowUIMessage(
		'MPProfileCheck',
		"<Strings:GearGameUI.MessageBoxStrings.NoSignInMP_Title>",
		"<Strings:GearGameUI.MessageBoxStrings.NoSignInMP_Message>",
		"<Strings:GearGameUI.MessageBoxStrings.NoSignInMP_Question>",
		ButtonAliases,
		bIsHorde ? OnPromptForProfileNotSignedInHordeComplete : OnPromptForProfileNotSignedInMPComplete,
		GetPlayerOwner(PlayerIndex) );

}

/** Callback from PromptForProfileNotSignedInMP for normal MP games */
function bool OnPromptForProfileNotSignedInMPComplete( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	if ( SelectedInputAlias == 'AcceptContinueAnyway' )
	{
		Sender.OnSceneDeactivated = OnPromptForProfileNotSignedInMPComplete_Finished;
	}
	return true;
}

/** Callback from OnPromptForProfileNotSignedInMPComplete for normal MP games after animation has faded us out */
function OnPromptForProfileNotSignedInMPComplete_Finished( UIScene ClosingScene )
{
	TransitionToPartyLobby(`PRIMARY_PLAYER_INDEX, false );
}

/** Callback from PromptForProfileNotSignedInMP for Horde games */
function bool OnPromptForProfileNotSignedInHordeComplete( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	if ( SelectedInputAlias == 'AcceptContinueAnyway' )
	{
		Sender.OnSceneDeactivated = OnPromptForProfileNotSignedInHordeComplete_Finished;
	}
	return true;
}

/** Callback from OnPromptForProfileNotSignedInHordeCompletefor normal MP games after animation has faded us out */
function OnPromptForProfileNotSignedInHordeComplete_Finished( UIScene ClosingScene )
{
	TransitionToPartyLobby( `PRIMARY_PLAYER_INDEX, true );
}

/** Opens the party lobby */
final function TransitionToPartyLobby( int PlayerIndex, bool bIsHorde )
{
	local GearProfileSettings Profile;
	local GearPC PC;

	if ( bIsHorde )
	{
		Profile = GetPlayerProfile(PlayerIndex);
		Profile.SetProfileSettingValueId(Profile.VERSUS_GAMETYPE, eGEARMP_CombatTrials);
		Profile.SetProfileSettingValueInt(Profile.PlaylistId, GetBestHordePlaylistId());
	}

	PC = GetGearPlayerOwner(PlayerIndex);
	PC.SaveProfile(OnProfileWriteCompleteForPartyTransition);
}

function OnProfileWriteCompleteForPartyTransition(byte LocalUserNum, bool bWasSuccessful)
{
	local int PlayerIndex;
	local OnlinePlayerInterface PlayerInt;
	local OnlineSubsystem OnlineSub;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != none)
	{
		PlayerInt = OnlineSub.PlayerInterface;
		if (PlayerInt != None)
		{
			PlayerInt.ClearWriteProfileSettingsCompleteDelegate(LocalUserNum ,OnProfileWriteCompleteForPartyTransition);
		}
	}

	PlayerIndex = class'UIInteraction'.static.GetPlayerIndex(LocalUserNum);
	OpenCreatePartyLobby(PlayerIndex);
}

/** Returns the playlist id of the most sensical Horde match */
final function int GetBestHordePlaylistId()
{
	local OnlinePlaylistManager PlaylistMan;
	local OnlineSubsystem OnlineSub;
	local int PlaylistIndex, GameIdx;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None )
	{
		PlaylistMan = OnlinePlaylistManager(OnlineSub.GetNamedInterface('PlaylistManager'));
		if ( PlaylistMan != None )
		{
			for ( PlaylistIndex = 0; PlaylistIndex < PlaylistMan.Playlists.length; PlaylistIndex++ )
			{
				for ( GameIdx = 0; GameIdx < PlaylistMan.Playlists[PlaylistIndex].ConfiguredGames.length; GameIdx++ )
				{
					if ( PlaylistMan.Playlists[PlaylistIndex].ConfiguredGames[GameIdx].GameSettingsClassName == "GearGame.GearHordeSettings" )
					{
						return PlaylistMan.Playlists[PlaylistIndex].PlaylistId;
					}
				}
			}
		}
	}

	return 0;
}

/**
 * Notifies the online subsystem to create a party session and open the party lobby.
 *
 * @param	PlayerIndex		the index of the player that generated this event
 */
function OpenCreatePartyLobby( int PlayerIndex )
{
	local GearMenuGame MenuGame;
	local int ControllerId;

	MenuGame = GetMenuGameInfo();
	if ( MenuGame != None )
	{
		ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
		if ( ControllerId != 255 )
		{
			MenuGame.CreateParty(ControllerId);
		}
		else
		{
			`warn(`location@": Invalid ControllerId for PlayerIndex" @ PlayerIndex $ ":" @ ControllerId);
		}
	}
}

/** Open campaign menu after warning about various things (sign-in, device, etc) */
final function CampaignMenu( int PlayerIndex )
{
	local UIScene SceneToOpen;
	local GearPC GPC;

	if ( IsConsole() )
	{
		SceneToOpen = UIScene(DynamicLoadObject("UI_Scenes_Common.UI_FETran_Campaign", class'UIScene'));
		OpenScene(SceneToOpen);
	}
	else
	{
		GPC = GetGearPlayerOwner(PlayerIndex);
		GPC.ClientTravel("GearStart?game=GearGameContent.GearCampaignLobbyGame?listen", TRAVEL_Absolute);
	}
}

/**
 * Opens the War Journal scene.
 *
 * @param	PlayerIndex		the index of the player that generated this event
 */
function OpenWarJournal( int PlayerIndex )
{
	OpenScene(WarJournalScene, GetPlayerOwner(PlayerIndex));
}

/**
 * Opens the Training grounds.
 *
 * @param	PlayerIndex		the index of the player that generated this event
 */
function OpenTraining( int PlayerIndex )
{
	local int PlayerCount;
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local int ControllerId;

	// Don't allow more than one player in training grounds
	PlayerCount = class'UIInteraction'.static.GetPlayerCount();
	if ( PlayerCount > 1 )
	{
		ButtonAliases.AddItem('GenericContinue');
		GameSceneClient = class'UIInteraction'.static.GetSceneClient();
		GameSceneClient.ShowUIMessage('ConfirmOnePlayerTraining',
			"<Strings:GearGameUI.MessageBoxErrorStrings.OnePlayerOnlyTrain_Title>",
			"<Strings:GearGameUI.MessageBoxErrorStrings.OnePlayerOnlyTrain_Message>",
			"",
			ButtonAliases, CloseOnContinue, GetPlayerOwner(PlayerIndex));
		return;
	}
	// Continue attempting to create a training grounds session
	else
	{
		ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);

		// See if they are logged in
		if ( IsLoggedIn(ControllerId) )
		{
			BeginTraining(PlayerIndex);
		}
		else
		{
			ButtonAliases.AddItem('GenericCancel');
			ButtonAliases.AddItem('AcceptContinueAnyway');

			GameSceneClient = GetSceneClient();
			GameSceneClient.ShowUIMessage(
				'NoSighIn',
				"<Strings:GearGameUI.MessageBoxStrings.NoSignInSolo_Title>",
				"<Strings:GearGameUI.MessageBoxStrings.NoSignInSolo_Message>",
				"<Strings:GearGameUI.MessageBoxStrings.NoSignInSolo_Question>",
				ButtonAliases,
				OnNoSignInForTrainingGroundsComplete,
				GetPlayerOwner(PlayerIndex) );
		}
	}
}

/** Return from the sign in check for a training grounds attempt */
function bool OnNoSignInForTrainingGroundsComplete( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	// They are not signed in but want to continue
	if ( SelectedInputAlias == 'AcceptContinueAnyway' )
	{
		Sender.OnSceneDeactivated = OnNoSignInForTrainingGroundsComplete_FinishedClosing;
	}

	return true;
}

/** Called when the NoSighIn is finished closing */
function OnNoSignInForTrainingGroundsComplete_FinishedClosing( UIScene ClosingScene )
{
	BeginTraining(`PRIMARY_PLAYER_INDEX);
}

/** Handler to close the scene on click */
function bool CloseOnContinue(UIMessageBoxBase Sender,name SelectedInputAlias,int PlayerIndex)
{
	return true;
}

/** Go ahead and start training grounds */
function BeginTraining(int PlayerIndex)
{
	local GearPC PC;
	local UIScene TrainingScene;

	TrainingScene = UIScene(DynamicLoadObject("UI_Scenes_FE.UI_FrontEnd_Training", class'UIScene'));
	if ( TrainingScene != None )
	{
		if ( None != OpenScene(TrainingScene, GetPlayerOwner(PlayerIndex)) )
		{
			PC = GetGearPlayerOwner(PlayerIndex);
			if ( PC != None && PC.ProfileSettings != None )
			{
				PC.ProfileSettings.MarkNavPathAsViewed( eNAVPATH_MainMenu_Training );
			}
		}
	}
}

/**
 * Starts reading the latest news
 */
function ReadNews()
{
	local OnlineSubsystem OnlineSub;
	local int ControllerId;

	// Figure out if we have an online subsystem registered
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		ControllerId = GetBestControllerId();

		// Grab the news interface to verify the subsystem supports it
		NewsInterface = OnlineSub.NewsInterface;
		if (NewsInterface != None)
		{
			// Mark the news as being read
			lblLatestNews.SetDataStoreBinding("<Strings:GearGameUI.Info.ReadingNews>");

			// Start news read
			NewsInterface.AddReadNewsCompletedDelegate(OnReadGameNewsCompleted);
			NewsInterface.ReadNews(ControllerId,ONT_GameNews);

		}
		else
		{
			`Log("Missing the news interface object");
			lblLatestNews.SetDataStoreBinding("<Strings:GearGameUI.Errors.FailedToReadNews>");
		}
	}
	else
	{
		`Log("Missing the online subsystem");
		lblLatestNews.SetDataStoreBinding("<Strings:GearGameUI.Errors.FailedToReadNews>");
	}
}

function CleanupUngracefulReturnToMenu()
{
	local GearPC GearPO;

	GearPO = GetGearPlayerOwner(GetPlayerOwnerIndex());
	if ( GearPO != None )
	{
		if ( GetOnlineGameSettings() != None )
		{
			LeaveMatch();
		}
		else if ( GetPartySettings() != None )
		{
			ClosePartySession(PartyEndCompleteDestroy);
		}
	}
}

/* == Delegate handlers == */
/**
 * Delegate used in notifying the UI/game that the news read operation completed
 *
 * @param bWasSuccessful true if the read completed ok, false otherwise
 * @param NewsType the type of news this callback is for
 */
function OnReadGameNewsCompleted(bool bWasSuccessful,EOnlineNewsType NewsType)
{
	local string News;
	`Log("OnReadGameNewsCompleted("$bWasSuccessful$","$NewsType$")");

	NewsInterface.ClearReadNewsCompletedDelegate(OnReadGameNewsCompleted);

	if (bWasSuccessful)
	{
		// Ask the online code for the news
		News = NewsInterface.GetNews(GetBestControllerId(),ONT_GameNews);
		if (len(News) == 0)
		{
			News = "<Strings:GearGameUI.Errors.FailedToReadNews>";
		}
	}
	else
	{
		News = "<Strings:GearGameUI.Errors.FailedToReadNews>";
	}
	lblLatestNews.SetDataStoreBinding(News);
}

/**
 * Checks to see if the player has a profile
 *  - Prompts if there profile is not logged in
 *  - Checks for Gears1 and Delta achievements if there is one, or one is logged in
 *
 * @return	TRUE if the sign-in blade had to be shown.; FALSE if the player is signed into a profile
 */
function bool PerformInitialProfileCheck()
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInt;
	local int PlayerIndex, ControllerId;
	local bool bResult;

	// initialize to TRUE so that we don't block if e.g. we don't have an OnlineSubsystem...
	PlayerIndex = GetPlayerOwnerIndex();
	ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None )
	{
		//@fixme splitscreen - we're not checking the second player here....
		PlayerInt = OnlineSub.PlayerInterface;
		if ( PlayerInt != None && OnlineSub.SystemInterface != None )
		{
			// Is not logged in
			if ( PlayerInt.GetLoginStatus(ControllerId) == LS_NotLoggedIn )
			{
				// Prompt for login
				if ( PlayerInt.ShowLoginUI() )
				{
					bResult = true;
				}
				else
				{
					UpdateScenePlayerState();
				}
			}
		}
	}

	SetDataStoreStringValue("<Registry:InitialProfileCheck>", "1");
	return bResult;
}

/**
 * Handles the notification that the async read of the profile data is done
 *
 * @param bWasSuccessful whether the call succeeded or not
 */
function OnReadProfileComplete(byte LocalUserNum, bool bWasSuccessful)
{
	local GearPC PCOwner;
	local OnlineSubsystem OnlineSub;
	local GearProfileSettings Profile;
	local int PlayerIndex;

	PlayerIndex = class'UIInteraction'.static.GetPlayerIndex(LocalUserNum);
`log(`location@`showvar(LocalUserNum) @ `showvar(PlayerIndex) @ `showvar(bWasSuccessful) @ `showvar(GetBestControllerId(),OwningControllerId),,'PlayerManagement');

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None && OnlineSub.PlayerInterface != None )
	{
		// Clear our callback function per player
		OnlineSub.PlayerInterface.ClearReadProfileSettingsCompleteDelegate(LocalUserNum,OnReadProfileComplete);
	}

	if ( bWasSuccessful && PlayerIndex == 0 )
	{
		Profile = GetPlayerProfile(PlayerIndex);
		Profile.ApplyNavPathAttractionToProvider();

		PCOwner = GetGearPlayerOwner(PlayerIndex);
		if ( !PCOwner.bIsExternalUIOpen && IsSceneActive(true) )
		{
			if ( bPendingProfileRefresh )
			{
				// Silently check for Gears 1 achievements
				CheckForGears1Achievements();

				// update the cached list of stored save games now the profile has changed.
				// this might not work if we're displaying the sign-in UI....
				CheckStorageDevice();
			}

			SetPendingProfileRefresh(false);
		}
	}

	UpdateScenePlayerState();
}

function OnLoginCompleted(bool bIsOpening)
{
	local int PrimaryControllerId;
	local GearProfileSettings Profile;

	// If the player is finished logging (and actually did log in) check Gears1 achievements
	if ( !bIsOpening )
	{
`if(`isdefined(FIXING_SIGNIN_ISSUES))
`log(`location @ `showvar(bPendingProfileRefresh));
`endif
		PrimaryControllerId = class'UIInteraction'.static.GetPlayerControllerId(`PRIMARY_PLAYER_INDEX);
		if ( bPendingProfileRefresh && IsSceneActive(true) )
		{
			Profile = GetPlayerProfile(`PRIMARY_PLAYER_INDEX);
			if ( Profile == None || Profile.AsyncState != OPAS_Read )
			{
				if ( IsLoggedIn(PrimaryControllerId) )
				{
					// Silently check for Gears 1 achievements
					CheckForGears1Achievements();

					// update the cached list of stored save games now the profile has changed.
					// this might not work if we're displaying the sign-in UI....
					CheckStorageDevice();

					SetPendingProfileRefresh(false);
				}
			}
		}

		UpdateScenePlayerState();
	}
}

/**
 * Notification that the login status a player has changed.
 *
 * @param	ControllerId	the id of the gamepad for the player that changed login status
 * @param	NewStatus		the value for the player's current login status
 *
 * @return	TRUE if this scene wishes to handle the event and prevent further processing.
 */
function bool NotifyLoginStatusChanged( int ControllerId, ELoginStatus NewStatus )
{
	local OnlineSubsystem OnlineSub;
	local bool bResult, bIsPlayerOwner;
	local int PrimaryControllerId;

	bIsPlayerOwner = PlayerOwner != None && ControllerId == PlayerOwner.ControllerId;
	SetPendingProfileRefresh(bIsPlayerOwner);

//SetUTracing(true);
	bResult = Super.NotifyLoginStatusChanged(ControllerId, NewStatus);
//SetUTracing(false);

	if ( bIsPlayerOwner )
	{
		PrimaryControllerId = class'UIInteraction'.static.GetPlayerControllerId(`PRIMARY_PLAYER_INDEX);
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();

		if ( NewStatus == LS_NotLoggedIn )
		{
			ClearMemorySlotAvailabilityCache();
		}
		else if ( OnlineSub != None && OnlineSub.PlayerInterface != None )
		{
			OnlineSub.PlayerInterface.AddReadProfileSettingsCompleteDelegate(PrimaryControllerId, OnReadProfileComplete);
		}
		else
		{
			// Silently check for Gears 1 achievements
			CheckForGears1Achievements();
			CheckStorageDevice();
		}
	}


	return bResult;
}

/**
 * Called when a player has been removed from the list of active players (i.e. split-screen players)
 *
 * @param	PlayerIndex		the index [into the GamePlayers array] where the player was located
 * @param	RemovedPlayer	the player that was removed
 */
function NotifyPlayerRemoved( int PlayerIndex, LocalPlayer RemovedPlayer )
{
	local GearPC PCOwner;
	local bool bRemovingPlayerOwner;

	bRemovingPlayerOwner = PlayerOwner == RemovedPlayer;

	Super.NotifyPlayerRemoved(PlayerIndex, RemovedPlayer);

	PCOwner = GetGearPlayerOwner(`PRIMARY_PLAYER_INDEX);
`if(`isdefined(FIXING_SIGNIN_ISSUES))
`log(`location @ `showvar(PlayerIndex) @ `showvar(bRemovingPlayerOwner) @ `showvar(bPendingProfileRefresh) @ `showvar(PCOwner.bIsExternalUIOpen));
`endif
	if ( bRemovingPlayerOwner && bPendingProfileRefresh && !PCOwner.bIsExternalUIOpen )
	{
		// if the player that owned this menu is gone, re-check for gears1 achievements and verify storage device
		CheckForGears1Achievements();

		CheckStorageDevice();

		SetPendingProfileRefresh(false);
	}
}

/** Send async call to get the Gears 1 achievements */
final function CheckForGears1Achievements()
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local OnlinePlayerInterface PlayerInt;
	local int PlayerIndex, ControllerId;
	local GearPC PC;

`if(`isdefined(FIXING_SIGNIN_ISSUES))
ScriptTrace();
`log(`location @ `showvar(PlayerOwner) @ `showvar(PlayerOwner.ControllerId) @ `showvar(GetGearEngine().GetCurrentDeviceID(),CurrentDeviceId));
`endif

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	PlayerIndex = GetPlayerOwnerIndex();
	ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
	PC = GetGearPlayerOwner(PlayerIndex);

	if ( PC != None && PC.ProfileSettings != None && !PC.ProfileSettings.Gears1CharactersAreUnlocked() )
	{
		if ( OnlineSub != None )
		{
			PlayerIntEx = OnlineSub.PlayerInterfaceEx;
			PlayerInt = OnlineSub.PlayerInterface;
			if ( PlayerIntEx != None && PlayerInt != None && PlayerInt.GetLoginStatus(ControllerId) == LS_LoggedIn )
			{
				PlayerIntEx.AddReadAchievementsCompleteDelegate(ControllerId, OnReadGears1XBoxAchievementsComplete);
				if ( !PlayerIntEx.ReadAchievements(ControllerId, GEARS1_TITLEID_XBOX) )
				{
					PlayerIntEx.ClearReadAchievementsCompleteDelegate( ControllerId, OnReadGears1XBoxAchievementsComplete );
				}
			}
		}
	}
}

/**
 * Called when the async achievements read has completed
 *
 * @param TitleId the title id that the read was for (0 means current title)
 */
function OnReadGears1XBoxAchievementsComplete(int TitleId)
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local array<AchievementDetails> AchievementList;
	local int PlayerIndex, ControllerId;
	local GearPC PC;

	if ( TitleId == GEARS1_TITLEID_XBOX )
	{
		PlayerIndex = GetPlayerOwnerIndex();
		ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
		PC = GetGearPlayerOwner(PlayerIndex);

		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if ( OnlineSub != None && PC != None && PC.ProfileSettings != None )
		{
			PlayerIntEx = OnlineSub.PlayerInterfaceEx;
			if ( PlayerIntEx != None )
			{
				PlayerIntEx.ClearReadAchievementsCompleteDelegate( ControllerId, OnReadGears1XBoxAchievementsComplete );
				PlayerIntEx.GetAchievements( ControllerId, AchievementList, TitleId );
				CheckAchievementsForUnlockables( AchievementList, PC );

				// See if we should now check Gears PC for the unlocks
				if ( !PC.ProfileSettings.Gears1CharactersAreUnlocked() )
				{
					//@fixme splitscreen
					PlayerIntEx.AddReadAchievementsCompleteDelegate(ControllerId, OnReadGears1PCAchievementsComplete);
					if ( !PlayerIntEx.ReadAchievements(ControllerId, GEARS1_TITLEID_PC) )
					{
						PlayerIntEx.ClearReadAchievementsCompleteDelegate( ControllerId, OnReadGears1PCAchievementsComplete );
					}
				}
			}
		}
	}
}

/**
* Called when the async achievements read has completed
*
* @param TitleId the title id that the read was for (0 means current title)
*/
function OnReadGears1PCAchievementsComplete(int TitleId)
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local array<AchievementDetails> AchievementList;
	local int PlayerIndex, ControllerId;
	local GearPC PC;

	if ( TitleId == GEARS1_TITLEID_PC )
	{
		PlayerIndex = GetPlayerOwnerIndex();
		ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
		PC = GetGearPlayerOwner(PlayerIndex);

		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if ( OnlineSub != None && PC != None && PC.ProfileSettings != None )
		{
			PlayerIntEx = OnlineSub.PlayerInterfaceEx;
			if ( PlayerIntEx != None )
			{
				PlayerIntEx.ClearReadAchievementsCompleteDelegate( ControllerId, OnReadGears1PCAchievementsComplete );
				PlayerIntEx.GetAchievements( ControllerId, AchievementList, TitleId );
				CheckAchievementsForUnlockables( AchievementList, PC );
			}
		}
	}
}

/** Checks the achievement list for the proper achievements and then unlocks the characters */
final function CheckAchievementsForUnlockables( array<AchievementDetails> AchievementList, GearPC PC )
{
	local int Idx;

	for ( Idx = 0; Idx < AchievementList.length; Idx++ )
	{
		// Check for "Completed Act 1 on Casual" for Carmine
		if ( AchievementList[Idx].Id == 2 )
		{
			if ( AchievementList[Idx].bWasAchievedOnline || AchievementList[Idx].bWasAchievedOffline )
			{
				PC.ProfileSettings.MarkUnlockableAsUnlockedButNotViewed( eUNLOCK_Character_Carmine1, PC );
			}
		}
		// Check for "Time to Remember" for Minh
		else if ( AchievementList[Idx].Id == 20 )
		{
			if ( AchievementList[Idx].bWasAchievedOnline || AchievementList[Idx].bWasAchievedOffline )
			{
				PC.ProfileSettings.MarkUnlockableAsUnlockedButNotViewed( eUNLOCK_Character_Minh, PC );
			}
		}
		// Check for "A Dish Best Served Cold" for Raam
		else if ( AchievementList[Idx].Id == 25 )
		{
			if ( AchievementList[Idx].bWasAchievedOnline || AchievementList[Idx].bWasAchievedOffline )
			{
				PC.ProfileSettings.MarkUnlockableAsUnlockedButNotViewed( eUNLOCK_Character_Raam, PC );
			}
		}
	}
}

/**
 * Handler for the scene's OnSceneActivated delegate.  Called after scene focus has been initialized, or when the scene becomes active as
 * the result of another scene closing.
 *
 * @param	ActivatedScene			the scene that was activated
 * @param	bInitialActivation		TRUE if this is the first time this scene is being activated; FALSE if this scene has become active
 *									as a result of closing another scene or manually moving this scene in the stack.
 */
function SceneActivationComplete( UIScene ActivatedScene, bool bInitialActivation )
{
	local string PreviouslySelectedItem, OutStringValue;
	local UILabelButton Target;
	local GearMenuGame MenuGame;
	local bool bDisplayingSignInBlade, bProfileCheckRequired;
	local OnlineSubsystem OnlineSub;
	local GearPC PCOwner;

	if ( ActivatedScene == Self )
	{
		if ( bInitialActivation )
		{
			OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
			if (OnlineSub != None && OnlineSub.SystemInterface != None)
			{
				OnlineSub.SystemInterface.AddExternalUIChangeDelegate(OnLoginCompleted);
			}

			CleanupUngracefulReturnToMenu();

			// Do stuff that we only want to happen the first time we ever come to the main menu after the title screen
			GetDataStoreStringValue("<Registry:InitialProfileCheck>", OutStringValue);
			bProfileCheckRequired = OutStringValue != "1";
			if ( bProfileCheckRequired )
			{
				bDisplayingSignInBlade = PerformInitialProfileCheck();
			}

			// if we've already performed the initial check to determine whether the player is signed in [and show the login UI if
			// necessary], OR if we did need to perform the initial check but the player was signed in
			if ( bProfileCheckRequired && !bDisplayingSignInBlade )
			{
				// Silently check for Gears 1 achievements
				CheckForGears1Achievements();

				// Wipe the memory slot cache and check the storage device.
				// This call is inside this if block because -
				//
				// bProfileCheckRequired: only check the first time we visit the main menu (or if connected storage devices have changed,
				// which we aren't handling yet).  Otherwise, I will get the device selection UI everytime I come to the main menu
				// if I don't select a valid device (i.e. cancel the device selection blade or don't have anything connected).
				//
				// !bDisplayingSigninBlade: Calling ShowDeviceSelectionUI fails if the sign-in blade is still active.  This is why I have
				// another check in OnLoginCompleted.  Also, we need to wait until the player has finished reading their profile....
				CheckStorageDevice();
			}

			if ( GetDataStoreStringValue("<Registry:MainMenu_Selection>", PreviouslySelectedItem, Self, GetPlayerOwner(GetPlayerOwnerIndex())) )
			{
				Target = UILabelButton(FindChild(name(PreviouslySelectedItem),true));
				if ( Target != None )
				{
					Target.SetFocus(None, GetPlayerOwnerIndex());
				}
			}
			else
			{
				InitializeSceneFocus();
			}

			MenuGame = GearMenuGame(GetWorldInfo().Game);
			if (MenuGame != None && MenuGame.bShowCreditsAtStartup)
			{
				OpenScene(UIScene(FindObject("UI_Scenes_Credits.UI_FE_Credits", class'UIScene')));
			}

			if ( GetTransitionValue("CompletedTraining") != "" )
			{
				OpenTraining(0);
			}

			// Clear all the navigation values we store to know where we are in the FE
			ClearAllTransitionValues();
		}
		else
		{
			PCOwner = GetGearPlayerOwner(`PRIMARY_PLAYER_INDEX);
			bDisplayingSignInBlade = PCOwner.bIsExternalUIOpen;
			if ( bPendingProfileRefresh && !PCOwner.bIsExternalUIOpen )
			{
				// Silently check for Gears 1 achievements
				CheckForGears1Achievements();

				// update the cached list of stored save games now the profile has changed.
				// this might not work if we're displaying the sign-in UI....
				CheckStorageDevice();

				SetPendingProfileRefresh(false);
			}
		}

		if ( !bDisplayingSignInBlade )
		{
			UpdateScenePlayerState();
		}
		else
		{
			SetPendingProfileRefresh(true);
		}
	}
}

/** Called when the scene is closed */
function OnSceneDeactivatedCallback( UIScene DeactivatedScene )
{
	local OnlineSubsystem OnlineSub;
	local int PlayerIndex, ControllerId;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	PlayerIndex = GetPlayerOwnerIndex();
	ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);

	if ( OnlineSub != None )
	{
		if ( OnlineSub.PlayerInterfaceEx != None )
		{
			OnlineSub.PlayerInterfaceEx.ClearReadAchievementsCompleteDelegate( ControllerId, OnReadGears1XBoxAchievementsComplete );
			OnlineSub.PlayerInterfaceEx.ClearReadAchievementsCompleteDelegate( ControllerId, OnReadGears1PCAchievementsComplete );
		}

		if (OnlineSub != None && OnlineSub.SystemInterface != None)
		{
			OnlineSub.SystemInterface.ClearExternalUIChangeDelegate(OnLoginCompleted);
		}
	}
}

/**
 * Handler for the scene's OnProcessInputKey delegate.
 */
function bool ProcessInputKey( const out SubscribedInputEventParameters EventParms )
{
	local GearUIInteraction UIController;

	if ( EventParms.InputAliasName == 'CloseScene' )
	{
		SetDataStoreStringValue("<Registry:HasDisplayedTitleScreen>", "0");
		SetDataStoreStringValue("<Registry:InitialProfileCheck>", "0");
		if ( !IsEditor() )
		{
			UIController = GearUIInteraction(GetCurrentUIController());
			if ( UIController != None )
			{
				UIController.bAttractModeAllowed = true;
			}
		}
	}

	return false;
}



defaultproperties
{
	SceneOpenedCue=None
	OnSceneActivated=SceneActivationComplete
	OnSceneDeactivated=OnSceneDeactivatedCallback
	OnProcessInputKey=ProcessInputKey
	OnRawInputKey=OnReceivedInputKey
	SceneInputMode=INPUTMODE_Selective
	NumEmptySaveSlots=-1
	NumUsedSaveSlots=-1
}

