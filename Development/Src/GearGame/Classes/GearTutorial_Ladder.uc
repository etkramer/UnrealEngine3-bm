/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_Ladder extends GearTutorial_Base
	config(Game);


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called when the tutorial gets added to the tutorial manager's list */
function OnTutorialAdded( GearTutorialManager TutMgr, SeqAct_ManageTutorials Action, bool IsKismetDriven  )
{
	Super.OnTutorialAdded( TutMgr, Action, bIsKismetTutorial );

	GearPC(Outer).AddGearEventDelegate( eGED_ClimbDownLadder, OnClimbDownLadder );
}

/** Called when the tutorial gets removed from the tutorial manager's list */
function OnTutorialRemoved()
{
	Super.OnTutorialRemoved();

	GearPC(Outer).ClearGearEventDelegate( eGED_ClimbDownLadder, OnClimbDownLadder );
}

/** Delegate fired when the player climbs down a ladder */
function OnClimbDownLadder()
{
	// Make sure this isn't fired when the tutorial has already completed
	if ( !bTutorialIsComplete )
	{
		OnTutorialCompleted();
	}
}

defaultproperties
{
	TutorialType=GEARTUT_Ladder
}
