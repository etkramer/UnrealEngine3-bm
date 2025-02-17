/**
 * GearProj_Cannon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_Cannon extends GearProj_Grenade
	config(Weapon);

var	SoundCue FlyBySound;
var SoundCue ImpactFleshSound;
var SoundCue ImpactStickSound;
var SoundCue BounceSound;

/** percentage of how much weapon was charged. 0 -> 1.0 */
var repnotify float ChargePct;

/** Time in seconds before projectile explodes, after impact. */
var() config float	TimeBeforeExplosion;


/** Normal vector of point of impact. To spawn effects */
var()	Vector	ImpactNormal;

/** True if this arrow has ever collided with anything */
var bool		bHasCollided;

/** X=min speed (0 charge), Y=max speed (full charge)) */
var() config vector2d	SpeedRange;

/** The GearPawn this arrow is stuck in, if any */
var GearPawn		StuckInPawn;

/** Damage type for this arrow if it is stuck in a pawn */
var class<DamageType>	   StuckInPawnDamageType;

var bool bAllowBounce;
var bool bAllowStickInPawn;
var byte BounceCnt;

/** This fires off when it embeds into a wall or ground **/
var ParticleSystem PS_ArrowImpactEffect;

/** This fires off when it embeds into a human **/
var ParticleSystem PS_ArrowImpactEffectHuman;

/** Attached sparks that we need to hide once this arrow blows up **/
var Emitter Sparkies;

/** Don't want HandleCollision() to be re-entrant (it does a setlocation()) */
var bool bProcessingCollision;

var RB_ConstraintActorSpawnable	VictimConstraint;

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
		Sparkies.SetTimer( 0.01, FALSE, nameof(Sparkies.HideSelf) );
		Sparkies = none;
	}


	TriggerExplosion(Location, ExploNormal, None);
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
	local GearWeap_Cannon	Bow;

	if ( Role == ROLE_Authority )
	{
		Bow = GearWeap_Cannon(Owner);
		bAllowBounce = true;
		//bAllowBounce = Bow.ChargeTimeCount < Bow.StickChargeTime;
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
	//local float Interp;

	// cubic seems to feel nice
	//Interp = ChargePct * ChargePct;

	if( Role == Role_Authority )
	{
		// on server, set speed. Client will have updated replicated velocity.
		//Speed = SpeedRange.X + ( Interp * (SpeedRange.Y - SpeedRange.X) );
		Speed = SpeedRange.X;
	}

	// adjust gravity scale.
	//GravityScale	= (1 - Interp) * default.GravityScale;
	//RotationRate	= (1 - Interp) * default.RotationRate;
}

simulated function Bounce(Vector HitLocation, Vector HitNormal, Actor Other)
{
	Velocity -= (1.f + Bounciness) * HitNormal * (Velocity Dot HitNormal);
	Velocity *= VelocityDampingFactor;

	GravityScale += 0.4;
	GravityScale = FMin(GravityScale, 1.f);

	PlaySound(BounceSound, true);
}

/** Handle arrow collision */
simulated function HandleCollision(Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal)
{
	local GearPawn			HitPawn;
	local GearDestructibleObject	GDO;
	local GearWeapon		HitWeapon;
	local TraceHitInfo	HitInfo;
	local Emitter		AnEmitter;

	//`log( WorldInfo.TimeSeconds @ GetFuncName() @ Other );
	if( bHitPhysicsVolume || bProcessingCollision || Other == None || Other == Instigator || Other.bTearOff)
	{
		// shouldn't collide with instigator
		return;
	}

	HitPawn		= GearPawn(Other);

	// If we hit a pawn...
	if(HitPawn != None)
	{
		// If this is the first pawn we hit, make a joint to it.
		if(StuckInPawn == None)
		{
			SetLocation(HitLocation + ImpactNormal * 4.f);
			HitPawn.PlayFallDown();
			StuckInPawn = HitPawn;			// ok if this is None
		}

		// Hitting a pawn does not result in a bounce.
		return;
	}

	// cannons knock right through destructibles
	GDO = GearDestructibleObject(Other);
	if(GDO != None)
	{
		HitInfo.HitComponent = OtherComp;
		GDO.TakeDamage(9999.f, InstigatorController, HitLocation, Normal(Velocity), class'GearGame.GDT_Ballistic', HitInfo);
		GDO.bWorldGeometry = false;
		return;
	}

	if(!Other.bWorldGeometry)
	{
		return;
	}

	// When hitting a static thing (eg a wall) whilst dragging a ragdoll - stop projectile and blow up
	if(StuckInPawn != None)
	{
		`log("STOPPING: HIT:"@Other);
		LifeSpan = 0.5;
		SetPhysics(PHYS_None);
		return;
	}

	bProcessingCollision = true;
	bHasCollided				= TRUE;
	bRotationFollowsVelocity	= FALSE;


	// Touch() happens after a full movement, so HitLocation is not Location of the impact.
	// So move Arrow back to HitLocation, and add push it back a little more, so effect is visible.
	//if( (HitPawn != None || Other.bProjTarget || Other.bWorldGeometry || Other.bBlockActors) && Location != HitLocation )
	//{
		//SetLocation(HitLocation + ImpactNormal * 4.f);
	//}

	if( ShouldBounce(Other, HitNormal) )
	{
		/*
		if( HitPawn != None )
		{
			// dampen the velocity more when bouncing off pawns
			VelocityDampingFactor = default.VelocityDampingFactor * 0.5f;
		}
		else
		{
			VelocityDampingFactor = default.VelocityDampingFactor;// * 0.7f;
		}
		*/

		Bounce(HitLocation, HitNormal, Other);
		BounceCnt++;

		if( vsize(Velocity) < 350.f)
		{
			SetPhysics(PHYS_RigidBody);
		}
	}
	else
	{
		HitWeapon	= GearWeapon(Other);

		if( HitPawn != None || Other.bProjTarget || Other.bWorldGeometry || Other.bBlockActors )
		{
			//`log(" Hit Something");
			ImpactNormal = HitNormal;

			SetPhysics(PHYS_None);

			if( !bIsSimulating )
			{
				LifeSpan = TimeBeforeExplosion;
				bCollideWorld = FALSE;

				// stick into whatever we hit
				if( HitPawn != None )
				{
					CheckHitInfo( HitInfo, HitPawn.Mesh, Normal(Velocity), HitLocation );
					SetCollision(FALSE, FALSE);
					HitPawn.TakeDamage(45.f,InstigatorController,HitLocation,vect(0,0,0),class'GDT_Ballistic',HitInfo);
					SetBase(Other,, SkeletalMeshComponent(HitInfo.HitComponent), HitInfo.BoneName);
					StuckInPawn = HitPawn;			// ok if this is None

					Sparkies = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( PS_ArrowImpactEffectHuman, HitLocation, Rotation );
					Sparkies.SetBase(Other);
					Sparkies.ParticleSystemComponent.ActivateSystem();
					// this is semi lame hack as it is some how possible to have the sparkies get attached
					// to a pawn in between the target and yourself  and then those sparkies will never
					// go away.  So we just set the sparkies to hide themselves at the time of the explosion
					Sparkies.SetTimer( TimeBeforeExplosion, FALSE, nameof(Sparkies.HideSelf) );
				}
				else
				{
					// stick to whatever we hit.
					bCollideWorld = false;
					SetBase(Other);
					AnEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( PS_ArrowImpactEffect, HitLocation, Rotation );
					AnEmitter.SetBase(Other);
					AnEmitter.ParticleSystemComponent.ActivateSystem();
				}
			}

			ImpactedActor = Other;
		}

		if (!bIsSimulating && Role == Role_Authority)
		{
			// Play Impact sounds
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
	}

	if( !bIsSimulating && ( InFlightSound != none ) )
	{
		//`log("Stoping");
		InFlightSound.FadeOut(0.5f,0.0f);
	}
	bProcessingCollision = false;
}


/**
 * Test condition to validate a rebound
 *
 * @param	Touched, actor the grenade touched
 * @param	Inst, Instigator (Pawn who did throw the grenande)
 * @param	bounceNb, number of bounces
 * @return	true if the grenade should bounce off
 */
simulated function bool ShouldBounce(Actor Touched, vector HitNormal, optional PrimitiveComponent HitComponent)
{
	local GearPawn P;

	return TRUE;

	P = GearPawn(Touched);
	if ( !bAllowStickInPawn && P != None )
	{
		return TRUE;
	}

	// If Shooter is NOT human controlled AND 
	// Target cover is protecting the angle
	if( P != None &&
		!Instigator.IsHumanControlled() && 
		P.IsProtectedByCover( Normal(P.Location-Instigator.Location) ) )
	{
		// Bounce
		return TRUE;
	}

	return (bAllowBounce);
}

/**
 * Grenade explodes
 * Made static, so can be called when cooking grenade, and player failed to throw it in time
 *
 * @param	HitLocation
 * @param	HitNormal
 */
simulated function GrenadeExplosion(vector HitLocation, vector HitNormal, bool bOnGround)
{
	local float				ExploDmg, ExploRadius, ExploRadiusMultiplier;
	local GearWeapon			WW;
	local class<DamageType>	DamType;

	//ExplosionInstigator.DrawDebugCoordinateSystem(HitLocation, Rotator(HitNormal), 50.f, TRUE);

	WW = GearPawn(Instigator).MyGearWeapon;
	ExploRadiusMultiplier = (WW != None) ? WW.GetExploRadiusMultiplier() : 1.f;

	if ( StuckInPawn != None )
	{
		DamType = StuckInPawnDamageType;
		ImpactedActor = StuckInPawn;
		ExploDmg = 99999.f;
		// the thinking here is that the victim absorbs most of the blast.. internally.
		ExploRadius = ExploDamageRadius * ExploRadiusMultiplier * 0.5f;
	}
	else
	{
		DamType = MyDamageType;
		ExploDmg = ExploDamage * ((WW != None) ? WW.GetActiveReloadDamageMultiplier() : 1.f);
		ExploRadius = ExploDamageRadius * ExploRadiusMultiplier;
	}

	//`log("ExploRadius"@ExploRadius@ExploRadius/default.InnerCoreDamageRadius);

	// @fixme jf: set up with new explosion system if this leaves prototype stage
	//if( WorldInfo.NetMode != NM_DedicatedServer )
	//{
	//	if( StuckInPawn != None )
	//	{
	//		if( GearGRI(WorldInfo.GRI).ShouldShowGore() )
	//		{
	//			Spawn(class'FX_BowArrowExplosionGore',,, HitLocation, rotator(HitNormal));
	//		}
	//		else
	//		{
	//			Spawn(class'FX_BowArrowExplosionInAir',,, HitLocation, rotator(HitNormal));
	//		}
	//	}
	//	else if( bOnGround )
	//	{
	//		Spawn(class'FX_BowArrowExplosion',,, HitLocation, rotator(HitNormal));
	//	}
	//	else
	//	{
	//		Spawn(class'FX_BowArrowExplosionInAir',,, HitLocation, rotator(HitNormal));
	//	}
	//}

	if( Role == Role_Authority )
	{
		if( GearAI(InstigatorController) != none )
		{
			// @hack jf for compiling
			//HurtRadius( InnerCoreDamageAI, InnerCoreDamageRadius, DamType, MomentumTransfer, HitLocation,, InstigatorController );
		}
		else
		{
			HurtRadius( ExploDmg, ExploRadius, DamType, MomentumTransfer, HitLocation, ,InstigatorController );
		}

		MakeNoise( 1.f );

		`if(`notdefined(FINAL_RELEASE))
		if( GearPawn(Instigator).bWeaponDebug_DamageRadius )
		{
			FlushPersistentDebugLines();
			DrawDebugSphere(HitLocation, ExploRadius, 16, 255, 16, 16, TRUE);
		}
		`endif
	}
}


defaultproperties
{
	bNetTemporary=FALSE
	bHardAttach=TRUE
	Lifespan=10.0
	MaxBounceCount=8
	MyDamageType=class'GDT_Ballistic'
	StuckInPawnDamageType=class'GDT_FragGrenade'
	GravityScale=0.f
	bRotationFollowsVelocity=true
	bAllowStickInPawn=true
	bBlockActors=false
	//bCollideWorld=false

	MomentumTransfer=1.f	// Scale momentum defined in DamageType
	VelocityDampingFactor=0.6
	Bounciness=0.9
	StopSimulatingVelocityThreshhold=1

	ImpactSound=SoundCue'Weapon_TorqueBow.Weapons.Crossbow_ArrowImpactWallCue'
	ImpactFleshSound=SoundCue'Weapon_TorqueBow.Weapons.Crossbow_ArrowImpactFleshCue'
	ImpactStickSound=SoundCue'Weapon_Grenade.Actions.GrenadeStick01Cue'
	//FlyBySound=SoundCue'Weapons.Bow.BowArrowFlyByCue'

	bCollideComplex=TRUE	// Ignore simple collision on StaticMeshes, and collide per poly

	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
    	//StaticMesh=StaticMesh'Locust_Torquebow.Locust_Torquebow_Rocket'
		StaticMesh=StaticMesh'Proto_Cannon.CannonBall'
		CollideActors=TRUE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=TRUE
		bDisableAllRigidBody=TRUE
		Scale=0.1
		Scale3D=(X=1.0,Y=1.0,Z=1.0)
		Translation=(X=0,Y=0,Z=0)
		RBCollideWithChannels=(Default=TRUE,BlockingVolume=TRUE)
    End Object
	Mesh=StaticMeshComponent0
	CollisionComponent=StaticMeshComponent0
	Components.Add(StaticMeshComponent0)

	// remove light environment
	Begin Object Name=MyLightEnvironment
	    bEnabled=FALSE
	End Object
	Components.Remove(MyLightEnvironment)
	ProjLightEnvironment=None


	TrailTemplate=ParticleSystem'Locust_Torquebow.EffectS.P_Torquebow_Arrow_Trail'

	BounceSound=SoundCue'Weapon_TorqueBow.Weapons.Crossbow_ArrowImpactRicochetCue'
	InFlightSoundTemplate=none;

	PS_ArrowImpactEffect=ParticleSystem'Locust_Torquebow.Effects.PS_Torquebow_Impact'
	PS_ArrowImpactEffectHuman=ParticleSystem'Locust_Torquebow.Effects.P_Torquebow_Sparks'

}
