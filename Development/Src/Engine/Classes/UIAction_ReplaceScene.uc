/**
 * Replaces an existing scene with a new scene.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class UIAction_ReplaceScene extends UIAction_OpenScene
	native(inherit);

var		UIScene		SceneInstanceToReplace;

cpptext
{
	/* === USequenceOp interface === */
	/**
	 * Replaces the scene specified by this action with a different scene.
	 *
	 * @note: this action must be safe to execute from outside the scope of the UI system, since it can be used
	 *			in level sequences.
	 */
	virtual void Activated();
}

DefaultProperties
{
	ObjName="Replace Scene"

	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Scene To Replace",PropertyName=SceneInstanceToReplace))
}
