/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_BrumakControl extends SeqAct_Latent
	native(Sequence);

/** List of actors to aim/shoot at */
var array<Actor>	TargetList;
/** AI should shoot at this target */
var() bool			bFire;
/** Should use target as Brumak focus */
var() bool			bSetFocus;
/** Duration to fire side/main guns */
var() float			FireDuration;


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
	return Super.GetObjClassVersion() + 4;
}

defaultproperties
{
	ObjName="Brumak Control"
	ObjCategory="AI"

	InputLinks.Empty
	InputLinks(0)=(LinkDesc="Side Guns")
	InputLinks(1)=(LinkDesc="Main Gun")
	InputLinks(2)=(LinkDesc="Rockets")
	InputLinks(3)=(LinkDesc="Roar")
	InputLinks(4)=(LinkDesc="Flinch")
	InputLinks(5)=(LinkDesc="Release AI")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Brumak",PropertyName=Targets)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Target",PropertyName=TargetList)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Bool',LinkDesc="bFire",PropertyName=bFire,bWriteable=FALSE,MinVars=1,MaxVars=1)

	bSetFocus=TRUE
}
