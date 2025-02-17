/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class SeqCond_GetDifficulty extends SequenceCondition
	native(Sequence);

cpptext
{
	virtual void Activated();
}

defaultproperties
{
	ObjName="Difficulty Setting"

	OutputLinks(0)=(LinkDesc="Casual")
	OutputLinks(1)=(LinkDesc="Normal")
	OutputLinks(2)=(LinkDesc="Hardcore")
	OutputLinks(3)=(LinkDesc="Insane");
}
