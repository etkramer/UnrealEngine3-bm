
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnim_BlendByWeaponClass extends GearAnim_BlendList
	native(Anim);

/** Internal cached pointer to GearPawn Owner */
var const	transient	GearPawn		MyGearPawn;
/** Internal cached pointer to owner's weapon */
var const	transient	GearWeapon		MyGearWeapon;

/** List Weapon Classes. Matches size of Children array - 1 */
var() editfixedsize Array<Class<GearWeapon> >	WeaponClassList<AllowAbstract>;

cpptext
{
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);

	/** Track Changes, and trigger updates */
	virtual void	PostEditChange(UProperty* PropertyThatChanged);

	/** Rename Child connectors upon edit/remove */
	virtual void	RenameChildConnectors();

	// AnimNodeBlendBase interface
	virtual void	OnAddChild(INT ChildNum);
	virtual void	OnRemoveChild(INT ChildNum);
};

defaultproperties
{
	bSkipTickWhenZeroWeight=TRUE

	Children(0)=(Name="Default")
	Children(1)=(Name="AssaultRifle")

	WeaponClassList(0)=class'GearWeap_AssaultRifle'
}
