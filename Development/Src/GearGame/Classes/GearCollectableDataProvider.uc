/**
 * Provides the UI with data about a single collectable item in Gears2.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearCollectableDataProvider extends GearResourceDataProvider
	native(inherit);

var	config		EGearDiscoverableType			CollectableId;
var	localized	string							CollectableName;
var	localized	string							CollectableDetails;

var	config		string							LockedIcon_Markup;
var	config		string							UnlockedIcon_Markup;
var	config		string							UnlockedImage_Markup;

var config		EChapterPoint					ContainingChapterId;
var	transient	GearCampaignChapterData			ContainingChapterProvider;

/**
 * Provides the data provider with the chance to perform initialization, including preloading any content that will be needed by the provider.
 *
 * @param	bIsEditor	TRUE if the editor is running; FALSE if running in the game.
 */
event InitializeProvider( bool bIsEditor )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> AllChapterProviders;
	local GearCampaignChapterData Provider;
	local int ChapterIdx;

	Super.InitializeProvider(bIsEditor);

	ContainingChapterProvider = None;
	GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
	GameResourceDS.GetResourceProviders('Chapters', AllChapterProviders);
	for ( ChapterIdx = 0; ChapterIdx < AllChapterProviders.Length; ChapterIdx++ )
	{
		Provider = GearCampaignChapterData(AllChapterProviders[ChapterIdx]);
		if ( Provider.ChapterType == ContainingChapterId )
		{
			ContainingChapterProvider = Provider;
			break;
		}
	}
}

DefaultProperties
{

}
