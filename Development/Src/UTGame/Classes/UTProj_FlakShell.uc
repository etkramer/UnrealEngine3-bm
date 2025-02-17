/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTProj_FlakShell extends UTProjectile;

simulated function Explode(vector HitLocation, vector HitNormal)
{
	local vector SpawnPos, BaseChunkDir;
	local actor HitActor;
	local rotator rot;
	local int i;
	local UTProj_FlakShard NewChunk;

	Super.Explode(HitLocation, HitNormal);

	SpawnPos = Location + 10 * HitNormal;

	HitActor = Trace(HitLocation, HitNormal, SpawnPos, Location, false);
	if (HitActor != None)
	{
		SpawnPos = HitLocation;
	}

	if ( (Role == ROLE_Authority) && (UTVehicle(ImpactedActor) == None) )
	{
		BaseChunkDir = Normal(HitNormal + 0.8 * Normal(Velocity));
		for (i = 0; i < 5; i++)
		{
			rot = rotator(4*BaseChunkDir + VRand());
			NewChunk = Spawn(class 'UTProj_FlakShard',, '', SpawnPos, rot);
			if (NewChunk != None)
			{
				NewChunk.bCheckShortRangeKill = false;
				NewChunk.Init(vector(rot));
			}
		}
	}
}

defaultproperties
{
	DamageRadius=+200.0
	speed=1200.000000
	Damage=100.000000
	MomentumTransfer=75000
	MyDamageType=class'UTDmgType_FlakShell'
	LifeSpan=6.0
	bCollideWorld=true
	TossZ=+305.0
	CheckRadius=40.0
    bRotationFollowsVelocity=true

	ProjFlightTemplate=ParticleSystem'WP_FlakCannon.Effects.P_WP_Flak_Alt_Smoke_Trail'
	ProjExplosionTemplate=ParticleSystem'WP_FlakCannon.Effects.P_WP_Flak_Alt_Explosion'
	ExplosionLightClass=class'UTGame.UTRocketExplosionLight'
	ExplosionDecal=MaterialInstanceTimeVarying'WP_FlakCannon.Decals.MITV_WP_FlakCannon_Impact_Decal01' 
	DecalWidth=128.0
	DecalHeight=128.0

	AmbientSound=SoundCue'A_Weapon_FlakCannon.Weapons.A_FlakCannon_FireAltInAirCue'
	ExplosionSound=SoundCue'A_Weapon_FlakCannon.Weapons.A_FlakCannon_FireAltImpactExplodeCue'

	Physics=PHYS_Falling
	CustomGravityScaling=1.0

	DrawScale=1.5

	bWaitForEffects=true
}
