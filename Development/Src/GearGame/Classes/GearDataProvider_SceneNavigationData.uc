/**
 * Provides information about a single navigation menu item in the UI.  The name of this data provider should
 * match the tag of the scene it is used in.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearDataProvider_SceneNavigationData extends UIResourceDataProvider
	Config(UI)
	PerObjectConfig
	native(UIPrivate);

/**
 * Defines the parameters for a single auto-generated label button in a UIScene.
 */
struct native strictconfig NavigationItemData
{
	/**
	 * The name to associate with this item; for now it's solely to help humans keep things straight, but
	 * we may find another use eventually.
	 */
	var		config		string		ItemTag;

	/**
	 * The pathname to the scene that should be opened when this navigation item is selected.
	 */
	var		config		string		DestinationScenePath;

	/**
	 * A unique string value used for finding a specific element in a scene navigation dataprovider, for purposes of marking
	 * it to display the attract icon.
	 */
	var		config		string		AttractMarkId;

	/**
	 * Cached reference to the scene that is referred to by DestinationScenePath.
	 */
	var		transient	UIScene		DestinationScene;

	/**
	 * The string to display as the text for this item in the UI.
	 */
	var		localized	databinding	string		DisplayName;

	/**
	 * A short description of what this item is for.
	 */
	var		localized	databinding	string		ItemHelpText;

	/**
	 * Indicates that there is something new or interesting to see in the scene linked to this navigation item.
	 */
	var					databinding	bool		bDisplayAttractIcon;

	/**
	 * Indicates that this item is currently disabled.
	 */
	var					databinding	bool		bItemDisabled;
};

/**
 * This list of items that will be shown on the menu associated with this scene navigation data provider.
 */
var		config		databinding		array<NavigationItemData>		NavigationItems;

/**
 * We'll put this here for now, but eventually we'll probably want to put this into the style that's applied to the list.
 */
var		config						string							AttractIconMarkup;

cpptext
{
	/* === UIDataProvider interface === */
	/**
	 * Gets the list of data fields exposed by this data provider.
	 *
	 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
	 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
	 */
	virtual void GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields );

	/**
	 * Resolves the value of the data field specified and stores it in the output parameter.
	 *
	 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
	 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
	 * @param	out_FieldValue	receives the resolved value for the property specified.
	 *							@see GetDataStoreValue for additional notes
	 * @param	ArrayIndex		optional array index for use with data collections
	 */
	virtual UBOOL GetFieldValue( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex=INDEX_NONE );

	/* === IUIListElementProvider interface === */
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
	 * Allows list element providers the chance to perform custom sorting of a collection of list elements.  Implementors should implement this
	 * method if they desire to perform complex sorting behavior, such as considering additional data when evaluting the order to sort the elements into.
	 *
	 * @param	CollectionDataFieldName		the name of a collection data field inside this UIListElementProvider associated with the
	 *										list items provided.  Guaranteed to one of the values returned from GetElementProviderTags.
	 * @param	ListItems					the array of list items that need sorting.
	 * @param	SortParameters				the parameters to use for sorting
	 *										PrimaryIndex:
	 *											the index [into the ListItems' Cells array] for the cell which the user desires to perform primary sorting with.
	 *										SecondaryIndex:
	 *											the index [into the ListItems' Cells array] for the cell which the user desires to perform secondary sorting with.  Not guaranteed
	 *											to be a valid value; Comparison should be performed using the value of the field indicated by PrimarySortIndex, then when these
	 *											values are identical, the value of the cell field indicated by SecondarySortIndex should be used.
	 *
	 * @return	TRUE to indicate that custom sorting was performed by this UIListElementProvider.  Custom sorting is not required - if this method returns FALSE,
	 *			the list bound to this UIListElementProvider will perform its default sorting behavior (alphabetical sorting of the desired cell values)
	 */
	virtual UBOOL SortListElements( FName CollectionDataFieldName, TArray<const struct FUIListItem>& ListItems, const struct FUIListSortingParameters& SortParameters )
	{
		// we never want our elements to be sorted
		return TRUE;
	}

	/**
	 * Determines whether a member of a collection should be considered "enabled" by subscribed lists.  Disabled elements will still be displayed in the list
	 * but will be drawn using the disabled state.
	 *
	 * @param	FieldName			the name of the collection data field that CollectionIndex indexes into.
	 * @param	CollectionIndex		the index into the data field collection indicated by FieldName to check
	 *
	 * @return	TRUE if FieldName doesn't correspond to a valid collection data field, CollectionIndex is an invalid index for that collection,
	 *			or the item is actually enabled; FALSE only if the item was successfully resolved into a data field value, but should be considered disabled.
	 */
	virtual UBOOL IsElementEnabled( FName FieldName, INT CollectionIndex );

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

	/* === IUIListElementCellProvider interface === */
	/**
	 * Retrieves the list of tags that can be bound to individual cells in a single list element.
	 *
	 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
	 *							instance provides element cells for multiple collection data fields.
	 * @param	out_CellTags	receives the list of tag/column headers that can be bound to element cells for the specified property.
	 */
	virtual void GetElementCellTags( FName FieldName, TMap<FName,FString>& out_CellTags );

	/**
	 * Retrieves the field type for the specified cell.
	 *
	 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
	 *							instance provides element cells for multiple collection data fields.
	 * @param	CellTag				the tag for the element cell to get the field type for
	 * @param	out_CellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
	 *
	 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
	 */
	virtual UBOOL GetCellFieldType( FName FieldName, const FName& CellTag, BYTE& out_CellFieldType );

	/**
	 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
	 *
	 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
	 *							instance provides element cells for multiple collection data fields.
	 * @param	CellTag			the tag for the element cell to resolve the value for
	 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
	 *							do not provide unique UIListElement objects for each element.
	 * @param	out_FieldValue	receives the resolved value for the property specified.
	 *							@see GetDataStoreValue for additional notes
	 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
	 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
	 *							to a data collection.
	 */
	virtual UBOOL GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex=INDEX_NONE );
}

/* == Delegates == */

/* == Natives == */

/* == Events == */

/* == UnrealScript == */
/**
 * Allows script only data stores to indicate whether they'd like to handle a property which is not natively supported.
 *
 * @param	UnsupportedProperty		the property that isn't supported natively
 *
 * @return	TRUE if this data provider wishes to perform custom logic to handle the property.
 */
function bool CheckComplexPropertyTypeSupport( Property UnsupportedProperty )
{
	local bool bResult;

	if ( UnsupportedProperty != None && UnsupportedProperty.Name == 'NavigationItems' )
	{
		bResult = true;
	}

	return bResult;
}

/**
 * Wrapper for checking whether a specific item in this providers list of scene navigation data structures has the "new item" flag set.
 *
 * @param	AttractMarkId	a string representing the type of navigation item to search for; should match the AttractMarkId value for one
 *							of this provider's array elements.
 *
 * @return	TRUE if the item with the specified ID has been marked has having new items to view.
 */
function bool IsNavItemAttracting( string AttractMarkId )
{
	return IsNavItemAtIndexAttracting(FindNavItemIndexByAttractId(AttractMarkId));
}
/** Alternate version of IsNavItemAttracting which takes an index into this provider's NavigationItems array */
function bool IsNavItemAtIndexAttracting( int ItemIndex )
{
	local bool bResult;

	if ( ItemIndex >= 0 && ItemIndex < NavigationItems.Length )
	{
		bResult = NavigationItems[ItemIndex].bDisplayAttractIcon;
	}

	return bResult;
}

/**
 * Accessor for getting the AttractMarkId of the NavigationItem specified.
 *
 * @param	ItemIndex	index into the NavigationItems array for the item to lookup.
 */
function string GetAttractMarkId( int ItemIndex )
{
	local string Result;

	if ( ItemIndex >= 0 && ItemIndex < NavigationItems.Length )
	{
		Result = NavigationItems[ItemIndex].AttractMarkId;
	}

	return Result;
}

/**
 * Finds the index [into the NavigationItems array] for the nav item with the matching value for AttractMarkId.
 *
 * @param	AttractMarkId	a string representing the type of navigation item to search for; should match the AttractMarkId value for one
 *							of this provider's array elements.
 *
 * @return	index for the item with the matching attractmark ID, or INDEX_NONE if it wasn't found.
 */
function int FindNavItemIndexByAttractId( string AttractMarkId )
{
	local int Idx, Result;

	Result = INDEX_NONE;

	if ( AttractMarkId != "" )
	{
		for ( Idx = 0; Idx < NavigationItems.Length; Idx++ )
		{
			if ( NavigationItems[Idx].AttractMarkId ~= AttractMarkId )
			{
				Result = Idx;
				break;
			}
		}
	}

	return Result;
}

/**
 * Changes the attract mark flag for the nav item with the specified mark id.
 *
 * @param	AttractMarkId	a string representing the type of navigation item to search for; should match the AttractMarkId value for one
 *							of this provider's array elements.
 * @param	bEnable			indicates whether the attract mark flag should be set.
 *
 * @return	TRUE if the flag was successfully set on the navigation item.
 */
function bool SetAttractMark( string AttractMarkId, optional bool bEnable=true )
{
	return SetAttractMarkAtIndex(FindNavItemIndexByAttractId(AttractMarkId), bEnable);
}
/** Alternate version of SetAttractMark which takes an index into the NavigationItems array */
function bool SetAttractMarkAtIndex( int ItemIndex, optional bool bEnable=true )
{
	local bool bResult, bDataChanged;

	if ( ItemIndex >= 0 && ItemIndex < NavigationItems.Length )
	{
		bDataChanged = NavigationItems[ItemIndex].bDisplayAttractIcon != bEnable;

		NavigationItems[ItemIndex].bDisplayAttractIcon = bEnable;
		bResult = true;

		if ( bDataChanged )
		{
			NotifyPropertyChanged('NavigationItems');
		}
	}
	else
	{
		`warn(`location @ "Invalid index specified (" $ ItemIndex $ "):" @ NavigationItems.Length @ "items available");
	}

//	`log(`location @ `showvar(ItemIndex) @ `showvar(bEnable) @ `showvar(NavigationItems[ItemIndex].AttractMarkId,AttractMarkId) @ `showvar(NavigationItems[ItemIndex].DisplayName));
	return bResult;
}

/* == SequenceAction handlers == */


DefaultProperties
{
	CanSupportComplexPropertyType=CheckComplexPropertyTypeSupport

	//@fixme ronp - it might be better to handle this via the script custom handlers
	ComplexPropertyTypes.Remove(class'ArrayProperty')
}
