/**
 * UTDmgType_Rocket
 *
 *
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTDmgType_Rocket extends UTDmgType_Burning
	abstract;

/** SpawnHitEffect()
 * Possibly spawn a custom hit effect
 */
static function SpawnHitEffect(Pawn P, float Damage, vector Momentum, name BoneName, vector HitLocation)
{
	local UTEmit_VehicleHit BF;

	if ( Vehicle(P) != None )
	{
		BF = P.spawn(class'UTEmit_VehicleHit',P,, HitLocation, rotator(Momentum));
		BF.AttachTo(P, BoneName);
	}
	else
	{
		Super.SpawnHitEffect(P, Damage, Momentum, BoneName, HitLocation);
	}
}

defaultproperties
{
	KillStatsName=KILLS_ROCKETLAUNCHER
	DeathStatsName=DEATHS_ROCKETLAUNCHER
	SuicideStatsName=SUICIDES_ROCKETLAUNCHER
	RewardCount=15
	RewardEvent=REWARD_ROCKETSCIENTIST
	RewardAnnouncementSwitch=10
	DamageWeaponClass=class'UTWeap_RocketLauncher'
	DamageWeaponFireMode=0
	KDamageImpulse=1000
	KDeathUpKick=200
	bKRadialImpulse=true
	VehicleMomentumScaling=4.0
	VehicleDamageScaling=0.8
	NodeDamageScaling=1.1
	bThrowRagdoll=true
	GibPerterbation=0.15
    AlwaysGibDamageThreshold=99
    CustomTauntIndex=7
}
