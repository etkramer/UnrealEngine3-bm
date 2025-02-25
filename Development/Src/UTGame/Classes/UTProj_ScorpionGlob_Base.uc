/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTProj_ScorpionGlob_Base extends UTProjectile
	abstract
	native;

/** Used in TickSpecial() manta seeking code */
var float LastTraceTime;

var pawn SeekTarget;

var float SeekRangeSq;

/** Acceleration while homing in on hover vehicle */
var float SeekAccel;

/** used to notify AI of bio on the ground */
var UTAvoidMarker FearSpot;

cpptext
{
	virtual void TickSpecial(float DeltaTime);
}

replication
{
	if ( bNetDirty )
		SeekTarget;
}

simulated event Destroyed()
{
	Super.Destroyed();

	if (FearSpot != None)
	{
		FearSpot.Destroy();
	}
}

simulated function Shutdown()
{
	super.ShutDown();

		if (FearSpot != None)
	{
		FearSpot.Destroy();
	}
}

simulated event PhysicsVolumeChange(PhysicsVolume NewVolume)
{
	if ( PhysicsVolume.bWaterVolume && !NewVolume.bWaterVolume )
	{
		SetTimer(5.0, false, 'Explode');
		Buoyancy = 0.5 * (Buoyancy + 1.08);

		// spawn marker so AI can avoid
		if (FearSpot == None && WorldInfo.Game != None && WorldInfo.Game.NumBots > 0)
		{
			FearSpot = Spawn(class'UTAvoidMarker');
		}
	}
}

/**
 * Explode this glob
 */
simulated function Explode(Vector HitLocation, vector HitNormal)
{
	super.Explode(HitLocation, HitNormal);
	if (FearSpot != None)
	{
		FearSpot.Destroy();
	}
}


defaultproperties
{
	Speed=4000.0
	MaxSpeed=4000.0
	Damage=80.0
	DamageRadius=220.0
	SeekRangeSq=250000.0
	SeekAccel=6000.0
	MomentumTransfer=40000
	LifeSpan=1.6
	MaxEffectDistance=7000.0
	Buoyancy=1.5
	TossZ=0.0
	CheckRadius=36.0
	Physics=PHYS_Falling
	ExplosionLightClass=class'UTGame.UTRocketExplosionLight'
	ExplosionDecal=MaterialInstanceTimeVarying'WP_RocketLauncher.Decals.MITV_WP_RocketLauncher_Impact_Decal01'
	DecalWidth=128.0
	DecalHeight=128.0
	
	bNetTemporary=false
	bUpdateSimulatedPosition=false
}
