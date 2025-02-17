/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTSkelControl_JetThruster extends SkelControlSingleBone
	hidecategories(Translation, Rotation, Adjustment)
	native(Animation);

var(Thruster) float MaxForwardVelocity;
var(Thruster) float BlendRate;

var transient float DesiredStrength;

cpptext
{
	virtual void TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp);
}


defaultproperties
{
	MaxForwardVelocity=1500
	DesiredStrength=1.0
	BlendRate=0.3
	bIgnoreWhenNotRendered=true
}
