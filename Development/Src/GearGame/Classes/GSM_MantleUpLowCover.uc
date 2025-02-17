
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_MantleUpLowCover extends GSM_MantleUpBase
	config(Pawn);

protected function bool InternalCanDoSpecialMove()
{
	if( PawnOwner.CoverType != CT_MidLevel || (PawnOwner.MyGearWeapon != None && PawnOwner.MyGearWeapon.WeaponType == WT_Heavy) )
	{
		return FALSE;
	}

	return Super.InternalCanDoSpecialMove();
}

defaultproperties
{
	BS_MantleUp=(AnimName[BS_FullBody]="AR_Mantle_Up")
	BS_MantleUpMirrored=(AnimName[BS_FullBody]="AR_Mantle_Up_Mirrored")
}
