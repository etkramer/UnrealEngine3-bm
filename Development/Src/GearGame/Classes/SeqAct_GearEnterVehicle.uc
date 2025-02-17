/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class SeqAct_GearEnterVehicle extends SequenceAction;

var Actor TargetVehicle;

/** index of the seat of the vehicle the enteree should use, or -1 for auto-select */
var() int SeatIndex;


/**
 * Return the version number for this class.  Child classes should increment this method by calling Super then adding
 * a individual class version to the result.  When a class is first created, the number should be 0; each time one of the
 * link arrays is modified (VariableLinks, OutputLinks, InputLinks, etc.), the number that is added to the result of
 * Super.GetObjClassVersion() should be incremented by 1.
 *
 * @return	the version number for this specific class.
 */
static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 0;
}

event Activated()
{
	local GearPC PC;

	//@HACK: trying to work around bug in Escape where you end up with no Marcus Pawn only when playing from the start of the game...
	foreach GetWorldInfo().LocalPlayerControllers(class'GearPC', PC)
	{
		if ((PC.Pawn == None || PC.Pawn.bDeleteMe) && !PC.IsInState('Dead'))
		{
			`Warn("Attempting Escape missing Pawn hack!!!!!!!!");
			PC.Pawn = None;
			GetWorldInfo().Game.RestartPlayer(PC);
			if (GearPawn(PC.Pawn) != None)
			{
				GearPawn(PC.Pawn).OnGearEnterVehicle(self);
			}
		}
		break;
	}
}

defaultproperties
{
	ObjName="Enter Vehicle"
	ObjCategory="Gear"

	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Vehicle",PropertyName=TargetVehicle)

	SeatIndex=-1
}
