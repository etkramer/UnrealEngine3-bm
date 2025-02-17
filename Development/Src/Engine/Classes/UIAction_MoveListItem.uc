/**
 * Action for moving elements within a UIList.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_MoveListItem extends UIAction
	native(inherit);

cpptext
{
	/**
	 * Executes the logic for this action.
	 */
	virtual void Activated();
}

/**
 * The index of the element to move.
 */
var()		int				ElementIndex;

/**
 * The number of places to move it
 */
var()		int				MoveCount;

DefaultProperties
{
	ObjName="Move Element"
	ObjCategory="List"

	bAutoActivateOutputLinks=false
	bAutoTargetOwner=true

	ElementIndex=INDEX_NONE

	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Element Index",PropertyName=ElementIndex))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Count",PropertyName=MoveCount))

	OutputLinks(0)=(LinkDesc="Failure")
	OutputLinks(1)=(LinkDesc="Success")
}
