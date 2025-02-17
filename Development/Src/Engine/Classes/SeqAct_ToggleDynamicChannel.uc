/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_ToggleDynamicChannel extends SequenceAction
	native(Sequence);

cpptext
{
	void Activated();
};

defaultproperties
{
	ObjName="Toggle Dynamic Channel"
	ObjCategory="Toggle"

	InputLinks.Empty
	InputLinks(0)=(LinkDesc="Turn On")
	InputLinks(1)=(LinkDesc="Turn Off")
	InputLinks(2)=(LinkDesc="Toggle")

	VariableLinks.Empty
    VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Light",MinVars=1)
}
