
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_StumbleFromMelee2 extends GSM_StumbleFromMelee;

defaultproperties
{
	// We need a different class for this animation, as it uses different root motion
	// We need root motion to be consistant between client and server.
	BS_Animation=(AnimName[BS_FullBody]="AR_Injured_React2")
}