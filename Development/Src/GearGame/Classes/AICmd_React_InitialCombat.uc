/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_React_InitialCombat extends AICommand_SpecialMove
	within GearAI_Cover;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool InitialCombatReact( GearAI_Cover AI )
{
	local AICmd_React_InitialCombat Cmd;

	if( AI != None )
	{
		Cmd = new(AI) class'AICmd_React_InitialCombat';
		if( Cmd != None )
		{
			AI.PushCommand( Cmd );
			return TRUE;
		}
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@Enemy;
}

function Pushed()
{
	Super.Pushed();

	// since we're already reacting suppress from stacking up
	ReactionManager.SuppressReactionsByType(class'AIReactCond_Flanked',false);

	GotoState('Command_SpecialMove');
}

function Popped()
{
	Super.Popped();

	// since we're already reacting suppress from stacking up
	ReactionManager.UnSuppressReactionsByType(class'AIReactCond_Flanked',false);
}

state Command_SpecialMove
{
	function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );

		if( SelectEnemy() )
		{
			DesiredRotation = rotator(Enemy.Location - Pawn.Location);
		}
		else
		{
			DesiredRotation = Pawn.Rotation;
		}
	}

	// Make sure AI is facing the correct way
	function bool ShouldFinishRotation()	{ return TRUE; }

	function ESpecialMove GetSpecialMove()
	{
		return SM_Reaction_InitCombat;
	}
}

defaultproperties
{
	bShouldCheckSpecialMove=true
}
