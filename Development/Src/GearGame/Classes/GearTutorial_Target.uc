/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_Target extends GearTutorial_UnlimitedAmmoBase
	config(Game);


/** Action to perform on the player when this tutorial is completed - implemented by all subclasses */
function PerformPCInputAction( bool bPressed )
{
	GearPlayerInput(InputHandler).HandleButtonInput_LeftTrigger(bPressed);
}

defaultproperties
{
	TutorialType=GEARTUT_Target
	GameButtonType=GB_LeftTrigger
}
