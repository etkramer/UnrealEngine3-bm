/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class AICmd_Emerge extends AICommand_SpecialMove
	within GearAI;


function Pushed()
{
	Super.Pushed();

	// FORCE pawn into phys none so no falling occurs just before emerge action is done
	// This is what was causing them to sometimes fall back through the emergence hole
	Pawn.SetPhysics( PHYS_None );

	MyGearPawn.bSpawning = TRUE;

	// Force our desired rotation to the pawn's rotation
	// so he doesn't rotate after spawning
	DesiredRotation = Pawn.Rotation;
	
	ReactionManager.SuppressAll();
	
	GotoState( 'Command_SpecialMove' );
}

function Popped()
{
	super.Popped();
	ReactionManager.UnSuppressAll();

	`AILog("Finished with emerge");
	if (Pawn != None)
	{
		Pawn.SetMovementPhysics();
		`AILog(GetFuncName()@self@"bSpawning FALSE!");
		MyGearPawn.bSpawning = FALSE;
	}
}

function bool AllowTransitionTo( class<AICommand> AttemptCommand )
{
	return false;
}

state Command_SpecialMove
{
	function ESpecialMove GetSpecialMove()
	{
		return EmergeSpecialMove;
	}
}

DefaultProperties
{
	bIgnoreNotifies=true
}