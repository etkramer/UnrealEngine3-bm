
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Wretch_MidLvlJumpOver extends GSM_MidLvlJumpOver;


defaultproperties
{
	BSList_Jump(0)=(AnimName[BS_FullBody]="mantle_over_start")
	BSList_Fall(0)=(AnimName[BS_FullBody]="mantle_over_fall")
	BSList_Land(0)=(AnimName[BS_FullBody]="mantle_over_land")

	bCheckForMirrorTransition=FALSE
	BSList_MirroredJump(0)=(AnimName[BS_FullBody]="mantle_over_start")
	BSList_MirroredFall(0)=(AnimName[BS_FullBody]="mantle_over_fall")
	BSList_MirroredLand(0)=(AnimName[BS_FullBody]="mantle_over_land")
}
