/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTDmgType_Enforcer extends UTDamageType
	abstract;

static function SpawnHitEffect(Pawn P, float Damage, vector Momentum, name BoneName, vector HitLocation)
{
	Super.SpawnHitEffect(P,Damage,Momentum,BoneName,HitLocation);
	if(UTPawn(P) != none)
	{
		UTPawn(P).SoundGroupClass.Static.PlayBulletImpact(P);
	}
}

defaultproperties
{
	KillStatsName=KILLS_ENFORCER
	DeathStatsName=DEATHS_ENFORCER
	SuicideStatsName=SUICIDES_ENFORCER

	RewardCount=15
	RewardEvent=REWARD_GUNSLINGER
	RewardAnnouncementSwitch=7
	DamageWeaponClass=class'UTWeap_Enforcer'
	DamageWeaponFireMode=2
    VehicleDamageScaling=0.33
	NodeDamageScaling=0.5
	VehicleMomentumScaling=0.0
    bBulletHit=True
	KDamageImpulse=200
	bNeverGibs=true
	bCausesBloodSplatterDecals=TRUE
	CustomTauntIndex=10
}
