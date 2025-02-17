/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_React_Flanked extends AICommand_SpecialMove
	within GearAI_Cover;

/** GoW global macros */

var ESpecialMove FlankDirection;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool Flanked( GearAI_Cover AI, ESpecialMove Direction )
{
	local AICmd_React_Flanked Cmd;

	if( AI != None )
	{
		Cmd = new(AI) class'AICmd_React_Flanked';
		if( Cmd != None )
		{
			Cmd.FlankDirection = Direction;

			AI.PushCommand( Cmd );
			return TRUE;
		}
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Dir:"@FlankDirection;
}

function Pushed()
{
	Super.Pushed();
	bWantsLedgeCheck = TRUE;
	// Keep pawn's rotation as desired
	DesiredRotation = Pawn.Rotation;

	// since we're already reacting suppress from stacking up
	ReactionManager.SuppressReactionsByType(class'AIReactCond_Flanked',false);

	GotoState('Command_SpecialMove');
}

function Popped()
{
	Super.Popped();
	bWantsLedgeCheck = FALSE;
	SetAcquireCover( ACT_Immediate, "Leaving Flanked Reaction" );
	ResetCoverAction( TRUE );

	// since we're already reacting suppress from stacking up
	ReactionManager.UnSuppressReactionsByType(class'AIReactCond_Flanked',false);
}

state Command_SpecialMove
{
	function ESpecialMove GetSpecialMove()
	{
		return FlankDirection;
	}

	function FinishedSpecialMove()
	{
		//debug
		`AILog( GetStateName() );

		HandleFlankReaction();
	}
}

defaultproperties
{
	bShouldCheckSpecialMove=true
}
