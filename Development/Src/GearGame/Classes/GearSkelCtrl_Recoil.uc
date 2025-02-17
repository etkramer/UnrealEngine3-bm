
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Weapon recoil bone controller.
 * Add a small recoil to a bone (similar to camera shakes).
 */
class GearSkelCtrl_Recoil extends GameSkelCtrl_Recoil
	native(Anim);

/** Internal cached pointer to GearPawn Owner */
var		const transient GearPawn	GearPawnOwner;

cpptext
{
	/** Pull aim information from Pawn */
	virtual FVector2D GetAim(USkeletalMeshComponent* InSkelComponent);
	/** Is skeleton currently mirrored */
	virtual UBOOL IsMirrored(USkeletalMeshComponent* InSkelComponent);
}
 
defaultproperties
{
}
