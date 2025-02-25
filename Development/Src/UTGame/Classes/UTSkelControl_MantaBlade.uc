/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTSkelControl_MantaBlade extends SkelControlSingleBone
	hidecategories(Translation, Adjustment)
	native(Animation);

var(Manta)  float		MaxRotationsPerSecond;
var(Manta)	float		SpinUpTime;
var(Manta)	bool		bCounterClockwise;
var			float		RotationsPerSecond, DesiredRotationsPerSecond;

cpptext
{
	virtual void TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp);
}


defaultproperties
{
	bAddRotation=true
	MaxRotationsPerSecond=20
	SpinUptime=1.2
	bApplyRotation=true
	BoneRotationSpace=BCS_ActorSpace
	bIgnoreWhenNotRendered=true
}
