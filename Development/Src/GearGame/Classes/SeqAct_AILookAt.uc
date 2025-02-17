/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class SeqAct_AILookAt extends SequenceAction;

/** List of targets to choose from. */
var() Actor FocusTarget;

defaultproperties
{
	ObjName="AI: Look At"
	ObjCategory="AI"

	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Focus Target",PropertyName=FocusTarget)
}
