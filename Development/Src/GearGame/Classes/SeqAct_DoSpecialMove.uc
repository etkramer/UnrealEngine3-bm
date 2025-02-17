
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class SeqAct_DoSpecialMove extends SequenceAction;

// List of special moves usable in Kismet.
enum EKismetSpecialMove
{
	EKSM_PutOnHelmet,
};

var()	EKismetSpecialMove	SpecialMoveToDo;

defaultproperties
{
	ObjName="Do Special Move!"
	ObjCategory="Gear"
}