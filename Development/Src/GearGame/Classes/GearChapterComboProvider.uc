/**
 * This class is responsible for providing centralized access to both static and dynamic data for a single Gears2 campaign act.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearChapterComboProvider extends GearCampaignComboProvider;

/**
 * @return	the act id for the campaign act associated with this data provider.
 */
function EGearAct GetActId()
{
	local EGearAct Result;
	local GearCampaignChapterData StaticChapterProvider;

	Result = GEARACT_MAX;
	StaticChapterProvider = GearCampaignChapterData(StaticDataProvider);
	if ( StaticChapterProvider != None )
	{
		Result = StaticChapterProvider.ActType;
	}

	return Result;
}

/**
 * @return	the chapter point id for the campaign chapter associated with this data provider.
 */
function EChapterPoint GetChapterId()
{
	local EChapterPoint Result;
	local GearCampaignChapterData StaticChapterProvider;

	Result = CHAP_MAX;
	StaticChapterProvider = GearCampaignChapterData(StaticDataProvider);
	if ( StaticChapterProvider != None )
	{
		Result = StaticChapterProvider.ChapterType;
	}

	return Result;
}

/**
 * Adds this data provider to the list of chapter data providers for the act that contains the chapter associated with this data
 * provider.  The act, chapter, and collectable data providers can potentially be initialized in any order, so this method ensures that
 * regardless of the order in which they're initialized, all cross-references will be set.
 */
function ConditionalInitializeOwningAct()
{
	local UIDataStore_DynamicResource DynamicResourceDS;
	local array<UIResourceCombinationProvider> ActProviders;
	local GearActComboProvider Provider;
	local int ProviderIdx, Location;
	local EGearAct ActId, ProviderActId;

	DynamicResourceDS = class'GearUIScene_Base'.static.GetDynamicResourceDataStore(ProfileProvider.Player);
	DynamicResourceDS.GetResourceProviders('Acts', ActProviders);

	ActId = GetActId();
	if ( ActId != GEARACT_MAX )
	{
		for ( ProviderIdx = 0; ProviderIdx < ActProviders.Length; ProviderIdx++ )
		{
			Provider = GearActComboProvider(ActProviders[ProviderIdx]);
			ProviderActId = Provider.GetActId();

			if ( ProviderActId != GEARACT_MAX && ProviderActId == ActId )
			{
				Location = Provider.ChapterProviders.Find(Self);
				if ( Location == INDEX_NONE )
				{
					Provider.ChapterProviders.AddItem(Self);
				}
			}
		}
	}
}

/**
 * Initializes the list of providers for the collectables contained in the chapter associated with this provider.
 */
function InitializeCollectables()
{
	local UIDataStore_DynamicResource DynamicResourceDS;
	local array<UIResourceCombinationProvider> CollectableProviders;
	local GearCollectableComboProvider Provider;
	local EChapterPoint ChapterType, CollectableChapterId;
	local int ProviderIdx;

	DynamicCollectableProviders.Length = 0;
	DynamicResourceDS = class'GearUIScene_Base'.static.GetDynamicResourceDataStore(ProfileProvider.Player);
	DynamicResourceDS.GetResourceProviders('Collectables', CollectableProviders);

	ChapterType = GetChapterId();
	if ( ChapterType != CHAP_MAX )
	{
		for ( ProviderIdx = 0; ProviderIdx < CollectableProviders.Length; ProviderIdx++ )
		{
			Provider = GearCollectableComboProvider(CollectableProviders[ProviderIdx]);
			CollectableChapterId = Provider.GetChapterId();

			// if the collectable's chapter id matches this provider's value, add it to our list
			if ( CollectableChapterId != CHAP_MAX && ChapterType == CollectableChapterId )
			{
				DynamicCollectableProviders.AddItem(Provider);
			}
		}
	}
}

function EDifficultyLevel GetHighestCompletedDifficulty()
{
	local int DiffIndex;
	local EDifficultyLevel Result;
	local EChapterPoint ChapterId;
	local GearProfileSettings Profile;

	Result = DL_MAX;
	Profile = GetGearProfile();
	if ( Profile != None )
	{
		ChapterId = GetChapterId();
		if ( ChapterId != CHAP_MAX )
		{
			// since we're checking whether the player has completed this chapter, we'll need to check whether the next chapter has been
			// unlocked for each difficulty, so increment the ChapterId first
			ChapterId = EChapterPoint(ChapterId + 1);

			for ( DiffIndex = DL_MAX - 1; DiffIndex >= 0; DiffIndex-- )
			{
				if ( Profile.HasChapterBeenUnlocked(ChapterId, EDifficultyLevel(DiffIndex)) )
				{
					Result = EDifficultyLevel(DiffIndex);
					break;
				}
			}
		}
		else
		{
			`log(`location @ "invalid ChapterId for GearChapterComboProvider:" @ ChapterId);
		}
	}
	else
	{
		`log(`location @ "no valid profile bound to this GearChapterComboProvider!");
	}

	return Result;
}

/* === GearCampaignComboProvider interface === */
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

	ConditionalInitializeOwningAct();
	InitializeCollectables();
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

	CellFieldTags[CellFieldTags.Length] = 'DifficultyCompletedIcon';
	ColumnHeaderDisplayText[ColumnHeaderDisplayText.Length] = "DifficultyCompletedIcon";
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
	if ( !bResult && CellTag == 'DifficultyCompletedIcon' )
	{
		FieldType = DATATYPE_Property;
		bResult = true;
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
	if ( !bResult && FieldName == 'DifficultyCompletedIcon' )
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
			`log(`location @ "has not completed chapter" @ GetChapterId() @ "on any difficulty.",,'RON_DEBUG');
		}
		bResult = true;
	}

//`log(`location @ `showvar(FieldName) @ `showvar(ArrayIndex) @ `showvar(bResult) @ `showvar(FieldValue.StringValue));
	return bResult || Super.GetFieldValue(FieldName, FieldValue, ArrayIndex);
}

DefaultProperties
{
}
