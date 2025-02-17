/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_ToggleConversation extends SeqAct_Latent
	deprecated
	native(Sequence);

/**
 * This aciton is deprecated.  Functionality is now rolled into UseCommlink.
 */


var() protected const bool	bPlayerCanAbort;

var protected transient bool bAbortedByPlayer;

cpptext
{
	virtual void Activated();
	virtual UBOOL UpdateOp(FLOAT DeltaTime);
	virtual void DeActivated();
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
	ObjName="Toggle Conversation Mode"
	ObjCategory="Gear"

	InputLinks(0)=(LinkDesc="Start")
	InputLinks(1)=(LinkDesc="Stop")

	OutputLinks(0)=(LinkDesc="Out")
	OutputLinks(1)=(LinkDesc="Started")
	OutputLinks(2)=(LinkDesc="Stopped")
	OutputLinks(3)=(LinkDesc="Player Aborted")
}
