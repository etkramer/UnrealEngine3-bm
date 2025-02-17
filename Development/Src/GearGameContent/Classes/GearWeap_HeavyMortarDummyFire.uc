/**
 * Mortar Heavy weapon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_HeavyMortarDummyFire extends GearWeap_HeavyMortarBase
	config(Weapon);

defaultproperties
{
	WeaponProjectiles(0)=class'GearProj_ClusterMortar'
	UntargetedProjectileClass=class'GearProj_MortarUntargeted'

	// Weapon anim set
	CustomAnimSets(0)=none
	CustomAnimSets(1)=none
	AimOffsetProfileNames(0)=none

	CrankSound=none
	RotateLoopSound=none
	RotAudioStartVelThreshold=2500.f
	RotAudioStopVelThreshold=100.f
	RotAudioVolumeVelRange=(X=5000.f,Y=15000.f)
	RotAudioVolumeRange=(X=0.25f,Y=1.f)

	FireSound=none
	FireSound_Player=none
	FireSoundUntargeted=none
	FireSoundUntargeted_Player=none

	WeaponEquipSound=None
	WeaponDeEquipSound=None

	PickupSound=none
	WeaponDropSound=none

	// muzzle flash emitter
	Begin Object Name=PSC_WeaponMuzzleMuzFlashComp
		Template=none
	End Object

	// Weapon Mesh Transform
	Begin Object Name=WeaponMesh
		SkeletalMesh=none
		PhysicsAsset=none
		AnimTreeTemplate=none
	    AnimSets(0)=none
    End Object

	MagazineMesh=None

	DamageTypeClassForUI=class'GDT_Mortar'
	WeaponIcon=(Texture=none,U=0,V=49,UL=143,VL=46)
	AnnexWeaponIcon=(U=0,V=430,UL=128,VL=36)
	CrosshairIcon=none

	HUDDrawData			= (DisplayCount=1,AmmoIcon=(Texture=none,U=0,V=486,UL=106,VL=7),ULPerAmmo=106)
	HUDDrawDataSuper	= (DisplayCount=1,AmmoIcon=(Texture=none,U=0,V=486,UL=106,VL=7),ULPerAmmo=106)

	MeleeImpactSound=None		// SoundCue'Weapon_Boomer.Reloads.BoomerHitCue';

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
		Samples(0)=(LeftAmplitude=100,RightAmplitude=100,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.600)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1

	PS_MountedImpact=none

	MortarCrankingClickSound=none
}
