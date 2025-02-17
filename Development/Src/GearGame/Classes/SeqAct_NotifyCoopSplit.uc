/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_NotifyCoopSplit extends SequenceAction
	native(Sequence);

/** whether this is a solo split (i.e. it's game over if the player goes DBNO because there's no one to revive them) */
var() bool bSoloSplit;

cpptext
{
	void Activated();
};


defaultproperties
{
	bCallHandler=FALSE

	ObjName="Notify Co-op Split"
	ObjCategory="Gear"

	InputLinks(0)=(LinkDesc="Split Started")
	InputLinks(1)=(LinkDesc="Split Ended")

	VariableLinks.Empty
}
