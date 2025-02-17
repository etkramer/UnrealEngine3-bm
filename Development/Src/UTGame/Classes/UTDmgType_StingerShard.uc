/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTDmgType_StingerShard extends UTDamageType;

static function PawnTornOff(UTPawn DeadPawn)
{
	if ( !class'GameInfo'.Static.UseLowGore(DeadPawn.WorldInfo) )
	{
	 	class'UTProj_StingerShard'.static.CreateSpike(DeadPawn, DeadPawn.TakeHitLocation, DeadPawn.Normal(DeadPawn.TearOffMomentum));
	}
}

defaultproperties
{
	KillStatsName=KILLS_STINGER
	DeathStatsName=DEATHS_STINGER
	SuicideStatsName=SUICIDES_STINGER
	RewardCount=15
	RewardEvent=REWARD_BLUESTREAK
	RewardAnnouncementSwitch=9
	DamageWeaponClass=class'UTWeap_Stinger'
	DamageWeaponFireMode=1
	KDamageImpulse=1000
	KDeathUpKick=200
	bKRadialImpulse=true
	VehicleMomentumScaling=2.0
	bThrowRagdoll=true
	bNeverGibs=true
	bCausesBloodSplatterDecals=TRUE
	VehicleDamageScaling=+0.6
	NodeDamageScaling=+0.6
	CustomTauntIndex=9
}
