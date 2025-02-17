/*=============================================================================
	UnUIEditorWindows.cpp: UI editor window class implementations.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Includes
#include "UnrealEd.h"

#include "EngineUIPrivateClasses.h"
#include "UnrealEdPrivateClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

#include "Properties.h"
#include "UnObjectEditor.h"
#include "UnLinkedObjEditor.h"
#include "Kismet.h"
#include "UnUIEditor.h"
#include "UnUIEditorProperties.h"
#include "UnUIStyleEditor.h"
#include "UIEditorStatusBar.h"
#include "UIDataStoreBrowser.h"
#include "UILayerBrowser.h"
#include "GenericBrowser.h"
#include "ScopedTransaction.h"
#include "UnLinkedObjDrawUtils.h"
#include "BusyCursor.h"
#include "ScopedObjectStateChange.h"

#include "UIDockingPanel.h"

#include "DlgUIListEditor.h"
#include "DlgUIEvent_MetaObject.h"
#include "DlgUISkinEditor.h"
#include "DlgUIEventKeyBindings.h"
#include "DlgUIWidgetEvents.h"
#include "DlgGenericComboEntry.h"
#include "WxDlgAddCalloutButton.h"

#include "UIRenderModifiers.h"
#include "UIWidgetTool.h"
#include "UnEdTran.h"

/* ==========================================================================================================
	FUISelectionTool
========================================================================================================== */
/**
 * Returns whether the user is allowed to move the anchor handle.
 */
UBOOL FUISelectionTool::IsAnchorTransformable( const TArray<UUIObject*>& SelectedObjects ) const
{
	UBOOL bResult = TRUE;

	if ( SelectedObjects.Num() == 1 )
	{
		bResult = SelectedObjects(0)->Rotation.AnchorType == RA_Absolute;
	}

	return bResult;
}

/** Maximum region that is valid for sequence objects to exist in. */
static INT	MaxUISequenceSize = 10000;

/* ==========================================================================================================
	UUISequenceHelper
========================================================================================================== */
IMPLEMENT_CLASS(UUISequenceHelper);

void UUISequenceHelper::ShowContextMenu( class WxKismet* InEditor ) const
{
	InEditor->OpenNewObjectMenu();
}

/* ==========================================================================================================
UUIEvent_MetaObjectHelper
========================================================================================================== */
IMPLEMENT_CLASS(UUIEvent_MetaObjectHelper);

void UUIEvent_MetaObjectHelper::ShowContextMenu( WxKismet* InEditor ) const
{
	
}

void UUIEvent_MetaObjectHelper::OnDoubleClick( const WxKismet* InEditor, USequenceObject* InObject ) const
{
	WxDlgUIEvent_MetaObject ObjectDialog(InEditor, InObject);

	if(ObjectDialog.ShowModal() == wxID_OK)
	{
		// Need to update the tree this way because we may be in the middle of a tree event callback.
		WxKismet* Editor = const_cast<WxKismet*>(InEditor);
		wxCommandEvent Event;
		Event.SetEventType(wxEVT_COMMAND_MENU_SELECTED);
		Event.SetId(IDM_UI_FORCETREEREFRESH);
		Editor->AddPendingEvent(Event);
	}
}

/* ==========================================================================================================
	UUISequenceObjectHelper
========================================================================================================== */
IMPLEMENT_CLASS(UUISequenceObjectHelper);

void UUISequenceObjectHelper::ShowContextMenu( class WxKismet* InEditor ) const
{
	InEditor->OpenObjectOptionsMenu();
}


/* ==========================================================================================================
	USequenceOpHelper_CalloutInputProxy
========================================================================================================== */
IMPLEMENT_CLASS(USequenceOpHelper_CalloutInputProxy);
/**
 * Called when the user double clicks on a sequence object, can be used to display object specific
 * property dialogs.
 *
 * @param InEditor	Pointer to the editor that initiated the callback.
 * @param InObject	Pointer to the object that was clicked on.
 */
void USequenceOpHelper_CalloutInputProxy::OnDoubleClick( const WxKismet* InEditor, USequenceObject* InObject ) const
{
	//@todo ronp
	Super::OnDoubleClick(InEditor, InObject);
}

/**
 * Called when the user right clicks on a sequence object, should show a object specific context menu.
 *
 * @param InEditor	Pointer to the editor that initiated the callback.
 */
void USequenceOpHelper_CalloutInputProxy::ShowContextMenu( WxKismet* InEditor ) const
{
	InEditor->OpenObjectOptionsMenu();
//	Super::ShowContextMenu(InEditor);	//@todo ronp
}

/* ==========================================================================================================
	WxObjectViewportHolder
========================================================================================================== */

BEGIN_EVENT_TABLE(WxObjectViewportHolder, WxViewportHolder)
	EVT_SIZE(OnSize)
END_EVENT_TABLE()

/**
 * Constructor
 * Creates the viewport client and viewport for this ui editor window.
 */
WxObjectViewportHolder::WxObjectViewportHolder( wxWindow* InParent, wxWindowID InID, WxUIEditorBase* InEditor )
: WxViewportHolder(InParent, InID, false, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL), wxScrollHelper(this)  // @UE3: This should be changed for Wx update!
{
	ObjectVC = new FEditorUIObjectViewportClient(InEditor);
	ObjectVC->Viewport = GEngine->Client->CreateWindowChildViewport(ObjectVC, (HWND)GetHandle());
	ObjectVC->Viewport->CaptureJoystickInput(false);

	SetViewport(ObjectVC->Viewport);
}

/**
 * Destructor
 */
WxObjectViewportHolder::~WxObjectViewportHolder()
{
	GEngine->Client->CloseViewport(Viewport);

	SetViewport(NULL);
	delete ObjectVC;
	ObjectVC = NULL;
}


/** Handler for when the pane is resized, updates the viewport. */
void WxObjectViewportHolder::OnSize(wxSizeEvent &Event)
{
	// Make sure to pass the event onto the base class.
	WxViewportHolder::OnSize(Event);

	if( ObjectVC != NULL )
	{
		if ( ObjectVC->UIEditor->OwnerScene != NULL && (ObjectVC->UIEditor->bWindowInitialized==TRUE) )
		{
			wxRect Rect = GetRect();

			ObjectVC->UIEditor->OwnerScene->RequestFormattingUpdate();
			ObjectVC->UIEditor->OwnerScene->RequestSceneUpdate(FALSE,TRUE);
			ObjectVC->UIEditor->OwnerScene->UpdateScene();
		}
	}
}

/* ==========================================================================================================
	WxUIEditorBase
========================================================================================================== */

/**
 * Constructor
 * 
 * @param	InParent	the window that opened this window
 * @param	InId		@todo
 * @param	InScene		the scene to edit
 */

WxUIEditorBase::WxUIEditorBase( wxWindow* InParent, wxWindowID InID, UUISceneManager* inSceneManager, UUIScene* InScene )
: WxTrackableFrame( InParent, InID, TEXT("UISceneEditor"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR)
, FDockingParent(this)
, ObjectVC(NULL)
, ViewportHolder(NULL)
, SceneManager(inSceneManager)
, OwnerScene(InScene)
, EditorOptions(NULL), PositionPanel(NULL), DockingPanel(NULL)
, MenuBar(NULL), MainToolBar(NULL), WidgetToolBar(NULL), StatusBar(NULL)
, PreviewPane(NULL), PropertyWindow(NULL), SceneToolsNotebook(NULL)
, SceneTree(NULL), LayerBrowser(NULL)
, ContainerOutline(NULL), TargetTooltip(NULL), WidgetTool(NULL)
, DragGridMenu(NULL), bWindowInitialized(FALSE), bSynchronizingSelection(FALSE)
{
	BackgroundTexture = LoadObject<UTexture2D>(NULL, TEXT("EditorMaterials.AnimTreeBackground"), NULL, LOAD_None, NULL);

	WinNameString = TEXT("UISceneEditor");
	UpdateTitleBar();

	if ( OwnerScene != NULL )
	{
		for ( TObjectIterator<UObject> It; It; ++It )
		{
			if ( It->IsIn(OwnerScene) )
			{
				It->RemoveFromRoot();
			}
		}
	}
}

/**
 * Destructor
 */
WxUIEditorBase::~WxUIEditorBase()
{
	RemoveViewportModifiers();

	// Save docking layout.
	SaveDockingLayout();

	// Save UI editor options.
	EditorOptions->WindowPosition = GetRect();
	EditorOptions->SaveConfig();
}

/**
 * This function is called when the window has been selected from within the ctrl + tab dialog.
 */
void WxUIEditorBase::OnSelected()
{
	Raise();
}

/**
 * Updates the editor window's titlebar.
 *
 * This version updates the titlebar with the tag of the scene we are currently editing.
 */
void WxUIEditorBase::UpdateTitleBar()
{
	SetTitle( *FString::Printf( LocalizeSecure(Localize(TEXT("UIEditor"), TEXT("SceneEditorTitle_f"), TEXT("UnrealEd")), *OwnerScene->SceneTag.ToString()) ) );
}

void WxUIEditorBase::CreateControls()
{
	//@todo ronp - change editor window to initialize viewport size combo from scene's CurrentviewportSize.
	// create the object that will store the options for this editor window
	EditorOptions = ConstructObject<UUIEditorOptions>(UUIEditorOptions::StaticClass(), GetPackage(), *(OwnerScene->GetTag().ToString() + TEXT("_Options")));
	checkSlow(EditorOptions);

	// Create the viewport client container, which will create the viewport client
	PropertyWindow = new WxPropertyWindow;
	PropertyWindow->Create( this, this );
	PropertyWindow->SetCustomControlSupport(TRUE);

	AddDockingWindow(PropertyWindow, FDockingParent::DH_Right, *LocalizeUI(TEXT("UIEditor_DockTitle_Properties")));

	// Create the position client container which will let the user position widgets.
	PositionPanel = new WxUIPositionPanel(this);
	AddDockingWindow(PositionPanel, FDockingParent::DH_Right, *LocalizeUI(TEXT("UIEditor_DockTitle_Positioning")));

	// Create the docking client container which will let the user dock widgets.
	DockingPanel = new WxUIDockingPanel(this);
	AddDockingWindow((wxWindow*)DockingPanel, FDockingParent::DH_Right, *LocalizeUI(TEXT("UIEditor_DockTitle_Docking")));

	// create the notebook control that will contain the scene tree and style browser
	SceneToolsNotebook = new wxNotebook( this, ID_UI_NOTEBOOK_SCENETOOLS, wxDefaultPosition, wxDefaultSize, wxNB_TOP|wxCLIP_CHILDREN  );
	{
		// Create the layer browser
//		LayerBrowser = new WxUILayerBrowser;
		if(LayerBrowser)
		{
			LayerBrowser->Create(this, SceneToolsNotebook, ID_UI_PANEL_LAYERBROWSER);
		}

		// create the scene browser
		SceneTree = new WxSceneTreeCtrl;
		SceneTree->Create( SceneToolsNotebook, ID_UI_TREE_SCENE, this );
		RefreshLayerBrowser();

		// create the style browser
		StyleBrowser = new WxStyleBrowser();
		StyleBrowser->Create(this,SceneToolsNotebook);

		// fill the notebook control's pages
		SceneToolsNotebook->AddPage(SceneTree, *LocalizeUI(TEXT("UIEditor_Scene")));

		if(LayerBrowser)
		{
			SceneToolsNotebook->AddPage(LayerBrowser, *LocalizeUI(TEXT("UIEditor_Layers")));
		}

		/*
		@todo: This is commented out for now because the resource tree isnt currently implemented but will be later - ASM 3/2/2006

		// Create the resource browser
		ResTree = new WxUIResTreeCtrl;
		ResTree->Create (SceneToolsNotebook, ID_UI_TREE_RESOURCE, this);

		SceneToolsNotebook->AddPage(ResTree, *LocalizeUI(TEXT("UIEditor_Resources")));
		*/

		SceneToolsNotebook->AddPage(StyleBrowser, *LocalizeUI(TEXT("UIEditor_Styles")));

		// Change the notebook's background color to something similar to the rest of windows.
		SceneToolsNotebook->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
	}
	AddDockingWindow(SceneToolsNotebook, FDockingParent::DH_Right, *LocalizeUI(TEXT("UIEditor_DockTitle_SceneTools")));

	// Add the client pane with the viewport in it.
	
	ViewportHolder = new WxObjectViewportHolder( this, -1, this );
	ObjectVC = ViewportHolder->ObjectVC;

	AddDockingWindow( ViewportHolder, DH_None, NULL );

	// Try to load a existing layout for the docking windows.
	LoadDockingLayout();

	// Create the menu bar
	MenuBar = new WxUIEditorMenuBar();
	MenuBar->Create(this);
	SetMenuBar( MenuBar );

	DragGridMenu = new WxUIDragGridMenu;

	// Create status bar
	StatusBar = new WxUIEditorStatusBar();
	StatusBar->Create(this, wxID_ANY);
	SetStatusBar(StatusBar);

	// Create tool bar
	MainToolBar = wxDynamicCast(CreateToolBar( wxTB_FLAT|wxTB_HORIZONTAL, ID_UI_TOOLBAR_MAIN ), WxUIEditorMainToolBar);
	MainToolBar->Realize();
	SetToolBar(MainToolBar);

	Centre();
	SetSize (EditorOptions->WindowPosition);
	Layout();

	// Default to the selection tool.
	SetToolMode(ID_UITOOL_SELECTION_MODE);

	SynchronizeSelectedWidgets(this);

	bWindowInitialized = TRUE;
}



/**
 * Returns the size of the toolbar, if one is attached.  If the toolbar is configured for vertical layout, only the height
 * member will have a value (and vice-versa).
 */
wxSize WxUIEditorBase::GetToolBarDisplacement() const
{
	wxSize Result;
	if ( WidgetToolBar != NULL && WidgetToolBar->IsShown() )
	{
		wxSize ToolBarSize = WidgetToolBar->GetSize();

		LONG ToolbarStyle = WidgetToolBar->GetWindowStyle();
		if ( (ToolbarStyle&wxTB_VERTICAL) != 0 )
		{
			Result.SetWidth( ToolBarSize.GetWidth() );
		}
		else
		{
			Result.SetHeight( ToolBarSize.GetHeight() );
		}
	}

	return Result;
}


/**
 * Determines whether the viewport boundary indicator should be rendered
 * 
 * @return	TRUE if the viewport boundary indicator should be rendered
 */
UBOOL WxUIEditorBase::ShouldRenderViewportOutline() const
{
	UBOOL bResult = TRUE;
	if ( EditorOptions != NULL )
	{
		bResult = EditorOptions->bRenderViewportOutline;
	}

	return bResult;
}

/**
 * Determines whether the container boundary indicator should be rendered
 * 
 * @return	TRUE if the container boundary indicator should be rendered
 */
UBOOL WxUIEditorBase::ShouldRenderContainerOutline() const
{
	UBOOL bResult = TRUE;
	if ( EditorOptions != NULL )
	{
		bResult = EditorOptions->bRenderContainerOutline;
	}

	return bResult;
}

/**
 * Determines whether the selection indicator should be rendered
 * 
 * @return	TRUE if the selection indicator should be rendered
 */
UBOOL WxUIEditorBase::ShouldRenderSelectionHandles() const
{
	UBOOL bResult = TRUE;
	if ( EditorOptions != NULL )
	{
		bResult = EditorOptions->bRenderSelectionHandles;
	}

	return bResult;
}

/* =======================================
	Input methods and notifications
=====================================*/
/**
 * Constructs a UI Widget of the specified type at the specified location with the specified size.
 *
 * @param TypeNum	Type index of the widget to create.
 * @param PosX		X Position of the new widget.
 * @param PosY		Y Position of the new widget.
 * @param SizeX		Width of the new widget.
 * @param SizeY		Height of the new widget.
 * @return			Pointer to the new widget, returns NULL if the operation failed.
 */
UUIObject* WxUIEditorBase::CreateWidget(INT TypeNum, INT PosX, INT PosY, INT SizeX, INT SizeY)
{
	FUIObjectResourceInfo& Info = SceneManager->UIWidgetResources(TypeNum);
	checkSlow(Info.UIResource != NULL);

	FUIWidgetPlacementParameters Parameters
	(
		PosX, PosY, SizeX, SizeY,
		Cast<UUIObject>(Info.UIResource), OwnerScene->GetEditorDefaultParentWidget(), FALSE
	);

	UUIObject *Widget = CreateNewWidget(Parameters);
	return Widget;
}

/**
 * Places a new instance of a widget in the current scene.
 *
 * @param	PlacementParameters		parameters for placing the widget in the scene
 *
 * @return	a pointer to the widget that was created, or NULL if the widget couldn't be placed.
 */
UUIObject* WxUIEditorBase::CreateNewWidget( const FUIWidgetPlacementParameters& PlacementParameters )
{
	check(PlacementParameters.Archetype);
	check(PlacementParameters.Parent);
	
	UUIObject* Result = NULL;

	INT CancelIndex = BeginTransaction(LocalizeUI(TEXT("UIEditor_MenuItem_PlaceWidget")));

	UUIScreenObject* WidgetParent = PlacementParameters.Parent;
	UUIObject* WidgetArchetype = PlacementParameters.Archetype;

	UBOOL bCancelled = !WidgetParent->IsA(WidgetArchetype->GetClass()->ClassWithin);
	if ( !bCancelled )
	{
		// create the new widget
		UUIObject* Widget = WidgetParent->CreateWidget(WidgetParent, WidgetArchetype->GetClass(), WidgetArchetype);
		FScopedObjectStateChange CreateNotification(Widget);


		// if we're supposed to prompt the user for the widget's name, do that now
		UBOOL bRenameWidget = FALSE;
		wxString WidgetNameString = *Widget->GetTag().ToString();

		if ( PlacementParameters.bPromptForWidgetName == TRUE )
		{
			WxNameTextValidator WidgetNameValidator(&WidgetNameString, VALIDATE_Name);
			wxTextEntryDialog Dlg(this, *LocalizeUI(TEXT("DlgRenameWidget_Label")), *LocalizeUI(TEXT("DlgRenameWidget_Title")), *Widget->GetName());
			Dlg.SetTextValidator( WidgetNameValidator );

			if ( Dlg.ShowModal() == wxID_OK )
			{
				bRenameWidget = TRUE;
			}
			else
			{
				UBOOL bWasPublic = Widget->HasAnyFlags(RF_Public);

				// clear the RF_Public flag so that we don't create a redirect
				Widget->ClearFlags(RF_Public);

				// rename the deleted child into the transient package so that we can create new widgets using the same name
				Widget->Rename(NULL, UObject::GetTransientPackage(), REN_ForceNoResetLoaders);

				bCancelled = TRUE;
			}
		}
		else
		{
			Widget->WidgetTag = Widget->GetFName();
		}

		// insert it into the parent's list of children
		if ( !bCancelled )
		{
			FScopedObjectStateChange InsertionNotification(WidgetParent);
			if ( WidgetParent->InsertChild(Widget) != INDEX_NONE )
			{
				if(bRenameWidget)
				{
					RenameObject(Widget, WidgetNameString.c_str());
				}

				// set the widget's position
				if(PlacementParameters.bUseWidgetDefaults == FALSE)
				{
					const FLOAT StartX = PlacementParameters.PosX, StartY = PlacementParameters.PosY;
					const FLOAT EndX = StartX + PlacementParameters.SizeX;
					const FLOAT EndY = StartY + PlacementParameters.SizeY;

					Widget->SetPosition(StartX, StartY, EndX, EndY);
				}

				// now update everything in the scene that needs to know about the new widget
				Selection->Modify();
				Selection->BeginBatchSelectOperation();
				Selection->DeselectAll();
				// the new widget will now be selected
				Selection->Select(Widget, TRUE);

				// immediately update the scene so that when UpdateSelectionRegion is called as a result of EndBatchOperation, positions have
				// been resolved in case we have a potentially recursive docking link
				Widget->RequestSceneUpdate(TRUE, TRUE);
				OwnerScene->UpdateScene();

				Selection->EndBatchSelectOperation();

				// refresh the layer browser so that it displays the new widget
				RefreshLayerBrowser();

				Result = Widget;
			}
			else
			{
				InsertionNotification.CancelEdit();
				bCancelled = TRUE;
			}
		}

		CreateNotification.FinishEdit(bCancelled);
	}

	if ( bCancelled )
	{
		CancelTransaction(CancelIndex);
	}
	else
	{
		// close the undo transaction
		EndTransaction();
	}

	return Result;
}

/**
 * Creates a new instance of a UIPrefab and inserts into the current scene.
 *
 * @param	PlacementParameters		parameters for placing the UIPrefabInstance in the scene
 *
 * @return	a pointer to the UIPrefabInstance that was created, or NULL if the widget couldn't be placed.
 */
UUIPrefabInstance* WxUIEditorBase::InstanceUIPrefab( const FUIWidgetPlacementParameters& PlacementParameters )
{
	check(PlacementParameters.Archetype);
	check(PlacementParameters.Parent);

	UUIPrefabInstance* Result = NULL;

	INT CancelIndex = BeginTransaction(LocalizeUI(TEXT("UIEditor_MenuItem_PlaceWidget")));

	UUIScreenObject* WidgetParent = PlacementParameters.Parent;
	UUIPrefab* SourcePrefab = CastChecked<UUIPrefab>(PlacementParameters.Archetype);

	// If we're supposed to prompt the user for the new UIPrefabInstance's name, do that now
	UBOOL bCancelled = FALSE;
	FName WidgetName = UObject::MakeUniqueObjectName(WidgetParent, UUIPrefabInstance::StaticClass());
	wxString WidgetNameString = *WidgetName.ToString();
	if ( PlacementParameters.bPromptForWidgetName == TRUE )
	{
		while ( !bCancelled )
		{
			WxNameTextValidator WidgetNameValidator(&WidgetNameString, VALIDATE_Name);
			wxTextEntryDialog Dlg(this, *LocalizeUI(TEXT("DlgRenameWidget_Label")), *LocalizeUI(TEXT("DlgRenameWidget_Title")), WidgetNameString);
			Dlg.SetTextValidator( WidgetNameValidator );
			if ( Dlg.ShowModal() == wxID_OK )
			{
				WidgetName = FName(WidgetNameString.c_str());
				if ( WidgetName == NAME_None )
				{
					// show an error dialog, then display the "enter name" dialog again
					wxMessageBox(*LocalizeUI(TEXT("DlgRenameWidget_Error_EmptyName")), *LocalizeUnrealEd("Error"), wxOK | wxICON_ERROR, &Dlg);
				}
				else
				{
					// verify that the entered name is valid
					if ( FIsUniqueObjectName(WidgetName, WidgetParent) )
					{
						break;
					}
					else
					{
						// show an error dialog, then display the "enter name" dialog again
						wxMessageBox(*LocalizeUI(TEXT("DlgRenameWidget_Error_NameInUse")), *LocalizeUnrealEd("Error"), wxOK | wxICON_ERROR, &Dlg);
					}
				}
			}
			else
			{
				bCancelled = TRUE;
			}
		}
	}

	FScopedObjectStateChange ChildAddedNotification(WidgetParent);
	if ( bCancelled == FALSE )
	{
		UUIPrefabInstance* NewInstance = SourcePrefab->InstancePrefab(WidgetParent, FName(WidgetNameString.c_str()));
		if ( NewInstance != NULL && WidgetParent->InsertChild(NewInstance) != INDEX_NONE )
		{
			// set the new UIPrefabInstance's position
			if ( !PlacementParameters.bUseWidgetDefaults )
			{
				const FLOAT StartX = PlacementParameters.PosX, StartY = PlacementParameters.PosY;

				const FLOAT EndX = StartX + PlacementParameters.SizeX;
				const FLOAT EndY = StartY + PlacementParameters.SizeY;

				NewInstance->SetPosition(StartX, StartY, EndX, EndY, EVALPOS_PixelViewport);
			}

			// now update everything in the scene that needs to know about the new widget
			Selection->Modify();
			Selection->BeginBatchSelectOperation();
			{
				Selection->DeselectAll();

				// the UIPrefabInstance should now be selected
				Selection->Select(NewInstance, TRUE);
			}
			Selection->EndBatchSelectOperation();

			// refresh the layer browser so that it displays the new widget
			RefreshLayerBrowser();

			// now refresh the data bindings for all newly created widgets
			{
				TArray<UUIObject*> PrefabChildren = NewInstance->GetChildren(TRUE);
				for ( INT ChildIndex = 0; ChildIndex < PrefabChildren.Num(); ChildIndex++ )
				{
					UUIObject* Widget = PrefabChildren(ChildIndex);
					Widget->ResolveDefaultDataBinding(UCONST_TOOLTIP_BINDING_INDEX);
					Widget->ResolveDefaultDataBinding(UCONST_CONTEXTMENU_BINDING_INDEX);

					// if this child implements the UIDataStoreSubscriber interface, tell the child to load the value from the data store
					IUIDataStoreSubscriber* DataStoreChild = (IUIDataStoreSubscriber*)Widget->GetInterfaceAddress(IUIDataStoreSubscriber::UClassType::StaticClass());
					if ( DataStoreChild != NULL )
					{
						DataStoreChild->RefreshSubscriberValue();
					}
				}
			}


			// close the undo transaction
			EndTransaction();
			Result = NewInstance;
		}
	}

	if ( Result == NULL )
	{
		ChildAddedNotification.CancelEdit();
		CancelTransaction(CancelIndex);
	}

	return Result;
}

/**
 * Uses UUISceneFactoryText to create new widgets based on the contents of the windows clipboard.
 *
 * @param	InParent	the widget that will be the parent for the new widgets.  If no value is specified, whatever is returned from GetEditorDefaultParentWidget()
 *						by the OwnerScene will be used as the parent for the new widgets.
 * @param	bClickPaste	Whether or not the user right clicked to paste the widgets.  If they did, then paste the widgets at the position of the mouse click.
 */
void WxUIEditorBase::PasteWidgets( UUIScreenObject* InParent/*=NULL*/, UBOOL bClickPaste/*=FALSE*/ )
{
	//@fixme ronp uiprefab: not yet supported.
	if ( OwnerScene != NULL && OwnerScene->IsA(UUIPrefabScene::StaticClass()) )
	{
		wxMessageBox(TEXT("This feature not yet supported for UIPrefabs"), TEXT("Error"), wxOK|wxCENTRE, this);
		return;
	}

	INT CancelIndex = BeginTransaction(LocalizeUnrealEd(TEXT("Paste")));
	SynchronizeSelectedWidgets(this);

	if ( InParent == NULL )
	{
		InParent = OwnerScene->GetEditorDefaultParentWidget();
	}

	//@fixme ronp uiprefab: copy/paste ops don't currently propagate to instances
	UBOOL bSuccessfulImport = FALSE;
	{
		FString PasteText = appClipboardPaste();
		const TCHAR* ImportText = *PasteText;

		UUISceneFactoryText* SceneFactory = new UUISceneFactoryText;
		bSuccessfulImport = SceneFactory->FactoryCreateText
		(
			UUIScene::StaticClass(),
			InParent,
			InParent->GetFName(),
			RF_Transactional, NULL,
			TEXT("paste"), 
			ImportText,
			ImportText + PasteText.Len(),
			GWarn
		) != NULL;
	}

	if ( bSuccessfulImport == TRUE )
	{
		// The parent shouldn't be selected after a paste.
		if(InParent && InParent->IsSelected())
		{
			Selection->Deselect(InParent);
		}

		// update the layer browser to reflect the newly added widgets
		RefreshLayerBrowser();

		// widgets are automatically selected when they are pasted into a scene, so make sure all other controls
		// which track selections are up to date
		SynchronizeSelectedWidgets(this);

		
		// If the user right clicked to paste widgets, then move the selected widgets to the location of the mouse click.
		if(bClickPaste == TRUE)
		{
			const FObjectViewportClick Click = ObjectVC->LastClick;
			const FVector &MousePos = Click.GetLocation();

			// Find the top left position of the currently selected widgets.
			FLOAT TopLeft[2] = {0,0};
			TArray<UUIObject*> SelectedWidgets;
			GetSelectedWidgets(SelectedWidgets, FALSE);

			for(INT WidgetIdx = 0; WidgetIdx < SelectedWidgets.Num(); WidgetIdx++)
			{	
				UUIObject* Widget = SelectedWidgets(WidgetIdx);

				// Find out what the top left position of the selection set is.
				if(WidgetIdx == 0)
				{
					TopLeft[0] = Widget->RenderBounds[UIFACE_Left];
					TopLeft[1] = Widget->RenderBounds[UIFACE_Top];
				}
				else
				{
					TopLeft[0] = Min(TopLeft[0], Widget->RenderBounds[UIFACE_Left]);
					TopLeft[1] = Min(TopLeft[1], Widget->RenderBounds[UIFACE_Top]);
				}
			}

			// Offset all of the widget positions.
			FVector Offset(MousePos[0] - TopLeft[0], MousePos[1] - TopLeft[1], 0.0f);

			MoveSelectedObjects(Offset);
		}
		EndTransaction();
	}
	else
	{
		CancelTransaction(CancelIndex);
	}
}

/**
 * Replaces the specified widget with a new widget of the specified class.  The new widget
 * will be selected if the widget being replaced was selected.
 * 
 * @param	WidgetToReplace			the widget to replace; must be a valid widget contained within this editor window
 * @param	ReplacementArchetype	the archetype to use for the replacement widget
 *
 * @return	a pointer to the newly created widget, or NULL if the widget couldn't be replaced
 */
UUIObject* WxUIEditorBase::ReplaceWidget( UUIObject* WidgetToReplace, UUIObject* ReplacementArchetype )
{
	//@fixme ronp uiprefab: not yet supported.
	if ( OwnerScene != NULL && OwnerScene->IsA(UUIPrefabScene::StaticClass()) )
	{
		wxMessageBox(TEXT("This feature not yet supported for UIPrefabs"), TEXT("Error"), wxOK|wxCENTRE, this);
		return NULL;
	}
	UUIObject* Result = NULL;

	INT CancelIndex = BeginTransaction(LocalizeUI(TEXT("TransReplaceWidget")));

	if ( WidgetToReplace != NULL && ReplacementArchetype != NULL )
	{
		UUIScreenObject* WidgetParent = WidgetToReplace->GetParent();
		FScopedObjectStateChange ParentReplacedChildNotification(WidgetParent), WidgetReplacedNotification(WidgetToReplace);
		
		if ( ReplacementArchetype->HasAnyFlags(RF_ArchetypeObject|RF_ClassDefaultObject) )
		{
			// create the new widget
			UUIObject* NewWidget = WidgetParent->CreateWidget(WidgetParent, ReplacementArchetype->GetClass(), ReplacementArchetype);
			if ( NewWidget != NULL )
			{

				INT ChildIndex = WidgetParent->Children.FindItemIndex(WidgetToReplace);
				checkSlow(ChildIndex != INDEX_NONE);

				if ( WidgetParent->ReplaceChild(WidgetToReplace, NewWidget) == TRUE )
				{
					// propagate the information that uniquely identifies this widget
					NewWidget->WidgetTag = WidgetToReplace->WidgetTag;
					NewWidget->WidgetID = WidgetToReplace->WidgetID;

					// propagate other stuff
					NewWidget->Position = WidgetToReplace->Position;
					NewWidget->TabIndex = WidgetToReplace->TabIndex;

					// @todo - request layer browser update

					if ( Selection->IsSelected(WidgetToReplace) )
					{
						Selection->BeginBatchSelectOperation();
						Selection->Select(WidgetToReplace,FALSE);
						Selection->Select(NewWidget,TRUE);
						Selection->EndBatchSelectOperation();
					}

					// now go the owning container's object references and replace any references to the old widget with the new widget
					TMap<UUIObject*,UUIObject*> ReplacementMap;
					ReplacementMap.Set(WidgetToReplace, NewWidget);
					FArchiveReplaceObjectRef<UUIObject> ReplaceAr(OwnerScene, ReplacementMap, FALSE, FALSE, FALSE);

					// everything should be good now - so assign the new widget to Result to indicate success
					Result = NewWidget;
				}
			}
		}
		else
		{
			ParentReplacedChildNotification.CancelEdit();
			WidgetReplacedNotification.CancelEdit();
		}
	}

	if ( Result != NULL )
	{
		EndTransaction();
	}
	else
	{
		CancelTransaction(CancelIndex);
	}

	return Result;
}

/**
 * Find the widget that is the lowest in the hierarchy which is the parent of all widgets in the set.
 *
 * @param	DefaultParent	the widget that is the top-most parent in the scene (usually the scene itself, unless editing a UIPrefab)
 * @param	SourceWidgets	the set of widgets to find a common parent for
 */
UUIScreenObject* WxUIEditorBase::FindCommonParent( UUIScreenObject* DefaultParent, const TArray<UUIObject*>& SourceWidgets )
{
	// first, choose the widget to use as the parent for the new UI archetype instance by finding the common parent of all selected widgets
	UUIScreenObject* BestParent = NULL;

	for ( INT SourceIndex = 0; BestParent != DefaultParent && SourceIndex < SourceWidgets.Num(); SourceIndex++ )
	{
		UUIObject* SourceWidget = SourceWidgets(SourceIndex);

		if ( BestParent == NULL )
		{
			BestParent = SourceWidget->GetParent();
		}
		else if ( !BestParent->ContainsChild(SourceWidget,TRUE) )
		{
			// this widget is not contained by the screen object previously determined as the best choice to be the parent
			// for the widget archetype, so we'll need to iterate up this widget's parent chain until we find a screen object
			// that contains both the BestParent and current widget
			for ( UUIScreenObject* CommonParent = SourceWidget->GetParent(); CommonParent; CommonParent = CommonParent->GetParent() )
			{
				if ( CommonParent == DefaultParent )
				{
					// we've gone as far as we can
					BestParent = CommonParent;
					break;
				}

				if ( CommonParent->ContainsChild(Cast<UUIObject>(BestParent),TRUE) )
				{
					BestParent = CommonParent;
					break;
				}
			}
		}
	}

	return BestParent;
}

/**
 * Creates archetypes for the specified widgets and groups them together into a resource that can be placed into a UIScene 
 * or another UIArchetype as a unit.
 *
 * @param	SourceWidgets	the widgets to create archetypes of
 *
 * @return	TRUE if the archetype was successfully created using the specified widgets.
 */
UBOOL WxUIEditorBase::CreateUIArchetype( TArray<UUIObject*>& SourceWidgets )
{
	// if no widgets were specified, can't create the archetype
	if ( SourceWidgets.Num() == 0 )
	{
		return FALSE;
	}

	// verify that the widgets specified are valid candidates for becoming archetypes
	for ( INT Idx = 0; Idx < SourceWidgets.Num(); Idx++ )
	{
		UUIObject* Widget = SourceWidgets(Idx);

		// if any widget has been deleted and is waiting for garbage collection, can't create an archetype
		if ( Widget->IsPendingKill() )
		{
			return FALSE;
		}

		// if any widget isn't part of a scene or is uninitialized, can't create an archetype
		if (Widget->GetScene() == NULL || !Widget->IsInitialized() )
		{
			return FALSE;
		}

		if ( Widget->IsInPrefabInstance() || Widget->IsAPrefabArchetype() )
		{
			if ( appMsgf(AMT_OKCancel, LocalizeSecure(LocalizeUI(TEXT("UIEditor_Error_NestedPrefabUnsupported")), *Widget->GetFullName())) )
			{
				SourceWidgets.Remove(Idx--);
				continue;
			}
			else
			{
				return FALSE;
			}
		}

		if ( !UUIPrefab::SupportsWidgetType(Widget) )
		{
			if ( appMsgf(AMT_OKCancel, LocalizeSecure(LocalizeUI(TEXT("UIEditor_Error_UnsupportedPrefabType")), *Widget->GetClass()->GetName(), *Widget->GetPathName())) )
			{
				SourceWidgets.Remove(Idx--);
				continue;
			}
			else
			{
				return FALSE;
			}
		}
	}

	UBOOL bSuccess = FALSE;
	TArray<UUIObject*> WidgetsThatCouldNotBeRemoved;
	INT CancelIndex = BeginTransaction(LocalizeUnrealEd(TEXT("Archetype_Create")));

	UUIObject* ArchetypeWidget=NULL;

	UPackage* ArchetypePackage, *ArchetypeOuter;
	FString ArchetypeName;
	// Prompt the user for the package/group/object name to use, and create the package and group, if necessary 
	if ( DisplayCreateArchetypeDialog(ArchetypePackage, ArchetypeOuter, ArchetypeName) == TRUE )
	{
		// Group will be used as the Outer for the archetype wrapper, so if the group is invalid, set it to the package itself
		if ( ArchetypeOuter == NULL )
		{
			ArchetypeOuter = ArchetypePackage;
		}

		// create the wrapper object for this UI archetype
		UUIPrefab* ArchetypeWrapper = ConstructObject<UUIPrefab>(UUIPrefab::StaticClass(), ArchetypeOuter,
			*ArchetypeName, RF_Public|RF_Standalone|RF_Transactional|RF_ArchetypeObject);

		if ( ArchetypeWrapper != NULL )
		{
			// immediately transact this object before we create any references to other objects
			// so that the user will be able to delete it after undoing its creation.
			ArchetypeWrapper->Modify();

			ArchetypeWrapper->Created(NULL);
			ArchetypeWrapper->CreateDefaultStates();
			ArchetypeWrapper->EventProvider->InitializeEventProvider();
			for ( INT StateIndex = 0; StateIndex < ArchetypeWrapper->InactiveStates.Num(); StateIndex++ )
			{
				ArchetypeWrapper->InactiveStates(StateIndex)->Created();
			}

			Selection->Modify();
			Selection->BeginBatchSelectOperation();

			// first, choose the widget to use as the parent for the new UI archetype instance by finding the common parent of all selected widgets
			UUIScreenObject* BestParent = FindCommonParent(OwnerScene->GetEditorDefaultParentWidget(), SourceWidgets);

			// using the widgets that will go into the new UIPrefab, generate a list of ArchetypeInstancePairs; here
			// we'll only fill in the instance portion - CreateWidgetArchetypes will fill in the archetype portion.
			TArray<FArchetypeInstancePair> WidgetPairs;
			WidgetPairs.AddZeroed(SourceWidgets.Num());

			// the bounds for the UI archetype instance will be the region that contains all of the source widgets
			FBox InstanceBounds(0);

			// next, remove the specified widgets from the scene
			for ( INT SourceIndex = 0; SourceIndex < WidgetPairs.Num(); SourceIndex++ )
			{
				UUIObject* SourceWidget = SourceWidgets(SourceIndex);
				UUIScreenObject* CurrentParent = SourceWidget->GetParent();

				FArchetypeInstancePair& Pair = WidgetPairs(SourceIndex);
				Pair.WidgetInstance = SourceWidget;
				for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
				{
					Pair.InstanceBounds[FaceIndex] = SourceWidget->GetPosition(FaceIndex, EVALPOS_PixelViewport, TRUE);
				}

				// remove this widget from the selection set, if necessary
				Selection->Deselect(SourceWidget);

				// save the current state of this object to the transaction buffer
				FScopedObjectStateChange RemovingChildNotification(CurrentParent), RemovedFromParentNotification(SourceWidget);

				InstanceBounds += FVector(Pair.InstanceBounds[UIFACE_Left], Pair.InstanceBounds[UIFACE_Top],0.f);
				InstanceBounds += FVector(Pair.InstanceBounds[UIFACE_Right], Pair.InstanceBounds[UIFACE_Bottom],0.f);

				//@fixme ronp - implement dialog to ask user whether they wish to replace existing widgets with archetype instance
				// then we should only perform this step if they say yes
				if ( !CurrentParent->RemoveChild(SourceWidget, &SourceWidgets) )
				{
					RemovingChildNotification.CancelEdit();
					RemovedFromParentNotification.CancelEdit();
					WidgetsThatCouldNotBeRemoved.AddItem(SourceWidget);

					debugf(NAME_Warning, TEXT("CreateUIArchetype: Failed to remove widget instance '%s' from scene!"), *SourceWidget->GetWidgetPathName());
					//@todo ronp - if we can't remove all children, then mark the operation as failed, add this widget to a separate list, and 
					// present a dialog to the user which tells them that the operation failed and displays the widgets that caused the failure
				}
			}

			if ( WidgetsThatCouldNotBeRemoved.Num() == 0 )
			{
				// just use the absolute pixel values as the basis for the new archetype's bounds
				for ( INT PairIndex = 0; PairIndex < WidgetPairs.Num(); PairIndex++ )
				{
					FArchetypeInstancePair& Pair = WidgetPairs(PairIndex);
					Pair.ArchetypeBounds[UIFACE_Left]	= Pair.InstanceBounds[UIFACE_Left];
					Pair.ArchetypeBounds[UIFACE_Right]	= Pair.InstanceBounds[UIFACE_Right];
					Pair.ArchetypeBounds[UIFACE_Top]	= Pair.InstanceBounds[UIFACE_Top];
					Pair.ArchetypeBounds[UIFACE_Bottom]	= Pair.InstanceBounds[UIFACE_Bottom];
				}

				const FLOAT WrapperWidth = InstanceBounds.Max.X - InstanceBounds.Min.X;
				const FLOAT WrapperHeight = InstanceBounds.Max.Y - InstanceBounds.Min.Y;
				ArchetypeWrapper->OriginalWidth.SetValue(OwnerScene, WrapperWidth);
				ArchetypeWrapper->OriginalHeight.SetValue(OwnerScene, WrapperHeight);
				if ( ArchetypeWrapper->CreateWidgetArchetypes(WidgetPairs, InstanceBounds) == TRUE )
				{
					// now we'll replace the selected widgets with an instance of the archetype
					//@todo - ask the user first, similar to level prefabs

					// create an instance of the archetype we just created
					FUIWidgetPlacementParameters Parameters(
						InstanceBounds.Min.X, InstanceBounds.Min.Y,
						WrapperWidth, WrapperHeight,
						ArchetypeWrapper, BestParent, TRUE
						);

					UUIPrefabInstance* UIArchetypeInstance = InstanceUIPrefab(Parameters);
					if ( UIArchetypeInstance != NULL )
					{
						bSuccess = TRUE;
					}
				}
			}

			Selection->EndBatchSelectOperation();
		}
	}

	if ( bSuccess == TRUE )
	{
		// now update the generic browser so that this wrapper appears in the list
		GCallbackEvent->Send(CALLBACK_RefreshEditor_GenericBrowser);
		EndTransaction();
	}
	else
	{
		EndTransaction();

		// here we actually *apply* the undo transaction so that we restore the state of everything we touched
		const UBOOL bChangedScene = GEditor->UndoTransaction();
		if ( bChangedScene )
		{
			GCallbackEvent->Send( CALLBACK_Undo );
		}
	}

	return bSuccess;
}

/**
 * Creates a new UISkin in the specified package
 *
 * @param	PackageName		the name of the package to create the skin in.  This can be the name of an existing package or a completely new package
 * @param	SkinName		the name to use for the new skin
 * @param	SkinArchetype	the skin to use as the archetype for the new skin
 *
 * @return	a pointer to the newly created UISkin, or NULL if it couldn't be created.
 */
UUISkin* WxUIEditorBase::CreateUISkin( const FString& PackageName, const FString& SkinName, UUISkin* SkinArchetype )
{
	UUISkin* NewSkin = NULL;
	if ( PackageName.Len() && SkinName.Len() )
	{
		FString QualifiedName = PackageName + TEXT(".") + SkinName;

		// should never find an existing skin, since WxDlgCreateSkin validates the name that was entered for uniqueness
		UUISkin* ExistingSkin = FindObject<UUISkin>(ANY_PACKAGE, *QualifiedName);
		if ( ExistingSkin == NULL )
		{
			UPackage* SkinPackage = UObject::CreatePackage(NULL, *PackageName);
			if ( SkinPackage != NULL )
			{
				SkinPackage->MarkPackageDirty();

				// don't specify any skin as the template for this new skin when creating the new skin, as we don't want to inherit all 
				// of the styles from the base skin
				NewSkin = ConstructObject<UUISkin>(UUISkin::StaticClass(), SkinPackage, *SkinName, RF_Public|RF_Transactional|RF_Standalone);
				NewSkin->SetArchetype(SkinArchetype);
				NewSkin->Tag = *SkinName;
			}
		}
		else
		{
			// display a warning to the user that this resource already exists
		}
	}

	return NewSkin;
}

/**
 * Called when the user right-clicks on an empty region of the viewport
 */
void WxUIEditorBase::ShowNewObjectMenu()
{
	SynchronizeSelectedWidgets(this);

	WxUIObjectNewContextMenu menu(this);
	menu.Create( NULL );

	FTrackPopupMenu tpm( this, &menu );
	tpm.Show();
}

/**
 * Called when the user right-clicks on an object in the viewport
 *
 * @param	ClickedObject	the object that was right-clicked on
 */
void WxUIEditorBase::ShowObjectContextMenu( UObject* ClickedObject )
{
	SynchronizeSelectedWidgets(this);

	WxUIObjectOptionsContextMenu menu(this);
	menu.Create( Cast<UUIObject>(ClickedObject) );

	FTrackPopupMenu tpm( this, &menu );
	tpm.Show();
}


/**
 * Called when the user double-clicks an object in the viewport
 *
 * @param	Obj		the object that was double-clicked on
 */
void WxUIEditorBase::DoubleClickedObject(UObject* Obj)
{
	UUIScreenObject* TargetObject = Cast<UUIScreenObject>(Obj);
	if ( TargetObject != NULL )
	{
		if ( ObjectVC->Input->IsAltPressed() )
		{
			// if the user is holding Alt, double-click action is specific to widget type
			//@todo: Implement this using a helper class.
			UUIList* List = Cast<UUIList>(TargetObject);
			if( List != NULL )
			{
				WxDlgUIListEditor Dlg(this, List);
				Dlg.ShowModal();
			}
			else if ( TargetObject->EventProvider != NULL && TargetObject->EventProvider->EventContainer != NULL )
			{
				// otherwise, if there is no special action for this widget, search for an "OpenScene" action and if one is found open the scene
				// that it links to
				TArray<UUIAction_OpenScene*> OpenSceneActions;
				TargetObject->EventProvider->EventContainer->FindSeqObjectsByClass(UUIAction_OpenScene::StaticClass(), ((TArray<USequenceObject*>&)OpenSceneActions), FALSE);
				for ( INT StateIdx = 0; OpenSceneActions.Num() == 0 && StateIdx < TargetObject->InactiveStates.Num(); StateIdx++ )
				{
					UUIState* State = TargetObject->InactiveStates(StateIdx);
					if ( State != NULL && State->StateSequence != NULL )
					{
						State->StateSequence->FindSeqObjectsByClass(UUIAction_OpenScene::StaticClass(), ((TArray<USequenceObject*>&)OpenSceneActions), FALSE);
					}
				}

				for ( INT ActionIdx = 0; ActionIdx < OpenSceneActions.Num(); ActionIdx++ )
				{
					UUIAction_OpenScene* OpenSceneAction = OpenSceneActions(ActionIdx);
					if ( OpenSceneAction->Scene != NULL && SceneManager->OpenScene(OpenSceneAction->Scene) )
					{
						break;
					}
				}
			}
		}
		else
		{
			// otherwise, show the kismet for the widget; callout buttons events are handled
			// at the scene level, so if the widget is a callout button, show the scene's kismet instead
			UUICalloutButtonPanel* ButtonPanel = Cast<UUICalloutButtonPanel>(Obj);
			if ( ButtonPanel == NULL )
			{
				ButtonPanel = Cast<UUICalloutButtonPanel>(Obj->GetOuter());
			}

			if ( ButtonPanel != NULL )
			{
				UUISequence* InputProxyOwnerSequence = NULL;

				// basically we need to find the sequence that contains the input proxy
				UUIEvent_CalloutButtonInputProxy* InputProxy = ButtonPanel->GetCalloutInputProxy(TRUE);
				if ( InputProxy != NULL )
				{
					InputProxyOwnerSequence = Cast<UUISequence>(InputProxy->ParentSequence);
				}

				if ( InputProxyOwnerSequence != NULL )
				{
					WxUIKismetEditor::OpenUISequenceInKismet(InputProxyOwnerSequence, this);

					//@todo - select the enabled state's sequence
				}
			}
			else if ( TargetObject->EventProvider != NULL && TargetObject->EventProvider->EventContainer != NULL )
			{
				WxUIKismetEditor::OpenUISequenceInKismet(TargetObject->EventProvider->EventContainer, this);
			}
		}
	}
}

/**
 * Called when the user left-clicks on an empty region of the viewport
 *
 * @return	TRUE to deselect all selected objects
 */
UBOOL WxUIEditorBase::ClickOnBackground()
{
	UBOOL bResult = TRUE;

	return bResult;
}


/**
 * Called when the user completes a drag operation while objects are selected.
 *
 * @param	Handle	the handle that was used to move the objects; determines whether the objects should be resized or moved.
 * @param	Delta	the amount to move the objects
 * @param   bPreserveProportion		Whether or not we should resize objects while keeping their original proportions.
 */
void WxUIEditorBase::MoveSelectedObjects(ESelectionHandle Handle, const FVector& Delta, UBOOL bPreserveProportion/* =FALSE  */)
{
	switch ( Handle )
	{
	case HANDLE_Outline:
		MoveSelectedObjects(Delta);
		break;

	case HANDLE_Anchor:

		// use anchor offset instead of the delta passed in because anchor offset may be snapped.
		MoveSelectedObjectsAnchor(ObjectVC->Selector->AnchorOffset);
		break;

	default:
		ResizeSelectedObjects(Handle, Delta, bPreserveProportion);
		break;
	}

	UpdatePropertyWindow();
}



/**
 * Called when the user releases the mouse after dragging the selection tool's rotation handle.
 *
 * @param	Anchor			The anchor to rotate the selected objects around.
 * @param	DeltaRotation	the amount of rotation to apply to objects
 */
void WxUIEditorBase::RotateSelectedObjects( const FVector& Anchor, const FRotator& DeltaRotation )
{
	if ( !DeltaRotation.IsZero() )
	{		
		TArray<UUIObject*> SelectedWidgets;
		GetSelectedWidgets(SelectedWidgets);

		INT NumSelectedWidgets = SelectedWidgets.Num();
		if(NumSelectedWidgets)
		{
			FScopedTransaction Transaction(*LocalizeUI("TransRotateWidget"));

			for(INT WidgetIdx = 0; WidgetIdx < NumSelectedWidgets; WidgetIdx++)
			{
				UUIObject* Widget = SelectedWidgets(WidgetIdx);

				// If there are multiple selected widgets then we are rotating about an anchor point other than ours.
				if( NumSelectedWidgets > 1 )
				{
					// have to change the anchor type to RA_Absolute
					Widget->Rotation.AnchorType = RA_Absolute;
					Widget->SetAnchorPosition(Widget->PixelToCanvas(FVector2D(Anchor)));
				}

				if(Widget->IsTransformable())
				{
					FScopedObjectStateChange RotationChangeNotifier(Widget);

					Widget->RotateWidget(DeltaRotation,TRUE);

					// any widgets which are docked (either directly or indirectly) to this one may need to reformat their strings/images
					OwnerScene->RequestFormattingUpdate();
				}
			}
		}
	}

	UpdatePropertyWindow();
}

/**
 * Called when the user releases the mouse after dragging the selection tool's outline handle.
 *
 * @param	Delta	the amount to move the objects
 */
namespace 
{
	struct FWidgetPosition
	{
		UUIObject* Widget;
		FLOAT Position[4];
	};
};

void WxUIEditorBase::MoveSelectedObjects( const FVector& Delta )
{
	if ( !Delta.IsNearlyZero() )
	{
		FScopedTransaction Transaction(*LocalizeUI("TransMoveWidget"));
		
		TArray<UUIObject*> SelectedWidgets;
		GetSelectedWidgets(SelectedWidgets);


		// Create a array with parent widgets before children widgets.
		// We add children to the end of the array using EndIdx and parents to the start of the array using StartIdx
		const INT NumWidgets = SelectedWidgets.Num();		
		TArray<FWidgetPosition> SortedWidgets;
		SortedWidgets.AddZeroed(NumWidgets);

		INT StartIdx = 0;
		INT EndIdx = NumWidgets - 1;
		for( INT WidgetIdx = 0; WidgetIdx < NumWidgets; WidgetIdx++)
		{
			UUIObject* Widget = SelectedWidgets(WidgetIdx);
			UUIObject* Parent = Cast<UUIObject>(Widget->GetParent());
			UBOOL bFoundParent = FALSE;

			while(Parent != NULL)
			{
				if(SelectedWidgets.ContainsItem(Parent) == TRUE)
				{
					bFoundParent = TRUE;
					break;
				}

				Parent = Cast<UUIObject>(Parent->GetParent());
			}

			INT Index;

			if(bFoundParent == FALSE)
			{
				Index = StartIdx;
				StartIdx++;
			}
			else
			{
				Index = EndIdx;
				EndIdx--;
			}

			FWidgetPosition &WidgetPosition = SortedWidgets(Index);
			WidgetPosition.Widget = Widget;
			WidgetPosition.Position[UIFACE_Left] = Widget->GetPosition(UIFACE_Left,EVALPOS_PixelViewport,TRUE);
			WidgetPosition.Position[UIFACE_Top] = Widget->GetPosition(UIFACE_Top,EVALPOS_PixelViewport,TRUE);
			WidgetPosition.Position[UIFACE_Right] = Widget->GetPosition(UIFACE_Right,EVALPOS_PixelViewport,TRUE);
			WidgetPosition.Position[UIFACE_Bottom] = Widget->GetPosition(UIFACE_Bottom,EVALPOS_PixelViewport,TRUE);
		}

		check(StartIdx > EndIdx);

		// Loop through all of the widgets and set their new positions.
		for( INT WidgetIdx = 0; WidgetIdx < NumWidgets; WidgetIdx++)
		{
			FWidgetPosition &WidgetPosition = SortedWidgets(WidgetIdx);
			UUIObject* Widget = WidgetPosition.Widget;

			FVector StartOffset, EndOffset;
			if ( ObjectVC->MouseTool != NULL )
			{
				StartOffset = ObjectVC->MouseTool->GetStartPoint();
				EndOffset = ObjectVC->MouseTool->GetEndPoint();
			}
			else
			{
				const FVector PosVector = Widget->GetPositionVector(TRUE);
				StartOffset = Widget->Project(PosVector);
				EndOffset = Widget->Project(PosVector) + Delta;
			}

			const FLOAT Width = (WidgetPosition.Position[UIFACE_Right] - WidgetPosition.Position[UIFACE_Left]);
			const FLOAT Height = (WidgetPosition.Position[UIFACE_Bottom] - WidgetPosition.Position[UIFACE_Top]);
			const FLOAT AspectRatio = Abs<FLOAT>(Width / Height);

			GetResizeOffsets(HANDLE_Outline, TRUE, AspectRatio, StartOffset, EndOffset, Widget, FALSE);

			if(Widget->IsTransformable())
			{
				FScopedObjectStateChange WidgetPositionNotifier(Widget);
				// In the case of resizing the left or top faces, the anchor point should not move as it will result in unintuitive behavior with rotated widgets.
				FVector2D Anchor(0.0f, 0.0f);
				Anchor.X = Widget->Rotation.AnchorPosition.GetValue(UIORIENT_Horizontal, EVALPOS_PixelOwner, Widget);
				Anchor.Y = Widget->Rotation.AnchorPosition.GetValue(UIORIENT_Vertical, EVALPOS_PixelOwner, Widget);

				Widget->SetPosition(WidgetPosition.Position[UIFACE_Left] + StartOffset.X, static_cast<BYTE>(UIFACE_Left), EVALPOS_PixelViewport,TRUE);
				Widget->SetPosition(WidgetPosition.Position[UIFACE_Top]  + StartOffset.Y, static_cast<BYTE>(UIFACE_Top), EVALPOS_PixelViewport,TRUE);
				Widget->SetPosition(WidgetPosition.Position[UIFACE_Right] + EndOffset.X, static_cast<BYTE>(UIFACE_Right), EVALPOS_PixelViewport,TRUE);
				Widget->SetPosition(WidgetPosition.Position[UIFACE_Bottom] + EndOffset.Y, static_cast<BYTE>(UIFACE_Bottom), EVALPOS_PixelViewport,TRUE);

				Widget->SetAnchorPosition( FVector(Anchor,0.f), EVALPOS_PixelOwner );
			}
		}

		UpdateSelectionRegion();
	}
}

/**
 * Moves the anchor for the selected objects.
 *
 * @param	Delta	Change in the position of the anchor.
 *
 */
void WxUIEditorBase::MoveSelectedObjectsAnchor(const FVector2D& Delta)
{
	if ( !Delta.IsNearlyZero() )
	{
		// If we only have 1 widget selected, then update its anchor position, otherwise just update the rendermodifier's anchor position.
		TArray<UUIObject*> SelectedWidgets;
		GetSelectedWidgets(SelectedWidgets);

		const INT NumWidgets = SelectedWidgets.Num();

		if(NumWidgets == 1)
		{
			UUIObject* Widget = SelectedWidgets(0);
			if ( Widget->Rotation.AnchorType == RA_Absolute )
			{
				FScopedTransaction Transaction(*LocalizeUI("TransMoveAnchor"));
				FScopedObjectStateChange AnchorLocationNotifier(Widget);

				FVector NewAnchorLocation;
				if ( ObjectVC->MouseTool != NULL )
				{
					NewAnchorLocation = (ObjectVC->MouseTool->GetEndPoint() * ObjectVC->Zoom2D) + FVector(ObjectVC->Origin2D, 0.f);
				}
				else
				{
						const FVector PosVector = FVector(
							Widget->GetPosition(UIFACE_Left, EVALPOS_PixelViewport, TRUE),
							Widget->GetPosition(UIFACE_Top, EVALPOS_PixelViewport, TRUE),
							0.f);
					NewAnchorLocation = Widget->Project(PosVector) + FVector(Delta,0.f);
				}

				Widget->SetAnchorPosition( Widget->PixelToCanvas(FVector2D(NewAnchorLocation)) );
				UpdateSelectionRegion(TRUE);
			}
		}
		else
		{
			ObjectVC->Selector->AnchorLocation.X += ObjectVC->Selector->AnchorOffset.X;
			ObjectVC->Selector->AnchorLocation.Y += ObjectVC->Selector->AnchorOffset.Y;
			UpdateSelectionRegion(TRUE);
		}	
	}
}

/**
 * Called when the user drags one of the resize handles of the selection tools.
 *
 * @param	Handle					the handle that was dragged
 * @param	Delta					the amount to move the objects
 * @param   bPreserveProportion		Whether or not we should resize objects while keeping their original proportions.
 */
void WxUIEditorBase::ResizeSelectedObjects( ESelectionHandle Handle, const FVector& Delta, UBOOL bPreserveProportion/*= FALSE*/ )
{
	if ( !Delta.IsNearlyZero() )
	{
		FScopedTransaction Transaction(*LocalizeUI("TransResizeWidget"));

		// Create a array with parent widgets before children widgets.
		// We add children to the end of the array using EndIdx and parents to the start of the array using StartIdx
		TArray<UUIObject*> SelectedWidgets;
		GetSelectedWidgets(SelectedWidgets);

		const INT NumWidgets = SelectedWidgets.Num();		
		TArray<FWidgetPosition> SortedWidgets;
		SortedWidgets.AddZeroed(NumWidgets);

		INT StartIdx = 0;
		INT EndIdx = NumWidgets - 1;
		for( INT WidgetIdx = 0; WidgetIdx < NumWidgets; WidgetIdx++)
		{
			UUIObject* Widget = SelectedWidgets(WidgetIdx);
			UUIObject* Parent = Cast<UUIObject>(Widget->GetParent());
			UBOOL bFoundParent = FALSE;

			while(Parent != NULL)
			{
				if(SelectedWidgets.ContainsItem(Parent) == TRUE)
				{
					bFoundParent = TRUE;
					break;
				}

				Parent = Cast<UUIObject>(Parent->GetParent());
			}

			INT Index;

			if(bFoundParent == FALSE)
			{
				Index = StartIdx;
				StartIdx++;
			}
			else
			{
				Index = EndIdx;
				EndIdx--;
			}

			FWidgetPosition &WidgetPosition = SortedWidgets(Index);
			WidgetPosition.Widget = Widget;
			WidgetPosition.Position[UIFACE_Left] = Widget->GetPosition(UIFACE_Left,EVALPOS_PixelViewport,TRUE);
			WidgetPosition.Position[UIFACE_Top] = Widget->GetPosition(UIFACE_Top,EVALPOS_PixelViewport,TRUE);
			WidgetPosition.Position[UIFACE_Right] = Widget->GetPosition(UIFACE_Right,EVALPOS_PixelViewport,TRUE);
			WidgetPosition.Position[UIFACE_Bottom] = Widget->GetPosition(UIFACE_Bottom,EVALPOS_PixelViewport,TRUE);
		}

		check(StartIdx > EndIdx);

		// Loop through all of the widgets and set their new positions.
		for( INT WidgetIdx = 0; WidgetIdx < NumWidgets; WidgetIdx++)
		{
			FWidgetPosition &WidgetPosition = SortedWidgets(WidgetIdx);
			UUIObject* Widget = WidgetPosition.Widget;

			if(Widget->IsTransformable())
			{
				// Calculate the aspect ratio for cases where we are resizing proportionately to the original widget size. Then calculate the offsets for the sides based on which handle
				// was selected and dragged.
				FVector StartPoint, EndPoint;
				if ( ObjectVC->MouseTool != NULL )
				{
					StartPoint = ObjectVC->MouseTool->GetStartPoint();
					EndPoint = ObjectVC->MouseTool->GetEndPoint();
				}
				else
				{
					const FVector PosVector = Widget->GetPositionVector(TRUE);
					StartPoint = Widget->Project(PosVector);
					EndPoint = Widget->Project(PosVector) + Delta;
#if 0
					const FLOAT Width = (WidgetPosition.Position[UIFACE_Right] - WidgetPosition.Position[UIFACE_Left]);
					const FLOAT Height = (WidgetPosition.Position[UIFACE_Bottom] - WidgetPosition.Position[UIFACE_Top]);
					const FLOAT AspectRatio = Abs<FLOAT>(Width / Height);

					GetResizeOffsets(Handle, bPreserveProportion, AspectRatio, StartOffset, EndOffset, Widget, FALSE);

					StartPoint = StartOffset;
					EndPoint = EndOffset;
#else				
				}

				const FLOAT Width = (WidgetPosition.Position[UIFACE_Right] - WidgetPosition.Position[UIFACE_Left]);
				const FLOAT Height = (WidgetPosition.Position[UIFACE_Bottom] - WidgetPosition.Position[UIFACE_Top]);
				const FLOAT AspectRatio = Abs<FLOAT>(Width / Height);

				FVector StartOffset(StartPoint), EndOffset(EndPoint);
				GetResizeOffsets(Handle, bPreserveProportion, AspectRatio, StartOffset, EndOffset, Widget, Widget->HasTransform(FALSE));
#endif

#if 0
				FVector LocalDirectionVector = Widget->PixelToCanvas(FVector2D(EndPoint)) - Widget->PixelToCanvas(FVector2D(StartPoint));

				FMatrix ScreenToWidget = Widget->GetInverseCanvasToScreen();
				FVector ScreenX = ScreenToWidget.TransformNormal(FVector(1,0,0));
				FVector ScreenY = ScreenToWidget.TransformNormal(FVector(0,-1,0));

				FVector DragDir(0.f);
				switch ( Handle )
				{
				case HANDLE_Left:
				case HANDLE_TopLeft:
				case HANDLE_BottomLeft:
					DragDir.X = -1; 
					break;

				case HANDLE_Right:
				case HANDLE_TopRight:
				case HANDLE_BottomRight:
					DragDir.X = 1;
					break;
				default:
					DragDir.X = 0;
				}
				switch ( Handle )
				{
				case HANDLE_TopLeft:
				case HANDLE_Top:
				case HANDLE_TopRight:
					DragDir.Y = -1;
					break;
				case HANDLE_Bottom:
				case HANDLE_BottomLeft:
				case HANDLE_BottomRight:
					DragDir.Y = 1;
					break;
				default:
					DragDir.Y = 0;
				}

				// Determines the widget edges affected by the X component of the drag.
				FVector DeterminantX = ScreenX * DragDir.X;

				// Determines the widget edges affected by the Y component of the drag.
				FVector DeterminantY = ScreenY * DragDir.Y;

				FVector2D StartOffset(
					(DeterminantX.X < -DELTA || DeterminantY.X < -DELTA ? LocalDirectionVector.X : 0),
					(DeterminantX.Y < -DELTA || DeterminantY.Y < -DELTA ? LocalDirectionVector.Y : 0));

				FVector2D EndOffset(
					(DeterminantX.X > DELTA || DeterminantY.X > DELTA ? LocalDirectionVector.X : 0),
					(DeterminantX.Y > DELTA || DeterminantY.Y > DELTA ? LocalDirectionVector.Y : 0));

#endif

				// In the case of resizing the left or top faces, the anchor point should not move as it will result in unintuitive behavior with rotated widgets.
				FVector2D Anchor(0.0f, 0.0f);
				Anchor.X = Widget->Rotation.AnchorPosition.GetValue(UIORIENT_Horizontal, EVALPOS_PixelViewport, Widget);
				Anchor.Y = Widget->Rotation.AnchorPosition.GetValue(UIORIENT_Vertical, EVALPOS_PixelViewport, Widget);

				FScopedObjectStateChange WidgetSizeChangeNotifier(Widget);

				if ( !Widget->DockTargets.IsDocked(UIFACE_Left) )
				{
					Widget->SetPosition(WidgetPosition.Position[UIFACE_Left] + StartOffset.X, static_cast<BYTE>(UIFACE_Left), EVALPOS_PixelViewport,TRUE);
				}
				if ( !Widget->DockTargets.IsDocked(UIFACE_Top) )
				{
					Widget->SetPosition(WidgetPosition.Position[UIFACE_Top]  + StartOffset.Y, static_cast<BYTE>(UIFACE_Top), EVALPOS_PixelViewport,TRUE);
				}
				if ( !Widget->DockTargets.IsDocked(UIFACE_Right) )
				{
					Widget->SetPosition(WidgetPosition.Position[UIFACE_Right] + EndOffset.X, static_cast<BYTE>(UIFACE_Right), EVALPOS_PixelViewport,TRUE);
				}
				if ( !Widget->DockTargets.IsDocked(UIFACE_Bottom) )
				{
					Widget->SetPosition(WidgetPosition.Position[UIFACE_Bottom] + EndOffset.Y, static_cast<BYTE>(UIFACE_Bottom), EVALPOS_PixelViewport,TRUE);
				}

				Widget->SetAnchorPosition( FVector(Anchor,0.f), EVALPOS_PixelViewport );
			}
		}

		UpdateSelectionRegion();
	}
}

/**
 * Calculates start and end offsets for widget faces when resizing a widget.
 *
 * @param	Handle					the handle that was dragged
 * @param   bPreserveProportion		Whether or not we should resize objects while keeping their original proportions.
 * @param   AspectRatio				The aspect ratio to resize by if preserve proportion is TRUE, should most likely be (Widget Width / Widget Height).
 * @param   OutStart				Offset for the start of the widget box (top left)
 * @param   OutEnd					Offset for the end of the widget box (bottom right)
 * @param	Widget					The widget that the handles modify. Primarily used to access the widget's rotation when calculating movement.
 * @param	bIncludeTransform		specify TRUE to have Delta tranformed using the widget's rotation matrix. Normally, this will be TRUE when getting offsets
 *									for rendering, and FALSE when getting offsets for applying to widgets' positions
 */
void WxUIEditorBase::GetResizeOffsets(ESelectionHandle Handle, UBOOL bPreserveProportion, FLOAT AspectRatio, FVector &OutStart, FVector &OutEnd, UUIObject* Widget/*=NULL*/, UBOOL bIncludeTranform/*=TRUE*/)
{
	FVector StartOffset(0), EndOffset(0);

	if ( Handle != HANDLE_None )
	{
		// remove any deltas that shouldn't be applied to this resize handle
		switch ( Handle )
		{
		case HANDLE_Left:
		case HANDLE_Right:
			OutEnd.Y = OutStart.Y;
			break;

		case HANDLE_Top:
		case HANDLE_Bottom:
			OutEnd.X = OutStart.X;
			break;
		}

	}

	FVector Delta(OutEnd - OutStart);

	if ( Widget != NULL )
	{
		UUIScreenObject* Projector = bIncludeTranform ? Widget : Widget->GetParent();
		
// 		const FVector WidgetCanvasLocation(Widget->GetPositionVector(TRUE));
// 		const FVector2D WidgetScreenLocation(Projector->Project(WidgetCanvasLocation));
// 
// 		const FVector2D DeltaStartScreenLocation(OutStart);
// 		const FVector DeltaStartCanvasLocation(Projector->PixelToCanvas(DeltaStartScreenLocation * ObjectVC->Zoom2D));
// 
// 		const FVector CanvasDeltaOffset = DeltaStartCanvasLocation - WidgetCanvasLocation;
// 		const FVector2D ScreenDeltaOffset = DeltaStartScreenLocation - WidgetScreenLocation;
// 
// 		const FVector2D EndScreenLocation = DeltaStartScreenLocation + FVector2D(Delta);
//  		const FVector EndCanvasLocation = Projector->PixelToCanvas(EndScreenLocation * ObjectVC->Zoom2D);
// 
// 		FVector RawDelta(Delta);
// 		Delta = EndCanvasLocation - WidgetCanvasLocation - CanvasDeltaOffset;
		Delta = Projector->PixelToCanvas(FVector2D(OutEnd * ObjectVC->Zoom2D)) - Projector->PixelToCanvas(FVector2D(OutStart * ObjectVC->Zoom2D));

#if 0
		if ( ObjectVC->Input->IsCtrlPressed() && ObjectVC->Input->IsAltPressed() )
		{
			debugf(TEXT("GetResizeOffsets: WidgetScreenLocation:(%.2f,%.2f)  DeltaStartCanvasLocation:(%.2f,%.2f)   EndScreenLocation:(%.2f,%.2f)  EndCanvasLocation:(%.2f,%.2f)   Delta:(%.2f,%.2f)"),
				WidgetScreenLocation.X, WidgetScreenLocation.Y,
				DeltaStartCanvasLocation.X, DeltaStartCanvasLocation.Y,
				EndScreenLocation.X, EndScreenLocation.Y,
				EndCanvasLocation.X, EndCanvasLocation.Y,
				RawDelta.X, RawDelta.Y);
		}
#endif

	}

	// Calculate how much to offset each face by depending on the selected handles and whether or not we should be resizing proportionally.
	if ( Handle == HANDLE_Outline )
	{
		StartOffset += Delta;
		EndOffset += Delta;
	}
	else if ( Handle != HANDLE_Anchor )
	{
		if ( Widget != NULL )
		{
			UUIScreenObject* Projector = bIncludeTranform ? Widget : Widget->GetParent();
			FMatrix ScreenToWidget = Projector->GetInverseCanvasToScreen();
			FVector ScreenX = ScreenToWidget.TransformNormal(FVector(1,0,0));
			FVector ScreenY = ScreenToWidget.TransformNormal(FVector(0,-1,0));

			FVector DragDir(0.f);
			switch ( Handle )
			{
			case HANDLE_Left:
			case HANDLE_TopLeft:
			case HANDLE_BottomLeft:
				DragDir.X = -1; 
				break;

			case HANDLE_Right:
			case HANDLE_TopRight:
			case HANDLE_BottomRight:
				DragDir.X = 1;
				break;
			default:
				DragDir.X = 0;
			}
			switch ( Handle )
			{
			case HANDLE_TopLeft:
			case HANDLE_Top:
			case HANDLE_TopRight:
				DragDir.Y = -1;
				break;
			case HANDLE_Bottom:
			case HANDLE_BottomLeft:
			case HANDLE_BottomRight:
				DragDir.Y = 1;
				break;
			default:
				DragDir.Y = 0;
			}

			// Determines the widget edges affected by the X component of the drag.
			FVector DeterminantX = ScreenX * DragDir.X;

			// Determines the widget edges affected by the Y component of the drag.
			FVector DeterminantY = ScreenY * DragDir.Y;

			StartOffset.X = (DeterminantX.X < -DELTA || DeterminantY.X < -DELTA ? Delta.X : 0);
			StartOffset.Y = (DeterminantX.Y < -DELTA || DeterminantY.Y < -DELTA ? Delta.Y : 0);
			EndOffset.X = (DeterminantX.X > DELTA || DeterminantY.X > DELTA ? Delta.X : 0);
			EndOffset.Y = (DeterminantX.Y > DELTA || DeterminantY.Y > DELTA ? Delta.Y : 0);
		}
		else
		{
			StartOffset += Delta;
			EndOffset += Delta;
		}

		if ( bPreserveProportion )
		{
			if ( (Handle&HANDLE_Left) != 0 )
			{
				if((Handle&HANDLE_Bottom) != 0)
				{
					EndOffset.Y = -StartOffset.X / AspectRatio;
				}
				else
				{
					StartOffset.Y = StartOffset.X / AspectRatio;
				}
			}
			if ( (Handle&HANDLE_Top) != 0 )
			{
				if((Handle&HANDLE_Right) != 0)
				{
					EndOffset.X = -StartOffset.Y * AspectRatio;
				}
				else
				{
					StartOffset.X = StartOffset.Y * AspectRatio;
				}
			}
			if ( (Handle&HANDLE_Right) != 0 )
			{
				if((Handle&HANDLE_Top) != 0)
				{
					StartOffset.Y = -EndOffset.X / AspectRatio;
				}
				else
				{
					EndOffset.Y = EndOffset.X / AspectRatio;
				}
			}

			if ( (Handle&HANDLE_Bottom) != 0 )
			{
				if((Handle&HANDLE_Left) != 0)
				{
					StartOffset.X = -EndOffset.Y * AspectRatio;
				}
				else
				{
					EndOffset.X = EndOffset.Y * AspectRatio;
				}
			}
		}
	}

	OutStart = StartOffset;
	OutEnd = EndOffset;
}

/**
 * Perform any perform special logic for key presses and override the viewport client's default behavior.
 *
 * @param	Viewport	the viewport processing the input
 * @param	Key			the key that triggered the input event (KEY_LeftMouseButton, etc.)
 * @param	Event		the type of event (press, release, etc.)
 *
 * @return	TRUE to override the viewport client's default handling of this event.
 */
UBOOL WxUIEditorBase::HandleKeyInput(FViewport* Viewport, FName Key, EInputEvent Event)
{
	UBOOL bResult = FALSE;

	//@todo - support for deleting, copying, pasting, etc.

	return bResult;
}

/**
 * Called once when the user mouses over an new object.  Child classes should implement
 * this function if they need to perform any special logic when an object is moused over
 *
 * @param	Obj		the object that is currently under the mouse cursor
 */
//	void WxUIEditorBase::OnMouseOver(UObject* Obj);

/* =======================================
	General methods and notifications
   =====================================*/

/**
* Generates a printable string using the widget name and the current dock face.
*
* @param InWidget		Widget that the dock handle belongs to.
* @param DockFace		The dock handle we are generating a string for.
* @param OutString		Container for the finalized string.
*/
void WxUIEditorBase::GetDockHandleString(const UUIScreenObject* InWidget, EUIWidgetFace DockFace, FString& OutString)
{
	FString FaceName;
	FString WidgetName;

	if(InWidget == NULL)
	{
		InWidget = OwnerScene;
	}

	if(InWidget && DockFace != UIFACE_MAX)
	{
		WidgetName = InWidget->GetTag().ToString();

		switch(DockFace)
		{
		case UIFACE_Top:
			FaceName = LocalizeUI(TEXT("UIEditor_Face_Top"));
			break;
		case UIFACE_Bottom:
			FaceName = LocalizeUI(TEXT("UIEditor_Face_Bottom"));
			break;
		case UIFACE_Left:
			FaceName = LocalizeUI(TEXT("UIEditor_Face_Left"));
			break;
		case UIFACE_Right:
			FaceName = LocalizeUI(TEXT("UIEditor_Face_Right"));
			break;
		default:
			FaceName = LocalizeUI(TEXT("UIEditor_Face_None"));
			break;
		}

		OutString = FString::Printf(LocalizeSecure(LocalizeUI(TEXT("UIEditor_FaceString")), *WidgetName, *FaceName));
	}
	else
	{
		OutString = *LocalizeUI(TEXT("UIEditor_Face_None"));
	}


}

/** @return Returns the viewport size in pixels for the current scene. */
FVector2D WxUIEditorBase::GetViewportSize() const
{
	FVector2D ViewportSize;
	OwnerScene->GetViewportSize(ViewportSize);

	return ViewportSize;
}

/** @return Returns the current grid box size setting in pixels (1,2,4,8, etc.) for the editor window. */
INT WxUIEditorBase::GetGridSize() const
{
	return EditorOptions->GridSize;
}

/** @return Returns the top left origin of the grid in pixels. */
FVector WxUIEditorBase::GetGridOrigin() const
{
	FVector Origin(-HALF_WORLD_MAX, -HALF_WORLD_MAX, 0.0f);

	return Origin;
}

/** @return Returns whether or not snap to grid is enabled. */
UBOOL WxUIEditorBase::GetUseSnapping() const
{
	return EditorOptions->bSnapToGrid;
}

UObject* WxUIEditorBase::GetPackage() const
{
	return OwnerScene->GetOutermost();
}

/**
 * Child classes should implement this function to render the objects that are currently visible.
 */
void WxUIEditorBase::DrawObjects(FViewport* Viewport, FCanvas* Canvas)
{
	if ( OwnerScene != NULL )
	{
		OwnerScene->SceneClient->RenderScenes(Canvas);
	}
}

/**
 * Draw the view background
 */
void WxUIEditorBase::DrawBackground (FViewport* Viewport, FCanvas* Canvas)
{
	Clear(Canvas, GEditor->C_OrthoBackground.ReinterpretAsLinear());

	if (EditorOptions->mViewDrawBkgnd)
	{
		// draw the background texture if specified
		if (BackgroundTexture != NULL)
		{
			FMatrix Ident;
			Ident.SetIdentity();

			Canvas->PushAbsoluteTransform(Ident);
			{
				const INT ViewWidth = ObjectVC->Viewport->GetSizeX();
				const INT ViewHeight = ObjectVC->Viewport->GetSizeY();

				// draw the texture to the side, stretched vertically
				DrawTile( Canvas, ViewWidth - BackgroundTexture->SizeX, 0,
					BackgroundTexture->SizeX, ViewHeight,
					0.f, 0.f,
					1.f, 1.f,
					FLinearColor::White,
					BackgroundTexture->Resource );

				// stretch the left part of the texture to fill the remaining gap
				if (ViewWidth > BackgroundTexture->SizeX)
				{
					DrawTile( Canvas, 0, 0,
						ViewWidth - BackgroundTexture->SizeX, ViewHeight,
						0.f, 0.f,
						0.1f, 0.1f,
						FLinearColor::White,
						BackgroundTexture->Resource );
				}

			}
			Canvas->PopTransform();
		}
	}

	DrawGrid(Viewport, Canvas);
}


namespace
{
	/**
	 * Draws either the vertical or horizontal lines necessary to draw the grid.
	 *
	 * @param Canvas				Drawing canvas to use.
	 * @param ViewportLocX			Viewport X Location
	 * @param ViewportLocY			Viewport Y Location
	 * @param A						Start position for lines.
	 * @param B						End position for lines.
	 * @param AX					Pointer to one of the elements of A, depending on which axis we are drawing.
	 * @param BX					Pointer to one of the elements of B, depending on which axis we are drawing.
	 * @param Axis					Which axis we are drawing to.
	 * @param AlphaCase				Whether we are drawing alphaed lines in this pass or not.
	 * @param SizeX					Horizontal size of the viewport.
	 * @param ProjectionMatrix		The current projection matrix of the canvas.
	 * @param ViewMatrix			The current view matrix of the canvas.
	 * @param BackgroundColor		The current background color of the viewport.
	 */
	static void DrawGridSection(FCanvas* Canvas, INT ViewportLocX,INT ViewportGridY,FVector* A,FVector* B,FLOAT* AX,FLOAT* BX,INT Axis,INT AlphaCase, FLOAT SizeX, const FMatrix &ProjectionMatrix, const FMatrix &ViewMatrix, const FColor &BackgroundColor)
	{
		if( !ViewportGridY )
			return;

		FMatrix	InvViewProjMatrix = ProjectionMatrix.Inverse() * ViewMatrix.Inverse();

		INT	Start = appTrunc(InvViewProjMatrix.TransformFVector(FVector(-1,-1,0.5f)).Component(Axis) / ViewportGridY);
		INT	End   = appTrunc(InvViewProjMatrix.TransformFVector(FVector(+1,+1,0.5f)).Component(Axis) / ViewportGridY);

		if(Start > End)
			Exchange(Start,End);

		FLOAT	Zoom = (1.0f / ProjectionMatrix.M[0][0]) * 2.0f / SizeX;
		INT     Dist  = appTrunc(SizeX * Zoom / ViewportGridY);

		// Figure out alpha interpolator for fading in the grid lines.
		FLOAT Alpha;
		INT IncBits=0;
		if( Dist+Dist >= SizeX/4 )
		{
			while( (Dist>>IncBits) >= SizeX/4 )
				IncBits++;
			Alpha = 2 - 2*(FLOAT)Dist / (FLOAT)((1<<IncBits) * SizeX/4);
		}
		else
			Alpha = 1.0;

		INT iStart  = ::Max<INT>(Start - 1,-HALF_WORLD_MAX/ViewportGridY) >> IncBits;
		INT iEnd    = ::Min<INT>(End + 1,  +HALF_WORLD_MAX/ViewportGridY) >> IncBits;

		for( INT i=iStart; i<iEnd; i++ )
		{
			*AX = (i * ViewportGridY) << IncBits;
			*BX = (i * ViewportGridY) << IncBits;
			if( (i&1) != AlphaCase )
			{
				FLinearColor Background = FColor(BackgroundColor).ReinterpretAsLinear();
				FLinearColor Grid(.5,.5,.5,0);
				FLinearColor Color  = Background + (Grid-Background) * (((i<<IncBits)&7) ? 0.5 : 1.0);
				if( i&1 ) Color = Background + (Color-Background) * Alpha;

				DrawLine(Canvas, *A,*B, FLinearColor(Color.Quantize()));
			}
		}
	}
}

/**
 * Draw the view grid
 */
void WxUIEditorBase::DrawGrid (FViewport* Viewport, FCanvas* Canvas )
{
	const INT GridSize = EditorOptions->GridSize;
	const UBOOL bValidGridSize = GridSize > 0;

	if (EditorOptions->mViewDrawGrid && bValidGridSize)
	{
		FMatrix ViewMatrix;
		FMatrix ProjectionMatrix;

		ViewMatrix = FTranslationMatrix(FVector(ObjectVC->Origin2D.X / ObjectVC->Zoom2D, ObjectVC->Origin2D.Y / ObjectVC->Zoom2D,0));
		ProjectionMatrix = FScaleMatrix(ObjectVC->Zoom2D) * Canvas->GetBottomTransform();

		FVector	Origin = ViewMatrix.Inverse().GetOrigin();
		FVector A,B;

		for( int AlphaCase=0; AlphaCase<=1; AlphaCase++ )
		{
			// Do Y-Axis lines.
			A.Y=+HALF_WORLD_MAX1; A.Z=0.0;
			B.Y=-HALF_WORLD_MAX1; B.Z=0.0;
			DrawGridSection( Canvas, Origin.X, GridSize, &A, &B, &A.X, &B.X, 0, AlphaCase, Viewport->GetSizeX(), ProjectionMatrix, ViewMatrix, ObjectVC->GetBackgroundColor());

			// Do X-Axis lines.
			A.X=+HALF_WORLD_MAX1; A.Z=0.0;
			B.X=-HALF_WORLD_MAX1; B.Z=0.0;
			DrawGridSection( Canvas, Origin.Y, GridSize, &A, &B, &A.Y, &B.Y, 1, AlphaCase, Viewport->GetSizeX(), ProjectionMatrix, ViewMatrix, ObjectVC->GetBackgroundColor());
		}
	}
}

/**
 * Render outlines for selected objects which are being moved / resized.
 */
void WxUIEditorBase::DrawDragSelection( FCanvas* Canvas, FMouseTool_DragSelection* DragTool )
{
	const FColor BorderColor = FColor(180,180,180);
	const INT RotDelta = appTrunc(DragTool->GetRotationDelta() * 65535.f / 360.0f);
	const FVector Delta = DragTool->GetDelta();

	// If the selected handle is an anchor we pass the delta to the rendermodifier and it will change the position of the anchor.
	// otherwise we need to draw the outlines for each of the widgets ourselves.
	if(DragTool->SelectionHandle == HANDLE_Anchor)
	{
		TArray<UUIObject*> SelectedWidgets;
		GetSelectedWidgets(SelectedWidgets);
		if ( SelectedWidgets.Num() == 1 )
		{
			// Calculate the aspect ratio for cases where we are resizing proportionately to the original widget size.
			FVector StartOffset(DragTool->GetStartPoint());
			FVector EndOffset(DragTool->GetEndPoint());

#ifdef _DEBUG
			if ( ObjectVC->Input->IsCtrlPressed()/* && ObjectVC->Input->IsAltPressed()*/ )
			{
				int i = 0;
			}
#endif // _DEBUG
			GetResizeOffsets(HANDLE_Outline, FALSE, 0.0f, StartOffset, EndOffset, SelectedWidgets(0), TRUE);
			ObjectVC->Selector->SetAnchorOffset( FVector2D(StartOffset) );
		}
		else
		{
			ObjectVC->Selector->SetAnchorOffset( FVector2D(Delta) );
		}
	}
	else if(DragTool->SelectionHandle == HANDLE_Rotation)
	{	
		const FVector2D Anchor = ObjectVC->Selector->AnchorLocation;
		const FVector TranslateToOrigin = FVector(Anchor.X , Anchor.Y, 0.0f);
		FTranslationMatrix TransMatrix(-TranslateToOrigin);
		FTranslationMatrix InvTransMatrix(TranslateToOrigin);
		FRotationMatrix RotMatrix(FRotator(0,RotDelta,0));
		
		Canvas->PushRelativeTransform(TransMatrix*RotMatrix*InvTransMatrix);
		{
			TArray<UUIObject*> SelectedWidgets;

			GetSelectedWidgets(SelectedWidgets);

			for ( INT WidgetIdx = 0; WidgetIdx < SelectedWidgets.Num(); WidgetIdx++ )
			{
				UUIObject* Widget = SelectedWidgets(WidgetIdx);

				FMatrix WidgetTransform = Widget->GenerateTransformMatrix();

				Canvas->PushDepthSortKey(appTrunc(Widget->GetZDepth()));
				Canvas->PushRelativeTransform(WidgetTransform);

				FVector2D OutlineStart( Widget->RenderBounds[UIFACE_Left], Widget->RenderBounds[UIFACE_Top] );
				FVector2D OutlineEnd( Widget->RenderBounds[UIFACE_Right], Widget->RenderBounds[UIFACE_Bottom] );

				DrawBox2D( Canvas, OutlineStart, OutlineEnd, BorderColor );

				Canvas->PopTransform();
				Canvas->PopDepthSortKey();
			}
			
		}
		Canvas->PopTransform();
	}
	else
	{
		TArray<UUIObject*> SelectedWidgets;
		GetSelectedWidgets(SelectedWidgets);

		for ( INT WidgetIdx = 0; WidgetIdx < SelectedWidgets.Num(); WidgetIdx++ )
		{
			UUIObject* Widget = SelectedWidgets(WidgetIdx);

			Canvas->PushDepthSortKey(-MAXINT+2);
			Canvas->PushRelativeTransform( Widget->GenerateTransformMatrix());

			// Calculate the aspect ratio for cases where we are resizing proportionately to the original widget size.
			FVector StartPoint(DragTool->GetStartPoint());
			FVector EndPoint(DragTool->GetEndPoint());

			FVector2D OutlineStart(Widget->RenderBounds[UIFACE_Left], Widget->RenderBounds[UIFACE_Top]);
			FVector2D OutlineEnd(Widget->RenderBounds[UIFACE_Right], Widget->RenderBounds[UIFACE_Bottom]);

// 			if ( DragTool->SelectionHandle == HANDLE_Outline )
			{
				FLOAT AspectRatio = Abs<FLOAT>((Widget->RenderBounds[UIFACE_Right] - Widget->RenderBounds[UIFACE_Left]) / (Widget->RenderBounds[UIFACE_Bottom] - Widget->RenderBounds[UIFACE_Top]));
				
#ifdef _DEBUG
				if ( ObjectVC->Input->IsCtrlPressed()/* && ObjectVC->Input->IsAltPressed()*/ )
				{
					int i = 0;
				}
#endif // _DEBUG

				GetResizeOffsets(DragTool->SelectionHandle, DragTool->bPreserveProportion, AspectRatio, StartPoint, EndPoint, Widget, TRUE);
				OutlineStart += FVector2D(StartPoint);
				OutlineEnd += FVector2D(EndPoint);
			}
#if 0
 			else
			{
				FVector LocalDirectionVector = Widget->PixelToCanvas(FVector2D(EndPoint)) - Widget->PixelToCanvas(FVector2D(StartPoint));

				FLOAT AspectRatio = Abs<FLOAT>((Widget->RenderBounds[UIFACE_Right] - Widget->RenderBounds[UIFACE_Left]) / (Widget->RenderBounds[UIFACE_Bottom] - Widget->RenderBounds[UIFACE_Top]));
				GetResizeOffsets(DragTool->SelectionHandle, DragTool->bPreserveProportion, AspectRatio, StartPoint, EndPoint, Widget, TRUE);
				FVector AltDirectionVector = EndPoint - StartPoint;

				FMatrix ScreenToWidget = Widget->GetInverseCanvasToScreen();
				FVector ScreenX = ScreenToWidget.TransformNormal(FVector(1,0,0));
				FVector ScreenY = ScreenToWidget.TransformNormal(FVector(0,-1,0));

				FVector DragDir(0.f);
				switch ( DragTool->SelectionHandle )
				{
				case HANDLE_Left:
				case HANDLE_TopLeft:
				case HANDLE_BottomLeft:
					DragDir.X = -1; 
					break;

				case HANDLE_Right:
				case HANDLE_TopRight:
				case HANDLE_BottomRight:
					DragDir.X = 1;
					break;
				default:
					DragDir.X = 0;
				}
				switch ( DragTool->SelectionHandle )
				{
				case HANDLE_TopLeft:
				case HANDLE_Top:
				case HANDLE_TopRight:
					DragDir.Y = -1;
					break;
				case HANDLE_Bottom:
				case HANDLE_BottomLeft:
				case HANDLE_BottomRight:
					DragDir.Y = 1;
					break;
				default:
					DragDir.Y = 0;
				}

				// Determines the widget edges affected by the X component of the drag.
				FVector DeterminantX = ScreenX * DragDir.X;

				// Determines the widget edges affected by the Y component of the drag.
				FVector DeterminantY = ScreenY * DragDir.Y;

				const FVector2D StartOffset(
					(DeterminantX.X < -DELTA || DeterminantY.X < -DELTA ? LocalDirectionVector.X : 0),
					(DeterminantX.Y < -DELTA || DeterminantY.Y < -DELTA ? LocalDirectionVector.Y : 0));

				const FVector2D EndOffset(
					(DeterminantX.X > DELTA || DeterminantY.X > DELTA ? LocalDirectionVector.X : 0),
					(DeterminantX.Y > DELTA || DeterminantY.Y > DELTA ? LocalDirectionVector.Y : 0));

				OutlineStart += StartOffset;
				OutlineEnd += EndOffset;
			}
#endif

			// first, draw a box that will show the outline of where the widget will end up, including rotation
			DrawBox2D( Canvas, OutlineStart, OutlineEnd, BorderColor );
			Canvas->PopTransform();
			Canvas->PopDepthSortKey();
		}

		// next, draw a box that will show the outline of the bounding region extents for the widget if the widget is rotated
		if ( SelectedWidgets.Num() > 1 || SelectedWidgets(0)->HasTransform(TRUE) )
		{
			FVector2D OutlineStart = ObjectVC->Selector->StartLocation;
			FVector2D OutlineEnd = ObjectVC->Selector->EndLocation;

			const FVector Delta = DragTool->GetDelta();
			if ( (DragTool->SelectionHandle&HANDLE_Left) != 0 )
			{
				OutlineStart.X += Delta.X;
			}
			if ( (DragTool->SelectionHandle&HANDLE_Top) != 0 )
			{
				OutlineStart.Y += Delta.Y;
			}			
			if ( (DragTool->SelectionHandle&HANDLE_Right) != 0 )
			{
				OutlineEnd.X += Delta.X;
			}
			if ( (DragTool->SelectionHandle&HANDLE_Bottom) != 0 )
			{
				OutlineEnd.Y += Delta.Y;
			}
			Canvas->PushDepthSortKey(-MAXINT+2);
			DrawBox2D( Canvas, OutlineStart, OutlineEnd, FColor(160,160,160).ReinterpretAsLinear() );
			Canvas->PopDepthSortKey();
		}
	}
}

/**
 * Called when the viewable region is changed, such as when the user pans or zooms the workspace.
 */
void WxUIEditorBase::ViewPosChanged()
{
	if ( OwnerScene != NULL && OwnerScene->SceneClient != NULL )
	{
		OwnerScene->SceneClient->UpdateCanvasToScreen();
		UpdateSelectionRegion(TRUE);
	}
}

/**
 * Repopulates the layer browser with the widgets in the scene.
 */
void WxUIEditorBase::RefreshLayerBrowser()
{
	if(LayerBrowser)
	{
		LayerBrowser->Refresh();
	}

	SceneTree->RefreshTree();
}

/**
 * Updates any UIPrefabInstances contained by this editor's scene.  Called when the scene is opened for edit.
 */
void WxUIEditorBase::UpdatePrefabInstances()
{
	if ( OwnerScene != NULL )
	{
		TArray<UUIObject*> SceneChildren;
		OwnerScene->GetChildren(SceneChildren, TRUE);

		TArray<UUIObject*> PrefabInstances;
		if ( ContainsObjectOfClass<UUIObject>(SceneChildren, UUIPrefabInstance::StaticClass(), FALSE, &PrefabInstances) )
		{
			UBOOL bUpdatedPrefabInstance = FALSE;

			FScopedTransaction Transaction(*LocalizeUI(TEXT("TransUpdateUIPrefabInstance")));
			for ( INT Index = 0; Index < PrefabInstances.Num(); Index++ )
			{
				UUIPrefabInstance* PrefInst = Cast<UUIPrefabInstance>(PrefabInstances(Index));
				if ( PrefInst->SourcePrefab != NULL )
				{
					if ( PrefInst->PrefabInstanceVersion < PrefInst->SourcePrefab->PrefabVersion )
					{
						FScopedObjectStateChange InstanceUpdateNotification(PrefInst);
						PrefInst->UpdateUIPrefabInstance();
						bUpdatedPrefabInstance = TRUE;
					}
				}
				else
				{
					// this prefab's source prefab couldn't be found.  This normally occurs when the user creates a new prefab but doesn't
					// save the package containing the new prefab.  Nothing else to do but "unprefab" it.
					PrefInst->DetachFromSourcePrefab();
					bUpdatedPrefabInstance = TRUE;
				}
			}

			if ( !bUpdatedPrefabInstance )
			{
				Transaction.Cancel();
			}
		}
	}
}

/**
 * Called when items within the Main Tool Bar need to be refreshed as a result
 * of the referenced data being changed.
 */
void WxUIEditorBase::UpdateMainToolBar()
{
	if(MainToolBar != NULL)
	{
		MainToolBar->UpdateWidgetPreviewStates();
	}
}

/**
 * Called when something happens in the linked object drawing that invalidates the property window's
 * current values, such as selecting a new object.
 * Child classes should implement this function to update any property windows which are being displayed.
 */
void WxUIEditorBase::UpdatePropertyWindow()
{
	TArray<UUIObject*> SelectedWidgets;

	if( HaveObjectsSelected() )
	{
		GetSelectedWidgets(SelectedWidgets);

		PropertyWindow->SetObjectArray( SelectedWidgets, TRUE, TRUE, TRUE );
		PositionPanel->SetObjectArray(SelectedWidgets);
		DockingPanel->SetSelectedWidgets(SelectedWidgets);
	}
	else
	{
		UUIScreenObject* DefaultWidget = OwnerScene->GetEditorDefaultParentWidget();
		PropertyWindow->SetObject( DefaultWidget, TRUE, TRUE, TRUE );
		PositionPanel->SetObject(DefaultWidget);
		DockingPanel->SetSelectedWidgets(SelectedWidgets);
	}
}

/**
 *	Refreshes only the values of the positioning panel, should NOT be used if the selection set has changed (Use UpdatePropertyWindow instead).
 */
void WxUIEditorBase::RefreshPositionPanelValues()
{
	PositionPanel->RefreshControls();
}

/**
 *	Refreshes only the values of the docking panel, should NOT be used if the selection set has changed (Use UpdatePropertyWindow instead).
 */
void WxUIEditorBase::RefreshDockingPanelValues()
{
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets, FALSE);

	DockingPanel->SetSelectedWidgets(SelectedWidgets);
}

/**
 * Displays the skin editor dialog.
 */
void WxUIEditorBase::ShowSkinEditor()
{
	check(SceneManager->ActiveSkin != NULL);

	WxDlgUISkinEditor* Dialog = new WxDlgUISkinEditor;
	Dialog->Create(GApp->EditorFrame, SceneManager->ActiveSkin);
	Dialog->Show();
}


/**
 * Begins an undo transaction.
 *
 * @param	TransactionLabel	the label to place in the Undo menu
 *
 * @return	the value of UTransactor::Begin()
 */
INT WxUIEditorBase::BeginTransaction( FString TransactionLabel )
{
	return GEditor->BeginTransaction( *TransactionLabel );
}

/**
 * Cancels an active transaction
 *
 * @param	StartIndex	the index of the transaction to cancel. All transaction in the buffers after this index will be removed and
 *						object states will be returned to this point.
 */
void WxUIEditorBase::CancelTransaction( INT StartIndex )
{
	//@todo ronp undo: hmmm, what could be really neat is if we had CancelTransaction end the transaction and attempt to perform and Undo...that way
	// anything that was modified would be undone
	GEditor->CancelTransaction(StartIndex);
}

/**
 * Ends an undo transaction.  Should be called when you are done modifying the objects.
 */
void WxUIEditorBase::EndTransaction()
{
	GEditor->EndTransaction();
}

/**
 * Called immediately after the window is created, prior to the first call to Show()
 */
void WxUIEditorBase::InitEditor()
{
	// make sure all positions and docking relationships are initialized
	OwnerScene->RequestSceneUpdate(TRUE,TRUE);

	// initialize the scene
	OwnerScene->Initialize(NULL,NULL);
	OwnerScene->eventPostInitialize();

	// create all controls and helper objects
	CreateControls();
	ObjectVC->InitializeClient();

	// activate the scene
	OwnerScene->Activate();
	OwnerScene->OnSceneActivated(TRUE);

	UpdatePropertyWindow();
	RefreshLayerBrowser();

	RegisterCallbacks();

	CreateViewportModifiers();

	// update any prefab instances contained in this scene
	UpdatePrefabInstances();
}

/**
 * Register any callbacks this editor would like to receive
 */
void WxUIEditorBase::RegisterCallbacks()
{
	GCallbackEvent->Register(CALLBACK_SelChange,this);
	GCallbackEvent->Register(CALLBACK_UIEditor_StyleModified, this);
	GCallbackEvent->Register(CALLBACK_UIEditor_RefreshScenePositions, this);
	GCallbackEvent->Register(CALLBACK_UIEditor_SceneRenamed, this);
	GCallbackEvent->Register(CALLBACK_Undo, this);
	GCallbackEvent->Register(CALLBACK_UIEditor_PreviewPlatformChanged, this);
}


/**
 * Create any viewport modifiers for this editor window.  Called when the scene is opened.
 */
void WxUIEditorBase::CreateViewportModifiers()
{
	check(ObjectVC);

	const INT InsertIndex = ObjectVC->RenderModifiers.FindItemIndex(ObjectVC->Selector);
	ContainerOutline = new FRenderModifier_ContainerOutline(this);
	ObjectVC->AddRenderModifier(ContainerOutline, InsertIndex);

	TargetTooltip = new FRenderModifier_TargetTooltip(this);
	ObjectVC->AddRenderModifier(TargetTooltip);
}

/**
 * Remove any viewport modifiers that were added by this editor.
 */
void WxUIEditorBase::RemoveViewportModifiers()
{
	check(ObjectVC);

	ObjectVC->RemoveRenderModifier(ContainerOutline);
	delete ContainerOutline;
	ContainerOutline = NULL;

	ObjectVC->RemoveRenderModifier(TargetTooltip);
	delete TargetTooltip;
	TargetTooltip = NULL;
}


/* =======================================
	Selection methods and notifications
   =====================================*/

static void ConstrainRegion( FBox& Region, FViewport* Viewport )
{
	Region.Min.X = Max( Region.Min.X, 0.f );
	Region.Min.Y = Max( Region.Min.Y, 0.f );

	Region.Max.X = Min<FLOAT>( Region.Max.X, Viewport->GetSizeX() );
	Region.Max.Y = Min<FLOAT>( Region.Max.Y, Viewport->GetSizeY() );
}

/**
 * Selects all objects within the specified region and notifies the viewport client so that it can
 * reposition the selection overlay.
 */
UBOOL WxUIEditorBase::BoxSelect( FBox NewSelectionRegion )
{
	TArray<UObject*> NewSelection;
	TArray<UUIObject*> Children = OwnerScene->GetChildren( TRUE );

	for(INT ChildIdx = 0; ChildIdx < Children.Num(); ChildIdx++)
	{
		UUIObject* ChildWidget = Children(ChildIdx);
		FBox ChildBox(0);

		ChildBox += FVector(ChildWidget->RenderBounds[UIFACE_Left], ChildWidget->RenderBounds[UIFACE_Top], -10);
		ChildBox += FVector(ChildWidget->RenderBounds[UIFACE_Right], ChildWidget->RenderBounds[UIFACE_Bottom], 10);

		const UBOOL bUnderMouse = NewSelectionRegion.Intersect(ChildBox);

		if( bUnderMouse )
		{
			NewSelection.AddUniqueItem( ChildWidget );
		}
	}


	UBOOL bSelectionChanged = !Selection->CompareWithSelection<UObject>(NewSelection);

	if ( NewSelection.Num() > 0 && bSelectionChanged )
	{
		Selection->BeginBatchSelectOperation();

		// Iterate over array adding each to selection.
		for(INT i=0; i<NewSelection.Num(); i++)
		{
			AddToSelection( NewSelection(i) );
		}

		Selection->EndBatchSelectOperation();
	}

	return bSelectionChanged;
}


/**
 * Called when the selected widgets are changed.
 */
void WxUIEditorBase::SelectionChangeNotify()
{
	if(Selection->IsBatchSelecting() == FALSE)
	{
		TArray<UUIObject*> SelectedWidgets;
		GetSelectedWidgets(SelectedWidgets);

		// Check to see if a widget that is a member of another widget is selected.
		// If so, propogate the selection to its owner.
		for( INT i = 0; i < SelectedWidgets.Num( ); ++i )
		{
			UUIObject * Widget = SelectedWidgets(i);

			if( Widget && Widget->IsPrivateBehaviorSet( UCONST_PRIVATE_NotEditorSelectable ) )
			{
				RemoveFromSelection( Widget );
				AddToSelection( Widget->GetParent( ) );
			}
		}

		ObjectVC->UpdateWidgetRenderModifiers();

		UpdateSelectionRegion(TRUE);

		UpdatePropertyWindow();

		UpdateMainToolBar();

		SynchronizeSelectedWidgets(this);
	}
}

/**
 * Calculates and updates the size of the selection handle to contain all selected objects.
 *
 * @param	bClearAnchorOffset		Whether or not to clear the current anchor offset(should be TRUE when the selection set changes).
 */
void WxUIEditorBase::UpdateSelectionRegion(UBOOL bClearAnchorOffset)
{
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);
	
	// Loop through all of our widgets and create a bounding box using their render bounds.
	// Then update the ObjectSelector's region using the bounding box's extents.
	const INT NumWidgets = SelectedWidgets.Num();

	if(NumWidgets)
	{
		FVector2D UpperLeft(MAX_FLT,MAX_FLT);
		FVector2D LowerRight(0,0);

		for ( INT WidgetIdx = 0; WidgetIdx < NumWidgets; WidgetIdx++ )
		{
			UUIObject* Widget = SelectedWidgets(WidgetIdx);

			FLOAT MinX = 0.0f, MaxX = 0.0f, MinY = 0.0f, MaxY = 0.0f;

			Widget->GetPositionExtents( MinX, MaxX, MinY, MaxY, TRUE, TRUE );

			UpperLeft.X = Min( UpperLeft.X, MinX );
			UpperLeft.Y = Min( UpperLeft.Y, MinY );

			LowerRight.X = Max( LowerRight.X, MaxX );
			LowerRight.Y = Max( LowerRight.Y, MaxY );
		}

		// If there is only 1 widget selected, then set the anchor position of the selection region to be the same position
		// as the widget's anchor (Upperleft corner of the widget + Anchor).  Otherwise, set it to be the center of the selection region.
		if(NumWidgets == 1)
		{
			UUIObject* Widget = SelectedWidgets(0);

			// GetAnchorPosition() returns a value relative to the widget's position, so include that as well
			ObjectVC->Selector->AnchorLocation = FVector2D(Widget->GetAnchorPosition(FALSE));
		}
		else
		{
			ObjectVC->Selector->AnchorLocation = (LowerRight + UpperLeft) * 0.5f;
		}

		if ( bClearAnchorOffset )
		{
			ObjectVC->Selector->AnchorOffset = FVector2D(0,0);
		}
		
		ObjectVC->Selector->SetPosition(UpperLeft, LowerRight);
		ObjectVC->RefreshViewport();
	}
}

/**
 * Returns the number of selected objects
 */
INT WxUIEditorBase::GetNumSelected() const
{
	return Selection->CountSelections<UUIObject>(TRUE);
}

// FCallbackEventDevice interface
void WxUIEditorBase::Send( ECallbackEventType InType )
{
	switch ( InType )
	{
	case CALLBACK_SelChange:
		SelectionChangeNotify();
		break;

	case CALLBACK_UIEditor_RefreshScenePositions:
		{
			UBOOL bUpdateSelection = FALSE;
			TArray<UUIObject*> SelectedWidgets;
			GetSelectedWidgets(SelectedWidgets, TRUE);
			for ( INT SelIndex = SelectedWidgets.Num() - 1; SelIndex >= 0; SelIndex-- )
			{
				if ( !OwnerScene->ContainsChild(SelectedWidgets(SelIndex)) )
				{
					bUpdateSelection = TRUE;
					SelectedWidgets.Remove(SelIndex);
				}
			}

			if ( bUpdateSelection )
			{
				SetSelectedWidgets(SelectedWidgets);
			}

			UpdateSelectionRegion();
			RefreshPositionPanelValues();
			RefreshDockingPanelValues();
		}
		break;

	case CALLBACK_UIEditor_ModifiedRenderOrder:
		UpdateSelectionRegion();
		break;

	case CALLBACK_Undo:
		OwnerScene->RequestFormattingUpdate();
		OwnerScene->RequestSceneUpdate(TRUE,TRUE,TRUE,TRUE);
		OwnerScene->RequestPrimitiveReview(TRUE, TRUE);
		RefreshLayerBrowser();

		// no need to call SelectionChangeNotify() here, as the transaction buffer will send a CALLBACK_SelChange event
		break;
	}
}

void WxUIEditorBase::Send( ECallbackEventType InType, UObject* InObject )
{
	switch ( InType )
	{
	case CALLBACK_SelectObject:
		// don't handle this callback, as it is never batched (called once for each individual selection action).  We handle
		// CALLBACK_SelChanged instead.
		break;

	case CALLBACK_UIEditor_StyleModified:
		NotifyStyleModified(Cast<UUIStyle>(InObject));
		break;

	case CALLBACK_UIEditor_RefreshScenePositions:
		{
			UBOOL bUpdateSelection = FALSE;
			TArray<UUIObject*> SelectedWidgets;
			GetSelectedWidgets(SelectedWidgets, TRUE);
			for ( INT SelIndex = SelectedWidgets.Num() - 1; SelIndex >= 0; SelIndex-- )
			{
				if ( !OwnerScene->ContainsChild(SelectedWidgets(SelIndex)) )
				{
					bUpdateSelection = TRUE;
					SelectedWidgets.Remove(SelIndex);
				}
			}

			if ( bUpdateSelection )
			{
				SetSelectedWidgets(SelectedWidgets);
			}

			UpdateSelectionRegion();
			RefreshPositionPanelValues();
			RefreshDockingPanelValues();
		}
		break;

	case CALLBACK_UIEditor_ModifiedRenderOrder:
		UpdateSelectionRegion();
		break;

	case CALLBACK_UIEditor_SceneRenamed:
		if(InObject == OwnerScene)
		{
			UpdatePropertyWindow();
			SceneTree->RefreshTree();
			UpdateTitleBar();
		}
		break;
	}
}

void WxUIEditorBase::Send( FCallbackEventParameters& Parms )
{
	switch ( Parms.EventType )
	{
	case CALLBACK_UIEditor_PreviewPlatformChanged:
		{
			if ( Parms.Sender == this )
			{
				if ( EditorOptions != NULL && EditorOptions->CurrentPreviewPlatform != Parms.EventString )
				{
					EditorOptions->CurrentPreviewPlatform = Parms.EventString;
				}

				if ( OwnerScene != NULL )
				{
					UUIDataStore_InputAlias* AliasDS = UUICalloutButton::StaticClass()->GetDefaultObject<UUICalloutButton>()->GetCalloutDataStore();
					if ( AliasDS != NULL )
					{
						AliasDS->eventRefreshSubscribers();
					}
				}
			}
			else
			{
				SetPreviewPlatform(Parms.EventString);
			}
			break;
		}
	}
}

void WxUIEditorBase::NotifyPostChange( void* Src, FEditPropertyChain* PropertyThatChanged )
{
	if ( PropertyThatChanged->Num() > 0 )
	{
		// this represents the inner-most property that the user modified
		UProperty* ModifiedProperty = PropertyThatChanged->GetTail()->GetValue();
		if ( ModifiedProperty != NULL )
		{
			FName PropertyName = ModifiedProperty->GetFName();
			if ( PropertyName == TEXT("AnchorType") )
			{
				// user changed the anchor type for the selected widget - update the anchor location
				UpdateSelectionRegion(FALSE);
			}
		}
	}
}

// FSeralizableObject interface.

/**
 * Method for serializing UObject references that must be kept around through GC's.
 *
 * @param Ar The archive to serialize with
 */
void WxUIEditorBase::Serialize(FArchive& Ar)
{
	Ar << SceneManager << OwnerScene << EditorOptions << SelectStyleMenuMap << EditStyleMenuMap << SetDataStoreBindingMap << ClearDataStoreBindingMap;
}

/**
 *	This function returns the name of the docking parent.  This name is used for saving and loading the layout files.
 *  @return A string representing a name to use for this docking parent.
 */
const TCHAR* WxUIEditorBase::GetDockingParentName() const
{
	return TEXT("UIEditor");
}

/**
 * @return The current version of the docking parent, this value needs to be increased every time new docking windows are added or removed.
 */
const INT WxUIEditorBase::GetDockingParentVersion() const
{
	return 0;
}

/**
 * Retrieves the widgets corresonding to the selected items in the tree control
 *
 * @param	bIgnoreChildrenOfSelected	if TRUE, widgets will not be added to the list if their parent is in the list
 */
void WxUIEditorBase::GetSelectedWidgets( TArray<UUIObject*> &OutWidgets, UBOOL bIgnoreChildrenOfSelected/*=FALSE*/ )
{
	OutWidgets.Empty();

	Selection->GetSelectedObjects<UUIObject>(OutWidgets);
	if ( bIgnoreChildrenOfSelected )
	{
		// purge any widgets that have parents in the selection set
		for ( INT ChildIndex = OutWidgets.Num() - 1; ChildIndex >= 0; ChildIndex-- )
		{
			UUIObject* CurrentWidget = OutWidgets(ChildIndex);
			for ( INT TestIndex = OutWidgets.Num() - 1; TestIndex >= 0; TestIndex-- )
			{
				UUIObject* PossibleParent = OutWidgets(TestIndex);
				if ( PossibleParent == CurrentWidget )
				{
					continue;
				}

				if ( CurrentWidget->IsContainedBy(PossibleParent) )
				{
					OutWidgets.Remove(ChildIndex);
					break;
				}
			}
		}
	}

	// finally, remove any pending kill widgets from the list
	for ( INT SelectedIndex = OutWidgets.Num() - 1; SelectedIndex >= 0; SelectedIndex-- )
	{
		if ( OutWidgets(SelectedIndex)->IsPendingKill() )
		{
			OutWidgets.Remove(SelectedIndex);
		}
	}
}

/**
 * Changes the selection set for this FSelectionInterface to the widgets specified.
 *
 * @return	TRUE if the selection set was accepted
 */
UBOOL WxUIEditorBase::SetSelectedWidgets( TArray<UUIObject*> SelectionSet )
{
	UBOOL bResult = FALSE;
	if ( !Selection->CompareWithSelection(SelectionSet) )
	{
		Selection->Modify();
		Selection->BeginBatchSelectOperation();
		Selection->DeselectAll();

		for ( INT i = 0; i < SelectionSet.Num(); i++ )
		{
			Selection->Select(SelectionSet(i), TRUE);
		}

		Selection->EndBatchSelectOperation();
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Synchronizes the selected widgets across all windows
 *
 * @param	Source	the object that requested the synchronization.  The selection set of this object will be
 *					used as the authoritative selection set.
 */
void WxUIEditorBase::SynchronizeSelectedWidgets( FSelectionInterface* Source )
{
	if ( !bSynchronizingSelection )
	{
		bSynchronizingSelection = TRUE;

		if ( Source == this )
		{
			// sync up the scene tree to the currently selected widgets
			SceneTree->SynchronizeSelectedWidgets(this);

			// sync up the layer tree to the currently selected widgets
			if(LayerBrowser)
			{
				LayerBrowser->SynchronizeSelectedWidgets(this);
			}

			// Update the status bar.
			TArray<UUIObject*> SelectedItems;
			GetSelectedWidgets(SelectedItems);
			StatusBar->UpdateSelectedWidgets(SelectedItems);
		}
		else if ( Source == SceneTree )
		{
			// sync up the editor selections with the items selected in the tree control
			TArray<UUIObject*> SelectedTreeItems;
			SceneTree->GetSelectedWidgets(SelectedTreeItems);

			SetSelectedWidgets(SelectedTreeItems);

			// sync up the layer tree to the currently selected widgets
			if(LayerBrowser)
			{
				LayerBrowser->SynchronizeSelectedWidgets(Source);
			}

			// Update the status bar.
			StatusBar->UpdateSelectedWidgets(SelectedTreeItems);
		}
		else
		{
			if(LayerBrowser)
			{
				WxUILayerTreeCtrl* LayerTree = LayerBrowser->GetLayerTree();
				if(LayerTree && (Source == LayerTree))
				{
					// sync up the editor selections with the items selected in the layer tree control
					TArray<UUIObject*> SelectedTreeItems;
					LayerTree->GetSelectedWidgets(SelectedTreeItems);

					SetSelectedWidgets(SelectedTreeItems);

					// sync up the scene tree to the currently selected widgets
					SceneTree->SynchronizeSelectedWidgets(this);

					// Update the status bar.
					StatusBar->UpdateSelectedWidgets(SelectedTreeItems);
				}
			}
		}

		bSynchronizingSelection = FALSE;
	}
}

void WxUIEditorBase::SynchronizeSelectedWidgets( const wxCommandEvent& Event )
{
	wxWindow* Source = GetEventGenerator(Event);
	if ( Source == this )
	{
		SynchronizeSelectedWidgets(this);
	}
	else if ( Source == SceneTree )
	{
		SynchronizeSelectedWidgets(SceneTree);
	}
	else
	{
		appMsgf(AMT_OK, TEXT("Unsupported event generator type: %s"), Source->GetLabel().c_str());
	}
}

/**
 * Gets the window that generated this wxCommandEvent
 */
wxWindow* WxUIEditorBase::GetEventGenerator( const wxCommandEvent& Event ) const
{
	// find out which wxWindow generated this event
	wxObject* EventObject = Event.GetEventObject();
	wxWindow* SourceWindow = wxDynamicCast(EventObject,wxWindow);
	if ( SourceWindow == NULL )
	{
		wxMenu* EventMenu = wxDynamicCast(EventObject,wxMenu);
		if ( EventMenu != NULL )
		{
			SourceWindow = EventMenu->GetInvokingWindow();
		}
		else
		{
			SourceWindow = wxDynamicCast(EventObject,wxMenuBar);
		}
	}

	check(SourceWindow);
	return SourceWindow;
}

/**
 * Displays a dialog for selecting a single widget
 *
 * @param	WidgetList	the list of widgets that should be available in the selection dialog.
 */
UUIScreenObject* WxUIEditorBase::DisplayWidgetSelectionDialog( TArray<UUIScreenObject*> WidgetChoices )
{
	// first, sort them alphabetically
	Sort<USE_COMPARE_POINTER(UUIScreenObject,UnUIEditor_AlphaSort)>( &WidgetChoices(0), WidgetChoices.Num()  );

	UUIScreenObject* Result = NULL;

	wxArrayString WidgetTags;
	for ( INT i = 0; i < WidgetChoices.Num(); i++ )
	{
		WidgetTags.Add( *WidgetChoices(i)->GetTag().ToString() );
	}

	wxString DlgResult = wxGetSingleChoice(
		*LocalizeUI(TEXT("DlgSelectWidget_Label")),	// message
		*LocalizeUI(TEXT("DlgSelectWidget_Title")),	// title
		WidgetTags,									// choices
		this										// parent
		);

	if ( DlgResult.Len() > 0 )
	{
		for ( INT i = 0; i < WidgetChoices.Num(); i++ )
		{
			UUIScreenObject* Widget = WidgetChoices(i);
			if ( Widget->GetTag() == DlgResult.c_str() )
			{
				Result = Widget;
				break;
			}
		}
	}
	

	return Result;
}


/**
 * Displays a dialog requesting the user to input the package name, group name, and name for a new UI archetype.
 *
 * @param	out_ArchetypePackage	will receive a pointer to the package that should contain the UI archetype
 * @param	out_ArchetypeGroup		will receive a pointer to the package that should be the group that will contain the UI archetype;
 *									NULL if the user doesn't specify a group name in the dialog.
 * @param	out_ArchetypeName		will receive the value the user entered as the name to use for the new UI archetype
 *
 * @return	TRUE if the user chose OK and all parameters were valid (i.e. a package was successfully created and the name chosen is valid); FALSE otherwise.
 */
UBOOL WxUIEditorBase::DisplayCreateArchetypeDialog( UPackage*& out_ArchetypePackage, UPackage*& out_ArchetypeGroup, FString& out_ArchetypeName )
{
	UBOOL bResult = FALSE;

	FString PackageName, GroupName, ArchetypeName;

	WxDlgNewArchetype dlg;
	int Result = dlg.ShowModal(PackageName,GroupName,ArchetypeName);
	if ( Result != wxID_CANCEL )
	{
		ArchetypeName = dlg.GetObjectName();
		GroupName = dlg.GetGroup();
		PackageName = dlg.GetPackage();

		if ( PackageName.Len() > 0 && ArchetypeName.Len() > 0 )
		{
			// create or load the package specified
			UPackage* Package = GEngine->CreatePackage(NULL,*PackageName);
			UPackage* Group = NULL;

			UBOOL bSuccessfulGroup = TRUE;
			if( GroupName.Len() )
			{
				// first try loading the group, in case it already exists
				Group = LoadObject<UPackage>(Package, *GroupName, NULL, LOAD_NoWarn|LOAD_FindIfFail|LOAD_Quiet,NULL);
				if ( Group == NULL )
				{
					Group = GEngine->CreatePackage(Package,*GroupName);
					bSuccessfulGroup = (Group != NULL);
				}	
			}

			if ( Package != NULL && bSuccessfulGroup == TRUE )
			{
				Package->Modify();
				Package->SetDirtyFlag(TRUE);

				// assign the output values
				out_ArchetypePackage = Package;
				out_ArchetypeGroup = Group;
				out_ArchetypeName = ArchetypeName;
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

/**
 * Displays the rename dialog for each of the widgets specified.
 *
 * @param	Widgets		the list of widgets to rename
 *
 * @return	TRUE if at least one of the specified widgets was renamed successfully
 */
UBOOL WxUIEditorBase::DisplayRenameDialog( TArray<UUIObject*> Widgets )
{
	UBOOL bSuccess = FALSE;
	for ( INT WidgetIdx = 0; WidgetIdx < Widgets.Num(); WidgetIdx++ )
	{
		UUIObject* Widget = Widgets(WidgetIdx);
		wxString WidgetNameString = *Widget->GetTag().ToString();
		WxNameTextValidator WidgetNameValidator(&WidgetNameString, VALIDATE_Name);

		wxTextEntryDialog Dlg(this, *LocalizeUI(TEXT("DlgRenameWidget_Label")), *LocalizeUI(TEXT("DlgRenameWidget_Title")), *Widget->GetName());
		Dlg.SetTextValidator( WidgetNameValidator );

		if ( Dlg.ShowModal() == wxID_OK )
		{
			bSuccess = RenameObject(Widget, WidgetNameString.c_str());
		}
	}

	return bSuccess;
}

/**
 * Displays the rename dialog for the scene specified.
 *
 * @param	Scene		The scene to rename
 *
 * @return	TRUE if the scene was renamed successfully.
 */
UBOOL WxUIEditorBase::DisplayRenameDialog( UUIScene* Scene )
{
	UBOOL bSuccess = FALSE;

	wxString SceneNameString = *Scene->SceneTag.ToString();
	WxNameTextValidator SceneNameValidator(&SceneNameString, VALIDATE_Name);

	wxTextEntryDialog Dlg(this, *LocalizeUI( TEXT("DlgRenameScene_Label") ), *LocalizeUI( TEXT("DlgRenameScene_Title") ), *Scene->SceneTag.ToString());
	Dlg.SetTextValidator( SceneNameValidator );

	if ( Dlg.ShowModal() == wxID_OK )
	{
		bSuccess = RenameObject(Scene, SceneNameString.c_str());
	}
	
	return bSuccess;
}

/**
 * Tries to rename a UIScene, if unsuccessful, will return FALSE and display a error messagebox.
 *
 * @param Scene		Scene to rename.
 * @param NewName	New name for the object.
 *
 * @return	Returns whether or not the object was renamed successfully.
 */
UBOOL WxUIEditorBase::RenameObject( UUIScene* Scene, const TCHAR* NewName )
{
	INT CancelIndex = BeginTransaction(LocalizeUI(TEXT("TransRenameWidget")));
	UBOOL bSuccess = FALSE;
	FName NewSceneTag = NewName;

	if ( NewSceneTag == NAME_None )
	{
		wxMessageBox(*LocalizeUI(TEXT("DlgRenameScene_Error_EmptyName")), *LocalizeUnrealEd("Error"), wxOK | wxICON_ERROR);
	}
	else if(NewSceneTag != Scene->SceneTag)
	{
		// Check to see if there is a another scene in this scene's package that already has the name we are trying to set.
		// If not, then rename the scene and return TRUE.
		UObject* SceneOuter = Scene->GetOuter();
		if(SceneOuter != NULL)
		{
			if ( FIsUniqueObjectName(NewSceneTag, SceneOuter) )
			{
				FScopedObjectStateChange SceneNotifier(Scene);

				bSuccess = Scene->Rename(*NewSceneTag.ToString(), NULL, REN_ForceNoResetLoaders);

				if(bSuccess == TRUE)
				{
					if(LayerBrowser)
					{
						LayerBrowser->Refresh();
					}
				}
			}
			else
			{
				wxMessageBox(*LocalizeUI(TEXT("DlgRenameScene_Error_NameInUse")), *LocalizeUnrealEd("Error"), wxOK | wxICON_ERROR);
			}
		}
	}

	if ( bSuccess )
	{
		EndTransaction();
	}
	else
	{
		CancelTransaction(CancelIndex);
	}

	return bSuccess;
}

/**
 * Tries to rename a UIObject, if unsuccessful, will return FALSE and display a error messagebox.
 *
 * @param Widget	Widget to rename.
 * @param NewName	New name for the object.
 *
 * @return	Returns whether or not the object was renamed successfully.
 */
UBOOL WxUIEditorBase::RenameObject( UUIObject* Widget, const TCHAR* NewName )
{
	INT CancelIndex = BeginTransaction(LocalizeUI(TEXT("TransRenameWidget")));

	UBOOL bSuccess = FALSE;
	FName NewWidgetTag = NewName;

	if ( NewWidgetTag == NAME_None )
	{
		wxMessageBox(*LocalizeUI(TEXT("DlgRenameWidget_Error_EmptyName")), *LocalizeUnrealEd("Error"), wxOK | wxICON_ERROR);

	}
	else if ( Widget && !Widget->IsPrivateBehaviorSet(UCONST_PRIVATE_EditorNoRename) )
	{
		// Check to see if there is a sibling of this widget that already has the name we are trying to set.
		UUIScreenObject* WidgetParent = Widget->GetParent();
		UUIObject* WidgetAlreadyUsingThisName = WidgetParent->FindChild(NewWidgetTag,FALSE);
		if ( WidgetAlreadyUsingThisName != NULL )
		{
			if ( WidgetAlreadyUsingThisName != Widget )
			{
				wxMessageBox(*LocalizeUI(TEXT("DlgRenameWidget_Error_NameInUse")), *LocalizeUnrealEd("Error"), wxOK | wxICON_ERROR);
			}
		}
		else
		{
			FScopedObjectStateChange WidgetNotifier(Widget);

			bSuccess = Widget->Rename(*NewWidgetTag.ToString(), NULL, REN_ForceNoResetLoaders);

			if(bSuccess == TRUE)
			{
				if(LayerBrowser)
				{
					LayerBrowser->Refresh();
				}
			}
		}
	}
	else
	{
		wxMessageBox(*LocalizeUI(TEXT("WidgetProtectRename")), *LocalizeUnrealEd("Error"), wxOK | wxICON_ERROR);

	}

	if ( bSuccess )
	{
		EndTransaction();
	}
	else
	{
		CancelTransaction(CancelIndex);
	}

	return bSuccess;
}

/**
 * Change the parent of the specified widgets.
 * 
 * @param	Widgets		the widgets to change the parent for
 * @param	NewParent	the object that should be the widget's new parent
 * 
 * @return	TRUE if parent was successfully changed for any widgets in the list
 */
UBOOL WxUIEditorBase::ReparentWidgets( TArray<UUIObject*> Widgets, UUIScreenObject* NewParent )
{
	UBOOL bResult = FALSE;

	if ( NewParent != NULL )
	{
		TArray<UUIObject*> ExclusionSet;
		for ( INT ObjIndex = 0; ObjIndex < Widgets.Num(); ObjIndex++ )
		{
			UUIObject* Widget = Widgets(ObjIndex);
			ExclusionSet.AddUniqueItem(Widget);

			Widget->GetChildren(ExclusionSet, TRUE, &ExclusionSet);
		}

		for ( INT ObjIndex = 0; ObjIndex < Widgets.Num(); ObjIndex++ )
		{
			UUIObject* Widget = Widgets(ObjIndex);
			bResult = ReparentWidget(Widget, NewParent, &ExclusionSet) || bResult;
		}
	}

	return bResult;
}

/**
 * Change the parent of the specified widgets.
 * 
 * @param	Widget		the widget to change the parent for
 * @param	NewParent	the object that should be the widget's new parent
 * 
 * @return	TRUE if parent was successfully changed for the widget
 */
UBOOL WxUIEditorBase::ReparentWidget( UUIObject* Widget, UUIScreenObject* NewParent, TArray<UUIObject*>* ExclusionSet/*=NULL*/ )
{
	//@fixme ronp uiprefab: not yet supported.
	if ( Widget->GetArchetype<UUIObject>()->IsInUIPrefab() )
	{
		wxMessageBox(TEXT("This feature not yet supported for UIPrefabs"), TEXT("Error"), wxOK|wxCENTRE, this);
// 		int result = wxMessageBox(	TEXT("This feature is not supported for UIPrefabInstances and should only be used if no instances of this UIPrefab have been created.  Would you still like to continue?"),
// 									TEXT("Error"), wxOK|wxCANCEL|wxCENTER, this);
// 		if ( result != wxID_OK )
// 		{
// 			return FALSE;
// 		}
	}

	UBOOL bResult = FALSE;

	INT CancelIndex = BeginTransaction(LocalizeUI(TEXT("TransChangeParent")));
	if ( NewParent != NULL && Widget != NULL )
	{
		if ( !Widget->IsPrivateBehaviorSet(UCONST_PRIVATE_EditorNoReparent ) )
		{
			FScopedObjectStateChange NewParentNotifier(NewParent), WidgetNotifier(Widget); 

			// Save the widget's position before modifying its parent.
			FLOAT Position[4];
			Position[UIFACE_Left] = Widget->GetPosition(UIFACE_Left,EVALPOS_PixelViewport,TRUE);
			Position[UIFACE_Top] = Widget->GetPosition(UIFACE_Top,EVALPOS_PixelViewport,TRUE);
			Position[UIFACE_Right] = Widget->GetPosition(UIFACE_Right,EVALPOS_PixelViewport,TRUE);
			Position[UIFACE_Bottom] = Widget->GetPosition(UIFACE_Bottom,EVALPOS_PixelViewport,TRUE);

			UUIScreenObject* CurrentParent = Widget->GetParent();
			if ( CurrentParent != NULL && CurrentParent != NewParent )
			{
				FScopedObjectStateChange CurrentParentNotifier(CurrentParent);

				// remember the index for this child into its current parent's Children array, in case the new parent does not accept it
				if ( CurrentParent->ReparentChild(Widget, NewParent) )
				{
					bResult = TRUE;
				}
				else
				{
					CurrentParentNotifier.CancelEdit();
				}
			}
			else if ( NewParent->InsertChild(Widget) != INDEX_NONE )
			{
				bResult = TRUE;
			}

			// Reset the widget's position to its old position.
			if ( bResult )
			{
				Widget->SetPosition(Position[UIFACE_Left], Position[UIFACE_Top], Position[UIFACE_Right], Position[UIFACE_Bottom], EVALPOS_PixelViewport, TRUE);
			}
			else
			{
				NewParentNotifier.CancelEdit();
				WidgetNotifier.CancelEdit();
			}
		}
		else
		{
			wxMessageBox(*LocalizeUI(TEXT("WidgetProtectParent")), *LocalizeUnrealEd("Error"), wxOK | wxICON_ERROR);
		}
	}

	if ( bResult )
	{
		EndTransaction();
	}
	else
	{
		CancelTransaction(CancelIndex);
	}

	return bResult;
}

/**
 * Centers on the currently selected widgets.
 */
void WxUIEditorBase::CenterOnSelectedWidgets()
{
	UUIObject* SelectedWidget = Selection->GetTop<UUIObject>();

	// Make sure that there is a widget selected.
	if(SelectedWidget)
	{
		const FVector2D CenterPosition = (ObjectVC->Selector->StartLocation + ObjectVC->Selector->EndLocation) * 0.5f * ObjectVC->Zoom2D;
		const FVector2D ViewportSize = FVector2D(ObjectVC->Viewport->GetSizeX(), ObjectVC->Viewport->GetSizeY());
		FVector2D Origin = (ViewportSize * 0.5f) - CenterPosition;
		ObjectVC->Origin2D = FIntPoint( appTrunc(Origin[0]), appTrunc(Origin[1]) );
	}
}

/**
 * Copy the current selected widgets.
 */
void WxUIEditorBase::CopySelectedWidgets()
{
	// Export the selected widgets to a string buffer;
	FStringOutputDevice Ar;
	const FExportObjectInnerContext Context;

	//@fixme ronp - should we use OwnerScene->GetEditorDefaultParentWidget instead here?
	UExporter::ExportToOutputDevice( &Context, OwnerScene, NULL, Ar, TEXT("copy"), 0, PPF_ExportsNotFullyQualified, TRUE );

	appClipboardCopy( *Ar );
}

/**
 * Cuts the current selected widgets.
 */
void WxUIEditorBase::CutSelectedWidgets()
{
	// Copy objects to clipboard.
	CopySelectedWidgets();

	// Delete the objects for cutting.
	BeginTransaction(LocalizeUnrealEd(TEXT("Cut")));
	{
		DeleteSelectedWidgets();
	}
	EndTransaction();
}

/**
 * Deletes the currently selected widgets.
 */
void WxUIEditorBase::DeleteSelectedWidgets()
{
	INT CancelIndex = BeginTransaction(LocalizeUnrealEd(TEXT("Delete")));

	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets, TRUE);

	UBOOL bDeleteSuccessful = FALSE;
	for ( INT i = 0; i < SelectedWidgets.Num(); i++ )
	{
		UUIObject* Child = SelectedWidgets(i);
		UUIScreenObject* WidgetParent = Child->GetParent();
		UUIObject* OwnerWidget = Child->GetOwner();

		FScopedObjectStateChange ParentNotifier(WidgetParent), ChildNotifier(Child);
		if ( !Child->IsPrivateBehaviorSet(UCONST_PRIVATE_EditorNoDelete) && WidgetParent->RemoveChild(Child) == TRUE )
		{
			bDeleteSuccessful = TRUE;

			// clear the RF_Public flag so that we don't create a redirect
			Child->ClearFlags(RF_Public);
			Child->RemoveFromRoot();
			Child->Rename(NULL, UObject::GetTransientPackage(), REN_ForceNoResetLoaders);

			// UUIScreenObject::RemoveChild will clear the value of UIObject::Owner, but once we've renamed a UIPrefab widget into the transient
			// package (when deleting), there is no way to determine which UIPrefab the widget came from so LoadInstancesFromPropagationArchive()
			// can't be called for that archetype widget.  To workaround this, here we restore the widget's Owner reference so that
			// UUIScreenObject::IsInUIPrefab() can use the Owner if the widget is no longer part of the parent-child hierarchy 
			Child->Owner = OwnerWidget;
		}
		else if ( Child->IsPrivateBehaviorSet(UCONST_PRIVATE_EditorNoDelete) )
		{
			wxMessageBox(*LocalizeUI(TEXT("WidgetProtectFromDelete")), *LocalizeUnrealEd("Error"), wxOK | wxICON_ERROR);
		}
		else
		{
			ChildNotifier.CancelEdit();
			ParentNotifier.CancelEdit();
		}


	}

	if ( bDeleteSuccessful )
	{
		RefreshLayerBrowser();
		Selection->Modify();
		Selection->BeginBatchSelectOperation();
		Selection->DeselectAll();

		Selection->EndBatchSelectOperation();
		EndTransaction();
	}
	else
	{
		CancelTransaction(CancelIndex);
	}
}

/**
 * Changes the position of a widget in the widget parent's Children array, which affects the render order of the scene
 * Called when the user selects a "Move Up/Move ..." command in the context menu
 *
 * @param Action - The type of reordering action to perform on each widget that is selected.
 */
void WxUIEditorBase::ReorderSelectedWidgets( EReorderAction Action )
{
	INT CancelIndex = BeginTransaction(LocalizeUI(TEXT("TransModifyRenderOrder")));
	
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets, TRUE);

	UBOOL bSuccess = FALSE;
	for ( INT WidgetIdx = 0; WidgetIdx < SelectedWidgets.Num(); WidgetIdx++ )
	{
		UUIObject* Widget = SelectedWidgets(WidgetIdx);
		UUIScreenObject* WidgetParent = Widget->GetParent();

		FScopedObjectStateChange ParentNotifier(WidgetParent), ChildNotifier(Widget);

		UBOOL bSuccessfulMove = FALSE;
		check(WidgetParent != NULL);
		switch ( Action )
		{
		case RA_MoveUp:
			{
				INT CurrentPosition = WidgetParent->Children.FindItemIndex(Widget);
				bSuccessfulMove = ReorderWidget(Widget, CurrentPosition + 1, FALSE);
			}
			break;

		case RA_MoveDown:
			{
				INT CurrentPosition = WidgetParent->Children.FindItemIndex(Widget);
				bSuccessfulMove = ReorderWidget(Widget, CurrentPosition - 1, FALSE);
			}
			break;

		case RA_MoveToTop:
			bSuccessfulMove = ReorderWidget(Widget, WidgetParent->Children.Num() - 1);
			break;

		case RA_MoveToBottom:
			bSuccessfulMove = ReorderWidget(Widget, 0);
			break;
		}

		if ( !bSuccessfulMove )
		{
			ParentNotifier.CancelEdit();
			ChildNotifier.CancelEdit();
		}

		bSuccess = bSuccess || bSuccessfulMove;
	}

	if ( bSuccess )
	{
		EndTransaction();
	}
	else
	{
		CancelTransaction(CancelIndex);
	}
}

/**
 * Repositions the specified widget within its parent's Children array.
 *
 * @param	Widget					the widget to move
 * @param	NewPosition				the new position of the widget. the value will be clamped to the size of the parent's Children array
 * @param	bRefreshSceneBrowser	if TRUE, calls RefreshLayerBrowser to update the scene browser tree control
 */
UBOOL WxUIEditorBase::ReorderWidget( UUIObject* Widget, INT NewPosition, UBOOL bRefreshSceneBrower/*=TRUE*/ )
{
	//@fixme ronp uiprefab: not yet supported.
	if ( OwnerScene != NULL && OwnerScene->IsA(UUIPrefabScene::StaticClass()) )
	{
//		wxMessageBox(TEXT("This feature not yet supported for UIPrefabs"), TEXT("Error"), wxOK|wxCENTRE, this);
		int result = wxMessageBox(	TEXT("This feature is not supported for UIPrefabInstances and should only be used if no instances of this UIPrefab have been created.  Would you still like to continue?"),
 									TEXT("Error"), wxYES_NO|wxCENTER|wxICON_EXCLAMATION, this);
 		if ( result != wxYES )
 		{
 			return FALSE;
 		}
	}

	INT CancelIndex = BeginTransaction(LocalizeUI(TEXT("TransModifyRenderOrder")));

	UBOOL bResult = FALSE;
	if ( Widget != NULL )
	{
		UUIScreenObject* WidgetParent = Widget->GetParent();

		check(WidgetParent != NULL);
		check(Widget != WidgetParent);

		//@todo - should this be moved into a method of UUIScreenObject?
		INT CurrentPosition = WidgetParent->Children.FindItemIndex(Widget);
		if ( CurrentPosition != INDEX_NONE )
		{
			NewPosition = Clamp(NewPosition, 0, WidgetParent->Children.Num() - 1);
			if ( NewPosition != CurrentPosition )
			{
				FScopedObjectStateChange ParentNotifier(WidgetParent);

				// if the new position is only 1 away from the current position, we can do a simple swap
				if ( Abs(NewPosition - CurrentPosition) == 1 )
				{
					WidgetParent->Children.SwapItems(NewPosition,CurrentPosition);
					bResult = TRUE;
				}
				else
				{
					// first remove the child from the old position
					WidgetParent->Children.Remove(CurrentPosition);

					// then insert the widget into the new position
					WidgetParent->Children.InsertItem(Widget, NewPosition);

					bResult = TRUE;
				}
			}
		}
	}

	if ( bResult )
	{
		// notify the scene that it should be updated
		Widget->RequestSceneUpdate(TRUE,TRUE);
		EndTransaction();
	}
	else
	{
		CancelTransaction(CancelIndex);
	}

	return bResult;
}

/**
 * Called when a toolbar is requested by wxFrame::CreateToolBar
 */
wxToolBar* WxUIEditorBase::OnCreateToolBar(long style, wxWindowID id, const wxString& name)
{
	WxUIEditorMainToolBar* Result = new WxUIEditorMainToolBar;
	Result->Create(this, id, this);

	return Result;
}

//****************************************************************

BEGIN_EVENT_TABLE( WxUIEditorBase, wxFrame )
	EVT_CLOSE( WxUIEditorBase::OnClose)
	EVT_SIZE(OnSize)

	EVT_MENU_RANGE( IDM_UI_NEWOBJECT_START, IDM_UI_NEWOBJECT_END, WxUIEditorBase::OnContext_CreateNew )
	EVT_MENU( IDM_UI_PLACEARCHETYPE, WxUIEditorBase::OnContext_CreateNewFromArchetype )
	EVT_MENU( IDMN_ObjectContext_Rename, WxUIEditorBase::OnContext_Rename )
	EVT_MENU( IDMN_ObjectContext_Delete, WxUIEditorBase::OnContext_Delete )
	EVT_MENU_RANGE(IDM_UI_LISTELEMENTTAG_START, IDM_UI_LISTELEMENTTAG_END, WxUIEditorBase::OnContext_BindListElement)
	EVT_MENU_RANGE( IDM_UI_STYLECLASS_BEGIN, IDM_UI_STYLECLASS_END, WxUIEditorBase::OnContext_SelectStyle )
	EVT_MENU_RANGE( ID_CONVERT_POSITION_START, ID_CONVERT_POSITION_END, WxUIEditorBase::OnContext_ConvertPosition )
	EVT_MENU(IDM_UI_EDITSTYLE, WxUIEditorBase::OnContext_EditStyle)
	EVT_MENU_RANGE( IDM_UI_STYLEREF_BEGIN, IDM_UI_STYLEREF_END, WxUIEditorBase::OnContext_EditStyle )
	EVT_MENU(IDM_UI_EDITEVENTS, WxUIEditorBase::OnContext_EditEvents)
	EVT_MENU(IDM_UI_CHANGEPARENT, WxUIEditorBase::OnContext_ChangeParent)
	EVT_MENU(IDM_UI_MOVEUP,WxUIEditorBase::OnContext_ModifyRenderOrder)
	EVT_MENU(IDM_UI_MOVEDOWN,WxUIEditorBase::OnContext_ModifyRenderOrder)
	EVT_MENU(IDM_UI_MOVETOTOP,WxUIEditorBase::OnContext_ModifyRenderOrder)
	EVT_MENU(IDM_UI_MOVETOBOTTOM,WxUIEditorBase::OnContext_ModifyRenderOrder)
	EVT_MENU(IDM_UI_DOCKINGEDITOR,WxUIEditorBase::OnContext_EditDockingSet)
	EVT_MENU(IDM_UI_KISMETEDITOR,WxUIEditorBase::OnContext_ShowKismet)

	EVT_MENU(IDM_CREATEARCHETYPE,WxUIEditorBase::OnContext_SaveAsResource)
	EVT_MENU(IDM_UI_DUMPPROPERTIES, WxUIEditorBase::OnContext_DumpProperties)
	EVT_MENU(IDM_UI_BINDWIDGETSTODATASTORE, WxUIEditorBase::OnContext_BindWidgetsToDataStore)
	EVT_MENU(IDM_UI_CLEARWIDGETSDATASTORE, WxUIEditorBase::OnContext_ClearWidgetsDataStore)

	EVT_MENU_RANGE(IDM_UI_SET_DATASTOREBINDING_START, IDM_UI_SET_DATASTOREBINDING_END, WxUIEditorBase::OnContext_BindWidgetsToDataStore)
	EVT_MENU_RANGE(IDM_UI_CLEAR_DATASTOREBINDING_START, IDM_UI_CLEAR_DATASTOREBINDING_END, WxUIEditorBase::OnContext_ClearWidgetsDataStore)

	EVT_MENU(IDM_UI_TABCONTROL_INSERTPAGE, WxUIEditorBase::OnContext_InsertTabPage)
	EVT_MENU(IDM_UI_TABCONTROL_REMOVEPAGE, WxUIEditorBase::OnContext_RemoveTabPage)

	EVT_MENU(IDM_UI_CALLOUTBUTTON_ADD, WxUIEditorBase::OnContext_InsertCalloutButton)
	EVT_MENU(IDM_UI_CALLOUTBUTTON_REMOVE, WxUIEditorBase::OnContext_RemoveCalloutButton)
	EVT_MENU(IDM_UI_CALLOUTBUTTON_CHANGEALIAS, WxUIEditorBase::OnContext_ChangeCalloutButtonAlias)

	// File Menu
	EVT_MENU(IDMN_UI_CLOSEWINDOW, WxUIEditorBase::OnCloseEditorWindow)

	// Edit Menu
	EVT_MENU(IDM_UNDO, WxUIEditorBase::OnUndo)
	EVT_MENU(IDM_REDO, WxUIEditorBase::OnRedo)
	EVT_MENU(IDM_COPY, WxUIEditorBase::OnCopy)
	EVT_MENU(IDM_CUT, WxUIEditorBase::OnCut)
	EVT_MENU(IDM_PASTE, WxUIEditorBase::OnPaste)
	EVT_MENU(IDM_UI_PASTEASCHILD, WxUIEditorBase::OnPasteAsChild)
	EVT_MENU(IDM_UI_CLICK_PASTE, WxUIEditorBase::OnContext_Paste)
	EVT_MENU(IDM_UI_CLICK_PASTEASCHILD, WxUIEditorBase::OnContext_PasteAsChild)
	EVT_MENU(IDM_UI_BINDUIEVENTKEYS, WxUIEditorBase::OnBindUIEventKeys)
	EVT_MENU(IDM_UI_DATASTOREBROWSER, WxUIEditorBase::OnShowDataStoreBrowser)

	// Edit Menu - Update UI
	EVT_UPDATE_UI(IDM_UNDO, WxUIEditorBase::OnEditMenu_UpdateUI)
	EVT_UPDATE_UI(IDM_REDO, WxUIEditorBase::OnEditMenu_UpdateUI)
	EVT_UPDATE_UI(IDM_COPY, WxUIEditorBase::OnEditMenu_UpdateUI)
	EVT_UPDATE_UI(IDM_CUT, WxUIEditorBase::OnEditMenu_UpdateUI)
	EVT_UPDATE_UI(IDM_PASTE, WxUIEditorBase::OnEditMenu_UpdateUI)
	EVT_UPDATE_UI(IDM_UI_PASTEASCHILD, WxUIEditorBase::OnEditMenu_UpdateUI)
	EVT_UPDATE_UI(IDMN_ObjectContext_Delete, WxUIEditorBase::OnEditMenu_UpdateUI)
	EVT_UPDATE_UI(IDM_UI_BINDWIDGETSTODATASTORE, WxUIEditorBase::OnEditMenu_UpdateUI)

	// Skin Menu
	EVT_MENU(IDM_UI_EDITSKIN, WxUIEditorBase::OnEditSkin)
	EVT_MENU(IDM_UI_SAVESKIN, WxUIEditorBase::OnSaveSkin)
	EVT_MENU(IDM_UI_LOADSKIN, WxUIEditorBase::OnLoadSkin)
	EVT_MENU(IDM_UI_CREATESKIN, WxUIEditorBase::OnCreateSkin)
	EVT_UPDATE_UI(IDM_UI_EDITSKIN, WxUIEditorBase::OnSkinMenu_UpdateUI)

	// Scene Menu
	EVT_MENU(IDM_UI_FORCE_SCENE_REFRESH, WxUIEditorBase::OnForceSceneUpdate)

	// View Menu
	EVT_MENU(ID_UI_CENTERONSELECTION, WxUIEditorBase::OnMenuCenterOnSelection)

	EVT_MENU_RANGE (ID_UITOOL_VIEW_START, ID_UITOOL_VIEW_END, WxUIEditorBase::OnMenu_View)
	EVT_UPDATE_UI_RANGE (ID_UITOOL_VIEW_START, ID_UITOOL_VIEW_END, WxUIEditorBase::OnUpdateUI)

	EVT_MENU_RANGE(ID_UITOOL_SHOWOUTLINE_START, ID_UITOOL_SHOWOUTLINE_END, WxUIEditorBase::OnToggleOutline)
	EVT_UPDATE_UI_RANGE (ID_UI_SHOWVIEWPORTOUTLINE, ID_UITOOL_SHOWOUTLINE_END, WxUIEditorBase::OnUpdateUI)


	// Align Menu
	EVT_MENU(ID_UI_ALIGN_VIEWPORT_TOP,						WxUIEditorBase::OnMenuAlignToViewport)
	EVT_MENU(ID_UI_ALIGN_VIEWPORT_BOTTOM,					WxUIEditorBase::OnMenuAlignToViewport)
	EVT_MENU(ID_UI_ALIGN_VIEWPORT_LEFT,						WxUIEditorBase::OnMenuAlignToViewport)
	EVT_MENU(ID_UI_ALIGN_VIEWPORT_RIGHT,					WxUIEditorBase::OnMenuAlignToViewport)
	EVT_MENU(ID_UI_ALIGN_VIEWPORT_CENTER_HORIZONTALLY,		WxUIEditorBase::OnMenuAlignToViewport)
	EVT_MENU(ID_UI_ALIGN_VIEWPORT_CENTER_VERTICALLY,		WxUIEditorBase::OnMenuAlignToViewport)

	
	EVT_MENU(ID_UI_TEXT_ALIGN_TOP,						WxUIEditorBase::OnMenuAlignText)
	EVT_MENU(ID_UI_TEXT_ALIGN_BOTTOM,					WxUIEditorBase::OnMenuAlignText)
	EVT_MENU(ID_UI_TEXT_ALIGN_LEFT,						WxUIEditorBase::OnMenuAlignText)
	EVT_MENU(ID_UI_TEXT_ALIGN_RIGHT,					WxUIEditorBase::OnMenuAlignText)
	EVT_MENU(ID_UI_TEXT_ALIGN_CENTER,					WxUIEditorBase::OnMenuAlignText)
	EVT_MENU(ID_UI_TEXT_ALIGN_MIDDLE,					WxUIEditorBase::OnMenuAlignText)


	EVT_MENU_RANGE (ID_UITOOL_MODES_START, ID_UITOOL_MODES_END, WxUIEditorBase::OnToolbar_SelectMode)
	EVT_UPDATE_UI_RANGE (ID_UITOOL_MODES_START, ID_UITOOL_MODES_END, WxUIEditorBase::OnUpdateUIToolMode)

	EVT_COMBOBOX(ID_UITOOL_SS_PREVIEW_MODE, WxUIEditorBase::OnToolbar_SelectSSPreviewMode)
//	EVT_UPDATE_UI_RANGE(ID_UITOOL_SS_PREVIEW_MODE_START, ID_UITOOL_SS_PREVIEW_MODE_END, WxUIEditorBase::OnUpdateUI)

	EVT_COMBOBOX(ID_UITOOL_VIEW_SIZE, WxUIEditorBase::OnToolbar_SelectViewportSize)
	EVT_COMBOBOX(ID_UITOOL_WIDGET_PREVIEW_STATE, WxUIEditorBase::OnToolbar_SelectWidgetPreviewState)
	EVT_COMBOBOX(ID_UI_COMBO_CHOOSEPLATFORM, WxUIEditorBase::OnToolbar_SelectPreviewPlatform)
	EVT_TEXT_ENTER(ID_UITOOL_VIEW_SIZE,WxUIEditorBase::OnToolbar_SelectViewportSize)

	EVT_TEXT(ID_UITOOL_VIEW_GUTTER, WxUIEditorBase::OnToolbar_EnterViewportGutterSize)
	EVT_SPINCTRL(ID_UITOOL_VIEW_GUTTER, WxUIEditorBase::OnToolbar_SelectViewportGutterSize)
	EVT_UPDATE_UI(ID_UITOOL_VIEW_GUTTER,WxUIEditorBase::OnUpdateUI)


	EVT_MENU_RANGE( IDM_DRAG_GRID_START, IDM_DRAG_GRID_END, WxUIEditorBase::OnMenuDragGrid )
	EVT_UPDATE_UI_RANGE( IDM_DRAG_GRID_START, IDM_DRAG_GRID_END, WxUIEditorBase::OnMenuDragGrid_UpdateUI )
END_EVENT_TABLE()

//****************************************************************

/* =======================================
	WxUIEditorBase interface.
   =====================================*/

/** Updates various wxWidgets elements. */
void WxUIEditorBase::UpdateUI()
{
	// Update the status bar as well.
	StatusBar->UpdateUI();
}

void WxUIEditorBase::OnUpdateUI( wxUpdateUIEvent& Event )
{
	if ( EditorOptions != NULL )
	{
		int id = Event.GetId();

		switch (id)
		{
		case ID_UITOOL_VIEW_DRAWBKGND:
			Event.Check (EditorOptions->mViewDrawBkgnd);
			break;
		case ID_UITOOL_VIEW_DRAWGRID:
			Event.Check (EditorOptions->mViewDrawGrid);
			break;
		case ID_UITOOL_VIEW_WIREFRAME:
			Event.Check( EditorOptions->bViewShowWireframe );
			break;
		case ID_UI_SHOWVIEWPORTOUTLINE:
			Event.Check(EditorOptions->bRenderViewportOutline);
			break;
		case ID_UI_SHOWCONTAINEROUTLINE:
			Event.Check(EditorOptions->bRenderContainerOutline);
			break;
		case ID_UI_SHOWSELECTIONOUTLINE:
			Event.Check(EditorOptions->bRenderSelectionOutline);
			break;
		case ID_UI_SHOWSELECTIONOUTLINE_PERWIDGET:
			Event.Check(!EditorOptions->bRenderPerWidgetSelectionOutline);
			break;
		case ID_UI_SHOWSELECTIONHANDLES:
			Event.Check(EditorOptions->bRenderSelectionHandles);
			break;
		case ID_UITOOL_SHOWDOCKHANDLES:
			Event.Check(EditorOptions->bShowDockHandles);
			break;
		case ID_UI_SHOWTITLESAFEREGION:
			Event.Check(EditorOptions->bRenderTitleSafeRegionOutline);
			break;
		case ID_UITOOL_VIEW_GUTTER:
			{
				FString GutterSizeString = appItoa(EditorOptions->ViewportGutterSize);
				Event.SetText(*GutterSizeString);

				//@fixme - Event.SetText doesn't work correctly for controls contained within toolbars (wx 2.6.2), so
				// we'll need to set the value of the gutter control manually
				if ( MainToolBar != NULL )
				{
					MainToolBar->SetGutterSizeValue(GutterSizeString);
				}
			}
			break;

//@todo -	problem with this is that it prevents the user from typing in custom viewport sizes...need to figure out a way to ignore
//			ui updates for this control as long as the user is typing something in the textbox; though it doesn't matter as long as the
//			toolbar is the only place the user can change the viewport size...
// 		case ID_UITOOL_VIEWPORTSIZE:
// 			{
// 				FString ViewportSizeString = EditorOptions->GetViewportSizeString();
// 				if ( ViewportSizeString.Len() == 0 )
// 				{
// 					//@fixme - localize/use variable
// 					ViewportSizeString = TEXT("Auto");
// 				}
// 
// 				//@fixme - Event.SetText doesn't work correctly for controls contained within toolbars (wx 2.6.2), so
// 				// we'll need to set the value of the gutter control manually
// 				Event.SetText(*ViewportSizeString);
// 			}
// 			break;
// 			case :
// 				break;
// 			case :
// 				break;

		default:
			Event.Skip();
			break;
		}
	}
	else
	{
		Event.Skip();
	}

	// Update non event related UI elements.
	UpdateUI();
}

/** Updates the toolbar buttons corresponding to the various UI tool modes */
void WxUIEditorBase::OnUpdateUIToolMode( wxUpdateUIEvent& Event )
{
	if(EditorOptions)
	{
		if(Event.GetId() == EditorOptions->ToolMode)
		{
			Event.Check( true );
		}
		else
		{
			Event.Check( false );
		}
	}
}

/** Shows the docking editor for the currently selected widgets. */
void WxUIEditorBase::ShowDockingEditor()
{
	INT CancelIndex = BeginTransaction(LocalizeUI(TEXT("TransEditDocking")));

	// get the list of selected widgets
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);

	UBOOL bSuccess = FALSE;
	long ExtraButtons = SelectedWidgets.Num() > 1 ? ID_CANCEL_ALL : 0;
	for ( INT i = 0; i < SelectedWidgets.Num(); i++ )
	{
		UUIObject* Widget = SelectedWidgets(i);

		WxDlgDockingEditor dlg;
		dlg.Create(this, ID_UI_DOCKINGEDITOR_DLG, ExtraButtons);

		FScopedObjectStateChange WidgetNotifier(Widget);
		INT DialogResult = dlg.ShowModal(OwnerScene, Widget);
		if ( DialogResult == wxID_OK )
		{
			// @todo - display a warning if one out of the set couldn't be saved successfully
			bSuccess = dlg.SaveChanges() || bSuccess;
		}
		else if ( DialogResult == ID_CANCEL_ALL )
		{
			WidgetNotifier.CancelEdit();
			break;
		}
	}

	if ( bSuccess )
	{
		EndTransaction();
	}
	else
	{
		CancelTransaction(CancelIndex);
	}
}

/** Shows the event editor for the currently selected widgets. */
void WxUIEditorBase::ShowEventEditor()
{
	INT CancelIndex = BeginTransaction(LocalizeUI(TEXT("TransEditModifyDisabledInputEvents")));

	// get the list of selected widgets
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);

	UBOOL bSuccess = FALSE;

	// Show a dialog for each widget if there are any selected, otherwise show a dialog letting the user edit scene events.
	if(SelectedWidgets.Num())
	{
		for ( INT WidgetIdx = 0; WidgetIdx < SelectedWidgets.Num(); WidgetIdx++ )
		{
			UUIScreenObject* Widget = SelectedWidgets(WidgetIdx);
			check(Widget);

			WxDlgUIWidgetEvents Dlg(this, Widget);

			INT DialogResult = Dlg.ShowModal();
			if ( DialogResult == wxID_OK )
			{
				bSuccess = TRUE;
			}
		}
	}
	else
	{
		WxDlgUIWidgetEvents Dlg(this, OwnerScene);

		INT DialogResult = Dlg.ShowModal();
		if ( DialogResult == wxID_OK )
		{
			bSuccess = TRUE;
		}
	}


	if ( bSuccess )
	{
		EndTransaction();
	}
	else
	{
		CancelTransaction(CancelIndex);
	}
}

/**
 * Changes the platform that the UI editor should simulate.
 *
 * @param	DesiredPreviewPlatform	the name of the platform to set; should match the string returned by calling FConsoleSupport::GetConsoleName() (FConsoleSupportIterator)
 *
 * @return	TRUE if the platform was changed successfully.
 */
UBOOL WxUIEditorBase::SetPreviewPlatform( const FString& DesiredPreviewPlatform )
{
	UBOOL bResult = FALSE;

	if ( DesiredPreviewPlatform.Len() > 0 )
	{
		if ( MainToolBar != NULL )
		{
			MainToolBar->SetPreviewPlatformSelection(DesiredPreviewPlatform);

			if ( EditorOptions != NULL && EditorOptions->CurrentPreviewPlatform != DesiredPreviewPlatform )
			{
				EditorOptions->CurrentPreviewPlatform = DesiredPreviewPlatform;
			}
		}
	}

	return bResult;
}

/**
 * Notification that the specified style was modified.  Calls ApplyStyle on all widgets in the scene which use this style.
 *
 * @param	ModifiedStyle	the style that was modified
 */
void WxUIEditorBase::NotifyStyleModified( UUIStyle* ModifiedStyle )
{
	// notify the skin about the style change
	UUISkin* Skin = Cast<UUISkin>(ModifiedStyle->GetOuter());
    verify(Skin);
	Skin->NotifyStyleModified(ModifiedStyle); 
	
    //Update widgets in the viewport that had their style modified
	FWidgetCollector AffectedWidgets;
	AffectedWidgets.ApplyFilter(ModifiedStyle);
	AffectedWidgets.Process(OwnerScene);
	
	UBOOL DirtyFlagValue = ModifiedStyle->IsDirty((UUIState*)NULL);

	// force all widgets to reapply the style data by marking it dirty
	ModifiedStyle->SetDirtiness(TRUE);
	for ( INT i = 0; i < AffectedWidgets.Num(); i++ )
	{
		UUIObject* Widget = AffectedWidgets(i);
		FScopedObjectStateChange StyleChangeNotifier(Widget);
		Widget->ResolveStyles(FALSE);
	}

	// all widgets have reapplied the style data, restore the flag to its previous state
	ModifiedStyle->SetDirtiness(DirtyFlagValue);
}

/**
 * Determines whether the currently selected data store tag is valid for the widgets specified.
 *
 * @param	WidgetsToBind	the list of widgets that we want to bind to the currently selected data store tag
 *
 * @return	TRUE if all of the widgets specified support the data store field type of the currently selected data store tag
 *
 * @fixme ronp - this should also return false when attempting to bind a widget to a field that is contained by a data provider accessible only through a provider collection tag in the parent data provider
 */
UBOOL WxUIEditorBase::IsSelectedDataStoreTagValid( TArray<UUIObject*>& WidgetsToBind )
{
	UBOOL bResult = FALSE;

	WxDlgUIDataStoreBrowser* DataStoreBrowser = SceneManager->GetDataStoreBrowser();
	if ( DataStoreBrowser != NULL )
	{
		EUIDataProviderFieldType TagFieldType = DataStoreBrowser->GetSelectedTagType();
		if ( TagFieldType != DATATYPE_MAX )
		{
			// the structure of the code that sets the values of bIsValidFieldType is setup like so:
			// 1. if no widgets are selected, then disable the "bind data field" item
			// 2. if any of the selected widgets can't support the selected data store field (i.e. don't contain UIDataStoreBinding properties which are compatible with the FieldType
			//		of the selected data store tag), then disable the "bind data field" item
			// 3. a widget is only required to have one data store binding property which is compatible with FieldType of the selected data store tag in order to be considered "OK"
			// 4. if a widget has more than one data store binding property that is compatible with the FieldType of the selected data store tag, it makes no difference
			UBOOL bIsValidFieldType = FALSE;

			// verify that the selected widgets are compatible with the selected data field tag type
			for ( INT WidgetIndex = 0; WidgetIndex < WidgetsToBind.Num(); WidgetIndex++ )
			{
				UUIObject* BindableWidget = WidgetsToBind(WidgetIndex);
				if ( BindableWidget != NULL )
				{
					TMultiMap<FName,FUIDataStoreBinding*> WidgetBindings;
					BindableWidget->GetDataBindings(WidgetBindings);

					UBOOL bWidgetSupportsFieldType = FALSE;
					for ( TMultiMap<FName,FUIDataStoreBinding*>::TIterator It(WidgetBindings); It; ++It )
					{
						FUIDataStoreBinding* Binding = It.Value();
						if ( Binding->IsValidDataField(TagFieldType) )
						{
							bWidgetSupportsFieldType = TRUE;
							break;
						}
					}

					// if this widget has at least one data store binding of the correct type, we're allowed to bind the widget to this data field
					//@todo ronp - fix WxUIEditorBase::OnContext_BindWidgetsToDataStore so that is aware of which data store binding to assign
					// (i.e. BindingIndex)
					if ( bWidgetSupportsFieldType == TRUE )
					{
						bIsValidFieldType = TRUE;
					}
					else
					{
						// otherwise, if none of this widget's data store bindings are of the correct type, then the "bind field" menu item should be greyed out
						bIsValidFieldType = FALSE;
						break;
					}
				}
			}

			bResult = bIsValidFieldType;
		}
	}

	return bResult;
}

void WxUIEditorBase::OnSize( wxSizeEvent& Event )
{
	wxFrame::OnSize(Event);
	if ( ObjectVC != NULL && bWindowInitialized )
	{
		GCallbackEvent->Send(CALLBACK_ViewportResized, ObjectVC->Viewport, 0);
	}
}

void WxUIEditorBase::OnClose( wxCloseEvent& Event )
{
	GCallbackEvent->UnregisterAll(this);

	bWindowInitialized = FALSE;

	// notify the scene manager that this window is being closed
	SceneManager->SceneClosed(OwnerScene);

	// Flush the transaction buffer so we maintain no links to this scene.
	// @todo: The UI Editor window should have its own transaction buffer instead of using the global one.
	GEditor->ResetTransaction(*LocalizeUnrealEd("Undo"));

	this->Destroy(); // destroy this window
}

/* =======================================
	Toolbar/Menubar event handlers.
   =====================================*/
/**
 * Handle view menu commands
 *
 * @param	Event	event data
 */
void WxUIEditorBase::OnMenu_View (wxCommandEvent& Event)
{
	switch (Event.GetId())
	{
	case ID_UITOOL_VIEW_RESET:	// Reset view position and zoom

		ObjectVC->Origin2D.X = 0;
		ObjectVC->Origin2D.Y = 0;
		ObjectVC->Zoom2D = 1.0f;
		if ( OwnerScene && OwnerScene->SceneClient )
		{
			OwnerScene->SceneClient->UpdateCanvasToScreen();
		}
		break;

	case ID_UITOOL_VIEW_DRAWBKGND:	// Set drawing of background
		EditorOptions->mViewDrawBkgnd = Event.IsChecked();
		return;

	case ID_UITOOL_VIEW_DRAWGRID:	// Set drawing of grid
		EditorOptions->mViewDrawGrid = Event.IsChecked();
		return;

	case ID_UITOOL_VIEW_WIREFRAME:
		EditorOptions->bViewShowWireframe = Event.IsChecked();
		return;

	case ID_UITOOL_SHOWDOCKHANDLES:
		EditorOptions->bShowDockHandles = Event.IsChecked();
		return;
	}

	if (OwnerScene)
	{
		OwnerScene->RequestFormattingUpdate();
		OwnerScene->RequestSceneUpdate(FALSE,TRUE);
	}
}

void WxUIEditorBase::OnToolbar_SelectSSPreviewMode( wxCommandEvent& Event )
{
	if ( EditorOptions != NULL )
	{
		ESplitscreenRenderMode NewSSPreviewMode = static_cast<ESplitscreenRenderMode>(Event.GetSelection());
		if ( NewSSPreviewMode < eSST_MAX && NewSSPreviewMode != EditorOptions->CurrentSSPreviewMode )
		{
			EditorOptions->CurrentSSPreviewMode = NewSSPreviewMode;

			if ( ObjectVC != NULL && ObjectVC->Viewport != NULL )
			{
				GCallbackEvent->Send(CALLBACK_ViewportResized, ObjectVC->Viewport, 0);
			}
			else if ( OwnerScene != NULL )
			{
				OwnerScene->RequestFormattingUpdate();
				OwnerScene->RequestSceneUpdate(FALSE,TRUE,TRUE);
			}
		}
	}
}

void WxUIEditorBase::OnToolbar_SelectViewportSize( wxCommandEvent& Event )
{
	if ( EditorOptions != NULL )
	{
		INT NewSizeX = EditorOptions->VirtualSizeX;
		INT NewSizeY = EditorOptions->VirtualSizeY;

		// get the new viewport size
		if ( MainToolBar != NULL && !MainToolBar->GetViewportSize(NewSizeX,NewSizeY) )
		{
			// if the "Auto" item is selected, set the value to 0 which means that the viewport should take up the entire editor window
			NewSizeX = NewSizeY = UCONST_AUTOEXPAND_VALUE;
		}

		if ( NewSizeX != EditorOptions->VirtualSizeX || NewSizeY != EditorOptions->VirtualSizeY )
		{
			EditorOptions->VirtualSizeX = NewSizeX;
			EditorOptions->VirtualSizeY = NewSizeY;

			if ( ObjectVC != NULL && ObjectVC->Viewport != NULL )
			{
				GCallbackEvent->Send(CALLBACK_ViewportResized, ObjectVC->Viewport, 0);
			}
			else if ( OwnerScene != NULL )
			{
				OwnerScene->RequestFormattingUpdate();
				OwnerScene->RequestSceneUpdate(FALSE,TRUE,TRUE);
			}
		}
	}
}

/**
 * Changes the preview state for all currently selected widgets.
 */
void WxUIEditorBase::OnToolbar_SelectWidgetPreviewState( wxCommandEvent& Event )
{
	if(MainToolBar != NULL)
	{
		MainToolBar->SetWidgetPreviewStateSelection(Event.GetSelection());
	}
}

void WxUIEditorBase::OnToolbar_SelectPreviewPlatform( wxCommandEvent& Event )
{
	if ( MainToolBar != NULL )
	{
		FString DesiredPlatform = Event.GetString().c_str();

		FCallbackEventParameters Parms(this, CALLBACK_UIEditor_PreviewPlatformChanged, DesiredPlatform);
		GCallbackEvent->Send( Parms );
	}
}

void WxUIEditorBase::OnToolbar_SelectViewportGutterSize( wxSpinEvent& Event )
{
	if ( MainToolBar != NULL && EditorOptions != NULL )
	{
		EditorOptions->ViewportGutterSize = MainToolBar->GetGutterSize();
		if ( ObjectVC != NULL && ObjectVC->Viewport != NULL )
		{
			GCallbackEvent->Send(CALLBACK_ViewportResized, ObjectVC->Viewport, 0);
		}
		else if ( OwnerScene != NULL )
		{
			OwnerScene->RequestFormattingUpdate();
			OwnerScene->RequestSceneUpdate(TRUE,TRUE,TRUE);
		}
	}
}

void WxUIEditorBase::OnToolbar_EnterViewportGutterSize( wxCommandEvent& Event )
{
	if ( MainToolBar != NULL && EditorOptions != NULL )
	{
		EditorOptions->ViewportGutterSize = MainToolBar->GetGutterSize();
		if ( ObjectVC != NULL && ObjectVC->Viewport != NULL )
		{
			GCallbackEvent->Send(CALLBACK_ViewportResized, ObjectVC->Viewport, 0);
		}
		else if ( OwnerScene != NULL )
		{
			OwnerScene->RequestFormattingUpdate();
			OwnerScene->RequestSceneUpdate(TRUE,TRUE,TRUE);
		}
	}
}

void WxUIEditorBase::OnToggleOutline( wxCommandEvent& Event )
{
	if ( EditorOptions != NULL )
	{
		UBOOL bNewValue = Event.IsChecked();
		switch ( Event.GetId() )
		{
		case ID_UI_SHOWVIEWPORTOUTLINE:
			EditorOptions->bRenderViewportOutline = bNewValue;
			break;
		case ID_UI_SHOWCONTAINEROUTLINE:
			EditorOptions->bRenderContainerOutline = bNewValue;
			break;
		case ID_UI_SHOWSELECTIONOUTLINE:
			EditorOptions->bRenderSelectionOutline = bNewValue;
			break;
		case ID_UI_SHOWSELECTIONOUTLINE_PERWIDGET:
			EditorOptions->bRenderPerWidgetSelectionOutline = !bNewValue;
			break;
		case ID_UI_SHOWSELECTIONHANDLES:
			EditorOptions->bRenderSelectionHandles = bNewValue;
			break;
		case ID_UI_SHOWTITLESAFEREGION:
			EditorOptions->bRenderTitleSafeRegionOutline = bNewValue;
			break;
		}
	}

	Event.Skip();
}

void WxUIEditorBase::OnToolbar_SelectMode (wxCommandEvent& Event)
{
	if (EditorOptions != NULL)
	{
		int id = Event.GetId();
		SetToolMode(id);
		MainToolBar->Refresh();
	}

	Event.Skip();
}


void WxUIEditorBase::OnMenuDragGrid_UpdateUI( wxUpdateUIEvent& In )
{
	INT MenuId = In.GetId();

	if( IDM_DRAG_GRID_TOGGLE == MenuId )
	{
		In.Check( EditorOptions->bSnapToGrid );
	}
	else 
	{
		INT GridIndex;

		GridIndex = -1;

		if( MenuId >= IDM_DRAG_GRID_1 && MenuId <= IDM_DRAG_GRID_1024 )
		{
			INT Value = 0;

			switch(MenuId)
			{
			case IDM_DRAG_GRID_1:
				Value = 1;
				break;
			case IDM_DRAG_GRID_2:
				Value = 2;
				break;
			case IDM_DRAG_GRID_4:
				Value = 4;
				break;
			case IDM_DRAG_GRID_8:
				Value = 8;
				break;
			case IDM_DRAG_GRID_16:
				Value = 16;
				break;
			case IDM_DRAG_GRID_32:
				Value = 32;
				break;
			case IDM_DRAG_GRID_64:
				Value = 64;
				break;
			case IDM_DRAG_GRID_128:
				Value = 128;
				break;
			case IDM_DRAG_GRID_256:
				Value = 256;
				break;
			}

			In.Check(EditorOptions->GridSize == Value);
		}
		
	}
}

void WxUIEditorBase::OnMenuDragGrid( wxCommandEvent& In )
{
	INT MenuId = In.GetId();

	if( IDM_DRAG_GRID_TOGGLE == MenuId )
	{	
		EditorOptions->bSnapToGrid = !EditorOptions->bSnapToGrid;
	}
	else 
	{
		if( MenuId >= IDM_DRAG_GRID_1 && MenuId <= IDM_DRAG_GRID_256 )
		{
			switch(MenuId)
			{
			case IDM_DRAG_GRID_1:
				EditorOptions->GridSize = 1;
				break;
			case IDM_DRAG_GRID_2:
				EditorOptions->GridSize = 2;
				break;
			case IDM_DRAG_GRID_4:
				EditorOptions->GridSize = 4;
				break;
			case IDM_DRAG_GRID_8:
				EditorOptions->GridSize = 8;
				break;
			case IDM_DRAG_GRID_16:
				EditorOptions->GridSize = 16;
				break;
			case IDM_DRAG_GRID_32:
				EditorOptions->GridSize = 32;
				break;
			case IDM_DRAG_GRID_64:
				EditorOptions->GridSize = 64;
				break;
			case IDM_DRAG_GRID_128:
				EditorOptions->GridSize = 128;
				break;
			case IDM_DRAG_GRID_256:
				EditorOptions->GridSize = 256;
				break;
			}
		}
	}
}

/**
 * Aligns the selected object to the viewport in various ways.
 */
void WxUIEditorBase::OnMenuAlignToViewport( wxCommandEvent& In )
{
	// Get the center of the selection set.
	FVector2D SelectionCenter = (ObjectVC->Selector->EndLocation + ObjectVC->Selector->StartLocation) * 0.5f;
	FVector2D Delta(0.0f, 0.0f);
	FVector2D ViewportOrigin;
	FVector2D ViewportSize;
	
	if(OwnerScene->GetViewportOrigin(ViewportOrigin) && OwnerScene->GetViewportSize(ViewportSize))
	{
		FVector2D ViewportEnd = ViewportOrigin + ViewportSize;
		FVector2D ViewportCenter = ViewportOrigin + ViewportSize * 0.5f;

		switch(In.GetId())
		{
		case ID_UI_ALIGN_VIEWPORT_TOP:
			Delta[1] = ViewportOrigin[1] - ObjectVC->Selector->StartLocation[1];
			break;
		case ID_UI_ALIGN_VIEWPORT_BOTTOM:
			Delta[1] = ViewportEnd[1] - ObjectVC->Selector->EndLocation[1];
			break;
		case ID_UI_ALIGN_VIEWPORT_LEFT:
			Delta[0] = ViewportOrigin[0] - ObjectVC->Selector->StartLocation[0];
			break;
		case ID_UI_ALIGN_VIEWPORT_RIGHT:
			Delta[0] = ViewportEnd[0] - ObjectVC->Selector->EndLocation[0];
			break;
		case ID_UI_ALIGN_VIEWPORT_CENTER_VERTICALLY:
			Delta[1] = ViewportCenter[1] - SelectionCenter[1];
			break;
		case ID_UI_ALIGN_VIEWPORT_CENTER_HORIZONTALLY:
			Delta[0] = ViewportCenter[0] - SelectionCenter[0];
			break;
		default:
			check(0);
			break;
		}

		// Apply the delta to all currently selected widgets.
		FVector Delta3D(Delta[0], Delta[1], 0.0f);
		MoveSelectedObjects(Delta3D);

		PositionPanel->RefreshControls();
	}
}

void WxUIEditorBase::OnMenuAlignText( wxCommandEvent& In )
{
	FScopedTransaction Transaction(*LocalizeUI(TEXT("TransAlignText")));

	TArray<UUIObject*> OutWidgets;
	GetSelectedWidgets(OutWidgets);

	for(INT WidgetIdx=0; WidgetIdx<OutWidgets.Num(); WidgetIdx++)
	{
		UUIObject* Widget = OutWidgets(WidgetIdx);

		if(Widget)
		{
			// see if the widget's class implements the interface we are trying to use.
			if(Widget->GetClass()->ImplementsInterface(IUIStringRenderer::UClassType::StaticClass()))
			{
				EUIAlignment HAlign = UIALIGN_MAX;
				EUIAlignment VAlign = UIALIGN_MAX;

				switch(In.GetId())
				{
				case ID_UI_TEXT_ALIGN_TOP:
					VAlign = UIALIGN_Left;
					break;
				case ID_UI_TEXT_ALIGN_MIDDLE:
					VAlign = UIALIGN_Center;
					break;
				case ID_UI_TEXT_ALIGN_BOTTOM:
					VAlign = UIALIGN_Right;
					break;
				case ID_UI_TEXT_ALIGN_LEFT:
					HAlign = UIALIGN_Left;
					break;
				case ID_UI_TEXT_ALIGN_CENTER:
					HAlign = UIALIGN_Center;
					break;
				case ID_UI_TEXT_ALIGN_RIGHT:
					HAlign = UIALIGN_Right;
					break;
				}

				FScopedObjectStateChange TextAlignmentChangeNotification(Widget);
				IUIStringRenderer* Interface = (IUIStringRenderer*)Widget->GetInterfaceAddress(IUIStringRenderer::UClassType::StaticClass());
				Interface->SetTextAlignment(HAlign, VAlign);
			}
		}
	}
}


/**
 * Callback for the center on selection menu item.  This function centers the viewport on the selected widgets.
 */
void WxUIEditorBase::OnMenuCenterOnSelection( wxCommandEvent& In )
{
	CenterOnSelectedWidgets();
}
/* =======================================
	File menu handlers.
=====================================*/

/**
 * Closes the editor window.
 */
void WxUIEditorBase::OnCloseEditorWindow(wxCommandEvent &In)
{
	Close();
}

/* =======================================
	Edit menu handlers.
=====================================*/
void WxUIEditorBase::OnUndo( wxCommandEvent& Event )
{
	const UBOOL bChangedScene = GEditor->UndoTransaction();
	if ( bChangedScene == TRUE )
	{
		GCallbackEvent->Send( CALLBACK_Undo );
	}
}

void WxUIEditorBase::OnRedo( wxCommandEvent& Event )
{
	const UBOOL bChangedScene = GEditor->RedoTransaction();
	if ( bChangedScene == TRUE )
	{
		GCallbackEvent->Send( CALLBACK_Undo );
	}
}

void WxUIEditorBase::OnCopy( wxCommandEvent& Event )
{
	CopySelectedWidgets();
}

void WxUIEditorBase::OnCut( wxCommandEvent& Event )
{
	CutSelectedWidgets();
}

void WxUIEditorBase::OnPaste( wxCommandEvent& Event )
{
	SynchronizeSelectedWidgets(Event);
	PasteWidgets();
}

void WxUIEditorBase::OnPasteAsChild( wxCommandEvent& Event )
{
	SynchronizeSelectedWidgets(Event);

	//determine which widget to use as the parent for the pasted widgets
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);

	// if any widgets are selected, use the first selected widget as the parent for the
	// pasted widgets
	UUIScreenObject* WidgetParent = SelectedWidgets.Num() > 0
		? SelectedWidgets(0)
		: OwnerScene->GetEditorDefaultParentWidget();

	PasteWidgets(WidgetParent);
}

void WxUIEditorBase::OnContext_Paste( wxCommandEvent& Event )
{
	SynchronizeSelectedWidgets(Event);
	PasteWidgets(NULL,TRUE);
}

void WxUIEditorBase::OnContext_PasteAsChild( wxCommandEvent& Event )
{
	SynchronizeSelectedWidgets(Event);

	//determine which widget to use as the parent for the pasted widgets
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);

	// if any widgets are selected, use the first selected widget as the parent for the
	// pasted widgets
	UUIScreenObject* WidgetParent = SelectedWidgets.Num() > 0
		? SelectedWidgets(0)
		: OwnerScene->GetEditorDefaultParentWidget();

	PasteWidgets(WidgetParent, TRUE);
}

/**
 * Spawns the bind UI Event key dialog so that the user can bind default keys for UI events.
 */
void WxUIEditorBase::OnBindUIEventKeys( wxCommandEvent &Event )
{
	WxDlgUIEventKeyBindings Dlg(this);
	Dlg.ShowModal();
}

/**
 * Spawns the bind UI Datastore Browser Dialog so that the user can select and bind datastores.
 */
void WxUIEditorBase::OnShowDataStoreBrowser( wxCommandEvent& Event )
{
	SceneManager->GetDataStoreBrowser()->Show();
}

void WxUIEditorBase::OnEditMenu_UpdateUI( wxUpdateUIEvent& Event )
{
	switch(Event.GetId())
	{
	case IDM_CUT: case IDM_COPY: 
		{
			UUIObject* SelectedWidget = Selection->GetTop<UUIObject>();

			// Make sure that the clipboard has UI editor data.
			if(SelectedWidget)
			{
				Event.Enable( true );
			}
			else
			{
				Event.Enable( false );
			}
		}
		break;
	case IDM_UNDO:
		Event.Enable( GUnrealEd->Trans->CanUndo() == TRUE );
		Event.SetText( *FString::Printf( LocalizeSecure(LocalizeUnrealEd("Undo_FA"), *GUnrealEd->Trans->GetUndoDesc()) ) );
		break;
	case IDM_REDO:
		Event.Enable( GUnrealEd->Trans->CanRedo() == TRUE );
		Event.SetText( *FString::Printf( LocalizeSecure(LocalizeUnrealEd("Redo_FA"), *GUnrealEd->Trans->GetRedoDesc()) ) );
		break;
	case IDM_UI_CLICK_PASTEASCHILD: case IDM_UI_PASTEASCHILD:
		{
			FString PasteText = appClipboardPaste();
			UUIObject* SelectedWidget = Selection->GetTop<UUIObject>();

			// Make sure that the clipboard has UI editor data.
			if(PasteText.Len() && PasteText.InStr(TEXT("Begin Scene")) == 0 && SelectedWidget != NULL)
			{
				Event.Enable( true );
			}
			else
			{
				Event.Enable( false );
			}
		}
		break;
	case IDM_PASTE: case IDM_UI_CLICK_PASTE:
		{
			FString PasteText = appClipboardPaste();

			// Make sure that the clipboard has UI editor data.
			if(PasteText.Len() && PasteText.InStr(TEXT("Begin Scene")) == 0)
			{
				Event.Enable( true );
			}
			else
			{
				Event.Enable( false );
			}
		}
		break;
	case IDMN_ObjectContext_Delete:
		{
			UUIObject* SelectedWidget = Selection->GetTop<UUIObject>();

			// Make sure that the clipboard has UI editor data.
			if(SelectedWidget)
			{
				Event.Enable( true );
			}
			else
			{
				Event.Enable( false );
			}
		}
		break;
	case IDM_UI_BINDWIDGETSTODATASTORE:
		{
			// don't initialize the data store if the user only wants to open the context menu
			if ( SceneManager->AreDataStoresInitialized() )
			{
				FName Tag;
				const UBOOL bTagSelected = SceneManager->GetDataStoreBrowser()->GetSelectedTag(Tag);

				TArray<UUIObject*> SelectedWidgets;
				GetSelectedWidgets(SelectedWidgets,FALSE);

				const UBOOL bIsValidTag = IsSelectedDataStoreTagValid(SelectedWidgets);
				if( bTagSelected && bIsValidTag )
				{
					Event.Enable(true);
				}
				else
				{
					Event.Enable(false);
				}
			}
			else
			{
				Event.Enable(false);
			}
		}
		break;

	default:
		break;
	}
}
/* =======================================
	Skin menu handlers.
   =====================================*/

/** Called when the user chooses to load an existing skin */
void WxUIEditorBase::OnLoadSkin( wxCommandEvent& Event )
{
	FString FileTypes = TEXT("All Files (*.*)|*.*");

	WxGenericBrowser* gb = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
	check(gb);

	WxFileDialog OpenFileDialog( this, 
		*LocalizeUnrealEd("OpenPackage"), 
		*gb->LastOpenPath, TEXT(""),
		*FileTypes, wxOPEN | wxFILE_MUST_EXIST | wxMULTIPLE);

	if( OpenFileDialog.ShowModal() == wxID_OK )
	{
		TArray<UPackage*> NewPackages;

		wxArrayString OpenFilePaths;
		OpenFileDialog.GetPaths(OpenFilePaths);

		for(UINT FileIndex = 0;FileIndex < OpenFilePaths.Count();FileIndex++)
		{
			const FFilename Filename = (const TCHAR *)OpenFilePaths[FileIndex];

			// if the package is already loaded, skip it
			UObject* ExistingPackage = UObject::StaticFindObject(UPackage::StaticClass(), ANY_PACKAGE, *Filename.GetBaseFilename());
			if (ExistingPackage)
			{
				continue;
			}

			gb->LastOpenPath = Filename.GetPath();

			// record the name of this file to make sure we load objects in this package on top of in-memory objects in this package
			GEditor->UserOpenedFile	= Filename;

			// clear any previous load errors
			EdClearLoadErrors();

			UPackage* Package = UObject::LoadPackage( NULL, *Filename, 0 );

			// display any load errors that happened while loading the package
			if (GEdLoadErrors.Num())
			{
				GCallbackEvent->Send( CALLBACK_DisplayLoadErrors );
			}

			// reset the opened package to nothing
			GEditor->UserOpenedFile	= FString("");

			if( Package )
			{
				NewPackages.AddItem( Package );
			}
		}

		gb->Update();
	}

	StyleBrowser->RefreshAvailableSkinCombo();
}

/** Called when the user selects the "Create New Skin" option in the load skin context menu */
void WxUIEditorBase::OnCreateSkin( wxCommandEvent& Event )
{
	// use the currently selected skin as the template for the new skin
	UUISkin* SkinTemplate = StyleBrowser->StyleTree->GetCurrentSkin();

	WxDlgCreateUISkin dlg;
	dlg.Create(this);

	if ( dlg.ShowModal(TEXT(""), TEXT(""), SkinTemplate) == wxID_OK )
	{
		FString PackageName = dlg.PackageNameString.c_str();
		FString SkinName = dlg.SkinNameString.c_str();
		UUISkin* Template = dlg.GetSkinTemplate();

		UUISkin* NewSkin = CreateUISkin(PackageName, SkinName, Template);
		if ( NewSkin != NULL )
		{
			StyleBrowser->RefreshAvailableSkinCombo(NewSkin);
		}
	}
}

void WxUIEditorBase::OnEditSkin( wxCommandEvent& Event )
{
	// Display the skin editor dialog.
	ShowSkinEditor();
}

void WxUIEditorBase::OnSaveSkin( wxCommandEvent& Event )
{
	UPackage* SkinPackage = SceneManager->ActiveSkin->GetOutermost();
	FString PackageName = SkinPackage->GetName();
	FString ExistingFile;
	FString SaveFileName;
	UBOOL bAbleToSave = FALSE;

	if( GPackageFileCache->FindPackageFile( *PackageName, NULL, ExistingFile ) )
	{
		// If the package already exists, save to the existing filename.
		SaveFileName = ExistingFile;
	}


	if ( SaveFileName.Len() )
	{
		const FScopedBusyCursor BusyCursor;
	
		if(GFileManager->IsReadOnly( *SaveFileName ) == FALSE)
		{
			if(GUnrealEd->Exec( *FString::Printf(TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\""), *PackageName, *SaveFileName) ) )
			{
				bAbleToSave = TRUE;
			}
		}
		else
		{
			debugf(*FString::Printf(LocalizeSecure(LocalizeUnrealEd("Error_CouldntWriteToFile_F"), *SaveFileName)));
		}


	}

	if(bAbleToSave == FALSE)
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_CouldntSavePackage") );
	}
}

void WxUIEditorBase::OnSkinMenu_UpdateUI( wxUpdateUIEvent& Event )
{
	switch(Event.GetId())
	{
	case IDM_UI_EDITSKIN:
		Event.Enable(SceneManager->ActiveSkin != NULL);
		break;
	}
}


/* =======================================
	Scene menu handlers.
   =====================================*/
void WxUIEditorBase::OnForceSceneUpdate( wxCommandEvent& Event )
{
	if ( OwnerScene != NULL )
	{
		// re-populate all data store bindings
		OwnerScene->LoadSceneDataValues();
		// reformat all strings
		OwnerScene->RequestFormattingUpdate();
		// rebuild docking stack, re-evaluate widget positions, reapply styles, rebuild navigation links
		OwnerScene->RequestSceneUpdate(TRUE, TRUE, TRUE, TRUE);
		// simulate a viewport size change to force everything else to update as well.
		OwnerScene->NotifyResolutionChanged(OwnerScene->CurrentViewportSize, OwnerScene->CurrentViewportSize);
	}
}

/* =======================================
	Context menu handlers.
   =====================================*/

/**
 * Called when the user uses the context menu to place an instance of widget class
 */
void WxUIEditorBase::OnContext_CreateNew( wxCommandEvent& Event )
{
	INT CancelIndex = BeginTransaction(LocalizeUI(TEXT("UIEditor_MenuItem_PlaceWidget")));
	SynchronizeSelectedWidgets(Event);

	INT ArchetypeIndex = Event.GetId() - IDM_UI_NEWOBJECT_START;
	check(ArchetypeIndex >= 0);
	check(ArchetypeIndex<SceneManager->UIWidgetResources.Num());

	FUIObjectResourceInfo& Info = SceneManager->UIWidgetResources(ArchetypeIndex);
	checkSlow(Info.UIResource!=NULL);

	//determine the owner for the new widget
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);

	// if only one widget is selected, create the new widget as a child of the selected widget,
	// otherwise, create it as a child of the owner container
	UUIScreenObject* WidgetParent = SelectedWidgets.Num() == 1
		? SelectedWidgets(0)
		: OwnerScene->GetEditorDefaultParentWidget();

	FVector ClickLocation = ObjectVC->LastClick.GetLocation();
	if ( ClickLocation.IsNearlyZero() )
	{
		ClickLocation.X = WidgetParent->GetPosition(UIFACE_Left, EVALPOS_PixelViewport);
		ClickLocation.Y = WidgetParent->GetPosition(UIFACE_Top, EVALPOS_PixelViewport);
	}

	// setup the parameters
	FUIWidgetPlacementParameters Parameters(
		ClickLocation.X, ClickLocation.Y, 30.f, 30.f,
		Cast<UUIObject>(Info.UIResource), WidgetParent, TRUE, TRUE
		);

	// now create the widget
	UUIObject* NewWidget = CreateNewWidget(Parameters);

	// if successful, close the undo transaction
	if ( NewWidget != NULL )
	{
		EndTransaction();
	}
	else
	{
		// otherwise, cancel it
		CancelTransaction(CancelIndex);
	}
}

/**
 * Called when the user uses the context menu to place an instance of a widget archetype
 */
void WxUIEditorBase::OnContext_CreateNewFromArchetype( wxCommandEvent& Event )
{
	UUIPrefab* SelectedArchetype = GEditor->GetSelectedObjects()->GetTop<UUIPrefab>();
	check(SelectedArchetype);

	INT CancelIndex = BeginTransaction(FString::Printf(LocalizeSecure(LocalizeUI(TEXT("UIEditor_MenuItem_PlaceArchetypef")), *SelectedArchetype->GetName())));
	SynchronizeSelectedWidgets(Event);

	//determine the owner for the new widget
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);

	// if only one widget is selected, create the new widget as a child of the selected widget,
	// otherwise, create it as a child of the owner container
	UUIScreenObject* WidgetParent = SelectedWidgets.Num() == 1
		? SelectedWidgets(0)
		: OwnerScene->GetEditorDefaultParentWidget();

	FVector ClickLocation = ObjectVC->LastClick.GetLocation();
	if ( ClickLocation.IsNearlyZero() )
	{
		ClickLocation.X = WidgetParent->GetPosition(UIFACE_Left, EVALPOS_PixelViewport);
		ClickLocation.Y = WidgetParent->GetPosition(UIFACE_Top, EVALPOS_PixelViewport);
	}

	if ( SelectedArchetype->OriginalWidth.GetValue(OwnerScene) < Abs(DELTA) )
	{
		// this prefab was created before OriginalSize was added - give the instance a reasonable default size
		SelectedArchetype->Modify();
		SelectedArchetype->OriginalWidth.SetValue(OwnerScene, 128.f);
	}
	if ( SelectedArchetype->OriginalHeight.GetValue(OwnerScene) < Abs(DELTA) )
	{
		// this prefab was created before OriginalSize was added - give the instance a reasonable default size
		SelectedArchetype->Modify();
		SelectedArchetype->OriginalHeight.SetValue(OwnerScene, 64.f);
	}

	// setup the parameters
	FUIWidgetPlacementParameters Parameters(
		ClickLocation.X, ClickLocation.Y,
		SelectedArchetype->OriginalWidth.GetValue(OwnerScene),
		SelectedArchetype->OriginalHeight.GetValue(OwnerScene),
		SelectedArchetype, WidgetParent, TRUE
		);

	// instance the archetype
	UUIPrefabInstance* NewInstance = InstanceUIPrefab(Parameters);

	// if successful, close the undo transaction
	if ( NewInstance != NULL )
	{
		EndTransaction();
	}
	else
	{
		// otherwise, cancel it
		CancelTransaction(CancelIndex);
	}
}


void WxUIEditorBase::OnContext_Rename( wxCommandEvent& Event )
{
	INT CancelIndex = BeginTransaction(LocalizeUnrealEd(TEXT("Rename")));
	SynchronizeSelectedWidgets(Event);

	UBOOL bRefresh = FALSE;
	
	// Try to rename any widgets that are selected.
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);

	if(SelectedWidgets.Num() > 0)
	{
		bRefresh = DisplayRenameDialog(SelectedWidgets);
	}
	
	// If the root of the scene tree is selected, then the scene had a rename operation performed on it.
	wxTreeItemId RootItem = SceneTree->GetRootItem();

	if(RootItem.IsOk() && (SceneTree->IsSelected(RootItem) == TRUE || SelectedWidgets.Num() == 0) )
	{
		if(OwnerScene != NULL)
		{
			 const UBOOL bRenamedScene = DisplayRenameDialog(OwnerScene);

			 bRefresh = bRefresh || bRenamedScene;

			 // If we renamed the scene, then set the titlebar of the window again.
			 if(bRenamedScene)
			 {
				UpdateTitleBar();
			 }
		}
	}

	// If we renamed atleast 1 object, then update all of the windows.
	if ( bRefresh )
	{
		RefreshLayerBrowser();
		UpdatePropertyWindow();
		EndTransaction();
	}
	else
	{
		CancelTransaction(CancelIndex);
	}
}

void WxUIEditorBase::OnContext_Delete( wxCommandEvent& Event )
{
	SynchronizeSelectedWidgets(Event);
	
	DeleteSelectedWidgets();
}

/**
 * Assigns the selected style to all selected widgets.  Called when the user chooses "Select Style" in the widget context menu.
 */
void WxUIEditorBase::OnContext_SelectStyle( wxCommandEvent& Event )
{
	FScopedTransaction Transaction(*LocalizeUI("TransAssignStyle"));

	// Use the menu ID to style mapping table to find the style that the user selected.
	FUIStyleMenuPair* StyleMenuRef = SelectStyleMenuMap.Find(Event.GetId());
	check(StyleMenuRef);

	UUIStyle* NewStyle = StyleMenuRef->Style;
	check(NewStyle);

	SynchronizeSelectedWidgets(Event);

	// get the list of widgets selected in the tree control
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);

	TArray<FScopedObjectStateChange> StyleChangeNotifications;
	for ( INT SelectionIdx = 0; SelectionIdx < SelectedWidgets.Num(); SelectionIdx++ )
	{
		new(StyleChangeNotifications) FScopedObjectStateChange(SelectedWidgets(SelectionIdx));
	}

	UBOOL bModifiedStyle = FALSE;

	//@hack
	//@fixme ronp - this is really hacky, come up with a better way to do this....
	if ( SelectedWidgets.Num() == 1 && SelectedWidgets(0)->IsA(UUIList::StaticClass()) && StyleMenuRef->StylePropertyId.GetStyleReferenceName() == TEXT("CellStyle") )
	{
		UUIList* SelectedList = Cast<UUIList>(SelectedWidgets(0));

		// we shouldn't have gotten here if the list didn't have a CellDataComponent...
		check(SelectedList->CellDataComponent);

		bModifiedStyle = SelectedList->CellDataComponent->SetCustomCellStyle(NewStyle, (EUIListElementState)StyleMenuRef->ArrayElement, StyleMenuRef->ExtraData);
	}

	if ( bModifiedStyle == FALSE )
	{
		for ( INT WidgetIdx = 0; WidgetIdx < SelectedWidgets.Num(); WidgetIdx++ )
		{
			UUIObject* Widget = SelectedWidgets(WidgetIdx);

			if ( Widget->SetWidgetStyle(NewStyle, StyleMenuRef->StylePropertyId, StyleMenuRef->ArrayElement) )
			{
				Widget->ResolveStyles(TRUE, StyleMenuRef->StylePropertyId);
				bModifiedStyle = TRUE;
			}
		}
	}
		
	// if a widget's style was modified, finalize this undo transaction
	if ( bModifiedStyle == FALSE )
	{
		for ( INT ObjIndex = 0; ObjIndex < StyleChangeNotifications.Num(); ObjIndex++ )
		{
			StyleChangeNotifications(ObjIndex).CancelEdit();
		}
		Transaction.Cancel();
	}

	SelectStyleMenuMap.Empty();
}

/**
 * Invokes the style editor for the style associated with the selected style (or style assigned to the selected widet)
 * Called when the user selects the "Edit Style" context menu option
 */
void WxUIEditorBase::OnContext_EditStyle( wxCommandEvent& Event )
{
	//@todo - while editing a style, highight all widgets that are currently using that style
	SynchronizeSelectedWidgets(Event);

	// get the list of widgets selected in the tree control
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);

	TArray<UUIStyle*> StylesToEdit;

	// first, determine which style we want to edit
	INT StyleMenuID = Event.GetId();
	if ( StyleMenuID == IDM_UI_EDITSTYLE )
	{
		// Use the menu ID to style mapping table to find the style that the user selected.
		FUIStyleMenuPair* StyleMenuRef = EditStyleMenuMap.Find(StyleMenuID);
		check(StyleMenuRef);

		UUIStyle* StyleToEdit = StyleMenuRef->Style;
		check(StyleToEdit);

		StyleBrowser->DisplayEditStyleDialog(StyleToEdit);
	}
	else
	{
		// Use the menu ID to style mapping table to find the style that the user selected.
		FUIStyleMenuPair* StyleMenuRef = EditStyleMenuMap.Find(StyleMenuID);
		check(StyleMenuRef);

		UUIStyle* StyleToEdit = StyleMenuRef->Style;
		check(StyleToEdit);
		StyleBrowser->DisplayEditStyleDialog(StyleToEdit);
	}

	EditStyleMenuMap.Empty();
}

void WxUIEditorBase::OnContext_EditDockingSet( wxCommandEvent& Event )
{
	SynchronizeSelectedWidgets(Event);
	ShowDockingEditor();
}

/**
 * Shows the edit events dialog for any selected widgets.  If no widgets are selected, it shows it for the scene.
 */
void WxUIEditorBase::OnContext_EditEvents( wxCommandEvent& Event )
{
	SynchronizeSelectedWidgets(Event);
	ShowEventEditor();
}

/**
 * Changes the parent for the selected widgets.  Called when the user selects the "Change Parent" command in the context menu.
 */
void WxUIEditorBase::OnContext_ChangeParent( wxCommandEvent& Event )
{
	INT CancelIndex = BeginTransaction(LocalizeUI(TEXT("TransChangeParent")));
	SynchronizeSelectedWidgets(Event);

	UBOOL bSuccess = FALSE;

	// grab the selected widgets
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets, TRUE);

	if ( SelectedWidgets.Num() > 0 )
	{
		// this will contain a list of the current parents of the selected widgets
		TArray<UUIScreenObject*> CurrentParents;

		// get the full list of widgets that exist in the scene
		TArray<UUIObject*> PotentialParents = OwnerScene->GetChildren(TRUE);

		// now purge the any widgets from the list of valid widgets that are ineligable to be the new
		// parent for the selected widgets
		for ( INT SelectedIndex = 0; SelectedIndex < SelectedWidgets.Num(); SelectedIndex++ )
		{
			UUIObject* SelectedWidget = SelectedWidgets(SelectedIndex);
			UUIScreenObject* WidgetParent = SelectedWidget->GetParent();

			if ( WidgetParent != NULL )
			{
				// add this widget's current parent to the list
				CurrentParents.AddUniqueItem(WidgetParent);
			}

			for ( INT ParentIndex = PotentialParents.Num() - 1; ParentIndex >= 0; ParentIndex-- )
			{
				UUIObject* PotentialParent = PotentialParents(ParentIndex);

				// if there is only one widget selected, we can tighten up the restrictions a little bit
				if ( SelectedWidgets.Num() == 1 )
				{
					if ( SelectedWidget == PotentialParent )
					{
						PotentialParents.Remove(ParentIndex);
					}
				}

				// remove any potential parents that are currently children of a selected widget
				if ( SelectedWidget->ContainsChild(PotentialParent) )
				{
					PotentialParents.Remove(ParentIndex);
				}

				// additional constraints here
			}
		}

		if ( CurrentParents.Num() == 1 )
		{
			// this means that all selected widgets had the same parent...in this case, remove that widget from the
			// list of potential parents, unless the common parent is the scene itself (that will be taken care of later)
			UUIObject* CommonParent = Cast<UUIObject>(CurrentParents(0));
			if ( CommonParent != NULL )
			{
				PotentialParents.RemoveItem(CommonParent);
			}
		}

		// We want to display the scene in the list of available widgets, 
		TArray<UUIScreenObject*> ValidParents;
		ValidParents.Empty(PotentialParents.Num()+1);

		// if the common parent is the owner container, don't add it to the list
		UBOOL bContainerIsCommonParent = CurrentParents.Num() == 1 && CurrentParents(0) == OwnerScene;
		// additional reasons why the owner container shouldn't be added go here:

		if ( !bContainerIsCommonParent )
		{
            ValidParents.AddItem(OwnerScene);
		}

		// the owner container might be a UIScene, so we first need to move all potential parents over into an
		// array that can contain both widgets and scenes
		for ( INT ParentIndex = 0; ParentIndex < PotentialParents.Num(); ParentIndex++ )
		{
			UUIObject* PotentialParent = PotentialParents(ParentIndex);
			ValidParents.AddItem(PotentialParent);
		}

		//@fixme ronp - this really shouldn't be hardcoded; better to use a method which returns whether the screen object can accept
		// other child widgets or not.
		if ( OwnerScene->IsA(UUIPrefabScene::StaticClass()) )
		{
			ValidParents.RemoveItem(OwnerScene);
		}

		UUIScreenObject* NewParent = DisplayWidgetSelectionDialog(ValidParents);
		bSuccess = ReparentWidgets(SelectedWidgets, NewParent);
	}

	if ( bSuccess )
	{
		EndTransaction();
	}
	else
	{
		CancelTransaction(CancelIndex);
	}
}

/**
 * Changes the scale type for the position of the selected widgets.  Called when the user selects the "Convert Position" command in the context menu.
 */
void WxUIEditorBase::OnContext_ConvertPosition( wxCommandEvent& Event )
{

	// begin an undo transaction
	INT CancelIndex = BeginTransaction(LocalizeUI(TEXT("TransConvertPosition")));
	SynchronizeSelectedWidgets(Event);

	const INT EventID = Event.GetId() - ID_CONVERT_POSITION_START;

	EPositionEvalType ConversionType = (EPositionEvalType)(EventID % EVALPOS_MAX);
	EUIWidgetFace ConversionFace = (EUIWidgetFace)(EventID / EVALPOS_MAX);

	// get the list of selected widgets
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets, TRUE);

	UBOOL bSuccess = FALSE;
	if ( SelectedWidgets.Num() > 0 )
	{
		for ( INT i = 0; i < SelectedWidgets.Num(); i++ )
		{
			UUIObject* Widget = SelectedWidgets(i);
			FScopedObjectStateChange ScaleTypeChangeNotification(Widget);

			Widget->Position.ChangeScaleType(Widget, ConversionFace, ConversionType);
			bSuccess = TRUE;
		}
	}

	if ( bSuccess )
	{
		EndTransaction();
		UpdatePropertyWindow();
	}
	else
	{
		CancelTransaction(CancelIndex);
	}
}

/**
 * Changes the position of a widget in the widget parent's Children array, which affects the render order of the scene
 * Called when the user selects a "Move Up/Move ..." command in the context menu
 */
void WxUIEditorBase::OnContext_ModifyRenderOrder( wxCommandEvent& Event )
{
	SynchronizeSelectedWidgets(Event);

	INT EventID = Event.GetId();

	switch(EventID)
	{
	case IDM_UI_MOVEUP:
		ReorderSelectedWidgets(RA_MoveUp);
		break;

	case IDM_UI_MOVEDOWN:
		ReorderSelectedWidgets(RA_MoveDown);
		break;

	case IDM_UI_MOVETOTOP:
		ReorderSelectedWidgets(RA_MoveToTop);
		break;

	case IDM_UI_MOVETOBOTTOM:
		ReorderSelectedWidgets(RA_MoveToBottom);
		break;
	}
	
}

void WxUIEditorBase::OnContext_SaveAsResource( wxCommandEvent& Event )
{
	INT CancelIndex = BeginTransaction(LocalizeUnrealEd(TEXT("Archetype_Create")));
	SynchronizeSelectedWidgets(Event);

	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets, TRUE);

	UBOOL bSuccess = CreateUIArchetype(SelectedWidgets);
	if ( bSuccess )
	{
		EndTransaction();
	}
	else
	{
		EndTransaction();

		const UBOOL bChangedScene = GEditor->UndoTransaction();
		if ( bChangedScene == TRUE )
		{
			GCallbackEvent->Send( CALLBACK_Undo );
		}

		//CancelTransaction(CancelIndex);
	}
}

/**
 * Binds the selected widgets to the selected datastore.
 *
 * @fixme ronp - currently no support for widgets which have multiple data store bindings (also @see WxUIMenu::AppendDataStoreMenu)
 */
void WxUIEditorBase::OnContext_BindWidgetsToDataStore( wxCommandEvent& Event )
{
	if ( SceneManager->AreDataStoresInitialized() )
	{
		INT CancelIndex = BeginTransaction(LocalizeUI(TEXT("TransBindDataStore")));

		// Get the selected datastore
		WxDlgUIDataStoreBrowser* DataStoreBrowser = SceneManager->GetDataStoreBrowser();
		UUIDataProvider* CurrentDataProvider = DataStoreBrowser->GetSelectedTagProvider();
		
		UBOOL bSuccess = FALSE;
		if(CurrentDataProvider != NULL)
		{
			FName Tag = NAME_None;
			const UBOOL bTagSelected = DataStoreBrowser->GetSelectedTag(Tag);

			if ( bTagSelected )
			{
				UUIDataStore* OwningDataStore = DataStoreBrowser->GetOwningDataStore(CurrentDataProvider);

				// Get the markup for the current tag.
				FString TagMarkup = CurrentDataProvider->GenerateDataMarkupString(OwningDataStore,Tag);

				const INT EventId = Event.GetId();
				if ( EventId == IDM_UI_BINDWIDGETSTODATASTORE )
				{
					// get the list of selected widgets
					SynchronizeSelectedWidgets(Event);
					TArray<UUIObject*> SelectedWidgets;
					GetSelectedWidgets(SelectedWidgets);

					for( INT WidgetIdx = 0; WidgetIdx < SelectedWidgets.Num(); WidgetIdx++ )
					{
						UUIObject* Widget = SelectedWidgets(WidgetIdx);

						// GetInterfaceUIDataStoreSubscriber() will return NULL if this widget doesn't implement that interface
						IUIDataStoreSubscriber* DataStoreWidget = (IUIDataStoreSubscriber*)Widget->GetInterfaceAddress(IUIDataStoreSubscriber::UClassType::StaticClass());
						if ( DataStoreWidget != NULL )
						{
							FScopedObjectStateChange DataStoreBindingNotification(Widget);

							bSuccess = TRUE;
							DataStoreWidget->SetDataStoreBinding(TagMarkup);
						}
					}
				}
				else
				{
					FUIDataBindingMenuItem* DataBindingItem = SetDataStoreBindingMap.Find(EventId);
					check(DataBindingItem);

					FDataBindingPropertyValueMap& DataBindingProperties = DataBindingItem->DataBindingProperties;
					TArray<FScopedObjectStateChange> DataStoreBindingNotifications;
					
					TArray<FWidgetDataBindingReference> DataBindingValues;
					DataBindingProperties.GenerateValueArray(DataBindingValues);
					for ( INT BindingIndex = 0; BindingIndex < DataBindingValues.Num(); BindingIndex++ )
					{
						FWidgetDataBindingReference& BindingValue = DataBindingValues(BindingIndex);

						IUIDataStoreSubscriber* DataStoreWidget = static_cast<IUIDataStoreSubscriber*>(
							BindingValue.DataBindingOwner->GetInterfaceAddress(IUIDataStoreSubscriber::UClassType::StaticClass()));

						if ( DataStoreWidget != NULL )
						{
							new(DataStoreBindingNotifications) FScopedObjectStateChange(BindingValue.DataBindingOwner);
							for ( INT ValueIndex = 0; ValueIndex < BindingValue.Bindings.Num(); ValueIndex++ )
							{
								FUIDataStoreBinding* Binding = BindingValue.Bindings(ValueIndex);
								DataStoreWidget->SetDataStoreBinding(TagMarkup, Binding->BindingIndex);
								bSuccess = TRUE;
							}
						}
					}
				}
			}
		}

		if ( bSuccess )
		{
			EndTransaction();
			UpdatePropertyWindow();
		}
		else
		{
			CancelTransaction(CancelIndex);
		}
	}
	else
	{
		// if the user has not yet opened the data store browser, re-route any "Bind widget to data store" actions to open the data store browser instead
		OnShowDataStoreBrowser(Event);
	}

	SetDataStoreBindingMap.Empty();
}

/**
 * Clears the datastore of the selected widgets.
 */
void WxUIEditorBase::OnContext_ClearWidgetsDataStore( wxCommandEvent& Event )
{
	UBOOL bSuccess = FALSE;
	INT CancelIndex = BeginTransaction(LocalizeUI(TEXT("TransClearDataStore")));

	const INT EventId = Event.GetId();
	if ( EventId == IDM_UI_CLEARWIDGETSDATASTORE )
	{
		// get the list of selected widgets
		SynchronizeSelectedWidgets(Event);
		TArray<UUIObject*> SelectedWidgets;
		GetSelectedWidgets(SelectedWidgets);

		// load the interface class which will be used for accessing the methods used to work with data stores
		static UClass* DataStoreInterfaceClass = FindObject<UClass>(ANY_PACKAGE,TEXT("Engine.UIDataStoreSubscriber"));
		checkSlow(DataStoreInterfaceClass);

		for(INT WidgetIdx = 0; WidgetIdx < SelectedWidgets.Num(); WidgetIdx++)
		{
			UUIObject* Widget = SelectedWidgets(WidgetIdx);

			// GetInterfaceUIDataStoreSubscriber() will return NULL if this widget doesn't implement that interface
			IUIDataStoreSubscriber* DataStoreWidget = (IUIDataStoreSubscriber*)Widget->GetInterfaceAddress(IUIDataStoreSubscriber::UClassType::StaticClass());
			if ( DataStoreWidget != NULL )
			{
				FScopedObjectStateChange DataStoreBindingNotification(Widget);

				bSuccess = TRUE;
				DataStoreWidget->SetDataStoreBinding(TEXT(""));
				DataStoreWidget->ClearBoundDataStores();
			}
		}
	}
	else
	{
		FUIDataBindingMenuItem* DataBindingItem = ClearDataStoreBindingMap.Find(EventId);
		check(DataBindingItem);

		FDataBindingPropertyValueMap& DataBindingProperties = DataBindingItem->DataBindingProperties;
		TArray<FScopedObjectStateChange> DataStoreBindingNotifications;

		TArray<FWidgetDataBindingReference> DataBindingValues;
		DataBindingProperties.GenerateValueArray(DataBindingValues);
		for ( INT BindingIndex = 0; BindingIndex < DataBindingValues.Num(); BindingIndex++ )
		{
			FWidgetDataBindingReference& BindingValue = DataBindingValues(BindingIndex);

			IUIDataStoreSubscriber* DataStoreWidget = static_cast<IUIDataStoreSubscriber*>(
				BindingValue.DataBindingOwner->GetInterfaceAddress(IUIDataStoreSubscriber::UClassType::StaticClass()));

			if ( DataStoreWidget != NULL )
			{
				new(DataStoreBindingNotifications) FScopedObjectStateChange(BindingValue.DataBindingOwner);
				for ( INT ValueIndex = 0; ValueIndex < BindingValue.Bindings.Num(); ValueIndex++ )
				{
					FUIDataStoreBinding* Binding = BindingValue.Bindings(ValueIndex);
					DataStoreWidget->SetDataStoreBinding(TEXT(""), Binding->BindingIndex);
					Binding->ClearDataBinding();
					bSuccess = TRUE;
				}
			}
		}
	}


	if ( bSuccess )
	{
		EndTransaction();
		UpdatePropertyWindow();
	}
	else
	{
		CancelTransaction(CancelIndex);
	}

	ClearDataStoreBindingMap.Empty();
}

void WxUIEditorBase::OnContext_ShowKismet( wxCommandEvent& Event )
{
	SynchronizeSelectedWidgets(Event);

	// get the list of selected widgets
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);

	UBOOL bSuccess = FALSE;
	UUIScreenObject* Selected = NULL;
	if ( SelectedWidgets.Num() > 0 )
	{
		Selected = SelectedWidgets(0);
	}
	else
	{
		Selected = OwnerScene->GetEditorDefaultParentWidget();
	}

	check(Selected);
	UUISequence* Sequence = Selected->EventProvider->EventContainer;

	WxUIKismetEditor::OpenUISequenceInKismet(Sequence, this);
}

void WxUIEditorBase::OnContext_DumpProperties( wxCommandEvent& Event )
{
	SynchronizeSelectedWidgets(Event);
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets( SelectedWidgets );

	for ( INT i = 0; i < SelectedWidgets.Num(); i++ )
	{
		GEditor->Exec(*FString::Printf(TEXT("OBJ DUMP %s"), *SelectedWidgets(i)->GetPathName()));
	}
}
/* =======================================
	UITabControl custom context menu item handlers.
=====================================*/
void WxUIEditorBase::OnContext_InsertTabPage( wxCommandEvent& Event )
{
	FScopedTransaction Transaction(*LocalizeUI(TEXT("TransInsertTabPage")));

	// Apply the new element to the list selected
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);

	UBOOL bSuccessful = FALSE;

	// currently only modifying a single tab control at a time is supported
	if( SelectedWidgets.Num() == 1 )
	{
		UUIObject* ClickedWidget = SelectedWidgets(0);

		UUITabControl* TabControl = NULL;
		for ( UUIObject* NextOwner = ClickedWidget; TabControl == NULL && NextOwner; NextOwner = NextOwner->GetOwner() )
		{
			TabControl = Cast<UUITabControl>(NextOwner);
		}

		// this menu option should not be available if the selected widget isn't a tab control/page/button, so assert so that this
		// code is updated if this ever changes
		check(TabControl);

		TArray<UClass*> ValidPageClasses;

		// next, generate a list of valid UITabPage classes to use
		for ( TObjectIterator<UClass> It; It; ++It )
		{
			if ( It->IsChildOf(UUITabPage::StaticClass())
			&&	!It->HasAnyClassFlags(CLASS_Abstract|CLASS_HideDropDown) )
			{
				ValidPageClasses.AddItem(*It);
			}
		}

		UClass* TabPageClass = UUITabPage::StaticClass();

		// if there is more than one option, show a dialog which allows the user to select the class to use
		if ( ValidPageClasses.Num() > 1 )
		{
			TArray<FString> ClassNames;
			INT SelectedItem = 0;
			for ( INT ClassIndex = 0; ClassIndex < ValidPageClasses.Num(); ClassIndex++ )
			{
				if ( ValidPageClasses(ClassIndex) == UUITabPage::StaticClass() )
				{
					SelectedItem = ClassIndex;
				}
				new(ClassNames) FString(ValidPageClasses(ClassIndex)->GetDescription());
			}

			WxDlgGenericComboEntry ClassSelectionDialog;
			if ( ClassSelectionDialog.ShowModal(*LocalizeUI(TEXT("DlgChooseTabPageClass_Title")), *LocalizeUI(TEXT("DlgChooseTabPageClass_ComboCaption")), 
				ClassNames, SelectedItem, FALSE) == wxID_OK )
			{
				INT SelectedIndex = ClassSelectionDialog.GetSelectedIndex();
				if ( ValidPageClasses.IsValidIndex(SelectedIndex) )
				{
					TabPageClass = ValidPageClasses(SelectedIndex);
				}
			}
			else
			{
				TabPageClass = NULL;
			}
		}

		if ( TabPageClass != NULL )
		{
			FScopedObjectStateChange InsertPageNotification(TabControl);

			// create the tab page and tab button
			UUITabPage* NewTabPage = TabControl->CreateTabPage(TabPageClass);

			// insert it into the tab control's list
			if ( NewTabPage != NULL && TabControl->eventInsertPage(NewTabPage, 0) )
			{
				bSuccessful = TRUE;
			}
			else
			{
				InsertPageNotification.CancelEdit();
			}
		}
	}

	if ( bSuccessful == TRUE )
	{
		UpdatePropertyWindow();
	}
	else
	{
		Transaction.Cancel();
	}
}

void WxUIEditorBase::OnContext_RemoveTabPage( wxCommandEvent& Event )
{
	FScopedTransaction Transaction(*LocalizeUI(TEXT("TransRemoveTabPage")));

	// Apply the new element to the list selected
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);

	UBOOL bSuccessful = FALSE;

	// currently only modifying a single tab control at a time is supported
	if( SelectedWidgets.Num() == 1 )
	{
		UUIObject* ClickedWidget = SelectedWidgets(0);

		UUITabControl* TabControl = NULL;
		for ( UUIObject* NextOwner = ClickedWidget; TabControl == NULL && NextOwner; NextOwner = NextOwner->GetOwner() )
		{
			TabControl = Cast<UUITabControl>(NextOwner);
		}

		// this menu option should not be available if the selected widget isn't a tab control/page/button, so assert so that this
		// code is updated if this ever changes
		check(TabControl);
		TArray<FString> PageCaptions;

		// next, generate a list of button captions
		for ( INT PageIndex = 0; PageIndex < TabControl->GetPageCount(); PageIndex++ )
		{
			UUITabPage* TabPage = TabControl->GetPageAtIndex(PageIndex);
			if ( TabPage != NULL )
			{
				// quick and dirty - just show the numbers for now
				PageCaptions.AddItem(appItoa(PageIndex));
			}
		}

		UUITabPage* PageToRemove = NULL;

		// if there is more than one option, show a dialog which allows the user to select the class to use
		if ( PageCaptions.Num() > 1 )
		{
			WxDlgGenericComboEntry PageSelectionDialog;
			if ( PageSelectionDialog.ShowModal(*LocalizeUI(TEXT("DlgRemoveTabPage_Title")), *LocalizeUI(TEXT("DlgRemoveTabPage_ComboCaption")), PageCaptions, 0, FALSE) == wxID_OK )
			{
				INT SelectedIndex = PageSelectionDialog.GetSelectedIndex();
				if ( PageCaptions.IsValidIndex(SelectedIndex) )
				{
					INT PageIndex = appAtoi(*PageCaptions(SelectedIndex));
					PageToRemove = TabControl->GetPageAtIndex(PageIndex);
				}
			}
		}
		else
		{
			INT PageIndex = appAtoi(*PageCaptions(0));
			PageToRemove = TabControl->GetPageAtIndex(PageIndex);
		}

		if ( PageToRemove != NULL )
		{
			FScopedObjectStateChange TabControlNotification(TabControl),
				TabButtonNotification(PageToRemove->TabButton), TabPageNotification(PageToRemove);

			bSuccessful = TabControl->eventRemovePage(PageToRemove,0);

			TabPageNotification.FinishEdit(!bSuccessful);
			TabButtonNotification.FinishEdit(!bSuccessful);
			TabControlNotification.FinishEdit(!bSuccessful);
		}
	}

	if ( bSuccessful == TRUE )
	{
		UpdatePropertyWindow();
	}
	else
	{
		Transaction.Cancel();
	}
}


void WxUIEditorBase::OnContext_InsertCalloutButton( wxCommandEvent& Event )
{
	FScopedTransaction Transaction(*LocalizeUI(TEXT("TransInsertCalloutButton")));

	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);

	UBOOL bSuccessful = FALSE;

	// currently only modifying a single callout button panel is supported
	if ( SelectedWidgets.Num() == 1 )
	{
		UUIObject* ClickedWidget = SelectedWidgets(0);

		// user may have clicked on the callout panel or one of its buttons.  find the panel
		UUICalloutButtonPanel* ButtonPanel = NULL;
		for ( UUIObject* NextOwner = ClickedWidget; ButtonPanel == NULL && NextOwner != NULL; NextOwner = NextOwner->GetOwner() )
		{
			ButtonPanel = Cast<UUICalloutButtonPanel>(NextOwner);
		}

		// this menu option should not be available if the selected widget isn't a UICalloutButtonPanel or UICalloutButton, so assert
		// so that this code is updated if this ever changes
		check(ButtonPanel);

		UClass* RequiredMetaClass = UUICalloutButton::StaticClass();
		if ( ButtonPanel->ButtonTemplate != NULL )
		{
			RequiredMetaClass = ButtonPanel->ButtonTemplate->GetClass();
		}

		WxDlgAddCalloutButton AddCalloutButtonDialog(ButtonPanel, this);
		if ( AddCalloutButtonDialog.ShowModal() == wxID_OK )
		{
			FName SelectedInputAlias = AddCalloutButtonDialog.GetSelectedAlias();
			FName DesiredWidgetTag = AddCalloutButtonDialog.GetButtonTag();
			FString SelectedButtonClassName = AddCalloutButtonDialog.GetSelectedClassName();

			UClass* ButtonClass = NULL;
			if ( SelectedButtonClassName.Len() > 0 )
			{
				ButtonClass = FindObject<UClass>(ANY_PACKAGE, *SelectedButtonClassName);
			}

			if ( ButtonClass != NULL && SelectedInputAlias != NAME_None && DesiredWidgetTag != NAME_None)
			{
				FScopedObjectStateChange AddButtonNotification(ButtonPanel);

				// create the callout button
				//@fixme ronp - currently doesn't use the selected ButtonClass - always uses the class of the button template from the panel class
				UUICalloutButton* NewButton = ButtonPanel->CreateCalloutButton(SelectedInputAlias, DesiredWidgetTag);
				if ( NewButton != NULL )
				{
					bSuccessful = TRUE;
				}
				else
				{
					AddButtonNotification.CancelEdit();
				}
			}
		}
	}

	if ( bSuccessful )
	{
		UpdatePropertyWindow();
	}
	else
	{
		Transaction.Cancel();
	}
}

void WxUIEditorBase::OnContext_RemoveCalloutButton( wxCommandEvent& Event )
{
	FScopedTransaction Transaction(*LocalizeUI(TEXT("TransRemoveCalloutButton")));

	// get the list of selected widgets
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);

	UBOOL bSuccessful = FALSE;
	for ( INT SelectedIdx = 0; SelectedIdx < SelectedWidgets.Num(); SelectedIdx++ )
	{
		UUICalloutButton* SelectedButton = Cast<UUICalloutButton>(SelectedWidgets(SelectedIdx));
		if ( SelectedButton != NULL )
		{
			UUICalloutButtonPanel* ButtonPanel = NULL;
			for ( UUIObject* NextOwner = SelectedButton; ButtonPanel == NULL && NextOwner != NULL; NextOwner = NextOwner->GetOwner() )
			{
				ButtonPanel = Cast<UUICalloutButtonPanel>(NextOwner);
			}

			// this menu option should only be available if a UICalloutButton was selected, and it should always be contained within a
			// UICalloutButtonPanel, so assert on this so that this code can be updated if this assumption changes
			check(ButtonPanel);
			FScopedObjectStateChange ButtonPanelNotification(ButtonPanel), ButtonNotification(SelectedButton);
			bSuccessful = ButtonPanel->eventRemoveButton(SelectedButton);
			if ( bSuccessful )
			{
				// clear the RF_Public flag so that we don't create a redirect
				SelectedButton->ClearFlags(RF_Public);
				SelectedButton->Rename(NULL, UObject::GetTransientPackage(), REN_ForceNoResetLoaders);

				// UUIScreenObject::RemoveChild will clear the value of UIObject::Owner, but once we've renamed a UIPrefab widget into the transient
				// package (when deleting), there is no way to determine which UIPrefab the widget came from so LoadInstancesFromPropagationArchive()
				// can't be called for that archetype widget.  To workaround this, here we restore the widget's Owner reference so that
				// UUIScreenObject::IsInUIPrefab() can use the Owner if the widget is no longer part of the parent-child hierarchy 
				SelectedButton->Owner = ButtonPanel;
			}

			ButtonPanelNotification.FinishEdit(!bSuccessful);
			ButtonNotification.FinishEdit(!bSuccessful);
		}
	}

	if ( bSuccessful == TRUE )
	{
		RefreshLayerBrowser();
		
		Selection->Modify();

		Selection->BeginBatchSelectOperation();
		Selection->DeselectAll();
		Selection->EndBatchSelectOperation();

		UpdatePropertyWindow();
	}
	else
	{
		Transaction.Cancel();
	}
}

void WxUIEditorBase::OnContext_ChangeCalloutButtonAlias( wxCommandEvent& Event )
{
	FScopedTransaction Transaction(*LocalizeUI(TEXT("TransChangeCalloutAlias")));

	// get the list of selected widgets
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);

	UBOOL bSuccessful = FALSE;
	if ( SelectedWidgets.Num() == 1 )
	{
		UUICalloutButton* CalloutButton = Cast<UUICalloutButton>(SelectedWidgets(0));
		if ( CalloutButton != NULL )
		{
			WxDlgAddCalloutButton AddCalloutButtonDialog(CalloutButton, this);
			if ( AddCalloutButtonDialog.ShowModal() == wxID_OK )
			{
				FName SelectedInputAlias = AddCalloutButtonDialog.GetSelectedAlias();
				if ( SelectedInputAlias != NAME_None )
				{
					FScopedObjectStateChange SetAliasNotification(CalloutButton);

					UUICalloutButtonPanel* ButtonPanel = CastChecked<UUICalloutButtonPanel>(CalloutButton->GetOwner());
					if ( ButtonPanel->eventSetButtonInputAlias(CalloutButton->InputAliasTag, SelectedInputAlias) )
					{
						bSuccessful = TRUE;
					}
					else
					{
						SetAliasNotification.CancelEdit();
					}
				}
			}
		}
	}
	else
	{
		appMsgf(AMT_OK, *LocalizeUI(TEXT("Error_ChangeCalloutAlias_Multiple")));
	}

	if ( bSuccessful )
	{
		UpdatePropertyWindow();
	}
	else
	{
		Transaction.Cancel();
	}
}

/* =======================================
	UI List Context menu handlers.
=====================================*/
void WxUIEditorBase::OnContext_BindListElement( wxCommandEvent& Event )
{
	FScopedTransaction Transaction(*LocalizeUI(TEXT("TransModifyCellBindings")));

	// Apply the new element to the list selected
	TArray<UUIObject*> SelectedWidgets;
	GetSelectedWidgets(SelectedWidgets);

	UBOOL bSuccessful = FALSE;
	if(SelectedWidgets.Num() == 1)
	{
		UUIList* ListWidget = Cast<UUIList>(SelectedWidgets(0));

		//Make sure that this is only called when we only have lists selected.
		check(ListWidget);

		// Get the data tag name using the element idx.
		const FName DataStoreField = ListWidget->DataSource.DataStoreField;
		if ( ListWidget->DataProvider )
		{
			FScopedObjectStateChange ListCellNotifier(ListWidget), ListCellDataNotifier(ListWidget->CellDataComponent);
			TScriptInterface<IUIListElementCellProvider>  SchemaProvider = ListWidget->DataProvider->GetElementCellSchemaProvider(DataStoreField);
			if ( SchemaProvider )
			{
				TMap<FName,FString> AvailableCellBindings;
				TArray<FName> CellTags;
				SchemaProvider->GetElementCellTags(DataStoreField,AvailableCellBindings);
				AvailableCellBindings.GenerateKeyArray(CellTags);

				const UBOOL bIsComboBoxList = ListWidget->GetOuter()->IsA(UUIComboBox::StaticClass());
				const INT AbsIdx = Event.GetId() - IDM_UI_LISTELEMENTTAG_START;

				// divide/mod by the number of cell tags available; if not a combobox, add 1 for the "clear binding" item
				INT ElementIdx = AbsIdx % (CellTags.Num() + (bIsComboBoxList ? 0 : 1));
				INT ColumnIdx = AbsIdx / (CellTags.Num() + (bIsComboBoxList ? 0 : 1));

				if(ColumnIdx >= 0)
				{
					if ( ElementIdx > 0 && ElementIdx <= CellTags.Num() )
					{
						FName CellTag = CellTags(ElementIdx - 1);

						INT ListCellCount = ListWidget->GetTotalColumnCount();

						const FString* ColumnHeader = AvailableCellBindings.Find(CellTag);
						if ( ColumnIdx > ListCellCount )
						{
							// a ColumnIdx higher than the list's cell count indicates that the user wants to insert a column
							FCellHitDetectionInfo ClickedCell;
							FVector LastClickLocation = ObjectVC->LastClick.GetLocation();
							FIntPoint HitLocation(appTrunc(LastClickLocation.X), appTrunc(LastClickLocation.Y));

							INT ClickedColumn = INDEX_NONE;
							if ( ListWidget->CalculateCellFromPosition(HitLocation, ClickedCell) )
							{
								ClickedColumn = ClickedCell.HitColumn;
							}

							// attempt to insert the selected tag just before the column the user clicked on
							if ( ListWidget->InsertSchemaCell(ClickedColumn, CellTag, ColumnHeader ? *ColumnHeader : TEXT("")) )
							{
								bSuccessful = TRUE;
							}
						}
						else
						{
							// SetCellBinding handles the range checking for column idx.
							if ( ListWidget->SetCellBinding(CellTag, ColumnHeader ? *ColumnHeader : TEXT(""), ColumnIdx) )
							{
								bSuccessful = TRUE;
							}
						}
					}
					else
					{
						// ClearCellBinding will automatically remove the specified column from the list
						if ( ListWidget->ClearCellBinding(ColumnIdx) )
						{
							bSuccessful = TRUE;
						}
					}
				}
			}

			if ( !bSuccessful )
			{
				ListCellNotifier.CancelEdit();
				ListCellDataNotifier.CancelEdit();
			}
		}
	}

	if ( bSuccessful == TRUE )
	{
		UpdatePropertyWindow();
	}
	else
	{
		Transaction.Cancel();
	}
}


/**
 * Set the tool mode and redraw buttons
 */
void WxUIEditorBase::SetToolMode (INT Mode)
{
	// Make sure that this is a valid mode.
	INT WidgetID = -1;
	const UBOOL bIsCustomMode = (Mode >= ID_UITOOL_CUSTOM_MODE_START && Mode <= ID_UITOOL_CUSTOM_MODE_END);
	if(bIsCustomMode == TRUE)
	{
		const UBOOL bIsValidID = MainToolBar->GetWidgetIDFromToolID(Mode, WidgetID);

		if(bIsValidID == FALSE)
		{
			return;
		}
	}

	// Set the new mode and refresh the toolbar to have the tool mode buttons mirror the new state.
	if(WidgetTool)
	{
		WidgetTool->ExitToolMode();
		delete WidgetTool;
		WidgetTool = NULL;
	}

	if(bIsCustomMode == TRUE)
	{
		WidgetTool = new FUIWidgetTool_CustomCreate(this, WidgetID);
	}
	else if(Mode ==ID_UITOOL_FOCUSCHAIN_MODE )
	{
		WidgetTool = new FUIWidgetTool_FocusChain(this);
	}
	else if ( Mode == ID_UITOOL_SELECTION_MODE )
	{
		WidgetTool = new FUIWidgetTool_Selection(this);
	}
	else
	{
		appMsgf(AMT_OK, TEXT("This button has not been hooked up yet!"));
		return;
	}

	WidgetTool->EnterToolMode();

	EditorOptions->ToolMode = Mode;
	MainToolBar->Refresh();
}

/* ==========================================================================================================
	WxUIKismetUpdate
========================================================================================================== */
BEGIN_EVENT_TABLE( WxUIKismetUpdate, WxKismetUpdate )
	EVT_CLOSE( WxUIKismetUpdate::OnClose )
END_EVENT_TABLE()

/**
 * Generates a list of sequence objects that are out of date and populates the list with their names
 */
void WxUIKismetUpdate::BuildUpdateList()
{
	if ( Kismet != NULL && UpdateList != NULL )
	{
		UpdateList->Freeze();
		UpdateList->Clear();

		if ( Kismet->Sequence != NULL )
		{
			TArray<USequence*> SequencesToUpdate;
			SequencesToUpdate.AddItem(Kismet->Sequence);

			for ( INT SequenceIndex = 0; SequenceIndex < SequencesToUpdate.Num(); SequenceIndex++ )
			{
				USequence* NextSequence = SequencesToUpdate(SequenceIndex);
				for ( INT ObjIdx = 0; ObjIdx < NextSequence->SequenceObjects.Num(); ObjIdx++ )
				{
					USequenceObject* SeqObj = NextSequence->SequenceObjects(ObjIdx);
					USequence* NestedSequence = Cast<USequence>(SeqObj);
					if ( NestedSequence != NULL )
					{
						SequencesToUpdate.AddItem(NestedSequence);
					}
					else if ( SeqObj->eventGetObjClassVersion() != SeqObj->ObjInstanceVersion )
					{
						UpdateList->Append(*SeqObj->GetSeqObjFullName(), SeqObj);
					}
				}
			}
		}

		UpdateList->Thaw();
	}
}

void WxUIKismetUpdate::OnClose(wxCloseEvent& Event)
{
	if ( Kismet != NULL )
	{
		if ( Kismet->ToolBar != NULL )
		{
			Kismet->ToolBar->ToggleTool(IDM_KISMET_OPENUPDATE, false);
		}

		((WxUIKismetEditor*)Kismet)->SequenceUpdateDialog = NULL;
	}

	WxKismetUpdate::OnClose(Event);
}


/* ==========================================================================================================
	WxUIKismetEditor
========================================================================================================== */
//IMPLEMENT_DYNAMIC_CLASS(WxUIKismetEditor,WxKismet)
BEGIN_EVENT_TABLE (WxUIKismetEditor, WxKismet)
	EVT_MENU( IDM_UI_FORCETREEREFRESH,  WxUIKismetEditor::OnRefreshTree)
	EVT_TREE_SEL_CHANGED( IDM_LINKEDOBJED_TREE, WxUIKismetEditor::OnTreeItemSelected )

	EVT_MENU(IDM_UIKISMET_CALLOUT_ADD, WxUIKismetEditor::OnContext_AddCalloutButton)
	EVT_MENU(IDM_UIKISMET_CALLOUT_REMOVE, WxUIKismetEditor::OnContext_RemoveCalloutButton)
	EVT_MENU_RANGE(IDM_UIKISMET_CALLOUT_CHANGE_START, IDM_UIKISMET_CALLOUT_CHANGE_END, WxUIKismetEditor::OnContext_ChangeCalloutButtonAlias)
	EVT_TOOL(IDM_KISMET_OPENUPDATE, WxUIKismetEditor::OnOpenUpdate)

END_EVENT_TABLE()

/** Constructor */
WxUIKismetEditor::WxUIKismetEditor( WxUIEditorBase* InParent, wxWindowID InID, class UUIScreenObject* InSequenceOwner, FString Title/*=TEXT("Kismet")*/ )
: WxKismet(InParent,InID,InSequenceOwner->EventProvider->EventContainer), SequenceOwner(InSequenceOwner)
, SequenceUpdateDialog(NULL)
, UpdatesLocked(0)
, UIEditor(InParent)
{
	// Add meta objects for all state sequences.
	CreateStateSequenceMetaObject(InSequenceOwner);
}

WxUIKismetEditor::~WxUIKismetEditor()
{
}

/** Called when the user clicks the "Update List" button. */
void WxUIKismetEditor::OnOpenUpdate( wxCommandEvent &In )
{
	if ( SequenceUpdateDialog == NULL )
	{
		SequenceUpdateDialog = new WxUIKismetUpdate(this, wxID_ANY);
		SequenceUpdateDialog->Show();
		SequenceUpdateDialog->BuildUpdateList();

		ToolBar->ToggleTool(IDM_KISMET_OPENUPDATE, true);
	}
	else
	{
		SequenceUpdateDialog->Close();
		SequenceUpdateDialog = NULL;
		ToolBar->ToggleTool(IDM_KISMET_OPENUPDATE, false);
	}
}


/**
 * Display the kismet editor for the specified sequence.  If there is already a kismet editor
 * open for the sequence, brings it to the front and gives focus.  Otherwise, creates a new kismet
 * editor for the sequence.
 *
 * @param	InSeq			the sequence to edit; the global sequence for this sequence's owner widget will be used as
 *							the root node of the kismet editor
 * @param	ParentWindow	the window which contains the sequence's owner widget
 *
 * @return	reference to the kismet editor window that was opened
 */
WxUIKismetEditor* WxUIKismetEditor::OpenUISequenceInKismet(UUISequence* InSeq, WxUIEditorBase* ParentWindow)
{
	WxUIKismetEditor* Result = NULL;

	if ( InSeq != NULL && ParentWindow != NULL )
	{
		UUIScreenObject* SeqOwner = InSeq->GetOwner();
		if ( SeqOwner != NULL && SeqOwner->EventProvider != NULL && SeqOwner->EventProvider->EventContainer != NULL )
		{
			UUISequence* SequenceRoot = SeqOwner->EventProvider->EventContainer;

			// first search existing kismet editors to see if we already have one open for this sequence
			for ( INT Idx = 0; Idx < GApp->KismetWindows.Num(); Idx++)
			{
				WxKismet* KismetWindow = GApp->KismetWindows(Idx);
				if ( KismetWindow->GetRootSequence() == SequenceRoot )
				{
					KismetWindow->Show();
					KismetWindow->SetFocus();

					Result = static_cast<WxUIKismetEditor*>(KismetWindow);
					break;
				}
			}

			if ( Result == NULL )
			{
				FString KismetWindowTitle = FString::Printf(TEXT("Kismet: %s"), *SeqOwner->GetTag().ToString());
				WxUIKismetEditor* KismetEditor = new WxUIKismetEditor(ParentWindow, -1, SeqOwner, KismetWindowTitle);
				KismetEditor->InitEditor();
				KismetEditor->Show();
				Result = KismetEditor;
			}

			// if the sequence passed in is not the root sequence for the owning widget, select the selected sequence now
			if ( Result != NULL && InSeq != SequenceRoot )
			{
				Result->ChangeActiveSequence(InSeq, TRUE);
			}
		}
	}

	return Result;
}

/**
 * Creates the tree control for this linked object editor.  Only called if TRUE is specified for bTreeControl
 * in the constructor.
 *
 * @param	TreeParent	the window that should be the parent for the tree control
 */
void WxUIKismetEditor::CreateTreeControl( wxWindow* TreeParent )
{
	TreeControl = new WxUIKismetTreeControl;
	((WxUIKismetTreeControl*)TreeControl)->Create(TreeParent, IDM_LINKEDOBJED_TREE, this);
}

/**
 * One or more objects changed, so tell the tree
 */
void WxUIKismetEditor::NotifyObjectsChanged()
{
	UpdatesLocked++;
	{
		((WxUIKismetTreeControl*)TreeControl)->NotifyObjectsChanged();
	}
	UpdatesLocked--;
}

/** Forces the kismet editor tree control to be refreshed. */
void WxUIKismetEditor::OnRefreshTree( wxCommandEvent& Event )
{
	NotifyObjectsChanged();
}
/** 
 * Called when a new item has been selected in the tree control.  If the selected item is a UIState, updates the
 * property window with the properties for the state selected.
 */
void WxUIKismetEditor::OnTreeItemSelected( wxTreeEvent &In )
{
	wxTreeItemId TreeId = In.GetItem();
	if (TreeId.IsOk() && UpdatesLocked == 0)
	{
		UpdatesLocked++;
		{
			WxTreeObjectWrapper* LeafData = (WxTreeObjectWrapper*)TreeControl->GetItemData(TreeId);
			if (LeafData)
			{
				UObject *clicked = LeafData->GetObject<UObject>();

				USequence *seq = Cast<USequence>(clicked);
				if (seq)
				{
					ChangeActiveSequence (seq, TRUE);

					// if we don't have any sequence objects selected and the selected item in the tree control is a sequence that
					// belongs to a UIState, display the properties for the UIState
					UUIStateSequence* SelectedSequence = Cast<UUIStateSequence>(seq);
					if ( SelectedSequence != NULL )
					{
						PropertyWindow->SetObject(SelectedSequence->GetOwnerState(), TRUE, TRUE, FALSE);
					}
				}
				else	// Not sequence
				{
					USequenceObject *obj = Cast<USequenceObject>(clicked);

					if (obj && (In.GetItem() != In.GetOldItem()) )
					{
						CenterViewOnSeqObj (obj);
					}
				}
			}
		}
		UpdatesLocked--;
	}
}

/**
 * Node was added to selection in view so select in tree
 *
 * @param	Obj		Object to select
 */
void WxUIKismetEditor::AddToSelection( UObject* Obj )
{
	Super::AddToSelection (Obj);

	USequenceObject *seqObj = Cast<USequenceObject>(Obj);
	if (seqObj)
	{
		UpdatesLocked++;
		{
			((WxUIKismetTreeControl *)TreeControl)->SelectObject (seqObj);
		}
		UpdatesLocked--;
	}
}

/**
 * Selection set was emptied, so unselect everything in the tree.
 */
void WxUIKismetEditor::EmptySelection()
{
	Super::EmptySelection();

	UpdatesLocked++;
	{
		TreeControl->UnselectAll();
	}
	UpdatesLocked--;
}
	
/**
 * Node was removed from selection in view so deselect in tree.
 *
 * @param	Obj		Object to deselect in tree.
 */
void WxUIKismetEditor::RemoveFromSelection( UObject* Obj )
{
	Super::RemoveFromSelection(Obj);

	USequenceObject *seqObj = Cast<USequenceObject>(Obj);
	if (seqObj)
	{
		UpdatesLocked++;
		{
			((WxUIKismetTreeControl *)TreeControl)->SelectObject (seqObj, FALSE);
		}
		UpdatesLocked--;
	}
}

/**
 * Updates the property window with the properties for the currently selected sequence objects, or if no sequence objects are selected
 * and a UIState is selected in the tree control, fills the property window with the properties of the selected state.
 */
void WxUIKismetEditor::UpdatePropertyWindow()
{
	if ( SelectedSeqObjs.Num() > 0 )
	{
		PropertyWindow->SetObjectArray( SelectedSeqObjs, TRUE, TRUE, FALSE );
	}
	else 
	{
		// We need to make sure that the user can't selected multiple sequences at once.  
		wxArrayTreeItemIds SelectionSet;
		

		INT NumSelections = TreeControl->GetSelections(SelectionSet);
		UBOOL bFoundSequence = FALSE;

		for(INT SelectionIdx = 0; SelectionIdx < NumSelections; SelectionIdx++)
		{
			wxTreeItemId SelectionItemId = SelectionSet[SelectionIdx];

			if ( SelectionItemId.IsOk() )
			{
				WxTreeObjectWrapper* LeafData	= (WxTreeObjectWrapper*)TreeControl->GetItemData(SelectionItemId);
			
	
				// Check to see if this item is a sequence...
				if(LeafData && LeafData->GetObject<UUISequence>() != NULL)
				{

					if(bFoundSequence == FALSE)
					{
						bFoundSequence = TRUE;

						// if we don't have any sequence objects selected and the selected item in the tree control is a sequence that
						// belongs to a UIState, display the properties for the UIState
						UUIStateSequence* SelectedSequence = LeafData->GetObject<UUIStateSequence>();
						if ( SelectedSequence != NULL )
						{
							PropertyWindow->SetObject(SelectedSequence->GetOwnerState(), TRUE, TRUE, FALSE);
						}
					}
					else
					{
						// Deselect this item because we already have a sequence item selected.
						TreeControl->SelectItem(SelectionItemId, false);
					}
				}
			}
		}
	}
}

/** 
 * Opens the context menu to let new users create new kismet objects. 
 *
 * This version opens up a UI Kismet specific menu.
 */
void WxUIKismetEditor::OpenNewObjectMenu()
{
	WxUIKismetNewObject menu( this );
	FTrackPopupMenu tpm( this, &menu );
	tpm.Show();
}


/**
 * Displays a context menu when the user right-clicks an object in the kismet editor.
 * This version adds additional options specific to the UI when certain types of objects are selected.
 */
void WxUIKismetEditor::OpenObjectOptionsMenu()
{
	if ( SelectedSeqObjs.Num() == 1 && SelectedSeqObjs(0)->IsA(UUIEvent_CalloutButtonInputProxy::StaticClass()) )
	{
		WxUIKismet_CalloutProxyOptions menu(this);
		FTrackPopupMenu tpm(this, &menu);
		tpm.Show();
	}
	else
	{
		WxMBKismetObjectOptions menu( this );
		FTrackPopupMenu tpm( this, &menu );
		tpm.Show();
	}
}

/**
 * Displays a context menu when the user right-clicks the connector node for a link on an object in the kismet editor.
 * This version adds additional options specific to the UI when certain types of objects are selected.
 */
void WxUIKismetEditor::OpenConnectorOptionsMenu()
{
	if ( ConnType == LOC_OUTPUT && ConnSeqOp != NULL && ConnSeqOp->IsA(UUIEvent_CalloutButtonInputProxy::StaticClass()) )
	{
		WxUIKismet_CalloutProxyConnectorOptions menu( this );
		FTrackPopupMenu tpm( this, &menu );
		tpm.Show();
	}
	else
	{
		WxMBKismetConnectorOptions menu( this );
		FTrackPopupMenu tpm( this, &menu );
		tpm.Show();
	}
}

/** 
 * Changes the active sequence for the kismet editor.
 *
 * This version changes the titlebar of the kismet editor to a path based on the widget that the sequenence belongs to.
 */
void WxUIKismetEditor::ChangeActiveSequence(USequence *NewSeq, UBOOL bNotifyTree)
{
	WxKismet::ChangeActiveSequence(NewSeq, bNotifyTree);

	// Set titlebar to the full widget path string.
	UUISequence* UISequence = Cast<UUISequence>(NewSeq);

	if(UISequence)
	{
		UUIScreenObject* Widget = UISequence->GetOwner();
		FString WidgetPath = Widget->GetPathName();
		

		// Add the sequence name to the end of the widget path.
		WidgetPath += TEXT(".");
		WidgetPath += GetFriendlySequenceName(UISequence);
		
		SetTitle( *FString::Printf( LocalizeSecure(LocalizeUnrealEd("KismetCaption_F"), *WidgetPath)) );
	}

}

/**
 * @return Returns a friendly/printable name for the sequence provided.
 */
FString WxUIKismetEditor::GetFriendlySequenceName(UUISequence *Sequence)
{
	const UUIScreenObject* Widget = Sequence->GetOwner();
	UBOOL bFoundFriendlyName = FALSE;
	FString ItemString;

	// See if this is a global sequence for the widget...
	if(Sequence == Widget->EventProvider->EventContainer)
	{
		ItemString = LocalizeUI(TEXT("UIEditor_GlobalSequence"));
		bFoundFriendlyName = TRUE;
	}
	else
	{
		UUIStateSequence* StateSequence = Cast<UUIStateSequence>(Sequence);

		if(StateSequence != NULL)
		{
			const UUIState* State = StateSequence->GetOwnerState();

			// Find a friendly name for the state sequenece.
			ItemString = State->GetClass()->GetDescription();
			bFoundFriendlyName = TRUE;
		}
	}

	if(bFoundFriendlyName == FALSE)
	{
		ItemString = Sequence->ObjName;
	}
	
	return ItemString;
}


/** 
 *	Create a new Object Variable and fill it in with the currently selected Widgets.
 *	If bAttachVarsToConnector is TRUE, it will also connect these new variables to the selected variable connector.
 *
 *  This version uses selected UI widgets instead of Actors.
 */
void WxUIKismetEditor::OnContextNewScriptingObjVarContext( wxCommandEvent& In )
{
	TArray<UUIObject*> SelectedWidgets;
	UIEditor->GetSelectedWidgets(SelectedWidgets);

	// Create the new sequence objects.
	const INT MultiEventSpacing = 100;
	const FScopedTransaction Transaction( *LocalizeUnrealEd("NewEvent") );
	EmptySelection();

	FScopedObjectStateChange SeqVarAddedNotification(Sequence);

	UBOOL bSuccess = FALSE;
	for(INT i=0; i<SelectedWidgets.Num(); i++)
	{
		UUIObject* Widget = SelectedWidgets(i);

		USeqVar_Object* NewObjVar = ConstructObject<USeqVar_Object>( USeqVar_Object::StaticClass(), Sequence, NAME_None, RF_Transactional|Sequence->GetMaskedFlags(RF_ArchetypeObject|RF_Public));
		if(bAttachVarsToConnector && ConnSeqOp && (ConnType == LOC_VARIABLE))
		{
			FIntPoint ConnectorPos = ConnSeqOp->GetConnectionLocation(LOC_VARIABLE, ConnIndex);			
			NewObjVar->ObjPosX = ConnectorPos.X - (LO_MIN_SHAPE_SIZE/2) + (MultiEventSpacing * i);
			NewObjVar->ObjPosY = ConnectorPos.Y + 10;
		}
		else
		{
			NewObjVar->ObjPosX = (LinkedObjVC->NewX - LinkedObjVC->Origin2D.X)/LinkedObjVC->Zoom2D + (MultiEventSpacing * i);
			NewObjVar->ObjPosY = (LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y)/LinkedObjVC->Zoom2D;
			NewObjVar->SnapPosition(KISMET_GRIDSIZE, MaxUISequenceSize);
		}

		NewObjVar->ObjValue = Widget;
		if ( Sequence->AddSequenceObject(NewObjVar) )
		{
			bSuccess = TRUE;
			NewObjVar->OnCreated();

			AddToSelection(NewObjVar);

			// If desired, and we have a connector selected, attach to it now
			if(bAttachVarsToConnector && ConnSeqOp && (ConnType == LOC_VARIABLE))
			{
				MakeVariableConnection(ConnSeqOp, ConnIndex, NewObjVar);
			}
		}
	}

	if ( bSuccess )
	{
		RefreshViewport();
		NotifyObjectsChanged();
	}
	else
	{
		SeqVarAddedNotification.CancelEdit();
	}
}

/** Called when the user selects the "Add Callout Button" context menu option */
void WxUIKismetEditor::OnContext_AddCalloutButton( wxCommandEvent& Event )
{
	if ( SelectedSeqObjs.Num() == 1 )
	{
		UUIEvent_CalloutButtonInputProxy* InputProxy = Cast<UUIEvent_CalloutButtonInputProxy>(SelectedSeqObjs(0));
		if ( InputProxy != NULL )
		{
			FScopedTransaction Transaction(*LocalizeUI(TEXT("TransInsertCalloutButton")));

			UBOOL bSuccessful = FALSE;

			UUICalloutButtonPanel* ButtonPanel = InputProxy->ButtonPanel;
			// this menu option should only be available if a UICalloutButton was selected, and it should always be contained within a
			// UICalloutButtonPanel, so assert on this so that this code can be updated if this assumption changes
			check(ButtonPanel);
			
			// determine the metaclass to use for the new button
			UClass* RequiredMetaClass = UUICalloutButton::StaticClass();
			if ( ButtonPanel->ButtonTemplate != NULL )
			{
				RequiredMetaClass = ButtonPanel->ButtonTemplate->GetClass();
			}

			WxDlgAddCalloutButton AddCalloutButtonDialog(ButtonPanel, this);
			if ( AddCalloutButtonDialog.ShowModal() == wxID_OK )
			{
				FName SelectedInputAlias = AddCalloutButtonDialog.GetSelectedAlias();
				FName DesiredWidgetTag = AddCalloutButtonDialog.GetButtonTag();
				FString SelectedButtonClassName = AddCalloutButtonDialog.GetSelectedClassName();

				UClass* ButtonClass = NULL;
				if ( SelectedButtonClassName.Len() > 0 )
				{
					ButtonClass = FindObject<UClass>(ANY_PACKAGE, *SelectedButtonClassName);
				}

				if ( ButtonClass != NULL && SelectedInputAlias != NAME_None && DesiredWidgetTag != NAME_None)
				{
					FScopedObjectStateChange AddButtonNotification(ButtonPanel);

					// create the callout button
					//@fixme ronp - currently doesn't use the selected ButtonClass - always uses the class of the button template from the panel class
					UUICalloutButton* NewButton = ButtonPanel->CreateCalloutButton(SelectedInputAlias, DesiredWidgetTag);
					if ( NewButton != NULL )
					{
						bSuccessful = TRUE;
					}
					else
					{
						AddButtonNotification.CancelEdit();
					}
				}
			}

			if ( bSuccessful )
			{
				UIEditor->UpdatePropertyWindow();
			}
			else
			{
				Transaction.Cancel();
			}
		}
	}
}

/** Called when the user selects the "Remove Callout Button" context menu option */
void WxUIKismetEditor::OnContext_RemoveCalloutButton( wxCommandEvent& Event )
{
	if ( ConnSeqOp != NULL && ConnType == LOC_OUTPUT && ConnSeqOp->OutputLinks.IsValidIndex(ConnIndex) )
	{
		UUIEvent_CalloutButtonInputProxy* InputProxy = Cast<UUIEvent_CalloutButtonInputProxy>(ConnSeqOp);
		if ( InputProxy != NULL )
		{
			FScopedTransaction Transaction(*LocalizeUI(TEXT("TransRemoveCalloutButton")));

			UUICalloutButtonPanel* ButtonPanel = InputProxy->ButtonPanel;
			// this menu option should only be available if a UICalloutButton was selected, and it should always be contained within a
			// UICalloutButtonPanel, so assert on this so that this code can be updated if this assumption changes
			check(ButtonPanel);

			FName AliasToRemove = *InputProxy->OutputLinks(ConnIndex).LinkDesc;
			UUICalloutButton* SelectedButton = ButtonPanel->eventFindButton(AliasToRemove);

			UBOOL bSuccessful = FALSE;
			if ( SelectedButton != NULL )
			{
				FScopedObjectStateChange ButtonPanelNotification(ButtonPanel), ButtonNotification(SelectedButton);
				bSuccessful = ButtonPanel->eventRemoveButton(SelectedButton);
				if ( bSuccessful )
				{
					// clear the RF_Public flag so that we don't create a redirect
					SelectedButton->ClearFlags(RF_Public);
					SelectedButton->Rename(NULL, UObject::GetTransientPackage(), REN_ForceNoResetLoaders);

					// UUIScreenObject::RemoveChild will clear the value of UIObject::Owner, but once we've renamed a UIPrefab widget into the transient
					// package (when deleting), there is no way to determine which UIPrefab the widget came from so LoadInstancesFromPropagationArchive()
					// can't be called for that archetype widget.  To workaround this, here we restore the widget's Owner reference so that
					// UUIScreenObject::IsInUIPrefab() can use the Owner if the widget is no longer part of the parent-child hierarchy 
					SelectedButton->Owner = ButtonPanel;
				}

				ButtonPanelNotification.FinishEdit(!bSuccessful);
				ButtonNotification.FinishEdit(!bSuccessful);
			}

			if ( bSuccessful )
			{
				UIEditor->RefreshLayerBrowser();
				if ( UIEditor->Selection->IsSelected(SelectedButton) )
				{
					UIEditor->Selection->Modify();

					UIEditor->Selection->BeginBatchSelectOperation();
					UIEditor->Selection->DeselectAll();
					UIEditor->Selection->EndBatchSelectOperation();
				}

				UpdatePropertyWindow();
			}
			else
			{
				Transaction.Cancel();
			}
		}
	}

}

/** Called when the user selects the "Change Callout Alias" context menu option */
void WxUIKismetEditor::OnContext_ChangeCalloutButtonAlias( wxCommandEvent& Event )
{
	if ( ConnSeqOp != NULL && ConnType == LOC_OUTPUT && ConnSeqOp->OutputLinks.IsValidIndex(ConnIndex) )
	{
		UUIEvent_CalloutButtonInputProxy* InputProxy = Cast<UUIEvent_CalloutButtonInputProxy>(ConnSeqOp);
		if ( InputProxy != NULL )
		{
			FScopedTransaction Transaction(*LocalizeUI(TEXT("TransChangeCalloutAlias")));

			UUICalloutButtonPanel* ButtonPanel = InputProxy->ButtonPanel;
			// this menu option should only be available if a UICalloutButton was selected, and it should always be contained within a
			// UICalloutButtonPanel, so assert on this so that this code can be updated if this assumption changes
			check(ButtonPanel);

			FName AliasToChange = *InputProxy->OutputLinks(ConnIndex).LinkDesc;
			UUICalloutButton* CalloutButton = ButtonPanel->eventFindButton(AliasToChange);

			UBOOL bSuccessful = FALSE;
			if ( CalloutButton != NULL )
			{
// 				WxDlgAddCalloutButton AddCalloutButtonDialog(CalloutButton, this);
// 				if ( AddCalloutButtonDialog.ShowModal() == wxID_OK )
// 				{
// 					FName SelectedInputAlias = AddCalloutButtonDialog.GetSelectedAlias();
					
					TArray<FName> AvailableAliases;
					ButtonPanel->GetAvailableCalloutButtonAliases(AvailableAliases);
					INT SelectedAliasIndex = Event.GetId() - IDM_UIKISMET_CALLOUT_CHANGE_START;

					if ( AvailableAliases.IsValidIndex(SelectedAliasIndex) )
					{
						FName SelectedInputAlias = AvailableAliases(SelectedAliasIndex);
						if ( SelectedInputAlias != NAME_None )
						{
							FScopedObjectStateChange SetAliasNotification(CalloutButton);
							if ( ButtonPanel->eventSetButtonInputAlias(CalloutButton->InputAliasTag, SelectedInputAlias) )
							{
								bSuccessful = TRUE;
							}
							else
							{
								SetAliasNotification.CancelEdit();
							}
						}
					}
// 				}
			}

			if ( bSuccessful )
			{
				UIEditor->UpdatePropertyWindow();
			}
			else
			{
				Transaction.Cancel();
			}
		}
	}
}

/**
 * Recursively loops through all of the widgets in the scene and creates input event meta objects for their state sequences.
 *
 * @param InSequenceOwner	The widget parent of the sequences we will be generating new meta objects for.
 */
void WxUIKismetEditor::CreateStateSequenceMetaObject(class UUIScreenObject* InSequenceOwner)
{
	// Loop through all of the states for this widget and generate new meta objects.
	for (INT StateIndex = 0; StateIndex < InSequenceOwner->InactiveStates.Num(); StateIndex++)
	{
		UUIState *State = InSequenceOwner->InactiveStates(StateIndex);

		UUIStateSequence *Sequence = State->StateSequence;

		// Tell the state sequence to create a input event meta object if it doesn't already have one within its list of sequence objects.
		Sequence->InitializeInputMetaObject();
	}

	// Loop through all of this widget's children and repeat the process.
	TArray<UUIObject*> Children = InSequenceOwner->GetChildren();

	for ( INT ChildIndex = 0; ChildIndex < Children.Num(); ChildIndex++ )
	{
		CreateStateSequenceMetaObject( Children(ChildIndex) );
	}
}

/**
 * Returns whether the specified class can be displayed in the list of ops which are available to be placed.
 *
 * @param	SequenceClass	a child class of USequenceObject
 *
 * @return	TRUE if the specified class should be available in the context menus for adding sequence ops
 */
UBOOL WxUIKismetEditor::IsValidSequenceClass( UClass* SequenceClass ) const
{
	UBOOL bResult = FALSE;
	if ( SequenceClass != NULL && !SequenceClass->HasAnyClassFlags(CLASS_Abstract|CLASS_Deprecated) )
	{
		USequenceObject* SequenceCDO = SequenceClass->GetDefaultObject<USequenceObject>();
		check(SequenceCDO != NULL);

		bResult = SequenceCDO->eventIsValidUISequenceObject();
	}

	return bResult;
}




// EOF






