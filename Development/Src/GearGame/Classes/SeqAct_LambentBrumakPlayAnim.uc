/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_LambentBrumakPlayAnim extends SequenceAction;

var()	name		AnimName;

static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 1;
}

defaultproperties
{
	ObjName="Lambent Brumak Play Anim"
	ObjCategory="Boss"

	InputLinks(0)=(LinkDesc="Play")
	InputLinks(1)=(LinkDesc="Stop")
}