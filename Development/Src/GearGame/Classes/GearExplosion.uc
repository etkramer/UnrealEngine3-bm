/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearExplosion extends Object
	dependson(GearPlayerCamera)
	editinlinenew;

//
// Gameplay parameters
//

/** Delay before applying damage after spawning FX, 0.f == no delay */
var() float DamageDelay;
/** Amount of damage done at the epicenter. */
var() float Damage;
/** Damage range. */
var() float DamageRadius;
/** Defines how damage falls off.  High numbers cause faster falloff, lower (closer to zero) cause slower falloff.  1 is linear. */
var() float DamageFalloffExponent;
/** Optional actor that does not receive any radial damage, to be specified at runtime */
var transient Actor ActorToIgnoreForDamage;

/** The actor class to ignore for damage from this explosion **/
var transient class<Actor> ActorClassToIgnoreForDamage;

/** The actor class to ignore for knockdowns and cringes from this explosion **/
var transient class<Actor> ActorClassToIgnoreForKnockdownsAndCringes;

/** True to allow teammates to cringe, regardless of friendly fire setting. */
var() bool bAllowTeammateCringes;

/** Option to force full damage to the attachee actor */
var transient bool	bFullDamageToAttachee;

/** What damagetype to use */
var() class<DamageType> MyDamageType;

/** radius at which people will be knocked down/ragdolled by the projectile's explosion **/
var() float	KnockDownRadius;
/** Knocked down pawns will play this effort audio. */
var() GearVoiceEffortID PawnKnockdownEffort;

/** radius at which people will cringe from the explosion **/
var() float	CringeRadius;

/** Percentage of damagetype's momentum to apply. */
var() float MomentumTransferScale;

/** Whether or not we should attach something to the attachee **/
var() bool bAttachExplosionEmitterToAttachee;

//
// Camera shake parameters
//

/** Set to TRUE if code-driven camera shake is desired upon explosion */
var() bool				bDoExploCameraShake;
/** Code-driven screenshake to play upon explosion */
var() ScreenShakeStruct	ExploShake;

/** Set to TRUE if CameraAnim-driven camera shake is desired upon explosion */
var() bool					bDoExploCameraAnimShake;
/** Screenshake to play when this grenade explodes */
var() ScreenShakeAnimStruct	ExploAnimShake;

/** Radius to within which to play full-powered screenshake (will be scaled within radius) */
var() float										ExploShakeInnerRadius;
/** Between inner and outer radii, scale shake from full to zero */
var() float										ExploShakeOuterRadius;
/** Exponent for intensity falloff between inner and outer radii. */
var() float										ExploShakeFalloff;



//
// Particle effect parameters
//

/** Which particle effect to play. */
var() ParticleSystem	ParticleEmitterTemplate;
/** Scalar for increasing/decreasing explosion effect size. */
var() float				ExplosionEmitterScale;


/** Play this CameraEffect when ever damage of this type is given.  This will primarily be used by explosions.  But could be used for other impacts too! **/
var() class<Emit_CameraLensEffectBase> CameraEffect;

/** This is the radius to play the camera effect on **/
var() float CameraEffectRadius;


//
// Fog Volume Parameters
//

/**
 * This tells the explosion to used a per material fog volume.  If the material has some special fogvolume (e.g. smoke / ink) then
 * this should be set to FALSE
 **/
var() bool bUsePerMaterialFogVolume;

/** The archetype that the artists set up that will be spawned with this explosion **/
var FogVolumeSphericalDensityInfo FogVolumeArchetype;


//
// Decal parameters
//

/** The actual DecalData for this Explosion.  (more than likely grabbed from the PhysicalMaterial system) **/
var DecalData DecalData;

/** Track if we've hit an actor, used to handle cases such as kidnapper protected from hostage damage */
var Actor HitActor;

/** We need the hit location and hit normal so we can trace down to the actor to apply the decal  (e.g. hitting wall or floor) **/
var vector HitLocation;
var vector HitNormal;

/**
 * Different projectiles have different ranges for where their "location" is.  
 * (e.g. the tagged grenades need 32 while the torque bow needs around 12 otherwise the decal projects onto players standing near it
 **/
var float DecalTraceDistance;

//
// Audio parameters
//

/** Audio to play at explosion time. */
var() SoundCue	ExplosionSound;

/** True to have a silent explosion. */
var() bool		bSuppressAudio;

//
// Dynamic light parameters
//

/** Defines the dynamic light cast by the explosion */
var() PointLightComponent	ExploLight;
/** Dynamic Light fade out time, in seconds */
var() float					ExploLightFadeOutTime;

//
// Fractured mesh parameters
//

/** Controls if this explosion will cause fracturing */
var() bool					bCausesFracture;
/** How far away from explosion we break bits off */
var() float					FractureMeshRadius;
/** How hard to throw broken off pieces */
var() float					FracturePartVel;

/** Are we attached to something?  Used to attach FX for stuff like the smoke grenade. */
var() Actor			Attachee;
var() Controller	AttacheeController;

/** If true, attempt to get effect information from the physical material system.  If false or a physicalmaterial is unavailable, just use the information above. */
var() bool bAllowPerMaterialFX;

/** So for tagged grenades we need override the particle system but still want material based decals and such. **/
var() bool bParticleSystemIsBeingOverriddenDontUsePhysMatVersion;

/** This tells the explosion to look in the Map's MapSpecific info **/
var() bool bUseMapSpecificValues;

var() bool bUseOverlapCheck;


defaultproperties
{
	ExplosionEmitterScale=1.f
	MyDamageType=class'GDT_Explosive'
	MomentumTransferScale=1.f
	ExploShakeFalloff=2.f
	bCausesFracture=TRUE
	DecalTraceDistance=48.0f
}
