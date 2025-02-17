
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Brumak_OverlayLftArmSwing extends GSM_Brumak_MeleeAttack;

defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="",AnimName[BS_Additive]="Upper_Arm_Swing_Lft")
	BlendInTime=0.33f
	BlendOutTime=0.67f

	NearbyPlayerSynchedCameraAnimName="Left_Arm_Swing_Upperbody"
	NearbyPlayerSynchedCameraAnimRadius=(X=1280,Y=2560)

	bCheckForGlobalInterrupts=FALSE

	bLockPawnRotation=FALSE
	bDisablePhysics=FALSE
	bDisableMovement=FALSE
}