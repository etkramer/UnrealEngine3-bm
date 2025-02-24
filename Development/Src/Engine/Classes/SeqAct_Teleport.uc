/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_Teleport extends SequenceAction;

var() bool bUpdateRotation;

// BM1
var() bool bDontResetCamera;
var() bool bDontResetState;
var() bool bSnapPlayerAnim;
var() bool bStopAllMovement;

defaultproperties
{
	ObjName="Teleport"
	ObjCategory="Actor"
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Destination")
	bUpdateRotation=TRUE
	bStopAllMovement=true
}
