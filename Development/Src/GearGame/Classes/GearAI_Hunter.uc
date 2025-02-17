/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_Hunter extends GearAI_Locust;

function SetupEnemyDistances(optional GearWeapon Wpn)
{
	Super.SetupEnemyDistances(Wpn);

	// don't mood scale short range so the hunters pull out the shotgun more when aggressive
	EnemyDistance_Short = Wpn.Range_Short;
	`AILog("- Overrode Short:"@EnemyDistance_Short);
}

defaultproperties
{
}
