/**
 * This action allows designers to add additional fields to
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_AddDataField extends UIAction_DataStoreField
	native(inherit);

cpptext
{
	/**
	 * Resolves the datastore specified by DataFieldMarkupString, and adds a new data field to the resolved data provider
	 * using the specified name.
	 */
	virtual void Activated();
}

/**
 * Indicates whether the data field should be saved to the UIDynamicFieldProvider's persistent storage area.
 */
var()	bool						bPersistentField;

/**
 * Specifies the type of data field being added.
 */
var()	EUIDataProviderFieldType	FieldType;

DefaultProperties
{
	ObjName="Add Data Field"
	FieldType=DATATYPE_Property

	OutputLinks.Add((LinkDesc="Success"))
}
