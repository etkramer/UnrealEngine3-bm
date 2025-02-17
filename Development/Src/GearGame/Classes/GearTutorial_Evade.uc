/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_Evade extends GearTutorial_Base
	config(Game);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** The number of times a player must evade before this tutorial goes active for this tutorial to auto-complete */
var config int NumEvadesForAutoComplete;

/** Number of times the player evaded since this tutorial was added to the system */
var int NumEvades;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called when the tutorial gets added to the tutorial manager's list */
function OnTutorialAdded( GearTutorialManager TutMgr, SeqAct_ManageTutorials Action, bool IsKismetDriven  )
{
	Super.OnTutorialAdded( TutMgr, Action, IsKismetDriven );

	GearPC(Outer).AddGearEventDelegate( eGED_Evade, OnEvade );
}

/** Called when the tutorial gets removed from the tutorial manager's list */
function OnTutorialRemoved()
{
	Super.OnTutorialRemoved();

	GearPC(Outer).ClearGearEventDelegate( eGED_Evade, OnEvade );
}

/** Delegate fired when the player evades (not from cover) */
function OnEvade()
{
	// Make sure this isn't fired when the tutorial has already completed
	if ( !bTutorialIsComplete )
	{
		// Keep track of how many evades happened because we will mark as complete if they've already mastered it
		NumEvades++;

		// Tutorial is complete if the evade happens when active
		if ( bTutorialIsActive || (NumEvades >= NumEvadesForAutoComplete) )
		{
			OnTutorialCompleted();
		}
	}
}

defaultproperties
{
	TutorialType=GEARTUT_Evade
}
