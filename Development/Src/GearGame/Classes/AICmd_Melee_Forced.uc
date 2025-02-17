/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class AICmd_Melee_Forced extends AICmd_Base_Melee
	within GearAI;

var actor MeleeTarget;
static function bool ForceMelee(GearAI AI, Actor InMeleeTarget)
{
	local AICmd_Melee_Forced Cmd;

	if(AI != none && InMeleeTarget != none)
	{
		Cmd = new(AI) class'AICmd_Melee_Forced';
		Cmd.MeleeTarget = InMeleeTarget;
		AI.PushCommand(Cmd);
		return TRUE;
	}

	return FALSE;
}

state InCombat
{
Begin:
	//debug
	`AILog( "BEGIN TAG"@Pawn.Weapon@Enemy, 'State' );

	if(MeleeTarget==none)
	{
		MeleeTarget = Enemy;
	}

	if(MeleeTarget == none)
	{
		`AILog("Aborting because we have no melee target!");
		GotoState('DelayFailure');
	}

	// if we have a lancer, start the saw early
	if(MyGearPawn != none && MyGearPawn.Weapon != none && 
		(MyGearPawn.Weapon.IsA('GearWeap_LocustAssaultRifle') || MyGearPawn.Weapon.IsA('GearWeap_AssaultRifle')) )
	{
		`AILog("Starting melee");
		bAllowedToFireWeapon = TRUE;
		StartMeleeAttack();
	}

	if (MyGearPawn != None && VSize(MyGearPawn.Location - MeleeTarget.Location) > MyGearPawn.GetAIMeleeAttackRange() && !MyGearPawn.ReachedDestination(MeleeTarget))
	{
		`AILog("Not in weapon's melee range, move to MeleeTarget..."@VSize(MyGearPawn.Location - MeleeTarget.Location)@MyGearPawn.GetAIMeleeAttackRange(), 'Combat');
		SetMoveGoal(MeleeTarget,,false,75.f);
	}
	// If no LOS to enemy
	else if( !CanSeeByPoints( Pawn.GetPawnViewLocation(), MeleeTarget.Location, Rotator(MeleeTarget.Location - Pawn.GetPawnViewLocation()) ) )
	{
		//debug
		`AILog( "No LOS, move to enemy...", 'Combat' );

		// Move to him first
		SetMoveGoal(MeleeTarget,,false);
	}

	if( ChildStatus != 'Aborted' && ChildStatus != 'Failure')
	{
		if(Pawn(MeleeTarget) != none)
		{
			DoMeleeAttack( Pawn(MeleeTarget) );
		}
		else
		{
			class'AICmd_Attack_Melee'.static.Melee(Outer,MeleeTarget);
		}
	}
	else
	{
		`AILog("Failed to move to target... trying again");
		Sleep(1.0f);
		Goto('Begin');
	}

	// Check combat transitions
	CheckCombatTransition();
	Goto('Begin');
}

defaultproperties
{
}
