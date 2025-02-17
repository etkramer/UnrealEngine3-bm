
/**
 * Locust Burst pistol
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_LocustBurstPistol extends GearWeap_LocustBurstPistolBase
	config(Weapon);

simulated function EjectClip()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Reloads.CogPistolClipEjectCue');
}
simulated function CockOpen()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Reloads.CogPistolCockOpenCue');
}
simulated function CockClose()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Reloads.CogPistolCockCloseCue');
}

simulated function InsertClip()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Reloads.CogPistolClipReloadInsertCue');
}

simulated function Jammed()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Actions.CogPistolDropCue');
}

simulated function HandSlam()
{
	WeaponPlaySound(SoundCue'Weapon_AssaultRifle.Reloads.CogRifleJammedHandCue');
}
simulated function ClipImpact()
{
	WeaponPlaySound(SoundCue'Weapon_Pistol.Reloads.CogPistolClipEjectImpactCue');
}

defaultproperties
{
	FireSound=SoundCue'Weapon_Pistol.Snub.LocustSnubFireEnemyCue'
	FireSound_Player=SoundCue'Weapon_Pistol.Snub.LocustSnubFirePlayerCue'
	FireNoAmmoSound=SoundCue'Weapon_Pistol.Firing.LocustPistolFireEmptyCue'
	
	WeaponWhipSound=SoundCue'Weapon_Pistol.Firing.LocustPistolWhipCue'

	WeaponReloadSound=None

	WeaponEquipSound=SoundCue'Weapon_Pistol.Actions.LocustPistolRaiseCue'
	WeaponDeEquipSound=SoundCue'Weapon_Pistol.Actions.LocustPistolLowerCue'
	PickupSound=SoundCue'Weapon_Pistol.Actions.LocustPistolPickupCue'
	WeaponDropSound=SoundCue'Weapon_Pistol.Actions.LocustPistolDropCue'

	// Weapon Mesh
	Begin Object Name=WeaponMesh
	    SkeletalMesh=SkeletalMesh'Locust_BurstPistol.Meshes.Locust_BurstPistol'
		PhysicsAsset=PhysicsAsset'Locust_BurstPistol.Meshes.Locust_BurstPistol_Physics'
	    AnimSets(0)=AnimSet'Locust_BurstPistol.Anims.Locust_BurstPistol_Animset'
		AnimTreeTemplate=AnimTree'Locust_BurstPistol.Anims.Locust_BurstPistol_AnimTree'
    End Object

	CustomAnimSets.Add(AnimSet'COG_MarcusFenix.Animations.ANimSetMarcus_Camskel_BurstPistol')

	Begin Object Class=StaticMeshComponent Name=MagazineMesh0
		StaticMesh=StaticMesh'Locust_BurstPistol.Meshes.Locust_BurstPistol_Magazine_SM'
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
		bAcceptsStaticDecals=FALSE
		bAcceptsDynamicDecals=FALSE
	End Object
	MagazineMesh=MagazineMesh0

	Begin Object Class=StaticMeshComponent Name=MagazineMesh2
		StaticMesh=StaticMesh'Locust_BurstPistol.Meshes.Locust_BurstPistol_Magazine_SM'
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
		bAcceptsStaticDecals=FALSE
		bAcceptsDynamicDecals=FALSE
	End Object
	MagazineMesh2=MagazineMesh2

	PSC_ShellEject=none // locust pistol doesn't eject shells!

	// muzzle flash emitter
	Begin Object Name=PSC_WeaponMuzzleMuzFlashComp
		Template=ParticleSystem'Locust_BurstPistol.Effects.P_BurstPistol_Muzzle_Flash'
	End Object
	MuzFlashEmitter=PSC_WeaponMuzzleMuzFlashComp

	// reload barrel smoke
	Begin Object Name=PSC_WeaponReloadBarrelSmokeComp
		Template=ParticleSystem'Locust_Pistol.EffectS.P_Geist_Pistol_Smoke'
	End Object
	PSC_ReloadBarrelSmoke=PSC_WeaponReloadBarrelSmokeComp

	MuzFlashParticleSystem=ParticleSystem'Locust_BurstPistol.Effects.P_BurstPistol_Muzzle_Flash'
	MuzFlashParticleSystemActiveReload=ParticleSystem'Locust_BurstPistol.Effects.P_BurstPistol_Muzzle_Flash_AR'

	DamageTypeClassForUI=class'GDT_LocustBurstPistol'
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=0,V=145,UL=143,VL=46)
	AnnexWeaponIcon=(U=128,V=28,UL=128,VL=40)
	CrosshairIcon=Texture2D'Warfare_HUD.HUD.HUD_Xhair_COG_Gorgon'
	HUDDrawData			= (DisplayCount=4,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=147,V=394,UL=106,VL=7),ULPerAmmo=26)
	HUDDrawDataSuper	= (DisplayCount=4,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=147,V=402,UL=106,VL=8),ULPerAmmo=26)

	MeleeImpactSound=SoundCue'Weapon_TorqueBow.Weapons.Crossbow_HitCue'

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
		Samples(0)=(LeftAmplitude=60,RightAmplitude=60,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.200)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1
}
