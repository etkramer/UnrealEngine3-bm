/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_PawnMuteGUDS extends SequenceAction;
	//native(Sequence);

defaultproperties
{
//	bCallHandler=FALSE

	ObjName="Mute Pawn GUDS"
	ObjCategory="Sound"

	InputLinks(0)=(LinkDesc="Mute")
	InputLinks(1)=(LinkDesc="Unmute")
}