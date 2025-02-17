/**
 * GearTutorialFirstTimeEvent_Base
 *
 * Base class for tutorials that will be triggered the first time a player encounters some game event
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorialFirstTimeEvent_Base extends GearTutorial_Base
	config(Game);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** The delegate event this tutorial will hook into in the PC */
var EGearEventDelegateType EventDelegateType;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called when the tutorial gets added to the tutorial manager's list */
function OnTutorialAdded( GearTutorialManager TutMgr, SeqAct_ManageTutorials Action, bool IsKismetDriven )
{
	Super.OnTutorialAdded( TutMgr, Action, IsKismetDriven );

	GearPC(Outer).AddGearEventDelegate( EventDelegateType, OnFirstTimeEventEncountered );
}

/** Called when the tutorial gets removed from the tutorial manager's list */
function OnTutorialRemoved()
{
	Super.OnTutorialRemoved();

	GearPC(Outer).ClearGearEventDelegate( EventDelegateType, OnFirstTimeEventEncountered );
}

/** Called when the tutorial's scene is finished closing */
function OnTutorialSceneClosed( UIScene DeactivatedScene )
{
	Super.OnTutorialSceneClosed( DeactivatedScene );

	// Tutorial is done after the scene closes
	OnTutorialCompleted();
}

/** Delegate fired when the player encounters some particular action defined by EventDelegateType */
function OnFirstTimeEventEncountered()
{
	// Make sure this isn't fired when the tutorial has already completed
	if ( !bTutorialIsComplete && !bTutorialIsActive )
	{
		bTutorialIsReadyForActivation = TRUE;
	}
}


defaultproperties
{
	TutorialPriority=TUTORIAL_PRIORITY_AUTOINIT
}
