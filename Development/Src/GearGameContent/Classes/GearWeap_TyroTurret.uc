
/**
 * Tyro Pillar Turret Cannon
 *
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_TyroTurret extends GearWeapon
	deprecated
	hidedropdown;


/** this weapon has no team based colors **/
simulated function SetMaterialBasedOnTeam( MeshComponent TheMesh, int TeamNum );

defaultproperties
{
	FireOffset=(X=-80,Y=3,Z=13)
	WeaponID=WC_TyroTurrent

	FireSound=none

	InstantHitDamageTypes(0)=class'GDT_Explosive'

	WeaponProjectiles(0)=class'GearProj_TyroRocket'
	WeaponFireTypes(0)=EWFT_Projectile

	// Muzzle Flash point light
    Begin Object Name=WeaponMuzzleFlashLightComp
		Brightness=3
	    LightColor=(R=64,G=160,B=255,A=255)
		Translation=(X=-100,Y=0,Z=20)
    End Object
    MuzzleFlashLight=WeaponMuzzleFlashLightComp

	bKillDuringLevelTransition=TRUE
}
