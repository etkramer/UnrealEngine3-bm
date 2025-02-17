/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICmd_Move_ClimbLadder extends AICommand_SpecialMove
	within GearAI;
	
/** GoW global macros */

/** Start and End Markers */
var LadderMarker	LadderStart, LadderEnd;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool ClimbLadder( GearAI AI, LadderMarker NewStart, LadderMarker NewEnd )
{
	local AICmd_Move_ClimbLadder Cmd;

	if( AI != None && NewStart != None && NewEnd != None )
	{
		Cmd = new(AI) class'AICmd_Move_ClimbLadder';
		if( Cmd != None )
		{
			Cmd.LadderStart = NewStart;
			Cmd.LadderEnd	= NewEnd;

			AI.PushCommand( Cmd );
			return TRUE;
		}
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Start:"@LadderStart@"End:"@LadderEnd@"Top:"@LadderStart.bIsTopOfLadder;
}

function Pushed()
{
	Super.Pushed();

	AIOwner.MyGearPawn.LadderTrigger = LadderStart.LadderTrigger;

	// Do the execution
	GotoState( 'Command_SpecialMove' );
}

state Command_SpecialMove
{
	function ESpecialMove GetSpecialMove()
	{
		if( LadderStart.bIsTopOfLadder )
		{
			return SM_LadderClimbDown;
		}
		else
		{
			return SM_LadderClimbUp;
		}
	}
}

function NavigationPoint GetUpdatedAnchor()
{
	return LadderEnd;
}

defaultproperties
{
	bUpdateAnchorOnSuccess=TRUE
}
