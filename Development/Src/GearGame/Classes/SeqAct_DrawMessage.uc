/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_DrawMessage extends SequenceAction
	native(Sequence);

var() string MessageText;
var() float DisplayTimeSeconds;

cpptext
{
	UBOOL UpdateOp(FLOAT deltaTime);
	virtual void Activated();
};


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
	ObjName="Draw Message"
	ObjCategory="Gear"

	InputLinks(0)=(LinkDesc="Show")
	InputLinks(1)=(LinkDesc="Hide")

	OutputLinks(1)=(LinkDesc="Time Expired")
	DisplayTimeSeconds=-1
	bLatentExecution=TRUE
	bAutoActivateOutputLinks=FALSE
}
