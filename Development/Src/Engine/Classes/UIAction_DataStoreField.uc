/**
 * This is the base class for all actions which interact with specific fields from a data provider.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_DataStoreField extends UIAction_DataStore
	native(inherit)
	abstract;

cpptext
{
	/**
	 * Resolves the specified markup string into a data store reference and the data provider portion of the markup string.
	 *
	 * @param	out_ResolvedProvider	receives a pointer to the data provider that contains the field referenced by DataFieldMarkupString
	 * @param	out_DataFieldName		receives the portion of DataFieldMarkupString that corresponds to the data field name
	 * @param	out_DataStore			allows the caller to receive a pointer to the data store resolved from the markup string.
	 *
	 * @return	TRUE if the markup was successfully resolved; FALSE otherwise.
	 */
	UBOOL ResolveMarkup( class UUIDataProvider*& out_ResolvedProvider, FString& out_DataFieldName, class UUIDataStore** out_ResolvedDataStore=NULL );
}

/**
 * The scene to use for resolving the datastore markup in DataFieldMarkupString
 */
var()			UIScene			TargetScene;

/**
 * The data store markup string corresponding to the data field to resolve.
 */
var()			string			DataFieldMarkupString;

DefaultProperties
{
	bCallHandler=false
	bAutoActivateOutputLinks=false

	// replace the "Targets" variable link with link for selecting the scene to use for resolving the markup
	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Target Scene",PropertyName=TargetScene,MaxVars=1,bHidden=true)

	// add a variable link to allow designers to pipe the value of a GetDataStoreMarkup action to this action
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Markup String",PropertyName=DataFieldMarkupString,MaxVars=1))

	OutputLinks(0)=(LinkDesc="Failure")
}
