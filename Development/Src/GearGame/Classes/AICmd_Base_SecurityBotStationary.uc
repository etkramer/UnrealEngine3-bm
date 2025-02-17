/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class AICmd_Base_SecurityBotStationary extends AICommand_Base_Combat
	within GearAI_SecurityBotStationary;

/** GoW global macros */

var Rotator SavedDesiredRot;
var() float TimeoutTime;

static function bool InitCommandUserActor( GearAI AI, Actor UserActor )
{
	local Pawn P;
	P = Pawn(UserActor);
	if(P != none)
	{		
		AI.SetEnemy(P);
		AI.FireTarget=P;
	}
	return Super.InitCommandUserActor(AI,UserActor);
}
function Pushed()
{
	Super.Pushed();
	SavedDesiredRot = MyBotPawn.DesiredTurretRot;
	MyBotPawn.AllowedYawError = 50;
	SetTurretRotation(rotator(GetTargetLocation() - GetAimCalcLoc()));
	`AILog(self@outer@"Enemy:"@Enemy);
	ReactionManager.SuppressReactionsByType(class'AIReactCond_EnemyCloseToFireLine',false);
	GearEnemy = GearPawn(Enemy);
	MyBotPawn.GetPissed();
	GotoState('InCombat');
}

function Popped()
{
	Super.Popped();
	SetTurretRotation(SavedDesiredRot);
	SetDefaultRotationRate();
	MyBotPawn.AllowedYawError = MyBotPawn.default.AllowedYawError;
	ReactionManager.UnSuppressReactionsByType(class'AIReactCond_EnemyCloseToFireLine',false);
	MyBotPawn.GetHappy();
	GearEnemy=none;
}

function SetTargetTrackingRotationRate()
{
	MyBotPawn.SetTurretRotationRate(MyBotPawn.TargetTrackingSpeed);	
}

function Rotator GetAdjustedAimFor(Weapon W, vector StartFireLoc)
{
	local Rotator	Rot;

	Rot = Pawn.GetBaseAimRotation();
	// base aim error
	Rot.Pitch += RandRange(-MyBotPawn.AccuracyConeDegrees*0.5f,MyBotPawn.AccuracyConeDegrees*0.5f);
	Rot.Yaw	 += RandRange(-MyBotPawn.AccuracyConeDegrees,MyBotPawn.AccuracyConeDegrees);

	return Rot;
}

function vector GetAimCalcLoc()
{
	return MyBotPawn.GetPhysicalFireStartLoc(vect(0,0,0));
}

function bool NotifyEnemyBecameVisible( Pawn VisibleEnemy )
{
	if( ChildCommand != None )
	{
		return ChildCommand.NotifyEnemyBecameVisible( VisibleEnemy );
	}

	GOtoState('InCombat','StartFiring');
	return TRUE;
}

state InCombat
{	
	function bool IsBreakingBeam()
	{
		local vector ClosestPt;
		local vector TargetLoc;
		local vector AimCalcLoc;

		TargetLoc = GetTargetLocation();
		AimCalcLoc = GetAimCalcLoc();
		//`log(GetFuncName()@VSizeSq(GetTargetLocation() - GetAimCalcLoc())@MyBotPawn.DetectionRange@MyBotPawn.DetectionRange * MyBotPawn.DetectionRange);
		if( VSizeSq( TargetLoc - AimCalcLoc) > MyBotPawn.DetectionRange * MyBotPawn.DetectionRange)
		{
			return false;
		}

		//`log(GetFuncName()@PointDistToLine(GetTargetLocation(),vector(Pawn.GetBaseAimRotation()),GetAimCalcLoc()));
		//DrawDebugLine(GetTargetLocation(),GetAimCalcLoc(),255,0,0);

		PointDistToLine(TargetLoc,vector(Pawn.GetBaseAimRotation()),GetAimCalcLoc(),ClosestPt);
		if(  VSize2D(ClosestPt - TargetLoc) > 40.f )
		{
			return false;
		}

		return true;
	}

Begin:
	if(!HasValidEnemy())
	{
		GotoState('DelayFailure');
	}
	ProcessStimulus(Enemy,PT_Force,'AICmd_Base_SecurityBotStationary');
	Sleep(MyBotPawn.StartFireDelay);

StartFiring:	
	if(IsBreakingBeam())
	{
		SetTargetTrackingRotationRate();
	}
	TimeoutTime = WorldInfo.TimeSeconds + MyBotPawn.ResetDelay;
	SetTurretRotation(GetRotation());
	FinishRotation();
	while(HasValidEnemy() && GearEnemy.IsAliveAndWell() && TimeSince(Squad.GetEnemyLastSeenTime(Enemy)) < 1.0f && Worldinfo.TimeSeconds < TimeoutTime)
	{
		SetTurretRotation(GetRotation());
		FinishRotation();
		// if he's in the beam, reset the timer, and fire
		if( IsBreakingBeam() )
		{
			TimeoutTime = WorldInfo.TimeSeconds + MyBotPawn.ResetDelay;
			SetTargetTrackingRotationRate();
			StartFiring();
		}
		Sleep(0.2);
	}
	SetTurretRotation(GetRotation());
	TimeoutTime = WorldInfo.TimeSeconds + MyBotPawn.StopFireDelay;
	while(WorldInfo.TimeSeconds < TimeoutTime)
	{
		if(TimeSince(Squad.GetEnemyLastSeenTime(Enemy)) < 1.0f)
		{
			Goto('StartFiring');
		}
		Sleep(0.15f);
	}
	StopFiring();
	GotoState('DelaySuccess');
	
};	


DefaultProperties
{
}