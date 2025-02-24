/**
 *	Simple controller that replaces or adds to the translation/rotation of a single bone.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SkelControlSingleBone extends SkelControlBase
	native(Anim);
	

cpptext
{
	// USkelControlBase interface
	virtual void GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices);
	virtual void CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms);	

	virtual INT GetWidgetCount();
	virtual FMatrix GetWidgetTM(INT WidgetIndex, USkeletalMeshComponent* SkelComp, INT BoneIndex);
	virtual void HandleWidgetDrag(INT WidgetIndex, const FVector& DragVec);
}
 
/** Whether to modify the translation of this bone. */
var(Adjustments)	bool				bApplyTranslation;

/** If false, replaces rotation with BoneRotation. If true, adds to existing rotation. */
var(Translation)	bool				bAddTranslation;

/** Whether to modify the translation of this bone. */
var(Adjustments)	bool				bApplyRotation;

/** If false, replaces rotation with BoneRotation. If true, adds to existing rotation. */
var(Rotation)		bool				bAddRotation;

/** New translation of bone to apply. */
var(Translation)	vector				BoneTranslation;

/** Reference frame to apply BoneTranslation in. */
var(Translation)	EBoneControlSpace	BoneTranslationSpace;

/** Reference frame to apply BoneRotation in. */
var(Rotation)		EBoneControlSpace	BoneRotationSpace;

/** Name of bone used if BoneTranslationSpace is BCS_OtherBoneSpace. */
var(Translation)	name				TranslationSpaceBoneName;
 
/** New rotation of bone to apply. */
var(Rotation)		rotator				BoneRotation;

/** Name of bone used if BoneRotationSpace is BCS_OtherBoneSpace. */
var(Rotation)		name				RotationSpaceBoneName;


defaultproperties
{
	
}
