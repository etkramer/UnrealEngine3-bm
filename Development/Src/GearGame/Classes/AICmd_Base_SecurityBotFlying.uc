/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class AIcmd_Base_SecurityBotFlying extends AICommand_Base_Combat
	within GearAI_SecurityBotFlying;

var float CircleCounter;

/** GoW global macros */


function Pushed()
{
	Super.Pushed();
	GotoState('InCombat');
}

function Popped()
{
	Super.Popped();

}

function Resumed( Name OldCommandName )
{
	Super.Resumed(OldCommandName);
	StartFiring();
}


state LostLOS
{

	function bool NotifyEnemyBecameVisible( Pawn VisibleEnemy )
	{
		GotoState('InCombat');
		return true;
	}

Begin:
	MoveTo(Pawn.location);
	StopFiring();
	// wait around while we still have an enemy
	while(HasValidEnemy()&&!IsEnemyVisible(Enemy))
	{
		Sleep(0.5);
	}
	if(!HasValidEnemy())
	{
		GotoState('DelaySuccess');
	}
	else
	{
		GotoState('InCombat');
	}
	
};

state InCombat
{

	function Vector GetCirclePos()
	{
		local vector GoPos;
		local vector Offset;
		
		Offset.X = cos(CircleCounter/CirclingPeriod) * CirclingRadius;
		Offset.Y = sin(CircleCounter/CirclingPeriod) * CirclingRadius;
		
		GoPos = Enemy.Location + Offset;
		GoPos.Z = Enemy.Location.Z + Enemy.GetCollisionHeight();
		return GoPos;

	}
Begin:
	
	if(!SelectEnemy())
	{
		`AILog("Could not select enemy!");
		GotoState('DelayFailure');
	}

	if(!IsEnemyVisible(Enemy))
	{
		GotoState('LostLOS');
	}

	OnNewEnemy();
	Focus=Enemy;
	FinishRotation();
	Sleep(EnemyAcquiredGracePeriod);
	`AILog("sleeping"@EnemyAcquiredGracePeriod);
	StartFiring();

	while(HasValidEnemy())
	{
		StartFiring();
		// if we're too far away, get closer
		if(!HasEnemyWithinDistance(EnemyDistance_Short, Enemy, FALSE))
		{
			class'AICmd_Hover_MoveToGoal'.static.MoveToGoal(outer,Enemy,EnemyDistance_Short,384.f);
			Sleep(RandRange(0.5f,2.f));
		}

		// circle around
		MoveTo(GetCirclePos(),Enemy);
		CircleCounter += 0.25f;

	}
	Sleep(1.f);
	Goto('Begin');

};

