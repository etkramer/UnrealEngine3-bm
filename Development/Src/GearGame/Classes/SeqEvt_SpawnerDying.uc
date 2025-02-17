/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class SeqEvt_SpawnerDying extends SequenceEvent;

defaultproperties
{
	ObjName="Spawner Dying"
	ObjCategory="Gear"

	MaxTriggerCount=2

	OutputLinks(0)=(LinkDesc="Dying")
	OutputLinks(1)=(LinkDesc="Dead")
}