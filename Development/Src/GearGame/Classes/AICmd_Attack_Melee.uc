/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Attack_Melee extends AICommand_SpecialMove
	within GearAI;

/** GoW global macros */

var name DefaultStartState;

var Actor MeleeTarget;
/** Simple constructor that pushes a new instance of the command for the AI */
static function bool Melee( GearAI AI, optional Actor InTarget )
{
	local AICmd_Attack_Melee Cmd;

	if( AI != None )
	{
		Cmd = new(AI) default.class;
		if( Cmd != None )
		{
			if(InTarget != none)
			{
				Cmd.MeleeTarget = InTarget;
			}
			else
			{
				Cmd.MeleeTarget = AI.Enemy;
			}
			AI.PushCommand( Cmd );
			return TRUE;
		}
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"MeleeTarget:"@MeleeTarget;
}

function Pushed()
{
	Super.Pushed();

	bDoingMeleeAttack = TRUE;

	InvalidateCover();
	StopFiring();

	// Do melee attack immediately
	GotoState( DefaultStartState );
}

function Paused( AICommand NewCommand )
{
	Super.Paused( NewCommand );

	StopFiring();
}

function Popped()
{
	Super.Popped();

	bDoingMeleeAttack = FALSE;

	StopMeleeAttack();
}

function bool AllowTransitionTo( class<AICommand> AttemptCommand )
{
	if(ChildCommand != none)
	{
		return ChildCommand.AllowTransitionTo( AttemptCommand );
	}

	return IsSpecialMoveComplete();

}
function bool IsSpecialMoveComplete()
{
	return true;
}


state Command_SpecialMove
{
	function bool ShouldFinishRotation()
	{
		// Pawn can't do full rotation when carrying crate so just don't wait for it and do the best we can
		return (MyGearPawn == None || MyGearPawn.CarriedCrate == None);
	}

	function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );

		DesiredRotation = Rotator(MeleeTarget.Location - Pawn.Location);
	}

	function bool ExecuteSpecialMove()
	{
		if( MyGearPawn == None )
			return FALSE;

		if( MyGearPawn.MyGearWeapon != None ) //&& Weap.MeleeAttackImpact(,,, TRUE ) )
		{
			//debug
			`AILog( "Target in melee range... do attack" @ MyGearPawn.MyGearWeapon, 'Combat' );

			MyGearPawn.MyGearWeapon.bHitWallThisAttack = FALSE;
			MyGearPawn.MyGearWeapon.NumMeleeAttackAttempts = 0;
			bAllowedToFireWeapon = TRUE;
			FireTarget = MeleeTarget;
			StartMeleeAttack();

			return TRUE;
		}

		return FALSE;
	}

	// Melee attack is complete if...
	function bool IsSpecialMoveComplete()
	{
		//`AILog(GetFuncname()@MyGearpawn@MyGearPawn.bDoingMeleeAttack@MyGearpawn.IsDoingSpecialMeleeAttack()@MeleeTarget@isValidMeleeTarget(GearPawn(MeleeTarget)));
		if( MyGearPawn != None )
		{
			if( !MyGearPawn.bDoingMeleeAttack && !MyGearPawn.IsDoingSpecialMeleeAttack() )
			{
				return TRUE;
			}
			else if(MyGearpawn.IsDoingSpecialMeleeAttack())
			{
				return FALSE;
			}
		}

		// No pawn or pawn doesn't want to melee and is not currently committed to an attack
		// Pawn can no longer engage in melee or target is no longer valid
		// or Target is no longer in range and is not currently committed to an attack
		if ( MyGearPawn == None || MeleeTarget == None || MeleeTarget.bDeleteMe ||
			(GearPawn(MeleeTarget) != None && !IsValidMeleeTarget(GearPawn(MeleeTarget))))
		{
			`AILog(GetFuncName()@"check #2 returning true, MyGearPawn:" @ MyGearPawn@MyGearPawn.IsDoingSpecialMeleeAttack()@IsValidMeleeTarget(GearPawn(Enemy)));
			return TRUE;
		}

		// if enemy is no longer in range, turn off wanting to melee and wait for any current attack to complete, unless reacheddestination thinks we're here, in which case keep going
		if ( VSize(MeleeTarget.Location - Pawn.Location) > MyGearPawn.GetAIMeleeAttackRange() &&
			(Pawn(MeleeTarget) != None || !Pawn.ReachedDestination(MeleeTarget)) )
		{
			//`AILog(GetFuncName()@"Stopping melee attack"@VSize(MeleeTarget.Location - Pawn.Location)@MyGearPawn.GetAIMeleeAttackRange());
			StopMeleeAttack();
		}

		return FALSE;
	}

	function FinishedSpecialMove()
	{
		bAllowedToFireWeapon = FALSE;
		EndOfMeleeAttackNotification();
	}
}

defaultproperties
{
	DefaultStartState=Command_SpecialMove
}
