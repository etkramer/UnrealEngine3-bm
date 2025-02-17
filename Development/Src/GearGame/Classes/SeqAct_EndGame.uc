/**
 * ends the game and returns to the menu
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class SeqAct_EndGame extends SequenceAction;

event Activated()
{
	local GearPC PC;

	// return to the party lobby instead if playing co-op
	if (GetWorldInfo().Game.NumPlayers > 1)
	{
		GetWorldInfo().Game.TellClientsToReturnToPartyHost();
	}
	else
	{
		foreach GetWorldInfo().LocalPlayerControllers(class'GearPC', PC)
		{
			PC.ReturnToMainMenu();
			break;
		}
	}
}

defaultproperties
{
	ObjName="End Game"
	ObjCategory="Gear"
	bCallHandler=false

	VariableLinks.Empty
}
