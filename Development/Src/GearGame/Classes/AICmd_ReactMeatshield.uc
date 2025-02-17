/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICmd_ReactMeatshield extends AICommand_SpecialMove
	within GearAI;

/** GoW global macros */

/** pawn we are reacting to **/
var GearPawn ReactTarget;


/** Simple constructor that pushes a new instance of the command for the AI */
static function bool React( GearAI AI, GearPawn InReactTarget )
{
	local AICmd_ReactMeatshield Cmd;

	if( AI != None && 
		InReactTarget != None &&
		InReactTarget.IsAKidnapper() )
	{
		Cmd = new(AI) class'AICmd_ReactMeatshield';
        if( Cmd != None )
        {
            Cmd.ReactTarget = InReactTarget;
            AI.PushCommand(Cmd);
            return TRUE;
        }
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"ReactingTo:"@ReactTarget;
}

function Pushed()
{
	Super.Pushed();
	//debug
	`AILog( "... ReactTarget"@ReactTarget );
	// MT->Temporary, until we get assets
	MessagePlayer(Pawn@"Playing meatshield reaction.");

	GotoState( 'Command_SpecialMove' );
}

state Command_SpecialMove
{
	function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );

		// Need to face the target
		DesiredRotation = Rotator(ReactTarget.Location-Pawn.Location);
	}

	// Turn to face the target before doing the special move
	function bool ShouldFinishRotation() { return TRUE; }

	function ESpecialMove GetSpecialMove()
	{
		return SM_CoverHead;
	}
}
	
defaultproperties
{
}