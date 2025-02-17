/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTAnimBlendBySlotActive extends AnimNodeBlendPerBone
	native(Animation);


/** Cached pointer to slot node that we'll be monitoring. */
var AnimNodeSlot	ChildSlot;

cpptext
{
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual	void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
}

defaultproperties
{
	Children(0)=(Name="Default",Weight=1.0)
	Children(1)=(Name="Slot")
}
