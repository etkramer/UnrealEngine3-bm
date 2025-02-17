/**
 * Base class for behavior of Berserker
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICmd_Base_Berserker extends AICommand_Base_Combat
	within GearAI_Berserker;

auto state InCombat
{

Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );

	SelectEnemy();
	FireTarget = Enemy;
	if (!HasValidEnemy())
	{
		Sleep(1.0);
	}
	else if (ActorReachable(Enemy))
	{
		SetChargeLocation(Enemy, Enemy.Location);
		BeginCombatCommand(class'AICmd_Berserker_Kamikaze', "Direct path to enemy");
	}
	else
	{
		if (MyGearPawn != None)
		{
			MyGearPawn.DoSpecialMove(SM_Berserker_Alert, true);
		}
		class'AICmd_MoveToEnemy'.static.MoveToEnemy(Outer, false, 512.0, 0.f);
		if (MyGearPawn != None && MyGearPawn.IsDoingSpecialMove(SM_Berserker_Alert))
		{
			MyGearPawn.EndSpecialMove();
		}
	}
	CheckCombatTransition();
	Goto('Begin');
}

defaultproperties
{
}
