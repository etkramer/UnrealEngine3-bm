/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_JackControl extends SeqAct_Latent;

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
	ObjName="Jack Control"
	ObjCategory="Gear"

	InputLinks(0)=(LinkDesc="Cloak")
	InputLinks(1)=(LinkDesc="Decloak")
	InputLinks(2)=(LinkDesc="Lights On")
	InputLinks(3)=(LinkDesc="Lights Off")
	InputLinks(4)=(LinkDesc="Unfold Monitor")
	InputLinks(5)=(LinkDesc="Fold Monitor")
	InputLinks(6)=(LinkDesc="Start Weld")
	InputLinks(7)=(LinkDesc="Stop Weld")
	InputLinks(8)=(LinkDesc="Recoil")
	InputLinks(9)=(LinkDesc="Start Scan")
	InputLinks(10)=(LinkDesc="Stop Scan")
	InputLinks(11)=(LinkDesc="Point")
	InputLinks(12)=(LinkDesc="Release")
	
}
