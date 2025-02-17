/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class GearAI_Boomer extends GearAI;

var bool bAimAtFeet;

var float NextFireTime;


event vector GetAimLocation(vector StartLoc, optional bool bActuallyFiring, optional Actor AimTarget )
{
	local Vector AimLoc;
	local Vector TestLocation;
	local CoverInfo EnemyCover;
	local GearPawn	E;

	if( AimTarget == None )
	{
		AimTarget = FireTarget;
	}

	AimLoc = Super.GetAimLocation(StartLoc, bActuallyFiring, AimTarget);

	// If enemy is not in cover
	E = GearPawn(AimTarget);
	if( E != None && bAimAtFeet )
	{
		EnemyCover = GetEnemyCover( E );
		if( EnemyCover.Link == None )
		{
			// Try shooting at enemy's feet
			TestLocation    = AimLoc;
			TestLocation.Z -= E.GetCollisionHeight() * 0.85f;

			if( CanSeeByPoints( StartLoc, TestLocation, Rotator(TestLocation - StartLoc) ) )
			{
				AimLoc = TestLocation;
			}
		}
	}

	return AimLoc;
}

function float GetFireDelay()
{
	`AIlog(GetFuncName()@self);
	MyGearPawn.TelegraphAttack();
	return 1.f;
}

function bool CanFireWeapon( Weapon Wpn, byte FireModeNum )
{
	local bool bSuper;
	bSuper = Super.CanFireWeapon(Wpn,FireModeNum);

	// always allow reloading
	if (FireModeNum == class'GearWeapon'.const.RELOAD_FIREMODE || MyGearPawn.MyGearWeapon.ShouldAutoReload())
	{
		return true;
	}

	// if we would have fired, call GetFireDelay first and see if we should wait
	if(bSuper)
	{

		`AILog(GetFuncName()@WorldInfo.TimeSeconds@NextFireTime@FireModeNum,'Weapon');
		if(WorldInfo.TimeSeconds < NextFireTime)
		{
			return false;
		}
		else if(NextFireTime == 0 && !(IsReloading() || IsSwitchingWeapons()))
		{		
			NextFireTime = WorldInfo.TimeSeconds + GetFireDelay();
			return false;
		}
		return true;
	}
	return false;
}

function NotifyWeaponFired( Weapon W, byte FireMode )
{
	Super.NotifyWeaponFired(W,FireMode);
	SetTimer(0.2f,FALSE,nameof(ResetNextFireTime));
}

function ResetNextFireTime()
{
	NextFireTime=0;
}

function bool ShouldAllowWalk()
{
	return false;
}

function AddBasePathConstraints()
{
	Super.AddBasePathConstraints();
	class'Path_AvoidInEscapableNodes'.static.DontGetStuck( Pawn );
}

function GoExecuteEnemy(GearPawn P);

defaultproperties
{
	DefaultCommand=class'AICmd_Base_Boomer'

	// Don't want reactions
	DefaultReactConditions.Empty
	DefaultReactConditionClasses.Empty
	// have to re-init the delegates here so they get the right function object
	// melee reaction
	Begin Object Name=EnemyInMeleeRangeReaction0
		OutputFunction=BeginMeleeCommandWrapper
	End Object
	DefaultReactConditions.Add(EnemyInMeleeRangeReaction0)

	// execute enemy
	Begin Object Name=ExecutEnemyReaction0
		OutputFunction=HandleExecuteReactionAndPushExecuteCommand
	End Object
	DefaultReactConditions.Add(ExecutEnemyReaction0)

	MaxStepAsideDist=196.f

	bAimAtFeet=TRUE
	bCanRevive=FALSE
	bCanExecute=FALSE

}
