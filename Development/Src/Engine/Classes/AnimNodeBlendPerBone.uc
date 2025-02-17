
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AnimNodeBlendPerBone extends AnimNodeBlend
	native(Anim);


/** If TRUE, blend will be done in local space. */
var()	const		bool			bForceLocalSpaceBlend;

/** List of branches to mask in from child2 */
var()				Array<Name>		BranchStartBoneName;

/** per bone weight list, built from list of branches. */
var					Array<FLOAT>	Child2PerBoneWeight;

/** Required bones for local to component space conversion */
var					Array<BYTE>		LocalToCompReqBones;

cpptext
{
	/** Do any initialisation, and then call InitAnim on all children. Should not discard any existing anim state though. */
	virtual void InitAnim(USkeletalMeshComponent* meshComp, UAnimNodeBlendBase* Parent);

	// AnimNode interface
	virtual void GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);
	virtual void SetChildrenTotalWeightAccumulator(const INT Index);

	virtual void PostEditChange(UProperty* PropertyThatChanged);
	void BuildWeightList();
}

defaultproperties
{
	Children(0)=(Name="Source",Weight=1.0)
	Children(1)=(Name="Target")
	bFixNumChildren=TRUE
}
