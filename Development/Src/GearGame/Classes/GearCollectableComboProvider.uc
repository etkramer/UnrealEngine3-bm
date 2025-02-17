/**
 * This class is responsible for providing centralized access to static data about collectables, as well as dynamic data such as which
 * collectables have been found, the date they were found, etc.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearCollectableComboProvider extends GearResourceCombinationProvider;

/**
 * Wrapper for retrieving the discoverable id for the collecatable associated with this provider.
 */
final function EGearDiscoverableType GetCollectableId()
{
	local GearCollectableDataProvider CollectableProvider;

	CollectableProvider = GearCollectableDataProvider(StaticDataProvider);
	return CollectableProvider.CollectableId;
}

/**
 * @return	the act id for the campaign act associated with this data provider.
 */
function EGearAct GetActId()
{
	local EGearAct Result;
	local GearCollectableDataProvider StaticProvider;

	Result = GEARACT_MAX;
	StaticProvider = GearCollectableDataProvider(StaticDataProvider);
	if ( StaticProvider != None && StaticProvider.ContainingChapterProvider != None )
	{
		Result = StaticProvider.ContainingChapterProvider.ActType;
	}

	return Result;
}

/**
 * @return	the chapter point id for the campaign chapter associated with this data provider.
 */
function EChapterPoint GetChapterId()
{
	local EChapterPoint Result;
	local GearCollectableDataProvider StaticProvider;

	Result = CHAP_MAX;
	StaticProvider = GearCollectableDataProvider(StaticDataProvider);
	if ( StaticProvider != None )
	{
		Result = StaticProvider.ContainingChapterId;
	}

	return Result;
}

/**
 * @return	the data provider for the chapter containing the collectable associated with this data provider.
 */
function GearActComboProvider FindContainingActProvider()
{
	local UIDataStore_DynamicResource DynamicResourceDS;
	local array<UIResourceCombinationProvider> ActProviders;
	local GearActComboProvider Provider, Result;
	local int ProviderIdx;
	local EGearAct ActId, ProviderActId;

	// get the act id for this collectable
	ActId = GetActId();
	if ( ActId != GEARACT_MAX )
	{
		DynamicResourceDS = class'GearUIScene_Base'.static.GetDynamicResourceDataStore(ProfileProvider.Player);
		DynamicResourceDS.GetResourceProviders('Acts', ActProviders);

		// search through the act providers for the one with the matching id
		for ( ProviderIdx = 0; ProviderIdx < ActProviders.Length; ProviderIdx++ )
		{
			Provider = GearActComboProvider(ActProviders[ProviderIdx]);
			ProviderActId = Provider.GetActId();

			if ( ProviderActId != GEARACT_MAX && ProviderActId == ActId )
			{
				Result = Provider;
				break;
			}
		}
	}

	return Result;
}

/**
 * @return	the data provider for the chapter containing the collectable associated with this data provider.
 */
function GearChapterComboProvider FindContainingChapterProvider()
{
	local UIDataStore_DynamicResource DynamicResourceDS;
	local array<UIResourceCombinationProvider> ChapterProviders;
	local GearChapterComboProvider Provider, Result;
	local EChapterPoint ChapterId, ProviderChapterId;
	local int ProviderIdx;

	// get id of the chapter containing this collectable.
	ChapterId = GetChapterId();
	if ( ChapterId != CHAP_MAX )
	{
		DynamicResourceDS = class'GearUIScene_Base'.static.GetDynamicResourceDataStore(ProfileProvider.Player);
		DynamicResourceDS.GetResourceProviders('Chapters', ChapterProviders);

		// search through the list of chapter data providers for one with a matching id.
		for ( ProviderIdx = 0; ProviderIdx < ChapterProviders.Length; ProviderIdx++ )
		{
			Provider = GearChapterComboProvider(ChapterProviders[ProviderIdx]);
			ProviderChapterId = Provider.GetChapterId();

			if ( ProviderChapterId != CHAP_MAX && ProviderChapterId == ChapterId )
			{
				Result = Provider;
				break;
			}
		}
	}

	return Result;
}

/**
 * Wrapper for determining whether this collectable has been found.
 */
final function bool HasCollectableBeenFound()
{
	local GearProfileSettings GearProfile;
	local EGearDiscoverableType CollectableId;

	CollectableId = GetCollectableId();
	GearProfile = GetGearProfile();

	return GearProfile.HasDiscoverableBeenFound(CollectableId);
}

/**
 * Wrapper for determining whether this collectable has been viewed.
 */
final function bool DoesCollectibleNeedViewing()
{
	local GearProfileSettings GearProfile;
	local EGearDiscoverableType CollectableId;
	local bool bResult;

	GearProfile = GetGearProfile();
	if ( GearProfile != None )
	{
		CollectableId = GetCollectableId();
		bResult = GearProfile.IsDiscoverableMarkedForAttract(CollectableId);
	}

	return bResult;
}

/**
 * Adds this data provider to the list of collectable data providers for the act that contains the collectable associated with this data
 * provider.  The act, chapter, and collectable data providers can potentially be initialized in any order, so this method ensures that
 * regardless of the order in which they're initialized, all cross-references will be set.
 */
function ConditionalInitializeActs()
{
	local GearActComboProvider Provider;

	Provider = FindContainingActProvider();
	if ( Provider != None )
	{
		if ( INDEX_NONE == Provider.DynamicCollectableProviders.Find(Self) )
		{
			Provider.DynamicCollectableProviders.AddItem(Self);
		}
	}
}

/**
 * Adds this data provider to the list of collectable data providers for the chapter that contains the collectable associated with this data
 * provider.  The act, chapter, and collectable data providers can potentially be initialized in any order, so this method ensures that
 * regardless of the order in which they're initialized, all cross-references will be set.
 */
function ConditionalInitializeChapters()
{
	local GearChapterComboProvider Provider;

	Provider = FindContainingChapterProvider();
	if ( Provider != None )
	{
		if ( INDEX_NONE == Provider.DynamicCollectableProviders.Find(Self) )
		{
			Provider.DynamicCollectableProviders.AddItem(Self);
		}
	}
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
	ConditionalInitializeActs();
	ConditionalInitializeChapters();
}

/* === UIDataProvider interface === */
/**
 * Callback to allow script-only child classes to add their own supported tags when GetSupportedDataFields is called.
 *
 * @param	out_Fields	the list of data tags supported by this data store.
 */
event GetSupportedScriptFields( out array<UIDataProviderField> out_Fields )
{
	local GearChapterComboProvider ChapterProvider;
	local UIDataProviderField Field;

	Super.GetSupportedScriptFields(out_Fields);

	// the ContainingChapterProvider field comes from our static data provider - replace the provider reference with a reference
	// to the dynamic provider for our chapter.
	ChapterProvider = FindContainingChapterProvider();
	if ( ChapterProvider != None )
	{
		ReplaceProviderValue(out_Fields, 'ContainingChapterProvider', ChapterProvider);
	}

	Field.FieldTag = 'HasBeenFound';
	Field.FieldType = DATATYPE_Property;
	out_Fields[out_Fields.Length] = Field;

	Field.FieldTag = 'DateFound';
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

	if ( FieldName == "HasBeenFound" )
	{
		FieldValue.PropertyTag = 'HasBeenFound';
		FieldValue.PropertyType = DATATYPE_Property;

		i = int(HasCollectableBeenFound());
		FieldValue.StringValue = string(i);
		FieldValue.ArrayValue[0] = i;
		bResult = true;
	}
	else if ( FieldName == "DateFound" )
	{
		FieldValue.PropertyTag = 'DateFound';
		FieldValue.PropertyType = DATATYPE_Property;

		//@todo - get the date found from the profile
		FieldValue.StringValue = "11/18/2008";
		//FieldValue.ArrayValue[0] = i;
		bResult = true;
	}
	else if ( FieldName == "HasUpdatedData" )
	{
		FieldValue.PropertyTag = 'HasUpdatedData';
		FieldValue.PropertyType = DATATYPE_Property;

		i = int(DoesCollectibleNeedViewing());
		FieldValue.StringValue = string(i);
		FieldValue.ArrayValue[0] = i;
		bResult = true;
	}

	return bResult || Super.GetFieldValue(FieldName, FieldValue, ArrayIndex);
}

DefaultProperties
{

}
