/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class SeqAct_AIMelee extends SequenceAction;

/** List of targets to choose from. */
var() Actor MeleeTarget;

defaultproperties
{
	ObjName="AI: Melee"
	ObjCategory="AI"

	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Melee Target",PropertyName=MeleeTarget)
}
