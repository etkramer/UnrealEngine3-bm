/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Berserker_SmashAttack extends AICommand_SpecialMove;

function Popped()
{
	Super.Popped();

	ForceUpdateOfEnemies();
	CheckCombatTransition();
}

auto state Command_SpecialMove
{
	event BeginState(name PreviousStateName)
	{
		Super.BeginState(PreviousStateName);

		DesiredRotation = Rotator(Enemy.Location-Pawn.Location);
		SetTimer( 0.5, false, 'WarnEnemiesOfAttack' );
	}

	function bool ShouldFinishRotation()
	{
		return true;
	}
	function ESpecialMove GetSpecialMove()
	{
		return SM_Berserker_Smash;
	}
}


DefaultProperties
{
	bIgnoreNotifies=true
}