/**
 * Base class for campaign related combo resource providers.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearCampaignComboProvider extends GearResourceCombinationProvider
	abstract;

var				array<string>							DifficultyImagePaths;

/** the providers for the list of collectables contained in this chapter or act */
var	transient	array<GearCollectableComboProvider>		DynamicCollectableProviders;

//@todo - make UIResourceCombinationProvider per-player - need access to the player's profile/
/**
 * Calculates the number of collectables that have been found according to the specified profile for this campaign resource provider.
 */
function int GetUnlockedCollectablesCount()
{
	local int ProviderIndex;
	local int Result;

	for ( ProviderIndex = 0; ProviderIndex < DynamicCollectableProviders.Length; ProviderIndex++ )
	{
		if ( DynamicCollectableProviders[ProviderIndex].HasCollectableBeenFound() )
		{
			Result++;
		}
	}

	return Result;
}

/**
 * Calculates the number of collectables that have NOT been found according to the specified profile for this campaign resource provider.
 */
function int GetLockedCollectablesCount()
{
	local int ProviderIndex;
	local int Result;

	for ( ProviderIndex = 0; ProviderIndex < DynamicCollectableProviders.Length; ProviderIndex++ )
	{
		if ( !DynamicCollectableProviders[ProviderIndex].HasCollectableBeenFound() )
		{
			Result++;
		}
	}

	return Result;
}

/**
 * Calculates the number of collectables that have been found but not yet viewed in the War Journal, according to the profile
 * for this campaign resource provider.
 */
function bool HasUnviewedCollectables()
{
	local int ProviderIndex;
	local bool bResult;

	for ( ProviderIndex = 0; ProviderIndex < DynamicCollectableProviders.Length; ProviderIndex++ )
	{
		if (DynamicCollectableProviders[ProviderIndex].HasCollectableBeenFound()
		&&	DynamicCollectableProviders[ProviderIndex].DoesCollectibleNeedViewing() )
		{
			bResult = true;
			break;
		}
	}

	return bResult;
}

/**
 * @return	TRUE if there is any data that hasn't yet been viewed by the player.
 */
function bool HasUpdatedData()
{
	return HasUnviewedCollectables();	// || HasSomethingElseHappened();
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
	// contained in this chapter
	ReplaceProviderCollection(out_Fields, 'Collectables', DynamicCollectableProviders);

	Field.FieldTag = 'HasFoundAllCollectables';
	Field.FieldType = DATATYPE_Property;
	out_Fields[out_Fields.Length] = Field;

	Field.FieldTag = 'CollectablesFound';
	out_Fields[out_Fields.Length] = Field;

	Field.FieldTag = 'TotalCollectables';
	out_Fields[out_Fields.Length] = Field;

	Field.FieldTag = 'HasUpdatedData';
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
	local int i;
	local bool bResult;

	if ( FieldName == "HasFoundAllCollectables" )
	{
		FieldValue.PropertyTag = 'HasFoundAllCollectables';
		FieldValue.PropertyType = DATATYPE_Property;

		i = int(GetLockedCollectablesCount() == 0);
		FieldValue.StringValue = string(i);
		FieldValue.ArrayValue[0] = i;
		bResult = true;
	}
	else if ( FieldName == "CollectablesFound" )
	{
		FieldValue.PropertyTag = 'CollectablesFound';
		FieldValue.PropertyType = DATATYPE_Property;

		i = GetUnlockedCollectablesCount();
		FieldValue.StringValue = string(i);
		FieldValue.RangeValue.CurrentValue = i;
		FieldValue.RangeValue.bIntRange = true;
		bResult = true;
	}
	else if ( FieldName == "TotalCollectables" )
	{
		FieldValue.PropertyTag = 'TotalCollectables';
		FieldValue.PropertyType = DATATYPE_Property;
		FieldValue.StringValue = string(DynamicCollectableProviders.Length);
		bResult = true;
	}
	else if ( FieldName == "HasUpdatedData" )
	{
		FieldValue.PropertyTag = 'HasUpdatedData';
		FieldValue.PropertyType = DATATYPE_Property;

		i = int(HasUpdatedData());
		FieldValue.StringValue = string(i);
		FieldValue.ArrayValue[0] = i;
		bResult = true;
	}

	return bResult || Super.GetFieldValue(FieldName, FieldValue, ArrayIndex);
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
	FieldNames.AddItem('Collectables');

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

	if ( FieldName == nameof(DynamicCollectableProviders) || FieldName == 'Collectables' )
	{
		Result = DynamicCollectableProviders.Length;
	}
	else
	{
		Result = Super.GetElementCount(FieldName);
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
event GetElementCellTags( name FieldName, out array<name> CellFieldTags, optional out array<string> ColumnHeaderDisplayText )
{
	Super.GetElementCellTags(FieldName, CellFieldTags, ColumnHeaderDisplayText);

	CellFieldTags.AddItem('HasFoundAllCollectables');
	CellFieldTags.AddItem('CollectablesFound');
	CellFieldTags.AddItem('TotalCollectables');
	CellFieldTags.AddItem('HasUpdatedData');
	ColumnHeaderDisplayText.AddItem("HasFoundAllCollectables");
	ColumnHeaderDisplayText.AddItem("CollectablesFound");
	ColumnHeaderDisplayText.AddItem("TotalCollectables");
	ColumnHeaderDisplayText.AddItem("HasUpdatedData");
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

	bResult = GetCellFieldType(FieldName, CellTag, FieldType);
	if ( !bResult )
	{
		if (CellTag == 'HasFoundAllCollectables'
		||	CellTag == 'CollectablesFound'
		||	CellTag == 'TotalCollectables'
		||	CellTag == 'HasUpdatedData')
		{
			FieldType = DATATYPE_Property;
			bResult = true;
		}
	}
	return bResult;
}

DefaultProperties
{
	DifficultyImagePaths(DL_Casual)="<Images:UI_Portraits.Difficulty.DL_Casual_SM>"
	DifficultyImagePaths(DL_Normal)="<Images:UI_Portraits.Difficulty.DL_Nrmal_SM>"
	DifficultyImagePaths(DL_Hardcore)="<Images:UI_Portraits.Difficulty.DL_Hardcore_SM>"
	DifficultyImagePaths(DL_Insane)="<Images:UI_Portraits.Difficulty.Difficulty_InsaneSM>"
}
