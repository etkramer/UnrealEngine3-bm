
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnim_BlendBySpeedDirection extends GearAnim_BlendList
	native(Anim);

/** Internal cached pointer to GearPawn Owner */
var	const transient GearPawn	GearPawnOwner;

var const transient INT		LastChildIndex;

cpptext
{
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual	void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
}

defaultproperties
{
	bSkipTickWhenZeroWeight=TRUE
	bFixNumChildren=TRUE
	Children(0)=(Name="Idle")
	Children(1)=(Name="Fwd")
	Children(2)=(Name="Bwd")

	ChildBlendInTime(0)=0.2f
	ChildBlendInTime(1)=0.2f
	ChildBlendInTime(2)=0.2f

	LastChildIndex=-1
}