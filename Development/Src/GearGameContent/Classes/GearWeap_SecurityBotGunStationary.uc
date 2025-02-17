/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class  GearWeap_SecurityBotGunStationary extends GearWeap_SecurityBotGun;


auto state Inactive
{
	simulated function StartFire( byte FireModeNum )
	{
		GotoState('Active');
	}
}

DefaultProperties
{
	MuzzleSocketName=laser


	FireLoopCue=SoundCue'Weapon_SecurityBots.SecurityBots.SecurityBot_WeaponFireLoopCue'
	FireStopCue=SoundCue'Weapon_SecurityBots.SecurityBots.SecurityBot_WeaponFireStopCue'

	BarrelSpinningStopCue=SoundCue'Weapon_Troika.Weapons.Troika_AmmoFeederStopCue'

	Begin Object Class=ParticleSystemComponent Name=PSC_BarrelSmoke0
		Template=ParticleSystem'COG_GatlingGun.Effects.P_GatlingGun_Smoke'	
		bAutoActivate=FALSE
		TickGroup=TG_PostUpdateWork
	End Object
	PSC_BarrelSmoke=PSC_BarrelSmoke0
	CasingImpactCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingCasingImpactCue'

}