/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTSkelControl_CicadaEngine extends SkelControlSingleBone
	hidecategories(Translation, Rotation, Adjustment)
	native(Animation);

/** This holds the max. amount the engine can pitch */
var() float ForwardPitch;

/** This holds the min. amount the engine can pitch */
var() float BackPitch;

/** How fast does it change direction */
var() float PitchRate;

/** Velocity Range */

var() float MaxVelocity;
var() float MinVelocity;

var() float MaxVelocityPitchRateMultiplier;

/** Used to time the change */
var transient float PitchTime;

/** Holds the last thrust value */
var transient float LastThrust;

/** This holds the desired pitches for a given engine */
var transient int DesiredPitch;

cpptext
{
	virtual void TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp);
}

defaultproperties
{
	bApplyRotation=true
	bAddRotation=true
	BoneRotationSpace=BCS_ActorSpace
	ControlStrength=1.0
	ForwardPitch=-40
	BackPitch=40
	PitchRate=0.5
	MaxVelocity=2100
	MinVelocity=100
	MaxVelocityPitchRateMultiplier=0.15
	bIgnoreWhenNotRendered=true
}
