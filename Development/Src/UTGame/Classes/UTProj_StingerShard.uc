/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTProj_StingerShard extends UTProjectile;

/** We need a pointer to our mesh component so we can switch collision when we create the constraint */
var StaticMeshComponent MyMesh;

/** This holds a link to our Constraint Actor */
var RB_ConstraintActor ShardVictimConstraint;

/** This is true if this projectile is currently spiking a victim */
var bool bSpiked;

/** Used to keep track of what actor to base ourselves on */
var Actor BaseActor;

var vector OldLocation;

var() float SpikeVictimVelFactor;

/**
 * Insure that our constraint is destroyed when we are destroyed
 */
simulated event Destroyed()
{
	if (ShardVictimConstraint != none)
	{
		ShardVictimConstraint.Destroy();
	}

	Super.Destroyed();
}

static function bool FindNearestBone(UTPawn DeadPawn, vector InitialHitLocation, out name BestBone, out vector BestHitLocation)
{
	local int i, dist, BestDist;
	local vector BoneLoc;
	local name BoneName;

	if (DeadPawn.Mesh.PhysicsAsset != none)
	{
		for (i=0;i<DeadPawn.Mesh.PhysicsAsset.BodySetup.Length;i++)
		{
			BoneName = DeadPawn.Mesh.PhysicsAsset.BodySetup[i].BoneName;
			// If name is not empty and bone exists in this mesh
			if ( BoneName != '' && DeadPawn.Mesh.MatchRefBone(BoneName) != INDEX_NONE)
			{
				BoneLoc = DeadPawn.Mesh.GetBoneLocation(BoneName);
				Dist = VSize(InitialHitLocation - BoneLoc);
				if ( i==0 || Dist < BestDist )
				{
					BestDist = Dist;
					BestBone = DeadPawn.Mesh.PhysicsAsset.BodySetup[i].BoneName;
					BestHitLocation = BoneLoc;
				}
			}
		}

		if (BestBone != '')
		{
			return true;
		}
	}
	return false;
}


/**
 * This function creates the actual spike in the world.  A duplicate Shard projectile is spawned
 * and that projectile is used to spike the pawn.  We use a secondary actor in order to avoid
 * having to wait for replication to occur since this actor is spawned client-side
 */
static function bool CreateSpike(UTPawn DeadPawn, vector InitialHitLocation, vector Direction)
{
	local UTProj_StingerShard Shard;
	local vector BoneLoc;
	local name	 BoneName;
	local TraceHitInfo 	HitInfo;
	local vector Start, End, HitLocation, HitNormal;

	if ( DeadPawn != None && !class'GameInfo'.static.UseLowGore(DeadPawn.WorldInfo) && !DeadPawn.bDeleteMe && DeadPawn.Health <= 0 && DeadPawn.Physics == PHYS_RigidBody
		&& DeadPawn.Mesh != None && DeadPawn.Mesh.PhysicsAssetInstance != None )
	{
		Start = InitialHitLocation - (256.0 * Normal(Direction));
		End = Start + (512.0 * Normal(Direction));

		if ( DeadPawn.WorldInfo.TraceComponent(HitLocation, HitNormal, DeadPawn.Mesh, End, Start, vect(0,0,0), HitInfo) )
		{
			if (HitInfo.BoneName != '')
			{
				BoneLoc  = DeadPawn.Mesh.GetBoneLocation(HitInfo.BoneName);
				BoneName = HitInfo.BoneName;
			}
		}

		// Begin by tracing towards just this component, looking for the nearest bone it collides with (if any)
		if ( BoneName != '' || FindNearestBone(DeadPawn, InitialHitLocation, BoneName, BoneLoc) )
		{
			// Spawn a shard
			BoneLoc -= (32 * Direction);

			Shard = DeadPawn.Spawn(class'UTProj_StingerShard',,, BoneLoc, rotator(Direction));
			if (Shard != None)
			{
				// Give this shard a push
				Shard.Speed = 800;
				Shard.AccelRate = 0.0f;
				Shard.Init(Direction);

				// Attach the victim to this shard
				Shard.SpikeVictim(DeadPawn,BoneName);

				return true;
			}
		}
	}

	return false;
}

/**
 * This function is called when the touch occurs locally in a remote client.  It checks
 * to see if the pawn is already ragdolling and if so, spikes it again
 */
simulated function ClientSideTouch(Actor Other, Vector HitLocation)
{
	local UTPawn DeadPawn;

	DeadPawn = UTPawn(Other);

	if (CreateSpike(DeadPawn, HitLocation, Normal(Vector(Rotation))))
	{
		Destroy();
	}
	else
	{
		Other.TakeDamage(Damage, InstigatorController, Location, MomentumTransfer * Normal(Velocity), MyDamageType,, self);
	}
}

/**
 * We have to override HitWall in order to store a pointer to the actor
 * that we hit (in the case of non-world geometry).  This way we can
 * be stuck to tanks, etc.
 */
simulated singular function HitWall(vector HitNormal, actor Wall, PrimitiveComponent WallComp)
{
	if (Wall != Instigator)
	{
		if (!Wall.bStatic && !Wall.bWorldGeometry)
		{
			if (DamageRadius == 0)
			{
				Wall.TakeDamage(Damage, InstigatorController, Location, MomentumTransfer * Normal(Velocity), MyDamageType,, self);
			}
			ImpactedActor = Wall;
		}
		else
		{
			BaseActor = Wall;
		}

		Explode(Location, HitNormal);
		ImpactedActor = None;
	}

	// If we are already spiked, stick in to the wall
	if (bSpiked)
	{
		// can't stick to blocking volumes
		if (BlockingVolume(Wall) != None)
		{
			Destroy();
		}
		else
		{
			Velocity = vect(0,0,0);
			SetBase(Wall);
			SetPhysics(PHYS_None);
		}
	}
}

/**
 * This function is responsible for creating the constraint between the projectile
 * and the victim.
 */
simulated function SpikeVictim(UTPawn Victim, name VictimBone)
{
	local UTProj_StingerShard OtherShard;

	// destroy any previous shards on the victim so we don't get nasty fighting between the two constraints
	foreach DynamicActors(class'UTProj_StingerShard', OtherShard)
	{
		if (OtherShard.ShardVictimConstraint != None && OtherShard.ShardVictimConstraint.ConstraintActor2 == Victim)
		{
			OtherShard.Destroy();
		}
	}

	CollisionComponent = MyMesh;

	// give projectile extent so that it collides the same way the pawn does
	// (so we don't get the ragdoll stuck in a blocking volume, etc)
	bSwitchToZeroCollision = false;
	CylinderComponent.SetCylinderSize(1.0, 1.0);
	CylinderComponent.SetActorCollision(true, false);

	SetLocation(Victim.Mesh.GetBoneLocation(VictimBone));

	ShardVictimConstraint = Spawn(class'RB_ConstraintActorSpawnable',,,Location);
	ShardVictimConstraint.SetBase(self);
	ShardVictimConstraint.InitConstraint( self, Victim, '', VictimBone, 2000.f);
	ShardVictimConstraint.LifeSpan = 8.0;

	// Add some velocity to the victim overall, to avoid the spike having to do all the work of pulling them
	Victim.Mesh.SetRBLinearVelocity(Velocity*SpikeVictimVelFactor, TRUE);

	if (WorldInfo.NetMode != NM_DedicatedServer)
	{
		Victim.LifeSpan += 8.0;
	}

	LifeSpan = 8.0;
	bSpiked = true;
	AdjustPhysicsForHit();

}

/**
 * We only want to process the touch if we are not spike and we are not a remote client.  In the case of
 * a remote client, ClientTouch should be used.
 */
simulated function ProcessTouch(Actor Other, Vector HitLocation, Vector HitNormal)
{
	local UTPawn P;

	if (!bSpiked && Role == ROLE_Authority)
	{
		if ( Other != Instigator )
		{
			P = UTPawn(Other);
			Other.TakeDamage(Damage, InstigatorController, HitLocation, MomentumTransfer * Normal(Velocity), MyDamageType,, self);
			if (P != None && (WorldInfo.NetMode == NM_DedicatedServer || !CreateSpike(P, HitLocation, Normal(Velocity))))
			{

				if (P != None && P.Health <= 0)
				{
					P.TearOffMomentum = Velocity;
					if (WorldInfo.NetMode != NM_DedicatedServer)
					{
						CreateSpike(P, HitLocation, Normal(Velocity));
					}
				}
			}
			Shutdown();
		}
	}

	if ( (Role < ROLE_Authority) && (Other.Role == ROLE_Authority) )
	{
		ClientSideTouch(Other, HitLocation);
	}
}

/**
 * Make sure to set our base properly
 */
simulated function Shutdown()
{
	if (BaseActor != none)
	{
		SetBase(BaseActor);
	}

	if (!bSpiked)
	{
		Super.ShutDown();
	}
}

/**
 * Setup this projectile's physics after it has spiked a victim
 */
simulated function AdjustPhysicsForHit()
{
	Velocity = Speed * Normal(vector(Rotation));
	Velocity.Z += 100.0;
	SetPhysics(PHYS_Falling);
	Acceleration = vect(0,0,0);
	CustomGravityScaling = 0.5;
}

defaultproperties
{
	DrawScale=1.0

	Begin Object Class=StaticMeshComponent Name=ProjectileMesh
		StaticMesh=StaticMesh'WP_Stinger.Mesh.S_WP_StingerShard'
		Scale=1.5
		MaxDrawDistance=12000
		CollideActors=false
		CastShadow=false
		bAcceptsLights=false
		BlockRigidBody=false
		BlockActors=false
		bUseAsOccluder=FALSE
	End Object
	Components.Add(ProjectileMesh)
	MyMesh=ProjectileMesh

	ProjExplosionTemplate=ParticleSystem'WP_Stinger.Particles.P_WP_Stinger_AltFire_Surface_Impact'
	MaxEffectDistance=7000.0

	SpikeVictimVelFactor=0.5

	Speed=2500
	MaxSpeed=4000
	AccelRate=4500.0

	Damage=38
	DamageRadius=0
	MomentumTransfer=70000
	CheckRadius=30.0

	MyDamageType=class'UTDmgType_StingerShard'
	LifeSpan=10.0

	bCollideWorld=true
	bProjTarget=false
	bNetTemporary=true
//	bAdvanceExplosionEffect=true
}
