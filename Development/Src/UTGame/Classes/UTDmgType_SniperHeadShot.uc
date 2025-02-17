/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTDmgType_SniperHeadShot extends UTDamageType
	abstract;

static function SpawnHitEffect(Pawn P, float Damage, vector Momentum, name BoneName, vector HitLocation)
{
	local UTPawn UTP;
	local name HeadBone;
	local UTEmit_HitEffect HitEffect;

	UTP = UTPawn(P);
	if (UTP != None && UTP.Mesh != None)
	{
		HeadBone = UTP.HeadBone;
	}
	HitEffect = P.Spawn(class'UTEmit_HeadShotBloodSpray',,, HitLocation, rotator(-Momentum));
	if (HitEffect != None)
	{
		HitEffect.AttachTo(P, HeadBone);
	}
}

static function int IncrementKills(UTPlayerReplicationInfo KillerPRI)
{
	if ( PlayerController(KillerPRI.Owner) != None )
	{
		PlayerController(KillerPRI.Owner).ReceiveLocalizedMessage( class'UTWeaponKillRewardMessage', 0 );
	}
	return super.IncrementKills(KillerPRI);
}

defaultproperties
{
	RewardCount=15
	RewardEvent=REWARD_HEADHUNTER
	RewardAnnouncementSwitch=2
	KillStatsName=KILLS_HEADSHOT
	DeathStatsName=DEATHS_HEADSHOT
	SuicideStatsName=SUICIDES_HEADSHOT
	DamageWeaponClass=class'UTWeap_SniperRifle'
	DamageWeaponFireMode=0
	bSeversHead=true
	bNeverGibs=true
	bIgnoreDriverDamageMult=true
	VehicleDamageScaling=0.4
	NodeDamageScaling=0.4
	DeathAnim=Death_Headshot
	CustomTauntIndex=3
}
