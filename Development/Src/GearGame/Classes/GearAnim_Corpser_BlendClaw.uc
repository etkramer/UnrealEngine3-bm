/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class GearAnim_Corpser_BlendClaw extends AnimNodeBlendMultiBone
		native(Anim);

cpptext
{
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );
}

/** Array of target weights for each child. Size must be the same as the Children array. */
var		array<float>	TargetWeight;
var		array<float>	BlendTimeToGo;
// Custom blend in time for each child
var()	array<float>	ChildBlendInTime;
var()	array<float>	ChildBlendOutTime;


native function AnimNodeSequence PlayAttackAnim( int LegIdx, float InTargetWeight, optional Name AnimSeqName );
		
defaultproperties
{
	bFixNumChildren=true
	Children(0)=(Name="Source",Weight=1.0)
	BlendTargetList(0)=(InitPerBoneIncrease=1.0)
	ChildBlendInTime(0)=0.0
	ChildBlendOutTime(0)=0.0
	Children(1)=(Name="Claw1")
	BlendTargetList(1)=(InitTargetStartBone="Lft_UpperArm1",InitPerBoneIncrease=1.0)
	ChildBlendInTime(1)=0.25
	ChildBlendOutTime(1)=0.25
	Children(2)=(Name="Claw2")
	BlendTargetList(2)=(InitTargetStartBone="Lft_UpperArm2",InitPerBoneIncrease=1.0)
	ChildBlendInTime(2)=0.25
	ChildBlendOutTime(2)=0.25
	Children(3)=(Name="Claw3")
	BlendTargetList(3)=(InitTargetStartBone="Lft_UpperArm3",InitPerBoneIncrease=1.0)
	ChildBlendInTime(3)=0.25
	ChildBlendOutTime(3)=0.25
	Children(4)=(Name="Claw4")
	BlendTargetList(4)=(InitTargetStartBone="Lft_UpperArm4",InitPerBoneIncrease=1.0)
	ChildBlendInTime(4)=0.25
	ChildBlendOutTime(4)=0.25
	Children(5)=(Name="Claw5")
	BlendTargetList(5)=(InitTargetStartBone="Lft_UpperArm4(mirrored)",InitPerBoneIncrease=1.0)
	ChildBlendInTime(5)=0.25
	ChildBlendOutTime(5)=0.25
	Children(6)=(Name="Claw6")
	BlendTargetList(6)=(InitTargetStartBone="Lft_UpperArm3(mirrored)",InitPerBoneIncrease=1.0)
	ChildBlendInTime(6)=0.25
	ChildBlendOutTime(6)=0.25
	Children(7)=(Name="Claw7")
	BlendTargetList(7)=(InitTargetStartBone="Lft_UpperArm2(mirrored)",InitPerBoneIncrease=1.0)
	ChildBlendInTime(7)=0.25
	ChildBlendOutTime(7)=0.25
	Children(8)=(Name="Claw8")
	BlendTargetList(8)=(InitTargetStartBone="Lft_UpperArm1(mirrored)",InitPerBoneIncrease=1.0)
	ChildBlendInTime(8)=0.25
	ChildBlendOutTime(8)=0.25
}
