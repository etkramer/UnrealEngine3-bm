/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearTutorial_Use2 extends GearTutorial_Base
	config(Game);


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called when the tutorial gets added to the tutorial manager's list */
function OnTutorialAdded( GearTutorialManager TutMgr, SeqAct_ManageTutorials Action, bool IsKismetDriven )
{
	Super.OnTutorialAdded( TutMgr, Action, IsKismetDriven );

	GearPC(Outer).AddGearEventDelegate( eGED_Pickup, OnPickup );
}

/** Called when the tutorial gets removed from the tutorial manager's list */
function OnTutorialRemoved()
{
	Super.OnTutorialRemoved();

	GearPC(Outer).ClearGearEventDelegate( eGED_Pickup, OnPickup );
}

/** Delegate fired when the player picks up an object */
function OnPickup()
{
	// Make sure this isn't fired when the tutorial has already completed
	if ( !bTutorialIsComplete )
	{
		OnTutorialCompleted();
	}
}

defaultproperties
{
	TutorialType=GEARTUT_Use2
}
