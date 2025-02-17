/**
 * GearTutorial_Base
 *
 * Base class for tutorial objects...
 * A tutorial object is a self contained instance of an in-game tutorial managed by the GearTutorialManager which
 * is located in the GearPC.
 *
 * A tutorial's UIScene can be of 2 varieties: Input-based (have to press a button to make the screen go away) and Timed (which goes away itself).
 * Some tutorials will only be a message while others will require the player to do something or else the screen will come back.  The system
 * is flexible and allows each tutorial to be extensible and unique since each has its' own input handler and completion logic.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_Base extends Object
	config(Game)
	native
	abstract;

/** GoW global macros */


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/**------------ Variables to be configured in INI ----------------------*/

/** Whether this tutorial will freeze the player while the scene is displaying */
var config bool bFreezeGameplay;

/** Whether this tutorial will pause the game while the scene is displaying (will be ignored if there are any networked players) */
var config bool bPauseGameplay;

/** Amount of time the UI scene should remain open - if this value is <= 0 it is assumed that closing the scene is input driven */
var config float SceneOpenTimeAmount;

/** Amount of time to pass until the scene of this tutorial is displayed again - defaults to 0.0f which means that it doesn't repeat */
var config float ElapsedTimeToRepeatScene;

/** String to use to look up the body text of the tutorial */
var config string LocalizedBodyText;

/** String to use to look up the title text of the tutorial */
var config string LocalizedTitleText;

/** String to use to look up the "press button" text of the tutorial */
var config string LocalizedButtonText;

/** Alternate localized text for the body of the scene for keyboards */
var config string LocalizedKeyboardBodyText;

/** Alternate localized text for the body of the scene for alternate controls */
var config string LocalizedAlternateBodyText;

/** Alternate localized text for the "press button" text of the scene for alternate controls */
var config string LocalizedAlternateButtonText;

/** String to use to look up the title text of the tutorial on the host of a coop game */
var config string LocalizedCoopTitleTextHost;

/** String to use to look up the title text of the tutorial on the client of a coop game */
var config string LocalizedCoopTitleTextClient;

/** Alternate localized text to be used for the host of a coop game */
var config string LocalizedCoopBodyTextHost;

/** Alternate localized text to be used for the client of a coop game */
var config string LocalizedCoopBodyTextClient;

/** Whether we want the opening of the scene to NOT happen right away when the tutorial is activated - will wait for ElapsedTimeToRepeatScene amount of time */
var config bool bWaitForFirstTimeLapse;

/** Whether this tutorial can be turned off via the options menu or not */
var config bool bOptionControlled;

/**------------ Variables to be configured in default properties -------*/

/** The type of tutorial this is */
var EGearTutorialType TutorialType;

/** Class of the input handler used by this tutorial */
var class<GearPlayerInput_Base> InputHandlerClass;

/** The UIScene asset reference used by this tutorial */
var UIScene SceneReference;

/** Is ready to be activated at the tutorial manager's next opportunity */
var bool bTutorialIsReadyForActivation;

/**
* The priority of this tutorial.   This is so that if multiple tutorials are active in the tutorial manager
* we can determine which tutorial object will be next to be active.
*/
var int TutorialPriority;

/** Whether we should save this tutorial's completeness to the profile or not */
var bool bSaveTutorialToProfile;

/**------------ General Variables --------------------------------------*/

/** Reference to the tutorial manager controlling this tutorial object */
var transient GearTutorialManager TutorialMgr;

/** Reference to the input handler used by this tutorial */
var transient GearPlayerInput_Base InputHandler;

/** Reference to the kismet action that started this tutorial - this will be None for tutorials started from script */
var transient SeqAct_ManageTutorials TutorialAction;

/** Whether this tutorial was called from kismet or not */
var transient bool bIsKismetTutorial;

/** Whether this tutorial is the currently active one in the tutorial manager's list */
var transient bool bTutorialIsActive;

/** Tutorial has been completed - the tutorial manager will check this value and handle things accordingly */
var transient bool bTutorialIsComplete;

/** Whether the game is currently frozen by this tutorial or not - works in conjunction with bFreezeGameplay */
var transient bool bGameplayIsFrozen;

/** Whether the game is currently paused by this tutorial or not - works in conjunction with bPauseGameplay */
var transient bool bGameplayIsPaused;

/** Audio component to play the looping menu music on */
var transient AudioComponent MusicLoopAC;

/**------------ Variables for managing the UIScene ---------------------*/

/** Whether the tutorial has a scene to open or not (used for tutorials that don't want to display any UI) */
var transient bool bTutorialHasUIScene;

/** The UIScene instance of the above tutorial reference */
var transient UIScene SceneInstance;

/** Whether the scene is opened or not */
var transient bool bSceneIsOpen;

/** Whether the scene has ever been opened or not */
var transient bool bSceneHasBeenOpened;

/** Time since the scene of this tutorial opened */
var transient float TimeOfSceneOpen;

/** Time since the scene of this tutorial closed */
var transient float NextTimeForSceneOpen;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called when the tutorial gets added to the tutorial manager's list */
function OnTutorialAdded( GearTutorialManager TutMgr, SeqAct_ManageTutorials Action, bool IsKismetDriven )
{
	`DEBUGTUTORIAL("GearTutorial_Base::OnTutorialAdded: Type="$TutorialType@"Obj="$self@"Action="$Action@"IsKismet="$IsKismetDriven);
	TutorialMgr = TutMgr;
	TutorialAction = Action;
	bIsKismetTutorial = IsKismetDriven;
}

/** Called when the tutorial gets removed from the tutorial manager's list */
function OnTutorialRemoved()
{
	`DEBUGTUTORIAL("GearTutorial_Base::OnTutorialRemoved: Type="$TutorialType@"Obj="$self@"Action="$TutorialAction);
	// Unfreeze game (make sure the input isn't left frozen
	ToggleFreezeGameplay( false );
}

/**
 * Opportunity for the tutorial to override the filtering system for the input object handling this tutorial's input
 *		@Param Filtered - whether this button should be filtered or not (non zero means true)
 *		@Return - whether we've handled the filtering or not
 */
function bool FilterButtonInput( Name ButtonName, bool bPressed, int ButtonIdx, out int Filtered )
{
	return false;
}


/**
 * Called when this tutorial's input handler is created (if this tutorial has it's own handler)
 *		This is where a tutorial can add InputButtonData mappings to the handler.
 */
function InitializeInputHandler()
{
	`DEBUGTUTORIAL("GearTutorial_Base::InitializeInputHandler: Type="$TutorialType@"Obj="$self@"InputHandlerClass="$InputHandlerClass@"InputHandler="$InputHandler);
	if ( InputHandler != None )
	{
		InputHandler.OnInputObjectCreate();
	}
}

/** Called when the tutorial has become the active tutorial by the tutorial manager */
function bool TutorialActivated()
{
	`DEBUGTUTORIAL("GearTutorial_Base::TutorialActivated: Type="$TutorialType@"Obj="$self);
	bTutorialIsActive = true;

	// Freeze the game (not pause) if we need to
	if ( bFreezeGameplay )
	{
		ToggleFreezeGameplay( true );
	}

	// Create the input handler if this tutorial needs one and set it in the manager
	if ( InputHandlerClass != None )
	{
		// Create the input handler
		InputHandler = new(TutorialMgr.Outer) InputHandlerClass;
		if ( InputHandler != None )
		{
			`DEBUGTUTORIAL("GearTutorial_Base::TutorialActivated: Created InputHandler of class"@InputHandlerClass@"Type="$TutorialType@"Obj="$self);
			InitializeInputHandler();
		}
		else
		{
			`DEBUGTUTORIAL("GearTutorial_Base::TutorialActivated: Could not create InputHandler of class"@InputHandlerClass@"Type="$TutorialType@"Obj="$self);
		}
	}

	// Init the time for the scene to open
	ResetUISceneOpenTimer( !bWaitForFirstTimeLapse );

	return true;
}

/**
 * Called when the tutorial manager wants this tutorial to deactivate itself, such as a death or a scripted event etc.
 * This will not remove or complete the tutorial, but just make it inactive until further notice.
 * @Param bReadyForActivation - whether the "ready for activation" flag should remain on or not
 */
function bool TutorialDeactivated( optional bool bReadyForActivation )
{
	`DEBUGTUTORIAL("GearTutorial_Base::TutorialDeactivated: Type="$TutorialType@"Obj="$self@"InputHandler="$InputHandler);

	// If the scene is showing, close it
	if ( bSceneIsOpen )
	{
		CloseTutorialUIScene();
	}

	// Clear the reference for the input handler so it can be cleaned up
	InputHandler = None;

	// Unpause the game if we need to
	if ( bPauseGameplay )
	{
		TogglePauseGameplay( false );
	}

	// Unfreeze the game (not unpause) if we need to
	if ( bFreezeGameplay )
	{
		ToggleFreezeGameplay( false );
	}

	// Clear the activated flag
	bTutorialIsActive = false;

	// Make the ready flag false
	bTutorialIsReadyForActivation = bReadyForActivation;

	return true;
}

/** Called every tick so that the tutorial has a chance to update itself */
event Update()
{
	// See if this tutorial's UI scene should be open, and do so if needed
	if ( ShouldOpenUIScene() )
	{
		`DEBUGTUTORIAL("GearTutorial_Base::Update: ShouldOpenUIScene Type="$TutorialType@"Obj="$self);
		OpenTutorialUIScene();
	}

	// See if this tutorial's UI scene should be closed, and do so if needed
	if ( ShouldCloseUIScene() )
	{
		`DEBUGTUTORIAL("GearTutorial_Base::Update: ShouldCloseUIScene Type="$TutorialType@"Obj="$self);
		CloseTutorialUIScene();
	}

	// See if this tutorial should unpause the game because a networked player has joined the game
	if ( bGameplayIsPaused && !CanPauseGame() )
	{
		TogglePauseGameplay( false );
	}
}

/** Called when the tutorial's scene is finished opening */
function OnTutorialSceneOpened()
{
	`DEBUGTUTORIAL("GearTutorial_Base::OnTutorialSceneOpened: Type="$TutorialType@"Obj="$self);

	bSceneIsOpen = true;
	bSceneHasBeenOpened = true;
	TimeOfSceneOpen = TutorialMgr.WorldInfo.TimeSeconds;

	// Set the scene closed delegate
	SceneInstance.OnSceneDeactivated = OnTutorialSceneClosed;

	// Initialized the strings for the scene
	InitializeUISceneLabels();

	// Fire an event on the action if it exists
	if ( TutorialAction != None )
	{
		// Fire the "UI Scene Opened" output for the action
		TutorialAction.OutputLinks[eMTOUTPUT_UISceneOpened].bHasImpulse = true;

		`DEBUGTUTORIAL("GearTutorial_Base::OnTutorialSceneOpened: Fired UISceneOpened impulse for action"@TutorialAction@"Type="$TutorialType@"Obj="$self);
	}
}

/** Called when the tutorial's scene is finished closing */
function OnTutorialSceneClosed( UIScene DeactivatedScene )
{
	bSceneIsOpen = false;

	// Mark for when the next time the scene should be opened
	ResetUISceneOpenTimer( false );

	SceneInstance = None;

	`DEBUGTUTORIAL("GearTutorial_Base::OnTutorialSceneClosed: NextTimeForSceneOpen="$NextTimeForSceneOpen@"Type="$TutorialType@"Obj="$self);
}

/** Called when this tutorial is completed */
function OnTutorialCompleted()
{
	`DEBUGTUTORIAL("GearTutorial_Base::OnTutorialCompleted: Type="$TutorialType@"Obj="$self);

	bTutorialIsComplete = true;
	bTutorialIsReadyForActivation = false;

	// Set the kismet action's value as well for firing correct outputs (we are the host)
	if ( TutorialAction != None )
	{
		TutorialAction.bTutorialCompleted = bTutorialIsComplete;
		TutorialAction.bActionIsDone = true;
		`DEBUGTUTORIAL("GearTutorial_Base::OnTutorialCompleted: Informing action"@TutorialAction@"Type="$TutorialType@"Obj="$self);
		bIsKismetTutorial = false;
		TutorialMgr.TellClientTutorialCompletedFromServer(TutorialType);
	}
	// We are the client of a tutorial that was spawned via kismet so tell the server we are closing
	else if ( bIsKismetTutorial )
	{
		TutorialMgr.TellServerTutorialCompletedFromClient(TutorialType);
		bIsKismetTutorial = false;
		`DEBUGTUTORIAL("GearTutorial_Base::OnTutorialCompleted: Client is informing Host about action"@TutorialAction@"Type="$TutorialType@"Obj="$self);
	}
}

/** Determine if we should open the UI scene or not */
function bool ShouldOpenUIScene()
{
	//`log("---------> bTutorialIsActive="$bTutorialIsActive@"bSceneIsOpen="$bSceneIsOpen@"bSceneIsOpen="$bSceneHasBeenOpened@"ElapsedTimeToRepeatScene="$ElapsedTimeToRepeatScene@"NextTimeForSceneOpen="$NextTimeForSceneOpen@"TimeSeconds="$TutorialMgr.WorldInfo.TimeSeconds);
	if ( bTutorialIsActive && bTutorialHasUIScene && !bSceneIsOpen && ((!bSceneHasBeenOpened && !bWaitForFirstTimeLapse) || ((ElapsedTimeToRepeatScene > 0.0f) && (NextTimeForSceneOpen < TutorialMgr.WorldInfo.TimeSeconds))) )
	{
		return true;
	}
	return false;
}

/** Determine if we should close the UI scene or not */
function bool ShouldCloseUIScene()
{
	if ( bTutorialIsActive && bSceneIsOpen && ((SceneOpenTimeAmount > 0) && (TutorialMgr.WorldInfo.TimeSeconds > SceneOpenTimeAmount+TimeOfSceneOpen)) )
	{
		return true;
	}
	return false;
}

/**
 * Resets the timer for openeing the scene
 * @Param bInstantOpen - whether to open the scene right away or to wait for the time lapse
 */
final function ResetUISceneOpenTimer( bool bInstantOpen )
{
	if ( bInstantOpen )
	{
		NextTimeForSceneOpen = TutorialMgr.WorldInfo.TimeSeconds;
	}
	else
	{
		NextTimeForSceneOpen = TutorialMgr.WorldInfo.TimeSeconds + ElapsedTimeToRepeatScene;
	}
}

/** Opens the UI scene for this tutorial */
final function OpenTutorialUIScene()
{
	local LocalPlayer LP;

	if ( !bSceneIsOpen )
	{
		`DEBUGTUTORIAL("GearTutorial_Base::OpenTutorialUIScene: Type="$TutorialType@"Obj="$self@"SceneRef="$SceneReference);

		// Try and open the UI scene
		LP = LocalPlayer(TutorialMgr.Player);
		if ( LP != None )
		{
			SceneInstance = SceneReference.OpenScene(SceneReference, LP);
		}

		// Successfully opened the scene
		if ( SceneInstance != None )
		{
			`DEBUGTUTORIAL("GearTutorial_Base::OpenTutorialUIScene: Success Type="$TutorialType@"Obj="$self);
			OnTutorialSceneOpened();
			GearUIScene_Tutorial(SceneInstance).OnTutorialOpenAnimationComplete = OnTutorialSceneOpenAnimationComplete;
		}
		else
		{
			`DEBUGTUTORIAL("GearTutorial_Base::OpenTutorialUIScene: Failure Type="$TutorialType@"Obj="$self@"LocalPlayer="$LP);
		}
	}
}

/** Handler for the completion of this scene's opening animation */
function OnTutorialSceneOpenAnimationComplete()
{
	// Pause the game if we need to (won't happen if there are networked players)
	if ( bPauseGameplay )
	{
		TogglePauseGameplay( true );
	}
}

/** Closes the UI scene for this tutorial */
final function CloseTutorialUIScene()
{
	local LocalPlayer LP;

	if ( bSceneIsOpen && SceneInstance != None )
	{
		`DEBUGTUTORIAL("GearTutorial_Base::CloseTutorialUIScene: Type="$TutorialType@"Obj="$self);

		// Try and close the UI scene
		LP = LocalPlayer(TutorialMgr.Player);
		if ( LP != None )
		{
			SceneInstance.CloseScene(SceneInstance, false);
		}
	}
}

/** Initializes the strings on the opened UI scene */
function InitializeUISceneLabels( optional bool bHideButtonText )
{
	local UILabel BodyLabel, TitleLabel, ButtonLabel;
	local UIImage ButtonBG;
	local string LocalizedString;
	local int PlayerIndex;
	local bool bIsSplitscreen;
	local bool bIsPlayingCoop;
	local bool bIsHost;

	`DEBUGTUTORIAL("GearTutorial_Base::InitializeUISceneLabels: Type="$TutorialType@"Obj="$self@"SceneRef="$SceneReference@"SceneInstance="$SceneInstance);

	if ( SceneInstance != None )
	{
		// Find all of the labels we need from the scene
		BodyLabel = UILabel(SceneInstance.FindChild('lblTutorialBody', true));
		TitleLabel = UILabel(SceneInstance.FindChild('lblTitle', true));
		ButtonLabel = UILabel(SceneInstance.FindChild('lblTutorialButton', true));
		ButtonBG = UIImage(SceneInstance.FindChild('imgTutorialButtonBG', true));

		// Gather game state data
		bIsSplitscreen = GearPC(Outer).IsSplitscreenPlayer(PlayerIndex);
		bIsPlayingCoop = GearPC(Outer).IsActuallyPlayingCoop();
		if (bIsPlayingCoop)
		{
			if (bIsSplitscreen)
			{
				bIsHost = (PlayerIndex == 0);
			}
			else
			{
				bIsHost = (GearPC(Outer).WorldInfo.NetMode != NM_Client);
			}
		}

		// Set the text for the body
		if ( (BodyLabel != None) && (LocalizedBodyText != "") )
		{
			// Check for coop host
			if (bIsPlayingCoop &&
				bIsHost &&
				LocalizedCoopBodyTextHost != "")
			{
				LocalizedString = TutorialMgr.Localize("TUTORIALS", LocalizedCoopBodyTextHost, "GearGame" );
			}
			// Check for coop client
			else if (bIsPlayingCoop &&
					 !bIsHost &&
					 LocalizedCoopBodyTextClient != "")
			{
				LocalizedString = TutorialMgr.Localize("TUTORIALS", LocalizedCoopBodyTextClient, "GearGame" );
			}
			// See if we should use the alternate controls version of the body string
			else if ( (LocalizedAlternateBodyText != "") && TutorialMgr.Outer.bUseAlternateControls )
			{
				LocalizedString = TutorialMgr.Localize("TUTORIALS", LocalizedAlternateBodyText, "GearGame" );
			}
			else
			{
				LocalizedString = TutorialMgr.Localize("TUTORIALS", LocalizedBodyText, "GearGame" );
			}

			BodyLabel.SetDataStoreBinding( LocalizedString );
			`DEBUGTUTORIAL("GearTutorial_Base::InitializeUISceneLabels: BodyVariable="$LocalizedBodyText@"BodyString="$LocalizedString);
		}

		// Set the text for the title
		if ( (TitleLabel != None) && (LocalizedTitleText != "") )
		{
			// Check for coop host
			if (bIsPlayingCoop &&
				bIsHost &&
				LocalizedCoopTitleTextHost != "")
			{
				LocalizedString = TutorialMgr.Localize("TUTORIALS", LocalizedCoopTitleTextHost, "GearGame" );
			}
			// Check for coop client
			else if (bIsPlayingCoop &&
					 !bIsHost &&
					 LocalizedCoopBodyTextClient != "")
			{
				LocalizedString = TutorialMgr.Localize("TUTORIALS", LocalizedCoopTitleTextClient, "GearGame" );
			}
			else
			{
				LocalizedString = TutorialMgr.Localize("TUTORIALS", LocalizedTitleText, "GearGame" );
			}

			TitleLabel.SetDataStoreBinding( LocalizedString );
			`DEBUGTUTORIAL("GearTutorial_Base::InitializeUISceneLabels: TitleVariable="$LocalizedTitleText@"TitleString="$LocalizedString);
		}

		// Set the text for the button message
		if (ButtonLabel != None && ButtonBG != None)
		{
			if (LocalizedButtonText != "")
			{
				ButtonLabel.SetVisibility(!bHideButtonText);
				ButtonBG.SetVisibility(!bHideButtonText);

				// See if we should use the alternate controls version of the body string
				if ( (LocalizedAlternateButtonText != "") && TutorialMgr.Outer.bUseAlternateControls )
				{
					LocalizedString = TutorialMgr.Localize("TUTORIALS", LocalizedAlternateButtonText, "GearGame" );
				}
				else
				{
					LocalizedString = TutorialMgr.Localize("TUTORIALS", LocalizedButtonText, "GearGame" );
				}

				ButtonLabel.SetDataStoreBinding( LocalizedString );
			}
			else
			{
				ButtonLabel.SetVisibility(false);
				ButtonBG.SetVisibility(false);
			}
			`DEBUGTUTORIAL("GearTutorial_Base::InitializeUISceneLabels: ButtonVariable="$LocalizedButtonText@"ButtonString="$LocalizedString);
		}
	}
}

/** Will freeze the game and the player, but does not pause the game */
function ToggleFreezeGameplay( bool bFreeze )
{
	if ( bFreeze && !bGameplayIsFrozen )
	{
		`DEBUGTUTORIAL("GearTutorial_Base::ToggleFreezeGameplay: Freeze Type="$TutorialType@"Obj="$self);
		TutorialMgr.DisableInput( true, true, true );
		bGameplayIsFrozen = true;
	}
	else if ( !bFreeze && bGameplayIsFrozen )
	{
		`DEBUGTUTORIAL("GearTutorial_Base::ToggleFreezeGameplay: Unfreeze Type="$TutorialType@"Obj="$self);
		TutorialMgr.EnableInput( true, true, true );
		bGameplayIsFrozen = false;
	}
}

/** Will pause the game */
function TogglePauseGameplay( bool bPause )
{
	local SoundCue CueToPlay;

	if ( bPause && !bGameplayIsPaused && CanPauseGame() )
	{
		`DEBUGTUTORIAL("GearTutorial_Base::TogglePauseGameplay: Pause Type="$TutorialType@"Obj="$self);
		TutorialMgr.Outer.SetPause(true, CanUnpause);
		bGameplayIsPaused = true;

		// Play music so we're not dead silent
		CueToPlay = SoundCue(DynamicLoadObject("Interface_Audio.Menu.MenuPauseLoopCue", class'SoundCue'));
		if (CueToPlay != None)
		{
			// Stop an existing sound
			if (MusicLoopAC != none)
			{
				MusicLoopAC.FadeOut(1.0f, 0.0f);
			}

			// Create and set the new sound
			MusicLoopAC = TutorialMgr.Outer.CreateAudioComponent(CueToPlay, false, true);
			if (MusicLoopAC != none)
			{
				MusicLoopAC.bAllowSpatialization = false;
				MusicLoopAC.bAutoDestroy = true;
				MusicLoopAC.bIsUISound = true;
				MusicLoopAC.Play();
			}
		}
	}
	else if ( !bPause && bGameplayIsPaused )
	{
		`DEBUGTUTORIAL("GearTutorial_Base::TogglePauseGameplay: Unpause Type="$TutorialType@"Obj="$self);
		bGameplayIsPaused = false;
		TutorialMgr.Outer.SetPause(false);

		// Stop the music
		if (MusicLoopAC != none)
		{
			MusicLoopAC.FadeOut(1.0f, 0.0f);
		}
	}
}

/** Whether we can pause the game or not */
function bool CanPauseGame()
{
	local WorldInfo WI;
	local GearPC PC;
	local GearGRI GRI;
	local int MatchType;

	WI = TutorialMgr.Outer.WorldInfo;
	GRI = GearGRI(WI.GRI);

	if (WI.NetMode < NM_Client)
	{
		// Check for MP game
		if (GRI.IsMultiPlayerGame())
		{
			// can't pause game in MP unless we're a local game or in training grounds
			foreach WI.LocalPlayerControllers(class'GearPC', PC)
			{
				// check for local game
				if (PC.ProfileSettings != none &&
					PC.ProfileSettings.GetProfileSettingValueId(class'GearProfileSettings'.const.VERSUS_MATCH_MODE, MatchType) &&
					MatchType == eGVMT_Local)
				{
					return true;
				}
				// check for training grounds
				else if (GRI.TrainingGroundsID >= 0)
				{
					return true;
				}
				break;
			}
			// is neither a local game nor training grounds
			return false;
		}
		// Is an SP game
		else
		{
			// Pause the game if there are no networked players
			foreach WI.AllControllers(class'GearPC', PC)
			{
				if (!PC.IsLocalPlayerController())
				{
					// Found a networked player, bail
					return false;
				}
			}
			// I am the server of a co-op game and never found a networked player
			return true;
		}
	}
	// I'm a networked client
	return false;
}

/** Callback the server uses to determine if the unpause can happen */
function bool CanUnpause()
{
	return (bTutorialIsComplete || !CanPauseGame());
}

/** Do any possible button remappings needed for controller scheme change */
function EGameButtons RemapGameButton( EGameButtons GameButton )
{
	if (  TutorialMgr.Outer.bUseAlternateControls )
	{
		switch ( GameButton )
		{
			case GB_X:
				return GB_Y;
			case GB_Y:
				return GB_RightStick_Push;
		}
	}

	return GameButton;
}

defaultproperties
{
	SceneReference=UIScene'UI_Scenes_Common.UI_HUD_Tutorial'
	TutorialPriority=TUTORIAL_PRIORITY_NORMAL
	bTutorialHasUIScene=true
	bSaveTutorialToProfile=true
}
