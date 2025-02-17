/**
 * GearProj_Arrow
 * Projectile tailored for Torque Bow weapon.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_ArrowBase extends GearProj_Grenade
	abstract
	config(Weapon);

var	protected const SoundCue FlyBySound;
var protected const SoundCue ImpactFleshSound;
var protected const SoundCue ImpactStickSound;
var protected const SoundCue RicochetSound;
var protected const SoundCue BounceSound;
var protected transient float LastBounceSoundTime;
var() protected const float RicochetSoundThresh;

var transient array<Actor> TemporarilyIgnoredActors;

/** percentage of how much weapon was charged. 0 -> 1.0 */
var protected repnotify transient float ChargePct;

/** Time in seconds before projectile explodes, after impact. */
var() protected const config float	TimeBeforeExplosion;

/** Normal vector of point of impact. To spawn effects */
var transient vector				ImpactNormal;

/** True if this arrow has ever collided with anything */
var protected transient bool		bHasCollided;

/** X=min speed (0 charge), Y=max speed (full charge)) */
var() protected config vector2d		SpeedRange;

/** The GearPawn this arrow is stuck in, if any */
var protected transient GearPawn	StuckInPawn;

/** Damage type for this arrow if it is stuck in a pawn */
var protected const class<GearDamageType>	StuckInPawnDamageType;

var transient bool				bAllowBounce;
/** True if the arrow may stick in a pawn, false otherwise. */
var bool						bAllowStickInPawn;
var protected transient byte	BounceCnt;

/** This fires off when it embeds into a wall or ground **/
var protected const ParticleSystem PS_ArrowImpactEffect;

/** This fires off when it embeds into a human **/
var protected const ParticleSystem PS_ArrowImpactEffectHuman;

/** Attached sparks that we need to hide once this arrow blows up **/
var protected transient Emitter Sparkies;

/** Don't want HandleCollision() to be re-entrant (it does a setlocation()) */
var protected transient bool bProcessingCollision;

/** Explosion to use if we're stuck in a pawn. */
var() protected const ParticleSystem	StuckInPawnExplosionEmitterTemplate;
var() protected const ParticleSystem	StuckInPawnExplosionNoGoreEmitterTemplate;

/** Explosion sound for explosions that happen when the arrow is stuck in a pawn. */
var() protected const SoundCue			StuckInPawnExplosionSound;

/** Effect to play when grenade explodes in the air */
var() protected const ParticleSystem	InAirExplosionEmitterTemplate;
/** Explosion sound for explosions that happen when the arrow is mid-air. */
var() protected const SoundCue			InAirExplosionSound;

replication
{
	if (bNetInitial && Role == ROLE_Authority)
		ChargePct, bAllowBounce;
}

simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	if( bDeleteMe )
	{
		return;
	}

	if( FlyBySound != None && WorldInfo.NetMode != NM_DedicatedServer )
	{
		PlaySound(FlyBySound,,, TRUE);
	}
}


/**
 * Figure out how to align explosion effects.
 */
simulated function Destroyed()
{
	local vector ExploNormal;

	// If we are being destroyed because we hit a physicsvolume - dont play explosion
	if(bHitPhysicsVolume)
	{
		return;
	}

	if( !IsZero(ImpactNormal) )
	{
		ExploNormal = ImpactNormal;
	}
	else if( VSize(Velocity) > 1 )
	{
		ExploNormal = Normal(Velocity) * -1.f;
	}
	else
	{
		ExploNormal = vect(0,0,1);
	}

	/*
	if( !bIsSimulating )
	{
		DrawDebugCoordinateSystem(Location, Rotator(ExploNormal), 50.f, TRUE);
	}
	*/

	// hide the sparkies if we have some
	if( Sparkies != none )
	{
		Sparkies.ParticleSystemComponent.DeactivateSystem();
		Sparkies.SetBase( None );
		Sparkies.SetTimer(0.01, FALSE, 'HideSelf');
		Sparkies = none;
	}

	if (AvoidanceCylinder != None)
	{
		AvoidanceCylinder.Destroy();
		AvoidanceCylinder = None;
	}

	TriggerExplosion(Location, ExploNormal, StuckInPawn);
}


simulated event ReplicatedEvent( name VarName )
{
	if( VarName == 'ChargePct' )
	{
		// if ChargePct is set, update client physics accordingly
		InitBasedOnChargePct();
	}

	super.ReplicatedEvent( VarName );
}


simulated function Init(vector Direction)
{
	local GearWeap_BowBase Bow;

	if ( Role == ROLE_Authority )
	{
		Bow = GearWeap_BowBase(Owner);
		bAllowBounce = Bow.ChargeTimeCount < Bow.StickChargeTime;
		ChargePct = FClamp(Bow.ChargeTimeCount / Bow.MaxChargeTime, 0.f, 1.f);
		ImpactNormal = vect(0,0,0);

		// Update physics based on time weapon has been charged
		// this MUST be called before the Super call, where velocity is set.
		InitBasedOnChargePct();
	}
	BounceCount = 0;

	super.Init(Direction);
}

/** Called once ChargePct has been set, to update physics */
simulated function InitBasedOnChargePct()
{
	local float Interp;

	// cubic seems to feel nice
	Interp = ChargePct * ChargePct * ChargePct;

	if( Role == Role_Authority )
	{
		// on server, set speed. Client will have updated replicated velocity.
		Speed = SpeedRange.X + ( Interp * (SpeedRange.Y - SpeedRange.X) );
	}

	// adjust gravity scale.
	GravityScale	= (1 - Interp) * default.GravityScale;
	RotationRate	= (1 - Interp) * default.RotationRate;
}

simulated function PlayBounceSound()
{
	if (TimeSince(LastBounceSoundTime) > 0.3f)
	{
		if (VSize(Velocity) > RicochetSoundThresh)
		{
			PlaySound(RicochetSound, true);
		}
		else
		{
			PlaySound(BounceSound, true);
		}

		LastBounceSoundTime = WorldInfo.TimeSeconds;
	}
}

simulated function Bounce(Vector HitLocation, Vector HitNormal, Actor Other)
{
	PlayBounceSound();

	Velocity -= (1.f + Bounciness) * HitNormal * (Velocity Dot HitNormal);
	Velocity *= VelocityDampingFactor;
}


/** Handle arrow collision */
simulated function HandleCollision(Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal)
{
	local Pawn			HitPawn;
	local GearWeapon	HitWeapon;
	local TraceHitInfo	HitInfo;
	local Emitter		AnEmitter;
	local GearPawn		GP;

	// make sure we're not ignored this actor
	if (!bIsSimulating && TemporarilyIgnoredActors.Find(Other) != INDEX_NONE)
	{
		return;
	}

	if (StuckInPawn != None)
	{
		//@note - touches getting stacked up, so in the case of a meatshield we'll first collide with the shield
		// and then process the kidnapper touch since the cylinders overlap, which would normally cause the arrow
		// to stick again into the second touch
		//`log("Already stuck, ignoring multiple collisions");
		return;
	}

	// make sure we haven't stopped moving already
	if (Physics != PHYS_Projectile)
	{
		return;
	}

	if( bHitPhysicsVolume || bProcessingCollision || Other == None || Other == Instigator || Other.bTearOff || Other.IsA('Trigger'))
	{
		// shouldn't collide with instigator
		return;
	}

	GP = GearPawn(Other);
	if ( !bIsSimulating && GP != None && !bAllowBounce && bAllowStickInPawn && !GP.ComponentIsAnEquippedShield(OtherComp) &&
		(GearAI(InstigatorController) == None || GearAI(InstigatorController).bCanHeadshot) )
	{
		CheckHitInfo( HitInfo, GP.Mesh, Normal(Velocity), HitLocation );
		if (GP.ShouldTorqueBowArrowGoThroughMe(self, HitInfo, HitLocation, Normal(Velocity)))
		{
			// blow off the head and keep traveling
			GP.TakeDamage(9999.f,InstigatorController,HitLocation,Normal(Velocity),class'GDT_TorqueBow_Impact',HitInfo);
			return;
		}
	}

	//`log( WorldInfo.TimeSeconds @ GetFuncName() @ Other @bIsSimulating @Base@ShouldBounce(Other,HitNormal, OtherComp));

	bProcessingCollision = true;
	HitPawn		= Pawn(Other);
	bHasCollided				= TRUE;
	bRotationFollowsVelocity	= FALSE;

	// Touch() happens after a full movement, so HitLocation is not Location of the impact.
	// So move Arrow back to HitLocation, and add push it back a little more, so effect is visible.
	if( (HitPawn != None || Other.bProjTarget || Other.bWorldGeometry || Other.bBlockActors) && Location != HitLocation )
	{
		SetLocation(HitLocation + HitNormal * 4.f);
	}

	// don't simulate bounces
	if( ShouldBounce(Other, HitNormal, OtherComp) )
	{
		if (!Other.bWorldGeometry)
		{
			TemporarilyIgnoredActors.AddItem(Other);
		}

		// no sticking after a bounce
		bAllowStickInPawn = FALSE;

		if( !bIsSimulating )
		{
			if( HitPawn != None )
			{
				// dampen the velocity more when bouncing off pawns
				VelocityDampingFactor = default.VelocityDampingFactor * 0.5f;
			}
			else
			{
				VelocityDampingFactor = default.VelocityDampingFactor * 0.7f;
			}

			Bounce(HitLocation, Normal(HitNormal), Other);
			BounceCnt++;
			if (BounceCnt > 4 && Role == ROLE_Authority)
			{
				TriggerExplosion(Location,Normal(Velocity),None);
			}
		}
		else
		{
			ImpactNormal = HitNormal;
			SetPhysics(PHYS_None);
			SetBase(None);
		}
	}
	else
	{
		HitWeapon	= GearWeapon(Other);

		if( HitPawn != None || Other.bProjTarget || Other.bWorldGeometry || Other.bBlockActors )
		{
			ImpactNormal = HitNormal;

			SetPhysics(PHYS_None);
			SetBase(None);

			if( !bIsSimulating )
			{
				LifeSpan = TimeBeforeExplosion;
				bCollideWorld = FALSE;

				// stick into whatever we hit
				if( HitPawn != None )
				{
					SetCollision(FALSE, FALSE);
					HitPawn.TakeDamage(45.f,InstigatorController,HitLocation,vect(0,0,1.f),class'GDT_Ballistic',HitInfo);
					SetBase(Other,, SkeletalMeshComponent(HitInfo.HitComponent), HitInfo.BoneName);
					StuckInPawn = GearPawn(HitPawn);			// ok if this is None

					if (StuckInPawn != None)
					{
						AttacheeController = StuckInPawn.Controller;

						GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_StuckExplosiveToEnemy, Instigator, StuckInPawn, 0.3f);
					}

					// we want to leave it semi decent chance of having the sparkies be seen on guys that are around you
					if( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, Location, 4000, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistance ) )
					{
						Sparkies = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( PS_ArrowImpactEffectHuman, HitLocation, Rotation );
						Sparkies.SetBase(Other);
						Sparkies.ParticleSystemComponent.ActivateSystem();
						// this is semi lame hack as it is some how possible to have the sparkies get attached
						// to a pawn in between the target and yourself  and then those sparkies will never
						// go away.  So we just set the sparkies to hide themselves at the time of the explosion
						Sparkies.SetTimer( TimeBeforeExplosion, FALSE, 'HideSelf' );
					}
				}
				else
				{
					// stick to whatever we hit.
					bCollideWorld = false;
					SetBase(Other);
					if( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, Location, 4000, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
					{
						AnEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( PS_ArrowImpactEffect, HitLocation, Rotation );
						AnEmitter.SetBase(Other);
						AnEmitter.ParticleSystemComponent.ActivateSystem();
					}
				}

				// since we hit something broadcast an avoidme radius
				SetupAvoidanceCylinder();
			}

			ImpactedActor = Other;
		}

		if (!bIsSimulating && Role == Role_Authority)
		{
			// Play Impact sounds
			if (BounceCnt == 0)
			{
				if (HitWeapon == None && HitPawn == None)
				{
					PlaySound(ImpactSound);
				}
				else if (HitPawn != None)
				{
					PlaySound(ImpactFleshSound);
					PlaySound(ImpactStickSound);
				}
			}
			else
			{
				PlayBounceSound();
			}
		}
	}

	if( !bIsSimulating && ( InFlightSound != none ) )
	{
		InFlightSound.FadeOut(0.5f,0.0f);
	}
	bProcessingCollision = false;
}


/**
 * Test condition to validate a rebound
 *
 * @param	Touched, actor the grenade touched
 * @return	true if the grenade should bounce off
 */
simulated function bool ShouldBounce(Actor Touched, vector HitNormal, optional PrimitiveComponent HitComponent)
{
	local GearPawn P;
	local GearGRI GRI;

	P = GearPawn(Touched);
	if ( !bAllowStickInPawn && P != None )
	{
		return TRUE;
	}

	if (GearShield(Touched) != None)
	{
		return TRUE;
	}

	// If Shooter is NOT human controlled AND
	// Target cover is protecting the angle
	// always stick into hostages
	if( P != None && !P.IsAHostage() )
	{
		GRI = GearGRI(WorldInfo.GRI);
		if( (!Instigator.IsHumanControlled() && (GRI == None || !GRI.IsMultiplayerGame() || GRI.IsCoopMultiplayerGame()) &&
				P.IsProtectedByCover( Normal(P.Location-Instigator.Location) )) ||
			P.ComponentIsAnEquippedShield(HitComponent) )
		{
			// bounce
			return TRUE;
		}

		// disallow sticking in friendlies or guys we shouldn't have been able to shoot due to cover
		if (GRI != None && GRI.IsMultiPlayerGame())
		{
			GRI = GearGRI(WorldInfo.GRI);
			if ( (GRI != None && !GRI.bAllowFriendlyFire && GRI.OnSameTeam(P, Instigator)) ||
				P.MultiplayerCoverDamageAdjust(Normal(Velocity), Instigator, Location, 100.0) == 0 )
			{
				return TRUE;
			}
		}
	}

	return (bAllowBounce && BounceCnt < 3);
}


simulated protected function GetRadialDamageValues(out float outDamage, out float outRadius, out float outFalloff)
{
	super.GetRadialDamageValues(outDamage, outRadius, outFalloff);

	if (StuckInPawn != None)
	{
		// the thinking here is that the victim absorbs most of the blast.. internally.
		outRadius *= 0.5f;
	}
}

simulated function PrepareExplosionTemplate()
{
	super.PrepareExplosionTemplate();

	ExplosionTemplate.AttacheeController = None;

	// use alternate explosion if stuck in pawn
	if (StuckInPawn != None)
	{
		ExplosionTemplate.AttacheeController = AttacheeController;

		if (WorldInfo.GRI.ShouldShowGore())
		{
			ExplosionTemplate.ParticleEmitterTemplate = StuckInPawnExplosionEmitterTemplate;
		}
		else
		{
			ExplosionTemplate.ParticleEmitterTemplate = StuckInPawnExplosionNoGoreEmitterTemplate;
		}
		
		ExplosionTemplate.ExplosionSound = StuckInPawnExplosionSound;
	}
	else if (!IsOnTheGround())
	{
		ExplosionTemplate.ParticleEmitterTemplate = InAirExplosionEmitterTemplate;
		ExplosionTemplate.ExplosionSound = InAirExplosionSound;
	}

	// @fixme, material based explosion effests, like the frag grenade?
}

simulated function TriggerExplosion(vector HitLocation, vector HitNormal, Actor HitActor)
{
	local int DamageToDo;
	if (StuckInPawn != None)
	{
		// blow up the unfortunate dude we're embedded in
		ImpactedActor = StuckInPawn;

		if (Role == Role_Authority)
		{
			// do enough damage to kill most anythign in one shot, but not boomers
			DamageToDo = 1000;
			if ( (!WorldInfo.GRI.IsMultiplayerGame() || WorldInfo.GRI.IsCoopMultiplayerGame()) &&
				StuckInPawn.IsHumanControlled() &&
				GearPRI(StuckInPawn.PlayerReplicationInfo).Difficulty.default.bCanDBNO )
			{
				DamageToDo = StuckInPawn.Health - ExploDamage - 20;
			}
			StuckInPawn.TakeDamage(DamageToDo, InstigatorController, Location, vect(0,0,1), StuckInPawnDamageType,, self);
		}
	}

	// also do normal explosion
	Super.TriggerExplosion(HitLocation, HitNormal, HitActor);
}


defaultproperties
{
	bNetTemporary=FALSE
	bHardAttach=TRUE
	Lifespan=10.0
	MaxBounceCount=8
	StuckInPawnDamageType=class'GDT_TorqueBow_Explosion'
	GravityScale=1.f
	bRotationFollowsVelocity=true
	bAllowStickInPawn=true

	//MomentumTransfer=1.f	// Scale momentum defined in DamageType
	VelocityDampingFactor=0.3
	Bounciness=0.33
	StopSimulatingVelocityThreshhold=1

	bCollideComplex=TRUE	// Ignore simple collision on StaticMeshes, and collide per poly

	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
		bDisableAllRigidBody=TRUE
		Scale=1.0
		Scale3D=(X=1.0,Y=1.0,Z=1.0)
		Rotation=(Yaw=+16384)
		Translation=(X=0,Y=0,Z=0)
    End Object
	Mesh=StaticMeshComponent0
	Components.Add(StaticMeshComponent0)

	// remove light environment
	Begin Object Name=MyLightEnvironment
	    bEnabled=FALSE
	End Object
	Components.Remove(MyLightEnvironment)
	ProjLightEnvironment=None


	Begin Object Name=CollisionCylinder
		CollideActors=TRUE
	End Object

	// explosion point light
	Begin Object Class=PointLightComponent Name=ExploLight0
	    Radius=800.000000
		Brightness=1000.000000
		LightColor=(B=60,G=107,R=249,A=255)
		Translation=(X=16)
		CastShadows=FALSE
		CastStaticShadows=FALSE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		bEnabled=FALSE
	End Object

	// explosion
	Begin Object Class=GearExplosion Name=ExploTemplate0
		MyDamageType=class'GDT_TorqueBow_Explosion'
		MomentumTransferScale=1.f	// Scale momentum defined in DamageType

		ExploLight=ExploLight0
		ExploLightFadeOutTime=0.17f

		bDoExploCameraAnimShake=TRUE
		ExploShakeInnerRadius=325
		ExploShakeOuterRadius=750

		FractureMeshRadius=80.0
		FracturePartVel=300.0

		bAllowPerMaterialFX=TRUE
		DecalTraceDistance=48.0f

		bUsePerMaterialFogVolume=TRUE
	End Object
	ExplosionTemplate=ExploTemplate0

	bStoppedByGrenadeBlockingVolume=FALSE

	RicochetSoundThresh=3000
}
