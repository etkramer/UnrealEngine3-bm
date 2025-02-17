/**
 * "Untargeted" flavor of the Mortar Projectile, for mortar shots
 * from the hip.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_MortarUntargetedBase extends GearProj_ClusterMortarBase
	abstract
	config(Weapon);

var() protected const config float		TimeBeforeMortarCanExplode;

simulated function TriggerExplosion(vector HitLocation, vector HitNormal, Actor HitActor)
{
	local int Idx;
	local GearProjectile SpawnedBomblet;
	local vector NudgedHitLoc, BombletInitDir;

	// skipping the ClusterMortar.Explode(), so we can skip the special bomblet spawning in there.
	super(GearProj_ExplosiveBase).TriggerExplosion(HitLocation, HitNormal, HitActor);

	// different bomblet code here than for the regular mortar.  just toss them randomly, most will explode nearby
	if (Role == ROLE_Authority)
	{
//		FlushPersistentDebugLines();
		NudgedHitLoc = HitLocation + HitNormal * 16;

		for (Idx=0; Idx<NumBombletsToSpawn; ++Idx)	
		{
			SpawnedBomblet = Spawn(BombletClass, Instigator,, NudgedHitLoc);
			if (SpawnedBomblet != None)
			{
				// random, but skewed in the dir of the normal
				BombletInitDir = Normal(VRand() + HitNormal);

				SpawnedBomblet.Init(BombletInitDir);
				SpawnedBomblet.Velocity = BombletInitDir * BombletMaxSprayVel * RandRange(0.2f, 1.f);

//				DrawDebugLine(NudgedHitLoc, NudgedHitLoc + BombletInitDir * 256, 255, 255, 0, TRUE);

				SpawnedBomblet.bSuppressAudio = bSuppressAudio;
			}
		}
	}
}

simulated protected function AnticipateImpact()
{
	// do nothing
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	SetTimer( TimeBeforeMortarCanExplode, FALSE, nameof(EnableMortarExplosion) );
}

// nothing to do, really.  just to catch the timer.
simulated function EnableMortarExplosion();

simulated function ProcessTouch(Actor Other, Vector HitLocation, Vector HitNormal)
{
	local Pawn P;
	P = Pawn(Other);
	// bounce off friendly players for the initial launch
	if (P != None && P.GetTeamNum() == Instigator.GetTeamNum() && IsTimerActive('EnableMortarExplosion'))
	{
		// don't allow explosion for first bounce or N seconds
		Bounce(Location, HitNormal);
	}
	else
	{
		super.ProcessTouch(Other, HitLocation, HitNormal);
	}
}

simulated function HitWall(vector HitNormal, actor Wall, PrimitiveComponent WallComp)
{
	if (IsTimerActive('EnableMortarExplosion'))
	{
		// don't allow explosion for first bounce or N seconds
		Bounce(Location, HitNormal);
	}
	else
	{
		super.HitWall(HitNormal, Wall, WallComp);
	}
}

defaultproperties
{
//	bUpdateSimulatedPosition=TRUE
//	Speed=1400			// defined in the weapons ini, see MortarLaunchSpeed variable
//	TrailTemplate=None
//	Lifespan=10.f

 	Bounciness=0.8sf
	VelocityDampingFactor=0.85f

	BombletMaxSprayVel=1500

//	RandSpinRange=(X=300000,Y=500000)
//	BounceRandomness=0.9f

	bStoppedByGrenadeBlockingVolume=TRUE
}
