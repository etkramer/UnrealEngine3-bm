/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_ObjectivesReminder extends GearTutorial_PCAction_Base
	config(Game);


/** Whether this tutorial was automatically closed because the player already had the weapon equipped */
var bool bAutoCompleted;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Action to perform on the player when this tutorial is completed - implemented by all subclasses */
function PerformPCInputAction( bool bPressed )
{
	if (!bAutoCompleted)
	{
		GearPlayerInput(InputHandler).HandleButtonInput_LeftBumper(bPressed);
	}
}

/** Determine if we should open the UI scene or not */
function bool ShouldOpenUIScene()
{
	// Don't open the scene if they have tutorials turned off
	if (!TutorialOptionIsOn())
	{
		return false;
	}

	return Super.ShouldOpenUIScene();
}

/** Whether the tutorial system game option is on or not */
function bool TutorialOptionIsOn()
{
	local GearPC PC;

	PC = GearPC(Outer);
	if (PC != none &&
		PC.ProfileSettings != none &&
		PC.ProfileSettings.GetTutorialConfigOption())
	{
		return true;
	}
	return false;
}

/** Called every tick so that the tutorial has a chance to update itself */
event Update()
{
	// See if we should auto complete this tutorial because the player already has the weapon equipped
	if (!bAutoCompleted && !TutorialOptionIsOn())
	{
		bAutoCompleted = true;
		OnTutorialCompleted();
	}

	Super.Update();
}

defaultproperties
{
	TutorialType=GEARTUT_ObjectivesReminder
	GameButtonType=GB_LeftBumper
}
