/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class SeqEvt_WDODamageModApplied extends SequenceEvent
	native(Sequence);

/** Only fire this event if the subobject damaged matches */
var() Name SubObjectName;

/** Only fire this event if the particular specified damage mod matches */
var() Name DamageModName;

cpptext
{
	void PostEditChange(UProperty* PropertyThatChanged);
};

defaultproperties
{
	ObjName="Destruction Applied"
	ObjCategory="Gear"
	bPlayerOnly=FALSE

	OutputLinks(0)=(LinkDesc="Out")
	VariableLinks.Empty()
}