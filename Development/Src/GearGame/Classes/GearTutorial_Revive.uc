/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_Revive extends GearTutorial_Base
	config(Game);


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called when the tutorial gets added to the tutorial manager's list */
function OnTutorialAdded( GearTutorialManager TutMgr, SeqAct_ManageTutorials Action, bool IsKismetDriven )
{
	Super.OnTutorialAdded( TutMgr, Action, IsKismetDriven );

	GearPC(Outer).AddGearEventDelegate( eGED_Revive, OnRevive );
}

/** Called when the tutorial gets removed from the tutorial manager's list */
function OnTutorialRemoved()
{
	Super.OnTutorialRemoved();

	GearPC(Outer).ClearGearEventDelegate( eGED_Revive, OnRevive );
}

/** Delegate fired when the player swat turns */
function OnRevive()
{
	// Make sure this isn't fired when the tutorial has already completed
	if ( !bTutorialIsComplete )
	{
		OnTutorialCompleted();
	}
}

defaultproperties
{
	TutorialType=GEARTUT_Revive
}
