/**
 * Gets the currently focused widget.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class UIAction_GetFocused extends UIAction_FocusActions
	native(inherit);

/** The widget that is currently focused; this var is used for the writeable variable link. */
var		UIScreenObject		FocusedChild;

/**
 * Specifies whether the desired result is the target's focused control or the innermost focused control of the target.
 */
var()	bool				bRecursiveSearch;

cpptext
{
	/**
	 * Gets the control that is currently focused in the specified parent and stores it in FocusedChild.
	 */
	virtual void Activated();
}

DefaultProperties
{
	ObjName="Get Focused Child"

	OutputLinks(FOCUSACTRESULT_Success)=(LinkDesc="Success")
	OutputLinks(FOCUSACTRESULT_Failure)=(LinkDesc="Failed")

	// add a variable link which will receive the value of the last focused widget.
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Focused Child",PropertyName=FocusedChild,bWriteable=true,MaxVars=1))
}
