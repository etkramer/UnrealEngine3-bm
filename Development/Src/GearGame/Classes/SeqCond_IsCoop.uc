/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqCond_IsCoop extends SequenceCondition
	native(Sequence);

cpptext
{
	virtual void Activated();
}

defaultproperties
{
	ObjName="Is Coop?"

	OutputLinks(0)=(LinkDesc="Yes")
	OutputLinks(1)=(LinkDesc="No")
}