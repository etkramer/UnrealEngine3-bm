/**
 * GearVWeap_RocketCannon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearVWeap_RocketCannon extends GearVWeap_RocketCannonBase
	hidedropdown
	config(Weapon);


defaultproperties
{
	WeaponProjectiles(0)=class'GearProj_RocketCannon'

	WeaponReloadSound=SoundCue'Vehicle_Centaur.Centaur.Centaur_ReloadSequence01Cue'

	//FireSound=SoundCue'Weapon_Boomer.Firing.BoomerFireCue'

	// muzzle flash emitter
	//Begin Object Name=PSC_WeaponMuzzleMuzFlashComp
	//	Template=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_MuzzleFlash'
	//End Object
	//MuzFlashEmitter=PSC_WeaponMuzzleMuzFlashComp

	// Weapon Mesh Transform
	/*
	Begin Object Name=WeaponMesh
		SkeletalMesh=SkeletalMesh'Locust_Boomshot.Mesh.Locust_Boomshot'
		AnimTreeTemplate=AnimTree'Locust_Boomshot.AT_Boomshot'
		PhysicsAsset=PhysicsAsset'Locust_Boomshot.Mesh.Locust_Boomshot_Physics'
		AnimSets(0)=AnimSet'Locust_Boomshot.Animation.Animset_Locust_Boomshot'
		Rotation=(Pitch=1092)	//+6d
		Translation=(X=+8)
		Scale=1.2f
	End Object
	Mesh=WeaponMesh
	*/

	WeaponIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=146,V=145,UL=413,VL=46)
	AnnexWeaponIcon=(U=0,V=144,UL=128,VL=39)

	CrosshairIcon=Texture2D'Warfare_HUD.WarfareHUD_XHairs_assaultrifle'
	DriveCrosshairIcon=Texture2D'Warfare_HUD.HUD_Xhair_COG_Centaur_fwd'
	ReverseCrosshairIcon=Texture2D'Warfare_HUD.HUD_Xhair_COG_Centaur_rev'
	DriveCrosshairYOffset=0.2

	HUDDrawData			= (DisplayCount=6,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=281,UL=106,VL=7),ULPerAmmo=17)
	HUDDrawDataSuper	= (DisplayCount=6,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=281,UL=106,VL=7),ULPerAmmo=17)

	//MeleeImpactSound=SoundCue'Weapon_Boomer.Reloads.BoomerHitCue'
}
