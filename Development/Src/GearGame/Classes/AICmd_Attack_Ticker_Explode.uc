/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Attack_Ticker_Explode extends AICommand
	within GearAI_Ticker;

/** GoW global macros */

function Pushed()
{
	Super.Pushed();

	DesiredRotation = Rotator(Enemy.Location - Pawn.Location);
	bWantsLedgeCheck = TRUE;
	GotoState( 'WaitForDeath' );
}

function Popped()
{
	super.Popped();
	MyGearPawn.GroundSpeed = MyGearPawn.DefaultGroundSpeed;
}
function Resumed( Name OldCommandName )
{
	// if we were resumed, cancel the 'splosion
	super.Resumed(OldCommandName);
	AbortCommand(self);
}

function bool AllowTransitionTo(class<AICommand> AttemptCommand )
{
	return FALSE;
}
function bool ShouldIgnoreTimeTransitions()
{
	return TRUE;
}

state WaitForDeath
{
Begin:
	bReachedCover			= FALSE;
	bPreparingMove			= TRUE;	// Don't move until move done
	bForceDesiredRotation	= TRUE;	// Fix pawn rotation and set the rotation

	MoveTo(Pawn.Location);
	if( MyGearPawn != None )
	{
		MyGearPawn.GroundSpeed = 0.0;
		Pawn.ZeroMovementVariables();
	}

	PlayExplodeAnim();
}

defaultproperties
{
	bIgnoreNotifies=true
}
