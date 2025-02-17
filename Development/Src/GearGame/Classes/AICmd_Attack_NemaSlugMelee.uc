/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Attack_NemaSlugMelee extends AICommand_SpecialMove
	within GearAI;

function Pushed()
{
	Super.Pushed();

	bWantsLedgeCheck = TRUE;
	GotoState( 'Command_SpecialMove' );
}

function Popped()
{
	Super.Popped();
	bWantsLedgeCheck = FALSE;
	// Allow normal rotation again
	bForceDesiredRotation = FALSE;
}

state Command_SpecialMove
{
	function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );

		DesiredRotation = Rotator(Enemy.Location - Pawn.Location);
	}

	function ESpecialMove GetSpecialMove()
	{
		return GSM_SwipeAttack;
	}

	function bool ShouldFinishRotation() { return TRUE; }
}

defaultproperties
{
}