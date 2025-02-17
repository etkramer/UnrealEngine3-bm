/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_PawnToPawnInteractionFollower extends AICommand
	within GearAI;

/** GoW global macros */

/**
* Base state for Follower.
* Designed so the Special Move can take over the Pawn completely.
*/
function Pushed()
{
	Super.Pushed();

	ClearTimer('SpecialMoveTimeout');

	bReachedCover = FALSE;
	bPreparingMove = TRUE;			// Don't move until move done
	bForceDesiredRotation = TRUE;	// Fix pawn rotation and set the rotation

	if( MyGearPawn != None )
	{
		// Force pawn rotation dest to ours
		MyGearPawn.DesiredRotation = DesiredRotation;

		// Kill movement
		Pawn.ZeroMovementVariables();

		MyGearPawn.StopMeleeAttack();
	}

	ReactionManager.SuppressChannel('Damage');
	bIgnoreStepAside=true;
}

function Popped()
{
	Super.Popped();

	// Turn off flags
	bPreparingMove			= FALSE;
	bForceDesiredRotation	= FALSE;

	ReactionManager.UnSuppressChannel('Damage');
	bIgnoreStepAside=Outer.default.bIgnoreStepAside;
}

auto state WaitForStuff
{
Begin:
	//debug
	`AILog( "BEGIN TAG" @ GetStateName(), 'State' );

	Stop;
}