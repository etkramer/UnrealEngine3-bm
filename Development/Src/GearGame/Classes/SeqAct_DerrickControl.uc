/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_DerrickControl extends SequenceAction;

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
	return Super.GetObjClassVersion() + 1;
}

defaultproperties
{
	ObjName="Derrick Controls"
	ObjCategory="Gear"

	InputLinks.Empty
	InputLinks(0)=(LinkDesc="Start Engine")
	InputLinks(1)=(LinkDesc="Stop Engine")

	InputLinks(2)=(LinkDesc="Open Hatch")
	InputLinks(3)=(LinkDesc="Close Hatch")

	InputLinks(4)=(LinkDesc="Open Cabin Door")
	InputLinks(5)=(LinkDesc="Close Cabin Door")

	InputLinks(6)=(LinkDesc="Left Arm Prepare")
	InputLinks(7)=(LinkDesc="Left Arm Deploy")

	InputLinks(8)=(LinkDesc="Right Arm Prepare")
	InputLinks(9)=(LinkDesc="Right Arm Deploy")
}
