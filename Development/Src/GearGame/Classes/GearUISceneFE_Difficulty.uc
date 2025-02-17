/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFE_Difficulty extends GearUISceneFrontEnd_Base
	ClassRedirect(GearUISceneFESO_Difficulty)
	Config(UI);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** References to the difficulty buttons */
var transient array<UILabelButton> DiffButtons;

/** Reference to the difficulty image */
var transient UIImage DiffImage;

/** String names of the paths to the difficulty images */
var array<string> DifficultyImagePaths;

/** Reference to the slot button bar */
var transient UICalloutButtonPanel ButtonBar;

/** Label for the name of the player who is setting their difficulty */
var transient UILabel PlayerNameLabel;

/** Controller icon for the player who is setting their difficulty */
var transient UILabel ControllerIconLabel;

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
	local int Idx;
	local string ButtonName;
	local GearPC PC;
	local string ControllerString;
	local LocalPlayer LP;
	local OnlineSubsystem OnlineSub;
	local string PlayerName;
	local UILabelButton LastEnabledButton;

	if ( !IsEditor() )
	{
		DiffButtons.length = DL_MAX;
		for (Idx = 0; Idx < DiffButtons.length; Idx++ )
		{
			ButtonName = "btnDifficulty" $ Idx;
			DiffButtons[Idx] = UILabelButton(FindChild(Name(ButtonName), true));
			DiffButtons[Idx].OnClicked = DifficultyButtonClicked;
			LastEnabledButton = DiffButtons[Idx];
		}

		// Disable the insane difficulty if they did not unlock it
		PC = GetGearPlayerOwner(GetBestPlayerIndex());
		if (PC != None &&
			PC.ProfileSettings != None &&
			!PC.ProfileSettings.HasUnlockableBeenUnlocked(eUNLOCK_InsaneDifficulty))
		{
			LastEnabledButton = DiffButtons[DL_Insane-1];
			DiffButtons[DL_Insane].SetEnabled(false);
		}

		// Set navigation
		DiffButtons[0].SetForcedNavigationTarget(UIFACE_Top, LastEnabledButton);
		LastEnabledButton.SetForcedNavigationTarget(UIFACE_Bottom, DiffButtons[0]);

		DiffImage = UIImage(FindChild('imgDescriptionImage', true));

		// Initialize the button bar
		ButtonBar = UICalloutButtonPanel(FindChild('pnlButtonBar', true));
		if ( ButtonBar != None )
		{
			ButtonBar.SetButtonCallback( 'GenericBack', OnCancelClicked );
		}

		// Make the labels invisible until we know whether we can resolve their data or not
		PlayerNameLabel = UILabel(FindChild('lblName', true));
		ControllerIconLabel = UILabel(FindChild('lblProfile', true));
		PlayerNameLabel.SetVisibility(false);
		ControllerIconLabel.SetVisibility(false);

		if (PC != None)
		{
			LP = LocalPlayer(PC.Player);
			if (LP != None)
			{
				// Get the widget references and set the values for the player's name
				OnlineSub = class'GameEngine'.static.GetOnlineSubSystem();
				if (OnlineSub != None && OnlineSub.PlayerInterface != None)
				{
					PlayerName = OnlineSub.PlayerInterface.GetPlayerNickname(LP.ControllerId);
				}
				else if ( LP != None )
				{
					PlayerName = LP.Actor.PlayerReplicationInfo.PlayerName;
				}
				PlayerNameLabel.SetVisibility(true);
				PlayerNameLabel.SetDataStoreBinding(PlayerName);

				// Set the player's profile icon
				ControllerString = GetControllerIconString(LP.ControllerId);
				ControllerIconLabel.SetVisibility(true);
				ControllerIconLabel.SetDataStoreBinding(ControllerString);
			}
		}
	}
}

/** Sets the focus on a particular difficulty selection */
function SetDifficultyFocus( EDifficultyLevel DiffType )
{
	if ( DiffButtons.length > DiffType )
	{
		DiffButtons[DiffType].SetFocus(None);
	}
}

/**
 * Handler for the OnClick delegate for all of the difficulty buttons in this scene.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool DifficultyButtonClicked(UIScreenObject EventObject, int PlayerIndex)
{
	local GearPC MyGearPC;
	local int Idx;

	MyGearPC = GetGearPlayerOwner( PlayerIndex );
	if ( MyGearPC != None && MyGearPC.ProfileSettings != None )
	{
		// Find the matching widget so we know which value to set the profile to
		for (Idx = 0; Idx < DiffButtons.length; Idx++)
		{
			if ( EventObject == DiffButtons[Idx] )
			{
				break;
			}
		}

		if ( Idx < DiffButtons.length )
		{
			if ( !MyGearPC.ProfileSettings.SetProfileSettingValueId(MyGearPC.ProfileSettings.GameIntensity, Idx) )
			{
				`log("Could not save difficulty"@EDifficultyLevel(Idx)@"to profile at ID"@MyGearPC.ProfileSettings.GameIntensity);
			}
		}
		else
		{
			`log(`location@`showvar(Idx)@"Could not find difficulty value because"@EventObject@"is not in the button list!");
		}
	}

	SetTransitionValue("SelectedDifficulty", string(Idx));
	ClearTransitionValue("ExitDifficulty");
	TransitionFromDifficultyScene(PlayerIndex);

	return true;
}

/** Cancel clicked */
function bool OnCancelClicked(UIScreenObject EventObject, int PlayerIndex)
{
	SetTransitionValue("ExitDifficulty", "Cancel");
	TransitionFromDifficultyScene(PlayerIndex);
	return true;
}

/** Close the scene or launches the game depending on where we came from to get here */
function TransitionFromDifficultyScene(int PlayerIndex)
{
	local string EnterString;
	local string ExitString;

	EnterString = GetTransitionValue("EnterDifficulty");
	ExitString = GetTransitionValue("ExitDifficulty");

	// Canceling
	if (ExitString == "Cancel")
	{
		CloseScene();
	}
	// A selection was made - see where we came from to determine where to go
	else
	{
		// Back to the lobby
		if (EnterString == "Lobby")
		{
			// Tells the lobby which player is setting their difficulty
			SetTransitionValue("DifficultyPlayer", string(PlayerIndex));
			CloseScene();
		}
		// Launch the game
		else
		{
			LaunchSoloCampaign(PlayerIndex, false);
		}
	}
}

/**
 * Called when a new UIState becomes the widget's currently active state, after all activation logic has occurred.
 *
 * @param	Sender					the widget that changed states.
 * @param	PlayerIndex				the index [into the GamePlayers array] for the player that activated this state.
 * @param	NewlyActiveState		the state that is now active
 * @param	PreviouslyActiveState	the state that used the be the widget's currently active state.
 */
function OnStateChanged( UIScreenObject Sender, int PlayerIndex, UIState NewlyActiveState, UIState PreviouslyActiveState )
{
	local int Idx;

	if ( DiffImage != None )
	{
		for ( Idx = 0; Idx < DiffButtons.length; Idx++ )
		{
			if ( DiffButtons[Idx] != None && DiffButtons[Idx] == Sender && NewlyActiveState.IsA('UIState_Focused') )
			{
				DiffImage.SetDataStoreBinding( DifficultyImagePaths[Idx] );
				break;
			}
		}
	}

	Super.OnStateChanged( Sender, PlayerIndex, NewlyActiveState, PreviouslyActiveState );
}

/** Initialize the scene with the correct difficulty focus */
function SceneActivationComplete( UIScene ActivatedScene, bool bInitialActivation )
{
	local string SelectedDiffString;
	local int DiffValue;

	if (bInitialActivation)
	{
		SelectedDiffString = GetTransitionValue("SelectedDifficulty");
		DiffValue = int(SelectedDiffString);

		SetDifficultyFocus(EDifficultyLevel(DiffValue));
	}
}

defaultproperties
{
	OnSceneActivated=SceneActivationComplete

	DifficultyImagePaths(DL_Casual)="<Images:UI_Portraits.Difficulty.DL_Casual>"
	DifficultyImagePaths(DL_Normal)="<Images:UI_Portraits.Difficulty.DL_Nrmal>"
	DifficultyImagePaths(DL_Hardcore)="<Images:UI_Portraits.Difficulty.DL_Hardcore>"
	DifficultyImagePaths(DL_Insane)="<Images:UI_Portraits.Difficulty.DL_Insane>"

	Begin Object Name=SceneEventComponent
		DisabledEventAliases.Add(CloseScene)
	End Object

	bAllowPlayerJoin=false
	bAllowSigninChanges=false
}
