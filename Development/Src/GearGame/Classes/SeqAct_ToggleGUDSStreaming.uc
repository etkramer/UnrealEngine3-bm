/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_ToggleGUDSStreaming extends SequenceAction
	native(Sequence);

/** If TRUE, All variety banks will be flushed on a Disable input.  Note that Garbage Collection will NOT be forced. */
var() bool bFlushVarietyBanksOnDisable;

cpptext
{
	virtual void Activated();
}

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

defaultproperties
{
	ObjName="Toggle GUDS Streaming"
	ObjCategory="Toggle"

	bFlushVarietyBanksOnDisable=TRUE

	InputLinks.Empty
	InputLinks(0)=(LinkDesc="Enable")
	InputLinks(1)=(LinkDesc="Disable")

	VariableLinks.Empty
}
