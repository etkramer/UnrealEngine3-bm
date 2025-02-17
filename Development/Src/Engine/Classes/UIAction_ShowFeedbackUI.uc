/**
 * This action tells the online subsystem to show the feedback ui
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_ShowFeedbackUI extends UIAction;

/** the value used to identify a player to the online service */
var	UniqueNetId		RemotePlayerNetId;

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

DefaultProperties
{
	ObjName="Show Feedback UI"
	ObjCategory="Online"
	bAutoTargetOwner=true

	// the index for the player that activated this event
	VariableLinks.Add((ExpectedType=class'SeqVar_UniqueNetId',LinkDesc="Remote Player Id",PropertyName="RemotePlayerNetId"))
}
