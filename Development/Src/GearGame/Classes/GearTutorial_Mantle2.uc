/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearTutorial_Mantle2 extends GearTutorial_Base
	config(Game);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Whether the player is in cover they can mantle or not */
var bool bPlayerIsInMantleCover;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called when the tutorial gets added to the tutorial manager's list */
function OnTutorialAdded( GearTutorialManager TutMgr, SeqAct_ManageTutorials Action, bool IsKismetDriven  )
{
	local GearPc MyGearPC;

	Super.OnTutorialAdded( TutMgr, Action, IsKismetDriven );

	// Set the delegates
	MyGearPC = GearPC(Outer);
	MyGearPC.AddGearEventDelegate( eGED_CoverLeave, OnCoverLeave );
	MyGearPC.AddGearEventDelegate( eGED_CoverAcquired, OnCoverAcquired );
	MyGearPC.AddGearEventDelegate( eGED_Mantle, OnMantle );

	// Add and start the Mantle1 tutorial which will instruct the player to get into cover
	TutorialMgr.AddTutorial( GEARTUT_Mantle1 );
	TutorialMgr.StartTutorial( GEARTUT_Mantle1 );
}

/** Called when the tutorial gets removed from the tutorial manager's list */
function OnTutorialRemoved()
{
	local GearPC MyGearPC;

	Super.OnTutorialRemoved();

	// Clear the delegates
	MyGearPC = GearPC(Outer);
	MyGearPC.ClearGearEventDelegate( eGED_CoverLeave, OnCoverLeave );
	MyGearPC.ClearGearEventDelegate( eGED_CoverAcquired, OnCoverAcquired );
	MyGearPC.ClearGearEventDelegate( eGED_Mantle, OnMantle );
	TutorialMgr.RemoveTutorial( GEARTUT_Mantle1 );
}

/** Delegate fired when the player leaves cover */
function OnCoverLeave()
{
	if ( bPlayerIsInMantleCover )
	{
		TutorialMgr.StopTutorial( GEARTUT_Mantle2 );
		TutorialMgr.AddTutorial( GEARTUT_Mantle1 );
		TutorialMgr.StartTutorial( GEARTUT_Mantle1 );
	}

	bPlayerIsInMantleCover = false;
}

/** Delegate fired when the player acquires cover */
function OnCoverAcquired()
{
	local GearPawn GP;

	// See if this cover is able to be mantled
	GP = GearPC(Outer).MyGearPawn;
	if ( GP != None )
	{
		bPlayerIsInMantleCover = GP.CurrentLink.Slots[GP.CurrentSlotIdx].bCanMantle;
	}

	if ( bPlayerIsInMantleCover && !bTutorialIsComplete )
	{
		TutorialMgr.RemoveTutorial( GEARTUT_Mantle1 );
		TutorialMgr.StartTutorial( GEARTUT_Mantle2 );
	}
}

/** Delegate fired when the player mantles */
function OnMantle()
{
	local GearPC MyGearPC;

	// Clear the delegates
	MyGearPC = GearPC(Outer);
	MyGearPC.ClearGearEventDelegate( eGED_CoverLeave, OnCoverLeave );
	MyGearPC.ClearGearEventDelegate( eGED_CoverAcquired, OnCoverAcquired );
	MyGearPC.ClearGearEventDelegate( eGED_Mantle, OnMantle );

	// Make the Mantle1 tutorial complete
	TutorialMgr.MarkTutorialComplete( GEARTUT_Mantle1 );

	// Tutorial is complete
	OnTutorialCompleted();
}


defaultproperties
{
	TutorialType=GEARTUT_Mantle2
}

