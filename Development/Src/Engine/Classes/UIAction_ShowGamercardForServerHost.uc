/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This action shows the gamercard of the player that create the online game
 * that is currently selected in the UI list
 */
class UIAction_ShowGamercardForServerHost extends UIAction
	native(inherit);

cpptext
{
	/** Callback for when the event is activated. */
	virtual void Activated();
}

defaultproperties
{
	ObjName="Show Server Gamercard"
	ObjCategory="Online"
	bAutoTargetOwner=true

	bAutoActivateOutputLinks=false
	OutputLinks(0)=(LinkDesc="Failure")
	OutputLinks(1)=(LinkDesc="Success")
}
