/**
 * GearVWeap_RocketCannon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearVWeap_ReaverMinigun extends GearVWeap_ReaverMinigunBase
	config(Weapon);

defaultproperties
{
	ActiveCoolingStartCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingHeatSteamVentOpenCue'
	ActiveCoolingStopCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingHeatSteamVentCloseCue'
	ActiveCoolingLoopCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingHeatSteamLoop01Cue'

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

	WeaponIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=146,V=0,UL=143,VL=46)
	HUDDrawData=(DisplayCount=106,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=494,UL=106,VL=7),ULPerAmmo=0)
	AnnexWeaponIcon=(U=0,V=144,UL=128,VL=39)

	//HUDDrawData			= (DisplayCount=1,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=257,UL=36,VL=8),ULPerAmmo=36)
	//HUDDrawDataSuper	= (DisplayCount=1,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=257,UL=36,VL=8),ULPerAmmo=36)

	//MeleeImpactSound=SoundCue'Weapon_Boomer.Reloads.BoomerHitCue'
}
