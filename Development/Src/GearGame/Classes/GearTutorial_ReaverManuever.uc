/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_ReaverManuever extends GearTutorial_Base
	config(Game);


/** Whether this tutorial was automatically closed because the player turned tutorials off */
var bool bAutoCompleted;

/** Called every tick so that the tutorial has a chance to update itself */
event Update()
{

	// If the tutorial option in the game settings is off, don't do the tutorial
	if ( !bAutoCompleted && !TutorialOptionIsOn() )
	{
		bAutoCompleted = true;
		OnTutorialCompleted();
	}

	Super.Update();
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

defaultproperties
{
	TutorialType=GEARTUT_Reaver2
}
