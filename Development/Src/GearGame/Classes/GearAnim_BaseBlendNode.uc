
/**
 * Handles determing which part of the tree to blend based
 * on the current state of the GearPawn, either using the
 * normal movement blending or the various cover blends.
 * 
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAnim_BaseBlendNode extends GearAnim_BlendList
	native(Anim);

/** Internal cached pointer to GearPawn Owner */
var const	transient	GearPawn	GearPawnOwner;

cpptext
{
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
}

defaultproperties
{
	bSkipTickWhenZeroWeight=TRUE
	NodeName="BaseBlendNode"
	
	bFixNumChildren=TRUE
	Children(0)=(Name="Normal")
	Children(1)=(Name="Cover")
	Children(2)=(Name="Kidnapper")
	Children(3)=(Name="Hostage")
	Children(4)=(Name="DBNO")
	Children(5)=(Name="DuelLeader")
	Children(6)=(Name="DuelFollower")
	Children(7)=(Name="ShieldCrouch")

	ChildBlendInTime(0)=0.15
	ChildBlendInTime(1)=0.30
	ChildBlendInTime(2)=0.15
	ChildBlendInTime(3)=0.0f
	ChildBlendInTime(4)=0.0f
	ChildBlendInTime(5)=0.0f
	ChildBlendInTime(6)=0.0f
	ChildBlendInTime(7)=0.0f
}
