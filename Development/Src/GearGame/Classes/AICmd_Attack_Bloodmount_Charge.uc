/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Attack_Bloodmount_Charge extends AICommand
	within GearAI_Bloodmount;


var actor ChargeActor;

static function bool InitCommandUserActor(GearAI AI, Actor UserActor)
{
	local AICmd_Attack_Bloodmount_Charge Cmd;
	local GearAI_Bloodmount BMAI;
	BMAI = GearAI_Bloodmount(AI);
	if(BMAI != none && UserActor != none)
	{
		Cmd = new(BMAI) class'AICmd_Attack_Bloodmount_Charge';
		if(Cmd != none)
		{
			Cmd.ChargeActor = UserActor;
			AI.PushCommand(Cmd);
			return TRUE;
		}
	}

	return FALSE;
}

function Pushed()
{
	Super.Pushed();

	if(Enemy == none)
	{
		GotoState('DelayFailure');
		return;
	}

	DesiredRotation = Rotator(Enemy.Location - Pawn.Location);
	bWantsLedgeCheck = TRUE;

	if(MyBloodMountPawn != none)
	{
		MyBloodMountPawn.NotifyCharging(ChargeActor);
	}

	ReactionManager.SuppressChannel('EnemyCloseEnoughToCharge');
	GotoState( 'Charge' );
}

function Popped()
{
	Super.Popped();
	bWantsLedgeCheck = FALSE;
	MyBloodMountPawn.NotifyCharging(none);
	ReactionManager.UnSuppressChannel('EnemyCloseEnoughToCharge');
}


state Charge
{
Begin:
	SetEnemyMoveGoal(TRUE);
	if(ChildStatus == 'Failure')
	{
		AbortCommand(self);
	}
	else
	{
		CheckInterruptCombatTransitions();
		PopCommand(self);
	}
}


defaultproperties
{
}