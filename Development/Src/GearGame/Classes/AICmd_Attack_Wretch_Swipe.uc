/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Attack_Wretch_Swipe extends AICommand_SpecialMove
	within GearAI;

/** GoW global macros */

function Pushed()
{
	Super.Pushed();

	GotoState( 'Command_SpecialMove' );
}

function LockdownAI()
{
	//debug
	`AILog( GetFuncName() );

	bReachedCover			= FALSE;
	bPreparingMove			= TRUE;	// Don't move until move done
	bForceDesiredRotation	= TRUE;	// Fix pawn rotation and set the rotation

	if( MyGearPawn != None )
	{
		// Force pawn rotation dest to ours and kill movement
		MyGearPawn.DesiredRotation	= DesiredRotation;
		Pawn.ZeroMovementVariables();
	}
}

state Command_SpecialMove
{
	function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );

		DesiredRotation = Rotator(Enemy.Location - Pawn.Location);

		LockdownAI();

	}

	function ESpecialMove GetSpecialMove()
	{
		return GSM_SwipeAttack;
	}

	function bool ShouldFinishRotation() { return TRUE; }

	function FinishedSpecialMove()
	{
		UnlockAI();
	}
}

defaultproperties
{
}