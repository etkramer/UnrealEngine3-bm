
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_MantleOverLocust extends GSM_MidLvlJumpOver
	native(SpecialMoves)
	config(Pawn);

var config		bool	bForceMantleVariation;

/** Returns specialized ideal camera origin for when this special move is playing */
simulated native function vector GetIdealCameraOrigin();

// Base function to handle mantle over variations.
function INT PackSpecialMoveFlags()
{
	if( bForceMantleVariation )
	{
		return 1;
	}

	return Rand(BSList_Jump.Length);
}

/**
 * Use SpecialMove flags so server and client versions are in sync.
 */
simulated function INT PickVariationIndex()
{
	return PawnOwner.SpecialMoveFlags;
}

defaultproperties
{
	BSList_Jump(0)=(AnimName[BS_FullBody]="AR_Mantle_Start")
	BSList_Fall(0)=(AnimName[BS_FullBody]="AR_Mantle_Mid")
	BSList_Land(0)=(AnimName[BS_FullBody]="AR_Mantle_End")
	BSList_MirroredJump(0)=(AnimName[BS_FullBody]="AR_Mantle_Mirrored_Start")
	BSList_MirroredFall(0)=(AnimName[BS_FullBody]="AR_Mantle_Mirrored_Mid")
	BSList_MirroredLand(0)=(AnimName[BS_FullBody]="AR_Mantle_Mirrored_End")

	BSList_Jump(1)=(AnimName[BS_FullBody]="AR_Mantle_B_Start")
	BSList_Fall(1)=(AnimName[BS_FullBody]="AR_Mantle_B_Mid")
	BSList_Land(1)=(AnimName[BS_FullBody]="AR_Mantle_B_End")
	BSList_MirroredJump(1)=(AnimName[BS_FullBody]="AR_Mantle_B_Mirrored_Start")
	BSList_MirroredFall(1)=(AnimName[BS_FullBody]="AR_Mantle_B_Mirrored_Mid")
	BSList_MirroredLand(1)=(AnimName[BS_FullBody]="AR_Mantle_B_Mirrored_End")
}