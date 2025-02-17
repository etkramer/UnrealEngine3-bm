
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AnimNodeAdditiveBlending extends AnimNodeBlend
	native(Anim);

/** 
 * if TRUE, pass through (skip additive animation blending) when mesh is not rendered
 */
var() bool	bPassThroughWhenNotRendered;

cpptext
{
	void GetChildAtoms(INT ChildIndex, FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);
	virtual void GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);
	virtual void SetChildrenTotalWeightAccumulator(const INT Index);
}

defaultproperties
{
	bPassThroughWhenNotRendered=TRUE
	bSkipTickWhenZeroWeight=TRUE
	bFixNumChildren=TRUE
	Children(0)=(Name="Base Anim Input",Weight=0.f)
	Children(1)=(Name="Additive Anim Input",Weight=1.f)
	Child2Weight=1.f
	Child2WeightTarget=1.f
}
