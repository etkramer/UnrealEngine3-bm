/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICmd_Base_Ticker extends AICommand_Base_Combat
	within GearAI_Ticker;

/** GoW global macros */

var CoverInfo Cover;

var() float MinWait;
var() float MaxWait;

var() float ChanceForNewCoverSpot;

var instanced Goal_AtCover InsectWander_AtCover;

function Pushed()
{
	Super.Pushed();
	GotoState('InCombat');
}

function bool ShouldIgnoreTimeTransitions()
{
	return TRUE;
}

// run around and act insecty
state InCombat
{

	function NewCoverSpot()
	{
		if( EvaluateCover(SearchType_Away,Pawn,Cover,,InsectWander_AtCover) )
		{
			Focus=none;
			SetMovePoint(Cover.Link.GetSlotLocation(Cover.SlotIdx));
		}		
	}

	function TwitchAround()
	{
		class'AICmd_StepAside'.static.StepAside( AIOwner, Pawn, FALSE );
	}

Begin:
	
	if( !SelectTarget() )
	{
		AutoAcquireEnemy();
		if(!SelectTarget())
		{
			GotoState('DelayFailure');
		}
	}
	
Loop:
	if(Enemy == none)
	{
		Goto('Begin');
	}

	// if we haven't seen our enemy in a long time, charge it
	if(TimeSince(LastSawEnemyTime) > 6.0f)
	{
		class'AICmd_Attack_Ticker_Charge'.static.InitCommandUserActor(outer,Enemy);
	}

	if(FRand() < ChanceForNewCoverSpot)
	{
		NewCoverSpot();
		Pawn.DesiredRotation=DesiredRotation;
		bForceDesiredRotation = TRUE;
		FinishRotation();
		bForceDesiredRotation = FALSE;
	}
	else
	{
		TwitchAround();
	}
	DesiredRotation=Rotator(Enemy.Location-Pawn.Location);
	Sleep(RandRange(MinWait,MaxWait));
	Goto('Loop');
}

defaultproperties
{
	MinWait=1.0
	MaxWait=3.5
	ChanceForNewCoverSpot=0.25

	// ---->Movement distance
	Begin Object Class=CovGoal_MovementDistance Name=CovGoal_MovDist0
		BestCoverDist=384.0f
		MinCoverDist=128.0f
		MaxCoverDist=512.0f
	End object

	Begin Object Class=Goal_AtCover name=AtCov_Insect0
		CoverGoalConstraints.Add(CovGoal_MovDist0)
	End object
	InsectWander_AtCover=AtCov_Insect0
}
