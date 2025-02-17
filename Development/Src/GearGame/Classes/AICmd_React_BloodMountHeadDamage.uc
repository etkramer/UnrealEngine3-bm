/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * - special move for the blood mount itself taking head damage
 */
class AICmd_React_BloodMountHeadDamage extends AICommand_SpecialMove
	within GearAI_Bloodmount;

/** GoW global macros */

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool React( GearAI AI )
{
	local AICmd_React_BloodMountHeadDamage Cmd;

	if( AI != None )
	{
		Cmd = new(GearAI_Bloodmount(AI)) class'AICmd_React_BloodMountHeadDamage';
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
	
	// only do this once, so as soon as we start suppress further reactions
	ReactionManager.SuppressReactionsByType(class'AIReactCond_BloodMountHeadDamage',false);

	GotoState( 'Command_SpecialMove' );
}

function Popped()
{
	super.Popped();
	// only notify the controller once the animation song and dance is finished
	NotifyHelmetBlownOff();
}

state Command_SpecialMove
{
	function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );

		// Need to face the target
		DesiredRotation = Rotator(Enemy.Location-Pawn.Location);
	}

	// Turn to face the target before doing the special move
	function bool ShouldFinishRotation() { return TRUE; }

	function ESpecialMove GetSpecialMove()
	{
		return SM_BloodMount_HitInFace;
	}
}
	
defaultproperties
{
	bIgnoreNotifies=TRUE
}