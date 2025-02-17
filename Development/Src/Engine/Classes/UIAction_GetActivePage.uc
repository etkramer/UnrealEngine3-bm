/**
 * Returns the tab control's currently active page.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_GetActivePage extends UIAction_GetPageReference;

/* == Events == */
/**
 * Called when this event is activated.  Performs the logic for this action.
 */
event Activated()
{
	if ( TabControl != None )
	{
		PageReference = TabControl.ActivePage;
	}

	// call Super.Activated after setting the PageReference, as the parent version sets the PageIndex value if we have
	// a valid PageReference
	Super.Activated();
}

DefaultProperties
{
	ObjName="Get Active Page"
}
