
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Brumak_OverlayRtArmSwing extends GSM_Brumak_OverlayLftArmSwing;

defaultproperties
{
	BS_Animation=(AnimName[BS_Additive]="Upper_Arm_Swing_Rt")
	BlendInTime=0.25f
	BlendOutTime=0.45f

	NearbyPlayerSynchedCameraAnimName="Right_Arm_Swing_Upperbody"
	NearbyPlayerSynchedCameraAnimRadius=(X=1280,Y=2560)
}
