/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUIScenePause_Base extends GearUIScene_Base
	abstract
	Config(UI);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Label button that quits the game */
var transient UILabelButton QuitButton;

/** Label button that returns you to the mp/coop lobby */
var transient UILabelButton QuitLobbyButton;

/** Label button that loads the last checkpoint */
var transient UILabelButton CheckpointButton;

/** Label button that restarts the current chapter */
var transient UILabelButton RestartChapterButton;

/** Label button that opens the options menu */
var transient UILabelButton OptionsButton;

/** Label button that resumes the game */
var transient UILabelButton ResumeButton;

/** Label button that opens the what's up screen */
var transient UILabelButton WUButton;

/** The sound cue to play when this screen is open */
var SoundCue LoopingAudioSoundResource;

/** AudioComponent for the looping sound */
var transient AudioComponent LoopingAudioAC;

/** Whether this is a Campaign scene or not */
var transient bool bIsCampaignScene;

/** The amount of time to block input when the scene is first opened */
var const float TotalInputBlockTime;

/** The time this scene was opened */
var transient float SceneOpenedTime;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/**
 * Called after this screen object's children have been initialized
 * Overloaded to initialized the references to this scene's widgets
 */
event PostInitialize()
{
	InitializeWidgetReferences();

	Super.PostInitialize();

	if (!IsEditor())
	{
		SetupCallbacks();
	}
}

/** Initializes the widget references for the UI scene */
function InitializeWidgetReferences()
{
	QuitButton = UILabelButton(FindChild('btnQuit', true));
	QuitLobbyButton = UILabelButton(FindChild('btnQuitLobby', true));
	CheckpointButton = UILabelButton(FindChild('btnLastCheckpoint', true));
	RestartChapterButton = UILabelButton(FindChild('btnRestartChapter', true));
	OptionsButton = UILabelButton(FindChild('btnOptions', true));
	ResumeButton = UILabelButton(FindChild('btnResume', true));
	WUButton = UILabelButton(FindChild('btnWU', true));
}

/** @return true if the return to lobby button can be used in this pause menu */
function bool CanReturnToLobby()
{
	local WorldInfo WI;
	local OnlineSubsystem OnlineSub;
	local OnlineGameSettings GameSettings;
	local GearGRI GRI;
	local bool bResult;
	local GearPC PC;
	local int PlayerIndex;

	WI = GetWorldInfo();
	GRI = GearGRI(WI.GRI);

	if (GRI != none)
	{
		// Multiplayer checks
		if (GRI.IsMultiPlayerGame())
		{
			OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
			if (OnlineSub != None && OnlineSub.GameInterface != None)
			{
				GameSettings = OnlineSub.GameInterface.GetGameSettings('Game');
			}
			// Don't let arbitrated matches know who the host is
			if (WI.NetMode == NM_ListenServer &&
				(GameSettings == None || !GameSettings.bUsesArbitration))
			{
				bResult = true;
			}
		}
		// Single player
		else
		{
			// If the player has a network connection and is the host
			if (WI.NetMode != NM_Client &&
				class'UIInteraction'.static.HasLinkConnection())
			{
				bResult = true;
			}
			else
			{
				// Check for splitscreen primary player
				PlayerIndex = GetPlayerOwnerIndex();
				if (PlayerIndex == 0)
				{
					PC = GetGearPlayerOwner(PlayerIndex);
					if (PC != none &&
						PC.IsSplitscreenPlayer())
					{
						bResult = true;
					}
				}
			}
		}
	}
	return bResult;
}

/** @return true if the return to lobby button can be used in this pause menu */
function bool IsArbitrated()
{
	local OnlineSubsystem OnlineSub;
	local OnlineGameSettings GameSettings;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None && OnlineSub.GameInterface != None)
	{
		GameSettings = OnlineSub.GameInterface.GetGameSettings('Game');
	}

	// Don't let arbitrated matches quit
	return (GameSettings != None && GameSettings.bUsesArbitration);
}

/**
 * Assigns delegates in important child widgets to functions in this scene class.
 */
function SetupCallbacks()
{
	local int PlayerIndex;
	local bool bIsPrimaryPlayer;
	local bool bIsServerPlayer;
	local UILabelButton FirstButton;
	local UILabelButton LastButton;

	FirstButton = none;
	LastButton = none;
	PlayerIndex = GetPlayerOwnerIndex();
	bIsPrimaryPlayer = PlayerIndex == 0;
	bIsServerPlayer = bIsPrimaryPlayer && GetWorldInfo().NetMode < NM_Client;

	// Resume
	if (ResumeButton != none)
	{
		FirstButton = ResumeButton;
	}

	// Checkpoint
	if (CheckpointButton != None)
	{
		if (bIsServerPlayer)
		{
			CheckpointButton.OnClicked = OnLastCheckpointClicked;
			// If there isn't a first button yet, but there's a checkpoint button enabled,
			// it should be the first button because we're in the gameover screen
			if (FirstButton == none)
			{
				FirstButton = CheckpointButton;
			}
		}
		else
		{
			// If checkpoint button is focused, we need to disable it and make options focused
			if (CheckpointButton.IsFocused())
			{
				CheckpointButton.KillFocus(none, PlayerIndex);
			}
			CheckpointButton.DisableWidget(PlayerIndex);
		}
	}

	// Restart Chapter
	if (RestartChapterButton != None)
	{
		if (bIsServerPlayer)
		{
			RestartChapterButton.OnClicked = OnRestartChapterClicked;
		}
		else
		{
			RestartChapterButton.DisableWidget(PlayerIndex);
		}
	}

	// Options option
	if (OptionsButton != none)
	{
		// If we still don't have a first button, we are a client in the gameover screen so mark the options as the first
		// Since we ARE a client on the gameover this is our only button so mark the last button too
		if (FirstButton == none)
		{
			FirstButton = OptionsButton;
		}
		LastButton = OptionsButton;
	}

	// What's up option
	if (WUButton != none)
	{
		if (!IsArbitrated() && bIsPrimaryPlayer)
		{
			LastButton = WUButton;
		}
		else
		{
			WUButton.DisableWidget(PlayerIndex);
		}
	}

	// Quit to Lobby
	if (QuitLobbyButton != None)
	{
		// Can we return to the previous lobby?
		if (CanReturnToLobby() && bIsPrimaryPlayer)
		{
			QuitLobbyButton.OnClicked = OnReturnToLobbyMenuClicked;
		}
		// We need to remove the button since clients can't do this
		else
		{
			QuitLobbyButton.DisableWidget(PlayerIndex);
		}
	}
	// Quit to main menu
	if (QuitButton != None)
	{
		// Only the primary has access to quit
		if (bIsPrimaryPlayer && !IsArbitrated())
		{
			QuitButton.OnClicked = OnReturnToMainMenuClicked;
			LastButton = QuitButton;
		}
		else
		{
			QuitButton.DisableWidget(PlayerIndex);
		}
	}

	// We have our first and last buttons so now set up the navigation and focus
	if (FirstButton != none && LastButton != none)
	{
		FirstButton.SetFocus(none, PlayerIndex);
		if (FirstButton != LastButton)
		{
			FirstButton.SetForcedNavigationTarget(UIFACE_Top, LastButton);
			LastButton.SetForcedNavigationTarget(UIFACE_Bottom, FirstButton);
		}
	}
	else
	{
		`log("ERROR: GearUIScenePause_Base - could not create navigation and focus properly, something is wrong!");
	}
}

/** Accessor for determining whether the currently selected gametype is horde */
final function bool IsHordeGametype()
{
	local GearGRI GRI;
	local string GameClassString;

	GRI = GetGRI();
	if (GRI != none)
	{
		GameClassString = ""$GRI.GameClass;
		if (GameClassString ~= "GearGameHorde")
		{
			return true;
		}
	}
	return false;
}

/** Called when the scene is activated so we can set the difficulty strings */
function OnSceneActivatedCallback( UIScene ActivatedScene, bool bInitialActivation )
{
	local GearPC PC;

	if (bInitialActivation)
	{
		if (LoopingAudioSoundResource != none)
		{
			PC = GetGearPlayerOwner(GetBestPlayerIndex());
			LoopingAudioAC = PC.CreateAudioComponent(LoopingAudioSoundResource, false, true);
			if (LoopingAudioAC != none)
			{
				LoopingAudioAC.bAllowSpatialization = false;
				LoopingAudioAC.bAutoDestroy = true;
				LoopingAudioAC.Play();
			}
		}

		// Keep track of when this scene was opened so we can block input
		SceneOpenedTime = GetWorldInfo().RealTimeSeconds;
	}
}

/** Called when the scene is closed so we can stop the music */
function OnSceneDeactivatedCallback( UIScene DeactivatedScene )
{
	if (LoopingAudioAC != none)
	{
		LoopingAudioAC.FadeOut(1.0f, 0.0f);
		LoopingAudioAC = none;
	}
}

/** Callback when the quit button is pressed */
function bool OnReturnToMainMenuClicked( UIScreenObject EventObject, int PlayerIndex )
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local WorldInfo WI;

	WI = GetWorldInfo();

	if ( WI != None && !IsBlockingInput() )
	{
		ButtonAliases.AddItem('GenericCancel');
		ButtonAliases.AddItem('GenericContinue');
		GameSceneClient = GetSceneClient();
		if ( WI.NetMode != NM_Client )
		{
			GameSceneClient.ShowUIMessage('ConfirmQuit',
				"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_Title>",
				"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_LeaderMessage>",
				"",
				ButtonAliases, OnReturnToMainMenuConfirmed, GetPlayerOwner(PlayerIndex));
		}
		else
		{
			GameSceneClient.ShowUIMessage('ConfirmQuit',
				"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_Title>",
				"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_MemberMessage>",
				"",
				ButtonAliases, OnReturnToMainMenuConfirmed, GetPlayerOwner(PlayerIndex));
		}
	}
	return true;
}

/** Callback when the user confirms leaving */
function bool OnReturnToMainMenuConfirmed( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local WorldInfo WI;
	local GearPC PC;
	local GearPC PCOwner;

	if ( SelectedInputAlias == 'GenericContinue' )
	{
		PCOwner = GetGearPlayerOwner(PlayerIndex);
		WI = GetWorldInfo();
		if ( PCOwner != None && WI != None )
		{
			// Save the profiles
			foreach WI.LocalPlayerControllers(class'GearPC', PC)
			{
				PC.SaveProfile();
			}

			foreach WI.AllControllers(class'GearPC', PC)
			{
				if (!PC.IsLocalPlayerController())
				{
					// Send a warning to the client
					PC.ClientSetProgressMessage(PMT_ConnectionFailure,
						"<Strings:GearGameUI.MessageBoxStrings.PartyDisbanded_Message>",
						"<Strings:GearGameUI.MessageBoxStrings.PartyDisbanded_Title>",
						true);
				}
			}
			PCOwner.ReturnToMainMenu();
		}
		CloseScene(Self);
	}
	return true;
}

/** Callback when the quit button is pressed */
function bool OnReturnToLobbyMenuClicked( UIScreenObject EventObject, int PlayerIndex )
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	if (!IsBlockingInput() && CanReturnToLobby())
	{
		ButtonAliases.AddItem('GenericCancel');
		ButtonAliases.AddItem('GenericContinue');
		GameSceneClient = GetSceneClient();
		GameSceneClient.ShowUIMessage('ConfirmQuit',
			"<Strings:GearGameUI.MessageBoxStrings.ReturnToLobby_Title>",
			"<Strings:GearGameUI.MessageBoxStrings.ReturnToLobby_Message>",
			"",
			ButtonAliases, OnReturnToLobbyConfirmed, GetPlayerOwner(PlayerIndex));
	}

	return true;
}

/** Callback when the user confirms leaving */
function bool OnReturnToLobbyConfirmed( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local WorldInfo WI;
	local GearPC PC;

	if ( SelectedInputAlias == 'GenericContinue' )
	{
		PC = GetGearPlayerOwner(0);
		WI = GetWorldInfo();
		if ( PC != None && WI != None )
		{
			WI.Game.TellClientsToReturnToPartyHost();
			// Save the profiles
			foreach WI.LocalPlayerControllers(class'GearPC', PC)
			{
				PC.SaveProfile();
			}
		}
		CloseScene(Self);
	}
	return true;
}

/** Callback when the load last checkpoint button is pressed */
function bool OnLastCheckpointClicked( UIScreenObject EventObject, int PlayerIndex )
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	if (!IsBlockingInput())
	{
		ButtonAliases.AddItem( 'GenericCancel' );
		ButtonAliases.AddItem( 'GenericAccept' );

		GameSceneClient = GetSceneClient();
		GameSceneClient.ShowUIMessage(
			'UpdatingPartyMessage',
			"<Strings:GearGame.Generic.ConfirmCheckPointLoad_Title>",
			"<Strings:GearGame.Generic.ConfirmCheckPointLoad>", "",
			ButtonAliases, LoadLastCheckpoint, GetPlayerOwner()
			);
	}

	return true;
}

/** Callback when the restart chapter button is pressed */
function bool OnRestartChapterClicked( UIScreenObject EventObject, int PlayerIndex )
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	if (!IsBlockingInput())
	{
		ButtonAliases.AddItem( 'GenericCancel' );
		ButtonAliases.AddItem( 'GenericAccept' );
		GameSceneClient = GetSceneClient();
		GameSceneClient.ShowUIMessage(
			'UpdatingPartyMessage',
			"<Strings:GearGame.Generic.ConfirmChapterLoad_Title>",
			"<Strings:GearGame.Generic.ConfirmChapterLoad>", "",
			ButtonAliases, RestartChapter, GetPlayerOwner()
			);
	}

	return true;
}

/** Load the last checkpoint */
function bool LoadLastCheckpoint( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local GearPC MyGearPC;

	if ( SelectedInputAlias == 'GenericAccept' )
	{
		// Make sure the looping audio stops since we're heading for a load screen
		if (LoopingAudioAC != none)
		{
			LoopingAudioAC.Stop();
			LoopingAudioAC = none;
		}

		if ( PlayerOwner != None && PlayerOwner.Actor != None )
		{
			MyGearPC = GearPC(PlayerOwner.Actor);
			MyGearPC.LoadCheckpoint();
		}
	}

	return true;
}

/** Restart the chapter */
function bool RestartChapter( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local GearPC MyGearPC;

	if ( SelectedInputAlias == 'GenericAccept' )
	{
		// Make sure the looping audio stops since we're heading for a load screen
		if (LoopingAudioAC != none)
		{
			LoopingAudioAC.Stop();
			LoopingAudioAC = none;
		}

		if ( PlayerOwner != None && PlayerOwner.Actor != None )
		{
			MyGearPC = GearPC(PlayerOwner.Actor);
			MyGearPC.RestartChapter();
		}
	}

	return true;
}

/** Whether to close the scene through normal input (B and Start) */
function bool CanCloseSceneWithInput()
{
	return true;
}

/**
* Callback function when the scene gets input
* @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
*/
function bool ProcessRawInput( out InputEventParameters EventParms )
{
	local LocalPlayer LP;

	if ( (EventParms.InputKeyName == 'XboxTypeS_B' || EventParms.InputKeyName == 'XboxTypeS_Start') )
	{
		LP = GetPlayerOwner();
		if ( LP != None && LP == GetPlayerOwner(EventParms.PlayerIndex) )
		{
			if ( EventParms.EventType == IE_Released && CanCloseSceneWithInput() )
			{
				//@todo ronp animation
				CloseScene();
			}

			return true;
		}
	}

	return false;
}

/**
 * Handler for the scene's OnProcessInputKey delegate.
 *
 * Called when an input key event is received which this widget responds to and is in the correct state to process.  The
 * keys and states widgets receive input for is managed through the UI editor's key binding dialog (F8).
 *
 * This delegate is called AFTER kismet is given a chance to process the input, but BEFORE any native code processes the input.
 *
 * @param	EventParms	information about the input event, including the name of the input alias associated with the
 *						current key name (Tab, Space, etc.), event type (Pressed, Released, etc.) and modifier keys (Ctrl, Alt)
 *
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool ProcessInputKey( const out SubscribedInputEventParameters EventParms )
{
	if ( EventParms.InputAliasName == 'CloseScene' )
	{
		//@todo ronp animation
		if ( CanCloseSceneWithInput() )
		{
			CloseScene();
		}

		return true;
	}

	return false;
}

/** Whether we are blocking input or not */
function bool IsBlockingInput()
{
	if (TotalInputBlockTime > 0 &&
		SceneOpenedTime > 0)
	{
		if ((GetWorldInfo().RealTimeSeconds - SceneOpenedTime) <= TotalInputBlockTime)
		{
			return true;
		}
	}
	return false;
}

defaultproperties
{
	TotalInputBlockTime=-1
	SceneOpenedTime=-1
	bIsCampaignScene=true
	OnSceneActivated=OnSceneActivatedCallback
	OnSceneDeactivated=OnSceneDeactivatedCallback
	OnRawInputKey=ProcessRawInput
	OnProcessInputKey=ProcessInputKey
	SceneRenderMode=SPLITRENDER_PlayerOwner
	SceneInputMode=INPUTMODE_MatchingOnly
	bPauseGameWhileActive=true

	Begin Object Name=SceneEventComponent
		DisabledEventAliases.Add(CloseScene)
	End Object
}
