/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_Boomer_Flame extends GearAI_Boomer;

event StartFiring(optional int InBurstsToFire = -1)
{
	// only fire a little when the enemy is way out of range
	// since if he sits there and spams, he looks dumb
	// but if he doesn't fire at all, he looks even dumber
	if (Enemy != None && VSize(Enemy.Location - Pawn.Location) > 1500.0)
	{
		InBurstsToFire = 1;
	}

	Super.StartFiring(InBurstsToFire);
}

defaultproperties
{
	DefaultCommand=class'AICmd_Base_Boomer_Flame'
	bAimAtFeet=FALSE
}
