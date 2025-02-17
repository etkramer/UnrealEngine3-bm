/**
 * Heavy Gatling Gun Weapon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_HeavyMiniGun extends GearWeap_HeavyMiniGunBase
	config(Weapon);

defaultproperties
{
	// Weapon anim set
	CustomAnimSets(0)=AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel_Heavy'
	CustomAnimSets(1)=AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel_Heavy_Gatling'
	AimOffsetProfileNames(0)="HeavyMiniGun"

	DamageTypeClassForUI=class'GDT_HeavyMiniGun'
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=0,V=0,UL=143,VL=46)
	AnnexWeaponIcon=(U=0,V=396,UL=128,VL=34)
	HUDDrawData=(DisplayCount=106,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=494,UL=106,VL=7),ULPerAmmo=0)

	// Weapon Mesh
	Begin Object Name=WeaponMesh
		SkeletalMesh=SkeletalMesh'COG_GatlingGun.Mesh.COG_GatlingGun'
		AnimTreeTemplate=AnimTree'COG_GatlingGun.GatlingGunAnimTree'
		PhysicsAsset=PhysicsAsset'COG_GatlingGun.Mesh.COG_GatlingGun_Physics'
		AnimSets(0)=AnimSet'COG_GatlingGun.Anims.COG_GatlingGun'
	End Object

	// MF flash
	Begin Object Name=PSC_WeaponMuzzleMuzFlashComp
		Template=ParticleSystem'COG_GatlingGun.Effects.P_GatlingGun_MF'
	End Object

	MuzSmokeParticleSystem=None		// above effect will handle it

	// shell case ejection emitter
	Begin Object Name=PSC_WeaponShellCaseComp
		Template=ParticleSystem'COG_GatlingGun.Effects.P_GatlingGun_Shell'
	End Object

	Begin Object Name=PSC_BarrelSmoke0
		Template=ParticleSystem'COG_GatlingGun.Effects.P_GatlingGun_Smoke'
	End Object

	// tracers
	TracerSmokeTrailEffect=ParticleSystem'COG_GatlingGun.Effects.P_GatlingGun_Tracer'
	TracerSmokeTrailEffectAR=ParticleSystem'COG_GatlingGun.Effects.P_GatlingGun_Tracer_AR'

	WeaponDropSound=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingDropCue'
	PickupSound=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingPickupCue'
	WeaponEquipSound=None
	WeaponDeEquipSound=None

	BarrelSpinningStartCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingBarrelStartCue'
	BarrelSpinningLoopCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingBarrelLoopCue'
	BarrelSpinningStopCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingBarrelStopCue'

	FireLoopCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingFireEnemyLoopCue'
	FireLoopCue_Player=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingFirePlayerLoopCue'
	FireStopCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingFireStopEnemyCue'
	FireStopCue_Player=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingFireStopPlayerCue'
	FireStopCue_Overheat=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingFireHeatStopCue'
	HeatBuildupCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingFireHeatBuildUpCue'
	CasingImpactCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingCasingImpactCue'

	ActiveCoolingStartCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingHeatSteamVentOpenCue'
	ActiveCoolingStopCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingHeatSteamVentCloseCue'
	ActiveCoolingLoopCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingHeatSteamLoop01Cue'

	RotateLoopSound=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingPlatformRotateCue'
	WeaponWhipSound=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingAmmoWhipCue'

	PS_MountedImpact=ParticleSystem'COG_GatlingGun.Effects.P_GatlingGun_Plant'
	MountedImpactSound=SoundCue'Ambient_NonLoop.AmbientNonLoop.MetalSlamMediumCue'		//@fixme placeholder

	Begin Object Name=ActiveCoolingSteam0
		Template=ParticleSystem'COG_GatlingGun.Effects.P_GatlingGun_Steam_Release'
	End Object
}




