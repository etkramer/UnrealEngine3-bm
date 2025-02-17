/**
 * Base class for all actions which return a reference to a UITabPage.  Each child class uses a different search mechanism.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_GetPageReference extends UIAction_TabControl
	abstract;

/** Reference to the page being searched for; NULL if not found */
var	UITabPage	PageReference;

/** Index of the page [into its tab control's Pages array] for the page that was found */
var		int		PageIndex;

/* == Events == */
/**
 * Called when this event is activated.  Performs the logic for this action.
 */
event Activated()
{
	Super.Activated();

	if ( TabControl != None && PageReference != None )
	{
		PageIndex = TabControl.FindPageIndexByPageRef(PageReference);
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

DefaultProperties
{
	PageIndex=INDEX_NONE

	ObjName="Find Page"

	// the page that was found
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Tab Page",bWriteable=true,PropertyName=PageReference))

	// receives the index of the currently active page, if needed
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Page Index",bWriteable=true,PropertyName=PageIndex,bHidden=true))
}
