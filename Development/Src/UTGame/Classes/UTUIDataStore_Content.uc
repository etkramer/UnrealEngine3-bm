/**
 * Specific derivation of UIDataStore to expose downloadable content data to the UI.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTUIDataStore_Content extends UIDataStore
	native(inherit)
	implements(UIListElementProvider)
	transient;

/** Reference to the dataprovider that will provide general stats details for a stats row. */
var transient UTUIDataProvider_InstalledContent InstalledContentProvider;

/** Reference to the dataprovider that will provide content that is available for download. */
var transient UTUIDataProvider_AvailableContent AvailableContentProvider;

cpptext
{
	/**
	 * Initializes the dataproviders for all of the various character parts.
	 */
	virtual void InitializeDataStore();

	/**
	 * Builds a list of available fields from the array of properties in the
	 * game settings object
	 *
	 * @param OutFields	out value that receives the list of exposed properties
	 */
	virtual void GetSupportedDataFields(TArray<FUIDataProviderField>& OutFields);

	/* === UIListElementProvider === */

	/**
	 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
	 *
	 * @return	the list of tags supported by this element provider which correspond to list element data.
	 */
	virtual TArray<FName> GetElementProviderTags();

	/**
	 * Returns the number of list elements associated with the data tag specified.
	 *
	 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
	 *						from GetElementProviderTags.
	 *
	 * @return	the total number of elements that are required to fully represent the data specified.
	 */
	virtual INT GetElementCount( FName FieldName );

	/**
	 * Retrieves the list elements associated with the data tag specified.
	 *
	 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
	 *							from GetElementProviderTags.
	 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
	 *
	 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
	 */
	virtual UBOOL GetListElements( FName FieldName, TArray<INT>& out_Elements );

	/**
	 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the available cells for this list element.
	 * Used by the UI editor to know which cells are available for binding to individual list cells.
	 *
	 * @param	FieldName		the tag of the list element data field that we want the schema for.
	 *
	 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
	 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
	 */
	virtual TScriptInterface<class IUIListElementCellProvider> GetElementCellSchemaProvider( FName FieldName );

	/**
	 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
	 * of the list element indicated by CellValueProvider.DataSourceIndex
	 *
	 * @param	FieldName		the tag of the list element data field that we want the values for
	 * @param	ListIndex		the list index for the element to get values for
	 *
	 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
	 */
	virtual TScriptInterface<class IUIListElementCellProvider> GetElementCellValueProvider( FName FieldName, INT ListIndex );

   	virtual TScriptInterface<class IUIListElementProvider> ResolveListElementProvider( const FString& PropertyName );
}

/** 
 * @param FieldName		Name of the field to return the provider for.
 *
 * @return Returns a stats element provider given its field name. 
 */
event UTUIDataProvider_SimpleElementProvider GetElementProviderFromName(name FieldName)
{
	if(FieldName=='AvailableContent')
	{
		return AvailableContentProvider;
	}
	else if(FieldName=='InstalledContent')
	{
		return InstalledContentProvider;
	}

	return None;
}

defaultproperties
{
	Tag=UTContent
}