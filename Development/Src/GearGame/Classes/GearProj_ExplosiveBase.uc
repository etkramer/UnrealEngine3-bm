/**
 * GearProj_ExplosiveBase
 * Base class for "explosive" projectiles.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearProj_ExplosiveBase extends GearProjectile
	config(Weapon)
	native(Weapon)
	abstract;

var() protected const bool bAutoDestroyOnExplosion;

//
// Note: some duplication of GearExplosion vars here so we can config these.
// Ideally we could mark GearExplosion vars as config and specify them in the projectile's subobject config.
//

/** Amount of damage done at the epicenter. Using this instead of Projectile.Damage so we can make it config. */
var() protected config float		ExploDamage;
/** Damage range.  Using this instead of Projectile.DamageRadius so we can make it config. */
var() protected const config float	ExploDamageRadius;
/** Defines how damage falls off.  High numbers cause faster falloff, lower (closer to zero) cause slower falloff.  1 is linear. */
var() protected const config float	ExploDamageFalloffExp;

/** radius at which people will be knocked down/ragdolled by the projectile's explosion **/
var() protected const config float	KnockDownRadius;
/** KnockDownRadius for multiplayer games. **/
var() protected const config float	KnockDownRadiusMP;
/** radius at which people will cringe from the explosion **/
var() protected const config float	CringeRadius;
/** CringeRadius for multiplayer games. */
var() protected const config float	CringeRadiusMP;

/** Damage scaling to apply if the Instigator is AI-controlled */
var() protected config const float AIDamageScale;

/** Internal.  True if this projectile has already blown up, used to ensure only a single explosion. */
var repnotify transient protected bool				bHasExploded;

/** Defines the explosion. */
var() instanced editinline GearExplosion	ExplosionTemplate;

/** Emitter class to spawn for the lingering smoke. */
var() protected const class<Emit_SmokeGrenade> SmokeEmitterClass;

/** If TRUE, ExplosionActor will be attached to InterpActor if it hits one */
var	bool	bAttachExplosionToHitMover;

replication
{
	// send initially so that if the projectile exploded on the first tick it was alive, the client handles it correctly
	// in other cases the client should be able to simulate everything
	if (bNetInitial)
		bHasExploded;
}

simulated protected function bool IsOnTheGround()
{
	local bool bOnGround;
	local vector TraceStart, TraceDest, HitLoc, HitNorm;
	local Actor TraceActor;

	// see if we are on the ground or in the air, pick an effect
	if( Physics == PHYS_None )
	{
		bOnGround = TRUE;
	}
	else
	{
		// we need to trace and see if the grenade is still doing small
		// bounces or if it is actually in the air
		TraceStart	= Location;
		TraceDest	= Location - vect(0,0,15);
		TraceActor	= Trace(HitLoc, HitNorm, TraceDest, TraceStart,,,, TRACEFLAG_PhysicsVolumes );
		bOnGround	= TraceActor != None;
	}

	return bOnGround;
}


/** Returns values for radial damage. */
simulated protected function GetRadialDamageValues(out float outDamage, out float outRadius, out float outFalloff)
{
	local GearWeapon GW;
	local float DamageScale, RadScale;

	DamageScale = 1.f;
	RadScale = 1.f;

	// Instigator is AI?
	if (Instigator != None && GearAI(InstigatorController) != None && GearAI_TDM(InstigatorController) == None)
	{
		DamageScale *= AIDamageScale;
	}

	if (Instigator != None)
	{
		// active reload scaling
		GW = GearWeapon(Instigator.Weapon);
		if (GW != None)
		{
			DamageScale *= GW.GetActiveReloadDamageMultiplier();
			RadScale *= GW.GetExploRadiusMultiplier();
		}
	}

	outDamage = ExploDamage * DamageScale;
	outRadius = ExploDamageRadius * RadScale;
	outFalloff = ExploDamageFalloffExp;
}


/**
 * Give the projectiles a chance to situationally customize/tweak any explosion parameters.
 * We will also copy in any data we exposed here for .ini file access.
 */
simulated function PrepareExplosionTemplate()
{
	local bool bIsMulti;
	local GearWeapon GW;

	GetRadialDamageValues(ExplosionTemplate.Damage, ExplosionTemplate.DamageRadius, ExplosionTemplate.DamageFalloffExponent);

	if( Instigator != none )
	{
		GW = GearWeapon(Instigator.Weapon);
		if(GW != None && GW.bSuppressDamage)
		{
			ExplosionTemplate.Damage = 0.0;
		}
	}

	bIsMulti = GearGRI(WorldInfo.GRI).IsMultiPlayerGame();
	ExplosionTemplate.KnockDownRadius = bIsMulti ? KnockDownRadiusMP : KnockDownRadius;
	ExplosionTemplate.CringeRadius = bIsMulti ? CringeRadiusMP : CringeRadius;
}

/** Static, for setting up a "default" projectile explosion when there is no actual projectile. */
simulated static function PrepareDefaultExplosionTemplate(GearExplosion ExploTemplate, optional bool bIsMulti)
{
	ExploTemplate.Damage = default.ExploDamage;
	ExploTemplate.DamageRadius = default.ExploDamageRadius;
	ExploTemplate.DamageFalloffExponent = default.ExploDamageFalloffExp;

	ExploTemplate.KnockDownRadius = bIsMulti ? default.KnockDownRadiusMP : default.KnockDownRadius;
	ExploTemplate.CringeRadius = bIsMulti ? default.CringeRadiusMP : default.CringeRadius;
}

/**
 * Trigger explosion.
 * @param	HitActor	Actor touched by exploding projectile. Used to handle specific damage conditions, for instance Kidnappers protected by Hostages.
 */
simulated function TriggerExplosion(Vector HitLocation, Vector HitNormal, Actor HitActor)
{
	local vector NudgedHitLocation;
	local GearExplosionActor ExplosionActor;

	if (!bHasExploded)
	{
		StopSimulating();

		if (ExplosionTemplate != None)
		{
			// using a hitlocation slightly away from the impact point is nice for certain things
			NudgedHitLocation = HitLocation + (HitNormal * 32.f);

			ExplosionActor = Spawn(class'GearExplosionActor',,, NudgedHitLocation, rotator(HitNormal));
			if (ExplosionActor != None)
			{
				ExplosionActor.Instigator = Instigator;
				ExplosionActor.InstigatorController = InstigatorController;

				PrepareExplosionTemplate();

				// these are needed for the decal tracing later in GearExplosionActor.Explode()
				ExplosionActor.bActiveReloadBonusActive = GearPawn(Instigator) != None ? GearPawn(Instigator).bActiveReloadBonusActive : FALSE;
				ExplosionTemplate.HitActor = HitActor;
				ExplosionTemplate.HitLocation = NudgedHitLocation;
				ExplosionTemplate.HitNormal = HitNormal;

				// If desired, attach to mover if we hit one
				if(bAttachExplosionToHitMover && InterpActor(HitActor) != None)
				{
					ExplosionTemplate.Attachee = HitActor;
					ExplosionTemplate.bAttachExplosionEmitterToAttachee = TRUE;
					ExplosionActor.SetBase(HitActor);
				}

				ExplosionActor.Explode(ExplosionTemplate);		// go bewm
			}
		}

		bHasExploded = TRUE;

		// done with it
		if (!bPendingDelete && !bDeleteMe && bAutoDestroyOnExplosion)
		{
			// defer destruction so any replication of explosion stuff can happen if necessary
			DeferredDestroy(1.f);
		}
	}
}

simulated function Explode(vector HitLocation, vector HitNormal)
{
	`warn(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "deprecated function, use TriggerExplosion instead!!");
	ScriptTrace();
	TriggerExplosion(HitLocation, HitNormal, None);
}

simulated event FellOutOfWorld(class<DamageType> dmgType)
{
	TriggerExplosion(Location, vect(0,0,1), None);
}

/** Makes projectile "dead", but doesn't actually kill it for some time. */
protected simulated function DeferredDestroy(float DelaySec)
{
	local MeshComponent ComponentIt;

	foreach ComponentList(class'MeshComponent', ComponentIt)
	{
		ComponentIt.SetHidden(true);
	}

	SetPhysics(PHYS_None);
	SetCollision(false, false);
	bCollideWorld = false;

	LifeSpan = DelaySec;

	// Put in a different state to prevent further calls to TriggerExplosion
	// bHasExploded prevents this, but many subclasses just ignore it. So that state is additional safety.
	GotoState('WaitingForDestruction');
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == nameof(bHasExploded))
	{
		bHasExploded = false;
		TriggerExplosion(Location, vect(0,0,1), None);
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated state WaitingForDestruction
{
	// Prevent double explosions once destroyed
	simulated function TriggerExplosion(Vector HitLocation, Vector HitNormal, Actor HitActor)
	{
		`Warn(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "called after already exploding!!");
		ScriptTrace();
	}
}

defaultproperties
{
	bAutoDestroyOnExplosion=TRUE
}

