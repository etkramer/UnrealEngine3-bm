/**
* Activated when a multiplayer match's round ends
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class SeqEvt_MPRoundEnd extends SequenceEvent
	native(Sequence);

cpptext
{
	UBOOL RegisterEvent();
}

defaultproperties
{
	ObjName="MP Round End"
	ObjCategory="Multiplayer"
	VariableLinks.Empty

	bPlayerOnly=false
	MaxTriggerCount=0
}