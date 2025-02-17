
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnim_BlendByHeavyWeaponMount extends GearAnim_BlendList
	native(Anim);

/** Internal cached pointer to GearPawn owner */
var const	transient	GearPawn MyGearPawn;

cpptext
{
	virtual void	InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual void	TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
};

defaultproperties
{
	bSkipTickWhenZeroWeight=TRUE
	bFixNumChildren=TRUE
	Children(0)=(Name="UnMounted")
	Children(1)=(Name="Mounted")

	ChildBlendInTime(0)=0.25f
	ChildBlendInTime(1)=0.25f
}
