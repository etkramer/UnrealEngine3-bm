/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_ToggleConversationCamera extends SeqAct_Latent
	deprecated
	native(Sequence);

/************************************************************************/
/*  DEPRECATED.  Use SeqAct_ToggleConversation.                         */
/************************************************************************/

var protected transient bool bAbortedByPlayer;


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
	return Super.GetObjClassVersion() + 1;
}

defaultproperties
{
	ObjName="Toggle Conversation Camera"
	ObjCategory="Camera"

	InputLinks(0)=(LinkDesc="Enable")
	InputLinks(1)=(LinkDesc="Disable")

	OutputLinks(0)=(LinkDesc="Out")
	OutputLinks(1)=(LinkDesc="Enabled")
	OutputLinks(2)=(LinkDesc="Disabled")
	OutputLinks(3)=(LinkDesc="Aborted")
}
