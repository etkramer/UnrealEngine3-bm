/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_Ticker extends GearAI_Cover
	config(AI)
	native(AI);


const ExplosionRadius = 300;
const ChargeRadius = 1768;

var config int ExplosionHealthThresh;

var float LastSawEnemyTime;


cpptext
{
	virtual void UpdatePawnRotation();
};

event NotifyEnemySeen( Pawn SeenEnemy )
{
	Super.NotifyEnemySeen(SeenEnemy);
	LastSawEnemyTime = WorldInfo.TimeSeconds;
}

function BanzaiAttack()
{
	// don't hide from enemies when we're banzaiing!
	ReactionManager.SuppressChannel('SurpriseEnemyLoc');
	ReactionManager.SuppressChannel('NewEnemy');
	Super.BanzaiAttack();
}

function PlayExplodeAnim()
{
	local GearPawn_LocustTickerBase Ticker;
	Ticker = GearPawn_LocustTickerBase(MyGearPawn);
	Ticker.PlayExplodeAnim();
}

function DoExplosion()
{
	Pawn.Died(self,class'GDT_Explosive',Pawn.Location);
}

function bool ShouldMoveToSquad( optional bool bSkipLastPositionCheck )
{
	return FALSE;
}

event SetSquadName( Name NewSquadName, optional bool bLeader )
{
	// I'm in my own squad dag-nabit!
	Super.SetSquadname(name,true);
}

singular function bool StepAsideFor( Pawn ChkPawn );

function DoMeleeAttack( optional Pawn NewEnemy );

function bool CanEngageMelee();

function OnAIToggleCombat(SeqAct_AIToggleCombat Action)
{
	Super.OnAIToggleCombat(Action);
	if(!bAllowCombatTransitions)
	{
		ReactionManager.SuppressAll();
		AbortCommand(CommandList);
		Pawn.Acceleration=vect(0,0,0);
		Pawn.Velocity=vect(0,0,0);
		Pawn.GroundSpeed=0;
	}
	else
	{
		ReactionManager.UnSuppressAll();
		AutoAcquireEnemy();
		Pawn.GroundSpeed=MyGearPawn.DefaultGroundSpeed;
	}
}

/** called to check if we should hide due to seeing a new enemy or being flanked */
function ConsiderHiding(Actor EventInstigator, AIReactChannel OrigChannel)
{
	if (LineOfSightTo(EventInstigator))
	{
		// cancel any previous hide commands before pushing a new one
		AbortCommand(None, class'AICmd_Hide_Ticker');
		class'AICmd_Hide_Ticker'.static.InitCommandUserActor(self, EventInstigator);
	}
}

function bool OtherTickerAboutToExplodeInArea()
{
	local GearPawn_LocustTickerBase Ticker;
	foreach WorldInfo.AllPawns(class'GearPawn_LocustTickerBase', Ticker)
	{
		if(Ticker != none && Ticker != Pawn && Ticker.bExplode && !Ticker.IsPendingKill() && Ticker.Health > 0 && 
			VSizeSq(pawn.Location - Ticker.Location) < Ticker.ExplosionDamageRadius * Ticker.ExplosionDamageRadius)
		{
			return true;
		}
	}

	return false;

}


function bool ShouldDelayExplode()
{
	if(GearPawn_LocustTickerBase(MyGearPawn).bFlipped)
	{
		return true;
	}

	return false;
}


defaultproperties
{
	DefaultCommand=class'AICmd_Base_Ticker'
	bCanExecute=false
	bCanRevive=false

	DefaultReactConditions.Empty
	// enemy close conduit to trigger the push of the explosion
	Begin Object Class=AIReactCond_EnemyCloseAndVisible Name=ExplosionCloseAndVis0
		AutoSubscribeChannels(1)=Hearing
		AutoSubscribeChannels(2)=Force
		DistanceThreshold=ExplosionRadius
	End Object
	DefaultReactConditions.Add(ExplosionCloseAndVis0)

	// enemy close conduit to trigger the charge when an enemy is within range
	Begin Object Class=AIReactCond_EnemyCloseAndVisible Name=ChargeCloseAndVis0
		AutoSubscribeChannels(1)=Hearing
		AutoSubscribeChannels(2)=Force
		DistanceThreshold=ChargeRadius
		OutputChannelName=EnemyCloseEnoughToCharge
	End Object
	DefaultReactConditions.Add(ChargeCloseAndVis0)


	// generic pusher for hide when we see an enemy for the first time, or they're in a surprising location
	Begin Object Class=AIReactCond_GenericCallDelegate Name=HideFromNewEnemy0
		AutoSubscribeChannels(0)=NewEnemy
		AutoSubscribeChannels(1)=SurpriseEnemyLoc
		OutputFunction=ConsiderHiding
	End Object
	DefaultReactConditions.Add(HideFromNewEnemy0)

	// generic pusher for charge command when an enemy is close
	Begin Object Class=AIReactCond_GenericPushCommand Name=TriggerCharge0
		AutoSubscribeChannels(0)=EnemyCloseEnoughToCharge
		AutoSubscribeChannels(1)=Damage
		CommandClass=class'AICmd_Attack_Ticker_Charge'
	End Object
	DefaultReactConditions.Add(TriggerCharge0)

	Begin Object Class=AIReactCond_NewEnemy Name=NewEnemy0
		TimeSinceSeenThresholdSeconds=5.0f
	End Object
	DefaultReactConditions.Add(NewEnemy0)

	DefaultReactConditionClasses.Empty
	DefaultReactConditionClasses.Add(class'AIReactCond_SurpriseEnemyLoc')
	DefaultReactConditionClasses.Add(class'AIReactCond_Ticker_ShouldExplode')

	bPreciseDestination=true	
}
