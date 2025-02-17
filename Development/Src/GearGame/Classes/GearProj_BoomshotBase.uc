/**
 * Boomer Weapon Projectile
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_BoomshotBase extends GearProj_ExplosiveBase
	abstract
	config(Weapon);

/** How many bomblets to spawn when I explode */
var() config int				NumBombletsToSpawn;
/** How many bomblets to spawn when I explode from an AR shot */
var() config int				NumBombletsToSpawnActiveReload;
var() config array<vector2d>	BombletMinMaxLifespans;

/** Essentially just an .ini-exposed version of the Speed var.*/
var() config const float		RocketLaunchSpeed;

var repnotify bool bDelayedArc;

var transient Actor	ImpactActor;

/** Effect to play when boomshot hits a pawn */
var() protected const ParticleSystem HitPawnExplosionTemplate;
var() protected const ParticleSystem HitPawnExplosionNoGoreTemplate;

var protected const class<GearProj_BoomshotBombletBase> BombletClass;


replication
{
	if (bNetInitial)
		bDelayedArc;
}

simulated function PostBeginPlay()
{
	super.PostBeginPlay();
	Speed = RocketLaunchSpeed;
}

function Init(vector Direction)
{
	Super.Init(Direction);
	if (Role == ROLE_Authority && Instigator != None && Instigator.IsHumanControlled())
	{
		DelayArc();
	}
}

simulated function DelayArc()
{
	bDelayedArc = true;
	Acceleration = vect(0,0,0);
	SetTimer(0.25f, false, nameof(StartArcing));
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == nameof(bDelayedArc))
	{
		if (bDelayedArc)
		{
			DelayArc();
		}
		else if (IsTimerActive(nameof(StartArcing)))
		{
			ClearTimer(nameof(StartArcing));
			StartArcing();
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated function StartArcing()
{
	Acceleration.Z = GetGravityZ() * GravityScale;
	bDelayedArc = false;
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
	if ( Other != Instigator && !Other.bTearOff && (!Other.IsA('Projectile') || Other.bProjTarget) )
	{
		TriggerExplosion(HitLocation, HitNormal, Other);
	}
}


simulated function HitWall(vector HitNormal, actor Wall, PrimitiveComponent WallComp)
{
	// This is called when projectiles hit vehicles - ignore projectiles launched from yourself
	if(Wall != Instigator)
	{
		TriggerExplosion(Location, HitNormal, Wall);
	}
}


simulated protected function GetRadialDamageValues(out float outDamage, out float outRadius, out float outFalloff)
{
	local GearPawn ImpactGP;

	ImpactGP = GearPawn(ImpactActor);
	if (ImpactGP != None)
	{
		// bomblets will not be spawned, so add their damage to the blast.  if we don't do this,
		// point-blank boomshot attackers get substantially less damage than their victim.
		ExploDamage += NumBombletsToSpawn * BombletClass.default.ExploDamage;
	}

	Super.GetRadialDamageValues(outDamage, outRadius, outFalloff);
}

simulated function PrepareExplosionTemplate()
{
	super.PrepareExplosionTemplate();

	// don't apply radial damage to the hit actor, we'll do it manually below
	ExplosionTemplate.ActorToIgnoreForDamage = ImpactActor;

	if( GearPawn(ImpactActor) != None )
	{
		//`log( "using HITPAWN Emitter!" );
		if( WorldInfo.GRI.ShouldShowGore() )
		{
			ExplosionTemplate.ParticleEmitterTemplate = HitPawnExplosionTemplate;
		}
		else
		{
			ExplosionTemplate.ParticleEmitterTemplate = HitPawnExplosionNoGoreTemplate;
		}
		
		ExplosionTemplate.bAllowPerMaterialFX = FALSE; // we need to set this here otherwise the GearExplosionActor will override
	}
}


simulated function TriggerExplosion(vector HitLocation, vector HitNormal, Actor HitActor)
{
	local vector					BombletDir, SpawnLocation, Reflect, ReflectRVec;
	local GearProj_BoomshotBombletBase	SpawnedBomblet;
	local float						VelDotN, ImpactActorDamage;
	local int						Idx;
	local GearPawn					HitGP, ImpactGP;
	local GearVehicle				HitGV;
	local Hydra_Base				HitHydra;

	// If we hit a hostage, then don't do anything special, he's going to die.
	// Otherwise spare that actor, and do special processing below.
	HitGP = GearPawn(HitActor);
	HitGV = GearVehicle(HitActor);
	HitHydra = Hydra_Base(HitActor);
	if( (HitGP != None && !HitGP.IsAHostage()) || (HitGV != None) || (HitHydra != None) )
	{
		ImpactActor = HitActor;
	}	

	//`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "HitActor:" @ HitActor @ "ImpactActor:" @ ImpactActor);
	//ScriptTrace();

	// do normal explosion
	Super.TriggerExplosion(HitLocation, HitNormal, HitActor);

	// now do extra stuff
	if( Role == ROLE_Authority )
	{
		ImpactGP = GearPawn(ImpactActor);

		// If we didn't hit someone directly, spawn bomblets
		if( ImpactGP == None )
		{
			SpawnLocation = HitLocation + (HitNormal * 15.f);

			VelDotN = Velocity dot HitNormal;
			Reflect = Normal(Velocity - (2 * VelDotN) * HitNormal);
			ReflectRVec = Normal(Reflect cross HitNormal);

			// spawn the bomblets
			for (Idx=0; Idx<NumBombletsToSpawn; ++Idx)
			{
				SpawnedBomblet = Spawn(BombletClass, Instigator,, SpawnLocation);
				if (SpawnedBomblet != None)
				{
					SpawnedBomblet.Instigator = Instigator;
					SpawnedBomblet.bSuppressAudio = bSuppressAudio;

					SpawnedBomblet.LifeSpan = RandRange(BombletMinMaxLifespans[Idx].X, BombletMinMaxLifespans[Idx].Y);
					SpawnedBomblet.TimeTilExplosion = SpawnedBomblet.LifeSpan;

					BombletDir = Reflect + (RandRange(-1.f, 1.f) * ReflectRVec);
					BombletDir = Normal(BombletDir);

					SpawnedBomblet.Init(BombletDir);
				}
			}
		}

		if (ImpactActor != None)
		{
			// For the actor that was hit, apply damage directly (he was ignored in the radial damage) - unless it was a kidnapper (meatshield absorbed the damage)
			ImpactActorDamage = ExploDamage;

			if (ImpactGP != None)
			{
				// hit a gearpawn, give him the damage and the bomblet damage too
				ImpactActorDamage += NumBombletsToSpawn * BombletClass.default.ExploDamage;
			}

			ImpactActor.TakeDamage(ImpactActorDamage, InstigatorController, HitLocation, vector(Rotation), ExplosionTemplate.MyDamageType,, self);
		}
	}
}

defaultproperties
{
//	Speed=3000			// set RocketLaunchSpeed in the weapons ini
	MaxSpeed=5000
	MyDamageType=class'GDT_Boomshot'
	MomentumTransfer=1.f	// Scale momentum defined in DamageType

	GravityScale=0.65f

	Physics=PHYS_Projectile

	bRotationFollowsVelocity=TRUE

	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
		HiddenGame=false
		CastShadow=false
		CollideActors=false
		BlockActors=false
		BlockZeroExtent=false
		BlockNonZeroExtent=false
		BlockRigidBody=false
		bDisableAllRigidBody=TRUE
		Scale=2.25
		Rotation=(Yaw=-16384)
		bAcceptsLights=FALSE
		bAcceptsDynamicLights=FALSE
		bCastDynamicShadow=FALSE
		bAcceptsStaticDecals=FALSE
		bAcceptsDynamicDecals=FALSE
		bUseAsOccluder=FALSE
	End Object
	Components.Add(StaticMeshComponent0)

	// remove light environment
	Begin Object Name=MyLightEnvironment
		bEnabled=FALSE
	End Object
	Components.Remove(MyLightEnvironment)
	ProjLightEnvironment=None

	RemoteRole=ROLE_SimulatedProxy

	Lifespan=10.0
	bNetInitialRotation=true
	bCollideComplex=TRUE	// Ignore simple collision on StaticMeshes, and collide per poly
	bStoppedByGrenadeBlockingVolume=TRUE

	// explosion point light
	Begin Object Class=PointLightComponent Name=ExploLight0
		Radius=400.000000
		Brightness=500.000000
		LightColor=(B=35,G=185,R=255,A=255)
		Translation=(X=16)
		CastShadows=FALSE
		CastStaticShadows=FALSE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		bEnabled=FALSE
	End Object

	// explosion
	Begin Object Class=GearExplosion Name=ExploTemplate0
		MyDamageType=class'GDT_Boomshot'
		MomentumTransferScale=1.f	// Scale momentum defined in DamageType

		ExploLight=ExploLight0
		ExploLightFadeOutTime=0.17f

		bDoExploCameraAnimShake=TRUE
		ExploShakeInnerRadius=650
		ExploShakeOuterRadius=1500

		FractureMeshRadius=100.0
		FracturePartVel=300.0

		bAllowPerMaterialFX=TRUE

		bUsePerMaterialFogVolume=TRUE
	End Object
	ExplosionTemplate=ExploTemplate0

}


