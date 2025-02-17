/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This action creates an online game based upon the settings object that
 * is bound to the UI widget
 */
class UIAction_CreateOnlineGame extends UIAction
	native(inherit);

/** The datastore to call create on */
var() name DataStoreName;

/** Map to load before starting the online game, if blank, no map will be loaded. */
var() string MapName;

cpptext
{
	/** Callback for when the event is activated. */
	virtual void Activated();
}

defaultproperties
{
	ObjName="Create Online Game"
	ObjCategory="Online"

	bAutoTargetOwner=true

	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Map Name",PropertyName=MapName))

	bAutoActivateOutputLinks=false
	OutputLinks(0)=(LinkDesc="Failure")
	OutputLinks(1)=(LinkDesc="Success")
}
