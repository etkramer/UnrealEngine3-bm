/**
 * Data provider for campaign chapters; DefaultGame.ini
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearCampaignChapterData extends GearCampaignResourceProvider
	native(inherit);

/** Enum of the chapter */
var config EChapterPoint	ChapterType;

/** the name of the map data provider associated with this chapter */
//var	config		string	MapProviderName;

/** the name of the map associated with this chapter */
var	config		string		MapName;

/**
 * Initializes the list of providers for the collectables contained in the chapter associated with this provider.
 */
function InitializeCollectables()
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> CollectableProviders;
	local GearCollectableDataProvider Provider;
	local int ProviderIdx;

	Collectables.Length = 0;

	GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
	GameResourceDS.GetResourceProviders('Collectables', CollectableProviders);
	for ( ProviderIdx = 0; ProviderIdx < CollectableProviders.Length; ProviderIdx++ )
	{
		Provider = GearCollectableDataProvider(CollectableProviders[ProviderIdx]);

		// if the collectable's chapter id matches this provider's value, add it to our list
		if ( Provider.ContainingChapterId == ChapterType )
		{
			Collectables.AddItem(Provider);
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

	InitializeCollectables();
}

DefaultProperties
{

}
