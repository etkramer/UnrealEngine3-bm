/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


/**
 * This action gets the widget that was last focused for a specified parent.
 */
class UIAction_GetLastFocused extends UIAction_FocusActions
	native(inherit);

/** The item that was last focused, this var is used for the writeable variable link. */
var		UIScreenObject		LastFocused;

/**
 * Specifies whether the desired result is the target's focused control or the innermost focused control of the target.
 */
var()	bool				bRecursiveSearch;

cpptext
{
	/**
	 * Gets the control that was previously focused in the specified parent and stores it in LastFocused.
	 */
	virtual void Activated();
}

DefaultProperties
{
	ObjName="Get Last Focused Child"

	OutputLinks(FOCUSACTRESULT_Success)=(LinkDesc="Success")
	OutputLinks(FOCUSACTRESULT_Failure)=(LinkDesc="Failed")

	// add a variable link which will receive the value of the last focused widget.
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Last Focused Child",PropertyName=LastFocused,bWriteable=true,MaxVars=1))
}
