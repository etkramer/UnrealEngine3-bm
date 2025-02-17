/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnim_GoreSystem extends AnimNodeBlendBase
	native(Anim)
	dependson(GearGame);

enum EGoreBoneState
{
	EGBS_Attached,
	EGBS_Falling,
	EGBS_Hidden
};

/** Struct containing info for one gore bone */
struct native GearGoreEntry
{
	/** Whether gore bone is currently enabled to take damage */
	var()	bool						bEnabled;
	/** Bone in the PhysicsAsset that you have to shoot to affect gore bone */
	var()	name						HitBodyName;
	/** Name of gore bone that will be removed */
	var()	name						GoreBoneName;
	/** How much damage has to be done to cause this gore bone to be removed */
	var()	int							Health;

	/** Index into GoreEffectInfos array, indicating which particle effect to use when this gore bone falls off */
	var()	int							EffectInfoIndex;
	/** Index into GoreMaterialParamInfos array, indicating which material param to modify when this gore bone falls off */
	var()	int							MaterialParamIndex;

	/** Offset applied in bone space for particle effect. */
	var()	vector						EffectOffset;

	/** Cached bone index of gore bone */
	var		transient int				GoreBoneIndex;
	/** Cached index of GearGoreEntry of child of this gore bone. */
	var		transient int				ChildEntryIndex;

	/** Position in world space gore bone is put in when using EGBS_Falling */
	var		vector						GoreBoneLocation;
	/** Rotation in world space gore bone it put in when using EGBS_Falling */
	var		vector						GoreBoneRotation;

	/** Velocity of gore bone - when using EGBS_Falling */
	var		vector						GoreBoneVelocity;
	/** Angular velocity of gore bone */
	var		vector						GoreBoneAngVelocity;

	/** What state the gore bone is in */
	var		EGoreBoneState				GoreBoneState;
	/** How long to keep it falling before hiding */
	var		float						FallingLifetime;

	structdefaultproperties
	{
		bEnabled=TRUE
		Health=100
		GoreBoneIndex=-1
		ChildEntryIndex=-1
		GoreBoneState=EGBS_Attached
	}
};

/** Array of gore entries */
var()	array<GearGoreEntry>	GoreSetup;
/** How long bones can fall for before hiding */
var()	float					FallingLifetime;
/** How much to scale default gravity for falling gore pieces */
var()	float					GoreFallGravScale;
/** How close a gore bone must be from an explosion (UpdateGoreDamageRadial) for it to be affected */
var()	float					ExplosiveGoreRadius;
/** Initial speed of gore piece - will move away from parent bone */
var()	float					GoreInitialVel;
/** Initial angular speed of gore piece */
var()	float					GoreInitialAngVel;
/** When a piece falls off, add this velocity - in actor space */
var()	vector					ActorSpaceAdditionalVel;

/** Sets of effects to use when gibs come off */
var()	array<GearGoreEffectInfo>	GoreEffectInfos;

/** Info for modifying material parameters as beast is gored */
struct native GearGoreMaterialParam
{
	/** Name of parameter to modify when a gore piece is lost */
	var()	name	ScalarParamName;
	/** How much to increase parameter by when gore piece is lost */
	var()	float	IncreasePerGore;

	structdefaultproperties
	{
		IncreasePerGore=0.3
	}
};

/** Set of material parameters to modify as gore pieces are lost. */
var()	array<GearGoreMaterialParam>	GoreMaterialParamInfos;

/** Used in AnimTree Editor to test all the gore falling off */
var()	bool					bTestAllGore;
/** Use internally to catch edge transition for testing/resetting all gore */
var		transient bool			bOldTestAllGore;

/** Used to trigger 'auto gore setup' in editor */
var()	bool					bAutoFillGoreSetup;

cpptext
{
	virtual void PreEditChange(UProperty* PropertyAboutToChange);
	virtual void PostEditChange(UProperty* PropertyThatChanged);

	virtual void InitAnim( USkeletalMeshComponent* meshComp, UAnimNodeBlendBase* Parent );
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );
	virtual void GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);

	void CalcGoreIndexInfo();

	/** Start the supplied gore piece falling */
	void LoseGorePiece(FGearGoreEntry& GoreEntry, const FMatrix& GoreBoneMatrix);

	void AutoFillGoreSetup(USkeletalMesh* SkelMesh);
	void SortGoreSetup();
}

/** Damage has been done to a body in the physicsasset - update any gore state associated with it */
native function UpdateGoreDamage(name ShotBoneName, vector HitLocation, int Damage);
/** 
 *	Damage done radially from a location - find  all gore bones affected and damage them 
 *	@param	bForceRemove	If TRUE, all bones are removed, regardless of health or position.
 */
native function UpdateGoreDamageRadial(vector HitLocation, int Damage, bool bForceRemoveAll);

/** Drop given gore piece */
native function	ForceLoseGorePiece( Name GoreBoneName );

/** Util to reset all gore (reattach/unhide all pieces) */
native function ResetAllGore();

defaultproperties
{
	bSkipTickWhenZeroWeight=TRUE
	bFixNumChildren=TRUE
	Children(0)=(Name="Input",Weight=1.0)

	GoreFallGravScale=2.0
	FallingLifetime=2.0
	ExplosiveGoreRadius=300.0
	GoreInitialVel=200.0
	GoreInitialAngVel=50.0
}