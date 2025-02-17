/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Move_Evade extends AICommand_SpecialMove
	within GearAI;

/** GoW global macros */

var ESpecialMove EvadeDirection;
var float EvadeDelay;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool Evade( GearAI AI, ESpecialMove Direction, optional float InEvadeDelay )
{
	local AICmd_Move_Evade Cmd;

	if( AI != None )
	{
		Cmd = new(AI) class'AICmd_Move_Evade';
		if( Cmd != None )
		{
			Cmd.EvadeDirection = Direction;
			Cmd.EvadeDelay = InEvadeDelay;
			AI.PushCommand( Cmd );
			return TRUE;
		}
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Dir:"@EvadeDirection;
}

function Pushed()
{
	Super.Pushed();

	bWantsLedgeCheck = TRUE;
	// Keep pawn's rotation as desired
	DesiredRotation = Pawn.Rotation;

	if(EvadeDelay > 0.f)
	{
		GotoState('Wait');
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
}

state Wait
{
Begin:
	Sleep(EvadeDelay);
	GotoState('Command_SpecialMove');
}

state Command_SpecialMove
{
	function ESpecialMove GetSpecialMove()
	{
		return EvadeDirection;
	}
}

defaultproperties
{
	bShouldCheckSpecialMove=true
}
