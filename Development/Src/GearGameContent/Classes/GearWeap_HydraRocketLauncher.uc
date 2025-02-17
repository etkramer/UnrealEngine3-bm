/**
 * Hydra Rocket Launcher
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_HydraRocketLauncher extends GearWeapon;

/** Set the actor we were fired at after its spawned */
simulated event Projectile ProjectileFireSimple( optional float AimErrorDeg )
{
	local Projectile Proj;
	local GearProj_HydraRocket Rocket;

	Proj = Super.ProjectileFireSimple(AimErrorDeg);
	Rocket = GearProj_HydraRocket(Proj);
	if(Rocket != None)
	{
		Rocket.TargetActor = DummyFireTargetActor;
		Rocket.SourceActor = DummyFireParent;

		// Save start location in source actor space
		Rocket.SourcePosActorSpace = (Rocket.Location - DummyFireParent.Location) << DummyFireParent.Rotation;
	}

	return Proj;
}

defaultproperties
{
	AIRating=1.5
	bWeaponCanBeReloaded=TRUE
	bIsSuppressive=TRUE

	WeaponProjectiles(0)=class'GearProj_HydraRocket'
	WeaponFireTypes(0)=EWFT_Projectile
	InstantHitDamageTypes(0)=class'GDT_HydraRocket'
	AmmoTypeClass=class'GearAmmoType_Boomer'

	FireSound=SoundCue'Weapon_Boomer.Firing.BoomerFireCue'

	WeaponDeEquipSound=SoundCue'Weapon_Shotgun.Actions.ShotgunRaiseCue'
	WeaponEquipSound=SoundCue'Weapon_Shotgun.Actions.ShotgunLowerCue'

	PickupSound=SoundCue'Weapon_Shotgun.Actions.ShotgunPickupCue'
	WeaponDropSound=SoundCue'Weapon_Sniper.Actions.CogSniperDropCue'

	// muzzle flash emitter
	Begin Object Class=ParticleSystemComponent Name=PSC_Muzzle0
		bAutoActivate=FALSE
		Template=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Gun_MuzzleFlash'
	End Object
	MuzFlashEmitter=PSC_Muzzle0

	FireOffset=(X=0,Y=0,Z=0)

	// Muzzle Flash point light
	Begin Object Name=WeaponMuzzleFlashLightComp
		LightColor=(B=35,G=185,R=255,A=255)
	End Object
	MuzzleFlashLight=WeaponMuzzleFlashLightComp
	MuzzleLightDuration=0.4f
	MuzzleLightPulseFreq=10
	MuzzleLightPulseExp=1.5

	DamageTypeClassForUI=class'GDT_HydraRocket'
	//WeaponIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=367,V=146,UL=143,VL=46)
	AnnexWeaponIcon=(U=0,V=144,UL=128,VL=39)
	//CrosshairIcon=Texture2D'Warfare_HUD.HUD.HUD_Xhair_COG_Boomshot'

	HUDDrawData			= (DisplayCount=1,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=257,UL=36,VL=8),ULPerAmmo=36)
	HUDDrawDataSuper	= (DisplayCount=1,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=257,UL=36,VL=8),ULPerAmmo=36)

	//MeleeImpactSound=SoundCue'Weapon_Boomer.Reloads.BoomerHitCue';

	LC_EmisDefaultCOG=(R=0.5,G=3.0,B=20.0,A=1.0)
	LC_EmisDefaultLocust=(R=3.0,G=3.0,B=3.0,A=1.0)

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
		Samples(0)=(LeftAmplitude=100,RightAmplitude=100,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.600)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1

	WeaponID=WC_ReaverRocket
}
