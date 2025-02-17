/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


/** Applies X-Ray effect to all pawns inside the volume */
class UTXRayVolume extends PhysicsVolume
	notplaceable
	abstract;

/** saved partial damage (in case of high frame rate */
var float	SavedDamage;

/** minimum SavedDamage before we actually apply it
* (needs to be large enough to counter any scaling factors that might reduce to below 1)
*/
var float MinimumDamage;

/** each second, LifeSpan is decreased by this much extra for each living Pawn in the volume */
var float PawnLifeDrainPerSec;

var ParticleSystemComponent SlowEffect, GeneratorEffect;
var SkeletalMeshComponent GeneratorMesh;

/** sounds played for various actions */
var SoundCue ActivateSound, DestroySound, EnterSound, ExitSound;
/** ambient sounds */
var SoundCue OutsideAmbientSound, InsideAmbientSound;
var AudioComponent AmbientSoundComponent;

/** camera emitter played on player in volume */
var class<UTEmitCameraEffect> InsideCameraEffect;

simulated event PostBeginPlay()
{
	local vector HitLocation, HitNormal;
	local vector TraceStart;

	Super.PostBeginPlay();

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
		`Warn("UTXRayVolume with no CollisionComponent");
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

simulated function Tick(float DeltaTime)
{
	local int SeatIndex;
	local float DamageAmount;
	local Actor HitActor;
	local Pawn HitPawn;
	local UTVehicle HitVehicle;

	DamageAmount = DamagePerSec;
	SavedDamage += DamageAmount * DeltaTime;
	DamageAmount = int(SavedDamage);

	// If the accumulated damage is large enough, pass it on to the actors
	if (DamageAmount >= MinimumDamage)
	{
		SavedDamage -= DamageAmount;
	}
	else
	{
		DamageAmount = 0;
	}

	foreach DynamicActors(class'Actor', HitActor)
	{
		if (Encompasses(HitActor))
		{
			HitPawn = Pawn(HitActor);
			HitVehicle = UTVehicle(HitActor);
			
			if (HitPawn != None || HitVehicle != None)
			{
				HitActor.TakeDamage(DamageAmount, DamageInstigator, HitActor.Location, vect(0,0,0), DamageType,,self);
			}
			if (HitVehicle != None)
			{
				// Now check each seat in the vehicle.
				for (SeatIndex = 0; SeatIndex < HitVehicle.Seats.length; ++SeatIndex)
				{
					HitPawn = HitVehicle.Seats[SeatIndex].StoragePawn;
					if (HitPawn != None)
					{
						HitPawn.TakeDamage(DamageAmount, DamageInstigator, HitActor.Location, vect(0,0,0), DamageType,,self);
					}
				}
			}
		}
	}
	Super.Tick(DeltaTime);
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
		}
	}


	if (Role == ROLE_Authority)
	{
		PlaySound(DestroySound);
	}
}

simulated event PawnEnteredVolume(Pawn Other)
{
	local UTPawn OtherPawn;
	local UTPlayerController PC;

	ActorEnteredVolume(Other);
	OtherPawn = UTPawn(Other);
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
	}
	if (OtherPawn != None)
	{
		OtherPawn.SetXRayEffect(true);
	}
}

simulated function PawnLeavingVolume(Pawn Other)
{
	local UTPawn OtherPawn;
	local UTPlayerController PC;

	OtherPawn = UTPawn(Other);
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
	}
	if (OtherPawn != None)
	{
		OtherPawn.SetXRayEffect(false);
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
	bProjTarget=false
	RemoteRole=ROLE_SimulatedProxy
	Priority=100000
	bPainCausing=false

	DamagePerSec=2.0
	MinimumDamage=1.0
	DamageType=class'UTDmgType_XRay'

	LifeSpan=180.0
	PawnLifeDrainPerSec=0.0

	bNetInitialRotation=true
}
