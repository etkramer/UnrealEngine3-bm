
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnim_BlendAnimsByAim extends AnimNodeSequenceBlendByAim
	native(Anim)
	hidecategories(Animations);

/** Internal cached pointer to GearPawn Owner */
var const transient GearPawn		GearPawnOwner;
/** What we are using as our Aim input. */
var() GearAnim_AimOffset.EAimInput	AimInput;

cpptext
{
	/** Pull aim information from Pawn */
	virtual FVector2D GetAim();

	// AnimNode interface
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
}

defaultproperties
{
}