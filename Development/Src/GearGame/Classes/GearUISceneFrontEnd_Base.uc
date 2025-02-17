/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearUISceneFrontEnd_Base extends GearUIScene_Base
	abstract
	native(inherit)
	Config(inherit);


/************************************************************************/
/* Constants, structures, enums, etc.									*/
/************************************************************************/

/** Different methods by which this screen will exit */
enum GearSaveSlotReturnType
{
	eGEARSAVE_None,
	eGEARSAVE_Cancel,
	eGEARSAVE_NewCampaign,
	eGEARSAVE_UseCheckpoint,
};


/************************************************************************/
/* Member Variables                                                     */
/************************************************************************/

/** Coop game settings for launching the game straight from the main menu */
var transient GearCoopGameSettings CoopSettings;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

final function BeginAsyncTask()
{
	local GearGameUISceneClient GameSceneClient;

	GameSceneClient = GearGameUISceneClient(GetSceneClient());
	GameSceneClient.BeginAsyncTask();
}

final function bool EndAsyncTask()
{
	local GearGameUISceneClient GameSceneClient;

	GameSceneClient = GearGameUISceneClient(GetSceneClient());
	return GameSceneClient.EndAsyncTask();
}

final function bool HasOutstandingAsyncTasks()
{
	local GearGameUISceneClient GameSceneClient;

	GameSceneClient = GearGameUISceneClient(GetSceneClient());
	return GameSceneClient.HasOutstandingAsyncTasks();
}

/**
 * Kicks off an async task to end the 'Game' session.
 */
function LeaveMatch()
{
	local GearPC PC;

	PC = GetGearPlayerOwner(0);

	if (PC.WorldInfo.NetMode != NM_Client)
	{
		PC.WorldInfo.Game.TellClientsToReturnToPartyHost();
	}
	else
	{
		PC.ClientReturnToMainMenu();
	}
}

/**
 * Kicks off an async task to end the 'Party' session.
 */
function ClosePartySession( delegate<OnlineGameInterface.OnEndOnlineGameComplete> EndPartyCompleteDelegate )
{
	DisbandParty();
}

/**
 * Called when the EndOnlineGame task for the 'Party' session is completed.
 *
 * @param	SessionName		the name of the session that the task operated on
 * @param	bWasSuccessful	TRUE if the task was completed successfully.
 */
function PartyEndComplete(name SessionName,bool bWasSuccessful)
{
	local OnlineGameInterface GameInterface;

	GameInterface = GetOnlineGameInterface();

	`Log(SessionName$" was ended "$bWasSuccessful);
	GameInterface.ClearEndOnlineGameCompleteDelegate(PartyEndComplete);
	EndAsyncTask();

	if ( HasOutstandingAsyncTasks() )
	{
		`log(`location @ "ignored due to active asynch tasks pending completion - better error message coming soon!");
	}
	else
	{
		`Log(SessionName$" should now be searchable/joinable");
		BeginAsyncTask();
		GameInterface.AddUpdateOnlineGameCompleteDelegate(PartyUpdateComplete);
		GameInterface.UpdateOnlineGame('Party',GetPartySettings(),true);
	}
}

function PartyUpdateComplete(name SessionName,bool bWasSuccessful)
{
	local OnlineGameInterface GameInterface;

	GameInterface = GetOnlineGameInterface();

	`Log(SessionName$" update was completed "$bWasSuccessful);
	GameInterface.ClearUpdateOnlineGameCompleteDelegate(PartyUpdateComplete);
	EndAsyncTask();
}

/**
 * Called when the EndOnlineGame task for the 'Party' session is completed, in cases
 * where we want to completely tear down the party so that it no longer exists.
 *
 * @param	SessionName		the name of the session that the task operated on
 * @param	bWasSuccessful	TRUE if the task was completed successfully.
 */
function PartyEndCompleteDestroy(name SessionName,bool bWasSuccessful)
{
	local OnlineGameInterface GameInterface;

	DisbandParty();

	GameInterface = GetOnlineGameInterface();

	`Log(SessionName$" was ended "$bWasSuccessful);
	GameInterface.ClearEndOnlineGameCompleteDelegate(PartyEndCompleteDestroy);
	EndAsyncTask();

	DestroyParty();
}

/**
 * Kicks off an asynch task to destroy a 'Party' session.
 */
function DestroyParty()
{
	local OnlineGameInterface GameInterface;
	if ( HasOutstandingAsyncTasks() )
	{
		`log(`location @ "ignored due to active asynch tasks pending completion - better error message coming soon!");
	}
	else
	{
		GameInterface = GetOnlineGameInterface();

		BeginAsyncTask();
		GameInterface.AddDestroyOnlineGameCompleteDelegate(DestroyPartyComplete);
		// Destroy the registered game
		GameInterface.DestroyOnlineGame('Party');
	}
}

/**
 * Called when the DestroyOnlineGame task for the 'Party' session is completed.
 *
 * @param	SessionName		the name of the session that the task operated on
 * @param	bWasSuccessful	TRUE if the task was completed successfully.
 */
function DestroyPartyComplete( name SessionName, bool bWasSuccessful )
{
	local OnlineGameInterface GameInterface;

	GameInterface = GetOnlineGameInterface();

	`Log(SessionName$" was destroyed "$bWasSuccessful);
	GameInterface.ClearDestroyOnlineGameCompleteDelegate(DestroyPartyComplete);
	EndAsyncTask();

	if ( HasOutstandingAsyncTasks() )
	{
		`log(`location @ "ignored due to active asynch tasks pending completion - better error message coming soon!");
	}
	else
	{
//		if ( SessionName == 'Party' && bWasSuccessful )
//		{
//			DisbandParty();
//		}
	}
}

function DisbandParty()
{
	local GearMenuGame MenuGI;
	local GearPC GearPO;

	ClearMenuGameDelegates();
	MenuGI = GetMenuGameInfo();
	if ( MenuGI != None )
	{
		MenuGI.DisbandParty();
	}
	else
	{
		GearPO = GetGearPlayerOwner(0);
		GearPO.ReturnToMainMenu();
	}
}

/**
 * Wrapper for retrieving the online game settings object for the party session.  Might return None on clients if still in the party lobby.
 */
final function GearPartyGameSettings GetPartySettings()
{
	local OnlineSubsystem OnlineSub;
	local GearPartyGameSettings Result;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None && OnlineSub.GameInterface != None )
	{
		Result = GearPartyGameSettings(OnlineSub.GameInterface.GetGameSettings('Party'));
	}

	return Result;
}

/**
 * Wrapper for retrieving the online game settings object for the game session.  Might return None if no game session is registered.
 */
final function OnlineGameSettings GetOnlineGameSettings()
{
	local OnlineSubsystem OnlineSub;
	local OnlineGameSettings Result;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None && OnlineSub.GameInterface != None )
	{
		Result = OnlineSub.GameInterface.GetGameSettings('Game');
	}

	return Result;
}

function ClearMenuGameDelegates()
{
	local GearMenuGame MenuGI;

	MenuGI = GetMenuGameInfo();
	if ( MenuGI != None )
	{
		MenuGI.ClearOnlineDelegates();
	}
}

/**
 * Called when a new player has been added to the list of active players (i.e. split-screen join)
 *
 * @param	PlayerIndex		the index [into the GamePlayers array] where the player was inserted
 * @param	AddedPlayer		the player that was added
 */
function NotifyPlayerAdded( int PlayerIndex, LocalPlayer AddedPlayer )
{
	Super.NotifyPlayerAdded(PlayerIndex, AddedPlayer);

	UpdateButtonBarInputMask();
}

/**
 * Called when a player has been removed from the list of active players (i.e. split-screen players)
 *
 * @param	PlayerIndex		the index [into the GamePlayers array] where the player was located
 * @param	RemovedPlayer	the player that was removed
 */
function NotifyPlayerRemoved( int PlayerIndex, LocalPlayer RemovedPlayer )
{
	Super.NotifyPlayerRemoved(PlayerIndex, RemovedPlayer);

	UpdateButtonBarInputMask();
}

/**
 * Sets the PlayerInputMask for the common button bar [which has Options and WhatsUp] to allow all active players to interact with it.
 */
function UpdateButtonBarInputMask()
{
	local UICalloutButtonPanel btnbarCommon;
	local UICalloutButton btnCommon;
	local int i, NumPlayers;
	local byte InputMask;

	if ( IsRuntimeInstance() )
	{
		// if this scene has a button bar with Options and/or WhatsUp buttons, make sure all players can access those
		btnbarCommon = UICalloutButtonPanel(FindChild('btnbarCommon', true));
		if ( btnbarCommon != None )
		{
			NumPlayers = class'UIInteraction'.static.GetPlayerCount();
			for ( i = 0; i < NumPlayers; i++ )
			{
				InputMask = InputMask | (1 << i);
			}
`if(`isdefined(FIXING_SIGNIN_ISSUES))
`log(`location @ `showvar(NumPlayers) @ `showvar(InputMask));
`endif
			btnCommon = btnbarCommon.FindButton('Options');
			if ( btnCommon != None )
			{
				btnCommon.SetInputMask(InputMask,,true);
			}

			btnCommon = btnbarCommon.FindButton('WhatsUp');
			if ( btnCommon != None )
			{
				btnCommon.SetInputMask(InputMask,,true);
			}
		}
	}
}

/**
 * Determines whether the DLC button should be enabled or not.
 */
function bool ShouldEnableDLC()
{
	local bool bResult;

	bResult = IsLoggedIn(class'UIInteraction'.static.GetPlayerControllerId(0), true);

	//@todo - add check for whether we actually have dlc?

	return bResult;
}

/* === UIScreenObject interface === */
/**
 * Called after this screen object's children have been initialized.  While the Initialized event is only called when
 * a widget is initialized for the first time, PostInitialize() will be called every time this widget receives a call
 * to Initialize(), even if the widget was already initialized.  Examples would be reparenting a widget.
 */
event PostInitialize()
{
	Super.PostInitialize();

	UpdateButtonBarInputMask();
}

/** Save the profiles of the players and launch the lobby */
function AdvanceToCampaignLobby( int PlayerIndex )
{
	local GearMenuGame MenuGame;
	local int ControllerId;
	local GearPC GPC;

	ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
	MenuGame = GetMenuGameInfo();

	if ( MenuGame != None )
	{
		if ( ControllerId != 255 )
		{
			// Save profile before creating the campaign
			GPC = GetGearPlayerOwner(PlayerIndex);
			if ( GPC != None )
			{
				//@todo - show 'don't turn off xbox' message
				GPC.SaveProfile( StartCampaignAfterProfileWrite );
			}
		}
		else
		{
			`warn(`location@": Invalid ControllerId for PlayerIndex" @ PlayerIndex $ ":" @ ControllerId);
		}
	}
}

/** Called when the profile is done writing - creates the campaign */
function StartCampaignAfterProfileWrite(byte LocalUserNum,bool bWasSuccessful)
{
	local int PlayerIndex, ControllerId;
	local GearMenuGame MenuGame;
	local GearPC PC;
	local string CampaignString;
	local string EnterLobbyString;

	PlayerIndex = class'UIInteraction'.static.GetPlayerIndex(LocalUserNum);
	ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);

	PC = GetGearPlayerOwner(PlayerIndex);
	if ( PC != None )
	{
		PC.ClearSaveProfileDelegate( StartCampaignAfterProfileWrite );
	}

	// Leave breadcrumbs so the lobby knows what mode it's in
	CampaignString = GetTransitionValue("Campaign");
	if (CampaignString == "Host")
	{
		EnterLobbyString = "MainMenu_HostCoop";
		SetTransitionValue("EnterLobby", EnterLobbyString);
	}
	else if (CampaignString == "Join")
	{
		EnterLobbyString = "MainMenu_JoinCoop";
		SetTransitionValue("EnterLobby", EnterLobbyString);
	}
	else if (CampaignString == "Split")
	{
		EnterLobbyString = "MainMenu_Splitscreen";
		SetTransitionValue("EnterLobby", EnterLobbyString);
	}
	else
	{
		`log("ERROR: GearUISceneFrontEnd:StartCampaignAfterProfileWrite: There are no Campaign breadcrumbs!!!  How did we get here?"@CampaignString);
	}

	MenuGame = GetMenuGameInfo();
	if ( MenuGame != None )
	{
		MenuGame.CampaignLobbyMode = class'GearUISceneFELobby_Campaign'.static.GetCampaignLobbyMode(EnterLobbyString);
		MenuGame.CreateCampaign(ControllerId, None, true, false, EChapterPoint(0));
	}
}

/** Do a device check so we can ask the player about a device if they haven't already selected one */
final function AttemptSoloCampaign( int PlayerIndex )
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local int ControllerId, DeviceID;

	ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);

	// See if they are logged in and warn them that they won't be able to save their game if they aren't signed in
	if ( IsLoggedIn(ControllerId) )
	{
		// There's a valid storage device so move to the save slot scene
		if ( StorageDeviceIsReady(PlayerIndex, DeviceID) )
		{
			AdvanceToSaveSlotSelection(PlayerIndex);
		}
		// No storage device so ask if they want to select one
		else
		{
			PromptForSelectStorageDeviceForSoloCampaign(PlayerIndex);
		}
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
			OnNoSignInForSoloCampaignComplete,
			GetPlayerOwner(PlayerIndex) );
	}
}

/** Prompts the player if they'd like to select a storage device for a quick campaign */
final function PromptForSelectStorageDeviceForSoloCampaign( int PlayerIndex )
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	ButtonAliases.AddItem('CancelContinueAnyway');
	ButtonAliases.AddItem('AcceptSelectDevice');

	GameSceneClient = GetSceneClient();
	GameSceneClient.ShowUIMessage(
		'NoStorageDevice',
		"<Strings:GearGameUI.MessageBoxStrings.NoDeviceLoad_Title>",
		"<Strings:GearGameUI.MessageBoxStrings.NoDeviceLoad_Message>",
		"<Strings:GearGameUI.MessageBoxStrings.NoDeviceLoad_Question>",
		ButtonAliases,
		OnNoDeviceCheckSoloCampaignComplete,
		GetPlayerOwner(PlayerIndex) );
}

/** Return from the sign in check for a quick campaign attempt */
function bool OnNoSignInForSoloCampaignComplete( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	// They are not signed in but want to continue. Since you must have a profile to have a valid device to save to
	// we can skip the device check and go straight to the difficulty screen
	if ( SelectedInputAlias == 'AcceptContinueAnyway' )
	{
		Sender.OnSceneDeactivated = OnNoSignInForSoloCampaignComplete_FinishedClosing;
	}

	return true;
}

/** Called when the NoSighIn is finished closing */
function OnNoSignInForSoloCampaignComplete_FinishedClosing( UIScene ClosingScene )
{
	SetTransitionValue("CanSaveSlot", "No");
	SetTransitionValue("EnterDifficulty", "MainMenu_New");
	AdvanceToDifficultySelect(GetBestPlayerIndex());
}

/** Return from the prompt asking if the player wanted to select a device */
function bool OnNoDeviceCheckSoloCampaignComplete( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	// They choose to select a storage device so start the async task of opening the device selection
	if ( SelectedInputAlias == 'AcceptSelectDevice' )
	{
		Sender.OnSceneDeactivated = OnNoDeviceCheckSoloCampaignComplete_FinishedClosingYes;
	}
	// They decided to NOT select a storage device so send them straight to the difficulty scene
	else
	{
		Sender.OnSceneDeactivated = OnNoDeviceCheckSoloCampaignComplete_FinishedClosingNo;
	}

	return true;
}

/** Called when the NoStorageDevice is finished closing with a Yes answer */
function OnNoDeviceCheckSoloCampaignComplete_FinishedClosingYes( UIScene ClosingScene )
{
	CheckForMemoryDevice( OnSoloCampaignDeviceMemoryCheckComplete, true );
}

/** Called when the NoStorageDevice is finished closing with a No answer */
function OnNoDeviceCheckSoloCampaignComplete_FinishedClosingNo( UIScene ClosingScene )
{
	SetTransitionValue("CanSaveSlot", "No");
	SetTransitionValue("EnterDifficulty", "MainMenu_New");
	AdvanceToDifficultySelect(GetBestPlayerIndex());
}

/** Return from the device check that allows us to know if they do or do not have a valid save device selected */
function OnSoloCampaignDeviceMemoryCheckComplete( bool bWasSuccessful )
{
	local int PlayerIndex;
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;

	PlayerIndex = GetBestPlayerIndex();

	// Unregister our callback
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None )
	{
		PlayerIntEx = OnlineSub.PlayerInterfaceEx;
		if (PlayerIntEx != None)
		{
			PlayerIntEx.ClearDeviceSelectionDoneDelegate(class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex), OnSoloCampaignDeviceMemoryCheckComplete );
		}
	}

	// They have a valid device so continue to the save slot scene unless there is no saved game (we can skip in that case)
	if ( bWasSuccessful )
	{
		AdvanceToSaveSlotSelection(PlayerIndex);
	}
	// No device so loop back to the prompt that asks them if they want to select a device
	else
	{
		PromptForSelectStorageDeviceForSoloCampaign(PlayerIndex);
	}
}

/** See if we should go to the select save slot screen or if we can skip it due to them all being empty */
final function AdvanceToSaveSlotSelection( int PlayerIndex )
{
	local int EmptySlots, UsedSlots;
	local GearPC CurrGearPC;
	local string CampaignString;

	GetMemorySlotAvailability( EmptySlots, UsedSlots );

	CampaignString = GetTransitionValue("Campaign");
	SetTransitionValue("CanSaveSlot", "Yes");

	if (CampaignString == "Solo")
	{
		// If there are no used slots this is a new game or they deleted their saves so
		// set the checkpoint to use and move to either the difficulty screen or the chapter select screen
		if (UsedSlots <= 0)
		{
			// Set the checkpoint to the first empty slot
			CurrGearPC = GetGearPlayerOwner(PlayerIndex);
			if ( CurrGearPC != None && CurrGearPC.ProfileSettings != None )
			{
				CurrGearPC.ProfileSettings.SetProfileSettingValueInt(CurrGearPC.ProfileSettings.LastCheckpointSlot, 0);
			}

			// See if they've unlocked any chapters, if they have we go to the chapter select screen
			if (CurrGearPC != none &&
				CurrGearPC.ProfileSettings != none &&
				CurrGearPC.ProfileSettings.HasUnlockedAnyChapters())
			{
				AdvanceToActSelect(PlayerIndex);
			}
			// We have no record of them playing so go to the difficulty screen
			else
			{
				SetTransitionValue("EnterDifficulty", "MainMenu_New");
				AdvanceToDifficultySelect(PlayerIndex);
			}
		}
		else
		{
			// Open the save slot screen
			SetTransitionValue("EnterSaveSlot", "MainMenu_Solo");
			AdvanceToSaveSelect(PlayerIndex);
		}
	}
	else
	{
		// If there are no used slots this is a new game or they deleted their saves so
		// set the checkpoint to use and move to the lobby
		if (UsedSlots <= 0)
		{
			AdvanceToCampaignLobby(PlayerIndex);
		}
		else
		{
			// Leave breadcrumbs of where we were and where we're going
			if (CampaignString == "Host")
			{
				SetTransitionValue("EnterSaveSlot", "MainMenu_HostCoop");
			}
			else
			{
				SetTransitionValue("EnterSaveSlot", "MainMenu_Splitscreen");
			}
			// Open the save slot screen
			AdvanceToSaveSelect(PlayerIndex);
		}
	}
}

/** Opens the save slot select screen */
final function AdvanceToSaveSelect( int PlayerIndex )
{
	local UIScene SceneToOpen;
	SceneToOpen = UIScene(DynamicLoadObject("UI_Scenes_FE.UI_SelectSaveSlot", class'UIScene'));
	OpenScene(SceneToOpen);
}

/** Opens the Act select screen */
final function AdvanceToActSelect( int PlayerIndex )
{
	local UIScene SceneToOpen;
	local int DiffValue;

	DiffValue = DL_Casual;
	SetTransitionValue("SelectedDifficulty", String(DiffValue));
	SetTransitionValue("SelectedChapter", "0");
	SceneToOpen = UIScene(DynamicLoadObject("UI_Scenes_FE.UI_FE_SelectAct", class'UIScene'));
	OpenScene(SceneToOpen);
}

/** Opens the difficulty select screen */
final function AdvanceToDifficultySelect( int PlayerIndex )
{
	local UIScene SceneToOpen;
	local int DiffValue;

	DiffValue = DL_Casual;
	SetTransitionValue("SelectedDifficulty", String(DiffValue));
	SceneToOpen = UIScene(DynamicLoadObject("UI_Scenes_FE.UI_FESO_Difficulty", class'UIScene'));
	OpenScene(SceneToOpen);
}

/** Get the memory slot availability that was cached in the main menu */
function GetMemorySlotAvailability( out int EmptySlots, out int UsedSlots );

/** Attempt to advance to the campaign via the host lobby way */
final function AttemptCampaignLobby( int PlayerIndex )
{
	local int PlayerCount;
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	// If there is more than one player (splitscreen) tell them that they are being taken to the campaign lobby if they continue
	PlayerCount = class'UIInteraction'.static.GetPlayerCount();
	if ( PlayerCount > 1 )
	{
		ButtonAliases.AddItem('GenericCancel');
		ButtonAliases.AddItem('AcceptPlaySplitScreen');

		GameSceneClient = GetSceneClient();
		GameSceneClient.ShowUIMessage(
			'CampaignRedirect',
			"<Strings:GearGameUI.MessageBoxStrings.CampaignRedirect_Title>",
			"<Strings:GearGameUI.MessageBoxStrings.CampaignRedirect_Message>",
			"<Strings:GearGameUI.MessageBoxStrings.CampaignRedirect_Question>",
			ButtonAliases,
			OnCampaignRedirectComplete,
			GetPlayerOwner(PlayerIndex) );
	}
	// Continue attempting to create a lobby
	else
	{
		CampaignLobby(PlayerIndex);
	}
}

/** Attempt to advance to the campaign via the quick solo way */
final function SoloCampaign( int PlayerIndex )
{
	local int PlayerCount;
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	// If there is more than one player (splitscreen) tell them that they are being taken to the campaign lobby if they continue
	PlayerCount = class'UIInteraction'.static.GetPlayerCount();
	if ( PlayerCount > 1 )
	{
		ButtonAliases.AddItem('GenericCancel');
		ButtonAliases.AddItem('AcceptPlaySplitScreen');

		GameSceneClient = GetSceneClient();
		GameSceneClient.ShowUIMessage(
			'CampaignRedirect',
			"<Strings:GearGameUI.MessageBoxStrings.CampaignRedirect_Title>",
			"<Strings:GearGameUI.MessageBoxStrings.CampaignRedirect_Message>",
			"<Strings:GearGameUI.MessageBoxStrings.CampaignRedirect_Question>",
			ButtonAliases,
			OnCampaignRedirectComplete,
			GetPlayerOwner(PlayerIndex) );
	}
	// Continue attempting to create a solo campaign
	else
	{
		AttemptSoloCampaign(PlayerIndex);
	}
}

/** Return from asking players if they want to redirect from the solo campaign option to the campaign lobby */
function bool OnCampaignRedirectComplete( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	if ( SelectedInputAlias == 'AcceptPlaySplitScreen' )
	{
		Sender.OnSceneDeactivated = OnCampaignRedirectComplete_FinishedClosing;
	}

	return true;
}

/** Called when the CampaignRedirect is finished closing */
function OnCampaignRedirectComplete_FinishedClosing( UIScene ClosingScene )
{
	OnSplitCampaign(none, GetBestPlayerIndex());
}

/** Called when the splitscreen campaign button is clicked */
function bool OnSplitCampaign(UIScreenObject EventObject, int PlayerIndex)
{
	// Clear all the navigation values we store to know where we are in the FE
	ClearAllTransitionValues();
	// Leave a breadcrumb of where we came from
	SetTransitionValue("Campaign", "Split");
	// Attempt to go to the lobby
	SplitCampaign(PlayerIndex);
	return true;
}

/** Attempt to advance to the splitscreen campaign lobby */
final function SplitCampaign( int PlayerIndex )
{
	local int PlayerCount;
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	// If there is not 2 players tell them that they can't enter
	PlayerCount = class'UIInteraction'.static.GetPlayerCount();
	if (PlayerCount < 2)
	{
		ButtonAliases.AddItem('GenericContinue');
		GameSceneClient = GetSceneClient();
		GameSceneClient.ShowUIMessage('NotEnoughPlayers',
			"<Strings:GearGameUI.MessageBoxErrorStrings.NeedTwoPlayers_Title>",
			"<Strings:GearGameUI.MessageBoxErrorStrings.NeedTwoPlayers_Message>",
			"",
			ButtonAliases,
			OnNotEnoughPlayers_Confirmed);
	}
	// Continue attempting to create a splitscreen campaign
	else
	{
		CampaignLobby(PlayerIndex);
	}
}

/** Called when the error message closes for not enough players */
function bool OnNotEnoughPlayers_Confirmed( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	return true;
}

/** Attempt to start a new campaign */
final function CampaignLobby( int PlayerIndex )
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local int DeviceID, PlayerCount;
	local string CampaignString;

	PlayerCount = class'UIInteraction'.static.GetPlayerCount();

	// See if all controllers are logged in and warn them that they won't be able to save their game if they aren't signed in
	if ( class'UIInteraction'.static.GetLoggedInPlayerCount() < PlayerCount )
	{
		ButtonAliases.AddItem('GenericContinue');
		GameSceneClient = GetSceneClient();
		GameSceneClient.ShowUIMessage('NotEnoughPlayers',
			"<Strings:GearGameUI.MessageBoxErrorStrings.NotLoggedIn_Title>",
			"<Strings:GearGameUI.MessageBoxErrorStrings.AControllerNotLoggedIn_Message>",
			"",
			ButtonAliases,
			OnSomeoneNotSignedIn_Confirmed);
	}
	else
	{
		// There's a valid storage device so move to the save slot screen
		if ( StorageDeviceIsReady(PlayerIndex, DeviceID) )
		{
			CampaignString = GetTransitionValue("Campaign");
			if (CampaignString == "Join")
			{
				AdvanceToCampaignLobby(PlayerIndex);
			}
			else
			{
				AdvanceToSaveSlotSelection(PlayerIndex);
			}
		}
		// No storage device so ask if they want to select one
		else
		{
			PromptForSelectStorageDeviceForCampaignLobby(PlayerIndex);
		}
	}
}

/** Called when the error message closes for someone not signed in */
function bool OnSomeoneNotSignedIn_Confirmed( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	return true;
}

/** Prompts the player if they'd like to select a storage device for campaign */
final function PromptForSelectStorageDeviceForCampaignLobby( int PlayerIndex )
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	ButtonAliases.AddItem('CancelContinueAnyway');
	ButtonAliases.AddItem('AcceptSelectDevice');

	GameSceneClient = GetSceneClient();
	GameSceneClient.ShowUIMessage(
		'NoStorageDevice',
		"<Strings:GearGameUI.MessageBoxStrings.NoDeviceLoad_Title>",
		"<Strings:GearGameUI.MessageBoxStrings.NoDeviceLoad_Message>",
		"<Strings:GearGameUI.MessageBoxStrings.NoDeviceLoad_Question>",
		ButtonAliases,
		OnNoDeviceCheckCampaignLobbyComplete,
		GetPlayerOwner(PlayerIndex) );
}

/** Return from the prompt asking if the player wanted to select a device */
function bool OnNoDeviceCheckCampaignLobbyComplete( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	// They choose to select a storage device so start the async task of opening the device selection
	if ( SelectedInputAlias == 'AcceptSelectDevice' )
	{
		Sender.OnSceneDeactivated = OnNoDeviceCheckCampaignLobbyComplete_FinishedClosingYes;
	}
	// They decided to NOT select a storage device so send them straight to the lobby
	else
	{
		Sender.OnSceneDeactivated = OnNoDeviceCheckCampaignLobbyComplete_FinishedClosingNo;
	}

	return true;
}

/** Called when the NoStorageDevice is finished closing with a Yes answer */
function OnNoDeviceCheckCampaignLobbyComplete_FinishedClosingYes( UIScene ClosingScene )
{
	CheckForMemoryDevice( OnCampaignLobbyDeviceMemoryCheckComplete, true );
}

/** Called when the NoStorageDevice is finished closing with a No answer */
function OnNoDeviceCheckCampaignLobbyComplete_FinishedClosingNo( UIScene ClosingScene )
{
	local GearEngine GE;

	// don't bother the user with another prompt about storage devices.
	GE = GetGearEngine();
	GE.bShouldWriteCheckpointToDisk = false;

	AdvanceToCampaignLobby(GetBestPlayerIndex());
}

/** Return from the device check that allows us to know if they do or do not have a valid save device selected */
function OnCampaignLobbyDeviceMemoryCheckComplete( bool bWasSuccessful )
{
	local int PlayerIndex;
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local string CampaignString;

	PlayerIndex = GetBestPlayerIndex();

	// Unregister our callback
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None )
	{
		PlayerIntEx = OnlineSub.PlayerInterfaceEx;
		if (PlayerIntEx != None)
		{
			PlayerIntEx.ClearDeviceSelectionDoneDelegate(class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex), OnCampaignLobbyDeviceMemoryCheckComplete );
		}
	}

	// They have a valid device so continue to the campaign lobby scene
	if ( bWasSuccessful )
	{
		CampaignString = GetTransitionValue("Campaign");
		if (CampaignString == "Join")
		{
			AdvanceToCampaignLobby(PlayerIndex);
		}
		else
		{
			AdvanceToSaveSlotSelection(PlayerIndex);
		}
	}
	// No device so loop back to the prompt that asks them if they want to select a device
	else
	{
		PromptForSelectStorageDeviceForCampaignLobby(PlayerIndex);
	}
}

/** Starts the campaign */
function LaunchSoloCampaign(int PlayerIndex, bool bDoOverwriteCheck)
{
	local GearPC GPC;
	local EGearCheckpointUsage CheckpointType;
	local string UseCheckpointString;
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local GearEngine Engine;
	local int CurrSlot;

	GPC = GetGearPlayerOwner(PlayerIndex);
	if (GPC != None && GPC.ProfileSettings != None)
	{
		UseCheckpointString = GetTransitionValue("UseCheckpoint");
		CheckpointType = (UseCheckpointString == "Yes") ? eGEARCHECKPOINT_UseLast : eGEARCHECKPOINT_Restart;
		GPC.ProfileSettings.SetProfileSettingValueId(GPC.ProfileSettings.const.CampCheckpointUsage, CheckpointType);

		Engine = GearEngine(GPC.Player.Outer);
		// Prepare the checkpoint system for access
		if (PrepareCheckpointSystem(PlayerIndex))
		{
			// Retrieve the save slot being used
			CurrSlot = -1;
			GPC.ProfileSettings.GetProfileSettingValueInt(GPC.ProfileSettings.LastCheckpointSlot, CurrSlot);
			CurrSlot = Max(0, CurrSlot);

			// Set the checkpoint to use
			Engine.FindCheckpointData(CurrSlot);
			// Prompt when overwriting
			if (Engine.CurrentCheckpoint != None &&
				!Engine.CurrentCheckpoint.CheckpointIsEmpty() &&
				CheckpointType == eGEARCHECKPOINT_Restart &&
				bDoOverwriteCheck)
			{
				ButtonAliases.AddItem('GenericCancel');
				ButtonAliases.AddItem('AcceptOverwrite');
				GameSceneClient = GetSceneClient();
				GameSceneClient.ShowUIMessage(
					'NoStorageDevice',
					"<Strings:GearGameUI.MessageBoxStrings.SaveOverwrite_Title>",
					"<Strings:GearGameUI.MessageBoxStrings.SaveOverwriteGeneric_Message>",
					"<Strings:GearGameUI.MessageBoxStrings.SaveOverwrite_Question>",
					ButtonAliases,
					OnCheckpointOverwriteCheckComplete,
					GetPlayerOwner(PlayerIndex) );
			}
			else
			{
				GPC.SaveProfile( StartSoloCampaignAfterProfileWrite );
			}
		}
		// Couldn't prepare the checkpoint system (no device set)
		else
		{
			// Set the flag in the Engine for whether we are currently saving checkpoints to disk
			Engine.bShouldWriteCheckpointToDisk = false;
			GPC.SaveProfile( StartSoloCampaignAfterProfileWrite );
		}
	}
}

/** Return from the prompt asking if the player wanted to overwrite their saved campaign */
function bool OnCheckpointOverwriteCheckComplete( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	if ( SelectedInputAlias == 'AcceptOverwrite' )
	{
		Sender.OnSceneDeactivated = OnCheckpointOverwriteCheckComplete_FinishedClosing;
	}
	return true;
}

/** Called when the Checkpoint overwrite check is finished closing with a Yes answer */
function OnCheckpointOverwriteCheckComplete_FinishedClosing( UIScene ClosingScene )
{
	local GearPC GPC;

	// Save profile before creating the campaign
	GPC = GetGearPlayerOwner(GetBestPlayerIndex());
	if (GPC != none)
	{
		GPC.SaveProfile(StartSoloCampaignAfterProfileWrite);
	}
}

/** Called when the profile is done writing - creates the campaign */
function StartSoloCampaignAfterProfileWrite(byte LocalUserNum,bool bWasSuccessful)
{
	local int PlayerIndex, ActId, NormalizedChapter;
	local GearMenuGame MenuGame;
	local GearPC PC;
	local GearEngine Engine;
	local GearCampaignActData ActData;
	local bool bUsingCheckpoint;
	local EChapterPoint SelectedChapter;
	local int ChapNumber;
	local string UseCheckpointString;

	PlayerIndex = class'UIInteraction'.static.GetPlayerIndex(LocalUserNum);

	PC = GetGearPlayerOwner(PlayerIndex);
	PC.ClearSaveProfileDelegate( StartSoloCampaignAfterProfileWrite );

	MenuGame = GetMenuGameInfo();
	Engine = GearEngine(PC.Player.Outer);
	if ( MenuGame != None && Engine != None )
	{
		UseCheckpointString = GetTransitionValue("UseCheckpoint");
		bUsingCheckpoint = (UseCheckpointString == "Yes") && Engine.bShouldWriteCheckpointToDisk;

		// Create the Coop settings
		CoopSettings = new class'GearCoopGameSettings';

		// Set the act and chapter
		ActId = 0;
		SelectedChapter = 0;
		if (bUsingCheckpoint && Engine.CurrentCheckpoint != None)
		{
			SelectedChapter = Engine.CurrentCheckpoint.Chapter;
		}
		else
		{
			ChapNumber = int(GetTransitionValue("SelectedChapter"));
			SelectedChapter = EChapterPoint(ChapNumber);
		}
		// Get the act provider
		ActData = class'GearUIDataStore_GameResource'.static.GetActDataProviderUsingChapter(SelectedChapter);
		if ( ActData != None )
		{
			ActId = ActData.ActType;
		}

		CoopSettings.SetIntProperty(CoopSettings.PROPERTY_ACTNUM, ActId+1);
		NormalizedChapter = class'GearUIDataStore_GameResource'.static.GetActChapterProviderIndexFromChapterId(SelectedChapter);
		CoopSettings.SetIntProperty(CoopSettings.PROPERTY_CHAPTERNUM, NormalizedChapter+1);

		// Clear all the other transition we were tracking
		ClearMainMenuTransitionValues();

		// Create the game
		MenuGame.CampaignLobbyMode = eGCLOBBYMODE_SoloPath;
		MenuGame.CreateCampaign(LocalUserNum, CoopSettings, false, false, SelectedChapter);
	}
}

defaultproperties
{
	bAllowPlayerJoin=true
}
