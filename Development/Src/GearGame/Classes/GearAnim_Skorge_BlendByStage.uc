
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * GearAnim_Skorge_BlendByStage
 */

class GearAnim_Skorge_BlendByStage extends GearAnim_BlendList
	native(Anim);

/** Internal cached pointer to Skorge Owner */
var const transient	GearPawn_LocustSkorgeBase	SkorgeOwner;

cpptext
{
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
};

defaultproperties
{
	bSkipTickWhenZeroWeight=TRUE
	bFixNumChildren=TRUE
	Children(0)=(Name="Staff")
	Children(1)=(Name="TwoStick")
	Children(2)=(Name="OneStick")

	ChildBlendInTime(0)=0.25
	ChildBlendInTime(1)=0.25
	ChildBlendInTime(2)=0.25
}
