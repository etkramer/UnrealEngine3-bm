/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_AIMoveToActor extends SeqAct_Latent
	native(Sequence);

cpptext
{
	virtual UBOOL UpdateOp(FLOAT deltaTime);
	virtual void  Activated();
}

/** Should this move be interruptable? */
var() bool bInterruptable;

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
	ObjName="Move To Actor (Latent)"
	ObjCategory="AI"

	OutputLinks(2)=(LinkDesc="Out")

	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Destination")
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Look At")
}
