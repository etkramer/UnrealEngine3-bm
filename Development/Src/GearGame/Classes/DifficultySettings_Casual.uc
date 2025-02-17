/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class DifficultySettings_Casual extends DifficultySettings
	abstract;

static function ApplyDifficultySettings(Controller C)
{
	local GearAI AI;

	Super.ApplyDifficultySettings(C);

	// AI never does head shots in Casual
	AI = GearAI(C);
	if (AI != None)
	{
		AI.bCanHeadShot = false;
	}
}

defaultproperties
{
	DifficultyLevel=DL_Casual
}
