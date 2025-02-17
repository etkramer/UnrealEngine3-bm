/**
 * Hydra Rocket Projectile
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_HomingRocket extends GearProj_ExplosiveBase
	native(Weapon)
	config(Weapon);

/** Essentially just an .ini-exposed version of the Speed var.*/
var() config const float		RocketLaunchSpeed;

/** Actor that we are homing in on */
var		Actor	TargetActor;

var		Actor	SourceActor;

var		vector	SourcePosActorSpace;

/** When closer than this distance, stop homing in. */
var config float	StopHomingDistance;

/** How much rockets home in */
var config float	TrackingStrength;

/** If TRUE, will use a simple mode where it simply interpolates from spawn location to TargetActor over InterpTime */
var bool	bUseInterpPhysics;

/** How long it will take to reach TargetActor when bUseInterpPhysics is TRUE */ 
var config float	InterpTime;

var	float	CurrentAlpha;

replication
{
	if( bNetInitial && Role == ROLE_Authority )
		TargetActor, SourceActor, SourcePosActorSpace;
}

cpptext
{
	virtual void physProjectile(FLOAT DeltaTime, INT Iterations);
	virtual UBOOL IgnoreBlockingBy( const AActor *Other ) const;
	virtual void TickSpecial( FLOAT DeltaSeconds );
};

simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	Speed = RocketLaunchSpeed;
}

simulated function Destroyed()
{
	// Clean up trail if spawned
	if( TrailEmitter != None )
	{
		TrailEmitter.bDestroyOnSystemFinish = true;
		TrailEmitter.ParticleSystemComponent.DeactivateSystem();
		TrailEmitter.SetBase( None );
		TrailEmitter = none;
	}

	super.Destroyed();
}

simulated function ProcessTouch(Actor Other, Vector HitLocation, Vector HitNormal)
{
	// ignore hits to self and/or dead people
	if ( (Other != Instigator) && (Other != SourceActor) && !Other.bTearOff && (!Other.IsA('Projectile') || Other.bProjTarget) )
	{
		TriggerExplosion(HitLocation, HitNormal, Other);
	}
}

simulated function HitWall(vector HitNormal, actor Wall, PrimitiveComponent WallComp)
{
	// This is called when projectiles hit vehicles - ignore projectiles launched from yourself
	if(Wall != Instigator)
	{
		TriggerExplosion(Location, HitNormal, None);
	}
}

/** Do homing (acceleration update) in Tick */
simulated event Tick(float DeltaTime)
{
	local vector ForceDir;
	local float VelMag;

	Super.Tick(DeltaTime);

	// Don't do this stuff if using interp physics
	if(bUseInterpPhysics)
	{
		return;
	}

	if ( TargetActor == None )
	{
		return;
	}

	// Calc vector from rocket to target.
	ForceDir = (TargetActor.Location - Location);

	// If we are too close, or are travelling away from target, don't home

	if(VSize(ForceDir) > StopHomingDistance && (ForceDir Dot Velocity) > 0.0)
	{
		// normalize direction vector
		ForceDir = Normal(ForceDir);

		// Set acceleration towards target
		VelMag = VSize(Velocity);
		Acceleration = TrackingStrength * VelMag * ForceDir;
	}
	else
	{
		// Don't change velocity
		Acceleration = vect(0.0, 0.0, 0.0);
	}
}

defaultproperties
{
	GravityScale=0.0
	MomentumTransfer=1.0	// Scale momentum defined in DamageType

	Physics=PHYS_Projectile

	bRotationFollowsVelocity=TRUE

	RemoteRole=ROLE_SimulatedProxy

	bNetInitialRotation=true
	bCollideComplex=TRUE	// Ignore simple collision on StaticMeshes, and collide per poly

	Lifespan=10.0


	// remove light environment
	Begin Object Name=MyLightEnvironment
	    bEnabled=FALSE
	End Object
	Components.Remove(MyLightEnvironment)
	ProjLightEnvironment=None


}