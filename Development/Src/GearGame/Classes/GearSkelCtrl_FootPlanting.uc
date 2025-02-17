
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearSkelCtrl_FootPlanting extends SkelControlLimb
	hidecategories(Effector)
	native(Anim);

/** if TRUE, foot will be locked when below FootLockZThreshold. */
var()	bool	bDoFootLocking;

/** Height used to find out if foot should be locked or not. */
var()	float	FootLockZThreshold;

/** 
 * Foot Bone Name. Used for locking and height checks.
 * Doesn't have the be the bone selected for IK. (for eg. for creatures that have more than 2 bones legs)
 */
var()	Name	FootBoneName;

/** 
 * If using IK Bone for foot positioning, use this bone. 
 */
var()	Name	IKFootBoneName;

/** time it takes to blend from locked position to animated position. */
var()	float	LockAlphaBlendTime;

/** Time left when blending from locked position to animated position. */
var const transient float	LockAlphaBlendTimeToGo;
var const transient float	LockAlpha;
var const transient float	LockAlphaTarget;
var const transient float	LastDeltaTime;
var const transient vector	LockFootLoc;

/** Internal, if TRUE foot is locked in place */
var const transient	bool	bLockFoot;
/** location of locked foot */
var const transient vector	LockedFootWorldLoc;

cpptext
{
	void UpdateLockAlpha(FLOAT DeltaSeconds);
	void SetLockAlphaTarget(UBOOL bUnlock);

	// USkelControlBase interface
	virtual void TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp);
	virtual void CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms);
	virtual void DrawSkelControl3D(const FSceneView* View, FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* SkelComp, INT BoneIndex);
}

defaultproperties
{
	LockAlphaBlendTime=0.2f
	LockAlphaTarget=0.f
	LockAlpha=0.f
}