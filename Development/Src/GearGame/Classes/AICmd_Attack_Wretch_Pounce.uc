/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Attack_Wretch_Pounce extends AICommand_SpecialMove
	within GearAI;

/** GoW global macros */

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

function Pushed()
{
	Super.Pushed();

	InvalidateCover();
	DesiredRotation = Rotator(Enemy.Location - Pawn.Location);
	bWantsLedgeCheck = TRUE;
	LockdownAI();
	GotoState( 'Command_SpecialMove' );
}

function Popped()
{
	Super.Popped();
	bWantsLedgeCheck = FALSE;
	// Allow normal rotation again
	bForceDesiredRotation = FALSE;
	UnlockAI();
}

state Command_SpecialMove
{
	function ESpecialMove GetSpecialMove()
	{
		return GSM_PounceAttack;
	}

	function bool ShouldFinishRotation() { return TRUE; }
}

defaultproperties
{
}