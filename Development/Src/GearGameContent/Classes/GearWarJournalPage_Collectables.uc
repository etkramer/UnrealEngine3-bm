/**
 * Panel for displaying a page of the War Journal collectables section.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearWarJournalPage_Collectables extends GearWarJournalPage_ChapterBase;

/** the scene that contains this panel */
var	transient				GearUISceneWJ_Discoverable		CollectablesWJScene;

/** the label that display the "COLLECTABLES" string */
var	transient				UILabel							lblPageTitle;
var	transient				UILabel							lblActName;
var	transient				UILabel							lblChapterName;
var	transient				UILabel							lblNumCollectablesFound;

/** the array of panels that display information on each collectable; each panel displays data for a single collectable */
var	transient				array<GearCollectablePanel>		CollectablePanels;

/** the id of the act and chapter being displayed on this page */
var	transient				EGearAct						PageActId;
var	transient				EChapterPoint					PageChapterId;

/* == Delegates == */

/* == Natives == */

/* == Events == */

/* == UnrealScript == */
/**
 * Wrapper for getting a reference to the owning war journal collectables scene.
 */
final function GearUISceneWJ_Discoverable GetOwnerCollectablesScene()
{
	return GearUISceneWJ_Discoverable(GetOwnerWarJournalScene());
}

/**
 * Wrapper for checking whether any of the collectibles on this page are unlocked
 */
final function bool HasUnlockedCollectables()
{
	local int PanelIdx, PlayerIndex;
	local GearCollectablePanel CurrentPanel;
	local UIPrefabInstance Inst;
	local bool bResult;

	PlayerIndex = GetPlayerOwnerIndex();
	for ( PanelIdx = 0; PanelIdx < CollectablePanels.Length; PanelIdx++ )
	{
		CurrentPanel = CollectablePanels[PanelIdx];
		Inst = UIPrefabInstance(CurrentPanel.GetOwner());
		if ( Inst.IsEnabled(PlayerIndex) )
		{
			bResult = true;
			break;
		}
	}

	return bResult;
}

/**
 * Synchronizes the number of collectable panels with the number of collectables being displayed for the current page.  If more panels
 * are needed, instances new panels from the panel prefab.  If too many panels exist, removes those panels.
 *
 * @param	TargetPanel		the panel that will contain the achievement panels.
 * @param	StartIndex		the index of the first achievement to display on that page.
 * @param	Count			the number of achievements that should be displayed on this page.
 */
function SynchronizeCollectables( int StartIndex, int Count )
{
	local int PanelIdx, CollectableIdx, PlayerIndex, VisiblePanels;
	local UIPrefab SourcePrefab;
	local UIPrefabInstance Inst, PreviousInst;
	local vector2D PanelPosition;
	local float Y;
	local GearCollectablePanel CurrentPanel, PreviousPanel;

	Count = Clamp(Count, 0, CollectablesWJScene.CollectableCount - StartIndex);

	//local float ButtonbarSize, HorzGutterPixels, X;
	//ButtonBarSize = CollectablesWJScene.btnbarMain.GetBounds(UIORIENT_Horizontal,EVALPOS_PixelViewport);
	//HorzGutterPixels = (ButtonbarSize - PanelPosition.X) * 0.5f;

	//X = GetPosition(UIFACE_Left,EVALPOS_PixelViewport) + (GetBounds(UIORIENT_Horizontal,EVALPOS_PixelViewport) * 0.5 - ButtonBarSize * 0.5) + HorzGutterPixels;
	Y = lblChapterName.GetPosition(UIFACE_Bottom, EVALPOS_PixelViewport);

	// if we have more panels than we need, remove the extra ones first
	PlayerIndex = GetPlayerOwnerIndex();
	VisiblePanels = CollectablePanels.Length;
	while ( VisiblePanels > Count )
	{
		PanelIdx = CollectablePanels.Length - 1;
		CurrentPanel = CollectablePanels[PanelIdx];
		if ( CurrentPanel != None )
		{
			Inst = UIPrefabInstance(CurrentPanel.GetOwner());
			if ( Inst != None )
			{
				if ( CurrentPanel.IsFocused(PlayerIndex) )
				{
					CurrentPanel.KillFocus(None, PlayerIndex);
				}
			}
		}
		else
		{
			CollectablePanels.Remove(PanelIdx,1);
		}

		VisiblePanels--;
		CurrentPanel = None;
		Inst = None;
	}

	for ( PanelIdx = 0; PanelIdx < Max(Count, CollectablePanels.Length); PanelIdx++ )
	{
		PreviousPanel = CurrentPanel;
		PreviousInst = Inst;

		CurrentPanel = None;
		Inst = None;

		SourcePrefab = CollectablesWJScene.CollectablePanelPrefab[PanelIdx % 2];
		PanelPosition.X = ResolveUIExtent(SourcePrefab.OriginalWidth, GetScene());
		PanelPosition.Y = ResolveUIExtent(SourcePrefab.OriginalHeight, GetScene());

		if ( PanelIdx < CollectablePanels.Length )
		{
			CurrentPanel = CollectablePanels[PanelIdx];
			if ( CurrentPanel == None )
			{
				CollectablePanels.Remove(PanelIdx--,1);
				continue;
			}

			Inst = UIPrefabInstance(CurrentPanel.GetOwner());
		}
		else
		{
			Inst = InstanceUIPrefab(CollectablesWJScene.CollectablePanelPrefab[PanelIdx % 2], name("CollectableUIPI_" $ PanelIdx), PanelPosition);
			if ( Inst != None )
			{
				// set the vertical position of the panel for this discoverable
				Inst.SetPosition(Y, UIFACE_Top, EVALPOS_PixelViewport);
				CurrentPanel = GetCollectablePanelFromPrefabInstance(Inst);
			}
		}

		Inst.SetVisibility(PanelIdx < Count);
		CollectableIdx = StartIndex + PanelIdx;
		if ( CurrentPanel != None )
		{
			CurrentPanel.SetCollectableIndex(CollectableIdx);

			// ensure the discoverable's panel is docked to this panel
			Inst.SetDockTarget(UIFACE_Left, Self, UIFACE_Left);
			Inst.SetDockTarget(UIFACE_Right, Self, UIFACE_Right);

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

			Y += PanelPosition.Y;
			Inst.SetEnabled(PanelIdx < Count && HasCollectableBeenFound(PlayerIndex, CollectableIdx), PlayerIndex);
		}
	}

	RequestSceneUpdate(true, true, true, false);
}

/* == SequenceAction handlers == */

/* == Delegate handlers == */
/**
 * Called immediately after the scene performs an update.  Only called if assigned and the value of
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
	local GearCollectablePanel InitiallyFocusedPanel;
	local int Idx, PlayerIndex;
	local bool bResult;

	PlayerIndex = GetPlayerOwnerIndex();
//	`log(`location @ `showvar(IsFocused()) @ `showvar(GetFocusedControl(),FocusedControl) @ `showvar(OnPostSceneUpdate) @ `showvar(bEnableSceneUpdateNotifications),,'REMOVE_ME');

	InitiallyFocusedPI = FindChild('CollectableUIPI_0',true);
	while ( InitiallyFocusedPI != None && InitiallyFocusedPI.IsDisabled(PlayerIndex,false) )
	{
		InitiallyFocusedPI = FindChild(name("CollectableUIPI_" $ ++Idx), true);
	}

	if ( InitiallyFocusedPI != None )
	{
		InitiallyFocusedPanel = GearCollectablePanel(InitiallyFocusedPI.FindChild('pnlCollectable'));
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

/**
 * Wrapper for checking whether a player has found a specific collectable.
 *
 * @param	PlayerIndex		the index of the player whose profile should be checked
 * @param	DiscoverableIdx	the index [into the full list of collectables] for the collectable to check.
 *
 * @return	TRUE if the specified collectable is unlocked in the player's profile.
 */
function bool HasCollectableBeenFound( int PlayerIndex, int DiscoverableIdx )
{
	local bool bResult;
	local UIDataStore_DynamicResource GameResourceDS;
	local UIProviderScriptFieldValue FieldValue;

	GameResourceDS = class'GearUIScene_Base'.static.GetDynamicResourceDataStore(GetPlayerOwner(PlayerIndex));
	if ( GameResourceDS.GetProviderFieldValue('Collectables', 'HasBeenFound', DiscoverableIdx, FieldValue) )
	{
		bResult = bool(FieldValue.StringValue);
	}
	else
	{
		`log("HasCollectableBeenFound - couldn't get field value for 'HasCollectableBeenFound' using CollectableIndex" @ DiscoverableIdx);
	}

	return bResult;
}

/**
 * Handler for the OnDisplayDetails delegate of each individual collectable's panel.  Called when the user requests the details for a
 * collectable.
 *
 * @param	PlayerIndex		index of the player that generated the event.
 * @param	DiscoverableIdx	the index into the list of collectables for the collectable being displayed by this panel.
 */
function DisplayCollectableDetails( int PlayerIndex, int DiscoverableIdx )
{
	local GearUISceneWJ_Discoverable DiscoverableScene;
	local GearUISceneWJ_DiscoverableDetails DetailsSceneInstance;

	if ( PlayerIndex != INDEX_NONE && DiscoverableIdx != INDEX_NONE )
	{
		if ( HasCollectableBeenFound(PlayerIndex, Discoverableidx) )
		{
			DiscoverableScene = GetOwnerCollectablesScene();
			if ( DiscoverableScene != None && DiscoverableScene.DetailsSceneResource != None )
			{
				DetailsSceneInstance = GearUISceneWJ_DiscoverableDetails(DiscoverableScene.OpenScene(DiscoverableScene.DetailsSceneResource));
				if ( DetailsSceneInstance != None )
				{
					DetailsSceneInstance.SetCollectableIndex(DiscoverableIdx);
				}
			}
		}
		else
		{
			PlayUISound('GenericError', 0);
		}
	}
}

/* == GearWarJournalPage_Base interface == */
/**
 * Assigns all member variables to the appropriate child widgets in this panel.
 */
function InitializePageReferences()
{
	lblPageTitle = UILabel(FindChild('lblMainTitle',true));
	lblActName = UILabel(FindChild('lblActName',true));
	lblChapterName = UILabel(FindChild('lblChapterName',true));
	lblNumCollectablesFound = UILabel(FindChild('lblNumCollected',true));

	Super.InitializePageReferences();
}

/**
 * Returns the data provider for the chapter which contains the collectable at the specified index
 *
 * @param	CollectableIndex	an index [into the array of all collectables] for the chapter to find
 */
function GearCampaignChapterData GetChapterProvider( int CollectableIndex )
{
	local EChapterPoint ChapterId;

	ChapterId = class'GearUISceneWJ_Discoverable'.static.GetChapterIdFromCollectableIndex(CollectableIndex);
	return class'GearUIDataStore_GameResource'.static.GetChapterDataProvider(ChapterId);
}

/**
 * Assigns the index for this page.
 *
 * @param	NewPageIndex	the index that is now associated with this page.
 */
function SetPageIndex( int NewPageIndex )
{
	local int TestPageIdx, CollectableIdx;

	Super.SetPageIndex(NewPageIndex);

	if ( IsValidPage() )
	{
		lblPageTitle.SetVisibility(true);

		// determine which collectable should appear be the first item of the left page, based on the current page index
		while ( TestPageIdx < PageIndex )
		{
			CollectableIdx += GetMaxItemCount(TestPageIdx++);
		}

		// update all the labels to match the values for this collectable item
		UpdateHeaderInfo(CollectableIdx);
	}
	else
	{
		lblPageTitle.SetVisibility(false);

		lblActName.SetDataStoreBinding("");
		lblChapterName.SetDataStoreBinding("");
		lblNumCollectablesFound.SetDataStoreBinding("");
	}

	SynchronizeCollectables(CollectableIdx, GetMaxItemCount(PageIndex,true));
}

/**
 * Updates the labels displaying the current chapter, act, number of discoverables unlocked, etc. for the current page index.
 *
 * @param	CollectableIndex	the index [into the complete list of collectables] for the collectable that appears as the first item
 *								in this panel.
 */
function UpdateHeaderInfo( int CollectableIndex )
{
	local int ActProviderIndex, ChapterProviderIndex, ActChapterProviderIndex;
	local string ChapterAccessString;

	PageActId = class'GearUISceneWJ_Discoverable'.static.GetActIdFromCollectableIndex(CollectableIndex);
	ActProviderIndex = class'GearUIDataStore_GameResource'.static.GetActProviderIndexFromActId(PageActId);

	PageChapterId = class'GearUISceneWJ_Discoverable'.static.GetChapterIdFromCollectableIndex(CollectableIndex);
	ChapterProviderIndex = class'GearUIDataStore_GameResource'.static.GetChapterProviderIndexFromChapterId(PageChapterId);
	ActChapterProviderIndex = class'GearUIDataStore_GameResource'.static.GetActChapterProviderIndexFromChapterId(PageChapterId);

//	`log(`location @ `showenum(EGearAct,PageActId) @ `showvar(ActProviderIndex) @ `showenum(EChapterPoint,PageChapterId) @ `showvar(ChapterProviderIndex),,'RON_DEBUG');

	lblActName.SetDataStoreBinding("<DynamicGameResource:Acts;" $ ActProviderIndex $ ".DisplayName>");
	lblChapterName.SetDataStoreBinding("<DynamicGameResource:Chapters;" $ ChapterProviderIndex $ ".DisplayName>");

	ChapterAccessString = "<DynamicGameResource:Acts;" $ ActProviderIndex $ ".ChapterProviders;" $ ActChapterProviderIndex;
	lblNumCollectablesFound.SetDataStoreBinding(ChapterAccessString $ ".CollectablesFound> / " $ ChapterAccessString $ ".TotalCollectables>");
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
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> ActProviders;
	local GearCampaignActData ActProvider;
	local GearCampaignChapterData ChapterProvider;
	local int ActIndex, ChapterIndex, CurrentPage, NumCollectablesOnCurrentPage, RemainingCollectablesInChapter;

	if ( PageIdx >= 0 && (!bRequireValidPageIndex || IsValidPage(PageIdx)) )
	{
		GameResourceDS = CollectablesWJScene.GetGameResourceDataStore();
		if ( GameResourceDS.GetResourceProviders('Acts', ActProviders) && ActProviders.Length > 0 )
		{
			// since each chapter can have a different number of collectables, we must check each chapter
			// Start at the beginning, accumulating the number of collectables until we have to move to the next page
			// once we've hit the target page, the return value is the number of collectables that can be on that page
			// which might be less than the maximum if that particular chapter doesn't have that many collectables.
			for ( ActIndex = 0; ActIndex < ActProviders.Length; ActIndex++ )
			{
				ActProvider = GearCampaignActData(ActProviders[ActIndex]);
				for ( ChapterIndex = 0; ChapterIndex < ActProvider.ChapterProviders.Length; ChapterIndex++ )
				{
					ChapterProvider = ActProvider.ChapterProviders[ChapterIndex];

					RemainingCollectablesInChapter = ChapterProvider.Collectables.Length;
					while ( RemainingCollectablesInChapter > 0 )
					{
						NumCollectablesOnCurrentPage = Clamp(RemainingCollectablesInChapter, 0, `MAX_COLLECTABLES_PER_PAGE);
						RemainingCollectablesInChapter -= NumCollectablesOnCurrentPage;

						if ( PageIdx == CurrentPage++ )
						{
							break;
						}
					}

					if ( CurrentPage > PageIdx )
					{
						break;
					}
				}

				if ( CurrentPage > PageIdx )
				{
					break;
				}
			}

		}
		else
		{
			`warn("No data providers found for campaign acts using tag Acts");
		}
	}

	return NumCollectablesOnCurrentPage;
}

/**
 * Accessor for finding a GearCollectablePanel inside a UIPrefabInstance.
 */
static function GearCollectablePanel GetCollectablePanelFromPrefabInstance( UIPrefabInstance Inst )
{
	local int ChildIdx;
	local GearCollectablePanel Result;

	if ( Inst != None )
	{
		for ( ChildIdx = 0; ChildIdx < Inst.Children.Length; ChildIdx++ )
		{
			Result = GearCollectablePanel(Inst.Children[ChildIdx]);
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

	CollectablesWJScene = GetOwnerCollectablesScene();
	`assert(CollectablesWJScene != None);
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
	local GearCollectablePanel ChildPanel;

	Super.AddedChild(WidgetOwner, NewChild);

	InstChild = UIPrefabInstance(NewChild);
	if ( InstChild != None )
	{
		ChildPanel = GetCollectablePanelFromPrefabInstance(InstChild);
		if ( ChildPanel != None && CollectablePanels.Find(ChildPanel) == INDEX_NONE )
		{
			CollectablePanels[CollectablePanels.Length] = ChildPanel;
			ChildPanel.OnDisplayDetails = DisplayCollectableDetails;
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
	local GearCollectablePanel ChildPanel;
	local int PanelIdx;

	Super.RemovedChild(WidgetOwner, OldChild, ExclusionSet);

	InstChild = UIPrefabInstance(OldChild);
	if ( InstChild != None )
	{
		ChildPanel = GetCollectablePanelFromPrefabInstance(InstChild);
		PanelIdx = CollectablePanels.Find(ChildPanel);
		if ( PanelIdx != INDEX_NONE )
		{
			ChildPanel.OnDisplayDetails = None;
			CollectablePanels.Remove(PanelIdx, 1);
		}
	}
}


DefaultProperties
{
	// don't enable this or it won't kill focus properly.....
	//bNeverFocus=true
	PageActId=GEARACT_MAX
	PageChapterId=CHAP_MAX
}

