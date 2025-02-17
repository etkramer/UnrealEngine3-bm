/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Custom code specialized for the Gears 2 flamethrower.
 */
class GearSkelCtrl_Flamethrower extends SkelControlBase
	native(Anim);

cpptext
{
	// USkelControlBase interface
	virtual void GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices);
	virtual void CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms);
}

defaultproperties
{
}
