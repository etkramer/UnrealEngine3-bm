/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqCond_GetServerType extends SequenceCondition
	native(Sequence);

cpptext
{
	virtual void Activated();
}

defaultproperties
{
	ObjName="Server Type"

	OutputLinks(0)=(LinkDesc="Standalone")
	OutputLinks(1)=(LinkDesc="Dedicated Server")
	OutputLinks(2)=(LinkDesc="Listen Server")
}
