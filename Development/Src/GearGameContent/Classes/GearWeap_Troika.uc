/**
 * Troika Weapon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_Troika extends GearWeap_TroikaBase
	config(Weapon);

defaultproperties
{
	WeaponWhipSound=SoundCue'Weapon_AssaultRifle.Firing.CogRifleWhipCue'

	// shell case ejection emitter
	Begin Object Name=PSC_Muzzle0
		Template=ParticleSystem'Locust_TroikaCabal.Effects.P_TroikaCabal_MF'
	End Object

	Begin Object Name=PSC_Reloading0
		Template=ParticleSystem'Locust_TroikaCabal.Effects.P_TroikaCabal_MF_Smoke'
	End Object

	FireLoopCue=SoundCue'Weapon_Troika.TroikaV2.Troika_FireEnemyLoopCue'
	FireLoopCue_Player=SoundCue'Weapon_Troika.TroikaV2.Troika_FirePlayerLoopCue'
	FireStopCue=SoundCue'Weapon_Troika.TroikaV2.Troika_FireStopEnemyCue'
	FireStopCue_Player=SoundCue'Weapon_Troika.TroikaV2.Troika_FireStopPlayerCue'

	BarrelSpinningStartCue=SoundCue'Weapon_Troika.Weapons.Troika_AmmoFeederStartCue'
	//BarrelSpinningLoopCue=SoundCue'Weapon_Troika.Weapons.Troika_AmmoFeederLoopCue'			// can't hear this, save the memory
	BarrelSpinningStopCue=SoundCue'Weapon_Troika.Weapons.Troika_AmmoFeederStopCue'

	Begin Object Class=ForceFeedbackWaveform Name=TroikaFiringWaveform0
		Samples(0)=(LeftAmplitude=40,RightAmplitude=40,LeftFunction=WF_Constant,RightFunction=WF_Constant,Duration=0.100)
	End Object
	WeaponFireWaveForm=TroikaFiringWaveform0

	FireStopCue_Overheat=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingFireHeatStopCue'
	HeatBuildupCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingFireHeatBuildUpCue'
	CasingImpactCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingCasingImpactCue'

	ActiveCoolingStartCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingHeatSteamVentOpenCue'
	ActiveCoolingStopCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingHeatSteamVentCloseCue'
	ActiveCoolingLoopCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingHeatSteamLoop01Cue'

	Begin Object Name=ActiveCoolingSteam0
		Template=ParticleSystem'COG_GatlingGun.Effects.P_GatlingGun_Steam_Release'
	End Object

	Begin Object Name=PSC_BarrelSmoke0
		Template=ParticleSystem'COG_GatlingGun.Effects.P_GatlingGun_Smoke'
	End Object

	DamageTypeClassForUI=class'GDT_Troika'
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=146,V=0,UL=143,VL=46)
	HUDDrawData=(DisplayCount=106,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=494,UL=106,VL=7),ULPerAmmo=0)
}
