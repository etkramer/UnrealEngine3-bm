/**
 * Finds the tab page at a particular index in the tab control's Pages array.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_FindPageByIndex extends UIAction_GetPageReference;

/** the index of the page to find */
var()	int		SearchIndex;

/* == Events == */
/**
 * Called when this event is activated.  Performs the logic for this action.
 */
event Activated()
{
	if ( TabControl != None )
	{
		PageReference = TabControl.GetPageAtIndex(SearchIndex);
	}

	// call Super.Activated after setting the PageReference, as the parent version sets the PageIndex value if we have
	// a valid PageReference
	Super.Activated();
}

DefaultProperties
{
	ObjName="Find Page By Index"
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Search Index",PropertyName=SearchIndex,bWriteable=true))
}
