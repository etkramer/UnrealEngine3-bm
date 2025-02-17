/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_Cover extends GearTutorial_Base
	config(Game);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Next time to check for if the player can be in cover */
var float TimeToCheckForCover;

/** Whether the player is in cover or not */
var bool bPlayerIsInCover;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called when the tutorial has become the active tutorial by the tutorial manager */
function bool TutorialActivated()
{
	Super.TutorialActivated();

	// Init the checking for cover data
	TimeToCheckForCover = 0;
	CheckForNearCover();

	// Set the acquired cover delegate
	GearPC(Outer).AddGearEventDelegate( eGED_CoverAcquired, OnCoverAcquired );
	
	return true;
}

/** Called every tick so that the tutorial has a chance to update itself */
event Update()
{
	CheckForNearCover();

	Super.Update();
}

/** See if we are in cover or not */
function CheckForNearCover()
{
	local GearPC GPC;
	local GearPawn GP;
	local CovPosInfo CoverData;

	// Timer expired, do the check
	if ( TimeToCheckForCover < TutorialMgr.WorldInfo.TimeSeconds )
	{
		GPC = GearPC(Outer);
		if ( GPC != None )
		{
			GP = GearPawn(GPC.Pawn);
			if ( GP != None )
			{
				// See if the player is in a situation where they can take cover
				bPlayerIsInCover = GP.CanPrepareRun2Cover( GP.Run2CoverMaxDist*0.75f, CoverData );

				// Reset the timer
				TimeToCheckForCover = TutorialMgr.WorldInfo.TimeSeconds + 0.1f;
			}
		}
	}
}

/** Overloaded so that we can open this scene if the player is in cover */
function bool ShouldOpenUIScene()
{
	if ( bTutorialIsActive && !bSceneIsOpen && (!bSceneHasBeenOpened || bPlayerIsInCover) )
	{
		return true;
	}
	return false;
}

/** Determine if we should close the UI scene or not */
function bool ShouldCloseUIScene()
{
	if ( bTutorialIsActive && bSceneIsOpen && ((SceneOpenTimeAmount > 0) && (!bPlayerIsInCover && TutorialMgr.WorldInfo.TimeSeconds > SceneOpenTimeAmount+TimeOfSceneOpen)) )
	{
		return true;
	}
	return false;
}

/** Delegate fired when the player acquires cover */
function OnCoverAcquired()
{
	// Clear the delegate
	GearPC(Outer).ClearGearEventDelegate( eGED_CoverAcquired, OnCoverAcquired );

	// Tutorial is complete
	OnTutorialCompleted();
}

defaultproperties
{
	TutorialType=GEARTUT_Cover
}
