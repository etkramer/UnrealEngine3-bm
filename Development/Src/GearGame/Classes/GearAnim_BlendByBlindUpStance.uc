
/** 
 * GearAnim_BlendByBlindUpStance
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAnim_BlendByBlindUpStance extends GearAnim_BlendList
	native(Anim);

/** Internal cached pointer to GearPawn Owner */
var const	transient	GearPawn		GearPawnOwner;
/** Cached pointer to Pawn's weapon */
var const	transient	GearWeapon	Weapon;

cpptext
{
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
};

defaultproperties
{
	bSkipTickWhenZeroWeight=TRUE
	Children(0)=(Name="Default")
	Children(1)=(Name="ForcedIdleStance")
}
