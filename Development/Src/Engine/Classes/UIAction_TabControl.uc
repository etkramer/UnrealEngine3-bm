/**
 * Base class for all actions which operate on UITabControl, UITabButton, or UITabPage.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_TabControl extends UIAction
	abstract;

/** Reference to the tab control to act on */
var()	UITabControl	TabControl;

DefaultProperties
{
	ObjName="Tab Control Action"
	ObjCategory="Tab Control"

	// don't call any handler functions
	bCallHandler=false

	// not expecting to be placed in a UITabControl sequence very often, so disable this feature.
	bAutoTargetOwner=false

	// we activate our own output links
	bAutoActivateOutputLinks=false

	// We'll use individual variable links instead of an array - remove the Targets variable link
	VariableLinks.RemoveIndex(0)

	// the tab control to perform the operation to
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Tab Control",PropertyName=TabControl))

	OutputLinks(0)=(LinkDesc="Success")
	OutputLinks(1)=(LinkDesc="Failure")
}
