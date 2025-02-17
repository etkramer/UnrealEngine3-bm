/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class SeqAct_HydraTentacleGrab extends SequenceAction;

/** Tentacle to do the grabbing */
var()	EHydraTentacle	Tentacle;
/** Actor that wants grabbing */
var		Actor			GrabActor;

defaultproperties
{
	ObjName="Hydra: Tentacle Grab"
	ObjCategory="Boss"

	InputLinks(0)=(LinkDesc="Grab")
	InputLinks(1)=(LinkDesc="Release")

	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="GrabTarget",PropertyName=GrabActor,MinVars=1,MaxVars=1)
}