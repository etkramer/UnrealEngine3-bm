/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Attack_KantusKnockDownScream extends AICommand_SpecialMove
	within GearAI_Kantus;

/** GoW global macros */

function Pushed()
{
	Super.Pushed();

	InvalidateCover();
	DesiredRotation = Rotator(Enemy.Location - Pawn.Location);
	bWantsLedgeCheck = TRUE;
	ReactionManager.SuppressAll();
	if(!CanDoKnockdown())
	{
		GotoState('DelayFailure');
	}
	else
	{
		GotoState( 'Command_SpecialMove' );
	}
}

function Popped()
{
	Super.Popped();
	bWantsLedgeCheck = FALSE;
	// Allow normal rotation again
	ReactionManager.UnSuppressAll();
	bForceDesiredRotation = FALSE;

	if(Status=='Success')
	{
		LastSuccessfulKnockdownTime=WorldInfo.TimeSeconds;
	}
}

state Command_SpecialMove
{
	function ESpecialMove GetSpecialMove()
	{
		return GSM_Kantus_KnockDownScream;
	}

	function bool ShouldFinishRotation() { return TRUE; }
}

defaultproperties
{
	bShouldCheckSpecialMove=true
}
