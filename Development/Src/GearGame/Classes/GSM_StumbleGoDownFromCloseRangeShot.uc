
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_StumbleGoDownFromCloseRangeShot extends GSM_StumbleGoDownFromExplosion
	config(Pawn);

defaultproperties
{
	BS_DropAnimation=(AnimName[BS_FullBody]="AR_Injured_Drop")
	BS_Idle=(AnimName[BS_FullBody]="AR_Injured_Idle")
}
