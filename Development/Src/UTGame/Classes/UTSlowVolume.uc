/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


/** slows down all actors inside the volume */
class UTSlowVolume extends GravityVolume
	native
	notplaceable
	abstract;

var float ScalingFactor;
var float ProjectileScalingFactor;
var float RBScalingFactor;

/** each second, LifeSpan is decreased by this much extra for each living Pawn in the volume */
var float PawnLifeDrainPerSec;

/** Gravity applied to rigid bodies within this volume. */
var float RBGravityZ;

/** array of ReachSpecs we modified the distance for, so we reset them when we are destroyed */
var array<ReachSpec> ModifiedSpecs;

var ParticleSystemComponent SlowEffect, GeneratorEffect;
var SkeletalMeshComponent GeneratorMesh;

/** sounds played for various actions */
var SoundCue ActivateSound, DestroySound, EnterSound, ExitSound;
/** ambient sounds */
var SoundCue OutsideAmbientSound, InsideAmbientSound;
var AudioComponent AmbientSoundComponent;

/** camera emitter played on player in volume */
var class<UTEmitCameraEffect> InsideCameraEffect;

cpptext
{
	virtual FLOAT GetVolumeRBGravityZ() { return RBGravityZ; }
}

simulated event PostBeginPlay()
{
	local vector HitLocation, HitNormal;
	local vector TraceStart;
	local int i;

	Super.PostBeginPlay();

	RBGravityZ = GravityZ * RBScalingFactor;

	//Position of visual mesh (relative to actor location)
	TraceStart = Location + (GeneratorMesh.Translation >> Rotation);

	//Get the Z location of where the visual mesh will be placed
	if (Trace(HitLocation, HitNormal, TraceStart - vect(0,0,250), TraceStart, false) != None)
	{
		//But spawn the thing in the X/Y of the actor location
		HitLocation.X = Location.X;
		HitLocation.Y = Location.Y;
		SetLocation(HitLocation);
	}
	else
	{
		Destroy();
		return;
	}

	if (CollisionComponent == None)
	{
		`Warn("UTSlowVolume with no CollisionComponent");
	}
	else if (!(ScalingFactor ~= 0.0))
	{
		// increase cost of overlapping ReachSpecs
		WorldInfo.NavigationPointCheck(CollisionComponent.GetPosition(), CollisionComponent.Bounds.BoxExtent,, ModifiedSpecs);
		for (i = 0; i < ModifiedSpecs.length; i++)
		{
			ModifiedSpecs[i].Distance /= ScalingFactor;
		}
	}
	if (GeneratorMesh != none)
	{
		GeneratorMesh.PlayAnim('Deploy');
		SetTimer(1.3f,false,'ActivateSlowEffect');
	}
	else
	{
		ActivateSlowEffect();
	}
	if (WorldInfo.NetMode != NM_DedicatedServer)
	{
		AmbientSoundComponent.SoundCue = OutsideAmbientSound;
		AmbientSoundComponent.Play();
	}
	if ( Role == ROLE_Authority )
	{
		SetTimer(1.0, true);
	}
}

simulated event CollisionChanged();

simulated function ActivateSlowEffect()
{
	local Actor A;

	SetCollision(true,false,false);
	// force any actors inside us to go slow
	foreach DynamicActors(class'Actor', A)
	{
		if ( Encompasses(A) && (A != self) )
		{
			A.SetZone(true);
		}
	}
	if (Role == ROLE_Authority)
	{
		PlaySound(ActivateSound);
	}
	if (WorldInfo.NetMode != NM_DedicatedServer)
	{
		if (SlowEffect != None)
		{
			SlowEffect.SetActive(true);
		}
		if (GeneratorEffect != None)
		{
			GeneratorEffect.SetActive(true);
		}
	}
}

simulated event Destroyed()
{
	local Actor A;
	local int i;

	Super.Destroyed();

	// force any actors inside us to go back to normal
	foreach DynamicActors(class'Actor', A)
	{
		if (Encompasses(A))
		{
			if (A.IsA('Pawn'))
			{
				PawnLeavingVolume(Pawn(A));
			}
			else
			{
				ActorLeavingVolume(A);
			}
		}
	}

	// return cost of ReachSpecs to normal
	for (i = 0; i < ModifiedSpecs.length; i++)
	{
		if (ModifiedSpecs[i] != None)
		{
			ModifiedSpecs[i].Distance *= ScalingFactor;
		}
	}

	if (Role == ROLE_Authority)
	{
		PlaySound(DestroySound);
	}
}


simulated event ActorEnteredVolume(Actor Other)
{
	if ( Projectile(Other) != None )
	{
		Other.CustomTimeDilation = ProjectileScalingFactor;
	}
	else if ( UTCarriedObject(Other) != None )
	{
		Other.CustomTimeDilation = 0.5;
	}
	else
	{
		Other.CustomTimeDilation = ScalingFactor;
	}
}

simulated event ActorLeavingVolume(Actor Other)
{
	if ( Weapon(Other) == None )
	{
		// weapons get their customtimedilation from their instigator
		Other.CustomTimeDilation = Other.Default.CustomTimeDilation;
	}
}

simulated event PawnEnteredVolume(Pawn Other)
{
	SlowPawnDown(Other);
}

simulated function SlowPawnDown(Pawn Other)
{
	local SVehicle V;
	local UTPlayerController PC;

	ActorEnteredVolume(Other);

	V = SVehicle(Other);
	if (V != None)
	{
		V.MaxSpeed = V.Default.MaxSpeed * ScalingFactor;
	}

	PC = UTPlayerController(Other.Controller);
	if (PC != None && PC.IsLocalPlayerController())
	{
		PC.ClientPlaySound(EnterSound);
		AmbientSoundComponent.Stop();
		AmbientSoundComponent.SoundCue = InsideAmbientSound;
		AmbientSoundComponent.Play();
		if (InsideCameraEffect != None)
		{
			PC.ClientSpawnCameraEffect(InsideCameraEffect);
		}
		PC.ConsoleCommand( "SETSOUNDMODE Slow", false );
	}
}

simulated function PawnLeavingVolume(Pawn Other)
{
	local SVehicle V;
	local UTPlayerController PC;

	ActorLeavingVolume(Other);

	V = SVehicle(Other);
	if (V != None)
	{
		V.MaxSpeed = V.Default.MaxSpeed;
	}

	PC = UTPlayerController(Other.Controller);
	if (PC != None && PC.IsLocalPlayerController())
	{
		PC.ClientPlaySound(ExitSound);
		AmbientSoundComponent.Stop();
		AmbientSoundComponent.SoundCue = OutsideAmbientSound;
		AmbientSoundComponent.Play();
		if (InsideCameraEffect != None)
		{
			PC.ClearCameraEffect();
		}
		PC.ConsoleCommand( "SETSOUNDMODE Default", false );
	}
}

simulated function NotifyPawnBecameViewTarget(Pawn P, PlayerController PC)
{
	local UTPlayerController UTPC;

	if (InsideCameraEffect != None)
	{
		UTPC = UTPlayerController(PC);
		if (UTPC != None)
		{
			UTPC.ClientSpawnCameraEffect(InsideCameraEffect);
		}
	}
}

function Timer()
{
	local Pawn P;
	local UTGameObjective O;

	ForEach TouchingActors(class'Pawn', P)
	{
		if ( P.PlayerReplicationInfo != None )
		{
			LifeSpan -= PawnLifeDrainPerSec;
		}
	}
	ForEach TouchingActors(class'UTGameObjective', O)
	{
		if ( !O.bIsDisabled )
		{
			LifeSpan -= 1.5 * PawnLifeDrainPerSec;
		}
	}
	if ( LifeSpan == 0.0 )
	{
		Lifespan = 0.001;
	}
}

function Reset()
{
	Destroy();
}

simulated function bool StopsProjectile(Projectile P)
{
	return false;
}

defaultproperties
{
	bCollideActors=false
	bBlockActors=false
	bStatic=false
	bNoDelete=false
	bHidden=false
	bProjTarget=true
	RemoteRole=ROLE_SimulatedProxy
	Priority=100000
	LifeSpan=180.0
	PawnLifeDrainPerSec=3.0

	ProjectileScalingFactor=0.125
	ScalingFactor=0.2
	RBScalingFactor=0.4
	RigidBodyDamping=3.5

	bNetInitialRotation=true
}
