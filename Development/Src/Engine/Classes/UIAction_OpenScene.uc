/**
 * Opens a new scene.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_OpenScene extends UIAction_Scene
	native(inherit);

/** Output variable for the scene that was opened. */
var	UIScene		OpenedScene;

cpptext
{
	/* === USequenceOp interface === */
	/**
	 * Opens the scene specified by this action.
	 *
	 * @note: this action must be safe to execute from outside the scope of the UI system, since it can be used
	 *			in level sequences.
	 */
	virtual void Activated();

	/**
	 * Called after all the op has been deactivated and all linked variable values have been propagated to the next op
	 * in the sequence.
	 *
	 * This version clears the value of OpenedScene.
	 */
    virtual void PostDeActivated();
}

DefaultProperties
{
	ObjName="Open Scene"

	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Opened Scene",PropertyName=OpenedScene,bWriteable=true))

}
