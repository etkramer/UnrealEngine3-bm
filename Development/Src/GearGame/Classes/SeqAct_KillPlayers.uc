/**
 * This will kill players or enemies or friends!
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_KillPlayers extends SequenceAction
	native(Sequence);


/** whether to gib or not **/
var() bool bShouldGib;

/** whether to kill the player **/
var() bool bKillPlayers;

/** Basically does:  "kill enemies" **/
var() bool bKillEnemies;

/** Basically does:  "kill friends" **/
var() bool bKillFriends;


cpptext
{
	void Activated();
};


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
	return Super.GetObjClassVersion() + 0;
}

defaultproperties
{
	ObjName="Kill Players / Friends / Enemeies"
	ObjCategory="Gear"

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Target",bHidden=TRUE)


	bKillPlayers=TRUE
}
