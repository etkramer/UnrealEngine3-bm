/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_RoadieRun extends GearTutorial_Base
	config(Game);


/** Whether this tutorial was automatically closed because the player already had the weapon equipped */
var bool bAutoCompleted;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called when the tutorial gets added to the tutorial manager's list */
function OnTutorialAdded( GearTutorialManager TutMgr, SeqAct_ManageTutorials Action, bool IsKismetDriven )
{
	Super.OnTutorialAdded( TutMgr, Action, IsKismetDriven );

	GearPC(Outer).AddGearEventDelegate( eGED_RoadieRun, OnRoadieRun );
}

/** Called when the tutorial gets removed from the tutorial manager's list */
function OnTutorialRemoved()
{
	Super.OnTutorialRemoved();

	GearPC(Outer).ClearGearEventDelegate( eGED_RoadieRun, OnRoadieRun );
}

/** Delegate fired when the player roadie runs */
function OnRoadieRun()
{
	// Make sure this isn't fired when the tutorial has already completed
	if ( !bTutorialIsComplete )
	{
		OnTutorialCompleted();
	}
}

/** Called every tick so that the tutorial has a chance to update itself */
event Update()
{
	local GearPC MyGearPC;

	MyGearPC = GearPC(Outer);

	// See if we should auto complete this tutorial because the player is already roadie running
	if (!bAutoCompleted && MyGearPC != None && MyGearPC.IsDoingSpecialMove(SM_RoadieRun))
	{
		bAutoCompleted = true;
		OnTutorialCompleted();
	}

	Super.Update();
}

defaultproperties
{
	TutorialType=GEARTUT_RoadieRun
}
