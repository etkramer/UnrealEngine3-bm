
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAnim_BlendByWeaponType extends GearAnim_BlendList
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
	Children(0)=(Name="AssaultRifle")
	Children(1)=(Name="Shotgun")
	Children(2)=(Name="SniperRifle")
	Children(3)=(Name="Pistol")

	ChildBlendInTime(0)=0.0
	ChildBlendInTime(1)=0.0
	ChildBlendInTime(2)=0.0
	ChildBlendInTime(3)=0.0
}
