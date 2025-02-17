/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_ChainsawAttack_Object_NoCamera extends GSM_ChainsawAttack_Object;

defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="AR_Melee_Saw_A")
	BlendInTime=0.25
	BlendOutTime=0.25

	SpecialMoveCameraBoneAnims.Empty

	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bDisableMovement=TRUE
	bDisableLook=TRUE
	bLockPawnRotation=TRUE
}
