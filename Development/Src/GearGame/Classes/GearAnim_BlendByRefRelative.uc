
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnim_BlendByRefRelative extends AnimNodeBlend
	native(Anim);

/** Cached ref to GearPawn Owner */
var protected transient GearPawn			GearPawnOwner;
/** Array containing ref pose atoms. */
var protected transient	Array<BoneAtom>		RefPoseAtoms;
/** AnimNodeSequence used to extract RefPose from animation. */
var protected transient	AnimNodeSequence	RefPoseSeqNode;
/** If None, will use SkelMesh default ref pose. Otherwise use first frame of this animation. */
var()					Name				RefPoseAnimName;
/** if TRUE, don't display a warning when RefPoseAnimName animation is not found. */
var()					bool				bDisableWarningWhenAnimNotFound;
/** Only Turn on for Rifle Set. */
var()					bool				bOnlyForRifleSet;

cpptext
{
	virtual void PostAnimNodeInstance(UAnimNode* SourceNode);

	/** Update Ref Pose Data. If taken from an animation, extract the data from it, and store it in RefPoseAtoms. */
	void UpdateRefPoseData();
	void GetChildAtoms(INT ChildIndex, FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);

	virtual void InitAnim(USkeletalMeshComponent* meshComp, UAnimNodeBlendBase* Parent);
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void AnimSetsUpdated();
	virtual void GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);
	virtual void SetChildrenTotalWeightAccumulator(const INT Index);
}

defaultproperties
{
	bSkipTickWhenZeroWeight=TRUE
	bFixNumChildren=TRUE
	Children(0)=(Name="Base Anim Input",Weight=0.f)
	Children(1)=(Name="Relative Anim Input",Weight=1.f)
	Child2Weight=1.f
	Child2WeightTarget=1.f
}
