/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_DriveBrumak extends SequenceAction;

var	  Pawn			BrumakPawn;
var	  Actor			Driver;
var	  array<Actor>	DismountDest;

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
	return Super.GetObjClassVersion() + 3;
}

defaultproperties
{
	ObjName="Ride Brumak"
	ObjCategory="Gear"

	InputLinks(0)=(LinkDesc="Mount")
	InputLinks(1)=(LinkDesc="Dismount")

	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Brumak",PropertyName=BrumakPawn)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Driver",PropertyName=Driver)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Object',LinkDesc="Dismount Dest",PropertyName=DismountDest)
}
