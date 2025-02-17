
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 *
 * Queries the Pawn's SpecialMove and activates the proper child
 * associated with each type.  
 */

class GearAnim_BlendBySpecialMove extends GearAnim_BlendList
	native(Anim);

/** Internal cached pointer to GearPawn Owner */
var const	transient	GearPawn	GearPawnOwner;

cpptext
{
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
};

defaultproperties
{
	bSkipTickWhenZeroWeight=TRUE
	bFixNumChildren=TRUE
	Children(0)=(Name="Default")
	Children(1)=(Name="Kidnapper")
	Children(2)=(Name="Hostage")
	Children(3)=(Name="DBNO")
	Children(4)=(Name="DuelLeader")
	Children(5)=(Name="DuelFollower")

	ChildBlendInTime(0)=0.0f
	ChildBlendInTime(1)=0.0f
	ChildBlendInTime(2)=0.0f
	ChildBlendInTime(3)=0.0f
	ChildBlendInTime(4)=0.0f
	ChildBlendInTime(5)=0.0f
}
