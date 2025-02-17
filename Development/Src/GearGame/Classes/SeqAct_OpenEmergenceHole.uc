/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class SeqAct_OpenEmergenceHole extends SequenceAction;

/** Whether or not this hole should open immediately **/
var() bool bOpenImmediately;


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
	return Super.GetObjClassVersion() + 5;
}

defaultproperties
{
	ObjName="Open/Close E-Hole"
	ObjCategory="Gear"

	InputLinks(0)=(LinkDesc="Open")
	InputLinks(1)=(LinkDesc="Close")
}
