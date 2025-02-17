
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Weapon recoil bone controller.
 * Integrated to an IK Solver
 */

class GearSkelCtrl_IKRecoil extends SkelControlLimb
	hidecategories(Effector)
	native(Anim);

/** Recoil Start-up */
enum EIKRecoilStart
{
	EIKRS_Zero,			// Start with random offset (default)
	EIKRS_Random,		// Start with zero offset
};

/** Recoil params */
struct native IKRecoilParams
{
	var() EIKRecoilStart	X, Y, Z;

	var transient const byte Padding;
};

/** Recoil definition */
struct native IKRecoilDef
{
	/** Time in seconds to go until current recoil finished */
	var		transient	float			TimeToGo;
	/** Duration in seconds of current recoil shake */
	var()				float			TimeDuration;

	/** Rotation amplitude */
	var()				vector			RotAmplitude;
	/** Rotation frequency */
	var()				vector			RotFrequency;
	/** Rotation Sine offset */
	var					vector			RotSinOffset;
	/** Rotation parameters */
	var()				IKRecoilParams	RotParams;
	/** Internal, Rotation offset for this frame. */
	var		transient	Rotator			RotOffset;

	/** Loc offset amplitude */
	var()				vector			LocAmplitude;
	/** Loc offset frequency */
	var()				vector			LocFrequency;
	/** Loc offset Sine offset */
	var					vector			LocSinOffset;
	/** Loc parameters */
	var()				IKRecoilParams	LocParams;
	/** Internal, Location offset for this frame. */
	var		transient	Vector			LocOffset;

	structdefaultproperties
	{
		TimeDuration=0.33f
	}
};

/** Recoil Information */
var()	IKRecoilDef			Recoil;

var()	Vector2D			Aim;

/** variable to play recoil */
var()	transient	bool	bPlayRecoil;
var		transient	bool	bOldPlayRecoil;

/** Internal, evaluates recoil is doing an effect and needs to be applied */
var		transient	bool	bApplyControl;

/** Internal cached pointer to WarPawn Owner */
var		const transient GearPawn		GearPawnOwner;

cpptext
{
	/** Pull aim information from Pawn */
	virtual FVector2D GetAim(USkeletalMeshComponent* InSkelComponent);

	/** Is skeleton currently mirrored */
	virtual UBOOL IsMirrored(USkeletalMeshComponent* InSkelComponent);

	// USkelControlBase interface
	virtual void TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp);
	virtual void CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms);	
}
 
defaultproperties
{
}
