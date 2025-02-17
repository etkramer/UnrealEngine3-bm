/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class SeqAct_PlayCinematic extends SequenceAction
	native(Sequence);

cpptext
{
	void Activated();
}


defaultproperties
{
	ObjName="PlayCinematic"

}