/**
 * GearVWeap_RideReaverCannon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearVWeap_RideReaverCannon extends GearVWeap_RideReaverCannonBase
	config(Weapon)
	hidedropdown;

defaultproperties
{
	WeaponProjectiles(0)=class'GearProj_RideReaverCannon'

//	FireSound=SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_RocketFireCue'
//	FireSound_Player=SoundCue'Locust_Reaver_Efforts.ReaverPlayer.ReaverPlayer_RocketFireCue'

	// muzzle flash emitter
	Begin Object Name=PSC_WeaponMuzzleMuzFlashComp
		Template=ParticleSystem'Locust_Boomshot.Effects.P_Boomshot_MF'
	End Object
	MuzFlashEmitter=PSC_WeaponMuzzleMuzFlashComp

	bIgnoreDifficultyDamageScale=TRUE

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

	WeaponIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=367,V=146,UL=143,VL=46)
	AnnexWeaponIcon=(U=0,V=144,UL=128,VL=39)
	CrosshairIcon=Texture2D'Warfare_HUD.HUD.HUD_Xhair_COG_Gorgon'
	HUDDrawData			= (DisplayCount=1,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=257,UL=36,VL=8),ULPerAmmo=36)
	HUDDrawDataSuper	= (DisplayCount=1,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=257,UL=36,VL=8),ULPerAmmo=36)

	//MeleeImpactSound=SoundCue'Weapon_Boomer.Reloads.BoomerHitCue'
}
