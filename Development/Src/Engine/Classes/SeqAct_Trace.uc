/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_Trace extends SequenceAction
	native(Sequence);

cpptext
{
	virtual void Activated();
	virtual void DeActivated()
	{
	}
}

/** Should actors be traced against? */
var() bool bTraceActors;

/** Should the world be traced against? */
var() bool bTraceWorld;

/** What extent should be used for the trace? */
var() vector TraceExtent;

var() vector StartOffset, EndOffset;

var() editconst Object HitObject;
var() editconst float Distance;

defaultproperties
{
	ObjName="Trace"
	ObjCategory="Level"

	bTraceWorld=TRUE

	VariableLinks.Empty
	VariableLinks(0)=(LinkDesc="Start",ExpectedType=class'SeqVar_Object')
	VariableLinks(1)=(LinkDesc="End",ExpectedType=class'SeqVar_Object')
	VariableLinks(2)=(LinkDesc="HitObject",ExpectedType=class'SeqVar_Object',bWriteable=TRUE,PropertyName=HitObject)
	VariableLinks(3)=(LinkDesc="Distance",ExpectedType=class'SeqVar_Float',bWriteable=TRUE,PropertyName=Distance)

	OutputLinks(0)=(LinkDesc="Not Obstructed")
	OutputLinks(1)=(LinkDesc="Obstructed")
}
