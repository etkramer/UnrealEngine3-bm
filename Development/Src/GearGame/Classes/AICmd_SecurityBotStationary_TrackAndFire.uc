/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class AICmd_SecurityBotStationary_TrackAndFire extends AICmd_Base_SecurityBotStationary;

/** GoW global macros */

function Pushed()
{
	Super.Pushed();
	SavedDesiredRot = MyBotPawn.DesiredTurretRot;
	SetTurretRotation(GetRotation());
	Pawn.AllowedYawError = 182.f; // one degree

	MyBotPawn.SetTurretRotationRate(MyBotPawn.TargetTrackingSpeed);	
	MyBotPawn.GetPissed();
	GotoState('InCombat');
}

function Popped()
{
	Super.Popped();
	SetDefaultRotationRate();
	MyBotPawn.GetHappy();
}

state InCombat
{	
Begin:	
	if(!SelectTarget())
	{
		`AILog("Nothing to do! FireTarget:"@FireTarget);
		GotoState('DelayFailure');
	}

	FinishRotation();
	while(HasValidEnemy() || FireTarget != none)
	{
		StartFiring();
		SetTurretRotation(GetRotation());
		FinishRotation();
		Sleep(0.25);
	}
	Sleep(MyBotPawn.StopFireDelay);
	StopFiring();
	GotoState('DelaySuccess');

};