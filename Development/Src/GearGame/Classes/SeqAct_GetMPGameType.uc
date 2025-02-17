/**
 * Action to get the currently played MP gametype
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_GetMPGameType extends SequenceAction
	native(Sequence);

cpptext
{
	virtual void Activated();
}

var		class<GearGame>			CurrentGameClass;

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
	return Super.GetObjClassVersion() + 3;
}

defaultproperties
{
	ObjName="Get MP Gametype"
	ObjCategory="Multiplayer"

	VariableLinks.Empty

	// all of the different MP gametypes
	OutputLinks(0)=(LinkDesc="Error")
	OutputLinks(1)=(LinkDesc="Warzone")
	OutputLinks(2)=(LinkDesc="Assassination")
	OutputLinks(3)=(LinkDesc="Annex")
	OutputLinks(4)=(LinkDesc="Horde")
	OutputLinks(5)=(LinkDesc="Wingman")
	OutputLinks(6)=(LinkDesc="Meatflag")
	OutputLinks(7)=(LinkDesc="KOTH")
	OutputLinks(8)=(LinkDesc="Execution")
	OutputLinks(9)=(LinkDesc="TrainingGrounds")

	VariableLinks.Add((LinkDesc="Game Class",ExpectedType=class'SeqVar_Object',PropertyName=CurrentGameClass,bWriteable=true))

	bAutoActivateOutputLinks=false
}
