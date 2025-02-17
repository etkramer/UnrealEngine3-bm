/**
 * Panel for Achievements area of War Journal.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearWarJournalPage_Achievements extends GearWarJournalPage_ChapterBase;

/** the scene that contains this panel */
var	transient				GearUISceneWJ_Achievements		AchievementsWJScene;

/** the label that display the "Achievements cont." string */
var	transient				UILabel							lblPageTitle;

/** the array of panels that display information on each achievement; each panel displays data for a single achievement */
var	transient				array<GearAchievementPanel>		AchievementPanels;

/* == Delegates == */

/* == Natives == */

/* == Events == */

/* == UnrealScript == */
/**
 * Wrapper for getting a reference to the owning war journal achievements scene.
 */
final function GearUISceneWJ_Achievements GetOwnerAchievementsScene()
{
	return GearUISceneWJ_Achievements(GetOwnerWarJournalScene());
}

/**
 * Synchronizes the number of achievement panels with the number of achievements being displayed for the current page.  If more panels
 * are needed, instances new panels from the panel prefab.  If too many panels exist, removes those panels.
 *
 * @param	TargetPanel		the panel that will contain the achievement panels.
 * @param	StartIndex		the index of the first achievement to display on that page.
 * @param	Count			the number of achievements that should be displayed on this page.
 */
function SynchronizeAchievements( int StartIndex, int Count )
{
	local int PanelIdx, AchievementIdx, PlayerIndex, VisiblePanels, PanelCount;
	local UIPrefabInstance Inst, PreviousInst;
	local vector2D PanelPosition;
	local float X, Y;
	local GearAchievementPanel CurrentPanel, PreviousPanel;
	local bool bViewingNewlyCompletedAchievement;

	Count = Clamp(Count, 0, AchievementsWJScene.AchievementCount - StartIndex);
	X = GetPosition(UIFACE_Left, EVALPOS_PixelViewport);
	if ( PageIndex == 0 )
	{
		Y = AchievementsWJScene.lblTotalGamerPoints.GetPosition(UIFACE_Bottom, EVALPOS_PixelViewport) + 5;
	}
	else
	{
		Y = lblPageTitle.GetPosition(UIFACE_Bottom, EVALPOS_PixelViewport);
	}
	PanelPosition.X = ResolveUIExtent(AchievementsWJScene.AchievementPanelPrefab.OriginalWidth,GetScene());
	PanelPosition.Y = ResolveUIExtent(AchievementsWJScene.AchievementPanelPrefab.OriginalHeight,GetScene());

	// if we have more panels than we need, remove the extra ones first
	PlayerIndex = GetPlayerOwnerIndex();
	VisiblePanels = AchievementPanels.Length;
	while ( VisiblePanels > Count )
	{
		PanelIdx = AchievementPanels.Length - 1;
		CurrentPanel = AchievementPanels[PanelIdx];
		if ( CurrentPanel != None )
		{
			Inst = UIPrefabInstance(CurrentPanel.GetOwner());
			if ( Inst != None && CurrentPanel.IsFocused(PlayerIndex) )
			{
				CurrentPanel.KillFocus(None, PlayerIndex);
			}
		}
		else
		{
			AchievementPanels.Remove(PanelIdx,1);
		}

		VisiblePanels--;
		CurrentPanel = None;
		Inst = None;
	}

	PanelCount = Max(3, Max(Count, AchievementPanels.Length));
	for ( PanelIdx = 0; PanelIdx < PanelCount; PanelIdx++ )
	{
		PreviousPanel = CurrentPanel;
		PreviousInst = Inst;

		CurrentPanel = None;
		Inst = None;

		if ( PanelIdx < AchievementPanels.Length )
		{
			CurrentPanel = AchievementPanels[PanelIdx];
			if ( CurrentPanel == None )
			{
				AchievementPanels.Remove(PanelIdx--,1);
				continue;
			}

			Inst = UIPrefabInstance(CurrentPanel.GetOwner());
		}
		else
		{
			Inst = InstanceUIPrefab(AchievementsWJScene.AchievementPanelPrefab, name("AchievementUIPI_" $ PanelIdx), PanelPosition);
			CurrentPanel = GetAchievementPanelFromPrefabInstance(Inst);
		}

		Inst.SetPosition(X, UIFACE_Left, EVALPOS_PixelViewport);
		Inst.SetPosition(Y, UIFACE_Top, EVALPOS_PixelViewport);

		Inst.SetVisibility(PanelIdx < Count);
		AchievementIdx = StartIndex + PanelIdx;
		if ( CurrentPanel != None )
		{
			bViewingNewlyCompletedAchievement = CurrentPanel.SetAchievementIndex(AchievementIdx) || bViewingNewlyCompletedAchievement;

			Inst.TabIndex = PanelIdx;
			if ( PanelIdx < Count )
			{
				Inst.SetForcedNavigationTarget(UIFACE_Top, PreviousInst);
			}
			else
			{
				Inst.SetForcedNavigationTarget(UIFACE_Top, None, true);
			}
			if ( PreviousPanel != None )
			{
				PreviousInst = UIPrefabInstance(PreviousPanel.GetOwner());
				if ( PanelIdx < Count )
				{
					PreviousInst.SetForcedNavigationTarget(UIFACE_Bottom, Inst);
				}
				else
				{
					PreviousInst.SetForcedNavigationTarget(UIFACE_Bottom, None, true);
				}
			}

			Inst.SetEnabled(PanelIdx < Count, PlayerIndex);
			Y += PanelPosition.Y;
		}
	}

	if ( bViewingNewlyCompletedAchievement )
	{
		AchievementsWJScene.bSaveProfileOnExit = true;
	}
}

/* == SequenceAction handlers == */

/* == Delegate handlers == */
/**
 * Called immediately before the scene performs an update.  Only called if assigned and the value of
 * bEnableSceneUpdateNotifications is true.
 *
 * This version initializes the focus chain since we had to dynamically add children after the initial focus propagation was setup.
 */
function PostSceneUpdate( UIObject Sender )
{
	bEnableSceneUpdateNotifications = false;
	OnPostSceneUpdate = None;
}

/**
 * Sets focus to the first eligable panel.
 *
 * @return	TRUE if a panel was focused; FALSE if none were eligible for focus.
 */
function bool InitializeFocus()
{
	local UIObject InitiallyFocusedPI, CurrentlyFocusedChild;
	local GearAchievementPanel InitiallyFocusedPanel;
	local int Idx, PlayerIndex;
	local bool bResult;

	PlayerIndex = GetPlayerOwnerIndex();
//	`log(`location @ `showvar(IsFocused()) @ `showvar(GetFocusedControl(),FocusedControl) @ `showvar(OnPostSceneUpdate) @ `showvar(bEnableSceneUpdateNotifications),,'REMOVE_ME');

	InitiallyFocusedPI = FindChild('AchievementUIPI_0',true);
	while ( InitiallyFocusedPI != None && InitiallyFocusedPI.IsDisabled(PlayerIndex,false) )
	{
		InitiallyFocusedPI = FindChild(name("AchievementUIPI_" $ ++Idx), true);
	}

	if ( InitiallyFocusedPI != None )
	{
		InitiallyFocusedPanel = GearAchievementPanel(InitiallyFocusedPI.FindChild('GearAchievementPanel_Arc_0'));
		if ( InitiallyFocusedPanel != None )
		{
			InitiallyFocusedPanel.SetFocus(None);
			if ( InitiallyFocusedPanel.IsFocused() )
			{
				// make sure the selection bar is visible in case the first panel was already
				// focused (in which case it's NotifyStateChange delegate wouldn't be called)
				InitiallyFocusedPanel.ShowSelectionBar(true);
				bResult = true;
			}
		}
		else
		{
			// otherwise, the is the first time we've generated the list of panels, so just let the focus chain do its thang
			SetFocus(None);
		}
	}
	else
	{
		// otherwise, the is the first time we've generated the list of panels, so just let the focus chain do its thang
		CurrentlyFocusedChild = GetFocusedControl(true, PlayerIndex);
		if ( CurrentlyFocusedChild != None )
		{
			CurrentlyFocusedChild.KillFocus(None, PlayerIndex);
		}
	}

//	`log(`location @ `showvar(IsFocused(PlayerIndex)) @ `showvar(GetFocusedControl(false, PlayerIndex),FocusedControl) @ `showvar(bResult),,'REMOVE_ME');
	return bResult;
}

/* == GearWarJournalPage_Base interface == */
/**
 * Assigns all member variables to the appropriate child widgets in this panel.
 */
function InitializePageReferences()
{
	lblPageTitle = UILabel(FindChild('lblPageTitle',true));
	Super.InitializePageReferences();
}

/**
 * Assigns the index for this page.
 *
 * @param	NewPageIndex	the index that is now associated with this page.
 */
function SetPageIndex( int NewPageIndex )
{
	local int TestPageIdx, AchievementIdx;

	Super.SetPageIndex(NewPageIndex);

	if ( IsValidPage() )
	{
		if ( PageIndex == 0 )
		{
			lblPageTitle.SetVisibility(false);

			AchievementsWJScene.lblMainTitle.SetVisibility(true);
			AchievementsWJScene.barOverallProgress.SetVisibility(true);
			AchievementsWJScene.lblTotalGamerPoints.SetVisibility(true);
		}
		else
		{
			if ( PageIndex % 2 == 0 )
			{
				AchievementsWJScene.lblMainTitle.SetVisibility(false);
				AchievementsWJScene.barOverallProgress.SetVisibility(false);
				AchievementsWJScene.lblTotalGamerPoints.SetVisibility(false);
			}

			lblPageTitle.SetVisibility(true);
		}

		while ( TestPageIdx < PageIndex )
		{
			AchievementIdx += GetMaxItemCount(TestPageIdx++);
		}
	}
	else
	{
		lblPageTitle.SetVisibility(false);
	}

	SynchronizeAchievements(AchievementIdx, GetMaxItemCount(PageIndex,true));

	bEnableSceneUpdateNotifications = true;
	OnPostSceneUpdate = PostSceneUpdate;
}

/**
 * Retrieve the maximum number of items that can fit on the page.
 *
 * @param	PageIdx	the index of the page to calculate the max items for.
 *
 * @return	the number of items that can fit on the page specified.
 */
function int GetMaxItemCount( int PageIdx, optional bool bRequireValidPageIndex )
{
	local float TopY, BottomY, SizeY;
	local int Result;

	if ( PageIdx >= 0 && (!bRequireValidPageIndex || IsValidPage(PageIdx)) )
	{
		if ( PageIdx == 0 )
		{
			TopY = AchievementsWJScene.lblTotalGamerPoints.GetPosition(UIFACE_Bottom, EVALPOS_PixelViewport) + 5;
		}
		else
		{
			TopY = lblPageTitle.GetPosition(UIFACE_Bottom, EVALPOS_PixelViewport);
		}

		BottomY = lblPageNumber.GetPosition(UIFACE_Top, EVALPOS_PixelViewport);
		SizeY = BottomY - TopY;

		Result = SizeY / ResolveUIExtent(AchievementsWJScene.AchievementPanelPrefab.OriginalHeight, GetScene());

		// make sure we're always showing at least 1 item so that calling code doesn't enter an infinite loop
		return Result;
	}

	`log(`location @ `showvar(PageIdx) @ "invalid page index - clearing all items",,'RON_DEBUG');
	return 0;
}

/**
 * Accessor for finding a GearAchievementPanel inside a UIPrefabInstance.
 */
static function GearAchievementPanel GetAchievementPanelFromPrefabInstance( UIPrefabInstance Inst )
{
	local int ChildIdx;
	local GearAchievementPanel Result;

	if ( Inst != None )
	{
		for ( ChildIdx = 0; ChildIdx < Inst.Children.Length; ChildIdx++ )
		{
			Result = GearAchievementPanel(Inst.Children[ChildIdx]);
			if ( Result != None )
			{
				break;
			}
		}
	}

	return Result;
}

/* === UIScreenObject interface === */
/**
 * Called once this screen object has been completely initialized, before it has activated its InitialState or called
 * Initialize on its children.  This event is only called the first time a widget is initialized.  If reparented, for
 * example, the widget would already be initialized so the Initialized event would not be called.
 */
event Initialized()
{
	Super.Initialized();

	AchievementsWJScene = GetOwnerAchievementsScene();
	`assert(AchievementsWJScene != None);
}

/**
 * Called immediately after a child has been added to this screen object.
 *
 * @param	WidgetOwner		the screen object that the NewChild was added as a child for
 * @param	NewChild		the widget that was added
 */
event AddedChild( UIScreenObject WidgetOwner, UIObject NewChild )
{
	local UIPrefabInstance InstChild;
	local GearAchievementPanel ChildPanel;

	Super.AddedChild(WidgetOwner, NewChild);

	InstChild = UIPrefabInstance(NewChild);
	if ( InstChild != None )
	{
		ChildPanel = GetAchievementPanelFromPrefabInstance(InstChild);
		if ( ChildPanel != None && AchievementPanels.Find(ChildPanel) == INDEX_NONE )
		{
			AchievementPanels[AchievementPanels.Length] = ChildPanel;
		}
	}

	//@todo ?
}

/**
 * Called immediately after a child has been removed from this screen object.
 *
 * @param	WidgetOwner		the screen object that the widget was removed from.
 * @param	OldChild		the widget that was removed
 * @param	ExclusionSet	used to indicate that multiple widgets are being removed in one batch; useful for preventing references
 *							between the widgets being removed from being severed.
 *							NOTE: If a value is specified, OldChild will ALWAYS be part of the ExclusionSet, since it is being removed.
 */
event RemovedChild( UIScreenObject WidgetOwner, UIObject OldChild, optional array<UIObject> ExclusionSet )
{
	local UIPrefabInstance InstChild;
	local GearAchievementPanel ChildPanel;
	local int PanelIdx;

	Super.RemovedChild(WidgetOwner, OldChild, ExclusionSet);

	InstChild = UIPrefabInstance(OldChild);
	if ( InstChild != None )
	{
		ChildPanel = GetAchievementPanelFromPrefabInstance(InstChild);
		PanelIdx = AchievementPanels.Find(ChildPanel);
		if ( PanelIdx != INDEX_NONE )
		{
			AchievementPanels.Remove(PanelIdx, 1);
		}
	}
}


DefaultProperties
{
}
