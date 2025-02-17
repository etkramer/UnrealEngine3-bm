/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/** Used to allow level designers to issue achievements */
class UTSeqAct_UnlockChar extends SequenceAction
	native(Sequence);

/** The character to unlock (needs to be in list in UTCustomChar_Data::UnlockableChars)*/
var() string	Char;

cpptext
{
	virtual void Activated();
};


defaultproperties
{
	ObjName="Unlock Char"
	ObjCategory="Misc"
}
