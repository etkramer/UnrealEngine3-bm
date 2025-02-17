/**
 * DamageType, the base class of all damagetypes.
 * this and its subclasses are never spawned, just used as information holders
 *
 * NOTE:  we can not do:  HideDropDown on this class as we need to be able to use it in SeqEvent_TakeDamage for objects taking
 * damage from any DamageType!
 * 
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class DamageType extends object
	native
	abstract;

// Description of a type of damage.
var() localized string     	DeathString;	 			// string to describe death by this type of damage
var() localized string		FemaleSuicide, MaleSuicide;	// Strings to display when someone dies
var() bool					bArmorStops;				// does regular armor provide protection against this damage
var() bool                  bAlwaysGibs;
var() bool bNeverGibs;
var() bool                  bLocationalHit;

var() bool					bCausesBlood;
/** Whether or not this damage type can cause a blood splatter **/
var() bool                  bCausesBloodSplatterDecals;
var() bool					bKUseOwnDeathVel;			// For ragdoll death. Rather than using default - use death velocity specified in this damage type.
var   bool					bCausedByWorld;				//this damage was caused by the world (falling off level, into lava, etc)
var   bool					bExtraMomentumZ;	// Add extra Z to momentum on walking pawns

/** Can break bits off FracturedStaticMeshActors. */
var() bool					bCausesFracture;

/** if true, ignore vehicle DriverDamageMult when calculating damage caused to its driver */
var bool bIgnoreDriverDamageMult;

/** if true, this damage type should never harm its instigator */
var bool bDontHurtInstigator;

var() float					GibModifier;

var(RigidBody)	float		KDamageImpulse;				// magnitude of impulse applied to KActor due to this damage type.
var(RigidBody)  float		KDeathVel;					// How fast ragdoll moves upon death
var(RigidBody)  float		KDeathUpKick;				// Amount of upwards kick ragdolls get when they die
var(RigidBody)  float		KImpulseRadius;				// Radius of impulse, if bKRadialImpulse is true
var(RigidBody)  bool		bKRadialImpulse;			// whether impulse applied to rigid bodies is radial

/** Size of impulse to apply when doing radial damage. */
var(RigidBody)	float		RadialDamageImpulse;

/** When applying radial impulses, whether to treat as impulse or velocity change. */
var(RigidBody)	bool		bRadialDamageVelChange;

var float VehicleDamageScaling;							// multiply damage by this for vehicles
var float VehicleMomentumScaling;

/** The forcefeedback waveform to play when you take damage */
var ForceFeedbackWaveform DamagedFFWaveform;

/** The forcefeedback waveform to play when you are killed by this damage type */
var ForceFeedbackWaveform KilledFFWaveform;

static function string DeathMessage(PlayerReplicationInfo Killer, PlayerReplicationInfo Victim)
{
	return Default.DeathString;
}

static function string SuicideMessage(PlayerReplicationInfo Victim)
{
	if ( (Victim != None) && Victim.bIsFemale )
		return Default.FemaleSuicide;
	else
		return Default.MaleSuicide;
}

static function float VehicleDamageScalingFor(Vehicle V)
{
	return Default.VehicleDamageScaling;
}

defaultproperties
{
	bArmorStops=true
	GibModifier=+1.0
    bLocationalHit=true
	bCausesBlood=true
	KDamageImpulse=800
	VehicleDamageScaling=+1.0
    VehicleMomentumScaling=+1.0
    bExtraMomentumZ=true
	KImpulseRadius=250.0
}
