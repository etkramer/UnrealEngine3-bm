/**
 * This scene class is used to display the items that the player has collected during the game.  For collectables which the player hasn't
 * yet found, a watermark is displayed as a placeholder.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUISceneWJ_Discoverable extends GearUISceneWJ_PageBase;

/** the total number of collectables available */
var	transient			int										CollectableCount;

/**
 * the prefab used to create the panel that displays a single collectable's data
 * 1st element is the prefab that has the icon on the left side; 2nd element has the icon on the right side.
 */
var	transient	const	UIPrefab								CollectablePanelPrefab[2];

/**
 * The scene to use for displaying more detailed information for a single discoverable.
 */
var	transient	const	GearUISceneWJ_DiscoverableDetails		DetailsSceneResource;

/**
 * Calculates the total number of pages required by this scene.
 *
 * @return	the total number of pages needed to display all content for this journal type.
 */
function int CalculateRequiredPageCount( int PlayerIndex=GetBestPlayerIndex() )
{
	local GearWarJournalPage_Collectables CPage;
	local GearUIDataStore_GameResource GameResourceDS;
	local int CurrentItemCount, NumPages;

	GameResourceDS = GetGameResourceDataStore();
	CollectableCount = GameResourceDS.GetProviderCount('Collectables');

	CPage = GearWarJournalPage_Collectables(LeftPage);
	while ( CPage != None && CurrentItemCount < CollectableCount )
	{
		CurrentItemCount += CPage.GetMaxItemCount(NumPages++);
	}

	return NumPages;
}

/**
 * Determine the ActId of the act that contains a collectable based on the index of the collectable.
 *
 * @param	CollectableIndex	the index [into the complete list of collectables] for the collectable to get the containing act for.
 *
 * @return	the ActType value for the act that contains the specified collectable.
 */
static function EGearAct GetActIdFromCollectableIndex( int CollectableIndex )
{
	local GearCampaignActData ActProvider;
	local EChapterPoint ChapterId;

	ChapterId = GetChapterIdFromCollectableIndex(CollectableIndex);
	ActProvider = class'GearUIDataStore_GameResource'.static.GetActDataProviderUsingChapter(ChapterId);
	return ActProvider.ActType;
}


/**
 * Determine the chapter id of the chapter that contains a collectable based on the index of the collectable.
 *
 * @param	CollectableIndex	the index [into the complete list of collectables] for the collectable to get the containing act for.
 *
 * @return	the ChapterType value for the chapter that contains the specified collectable.
 */
static function EChapterPoint GetChapterIdFromCollectableIndex( int CollectableIndex )
{
	local EChapterPoint Result;
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue FieldValue;

	GameResourceDS = GetGameResourceDataStore();
	if ( GameResourceDS.GetProviderFieldValue('Collectables', 'ContainingChapterId', CollectableIndex, FieldValue) )
	{
		Result = EChapterPoint(ConvertEnumStringToValue(FieldValue.StringValue, CHAP_MAX, enum'EChapterPoint'));
	}
	else
	{
		`log("GetChapterIdFromCollectableIndex - couldn't get field value for 'ContainingChapterId' using CollectableIndex" @ CollectableIndex);
	}

	return Result;
}

/**
 * Wrapper for retrieving the collectable id of a specific collectable.
 *
 * @param	CollectableIndex	the index [into the complete list of collectables] for the collectable to get the collectable id for.
 *
 * @return	the EGearDiscoverableType value for the specified collectable.
 */
static function EGearDiscoverableType GetCollectableIdFromCollectableIndex( int CollectableIndex )
{
	local EGearDiscoverableType Result;
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue FieldValue;

	GameResourceDS = GetGameResourceDataStore();
	if ( GameResourceDS.GetProviderFieldValue('Collectables', 'CollectableId', CollectableIndex, FieldValue) )
	{
		Result = EGearDiscoverableType(ConvertEnumStringToValue(FieldValue.StringValue, eDISC_MAX, enum'EGearDiscoverableType'));
	}
	else
	{
		`log("GetCollectableIdFromCollectableIndex - couldn't get field value for 'CollectableId' using CollectableIndex" @ CollectableIndex);
	}

	return Result;
}

/**
 * Calculates the page index of the first collectable in a particular act.
 *
 * @param	DesiredActId	the id of the act to get the page index for.
 *
 * @return	the index of the page that contains the first collectable in the specified act, or INDEX_NONE if the page index
 *			couldn't be calculated.
 */
static function int CalculatePageIndexForAct( EGearAct DesiredActId )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> ActProviders;
	local GearCampaignActData ActProvider;
	local GearCampaignChapterData ChapterProvider;
	local int ActIndex, ChapterIndex, PageIdx, RemainingCollectablesInChapter, Result;

	Result = INDEX_NONE;
	PageIdx = 0;

	GameResourceDS = GetGameResourceDataStore();
	if ( GameResourceDS.GetResourceProviders('Acts', ActProviders) && ActProviders.Length > 0 )
	{
		// since each chapter can have a different number of collectables, we must check each chapter.
		// Start at the beginning, accumulating the number of collectables until we either run out of collectables
		// for the current chapter, or run out of space on the page.  Keep going until we reach the correct act, keeping
		// track of the number of pages we've traversed
		for ( ActIndex = 0; ActIndex < ActProviders.Length; ActIndex++ )
		{
			ActProvider = GearCampaignActData(ActProviders[ActIndex]);
			if ( ActProvider.ActType == DesiredActId )
			{
				Result = PageIdx;
				break;
			}

			for ( ChapterIndex = 0; ChapterIndex < ActProvider.ChapterProviders.Length; ChapterIndex++ )
			{
				ChapterProvider = ActProvider.ChapterProviders[ChapterIndex];

				RemainingCollectablesInChapter = ChapterProvider.Collectables.Length;
				while ( RemainingCollectablesInChapter > 0 )
				{
					RemainingCollectablesInChapter -= Clamp(RemainingCollectablesInChapter, 0, `MAX_COLLECTABLES_PER_PAGE);
					PageIdx++;
				}
			}
		}
	}
	else
	{
		`warn("No data providers found for campaign acts using tag Acts");
	}

	return Result;
}

/* == Delegates == */

/* == Natives == */

/* == Events == */

/* == UnrealScript == */

/* == SequenceAction handlers == */

/* == Delegate handlers == */

`if(`isdefined(WJ_CHAPTER_NAVIGATION_SUPPORTED))
/* === GearUISceneWJ_PageBase interface === */
/**
 * Wrapper for replacing this scene with the scene for the previous section of the War Journal
 */
function bool OpenPreviousChapter( int PlayerIndex )
{
	local GearWarJournalPage_Collectables CPage;
	local EGearAct PreviousActId;
	local int PreviousActPageIndex;
	local bool bResult;

	// always use the right page - if the collectables for an act start on the right page, then the left and right pages would be
	// different acts, but if the collectables start on the left page, the right page will always be the same act
	CPage = GearWarJournalPage_Collectables(RightPage);
	if ( CPage.PageActId > GEARACT_I )
	{
		PreviousActId = EGearAct(CPage.PageActId - 1);
		PreviousActPageIndex = CalculatePageIndexForAct(PreviousActId);
		if ( PreviousActPageIndex != INDEX_NONE )
		{
			bResult = SetCurrentPage(PreviousActPageIndex);
		}
	}

	return bResult || Super.OpenPreviousChapter(PlayerIndex);
}

/**
 * Wrapper for replacing this scene with the scene for the next section of the War Journal
 */
function bool OpenNextChapter( int PlayerIndex )
{
	local GearWarJournalPage_Collectables CPage;
	local EGearAct NextActId;
	local int NextActPageIndex;
	local bool bResult;

	// always use the right page - if the collectables for an act start on the right page, then the left and right pages would be
	// different acts, but if the collectables start on the left page, the right page will always be the same act
	CPage = GearWarJournalPage_Collectables(RightPage);
	if ( CPage.PageActId < GEARACT_MAX - 1 )
	{
		NextActId = EGearAct(CPage.PageActId + 1);
		NextActPageIndex = CalculatePageIndexForAct(NextActId);
		if ( NextActPageIndex != INDEX_NONE )
		{
			bResult = SetCurrentPage(NextActPageIndex);
		}
	}

	return bResult || Super.OpenNextChapter(PlayerIndex);
}
`endif


/**
 * Changes the currently active "page".
 *
 * @param	NewPageIndex	the index for the page that should be active (should always be the index of a page on the left)
 *
 * @return	true if the page was changed successfully.
 */
function bool SetCurrentPage( int NewPageIndex )
{
	local UIObject CurrentlyFocused;
	local bool bResult, bFirstUnlocked, bSecondUnlocked;
	local bool bRightFocused;
	local int PlayerIndex;
	local GearWarJournalPage_Collectables FirstPage, SecondPage;

	bResult = Super.SetCurrentPage(NewPageIndex);
	if ( bResult )
	{
		PlayerIndex = GetPlayerOwnerIndex();

		bRightFocused = RightPage.IsFocused(PlayerIndex) || NewPageIndex == RightPage.PageIndex;

		// if the left page is currently focused, try to focus a collectable on that page first
		if ( !bRightFocused )
		{
			FirstPage = GearWarJournalPage_Collectables(LeftPage);
			SecondPage = GearWarJournalPage_Collectables(RightPage);
		}
		else
		{
			FirstPage = GearWarJournalPage_Collectables(RightPage);
			SecondPage = GearWarJournalPage_Collectables(LeftPage);
		}

		bFirstUnlocked = FirstPage.InitializeFocus();
		if ( !bFirstUnlocked )
		{
			bSecondUnlocked = SecondPage.InitializeFocus();
		}

		// if neither page had unlocked collectables, kill focus on the currently focused panel and reset focus on the page itself
		// so that the selection bar goes away and navigation is disabled.
		if ( !bFirstUnlocked && !bSecondUnlocked )
		{
			CurrentlyFocused = GetFocusedControl(true, PlayerIndex);
			if ( CurrentlyFocused != None )
			{
//				`log(`showvar(CurrentlyFocused.KillFocus(None, PlayerIndex)));
				CurrentlyFocused.KillFocus(None, PlayerIndex);
			}

			SetFocus(None, PlayerIndex);
		}

//		`log(`location @ `showvar(CurrentPage) @ `showvar(bRightFocused) @ `showvar(bFirstUnlocked) @ `showvar(bSecondUnlocked) @ `showobj(CurrentlyFocused));
	}

	return bResult;
}
/**
 * Sets up forced navigation links between panels in the left and right pages.
 */
function SetupForcedNavigationTargets()
{
	local int PanelIdx, PanelCount, LeftCount, RightCount;
	local GearWarJournalPage_Collectables FirstPage, SecondPage;
	local GearCollectablePanel LeftPanel, RightPanel, LastLeftPanel, LastRightPanel, TopPanel;

	Super.SetupForcedNavigationTargets();

	FirstPage = GearWarJournalPage_Collectables(LeftPage);
	SecondPage = GearWarJournalPage_Collectables(RightPage);

	if ( FirstPage != None && SecondPage != None )
	{
		LeftCount = FirstPage.CollectablePanels.Length;
		for ( PanelIdx = 0; PanelIdx < FirstPage.CollectablePanels.Length; PanelIdx++ )
		{
			if ( FirstPage.CollectablePanels[PanelIdx].IsVisible(true) )
			{
				LastLeftPanel = FirstPage.CollectablePanels[PanelIdx];
			}
			else
			{
				LeftCount = PanelIdx;
				break;
			}
		}

		RightCount = SecondPage.CollectablePanels.Length;
		for ( PanelIdx = 0; PanelIdx < SecondPage.CollectablePanels.Length; PanelIdx++ )
		{
			if ( SecondPage.CollectablePanels[PanelIdx].IsVisible(true) )
			{
				LastRightPanel = SecondPage.CollectablePanels[PanelIdx];
				if ( TopPanel == None )
				{
					TopPanel = LastRightPanel;
				}
			}
			else
			{
				RightCount = PanelIdx;
				break;
			}
		}

		PanelCount = Max(LeftCount, RightCount);
//		`log(`location @ `showvar(LeftCount) @ `showvar(RightCount) @ `showvar(PanelCount) @ `showvar(PathName(LastLeftPanel),LastPanelLeft) @ `showvar(PathName(LastRightPanel),LastPanelRight) @ `showvar(PathName(TopPanel),TopPanel));
		for ( PanelIdx = 0; PanelIdx < PanelCount; PanelIdx++ )
		{
			LeftPanel = PanelIdx < LeftCount ? FirstPage.CollectablePanels[PanelIdx] : None;
			RightPanel = PanelIdx < RightCount ? SecondPage.CollectablePanels[PanelIdx] : None;
			if ( LeftPanel != None )
			{
//			`log(PanelIdx $ ")" @ PathName(LeftPanel) @ "setting forced navigation right to" @ PathName(RightPanel));
				LeftPanel.SetForcedNavigationTarget(UIFACE_Right, RightPanel != None ? RightPanel : LastRightPanel, true);
				LeftPanel.SetForcedNavigationTarget(UIFACE_Bottom, None);
			}

			if ( RightPanel != None )
			{
//			`log(PanelIdx $ ")" @ PathName(RightPanel) @ "setting forced navigation left to" @ PathName(LeftPanel));
				RightPanel.SetForcedNavigationTarget(UIFACE_Left, LeftPanel != None ? LeftPanel : LastLeftPanel, true);
				RightPanel.SetForcedNavigationTarget(UIFACE_Bottom, None);
			}
		}

		if ( LastLeftPanel != None )
		{
//		`log(PathName(LastLeftPanel) @ "setting forced wrap to top panel" @ PathName(TopPanel));
			LastLeftPanel.SetForcedNavigationTarget(UIFACE_Bottom, TopPanel, true);
		}

		if ( TopPanel != None )
		{
//		`log(PathName(TopPanel) @ "setting forced wrap to bottom left panel" @ PathName(LastLeftPanel));
			TopPanel.SetForcedNavigationTarget(UIFACE_Top, LastLeftPanel, true);
		}
	}
}


DefaultProperties
{
	CollectablePanelPrefab(0)=UIPrefab'UI_Scenes_Prefabs.WarJournal.CollectablePanelPrefab'
	CollectablePanelPrefab(1)=UIPrefab'UI_Scenes_Prefabs.WarJournal.CollectablePanelPrefabReverse'
	DetailsSceneResource=GearUISceneWJ_DiscoverableDetails'UI_Scenes_WarJournal.DiscoverableDetails'

	PreviousChapterScene=GearUISceneWJ_Base'UI_Scenes_WarJournal.TOC.WarJournalCollectableTOC'
	NextChapterScene=GearUISceneWJ_Base'UI_Scenes_WarJournal.UnlockablesJournal'
}
