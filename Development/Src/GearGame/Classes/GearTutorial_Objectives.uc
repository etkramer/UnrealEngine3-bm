/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_Objectives extends GearTutorial_PCAction_Base
	config(Game);


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Action to perform on the player when this tutorial is completed - implemented by all subclasses */
function PerformPCInputAction( bool bPressed )
{
	GearPlayerInput(InputHandler).HandleButtonInput_LeftBumper(bPressed);
}

defaultproperties
{
	TutorialType=GEARTUT_Objectives
	GameButtonType=GB_LeftBumper
}
