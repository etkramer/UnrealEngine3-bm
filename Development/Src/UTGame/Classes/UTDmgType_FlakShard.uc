/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTDmgType_FlakShard extends UTDamageType
	abstract;

defaultproperties
{
	RewardCount=15
	RewardEvent=REWARD_FLAKMASTER
	RewardAnnouncementSwitch=1
	KillStatsName=KILLS_FLAKCANNON
	DeathStatsName=DEATHS_FLAKCANNON
	SuicideStatsName=SUICIDES_FLAKCANNON
	DamageWeaponClass=class'UTWeap_FlakCannon'
	DamageWeaponFireMode=0
	KDamageImpulse=600
	VehicleMomentumScaling=0.65
	VehicleDamageScaling=0.8
	bBulletHit=True
	GibThreshold=-15
	MinAccumulateDamageThreshold=55
	AlwaysGibDamageThreshold=80
	CustomTauntIndex=6
}
