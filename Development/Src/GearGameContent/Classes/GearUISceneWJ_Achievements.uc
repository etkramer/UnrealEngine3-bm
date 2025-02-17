/**
 * This scene class is used for displaying lists of achievements, both completed and not.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUISceneWJ_Achievements extends GearUISceneWJ_PageBase;

/** the total number of achievements available */
var	transient				int								AchievementCount;

/** labels for displaying the title on the achievements TOC and the total gamer points earned so far */
var	transient				UILabel							lblMainTitle, lblTotalGamerPoints;

/** displays the ratio of earned gamer points to total gamer points */
var	transient				UIProgressBar					barOverallProgress;

/** the prefab used to create the panel that displays a single achievement's data */
var	transient	const		UIPrefab						AchievementPanelPrefab;

/** indicates that the profile should be saved when the user exits the achievements area */
var	transient				bool							bSaveProfileOnExit;

/* == Delegates == */

/* == Natives == */

/* == Events == */

/* == UnrealScript == */
/**
 * Assigns all member variables to appropriate child widget from this scene.
 */
function InitializeWidgetReferences()
{
	local UIDataStore_OnlinePlayerData OnlinePlayerDS;

	OnlinePlayerDS = UIDataStore_OnlinePlayerData(ResolveDataStore('OnlinePlayerData', GetPlayerOwner()));
	if ( OnlinePlayerDS != None && OnlinePlayerDS.AchievementsProvider != None )
	{
		OnlinePlayerDS.AchievementsProvider.AddPropertyNotificationChangeRequest(OnAchievementsUpdated, false);
	}

	lblMainTitle = UILabel(FindChild('lblMainTitle',true));
	lblTotalGamerPoints = UILabel(FindChild('lblTotalGamerPoints',true));
	barOverallProgress = UIProgressBar(FindChild('barOverallProgress',true));

	Super.InitializeWidgetReferences();
}

/**
 * Calculates the total number of pages required by this scene.
 *
 * @return	the total number of pages needed to display all content for this journal type.
 */
function int CalculateRequiredPageCount( int PlayerIndex=GetBestPlayerIndex() )
{
	local GearWarJournalPage_Achievements APage;
	local UIDataStore_OnlinePlayerData OnlinePlayerDS;
	local int CurrentItemCount, NumPages;

	OnlinePlayerDS = UIDataStore_OnlinePlayerData(ResolveDataStore('OnlinePlayerData', GetPlayerOwner(PlayerIndex)));
	if ( OnlinePlayerDS != None )
	{
		AchievementCount = OnlinePlayerDS.AchievementsProvider.Achievements.Length;
		APage = GearWarJournalPage_Achievements(LeftPage);

		while ( APage != None && CurrentItemCount < AchievementCount )
		{
			CurrentItemCount += APage.GetMaxItemCount(NumPages++);
		}
	}

	return NumPages;
}

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
	local GearWarJournalPage_Achievements FirstPage, SecondPage;

	bResult = Super.SetCurrentPage(NewPageIndex);
	if ( bResult )
	{
		PlayerIndex = GetPlayerOwnerIndex();

		bRightFocused = RightPage.IsFocused(PlayerIndex) || NewPageIndex == RightPage.PageIndex;

		// if the left page is currently focused, try to focus a collectable on that page first
		if ( !bRightFocused )
		{
			FirstPage = GearWarJournalPage_Achievements(LeftPage);
			SecondPage = GearWarJournalPage_Achievements(RightPage);
		}
		else
		{
			FirstPage = GearWarJournalPage_Achievements(RightPage);
			SecondPage = GearWarJournalPage_Achievements(LeftPage);
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
	local GearWarJournalPage_Achievements FirstPage, SecondPage;
	local GearAchievementPanel LeftPanel, RightPanel, LastLeftPanel, LastRightPanel, TopPanel;

	Super.SetupForcedNavigationTargets();

	FirstPage = GearWarJournalPage_Achievements(LeftPage);
	SecondPage = GearWarJournalPage_Achievements(RightPage);

	if ( FirstPage != None && SecondPage != None )
	{
		LeftCount = FirstPage.AchievementPanels.Length;
		for ( PanelIdx = 0; PanelIdx < FirstPage.AchievementPanels.Length; PanelIdx++ )
		{
			if ( FirstPage.AchievementPanels[PanelIdx].IsVisible(true) )
			{
				LastLeftPanel = FirstPage.AchievementPanels[PanelIdx];
			}
			else
			{
				LeftCount = PanelIdx;
				break;
			}
		}

		RightCount = SecondPage.AchievementPanels.Length;
		for ( PanelIdx = 0; PanelIdx < SecondPage.AchievementPanels.Length; PanelIdx++ )
		{
			if ( SecondPage.AchievementPanels[PanelIdx].IsVisible(true) )
			{
				LastRightPanel = SecondPage.AchievementPanels[PanelIdx];
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
			LeftPanel = PanelIdx < LeftCount ? FirstPage.AchievementPanels[PanelIdx] : None;
			RightPanel = PanelIdx < RightCount ? SecondPage.AchievementPanels[PanelIdx] : None;
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

/* == SequenceAction handlers == */

/* == Delegate handlers == */
/**
 * Delegate that notifies that a property has changed  Intended only for use between data providers and their owning data stores.
 * For external notifications, use the callbacks in UIDataStore instead.
 *
 * @param	SourceProvider		the data provider that generated the notification
 * @param	PropTag				the property that changed
 */
function OnAchievementsUpdated( UIDataProvider SourceProvider, optional name PropTag )
{
	if ( PropTag == 'Achievements' )
	{
		if ( LeftPage != None )
		{
			LeftPage.SetPageIndex(LeftPage.PageIndex);
		}

		if ( RightPage != None )
		{
			RightPage.SetPageIndex(RightPage.PageIndex);
		}

		SetupForcedNavigationTargets();

		// currently, labels do not automatically refresh their data store values if the data store reference is embedded in string markup
		lblTotalGamerPoints.RefreshSubscriberValue();
	}
}

/* === UIScene interface === */
event SceneDeactivated()
{
	local UIDataStore_OnlinePlayerData OnlinePlayerDS;
	local GearPC OwnerPC;

	OnlinePlayerDS = UIDataStore_OnlinePlayerData(ResolveDataStore('OnlinePlayerData', GetPlayerOwner()));
	if ( OnlinePlayerDS != None && OnlinePlayerDS.AchievementsProvider != None )
	{
		OnlinePlayerDS.AchievementsProvider.RemovePropertyNotificationChangeRequest(OnAchievementsUpdated);
	}

	if ( bSaveProfileOnExit )
	{
		OwnerPC = GetGearPlayerOwner(GetPlayerOwnerIndex());
		OwnerPC.SaveProfile();
	}

	Super.SceneDeactivated();
}

DefaultProperties
{
	AchievementPanelPrefab=UIPrefab'UI_Scenes_Prefabs.AchievementPanelPrefab'

	PreviousChapterScene=GearUISceneWJ_Base'UI_Scenes_WarJournal.UnlockablesJournal'
	NextChapterScene=None //GearUISceneWJ_Base'UI_Scenes_WarJournal.TOC.WarJournalMainTOC'
}
