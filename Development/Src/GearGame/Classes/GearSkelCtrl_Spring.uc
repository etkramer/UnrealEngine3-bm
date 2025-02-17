/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Allows a chain of bones to 'trail' behind its head.
 */
class GearSkelCtrl_Spring extends SkelControlBase
	native(Anim);

cpptext
{
	// USkelControlBase interface
	virtual void TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp);
	virtual void GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices);
	virtual void CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms);	
}
 
/** Limit the amount that a bone can stretch from its ref-pose length. */
var(Spring)		bool	bLimitDisplacement;

/** If bLimitStretch is true, this indicates how long a bone can stretch beyond its length in the ref-pose. */
var(Spring)		float	MaxDisplacement;

/** Stiffness of spring */
var(Spring)		float	SpringStiffness;

/** Damping of spring */
var(Spring)		float	SpringDamping;

/** If spring stretches more than this, reset it. Useful for cathing teleports etc */
var(Spring)		float	ErrorResetThresh;

/** If TRUE, Z position is always correct, no spring applied */
var(Spring)		bool	bNoZSpring;

/** Internal use - we need the timestep to do the relaxation in CalculateNewBoneTransforms. */
var				transient float		ThisTimstep;

/** Did we have a non-zero ControlStrength last frame. */
var				transient bool		bHadValidStrength;

/** World-space location of the bone. */
var				transient vector	BoneLocation;

/** World-space velocity of the bone. */
var				transient vector	BoneVelocity;

defaultproperties
{
	SpringStiffness=50.0
	SpringDamping=4.0
	ErrorResetThresh=256.0
}
