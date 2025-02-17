#include "UnrealEd.h"
#include "TagBrowser.h"
#include "GenericBrowser.h"

class WxTagBrowserContextMenu : public wxMenu
{
public:
	WxTagBrowserContextMenu()
	{
		Append(IDMN_ObjectContent_AddTag,*LocalizeUnrealEd("AddTag"),TEXT(""));
		Append(ID_SyncGenericBrowser,*LocalizeUnrealEd("SyncGenericBrowser"),TEXT(""));
	}
};

BEGIN_EVENT_TABLE(WxTagBrowser,WxBrowser)
	EVT_SCROLL( WxTagBrowser::OnScroll )
	EVT_SIZE( WxTagBrowser::OnSize )
	EVT_COMBOBOX( IDMN_TB_ASSETTYPE, WxTagBrowser::OnAssetTypeChange )
	EVT_COMBOBOX( IDMN_TB_SEARCHTYPE, WxTagBrowser::OnTagListChange )
	EVT_CHECKLISTBOX( IDMN_TB_REQTAGLIST, WxTagBrowser::OnTagListChange )
	EVT_CHECKLISTBOX( IDMN_TB_OPTTAGLIST, WxTagBrowser::OnFilterListChange )
	EVT_BUTTON(IDMN_TB_LOAD, WxTagBrowser::OnLoadButtonClick )
	EVT_CHECKBOX(IDMN_TB_AUTOLOAD, WxTagBrowser::OnAutoLoadChange )


	EVT_MENU(IDMN_ObjectContent_AddTag,WxTagBrowser::OnAddTag )
	EVT_MENU(ID_SyncGenericBrowser,WxTagBrowser::OnSyncGB )
END_EVENT_TABLE()

WxTagBrowser::WxTagBrowser()
{
	Selection = NULL;
	TagIndexObject = NULL;
	Viewport = NULL;
}

WxTagBrowser::~WxTagBrowser()
{
	if (Viewport != NULL)
	{
		GEngine->Client->CloseViewport(Viewport);
		Viewport = NULL;
	}
}

void WxTagBrowser::Serialize(FArchive& Ar)
{
	Ar << Assets;
}

void WxTagBrowser::Create(INT DockID,const TCHAR* FriendlyName,wxWindow* Parent)
{
	WxBrowser::Create(DockID,FriendlyName,Parent);

	// Add a menu bar
	MenuBar = new wxMenuBar();

	// Append the docking menu choices
	WxBrowser::AddDockingMenu( MenuBar );


	wxBoxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
	{
		// main vertical split
		SplitterWindow = new wxSplitterWindow( this, -1, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE | wxSP_BORDER );

		// left side
		wxPanel *LeftPanel = new wxPanel(SplitterWindow);
		{
			wxBoxSizer *LeftSizer = new wxBoxSizer(wxVERTICAL);

			wxBoxSizer *TopSizer = new wxBoxSizer(wxHORIZONTAL);
			{
				TopSizer->Add(new wxStaticText(LeftPanel,-1,TEXT("Asset Type:"), wxDefaultPosition, wxDefaultSize, 0),0,wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxRIGHT,5);
				AssetTypes = new wxComboBox( LeftPanel, IDMN_TB_ASSETTYPE,  TEXT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxCB_READONLY);
				TopSizer->Add(AssetTypes,1,wxALIGN_LEFT|wxALIGN_TOP,5);
			}

			LeftSizer->Add(TopSizer,0,wxALIGN_LEFT|wxALIGN_TOP|wxALL,15);

			wxBoxSizer *TagLabelSizer = new wxBoxSizer(wxHORIZONTAL);
			TagLabelSizer->Add(new wxStaticText(LeftPanel,-1,TEXT("Base Tags:"), wxDefaultPosition, wxDefaultSize, 0),0,wxALIGN_LEFT,5);
			SearchType = new wxComboBox( LeftPanel, IDMN_TB_SEARCHTYPE, TEXT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxCB_READONLY);
			SearchType->Append(TEXT("Must Have All Selected"));
			SearchType->Append(TEXT("Can Have Any Selected"));
			SearchType->SetValue(TEXT("Must Have All Selected"));
			TagLabelSizer->Add(SearchType,0,wxALIGN_TOP|wxALIGN_RIGHT,5);
			LeftSizer->Add(TagLabelSizer);
			RequiredTagList = new wxCheckListBox( LeftPanel, IDMN_TB_REQTAGLIST, wxDefaultPosition, wxDefaultSize, 0, wxLB_EXTENDED );
			LeftSizer->Add(RequiredTagList,1,wxALIGN_TOP|wxALIGN_LEFT|wxEXPAND|wxALL,5);

			LeftSizer->Add(new wxStaticText(LeftPanel,-1,TEXT("Additional Tags:"), wxDefaultPosition, wxDefaultSize, 0),0,wxALIGN_TOP|wxALIGN_LEFT,5);
			OptionalTagList = new wxCheckListBox( LeftPanel, IDMN_TB_OPTTAGLIST, wxDefaultPosition, wxDefaultSize, 0, wxLB_EXTENDED );
			LeftSizer->Add(OptionalTagList,1,wxALIGN_TOP|wxALIGN_LEFT|wxEXPAND|wxALL,5);

			wxBoxSizer *BottomSizer = new wxBoxSizer(wxVERTICAL);
			{
				TagResultsText = new wxStaticText(LeftPanel,-1,TEXT("Matching results: 0"), wxDefaultPosition, wxDefaultSize, 0);
				wxBoxSizer *LoadSizer = new wxBoxSizer(wxHORIZONTAL);
				{
					AutoLoad = new wxCheckBox(LeftPanel,IDMN_TB_AUTOLOAD,TEXT("Auto-load"),wxDefaultPosition,wxDefaultSize);
					LoadButton = new wxButton(LeftPanel,IDMN_TB_LOAD,TEXT("Load Results"));
					LoadButton->SetDefault();
					LoadSizer->Add(AutoLoad,0,wxALIGN_LEFT|wxALL|wxALIGN_CENTER_VERTICAL,15);
					LoadSizer->Add(LoadButton,0,wxALIGN_LEFT|wxALL,15);
					AutoLoad->SetValue(FALSE);
					//LoadButton->Disable();
				}
				BottomSizer->Add(TagResultsText,0,wxALIGN_LEFT|wxALIGN_TOP,5);
				BottomSizer->Add(LoadSizer,0,wxALIGN_LEFT|wxALIGN_TOP);
			}
			LeftSizer->Add(BottomSizer,1,wxALIGN_LEFT|wxALL,15);

			LeftPanel->SetSizer(LeftSizer);
			LeftSizer->Fit(LeftPanel);
		}

		// Init the various viewport classes
		ViewportHolder = new WxViewportHolder( SplitterWindow,-1,1);
		Viewport = GEngine->Client->CreateWindowChildViewport(this,(HWND)ViewportHolder->GetHandle());
		Viewport->CaptureJoystickInput(FALSE);
		ViewportHolder->SetViewport( Viewport );
		ViewportHolder->Show();

		// split between tag list and viewport area
		SplitterWindow->SplitVertically( LeftPanel, ViewportHolder, 100 );
		SplitterWindow->SetSashGravity( 0.1 );
		SplitterWindow->SetMinimumPaneSize( 150 );
	}
	// add the base splitter to the auto-sizer
	MainSizer->Add( SplitterWindow, 1, wxEXPAND, 5 );

	//MenuBar = new WxMBGroupBrowser();

	SetSizer( MainSizer );
	MainSizer->Fit(this);

	TagIndexObject = UContentTagIndex::GetLocalContentTagIndex();
	if (TagIndexObject != NULL && TagIndexObject->Tags.Num())
	{
		// fill in the asset types
		for (INT Idx = 0; Idx < TagIndexObject->Tags.Num(); Idx++)
		{
			AssetTypes->Append(*TagIndexObject->Tags(Idx).AssetTypeName);
		}
		// initial update of the tag list based on default selection
		AssetTypes->SetValue(*TagIndexObject->Tags(0).AssetTypeName);
		wxCommandEvent DummyEvt;
		OnAssetTypeChange(DummyEvt);
	}
	else
	{
		AssetTypes->Append(TEXT("No content tags found"));
	}
}

void WxTagBrowser::OnAutoLoadChange(wxCommandEvent &In)
{
	if (AutoLoad->IsChecked())
	{
		LoadButton->Disable();
	}
	else
	{
		LoadButton->Enable();
	}
}

void WxTagBrowser::OnLoadButtonClick(wxCommandEvent &In)
{
	if (IsEnabled())
	{
		INT NumLoaded = 0;
		GWarn->BeginSlowTask(TEXT("Loading assets"),TRUE);
		for (INT AssetIdx = 0; AssetIdx < Assets.Num(); AssetIdx++)
		{
			GWarn->StatusUpdatef( NumLoaded, Assets.Num(), TEXT("Loading assets... %d/%d"), NumLoaded, Assets.Num());
			Assets(AssetIdx).Asset = UObject::StaticLoadObject(UObject::StaticClass(),NULL,*Assets(AssetIdx).AssetName,NULL,0,NULL);
			if (Assets(AssetIdx).Asset != NULL)
			{
				NumLoaded++;
			}
		}
		TagResultsText->SetLabel(*FString::Printf(TEXT("Matching results: %d (%d loaded)"),Assets.Num(),NumLoaded));
		GWarn->EndSlowTask();
		Viewport->Invalidate();
	}
}

/** Import to re-use the same functionality as the generic browser. */
extern void AddTagsToSelectedAssets(USelection *Selection);

void WxTagBrowser::OnAddTag(wxCommandEvent &In)
{
	AddTagsToSelectedAssets(GetSelection());
	// refresh the current assets in case tags were removed
	wxCommandEvent DummyEvt;
	OnTagListChange(DummyEvt);
}

void WxTagBrowser::OnSyncGB(wxCommandEvent &In)
{
	TArray<UObject*> Objects;
	if (GetSelection()->GetSelectedObjects(UObject::StaticClass(),Objects) > 0)
	{
		WxGenericBrowser* GenericBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
		if ( GenericBrowser )
		{
			// update the GB as well as new packages may have been loaded
			GenericBrowser->Update();

			// Make sure the window is visible.  The window needs to be visible *before*
			// the browser is sync'd to objects so that redraws actually happen!
			GUnrealEd->GetBrowserManager()->ShowWindow( GenericBrowser->GetDockID(), TRUE );

			// Sync.
			GenericBrowser->SyncToObjects( Objects );
		}
	}
}

IMPLEMENT_COMPARE_CONSTREF(FName,TagBrowser, { return appStricmp(*A.GetNameString(),*B.GetNameString()); })

void WxTagBrowser::OnAssetTypeChange(wxCommandEvent &In)
{
	// rebuild the tag list
	if (TagIndexObject != NULL)
	{
		// figure out which type is selected
		FString AssetTypeString = AssetTypes->GetValue().c_str();
		if (CurrentAssetTypeString != AssetTypeString)
		{
			CurrentAssetTypeString = AssetTypeString;
			// and fill in the matching tags
			RequiredTagListNames.Empty();
			RequiredTagList->Clear();
			OptionalTagListNames.Empty();
			OptionalTagList->Clear();
			for (INT Idx = 0; Idx < TagIndexObject->Tags.Num(); Idx++)
			{
				if (TagIndexObject->Tags(Idx).AssetTypeName == AssetTypeString)
				{
					TArray<FName> &SelectedTags = TagIndexObject->Tags(Idx).Tags;
					RequiredTagListNames.Append(SelectedTags);
				}
			}
			Sort<USE_COMPARE_CONSTREF(FName,TagBrowser)>(&RequiredTagListNames(0),RequiredTagListNames.Num());
			for (INT TagIdx = 0; TagIdx < RequiredTagListNames.Num(); TagIdx++)
			{
				RequiredTagList->Append(*FString::Printf(TEXT("%s"),*RequiredTagListNames(TagIdx).GetNameString()));
			}
		}
		Assets.Empty();
		Viewport->Invalidate();
		TagResultsText->SetLabel(*FString::Printf(TEXT("Matching results: 0")));
	}
	Update();
}

void WxTagBrowser::OnFilterListChange(wxCommandEvent &In)
{
	// just rebuild the asset list based on the new selection
	UContentTagIndex::AssetMapType AssetTypeMap;
	TagIndexObject->BuildAssetTypeToAssetMap(AssetTypeMap);
	BuildAndLoadAssetList(AssetTypeMap);
}

void WxTagBrowser::BuildAndLoadAssetList(UContentTagIndex::AssetMapType &AssetTypeMap)
{
	UBOOL bAutoLoad = AutoLoad->IsChecked();
	UBOOL bRequireAll = SearchType->GetSelection() == 0;
	GWarn->BeginSlowTask(TEXT("Loading assets"),TRUE);
	TArray<FName> CurrentOptTagListNames;
	TArray<FName> CurrentReqTagListNames;
	UClass *AssetType = NULL;
	GetCurrentAssetAndTagSelections(&AssetType,CurrentReqTagListNames,CurrentOptTagListNames);
	Assets.Empty();
	TMap<FString,TArray<FName> > *AssetMap = AssetTypeMap.Find(AssetType);
	INT NumLoaded = 0, NumToLoad = 0;
	if (AssetMap != NULL)
	{
		for (TMap<FString,TArray<FName> >::TIterator AssetIter(*AssetMap); AssetIter; ++AssetIter)
		{
			TArray<FName> &Tags = AssetIter.Value();
			// make sure it contains one of the required
			UBOOL bHasRequired = !bRequireAll;
			for (TArray<FName>::TIterator TagIter(CurrentReqTagListNames); TagIter; ++TagIter)
			{
				if (Tags.ContainsItem(*TagIter))
				{
					bHasRequired = TRUE;
					if (!bRequireAll)
					{
						// don't need all tags so break now
						break;
					}
				}
				else if (bRequireAll)
				{
					bHasRequired = FALSE;
					break;
				}
			}
			if (bHasRequired)
			{
				// make sure each asset matches at least one filter (unless there are no filters available)
				UBOOL bHasOptional = OptionalTagListNames.Num() == 0;
				for (TArray<FName>::TIterator TagIter(CurrentOptTagListNames); TagIter; ++TagIter)
				{
					if (Tags.ContainsItem(*TagIter))
					{
						bHasOptional = TRUE;
						break;
					}
				}
				if (bHasOptional)
				{
					INT AssetIdx = Assets.AddZeroed();
					Assets(AssetIdx).AssetName = AssetIter.Key();
					NumToLoad++;
				}
			}
		}
	}
	for (INT AssetIdx = 0; AssetIdx < Assets.Num(); AssetIdx++)
	{
		GWarn->StatusUpdatef( NumLoaded, NumToLoad, TEXT("Loading assets... %d/%d"), NumLoaded, NumToLoad);
		if (bAutoLoad)
		{
			Assets(AssetIdx).Asset = UObject::StaticLoadObject(UObject::StaticClass(),NULL,*Assets(AssetIdx).AssetName,NULL,LOAD_Quiet,NULL);
		}
		else
		{
			// do a find as it might already be loaded
			Assets(AssetIdx).Asset = UObject::StaticFindObject(UObject::StaticClass(),NULL,*Assets(AssetIdx).AssetName);
		}
		if (Assets(AssetIdx).Asset != NULL)
		{
			NumLoaded++;
		}
	}
	GWarn->EndSlowTask();
	TagResultsText->SetLabel(*FString::Printf(TEXT("Matching results: %d (%d loaded)"),Assets.Num(),NumLoaded));
	Update();
}

void WxTagBrowser::GetCurrentAssetAndTagSelections(UClass **OutAssetType, TArray<FName> &OutReqTagListNames, TArray<FName> &OutOptionalTagListNames)
{
	if (TagIndexObject != NULL && AssetTypes != NULL)
	{
		FString SelectedAssetType = AssetTypes->GetValue().c_str();
		for (INT Idx = 0; Idx < TagIndexObject->Tags.Num(); Idx++)
		{
			if (TagIndexObject->Tags(Idx).AssetTypeName == SelectedAssetType)
			{
				*OutAssetType = TagIndexObject->Tags(Idx).AssetType;
				break;
			}
		}
	}
	for (UINT Idx = 0; Idx < OptionalTagList->GetCount(); Idx++)
	{
		if (OptionalTagList->IsChecked(Idx))
		{
			OutOptionalTagListNames.AddItem(OptionalTagListNames((INT)Idx));
		}
	}
	for (UINT Idx = 0; Idx < RequiredTagList->GetCount(); Idx++)
	{
		if (RequiredTagList->IsChecked(Idx))
		{
			OutReqTagListNames.AddItem(RequiredTagListNames((INT)Idx));
		}
	}
}

void WxTagBrowser::OnTagListChange(wxCommandEvent &In)
{
	// calculate new selection and load assets
	if (TagIndexObject != NULL)
	{
		// gather the current filter options
		TArray<FName> PrevOptionalTagListNames;
		TArray<FName> CurrentReqTagListNames;
		UClass *AssetType = NULL;
		GetCurrentAssetAndTagSelections(&AssetType,CurrentReqTagListNames,PrevOptionalTagListNames);
		//@note - experimental: grab the unchecked items, as new items will be checked by default
		PrevOptionalTagListNames.Empty();
		for (UINT Idx = 0; Idx < OptionalTagList->GetCount(); Idx++)
		{
			if (!OptionalTagList->IsChecked(Idx))
			{
				PrevOptionalTagListNames.AddItem(OptionalTagListNames((INT)Idx));
			}
		}
		// clear the filter list since we're about to rebuild it
		OptionalTagListNames.Empty();
		OptionalTagList->Clear();
		// build the map of types to assets to tags
		UContentTagIndex::AssetMapType AssetTypeMap;
		TagIndexObject->BuildAssetTypeToAssetMap(AssetTypeMap);
		// get the map for the specified asset type
		TMap<FString,TArray<FName> > *AssetMap = AssetTypeMap.Find(AssetType);
		debugf(TEXT("- searching through asset type %s/%s (%s)"),*AssetType->GetPathName(),AssetTypes->GetValue().c_str(),AssetMap!=NULL?TEXT("valid"):TEXT("invalid"));
		if (AssetMap != NULL)
		{
			// build a list of all assets that contain the base types
			TMap<FString,TArray<FName> *> MatchingAssets;
			for (TMap<FString,TArray<FName> >::TIterator AssetIter(*AssetMap); AssetIter; ++AssetIter)
			{
				TArray<FName> &Tags = AssetIter.Value();
				for (TArray<FName>::TIterator ReqTagIter(CurrentReqTagListNames); ReqTagIter; ++ReqTagIter)
				{
					if (Tags.ContainsItem(*ReqTagIter))
					{
						// add this asset and tags to the matching set
						MatchingAssets.Set(AssetIter.Key(),&Tags);
						break;
					}
				}
			}
			// build the list of filter tags based on the matching set
			for (TMap<FString,TArray<FName> *>::TIterator AssetIter(MatchingAssets); AssetIter; ++AssetIter)
			{
				for (TArray<FName>::TIterator TagIter(*(AssetIter.Value())); TagIter; ++TagIter)
				{
					// filter out the currently selected base tags
					//@note - currently disabled since it would prevent seeing assets w/ only one tag
					if (TRUE || !CurrentReqTagListNames.ContainsItem(*TagIter))
					{
						OptionalTagListNames.AddUniqueItem(*TagIter);
					}
				}
			}
			// fill in the new filter tags
			Sort<USE_COMPARE_CONSTREF(FName,TagBrowser)>(&OptionalTagListNames(0),OptionalTagListNames.Num());
			for (TArray<FName>::TIterator TagIter(OptionalTagListNames); TagIter; ++TagIter)
			{
				UINT Idx = OptionalTagList->Append(*((*TagIter).GetNameString()));
				OptionalTagList->Check(Idx,TRUE);
			}
			// and select any pre-selected filter tags
			for (TArray<FName>::TIterator PrevTagIter(PrevOptionalTagListNames); PrevTagIter; ++PrevTagIter)
			{
				INT TagIdx = OptionalTagListNames.FindItemIndex(*PrevTagIter);
				if (TagIdx != INDEX_NONE)
				{
					OptionalTagList->Check((UINT)TagIdx,FALSE);
				}
			}
		}
		BuildAndLoadAssetList(AssetTypeMap);
	}
	Update();
}

const TCHAR* WxTagBrowser::GetLocalizationKey() const
{
	return TEXT("TagBrowser");
}

void WxTagBrowser::Send(ECallbackEventType Event)
{
	Update();
}

void WxTagBrowser::Update()
{
	if (IsShown() && !GIsPlayInEditorWorld)
	{
		Viewport->Invalidate();
	}
}

/** Responds to size events by updating the splitter. */
void WxTagBrowser::OnSize(wxSizeEvent& In)
{
	// During the creation process a sizing message can be sent so don't
	// handle it until we are initialized
	if ( bAreWindowsInitialized )
	{
		SplitterWindow->SetSize( GetClientRect() );
	}
}

/** Handler for IDM_RefreshBrowser events; updates the browser contents. */
void WxTagBrowser::OnRefresh( wxCommandEvent& In )
{
	if (IsShown())
	{
		Update();
	}
}

void WxTagBrowser::OnScroll(wxScrollEvent& InEvent)
{
	ViewportHolder->ScrollViewport(InEvent.GetPosition(),FALSE);
}

UBOOL WxTagBrowser::InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed/*=1.0f*/,UBOOL bGamepad/*=FALSE*/)
{
	const INT	HitX = Viewport->GetMouseX();
	const INT	HitY = Viewport->GetMouseY();
	HHitProxy*	HitResult = Viewport->GetHitProxy(HitX,HitY);
	UObject*	HitObject = NULL;
	UBOOL bSelectionChanged = FALSE;

	const UBOOL bCtrlDown = Viewport->KeyState(KEY_LeftControl) || Viewport->KeyState(KEY_RightControl);

	if ( Key == KEY_MouseScrollUp )
	{
		ViewportHolder->ScrollViewport( -50 );
		Viewport->InvalidateDisplay();
	}
	else if ( Key == KEY_MouseScrollDown )
	{
		ViewportHolder->ScrollViewport( 50 );
		Viewport->InvalidateDisplay();
	}

	// grab the hit proxy object
	else if( HitResult != NULL ) 
	{
		if (HitResult->IsA(HObject::StaticGetType()))
		{
			HitObject = ((HObject*)HitResult)->Object;
		}
		if ((Event == IE_Pressed || Event == IE_DoubleClick) && (Key == KEY_LeftMouseButton || Key == KEY_RightMouseButton))
		{
			// if clicked on an object then handle that object selection
			if (HitObject != NULL)
			{
				if (Key == KEY_LeftMouseButton && Event == IE_DoubleClick)
				{
					GetSelection()->Select(HitObject);
					wxCommandEvent DummyEvt;
					OnSyncGB(DummyEvt);
				}
				else
				{
					if (HitObject->IsSelected() && Key == KEY_LeftMouseButton)
					{
						GetSelection()->Deselect(HitObject);
					}
					else
					{
						// deselect other objects if ctrl isn't pressed
						if (!bCtrlDown)
						{
							GetSelection()->DeselectAll();
						}
						GetSelection()->Select(HitObject);
					}
					bSelectionChanged = TRUE;
				}
			}
			else
			{
				// deselect other objects if ctrl isn't pressed
				if (!bCtrlDown)
				{
					GetSelection()->DeselectAll();
					bSelectionChanged = TRUE;
				}
			}
		}
		if (Key == KEY_RightMouseButton)
		{
			if (HitObject != NULL)
			{
				WxTagBrowserContextMenu ContextMenu;
				POINT pt;
				GetCursorPos(&pt);
				PopupMenu(&ContextMenu,ScreenToClient(wxPoint(pt.x,pt.y)));
			}
		}
		Viewport->Invalidate();
		if (bSelectionChanged)
		{
			// silently sync the GB to this object
			TArray<UObject*> Objects;
			GetSelection()->GetSelectedObjects(UObject::StaticClass(),Objects);
			WxGenericBrowser* GenericBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
			if ( GenericBrowser )
			{
				// Sync.
				GenericBrowser->SyncToObjects( Objects );
			}
		}
	}
	return TRUE;
}

void WxTagBrowser::Draw(FViewport* Viewport, FCanvas* Canvas)
{
	//debugf(TEXT("Viewport update..."));
	if (TagIndexObject != NULL)
	{
		// Clear the viewport with our background image
		DrawTile(Canvas,0,0,Viewport->GetSizeX(),Viewport->GetSizeY(),0.f,0.f,
			(FLOAT)Viewport->GetSizeX() / (FLOAT)GEditor->Bkgnd->SizeX,
			(FLOAT)Viewport->GetSizeY() / (FLOAT)GEditor->Bkgnd->SizeY,
			FLinearColor::White,GEditor->Bkgnd->Resource);

		// for each reference draw the thumbnail

		// Figure out where in the viewport we are currently positioned
		INT ScrollBarPosition = ViewportHolder->GetScrollThumbPos();
		// Current X & Y draw positions
		INT XPos = STD_TNAIL_PAD_HALF, YPos = STD_TNAIL_PAD_HALF - ScrollBarPosition;
		// This seems to be a good fixed size
		INT FixedSize = 196;
		// The tallest item on a give row
		INT YHighest = 0;
		// Total size drawn so we can adjust the scrollbars properly
		INT SBTotalHeight = 0;
		INT Ignored;
		// Font height for the referencer name
		INT ReferencerFontHeight;
		StringSize(GEngine->MediumFont,Ignored,ReferencerFontHeight,TEXT("X"));

		FMemMark Mark(GMainThreadMemStack);
		for (INT Idx = 0; Idx < Assets.Num(); Idx++)
		{
			UObject *Asset = Assets(Idx).Asset;
			if (Asset != NULL)
			{
				// Get the rendering info for this object
				FThumbnailRenderingInfo* RenderInfo =
					GUnrealEd->GetThumbnailManager()->GetRenderingInfo(Asset);

				if ( RenderInfo != NULL && RenderInfo->Renderer != NULL )
				{
					DWORD MaxLabelX=0, MaxLabelY=0;
					UThumbnailLabelRenderer* LabelRenderer = RenderInfo->LabelRenderer;

					// Setup thumbnail options
					UThumbnailLabelRenderer::ThumbnailOptions ThumbnailOptions;
					{
						// We'll always draw content tags in the tag browser window
						ThumbnailOptions.bShowContentTags = TRUE;
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
	}
}


/**
* Returns the shared selection set for the this browser.
*/
USelection* WxTagBrowser::GetSelection()
{
	if ( Selection == NULL )
	{
		Selection = new( UObject::GetTransientPackage(), TEXT("SelectedAssets"), RF_Transactional ) USelection;
		Selection->AddToRoot();
	}

	return Selection;
}
