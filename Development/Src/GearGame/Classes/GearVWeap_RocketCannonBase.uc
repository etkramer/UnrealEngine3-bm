/**
 * GearVWeap_RocketCannon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearVWeap_RocketCannonBase extends GearVehicleWeapon
	abstract
	hidedropdown
	config(Weapon);

/** the fire rate after a 'super sweet' active reload */
var config float AR_SuperSweetRateOfFire;
/** the fire rate after a 'sweet, not so super' active reload */
var config float AR_SweetRateOfFire;

var float MyWeaponRateOfFire;

var Texture2D	DriveCrosshairIcon;
var Texture2D	ReverseCrosshairIcon;

var float		DriveCrosshairYOffset;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	MyWeaponRateOfFire = WeaponRateOfFire;
}

simulated function SetActiveReloadBonusActive(bool bSetSuperSweetSpotReward)
{
	Super.SetActiveReloadBonusActive(bSetSuperSweetSpotReward);

	if(bSetSuperSweetSpotReward)
	{
		MyWeaponRateOfFire = AR_SuperSweetRateOfFire;
	}
	else
	{
		MyWeaponRateOfFire = AR_SweetRateOfFire;
	}
}

simulated function TurnOffActiveReloadBonus()
{
	Super.TurnOffActiveReloadBonus();
	MyWeaponRateOfFire = WeaponRateOfFire;
}

simulated function float GetRateOfFire()
{
	return MyWeaponRateOfFire;
}

simulated function PlayWeaponAnimation(Name AnimName, float Rate, optional bool bLoop, optional SkeletalMeshComponent SkelMesh)
{
}


simulated function PlayShellCaseEject()
{
}


simulated function PlayMuzzleFlashEffect()
{
}


simulated function bool HasInfiniteSpareAmmo()
{
	return TRUE;
}

/** For centaur with 2 humans in, we give a different icon for the driver. Note that only the host can ever drive! */
simulated function Texture2D GetCrosshairIcon(GearPC PC, out float YOffset)
{
	local Vehicle_Centaur_Base Centaur;
	local bool bHumanGunner;

	Centaur = Vehicle_Centaur_Base(Instigator);

	// If not the main driver, always use gun icon
	if(Centaur == None)
	{
		return CrosshairIcon;
	}

	// See if its a human in the passenger seat
	bHumanGunner = ((Centaur.Seats.length == 2) && (Centaur.Seats[1].SeatPawn != None) && (GearPC(Centaur.Seats[1].SeatPawn.Controller) != None));

	// If it is a human just driving, use drive crosshair, but we choose a different one if we will reverse
	if(bHumanGunner)
	{
		YOffset = DriveCrosshairYOffset;

		if(Centaur.bLookSteerFacingReverse)
		{
			return ReverseCrosshairIcon;
		}
		else
		{
			return DriveCrosshairIcon;
		}
	}
	else
	{
		return CrosshairIcon;
	}
}

defaultproperties
{
	bWeaponCanBeReloaded=TRUE
	bIsSuppressive=TRUE

	WeaponFireTypes(0)=EWFT_Projectile
	InstantHitDamageTypes(0)=class'GDT_RocketCannon'
	AmmoTypeClass=class'GearAmmoType_Boomer'

	WeaponReloadSound=SoundCue'Vehicle_Centaur.Centaur.Centaur_ReloadSequence01Cue'

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

	WeaponID=WC_CentaurCannon

	MaxFinalAimAdjustment=0.25
}
