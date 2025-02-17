
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearSkelCtrl_CorpserIK extends SkelControlLimb
	native(Anim);

var()			float		ShoulderLengthStretchAllowance;
var()			Vector2D	InterpDistRange;

cpptext
{
	// USkelControlBase interface
	virtual void GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices);
	virtual void CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms);	
	virtual void DrawSkelControl3D(const FSceneView* View, FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* SkelComp, INT BoneIndex);
}

defaultproperties
{
	ShoulderLengthStretchAllowance=0.25f
	InterpDistRange=(X=50,Y=100)
}