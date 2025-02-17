/**
 * http://www.googlefight.com/index.php?lang=en_GB&word1=Berserker&word2=berserker
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class GearPawn_LocustBerserkerBase extends GearPawn
	config(Pawn)
	abstract;

/** Is the Berserker currently vulnerable to all types of damage? */
var repnotify bool bVulnerableToDamage;

/** How much longer till the Berserker is no longer vulnerable */
var float RemainingVulnerableTime;

/** Damage types the Berserker is always vulnerable to */
var() array<class<DamageType> > AlwaysVulnerableDamageTypes;

/** Damage types that cause temporary vulnerability */
var() array<class<DamageType> > VulnerabilityDamageTypes;

/** Threshold of damage taken from VulnerabilityDamageTypes before enabling vulnerability to all damage types */
var() config float VulnerabilityDamageThreshold;

/** Current amount of vulnerable damage taken */
var float VulnerabilityDamageCount;

/** Rate that VulnerabilityDamageCount naturally decays */
var() config float VulnerabilityDamageRecoverRate;

/** Amount of time to remain vulnerable */
var() config float VulnerabilityTimer;

/** When hit by the Hammer of Dawn, this is the time the berzeker will play the hit reaction animation */
var() config float HODHitReactionTime;

var bool bAttackInProgress;
/** these are the functions that are going to play various effects **/

/** Speed scale when heated up */
var() config float HeatedUpSpeedScale;

/** Desired rotation to reach with Root Rotation */
var repnotify int DesiredYawRootRotation;

replication
{
	if (bNetInitial)
		VulnerabilityDamageThreshold;
	if (bNetDirty)
		VulnerabilityDamageCount, DesiredYawRootRotation, bVulnerableToDamage;
}

/** A light dust cloud to be played when she makes contact with the player in her charge. - should play from the point of impact with the X axis facing in towards her. **/
simulated function PlayChargeHitPlayerEffect( vector HitLocation, vector HitNormal );

/** A large dust impact for when she slams into the wall after missing a charge attack and becomes dazed. - should play from the point of impact with the X axis facing in towards her. **/
simulated function PlayChargeHitWallEffect( vector HitLocation, vector HitNormal );

/** A larger directional dust for when she crouches and takes off in a run from a standing position. The X axis should face the opposite direction from the way she is running. To be played at her feet. **/
simulated function PlayChargeTakeOffEffect( vector PawnLocation, vector HitNormal );

/** The Various functions for the berserker breath effects **/
simulated function PlayBreathChargeHitOne();
simulated function PlayBreathChargeHitTwo();
simulated function PlayBreathIdle();
simulated function PlayBreathDazed();

/** This function stops needed soundeffects and plays the 'prepare to attack' sound*/
simulated function PlayPrepareToAttack();
simulated function StartScream();
simulated function StopScream();

/** replicated event */
simulated event ReplicatedEvent(Name VarName)
{
	switch( VarName )
	{
		case 'bVulnerableToDamage':
			VulnerabilityChanged();
			return;
		case 'DesiredYawRootRotation':
			DesiredRootRotationUpdated();
			return;
	}

	Super.ReplicatedEvent(VarName);
}

/** Berserkers can't be special melee attacked **/
simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	return FALSE;
}

/** Notification called when bVulnerableToDamage changes */
simulated function VulnerabilityChanged();
/** Notification when sniffing for player*/
simulated function Sniff();
//
// Desired Root Rotation
//

/** Sets Target Root Rotation */
simulated final function SetDesiredRootRotation(Rotator Rot)
{
	// Yaw from -32768 to +32767
	DesiredYawRootRotation = Normalize(Rot).Yaw;

	DesiredRootRotationUpdated();
}

/** Clears Desired Root Rotation */
simulated final function ClearDesiredRootRotation()
{
	DesiredYawRootRotation = 65535;	// indicates no rotation.

	DesiredRootRotationUpdated();
}

/** Returns TRUE if Pawn has a DesiredRootRotation set. */
simulated final function bool HasValidDesiredRootRotation()
{
	return (DesiredYawRootRotation != 65535);
}

/** Event called when DesiredRootRotation has been updated */
simulated function DesiredRootRotationUpdated();

defaultproperties
{
	bAttackInProgress=FALSE
	bRespondToExplosions=FALSE
	bCanDBNO=false
}
