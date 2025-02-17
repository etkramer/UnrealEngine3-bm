/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_ChangeSquad extends SequenceAction;

var() Name	SquadName;

/** Default tether actor when squad changes */
var() Actor TetherActor;
/** Tether distance for COGs when squad changes */
var() float TetherDistance;
/** The controller being changed should become the squad leader */
var() bool	bSquadLeader;

/**
 * Return the version number for this class.  Child classes should increment this method by calling Super then adding
 * a individual class version to the result.  When a class is first created, the number should be 0; each time one of the
 * link arrays is modified (VariableLinks, OutputLinks, InputLinks, etc.), the number that is added to the result of
 * Super.GetObjClassVersion() should be incremented by 1.
 *
 * @return	the version number for this specific class.
 */
static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 2;
}

defaultproperties
{
	ObjName="Change Squad"
	ObjCategory="Gear"

	TetherDistance=768.f
	bSquadLeader=TRUE

	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Tether",PropertyName=TetherActor)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Float',LinkDesc="Tether Dist",PropertyName=TetherDistance)
}
