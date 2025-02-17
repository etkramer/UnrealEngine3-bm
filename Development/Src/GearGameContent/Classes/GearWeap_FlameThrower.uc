/**
 * Flamethrower!
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_FlameThrower extends GearWeap_FlameThrowerBase;

defaultproperties
{
	// ----------------------------------
	// GearWeap_FlamethrowerBase parameters
	// ----------------------------------

	FlameSprayClass=class'FlameThrowerSpray'

	Begin Object Name=FlameEndSpray0
		Template=ParticleSystem'COG_Flamethrower.Effects.P_FireWhip_End_Spit'
	End Object

	PilotLightLoopSound=SoundCue'Weapon_FlameThrower.FlameThrower.FlameThrower_PilotLoop01Cue'
	PilotLightIgniteSound=SoundCue'Weapon_FlameThrower.FlameThrower.FlameThrower_PilotStop01Cue'
	PilotLightExtinguishSound=SoundCue'Weapon_FlameThrower.FlameThrower.FlameThrower_PilotStart01Cue'

	FireNoAmmoSound=SoundCue'Weapon_FlameThrower.FlameThrower.FlameThrower_FireEmpty01Cue'
	WeaponEquipSound=SoundCue'Weapon_FlameThrower.FlameThrower.FlameThrower_Raise01Cue'
	WeaponDeEquipSound=SoundCue'Weapon_FlameThrower.FlameThrower.FlameThrower_Lower01Cue'

	PickupSound=SoundCue'Weapon_FlameThrower.FlameThrower.FlameThrower_Pickup01Cue'
	WeaponDropSound=SoundCue'Weapon_FlameThrower.FlameThrower.FlameThrower_Drop01Cue'

	MeleeImpactSound=SoundCue'Weapon_TorqueBow.Weapons.Crossbow_HitCue'

	Begin Object Name=PilotLight0
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Flame_Tip'
	End Object

	Begin Object Name=UnignitedFuel0
		Template=ParticleSystem'COG_Flamethrower.Effects.P_Flamethrower_Tip'
	End Object


	// --------------------------------
	// GearWeapon parameters
	// --------------------------------

	CustomAnimSets.Add(AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel_Scorcher')

	// Weapon Mesh
	Begin Object Name=WeaponMesh
		SkeletalMesh=SkeletalMesh'COG_FlameThrower.Mesh.COG_Scorcher_Skel'
		PhysicsAsset=PhysicsAsset'COG_FlameThrower.Mesh.COG_Scorcher_Skel_Physics'
		AnimSets(0)=AnimSet'COG_Flamethrower.Animations.COG_Flamethrower_Anims'
    End Object

	// no traditional muzzle flash, we'll deal with it ourselves
	MuzFlashEmitter=None
	MuzFlashParticleSystem=None
	MuzFlashParticleSystemActiveReload=None
	MuzSmokeParticleSystem=None

	bCanDisplayReloadTutorial=TRUE
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=0,V=193,UL=143,VL=46)
	AnnexWeaponIcon=(U=0,V=466,UL=128,VL=34)
	CrosshairIcon=Texture2D'Warfare_HUD.HUD.HUD_Xhair_COG_Gorgon'
	HUDDrawData			= (DisplayCount=106,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=494,UL=106,VL=7),ULPerAmmo=0)
	HUDDrawDataSuper	= (DisplayCount=106,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=494,UL=106,VL=7),ULPerAmmo=0)
}




