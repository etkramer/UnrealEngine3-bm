/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_GetPlayerTeam extends SequenceAction
	native(Sequence);

cpptext
{
	virtual void Activated();
}

defaultproperties
{
	ObjName="Get Player Team"
	ObjCategory="Pawn"

	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Team",bWriteable=TRUE)
}
