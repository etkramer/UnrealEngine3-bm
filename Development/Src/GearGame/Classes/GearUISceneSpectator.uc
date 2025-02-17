/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearUISceneSpectator extends GearUISceneMP_Base
	Config(UI);

/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Label for displaying what the player is spectating in detail view */
var transient UILabel TitleLabel;

/** Widgets to disable when there is no players to view */
var transient UILabel LeftBumperLabel;
var transient UILabel RightBumperLabel;
var transient UIImage PlayerCamImage;

/** Widgets to disable if we are in Ghost Cam */
var transient UILabel GhostCamLabel;
var transient UILabel GhostCamXLabel;

/** Widget to set the text on for toggling names */
var transient UILabel ToggleNamesLabel;

/** Widgets to disable if we are not in player camera */
var transient UILabel PlayercardLabel;
var transient UILabel PlayercardYLabel;

var transient UILabel ShotLabel;
var transient UILabel ShotInputLabel;

/** Localized strings for displaying the camera modes */
var localized string ToggleCamera;
var localized string Normalcam;
var localized string PlayerNameToggleShow;
var localized string PlayerNameToggleHide;

var transient bool bCanTakeScreenshot;


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
}

/** Initialize the widget references */
final function InitializeWidgetReferences()
{
	TitleLabel = UILabel(FindChild('lblTitle', true));
	LeftBumperLabel = UILabel(FindChild('lblLBumper', true));
	RightBumperLabel = UILabel(FindChild('lblRBumper', true));
	PlayerCamImage = UIImage(FindChild('imgCamera', true));
	GhostCamLabel = UILabel(FindChild('lblMinimize', true));
	GhostCamXLabel = UILabel(FindChild('UIX', true));
	ToggleNamesLabel = UILabel(FindChild('lblNames', true));
	PlayercardLabel = UILabel(FindChild('lblGamercard', true));
	PlayercardYLabel = UILabel(FindChild('lblY', true));

	ShotLabel = UILabel(FindChild('lblScreenshot',true));
	ShotInputLabel = UILabel(FindChild('lblBack',true));
}

/** Sets the state of a label if that state isn't already active */
final function SetLabelStateByClass( UIObject LabelToSet, bool bEnable, int PlayerIndex )
{
	local class<UIState> StateToSetTo;

	StateToSetTo = bEnable ? class'UIState_Enabled' : class'UIState_Disabled';

	if ( LabelToSet.GetCurrentState().class != StateToSetTo )
	{
		LabelToSet.ActivateStateByClass( StateToSetTo, PlayerIndex );
	}
}

/** Update the widget styles, strings, etc. */
function UpdateWidgetData(float DeltaTime)
{
	local GearPC MyGearPC;
	local String TitleString;
	local LocalPlayer LP;
	local int BestPlayerIndex;
	local bool bIsInGhostState;
	local bool bIsHumanPlayer;
	local bool bIsLoggedIn;

	LP = GetPlayerOwner();
	BestPlayerIndex = GetBestControllerId();
	if ( LP != None )
	{
		MyGearPC = GearPC(LP.Actor);
		if ( MyGearPC != None )
		{
			// Set string to Ghost Camera
			bIsInGhostState = (MyGearPC.GetStateName() == 'GhostSpectating');
			if ( bIsInGhostState )
			{
				TitleString = ToggleCamera;
			}

			// Enable/Disable the ghost camera labels
			SetLabelStateByClass( GhostCamLabel, !bIsInGhostState, BestPlayerIndex );
			SetLabelStateByClass( GhostCamXLabel, !bIsInGhostState, BestPlayerIndex );

			// Set string to Player Camera
			if ( MyGearPC.GetStateName() == 'PlayerSpectating' )
			{
				TitleString = MyGearPC.CurrSpectatingString;
				if (MyGearPC.SpectatingPRI != None && !MyGearPC.SpectatingPRI.bBot)
				{
					bIsHumanPlayer = true;
				}
				SetLabelStateByClass( PlayercardLabel, bIsHumanPlayer, BestPlayerIndex );
				SetLabelStateByClass( PlayercardYLabel, bIsHumanPlayer, BestPlayerIndex );
			}
			else
			{
				SetLabelStateByClass( PlayercardLabel, FALSE, BestPlayerIndex );
				SetLabelStateByClass( PlayercardYLabel, FALSE, BestPlayerIndex );
			}

			// Set string to Battle Camera
			if ( MyGearPC.GetStateName() == 'CustomSpectating' )
			{
				TitleString = Normalcam;
			}

			// Enable the player camera labels
			if ( MyGearPC.IsSpectating() &&
				 MyGearPC.CanSpectateTeammate() &&
				 !MyGearPC.IsInState('PlayerWaiting') )
			{
				SetLabelStateByClass( PlayerCamImage, TRUE, BestPlayerIndex );
				SetLabelStateByClass( LeftBumperLabel, TRUE, BestPlayerIndex );
				SetLabelStateByClass( RightBumperLabel, TRUE, BestPlayerIndex );
			}
			// Disable the player camera labels
			else
			{
				SetLabelStateByClass( PlayerCamImage, FALSE, BestPlayerIndex );
				SetLabelStateByClass( LeftBumperLabel, FALSE, BestPlayerIndex );
				SetLabelStateByClass( RightBumperLabel, FALSE, BestPlayerIndex );
			}

			// Set the correct ToggleNames string
			if ( MyGearPC.MyGearHud != None )
			{
				ToggleNamesLabel.SetValue( MyGearPC.MyGearHud.bShowNamesWhenSpectating ? PlayerNameToggleHide : PlayerNameToggleShow );
			}

			bIsLoggedIn = class'UIInteraction'.static.IsLoggedIn(BestPlayerIndex);
			if ( !bCanTakeScreenshot || !bIsLoggedIn || !MyGearPC.AllowedToTakeScreenshots() )
			{
				SetLabelStateByClass( ShotLabel, FALSE, BestPlayerIndex );
				SetLabelStateByClass( ShotInputLabel, FALSE, BestPlayerIndex );
			}
			else
			{
				SetLabelStateByClass( ShotLabel, TRUE, BestPlayerIndex );
				SetLabelStateByClass( ShotInputLabel, TRUE, BestPlayerIndex );
			}

			// Set the title
			TitleLabel.SetValue( TitleString );
		}
	}
}

/** Opens the gamercard for the given pri */
function OpenGamerCardBlade(GearPRI PRI, int PlayerIndex)
{
	local UniqueNetID ZeroId;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local int ControllerId;
	local OnlineSubsystem OnlineSub;

	if (PRI.UniqueId != ZeroId)
	{
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None)
		{
			PlayerIntEx = OnlineSub.PlayerInterfaceEx;
			if (PlayerIntEx != None)
			{
				ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
				PlayerIntEx.ShowGamerCardUI(ControllerId, PRI.UniqueId);
			}
		}
	}
}

/**
 * Callback function when the scene gets input
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool ProcessInput( out InputEventParameters EventParms )
{
	local GearPC MyGearPC;
	local LocalPlayer LP;

	LP = GetPlayerOwner(EventParms.PlayerIndex);
	if ( LP != None )
	{
		MyGearPC = GearPC(LP.Actor);
	}

	// See if we need to take a screenshot
	if ( EventParms.InputKeyName == 'XboxTypeS_B' &&
	     EventParms.EventType == IE_Released )
	{
		if (!bCanTakeScreenshot)
		{
			MyGearPC.ShowSavingScreenshotMessage(TRUE,TRUE);
		}
		else
		{
			MyGearPC.TakeScreenshot();
		}
		return TRUE;
	}
	// Check for looking at the player card of the player being viewed
	if ( EventParms.InputKeyName == 'XboxTypeS_Y' &&
			  EventParms.EventType == IE_Released &&
			  MyGearPC != None &&
			  MyGearPC.IsInState('PlayerSpectating') )
	{
		if (MyGearPC.SpectatingPRI != None && !MyGearPC.SpectatingPRI.bBot)
		{
			OpenGamerCardBlade(MyGearPC.SpectatingPRI, EventParms.PlayerIndex);
		}
		// Call the playercard blade for GearPC(PlayerOwner.Actor).ViewTarget
		return TRUE;
	}
	// Toggle names
	if ( EventParms.InputKeyName == 'XboxTypeS_A' )
	{
		if ( EventParms.EventType == IE_Pressed &&
			MyGearPC != None &&
			MyGearPC.IsSpectating() &&
			MyGearPC.MyGearHud != None )
		{
			// Toggle the names
			MyGearPC.MyGearHud.bShowNamesWhenSpectating = !MyGearPC.MyGearHud.bShowNamesWhenSpectating;
		}
		// Always eat the B input
		return TRUE;
	}
	// Ghost Camera
	if ( EventParms.InputKeyName == 'XboxTypeS_X' )
	{
		if ( EventParms.EventType == IE_Pressed &&
			 MyGearPC != None &&
			 MyGearPC.IsSpectating() )
		{
			// See if we need to transition to the Ghost Camera
			 if ( !MyGearPC.IsInState('PlayerWaiting') &&
				  MyGearPC.GetStateName() != 'GhostSpectating' )
			 {
				 MyGearPC.TransitionToSpectate('GhostSpectating');
			 }
		}
		// Always eat the X input
		return TRUE;
	}
	// Battle Camera
	if ( EventParms.InputKeyName == 'XboxTypeS_LeftTrigger' || EventParms.InputKeyName == 'XboxTypeS_RightTrigger' )
	{
		if ( EventParms.EventType == IE_Pressed &&
			MyGearPC != None &&
			MyGearPC.IsSpectating() )
		{
			// See if we need to transition to the Battle Camera
			if ( !MyGearPC.IsInState('PlayerWaiting') &&
				 MyGearPC.GetStateName() != 'CustomSpectating' )
			{
				MyGearPC.TransitionToSpectate('CustomSpectating');
			}
			// See if we need to move to the next target
			else if ( EventParms.InputKeyName == 'XboxTypeS_RightTrigger' )
			{
				MyGearPC.PickNextViewTarget();
			}
			// Move to the previous target
			else
			{
				MyGearPC.PickPrevViewTarget();
			}
		}
		// Always eat the Trigger inputs
		return TRUE;
	}
	// Player Camera
	if ( EventParms.InputKeyName == 'XboxTypeS_LeftShoulder' || EventParms.InputKeyName == 'XboxTypeS_RightShoulder' )
	{
		if ( EventParms.EventType == IE_Pressed &&
			MyGearPC != None &&
			MyGearPC.IsSpectating() &&
			MyGearPC.CanSpectateTeammate() )
		{
			// See if we need to transition to the Player Camera
			if ( !MyGearPC.IsInState('PlayerWaiting') &&
				 MyGearPC.GetStateName() != 'PlayerSpectating' )
			{
				MyGearPC.TransitionToSpectate('PlayerSpectating');
			}
			// See if we need to move to the next target
			else if ( EventParms.InputKeyName == 'XboxTypeS_RightShoulder' )
			{
				MyGearPC.PickNextViewTarget();
			}
			// Move to the previous target
			else
			{
				MyGearPC.PickPrevViewTarget();
			}
		}
		// Always eat the Shoulder inputs
		return TRUE;
	}

	// Not handled, so pass the input to the game
	PassInputToGame( EventParms );
	return FALSE;
}

/** Called when the scene is activated */
function OnSceneActivatedCallback( UIScene ActivatedScene, bool bInitialActivation )
{
	local GearEngine Engine;
	local GearPC CurrGearPC;

	if(bInitialActivation)
	{
		CurrGearPC = GetGearPlayerOwner(GetBestPlayerIndex());
		if ( CurrGearPC != None )
		{
			Engine = GearEngine(CurrGearPC.Player.Outer);
			if ( Engine != None )
			{
				bCanTakeScreenshot = Engine.IsCurrentDeviceValid(500 * 1024);
			}
		}
	}

	Super.OnSceneActivatedCallback(ActivatedScene, bInitialActivation);
}

defaultproperties
{
	OnGearUISceneTick=UpdateWidgetData
	OnRawInputKey=ProcessInput
	OnSceneActivated=OnSceneActivatedCallback
	SceneRenderMode=SPLITRENDER_PlayerOwner
	SceneInputMode=INPUTMODE_MatchingOnly
	bCaptureMatchedInput=false
}
