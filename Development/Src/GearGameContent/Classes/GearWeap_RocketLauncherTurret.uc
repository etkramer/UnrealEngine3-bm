
/**
 * Rocket Launcher Turret Weapon
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_RocketLauncherTurret extends GearWeap_MinigunTurret;

var() const config	float RocketRateOfFire;

simulated function GetProjectileFirePosition(out vector out_ProjLoc, out vector out_ProjDir)
{
	local Vector StartTrace;

	// This is where we would start an instant trace. (what CalcWeaponFire uses)
	StartTrace	= Instigator.GetWeaponStartTraceLocation();
	out_ProjDir	= Vector(GetAdjustedAim( StartTrace ));

	// this is the location where the projectile is spawned.
	out_ProjLoc	= GetPhysicalFireStartLoc(out_ProjDir);
}

simulated function float GetRateOfFire()
{
	if( CurrentFireMode == ALTFIRE_FIREMODE )
	{
		return RocketRateOfFire;
	}
	return Super.GetRateOfFire();
}

simulated function float GetPlayerAimError()
{
	if( CurrentFireMode == ALTFIRE_FIREMODE )
	{
		return 0.f;
	}

	return Super.GetPlayerAimError();
}
 
defaultproperties
{
	FiringStatesArray(ALTFIRE_FIREMODE)="WeaponFiring"
	WeaponProjectiles(ALTFIRE_FIREMODE)=class'GearProj_RocketLauncherTurret'
	WeaponFireTypes(ALTFIRE_FIREMODE)=EWFT_Projectile
	InstantHitDamageTypes(ALTFIRE_FIREMODE)=class'GDT_ReaverCannon'
	AmmoTypeClass=class'GearAmmoType_Boomer'

	WeaponID=WC_RocketLauncherTurret
	bKillDuringLevelTransition=TRUE


	FireLoopCue=None
	FireLoopCue_Player=None
	FireStopCue=None
	FireStopCue_Player=None
	BarrelSpinningStartCue=None
	BarrelSpinningStopCue=None

	bAltFireWeapon=TRUE
}
