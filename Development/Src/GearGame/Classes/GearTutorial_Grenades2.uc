/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearTutorial_Grenades2 extends GearTutorial_Base
	config(Game);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Whether the player has started targetting their grenade yet */
var transient bool bStartedTargettingGrenade;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/**
 * Called when this tutorial's input handler is created (if this tutorial has it's own handler)
 *		This is where a tutorial can add InputButtonData mappings to the handler.
 */
function InitializeInputHandler()
{
	// Must call super first so that there is button data to set
	Super.InitializeInputHandler();

	if ( InputHandler != None )
	{
		InputHandler.SetInputButtonHandle( GB_LeftTrigger, HandleButtonInput_Targetting );
	}
}

/** Called when the tutorial gets added to the tutorial manager's list */
function OnTutorialAdded( GearTutorialManager TutMgr, SeqAct_ManageTutorials Action, bool IsKismetDriven  )
{
	Super.OnTutorialAdded( TutMgr, Action, IsKismetDriven );

	GearPC(Outer).AddGearEventDelegate( eGED_GrenadeToss, OnGrenadeToss );
}

/** Called when the tutorial gets removed from the tutorial manager's list */
function OnTutorialRemoved()
{
	Super.OnTutorialRemoved();

	GearPC(Outer).ClearGearEventDelegate( eGED_GrenadeToss, OnGrenadeToss );
}

/**
 * Opportunity for the tutorial to override the filtering system for the input object handling this tutorial's input
 *		@Param Filtered - whether this button should be filtered or not (non zero means true)
 *		@Return - whether we've handled the filtering or not
 */
function bool FilterButtonInput( Name ButtonName, bool bPressed, int ButtonIdx, out int Filtered )
{
	// If we've started targetting let the parent handle the input
	if ( bStartedTargettingGrenade )
	{
		return false;
	}

	// Have the tutorial's input handler handle this input
	if ( ButtonName == InputHandler.InputButtonDataList[GB_LeftTrigger].ButtonNameMapping )
	{
		Filtered = 0;
	}
	else
	{
		Filtered = 1;
	}

	return true;
}

/** Callback when the player targets using the grenades */
function HandleButtonInput_Targetting( bool bPressed, optional bool bDblClickMove )
{
	local UILabel ButtonLabel;
	local UIImage ButtonBG;

	if ( bPressed )
	{
		bStartedTargettingGrenade = true;

		// Unfreeze the game
		ToggleFreezeGameplay( false );

		// Reset the left trigger handle and call it
		InputHandler.SetInputButtonHandle( GB_LeftTrigger, GearPlayerInput(InputHandler).HandleButtonInput_LeftTrigger );
		GearPlayerInput(InputHandler).HandleButtonInput_LeftTrigger( bPressed );
		InputHandler.ForceButtonRelease( GB_DPad_Up );

		// Change the UI scene to not show the 'press LT' part
		if ( bSceneIsOpen )
		{
			ButtonLabel = UILabel(SceneInstance.FindChild('lblTutorialButton', true));
			ButtonBG = UIImage(SceneInstance.FindChild('imgTutorialButtonBG', true));

			if ( ButtonLabel != None )
			{
				ButtonLabel.SetVisibility( false );
			}

			if ( ButtonBG != None )
			{
				ButtonBG.SetVisibility( false );
			}
		}
	}
}

/** Callback for when the player tosses the grenade */
function OnGrenadeToss()
{
	OnTutorialCompleted();
}

defaultproperties
{
	TutorialType=GEARTUT_Grenades2
	InputHandlerClass=class'GearPlayerInputTutorialPC'
}
