/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "BusyCursor.h"
#include "EnginePrefabClasses.h"
#include "Properties.h"
#include "ReferencedAssetsToolbar.h"
#include "ReferencedAssetsBrowser.h"

#define _GB_SCROLL_SPEED	50

/**
 * Menu bar for the Referenced Assets Browser.
 */
class WxMBReferencedAssetsBrowser : public wxMenuBar
{
public:
	WxMBReferencedAssetsBrowser::WxMBReferencedAssetsBrowser()
	{
		// View menu
		wxMenu* ViewMenu = new wxMenu();
		ViewMenu->Append( IDM_RefreshBrowser, *LocalizeUnrealEd("RefreshWithHotkey"), TEXT("") );
		Append( ViewMenu, *LocalizeUnrealEd("View") );
		WxBrowser::AddDockingMenu( this );
	}
};

USelection* WxReferencedAssetsBrowser::Selection = NULL;

BEGIN_EVENT_TABLE(WxReferencedAssetsBrowser,WxBrowser)
	EVT_LIST_ITEM_SELECTED( ID_LIST_VIEW, WxReferencedAssetsBrowser::OnSelectListItem )
	EVT_LIST_ITEM_DESELECTED( ID_LIST_VIEW, WxReferencedAssetsBrowser::OnDeselectListItem )
	EVT_LIST_COL_CLICK( ID_LIST_VIEW, WxReferencedAssetsBrowser::OnListColumnClicked )

	EVT_TREE_SEL_CHANGED( ID_REFERENCE_GRAPH_TREE, WxReferencedAssetsBrowser::OnTreeItemSelected )
	EVT_TREE_ITEM_ACTIVATED( ID_REFERENCE_GRAPH_TREE, WxReferencedAssetsBrowser::OnTreeItemActivated )

	EVT_MENU(IDM_RefreshBrowser,WxReferencedAssetsBrowser::OnRefresh)
	EVT_SIZE(WxReferencedAssetsBrowser::OnSize)
	EVT_SCROLL(WxReferencedAssetsBrowser::OnScroll)
	EVT_UPDATE_UI(ID_VIEW_LIST,WxReferencedAssetsBrowser::UI_ViewMode)
	EVT_UPDATE_UI(ID_VIEW_THUMBNAIL,WxReferencedAssetsBrowser::UI_ViewMode)
	EVT_UPDATE_UI_RANGE(IDM_REFERENCEDEPTH_START, IDM_REFERENCEDEPTH_END, WxReferencedAssetsBrowser::UI_DepthMode)
	EVT_UPDATE_UI(IDM_SHOWDEFAULTREFS, WxReferencedAssetsBrowser::UI_IncludeDefaultRefs)
	EVT_UPDATE_UI(IDM_SHOWSCRIPTREFS, WxReferencedAssetsBrowser::UI_IncludeScriptRefs)
	EVT_UPDATE_UI(IDM_GROUPBYCLASS, WxReferencedAssetsBrowser::UI_GroupByClass)

	EVT_MENU( ID_VIEW_LIST, WxReferencedAssetsBrowser::OnViewMode )
	EVT_MENU( ID_VIEW_THUMBNAIL, WxReferencedAssetsBrowser::OnViewMode )

	EVT_MENU_RANGE(IDM_REFERENCEDEPTH_START, IDM_REFERENCEDEPTH_END, WxReferencedAssetsBrowser::OnDepthMode)
	EVT_MENU( IDM_SHOWDEFAULTREFS, WxReferencedAssetsBrowser::OnIncludeDefaultRefs )
	EVT_MENU( IDM_SHOWSCRIPTREFS, WxReferencedAssetsBrowser::OnIncludeScriptRefs )
	EVT_MENU( IDM_GROUPBYCLASS, WxReferencedAssetsBrowser::OnGroupByClass )

	EVT_TEXT( ID_CUSTOM_REFERENCE_DEPTH, WxReferencedAssetsBrowser::OnCustomDepthChanged )
END_EVENT_TABLE()

/**
 * Factorizes out the creation of a new property window frame for the
 * UGenericBrowserType::ShowOBjectProperties(...) family of methods.
 */
static inline void ReferencedAssetsBrowser_CreateNewPropertyWindowFrame()
{
	if(!GApp->ObjectPropertyWindow)
	{
		GApp->ObjectPropertyWindow = new WxPropertyWindowFrame;
		GApp->ObjectPropertyWindow->Create( GApp->EditorFrame, -1, TRUE, GUnrealEd );
		GApp->ObjectPropertyWindow->SetSize( 64,64, 350,600 );
	}
}

/**
 * Generic implementation for opening a property window for an object.
 */
static void ShowObjectProperties( UObject* InObject )
{
	ReferencedAssetsBrowser_CreateNewPropertyWindowFrame();
	GApp->ObjectPropertyWindow->SetObject( InObject, TRUE, TRUE, TRUE );
	GApp->ObjectPropertyWindow->Show();
}

/**
 * Constructor. Builds the list of items to ignore
 */
 WxReferencedAssetsBrowser::WxReferencedAssetsBrowser(void)
 : WxBrowser(), FViewportClient(), FTickableObject(), FSerializableObject()
 , ViewMode(RAV_Thumbnail), DepthMode(REFDEPTH_Infinite), bGroupByClass(FALSE), bIncludeDefaultRefs(TRUE), bIncludeScriptRefs(FALSE)
 , bIsScrolling(FALSE), bNeedsUpdate(FALSE), SelectionMutex(0), SplitterPos(STD_SPLITTER_SZ), SyncToNextDraw(NULL), MainSplitter(NULL), ViewModeSplitter(NULL), ToolBar(NULL)
 , ReferenceTree(NULL), ListView(NULL), ViewportHolder(NULL), Viewport(NULL)
{
	// Register the selection change callback
	GCallbackEvent->Register(CALLBACK_SelectObject,this);
	GCallbackEvent->Register(CALLBACK_MapChange, this);

	// Set up our ignore lists
	IgnoreClasses.AddItem(ULevel::StaticClass());
	IgnoreClasses.AddItem(UWorld::StaticClass());

	IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EditorMaterials"),TRUE));
	IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EngineResources"),TRUE));
	IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EngineFonts"),TRUE));
	IgnorePackages.AddItem(FindObject<UPackage>(NULL,TEXT("EngineMaterials"),TRUE));
	IgnorePackages.AddItem(UObject::GetTransientPackage());

	// initialize the column sizes
	for ( INT i = 0; i < COLHEADER_MAX; i++ )
	{
		ColumnWidth[i] = 200;
	}
}

/**
 * Forwards the call to our base class to create the window relationship.
 * Creates any internally used windows after that
 *
 * @param DockID the unique id to associate with this dockable window
 * @param FriendlyName the friendly name to assign to this window
 * @param Parent the parent of this window (should be a Notebook)
 */
void WxReferencedAssetsBrowser::Create(INT DockID,const TCHAR* FriendlyName,
	wxWindow* Parent)
{
	// Let our base class start up the windows
	WxBrowser::Create(DockID,FriendlyName,Parent);

	// make sure the shared selection set has been created
	GetSelection();

	// Add a menu bar.
	MenuBar = new WxMBReferencedAssetsBrowser();

	// Add the toolbar
	ToolBar = new WxRABrowserToolBar( (wxWindow*)this, -1 );

	// create the primary controls
	wxBoxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
	{
		// this is the splitter that separates the tree control & asset area
		MainSplitter = new wxSplitterWindow( this, -1, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE | wxSP_BORDER );
		{
			// the tree control that displays the object reference graph
			ReferenceTree = new WxTreeCtrl( MainSplitter, ID_REFERENCE_GRAPH_TREE, NULL/*, new wxRATreeContextMenu*/, wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT );

			// the area that displays the referenced assets, in either thumbnail or list format
			ViewModeSplitter = new wxSplitterWindow( MainSplitter, -1, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE | wxSP_BORDER );
			{
				ListView = new wxListView( ViewModeSplitter, ID_LIST_VIEW, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_ALIGN_LEFT | wxLC_SORT_ASCENDING );

				// Init the various viewport classes
				ViewportHolder = new WxViewportHolder( ViewModeSplitter,-1,1);
				Viewport = GEngine->Client->CreateWindowChildViewport(this,(HWND)ViewportHolder->GetHandle());
				Viewport->CaptureJoystickInput(FALSE);
				ViewportHolder->SetViewport(Viewport);
			}
		}
		MainSplitter->SplitVertically( ReferenceTree, ViewModeSplitter, STD_SPLITTER_SZ );
		MainSplitter->SetMinimumPaneSize(20);

		ViewModeSplitter->SplitVertically( ListView, ViewportHolder, STD_SPLITTER_SZ );

		MainSizer->Add(MainSplitter, 1, wxEXPAND);
	}

	SetSizer(MainSizer);
	SetAutoLayout(TRUE);

	ListView->InsertColumn( COLHEADER_Name,		*LocalizeUnrealEd("Object"), wxLIST_FORMAT_LEFT, ColumnWidth[COLHEADER_Name] );
	ListView->InsertColumn( COLHEADER_Info,		*LocalizeUnrealEd("Info"), wxLIST_FORMAT_LEFT, ColumnWidth[COLHEADER_Info] );
	ListView->InsertColumn( COLHEADER_Group,	*LocalizeUnrealEd("Group"), wxLIST_FORMAT_LEFT, ColumnWidth[COLHEADER_Group] );
	ListView->InsertColumn( COLHEADER_Size,		*LocalizeUnrealEd("ResourceSizeK"), wxLIST_FORMAT_LEFT, ColumnWidth[COLHEADER_Size] );
	ListView->InsertColumn( COLHEADER_Class,	*LocalizeUnrealEd("Class"), wxLIST_FORMAT_LEFT, ColumnWidth[COLHEADER_Class] );

	MainSizer->Fit(this);
	LoadSettings();
}

/**
 * Destructor. Cleans up allocated resources
 */
WxReferencedAssetsBrowser::~WxReferencedAssetsBrowser(void)
{
	SaveSettings();

	if (Viewport != NULL)
	{
		GEngine->Client->CloseViewport(Viewport);
		Viewport = NULL;
	}
}

/**
 * Returns the shared selection set for the this browser.
 */
USelection* WxReferencedAssetsBrowser::GetSelection()
{
	if ( Selection == NULL )
	{
		Selection = new( UObject::GetTransientPackage(), TEXT("SelectedAssets"), RF_Transactional ) USelection;
		Selection->AddToRoot();
	}

	return Selection;
}

void WxReferencedAssetsBrowser::LoadSettings()
{
	GConfig->GetInt( TEXT("ReferencedAssets"), TEXT("ViewMode"), (INT&)ViewMode, GEditorUserSettingsIni );
	GConfig->GetInt( TEXT("ReferencedAssets"), TEXT("SplitterPos"), SplitterPos, GEditorUserSettingsIni );
	GConfig->GetInt( TEXT("ReferencedAssets"), TEXT("DepthMode"), (INT&)DepthMode, GEditorUserSettingsIni );
	GConfig->GetBool( TEXT("ReferencedAssets"), TEXT("GroupByClass"), bGroupByClass, GEditorUserSettingsIni );
	GConfig->GetBool( TEXT("ReferencedAssets"), TEXT("IncludeDefaultRefs"), bIncludeDefaultRefs, GEditorUserSettingsIni );
	GConfig->GetBool( TEXT("ReferencedAssets"), TEXT("IncludeScriptRefs"), bIncludeScriptRefs, GEditorUserSettingsIni );

	INT CustomDepth = 0;
	GConfig->GetInt( TEXT("ReferencedAssets"), TEXT("CustomDepth"), CustomDepth, GEditorUserSettingsIni );

	for ( INT ColIndex = 0; ColIndex < COLHEADER_MAX; ColIndex++ )
	{
		FString Key = FString::Printf(TEXT("ColumnWidth[%d]"), ColIndex);
		GConfig->GetInt( TEXT("ReferencedAssets"), *Key, (INT&)ColumnWidth[ColIndex], GEditorUserSettingsIni );

		ListView->SetColumnWidth( ColIndex, ColumnWidth[ColIndex] );
	}

	MainSplitter->SetSashPosition(SplitterPos);
	SetViewMode(ViewMode);
	SetDepthMode(DepthMode);

	// set the initial custom depth - change the current DepthMode to something other than REFDEPTH_Custom so that we don't
	// request a full update as a result of initializing this text control
	EReferenceDepthMode RealDepthMode = DepthMode;
	DepthMode = REFDEPTH_Direct;
	ToolBar->SetCustomDepth(CustomDepth);

	// now restore the configured depth mode.
	DepthMode = RealDepthMode;

	wxSizeEvent DummyEvent;
	OnSize( DummyEvent );
}

void WxReferencedAssetsBrowser::SaveSettings()
{
	SplitterPos = MainSplitter->GetSashPosition();
	for ( INT ColIndex = 0; ColIndex < COLHEADER_MAX; ColIndex++ )
	{
		ColumnWidth[ColIndex] = ListView->GetColumnWidth(ColIndex);
	}

	GConfig->SetInt( TEXT("ReferencedAssets"), TEXT("ViewMode"), (INT&)ViewMode, GEditorUserSettingsIni );
	GConfig->SetInt( TEXT("ReferencedAssets"), TEXT("SplitterPos"), SplitterPos, GEditorUserSettingsIni );
	GConfig->SetInt( TEXT("ReferencedAssets"), TEXT("DepthMode"), (INT&)DepthMode, GEditorUserSettingsIni );
	GConfig->SetBool( TEXT("ReferencedAssets"), TEXT("GroupByClass"), bGroupByClass, GEditorUserSettingsIni );
	GConfig->SetBool( TEXT("ReferencedAssets"), TEXT("IncludeDefaultRefs"), bIncludeDefaultRefs, GEditorUserSettingsIni );
	GConfig->SetBool( TEXT("ReferencedAssets"), TEXT("IncludeScriptRefs"), bIncludeScriptRefs, GEditorUserSettingsIni );

	INT CustomDepth = ToolBar->GetCustomDepth();
	GConfig->SetInt( TEXT("ReferencedAssets"), TEXT("CustomDepth"), CustomDepth, GEditorUserSettingsIni );
}

/**
 * Changes the asset view mode to the mode specified.
 */
void WxReferencedAssetsBrowser::SetViewMode( EReferencedAssetViewMode InViewMode )
{
	ViewMode = InViewMode;

	ListView->Show();
	ViewportHolder->Show();

	ViewModeSplitter->SplitVertically( ListView, ViewportHolder, STD_SPLITTER_SZ );
	switch( ViewMode )
	{
	case RAV_List:
		ViewModeSplitter->Unsplit( ViewportHolder );
		break;

	case RAV_Thumbnail:
		ViewModeSplitter->Unsplit( ListView );
		break;
	}
}

/**
 * Changes the reference depth mode to the mode specified.
 */
void WxReferencedAssetsBrowser::SetDepthMode( EReferenceDepthMode InDepthMode )
{
	DepthMode = InDepthMode;
	ToolBar->EnableCustomDepth(DepthMode==REFDEPTH_Custom);

	// indicate that we need to rebuild the referenced asset list next frame
	RequestUpdate();
}

/**
 * Retrieves the currently configured serialization recursion depth, as determine by the depth mode and the configured custom depth.
 *
 * @return	the current maximum recursion depth to use when searching for referenced assets; 0 if infinite recursion depth is desired.
 */
INT WxReferencedAssetsBrowser::GetRecursionDepth() const
{
	INT Result = 0;

	switch ( DepthMode )
	{
	case REFDEPTH_Direct:
		Result = 1;
		break;

	case REFDEPTH_Infinite:
		Result = 0;
		break;

	case REFDEPTH_Custom:
		Result = ToolBar->GetCustomDepth();
		break;
	}

	return Result;
}


/**
 * Caches the names of the objects referenced by the currently selected actors.
 */
ObjectNameMap ObjectNameCache;

/**
 * Builds a list of assets to display from the currently selected actors.
 * NOTE: It ignores assets that are there because they are always loaded
 * such as default materials, textures, etc.
 */
void WxReferencedAssetsBrowser::BuildAssetList(void)
{
	// Clear the old list
	Referencers.Empty();
	ReferenceGraph.Empty();
	ObjectNameCache.Empty();

	TArray<UObject*> BspMats;
	// Search all BSP surfaces for ones that are selected and add their
	// materials to a temp list
	for (INT Index = 0; Index < GWorld->GetModel()->Surfs.Num(); Index++)
	{
		// Only add materials that are selected
		if (GWorld->GetModel()->Surfs(Index).PolyFlags & PF_Selected)
		{
			// No point showing the default material
			if (GWorld->GetModel()->Surfs(Index).Material != NULL)
			{
				BspMats.AddUniqueItem(GWorld->GetModel()->Surfs(Index).Material);
			}
		}
	}
	// If any BSP surfaces are selected
	if (BspMats.Num() > 0)
	{
		FReferencedAssets* Referencer = new(Referencers) FReferencedAssets(GWorld->GetModel());

		// Now copy the array
		Referencer->AssetList = BspMats;
		ReferenceGraph.Set(GWorld->GetModel(), BspMats);
	}

	// this is the maximum depth to use when searching for references
	const INT MaxRecursionDepth = GetRecursionDepth();

	USelection* ActorSelection = GEditor->GetSelectedActors();

	// first, we need to determine if we have any PrefabInstances selected.  If so, and prefab selection lock is enabled,
	// then all of the PrefabInstance's actors will be part of the reference set so we'll need to ignore them since that would just
	// result in a huge list of duplicate assets
	TArray<AActor*> ActorsToSkip;
	if ( GEditor->bPrefabsLocked )
	{
		TArray<APrefabInstance*> SelectedPrefabInstances;
		ActorSelection->GetSelectedObjects<APrefabInstance>(SelectedPrefabInstances);
		for ( INT PrefabIndex = 0; PrefabIndex < SelectedPrefabInstances.Num(); PrefabIndex++ )
		{
			TArray<AActor*> PrefabActors;
			SelectedPrefabInstances(PrefabIndex)->GetActorsInPrefabInstance(PrefabActors);
			for ( INT ActorIndex = 0; ActorIndex < PrefabActors.Num(); ActorIndex++ )
			{
				ActorsToSkip.AddUniqueItem(PrefabActors(ActorIndex));
			}
		}
	}

	// Mark all objects so we don't get into an endless recursion
	for (FObjectIterator It; It; ++It)
	{
		// Skip the level, world, and any packages that should be ignored
		if ( ShouldSearchForAssets(*It,IgnoreClasses,IgnorePackages,bIncludeDefaultRefs) )
		{
			It->SetFlags(RF_TagExp);
		}
		else
		{
			It->ClearFlags(RF_TagExp);
		}
	}

	TArray<AActor*> SelectedActors;
	// Get the list of currently selected actors
	ActorSelection->GetSelectedObjects<AActor>(SelectedActors);

	// Build the list of assets from the set of selected actors
	for (INT Index = 0; Index < SelectedActors.Num(); Index++)
	{
		if ( !ActorsToSkip.ContainsItem(SelectedActors(Index)) )
		{
			// Create a new entry for this actor
			FReferencedAssets* Referencer = new(Referencers) FReferencedAssets(SelectedActors(Index));

			// Add to the list of referenced assets
			FFindAssetsArchive(SelectedActors(Index),IgnoreClasses,IgnorePackages,
				Referencer->AssetList,&ReferenceGraph,MaxRecursionDepth,bIncludeScriptRefs,bIncludeDefaultRefs);
		}
	}

	// rebuild the name cache
	for ( INT RefIndex = 0; RefIndex < Referencers.Num(); RefIndex++ )
	{
		FReferencedAssets& Referencer = Referencers(RefIndex);
		if ( !ObjectNameCache.HasKey(Referencer.Referencer) )
		{
			ObjectNameCache.Set(Referencer.Referencer, *Referencer.Referencer->GetName());
		}

		for ( INT AssetIndex = 0; AssetIndex < Referencer.AssetList.Num(); AssetIndex++ )
		{
			if ( !ObjectNameCache.HasKey(Referencer.AssetList(AssetIndex)) )
			{
				ObjectNameCache.Set(Referencer.AssetList(AssetIndex), *Referencer.AssetList(AssetIndex)->GetName());
			}
		}
	}

	SortAssetList();

	// now deselect any objects which are no longer part of the reference set
	FScopedSelectionNotificationHandler Handler(this);
	for ( USelection::TObjectIterator It( Selection->ObjectItor() ) ; It ; ++It )
	{
		UObject* SelectedAsset = *It;
		if ( ReferenceGraph.Find(SelectedAsset) == NULL )
		{
			GetSelection()->Deselect(SelectedAsset);
		}
	}
}

/**
 * Returns a list of all assets referenced by the specified UObject.
 */
void WxReferencedAssetsBrowser::BuildAssetList(UObject *Object, const TArray<UClass*>& IgnoreClasses, const TArray<UObject*>& IgnorePackages, TArray<UObject*>& ReferencedAssets)
{
	TArray<FReferencedAssets> LocalReferencers;

	// Create a new entry for this actor.
	new( LocalReferencers ) FReferencedAssets( Object );

	for (FObjectIterator It; It; ++It)
	{
		// Skip the level, world, and any packages that should be ignored
		if ( ShouldSearchForAssets(*It,IgnoreClasses,IgnorePackages) )
		{
			It->SetFlags(RF_TagExp);
		}
		else
		{
			It->ClearFlags(RF_TagExp);
		}
	}

	// Add to the list of referenced assets.
	FFindAssetsArchive( Object, IgnoreClasses, IgnorePackages, LocalReferencers.Last().AssetList );

	ReferencedAssets = LocalReferencers.Last().AssetList;
}

const TCHAR* GetObjectNameFromCache( UObject* Obj )
{
	FString* CachedObjectName = ObjectNameCache.Find(Obj);
	if ( CachedObjectName == NULL )
	{
		CachedObjectName = &ObjectNameCache.Set(Obj, *Obj->GetName());
	}
	return **CachedObjectName;
}

/**
 * For use with the templated sort. Sorts by class name then object name
 */
struct FClassNameObjNameCompare_RABrowser
{
	static INT Compare(UObject* A,UObject* B)
	{
		INT Class = appStricmp(GetObjectNameFromCache(A->GetClass()),GetObjectNameFromCache(B->GetClass()));
		return Class == 0 ? appStricmp(GetObjectNameFromCache(A),GetObjectNameFromCache(B)) : Class;
	}
};
struct FReferencerClassCompare
{
	static INT Compare( const WxReferencedAssetsBrowser::FReferencedAssets& A, const WxReferencedAssetsBrowser::FReferencedAssets& B )
	{
		return FClassNameObjNameCompare_RABrowser::Compare(A.Referencer, B.Referencer);
	}
};

/**
 * For use with the templated sort. Sorts by object name
 */
struct FObjNameCompare_RABrowser
{
	static INT Compare(UObject* A,UObject* B)
	{
		return appStricmp(GetObjectNameFromCache(A),GetObjectNameFromCache(B));
	}
};
struct FReferencerObjCompare
{
	static INT Compare( const WxReferencedAssetsBrowser::FReferencedAssets& A, const WxReferencedAssetsBrowser::FReferencedAssets& B )
	{
		return FObjNameCompare_RABrowser::Compare(A.Referencer, B.Referencer);
	}
};

/**
 * Sorts the list of referenced assets that is used for rendering the thumbnails.
 */
void WxReferencedAssetsBrowser::SortAssetList()
{
	// first, sort the list of referencers
	if ( bGroupByClass == TRUE )
	{
		// Sort by class, then object name.
		Sort<FReferencedAssets,FReferencerClassCompare>(&Referencers(0), Referencers.Num());
	}
	else
	{
		// Sort by name.
		Sort<FReferencedAssets,FReferencerObjCompare>(&Referencers(0), Referencers.Num());
	}

	// now sort the asset lists for each referencer - these lists are used to determine which order to render the thumbnails in
	for ( INT ActorIndex = 0; ActorIndex < Referencers.Num(); ActorIndex++ )
	{
		FReferencedAssets& Asset = Referencers(ActorIndex);
		if ( bGroupByClass == TRUE )
		{
			// Sort by class, then object name.
			Sort<UObject*,FClassNameObjNameCompare_RABrowser>(&Asset.AssetList(0), Asset.AssetList.Num());
		}
		else
		{
			// Sort by just object name
			Sort<UObject*,FObjNameCompare_RABrowser>(&Asset.AssetList(0), Asset.AssetList.Num());
		}
	}
}


/**
 * Handles the key events for the thumbnail viewport. It only supports
 * scrolling.
 */
UBOOL WxReferencedAssetsBrowser::InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed/*=1.0f*/,UBOOL bGamepad/*=FALSE*/)
{
	// Hide and lock mouse cursor if we're capturing mouse input
	Viewport->ShowCursor( !Viewport->HasMouseCapture() );
	Viewport->LockMouseToWindow( Viewport->HasMouseCapture() );

	// If we are scrolling and the we've released the button, stop scrolling
	if (bIsScrolling == TRUE && Key == KEY_LeftMouseButton && Event == IE_Released)
	{
		bIsScrolling = FALSE;
		Viewport->Invalidate();
	}
	// Skip this if we are scrolling
	else if (bIsScrolling == FALSE)
	{
		if((Key == KEY_LeftMouseButton || Key == KEY_RightMouseButton) && (Event == IE_Pressed || Event == IE_DoubleClick))
		{
			const INT	HitX = Viewport->GetMouseX();
			const INT	HitY = Viewport->GetMouseY();
			HHitProxy*	HitResult = Viewport->GetHitProxy(HitX,HitY);
			UObject*	HitObject = NULL;

			if( HitResult )
			{
				if( HitResult->IsA(HObject::StaticGetType()) )
				{
					HitObject = ((HObject*)HitResult)->Object;
				}
			}
			else
			{
				// Clicking on the background with the left button initiates scrolling.
				bIsScrolling = TRUE;
				Viewport->Invalidate();
			}

			if( HitObject )
			{
				FScopedSelectionNotificationHandler Handler(this);

				UBOOL bDeselectAll = TRUE;
				UBOOL bSelectionChanged = FALSE;

				// Right-clicking on an object that is already selected will not deselect anything,
				// and instead display a context menu for all selected items.
				// Right-clicking on an object that is currently unselected will select only that object,
				// then display the context menu for that item.
				if ( Key == KEY_RightMouseButton )
				{
					const UBOOL bObjectAlreadySelected = GetSelection()->IsSelected( HitObject );
					if ( bObjectAlreadySelected )
					{
						bDeselectAll = FALSE;
					}
				}

				// Deselect everything unless CTRL is held down.
				if ( bDeselectAll )
				{
					if( !Viewport->KeyState(KEY_LeftControl) && !Viewport->KeyState(KEY_RightControl) )
					{
						GetSelection()->DeselectAll();
						bSelectionChanged = TRUE;
					}
				}

				if( Key == KEY_RightMouseButton )
				{
					if(HitObject->IsSelected() == FALSE)
					{
						GetSelection()->Select( HitObject );
						bSelectionChanged = TRUE;
					}
				}
				else
				{
					GetSelection()->ToggleSelect( HitObject );
					bSelectionChanged = TRUE;
				}

				if( bSelectionChanged )
				{
					UpdateList();
					SynchronizeTreeSelections();
				}
			}

			Viewport->InvalidateDisplay();

			if( HitObject )
			{
				if( Key == KEY_LeftMouseButton && Event == IE_DoubleClick )
				{
					ShowObjectProperties(HitObject);
				}
				//@todo ronp
// 				else if( Key == KEY_RightMouseButton && Event == IE_Pressed )
// 				{
// 					ShowContextMenu();
// 				}
			}
		}

		// Did they scroll using the mouse wheel?
		else if (Key == KEY_MouseScrollUp)
		{
			ViewportHolder->ScrollViewport(-_GB_SCROLL_SPEED);
		}
		// Did they scroll using the mouse wheel?
		else if (Key == KEY_MouseScrollDown)
		{
			ViewportHolder->ScrollViewport(_GB_SCROLL_SPEED);
		}

		// Handle refresh requests if this window has focus
		else if ( Key == KEY_F5 && Event == IE_Released )
		{
			wxCommandEvent DummyEvent;
			OnRefresh(DummyEvent);
		}
		else if ( Key == KEY_C && Event == IE_Released && IsCtrlDown(Viewport) )
		{
			FString SelectedObjectList;
			for ( USelection::TObjectIterator It(GetSelection()->ObjectItor()); It; ++It )
			{
				if ( SelectedObjectList.Len() > 0 )
				{
					SelectedObjectList += LINE_TERMINATOR;
				}

				SelectedObjectList += (*It)->GetFullName();
			}

			appClipboardCopy(*SelectedObjectList);
		}
	}

	return TRUE;
}

/**
 * Updates the scrolling location based on the mouse movement
 */
UBOOL WxReferencedAssetsBrowser::InputAxis(FViewport* Viewport,INT ControllerId,FName Key,
	FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad)
{
	// If we are scrolling, update the scroll position based upon the movement
	if (bIsScrolling == TRUE)
	{
		const FLOAT GRAB_SCROLL_SPEED = 5.0f;
		ViewportHolder->ScrollViewport(-appTrunc(Delta * GRAB_SCROLL_SPEED));
	}

	return TRUE;
}

/**
 * Repaints the viewport with the thumbnail data
 *
 * @param Viewport the viewport to draw in
 * @param RI the render interface to draw with
 */
void WxReferencedAssetsBrowser::Draw( FViewport* Viewport, FCanvas* Canvas )
{
	// Don't draw anything if hit testing, since this doesn't support selection
	//@todo add hit testing support
	if ( ViewMode == RAV_Thumbnail )
	{
		// Clear the viewport with our background image
		DrawTile(Canvas,0,0,Viewport->GetSizeX(),Viewport->GetSizeY(),0.f,0.f,
			(FLOAT)Viewport->GetSizeX() / (FLOAT)GEditor->Bkgnd->SizeX,
			(FLOAT)Viewport->GetSizeY() / (FLOAT)GEditor->Bkgnd->SizeY,
			FLinearColor::White,GEditor->Bkgnd->Resource);

		// Figure out where in the viewport we are currently positioned
		INT ScrollBarPosition = ViewportHolder->GetScrollThumbPos();
		// Current X & Y draw positions
		INT XPos = STD_TNAIL_PAD_HALF, YPos = STD_TNAIL_PAD_HALF - ScrollBarPosition;
		// The tallest item on a give row
		INT YHighest = 0;
		// Total size drawn so we can adjust the scrollbars properly
		INT SBTotalHeight = 0;

		INT Ignored;
		// Font height for the referencer name
		INT ReferencerFontHeight;
		StringSize(GEngine->MediumFont,Ignored,ReferencerFontHeight,TEXT("X"));

		// This seems to be a good fixed size
		INT FixedSize = 196;

		FMemMark Mark(GMainThreadMemStack);

		// For each referencer list the assets its holding refs to
		for (INT Index = 0; Index < Referencers.Num(); Index++)
		{
			FReferencedAssets& Asset = Referencers(Index);

			// Skip padding the first line as it looks bad
			if (Index > 0)
			{
				// Move to the next "line"
				YPos += YHighest + (STD_TNAIL_PAD * 2);
				SBTotalHeight += YHighest + (STD_TNAIL_PAD * 2);
			}
			// Indicate where the references are coming from
			FString ReferencerTitle = (Asset.Referencer == GWorld->GetModel())
				? FString::Printf(TEXT("BSP.%s"), GetObjectNameFromCache(Asset.Referencer))
				: GetObjectNameFromCache(Asset.Referencer);

			DrawString(Canvas,STD_TNAIL_PAD_HALF,YPos,
				*ReferencerTitle,GEngine->MediumFont,
				FLinearColor::White);

			// Update where to draw next
			YPos += ReferencerFontHeight + STD_TNAIL_PAD_HALF;
			XPos = STD_TNAIL_PAD_HALF;
			// And our total height used for all drawing
			SBTotalHeight += ReferencerFontHeight + STD_TNAIL_PAD_HALF;
			// Get a ref to the asset list so I don't overwork my lazy fingers
			const TArray<UObject*>& AssetList = Referencers(Index).AssetList;
			// Now draw each asset being referenced
			for (INT AssetIndex = 0; AssetIndex < AssetList.Num(); AssetIndex++)
			{
				// Get the object to draw a thumbnail for
				UObject* Asset = AssetList(AssetIndex);

				// Get the rendering info for this object
				FThumbnailRenderingInfo* RenderInfo =
					GUnrealEd->GetThumbnailManager()->GetRenderingInfo(Asset);

				if ( RenderInfo != NULL && RenderInfo->Renderer != NULL )
				{
					// If we are trying to sync to a specific object and we find it, record the vertical position.
					if ( Asset == SyncToNextDraw )
					{
						ScrollBarPosition = YPos + ViewportHolder->GetScrollThumbPos();
					}

					DWORD MaxLabelX=0, MaxLabelY=0;
					UThumbnailLabelRenderer* LabelRenderer = RenderInfo->LabelRenderer;

					// Setup thumbnail options
					UThumbnailLabelRenderer::ThumbnailOptions ThumbnailOptions;
					{
						// Never draw content tags in the referenced asset browser
						ThumbnailOptions.bShowContentTags = FALSE;
					}


					// If it has a thumbnail label renderer
					if ( LabelRenderer != NULL )
					{
						// Get the width/height of the thumbnail labels
						RenderInfo->LabelRenderer->GetThumbnailLabelSize(Asset,
							GEngine->SmallFont,Viewport,Canvas, ThumbnailOptions, MaxLabelX,MaxLabelY);
					}
					const DWORD MaxWidth = Max<DWORD>(FixedSize, MaxLabelX);

					// If this thumbnail is too large for the current line,
					// move to the next line
					if (static_cast<UINT>( XPos + MaxWidth ) > Viewport->GetSizeX())
					{
						YPos += YHighest + STD_TNAIL_PAD_HALF;
						SBTotalHeight += YHighest + STD_TNAIL_PAD_HALF;
						YHighest = 0;
						XPos = STD_TNAIL_PAD_HALF;
					}

					// Don't bother drawing if it won't be visible
					if (YPos + FixedSize + STD_TNAIL_PAD >= 0 && YPos <= (INT)Viewport->GetSizeY())
					{
						if ( Canvas->IsHitTesting() )
						{
							// If hit testing, draw a tile instead of the actual thumbnail to avoid rendering thumbnail scenes(which screws up hit detection).
							Canvas->SetHitProxy(new HObject(Asset));
							DrawTile(Canvas,XPos,YPos,MaxWidth,FixedSize + MaxLabelY,0.0f,0.0f,1.f,1.f,FLinearColor::White,GEditor->BkgndHi->Resource);
							Canvas->SetHitProxy(NULL);
						}
						else
						{
							// Draw the border with the configured color
							DrawTile(Canvas,XPos - STD_TNAIL_HIGHLIGHT_EDGE,
								YPos - STD_TNAIL_HIGHLIGHT_EDGE,
								MaxWidth + (STD_TNAIL_HIGHLIGHT_EDGE * 2),
								FixedSize + MaxLabelY + (STD_TNAIL_HIGHLIGHT_EDGE * 2),
								0.f,0.f,1.f,1.f,RenderInfo->BorderColor);

							// Figure whether to draw it with black or highlight color
							FColor Backdrop(0,0,0);
							if (GetSelection()->IsSelected( Asset ))
							{
								Backdrop = FColor(255,255,255);
							}

							// Draw either a black backdrop or the highlight color one
							DrawTile(Canvas,XPos - STD_TNAIL_HIGHLIGHT_EDGE + 2,
								YPos - STD_TNAIL_HIGHLIGHT_EDGE + 2,
								MaxWidth + (STD_TNAIL_HIGHLIGHT_EDGE * 2) - 4,
								FixedSize + MaxLabelY + (STD_TNAIL_HIGHLIGHT_EDGE * 2) - 4,
								0.f,0.f,1.f,1.f,Backdrop,GEditor->BkgndHi->Resource);

							// Draw the thumbnail with the background
							RenderInfo->Renderer->Draw(Asset,TPT_Plane,XPos,YPos,
								FixedSize,FixedSize,Viewport,Canvas,TBT_None, FColor(0, 0, 0), FColor(0, 0, 0));

							if ( LabelRenderer != NULL )
							{
								// Now draw the labels
								RenderInfo->LabelRenderer->DrawThumbnailLabels(Asset,
									GEngine->SmallFont,XPos,YPos + FixedSize,Viewport,Canvas,ThumbnailOptions);
							}
						}
					}

					// Keep track of the tallest thumbnail on this line
					if (FixedSize + MaxLabelY > static_cast<UINT>(YHighest) )
					{
						YHighest = FixedSize + MaxLabelY;
					}
					// Update XPos based upon the max of the label or the fixed size
					XPos += MaxWidth + STD_TNAIL_PAD_HALF;
				}
			}
		}
		Mark.Pop();

		// Update the scrollbar in the viewport holder
		SBTotalHeight += YHighest + STD_TNAIL_PAD_HALF;
		ViewportHolder->UpdateScrollBar(ScrollBarPosition,SBTotalHeight);

		// If we are trying to sync to a specific object, redraw the viewport so it
		// will be visible.
		if( SyncToNextDraw )
		{
			SyncToNextDraw = NULL;
			Draw( Viewport, Canvas );
		}
	}
}
/**
 * Selects the specified resource and scrolls the viewport so that resource is in view.
 *
 * @param	InObject		The resource to sync to.  Must be a valid pointer.
 */
void WxReferencedAssetsBrowser::SyncToObject(UObject* InObject )
{
	check( InObject );

	GetSelection()->DeselectAll();
	GetSelection()->Select( InObject );

	// Scroll the object into view
	SyncToNextDraw = InObject;

	Redraw();
}

/**
 * Refreshes the browser when an object selection changes
 *
 * @param InType ignored (we only sign up for selection change)
 * @param InObject ignored
 */
void WxReferencedAssetsBrowser::Send(ECallbackEventType InType,UObject*)
{
	// if the currently selected object has been changed, request the asset list be updated
	if ( InType == CALLBACK_SelectObject && SelectionMutex == 0 )
	{
		RequestUpdate();
	}
}

void WxReferencedAssetsBrowser::Send( ECallbackEventType InType, DWORD Flag )
{
	if ( InType == CALLBACK_MapChange && Flag != MapChangeEventFlags::Default )
	{
		FScopedSelectionNotificationHandler Handler(this);

		GetSelection()->DeselectAll();

		// the level is being changed in a way that makes it unsafe to maintain references to actors in the current level
		// clear all references
		BeginUpdate();
		{
			ReferenceTree->DeleteAllItems();
			ListView->DeleteAllItems();
		}
		EndUpdate();

		// Update should take care of clearing all references, since any map load event should be preceeded by deselecting all actors
		// but just to be on the safe side
		ObjectNameCache.Empty();
		Referencers.Empty();
		ReferenceGraph.Empty();
		ReferenceGraphMap.Empty();
		SyncToNextDraw = NULL;
	}
}

/* === FTickableObject interface === */
void WxReferencedAssetsBrowser::Tick( FLOAT DeltaTime )
{
	if ( bNeedsUpdate == TRUE )
	{
		Update();
	}
}

/**
 * Determines whether this tickable object is ready to be ticked.
 */
UBOOL WxReferencedAssetsBrowser::IsTickable() const
{
	return (bAreWindowsInitialized && IsShown()) == TRUE;
}

/* === FTickableObject interface === */
void WxReferencedAssetsBrowser::Serialize( FArchive& Ar )
{
	// serialize all of our object references
	if ( !Ar.IsPersistent() )
	{
		Ar << IgnoreClasses << IgnorePackages << Referencers << ReferenceGraph << SyncToNextDraw;
	}
}

/**
 * Called when the browser is getting activated (becoming the visible
 * window in it's dockable frame).
 */
void WxReferencedAssetsBrowser::Activated(void)
{
	// Let the super class do it's thing
	WxBrowser::Activated();

	// Update the viewport
	Redraw();
}

/**
 * Sets a flag indicating that the list of referenced assets needs to be regenerated. It is preferable to call
 * this method, rather than directly calling Update() in case multiple operations in a single tick/callstack
 * invalidate the asset list.
 */
void WxReferencedAssetsBrowser::RequestUpdate()
{
	bNeedsUpdate = TRUE;
}

/**
 * Requests that the contents of the currently visible window be refreshed.
 */
void WxReferencedAssetsBrowser::Redraw()
{
	if ( ViewMode == RAV_List )
	{
		UpdateList();
	}
	else
	{
		Viewport->Invalidate();
	}
}

/**
 * Tells the browser to update itself
 */
void WxReferencedAssetsBrowser::Update(void)
{
	BeginUpdate();

	// Do nothing unless we are visible
	if (IsShown() == TRUE)
	{
		const FScopedBusyCursor BusyCursor;
		GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("RefreshingAssetBrowser")), TRUE );

		bNeedsUpdate = FALSE;

		// recalculate the list of referenced assets
		BuildAssetList();

		// repopulate the reference graph tree
		UpdateTree();

		// repopulate the listview with the new set of referenced assets
		UpdateList();

		// redraw the viewport/list
		Viewport->Invalidate();

		ObjectNameCache.Shrink();
//		debugf(TEXT("WxReferencedAssetsBrowser::Update: ObjectNameCache consuming %d bytes"), ObjectNameCache.GetAllocatedSize());
		GWarn->EndSlowTask();
	}

	EndUpdate();
}

/**
 * Populates the reference tree with the current set of assets
 *
 * @todo ronp - restore the selected items; restore the expanded items
 */
void WxReferencedAssetsBrowser::UpdateTree()
{
	ReferenceTree->Freeze();

	// save the selection and expansion states of the tree
	ReferenceTree->SaveSelectionExpansionState();

	// first, delete all items
	ReferenceTree->DeleteAllItems();
	ReferenceGraphMap.Empty();

	// add the root item
	wxTreeItemId RootId = ReferenceTree->AddRoot(*LocalizeUnrealEd(TEXT("ReferenceGraphTreeText")));
	for ( INT ReferenceIndex = 0; ReferenceIndex < Referencers.Num(); ReferenceIndex++ )
	{
		// add an item at the root level for the selected actor
		FReferencedAssets& Asset = Referencers(ReferenceIndex);
		const wxTreeItemId BaseItemId = ReferenceTree->AppendItem( RootId, GetObjectNameFromCache(Asset.Referencer), -1, -1, new WxTreeObjectWrapper(Asset.Referencer) );

		TArray<UObject*>* ReferencedAssets = ReferenceGraph.Find(Asset.Referencer);
		check(ReferencedAssets);

		AddReferencedAssets( BaseItemId, Asset.Referencer, ReferencedAssets );
	}

	// now restore the selection expansion state of the tree
	ReferenceTree->Expand( RootId );
	ReferenceTree->RestoreSelectionExpansionState();
	ReferenceTree->Thaw();
}

/**
 * Returns the object associated with the specified tree item id, or NULL if there is no
 * object associated with that tree item or 
 */
UObject* WxReferencedAssetsBrowser::GetTreeObject( wxTreeItemId TreeId )
{
	UObject* Result = NULL;
	if ( TreeId.IsOk() )
	{
		WxTreeObjectWrapper* ItemData = (WxTreeObjectWrapper*)ReferenceTree->GetItemData(TreeId);
		if ( ItemData != NULL )
		{
			Result = ItemData->GetObject<UObject>();
		}
	}

	return Result;
}

/**
 * Ensures that the tree item associated with the selected objects is the only items selected in the tree
 */
void WxReferencedAssetsBrowser::SynchronizeTreeSelections()
{
	wxArrayTreeItemIds SelectedItems;
	if ( ReferenceTree != NULL )
	{
		ReferenceTree->Freeze();
		ReferenceTree->UnselectAll();

		// now select all objects which are currently selected
		for ( USelection::TObjectIterator It( GetSelection()->ObjectItor() ) ; It ; ++It )
		{
			UObject* SelectedObject = *It;
			wxTreeItemId* ItemId = ReferenceGraphMap.Find(SelectedObject);
			if ( ItemId != NULL )
			{
				ReferenceTree->SelectItem(*ItemId);
				ReferenceTree->EnsureVisible(*ItemId);
			}
		}

		ReferenceTree->Thaw();
	}
}

/**
 * Adds tree items for the objects specified under the specified parent id
 *
 * @param	ParentId	the tree id to add child items under
 * @param	BaseObject	the object corresponding to the tree item indicated by ParentId
 * @param	AssetList	the list of objects that should be added to the tree
 */
void WxReferencedAssetsBrowser::AddReferencedAssets( wxTreeItemId ParentId, UObject* BaseObject, TArray<UObject*>* AssetList )
{
	check(AssetList);

	ReferenceGraphMap.Set(BaseObject, ParentId);

	const FString ScriptItemString = LocalizeUnrealEd(TEXT("Script"));
	const FString DefaultsItemString = LocalizeUnrealEd(TEXT("Defaults"));
	for ( INT AssetIndex = 0; AssetIndex < AssetList->Num(); AssetIndex++ )
	{
		UObject* ReferencedObject = (*AssetList)(AssetIndex);
		check(ReferencedObject);

		// get the list of assets this object is referencing
		TArray<UObject*>* ReferencedAssets = ReferenceGraph.Find(ReferencedObject);
		UBOOL bRequiresChildren = FALSE;

		// add a new tree item for this referenced asset
		FString ItemString;
		if ( ReferencedObject == BaseObject->GetClass() )
		{
			ItemString = *ScriptItemString;
			if ( ReferencedAssets == NULL || ReferencedAssets->Num() == 0 )
			{
				// special case for the "Script" node - don't add it if it doesn't have any children
				continue;
			}
			else
			{
				bRequiresChildren = TRUE;
			}
		}
		else if ( ReferencedObject == BaseObject->GetArchetype() )
		{
			ItemString = *DefaultsItemString;
			if ( ReferencedAssets == NULL || ReferencedAssets->Num() == 0 )
			{
				// special case for the "Defaults" node - don't add it if it doesn't have any children
				continue;
			}
			else
			{
				bRequiresChildren = TRUE;
			}
		}
		else
		{
			ItemString = ReferencedObject->GetPathName();
		}

		wxTreeItemId AssetId = ReferenceTree->AppendItem( ParentId, *ItemString, -1, -1, new WxTreeObjectWrapper(ReferencedObject) );
		if ( ReferencedAssets != NULL )
		{
			// if this object is referencing other objects, add that list to the tree now.
			AddReferencedAssets( AssetId, ReferencedObject, ReferencedAssets );
		}

		if ( bRequiresChildren == TRUE && ReferenceTree->ItemHasChildren(AssetId) == false )
		{
			ReferenceTree->Delete(AssetId);
			ReferenceGraphMap.Remove(ReferencedObject);
		}
	}

	// sort the items contained within the specified parent
	ReferenceTree->SortChildren(ParentId);
}


/**
 * Checks an object to see if it should be included for asset searching
 *
 * @param Object the object in question
 * @param ClassesToIgnore the list of classes to skip
 * @param PackagesToIgnore the list of packages to skip
 * @param bIncludeDefaults specify TRUE to include content referenced through defaults
 *
 * @return TRUE if it should be searched, FALSE otherwise
 */
UBOOL WxReferencedAssetsBrowser::ShouldSearchForAssets( const UObject* Object, const TArray<UClass*>& ClassesToIgnore, const TArray<UObject*>& PackagesToIgnore, UBOOL bIncludeDefaults/*=FALSE*/ )
{
	UBOOL bShouldSearch = TRUE;

	if ( Object->HasAnyFlags(RF_ClassDefaultObject) && Object->GetOutermost()->GetFName() == NAME_Core )
	{
		// ignore all class default objects for classes which are declared in Core
		bShouldSearch = FALSE;
	}

	// Check to see if we should ignore a class
	for (INT Index = 0; Index < ClassesToIgnore.Num(); Index++)
	{
		// Bail if we are on the ignore list
		if ( Object->IsA(ClassesToIgnore(Index)) )
		{
			bShouldSearch = FALSE;
			break;
		}
	}

	if ( bShouldSearch )
	{
		// Check to see if we should ignore it due to package
		for ( INT Index = 0; Index < PackagesToIgnore.Num(); Index++ )
		{
			// If this object belongs to this package, bail
			if ( Object->IsIn(PackagesToIgnore(Index)) )
			{
				bShouldSearch = FALSE;
				break;
			}
		}
	}

	if ( bShouldSearch && !bIncludeDefaults && Object->IsTemplate() )
	{
		// if this object is an archetype and we don't want to see assets referenced by defaults, don't include this object
		bShouldSearch = FALSE;
	}

	return bShouldSearch;
}

int wxCALLBACK WxRABrowserListSort( long InItem1, long InItem2, long InSortData )
{
	UObject* A = (UObject*)InItem1;
	UObject* B = (UObject*)InItem2;
	FListViewSortOptions* so = (FListViewSortOptions*)InSortData;

	FString CompA;
	FString CompB;

	// Generate a string to run the compares against for each object based on
	// the current column.
	UBOOL bDoStringCompare = FALSE;
	int Ret = 0;
	switch( so->Column )
	{
	case WxReferencedAssetsBrowser::COLHEADER_Name:
		CompA = GetObjectNameFromCache(A);
		CompB = GetObjectNameFromCache(B);
		bDoStringCompare = TRUE;
		break;

	case WxReferencedAssetsBrowser::COLHEADER_Info:
		CompA = A->GetDesc();
		CompB = B->GetDesc();
		bDoStringCompare = TRUE;
		break;

	case WxReferencedAssetsBrowser::COLHEADER_Group:
		{
			if ( A->GetOuter() != NULL )
			{
				CompA = A->GetOuter()->GetPathName();
			}
			if ( B->GetOuter() != NULL )
			{
				CompB = B->GetOuter()->GetPathName();
			}
			bDoStringCompare = TRUE;
		}
		break;

	case WxReferencedAssetsBrowser::COLHEADER_Size:			// Resource size.
		{
			const INT SizeA = A->GetResourceSize();
			const INT SizeB = B->GetResourceSize();
			Ret = ( SizeA < SizeB ) ? -1 : ( SizeA > SizeB ? 1 : 0 );
		}
		break;

	default:
		check(0);	// Invalid column!
		break;
	}


	if ( bDoStringCompare )
	{
		Ret = appStricmp( *CompA, *CompB );
	}

	// If we are sorting backwards, invert the string comparison result.
	if( !so->bSortAscending )
	{
		Ret *= -1;
	}

	return Ret;
}

/**
 * This version of the list sorting callback sorts first by the class of the object, then by the selected column
 */
int wxCALLBACK WxRABrowserListSort_Class( long InItem1, long InItem2, long InSortData )
{
	UObject* A = (UObject*)InItem1;
	UObject* B = (UObject*)InItem2;

	int Result = appStricmp(GetObjectNameFromCache(A->GetClass()),GetObjectNameFromCache(B->GetClass()));
	if ( Result == 0 )
	{
		// class names were identical - fallback to normal sorting for these elements
		Result = WxRABrowserListSort(InItem1, InItem2, InSortData);
	}

	return Result;
}

/**
 * Populates the list view with the current set of assets
 */
void WxReferencedAssetsBrowser::UpdateList()
{
	ListView->Freeze();
	ListView->DeleteAllItems();

	int FirstSelectedItem = -1;
	for ( INT ReferenceIndex = 0; ReferenceIndex < Referencers.Num(); ReferenceIndex++ )
	{
		// add an item at the root level for the selected actor
		FReferencedAssets& Asset = Referencers(ReferenceIndex);
		for ( INT AssetIndex = 0; AssetIndex < Asset.AssetList.Num(); AssetIndex++ )
		{
			UObject* ReferencedAsset = Asset.AssetList(AssetIndex);

			// WDM : should grab a bitmap to use for each different kind of resource.  For now, they'll all look the same.
			const INT item = ListView->InsertItem( COLHEADER_Name, GetObjectNameFromCache(ReferencedAsset)/*, GBTCI_Resource*/ );

			// Create a string for the resource size.
			const INT ResourceSize = ReferencedAsset->GetResourceSize();
			FString ResourceSizeString;
			if ( ResourceSize > 0 )
			{
				ResourceSizeString = FString::Printf( TEXT("%.2f"), ((FLOAT)ResourceSize)/1024.f );
			}

			FString ObjectPathName;
			if ( ReferencedAsset->GetOuter() != NULL )
			{
				ObjectPathName = ReferencedAsset->GetOuter()->GetPathName();
			}

			// Add this referenced asset's information to the list.
			ListView->SetItem( item, COLHEADER_Info, *ReferencedAsset->GetDesc() );
			ListView->SetItem( item, COLHEADER_Group, *ObjectPathName );
			ListView->SetItem( item, COLHEADER_Size, *ResourceSizeString );
			ListView->SetItem( item, COLHEADER_Class, GetObjectNameFromCache(ReferencedAsset->GetClass()) );
			ListView->SetItemData( item, (long)ReferencedAsset );

			// select the item if the object is part of the current selection set
			if ( GetSelection()->IsSelected(ReferencedAsset) )
			{
				if ( !ListView->IsSelected(item) )
				{
					ListView->Select(item);
				}

				if ( FirstSelectedItem == -1 )
				{
					FirstSelectedItem = item;
				}
			}
		}
	}

	// If we have any items selected, jump to the first selected item.
	if ( FirstSelectedItem > -1 )
	{
		ListView->EnsureVisible(FirstSelectedItem);
	}

	if ( bGroupByClass == TRUE )
	{
		// sort by class, first
		ListView->SortItems( WxRABrowserListSort_Class, (LONG)&ListSortOptions );
	}
	else
	{
		// sort normally
		ListView->SortItems( WxRABrowserListSort, (LONG)&ListSortOptions );
	}

	ListView->Thaw();
}

void WxReferencedAssetsBrowser::OnSelectListItem( wxListEvent& Event )
{
	UObject* obj = (UObject*)Event.GetData();

	if(obj != NULL && obj->IsSelected() == FALSE)
	{
		FScopedSelectionNotificationHandler Handler(this);
		GetSelection()->Select( obj );
		SynchronizeTreeSelections();
	}
}

void WxReferencedAssetsBrowser::OnDeselectListItem( wxListEvent& Event )
{
	UObject* obj = (UObject*)Event.GetData();
	if(obj != NULL)
	{
		FScopedSelectionNotificationHandler Handler(this);
		GetSelection()->Deselect( obj );
		SynchronizeTreeSelections();
	}
}

void WxReferencedAssetsBrowser::OnListColumnClicked( wxListEvent& Event )
{
	int Column = Event.GetColumn();
	if( Column > -1  && Column != COLHEADER_Class )
	{
		if( Column == ListSortOptions.Column )
		{
			// Clicking on the same column will flip the sort order
			ListSortOptions.bSortAscending = !ListSortOptions.bSortAscending;
		}
		else
		{
			// Clicking on a new column will set that column as current and reset the sort order.
			ListSortOptions.Column = Column;
			ListSortOptions.bSortAscending = TRUE;
		}

		if ( bGroupByClass == TRUE )
		{
			// sort by class, first
			ListView->SortItems( WxRABrowserListSort_Class, (LONG)&ListSortOptions );
		}
		else
		{
			// sort normally
			ListView->SortItems( WxRABrowserListSort, (LONG)&ListSortOptions );
		}
	}
}

void WxReferencedAssetsBrowser::OnTreeItemSelected( wxTreeEvent& Event )
{
	if ( SelectionMutex == 0 )
	{
		wxTreeItemId ItemId = Event.GetItem();
		UObject* ActivatedObject = GetTreeObject(ItemId);
		if ( ActivatedObject != NULL )
		{
			FScopedSelectionNotificationHandler Handler(this);
			
			GetSelection()->DeselectAll();
			GetSelection()->Select(ActivatedObject);
			if ( ViewMode == RAV_Thumbnail )
			{
				SyncToNextDraw = ActivatedObject;
			}

			Redraw();
		}
	}
}

void WxReferencedAssetsBrowser::OnTreeItemActivated( wxTreeEvent& Event )
{
	wxTreeItemId ItemId = Event.GetItem();
	UObject* ActivatedObject = GetTreeObject(ItemId);
	ShowObjectProperties(ActivatedObject);
}

/**
 * Refreshes the contents of the window when requested
 *
 * @param In the command that was sent
 */
void WxReferencedAssetsBrowser::OnRefresh(wxCommandEvent& In)
{
	// Do nothing unless we are visible
	if (IsShown() == TRUE)
	{
		// Update the viewport
		RequestUpdate();
	}
}

/**
 * Sets the size of the viewport holder window based upon our new size
 *
 * @param In the command that was sent
 */
void WxReferencedAssetsBrowser::OnSize(wxSizeEvent& In)
{
	// During the creation process a sizing message can be sent so don't
	// handle it until we are initialized
	if (bAreWindowsInitialized)
	{
		const wxRect rc = GetClientRect();
		const wxRect rcT = ToolBar->GetClientRect();

		ToolBar->SetSize( 0, 0, rc.GetWidth(), rcT.GetHeight() );
		MainSplitter->SetSize( 0, rcT.GetHeight(), rc.GetWidth(), rc.GetHeight() - rcT.GetHeight() );
	}
}

void WxReferencedAssetsBrowser::OnViewMode( wxCommandEvent& Event )
{
	switch( Event.GetId() )
	{
	case ID_VIEW_LIST:
		SetViewMode( RAV_List );
		break;

	case ID_VIEW_THUMBNAIL:
		SetViewMode( RAV_Thumbnail );
		break;
	}

	wxSizeEvent DummyEvent;
	OnSize( DummyEvent );
}

void WxReferencedAssetsBrowser::OnDepthMode( wxCommandEvent& Event )
{
	switch( Event.GetId() )
	{
	case ID_REFERENCEDEPTH_DIRECT:
		SetDepthMode(REFDEPTH_Direct);
		break;
	case ID_REFERENCEDEPTH_ALL:
		SetDepthMode(REFDEPTH_Infinite);
	    break;
	case ID_REFERENCEDEPTH_CUSTOM:
		SetDepthMode(REFDEPTH_Custom);
	    break;
	}
}

void WxReferencedAssetsBrowser::OnIncludeDefaultRefs( wxCommandEvent& Event )
{
	bIncludeDefaultRefs = Event.IsChecked();

	RequestUpdate();
}

void WxReferencedAssetsBrowser::OnIncludeScriptRefs( wxCommandEvent& Event )
{
	bIncludeScriptRefs = Event.IsChecked();

	RequestUpdate();
}

void WxReferencedAssetsBrowser::OnGroupByClass( wxCommandEvent& Event )
{
	bGroupByClass = Event.IsChecked();

	// re-sort the list of assets that will be used for rendering thumbnails
	SortAssetList();

	// this call also triggers the list to be re-sorted.
	Redraw();
}

void WxReferencedAssetsBrowser::OnCustomDepthChanged( wxCommandEvent& Event )
{
	// only request an update if the custom depth mode is enabled
	if ( DepthMode == REFDEPTH_Custom )
	{
		RequestUpdate();
	}
}

void WxReferencedAssetsBrowser::UI_ViewMode( wxUpdateUIEvent& Event )
{
	switch( Event.GetId() )
	{
	case ID_VIEW_LIST:
		Event.Check( ViewMode == RAV_List );
		break;

	case ID_VIEW_THUMBNAIL:
		Event.Check( ViewMode == RAV_Thumbnail );
		break;
	}
}

void WxReferencedAssetsBrowser::UI_DepthMode( wxUpdateUIEvent& Event )
{
	switch ( Event.GetId() )
	{
	case ID_REFERENCEDEPTH_DIRECT:
		Event.Check(DepthMode == REFDEPTH_Direct);
		break;

	case ID_REFERENCEDEPTH_ALL:
		Event.Check(DepthMode == REFDEPTH_Infinite);
		break;

	case ID_REFERENCEDEPTH_CUSTOM:
		Event.Check(DepthMode == REFDEPTH_Custom);
		break;
	}
}

void WxReferencedAssetsBrowser::UI_IncludeDefaultRefs( wxUpdateUIEvent& Event )
{
	Event.Check(bIncludeDefaultRefs == TRUE);
}

void WxReferencedAssetsBrowser::UI_IncludeScriptRefs( wxUpdateUIEvent& Event )
{
	Event.Check(bIncludeScriptRefs == TRUE);
}

void WxReferencedAssetsBrowser::UI_GroupByClass( wxUpdateUIEvent& Event )
{
	Event.Check(bGroupByClass == TRUE);
}


/**
 * Scrolls the thumbnail viewport when it scroll bar messages are received
 *
 * @param InEvent the scroll information to apply
 */
void WxReferencedAssetsBrowser::OnScroll(wxScrollEvent& InEvent)
{
	ViewportHolder->ScrollViewport(InEvent.GetPosition(),FALSE);
}

/**
 * Functor that starts the serialization process
 *
 * @param Search the object to start searching
 * @param IgnoreClasses the list of classes to skip
 * @param IgnorePackages the list of packages to skip
 */
WxReferencedAssetsBrowser::FFindAssetsArchive::FFindAssetsArchive(
	UObject* Search,
	TArray<UClass*> IgnoreClasses,
	const TArray<UObject*>& IgnorePackages,
	TArray<UObject*>& OutAssetList,
	ObjectReferenceGraph* ReferenceGraph/*=NULL*/,
	INT MaxRecursion/*=0*/, UBOOL bIncludeClasses/*=TRUE*/, UBOOL bIncludeDefaults/*=FALSE*/ ) 
: StartObject(Search), AssetList(OutAssetList), CurrentReferenceGraph(ReferenceGraph)
, bIncludeScriptRefs(bIncludeClasses), bIncludeDefaultRefs(bIncludeDefaults), MaxRecursionDepth(MaxRecursion)
, CurrentDepth(0)
{
	ArIsObjectReferenceCollector = TRUE;
	ArIgnoreClassRef = !bIncludeScriptRefs;

	CurrentObject = StartObject;

	*this << StartObject;
}

/**
 * Adds the object refence to the asset list if it supports thumbnails.
 * Recursively searches through its references for more assets
 *
 * @param Obj the object to inspect
 */
FArchive& WxReferencedAssetsBrowser::FFindAssetsArchive::operator<<(UObject*& Obj)
{
	// Don't check null references or objects already visited
	if ( Obj != NULL && Obj->HasAnyFlags(RF_TagExp) &&

		// if we wish to filter out assets referenced through script, we need to ignore
		// all class objects, not just the UObject::Class reference
		(!ArIgnoreClassRef || Obj->GetClass() != UClass::StaticClass()) )
	{
		// Clear the search flag so we don't revisit objects
		Obj->ClearFlags(RF_TagExp);
		if ( Obj->IsA(UField::StaticClass()) )
		{
			// skip all of the other stuff because the serialization of UFields will quickly overflow
			// our stack given the number of temporary variables we create in the below code
			Obj->Serialize(*this);
		}
		else
		{
			// Only report this object reference if it supports thumbnail display
			// this eliminates all of the random objects like functions, properties, etc.
			const UBOOL bCDO = Obj->HasAnyFlags(RF_ClassDefaultObject);
			const UBOOL bIsContent = GUnrealEd->GetThumbnailManager()->GetRenderingInfo(Obj) != NULL;
			const UBOOL bIncludeAnyway = Obj->GetOuter() == CurrentObject && CurrentObject->GetClass() != UClass::StaticClass();
			const UBOOL bShouldReportAsset = !bCDO && (bIsContent || bIncludeAnyway);

			// remember which object we were serializing
			UObject* PreviousObject = CurrentObject;
			if ( bShouldReportAsset )
			{
				CurrentObject = Obj;

				// Add this object to the list to display
				AssetList.AddItem(CurrentObject);
				if ( CurrentReferenceGraph != NULL )
				{
					TArray<UObject*>* CurrentObjectAssets = GetAssetList(PreviousObject);
					check(CurrentObjectAssets);

					// add this object to the list of objects referenced by the object currently being serialized
					CurrentObjectAssets->AddItem(CurrentObject);	
					HandleReferencedObject(CurrentObject);
				}
			}
			else if ( Obj == StartObject )
			{
				HandleReferencedObject(Obj);
			}

			if ( MaxRecursionDepth == 0 || CurrentDepth < MaxRecursionDepth )
			{
				CurrentDepth++;

				// Now recursively search this object for more references
				Obj->Serialize(*this);

				CurrentDepth--;
			}

			// restore the previous object that was being serialized
			CurrentObject = PreviousObject;
		}
	}
	return *this;
}

/**
 * Manually serializes the class and archetype for the specified object so that assets which are referenced
 * through the object's class/archetype can be differentiated.
 */
void WxReferencedAssetsBrowser::FFindAssetsArchive::HandleReferencedObject(UObject* Obj )
{
	if ( CurrentReferenceGraph != NULL )
	{
		// here we allow recursion if the current depth is less-than-equal (as opposed to less-than) because the archetype and class are treated as transparent objects
		// serialization of the class and object are controlled by the "show class refs" and "show default refs" buttons
		if ( MaxRecursionDepth == 0 || CurrentDepth < MaxRecursionDepth )
		{
			// now change the current reference list to the one for this object
			if ( bIncludeDefaultRefs == TRUE )
			{
				TArray<UObject*>* ReferencedAssets = GetAssetList(Obj);

				// @see the comment for the bIncludeScriptRefs block
				UObject* ObjectArc = Obj->GetArchetype();
				ReferencedAssets->AddUniqueItem(ObjectArc);

				UObject* PreviousObject = CurrentObject;
				CurrentObject = ObjectArc;

				if ( ObjectArc->HasAnyFlags(RF_TagExp) )
				{
					// temporarily disable serialization of the class, as we need to specially handle that as well
					UBOOL bSkipClassSerialization = ArIgnoreClassRef;
					ArIgnoreClassRef = TRUE;

					ObjectArc->ClearFlags(RF_TagExp);
					ObjectArc->Serialize(*this);

					ArIgnoreClassRef = bSkipClassSerialization;
				}

				CurrentObject = PreviousObject;
			}

			if ( bIncludeScriptRefs == TRUE )
			{
				TArray<UObject*>* ReferencedAssets = GetAssetList(Obj);

				// we want to see assets referenced by this object's class, but classes don't have associated thumbnail rendering info
				// so we'll need to serialize the class manually in order to get the object references encountered through the class to fal
				// under the appropriate tree item

				// serializing the class will result in serializing the class default object; but we need to do this manually (for the same reason
				// that we do it for the class), so temporarily prevent the CDO from being serialized by this archive
				UClass* ObjectClass = Obj->GetClass();
				ReferencedAssets->AddUniqueItem(ObjectClass);

				UObject* PreviousObject = CurrentObject;
				CurrentObject = ObjectClass;

				if ( ObjectClass->HasAnyFlags(RF_TagExp) )
				{
					ObjectClass->ClearFlags(RF_TagExp);
					ObjectClass->Serialize(*this);
				}

				CurrentObject = PreviousObject;
			}
		}
	}
}

/**
 * Retrieves the referenced assets list for the specified object.
 */
TArray<UObject*>* WxReferencedAssetsBrowser::FFindAssetsArchive::GetAssetList( UObject* Referencer )
{
	check(Referencer);

	TArray<UObject*>* ReferencedAssetList = CurrentReferenceGraph->Find(Referencer);
	if ( ReferencedAssetList == NULL )
	{
		// add a new entry for the specified object
		ReferencedAssetList = &CurrentReferenceGraph->Set(Referencer, TArray<UObject*>());
	}

	return ReferencedAssetList;
}


