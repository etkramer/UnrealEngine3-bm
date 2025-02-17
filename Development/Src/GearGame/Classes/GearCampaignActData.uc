/**
 * Data provider for displaying acts; DefaultGame.ini
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearCampaignActData extends GearCampaignResourceProvider
	native(inherit);

/**
 * The list of providers for the chapters contained in this act
 * Generated at startup when the GearUIDataStore_GameResource data store is registered.
 */
var transient array<GearCampaignChapterData> ChapterProviders;

/** static data */
/** Whether this act counts toward beating the shipped game or not */
var config bool bRequiredActForGameCompletion;

/**
 * Populates the list of chapter providers for the chapters contained in the act associated with this data provider.
 */
function InitializeChapters()
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> AllChapterProviders;
	local GearCampaignChapterData Provider;
	local int ChapterIdx;

	ChapterProviders.Length = 0;

	GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
	GameResourceDS.GetResourceProviders('Chapters', AllChapterProviders);

	for ( ChapterIdx = 0; ChapterIdx < AllChapterProviders.Length; ChapterIdx++ )
	{
		Provider = GearCampaignChapterData(AllChapterProviders[ChapterIdx]);
		if ( Provider.ActType == ActType )
		{
			ChapterProviders.AddItem(Provider);
		}
	}
}

/**
 * Initializes the list of providers for the collectables contained in the act associated with this provider.
 */
function InitializeCollectables()
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> AllCollectables;
	local GearCampaignChapterData Provider;
	local GearCollectableDataProvider CollectableProvider;
	local int ChapterIdx, CollectableIdx;

	Collectables.Length = 0;
	GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
	GameResourceDS.GetResourceProviders('Collectables', AllCollectables);

	for ( ChapterIdx = 0; ChapterIdx < ChapterProviders.Length; ChapterIdx++ )
	{
		Provider = ChapterProviders[ChapterIdx];
		for ( CollectableIdx = 0; CollectableIdx < AllCollectables.Length; CollectableIdx++ )
		{
			CollectableProvider = GearCollectableDataProvider(AllCollectables[CollectableIdx]);
			if ( CollectableProvider.ContainingChapterId == Provider.ChapterType )
			{
				Collectables.AddItem(CollectableProvider);
				AllCollectables.Remove(CollectableIdx--, 1);
			}
		}
	}
}

/* === UIResourceDataProvider interface === */
/**
 * Provides the data provider with the chance to perform initialization, including preloading any content that will be needed by the provider.
 *
 * @param	bIsEditor	TRUE if the editor is running; FALSE if running in the game.
 */
event InitializeProvider( bool bIsEditor )
{
	Super.InitializeProvider(bIsEditor);

	InitializeChapters();
	InitializeCollectables();
}


DefaultProperties
{

}


