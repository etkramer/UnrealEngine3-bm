
/** 
 * GearAnim_TurnInPlace_Rotator
 * Node to to rotate mesh, used with GearAnim_TurnInPlace
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnim_TurnInPlace_Rotator extends AnimNodeBlendBase
	native(Anim);

/** Internal cached pointer to GearPawn Owner */
var	const transient GearPawn				GearPawnOwner;
var const transient GearAnim_TurnInPlace	TurnInPlaceNode;

cpptext
{
	// AnimNode interface
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual void GetBoneAtoms(FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion);
}

defaultproperties
{
	Children(0)=(Name="Input",Weight=1.0)
	bFixNumChildren=TRUE
	bSkipTickWhenZeroWeight=TRUE
}