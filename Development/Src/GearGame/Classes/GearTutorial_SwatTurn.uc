/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_SwatTurn extends GearTutorial_Base
	config(Game);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Number of swat turns completed */
var int NumCompletedSwatTurns;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called when the tutorial gets added to the tutorial manager's list */
function OnTutorialAdded( GearTutorialManager TutMgr, SeqAct_ManageTutorials Action, bool IsKismetDriven )
{
	// Manually add the objective
	Action.Objective_bNotifyPlayer = true;
	Action.Objective_Name = 'SwatTurn';
	Action.Objective_Desc = "TUTORIAL_OBJ_07";

	Super.OnTutorialAdded( TutMgr, Action, IsKismetDriven );

	GearPC(Outer).AddGearEventDelegate( eGED_SwatTurn, OnSwatTurn );
}

/** Called when the tutorial gets removed from the tutorial manager's list */
function OnTutorialRemoved()
{
	Super.OnTutorialRemoved();

	GearPC(Outer).ClearGearEventDelegate( eGED_SwatTurn, OnSwatTurn );
}

/** Delegate fired when the player swat turns */
function OnSwatTurn()
{
	local GearPC MyGearPC;

	// Make sure this isn't fired when the tutorial has already completed
	if ( !bTutorialIsComplete )
	{
		// Increment swat turn counter
		NumCompletedSwatTurns++;

		MyGearPC = GearPC(Outer);

		// Update objective unless the tutorial is over (swat turned 3 times)
		switch ( NumCompletedSwatTurns )
		{
		case 1:
			MyGearPC.ObjectiveMgr.AddObjective( 'SwatTurn', "TUTORIAL_OBJ_07a", true );
			ResetUISceneOpenTimer( false );
			break;

		case 2:
			MyGearPC.ObjectiveMgr.AddObjective( 'SwatTurn', "TUTORIAL_OBJ_07b", true );
			ResetUISceneOpenTimer( false );
			break;

		default:
			OnTutorialCompleted();
			break;
		}
	}
}

defaultproperties
{
	TutorialType=GEARTUT_SwatTurn
}
