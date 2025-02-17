/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqEvt_TortureBargeLegHurt extends SequenceEvent;

defaultproperties
{
	ObjName="Torture Barge Leg Hurt"
	ObjCategory="Gear"

	bPlayerOnly=FALSE
	MaxTriggerCount=0

	OutputLinks(0)=(LinkDesc="RearLeft")
	OutputLinks(1)=(LinkDesc="RearRight")
	OutputLinks(2)=(LinkDesc="FrontLeft")
	OutputLinks(3)=(LinkDesc="FrontRight")
}

