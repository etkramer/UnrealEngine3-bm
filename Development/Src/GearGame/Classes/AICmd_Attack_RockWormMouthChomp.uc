/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Attack_RockWormMouthChomp extends AICommand_SpecialMove
	within GearAI_RockWorm;

/** GoW global macros */

static function bool Chomp( GearAI_RockWorm AI, GearPawn Chompee )
{
	local AICmd_Attack_RockWormMouthChomp Cmd;

	if( AI != None )
	{
		Cmd = new(AI) Default.Class;
		if( Cmd != None )
		{
			AI.PushCommand(Cmd);
			return TRUE;
		}
	}

	return FALSE;
}

function Pushed()
{
	Super.Pushed();

	GotoState( 'Command_SpecialMove' );
}


state Command_SpecialMove
{
	function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );

		DesiredRotation = Rotator(GearPawn_RockwormBase(MyGearPawn).ChompVictim.Location - Pawn.Location);
	}

	function ESpecialMove GetSpecialMove()
	{
		return GSM_SwipeAttack;
	}

	function bool ShouldFinishRotation() { return FALSE; }
}

defaultproperties
{
}