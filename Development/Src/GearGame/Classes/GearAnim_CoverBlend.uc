
/** 
 * GearAnim_CoverBlend
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAnim_CoverBlend extends GearAnim_BlendList
	native(Anim);

/** Internal cached pointer to GearPawn Owner */
var		const	transient	GearPawn		GearPawnOwner;
/** If TRUE, do not update Pawn.LastCoverActionFiringAnimReadyUpdateTime flag */
var()	const				bool			bUpdatePawnActionFiringFlag;
/** Cover Action that the animations are currently playing */
var		const	transient	ECoverAction	AnimCoverAction;

/** if TRUE, node will be locked wen pawn uses a heavy weapon. Disabling lean and pop up animations. */
var()					bool				bLockForHeavyWeapons;
/** Cached weapon to track when player switches to a heavy weapon. */
var		const transient Weapon				CachedWeapon;
/** cached flag when player carries a heavy weapon. */
var		const transient bool				bCarryingHeavyWeapon;

cpptext
{
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
	/** Get notification that this node is no longer relevant for the final blend. ie TotalWeight is now == 0 */
	virtual void OnCeaseRelevant();

	virtual void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue) 
	{
		check(0 == SliderIndex && 0 == ValueIndex);
		SliderPosition	= NewSliderValue;
	}
};

defaultproperties
{
	bSkipTickWhenZeroWeight=TRUE
	bUpdatePawnActionFiringFlag=TRUE
	bFixNumChildren=TRUE
	Children(0)=(Name="Idle")
	Children(1)=(Name="BackToWall")
	Children(2)=(Name="BlindFireSide")
	Children(3)=(Name="BlindFireUp")
	Children(4)=(Name="PopUp")
	Children(5)=(Name="Lean")
	Children(6)=(Name="Unused")
	Children(7)=(Name="PeekSide")
	Children(8)=(Name="PeekUp")
	Children(9)=(Name="Suppressed")

	BlendType=ABT_EaseInOutExponent3
	ChildBlendInTime(0)=0.50
	ChildBlendInTime(1)=0.25
	ChildBlendInTime(2)=0.25
	ChildBlendInTime(3)=0.25
	ChildBlendInTime(4)=0.25
	ChildBlendInTime(5)=0.25
	ChildBlendInTime(6)=0.25
	ChildBlendInTime(7)=0.25
	ChildBlendInTime(8)=0.25
	ChildBlendInTime(9)=0.25
}
