/**
 * Troika Weapon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_MinigunTurret extends GearWeap_Troika
	hidedropdown;

/** How long the weapon has been firing. */
var protected transient float StartedFiringTime;

simulated function WeaponFired(byte FiringMode, optional vector HitLocation)
{
	if (FireLoopAC == None)
	{
		// just started firing, audio hasn't kicked in yet
		StartedFiringTime = WorldInfo.TimeSeconds;
	}
	super.WeaponFired(FiringMode, HitLocation);
}

simulated function WeaponStoppedFiring(byte FiringMode)
{
	super.WeaponStoppedFiring(FiringMode);

	if ( (WorldInfo.TimeSeconds - StartedFiringTime) > 2.f )
	{
		PSC_Reloading.ActivateSystem(TRUE);
	}
}

defaultproperties
{
	BarrelSpinningStartCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingBarrelStartCue'
	//BarrelSpinningLoopCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingBarrelLoopCue'				// can't hear it over the firing
	BarrelSpinningStopCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingBarrelStopCue'

	// muzzle flash
	Begin Object Name=PSC_Muzzle0
		Template=ParticleSystem'COG_Minigun.Effects.P_MiniGun_MF'
	End Object

	// after-firing smoke
	Begin Object Name=PSC_Reloading0
		Template=ParticleSystem'COG_Minigun.Effects.P_Minigun_Smoke'
	End Object

	InstantHitDamageTypes(0)=class'GDT_MinigunTurret'

	DamageTypeClassForUI=class'GDT_MinigunTurret'
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=146,V=47,UL=143,VL=46)
	HUDDrawData=(DisplayCount=106,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=494,UL=106,VL=7),ULPerAmmo=0)



}
