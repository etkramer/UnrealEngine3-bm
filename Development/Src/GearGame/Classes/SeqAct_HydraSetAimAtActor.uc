/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_HydraSetAimAtActor extends SequenceAction;

/** New actor to aim at. */
var()	Actor	NewAimAtActor;

defaultproperties
{
	ObjName="Hydra: AimAtActor"
	ObjCategory="Boss"

	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="AimAt",PropertyName=NewAimAtActor,MaxVars=1)
}