/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqCond_HasCOGTag extends SequenceCondition
	deprecated
	native(Sequence);

cpptext
{
	void Activated();
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
	ObjName="Has COGTag"

	OutputLinks(0)=(LinkDesc="No")
	OutputLinks(1)=(LinkDesc="Yes")
	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Player(s)",MinVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="COGTag",MaxVars=1,MinVars=1)
}
