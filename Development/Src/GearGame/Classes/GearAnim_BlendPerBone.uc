
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnim_BlendPerBone extends AnimNodeBlendBase
	native(Anim);

/** Internal cached pointer to GearPawn Owner */
var		const transient GearPawn		GearPawnOwner;

/** Enum specifying how the weight should be checked */
enum EWeightCheck
{
	/** Weight must be >= 1.f - ZERO_ANIMWEIGHT_THRESH */
	EWC_ChildIndexFullWeight,
	/** Weight must be < 1.f - ZERO_ANIMWEIGHT_THRESH */
	EWC_ChildIndexNotFullWeight,
	/** Weight must be > ZERO_ANIMWEIGHT_THRESH */
	EWC_ChildIndexRelevant,
	/** Weight must be <= ZERO_ANIMWEIGHT_THRESH */
	EWC_ChildIndexNotRelevant,
};

/** Rule put on a node. */
struct native WeightNodeRule
{
	/** Name of node */
	var()	Name				NodeName;
	/** Reference to node */
	var		AnimNodeBlendBase	CachedNode;
	/** How the weight should be checked. */
	var()	EWeightCheck		WeightCheck;
	/** Child index of node to check weight for */
	var()	INT					ChildIndex;
};

/** Definition of a mask rule. */
struct native WeightRule
{
	var()	WeightNodeRule	FirstNode;
	var()	WeightNodeRule	SecondNode;
};


struct native BranchInfo
{
	// Exposed properties
	/** Name of bone branch is starting from */
	var()	Name	BoneName;
	/** Used to set up smooth blending */
	var()	float	PerBoneWeightIncrease;

	structdefaultproperties
	{
		PerBoneWeightIncrease=1.f
	}
};

/** Per bone masking definition */
struct native PerBoneMaskInfo
{
	// Exposed properties
	var()	Array<BranchInfo>		BranchList;

	/** Desired weight for this Mask */
	var()	float					DesiredWeight;
	var()	float					BlendTimeToGo;

	/** 
	 * Rules for turning off Mask. 
	 * This system allows to turn off a mask based on a set of rules.
	 * Most of the time BlendPerBone is associated with a AnimNodeSlot
	 * to play cutsom animations.
	 * So with this system, it's possible to make the BlendPerBone a pass through node
	 * when no custom animation is played on the AnimNodeSlot. Hence optimizing significantly the tree.
	 * 
	 * Example:
	 * - NodeName = Name of AnimNodeSlot
	 * - ChildIndex = 0 (source of AnimNodeSlot, when no custom animation is playing)
	 * - WeightCheck = EWC_ChildIndexFullWeight
	 * So this reads, if the Source children of the AnimNodeSlot is full weight
	 * (ie no custom animation is playing), then turn off this mask and 
	 * make this BlendPerBone a pass through node.
	 *
	 * @note: When setting up multiple rules, ALL of them must be true in order to turn off the mask.
	 * if one fails, then the mask will NOT be disabled.
	 */
	var()	Array<WeightRule>		WeightRuleList;
	var()	bool					bWeightBasedOnNodeRules;

	/** 
	 * If the owner is not a local human player, then ignore this branch. 
	 * (ie AI, other players in network...)
	 */
	var()	bool					bDisableForNonLocalHumanPlayers;
	/** Disable if GearPawn::CameraBoneMotionScale is zero */
	var()	bool					bDisableIfCameraBoneMotionScaleIsZero;

	/** Set when there is a blend pending, and it's being delayed by CanBlendTo()/CanBlendOutFrom() */
	var	transient	bool			bPendingBlend;

	// Internal properties
	/** Weight scaling for each bone of the skeleton. Must be same size as RefSkeleton of SkeletalMesh. If all 0.0, no animation can ever be drawn from Child2. */
	var	transient	Array<FLOAT>	PerBoneWeights;
	/** Array of parent bone space bone atoms */
	var	transient	Array<BoneAtom>	MaskAtoms;
	/** Array of Mesh Space transform matrices. Used by Mesh Space blending. */
	var	transient	Array<Matrix>	MeshSpaceTM;

	/** 
	 * Bones required to be transformed to mesh space.
	 * When doing a MeshSpace blending, this array defines which bones need to be blended that way
	 * as an optimization. As it is expensive to convert from Parent Bone Space -> Mesh Space and back.
	 * So this ensures that the conversion is only performed on the critical bones.
	 * These are the bones which have a different mask weight than their parents (so they will be blended)
	 * and their parents (needed to build the mesh space skeleton, as we are converting from PARENT bone space.
	 * The other bones can be done with the faster parent bone space blend.
	 */
	var transient	Array<BYTE>		TransformReqBone;

	/** Index to navigate above array */
	var transient	INT				TransformReqBoneIndex;
};

/** List of Masks. Matches size of Children array - 1 */
var() editfixedsize editinline Array<PerBoneMaskInfo>	MaskList;

/** Describes how a blend should be performed. */
enum EBlendType
{
	EBT_ParentBoneSpace,
	EBT_MeshSpace,
};

/** How rotation should be blended */
var()			EBlendType		RotationBlendType;

cpptext
{
	/** Do any initialisation, and then call InitAnim on all children. Should not discard any existing anim state though. */
	virtual void	InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);

	/** Ticking, updates weights... */
	virtual	void	TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);

	/** 
	 * Calculates total weight of children. 
	 * Set a full weight on source, because it's potentially always feeding animations into the final blend.
	 */
	virtual void	SetChildrenTotalWeightAccumulator(const INT Index);

	/** @see UAnimNode::GetBoneAtoms. */
	virtual void	GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);

	// Special Optimized Paths
	FORCEINLINE void	MeshSpaceBlendMultipleMasks(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, TArray<INT>& RelevantChildren);
	FORCEINLINE void	LocalBlendMultipleMasks(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, TArray<INT>& RelevantChildren);
	FORCEINLINE void	MeshSpaceBlendSingleMask(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, TArray<INT>& RelevantChildren);
	FORCEINLINE void	LocalBlendSingleMask(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, TArray<INT>& RelevantChildren);

	/** Parent node is requesting a blend out. Give node a chance to delay that. */
	virtual UBOOL	CanBlendOutFrom();
	/** parent node is requesting a blend in. Give node a chance to delay that. */
	virtual UBOOL	CanBlendTo();

	/**
	 * Utility for creating the Mask PerBoneWeights array. 
	 * Walks down the hierarchy increasing the weight by PerBoneWeightIncrease each step.
	 */
	virtual void	CalcMaskWeight(INT MaskIndex);

	virtual	void	UpdateRules();

	/** Track Changes, and trigger updates */
	virtual void	PostEditChange(UProperty* PropertyThatChanged);

	/** Rename Child connectors upon edit/remove */
	virtual void	RenameChildConnectors();

	// AnimNodeBlendBase interface
	virtual void	OnAddChild(INT ChildNum);
	virtual void	OnRemoveChild(INT ChildNum);
}


/** 
 * Control the weight of a given Mask.
 */
native final function SetMaskWeight(INT MaskIndex, FLOAT DesiredWeight, FLOAT BlendTime);

defaultproperties
{
	bSkipTickWhenZeroWeight=TRUE
	Children(0)=(Name="Source",Weight=1.f)
}
