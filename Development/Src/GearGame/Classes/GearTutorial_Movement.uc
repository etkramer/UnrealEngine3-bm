/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_Movement extends GearTutorial_Base
	config(Game);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Amount of time the player must move to have this tutorial complete */
var config float TotalMovementTimeForCompletetion;

/** Total time this player has moved since this tutorial was added to the system */
var float TotalMovementTime;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Called when the tutorial gets added to the tutorial manager's list */
function OnTutorialAdded( GearTutorialManager TutMgr, SeqAct_ManageTutorials Action, bool IsKismetDriven  )
{
	Super.OnTutorialAdded( TutMgr, Action, IsKismetDriven );

	bTutorialIsReadyForActivation = true;
}

/** Called every tick so that the tutorial has a chance to update itself */
event Update()
{
	local GearPlayerInput_Base MyPCInput;

	if ( !bTutorialIsComplete )
	{
		MyPCInput = GearPlayerInput_Base(GearPC(Outer).PlayerInput);
		if ( MyPCInput != None )
		{
			// If the player is moving, increment the movement time
			if ( MyPCInput.IsButtonActive(GB_LeftStick_Up) || MyPCInput.IsButtonActive(GB_LeftStick_Down) || MyPCInput.IsButtonActive(GB_LeftStick_Left) || MyPCInput.IsButtonActive(GB_LeftStick_Right) )
			{
				TotalMovementTime += TutorialMgr.WorldInfo.DeltaSeconds;
			}
		}

		// See if they moved enough. If so then complete the tutorial
		if ( TotalMovementTime > TotalMovementTimeForCompletetion )
		{
			OnTutorialCompleted();
		}
	}

	Super.Update();
}


defaultproperties
{
	TutorialType=GEARTUT_Movement
	TutorialPriority=TUTORIAL_PRIORITY_AUTOINIT
}
