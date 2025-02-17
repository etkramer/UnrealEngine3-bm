/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_CoverSlip extends GearTutorial_Base
	config(Game);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Number of cover slips completed */
var int NumCompletedCoverSlips;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called when the tutorial gets added to the tutorial manager's list */
function OnTutorialAdded( GearTutorialManager TutMgr, SeqAct_ManageTutorials Action, bool IsKismetDriven  )
{
	// Manually add the objective
	Action.Objective_bNotifyPlayer = true;
	Action.Objective_Name = 'CoverSlip';
	Action.Objective_Desc = "TUTORIAL_OBJ_06";

	Super.OnTutorialAdded( TutMgr, Action, IsKismetDriven );

	GearPC(Outer).AddGearEventDelegate( eGED_CoverSlip, OnCoverSlip );
}

/** Called when the tutorial gets removed from the tutorial manager's list */
function OnTutorialRemoved()
{
	Super.OnTutorialRemoved();

	GearPC(Outer).ClearGearEventDelegate( eGED_CoverSlip, OnCoverSlip );
}

/** Delegate fired when the player cover slips */
function OnCoverSlip()
{
	local GearPC MyGearPC;

	// Make sure this isn't fired when the tutorial has already completed
	if ( !bTutorialIsComplete )
	{
		// Increment cover slip counter
		NumCompletedCoverSlips++;

		MyGearPC = GearPC(Outer);

		// Update objective unless the tutorial is over (cover slipped 3 times)
		switch ( NumCompletedCoverSlips )
		{
			case 1:
				MyGearPC.ObjectiveMgr.AddObjective( 'CoverSlip', "TUTORIAL_OBJ_06a", true );
				ResetUISceneOpenTimer( false );
				break;

			case 2:
				MyGearPC.ObjectiveMgr.AddObjective( 'CoverSlip', "TUTORIAL_OBJ_06b", true );
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
	TutorialType=GEARTUT_CoverSlip
}
