/** 
 * Helper class for curb stomp victim so they don't get up out of the special move 
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_React_CurbStompVictim extends AICommand_SpecialMove
	within GearAI;

function Pushed()
{
	Super.Pushed();

	// Don't let them get up
	Pawn.ClearTimer( 'GetBackUpFromKnockDown' );
}

defaultproperties
{
}