/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_Boomer_Shield extends GearAI_Boomer
	config(AI);

/** ground speed to apply to pawn when the shield is deployed */
var config float			ShieldOpenGroundSpeed;

/** Next time AI is allowed to move */
var	float	AllowedMoveTime;
/** Next time AI is alloewd to switch enemies because of damage */
var float	NextEnemySwitchTime;
/** Amount of damage Boomer will take before pausing movement */
var int		PauseDamageThreshold;

var AIReactCond_DmgThreshold PauseThresh;

function float GetFireDelay()
{
	// overridden to skip telegraph.  pawns handles it for mauler
	return 0.f;
}

event Possess(Pawn NewPawn, bool bVehicleTransition)
{
	Super.Possess( NewPawn, bVehicleTransition );

	PauseThresh = new(self) class'AIReactCond_DmgThreshold';
	PauseThresh.DamageThreshold = PauseDamageThreshold;
	PauseThresh.OutputFunction = PauseFromDamage;
	PauseThresh.Initialize();
}

function PauseFromDamage(Actor EventInstigator, AIReactChannel OrigChannel)
{
	//debug
	`AILog( GetFuncName()@EventInstigator@ShouldCharge()@Enemy@IsShortRange(Enemy.Location)@GetHealthPercentage()@IsUnderHeavyFire()@GearPawn(Enemy).IsReloadingWeapon() );

	// If not within charge distance from enemy
	if( !ShouldCharge() )
	{
		// Update the next time we are free to move toward enemy
		AllowedMoveTime = WorldInfo.TimeSeconds + 3.f;

		// If shot by a different enemy and we haven't switched recently
		if( EventInstigator != Enemy && WorldInfo.TimeSeconds > NextEnemySwitchTime )
		{
			// Update the enemy and the thrash timer
			NextEnemySwitchTime = WorldInfo.TimeSeconds + 1.f;
			SetEnemy( Pawn(EventInstigator) );
		}

		OpenShield( TRUE );
		StopFiring();
	}
}

final function bool ShouldCharge()
{
	local GearPawn EnemyGP;

	// Charge if enemy is in close range
	if( IsShortRange(Enemy.Location) )
	{
		return TRUE;
	}

	// Charge if health is low
	if( GetHealthPercentage() < 0.25f )
	{
		return TRUE;
	}

	if( !IsUnderHeavyFire() )
	{
		EnemyGP = GearPawn(Enemy);
		return (EnemyGP != None && EnemyGP.IsReloadingWeapon());
	}

	return FALSE;
}

function bool ShouldRoadieRun()
{
	if( bShouldRoadieRun && !MyGearPawn.bIsWalking )
	{
		return TRUE;
	}
	return FALSE;
}

function float GetMoveTimeOutDuration(vector dest, bool bDontCare)
{
	return 2.f;
}

function OpenShield( optional bool bDoSpecialMove )
{
	MyGearPawn.OpenShield( TRUE, ShieldOpenGroundSpeed, bDoSpecialMove );
}

function CloseShield()
{
	MyGearPawn.CloseShield();
}


defaultproperties
{
	DefaultCommand=class'AICmd_Base_ShieldBoomer'
	AI_NearMissDistance=100.0
	bAimAtFeet=FALSE

	PauseDamageThreshold=40
}
