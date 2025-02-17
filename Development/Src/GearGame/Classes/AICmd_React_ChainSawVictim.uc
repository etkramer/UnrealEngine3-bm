/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_React_ChainSawVictim extends AICommand_SpecialMove
	within GearAI;

function Pushed()
{
	Super.Pushed();

	GotoState( 'Command_SpecialMove' );
}
	
state Command_SpecialMove
{
	function bool ShouldFinishRotation() { return TRUE; }

	function ESpecialMove GetSpecialMove()
	{
		return SM_ChainSawVictim;
	}
}

defaultproperties
{
}