/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class UTDmgType_StingerBullet extends UTDamageType
	abstract;


defaultproperties
{
	KillStatsName=KILLS_STINGER
	DeathStatsName=DEATHS_STINGER
	SuicideStatsName=SUICIDES_STINGER
	RewardCount=15
	RewardEvent=REWARD_BLUESTREAK
	RewardAnnouncementSwitch=9
	DamageWeaponClass=class'UTWeap_Stinger'
	DamageWeaponFireMode=0
	KDamageImpulse=200
	VehicleDamageScaling=+0.6
	VehicleMomentumScaling=0.75
	NodeDamageScaling=0.6
	bBulletHit=True
	bNeverGibs=true
	DeathAnim=Death_Stinger
	DeathAnimRate=0.9
	bCausesBloodSplatterDecals=TRUE
	StopAnimAfterDamageInterval=0.5
	CustomTauntIndex=8
}
