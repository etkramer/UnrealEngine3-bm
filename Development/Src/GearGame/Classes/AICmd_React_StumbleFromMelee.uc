/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_React_StumbleFromMelee extends AICommand_SpecialMove
	within GearAI;

function Pushed()
{
	Super.Pushed();

	DesiredRotation = Rotator(GetFireTargetLocation() - Pawn.Location);

	GotoState( 'Command_SpecialMove' );
}

state Command_SpecialMove
{
	function ESpecialMove GetSpecialMove()
	{
		// Pick one of the 2 variations.
		if( FRand() < 0.5f )
		{
			return SM_StumbleFromMelee;
		}

		return SM_StumbleFromMelee2;
	}

	function float GetPostSpecialMoveSleepTime()
	{
		return 0.25f;
	}
}