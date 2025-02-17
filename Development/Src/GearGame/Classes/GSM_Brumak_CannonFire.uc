
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Brumak_CannonFire extends GSM_Brumak_BasePlaySingleAnim;

defaultproperties
{
	BS_Animation=(AnimName[BS_Std_Up]="big_cannon_fire",AnimName[BS_Std_Idle_Lower]="big_cannon_fire")
	BlendInTime=0.45f
	BlendOutTime=0.67f

	NearbyPlayerSynchedCameraAnimName="Cannon_Fire_Brumak"

	bLockPawnRotation=TRUE
}