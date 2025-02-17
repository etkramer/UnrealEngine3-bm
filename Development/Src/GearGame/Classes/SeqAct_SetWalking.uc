/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class SeqAct_SetWalking extends SequenceAction;

var() bool bWalking;

defaultproperties
{
	ObjName="Set Walking"
	ObjCategory="Pawn"

	VariableLinks(1)=(ExpectedType=class'SeqVar_Bool',LinkDesc="Walking",PropertyName=bWalking)
}
