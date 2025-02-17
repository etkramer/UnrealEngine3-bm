
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * GearAnim_Skorge_BlockingBullets
 */

class GearAnim_Skorge_BlockingBullets extends GearAnim_BlendList
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
	Children(0)=(Name="Normal")
	Children(1)=(Name="BlockingBullets")

	ChildBlendInTime(0)=0.25
	ChildBlendInTime(1)=0.25
}
