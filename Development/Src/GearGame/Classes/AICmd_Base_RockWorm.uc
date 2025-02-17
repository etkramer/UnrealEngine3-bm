/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Base_RockWorm extends AICommand_Base_Combat;

function bool ShouldIgnoreTimeTransitions()
{
	return TRUE;
}


auto state Dumb
{
Begin:
	Pawn.Velocity = vect(0,0,0);
	Pawn.Acceleration = vect(0,0,0);
	SetDestinationPosition(Pawn.Location);
	Stop;
}
