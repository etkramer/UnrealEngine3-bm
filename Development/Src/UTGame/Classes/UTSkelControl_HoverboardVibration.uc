/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 *	Controller used by hoverboard for moving lower part in response to wheel movements.
 */

class UTSkelControl_HoverboardVibration extends SkelControlSingleBone
	hidecategories(Translation,Rotation)
	native(Animation);

cpptext
{
	// SkelControlWheel interface
	virtual void TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp);
	virtual void CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms);
}

var()	float	VibFrequency;
var()	float	VibSpeedAmpScale;
var()	float	VibTurnAmpScale;
var()	float	VibMaxAmplitude;

var		float	VibInput;

defaultproperties
{
	bApplyTranslation=true
	bAddTranslation=true
	BoneTranslationSpace=BCS_BoneSpace
}