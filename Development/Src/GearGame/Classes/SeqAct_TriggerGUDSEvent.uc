/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_TriggerGUDSEvent extends SequenceAction
	native(Sequence);

/** Which event to throw. */
var() EGUDEventID		EventID;

/** Delay between event being thrown and event being handled. */
var() float				DelaySeconds;

cpptext
{
	void Activated();
};

defaultproperties
{
	EventID=GUDEvent_None
	bCallHandler=FALSE

	ObjName="Trigger GUDS Event"
	ObjCategory="Sound"

	InputLinks(0)=(LinkDesc="In")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Instigator",PropertyName=Instigator,MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Recipient",PropertyName=Recipient,MaxVars=1)
}
