/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 *
 * Queries the current CoverType and activates the proper child
 * associated with each type.  Currently the mapping of ECoverType
 * is:
 * 
 * Children(0) - CT_Standing
 * Children(1) - CT_MidLevel
 */
class GearAnim_BlendByCoverType extends GearAnim_BlendList
	native(Anim);

/** Internal cached pointer to GearPawn Owner */
var const	transient	GearPawn		GearPawnOwner;


cpptext
{
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
};

defaultproperties
{
	bSkipTickWhenZeroWeight=TRUE
	bFixNumChildren=TRUE
	Children(0)=(Name="StandingCover")
	Children(1)=(Name="MidLevelCover")

	BlendType=ABT_EaseInOutExponent3
	ChildBlendInTime(0)=0.5
	ChildBlendInTime(1)=0.5
}
