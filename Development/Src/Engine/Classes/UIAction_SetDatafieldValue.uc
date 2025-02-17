/**
 * This class allows designers to change the value of a data field in a data store.  Whichever object
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_SetDatafieldValue extends UIAction_DataStoreField
	native(inherit);

cpptext
{
	/**
	 * Resolves the datastore specified by DataFieldMarkupString, and copies the value from DataFieldStringValue/ImageValue
	 * to the resolved data provider.
	 */
	virtual void Activated();
}

/**
 * The value of the specified field in the resolved data provider, if the datafield's value corresponds to a string.
 */
var()				string				DataFieldStringValue;

/**
 * The value of the specified field in the resolved data provider, if the datafield's value corresponds to an image.
 */
var()				Surface				DataFieldImageValue;

/**
 * The value of the specified field in the resolved data provider, if the datafield's value corresponds to a collection.
 */
var()				array<int>			DataFieldArrayValue;

/**
 * The value of the specified field in the resolved data provider, if the datafield's value corresponds to a range value.
 */
var()				UIRoot.UIRangeData	DataFieldRangeValue;

/**
 * Value for the field, when type is a UniqueNetId
 */
var()	editconst	UniqueNetId			DataFieldNetIdValue;

/**
 * If TRUE, immediately calls Commit on the owning data store.
 */
var()				bool				bCommitValueImmediately;

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
	return Super.GetObjClassVersion() + 2;
}

DefaultProperties
{
	ObjName="Set Datastore Value"

	// add a variable link which will receive the value of the datafield if it corresonds to string data
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="String Value",PropertyName=DataFieldStringValue,MaxVars=1))

	// add a variable link which will receive the value of the datafield, if it corresponds to image data
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Image Value",PropertyName=DataFieldImageValue,MaxVars=1))

	// add a variable link which will receive the value of the datafield, if it corresponds to array data
	//@todo ronp - support this
//	VariableLinks.Add((ExpectedType=class'SeqVar_Array',LinkDesc="Image Value",PropertyName=DataFieldArrayValue,MaxVars=1))

	// add a variable link which will receive the avlue of the datafield if it corresponds to range data
	VariableLinks.Add((ExpectedType=class'SeqVar_UIRange',LinkDesc="Range Value",PropertyName=DataFieldRangeValue,MaxVars=1))

	VariableLinks.Add((ExpectedType=class'SeqVar_UniqueNetId',LinkDesc="NetId Value",PropertyName=DataFieldNetIdValue,MaxVars=1))

	OutputLinks.Add((LinkDesc="Success"))
}
