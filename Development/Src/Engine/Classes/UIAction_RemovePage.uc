/**
 * Removes an existing tab page from a tab control.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_RemovePage extends UIAction_TabControl;

/** reference to the page that should be removed */
var		UITabPage	PageToRemove;

/* == Events == */
/**
 * Called when this event is activated.  Performs the logic for this action.
 */
event Activated()
{
	Super.Activated();

	if ( TabControl != None && PageToRemove != None )
	{
		if ( TabControl.RemovePage(PageToRemove, PlayerIndex) )
		{
			if ( !OutputLinks[0].bDisabled )
			{
				OutputLinks[0].bHasImpulse = true;
			}
		}
		else
		{
			if ( !OutputLinks[1].bDisabled )
			{
				OutputLinks[1].bHasImpulse = true;
			}
		}
	}
}

DefaultProperties
{
	ObjName="Remove Page"
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Tab Page",PropertyName=PageToRemove))
}
