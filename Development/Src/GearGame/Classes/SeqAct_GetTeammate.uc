/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_GetTeammate extends SequenceAction
	native(Sequence);

cpptext
{
	virtual void Activated();
	virtual void DeActivated()
	{
		// do nothing, outputs activated in Activated()
	}
}

/** Required inventory to filter teammates by */
var() array<class<Inventory> > RequiredInventory;

defaultproperties
{
	ObjName="Get Teammate"
	ObjCategory="Pawn"

	OutputLinks(1)=(LinkDesc="Failed")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Player")
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Teammate",bWriteable=true)
}
