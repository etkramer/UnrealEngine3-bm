/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_ChangeWeapon extends GearTutorial_UnlimitedAmmoBase
	config(Game);


/** Whether this tutorial was automatically closed because the player already had the weapon equipped */
var bool bAutoCompleted;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Action to perform on the player when this tutorial is completed - implemented by all subclasses */
function PerformPCInputAction( bool bPressed )
{
	local EGameButtons DPadButton;

	if ( !bAutoCompleted )
	{
		// figure out which button is currently active
		//@note - multiple may be active and might produce inaccurate results
		if (TutorialMgr.IsButtonActive(GB_DPad_Up))
		{
			DPadButton = GB_DPad_Up;
		}
		else
		if (TutorialMgr.IsButtonActive(GB_DPad_Left))
		{
			DPadButton = GB_DPad_Left;
		}
		else
		if (TutorialMgr.IsButtonActive(GB_DPad_Down))
		{
			DPadButton = GB_DPad_Down;
		}
		else
		if (TutorialMgr.IsButtonActive(GB_DPad_Right))
		{
			DPadButton = GB_DPad_Right;
		}
		GearPlayerInput(InputHandler).HandleButtonInput_DPad(DPadButton,bPressed);
	}
}

/** Whether the player is already carrying the weapon or not */
function bool HasPistolEquipped()
{
	local GearPC MyGearPC;
	local GearPawn MyGearPawn;
	local GearWeapon CurrWeapon;

	MyGearPC = GearPC(Outer);
	if ( MyGearPC != None )
	{
		MyGearPawn = GearPawn(MyGearPC.Pawn);
		if ( MyGearPawn != None )
		{
			CurrWeapon = MyGearPawn.MyGearWeapon;
			if ( (CurrWeapon != None) && ClassIsChildOf(CurrWeapon.Class, class'GearWeap_PistolBase') )
			{
				return true;
			}
		}
	}

	return false;
}

/** Determine if we should open the UI scene or not */
function bool ShouldOpenUIScene()
{
	// Don't open the scene if they already have the weapon equipped (this tutorial will be getting auto-completed)
	if ( HasPistolEquipped() )
	{
		return false;
	}

	return Super.ShouldOpenUIScene();
}

/** Called every tick so that the tutorial has a chance to update itself */
event Update()
{
	// See if we should auto complete this tutorial because the player already has the weapon equipped
	if ( !bAutoCompleted && HasPistolEquipped() )
	{
		bAutoCompleted = true;
		OnTutorialCompleted();
	}

	Super.Update();
}

defaultproperties
{
	TutorialType=GEARTUT_ChangeWeapon
	GameButtonType=GB_DPad_Down
}
