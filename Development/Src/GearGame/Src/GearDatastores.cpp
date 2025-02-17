/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "GearGame.h"
#include "GearGameUIClasses.h"
#include "GearGameUIPrivateClasses.h"

#include "GearGameUIClasses.h"

#include "EngineUIPrivateClasses.h"

IMPLEMENT_CLASS(UGearResourceDataProvider);
IMPLEMENT_CLASS(UGearCampaignResourceProvider);
IMPLEMENT_CLASS(UGearCollectableDataProvider);

/* ==========================================================================================================
	UGearCampaignChapterData
========================================================================================================== */
IMPLEMENT_CLASS(UGearCampaignChapterData);


/* ==========================================================================================================
UGearCampaignChapterData
========================================================================================================== */
IMPLEMENT_CLASS(UGearCampaignActData);


/* ==========================================================================================================
	UGearUIDataStore_GameResource
========================================================================================================== */
IMPLEMENT_CLASS(UGearUIDataStore_GameResource);

/* === UUIDataStore interface === */
/**
 * Resolves PropertyName into a list element provider that provides list elements for the property specified.
 *
 * @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
 *
 * @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
 *			there is no list element provider associated with the specified property.
 */
TScriptInterface<IUIListElementProvider> UGearUIDataStore_GameResource::ResolveListElementProvider( const FString& PropertyName )
{
	TScriptInterface<IUIListElementProvider> Result;

	FString NextFieldName = PropertyName, FieldTag;
	ParseNextDataTag(NextFieldName, FieldTag);
	while ( FieldTag.Len() > 0 ) 
	{
		if ( IsDataTagSupported(*FieldTag) )
		{
			// pull off the array delimiter first, so that we can lookup this provider's type
			INT InstanceIndex = ParseArrayDelimiter(FieldTag);
			FName ProviderTypeId = FName(*FieldTag);

			// if this provider type is configured to be used as a nested element provider, use the data store as the element provider
			// so that we can handle parsing off the passed in data field names
			INT ProviderTypeIndex = FindProviderTypeIndex(ProviderTypeId);
			if ( ElementProviderTypes.IsValidIndex(ProviderTypeIndex) )
			{
				FGameResourceDataProvider& ProviderTypeInfo = ElementProviderTypes(ProviderTypeIndex);
				if ( ProviderTypeInfo.bExpandProviders )
				{
					Result = this;
				}
			}

			if ( !Result && InstanceIndex != INDEX_NONE )
			{
				TArray<UUIResourceDataProvider*> ProviderInstances;
				ListElementProviders.MultiFind(ProviderTypeId, ProviderInstances, FALSE);

				if ( ProviderInstances.IsValidIndex(InstanceIndex) )
				{
					UUIResourceDataProvider* Provider = ProviderInstances(InstanceIndex);
					Result = Provider->ResolveListElementProvider(NextFieldName);
				}
			}

			if ( !Result )
			{
				Result = this;
			}
		}

		ParseNextDataTag(NextFieldName, FieldTag);
	}

	if ( !Result )
	{
		Result = Super::ResolveListElementProvider(PropertyName);
	}

	return Result;
}

/* === UUIDataProvider interface === */
/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UGearUIDataStore_GameResource::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	Super::GetSupportedDataFields(out_Fields);
}

/* === IUIListElementProvider interface === */
/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UGearUIDataStore_GameResource::GetElementProviderTags()
{
	return Super::GetElementProviderTags();
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UGearUIDataStore_GameResource::GetElementCount( FName FieldName )
{
	INT Result = INDEX_NONE;

	FString NextFieldName = FieldName.ToString(), FieldTag;
	if (ParseNextDataTag(NextFieldName, FieldTag) && IsDataTagSupported(*FieldTag) )
	{
		INT InstanceIndex = ParseArrayDelimiter(FieldTag);
		if ( InstanceIndex != INDEX_NONE )
		{
			FName InternalFieldName = FName(*NextFieldName);
			TArray<UUIResourceDataProvider*> ProviderInstances;
			ListElementProviders.MultiFind(*FieldTag, ProviderInstances);

			if ( ProviderInstances.IsValidIndex(InstanceIndex) )
			{
				UUIResourceDataProvider* Provider = ProviderInstances(InstanceIndex);
				IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(Provider);
				if ( ElementProvider != NULL )
				{
					Result = ElementProvider->GetElementCount(InternalFieldName);
				}
			}
		}
		else
		{
			Result = Super::GetElementCount(*FieldTag);
		}
	}

	if ( Result == INDEX_NONE )
	{
		Result = Super::GetElementCount(FieldName);
	}

	return Result;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UGearUIDataStore_GameResource::GetListElements( FName DataTag, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	FString NextFieldName = DataTag.ToString(), FieldTag;
	if (ParseNextDataTag(NextFieldName, FieldTag) && IsDataTagSupported(*FieldTag) )
	{
		INT InstanceIndex = ParseArrayDelimiter(FieldTag);
		if ( InstanceIndex != INDEX_NONE )
		{
			FName InternalFieldName = FName(*NextFieldName);
			TArray<UUIResourceDataProvider*> ProviderInstances;
			ListElementProviders.MultiFind(*FieldTag, ProviderInstances);

			if ( ProviderInstances.IsValidIndex(InstanceIndex) )
			{
				UUIResourceDataProvider* Provider = ProviderInstances(InstanceIndex);
				IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(Provider);
				if ( ElementProvider != NULL )
				{
					bResult = ElementProvider->GetListElements(InternalFieldName, out_Elements);
				}
			}
		}
		else
		{
			bResult = Super::GetListElements(*FieldTag, out_Elements);
		}
	}

	return bResult || Super::GetListElements(DataTag, out_Elements);
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
UBOOL UGearUIDataStore_GameResource::IsElementEnabled( FName FieldName, INT CollectionIndex )
{
	UBOOL bResult = FALSE;

	FString NextFieldName = FieldName.ToString(), FieldTag;
	if (ParseNextDataTag(NextFieldName, FieldTag) && IsDataTagSupported(*FieldTag) )
	{
		INT InstanceIndex = ParseArrayDelimiter(FieldTag);
		if ( InstanceIndex != INDEX_NONE )
		{
			FName InternalFieldName = FName(*NextFieldName);
			TArray<UUIResourceDataProvider*> ProviderInstances;
			ListElementProviders.MultiFind(*FieldTag, ProviderInstances);

			if ( ProviderInstances.IsValidIndex(InstanceIndex) )
			{
				UUIResourceDataProvider* Provider = ProviderInstances(InstanceIndex);
				IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(Provider);
				if ( ElementProvider != NULL )
				{
					bResult = ElementProvider->IsElementEnabled(InternalFieldName, CollectionIndex);
				}
			}
		}
		else
		{
			bResult = Super::IsElementEnabled(*FieldTag, CollectionIndex);
		}
	}

	return bResult || Super::IsElementEnabled(FieldName, CollectionIndex);
}

/**
 * Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	DataTag			the tag of the list element data provider that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<IUIListElementCellProvider> UGearUIDataStore_GameResource::GetElementCellSchemaProvider( FName DataTag )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	FString NextFieldName = DataTag.ToString(), FieldTag;
	if (ParseNextDataTag(NextFieldName, FieldTag) && IsDataTagSupported(*FieldTag) )
	{
		INT InstanceIndex = ParseArrayDelimiter(FieldTag);
		if ( InstanceIndex != INDEX_NONE )
		{
			FName InternalFieldName = FName(*NextFieldName);
			TArray<UUIResourceDataProvider*> ProviderInstances;
			ListElementProviders.MultiFind(*FieldTag, ProviderInstances);

			if ( ProviderInstances.IsValidIndex(InstanceIndex) )
			{
				UUIResourceDataProvider* Provider = ProviderInstances(InstanceIndex);
				IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(Provider);
				if ( ElementProvider != NULL )
				{
					Result = ElementProvider->GetElementCellSchemaProvider(InternalFieldName);
				}
			}
		}
		else
		{
			Result = Super::GetElementCellSchemaProvider(*FieldTag);
		}
	}

	if ( !Result )
	{
		Result = Super::GetElementCellSchemaProvider(DataTag);
	}

	return Result;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 * 
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<IUIListElementCellProvider> UGearUIDataStore_GameResource::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	FString NextFieldName = FieldName.ToString(), FieldTag;
	if (ParseNextDataTag(NextFieldName, FieldTag) && IsDataTagSupported(*FieldTag) )
	{
		INT InstanceIndex = ParseArrayDelimiter(FieldTag);
		if ( InstanceIndex != INDEX_NONE )
		{
			FName InternalFieldName = FName(*NextFieldName);
			TArray<UUIResourceDataProvider*> ProviderInstances;
			ListElementProviders.MultiFind(*FieldTag, ProviderInstances);

			if ( ProviderInstances.IsValidIndex(InstanceIndex) )
			{
				UUIResourceDataProvider* Provider = ProviderInstances(InstanceIndex);
				IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(Provider);
				if ( ElementProvider != NULL )
				{
					Result = ElementProvider->GetElementCellValueProvider(InternalFieldName, ListIndex);
				}
			}
		}
		else
		{
			Result = Super::GetElementCellValueProvider(*FieldTag, ListIndex);
		}
	}

	if ( !Result )
	{
		Result = Super::GetElementCellValueProvider(FieldName, ListIndex);
	}

	return Result;
}

/* ==========================================================================================================
	UGearDataProvider_SceneNavigationData
========================================================================================================== */
IMPLEMENT_CLASS(UGearDataProvider_SceneNavigationData);

#define NAVITEMS_VARNAME	TEXT("NavigationItems")
#define DISPLAYTEXT_VARNAME		TEXT("DisplayName")

/**
 * @return	the name of the NavigationItems member variable.
 */
static FName GetNavItemPropertyName()
{
	static FName NavItemPropertyName(NAVITEMS_VARNAME);
	return NavItemPropertyName;
}
/**
 * @return	the name of the NavigationItemData.DisplayText variable.
 */
static FName GetDisplayTextPropertyName()
{
	static FName DisplayTextPropertyName(DISPLAYTEXT_VARNAME);
	return DisplayTextPropertyName;
}
/**
 * @return	a reference to the NavigationItemData script struct from UGearDataProvider_SceneNavigationData class
 */
static UScriptStruct* GetNavItemDataStruct( UClass* OwnerClass )
{
	checkSlow(OwnerClass);

	static UScriptStruct* NavItemDataStruct = FindField<UScriptStruct>(OwnerClass, TEXT("NavigationItemData"));
	checkSlow(NavItemDataStruct);
	return NavItemDataStruct;
}
/**
 * @return	a reference to the DisplayName variable of the NavigationItemData script struct
 */
static UProperty* GetDisplayNameProperty( UClass* OwnerClass )
{
	static UProperty* DisplayNameProp = FindField<UProperty>(GetNavItemDataStruct(OwnerClass), TEXT("DisplayName"));
	checkSlow(DisplayNameProp);

	return DisplayNameProp;
}

/* === UIDataProvider interface === */
/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UGearDataProvider_SceneNavigationData::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	new(out_Fields) FUIDataProviderField(GetNavItemPropertyName(), DATATYPE_Collection);

	UUIDataProvider::GetSupportedDataFields(out_Fields);
}

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UGearDataProvider_SceneNavigationData::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;
	return bResult || Super::GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
}

/* === IUIListElementProvider interface === */
/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UGearDataProvider_SceneNavigationData::GetElementProviderTags()
{
	TArray<FName> ProviderTags;

	// iterate over all properties which are declared in this class or a child
	const INT ParentClassSize = ThisClass::Super::StaticClass()->GetPropertiesSize();
	for ( UProperty* Prop = GetClass()->PropertyLink; Prop; Prop = Prop->PropertyLinkNext )
	{
		if ( Prop->Offset < ParentClassSize )
		{
			break;
		}

		// any array property marked with the data binding keyword will considered a collection provider
		if ( Prop->HasAnyPropertyFlags(CPF_DataBinding)
		&&	(Prop->ArrayDim > 1 || Prop->IsA(UArrayProperty::StaticClass())) )
		{
			ProviderTags.AddUniqueItem(Prop->GetFName());
		}
	}

	return ProviderTags;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UGearDataProvider_SceneNavigationData::GetElementCount( FName FieldName )
{
	INT Result = 0;

	// FieldName should be the name of a member property of this class which holds collection data
	//@todo ronp - we should still perform the parsing in order to support additional layers of nested collections
	if ( FieldName == GetNavItemPropertyName() )
	{
		Result = NavigationItems.Num();
	}

	//@todo - what about script only subclasses?
	return Result;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UGearDataProvider_SceneNavigationData::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	// FieldName should be the name of a member property of this class which holds collection data
	//@todo ronp - we should still perform the parsing in order to support additional layers of nested collections
	out_Elements.Empty();

	if ( FieldName == GetNavItemPropertyName() )
	{
		for ( INT i = 0; i < NavigationItems.Num(); i++ )
		{
			out_Elements.AddItem(i);
		}

		bResult = TRUE;
	}

	return bResult;
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
UBOOL UGearDataProvider_SceneNavigationData::IsElementEnabled( FName FieldName, INT CollectionIndex )
{
	UBOOL bResult = TRUE;

	// FieldName should be the name of a member property of this class which holds collection data
	//@todo ronp - we should still perform the parsing in order to support additional layers of nested collections
	if ( FieldName == GetNavItemPropertyName() )
	{
		if ( NavigationItems.IsValidIndex(CollectionIndex) )
		{
			bResult = !NavigationItems(CollectionIndex).bItemDisabled;
		}
	}

	return bResult;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	FieldName		the tag of the list element data field that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<IUIListElementCellProvider> UGearDataProvider_SceneNavigationData::GetElementCellSchemaProvider( FName FieldName )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	// FieldName will contain the entire provider path minus the data store name so we need to parse it again
	FString ParsedDataTag = FieldName.ToString(), ProviderTag;
	while ( ParsedDataTag.Len() > 0 )
	{
		ParseNextDataTag(ParsedDataTag, ProviderTag);

		// So ParsedDataTag is the left-most node of the path, and ProviderTag contains the rest
		// pull off any array indices
		ParseArrayDelimiter(ProviderTag);

		// now check whether this the tag we're looking for
		if ( ProviderTag == NAVITEMS_VARNAME )
		{
			// found it - we're the cell provider.
			Result = this;
			break;
		}
	}

	return Result;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 *
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<IUIListElementCellProvider> UGearDataProvider_SceneNavigationData::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	// this is called by our owning data store, after it has parsed the first portion of the tag.
	//@todo ronp - we should still perform the parsing in order to support additional layers of nested collections
	if ( FieldName == GetNavItemPropertyName() )
	{
		Result = this;
	}

	return Result;
}

/* === IUIListElementCellProvider interface === */
/**
 * Retrieves the list of tags that can be bound to individual cells in a single list element.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	out_CellTags	receives the list of tag/column headers that can be bound to element cells for the specified property.
 */
void UGearDataProvider_SceneNavigationData::GetElementCellTags( FName FieldName, TMap<FName,FString>& out_CellTags )
{
	out_CellTags.Empty();

	// Because this method is called directly by widgets and components, FieldName will contain the entire provider path minus the data
	// store name so we need to parse it again
	FString ParsedDataTag = FieldName.ToString(), ProviderTag;
	while ( ParsedDataTag.Len() > 0 )
	{
		ParseNextDataTag(ParsedDataTag, ProviderTag);

		// So ParsedDataTag is the left-most node of the path, and ProviderTag contains the rest
		// pull off any array indices
		ParseArrayDelimiter(ProviderTag);

		// now check whether the requested tag corresponds to the NavigationItems variable
		if ( ProviderTag == NAVITEMS_VARNAME )
		{
			UScriptStruct* NavItemDataStruct = GetNavItemDataStruct(GetClass());
			checkSlow(NavItemDataStruct);

			// if so, add all properties of the struct which are marked with the databinding keyword
			// as bindable cell tags
			for ( UProperty* Prop = NavItemDataStruct->PropertyLink; Prop; Prop = Prop->PropertyLinkNext )
			{
				if ( Prop->HasAnyPropertyFlags(CPF_DataBinding) )
				{
					out_CellTags.Set(Prop->GetFName(), *Prop->GetFriendlyName(GetClass()));
				}
			}
			break;
		}
	}
}

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
UBOOL UGearDataProvider_SceneNavigationData::GetCellFieldType( FName FieldName, const FName& CellTag, BYTE& out_CellFieldType )
{
	UBOOL bResult = FALSE;

	// Because this method is called directly by widgets and components, FieldName will contain the entire provider path minus the data
	// store name so we need to parse it again
	FString ParsedDataTag = FieldName.ToString(), ProviderTag;
	while ( ParsedDataTag.Len() > 0 )
	{
		ParseNextDataTag(ParsedDataTag, ProviderTag);

		// So ParsedDataTag is the left-most node of the path, and ProviderTag contains the rest
		// pull off any array indices
		ParseArrayDelimiter(ProviderTag);

		// now check whether this the tag we're looking for
		if ( ProviderTag == NAVITEMS_VARNAME )
		{
			// search for a property with the specified name - since we return the property names as cell tags
			// in GetElementCellTags, we should find it
			if ( FindFieldWithFlag<UProperty,CASTCLASS_UProperty>(GetNavItemDataStruct(GetClass()), CellTag) != NULL )
			{
				out_CellFieldType = DATATYPE_Property;
				bResult = TRUE;
				break;
			}
		}
	}

	return bResult || Super::GetCellFieldType(FieldName, CellTag, out_CellFieldType);;
}

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
UBOOL UGearDataProvider_SceneNavigationData::GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	FString ParsedDataTag = FieldName.ToString(), ProviderTag;

	UBOOL bProcessed=FALSE;
	while ( ParsedDataTag.Len() > 0 )
	{
		ParseNextDataTag(ParsedDataTag, ProviderTag);

		// So ParsedDataTag is the left-most node of the path, and ProviderTag contains the rest
		// pull off any array indices
		ParseArrayDelimiter(ProviderTag);

		if ( ProviderTag == NAVITEMS_VARNAME && NavigationItems.IsValidIndex(ListIndex) )
		{
			// get a reference to the NavigationItemData struct
			UScriptStruct* NavItemDataStruct = GetNavItemDataStruct(ThisClass::StaticClass());
			checkSlow(NavItemDataStruct);

			// find the GearDataProvider_SceneNavigationData.NavigationItems property
			static UProperty* NavArrayProperty = FindFieldWithFlag<UProperty,CASTCLASS_UProperty>(ThisClass::StaticClass(),*ProviderTag);
			checkSlow(NavArrayProperty);

			// search for a property with the specified name - since we return the property names as cell tags
			// in GetElementCellTags, we should find it
			UProperty* Prop = FindField<UProperty>(NavItemDataStruct, CellTag);
			if ( Prop != NULL )
			{
				// found the correct field - now we need to lookup the address of the element specified by ListIndex
				// and pass that to the CopyPropertyValueIntoFieldValue method to retrieve it's value.
				TArray<FNavigationItemData>* ArrayValue = (TArray<FNavigationItemData>*)((BYTE*)this + NavArrayProperty->Offset);
				BYTE* PropertyValueAddress = (BYTE*)(&((*ArrayValue)(ListIndex)));

				bResult = CopyPropertyValueIntoFieldValue(Prop, PropertyValueAddress, ArrayIndex, out_FieldValue);

				// if this item has been marked as needing to be viewed, append the nav/attract icon markup string.
				if ( CellTag == GetDisplayTextPropertyName() && out_FieldValue.StringValue.Len() > 0 )
				{
					FNavigationItemData& NavItem = *(FNavigationItemData*)(PropertyValueAddress);
					if ( NavItem.bDisplayAttractIcon )
					{
						out_FieldValue.StringValue = out_FieldValue.StringValue + TEXT(" ") + AttractIconMarkup;
					}
				}
			}

			bProcessed = TRUE;
			break;
		}
	}

	if ( !bProcessed )
	{
		bResult = Super::GetCellFieldValue(FieldName, CellTag, ListIndex, out_FieldValue, ArrayIndex);
	}

	return bResult;
}

#undef NAVITEMS_VARNAME

/* ==========================================================================================================
	UGearDataProvider_StringValueCollection
========================================================================================================== */
IMPLEMENT_CLASS(UGearDataProvider_StringValueCollection);

// #define NAVITEMS_VARNAME	TEXT("NavigationItems")
#define STRINGVALUE_VARNAME TEXT("StringValueCollections")

/**
 * @return	the name of the NavigationItems member variable.
 */
static FName GetStringValuePropertyName()
{
	static FName StringValueCollectionPropertyName(STRINGVALUE_VARNAME);
	return StringValueCollectionPropertyName;
}
/**
 * @return	a reference to the NavigationItemData script struct from UGearDataProvider_SceneNavigationData class
 */
static UScriptStruct* GetStringValueCollectionStruct( UClass* OwnerClass )
{
	checkSlow(OwnerClass);

	static UScriptStruct* StringValueCollectionStruct = FindField<UScriptStruct>(OwnerClass, TEXT("StringValueCollection"));
	checkSlow(StringValueCollectionStruct);
	return StringValueCollectionStruct;
}
/**
 * @return	a reference to the DisplayName variable of the NavigationItemData script struct
 */
// static UProperty* GetDisplayNameProperty( UClass* OwnerClass )
// {
// 	static UProperty* DisplayNameProp = FindField<UProperty>(GetNavItemDataStruct(OwnerClass), TEXT("DisplayName"));
// 	checkSlow(DisplayNameProp);
// 
// 	return DisplayNameProp;
// }

/**
 * Find the index of the StringValueCollection that has the tag specified.
 *
 * @param	CollectionTag	the name to search for; should match the CollectionName of an element of the StringValueCollection
 *
 * @return	the index [into the StringValueCollection array] for the collection with the specified tag, or INDEX_NONE if
 *			it isn't found
 */
INT UGearDataProvider_StringValueCollection::FindCollectionIndex( FName CollectionTag ) const
{
	INT Result = INDEX_NONE;

	for ( INT CollectionIndex = 0; CollectionIndex < StringValueCollections.Num(); CollectionIndex++ )
	{
		const FStringValueCollection& Collection = StringValueCollections(CollectionIndex);
		if ( Collection.CollectionName == CollectionTag )
		{
			Result = CollectionIndex;
			break;
		}
	}
	return Result;
}

/**
 * Find the index of a string in a particular string collection.
 *
 * @param	CollectionTag	the name of the collection to search in; should match the CollectionName of an element of
 *							the StringValueCollection
 * @param	SearchString	the string value to search for within the collection's list of strings.
 *
 * @return	the index [into the string value collection's StringValues array] for the string specified, or INDEX_NONE
 *			if either the collection or the string isn't found.
 */
INT UGearDataProvider_StringValueCollection::FindStringValueIndex( FName CollectionTag, const FString& SearchString ) const
{
	INT Result = INDEX_NONE;

	const INT CollectionIndex = FindCollectionIndex(CollectionTag);
	if ( StringValueCollections.IsValidIndex(CollectionIndex) )
	{
		const FStringValueCollection& Collection = StringValueCollections(CollectionIndex);
		Result = Collection.StringValues.FindItemIndex(SearchString);
	}

	return Result;
}

/* === UIDataProvider interface === */
/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UGearDataProvider_StringValueCollection::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	Super::GetSupportedDataFields(out_Fields);
}

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UGearDataProvider_StringValueCollection::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;
	return bResult || Super::GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
}

/* === IUIListElementProvider interface === */
/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UGearDataProvider_StringValueCollection::GetElementProviderTags()
{
	TArray<FName> ProviderTags;

	const FName StringValueCollectionName = GetStringValuePropertyName();

	ProviderTags.AddItem(StringValueCollectionName);
	for ( INT CollectionIndex = 0; CollectionIndex < StringValueCollections.Num(); CollectionIndex++ )
	{
		FStringValueCollection& Collection = StringValueCollections(CollectionIndex);
		if ( Collection.CollectionName != NAME_None )
		{
			//@todo ronp - is this the right thing to do???  or should we add them like StringValueCollections;0.<Collection.Name> ?? ??
			ProviderTags.AddUniqueItem(Collection.CollectionName);
		}
	}

	// iterate over all properties which are declared in this class or a child
	const INT ParentClassSize = ThisClass::Super::StaticClass()->GetPropertiesSize();
	for ( UProperty* Prop = GetClass()->PropertyLink; Prop; Prop = Prop->PropertyLinkNext )
	{
		if ( Prop->Offset < ParentClassSize )
		{
			break;
		}

		// any array property marked with the data binding keyword will considered a collection provider
		if ( Prop->HasAnyPropertyFlags(CPF_DataBinding)
		&&	Prop->GetFName() != StringValueCollectionName
		&&	(Prop->ArrayDim > 1 || Prop->IsA(UArrayProperty::StaticClass())) )
		{
			ProviderTags.AddUniqueItem(Prop->GetFName());
		}
	}

	return ProviderTags;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UGearDataProvider_StringValueCollection::GetElementCount( FName FieldName )
{
	INT Result = 0;

	// FieldName should be the name of a member property of this class which holds collection data
	//@todo ronp - we should still perform the parsing in order to support additional layers of nested collections
	if ( FieldName == GetStringValuePropertyName() )
	{
		Result = StringValueCollections.Num();
	}
	else
	{
		Result = Super::GetElementCount(FieldName);
	}

	//@todo - what about script only subclasses?
	return Result;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UGearDataProvider_StringValueCollection::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	// FieldName should be the name of a member property of this class which holds collection data
	//@todo ronp - we should still perform the parsing in order to support additional layers of nested collections
	out_Elements.Empty();

	debugf(TEXT("UGearDataProvider_StringValueCollection::GetListElements - FieldName: '%s'"), *FieldName.ToString());
	if ( FieldName == GetStringValuePropertyName() )
	{
		for ( INT i = 0; i < StringValueCollections.Num(); i++ )
		{
			out_Elements.AddItem(i);
		}

		bResult = TRUE;
	}
	else
	{
		bResult = Super::GetListElements(FieldName, out_Elements);
	}

	return bResult;
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
UBOOL UGearDataProvider_StringValueCollection::IsElementEnabled( FName FieldName, INT CollectionIndex )
{
	UBOOL bResult = TRUE;

	// FieldName should be the name of a member property of this class which holds collection data
	//@todo ronp - we should still perform the parsing in order to support additional layers of nested collections
	if ( FieldName == GetStringValuePropertyName() )
	{
		//@fixme ronp
// 		if ( StringValueCollections.IsValidIndex(CollectionIndex) )
// 		{
// 			bResult = !StringValueCollections(CollectionIndex).bItemDisabled;
// 		}
	}

	return bResult;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	FieldName		the tag of the list element data field that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<IUIListElementCellProvider> UGearDataProvider_StringValueCollection::GetElementCellSchemaProvider( FName FieldName )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	// FieldName will contain the entire provider path minus the data store name so we need to parse it again
	FString ParsedDataTag = FieldName.ToString(), ProviderTag;
	while ( ParsedDataTag.Len() > 0 )
	{
		ParseNextDataTag(ParsedDataTag, ProviderTag);

		// So ParsedDataTag is the left-most node of the path, and ProviderTag contains the rest
		// pull off any array indices
		ParseArrayDelimiter(ProviderTag);

		// now check whether this the tag we're looking for
		if ( ProviderTag == STRINGVALUE_VARNAME )
		{
			// found it - we're the cell provider.
			Result = this;
			break;
		}
	}

	return Result;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 *
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<IUIListElementCellProvider> UGearDataProvider_StringValueCollection::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	debugf(TEXT("UGearDataProvider_StringValueCollection::GetElementCellValueProvider - FieldName: '%s'  ListIndex:%i"), *FieldName.ToString(), ListIndex);
	// this is called by our owning data store, after it has parsed the first portion of the tag.
	//@todo ronp - we should still perform the parsing in order to support additional layers of nested collections
	if ( FieldName == GetStringValuePropertyName() )
	{
		Result = this;
	}

	return Result;
}

/**
 * Resolves PropertyName into a list element provider that provides list elements for the property specified.
 *
 * @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
 *
 * @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
 *			there is no list element provider associated with the specified property.
 */
TScriptInterface<IUIListElementProvider> UGearDataProvider_StringValueCollection::ResolveListElementProvider( const FString& PropertyName )
{
	TScriptInterface<IUIListElementProvider> Result;

	debugf(TEXT("UGearDataProvider_StringValueCollection::ResolveListElementProvider - PropertyName: '%s'"), *PropertyName);

	return Result;
}

/* === IUIListElementCellProvider interface === */
/**
 * Retrieves the list of tags that can be bound to individual cells in a single list element.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	out_CellTags	receives the list of tag/column headers that can be bound to element cells for the specified property.
 */
void UGearDataProvider_StringValueCollection::GetElementCellTags( FName FieldName, TMap<FName,FString>& out_CellTags )
{
	out_CellTags.Empty();

	// Because this method is called directly by widgets and components, FieldName will contain the entire provider path minus the data
	// store name so we need to parse it again
	FString ParsedDataTag = FieldName.ToString(), ProviderTag;
	while ( ParsedDataTag.Len() > 0 )
	{
		ParseNextDataTag(ParsedDataTag, ProviderTag);

		// So ParsedDataTag is the left-most node of the path, and ProviderTag contains the rest
		// pull off any array indices
		ParseArrayDelimiter(ProviderTag);

		// now check whether the requested tag corresponds to the NavigationItems variable
		if ( ProviderTag == STRINGVALUE_VARNAME )
		{
			for ( INT CollectionIndex = 0; CollectionIndex < StringValueCollections.Num(); CollectionIndex++ )
			{
				FStringValueCollection& Collection = StringValueCollections(CollectionIndex);
				if ( Collection.CollectionName != NAME_None )
				{
					out_CellTags.Set(Collection.CollectionName, Collection.CollectionFriendlyName);
				}
			}

			break;
		}
	}
}

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
UBOOL UGearDataProvider_StringValueCollection::GetCellFieldType( FName FieldName, const FName& CellTag, BYTE& out_CellFieldType )
{
	UBOOL bResult = FALSE;

	// Because this method is called directly by widgets and components, FieldName will contain the entire provider path minus the data
	// store name so we need to parse it again
	FString ParsedDataTag = FieldName.ToString(), ProviderTag;
	while ( ParsedDataTag.Len() > 0 )
	{
		ParseNextDataTag(ParsedDataTag, ProviderTag);

		// So ParsedDataTag is the left-most node of the path, and ProviderTag contains the rest
		// pull off any array indices
		ParseArrayDelimiter(ProviderTag);

		// now check whether this the tag we're looking for
		if ( ProviderTag == STRINGVALUE_VARNAME )
		{
			// may need to parse ParsedDataTag further
			INT CollectionIndex = FindCollectionIndex(*ParsedDataTag);
			if ( StringValueCollections.IsValidIndex(CollectionIndex) )
			{
				FStringValueCollection& Collection = StringValueCollections(CollectionIndex);
				out_CellFieldType = DATATYPE_Collection;
				bResult = TRUE;
			}
		}
	}

	return bResult || Super::GetCellFieldType(FieldName, CellTag, out_CellFieldType);;
}

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
UBOOL UGearDataProvider_StringValueCollection::GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	FString ParsedDataTag = FieldName.ToString(), ProviderTag;

	UBOOL bProcessed=FALSE;
	while ( ParsedDataTag.Len() > 0 )
	{
		ParseNextDataTag(ParsedDataTag, ProviderTag);

		// So ParsedDataTag is the left-most node of the path, and ProviderTag contains the rest
		// pull off any array indices
		ParseArrayDelimiter(ProviderTag);

		if ( ProviderTag == STRINGVALUE_VARNAME && StringValueCollections.IsValidIndex(ListIndex) )
		{
			FStringValueCollection& StringCollection = StringValueCollections(ListIndex);
			INT StringIndex = StringCollection.StringValues.FindItemIndex(CellTag.ToString());

			UScriptStruct* StringValueCollectionStruct = GetStringValueCollectionStruct(ThisClass::StaticClass());
			checkSlow(StringValueCollectionStruct);

			// search for a property with the specified name - since we return the property names as cell tags
			// in GetElementCellTags, we should find it
			UProperty* Prop = FindField<UProperty>(StringValueCollectionStruct, CellTag);
			if ( Prop != NULL )
			{
				// found the correct field - now we need to lookup the address of the element specified by ListIndex
				// and pass that to the CopyPropertyValueIntoFieldValue method to retrieve it's value.
				bResult = CopyPropertyValueIntoFieldValue(Prop, ((BYTE*)&StringValueCollections(ListIndex)), ArrayIndex, out_FieldValue);
				break;
			}

			bProcessed = TRUE;
			break;
		}
	}

	if ( !bProcessed )
	{
		bResult = Super::GetCellFieldValue(FieldName, CellTag, ListIndex, out_FieldValue, ArrayIndex);
	}

	return bResult;
}


#undef STRINGVALUE_VARNAME

/************************************************************************/
/* UGearUIDataStore_StringAliasMap                                       */
/************************************************************************/

IMPLEMENT_CLASS(UGearUIDataStore_StringAliasMap);

/**
 * Set MappedString to be the localized string using the FieldName as a key
 * Returns the index into the mapped string array of where it was found.
 */
INT UGearUIDataStore_StringAliasMap::GetStringWithFieldName( const FString& FieldName, FString& MappedString )
{
	INT FieldIdx = INDEX_NONE;
	UBOOL bIsUsingGamepad = FALSE;
	FString FinalFieldName = FieldName;

	// Get the player controller.
	ULocalPlayer* LP = GetPlayerOwner();
	AGearPC* MyGearPC = NULL;
	if ( LP )
	{
		MyGearPC = Cast<AGearPC>(LP->Actor);
	}

#if XBOX
	bIsUsingGamepad = TRUE;
#else
	if ( MyGearPC )
	{
		UGearPlayerInput* GearInput = Cast<UGearPlayerInput>(MyGearPC->MainPlayerInput);
		if ( GearInput )
		{
			bIsUsingGamepad = GearInput->bUsingGamepad;
		}
	}
#endif

	// Try to find platform specific versions first
	FString SetName = bIsUsingGamepad ? TEXT("360") : TEXT("PC");

	FieldIdx = FindMappingWithFieldName(FinalFieldName, SetName);

	if(FieldIdx == INDEX_NONE)
	{
		FieldIdx = FindMappingWithFieldName(FinalFieldName);
	}

	if(FieldIdx == INDEX_NONE)
	{
		FieldIdx = FindMappingWithFieldName();
	}

	if(FieldIdx != INDEX_NONE)
	{
		MappedString = MenuInputMapArray(FieldIdx).MappedText;
	}

	return FieldIdx;
}

/************************************************************************/
/* UGearUIDataStore_StringAliasBindingsMap                              */
/************************************************************************/

IMPLEMENT_CLASS(UGearUIDataStore_StringAliasBindingsMap);

//Clear the command to input keybinding cache
void UGearUIDataStore_StringAliasBindingsMap::ClearBoundKeyCache()
{
	CommandToBindNames.Empty();
}

/**
 * Given an input command of the form GBA_ return the mapped keybinding string 
 * Returns TRUE if it exists, FALSE otherwise
 */
UBOOL UGearUIDataStore_StringAliasBindingsMap::FindMappingInBoundKeyCache(const FString& Command, FString& MappingStr, INT& FieldIndex)
{
	UBOOL bIsUsingGamepad = FALSE;

#if XBOX
	bIsUsingGamepad = TRUE;
#else
	// See if we need to clear the cache because of input change
	ULocalPlayer* LP = GetPlayerOwner();
	AGearPC* MyGearPC = NULL;
	if ( LP )
	{
		MyGearPC = Cast<AGearPC>(LP->Actor);
		if ( MyGearPC && MyGearPC->PlayerInput )
		{
			bIsUsingGamepad = MyGearPC->PlayerInput->bUsingGamepad;
		}
	}
#endif

	if ( bPreviousUsingGamepadValue != bIsUsingGamepad )
	{
		ClearBoundKeyCache();
		bPreviousUsingGamepadValue = (BITFIELD)bIsUsingGamepad;
		return FALSE;
	}

	bPreviousUsingGamepadValue = (BITFIELD)bIsUsingGamepad;

	const FName Key(*Command);
	// Does the data already exist
	const FBindCacheElement* CacheElement = CommandToBindNames.Find(Key);
	if (CacheElement != NULL)
	{
		MappingStr = CacheElement->MappingString;
		FieldIndex = CacheElement->FieldIndex;
	}

	return (CacheElement != NULL);
}

/** Given a input command of the form GBA_ and its mapping store that in a lookup for future use */
void UGearUIDataStore_StringAliasBindingsMap::AddMappingToBoundKeyCache(const FString& Command, const FString& MappingStr, const INT FieldIndex)
{
	const FName Key(*Command);

	// Does the data already exist
	const FBindCacheElement* CacheElement = CommandToBindNames.Find(Key);

	if (CacheElement == NULL)
	{
		// Initialize a new FBindCacheElement.  It contains a FStringNoInit, so it needs to be initialized to zero.
		FBindCacheElement NewElement;
		appMemzero(&NewElement,sizeof(NewElement));

		NewElement.KeyName = Key;
		NewElement.MappingString = MappingStr;
		NewElement.FieldIndex = FieldIndex;
		CommandToBindNames.Set(Key, NewElement);
	}
}

/**
 * Set MappedString to be the localized string using the FieldName as a key
 * Returns the index into the mapped string array of where it was found.
 */
INT UGearUIDataStore_StringAliasBindingsMap::GetStringWithFieldName( const FString& FieldName, FString& MappedString )
{
	INT StartIndex = UCONST_SABM_FIND_FIRST_BIND;
	INT FieldIndex = INDEX_NONE;

	if ( !FindMappingInBoundKeyCache(FieldName, MappedString, FieldIndex) )
	{
		FieldIndex = GetBoundStringWithFieldName( FieldName, MappedString, &StartIndex );
		AddMappingToBoundKeyCache(FieldName, MappedString, FieldIndex);
	}

	return FieldIndex;
}

/**
 * Called by GetStringWithFieldName() to retrieve the string using the input binding system.
 */
INT UGearUIDataStore_StringAliasBindingsMap::GetBoundStringWithFieldName( const FString& FieldName, FString& MappedString, INT* StartIndex/*=NULL*/, FString* BindString/*=NULL*/ )
{
	// String to set MappedString to
	FString LocalizedString = TEXT(" ");

	// Get the index in the MenuInputMapArray using FieldName as the key.
	INT FieldIdx = INDEX_NONE;
	FName KeyName = FName(*FieldName);
	for ( INT Idx = 0; Idx < MenuInputMapArray.Num(); Idx++ )
	{
		if ( KeyName == MenuInputMapArray(Idx).FieldName )
		{
			// Found it
			FieldIdx = Idx;
			break;
		}
	}

	// If we found the entry in our array find the binding and map it to a localized string.
	if ( FieldIdx != INDEX_NONE )
	{
		// Get the player controller.
		ULocalPlayer* LP = GetPlayerOwner();
		AGearPC* MyGearPC = NULL;
		if ( LP )
		{
			MyGearPC = Cast<AGearPC>(LP->Actor);
		}

		FString NameSearch = TEXT("");
		INT BindIndex = -1;

		if ( MyGearPC )
		{
			// Get the bind using the mapped FieldName as the key
			FString KeyCommand = MenuInputMapArray(FieldIdx).FieldName.ToString();
			if ( KeyCommand.Len() > 0 )
			{
				UGearPlayerInput* GearInput = Cast<UGearPlayerInput>(MyGearPC->MainPlayerInput);
				if ( GearInput )
				{
#if XBOX
					UBOOL bIsUsingGamepad = TRUE;
#else
					UBOOL bIsUsingGamepad = GearInput->bUsingGamepad;
#endif
					if ( StartIndex && *StartIndex == UCONST_SABM_FIND_FIRST_BIND )
					{
						// Get the game logic specific bind based from the command.
						KeyCommand = GearInput->GetGearBindNameFromCommand( *KeyCommand );
					}
					else
					{
						// Get the bind starting from the back at index StartIndex.
						KeyCommand = GearInput->GetBindNameFromCommand( *KeyCommand, StartIndex );

						// Don't allow controller binds to be shown on PC.
						if ( bIsUsingGamepad )
						{
							while( KeyCommand.StartsWith(TEXT("XBoxTypeS")) && (StartIndex && *StartIndex > -1) )
							{
								(*StartIndex)--;
								KeyCommand = GearInput->GetBindNameFromCommand( *KeyCommand, StartIndex );
							}
						}
					}

					// Set the bind string to the string we found.
					if ( BindString )
					{
						*BindString = KeyCommand;
					}

					// If this is a controller string we have to check the ControllerMapArray for the localized text.
					if ( KeyCommand.StartsWith(TEXT("XBoxTypeS")) )
					{
						// Prefix the mapping with the localized string variable prefix.
						FString SubString = FString::Printf(TEXT("GMS_%s"),*KeyCommand);

						// If this is a controller, map it to the localized button strings.
						if ( bIsUsingGamepad )
						{
							FName CommandName = FName(*SubString);
							for ( INT Idx = 0; Idx < ControllerMapArray.Num(); Idx++ )
							{
								if ( CommandName == ControllerMapArray(Idx).KeyName )
								{
									// Found it
									SubString = ControllerMapArray(Idx).XBoxMapping;

									// Try and localize it using the ButtonFont section.
									LocalizedString = Localize( TEXT("ControllerFont"), *SubString, TEXT("GearGameUI") );
									break;
								}
							}
						}
						else
						{
							// Try and localize it using the GameMappedStrings section.
							LocalizedString = Localize( TEXT("GameMappedStrings"), *SubString, TEXT("GearGameUI") );
						}
					}
					else
					{
						// Could not find a mapping... if this happens the game is trying to draw the string for a bind that
						// it didn't ensure would exist.
						if ( KeyCommand.Len() <= 0 )
						{
							LocalizedString = TEXT("");
						}
						// Found a bind.
						else
						{
							// Prefix the mapping with the localized string variable prefix.
							FString SubString = FString::Printf(TEXT("GMS_%s"),*KeyCommand);
							// Try and localize it using the GameMappedStrings section.
							LocalizedString = Localize( TEXT("GameMappedStrings"), *SubString, TEXT("GearGameUI") );
						}
					}
				}
			}
		}
	}

	// Set the localized string and return the index.
	MappedString = LocalizedString;
	return FieldIdx;
}

IMPLEMENT_CLASS(UGearLeaderboardsDataStoreBase);

/**
 * Loads and creates an instance of the registered filter object
 */
void UGearLeaderboardsDataStoreBase::InitializeDataStore(void)
{
	Super::InitializeDataStore();
	// Create settings object
	LeaderboardSettings = ConstructObject<USettings>(LeaderboardSettingsClass);
	if (LeaderboardSettings != NULL)
	{
		SettingsProvider = ConstructObject<UUIDataProvider_Settings>(UUIDataProvider_Settings::StaticClass());
		if (SettingsProvider->BindSettings(LeaderboardSettings) == FALSE)
		{
			debugf(NAME_Error,TEXT("Failed to bind leaderboard filter settings object to %s"),
				*LeaderboardSettings->GetName());
		}
	}
	else
	{
		debugf(NAME_Error,TEXT("Failed to create leaderboard filter settings object %s"),
			*LeaderboardSettingsClass->GetName());
	}
}

IMPLEMENT_CLASS(UGearUIDataProvider_Screenshots);

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
 *							do not provide unique UIListElement objects for each element.
 * @param	OutFieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
 *							to a data collection.
 */
UBOOL UGearUIDataProvider_Screenshots::GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	OutFieldValue.PropertyTag = CellTag;
	OutFieldValue.PropertyType = DATATYPE_Property;

	if (Screenshots.IsValidIndex(ListIndex))
	{
		if (CellTag == FName(TEXT("Rating")))
		{
			OutFieldValue.StringValue = appItoa(Screenshots(ListIndex).Rating);
			bResult = TRUE;
		}
		else if (CellTag == FName(TEXT("GameType")))
		{
			OutFieldValue.StringValue = Screenshots(ListIndex).GameType;
			bResult = TRUE;
		}
		else if (CellTag == FName(TEXT("MapName")))
		{
			OutFieldValue.StringValue = Screenshots(ListIndex).MapName;
			bResult = TRUE;
		}
		else if (CellTag == FName(TEXT("DateTime")))
		{
			OutFieldValue.StringValue = Screenshots(ListIndex).Date;
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	OutFieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UGearUIDataProvider_Screenshots::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(UCONST_UnknownCellDataFieldName,FName(*FieldName),ArrayIndex,OutFieldValue,INDEX_NONE)
		|| Super::GetFieldValue(FieldName, OutFieldValue, ArrayIndex);
}

/**
 * Compares two screenshots for sorting purposes.
 */
static inline INT CompareScreenshots(const FSavedScreenshotInfo& Screenshot1, const FSavedScreenshotInfo& Screenshot2)
{
	// sort by date
	if(Screenshot1.Year != Screenshot2.Year)
	{
		return Screenshot2.Year - Screenshot1.Year;
	}
	if(Screenshot1.Month != Screenshot2.Month)
	{
		return Screenshot2.Month - Screenshot1.Month;
	}
	if(Screenshot1.Day != Screenshot2.Day)
	{
		return Screenshot2.Day - Screenshot1.Day;
	}
	// sort by map
	INT MapDiff = appStricmp(*Screenshot1.MapName, *Screenshot2.MapName);
	if(MapDiff != 0)
	{
		return MapDiff;
	}
	// sort by gametype
	INT GameTypeDiff = appStricmp(*Screenshot1.GameType, *Screenshot2.GameType);
	if(GameTypeDiff != 0)
	{
		return GameTypeDiff;
	}
	// sort by rating
	if(Screenshot1.Rating != Screenshot2.Rating)
	{
		return Screenshot2.Rating - Screenshot1.Rating;
	}
	// sort by ID (to make this deterministic)
	if(Screenshot1.Id.A != Screenshot2.Id.A)
	{
		return (Screenshot2.Id.A - Screenshot1.Id.A);
	}
	if(Screenshot1.Id.B != Screenshot2.Id.B)
	{
		return (Screenshot2.Id.B - Screenshot1.Id.B);
	}
	if(Screenshot1.Id.C != Screenshot2.Id.C)
	{
		return (Screenshot2.Id.C - Screenshot1.Id.C);
	}
	return (Screenshot2.Id.D - Screenshot1.Id.D);
}

/** Used for screenshot sorting. */
IMPLEMENT_COMPARE_CONSTREF(FSavedScreenshotInfo,GearDatastores,{ return CompareScreenshots(A,B); })

/**
 * Sort the screenshots.
 */
void UGearUIDataProvider_Screenshots::SortScreenshots()
{
	Sort<USE_COMPARE_CONSTREF(FSavedScreenshotInfo,GearDatastores)>(&Screenshots(0), Screenshots.Num());
}

IMPLEMENT_CLASS(UGearUIDataStore_OnlinePlayerData);

/**
 * Loads the game specific OnlineProfileSettings class
 */
void UGearUIDataStore_OnlinePlayerData::LoadDependentClasses(void)
{
	Super::LoadDependentClasses();
	
	if (ScreenshotsProviderClassName.Len() > 0)
	{
		// Try to load the specified class
		ScreenshotsProviderClass = LoadClass<UGearUIDataProvider_Screenshots>(NULL,*ScreenshotsProviderClassName,NULL,LOAD_None,NULL);
	}

	if (ScreenshotsProviderClass == NULL)
	{
		ScreenshotsProviderClass = UGearUIDataProvider_Screenshots::StaticClass();

		// and make sure it's loaded!
		LoadClass<UGearUIDataProvider_Screenshots>(NULL, TEXT("GearGame.GearUIDataProvider_Screenshots"), NULL, LOAD_NoWarn|LOAD_Quiet, NULL);
	}
}

/**
 * Creates the data providers exposed by this data store
 */
void UGearUIDataStore_OnlinePlayerData::InitializeDataStore(void)
{
	Super::InitializeDataStore();
	if (ScreenshotsProvider == NULL)
	{
		ScreenshotsProvider = ConstructObject<UGearUIDataProvider_Screenshots>(ScreenshotsProviderClass);
	}
	check(ScreenshotsProvider);
}

/**
 * Forwards the calls to the data providers so they can do their start up
 *
 * @param Player the player that will be associated with this DataStore
 */
void UGearUIDataStore_OnlinePlayerData::OnRegister(ULocalPlayer* Player)
{
	// make sure to call the screenshot provider's OnRegister event BEFORE calling Super, so that we are able to
	// subscribe to the onlinesubsystem's playerinterface OnReadProfileComplete delegate before the UIDataProvider_OnlineProfileSettings
	// class calls ReadProfileSettings..
	if (ScreenshotsProvider)
	{
		ScreenshotsProvider->eventOnRegister(Player);
	}
	Super::OnRegister(Player);
}

/**
 * Tells all of the child providers to clear their player data
 *
 * @param Player ignored
 */
void UGearUIDataStore_OnlinePlayerData::OnUnregister(ULocalPlayer* Player)
{
	if (ScreenshotsProvider)
	{
		ScreenshotsProvider->eventOnUnregister();
	}
	Super::OnUnregister(Player);
}

/**
 * Gets the list of data fields exposed by this data provider
 *
 * @param OutFields Filled in with the list of fields supported by its aggregated providers
 */
void UGearUIDataStore_OnlinePlayerData::GetSupportedDataFields(TArray<FUIDataProviderField>& OutFields)
{
	Super::GetSupportedDataFields(OutFields);

	if (GIsEditor && !GIsGame)
	{
		// Use the default objects in the editor
		UGearUIDataProvider_Screenshots* DefaultScreenshotsProvider = UGearUIDataProvider_Screenshots::StaticClass()->GetDefaultObject<UGearUIDataProvider_Screenshots>();
		check(DefaultScreenshotsProvider);
		DefaultScreenshotsProvider->GetSupportedDataFields(OutFields);
	}
	else
	{
		check(ScreenshotsProvider);
		// Ask the providers for their fields
		ScreenshotsProvider->GetSupportedDataFields(OutFields);
	}
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	OutElements		will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UGearUIDataStore_OnlinePlayerData::GetListElements(FName FieldName,TArray<INT>& OutElements)
{
	UBOOL bResult = Super::GetListElements(FieldName, OutElements);

	//@todo
	if (!bResult && ScreenshotsProvider && FieldName == FName(TEXT("Screenshots")))
	{
		// For each screenshot add the provider as an entry
		for (INT Index = 0; Index < ScreenshotsProvider->Screenshots.Num(); Index++)
		{
			OutElements.AddItem(Index);
		}
	}
	return bResult ||
		FieldName == FName(TEXT("Screenshots"));
}


/**
 * Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	FieldName		the tag of the list element data provider that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<IUIListElementCellProvider> UGearUIDataStore_OnlinePlayerData::GetElementCellSchemaProvider(FName FieldName)
{
	if (FieldName == FName(TEXT("Screenshots")))
	{
		return ScreenshotsProvider;
	}
	return Super::GetElementCellSchemaProvider(FieldName);
}