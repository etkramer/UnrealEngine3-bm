/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_Fire extends GearTutorial_UnlimitedAmmoBase
	config(Game);

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called when the tutorial gets added to the tutorial manager's list */
function OnTutorialAdded( GearTutorialManager TutMgr, SeqAct_ManageTutorials Action, bool IsKismetDriven  )
{
	Super.OnTutorialAdded( TutMgr, Action, IsKismetDriven );

	// Add and start the UnlimitedAmmo tutorial
	TutorialMgr.AddTutorial( GEARTUT_UnlimitedAmmo );
	TutorialMgr.StartTutorial( GEARTUT_UnlimitedAmmo );
}

/** Action to perform on the player when this tutorial is completed - implemented by all subclasses */
function PerformPCInputAction( bool bPressed )
{
	GearPlayerInput(InputHandler).HandleButtonInput_RightTrigger(bPressed);
}

/**
 * Will freeze the game and the player, but does not pause the game
 *  - overridden to allow rotation
 */
function ToggleFreezeGameplay( bool bFreeze )
{
	if ( bFreeze && !bGameplayIsFrozen )
	{
		`DEBUGTUTORIAL("GearTutorial_Base::ToggleFreezeGameplay: Freeze Type="$TutorialType@"Obj="$self);
		TutorialMgr.DisableInput( true, true, false );
		bGameplayIsFrozen = true;
	}
	else if ( !bFreeze && bGameplayIsFrozen )
	{
		`DEBUGTUTORIAL("GearTutorial_Base::ToggleFreezeGameplay: Unfreeze Type="$TutorialType@"Obj="$self);
		TutorialMgr.EnableInput( true, true, false );
		bGameplayIsFrozen = false;
	}
}

defaultproperties
{
	TutorialType=GEARTUT_Fire
	GameButtonType=GB_RightTrigger
}
