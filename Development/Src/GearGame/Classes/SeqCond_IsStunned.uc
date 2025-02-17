/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class SeqCond_IsStunned extends SequenceCondition
	native(Sequence);

cpptext
{
	virtual void Activated();
}

defaultproperties
{
	ObjName="IsStunned"

	OutputLinks(0)=(LinkDesc="True")
	OutputLinks(1)=(LinkDesc="False")
	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Target")
}