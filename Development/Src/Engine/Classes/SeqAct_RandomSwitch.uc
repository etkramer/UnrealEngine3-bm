/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_RandomSwitch extends SeqAct_Switch
	native(Sequence);

/** List of indices we've already used once and disabled (for when bLooping and bAutoDisableLinks are both checked) **/
var array<INT> AutoDisabledIndices; 

cpptext
{
	virtual void Activated();
};


defaultproperties
{
	ObjName="Random"
	ObjCategory="Switch"
	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Active Link",bWriteable=true,MinVars=0,PropertyName=Indices)
}
