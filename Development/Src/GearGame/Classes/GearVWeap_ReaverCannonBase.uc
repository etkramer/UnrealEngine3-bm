/**
 * GearVWeap_RocketCannon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearVWeap_ReaverCannonBase extends GearVehicleWeapon
	abstract
	config(Weapon); 

/** Min time between rockets firing */
var()	float	MinTimeBetweenRockets;


simulated function PlayWeaponAnimation(Name AnimName, float Rate, optional bool bLoop, optional SkeletalMeshComponent SkelMesh)
{
}


simulated function PlayShellCaseEject()
{
}


simulated function PlayMuzzleFlashEffect()
{
}

simulated function bool HasAmmo( byte FireModeNum, optional int Amount )
{
	return TRUE;
}

simulated function bool HasInfiniteSpareAmmo()
{
	return TRUE;
}

simulated function float GetFireInterval( byte FireModeNum )
{
	if( MinTimeBetweenRockets > 0.f ) 
	{
		return MinTimeBetweenRockets;
	}
	return Super.GetFireInterval( FireModeNum );
}


defaultproperties
{
	bWeaponCanBeReloaded=TRUE
	bIsSuppressive=TRUE

	WeaponFireTypes(0)=EWFT_Projectile
	InstantHitDamageTypes(0)=class'GDT_ReaverCannon'
	AmmoTypeClass=class'GearAmmoType_Boomer'

	FireOffset=(X=68,Y=0,Z=9)

	// Muzzle Flash point light
	Begin Object Name=WeaponMuzzleFlashLightComp
		LightColor=(B=35,G=185,R=255,A=255)
	End Object
	MuzzleFlashLight=WeaponMuzzleFlashLightComp
	MuzzleLightDuration=0.4f
	MuzzleLightPulseFreq=10
	MuzzleLightPulseExp=1.5

	//LC_EmisDefaultCOG=(R=0.5,G=3.0,B=20.0,A=1.0)
	//LC_EmisDefaultLocust=(R=3.0,G=3.0,B=3.0,A=1.0)

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
		Samples(0)=(LeftAmplitude=100,RightAmplitude=100,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.600)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1

	WeaponID=WC_Boomshot

	MaxFinalAimAdjustment=0.25
}
