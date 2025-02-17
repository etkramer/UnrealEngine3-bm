/**
 * This action retrieves the number of elements in a list.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_GetListItemCount extends UIAction
	native(inherit);

cpptext
{
	/**
	 * Activated handler, copies the number of items in the list to the output variable link.
	 */
	virtual void Activated();
}

/** Output variable for the number of items in the attached list. */
var int	ItemCount;

DefaultProperties
{
	ObjName="Get List Item Count"

	// add a variable link to receive the value of the list's selected item
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Item Count",PropertyName=ItemCount,bWriteable=true))
}
