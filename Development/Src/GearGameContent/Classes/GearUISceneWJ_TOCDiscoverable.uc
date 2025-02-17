/**
 * Table of content for the discoverables area of the War Journal.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUISceneWJ_TOCDiscoverable extends GearUISceneWJ_TOCBase;

/** the label that contains the main "COLLECTIBLES" string */
var	transient				UILabel					lblMainTitle;

/**
 * The prefab used to create buttons for selecting a specific act from the list.
 */
var	transient	const		UIPrefab				CollectableTOC_ActPrefab;

/**
 * The prefab used to create buttons for selecting a specific chapter from the list.
 */
var	transient	const		UIPrefab				CollectableTOC_ChapterPrefab;

/** the size of the source prefabs used for creating the act and chapter toc entries */
var	transient				Vector2D				ActPrefabSize, ChapterPrefabSize;

var	transient	editinline	array<UIPrefabInstance>	GeneratedPrefabs;

/** the amount of spacing to add between rows of the TOC list */
var(Appearance)				UIScreenValue_Extent	RowSpacing;


/* == Delegates == */

/* == Natives == */

/* == Events == */

/* == UnrealScript == */
/**
 * Assigns all member variables to appropriate child widget from this scene.
 */
function InitializeWidgetReferences()
{
	Super.InitializeWidgetReferences();

	lblMainTitle = UILabel(LeftPage.FindChild('lblMainTitle'));
}

/**
 * Calculates the number of pixels required to display labels for this act and all of the chapters containing collectibles in the act.
 * Used to determine whether the next act should be put on the right page.
 */
function float GetNumPixelsRequiredForActTOC( GearActComboProvider ActProvider, float ActPrefabHeight, float ChapterPrefabHeight )
{
	local int ChapterIndex;
	local GearChapterComboProvider ChapterProvider;

	local float ItemPadding, Result;

	if ( ActProvider != None )
	{
		ItemPadding = ResolveUIExtent(RowSpacing, Self);
		for ( ChapterIndex = 0; ChapterIndex < ActProvider.ChapterProviders.Length; ChapterIndex++ )
		{
			ChapterProvider = ActProvider.ChapterProviders[ChapterIndex];
			if ( GearCampaignResourceProvider(ChapterProvider.StaticDataProvider).Collectables.Length > 0 )
			{
				// add ItemPadding to each chapter; technically we don't need padding for the last item, but we don't know which one
				// that will be so we handle that by not adding padding to the act's row
				Result += ChapterPrefabHeight + ItemPadding;
			}
		}

		if ( Result > 0 )
		{
			Result += ActPrefabHeight;
		}
	}

	return Result;
}

/**
 * Generates the table of contents for the collectables area of the war journal.  Each "element" in the list is instanced from a prefab
 * and all data store bindings, positions, and most attributes are set here as well.
 *
 * Safe to call multiple times.
 *
 * @param	bRecreateAllPrefabs		by default, any prefabs previously created are re-used; specify TRUE to clear these prefabs and create
 *									new instances for all prefabs.
 */
function GenerateTableOfContents( optional bool bRecreateAllPrefabs )
{
	local UIDataStore_DynamicResource DynamicResourceDS;
	local array<UIResourceCombinationProvider> ActProviders;
	local GearActComboProvider ActProvider;
	local GearChapterComboProvider ChapterProvider;
	local int ActIndex, ChapterIndex;
	local Vector2D PlacementLocation;
	local name PrefabName;
	local UIPrefabInstance PrefabInst, FirstPrefabInst, ActPrefabInst, ChapterPrefabInst;
	local UIObject CurrentDockTarget, ParentPage, InitialFocus;
	local float CurrentY, ClipY, RequiredHeight, ItemPadding;

	DynamicResourceDS = GetDynamicResourceDataStore(GetPlayerOwner());
	if ( DynamicResourceDS.GetResourceProviders('Acts', ActProviders) && ActProviders.Length > 0 )
	{
		if ( bRecreateAllPrefabs && GeneratedPrefabs.Length > 0 )
		{
			LeftPage.RemoveChildren(GeneratedPrefabs);
			RightPage.RemoveChildren(GeneratedPrefabs);
			GeneratedPrefabs.Length = 0;
		}

		ItemPadding = ResolveUIExtent(RowSpacing, Self);

		ActPrefabSize.X = ResolveUIExtent(CollectableTOC_ActPrefab.OriginalWidth, Self);
		ActPrefabSize.Y = ResolveUIExtent(CollectableTOC_ActPrefab.OriginalHeight, Self);

		ChapterPrefabSize.X = ResolveUIExtent(CollectableTOC_ChapterPrefab.OriginalWidth, Self);
		ChapterPrefabSize.Y = ResolveUIExtent(CollectableTOC_ChapterPrefab.OriginalHeight, Self);


		ParentPage = LeftPage;
		CurrentY = lblMainTitle.GetPosition(UIFACE_Bottom, EVALPOS_PixelViewport);
		ClipY = btnbarMain.GetPosition(UIFACE_Top, EVALPOS_PixelViewport) - 10;

		for ( ActIndex = 0; ActIndex < ActProviders.Length; ActIndex++ )
		{
			ActProvider = GearActComboProvider(ActProviders[ActIndex]);
			PrefabName = name("TOCAct_" $ int(ActProvider.GetActId()));
			PrefabInst = FindPrefabByName(PrefabName);
			if ( PrefabInst != None )
			{
				ActPrefabInst = PrefabInst;
				ActPrefabSize.X = ActPrefabInst.GetBounds(UIORIENT_Horizontal, EVALPOS_PixelViewport);
				ActPrefabSize.Y = ActPrefabInst.GetBounds(UIORIENT_Vertical, EVALPOS_PixelViewport);

				for ( ChapterIndex = 0; ChapterIndex < ActProvider.ChapterProviders.Length; ChapterIndex++ )
				{
					ChapterProvider = ActProvider.ChapterProviders[ChapterIndex];
					if ( GearCampaignResourceProvider(ChapterProvider.StaticDataProvider).Collectables.Length > 0 )
					{
						ChapterPrefabInst = FindPrefabByName(name("TOCChapter_" $ int(ChapterProvider.GetChapterId())));
						ChapterPrefabSize.X = ChapterPrefabInst.GetBounds(UIORIENT_Horizontal, EVALPOS_PixelViewport);
						ChapterPrefabSize.Y = ChapterPrefabInst.GetBounds(UIORIENT_Vertical, EVALPOS_PixelViewport);
						break;
					}
				}
			}

			RequiredHeight = GetNumPixelsRequiredForActTOC(ActProvider, ActPrefabSize.Y, ChapterPrefabSize.Y);
			if ( ParentPage == LeftPage && (ActIndex == 3 || CurrentY + RequiredHeight > ClipY) )	// force Act4 to begin on the second page
			{
				`assert(ParentPage == LeftPage);
				ParentPage = RightPage;
				CurrentDockTarget = None;
			}

			PlacementLocation.X = (ParentPage.GetPosition(UIFACE_Right, EVALPOS_PixelViewport) + 10) - ActPrefabSize.X;
			PlacementLocation.Y = CurrentY;

			if ( PrefabInst != None )
			{
				PrefabInst.SetPosition(PlacementLocation.X, UIFACE_Left, EVALPOS_PixelViewport);
				PrefabInst.SetPosition(PlacementLocation.Y, UIFACE_Top, EVALPOS_PixelViewport);
				PrefabInst.SetPosition(ActPrefabSize.X, UIFACE_Right, EVALPOS_PixelOwner);
				PrefabInst.SetPosition(ActPrefabSize.Y, UIFACE_Bottom, EVALPOS_PixelOwner);
			}
			else
			{
				PrefabInst = ParentPage.InstanceUIPrefab(CollectableTOC_ActPrefab, PrefabName, PlacementLocation);
				GeneratedPrefabs.AddItem(PrefabInst);
			}

			if ( CurrentDockTarget == None )
			{
				if ( ParentPage == LeftPage )
				{
					`assert(FirstPrefabInst == None);
					PrefabInst.SetDockTarget(UIFACE_Top, lblMainTitle, UIFACE_Bottom);
					FirstPrefabInst = PrefabInst;
				}
				else
				{
					`assert(FirstPrefabInst != None);
					PrefabInst.SetDockTarget(UIFACE_Top, FirstPrefabInst, UIFACE_Top);
				}

				PrefabInst.SetDockParameters(UIFACE_Right, ParentPage, UIFACE_Right, 10);
			}
			else
			{
				CurrentY += ItemPadding * 2;
				PrefabInst.SetDockParameters(UIFACE_Top, CurrentDockTarget, UIFACE_Bottom, ItemPadding * 2);
				PrefabInst.SetDockTarget(UIFACE_Right, CurrentDockTarget, UIFACE_Right);
			}

			InitializeActTOCPanel(PrefabInst, ActIndex, ActProvider);

			CurrentDockTarget = PrefabInst;
			CurrentY += ActPrefabSize.Y;

			for ( ChapterIndex = 0; ChapterIndex < ActProvider.ChapterProviders.Length; ChapterIndex++ )
			{
				ChapterProvider = ActProvider.ChapterProviders[ChapterIndex];
				if ( GearCampaignResourceProvider(ChapterProvider.StaticDataProvider).Collectables.Length > 0 )
				{
					CurrentY += ItemPadding;
					PlacementLocation.Y = CurrentY;

					PrefabName = name("TOCChapter_" $ int(ChapterProvider.GetChapterId()));
					PrefabInst = FindPrefabByName(PrefabName);
					if ( PrefabInst != None )
					{
						PrefabInst.SetPosition(PlacementLocation.X, UIFACE_Left, EVALPOS_PixelViewport);
						PrefabInst.SetPosition(PlacementLocation.Y, UIFACE_Top, EVALPOS_PixelViewport);
						PrefabInst.SetPosition(ChapterPrefabSize.X, UIFACE_Right, EVALPOS_PixelOwner);
						PrefabInst.SetPosition(ChapterPrefabSize.Y, UIFACE_Bottom, EVALPOS_PixelOwner);
					}
					else
					{
						PrefabInst = ParentPage.InstanceUIPrefab(CollectableTOC_ChapterPrefab, PrefabName, PlacementLocation);
						GeneratedPrefabs.AddItem(PrefabInst);
					}

					PrefabInst.SetDockParameters(UIFACE_Top, CurrentDockTarget, UIFACE_Bottom, ItemPadding);
					PrefabInst.SetDockTarget(UIFACE_Right, CurrentDockTarget, UIFACE_Right);

					InitializeChapterTOCPanel(PrefabInst, ActIndex, ChapterIndex, ChapterProvider);

					CurrentDockTarget = PrefabInst;
					CurrentY += ChapterPrefabSize.Y;
				}
			}
		}
	}

	if ( FirstPrefabInst != None && bRecreateAllPrefabs )
	{
		InitialFocus = FirstPrefabInst.FindChild('btnSelectAct');
		InitialFocus.SetFocus(None, GetPlayerOwnerIndex());
	}
}

/**
 * Initializes the datastore bindings for all child widgets of a single prefab instance associated with a campaign act.
 */
function InitializeActTOCPanel( UIPrefabInstance PrefabInst, int ActIndex, GearActComboProvider ActProvider )
{
	local UILabelButton btnSelectProvider;
	local UILabel lblAttractIcon, lblCollectibleProgress;
	local UICheckbox chkCollectibles;
	local string MarkupPrefix;

	local GearCollectableComboProvider CollectableProvider;
	local GearProfileSettings Profile;
	local int CollectableIndex;
	local bool bShowAttractIcon;

	btnSelectProvider		= UILabelButton(PrefabInst.FindChild('btnSelectAct', true));

	chkCollectibles			= UICheckBox(btnSelectProvider.FindChild('chkCollectibles',true));
	lblAttractIcon			= UILabel(btnSelectProvider.FindChild('lblNew', true));
	lblCollectibleProgress	= UILabel(btnSelectProvider.FindChild('lblCollectibleProgress',true));

	btnSelectProvider.OnClicked = TOCClicked;

	MarkupPrefix = "<DynamicGameResource:Acts;" $ ActIndex $ ".";
	btnSelectProvider.SetDataStoreBinding(MarkupPrefix $ "DisplayName>");
	chkCollectibles.SetDataStoreBinding(MarkupPrefix $ "HasFoundAllCollectables>");
	lblCollectibleProgress.SetDataStoreBinding(MarkupPrefix $ "CollectablesFound>/" $ MarkupPrefix $ "TotalCollectables>");

	Profile = GetPlayerProfile(GetPlayerOwnerIndex());
	if ( Profile != None )
	{
		for ( CollectableIndex = 0; CollectableIndex < ActProvider.DynamicCollectableProviders.Length; CollectableIndex++ )
		{
			CollectableProvider = ActProvider.DynamicCollectableProviders[CollectableIndex];
			if ( CollectableProvider != None && Profile.IsDiscoverableMarkedForAttract(CollectableProvider.GetCollectableId()) )
			{
				bShowAttractIcon = true;
				break;
			}
		}
	}

	lblAttractIcon.SetVisibility(bShowAttractIcon);
}

/**
 * Initializes the datastore bindings for all child widgets of a single prefab instance associated with a campaign chapter.
 */
function InitializeChapterTOCPanel( UIPrefabInstance PrefabInst, int ActIndex, int ChapterIndex, GearChapterComboProvider ChapterProvider )
{
	local UILabelButton btnSelectProvider;
	local UILabel lblAttractIcon, lblCollectibleProgress;
	local UICheckbox chkCollectibles;
	local string MarkupPrefix;

	local GearCollectableComboProvider CollectableProvider;
	local GearProfileSettings Profile;
	local int CollectableIndex;
	local bool bShowAttractIcon;

	btnSelectProvider		= UILabelButton(PrefabInst.FindChild('btnSelectChapter', true));

	chkCollectibles			= UICheckBox(btnSelectProvider.FindChild('chkCollectibles',true));
	lblAttractIcon			= UILabel(btnSelectProvider.FindChild('lblNew', true));
	lblCollectibleProgress	= UILabel(btnSelectProvider.FindChild('lblCollectibleProgress',true));

	btnSelectProvider.OnClicked = TOCClicked;

	MarkupPrefix = "<DynamicGameResource:Acts;" $ ActIndex $ ".ChapterProviders;" $ ChapterIndex $ ".";
	btnSelectProvider.SetDataStoreBinding(MarkupPrefix $ "DisplayName>");
	chkCollectibles.SetDataStoreBinding(MarkupPrefix $ "HasFoundAllCollectables>");
	lblCollectibleProgress.SetDataStoreBinding(MarkupPrefix $ "CollectablesFound>/" $ MarkupPrefix $ "TotalCollectables>");

	Profile = GetPlayerProfile(GetPlayerOwnerIndex());
	if ( Profile != None )
	{
		for ( CollectableIndex = 0; CollectableIndex < ChapterProvider.DynamicCollectableProviders.Length; CollectableIndex++ )
		{
			CollectableProvider = ChapterProvider.DynamicCollectableProviders[CollectableIndex];
			if ( CollectableProvider != None && Profile.IsDiscoverableMarkedForAttract(CollectableProvider.GetCollectableId()) )
			{
				bShowAttractIcon = true;
				break;
			}
		}
	}

	lblAttractIcon.SetVisibility(bShowAttractIcon);
}

/**
 * Changes the states for the non-interactive widgets in the prefab instance so that their colors contrast against the background.
 */
function TogglePrefabChildStates( UILabelButton TOCButton, int PlayerIndex, bool bIsSelected )
{
	local UILabel lblAttractIcon, lblCollectibleProgress, lblDottedLines;
	local UICheckbox chkCollectibles;

	chkCollectibles			= UICheckBox(TOCButton.FindChild('chkCollectibles',true));
	lblAttractIcon			= UILabel(TOCButton.FindChild('lblNew', true));
	lblCollectibleProgress	= UILabel(TOCButton.FindChild('lblCollectibleProgress',true));
	lblDottedLines			= UILabel(TOCButton.FindChild('lblDot01', true));

	if ( bIsSelected )
	{
		// for the checkbox, the pressed state has the correct color
		chkCollectibles.ActivateStateByClass(class'UIState_Pressed', PlayerIndex);

		// for the labels, the disabled state contains the correct color
		lblAttractIcon.SetEnabled(false, PlayerIndex);
		lblCollectibleProgress.SetEnabled(false, PlayerIndex);
		lblDottedLines.SetEnabled(false, PlayerIndex);
	}
	else
	{
		// remove pressed state from checkbox
		chkCollectibles.DeactivateStateByClass(class'UIState_Pressed', PlayerIndex);

		// labels need to go to enabled state
		lblAttractIcon.SetEnabled(true, PlayerIndex);
		lblCollectibleProgress.SetEnabled(true, PlayerIndex);
		lblDottedLines.SetEnabled(true, PlayerIndex);
	}
}

/**
 * Calculates the page index of the first collectable in a particular act.
 *
 * @param	DesiredActId	the id of the act to get the page index for.
 *
 * @return	the index of the page that contains the first collectable in the specified act, or INDEX_NONE if the page index
 *			couldn't be calculated.
 */
static function int CalculatePageIndexForChapter( EChapterPoint DesiredChapterId )
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
//			`log(`showvar(ActIndex) @ `showvar(ActPRovider.ChapterProviders.Length));
			for ( ChapterIndex = 0; ChapterIndex < ActProvider.ChapterProviders.Length; ChapterIndex++ )
			{
				ChapterProvider = ActProvider.ChapterProviders[ChapterIndex];
//				`log(`showvar(ChapterIndex) @ `showenum(EChapterPoint,ChapterProvider.ChapterType,ProviderChapter) @ `showenum(EChapterPoint,DesiredChapterId) @ `showvar(desiredChapterId) @ `showvar(ChapterProvider.collectables.length));
				if ( ChapterProvider.ChapterType == DesiredChapterId )
				{
					Result = PageIdx;
					break;
				}

				RemainingCollectablesInChapter = ChapterProvider.Collectables.Length;
				while ( RemainingCollectablesInChapter > 0 )
				{
					RemainingCollectablesInChapter -= Clamp(RemainingCollectablesInChapter, 0, `MAX_COLLECTABLES_PER_PAGE);
					PageIdx++;
				}


//				`log(`showvar(PageIdx));
			}

			if ( Result != INDEX_NONE )
			{
				break;
			}
		}
	}
	else
	{
		`warn("No data providers found for campaign acts using tag Acts");
	}

	return Result;
}

/**
 * Wrapper for finding a reference to a specific prefab instance.
 */
function UIPrefabInstance FindPrefabByName( name PrefabName )
{
	local int i;
	local UIPrefabInstance Result;

	for ( i = 0; i < GeneratedPrefabs.Length; i++ )
	{
		if ( GeneratedPrefabs[i].Name == PrefabName )
		{
			Result = GeneratedPrefabs[i];
			break;
		}
	}

	return Result;
}

/* == Delegate handlers == */
/**
 * Handler for the TOC buttons' OnClicked delegate.  Opens the appropriate page of the collectables war journal area.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool TOCClicked(UIScreenObject EventObject, int PlayerIndex)
{
	local string IndexString, PrefabName;
	local UILabelButton ButtonSender;
	local UIPrefabInstance PrefInst;
	local EGearAct SelectedAct;
	local EChapterPoint SelectedChapter;
	local int PageIdx;
	local UIScene SceneInstance;
	local GearUISceneWJ_Discoverable WarJournalScene;

	SelectedAct = GEARACT_MAX;
	SelectedChapter = CHAP_MAX;

	ButtonSender = UILabelButton(EventObject);
	if ( ButtonSender != None )
	{
		PrefInst = UIPrefabInstance(ButtonSender.GetOwner());
		PrefabName = string(PrefInst.Name);
		IndexString = Split(PrefabName, "TOCAct_", true);
		if ( IndexString != PrefabName )
		{
			SelectedAct = EGearAct(byte(IndexString));
			PageIdx = class'GearUISceneWJ_Discoverable'.static.CalculatePageIndexForAct(SelectedAct);
		}
		else
		{
			IndexString = Split(PrefabName, "TOCChapter_", true);
			if ( IndexString != PrefabName )
			{
				SelectedChapter = EChapterPoint(byte(IndexString));
				PageIdx = CalculatePageIndexForChapter(SelectedChapter);
			}
		}
	}

//	`log(`location @ `showobj(EventObject.GetParent(),Sender) @ `showvar(SelectedAct) @ `showvar(SelectedChapter) @ `showvar(IndexString) @ `showvar(PageIdx));
	if ( PageIdx != INDEX_NONE )
	{
		SceneInstance = OpenScene(NextChapterScene);
		if ( SceneInstance != None )
		{
			WarJournalScene = GearUISceneWJ_Discoverable(SceneInstance);
			if ( WarJournalScene != None )
			{
				WarJournalScene.SetCurrentPage(PageIdx);
			}
		}
	}

	return true;
}


/* === GearUIScene_Base interface === */
/**
 * Called when a new UIState becomes the widget's currently active state, after all activation logic has occurred.
 *
 * @param	Sender					the widget that changed states.
 * @param	PlayerIndex				the index [into the GamePlayers array] for the player that activated this state.
 * @param	NewlyActiveState		the state that is now active
 * @param	PreviouslyActiveState	the state that used the be the widget's currently active state.
 */
function OnStateChanged( UIScreenObject Sender, int PlayerIndex, UIState NewlyActiveState, UIState PreviouslyActiveState )
{
	local UILabelButton TOCButton;
	local bool bGainingFocus, bLosingFocus;

	Super.OnStateChanged(Sender, PlayerIndex, NewlyActiveState, PreviouslyActiveState);

	TOCButton = UILabelButton(Sender);
	if ( TOCButton != None && UIPrefabInstance(TOCButton.GetParent()) != None )
	{
		bGainingFocus = UIState_Focused(NewlyActiveState) != None;
		bLosingFocus = !bGainingFocus && UIState_Focused(PreviouslyActiveState) != None;
		if ( bGainingFocus || bLosingFocus )
		{
			TogglePrefabChildStates(TOCButton, PlayerIndex, bGainingFocus);
		}
	}
}


/* === UIScene interface === */
/**
 * Handler for the scene's OnSceneActivated delegate.  Called after scene focus has been initialized, or when the scene becomes active as
 * the result of another scene closing.
 *
 * @param	ActivatedScene			the scene that was activated
 * @param	bInitialActivation		TRUE if this is the first time this scene is being activated; FALSE if this scene has become active
 *									as a result of closing another scene or manually moving this scene in the stack.
 */
function SceneActivationComplete( UIScene ActivatedScene, bool bInitialActivation )
{
	Super.SceneActivationComplete(ActivatedScene, bInitialActivation);

	if ( ActivatedScene == Self )
	{
		GenerateTableOfContents(bInitialActivation);
		PlayUISound('G2UI_JournalNavigateCue');
	}
}

/* === UIScreenObject interface === */
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
	local UIPrefabInstance PrefInst;

	Super.RemovedChild(WidgetOwner, OldChild, ExclusionSet);

	PrefInst = FindPrefabByName(OldChild.Name);
	if ( PrefInst != None )
	{
		GeneratedPrefabs.RemoveItem(PrefInst);
	}
}

DefaultProperties
{
	PreviousChapterScene=None //GearUISceneWJ_Base'UI_Scenes_WarJournal.TOC.WarJournalMainTOC'
	NextChapterScene=GearUISceneWJ_Base'UI_Scenes_WarJournal.CollectablesJournal'

	CollectableTOC_ActPrefab=UIPrefab'UI_Scenes_Prefabs.TOCActPrefab'
	CollectableTOC_ChapterPrefab=UIPrefab'UI_Scenes_Prefabs.TOCChapterPrefab'

	RowSpacing=(Value=3,Orientation=UIORIENT_Vertical,ScaleType=UIEXTENTEVAL_Pixels)
}
