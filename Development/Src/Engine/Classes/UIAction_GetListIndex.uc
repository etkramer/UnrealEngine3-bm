/**
 * This action retrieves the index and value of the item at the index for the list that activated this action.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_GetListIndex extends UIAction_GetValue
	native(inherit);

cpptext
{
	/**
	 * Copies the value of the current element for the UILists in the Targets array to the Selected Item variable link.
	 */
	virtual void Activated();
}

/**
 * Return the version number for this class.  Child classes should increment this method by calling Super then adding
 * a individual class version to the result.  When a class is first created, the number should be 0; each time one of the
 * link arrays is modified (VariableLinks, OutputLinks, InputLinks, etc.), the number that is added to the result of
 * Super.GetObjClassVersion() should be incremented by 1.
 *
 * @return	the version number for this specific class.
 */
static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 1;
}

DefaultProperties
{
	ObjName="Selected List Item"

	bAutoTargetOwner=true

	// add a variable link to receive the value of the list's selected item
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Current Item",bWriteable=true))

	// add a variable link to receive the index of the list
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Current Index",bWriteable=true,bHidden=true))

	bAutoActivateOutputLinks=false
	OutputLinks(0)=(LinkDesc="Valid")
	OutputLinks(1)=(LinkDesc="Invalid")
}
