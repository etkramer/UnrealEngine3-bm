/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_HydraHideClaw extends SequenceAction
	deprecated;

/** Claw to hide */
var()	EHydraTentacle	Claw;


defaultproperties
{
	ObjName="Hydra: Hide Claw"
	ObjCategory="Boss"

	InputLinks(0)=(LinkDesc="Hide")
	InputLinks(1)=(LinkDesc="Unhide")
}