/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Attack_Ticker_Charge extends AICommand
	within GearAI_Ticker;

/** GoW global macros */

var actor ChargeActor;

static function bool InitCommandUserActor(GearAI AI, Actor UserActor)
{
	local AICmd_Attack_Ticker_Charge Cmd;
	local GearAI_Ticker ChigAI;
	ChigAI = GearAI_Ticker(AI);
	if(ChigAI != none && UserActor != none)
	{
		Cmd = new(ChigAI) class'AICmd_Attack_Ticker_Charge';
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
	MyGearPawn.GroundSpeed=MyGearPawn.DefaultGroundSpeed*1.15;
	GearPawn_LocustTickerBase(MyGearPawn).PlayChargeSound();

	ReactionManager.SuppressChannel('EnemyCloseEnoughToCharge');
	GotoState( 'Charge' );
}

function Popped()
{
	Super.Popped();
	bWantsLedgeCheck = FALSE;
	MyGearPawn.GroundSpeed=MyGearPawn.DefaultGroundSpeed;
	// Allow normal rotation again
	bForceDesiredRotation = FALSE;
	GearPawn_LocustTickerBase(MyGearPawn).bCharging=false;
	ReactionManager.UnSuppressChannel('EnemyCloseEnoughToCharge');
}


state Charge
{
Begin:
	SetEnemyMoveGoal(true);
	if(ChildStatus == 'Failure')
	{
		AbortCommand(self);
	}
	else
	{
		if(Enemy != none && VSizeSq(Enemy.Location - MyGearPawn.Location) > ExplosionRadius * ExplosionRadius)
		{
			Goto('Begin');
		}

		if(!OtherTickerAboutToExplodeInArea())
		{
			class'AIcmd_Attack_Ticker_Explode'.static.InitCommand(outer);
		}		
		PopCommand(self);
	}
}


defaultproperties
{
}