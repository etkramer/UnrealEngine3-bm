/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */ 
class GearAI_Brumak_Slave extends AIController
	native(AI)
	config(AI);

cpptext
{
	virtual UBOOL IsPlayerOwner()
	{
		return FALSE;
	}
}

/** GoW global macros */

var() GearPawn_LocustBrumakBase		Brumak;
var() GearAI_Brumak					BrumakAI;

/** Weapon is able to fire based on fire pattern */
var() bool	bAbleToFire;
/** LDs have assigned a fire target to this gun */
var() Actor	ScriptedFireTarget;

function GetBrumak()
{
	Brumak = GearPawn_LocustBrumakBase(Pawn.Base);
	BrumakAI = GearAI_Brumak(Brumak.Controller);

	//debug
	`log( self@GetFuncName()@Brumak@BrumakAI, bDebug );
}

function PawnDied( Pawn InPawn )
{
	super.PawnDied( InPawn );
	SetEnemy( None );
	Destroy();
}


function float GetTimerStatus();
function float RateEnemy( GearAI_Brumak AI, GearPawn EnemyWP );

function NotifyKilled( Controller Killer, Controller Killed, Pawn KilledPawn )
{
	//debug
	`log( GetGunName()@GetFuncName()@Killer@KilledPawn@Enemy, bDebug );

	if( KilledPawn == Enemy )
	{
		SelectEnemy();
	}

	if( KilledPawn == Brumak )
	{
		StopFiring();
		SetScriptedFireTarget( None );
	}

	//debug
	`log( "NewEnemy"@Enemy, bDebug );
}

function bool SelectEnemy()
{
	local GearPawn		EnemyWP, BestEnemy;
	local float			Rating, BestRating;

	if( BrumakAI != None && BrumakAI.Squad != None )
	{
		// Init
		BestRating	= -1.f;

		foreach BrumakAI.Squad.AllEnemies( class'GearPawn', EnemyWP )
		{
			if( BrumakAI.ProhibitedTargetList.Find( EnemyWP ) >= 0 )
				continue;

			Rating = RateEnemy( BrumakAI, EnemyWP );
			if( Rating > 0.f &&
				(BestRating <= 0.f || Rating > BestRating) )
			{
				BestEnemy	= EnemyWP;
				BestRating	= Rating;
			}
		}

		SetEnemy( BestEnemy );
	}
	else
	{
		SetEnemy( None );
	}

	return (Enemy != None);
}

function SetEnemy( Pawn NewEnemy )
{
	//debug
	`log( GetGunName()@GetFuncName()@NewEnemy, bDebug );

	Enemy = NewEnemy;
}

function SetScriptedFireTarget( Actor Targ, optional bool bShouldFire, optional float Duration )
{
	//debug
	`log( self@GetFuncName()@Targ@bShouldFire, bDebug );

	bAbleToFire = bShouldFire;
	ScriptedFireTarget = Targ;

	ClearTimer( 'ClearScriptedFireTarget' );
	if( bShouldFire && Duration > 0.f )
	{
		SetTimer( Duration, FALSE, nameof(ClearScriptedFireTarget) );
	}

	if( !bAbleToFire )
	{
		StopFiring();
	}
	else
	{
		GotoState( 'Active', 'Begin' );
	}
}

function ClearScriptedFireTarget()
{
	SetScriptedFireTarget( None );
	BrumakAI.ClearLatentAction( class'SeqAct_BrumakControl', FALSE );
}

function NotifyEnemyOutsideOfGunFOV()
{
	//debug
	`log( GetGunName()@GetFuncName(), bDebug );

	StopFiring();
	SetEnemy( None );
	ClearFirePattern();
}

function ClearFirePattern();

auto state Active
{
	function float GetRefireSleepTime()
	{
		local float Time;
		Time = GetTimerCount( 'RefireWeapon' ) * (0.5+0.5*FRand()) ;
		return Time;
	}

	function bool CanFire()
	{
		if( !bAbleToFire )
		{
			return FALSE;
		}

		if( Brumak == None || GetFireTarget() == None || !Brumak.IsWithinLeftGunFOV(GetFireTarget().Location) )
		{
			return FALSE;
		}

		if( Brumak.Physics == PHYS_Falling )
		{
			return FALSE;
		}

		return TRUE;
	}

Begin:
	if( Brumak == None || BrumakAI == None )
	{
		GetBrumak();
	}

	//debug
	`log( GetGunName()@GetStateName()@"BEGIN TAG"@IsTimerActive('RefireWeapon')@IsTimerActive('PollRefireWeapon')@bAbleToFire, bDebug );

	// If waiting to refire
	if( IsTimerActive('RefireWeapon') )
	{
		// Some where between half to full time left
		Sleep( GetRefireSleepTime() );
	}

	if( BrumakAI != None && !BrumakAI.bHijackedByLD )
	{
		//debug
		`log( GetGunName()@"Try to select enemy...", bDebug );

		// Select an enemy
		if( !SelectEnemy() )
		{
			//debug
			`log( GetGunName()@"Failed to find enemy", bDebug );

			StopFiring();
			Sleep( 1.f );
			Goto( 'End' );
		}

		//debug
		`log( GetGunName()@"Got enemy..."@Enemy, bDebug );

		// If weapon isn't already firing (or waiting to fire through RefireWeapon)
		if( Enemy == None ) 
		{ 
			Goto( 'End' ); 
		}
	}

	if( !CanFire() )
	{
		//debug
		`log( GetGunName()@"Unable to fire"@GetTimerStatus()@GetFireTarget()@Enemy, bDebug );

		StopFiring();
		Sleep( 0.25f );
		Goto( 'End' );
	}

	//debug
	`log( GetGunName()@GetStateName()@"StartFiring...", bDebug );

	StartFiring();
	// wait for the weapon to finish firing
	do
	{
		//debug
		`log( GetGunName()@GetStateName()@"Fire loop...", bDebug );

		Sleep( 0.25f );
	} until( !IsFiringWeapon() );

	if( !BrumakAI.bHijackedByLD )
	{
		bAbleToFire = FALSE; // Stop firing
	}

	//debug
	`log( GetGunName()@GetStateName()@"Done firing..."@bAbleToFire, bDebug );

End:
	//debug
	`log( GetGunName()@GetStateName()@"END TAG"@IsTimerActive('RefireWeapon')@bFire@Enemy@bAbleToFire, bDebug );

	Goto( 'Begin' );
}

function bool IsFiringWeapon()
{
	local GearWeapon Weap;

	Weap = GearWeapon(Pawn.Weapon);
	if( Weap != None )
	{
		return Weap.IsFiring();
	}
	return FALSE;
}

function StartFiring()
{
	bFire = 1;
}

/**
 * Stops the weapon from firing anymore.
 */
function StopFiring()
{
	bFire = 0;
}

function bool IsFireLineObstructed()
{
	return (BrumakAI == None || Pawn == None || (Enemy != None && !BrumakAI.CanFireAt(Enemy,Pawn.GetWeaponStartTraceLocation())));
}

function Rotator GetAdjustedAimFor( Weapon W, vector StartFireLoc )
{
	local Rotator AimRot;
	local Actor OldFT;
	
	if( BrumakAI != None )
	{
		OldFT = BrumakAI.FireTarget;
		BrumakAI.FireTarget = GetFireTarget();
		AimRot = BrumakAI.GetAdjustedAimFor( W, StartFireLoc );
		BrumakAI.FireTarget = OldFT;
	}

	return AimRot;
}


function Actor GetFireTarget()
{
	if( BrumakAI != None )
	{
		if( BrumakAI.bHijackedByLD )
		{
			return ScriptedFireTarget;
		}
		if( BrumakAI.TargetList.Length > 0 )
		{
			return BrumakAI.TargetList[Rand(BrumakAI.TargetList.Length)];
		}
	}

	if( Enemy != None )
	{
		return Enemy;
	}
	else
	if( SelectEnemy() )
	{
		return Enemy;
	}
	else
	if( BrumakAI.Enemy != None )
	{
		return BrumakAI.Enemy;
	}
	
	return None;	
}

state Silent
{
	function BeginState( Name PreviousStateName )
	{
		ClearTimer( 'SelectEnemy' );
		StopFiring();
		SetEnemy( None );
		bAbleToFire = FALSE;
	}
Begin:
	Stop;
}

function String GetGunName();

defaultproperties
{
//	bDebug=TRUE
}