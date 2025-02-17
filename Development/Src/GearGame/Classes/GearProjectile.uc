/**
 * Base class for Gear Projectiles
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearProjectile extends Projectile
	native(Weapon)
	config(Weapon);

/** TRUE to suppress all audio effects for this projectile, FALSE otherwise */
var bool								bSuppressAudio;

/** Multiplier affecting how much gravity to apply to this projectile. */
var() float								GravityScale;

/** Internal.  The AC for the active in-flight sound **/
var protected transient AudioComponent	InFlightSound;
/** SoundCue to use for looping in-flight audio. */
var() protected const SoundCue			InFlightSoundTemplate;
/** True if the InFlightAudio loop should start immediately, false otherwise. */
var() protected const bool				bAutoStartInFlightAudio;

var() bool								bAddBaseVelocity;

/** World velocity of the projectile's frame of reference at the time of firing. */
var transient repnotify vector			InitialFrameOfRefVelocity;

/** Trail Particle System */
var	protected const ParticleSystem		TrailTemplate;
/** Trail emitter */
var transient SpawnedGearEmitter		TrailEmitter;

var	DynamicLightEnvironmentComponent	ProjLightEnvironment;

var	bool								bIgnoreInstigatorCollision;

/** If TRUE, should be stopped by GrenadeBlockingVolume */
var	bool								bStoppedByGrenadeBlockingVolume;
/** How large the radius of perturbation is when we hit a fluid surface */
var	float								FluidForceRadius;

cpptext
{
	virtual void TickSpecial( FLOAT DeltaSeconds );
	virtual UBOOL IgnoreBlockingBy( const AActor *Other ) const;
}

simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	Acceleration.X = 0;
	Acceleration.Y = 0;
	Acceleration.Z = GetGravityZ() * GravityScale;

	if (!bSuppressAudio && bAutoStartInFlightAudio)
	{
		StartInFlightAudio();
	}

	if (TrailTemplate != None)
	{
		SpawnTrail();
	}
}

/** Spawn trail behind projectile. */
simulated protected function SpawnTrail()
{
	if ( (WorldInfo.NetMode != NM_DedicatedServer) && (TrailTemplate != None) )
	{
		// @todo look to moving this to the object pool
		TrailEmitter = Spawn(class'SpawnedGearEmitter', Self);
		if( TrailEmitter != None )
		{
			TrailEmitter.bPostUpdateTickGroup = true;
			TrailEmitter.SetTemplate(TrailTemplate, true );
			TrailEmitter.SetBase( Self );
			TrailEmitter.LifeSpan = 5.0f;
		}
	}
}

simulated protected function StopInFlightAudio()
{
	if (InFlightSound != None)
	{
		InFlightSound.FadeOut(0.2f, 0.f);
		InFlightSound = None;
	}
}

simulated protected function StartInFlightAudio()
{
	if( (InFlightSound == None) &&
		(InFlightSoundTemplate != None) &&
		(WorldInfo.NetMode != NM_DedicatedServer)
		)
	{
		InFlightSound = CreateAudioComponent( InFlightSoundTemplate, TRUE, TRUE );
		if(InFlightSound != None)
		{
			InFlightSound.bAutoDestroy = TRUE;
			InFlightSound.Location = Location;
			AttachComponent( InFlightSound );
			InFlightSound.FadeIn(0.3f, 1.f);
		}
	}
}

simulated function SuppressAudio(bool bSuppress)
{
	if (bSuppress && !bSuppressAudio)
	{
		// audio turning off, stop the inflight audio
		StopInFlightAudio();
	}
	else if (!bSuppress && bSuppress)
	{
		// audio turning on, restart flight sound
		StartInFlightAudio();
	}

	bSuppressAudio = bSuppress;
}

simulated function Destroyed()
{
	StopInFlightAudio();

	// Clean up trail if spawned
	if( TrailEmitter != None )
	{
		TrailEmitter.bDestroyOnSystemFinish = true;
		TrailEmitter.ParticleSystemComponent.DeactivateSystem();
		TrailEmitter.SetBase( None );
		TrailEmitter = None;
	}

	super.Destroyed();
}

function Init(vector Direction)
{
	// call super first to get the vel set, then maybe adjust it
	super.Init(Direction);

	// tweak direction to account for motion of base
	if ( bAddBaseVelocity && (Instigator != None) && (Instigator.Base != None) && !Instigator.Base.bStatic )
	{
		// @fixme, this will not account for linear vel as a result of base rotation.
		// need to find a way to get that information.
		InitialFrameOfRefVelocity = Instigator.Base.Velocity + (Instigator.Base.AngularVelocity cross (Instigator.Location - Instigator.Base.Location));
		Velocity += InitialFrameOfRefVelocity;
//		`log("adding base vel"@InitialFrameOfRefVelocity@"to old vel"@Direction*Speed@"newvel"@Velocity);
	}
	else
	{
		InitialFrameOfRefVelocity = vect(0,0,0);
	}
}

/** Stops projectile simulation without destroying it.  Projectile is resting, essentially. */
simulated protected function StopSimulating()
{
	Velocity = vect(0,0,0);
	Acceleration = vect(0,0,0);
	RotationRate = rot(0,0,0);
	SetPhysics(PHYS_None);
	SetCollision(FALSE, FALSE);

	StopInFlightAudio();
	StopTrailEmitter();

	bRotationFollowsVelocity = FALSE;
}

simulated protected function StopTrailEmitter()
{
	if( TrailEmitter != None )
	{
		TrailEmitter.bDestroyOnSystemFinish = true;
		TrailEmitter.ParticleSystemComponent.DeactivateSystem();
		TrailEmitter.SetBase( None );
		TrailEmitter = None;
	}
}


/** Called when this actor touches a fluid surface */
simulated function ApplyFluidSurfaceImpact( FluidSurfaceActor Fluid, vector HitLocation)
{
	local float AdjustedVelocity;
	local Emitter SplashEmitter;

	if (bAllowFluidSurfaceInteraction)
	{
		AdjustedVelocity = 0.01 * Abs(Velocity.Z);
		Fluid.FluidComponent.ApplyForce( HitLocation, AdjustedVelocity * Fluid.FluidComponent.ForceImpact, FluidForceRadius, True );
	}

	if ( CanSplash() )
	{
		if ( WorldInfo.NetMode != NM_DedicatedServer && 
			(Instigator != None) && 
			Instigator.IsPlayerPawn() && 
			Instigator.IsLocallyControlled() )
		{
			SplashEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter(Fluid.ProjectileEntryEffect, HitLocation, rotator(vect(0,0,1)));
			SplashEmitter.ParticleSystemComponent.ActivateSystem();
		}
	}
}

defaultproperties
{
	FluidForceRadius=10.0

	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment
		bEnabled=TRUE
		InvisibleUpdateTime=5.0
		MinTimeBetweenFullUpdates=2.0
		TickGroup=TG_DuringAsyncWork
		bCastShadows=FALSE
	End Object
	ProjLightEnvironment=MyLightEnvironment
	Components.Add(MyLightEnvironment)

	bAutoStartInFlightAudio=TRUE
	bAddBaseVelocity=TRUE

	LifeSpan=+0010.000000
}





