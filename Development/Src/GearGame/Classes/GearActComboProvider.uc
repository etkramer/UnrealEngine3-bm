/**
 * This class is responsible for providing centralized access to both static and dynamic data for a single Gears2 campaign act.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearActComboProvider extends GearCampaignComboProvider;

/**
 * The list of providers for the chapters contained in this act
 * Generated at startup when the GearUIDataStore_GameResource data store is registered.
 */
var transient array<GearChapterComboProvider> ChapterProviders;

function EGearAct GetActId()
{
	local EGearAct Result;
	local GearCampaignActData StaticActProvider;

	Result = GEARACT_MAX;
	StaticActProvider = GearCampaignActData(StaticDataProvider);
	if ( StaticActProvider != None )
	{
		Result = StaticActProvider.ActType;
	}

	return Result;
}

/**
 * Provides the data provider with the chance to perform initialization, including preloading any content that will be needed by the provider.
 *
 * @param	bIsEditor					TRUE if the editor is running; FALSE if running in the game.
 * @param	InStaticResourceProvider	the data provider that provides the static resource data for this combo provider.
 * @param	InProfileProvider			the data provider that provides profile data for the player associated with the owning data store.
 */
event InitializeProvider( bool bIsEditor, UIResourceDataProvider InStaticResourceProvider, UIDataProvider_OnlineProfileSettings InProfileProvider )
{
	Super.InitializeProvider(bIsEditor, InStaticResourceProvider, InProfileProvider);

	InitializeChapters();
	InitializeCollectables();
}

/**
 * Populates the list of chapter providers for the chapters contained in the act associated with this data provider.
 */
function InitializeChapters()
{
	local UIDataStore_DynamicResource DynamicResourceDS;
	local array<UIResourceCombinationProvider> AllChapterProviders;
	local GearChapterComboProvider Provider;
	local int ChapterIdx;
	local EGearAct ActType, ChapterActType;

	ChapterProviders.Length = 0;
	DynamicResourceDS = class'GearUIScene_Base'.static.GetDynamicResourceDataStore(ProfileProvider.Player);
	DynamicResourceDS.GetResourceProviders('Chapters', AllChapterProviders);
	ActType = GetActId();
	if ( ActType != GEARACT_MAX )
	{
		for ( ChapterIdx = 0; ChapterIdx < AllChapterProviders.Length; ChapterIdx++ )
		{
			Provider = GearChapterComboProvider(AllChapterProviders[ChapterIdx]);
			ChapterActType = Provider.GetActId();

			if ( ChapterActType != GEARACT_MAX && ChapterActType == ActType )
			{
				ChapterProviders.AddItem(Provider);
			}
		}
	}
}

/**
 * Initializes the list of providers for the collectables contained in the act associated with this provider.
 */
function InitializeCollectables()
{
	local UIDataStore_DynamicResource DynamicResourceDS;
	local array<UIResourceCombinationProvider> CollectableProviders;
	local GearCollectableComboProvider CollectableProvider;
	local int CollectableIdx;
	local EGearAct ActId, CollectableActId;

	DynamicCollectableProviders.Length = 0;
	DynamicResourceDS = class'GearUIScene_Base'.static.GetDynamicResourceDataStore(ProfileProvider.Player);
	DynamicResourceDS.GetResourceProviders('Collectables', CollectableProviders);
	ActId = GetActId();

	if ( ActId != GEARACT_MAX )
	{
		for ( CollectableIdx = 0; CollectableIdx < CollectableProviders.Length; CollectableIdx++ )
		{
			CollectableProvider = GearCollectableComboProvider(CollectableProviders[CollectableIdx]);
			CollectableActId = CollectableProvider.GetActId();

			if ( CollectableActId != GEARACT_MAX && ActId == CollectableActId )
			{
				DynamicCollectableProviders.AddItem(CollectableProvider);
				CollectableProviders.Remove(CollectableIdx--, 1);
			}
		}
	}
}

/**
 * Determine the highest difficulty this act has been completed on.
 *
 * @return	the enum value for the highest difficulty the player has completed all chapters in this act on, or DL_MAX if the player
 *			hasn't completed all chapters.
 */
function EDifficultyLevel GetHighestCompletedDifficulty()
{
	local int DiffIndex;
	local EDifficultyLevel Result;
	local EGearAct ActId;
	local GearProfileSettings Profile;

	Result = DL_MAX;
	Profile = GetGearProfile();
	if ( Profile != None )
	{
		ActId = GetActId();
		if ( ActId != GEARACT_MAX )
		{
			for ( DiffIndex = DL_MAX - 1; DiffIndex >= 0; DiffIndex-- )
			{
				if ( Profile.HasActBeenCompleted(ActId, EDifficultyLevel(DiffIndex)) )
				{
					Result = EDifficultyLevel(DiffIndex);
					break;
				}
			}
		}
		else
		{
			`log(`location @ "invalid ActId for GearActComboProvider:" @ ActId);
		}
	}
	else
	{
		`log(`location @ "no valid profile bound to this GearActComboProvider!");
	}

	return Result;
}

/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
event array<name> GetElementProviderTags()
{
	local array<name> FieldNames;

	FieldNames = Super.GetElementProviderTags();
	FieldNames.AddItem('ChapterProviders');

	return FieldNames;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
event int GetElementCount( name FieldName )
{
	local int Result;

	if ( FieldName == nameof(ChapterProviders) )
	{
		Result = ChapterProviders.Length;
	}
	else
	{
		Result = Super.GetElementCount(FieldName);
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
event bool GetListElements(name FieldName, out array<int> out_Elements)
{
	// not needed - we don't have any additional provider fields that correspond to collection data
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
//@todo
event bool IsElementEnabled( name FieldName, int CollectionIndex )
{
//	local bool bResult;
//
//	bResult = false;
//
//	`log(`location @ `showvar(FieldName));
//	return bResult;
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
event bool GetElementCellSchemaProvider( name FieldName, out UIListElementCellProvider out_SchemaProvider )
{
	local bool bResult;
	local int pos;

	bResult = Super.GetElementCellSchemaProvider(FieldName, out_SchemaProvider);

	pos = InStr(FieldName, ".", true);
	if ( pos != INDEX_NONE )
	{
		FieldName = name(Mid(FieldName, pos + 1));
	}

	if ( !bResult && FieldName == nameof(ChapterProviders) )
	{
		if ( ChapterProviders.Length > 0 )
		{
			out_SchemaProvider = ChapterProviders[0];
		}
		else
		{
			out_SchemaProvider = Self;
		}
		bResult = true;
	}

//	`log(`location @ `showvar(FieldName) @ `showvar(out_SchemaProvider) @ `showvar(bResult));
	return bResult;
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
event bool GetElementCellValueProvider( name FieldName, int ListIndex, out UIListElementCellProvider out_ValueProvider )
{
	local bool bResult;

	bResult = Super.GetElementCellValueProvider(FieldName, ListIndex, out_ValueProvider);
	if ( !bResult && FieldName == nameof(ChapterProviders) && ListIndex >= 0 && ListIndex < ChapterProviders.Length )
	{
		out_ValueProvider = ChapterProviders[ListIndex];
		bResult = true;
	}

//	`log(`location @ `showvar(FieldName) @ `showvar(ListIndex) @ `showvar(out_ValueProvider) @ `showvar(bResult));
	return bResult;
}

/* === IUIListElementCellProvider interface === */
/**
 * Retrieves the list of tags that can be bound to individual cells in a single list element.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	out_CellTags	receives the list of tag/column headers that can be bound to element cells for the specified property.
 */
event GetElementCellTags( name FieldName, out array<name> CellFieldTags, optional out array<string> ColumnHeaderDisplayText )
{
	Super.GetElementCellTags(FieldName, CellFieldTags, ColumnHeaderDisplayText);
	if ( ChapterProviders.Length > 0 && IsEditor() )
	{
		ChapterProviders[0].GetElementCellTags(FieldName, CellFieldTags, ColumnHeaderDisplayText);
	}
	else
	{
		CellFieldTags[CellFieldTags.Length] = 'DifficultyCompletedIcon';
		ColumnHeaderDisplayText[ColumnHeaderDisplayText.Length] = "DifficultyCompletedIcon";
	}
//	`log(`location @ `showvar(FieldName));
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
event bool GetCellFieldType( name FieldName, name CellTag, out EUIDataProviderFieldType FieldType )
{
	local bool bResult;

	bResult = Super.GetCellFieldType(FieldName, CellTag, FieldType);
	if ( !bResult )
	{
		if ( ChapterProviders.Length > 0 && IsEditor() )
		{
			ChapterProviders[0].GetCellFieldType(FieldName, CellTag, FieldType);
		}
		else if ( CellTag == 'DifficultyCompletedIcon' )
		{
			FieldType = DATATYPE_Property;
			bResult = true;
		}
	}
	return bResult;
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
event bool GetCellFieldValue( name FieldName, name CellTag, int ListIndex, out UIProviderFieldValue out_FieldValue, optional int ArrayIndex=INDEX_NONE )
{
	local bool bResult;
	local UIProviderScriptFieldValue FieldValue;

	bResult = Super.GetCellFieldValue(FieldName, CellTag, ListIndex, out_FieldValue, ArrayIndex);
	if ( !bResult && (FieldName == 'DifficultyCompletedIcon' || CellTag == 'DifficultyCompletedIcon') )
	{
		if ( IsEditor() )
		{
			bResult = true;
			out_FieldValue.StringValue = DifficultyImagePaths[ListIndex % DifficultyImagePaths.Length];
		}
		else
		{
			bResult = GetFieldValue("DifficultyCompletedIcon", FieldValue, ArrayIndex);
			if ( bResult )
			{
				out_FieldValue = FieldValue;
			}
		}
	}
	if ( !bResult && FieldName == nameof(ChapterProviders) && ListIndex >= 0 && ListIndex < ChapterProviders.Length )
	{
		if ( ChapterProviders[ListIndex].GetFieldValue(string(CellTag), FieldValue, ArrayIndex) )
		{
			out_FieldValue = FieldValue;
			bResult = true;
		}
	}

//	`log(`location @ `showvar(FieldName) @ `showvar(CellTag) @ `showvar(ListIndex) @ `showvar(bResult) @ `showvar(out_Fieldvalue.StringValue));
	return bResult;
}

/* === UIDataProvider interface === */
/**
 * Callback to allow script-only child classes to add their own supported tags when GetSupportedDataFields is called.
 *
 * @param	out_Fields	the list of data tags supported by this data store.
 */
event GetSupportedScriptFields( out array<UIDataProviderField> out_Fields )
{
	local UIDataProviderField Field;

	Super.GetSupportedScriptFields(out_Fields);

	// the Collectables field comes from our static data provider - replace the provider collection with the dynamic collection providers
	// contained in this act
	ReplaceProviderCollection(out_Fields, 'ChapterProviders', ChapterProviders);

	Field.FieldTag = 'DifficultyCompletedIcon';
	Field.FieldType = DATATYPE_Property;
	out_Fields[out_Fields.Length] = Field;
}

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see ParseDataStoreReference for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 *
 * @return	TRUE to indicate that this value was processed by script.
 */
event bool GetFieldValue( string FieldName, out UIProviderScriptFieldValue FieldValue, optional int ArrayIndex=INDEX_NONE )
{
	local bool bResult;
	local EDifficultyLevel HighestCompletedDifficulty;

	if ( FieldName == "DifficultyCompletedIcon" )
	{
		FieldValue.PropertyTag = 'DifficultyCompletedIcon';
		FieldValue.PropertyType = DATATYPE_Property;

		HighestCompletedDifficulty = GetHighestCompletedDifficulty();
		if ( HighestCompletedDifficulty != DL_MAX )
		{
			FieldValue.StringValue = DifficultyImagePaths[HighestCompletedDifficulty];
		}
		else
		{
			`log(`location @ "has not completed act" @ GetActId() @ "on any difficulty.",,'RON_DEBUG');
		}
		bResult = true;
	}
//`log(`location @ `showvar(FieldName) @ `showvar(ArrayIndex) @ `showvar(bResult) @ `showvar(FieldValue.StringValue));
	return bResult || Super.GetFieldValue(FieldName, FieldValue, ArrayIndex);
}

DefaultProperties
{

}
