/**
 * Brumak Main Gun Weapon for player driven Brumak
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_BrumakMainGun_Player extends GearWeap_BrumakMainGun;

simulated function bool IsHeatLimited()
{
	return TRUE;
}

defaultproperties
{
	TracerType=WTT_BrumakPlayer
	
	InstantHitDamageTypes(0)=class'GDT_BrumakBulletPlayer'
	InstantHitDamageTypes(ALTFIRE_FIREMODE)=class'GDT_BrumakCannonPlayer'
	WeaponProjectiles(ALTFIRE_FIREMODE)=class'GearProj_BrumakRocket_Player'

	Begin Object Name=PSC_Muzzle0
		Template=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Rideable_Gun_MuzzleFlash'
	End Object

	Begin Object Name=PSC_Muzzle1
		Template=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Rideable_Rocket_MuzzleFlash'
	End Object

	ActiveCoolingStartCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingHeatSteamVentOpenCue'
	ActiveCoolingStopCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingHeatSteamVentCloseCue'
	ActiveCoolingLoopCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingHeatSteamLoop01Cue'

	WeaponIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=146,V=0,UL=143,VL=46)
	HUDDrawData=(DisplayCount=106,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=494,UL=106,VL=7),ULPerAmmo=0)

	bKillDuringLevelTransition=FALSE
}
