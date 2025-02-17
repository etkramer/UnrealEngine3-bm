/**
 * Hammer of Dawn Beam.
 * An instance of a HOD beam, handles sequencing of a single shot
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class HOD_BeamBase extends Actor
	abstract
	config(Weapon);

/** Time before deathray warmup cycle begins.*/
var() protected config float PreWarmupTimeSec;
/** Effectively the delay between firing the targeting beam and seeing the death ray from the sky */
var() protected config float WarmupTimeSec;
/** Time between shot stopping actively firing and the ability to fire another shot */
var() protected config float CooldownTimeSec;
/** How long a single shot of the death ray lasts */
var() config float FireDurationSec;

/** Rate at which beam applies damage */
var() config float BaseDamagePerSecond;


/** Defines the explosion. */
var() instanced editinline GearExplosion	InitialExplosionTemplate;

/** How much damage to apply at epicenter of inital beam strike */
var() protected config float InitialSplashDamage;
/** Size of initial beam strike blast.  Damage falloff is linear with distance. */
var() config float InitialSplashDamageRadius;
/** Falloff exponent for radial damage */
var() protected config float InitialSplashDamageFalloff;

/** amount of momentum transferred to folks hit by the blast */
var() protected config float InitialSplashDamageMomemtum;

/** Beam starts at this radius when first fired */
var() protected config float	BaseDamageRadius;

/** If TRUE, HOD beam uses zero extent trace. */
var bool bZeroExtentBeam;
/** If TRUE, extent hurt trace below the beam location as well */
var bool bHurtBelowBeam;

/** Beam ends at this scale factor when firing is finished */
var() protected float	MaxBeamScale;
/** Beam scaling factor, since it grows over time.  Applied to several things */
var	  protected float	CurBeamScale;

/** Controls how aggressively the deathray tracks the target */
var() config protected float TrackingSpeed;
/** This is an actual-velocity cap on how fast the beam target cam move around.  In worldunits/sec. */
var() config protected float MaxBeamTrackingVel;

/** Main beam effect */
var()	protected ParticleSystemComponent	PSC_MainBeam;
/** Tendril beam effect */
var()	protected ParticleSystem			PS_TendrilBeam;
/** Tendril beam surface impact effect */
var()	protected ParticleSystem			PS_TendrilImpact;

// special-case impact effects
var	protected ParticleSystemComponent	PSC_BerserkerImpact;
var protected transient bool			bPlayingBerserkerImpactEffect;

/** How many warmup tendrils to spawn */
var()	protected config int	NumWarmupTendrils;
/** Tendrils start this far from main beam and move closer over time */
var()	protected float			TendrilInitialRadius;

/** structure to contain the data associated with a single warmun tendril beam */
struct TendrilBeam
{
	/** ref to the emitter that represents the beam */
	var		GearEmitter		BeamEmitter;
	/** ref to the emitter that represents the surface impact effects */
	var		GearEmitter		SurfaceImpactEmitter;
	/** direction, in world space, from the main beam/target spot to this particular beam's target location */
	var		vector			OffsetDir;
	/** How long, in seconds, until this beam will spawn */
	var		float			TimeUntilSpawn;
	/** True if this beam is ready to be spawned (if timer is up) */
	var		bool			bReadyToSpawn;
};
/** array of tendril beams that are alive */
var	protected array<TendrilBeam>	TendrilBeams;


/** true if the death ray is actually firing (as in, not warming up), false otherwise */
var repnotify bool bActuallyFiring;

/** Last valid target the death ray knows about.  Location will interp towards this  */
var protected vector			LastValidTargetPos;
/** True if beam has no currently valid target. */
var protected transient bool	bTargetIsInvalid;
/** How long target has been invalid */
var protected transient float	InvalidTargetTime;
/** How long the target can be invalid before the beam shot is aborted */
var() protected float			MaxInvalidTargetTime;

/** basically self.Location from the previous Tick */
var protected transient vector	LastActualPos;

/** Beam is 'off', but continues to track and is alive indefinitely. */
var bool	bDormant;

/** If true, disallow firing. */
var protected transient bool bDoNotFire;

/** True if the warmup cycle for this beam is complete. */
var protected transient bool bWarmedUp;

/** True if this beam should autofire when it is warmed up */
var protected transient bool bAutoFire;

/** What time the warmup effects began. */
var protected transient float WarmupEffectsStartedTime;

/** TRUE if player has requested to actually fire, FALSE otherwise */
var protected transient bool bOkToFire;


//
// Effects-related vars
//

/** Sounds */
var	protected	AudioComponent	WarmupSound;
var protected	SoundCue		ImpactSound;
var protected 	SoundCue		CoolDownSound;
var	protected	AudioComponent	RippingEarthLoopSound;
var() protected	float			RippingEarthBeamSpeedForMaxVolume;
var() protected	float			RippingEarthVolumeInterpSpeed;
var protected	AudioComponent	FireLoopSound;

/** Dynamic Light */
var protected	PointLightComponent		BeamLight;
/** Speed beam light fades out, brightness units per second */
var() protected	float					BeamLightFadeRate;
/** true if beamlight is fading out, false otherwise */
var protected	transient bool			bBeamLightFadingAway;

/** Camera shake for the death ray */
var() ScreenShakeStruct		HODCamShake;
/** Distances for cam shake scaling. */
var() const protected float HODCamShakeOuterRadius;
/** Distances for cam shake scaling. */
var() const protected float HODCamShakeInnerRadius;

var() array<Vector>	EntryEffectLocation,	EntryEffectNormal;
var() array<Vector>	ExitEffectLocation,		ExitEffectNormal;
var() array<GearEmitter> EntryFX, ExitFX;
var() array<byte> EntryFXActive, ExitFXActive;

var() protected ParticleSystem PS_EntryEffect;
var() protected ParticleSystem PS_ExitEffect;

// OXM HACK
// the "oxm hack" here is spawning a stream of emitters to simulate the damage trail on the
// ground.  preferably, the trail would be wholly contained within a single effect that we could move around,
// but the particle code doesn't support storing per-particle orients at the moment.
var protected ParticleSystem PS_GroundDamage;
var() protected float ImpactEmitterSpawnRate_OXMHACK;
var() protected float MinEmitterSpawnRate_OXMHACK;
var() protected float LeftOverSpawnTime_OXMHACK;
var() protected bool bOXMHack;

/** TRUE to render a debug cylinder representing the damage region, FALSE to not render it. */
var protected config bool bDrawDamageCylinder;

/** Placed when the hammer impacts the ground **/
var protected DecalMaterial ScorchDecal;

/** replicated variable to notify when to stop warming up on clients */
var protected transient repnotify bool bAborted;

/** This is the damage type for this beam.  @see Tick and HurtBeam **/
var protected class<GearDamageType> BeamDamageType;

var transient bool bImpactTrailIntact;

/** Time until next fracture check */
var float		TimeUntilNextFractureCheck;
/** How far away from beam we break bits off */
var() float		DragFractureRadius;
/** How hard to throw broken off pieces */
var() float		DragFracturePartVel;

var repnotify protected byte bDoInitialExplosionEffect;

replication
{
	if ( Role == ROLE_Authority )
		bActuallyFiring, bAborted, bDoInitialExplosionEffect;

	if ( bNetInitial && bNetOwner )
		PreWarmupTimeSec, WarmupTimeSec, CooldownTimeSec, FireDurationSec, TrackingSpeed, MaxBeamTrackingVel, NumWarmupTendrils;
}

simulated event ReplicatedEvent(name VarName)
{
	switch (VarName)
	{
	case 'bDoInitialExplosionEffect':
		DoInitialExplosion();
		break;

	case 'bActuallyFiring':
		if (bActuallyFiring)
		{
			DoHODFireEffects();
		}
		else
		{
			DoHODCooldownEffects();
		}
		break;
	case 'bAborted':
		// false autofire, since fire command will come replicated
		AbortFire(TRUE);
		break;
	default:
		super.ReplicatedEvent(VarName);
	}
}

simulated event PostBeginPlay()
{
	if (Role < ROLE_Authority)
	{
		// on clients, set timer for warmup effects to start
		// on server, the normal timer chain will execute
		if (PreWarmupTimeSec > 0.f)
		{
			SetTimer( PreWarmupTimeSec, FALSE, nameof(DoHODWarmupEffects) );
		}
		else
		{
			DoHODWarmupEffects();
		}
	}

	//AttachComponent(PSC_MainBeam);

	CurBeamScale = 1.f;
	LastActualPos = Location;

	// to prevent name collision issues with multiple beams at once
	HODCamShake.ShakeName = Name;
}


/** Intialize a beam so it is ready to fire */
simulated function Init(Pawn ShotInstigator)
{
	Instigator = ShotInstigator;
}

/** Tracks actual target towards the desired target. Server only.*/
protected function DoBeamTargetTracking(float DeltaTime)
{
	local vector TmpLoc;

	if (bBeamLightFadingAway)
	{
		// stop tracking altogether, do nothing
		TmpLoc = Location;
	}
	else if (IsTimerActive('NotifyPreWarmupFinished'))
	{
		// during prewarmup, just bash position, don't do tracking interpolation.
		TmpLoc = LastValidTargetPos;
	}
	else
	{
		// introduce some lag into the death ray tracking.  note we do this even when only warming up
		// so that the beam starts nearish to the target position even if it moves during the warmup
		//@fixme, feels ok, but maybe give the beam some momentum?  this will change dirs on a dime.
		TmpLoc = VInterpTo(Location, LastValidTargetPos, DeltaTime, TrackingSpeed);

		// impose an absolute speed cap.  this helps the ground effect look ok since it lays down
		// a blanket of sprites at a constant rate
		if ( (VSizeSq(TmpLoc-Location) / DeltaTime) > MaxBeamTrackingVel )
		{
			TmpLoc = Location + ( Normal(LastValidTargetPos - Location) * MaxBeamTrackingVel * DeltaTime );
		}
	}

	SetLocation(TmpLoc);

	//`log("New Loc CALCULATED to be"@TmpLoc);
}

simulated function SetEntryEffectEnabled( bool bEnabled )
{
	local int Idx;

	if( bEnabled )
	{
		for( Idx = 0; Idx < EntryEffectLocation.Length; Idx++ )
		{
			if( Idx >= EntryFX.Length )
			{
				EntryFX[Idx] = Spawn(class'SpawnedGearEmitter');
				EntryFX[Idx].LifeSpan = 0.0f;
				EntryFX[Idx].ParticleSystemComponent.bAutoActivate = FALSE;
				EntryFX[Idx].ParticleSystemComponent.SetAbsolute( TRUE, TRUE, TRUE );
				EntryFX[Idx].SetTemplate(PS_EntryEffect, FALSE);
				EntryFXActive[Idx] = 0;
			}

			if( EntryFXActive[Idx] == 0 )
			{
				EntryFX[Idx].ParticleSystemComponent.ActivateSystem();
				EntryFXActive[Idx] = 1;
			}

			EntryFX[Idx].ParticleSystemComponent.SetTranslation( EntryEffectLocation[Idx] );
			EntryFX[Idx].ParticleSystemComponent.SetRotation( Rotator(EntryEffectNormal[Idx]) );
		}

		for( Idx = EntryEffectLocation.Length; Idx < EntryFX.Length; Idx++ )
		{
			EntryFX[Idx].ParticleSystemComponent.DeactivateSystem();
			EntryFXActive[Idx] = 0;
		}
	}
	else
	{
		for( Idx = 0; Idx < EntryFX.Length; Idx++ )
		{
			EntryFX[Idx].ParticleSystemComponent.DeactivateSystem();
		}
	}
}

simulated function SetExitEffectEnabled( bool bEnabled )
{
	local int Idx;

	if( bEnabled )
	{
		for( Idx = 0; Idx < ExitEffectLocation.Length; Idx++ )
		{
			if( Idx >= ExitFX.Length )
			{
				ExitFX[Idx] = Spawn(class'SpawnedGearEmitter');
				ExitFX[Idx].LifeSpan = 0.0f;
				ExitFX[Idx].ParticleSystemComponent.bAutoActivate = FALSE;
				ExitFX[Idx].ParticleSystemComponent.SetAbsolute( TRUE, TRUE, TRUE );
				ExitFX[Idx].SetTemplate(PS_ExitEffect, FALSE);
				ExitFXActive[Idx] = 0;
			}

			if( ExitFXActive[Idx] == 0 )
			{
				ExitFX[Idx].ParticleSystemComponent.ActivateSystem();
				ExitFXActive[Idx] = 1;
			}

			ExitFX[Idx].ParticleSystemComponent.SetTranslation( ExitEffectLocation[Idx] );
			ExitFX[Idx].ParticleSystemComponent.SetRotation( Rotator(ExitEffectNormal[Idx]) );
		}

		for( Idx = ExitEffectLocation.Length; Idx < ExitFX.Length; Idx++ )
		{
			ExitFX[Idx].ParticleSystemComponent.DeactivateSystem();
			ExitFXActive[Idx] = 0;
		}
	}
	else
	{
		for( Idx = 0; Idx < ExitFX.Length; Idx++ )
		{
			ExitFX[Idx].ParticleSystemComponent.DeactivateSystem();
		}
	}
}

simulated function GatherEntryAndExitLocations( Actor HitActor )
{
	local SkeletalMeshActor SMA;
	local array<ImpactInfo> EntryHits, ExitHits;
	local int Idx;

	local int NextEntryIdx, NextExitIdx;
	local int Depth;
	local float NextExitZ, NextEntryZ;

	SMA = SkeletalMeshActor(HitActor);

//	FlushPersistentDebugLines();
//	DrawDebugBox(Location, vect(30,30,30), 255, 255, 255, TRUE);
//	DrawDebugBox(Location+vect(0,0,20000), vect(30,30,30), 255, 255, 255, TRUE);
//	DrawDebugLine(Location, Location+vect(0,0,20000), 255, 255, 255, TRUE);

	// Gather Entry Points
	if( TraceAllPhysicsAssetInteractions( SMA.SkeletalMeshComponent, Location, Location + vect(0,0,20000), EntryHits ) && 
		TraceAllPhysicsAssetInteractions( SMA.SkeletalMeshComponent, Location + vect(0,0,20000), Location, ExitHits ) )
	{
		//`log( "Total Hits"@EntryHits.Length@"/"@ExitHits.Length );
		//for( Idx = 0; Idx < EntryHits.Length; Idx++ )
		//{
		//	`log( "Entry"@EntryHits[Idx].HitLocation.Z@Idx@EntryHits[Idx].HitInfo.BoneName@EntryHits[Idx].HitInfo.Item );
		//}
		//for( Idx = 0; Idx < ExitHits.Length; Idx++ )
		//{
		//	`log( "Exit"@ExitHits[Idx].HitLocation.Z@Idx@ExitHits[Idx].HitInfo.BoneName@ExitHits[Idx].HitInfo.Item );
		//}

		// assumption that return hits are sorted along the line.  So EntryHits is sorted in decreasing Z, 
		// and ExitHits sorted in increasing Z.
		NextExitIdx = ExitHits.length - 1;
		NextEntryIdx = 0;
		Depth = 0;

		// walk top down, counting entrances and exits, discarding "internal" hits
		while ( (NextExitIdx >= 0) || (NextEntryIdx < EntryHits.length - 1) )
		{
			NextExitZ = (NextExitIdx >= 0) ? ExitHits[NextExitIdx].HitLocation.Z : -99999.f;
			NextEntryZ = (NextEntryIdx < EntryHits.length - 1) ? EntryHits[NextEntryIdx].HitLocation.Z : -99999.f;

			if (NextExitZ > NextEntryZ)
			{
				// next hit is an exit hit
				if (Depth > 1)
				{
					// throw it out, only keep 1->0 transitions
					ExitHits.Remove(NextExitIdx, 1);
				}

				NextExitIdx--;
				Depth--;
			}
			else
			{
				// next hit is an entry hit
				if (Depth > 0)
				{
					// throw it out, only keep 0->1 transitions
					EntryHits.Remove(NextEntryIdx, 1);
					
				}
				else
				{
					NextEntryIdx++;
				}

				Depth++;
			}
		}

		//for( Idx = 0; Idx < EntryHits.Length; Idx++ )
		//{
		//	DrawDebugBox( EntryHits[Idx].HitLocation, vect(20,20,20), 0, 255, 0, TRUE );
		//}
		//for( Idx = 0; Idx < ExitHits.Length; Idx++ )
		//{
		//	DrawDebugBox( ExitHits[Idx].HitLocation, vect(20,20,20), 255, 0, 0, TRUE );
		//}

		// Store off all entry/exit location/normal pairs
		EntryEffectLocation.Length = EntryHits.Length;
		EntryEffectNormal.Length   = EntryHits.Length;
		for( Idx = 0; Idx < EntryHits.Length; Idx++ )
		{
			EntryEffectLocation[Idx] = EntryHits[Idx].HitLocation;
			EntryEffectNormal[Idx]   = EntryHits[Idx].HitNormal;
		}
		ExitEffectLocation.Length = ExitHits.Length;
		ExitEffectNormal.Length   = ExitHits.Length;
		for( Idx = 0; Idx < ExitHits.Length; Idx++ )
		{
			ExitEffectLocation[Idx] = ExitHits[Idx].HitLocation;
			ExitEffectNormal[Idx]   = ExitHits[Idx].HitNormal;
		}
	}
	else
	{
		EntryEffectLocation.Length	= 0;
		EntryEffectNormal.Length	= 0;
	}
}

simulated protected function FindGround(vector BaseLoc, float TraceDist, out vector HitLocation, out vector HitNormal, out Actor HitActor)
{
	local vector StartTrace, EndTrace;
	local Weapon W;
	local ImpactInfo Impact;
	local Actor HA;
	local Vector HL, HN;

	W = Instigator.Weapon;

	// find new surface normal
	StartTrace = BaseLoc;
	StartTrace.Z += TraceDist;			// way up!
	EndTrace = BaseLoc;
	EndTrace.Z -= TraceDist;			// way down!

	Impact = W.CalcWeaponFire(StartTrace, EndTrace);

	if( Impact.HitActor != None && Impact.HitActor.Tag == 'LambentBrumak' )
	{
		TrackingSpeed		= 10000.f;
		MaxBeamTrackingVel	= 10000.f;

		StartTrace = Impact.HitLocation;
		EndTrace   = StartTrace + vect(0,0,-20000);

		HA = Impact.HitActor.Trace( HL, HN, EndTrace, StartTrace, FALSE );
		if( HA != None )
		{
			Impact.HitLocation = HL;
			Impact.HitNormal   = HN;
		}
		else
		{
			Impact.HitLocation = EndTrace;
			Impact.HitNormal   = vect(0,0,1);
		}			
	}

	HitActor	= Impact.HitActor;
	HitNormal	= Impact.HitNormal;
	HitLocation = Impact.HitLocation;
}

/** Sticks this object to the ground below.  Returns normal of what it stuck to. */
simulated protected function StickToGround(out vector HitNormal, out Actor HitActor)
{
	local vector HitLoc;

	FindGround(Location, 20000.f, HitLoc, HitNormal, HitActor);

	if (HitActor != None)
	{
		if (Role == ROLE_Authority)
		{
			SetLocation(HitLoc);
		}
	}
	else
	{
		// hit nothing (wth?), just return upward
		HitNormal = vect(0,0,1);
	}
}


simulated protected function vector GetBeamDirection()
{
	return class'HOD_BeamManagerBase'.default.DirectionToDeathRayOriginNorm;
}

simulated protected function float GetBeamLength()
{
	return 5000;
}

/**
 *
 */
simulated function Tick(float DeltaTime)
{
	local int		Idx;
	local vector	HitLoc, HitNormal, TendrilBaseLoc, StartTrace, BeamDir, CurrentSurfaceNormal, HurtEnd;
	local Rotator	TmpRot;
	local float		Pct, BeamSpeed, ScaleRate, FlatDist;
	local Actor		HitActor;
	local bool		bTendrilNeedsActivated;

	// OXM Hack vars
	local GearEmitter NewImpactEmitter_OXMHACK;
	local int NumToSpawn;
	local vector SpawnLoc;
	local float SpawnRate_OXMHACK;

	local bool bHitBerserker, bNoGroundFX;


	// update the target tracking, server only
	if (Role == ROLE_Authority)
	{
		DoBeamTargetTracking(DeltaTime);
	}

	StickToGround(CurrentSurfaceNormal, HitActor);

	//DrawDebugSphere(Location, 32, 16, 255, 255, 0);

	BeamDir = GetBeamDirection();
	if (bActuallyFiring)
	{
		ScaleRate = (MaxBeamScale - 1.f) / FireDurationSec;

		CurBeamScale += ScaleRate * DeltaTime;
		CurBeamScale = FMin(CurBeamScale, MaxBeamScale);

		BeamLight.Radius = BeamLight.default.Radius * CurBeamScale;

		// draw the beam, debug style
		if (bDrawDamageCylinder)
		{
			DrawDebugCylinder(Location, Location + BeamDir * 5000, CurBeamScale*BaseDamageRadius, 16, 255, 64, 64, FALSE);
		}

		// make beam do some damage to some folks

		// Add additional trace below beam location if desired
		HurtEnd = Location;
		if(bHurtBelowBeam)
		{
			HurtEnd -= BeamDir * 1000;
		}

		HurtBeam(BaseDamagePerSecond*DeltaTime*CurBeamScale, HurtEnd, Location + BeamDir * 5000, CurBeamScale*BaseDamageRadius, BeamDamageType);

		if (WorldInfo.NetMode != NM_DedicatedServer)
		{
			class'GearPlayerCamera'.static.PlayWorldCameraShake(HODCamShake, self, Location, HODCamShakeInnerRadius, HODCamShakeOuterRadius, 2.0f, TRUE );
		}

		// determine if hitactor is something that gets special impact effects
		if (HitActor != None)
		{
			if( HitActor.Tag == 'LambentBrumak' )
			{
				GatherEntryAndExitLocations( HitActor );
			}

			if (HitActor.IsA('GearPawn_LocustBerserker'))
			{
				bHitBerserker = TRUE;
			}
			else if (   HitActor.IsA('GearPawn')
					 || HitActor.IsA('DynamicSMActor')
					 || HitActor.IsA('SkeletalMeshActor')
					 || HitActor.IsA('GearDestructibleObject'))
			{
				bNoGroundFX = TRUE;
			}
			// Do a triangle of extra traces if hit a static mesh
			// If vertical offset is too much, don't draw ground effects
			else if( HitActor.IsA('StaticMeshActor') )
			{
				FlatDist = 32.f;

				if( !bNoGroundFX )
				{
					StartTrace = Location;
					StartTrace.X += FlatDist;
					FindGround(StartTrace, 500.f, HitLoc, HitNormal, HitActor);

					if( HitActor == None || Abs(HitLoc.Z-Location.Z) > 16.f )
					{
						bNoGroundFX = TRUE;
					}
				}
				if( !bNoGroundFX )
				{
					StartTrace = Location;
					StartTrace.X -= FlatDist * 0.5f;
					StartTrace.Y += FlatDist * 0.5f;
					FindGround(StartTrace, 500.f, HitLoc, HitNormal, HitActor);

					if( HitActor == None || Abs(HitLoc.Z-Location.Z) > 16.f )
					{
						bNoGroundFX = TRUE;
					}
				}
				if( !bNoGroundFX )
				{
					StartTrace = Location;
					StartTrace.X -= FlatDist * 0.5f;
					StartTrace.Y -= FlatDist * 0.5f;
					FindGround(StartTrace, 500.f, HitLoc, HitNormal, HitActor);

					if( HitActor == None || Abs(HitLoc.Z-Location.Z) > 16.f )
					{
						bNoGroundFX = TRUE;
					}
				}
			}
		}

		SetEntryEffectEnabled( EntryEffectLocation.Length != 0 );
		SetExitEffectEnabled( ExitEffectLocation.Length != 0 );

		if (bHitBerserker)
		{
			if (!bPlayingBerserkerImpactEffect)
			{
				// turn on
				PSC_BerserkerImpact.ActivateSystem();
				bPlayingBerserkerImpactEffect = TRUE;
			}
			// break trail
			bImpactTrailIntact = FALSE;
		}
		else if (bNoGroundFX)
		{
			// break trail
			bImpactTrailIntact = FALSE;
		}
		else
		{
			if (bPlayingBerserkerImpactEffect)
			{
				// turn off
				PSC_BerserkerImpact.DeActivateSystem();
				bPlayingBerserkerImpactEffect = FALSE;
			}

			// OXM HACK to spawn trail emitters
			if ( (bOXMHack) && (WorldInfo.NetMode != NM_DedicatedServer) )
			{
				LeftOverSpawnTime_OXMHACK += DeltaTime;

				if (bImpactTrailIntact)
				{
					BeamSpeed = VSize(Location - LastActualPos) / DeltaTime;
					SpawnRate_OXMHACK = FMax(MinEmitterSpawnRate_OXMHACK, (ImpactEmitterSpawnRate_OXMHACK * FClamp(BeamSpeed / MaxBeamTrackingVel, 0.f, 1.f)));
					NumToSpawn = LeftOverSpawnTime_OXMHACK * SpawnRate_OXMHACK;		// truncates to int
					LeftOverSpawnTime_OXMHACK -= NumToSpawn / SpawnRate_OXMHACK;

					//`log("Spawning"@NumToSpawn@"emitters this frame!"@WorldInfo.TimeSeconds@SpawnRate_OXMHACK@BeamSpeed);
					for (Idx=0; Idx<NumToSpawn; Idx++)
					{
						SpawnLoc = LastActualPos + (Location - LastActualPos) * (Idx / NumToSpawn);
						//DrawDebugCoordinateSystem(SpawnLoc, TmpRot, 20.f, TRUE);
						//@todo use the object pool for this
						NewImpactEmitter_OXMHACK = Spawn(class'SpawnedGearEmitter',,, SpawnLoc, TmpRot);
						NewImpactEmitter_OXMHACK.SetTemplate(PS_GroundDamage, TRUE);		// will die when done
						NewImpactEmitter_OXMHACK.LifeSpan = 60.0f;
					}
				}
				else
				{
					// just transition from not doing trail to doing trail, reset interpolation-from-last-frame
					// parameters and wait till next frame to get it going agsin.
					LeftOverSpawnTime_OXMHACK = 0;
				}

				bImpactTrailIntact = TRUE;
			}
		}

		// scale volume of ripped-earth sound with the velocity of the beam
		if (RippingEarthLoopSound != None)
		{
			BeamSpeed = VSize(Location - LastActualPos) / DeltaTime;
			RippingEarthLoopSound.VolumeMultiplier = FInterpTo(RippingEarthLoopSound.VolumeMultiplier, FClamp(BeamSpeed / RippingEarthBeamSpeedForMaxVolume, 0.f, 1.f), DeltaTime, RippingEarthVolumeInterpSpeed);
		}
	}	

	// tick the tendril beams
	for (Idx=0; Idx<TendrilBeams.Length; ++Idx)
	{
		bTendrilNeedsActivated = FALSE;

		if (TendrilBeams[Idx].TimeUntilSpawn > 0.f)
		{
			// waiting to spawn
			TendrilBeams[Idx].TimeUntilSpawn -= DeltaTime;
		}
		else
		{
			if (TendrilBeams[Idx].bReadyToSpawn == TRUE)
			{
				// timer is <= 0 and beam is ready, so spawn!
				SpawnTendrilBeam(Idx);
				bTendrilNeedsActivated = TRUE;
			}

			if ( (TendrilBeams[Idx].BeamEmitter != None) || TendrilBeams[Idx].bReadyToSpawn )
			{
				Pct = 1.f - TimeSince(WarmupEffectsStartedTime) / WarmupTimeSec;
				Pct = FClamp(Pct, 0.f, 1.f);

				TendrilBaseLoc = Location + TendrilBeams[Idx].OffsetDir * Pct * TendrilInitialRadius;

				// note, assuming vertical tendrils here
				FindGround(TendrilBaseLoc, 5000.f, HitLoc, HitNormal, HitActor);

				TmpRot = rotator(HitNormal);
				TmpRot.Pitch = NormalizeRotAxis(TmpRot.Pitch - 16384);

				TendrilBeams[Idx].BeamEmitter.SetLocation(HitLoc);
				TendrilBeams[Idx].SurfaceImpactEmitter.SetLocation(HitLoc);
				TendrilBeams[Idx].SurfaceImpactEmitter.SetRotation(TmpRot);

				if (bTendrilNeedsActivated)
				{
					ActivateTendrilBeam(Idx);
				}


				//DrawDebugSphere(TendrilBeams[Idx].BeamEmitter.Location, 64, 12, 255, 32, 32, FALSE);
				//`log("TICK"@Idx@Pct@TendrilBeams[Idx].OffsetDir@TendrilBeams[Idx].BeamEmitter.Location@GetTimerCount('NotifyWarmupFinished')@WarmupTimeSec);
			}
		}
	}

	if (bTargetIsInvalid)
	{
		InvalidTargetTime += DeltaTime;
		if ((InvalidTargetTime > MaxInvalidTargetTime) && IsWarmingUp() && (bDoNotFire == FALSE))
		{
			// fade out the visual effect somehow?
			LifeSpan = 0.2f;
			WarmupSound.FadeOut(0.2f, 0.f);
			bDoNotFire = TRUE;
		}
	}

	if (bBeamLightFadingAway)
	{
		BeamLight.SetLightProperties(BeamLight.Brightness - DeltaTime * BeamLightFadeRate);

		if (BeamLight.Brightness < 0.f)
		{
			BeamLight.SetEnabled(FALSE);
			bBeamLightFadingAway = FALSE;
		}
	}

	// See if we need to fracture shit
	if (bActuallyFiring)
	{
		TimeUntilNextFractureCheck -= DeltaTime;
		if(TimeUntilNextFractureCheck <= 0.0)
		{
			TimeUntilNextFractureCheck = default.TimeUntilNextFractureCheck;
			DoNearbyActorEffects();
		}
	}

	LastActualPos = Location;
}

/** Break pieces off nearby meshes every so often */
simulated function DoNearbyActorEffects()
{
	local FracturedStaticMeshActor FracActor;
	local byte bWantPhysChunksAndParticles;
	local CrowdAgent Agent;

	//DrawDebugSphere(Location, DragFractureRadius, 16, 255, 255, 255, TRUE);

	foreach CollidingActors(class'FracturedStaticMeshActor', FracActor, DragFractureRadius, Location, TRUE)
	{
		if((FracActor.Physics == PHYS_None) && FracActor.IsFracturedByDamageType(BeamDamageType))
		{
			// Make sure the impacted fractured mesh is visually relevant
			if( FracActor.FractureEffectIsRelevant( FALSE, Instigator, bWantPhysChunksAndParticles ) )
			{
				FracActor.BreakOffPartsInRadius(Location, DragFractureRadius, DragFracturePartVel, bWantPhysChunksAndParticles == 1 ? TRUE : FALSE);
			}
		}
	}

	foreach CollidingActors(class'CrowdAgent', Agent, DragFractureRadius, Location, TRUE)
	{
		Agent.TakeDamage(1000000, Instigator.Controller, Location, vect(0,0,1), class'GearGame.GDT_HOD');
	}
}

/** Sets a new target position for this beam. */
simulated function UpdateTarget(vector NewTargetPos)
{
	LastValidTargetPos = NewTargetPos;

	if (bOkToFire)
	{
		bTargetIsInvalid = FALSE;
	}
}

/** notify any nearby AI that the death ray cometh so they can consider performing evasive action */
final function WarnAI_Initial()
{
	local GearAI AI;

	foreach WorldInfo.AllControllers(class'GearAI', AI)
	{
		if (AI.Pawn != None && VSize(AI.Pawn.Location - Location) < InitialSplashDamageRadius)
		{
			AI.ReceiveChargeTargetWarning(Instigator);
		}
	}
}

/** called while the beam is active to warn AI that are in the path of the beam's movement */
final function WarnAI_BeamMovement()
{
	local GearAI AI;

	foreach WorldInfo.AllControllers(class'GearAI', AI)
	{
		if (AI.Pawn != None && PointDistToSegment(AI.Pawn.Location, Location, LastValidTargetPos) < BaseDamageRadius + AI.Pawn.GetCollisionRadius())
		{
			AI.ReceiveChargeTargetWarning(Instigator);
		}
	}
}

/** Called when the pre-warmup time is over. */
simulated function NotifyPreWarmupFinished()
{
	local float NotifyAITime;

	if (WarmupTimeSec > 0.f)
	{
		SetTimer( WarmupTimeSec, FALSE, nameof(NotifyWarmupFinished) );

		if (WorldInfo.NetMode != NM_DedicatedServer)
		{
			DoHODWarmupEffects();
		}

		if (Role == ROLE_Authority && Instigator != None)
		{
			NotifyAITime = WarmupTimeSec - 0.5 - 0.5 * FRand();
			if (NotifyAITime > 0.0)
			{
				SetTimer(NotifyAITime, false, nameof(WarnAI_Initial));
			}
			else
			{
				WarnAI_Initial();
			}
		}
	}
	else
	{
		NotifyWarmupFinished();
	}
}

simulated function DestroyTendrilBeams()
{
	local int Idx;

	// turn off tendrils
	for (Idx=0; Idx<TendrilBeams.Length; ++Idx)
	{
		if (TendrilBeams[Idx].BeamEmitter != None)
		{
			TendrilBeams[Idx].BeamEmitter.ParticleSystemComponent.DeactivateSystem();
			TendrilBeams[Idx].BeamEmitter.bDestroyOnSystemFinish = TRUE;
			TendrilBeams[Idx].BeamEmitter = none;
		}

		if (TendrilBeams[Idx].SurfaceImpactEmitter != None)
		{
			TendrilBeams[Idx].SurfaceImpactEmitter.ParticleSystemComponent.DeactivateSystem();
			TendrilBeams[Idx].SurfaceImpactEmitter.bDestroyOnSystemFinish = TRUE;
			TendrilBeams[Idx].SurfaceImpactEmitter = none;
		}
	}
	TendrilBeams.Length = 0;
}

/**
 * Turns on effects for the given tendril beam.
 */
simulated protected function ActivateTendrilBeam(int BeamIdx)
{
	TendrilBeams[BeamIdx].BeamEmitter.ParticleSystemComponent.ActivateSystem();
	TendrilBeams[BeamIdx].SurfaceImpactEmitter.ParticleSystemComponent.ActivateSystem();
}

/**
 * Spawns and sets up a tendril.  Doesn't turn on the particle systems though, because
 * final position is not yet set.  Call ActivateTendrilBeam to turn on effects.
 */
simulated protected function SpawnTendrilBeam(int BeamIdx)
{
	local rotator TmpRot;

	// spawn and set up beam emitter
	TendrilBeams[BeamIdx].BeamEmitter = Spawn(class'SpawnedGearEmitter');
	TendrilBeams[BeamIdx].BeamEmitter.LifeSpan = 0.0f;
	TendrilBeams[BeamIdx].BeamEmitter.ParticleSystemComponent.bAutoActivate = FALSE;
	TendrilBeams[BeamIdx].BeamEmitter.SetTemplate(PS_TendrilBeam, TRUE);

	// spawn and set up surface impact emitter. this one gets the passed in surface rot
	TendrilBeams[BeamIdx].SurfaceImpactEmitter = Spawn(class'SpawnedGearEmitter');
	TendrilBeams[BeamIdx].SurfaceImpactEmitter.LifeSpan = 0.0f;
	TendrilBeams[BeamIdx].SurfaceImpactEmitter.ParticleSystemComponent.bAutoActivate = FALSE;
	TendrilBeams[BeamIdx].SurfaceImpactEmitter.SetTemplate(PS_TendrilImpact, FALSE);

	TmpRot.Yaw = BeamIdx * 65535 / NumWarmupTendrils;
	TmpRot.Roll = 0.f;
	TmpRot.Pitch = 0.f;
	TendrilBeams[BeamIdx].OffsetDir = Normal(Vector(TmpRot));

	TendrilBeams[BeamIdx].TimeUntilSpawn = -1.f;
	TendrilBeams[BeamIdx].bReadyToSpawn = FALSE;
}

simulated function DoHODWarmupEffects()
{
	local int Idx;

	if (WarmupSound != None)
	{
		WarmupSound.Play();
	}

	// set spawn timers on the minibeams
	TendrilBeams.Length = NumWarmupTendrils;
	for (Idx=0; Idx<NumWarmupTendrils; ++Idx)
	{
		TendrilBeams[Idx].TimeUntilSpawn = RandRange(0.f, WarmupTimeSec*0.35f);
		TendrilBeams[Idx].bReadyToSpawn = TRUE;
	}

	// pick a random one and cause it to spawn immediately
	if (NumWarmupTendrils > 0)
	{
		TendrilBeams[Rand(NumWarmupTendrils)].TimeUntilSpawn = 0.f;
	}

	WarmupEffectsStartedTime = WorldInfo.TimeSeconds;
}


/**
 * Begin warming up a shot from the HoD satellite.
 * Beam will not actually fire until Fire() is called.
 */
simulated function BeginWarmup(vector TargetLoc, optional bool bShouldAutoFire)
{
	SetLocation(TargetLoc);
	LastValidTargetPos = TargetLoc;
	bActuallyFiring = FALSE;
	bAutoFire = bShouldAutoFire;

	if (PreWarmupTimeSec > 0.f)
	{
		SetTimer( PreWarmupTimeSec, FALSE, nameof(NotifyPreWarmupFinished) );
	}
	else
	{
		NotifyPreWarmupFinished();
	}
}

/**
 * Aborts firing cycle from wherever it is.
 */
simulated function AbortFire(optional bool bForce)
{
	// figure out where in the shot cycle we are and react accordingly.
	if (bActuallyFiring || IsWarmingUp() || bForce)
	{
		// jump to cooldown
		if (Role == ROLE_Authority)
		{
			bAborted = TRUE;
		}

		NotifyFireFinished();
		if (WarmupSound != None)
		{
			WarmupSound.FadeOut(0.2f, 0.f);
		}
		
		SetEntryEffectEnabled( FALSE );
		SetExitEffectEnabled( FALSE );
		
		DestroyTendrilBeams();
		ClearTimer('NotifyFireFinished');
		ClearTimer('NotifyWarmupFinished');
		ClearTimer('NotifyPreWarmupFinished');
	}
	else
	{
		// already in cooldown, do nothing
	}
}

/**
 * Sends this beam into 'dormant' mode.  This means it is 'off', as in
 * not actually firing, but it will stay alive indefinitely and will
 * continue to simulate/track tragets.
 */
simulated function GoDormant()
{
	bDormant = TRUE;
	bActuallyFiring = FALSE;
	BeamLight.SetEnabled(FALSE);
	LifeSpan = 0.f;
	ClearTimer('NotifyPreWarmupFinished');
	ClearTimer('NotifyWarmupFinished');
	ClearTimer('NotifyFireFinished');
	ClearTimer(nameof(WarnAI_BeamMovement));

	// stop any looping sounds
	if (FireLoopSound != None)
	{
		FireLoopSound.FadeOut(0.25f, 0.f);
	}
	if (RippingEarthLoopSound != None)
	{
		RippingEarthLoopSound.FadeOut(0.5f, 0.f);
	}

	// turn off the beam effect
	PSC_MainBeam.DeActivateSystem();
	
	SetEntryEffectEnabled( FALSE );
	SetExitEffectEnabled( FALSE );
}


simulated function NotifyFireFinished()
{
	local float CoolDownEffectTimeSec;

	CoolDownEffectTimeSec = DoHODCooldownEffects() + 0.25f;

	if (bActuallyFiring)
	{
		// don't enforce the cooldown wait period if main beam didnt fire
		CoolDownEffectTimeSec = FMax(CoolDownTimeSec, CoolDownEffectTimeSec);
		CoolDownEffectTimeSec = FMax(0.0001f, CoolDownEffectTimeSec);		// 0.f LifeSpan means "never die", and we want "die now"
	}
	else
	{
		// shot was aborted or something, only do do short cooldown
		CoolDownEffectTimeSec = 1.f;
	}

	// beam will destroy itself after cooldown time is up
	LifeSpan = (bDormant) ? 0.f : CoolDownEffectTimeSec;;

	// not firing any more
	bActuallyFiring = FALSE;
	ClearTimer(nameof(WarnAI_BeamMovement));
}

/** Returns a time, in seconds, for how long the cooldown effects will play */
simulated function float DoHODCooldownEffects()
{
	local bool bPlayedCoolDownSound;

	if (WorldInfo.NetMode != NM_DedicatedServer)
	{
		// turn off the light
		bBeamLightFadingAway = TRUE;

		// play cooldown audio
		if ( (CoolDownSound != None) && bActuallyFiring )
		{
			PlaySound(CoolDownSound, TRUE, TRUE);
			bPlayedCoolDownSound = TRUE;
		}

		// stop looping sounds
		if (FireLoopSound != None)
		{
			FireLoopSound.FadeOut(0.25f, 0.f);
		}
		if (RippingEarthLoopSound != None)
		{
			RippingEarthLoopSound.FadeOut(0.5f, 0.f);
		}

		// turn off the beam effect
		PSC_MainBeam.DeActivateSystem();

		SetEntryEffectEnabled( FALSE );
		SetExitEffectEnabled( FALSE );

		if (bPlayingBerserkerImpactEffect)
		{
			// turn off
			PSC_BerserkerImpact.DeActivateSystem();
			bPlayingBerserkerImpactEffect = FALSE;
		}
	}

	if ( (CoolDownSound == None) || !bPlayedCoolDownSound )
	{
		return 0.f;
	}
	else
	{
		return CoolDownSound.GetCueDuration();
	}
}

/**
 * Attempt to fire main beam.
 * Returns true if successful, false otherwise.
 * Server only.
 */
function bool Fire()
{
	local GearWeap_HODBase Weap;

	if (IsReadyToFire())
	{
		// test
		//`log( self@GetFuncName()@Owner@Instigator );

		Weap = GearWeap_HODBase(Instigator.Weapon);
		if( Weap != None )
		{
			Weap.NotifyBeamFired();
		}

		// turn on the death ray!
		bActuallyFiring = TRUE;
		bWarmedUp = FALSE;			// firing, not warmed up anymore

		if (WorldInfo.NetMode != NM_DedicatedServer)
		{
			DoHODFireEffects();
		}

		if (Role == ROLE_Authority)
		{
			SetTimer(1.0, true, nameof(WarnAI_BeamMovement));
		}

		// initial splash damage
		bDoInitialExplosionEffect++;
		DoInitialExplosion();

		SetTimer( FireDurationSec, FALSE, nameof(NotifyFireFinished) );

		return TRUE;
	}

	return FALSE;
}

simulated function DoInitialExplosion()
{
	local GearExplosionActor ExplosionActor;
	local vector NudgedHitLocation;
	local GearWeapon GW;

	NudgedHitLocation = Location + vect(0,0,16);

	ExplosionActor = Spawn(class'GearExplosionActor',,, NudgedHitLocation, rot(16384,0,0));
	if (ExplosionActor != None)
	{
		ExplosionActor.InstigatorController = Instigator.Controller;

		InitialExplosionTemplate.Damage = InitialSplashDamage;
		InitialExplosionTemplate.DamageRadius = InitialSplashDamageRadius;
		InitialExplosionTemplate.DamageFalloffExponent = InitialSplashDamageFalloff;
		InitialExplosionTemplate.MomentumTransferScale = InitialSplashDamageMomemtum;

		// respect bSuppressDamage if necessary
		if( Instigator != None )
		{
			GW = GearWeapon(Instigator.Weapon);
			if (GW != None && GW.bSuppressDamage)
			{
				InitialExplosionTemplate.Damage = 0.0;
			}
		}

		// these are needed for the decal tracing later in GearExplosionActor.Explode()
		ExplosionActor.bActiveReloadBonusActive = GearPawn(Instigator) != None ? GearPawn(Instigator).bActiveReloadBonusActive : FALSE;
		InitialExplosionTemplate.HitActor = None;
		InitialExplosionTemplate.HitLocation = NudgedHitLocation;
		InitialExplosionTemplate.HitNormal = vect(0,0,1);
		ExplosionActor.Explode(InitialExplosionTemplate);		// go bewm
	}
}


/** Returns true if beam is ready to fire the main shot, false otherwise. */
simulated protected function bool IsReadyToFire()
{
	return ( bWarmedUp && !bTargetIsInvalid && !bDoNotFire );
}

/**
 * Called once the targeting beam has been on long enough for the death ray to
 * acquire the target and actually fire
 */
simulated function NotifyWarmupFinished()
{
	local bool bFiredOK;

	if (FireDurationSec > 0.f)
	{
		bWarmedUp = TRUE;

		if ( (bAutoFire) || (bOkToFire) )
		{
			// bewm!
			bFiredOK = Fire();
			if (!bFiredOK)
			{
				// die
				LifeSpan = 0.01f;
			}
		}
	}
	else
	{
		NotifyFireFinished();
	}
}

simulated function DoHODFireEffects()
{
	BeamLight.SetLightProperties(BeamLight.default.Brightness);
	BeamLight.SetEnabled(TRUE);
	bBeamLightFadingAway = FALSE;		// just to be sure

	//@todo make this some sort of looping sound
	if (FireLoopSound != None)
	{
		FireLoopSound.Play();
	}
	if (RippingEarthLoopSound != None)
	{
		RippingEarthLoopSound.Play();
	}
	if (ImpactSound != None)
	{
		PlaySound(ImpactSound);
	}

	// turn on the beam effect
	PSC_MainBeam.ActivateSystem();

	// turn off tendrils
	DestroyTendrilBeams();

	//`log("destryed all beams doing fire effects!  remaining:"@TendrilBeams.Length);
}

simulated function Destroyed()
{
	local int Idx;

	DestroyTendrilBeams();

	for( Idx = 0; Idx < EntryFX.Length; Idx++ )
	{
		if (EntryFXActive[Idx] == 0)
		{
			EntryFX[Idx].Destroy();
		}
		else
		{
			EntryFX[Idx].bDestroyOnSystemFinish = TRUE;
			EntryFX[Idx].ParticleSystemComponent.DeactivateSystem();
			EntryFX[Idx].LifeSpan = 5.f;		// failsafe
		}
		EntryFX[Idx] = None;
	}
	EntryFX.length = 0;

	for( Idx = 0; Idx < ExitFX.Length; Idx++ )
	{
		if (ExitFXActive[Idx] == 0)
		{
			ExitFX[Idx].Destroy();
		}
		else
		{
			ExitFX[Idx].bDestroyOnSystemFinish = TRUE;
			ExitFX[Idx].ParticleSystemComponent.DeactivateSystem();
			ExitFX[Idx].LifeSpan = 5.f;		// failsafe
		}
		ExitFX[Idx] = None;
	}
	ExitFX.length = 0;

	//`log("destryed all beams in destroyed()!  remaining:"@TendrilBeams.Length);

	Super.Destroyed();
}


/** Returns true if beam is in it's pre-shot warmup sequence */
simulated function bool IsWarmingUp()
{
	return IsTimerActive('NotifyWarmupFinished') || IsTimerActive('NotifyPreWarmupFinished');
}


/**
 * Hurts all Pawns touching the specified beam.
 * Similar conceptually to HurtRadius, except with a different shape.
 */
simulated function HurtBeam
(
	float				BaseDamage,
	vector				CylinderEndPt1,
	vector				CylinderEndPt2,
	float				CylinderRadius,
	class<GearDamageType>	DamageType
 )
{
	local Actor Victim;
	local vector HitLoc, HitNorm, Extent;
	local TraceHitInfo HitInfo;
	local FracturedStaticMeshActor FracActor;

	//FlushPersistentDebugLines();

	if(!bZeroExtentBeam)
	{
		Extent.X = CylinderRadius;
		Extent.Y = CylinderRadius;
		Extent.Z = 1.0;
	}

	//`log("TRACE!"@Extent);
	//DrawDebugLine(vect(0,0,0), CylinderEndPt1, 0,255,0, TRUE);
	//DrawDebugLine(vect(0,0,0), CylinderEndPt2, 0,255,0, TRUE);
	//DrawDebugLine(CylinderEndPt1, CylinderEndPt2, 0,0,255, TRUE);
	foreach TraceActors(class'Actor', Victim, HitLoc, HitNorm, CylinderEndPt1, CylinderEndPt2, Extent, HitInfo)
	{
		if(Victim != self)
		{
			if ( (Role == ROLE_Authority) && (Victim.Role == ROLE_Authority) )
			{
				//DrawDebugSphere(HitLoc, 16, 12, 255, 16, 16, TRUE);
				//`log("hurting"@Victim@BaseDamage@"points of damage");
				Victim.TakeDamage( BaseDamage, Instigator.Controller, HitLoc, vect(0,0,1), DamageType, HitInfo, self );
			}

			// Handle hitting FSMAs with HOD
			FracActor = FracturedStaticMeshActor(Victim);
			if((FracActor != None) && (FracActor.Physics == PHYS_None))
			{
				FracActor.BreakOffPartsInRadius(HitLoc, 2.0*CylinderRadius, 500.0, true);
			}
		}
	}
}

simulated function bool WasInstigatedBy(Pawn P)
{
	return (Instigator == P) ? TRUE : FALSE;
}

simulated function NotifyInvalidTarget()
{
	if (bTargetIsInvalid == FALSE)
	{
		bTargetIsInvalid = TRUE;
		InvalidTargetTime = 0.f;
	}
}


defaultproperties
{
	BeamDamageType=class'GDT_HOD'

	bOkToFire=TRUE					// this was used for HOD iterations where the player controlled fire time
	MaxBeamScale=1.f

	MaxInvalidTargetTime=0.0f

	TendrilInitialRadius=200

	HODCamShake=(ShakeName="x",TimeDuration=1.5f,FOVAmplitude=0,LocAmplitude=(X=1,Y=1,Z=1),LocFrequency=(X=5,Y=50,Z=100),RotAmplitude=(X=150,Y=150,Z=150),RotFrequency=(X=100,Y=60,Z=120))
	HODCamShakeInnerRadius=256
	HODCamShakeOuterRadius=2048

	RippingEarthBeamSpeedForMaxVolume=512
	RippingEarthVolumeInterpSpeed=2

	// stuff from actor
	RemoteRole=ROLE_SimulatedProxy
	bCanBeDamaged=FALSE
	bReplicateMovement=TRUE
	bUpdateSimulatedPosition=TRUE
	bAlwaysRelevant=TRUE
	bReplicateInstigator=true

	//point light
    Begin Object Class=PointLightComponent Name=PointLightComponent0
	Radius=200.f
	Brightness=48.f
	LightColor=(B=80,G=128,R=255,A=255)
		Translation=(Z=96)
		CastShadows=TRUE
		CastStaticShadows=FALSE
		CastDynamicShadows=TRUE
		bCastCompositeShadow=FALSE
		bForceDynamicLight=FALSE
		bEnabled=FALSE
    End Object
	BeamLight=PointLightComponent0
	Components.Add(PointLightComponent0)
	BeamLightFadeRate=120.f

	// main beam effects
	Begin Object Class=ParticleSystemComponent Name=PSC_MainBeam0
		bAutoActivate=FALSE
	End Object
	PSC_MainBeam=PSC_MainBeam0
	Components.Add(PSC_MainBeam0)

	ImpactEmitterSpawnRate_OXMHACK=10
	MinEmitterSpawnRate_OXMHACK=2
	bOXMHack=TRUE

	Begin Object Class=ParticleSystemComponent Name=PSC_BerserkerImpact0
		bAutoActivate=FALSE
	End Object
	PSC_BerserkerImpact=PSC_BerserkerImpact0
	Components.Add(PSC_BerserkerImpact0)

	TimeUntilNextFractureCheck=0.5
	DragFractureRadius=100.0
	DragFracturePartVel=500.0
}



