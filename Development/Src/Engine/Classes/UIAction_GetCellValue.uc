/**
 * This action allows designers to retrieve data associated with a specific cell in a UIList element.
 *
 * This action's Targets array should be linked to UIList objects.  This action builds a markup string
 * that can be used to access the data in a single cell of a UIListElementCellProvider using the values
 * assigned to CollectionIndex and CellFieldName, and writes both the resulting markup string and the actual
 * value to the output varable links.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_GetCellValue extends UIAction_DataStore
	native(inherit);

cpptext
{
	/**
	 * Builds a data store markup string which can be used to access the value of CellFieldName for the element specified
	 * by CollectionIndex from the data store bound to the UIList objects contained in the targets array, and writes either
	 * the markup string or the actual value to the output string variable.
	 */
	virtual void Activated();
}

/**
 * The index for the item to retrieve the value for from the data store bound to the this action's associated list.
 */
var()			int				CollectionIndex;

/**
 * The name of the field to retrieve the data markup for
 */
var()			name			CellFieldName;

/**
 * The data store markup text used for accessing the value of the specified cell in the list's data provider.
 */
var				string			CellFieldMarkup;

/**
 * The resolved value of the specified cell in the list's data provider, if the cell field's value corresponds to a string.
 */
var				string			CellFieldStringValue;

/**
 * The resolved value of the specified cell in the list's data provider, if the cell field's value corresponds to an image.
 */
var				Surface			CellFieldImageValue;

/**
 * The value of the specified field in the resolved data provider, if the datafield's value corresponds to a range value.
 * @todo ronp - hmmm, since this is read-only, do we need to support a range value here???
 */
var				UIRoot.UIRangeData	CellFieldRangeValue;

/**
 * The resolved value of the specified cell in the list's data provider, if the cell field's value corresponds to a UniqueNetId.
 */
var				UniqueNetId			CellFieldNetIdValue;

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
	return Super.GetObjClassVersion() + 3;
}

DefaultProperties
{

	CollectionIndex=INDEX_NONE

	bCallHandler=false

	ObjName="Get Cell Value"
	bAutoActivateOutputLinks=false

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Target",PropertyName=Targets,MaxVars=1)

	// add a variable link which allows the designer to specify the index to use for data retrieval
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Index",PropertyName=CollectionIndex,MaxVars=1))

	// add a variable link which will receive the value of the markup string necessary to access the cell data desired
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Markup",PropertyName=CellFieldMarkup,bWriteable=true))

	// add a variable link which will receive the value of cell field if it corresonds to string data
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="String Value",PropertyName=CellFieldStringValue,bWriteable=true,bHidden=true))

	// add a variable link which will receive the value of cell field, if the cell field is an image
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Image Value",PropertyName=CellFieldImageValue,bWriteable=true,bHidden=true))

	// add a variable link which will receive the value of cell field, if the cell field is a range value
	VariableLinks.Add((ExpectedType=class'SeqVar_UIRange',LinkDesc="Range Value",PropertyName=CellFieldRangeValue,bWriteable=true,bHidden=true))

	VariableLinks.Add((ExpectedType=class'SeqVar_UniqueNetId',LinkDesc="NetId Value",PropertyName=CellFieldNetIdValue,bWriteable=true,bHidden=true))

	OutputLinks(0)=(LinkDesc="Success")
	OutputLinks(1)=(LinkDesc="Failure")
}
