/**
 * Mortar Heavy weapon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_HeavyMortar extends GearWeap_HeavyMortarBase
	config(Weapon);

defaultproperties
{
	WeaponProjectiles(0)=class'GearProj_ClusterMortar'
	UntargetedProjectileClass=class'GearProj_MortarUntargeted'

	// Weapon anim set
	CustomAnimSets(0)=AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel_Heavy'
	CustomAnimSets(1)=AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel_Heavy_Mortar'
	AimOffsetProfileNames(0)="HeavyMiniGun"

	CrankSound=SoundCue'Weapon_Mortar.Sounds.MortarDeployCrankingUpLoopingCue'
	RotateLoopSound=SoundCue'Weapon_Mortar.Sounds.MortarDeployRotateLoopCue'
	RotAudioStartVelThreshold=2500.f
	RotAudioStopVelThreshold=100.f
	RotAudioVolumeVelRange=(X=5000.f,Y=15000.f)
	RotAudioVolumeRange=(X=0.25f,Y=1.f)

	FireSound=SoundCue'Weapon_Mortar.Sounds.MortarFireEnemyCue'
	FireSound_Player=SoundCue'Weapon_Mortar.Sounds.MortarFirePlayerCue'
	FireSoundUntargeted=SoundCue'Weapon_Mortar.Sounds.MortarFireEnemyWaistCue'
	FireSoundUntargeted_Player=SoundCue'Weapon_Mortar.Sounds.MortarFirePlayerWaistCue'

	WeaponEquipSound=None
	WeaponDeEquipSound=None

	PickupSound=SoundCue'Weapon_Mortar.Sounds.MortarPickupCue'
	WeaponDropSound=SoundCue'Weapon_Mortar.Sounds.MortarDropCue'

	// muzzle flash emitter
	Begin Object Name=PSC_WeaponMuzzleMuzFlashComp
		Template=ParticleSystem'COG_Mortar.Effects.P_Mortar_Muzzle_Effect'
	End Object

	// Weapon Mesh Transform
	Begin Object Name=WeaponMesh
		SkeletalMesh=SkeletalMesh'COG_Mortar.Mesh.COG_Mortar_SK'
		PhysicsAsset=PhysicsAsset'COG_Mortar.Mesh.COG_Mortar_SK_Physics'
		AnimTreeTemplate=AnimTree'COG_Mortar.COG_Mortar_AnimTree'
	    AnimSets(0)=AnimSet'COG_Mortar.Animations.COG_Mortar'
    End Object

	Begin Object Class=SkeletalMeshComponent Name=MortarShell0
		SkeletalMesh=SkeletalMesh'COG_Mortar.Mesh.Mortar_Shell'
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
		bAcceptsStaticDecals=FALSE
		bAcceptsDynamicDecals=FALSE
	End Object
	MortarShell=MortarShell0
	MagazineMesh=None

	DamageTypeClassForUI=class'GDT_Mortar'
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=0,V=49,UL=143,VL=46)
	AnnexWeaponIcon=(U=0,V=430,UL=128,VL=36)
	CrosshairIcon=Texture2D'Warfare_HUD.HUD.HUD_Xhair_COG_Mortar'

	HUDDrawData			= (DisplayCount=1,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=486,UL=106,VL=7),ULPerAmmo=106)
	HUDDrawDataSuper	= (DisplayCount=1,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=486,UL=106,VL=7),ULPerAmmo=106)

	MeleeImpactSound=None		// SoundCue'Weapon_Boomer.Reloads.BoomerHitCue';

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
		Samples(0)=(LeftAmplitude=100,RightAmplitude=100,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.600)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1

	PS_MountedImpact=ParticleSystem'COG_Mortar.Effects.P_Mortar_Plant'

	MortarCrankingClickSound=SoundCue'Weapon_Mortar.Sounds.MortarCrankingClickCue'
}
