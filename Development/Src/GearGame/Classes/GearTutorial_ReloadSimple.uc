/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_ReloadSimple extends GearTutorial_UnlimitedAmmoBase
	config(Game);


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called when the tutorial gets added to the tutorial manager's list */
function OnTutorialAdded( GearTutorialManager TutMgr, SeqAct_ManageTutorials Action, bool IsKismetDriven  )
{
	Super.OnTutorialAdded( TutMgr, Action, IsKismetDriven );

	GearPC(Outer).AddGearEventDelegate( eGED_Reload, OnReload );

	// Freeze game
	ToggleFreezeGameplay( true );
}

/** Called when the tutorial gets removed from the tutorial manager's list */
function OnTutorialRemoved()
{
	Super.OnTutorialRemoved();

	GearPC(Outer).ClearGearEventDelegate( eGED_Reload, OnReload );

	TutorialMgr.RemoveTutorial(GEARTUT_UnlimitedAmmo);
	TutorialMgr.MarkTutorialComplete(GEARTUT_UnlimitedAmmo, false);
}

/** Delegate fired when the player reloads */
function OnReload()
{
	// Mark the other reload tutorials as complete if this one was completed
	// Don't let the first one save the profile so we don't have back to back saving
	TutorialMgr.MarkTutorialComplete( GEARTUT_Reload, false );
	TutorialMgr.MarkTutorialComplete( GEARTUT_ActiveReload );

	// Unfreeze game
	ToggleFreezeGameplay( false );

	// Tutorial is done if they reloaded
	OnTutorialCompleted();
}

/** Will freeze the game and the player, but does not pause the game */
function ToggleFreezeGameplay( bool bFreeze )
{
	local GearPC MyGearPC;

	Super.ToggleFreezeGameplay( bFreeze );

	if ( bFreeze )
	{
		// Let's make sure we have ammo and don't have to reload after this
		MyGearPC = GearPC(Outer);
		if ( MyGearPC != None )
		{
			MyGearPC.StopFiringWeaponAfterTime(0.5f);
		}
	}
}

/** Action to perform on the player when this tutorial is completed - implemented by all subclasses */
function PerformPCInputAction( bool bPressed )
{
	GearPlayerInput(InputHandler).HandleButtonInput_RightBumper(bPressed);
}

defaultproperties
{
	TutorialType=GEARTUT_ReloadSimple
	GameButtonType=GB_RightBumper
}
