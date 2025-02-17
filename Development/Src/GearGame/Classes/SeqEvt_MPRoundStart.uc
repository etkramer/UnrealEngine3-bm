/**
* Activated twice when a multiplayer match's round begins:
*  - "Countdown" (when the round countdown to start the round begins)
*  - "Gameplay" (when the round actually starts in players start controlling their character)
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class SeqEvt_MPRoundStart extends SequenceEvent
	native(Sequence);

cpptext
{
	UBOOL RegisterEvent();
}

defaultproperties
{
	ObjName="MP Round Start"
	ObjCategory="Multiplayer"

	VariableLinks.Empty

	// when the round countdown to start begins
	OutputLinks(0)=(LinkDesc="Countdown")
	OutputLinks(1)=(LinkDesc="Gameplay")

	bPlayerOnly=false
	MaxTriggerCount=0
}