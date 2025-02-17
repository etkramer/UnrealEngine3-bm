/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "Controls.h"
#include "DlgSelectGroup.h"

/*-----------------------------------------------------------------------------
	WxStatusBar.
-----------------------------------------------------------------------------*/

WxStatusBar::WxStatusBar()
	: wxStatusBar()
{
}

WxStatusBar::WxStatusBar( wxWindow* parent, wxWindowID id )
	: wxStatusBar( parent, id )
{
}

/*-----------------------------------------------------------------------------
	WxTextCtrl.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxTextCtrl, wxTextCtrl )
	EVT_CHAR( WxTextCtrl::OnChar )
END_EVENT_TABLE()

WxTextCtrl::WxTextCtrl()
	: wxTextCtrl()
{
}

WxTextCtrl::WxTextCtrl( wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size, long style ) :
	wxTextCtrl( parent, id, value, pos, size, style )
{
}

void WxTextCtrl::OnChar( wxKeyEvent& In )
{
	switch( In.GetKeyCode() )
	{
		case WXK_DOWN:
		case WXK_UP:
		case WXK_TAB:
			GetParent()->AddPendingEvent( In );
			break;

		default:
			In.Skip();
			break;
	}
}

wxDragResult WxTextCtrl::OnEnter(wxCoord x, wxCoord y, wxDragResult def)
{
	//m_frame->SetStatusText(_T("Mouse entered the frame"));

	return OnDragOver(x, y, def);
}

void WxTextCtrl::OnLeave()
{
	//m_frame->SetStatusText(_T("Mouse left the frame"));
}

wxDragResult WxTextCtrl::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
	//if( !GetData() )
	//{
	//	wxLogError(wxT("Failed to get drag and drop data"));

	//	return wxDragNone;
	//}

	//m_frame->OnDrop(x, y, ((DnDShapeDataObject *)GetDataObject())->GetShape());

	return def;
}

/**
 * Constructor
 *
 * @param	StorageLocation		a pointer to a member of the owner window where the validated text will be placed
 * @param	NameValidationMask	bitmask for the type of validation to apply
 */
WxNameTextValidator::WxNameTextValidator( wxString* StorageLocation, DWORD NameValidationMask/*=VALIDATE_PackageName*/ )
: wxTextValidator(wxFILTER_EXCLUDE_CHAR_LIST, StorageLocation)
{
	FString InvalidCharacters;
	if ( (NameValidationMask&VALIDATE_Name) != 0 )
	{
		InvalidCharacters += INVALID_NAME_CHARACTERS;
	}

	if ( (NameValidationMask&VALIDATE_ObjectName) != 0 )
	{
		InvalidCharacters += INVALID_OBJECTNAME_CHARACTERS;
	}

	if ( (NameValidationMask&VALIDATE_PackageName) != 0 )
	{
		InvalidCharacters += INVALID_PACKAGENAME_CHARACTERS;
	}

	else if ( (NameValidationMask&VALIDATE_GroupName) != 0 )
	{
		InvalidCharacters += INVALID_GROUPNAME_CHARACTERS;
	}
	
	// Create an array of single characters from the InvalidCharacters string
	wxArrayString InvalidCharactersArray;
	wxString Char;
	for ( int a = 0; a < InvalidCharacters.Len(); ++a )
	{
		Char = InvalidCharacters[a];
		if(InvalidCharactersArray.Index(Char) == wxNOT_FOUND)
		{
			InvalidCharactersArray.Add( Char );
		}
	}
	
	SetExcludes( InvalidCharactersArray );
}


/*-----------------------------------------------------------------------------
	WxPkgGrpNameCtrl.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxPkgGrpNameCtrl, wxPanel )
	EVT_BUTTON( IDPB_BROWSE, WxPkgGrpNameCtrl::OnBrowse )
END_EVENT_TABLE()

WxPkgGrpNameCtrl::WxPkgGrpNameCtrl( wxWindow* parent, wxWindowID id, wxSizer* InParentSizer, UBOOL bDeferLocalization )
	: wxPanel( parent, id )
{
	FlexGridSizer = new wxFlexGridSizer( 3, 3, 0, 0 );
	FlexGridSizer->AddGrowableCol( 1 );
	{
		PkgLabel = new wxStaticText( this,	 wxID_STATIC, *LocalizeUnrealEd("Package") );
		FlexGridSizer->Add(PkgLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

		PkgCombo = new WxComboBox( this, IDCB_PACKAGE, TEXT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxCB_SORT );
		TArray<UPackage*> Packages;
		GEditor->GetPackageList( &Packages, NULL );
		for( INT x = 0 ; x < Packages.Num() ; ++x )
		{
			PkgCombo->Append( *Packages(x)->GetName(), Packages(x) );
		}
		FlexGridSizer->Add( PkgCombo, 0, wxALIGN_LEFT|wxGROW|wxALL, 5 );

		FlexGridSizer->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

		GrpLabel = new wxStaticText( this, wxID_STATIC, *LocalizeUnrealEd("Group") );
		FlexGridSizer->Add( GrpLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE|wxEXPAND, 5 );

		GrpEdit = new wxTextCtrl( this, IDEC_GROUP, TEXT(""), wxDefaultPosition, wxSize(200, -1) );
		FlexGridSizer->Add( GrpEdit, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

		BrowseB.Load( TEXT("Search") );
		BrowseButton = new WxBitmapButton( this, IDPB_BROWSE, BrowseB, wxDefaultPosition, wxSize(24, -1) );
		FlexGridSizer->Add( BrowseButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

		NameLabel = new wxStaticText( this, wxID_STATIC, *LocalizeUnrealEd("Name"));
		FlexGridSizer->Add( NameLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE|wxEXPAND, 5 );

		NameEdit = new wxTextCtrl( this, IDEC_NAME, TEXT("") );
		FlexGridSizer->Add( NameEdit, 0, wxALIGN_LEFT|wxGROW|wxALL, 5 );
	}
	if( InParentSizer )
	{
		InParentSizer->Add( FlexGridSizer, 1, wxEXPAND|wxALL, 5 );
	}
	else
	{
		SetSizer( FlexGridSizer );
	}

	// Defer localization -- happens if eg derived classes that want to add additional localized text.
	if ( !bDeferLocalization )
	{
		FLocalizeWindow( this );
	}
}

void WxPkgGrpNameCtrl::OnBrowse( wxCommandEvent& In )
{
	const FString PkgName = (const TCHAR*)PkgCombo->GetValue();
	const FString GroupName = (const TCHAR*)GrpEdit->GetValue();

	WxDlgSelectGroup dlg( this );
	if( dlg.ShowModal( PkgName, GroupName ) == wxID_OK )
	{
		PkgCombo->SetValue( *dlg.Package );
		GrpEdit->SetValue( *dlg.Group );
	}
}

/*-----------------------------------------------------------------------------
	WxTreeCtrl.
-----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC_CLASS(WxTreeCtrl,wxTreeCtrl)

BEGIN_EVENT_TABLE( WxTreeCtrl, wxTreeCtrl )
	EVT_RIGHT_DOWN( WxTreeCtrl::OnRightButtonDown )
END_EVENT_TABLE()

WxTreeCtrl::WxTreeCtrl( wxWindow* InParent, wxWindowID InID, wxMenu* InMenu, LONG InStyle )
:	wxTreeCtrl( InParent, InID, wxDefaultPosition, wxDefaultSize, InStyle ),
ContextMenu( InMenu ),
bInSetFocus(FALSE)
{
}

WxTreeCtrl::~WxTreeCtrl()
{
	delete ContextMenu;
}

/**
 * Initializes this window when using two-stage dynamic window creation
 */
UBOOL WxTreeCtrl::Create( wxWindow* InParent, wxWindowID InID, wxMenu* InMenu, LONG style/*=wxTR_HAS_BUTTONS|wxTR_MULTIPLE|wxTR_LINES_AT_ROOT*/ )
{
	ContextMenu = InMenu;
	return wxTreeCtrl::Create(InParent, InID,  wxDefaultPosition, wxDefaultSize, style);
}

/**
 * Since we derive from wxTreeCtrl, the base code can't use the fast
 * mechanism for sorting. Therefore, we provide this method to bypass
 * the slower method.
 *
 * @param Item the tree item to sort
 */
void WxTreeCtrl::SortChildren(const wxTreeItemId& item)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );
	// Just use the default sorting code
	TreeView_SortChildren(GetHwnd(), item.m_pItem, 0);
}

/**
 * Sets all of the item's images to the same image.
 *
 *
 * @param Item	Item to modify.
 * @param Image	Index of the image in the tree control's image list.
 */
void WxTreeCtrl::SetAllItemImages(wxTreeItemId& Item, INT Image)
{
	SetItemImage(Item, Image, wxTreeItemIcon_Normal);
	SetItemImage(Item, Image, wxTreeItemIcon_Selected);
	SetItemImage(Item, Image, wxTreeItemIcon_Expanded);
	SetItemImage(Item, Image, wxTreeItemIcon_SelectedExpanded);
}

/**
 * Intercepts the set focus call in order to trap the item selection
 */
void WxTreeCtrl::SetFocus(void)
{
	bInSetFocus = TRUE;
	wxTreeCtrl::SetFocus();
	bInSetFocus = FALSE;
}

/**
 * Callback for wxWindows RMB events. Default behaviour is to show the popup menu if a WxTreeObjectWrapper
 * is attached to the tree item that was clicked.
 */
void WxTreeCtrl::OnRightButtonDown( wxMouseEvent& In )
{
	const wxTreeItemId TreeId = HitTest( wxPoint( In.GetX(), In.GetY() ) );

	UBOOL bWasHandled = FALSE;

	if( TreeId.IsOk() == true )
	{
		WxTreeObjectWrapper* ItemData = (WxTreeObjectWrapper*)GetItemData( TreeId );
		if( ItemData )
		{
			ShowPopupMenu(ItemData);			
			if( ContextMenu != NULL )
			{
				bWasHandled = TRUE;
			}
		}
	}

	if( !bWasHandled )
	{
		// OK, we didn't summon a context menu, so make sure the event gets passed on to other handlers
		// such as the EVT_TREE_ITEM_RIGHT_CLICK handler!
		In.Skip();
	}
}

/**
 * Activates the context menu associated with this tree control.
 *
 * @param	ItemData	the data associated with the item that the user right-clicked on
 */
void WxTreeCtrl::ShowPopupMenu( WxTreeObjectWrapper* ItemData )
{
	if ( ContextMenu != NULL )
	{
		FTrackPopupMenu tpm( GetParent(), ContextMenu );
		tpm.Show();
	}
}


/** 
 * Loops through all of the elements of the tree and adds selected items to the selection set,
 * and expanded items to the expanded set.
 */
void WxTreeCtrl::SaveSelectionExpansionState()
{
	SavedSelections.Empty();
	SavedExpansions.Empty();

	// Recursively traverse and tree and store selection states based on the client data.
	wxTreeItemId Root = GetRootItem();

	if(Root.IsOk())
	{
		SaveSelectionExpansionStateRecurse(Root);
	}
}

/** 
 * Loops through all of the elements of the tree and sees if the client data of the item is in the 
 * selection or expansion set, and modifies the item accordingly.
 */
void WxTreeCtrl::RestoreSelectionExpansionState()
{
	// Recursively traverse and tree and restore selection states based on the client data.
	Freeze();
	{
		wxTreeItemId Root = GetRootItem();

		if(Root.IsOk())
		{
			RestoreSelectionExpansionStateRecurse(Root);
		}
	}
	Thaw();

	SavedSelections.Empty();
	SavedExpansions.Empty();
}


/** 
 * Recursion function that loops through all of the elements of the tree item provided and saves their select/expand state. 
 * 
 * @param Item Item to use for the root of this recursion.
 */
void WxTreeCtrl::SaveSelectionExpansionStateRecurse(wxTreeItemId& Item)
{
	// Expand and select this item
	WxTreeObjectWrapper* ObjectWrapper = static_cast<WxTreeObjectWrapper*>(GetItemData(Item));

	const UBOOL bIsRoot = (Item == GetRootItem());
	const UBOOL bVirtualRoot = ((GetWindowStyle() & wxTR_HIDE_ROOT) == wxTR_HIDE_ROOT);
	const UBOOL bProcessItem = (bIsRoot == FALSE) || (bVirtualRoot == FALSE);

	if( bProcessItem )
	{
		if(ObjectWrapper != NULL)
		{
			UObject* ObjectPointer = ObjectWrapper->GetObject<UObject>();

			if(IsSelected(Item))
			{
				SavedSelections.Set(ObjectPointer, ObjectPointer);
			}

			if(IsExpanded(Item))
			{
				SavedExpansions.Set(ObjectPointer, ObjectPointer);
			}
		}
	}


	// Loop through all of the item's children and store their state.
	wxTreeItemIdValue Cookie;
	wxTreeItemId ChildItem =GetFirstChild( Item, Cookie );

	while(ChildItem.IsOk())
	{
		SaveSelectionExpansionStateRecurse(ChildItem);

		ChildItem = GetNextChild(Item, Cookie);
	}
}


/** 
 * Recursion function that loops through all of the elements of the tree item provided and restores their select/expand state. 
 * 
 * @param Item Item to use for the root of this recursion.
 */
void WxTreeCtrl::RestoreSelectionExpansionStateRecurse(wxTreeItemId& Item)
{
	// Expand and select this item
	WxTreeObjectWrapper* ObjectWrapper = static_cast<WxTreeObjectWrapper*>(GetItemData(Item));

	const UBOOL bIsRoot = (Item == GetRootItem());
	const UBOOL bVirtualRoot = ((GetWindowStyle() & wxTR_HIDE_ROOT) == wxTR_HIDE_ROOT);
	const UBOOL bProcessItem = (bIsRoot == FALSE) || (bVirtualRoot == FALSE);

	if( bProcessItem )
	{
		if(ObjectWrapper != NULL)
		{
			UObject* ObjectPointer = ObjectWrapper->GetObject<UObject>();
			const UBOOL bItemSelected = SavedSelections.Find(ObjectPointer) != NULL;
			const UBOOL bItemExpanded = SavedExpansions.Find(ObjectPointer) != NULL;
				
			if(bItemSelected == TRUE)
			{
				SelectItem(Item);
			}

			if(bItemExpanded == TRUE)
			{
				Expand(Item);
			}
		}
	}

	// Loop through all of the item's children and select/expand them.
	wxTreeItemIdValue Cookie;
	wxTreeItemId ChildItem =GetFirstChild( Item, Cookie );

	while(ChildItem.IsOk())
	{
		RestoreSelectionExpansionStateRecurse(ChildItem);

		ChildItem = GetNextChild(Item, Cookie);
	}
}


/** For runtime creation of WxComboBox items */
IMPLEMENT_DYNAMIC_CLASS(WxComboBox,wxComboBox);


/*-----------------------------------------------------------------------------
WxListView.
-----------------------------------------------------------------------------*/

/**
 * Returns the text of the item.
 * @param Index Index of the item.
 * @param Column Column of the item.
 */
wxString WxListView::GetColumnItemText(long Index, long Column)
{
	wxListItem ListItem;

	ListItem.SetId(Index);
	ListItem.SetColumn(Column);
	ListItem.m_mask = wxLIST_MASK_TEXT;
	GetItem(ListItem);

	return ListItem.GetText();
}

/*-----------------------------------------------------------------------------
	WxSpinCtrlReal
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE(WxSpinCtrlReal, wxPanel)
	EVT_MOUSE_EVENTS(WxSpinCtrlReal::OnMouseEvent)
	EVT_TEXT_ENTER(IDT_SPINCTRLREAL, WxSpinCtrlReal::OnText)
	EVT_BUTTON(IDB_SPIN_UP, WxSpinCtrlReal::OnSpin)
	EVT_BUTTON(IDB_SPIN_DOWN, WxSpinCtrlReal::OnSpin)
END_EVENT_TABLE()

WxSpinCtrlReal::WxSpinCtrlReal(wxWindow* InParent, wxWindowID InID/* =wxID_ANY */, FLOAT Value/* =0::0f */, wxPoint Position/* =wxDefaultPosition */, wxSize Size/* =wxDefaultSize */, FLOAT InMinValue/* =-100000::0f */, FLOAT InMaxValue/* =100000::0f */) : 
wxPanel(InParent, InID, Position, Size),
bFixedIncrement(FALSE),
FixedIncrementAmount(1.0f),
MinValue(InMinValue),
MaxValue(InMaxValue)
{
	SetMinSize(wxSize(100,-1));

	// Create spinner buttons
	wxSizer* MainSizer = new wxBoxSizer(wxHORIZONTAL);
	{
		//using second parameter for wxTextValidator to remove wxWidgets assertion during code execution
		const FLOAT val = Clamp<FLOAT>(Value, MinValue, MaxValue);
		ValidatorStringValue.Printf(TEXT("%f"), val);
		TextCtrl = new wxTextCtrl(this, IDT_SPINCTRLREAL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_NUMERIC, &ValidatorStringValue));
		MainSizer->Add(TextCtrl, 1, wxEXPAND);

		ButtonSizer = new wxBoxSizer(wxVERTICAL);
		{
			{
				WxMaskedBitmap bmp;
				bmp.Load(TEXT("Spinner_Up"));
				wxBitmapButton* SpinButton = new wxBitmapButton( this, IDB_SPIN_UP, bmp, wxDefaultPosition, wxSize(-1,1)  );
				ButtonSizer->Add(SpinButton, 1, wxEXPAND);
			}

			ButtonSizer->AddSpacer(4);

			{
				WxMaskedBitmap bmp;
				bmp.Load(TEXT("Spinner_Down"));
				wxBitmapButton* SpinButton = new wxBitmapButton( this, IDB_SPIN_DOWN, bmp, wxDefaultPosition, wxSize(-1,1) );
				ButtonSizer->Add(SpinButton, 1, wxEXPAND);
			}
		}
		MainSizer->Add(ButtonSizer,0, wxEXPAND);
	}
	SetSizer(MainSizer);
	


	// Create a Blank Cursor
	WxMaskedBitmap bitmap(TEXT("blank"));
	wxImage BlankImage = bitmap.ConvertToImage();
	BlankImage.SetMaskColour(192,192,192);
	BlankCursor = wxCursor(BlankImage);

	// Set the initial value for the spinner
	SetValue(Value);
}

void WxSpinCtrlReal::OnSize(wxSizeEvent &InEvent)
{
	wxRect Rect = InEvent.GetRect();

	TextCtrl->SetSize(Rect.x, Rect.y, Rect.width - 12, Rect.height);
	ButtonSizer->SetDimension(Rect.width + Rect.x - 12, Rect.y, 12, Rect.height);
}

/**
 * @return	Returns the current value of the spin control.
 */
FLOAT WxSpinCtrlReal::GetValue() const
{
	const FLOAT Value = appAtof(TextCtrl->GetValue());

	return Value;
}

/**
 * Sets the current value for the spin control.
 *
 * @param	Value		The new value of the spin control.
 * @param	bSendEvent	Whether or not to send a SPIN event letting bound controls know that we changed value.
 */
void WxSpinCtrlReal::SetValue(FLOAT Value, UBOOL bSendEvent)
{
	wxString NewStr;

	Value = Clamp<FLOAT>(Value, MinValue, MaxValue);
	NewStr.Printf(TEXT("%f"), Value);
	ValidatorStringValue = NewStr;
	TextCtrl->SetValue(NewStr);

	// Post a wx event.
	if(bSendEvent == TRUE)
	{
		wxSpinEvent Evt(wxEVT_COMMAND_SPINCTRL_UPDATED, GetId());
		wxPostEvent(GetParent(), Evt);
	}
}

/** Event handler for when the user enters text into the spin control textbox. */
void WxSpinCtrlReal::OnText(wxCommandEvent &InEvent)
{
	// Post a wx spinctrl changed event.
	wxSpinEvent Evt(wxEVT_COMMAND_SPINCTRL_UPDATED, GetId());
	wxPostEvent(GetParent(), Evt);	
}

void WxSpinCtrlReal::OnMouseEvent(wxMouseEvent& InEvent)
{
	const UBOOL bHasCapture = HasCapture();

	// See if we should be capturing the mouse.
	if(InEvent.LeftDown())
	{
		CaptureMouse();

		MouseDelta = 0;
		MouseStartPoint = InEvent.GetPosition();

		// Solve for the equation value and pass that value back to the property
		StartValue = GetValue();

		// Change the cursor and background color for the panel to indicate that we are dragging.
		wxSetCursor(BlankCursor);

		SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
		Refresh();
	}
	else if(InEvent.LeftUp() && bHasCapture)
	{
		WarpPointer(MouseStartPoint.x, MouseStartPoint.y);
		ReleaseMouse();

		// Change the cursor back to normal and the background color of the panel back to normal.
		wxSetCursor(wxNullCursor);

		SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
		Refresh();
	}
	else if(InEvent.Dragging() && bHasCapture)
	{
		const INT MoveDelta = InEvent.GetY() - MouseStartPoint.y;
		const INT DeltaSmoother = 3;

		// To keep the movement from being completely twitchy, we use a DeltaSmoother which means the mouse needs to move at least so many pixels before registering a change.
		if(Abs<INT>(MoveDelta) >= DeltaSmoother)
		{
			// Add contribution of delta to the total mouse delta, we need to invert it because Y is 0 at the top of the screen.
			MouseDelta += -(MoveDelta) / DeltaSmoother;

			// Solve for the equation value and pass that value back to the property, we always increment by 1% of the original value.
			FLOAT ChangeAmount;

			if(bFixedIncrement == FALSE)
			{
				ChangeAmount = Abs(StartValue) * 0.01f;
				const FLOAT SmallestChange = 0.01f;
				if(ChangeAmount < SmallestChange)
				{
					ChangeAmount = SmallestChange;
				}
			}
			else
			{
				ChangeAmount = FixedIncrementAmount;
			}

			WarpPointer(MouseStartPoint.x, MouseStartPoint.y);

			
			// We're spinning for a single value, so just set the new value.
			const FLOAT FinalValue = ChangeAmount * MouseDelta + StartValue;
			SetValue( FinalValue, TRUE );
		}
	}
	else if(InEvent.Moving())
	{
		// Change the cursor to a NS drag cursor when the user mouses over it so that they can tell that there is a drag opportunity here.
		wxSetCursor(wxCURSOR_SIZENS);
	}
}

void WxSpinCtrlReal::OnSpin(wxCommandEvent& InEvent)
{
	// Calculate how much to change the value by.  If there is no fixed amount set,
	// then use 1% of the current value.  Otherwise, use the fixed value that was set.

	const int WidgetId = InEvent.GetId();
	const FLOAT DirectionScale = (WidgetId == IDB_SPIN_UP) ? 1.0f : -1.0f;

	if ( bFixedIncrement )
	{
		const FLOAT ChangeAmount = FixedIncrementAmount * DirectionScale;
		const FLOAT EqResult = GetValue();
		SetValue( EqResult + ChangeAmount, TRUE );
	}
	else
	{
		const FLOAT SmallestChange = 0.01f;
		const FLOAT EqResult = GetValue();

		FLOAT ChangeAmount = Abs(EqResult) * 0.01f;
		if( ChangeAmount < SmallestChange )
		{
			ChangeAmount = SmallestChange;
		}

		SetValue( EqResult + ChangeAmount*DirectionScale, TRUE );
	}
}


/*-----------------------------------------------------------------------------
	WxCursor
-----------------------------------------------------------------------------*/

/**
 * Generates a cursor given only a local filename.  Uses the same mask as WxBitmap, (192,192,192).
 */
WxCursor::WxCursor(const char* filename)
{
	WxMaskedBitmap CursorBitmap(filename);
	wxImage CursorImage = CursorBitmap.ConvertToImage();
	CursorImage.SetMask(true);
	CursorImage.SetMaskColour(192,192,192);

	wxCursor TempCursor(CursorImage);

	*((wxCursor*)this) = TempCursor;
}

/*-----------------------------------------------------------------------------
	FCursorManager
-----------------------------------------------------------------------------*/

WxCursor& FCursorManager::GetCursor(const ANSICHAR* Filename)
{
	static TMap<const FString, WxCursor> CursorMap;
	WxCursor* Cursor = CursorMap.Find( FString( ANSI_TO_TCHAR(Filename) ) );

	if(Cursor == NULL)
	{
		FString WideFilename( ANSI_TO_TCHAR(Filename) );

		Cursor = &CursorMap.Set(WideFilename, WxCursor(Filename));

		check(Cursor);
	}

	return (*Cursor);
}


