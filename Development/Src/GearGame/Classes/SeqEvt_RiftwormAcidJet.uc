/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqEvt_RiftwormAcidJet extends SequenceEvent;

defaultproperties
{
	ObjName="Acid Jet"
	ObjCategory="Gear"

	bPlayerOnly=FALSE
	MaxTriggerCount=0

	OutputLinks(0)=(LinkDesc="Inhale")
	OutputLinks(1)=(LinkDesc="Exhale")
	OutputLinks(2)=(LinkDesc="Burst")
}

