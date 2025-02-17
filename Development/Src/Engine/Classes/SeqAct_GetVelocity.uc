/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_GetVelocity extends SequenceAction
	native(Sequence);

cpptext
{
	void Activated();
}

var() editconst float Velocity;

defaultproperties
{
	ObjName="Get Velocity"
	ObjCategory="Actor"
	VariableLinks(1)=(ExpectedType=class'SeqVar_Float',LinkDesc="Velocity",bWriteable=true,PropertyName=Velocity)
}
