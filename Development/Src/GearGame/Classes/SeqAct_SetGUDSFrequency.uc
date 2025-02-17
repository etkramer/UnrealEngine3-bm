/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_SetGUDSFrequency extends SequenceAction
	native(Sequence);

/** Frequency multiplier for entire GUDS system */
var() float					GlobalFrequencyMultiplier;

cpptext
{
	void Activated();
};

defaultproperties
{
	bCallHandler=FALSE

	ObjName="Set Global GUDS Frequency"
	ObjCategory="Sound"

	InputLinks(0)=(LinkDesc="In")

	VariableLinks.Empty
}