/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "UnrealEd.h"

// -----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( WxFloatingFrame, wxFrame )
	EVT_ACTIVATE(WxFloatingFrame::OnActivate)
	EVT_HOTKEY(ID_POPUP_CTRLTAB, WxFloatingFrame::OnCtrlTab)
	EVT_HOTKEY(ID_POPUP_CTRLSHIFTTAB, WxFloatingFrame::OnCtrlTab)
	EVT_SIZE( WxFloatingFrame::OnSize )
	EVT_MENU_RANGE( wxID_HIGHEST, wxID_HIGHEST+65535, WxFloatingFrame::OnCommand )
	EVT_CLOSE( WxFloatingFrame::OnClose )
	EVT_UPDATE_UI( IDM_DockWindow, WxFloatingFrame::UI_DockWindow )
	EVT_UPDATE_UI( IDM_FloatWindow, WxFloatingFrame::UI_FloatWindow )
END_EVENT_TABLE()

WxFloatingFrame::WxFloatingFrame( wxWindow* InParent, wxWindowID InID, const FString& InDockName )
	: wxFrame( InParent, InID, TEXT("FloatingFrame"), wxDefaultPosition, wxDefaultSize, wxMAXIMIZE_BOX | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR | wxCLIP_SIBLINGS )
{
	ChildWindow = NULL;
	DockName = InDockName;

	FWindowUtil::LoadPosSize( *FString::Printf( TEXT("FloatingFrame_%s"), *DockName ), this, -1, -1, 800, 600 );
}

WxFloatingFrame::~WxFloatingFrame()
{
	FWindowUtil::SavePosSize( *FString::Printf( TEXT("FloatingFrame_%s"), *DockName ), this );
}

void WxFloatingFrame::OnActivate(wxActivateEvent& Event)
{
	// If the frame is being activated register ctrl + tab hot keys
	if(Event.GetActive())
	{
		WxBrowser* Browser = GetChildWindow();

		// Set focus to update the window entry in the list of trackable windows
		if(Browser)
		{
			Browser->SetFocus();
		}

		RegisterHotKey(ID_POPUP_CTRLTAB, wxMOD_CONTROL, WXK_TAB);
		RegisterHotKey(ID_POPUP_CTRLSHIFTTAB, wxMOD_CONTROL | wxMOD_SHIFT, WXK_TAB);
	}
	else
	{
		UnregisterHotKey(ID_POPUP_CTRLTAB);
		UnregisterHotKey(ID_POPUP_CTRLSHIFTTAB);
	}
}

void WxFloatingFrame::OnCtrlTab(wxKeyEvent& Event)
{
	WxTrackableWindowBase::HandleCtrlTab(GApp->EditorFrame, Event.ShiftDown());
}

void WxFloatingFrame::OnSize( wxSizeEvent& In )
{
	wxRect rc = GetClientRect();

	if( ChildWindow )
	{
		ChildWindow->SetSize( rc );
	}
}

void WxFloatingFrame::OnCommand( wxCommandEvent& In )
{
	if( ChildWindow )
	{
		::wxPostEvent( ChildWindow, In );
	}
}

void WxFloatingFrame::OnClose( wxCloseEvent& In )
{
	// Don't actually close these windows.  Just hide them so they can be recalled later.
	Hide();

	In.Veto();
}

void WxFloatingFrame::UI_DockWindow( wxUpdateUIEvent& In )
{
	In.Check( 0 );
}

void WxFloatingFrame::UI_FloatWindow( wxUpdateUIEvent& In )
{
	In.Check( 1 );
}

void WxFloatingFrame::SetChildWindow(WxBrowser* InChildWindow)
{
	ChildWindow = InChildWindow;
	if (InChildWindow)
	{
		// Have wxWindows properly change parentage
		InChildWindow->Reparent(this);
	}
}

// -----------------------------------------------------------------------------

// Used for dynamic creation of the window. This must be declared for any
// subclasses of WxBrowser
IMPLEMENT_DYNAMIC_CLASS(WxBrowser,WxTrackableWindow);

BEGIN_EVENT_TABLE( WxBrowser, WxTrackableWindow )
	EVT_SIZE( WxBrowser::OnSize )
	EVT_MENU( IDM_FloatWindow, WxBrowser::OnFloatWindow )
	EVT_MENU( IDM_DockWindow, WxBrowser::OnDockWindow )
	EVT_MENU( IDM_CloneBrowser, WxBrowser::OnClone )
	EVT_MENU( IDM_RemoveBrowser, WxBrowser::OnRemove )
	EVT_MENU( IDM_RefreshBrowser, WxBrowser::OnRefresh )
END_EVENT_TABLE()

WxBrowser::WxBrowser(void) :
	DockID( -1 ),
	bAreWindowsInitialized(FALSE),
	Panel( NULL ),
	MenuBar( NULL )
{
	// Register the AllBrowsers event
	GCallbackEvent->Register(CALLBACK_RefreshEditor_AllBrowsers,this);
}

/**
 * This function is called when the window has been selected from within the ctrl + tab dialog.
 */
void WxBrowser::OnSelected()
{
	wxWindow* Parent = GetParent();
	if(IsDocked())
	{
		wxNotebook* Notebook = (wxNotebook*)Parent;

		if(!Notebook->GetParent()->IsShown())
		{
			Notebook->GetParent()->Show();
		}
		
		for(INT PageIndex = 0; PageIndex < (INT)Notebook->GetPageCount(); ++PageIndex)
		{
			if(Notebook->GetPage(PageIndex) == this)
			{
				// If the page is visible but does not have focus then give it the focus
				if(Notebook->GetSelection() == PageIndex)
				{
					this->SetFocus();
				}
				else
				{
					Notebook->SetSelection(PageIndex);
				}
			}
		}
	}
	else
	{
		if(!Parent->IsShown())
		{
			Parent->Show();
		}

		Parent->Raise();
	}
}

/**
 * Part 2 of the 2 phase creation process. Sets the pane id, friendly name,
 * and the parent window pointers. Forwards the call to wxWindow
 *
 * @param InDockID the unique id to associate with this dockable window
 * @param FriendlyName the friendly name to assign to this window
 * @param Parent the parent of this window (should be a Notebook)
 */
void WxBrowser::Create(INT InDockID,const TCHAR* FriendlyName,
	wxWindow* Parent)
{
	// Assert if the base level fails
	verify(wxWindow::Create(Parent,-1, wxDefaultPosition, wxDefaultSize, 0, FriendlyName));
	// Copy our specific data
	DockID = InDockID;
	DockingName = FriendlyName;
	// Say we've inited
	bAreWindowsInitialized = TRUE;
	SetLabel(FriendlyName);
}

void WxBrowser::SetParentCaption()
{
	wxWindow* parent = GetParent();
	if( IsDocked() )
	{
		parent = parent->GetParent();
	}

	parent->SetLabel( GetLocalizedCaption() );
}

/**
 * Loads the window state for this dockable window
 */
void WxBrowser::LoadWindowState(void)
{
	// Assume it was docked (for the empty INI case)
	UBOOL bIsDocked = TRUE;
	FString LocKey = GetLocalizationKey();
	// Load the state from the INI
	GConfig->GetBool(TEXT("Docking"),*(LocKey + TEXT("_Docked")),bIsDocked,
		GEditorUserSettingsIni);
	// Now save our state so the container can query it and create the correct
	// wrapper window class
	DockState = bIsDocked == TRUE ? BDS_Docked : BDS_Floating;
	// Assume it was visible
	bIsVisible = TRUE;
	// Load the visible state directly in
	GConfig->GetBool(TEXT("Docking"),*(LocKey + TEXT("_Visible")),
		bIsVisible,GEditorUserSettingsIni);
	// Load the name of the "package" that the localization is in
	// (MyBrowser.int for instance)
	FString Package;
	if (GConfig->GetString(TEXT("Docking"),*(LocKey + TEXT("_Package")),
		Package,GEditorIni) == FALSE)
	{
		Package = TEXT("UnrealEd");
	}
	// Get the localized name for this window
	LocalizedCaption = Localize(TEXT("Caption"),*LocKey,*Package,NULL,TRUE);
	// In case there isn't any localization for this window, use the friendly name
	if (LocalizedCaption.Len() == 0)
	{
		LocalizedCaption = DockingName;
	}
}

/**
 * Saves the window state for this dockable window
 */
void WxBrowser::SaveWindowState(void)
{
	FString LocKey = GetLocalizationKey();
	// Write out the current docking state
	GConfig->SetBool(TEXT("Docking"),*(LocKey + TEXT("_Docked")),
		IsDocked(),GEditorUserSettingsIni);
	// And now whether it's visible or not
	GConfig->SetBool(TEXT("Docking"),*(LocKey + TEXT("_Visible")),
		bIsVisible,GEditorUserSettingsIni);
}

/**
 * Detaches the menus from there frame window
 */
void WxBrowser::DetachMenus(void)
{
	// Detach the menu bar if it is valid and currently attached
	if (MenuBar && MenuBar->IsAttached())
	{
		wxFrame* Frame = GetFrame();
		// Give it the default accelerator table
		Frame->SetAcceleratorTable(wxNullAcceleratorTable);
		// Zero these so they aren't cleaned up
		Frame->SetMenuBar(NULL);
		Frame->SetToolBar(NULL);
	}
}

// Called when the browser is first constructed.  Gives it a chance to fill controls, lists and such.
void WxBrowser::InitialUpdate()
{
}

void WxBrowser::Update()
{
}

void WxBrowser::BeginUpdate()
{
	//@DB: Please do not delete the commented-out debugging code! Thanks :)
	//UpdateStartTime = appSeconds();

	Freeze();
}

void WxBrowser::EndUpdate()
{
	Thaw();

	//@DB: Please do not delete the commented-out debugging code! T hanks :)
	//debugf( TEXT("%s Update() - %i ms"), *LocalizedCaption, appTrunc((appSeconds() - UpdateStartTime) * 1000) );
}

// Called when the browser is getting activated (becoming the visible window in it's dockable frame)
void WxBrowser::Activated()
{
	wxFrame* Frame = GetFrame();
	// Now set them to the new owner
	Frame->SetMenuBar( MenuBar );
	// Have the browser attach its key bindings
	SetBrowserAcceleratorTable(Frame);
	WxTrackableWindowBase::MakeFirstEntry(FTrackableEntry(this, this));
}

/**
 * Assembles an accelerator key table for the browser by calling down to AddAcceleratorTableEntries.
 */
void WxBrowser::SetBrowserAcceleratorTable(wxFrame* Frame)
{
	TArray<wxAcceleratorEntry> Entries;

	// Allow derived classes an opportunity to register accelerator keys.
	AddAcceleratorTableEntries( Entries );

	// Create the new table with these.
	Frame->SetAcceleratorTable( wxAcceleratorTable(Entries.Num(),Entries.GetTypedData()) );
}

/**
 * Adds entries to the browser's accelerator key table.  Derived classes should call up to their parents.
 */
void WxBrowser::AddAcceleratorTableEntries(TArray<wxAcceleratorEntry>& Entries)
{
	// Bind F5 to refresh.
	Entries.AddItem( wxAcceleratorEntry(wxACCEL_NORMAL,WXK_F5,IDM_RefreshBrowser) );
}

void WxBrowser::PostCreateInit()
{
}

void WxBrowser::OnSize( wxSizeEvent& In )
{
	if( Panel )
	{
		Panel->SetSize( GetClientRect() );
	}
}

void WxBrowser::OnFloatWindow( wxCommandEvent& In )
{
	WxDockEvent evt( DockID, DCT_Floating );
	evt.SetEventType( wxEVT_DOCKINGCHANGE );

	::wxPostEvent( GApp->EditorFrame, evt );
}

void WxBrowser::OnDockWindow( wxCommandEvent& In )
{
	WxDockEvent evt( DockID, DCT_Docking );
	evt.SetEventType( wxEVT_DOCKINGCHANGE );

	::wxPostEvent( GApp->EditorFrame, evt );
}

/**
 * Forwards this event onto the frame window. This must be done because
 * the event may need to modify this window in a way that would cause
 * aberrant behavior if done synchronously
 *
 * @param Event the event that is going to be forwarded
 */
void WxBrowser::OnClone(wxCommandEvent& Event)
{
	// Tell the frame to make a clone of this browser
	WxDockEvent NewEvent(DockID,DCT_Clone);
	NewEvent.SetEventType( wxEVT_DOCKINGCHANGE );
	// Async post this
	::wxPostEvent(GApp->EditorFrame,NewEvent);
}

/**
 * Forwards this event onto the frame window. This must be done because
 * the event may need to modify this window in a way that would cause
 * aberrant behavior if done synchronously
 *
 * @param Event the event that is going to be forwarded
 */
void WxBrowser::OnRemove(wxCommandEvent& Event)
{
	// Tell the frame to remove this browser
	WxDockEvent NewEvent(DockID,DCT_Remove);
	NewEvent.SetEventType( wxEVT_DOCKINGCHANGE );
	// Async post this
	::wxPostEvent(GApp->EditorFrame,NewEvent);
}

/**
 * Default handler for IDM_RefreshBrowser events.  Derived classes
 * override this function to handle browser refreshes.
 */
void WxBrowser::OnRefresh(wxCommandEvent& Event)
{
}

/**
 * Builds a docking menu and appends it to the menu passed in
 *
 * @param InMenu the menu to add the hand built one to
 */
void WxBrowser::AddDockingMenu(wxMenuBar* InMenu)
{
	wxMenu* DockMenu = new wxMenu();

	DockMenu->AppendCheckItem(IDM_DockWindow,*LocalizeUnrealEd("Docked"),TEXT(""));
	DockMenu->AppendCheckItem(IDM_FloatWindow,*LocalizeUnrealEd("Floating"),TEXT(""));

	DockMenu->AppendSeparator();

	DockMenu->Append(IDM_CloneBrowser,*LocalizeUnrealEd("CloneBrowser"),TEXT(""));
	DockMenu->Append(IDM_RemoveBrowser,*LocalizeUnrealEd("RemoveBrowser"),TEXT(""));

	InMenu->Append( DockMenu, *LocalizeUnrealEd("Docking") );
}

// -----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( WxDockingContainer, WxFloatingFrame )
	EVT_SIZE( WxDockingContainer::OnSize )
	EVT_NOTEBOOK_PAGE_CHANGED( ID_NOTEBOOK_DOCK, WxDockingContainer::OnPageChanged )
	EVT_MENU_RANGE( wxID_HIGHEST, wxID_HIGHEST+65535, WxDockingContainer::OnCommand )
	EVT_CLOSE( WxDockingContainer::OnClose )
	EVT_UPDATE_UI( IDM_DockWindow, WxDockingContainer::UI_DockWindow )
	EVT_UPDATE_UI( IDM_FloatWindow, WxDockingContainer::UI_FloatWindow )
	EVT_UPDATE_UI(IDM_RemoveBrowser,WxDockingContainer::UI_RemoveWindow)
	EVT_UPDATE_UI(IDM_CloneBrowser,WxDockingContainer::UI_CloneWindow)
END_EVENT_TABLE()

WxDockingContainer::WxDockingContainer( wxWindow* InParent, wxWindowID InID )
	: WxFloatingFrame( InParent, InID, TEXT("DockingContainer") )
{
	Notebook = new wxNotebook( this, ID_NOTEBOOK_DOCK );

	FWindowUtil::LoadPosSize( TEXT("DockingContainer"), this, -1, -1, 800, 600 );
}

WxDockingContainer::~WxDockingContainer()
{
	Notebook->Destroy();

	FWindowUtil::SavePosSize( TEXT("DockingContainer"), this );
}

void WxDockingContainer::OnSize( wxSizeEvent& In )
{
	Notebook->SetSize( GetClientRect() );
}

void WxDockingContainer::OnPageChanged(	wxNotebookEvent& In )
{
	WxBrowser* dw = (WxBrowser*)Notebook->GetPage( In.GetSelection() );
	dw->Activated();
	dw->SetParentCaption();
}

void WxDockingContainer::OnCommand( wxCommandEvent& In )
{
	wxWindow* win = Notebook->GetPage( Notebook->GetSelection() );
	::wxPostEvent( win, In );
}

void WxDockingContainer::OnClose( wxCloseEvent& In )
{
	// Don't actually close these windows.  Just hide them so they can be recalled later.
	Hide();

	In.Veto();
}

void WxDockingContainer::UI_DockWindow( wxUpdateUIEvent& In )
{
	In.Check( 1 );
}

void WxDockingContainer::UI_FloatWindow( wxUpdateUIEvent& In )
{
	In.Check( 0 );
}

/**
 * Determines whether this menu item can be selected or not. Docked clones
 * are removed. Docked originals aren't affected. Floating originals are
 * hidden. Floating clones are removable.
 *
 * @param Event the update event to modify
 */
void WxDockingContainer::UI_RemoveWindow(wxUpdateUIEvent& Event)
{
	// Figure out which page is selected
	INT Index = Notebook->GetSelection();
	// Get the pointer to that browser so we can get the dock id
	WxBrowser* Page = (WxBrowser*)Notebook->GetPage(Index);
	// Enable it based upon state
	Event.Enable(Page->IsFloating() ||
		(Page->IsDocked() && 
		GUnrealEd->GetBrowserManager()->IsCanonicalBrowser(Page->GetDockID()) == FALSE));
}

/**
 * Determines whether this menu item can be selected or not. It only allows
 * the number of panes that matches the number of available menu items
 *
 * @param Event the update event to modify
 */
void WxDockingContainer::UI_CloneWindow(wxUpdateUIEvent& Event)
{
	// Figure out which page is selected
	INT Index = Notebook->GetSelection();
	// Get the pointer to that browser so we can get the dock id
	WxBrowser* Page = (WxBrowser*)Notebook->GetPage(Index);
	// Enable it based upon whether it can be cloned and the number of browsers
	// in the manager is acceptable
	Event.Enable(Page->IsClonable() &&
		GUnrealEd->GetBrowserManager()->GetBrowserCount() <
			(IDM_BROWSER_END - IDM_BROWSER_START));
}

