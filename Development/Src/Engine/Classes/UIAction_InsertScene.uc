/**
 * Inserts a new scene into the stack at a specific location.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class UIAction_InsertScene extends UIAction_OpenScene
	native(inherit);

/** the index to insert the new scene at */
var					int		DesiredInsertIndex;

/**
 * the location that the scene was inserted at; could be different if the scene's
 * priority is lower than another scene in the stack, another scene altered the scene
 * stack order, etc.
 */
var		transient	int		ActualInsertIndex;

cpptext
{
	/* === USequenceOp interface === */
	/**
	 * Inserts the scene specified by this action into the stack at the desired location.
	 *
	 * @note: this action must be safe to execute from outside the scope of the UI system, since it can be used
	 *			in level sequences.
	 */
	virtual void Activated();
}

DefaultProperties
{
	ObjName="Insert Scene"

	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Desired Insert Index",PropertyName=DesiredInsertIndex))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Resulting Insert Index",PropertyName=ActualInsertIndex,bHidden=true,bWriteable=true))
}
