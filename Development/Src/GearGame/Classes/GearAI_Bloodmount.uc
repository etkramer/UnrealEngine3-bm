/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_Bloodmount extends GearAI
	config(AI)
	native(AI);

var float LastMeleeCheckTime;

var GearPawn_LocustBloodMount MyBloodMountPawn;

// maximum distance to travel when roaming to player
var config float MaxRoamDist;
// outer distance for roaming envelope
var config float RoamEnvelopeOuter;
// inner distance for roaming envelope
var config float RoamEnvelopeInner;

var config float RoamWaitMax;
var config float RoamWaitMin;

// if we've been roaming for more than this amount of time, we'll give up the roam and go straight for charge
var config float OverallRoamDurationMax;
// gametime we started roaming
var float RoamStartTime;

var AIReactCond_EnemyCloseAndVisible ChargeCondition;

var bool bHelmetGone;


cpptext
{
	virtual void UpdatePawnRotation();
}

function SetEnableDeleteWhenStale(bool bAmIExpendable)
{
	local GearAI GAI;

	Super.SetEnableDeleteWhenStale(bAmIExpendable);

	if(MyBloodMountPawn != none && MyBloodMountPawn.Driver != none)
	{
		GAI = GearAI(MyBloodMountPawn.Driver.Controller);
		if(GAI != none)
		{
			GAI.SetEnableDeleteWhenStale(bAmIExpendable);
		}
	}
}

event Possess(Pawn Newpawn, bool bVehicleTransition )
{
	Super.Possess(Newpawn,bVehicleTransition);
	MyBloodMountPawn = GearPawn_LocustBloodMount(Pawn);
	
	// set the proper charge reaction distance
	ChargeCondition = new(self) class'AIReactCond_EnemyCloseAndVisible';
	ChargeCondition.AutoSubscribeChannels[1]='Hearing';
	ChargeCondition.AutoSubscribeChannels[2]='Force';
	ChargeCondition.DistanceThreshold = EnemyDistance_Short;
	ChargeCondition.OutputChannelName = 'EnemyCloseEnoughToCharge';
	ChargeCondition.MinTimeBetweenActivations = 4.0f;
	ChargeCondition.Initialize();

}

function BeginMeleeCommand( Pawn TargetPawn, optional coerce String Reason )
{
	`AILog(GetFuncName());
	if( TargetPawn != None )
	{
		ProcessStimulus( TargetPawn, PT_Force, 'BeginMeleeCommand' );
		SetEnemy( TargetPawn );
	}

	if(TimeSince(LastMeleeCheckTime) > 0.5f && FastTrace(MyGearPawn.Location,TargetPawn.Location))
	{
		LastMeleeCheckTime = WorldInfo.TimeSeconds;
		class'AICmd_Attack_BloodMountMelee'.static.InitCommand(self);
	}	
}

function DoMeleeAttack( optional Pawn NewEnemy )
{
	if (CommandList == None || CommandList.IsAllowedToFireWeapon())
	{
		if (NewEnemy != None)
		{
			SetEnemy(NewEnemy);
		}

		class'AICmd_Attack_BloodMountMelee'.static.InitCommand(self);
	}
}

function OnNewEnemy(Actor InInstigator, AIReactChannel OrigChannel)
{
	GearPawn_LocustBloodmount(MyGearPawn).PlayEnemyAcquiredSound();
}

function OnAIToggleCombat(SeqAct_AIToggleCombat Action)
{
	local GearPawn_LocustBloodMount bm;

	// pass the call along to our driver
	bm = GearPawn_LocustBloodMount(MyGearPawn);
	if(bm != none && bm.Driver != none)
	{
		bm.Driver.MyGearAI.OnAIToggleCombat(Action);
	}

	Super.OnAIToggleCombat(Action);

	MyBloodMountPawn.AvoidanceCylinder.SetEnabled( bAllowCombatTransitions );
}

function NotifyDriverDetached()
{
	Charge();
}

function NotifyHelmetBlownOff()
{
	Charge();
	bHelmetGone=true;
}

function bool ShouldCharge()
{
	if(bHelmetGone 
		|| MyBloodMountPawn.Driver == none
		|| !MyBloodMountPawn.Driver.IsAliveAndWell() 
		|| TimeSince(RoamStartTime) > OverallRoamDurationMax)
	{
		return TRUE;
	}

	return FALSE;
}

function ChargeWrapper(Actor EventInstigator, AIReactChannel OrigChannel)
{
	SetEnemy(Pawn(EventInstigator));
	Charge();
}

function Charge()
{
	//MessagePlayer("Charging");
	`AILog("Charging "@Enemy);
	class'AICmd_Attack_Bloodmount_Charge'.static.InitCommandUserActor(self,Enemy);
}

function HandleExecuteReactionAndPushExecuteCommand(Actor Inst, AIReactChannel OrigChan)
{
	class'AICmd_Attack_BloodMountMelee'.static.InitCommand(self);
}

function AddBasePathConstraints()
{
	Super.AddBasePathConstraints();
	class'Path_AvoidInEscapableNodes'.static.DontGetStuck( Pawn );
}

defaultproperties
{
	DefaultCommand=class'AICmd_Base_Bloodmount'

	DefaultReactConditions.Empty
	DefaultReactConditionClasses.Empty
	DefaultReactConditionClasses.Add(class'AIReactCond_HeadDamage')
	DefaultReactConditionClasses.Add(class'AIReactCond_BloodMountHeadDamage')
	// have to re-init the delegates here so they get the right function object
	// melee reaction
	Begin Object Name=EnemyInMeleeRangeReaction0
		OutputFunction=BeginMeleeCommandWrapper
	End Object
	DefaultReactConditions.Add(EnemyInMeleeRangeReaction0)

	// plays a sound whenever we have a new enemy
	Begin Object class=AIReactCond_GenericCallDelegate Name=SoundCaller0
		AutoSubscribeChannels(0)=NoticedEnemy
		OutputFunction=OnNewEnemy
	End Object
	DefaultReactConditions.Add(SoundCaller0)

	Begin Object class=AIReactCond_GenericCallDelegate Name=ChargeCallDelegate0
		AutoSubscribeChannels(0)=EnemyCloseEnoughToCharge
		OutputFunction=ChargeWrapper
	End Object
	DefaultReactConditions.Add(ChargeCallDelegate0)




	// execute enemy
	Begin Object Name=ExecutEnemyReaction0
		OutputFunction=HandleExecuteReactionAndPushExecuteCommand
	End Object
	DefaultReactConditions.Add(ExecutEnemyReaction0)

	bCanRevive=false
}