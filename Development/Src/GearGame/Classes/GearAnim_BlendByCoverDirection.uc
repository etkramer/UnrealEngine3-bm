
/** 
 * GearAnim_BlendByCoverDirection
 * Blends between Left and Right side in cover.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAnim_BlendByCoverDirection extends GearAnim_BlendList
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
	bFixNumChildren=TRUE

	Children(0)=(Name="Idle")
	Children(1)=(Name="Right")
	Children(2)=(Name="Left")
}
