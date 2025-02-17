/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This action tells the stats data store to resubmit its read request
 */
class UIAction_RefreshStats extends UIAction
	native(inherit);

cpptext
{
	/** Callback for when the event is activated. */
	virtual void Activated();
}

defaultproperties
{
	ObjName="Refresh Stats"
	ObjCategory="Online"
	bAutoTargetOwner=true
	bAutoActivateOutputLinks=false

	OutputLinks(0)=(LinkDesc="Failure")
	OutputLinks(1)=(LinkDesc="Success")
}
