/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "UnrealEdPrivateClasses.h"
#include "Properties.h"
#include "PropertyUtils.h"
#include "PropertyWindowManager.h"

#define OFFSET_None				-1
#define ARROW_Width				16

/** Internal convenience type. */
typedef WxPropertyWindow_Objects::TObjectIterator		TPropObjectIterator;

/** Internal convenience type. */
typedef WxPropertyWindow_Objects::TObjectConstIterator	TPropObjectConstIterator;

///////////////////////////////////////////////////////////////////////////////
//
// Property window toolbars.
//
///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
	WxPropertyWindowToolBarBase.
-----------------------------------------------------------------------------*/

WxPropertyWindowToolBarBase::WxPropertyWindowToolBarBase( wxWindow* InParent, wxWindowID InID )
: wxToolBar( InParent, InID, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxNO_BORDER | wxTB_3DBUTTONS )
{}

UBOOL WxPropertyWindowToolBarBase::Init()
{
	if ( !OnInit() )
	{
		return FALSE;
	}

	// We're finished adding to the toolbar.  Finalize with Wx and return.
	const bool bWasRealizeSuccessful = Realize();
	check( bWasRealizeSuccessful );
	return TRUE;
}

UBOOL WxPropertyWindowToolBarBase::OnInit()
{
	return TRUE;
}

/*-----------------------------------------------------------------------------
	WxPropertyWindowToolBarCopyLock.
-----------------------------------------------------------------------------*/

WxPropertyWindowToolBarCopyLock::WxPropertyWindowToolBarCopyLock( wxWindow* InParent, wxWindowID InID )
: WxPropertyWindowToolBarBase( InParent, InID )
{}


UBOOL WxPropertyWindowToolBarCopyLock::OnInit()
{
	// Load all toolbar button bitmaps.
	if ( !CopyB.Load( TEXT("CopyToClipboard") ) )					{ return FALSE;	}
	if ( !CopyCompleteB.Load( TEXT("CompleteCopyToClipboard") ) )	{ return FALSE;	}
	if ( !ExpandAllB.Load( TEXT("ExpandAllCategories") ) )			{ return FALSE;	}
	if ( !CollapseAllB.Load( TEXT("CollapseAllCategories") ) )		{ return FALSE;	}
	if ( !LockB.Load( TEXT("Lock") ) )								{ return FALSE; }

	// Set up the ToolBar
	AddSeparator();
	AddCheckTool( ID_LOCK_SELECTIONS, TEXT(""), LockB, LockB, *LocalizeUnrealEd("LockSelectedActorsQ") );
	AddSeparator();
	AddTool( IDM_COPY, TEXT(""), CopyB, *LocalizeUnrealEd("CopyToClipboard") );
	AddSeparator();
	AddTool( IDM_COPYCOMPLETE, TEXT(""), CopyCompleteB, *LocalizeUnrealEd("CompleteCopyToClipboard") );

	AddSeparator();
	AddTool( ID_EXPAND_ALL, TEXT(""), ExpandAllB, *LocalizeUnrealEd("ExpandAllCategories") );
	AddSeparator();
	AddTool( ID_COLLAPSE_ALL, TEXT(""), CollapseAllB, *LocalizeUnrealEd("CollapseAllCategories") );

	return TRUE;
}	

///////////////////////////////////////////////////////////////////////////////
//
// Property window frame.
//
///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
	WxPropertyWindowFrame
-----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC_CLASS(WxPropertyWindowFrame,wxFrame);

BEGIN_EVENT_TABLE( WxPropertyWindowFrame, wxFrame )
	EVT_SIZE( WxPropertyWindowFrame::OnSize )
	EVT_CLOSE( WxPropertyWindowFrame::OnClose )
	EVT_MENU( IDM_COPY, WxPropertyWindowFrame::OnCopy )
	EVT_MENU( IDM_COPYCOMPLETE, WxPropertyWindowFrame::OnCopyComplete )
	EVT_MENU( ID_EXPAND_ALL, WxPropertyWindowFrame::OnExpandAll )
	EVT_MENU( ID_COLLAPSE_ALL, WxPropertyWindowFrame::OnCollapseAll )
	EVT_MENU( ID_LOCK_SELECTIONS, WxPropertyWindowFrame::OnLock )
	EVT_UPDATE_UI( ID_LOCK_SELECTIONS, WxPropertyWindowFrame::UI_Lock )
END_EVENT_TABLE()

void WxPropertyWindowFrame::Create( wxWindow* Parent, wxWindowID id, UBOOL bShowToolBar, FNotifyHook* InNotifyHook )
{
	const bool bWasCreateSuccessful = wxFrame::Create(Parent, id, TEXT("Properties"), wxDefaultPosition, wxDefaultSize, (Parent ? wxFRAME_FLOAT_ON_PARENT : wxDIALOG_NO_PARENT ) | wxDEFAULT_FRAME_STYLE | wxFRAME_TOOL_WINDOW | wxFRAME_NO_TASKBAR );
	check( bWasCreateSuccessful );

	bLocked = FALSE;

	RegisterCreation();

	// Closing is allowed if the frame is not parented.
	if ( Parent )
	{
		DisallowClose();
	}
	else
	{
		AllowClose();
	}

	// Allocated a window for this frame to contain.
	PropertyWindow = new WxPropertyWindow;
	check( PropertyWindow );

	PropertyWindow->Create( this, InNotifyHook );
	PropertyWindow->SetCanBeHiddenByPropertyWindowManager( TRUE );

	// Create a toolbar if one was requested.
	if( bShowToolBar )
	{
		ToolBar = new WxPropertyWindowToolBarCopyLock( this, -1 );
		check( ToolBar );

		// Initialize the toolbar and bind it to the frame.
		const UBOOL bWasToolbarInitSuccessful = ToolBar->Init();
		check( bWasToolbarInitSuccessful );
		SetToolBar( ToolBar );
	}

	// Read the property window's position and size from ini.
	FWindowUtil::LoadPosSize( TEXT("FloatingPropertyWindow"), this, 64,64,350,500 );
}

WxPropertyWindowFrame::~WxPropertyWindowFrame()
{
	// Write the property window's position and size to ini.
	FWindowUtil::SavePosSize( TEXT("FloatingPropertyWindow"), this );
}

void WxPropertyWindowFrame::OnClose( wxCloseEvent& In )
{
	// Apply any outstanding edit changes before we hide/close
	PropertyWindow->FlushLastFocused();

	// if closing is allowed,  this property frame will destroy itself.  Otherwise, it just hides.
	if( IsCloseAllowed() )
	{
		// Issue the destroy notification.
		PropertyWindow->NotifyDestroy();

		// Pass the close event on to the next handler.
		In.Skip();
	}
	else
	{
		// Closing isn't allowed, so just hide.  Notify the app that the close didn't happen.
		In.Veto();
		Hide();
	}
}

void WxPropertyWindowFrame::OnSize( wxSizeEvent& In )
{
	if ( !IsCreated() )
	{
		In.Skip();
		return;
	}

	check( PropertyWindow );

	// Pass the size event down to the handled property window.
	const wxPoint pt = GetClientAreaOrigin();
	wxRect rc = GetClientRect();
	rc.y -= pt.y;

	PropertyWindow->SetSize( rc );
}

void WxPropertyWindowFrame::OnCopy( wxCommandEvent& In )
{
	FStringOutputDevice Ar;
	const FExportObjectInnerContext Context;
	for( TPropObjectIterator Itor( PropertyWindow->Root->ObjectIterator() ) ; Itor ; ++Itor )
	{
		UExporter::ExportToOutputDevice( &Context, *Itor, NULL, Ar, TEXT("T3D"), 0 );
		Ar.Logf(LINE_TERMINATOR);
	}

	appClipboardCopy(*Ar);
}

void WxPropertyWindowFrame::OnCopyComplete( wxCommandEvent& In )
{
	FStringOutputDevice Ar;

	// this will contain all objects to export, including editinline objects
	TArray<UObject*> AllObjects;
	for( TPropObjectIterator Itor( PropertyWindow->Root->ObjectIterator() ) ; Itor ; ++Itor )
	{
		AllObjects.AddUniqueItem( *Itor );
	}

	// now go through and find all of the editinline objects, and add to our list, "recursively"
	// not true recursion because i don't want to fall into some nasty infinite loop
	INT OldNumObjects, NewNumObjects;
	do
	{
		OldNumObjects = AllObjects.Num();
		// go through all objects looking for editlines
		//@todo: this could be faster if we only started looking at the newly added objects
		for (INT iObject = 0; iObject < OldNumObjects; iObject++)
		{
			// loop through all of the properties
			for (TFieldIterator<UProperty,CASTCLASS_UProperty> It(AllObjects(iObject)->GetClass()); It; ++It)
			{
				// We need to add on objects that editline object properties point to
				if ((It->PropertyFlags & (CPF_EditInline | CPF_EditInlineUse)) && Cast<UObjectProperty>(*It))
				{
					// get the value of the property
					UObject* EditInlineObject;
					It->CopySingleValue(&EditInlineObject, ((BYTE*)AllObjects(iObject)) + It->Offset);

					// did the property actually point to something?
					if (EditInlineObject)
					{
						// add it to our list
						AllObjects.AddUniqueItem(EditInlineObject);
					}
				}
			}
		}
		// find out if we added any new objects
		NewNumObjects = AllObjects.Num();
	} while (OldNumObjects != NewNumObjects);

	const FExportObjectInnerContext Context;
	for( INT o = 0 ; o < AllObjects.Num() ; ++o )
	{
		UObject* obj = AllObjects(o);
		UExporter::ExportToOutputDevice( &Context, obj, NULL, Ar, TEXT("T3D"), 0 );
		Ar.Logf(LINE_TERMINATOR);
	}

	appClipboardCopy(*Ar);
}

void WxPropertyWindowFrame::OnExpandAll( wxCommandEvent& In )
{
	TArray<UObject*> AllObjects;
	for( TPropObjectIterator Itor( PropertyWindow->Root->ObjectIterator() ) ; Itor ; ++Itor )
	{
		AllObjects.AddUniqueItem( *Itor );
	}
	PropertyWindow->SetObjectArray( AllObjects, 1, PropertyWindow->IsSorted(), PropertyWindow->ShouldShowCategories() );
}

void WxPropertyWindowFrame::OnCollapseAll( wxCommandEvent& In )
{
	PropertyWindow->CollapseAllItems();
	TArray<UObject*> AllObjects;
	for( TPropObjectIterator Itor( PropertyWindow->Root->ObjectIterator() ) ; Itor ; ++Itor )
	{
		AllObjects.AddUniqueItem( *Itor );
	}
	PropertyWindow->SetObject( NULL, 0, PropertyWindow->IsSorted(), PropertyWindow->ShouldShowCategories() );
	PropertyWindow->SetObjectArray( AllObjects, 0, PropertyWindow->IsSorted(), PropertyWindow->ShouldShowCategories() );
}

void WxPropertyWindowFrame::OnLock( wxCommandEvent& In )
{
	// Lock or unlock the property window based on the incoming event.
	bLocked = In.IsChecked();
}

void WxPropertyWindowFrame::UI_Lock( wxUpdateUIEvent& In )
{
	// Update the UI with the locked status of the contained property window.
	In.Check( IsLocked() ? true : false );
}

// Updates the caption of the floating frame based on the objects being edited
void WxPropertyWindowFrame::UpdateTitle()
{
	FString Caption;

	if( !PropertyWindow->Root->GetBaseClass() )
	{
		Caption = LocalizeGeneral("PropNone",TEXT("XWindow"));
	}
	else if( PropertyWindow->Root->GetNumObjects() == 1 )
	{
		// if the object is the default metaobject for a UClass, use the UClass's name instead
		const UObject* Object = *PropertyWindow->Root->ObjectConstIterator();
		FString ObjectName = Object->GetName();
		if ( Object->GetClass()->GetDefaultObject() == Object )
			ObjectName = Object->GetClass()->GetName();

		Caption = FString::Printf( LocalizeSecure(LocalizeGeneral("PropSingle",TEXT("XWindow")), *ObjectName) );
	}
	else
	{
		Caption = FString::Printf( LocalizeSecure(LocalizeGeneral("PropMulti",TEXT("XWindow")), *PropertyWindow->Root->GetBaseClass()->GetName(), PropertyWindow->Root->GetNumObjects()) );
	}

	SetTitle( *Caption );
}

// @todo DB: remove this method which is just a hack for UUnrealEdEngine::UpdatePropertyWindows.
void WxPropertyWindowFrame::RemoveActor(AActor* Actor)
{
	PropertyWindow->Root->RemoveActor( Actor );
	PropertyWindow->Root->Finalize();
}

///////////////////////////////////////////////////////////////////////////////
//
// Property window.
//
///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
	WxPropertyWindow
-----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC_CLASS(WxPropertyWindow,wxWindow);

BEGIN_EVENT_TABLE( WxPropertyWindow, wxWindow )
	EVT_SIZE( WxPropertyWindow::OnSize )
	EVT_SCROLL( WxPropertyWindow::OnScroll )
	EVT_MENU( wxID_REFRESH, WxPropertyWindow::OnRefresh )
	EVT_MOUSEWHEEL( WxPropertyWindow::OnMouseWheel )
	EVT_KEY_DOWN(WxPropertyWindow::OnKeyDown)
END_EVENT_TABLE()

void WxPropertyWindow::Create( wxWindow* InParent, FNotifyHook* InNotifyHook )
{
	//register event for handling rebuild messages
	wxEvtHandler::Connect(wxID_ANY, ID_UI_PROPERTYWINDOW_REBUILD, wxCommandEventHandler(WxPropertyWindow::OnRebuild));
	wxEvtHandler::Connect(wxID_ANY, ID_TOGGLE_SHOW_ALL_BUTTONS, wxCommandEventHandler(WxPropertyWindow::OnShowAllItemButtons));

	const bool bWasCreationSuccesful = wxWindow::Create( InParent, -1, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN );
	check( bWasCreationSuccesful );

	RebuildLocked = 0;
	SplitterPos = 175;
	LastFocused = NULL;
	bReadOnly = FALSE;
	bSupportsCustomControls = FALSE;
	bCanBeHiddenByPropertyWindowManager = FALSE;
	bColorPickModeEnabled = FALSE;
	ThumbPos = 0;
	NotifyHook = InNotifyHook;
	bAllowEnterKeyToApplyChanges = TRUE;

	RegisterCreation();

	// Create a vertical scrollbar for the property window.
	ScrollBar = new wxScrollBar( this, -1, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL );

	// Create the top-level property window node.
	Root = new WxPropertyWindow_Objects;
	Root->Create(	this,			// wxWindow InParent
					NULL,			// WxProperytWindow_Base* InParentItem
					this,			// WxPropertyWdinow* InTopPropertyWindow
					NULL,			// UProperty* InProperty
					-1,				// INT InPropertyOffest
					INDEX_NONE);	// INT InArrayIdx

	SetShowCategories( TRUE );
	SetShowNonEditable( FALSE );
	StopDraggingSplitter();

	GPropertyWindowManager->RegisterWindow( this );
}

WxPropertyWindow::~WxPropertyWindow()
{
	GPropertyWindowManager->UnregisterWindow( this );

	Root->RemoveAllObjects();
	Root->Destroy();
}

// Links up the Next/Prev pointers the children.  Needed for things like keyboard navigation.
void WxPropertyWindow::LinkChildren()
{
	Root->Prev = LinkChildrenForItem( Root );
}

// Recursive minion of LinkChildren.
WxPropertyWindow_Base* WxPropertyWindow::LinkChildrenForItem(WxPropertyWindow_Base* InItem)
{
	WxPropertyWindow_Base* Prev = InItem;

	for( INT x = 0 ; x < InItem->ChildItems.Num() ; ++x )
	{
		WxPropertyWindow_Base* Cur = InItem->ChildItems(x);
	
		Prev->Next = Cur;
		Cur->Prev = Prev;

		if( Cur->ChildItems.Num() > 0 )
		{
			Prev = LinkChildrenForItem( Cur );
		}
		else
		{
			Prev = Cur;
		}
	}

	return Prev;
}

void WxPropertyWindow::MoveFocusToNextItem( WxPropertyWindow_Base* InItem )
{
	WxPropertyWindow_Base* Cur = InItem;

	while ( Cur->Next && Cur->Next != Root )
	{
		if( !wxDynamicCast( Cur->Next, WxPropertyWindow_Category ) )
		{
			Cur->Next->SetFocus();
			return;
		}
		Cur = Cur->Next;
	}
}

void WxPropertyWindow::MoveFocusToPrevItem( WxPropertyWindow_Base* InItem )
{
	WxPropertyWindow_Base* Cur = InItem;

	while ( Cur->Prev && Cur->Prev != Root )
	{
		if( !wxDynamicCast( Cur->Prev, WxPropertyWindow_Category ) )
		{
			Cur->Prev->SetFocus();
			return;
		}
		Cur = Cur->Prev;
	}
}

/** Utility for finding if a particular object is being edited by this property window. */
UBOOL WxPropertyWindow::IsEditingObject(const UObject* InObject) const
{
	for( TObjectConstIterator Itor = ObjectConstIterator() ; Itor ; ++Itor )
	{
		UObject* Obj = *Itor;
		if(Obj == InObject)
		{
			return true;
		}
	}

	return false;
}


void WxPropertyWindow::OnSize( wxSizeEvent& In )
{
	if ( !IsCreated() )
	{
		In.Skip();
		return;
	}

	const wxRect rc = GetClientRect();
	const INT Width = wxSystemSettings::GetMetric( wxSYS_VSCROLL_X );

	//Root->SetSize( wxRect( rc.x, rc.y, rc.GetWidth()-Width, rc.GetHeight() ) );
	ScrollBar->SetSize( wxRect( rc.GetWidth()-Width, rc.y, Width, rc.GetHeight() ) );

	PositionChildren();
}

// Updates the scrollbar values.
void WxPropertyWindow::UpdateScrollBar()
{
	const wxRect rc = GetClientRect();
	const INT iMax = Max(0, MaxH - rc.GetHeight());
	ThumbPos = Clamp( ThumbPos, 0, iMax );

	ScrollBar->SetScrollbar( ThumbPos, rc.GetHeight(), MaxH, rc.GetHeight() );
}

/** 
* Used for color picking callbacks.  If the color picking flag is set then this function will update the selected property with the color.
*
* @param InColor	Color that was picked from scene.
*/
void WxPropertyWindow::OnColorPick( const FColor& InColor )
{
	// If we are in color pick mode, update the selected property with the color that was picked,
	// and disable color pick mode.
	if( bColorPickModeEnabled )
	{

		FString ColorString;

		UPropertyInputColor::GenerateColorString( LastFocused, InColor, ColorString );

		LastFocused->InputProxy->SendTextToObjects( LastFocused, ColorString );
		LastFocused->Refresh();

		bColorPickModeEnabled = FALSE;
	}
}

void WxPropertyWindow::OnRefresh( wxCommandEvent& In )
{
	PositionChildren();
}

void WxPropertyWindow::OnScroll( wxScrollEvent& In )
{
	ThumbPos = In.GetPosition();
	wxRect RC = GetClientRect();

	PositionChildren();
	Refresh();
	Update();
}

void WxPropertyWindow::OnMouseWheel( wxMouseEvent& In )
{
	/*
	ThumbPos -= (In.GetWheelRotation() / In.GetWheelDelta()) * PROP_ItemHeight;
	ThumbPos = Clamp( ThumbPos, 0, MaxH - PROP_ItemHeight );
	PositionChildren();
	*/

	ThumbPos -= (In.GetWheelRotation() / In.GetWheelDelta()) * PROP_DefaultItemHeight;
	const wxRect rc = GetClientRect();
	const INT iMax = Max(0, MaxH - rc.GetHeight());
	ThumbPos = Clamp( ThumbPos, 0, iMax );
	PositionChildren();
}

// Positions existing child items so they are in the proper positions, visible/hidden, etc.
void WxPropertyWindow::PositionChildren()
{
	LinkChildren();

	const wxRect rc = GetClientRect();
	const INT iMax = Max(0, MaxH - rc.GetHeight());
	ThumbPos = Clamp( ThumbPos, 0, iMax );

	const INT OldThumbPos = ThumbPos;

	MaxH = PositionChild( Root, 0, -ThumbPos );

	UpdateScrollBar();

	// This check is for the special case when a category that is so large its header isn't visible is being collapsed.
	// If this check isn't performed then the entire property window will disappear.
	if(OldThumbPos != ThumbPos)
	{
		PositionChild(Root, 0, -ThumbPos);
	}
}

// Recursive minion of PositionChildren.
INT WxPropertyWindow::PositionChild( WxPropertyWindow_Base* InItem, INT InX, INT InY )
{
	wxRect rc = GetClientRect();
	rc.width -= wxSystemSettings::GetMetric( wxSYS_VSCROLL_X );

	// Since the items's height can change depending on the input proxy and draw proxy and whether or not the item is focused, we need
	// to generate the item's height specifically for this resize.
	INT ItemHeight = 0;

	if(InItem->ChildHeight == PROP_GenerateHeight)
	{
		ItemHeight = InItem->GetPropertyHeight();
	}

	INT H = InItem->ChildHeight + ItemHeight;

	InX += PROP_Indent;

	if( ( wxDynamicCast( InItem, WxPropertyWindow_Category ) && InItem->GetParent() == Root ) ||
		  wxDynamicCast( InItem, WxPropertyWindow_Objects ) )
	{
		InX -= PROP_Indent;
	}

	////////////////////////////
	// Set position/size of any children this window has.

	for( INT x = 0 ; x < InItem->ChildItems.Num() ; ++x )
	{
		const UBOOL bShow = IsItemShown( InItem->ChildItems(x) );

		const INT WkH = PositionChild( InItem->ChildItems(x), InX, H );
		if( bShow )
		{
			H += WkH;
		}

		InItem->ChildItems(x)->Show( bShow > 0 );
	}

	if( InItem->bExpanded || InItem == Root )
	{
		H += InItem->ChildSpacer;
	}

	////////////////////////////
	// Set pos/size of this window.

	InItem->IndentX = InX;
	InItem->SetSize( 0,InY, rc.GetWidth(),H );

	// Resize the input proxy on this item.
	rc = InItem->GetClientRect();
	InItem->InputProxy->ParentResized( InItem, wxRect( rc.x+SplitterPos,rc.y,rc.width-SplitterPos, InItem->GetPropertyHeight() ) );

	// Force redraw.
	InItem->Refresh();

	return H;
}

// Look up the tree to determine the hide/show status of InItem.
UBOOL WxPropertyWindow::IsItemShown( WxPropertyWindow_Base* InItem )
{
	WxPropertyWindow_Base* Win = wxDynamicCast( InItem->GetParent(), WxPropertyWindow_Base );
	check(Win);

	// Search up to the root object window.  If anything along the way is not expanded,
	// the item must be hidden.
	while( Win != Root )
	{
		// An ancestor was hidden; this item must be invisible.
		if( Win->bCanBeExpanded && !Win->bExpanded )
		{
			return 0;
		}

		Win = wxDynamicCast( Win->GetParent(), WxPropertyWindow_Base );
		check(Win);
	}

	// No ancestors were hidden; the item must be visible.
	return 1;
}

/**
 * Searches for nearest WxPropertyWindow_Objects ancestor, returning NULL if none exists or InNode is NULL.
 * Different from the WxPropertyWindow_Base version in that an input of NULL is acceptable, and that
 * calling with an InNode of type WxPropertyWindow_Objects returns the *enclosing* WxPropertyWindow_Objects,
 * not the input itself.
 */
static WxPropertyWindow_Objects* NotifyFindObjectItemParent(WxPropertyWindow_Base* InNode)
{
	WxPropertyWindow_Objects* Result = NULL;
	wxWindow* CurNode = InNode->GetParent();
	while ( CurNode )
	{
		Result = wxDynamicCast( CurNode, WxPropertyWindow_Objects );
		if ( Result )
		{
			break;
		}
		CurNode = CurNode->GetParent();
	}
	return Result;
}

/**
 * Builds an ordered list of nested UProperty objects from the specified property up through the
 * object tree, as specified by the property window hierarchy.
 * The list tail is the property that was modified.  The list head is the class member property within
 * the top-most property window object.
 */
static FEditPropertyChain* BuildPropertyChain(WxPropertyWindow_Base* InItem, UProperty* PropertyThatChanged)
{
	WxPropertyWindow_Objects*	RootItem		= InItem->GetPropertyWindow()->GetRoot();
	FEditPropertyChain*			PropertyChain	= new FEditPropertyChain;
	WxPropertyWindow_Base*		Item			= InItem;

	WxPropertyWindow_Objects*	ObjectNode		= Item->FindObjectItemParent();
	UProperty* MemberProperty					= PropertyThatChanged;
	do
	{
		if ( Item == ObjectNode )
		{
			MemberProperty = PropertyChain->GetHead()->GetValue();
		}

		UProperty*				Property		= Item->Property;
		if ( Property )
		{
			// Skip over property window items that correspond to a single element in a static array,
			// or the inner property of another UProperty (e.g. UArrayProperty->Inner).
			if ( Item->ArrayIndex == INDEX_NONE && Property->GetOwnerProperty() == Property )
			{
				PropertyChain->AddHead( Property );
			}
		}
		Item = wxDynamicCast( Item->GetParent(), WxPropertyWindow_Base );
	} while( Item != RootItem );

	// If the modified property was a property of the object at the root of this property window, the member property will not have been set correctly
	if ( Item == ObjectNode )
	{
		MemberProperty = PropertyChain->GetHead()->GetValue();
	}

	PropertyChain->SetActivePropertyNode( PropertyThatChanged );
	PropertyChain->SetActiveMemberPropertyNode( MemberProperty );

	return PropertyChain;
}

// Calls PreEditChange on Root's objects, and passes PropertyAboutToChange to NotifyHook's NotifyPreChange.
void WxPropertyWindow::NotifyPreChange(WxPropertyWindow_Base* InItem, UProperty* PropertyAboutToChange, UObject* Object)
{
	FEditPropertyChain*			PropertyChain		= BuildPropertyChain( InItem, PropertyAboutToChange );
	WxPropertyWindow_Objects*	ObjectNode			= InItem->FindObjectItemParent();
	UProperty*					CurProperty			= PropertyAboutToChange;

	// Begin a property edit transaction.
	if ( GEditor )
	{
		GEditor->BeginTransaction( *LocalizeUnrealEd("PropertyWindowEditProperties") );
	}

	// Call through to the property window's notify hook.
	if( NotifyHook )
	{
		if ( PropertyChain == NULL || PropertyChain->Num() == 0 )
		{
			NotifyHook->NotifyPreChange( InItem, PropertyAboutToChange );
		}
		else
		{
			NotifyHook->NotifyPreChange( Object, PropertyChain );
		}
	}

	// Call PreEditChange on the object chain.
	while ( TRUE )
	{
		for( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
		{
			if ( PropertyChain == NULL || PropertyChain->Num() == 0 )
			{
				(*Itor)->PreEditChange( CurProperty );
			}
			else
			{
				(*Itor)->PreEditChange( *PropertyChain );
			}
		}

		// Pass this property to the parent's PreEditChange call.
		CurProperty = ObjectNode->GetStoredProperty();
		WxPropertyWindow_Objects* PreviousObjectNode = ObjectNode;

		// Traverse up a level in the nested object tree.
		ObjectNode = NotifyFindObjectItemParent( ObjectNode );
		if ( !ObjectNode )
		{
			// We've hit the root -- break.
			break;
		}
		else if ( PropertyChain != NULL && PropertyChain->Num() > 0 )
		{
			PropertyChain->SetActivePropertyNode( CurProperty->GetOwnerProperty() );
			for ( WxPropertyWindow_Base* BaseItem = PreviousObjectNode; BaseItem && BaseItem != ObjectNode; BaseItem = wxDynamicCast(BaseItem->GetParent(),WxPropertyWindow_Base) )
			{
				UProperty* ItemProperty = BaseItem->Property;
				if ( ItemProperty == NULL )
				{
					// if this property item doesn't have a Property, skip it...it may be a category item or the virtual
					// item used as the root for an inline object
					continue;
				}

				// skip over property window items that correspond to a single element in a static array, or
				// the inner property of another UProperty (e.g. UArrayProperty->Inner)
				if ( BaseItem->ArrayIndex == INDEX_NONE && ItemProperty->GetOwnerProperty() == ItemProperty )
				{
					PropertyChain->SetActiveMemberPropertyNode(ItemProperty);
				}
			}
		}
	}

	delete PropertyChain;
}

// Calls PostEditChange on Root's objects, and passes PropertyThatChanged to NotifyHook's NotifyPostChange.
void WxPropertyWindow::NotifyPostChange(WxPropertyWindow_Base* InItem, UProperty* PropertyThatChanged, UObject* Object)
{
	FEditPropertyChain*			PropertyChain		= BuildPropertyChain( InItem, PropertyThatChanged );
	WxPropertyWindow_Objects*	ObjectNode			= InItem->FindObjectItemParent();
	UProperty*					CurProperty			= PropertyThatChanged;

	// Fire CALLBACK_LevelDirtied when falling out of scope.
	FScopedLevelDirtied		LevelDirtyCallback;

	// remember the property that was the chain's original active property; this will correspond to the outermost property of struct/array that was modified
	UProperty* OriginalActiveProperty = PropertyChain ? PropertyChain->GetActiveMemberNode()->GetValue() : NULL;

	// Call PostEditChange on the object chain.
	while ( TRUE )
	{
		for( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
		{
			if ( PropertyChain == NULL || PropertyChain->Num() == 0 )
			{
				(*Itor)->PostEditChange( CurProperty );
			}
			else
			{
				(*Itor)->PostEditChange(*PropertyChain);
			}
			LevelDirtyCallback.Request();
		}

		// Pass this property to the parent's PostEditChange call.
		CurProperty = ObjectNode->GetStoredProperty();
		WxPropertyWindow_Objects* PreviousObjectNode = ObjectNode;

		// Traverse up a level in the nested object tree.
		ObjectNode = NotifyFindObjectItemParent( ObjectNode );
		if ( !ObjectNode )
		{
			// We've hit the root -- break.
			break;
		}
		else if ( PropertyChain != NULL && PropertyChain->Num() > 0 )
		{
			PropertyChain->SetActivePropertyNode(CurProperty->GetOwnerProperty());
			for ( WxPropertyWindow_Base* BaseItem = PreviousObjectNode; BaseItem && BaseItem != ObjectNode; BaseItem = wxDynamicCast(BaseItem->GetParent(),WxPropertyWindow_Base) )
			{
				UProperty* ItemProperty = BaseItem->Property;
				if ( ItemProperty == NULL )
				{
					// if this property item doesn't have a Property, skip it...it may be a category item or the virtual
					// item used as the root for an inline object
					continue;
				}

				// skip over property window items that correspond to a single element in a static array, or
				// the inner property of another UProperty (e.g. UArrayProperty->Inner)
				if ( BaseItem->ArrayIndex == INDEX_NONE && ItemProperty->GetOwnerProperty() == ItemProperty )
				{
					PropertyChain->SetActiveMemberPropertyNode(ItemProperty);
				}
			}
		}
	}

	// Call through to the property window's notify hook.
	if( NotifyHook )
	{
		if ( PropertyChain == NULL || PropertyChain->Num() == 0 )
		{
			NotifyHook->NotifyPostChange( InItem, PropertyThatChanged );
		}
		else
		{
			PropertyChain->SetActiveMemberPropertyNode( OriginalActiveProperty );
			PropertyChain->SetActivePropertyNode( PropertyThatChanged );
			NotifyHook->NotifyPostChange( InItem, PropertyChain );
		}
	}

	// End the property edit transaction.
	if ( GEditor )
	{
		GEditor->EndTransaction();
	}

	delete PropertyChain;

	bExecDeselectOnEscape = FALSE;
}

void WxPropertyWindow::NotifyDestroy()
{
	if( NotifyHook )
	{
		NotifyHook->NotifyDestroy( this );
	}
}

void WxPropertyWindow::SetObject( UObject* InObject, UBOOL InExpandCategories, UBOOL InSorted, UBOOL InShowCategories, UBOOL InShowNonEditable )
{
	// Don't allow property editing of the builder brush.
	// (during gameplay, there is no builder brush so we don't have to worry about it)
	AActor* Actor = Cast<AActor>(InObject);
	if ( (GWorld && GWorld->HasBegunPlay()) || Actor == NULL || !Actor->IsABuilderBrush() )
	{
		PreSetObject();
		if( InObject )
		{
			Root->AddObject( InObject );
		}

		// if the object's class has CollapseCategories, then don't show categories
		if (InObject && InObject->GetClass()->ClassFlags & CLASS_CollapseCategories)
		{
			InShowCategories = FALSE;
		}

		PostSetObject( InExpandCategories, InSorted, InShowCategories, InShowNonEditable );
	}
}

// Relinquishes focus to LastFocused.
void WxPropertyWindow::FinalizeValues()
{
	if( LastFocused )
	{
		LastFocused->SetFocus();
		wxSafeYield();
	}
}

/**
 * Flushes the last focused item to the input proxy.
 */
void WxPropertyWindow::FlushLastFocused()
{
	if ( LastFocused && LastFocused->InputProxy )
	{
		LastFocused->InputProxy->SendToObjects( LastFocused );
	}
}

void WxPropertyWindow::PreSetObject()
{
	// Remember expanded items.
	Root->bExpanded = 1;

	TArray<FString> WkExpandedItems;
	Root->RememberExpandedItems( WkExpandedItems );

	for( UClass* Class = Root->GetBaseClass() ; Class ; Class = Class->GetSuperClass() )
	{
		ExpandedItems.RemoveKey( *Class->GetName() );

		for( INT x = 0 ; x < WkExpandedItems.Num() ; ++x )
		{
			ExpandedItems.Add( *Class->GetName(), *WkExpandedItems(x) );
		}
	}

	// Make sure that the contents of LastFocused get passed to its input proxy.
	if( LastFocused )
	{
		LastFocused->Refresh();
		FlushLastFocused();
	}

	// Remove all objects.
	Root->RemoveAllObjects();
}

void WxPropertyWindow::PostSetObject(UBOOL InExpandCategories,
									 UBOOL InSorted,
									 UBOOL InShowCategories,
									 UBOOL InShowNonEditable)
{
	SetSorted( InSorted );
	SetShowCategories( InShowCategories );
	SetShowNonEditable( InShowNonEditable );

	Root->SetSorted( InSorted );
	Root->Finalize();

	// Restore expanded items.
	Root->bExpanded = 0;
	for( UClass* Class = Root->GetBaseClass() ; Class ; Class = Class->GetSuperClass() )
	{
		TArray<FString> WkExpandedItems;
		ExpandedItems.MultiFind( *Class->GetName(), WkExpandedItems );

		Root->RestoreExpandedItems( WkExpandedItems );
	}

	Root->Expand();

	// Expand categories..
	if( InExpandCategories )
	{
		Root->ExpandCategories();
	}
}

void WxPropertyWindow::Serialize( FArchive& Ar )
{
	// Serialize the root window, which will in turn serialize its children.
	Root->Serialize( Ar );
}

/**
 * Recursively searches through children for a property named PropertyName and expands it.
 * If it's a UArrayProperty, the propery's ArrayIndex'th item is also expanded.
 */
void WxPropertyWindow::ExpandItem( const FString& PropertyName, INT ArrayIndex )
{
	Root->ExpandItem(PropertyName, ArrayIndex);
}

/**
 * Expands all items in this property window.
 */
void WxPropertyWindow::ExpandAllItems()
{
	Root->ExpandAllItems();
}

/**
 * Recursively searches through children for a property named PropertyName and collapses it.
 * If it's a UArrayProperty, the propery's ArrayIndex'th item is also collapsed.
 */
void WxPropertyWindow::CollapseItem( const FString& PropertyName, INT ArrayIndex)
{
	Root->CollapseItem(PropertyName, ArrayIndex);
}

/**
 * Collapses all items in this property window.
 */
void WxPropertyWindow::CollapseAllItems()
{
	Root->Collapse();
}



/**
 * Rebuild all the properties and categories, with the same actors 
 *
 * @param IfContainingObject Only rebuild this property window if it contains the given object in the object hierarchy
 */
void WxPropertyWindow::Rebuild(UObject* IfContainingObject)
{
	// by default, rebuild this window, unless we pass in an object
	if(RebuildLocked == 0)
	{
		UBOOL bRebuildWindow = IfContainingObject == NULL;

		if (IfContainingObject)
		{
			for (TPropObjectIterator It(Root->ObjectIterator()); It; ++It)
			{
				if (*It == IfContainingObject)
				{
					bRebuildWindow = TRUE;
					break;
				}
			}
		}

		if (bRebuildWindow)
		{
			TArray<FString> ExpandedItems;

			// rebuild the properties by collapsing and restoring the categories, etc
			Root->bExpanded = 1;
			Root->RememberExpandedItems(ExpandedItems);
			Root->bExpanded = 0;
			Root->RestoreExpandedItems(ExpandedItems);
		}
	}
}

/**
 * Queues a message in the property window message queue to Rebuild all the properties and categories, with the same actors.  
 *
 * @param IfContainingObject Only rebuild this property window if it contains the given object in the object hierarchy
 */
void WxPropertyWindow::PostRebuild(UObject* IfContainingObject)
{
	wxCommandEvent Event;
	Event.SetEventType(ID_UI_PROPERTYWINDOW_REBUILD);
	Event.SetEventObject(this);
	Event.SetClientData(IfContainingObject);
	GetEventHandler()->AddPendingEvent(Event);
}

/** 
 * Command handler for the rebuild event.
 *
 * @param	Event	Information about the event.
 */
void WxPropertyWindow::OnRebuild(wxCommandEvent &Event)
{
	Rebuild((UObject*)Event.GetClientData());
}

/**
 * Event handler for when a key is pressed.
 *
 * @param	In	Information about the event.
 */
void WxPropertyWindow::OnKeyDown(wxKeyEvent &In)
{
	if(In.GetKeyCode() == WXK_ESCAPE && bExecDeselectOnEscape)
	{
		GUnrealEd->Exec(TEXT("ACTOR DESELECT"));
	}
}

/**
 * Freezes rebuild requests, so we do not process them.  
 */
void WxPropertyWindow::FreezeRebuild()
{
	RebuildLocked++;
}

/**
 * Resumes rebuild request processing.
 *
 * @param bRebuildNow	Whether or not to rebuild the window right now if rebuilding is unlocked.
 */
void WxPropertyWindow::ThawRebuild(UBOOL bRebuildNow)
{
	RebuildLocked--;

	if(RebuildLocked==0 && bRebuildNow)
	{
		Rebuild(NULL);
	}
}

/**
 * Recursively shows the buttons for every visible property item.
 */
void WxPropertyWindow::ShowAllItemButtons(WxPropertyWindow_Base *Item)
{
	if(Item != LastFocused)
	{
		const wxRect rc = Item->GetClientRect();
		Item->InputProxy->CreateControls(Item,
			Item->FindObjectItemParent()->GetBaseClass(),
			wxRect( rc.x + Item->GetPropertyWindow()->GetSplitterPos(), rc.y,
			rc.width - Item->GetPropertyWindow()->GetSplitterPos(), Item->GetPropertyHeight() ),
			*Item->GetPropertyText() );
	}

	for(int Index = 0; Index < Item->ChildItems.Num(); ++Index)
	{
		ShowAllItemButtons(Item->ChildItems(Index));
	}
}

/**
 * Recursively hides the buttons for every visible property item.
 */
void WxPropertyWindow::HideAllItemButtons(WxPropertyWindow_Base *Item)
{
	if(Item != LastFocused)
	{
		Item->InputProxy->DeleteControls();
	}
	
	for(int Index = 0; Index < Item->ChildItems.Num(); ++Index)
	{
		HideAllItemButtons(Item->ChildItems(Index));
	}
}

/**
 * Event handler for showing/hiding all of the property item buttons.
 */
void WxPropertyWindow::OnShowAllItemButtons(wxCommandEvent &Event)
{
	Freeze();
	if(GPropertyWindowManager->GetShowAllItemButtons() && !bReadOnly)
	{
		ShowAllItemButtons(Root);
	}
	else
	{
		HideAllItemButtons(Root);
	}
	Thaw();
}

/*-----------------------------------------------------------------------------
	WxPropertyWindow_Base
-----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC_CLASS(WxPropertyWindow_Base,wxWindow);

BEGIN_EVENT_TABLE( WxPropertyWindow_Base, wxWindow )
	EVT_LEFT_DOWN( WxPropertyWindow_Base::OnLeftClick )
	EVT_LEFT_UP( WxPropertyWindow_Base::OnLeftUnclick )
	EVT_LEFT_DCLICK( WxPropertyWindow_Base::OnLeftDoubleClick )
	EVT_RIGHT_DOWN( WxPropertyWindow_Base::OnRightClick )
	EVT_SET_FOCUS( WxPropertyWindow_Base::OnSetFocus )
	EVT_COMMAND_RANGE( ID_PROP_CLASSBASE_START, ID_PROP_CLASSBASE_END, wxEVT_COMMAND_MENU_SELECTED, WxPropertyWindow_Base::OnEditInlineNewClass )
	EVT_BUTTON( ID_PROP_PB_ADD, WxPropertyWindow_Base::OnPropItemButton )
	EVT_BUTTON( ID_PROP_PB_EMPTY, WxPropertyWindow_Base::OnPropItemButton )
	EVT_BUTTON( ID_PROP_PB_INSERT, WxPropertyWindow_Base::OnPropItemButton )
	EVT_BUTTON( ID_PROP_PB_DELETE, WxPropertyWindow_Base::OnPropItemButton )
	EVT_BUTTON( ID_PROP_PB_BROWSE, WxPropertyWindow_Base::OnPropItemButton )
	EVT_BUTTON( ID_PROP_PB_PICK, WxPropertyWindow_Base::OnPropItemButton )
	EVT_BUTTON( ID_PROP_PB_CLEAR, WxPropertyWindow_Base::OnPropItemButton )
	EVT_BUTTON( ID_PROP_PB_FIND, WxPropertyWindow_Base::OnPropItemButton )
	EVT_BUTTON( ID_PROP_PB_USE, WxPropertyWindow_Base::OnPropItemButton )
	EVT_BUTTON( ID_PROP_PB_NEWOBJECT, WxPropertyWindow_Base::OnPropItemButton )
	EVT_BUTTON( ID_PROP_PB_DUPLICATE, WxPropertyWindow_Base::OnPropItemButton )
	EVT_MOTION( WxPropertyWindow_Base::OnMouseMove )
	EVT_CHAR( WxPropertyWindow_Base::OnChar )
END_EVENT_TABLE()

void WxPropertyWindow_Base::Create(wxWindow* InParent,
									WxPropertyWindow_Base* InParentItem,
									WxPropertyWindow* InTopPropertyWindow,
									UProperty* InProperty,
									INT InPropertyOffset,
									INT InArrayIdx,
									UBOOL bInSupportsCustomControls/*=FALSE*/)
{
	const bool bWasCreationSuccessful = wxWindow::Create( InParent, -1 );
	check( bWasCreationSuccessful );

	IndentX = 0;
	Property = InProperty;
	PropertyOffset = InPropertyOffset;
	ArrayIndex = InArrayIdx;
	ParentItem = InParentItem;
	TopPropertyWindow = InTopPropertyWindow;
	bExpanded = FALSE;
	bCanBeExpanded = FALSE;
	bSupportsCustomControls = bInSupportsCustomControls;
	ChildHeight = PROP_GenerateHeight;
	ChildSpacer = 0;
	Next = NULL;
	Prev = NULL;
	PropertyClass = NULL;

	check(TopPropertyWindow);
	RegisterCreation();
}

void WxPropertyWindow_Base::FinishSetup( UProperty* InProperty )
{
	if ( !InProperty )
	{
		// Disable all flags if no property is bound.
		bSingleSelectOnly = FALSE;
		bEditInline = FALSE;
		bEditInlineUse = FALSE;
	}
	else
	{
		TArray<BYTE*> ReadAddresses;
		const UBOOL GotReadAddresses = GetReadAddress( this, FALSE, ReadAddresses, FALSE );
		bSingleSelectOnly = GetReadAddress( this, TRUE, ReadAddresses );

		// TRUE if the property can be expanded into the property window; that is, instead of seeing
		// a pointer to the object, you see the object's properties.
		bEditInline = ( (InProperty->PropertyFlags&CPF_EditInline) && Cast<UObjectProperty>(InProperty) && GotReadAddresses );

		// TRUE if the property is EditInline with a use button.
		bEditInlineUse = ( (InProperty->PropertyFlags&CPF_EditInlineUse) && Cast<UObjectProperty>(InProperty) && GotReadAddresses );

		// Look for tooltip metadata.
		if ( Property->HasMetaData(TEXT("tooltip")) )
		{
			SetToolTip(*Property->GetMetaData(TEXT("tooltip")));
		}
	}

	// Find and construct proxies for this property

	DrawProxy = NULL;
	InputProxy = NULL;

	UClass* DrawProxyClass = GetDrawProxyClass();
	UClass* InputProxyClass = GetInputProxyClass();
	
	// If we couldn't find a specialized proxies, use the base proxies.
	DrawProxy	= ConstructObject<UPropertyDrawProxy>( DrawProxyClass );
	InputProxy	= ConstructObject<UPropertyInputProxy>( InputProxyClass );

	// Copy IsSorted() flag from parent.  Default sorted to TRUE if no parent exists.
	SetSorted( ParentItem ? ParentItem->IsSorted() : 1 );

	SetWindowStyleFlag( wxCLIP_CHILDREN|wxCLIP_SIBLINGS );

	if(GPropertyWindowManager->GetShowAllItemButtons() && !TopPropertyWindow->IsReadOnly())
	{
		const wxRect rc = GetClientRect();
		InputProxy->CreateControls( this,
			FindObjectItemParent()->GetBaseClass(),
			wxRect( rc.x + TopPropertyWindow->GetSplitterPos(), rc.y,
			rc.width - TopPropertyWindow->GetSplitterPos(), GetPropertyHeight() ),
			*GetPropertyText() );
	}
}

WxPropertyWindow_Base::~WxPropertyWindow_Base()
{
	// This node is being destroyed, so clear references to it in the property window.
	if ( TopPropertyWindow->GetLastFocused() == this )
	{
		TopPropertyWindow->ClearLastFocused();
	}
}

BYTE* WxPropertyWindow_Base::GetBase( BYTE* Base )
{
	return ParentItem ? ParentItem->GetContents( Base ) : NULL;
}

BYTE* WxPropertyWindow_Base::GetContents( BYTE* Base )
{
	return GetBase( Base );
}

/**
 *  @return	The height of the property item, as determined by the input and draw proxies.
 */
INT WxPropertyWindow_Base::GetPropertyHeight()
{
	INT ItemHeight = PROP_DefaultItemHeight;
	const UBOOL bIsFocused = TopPropertyWindow->GetLastFocused() == this;

	// Return the height of the input proxy if we are focused, otherwise return the height of the draw proxy.
	if(bIsFocused == TRUE && InputProxy != NULL)
	{
		ItemHeight = InputProxy->GetProxyHeight();
	}
	else
	{
		ItemHeight = DrawProxy->GetProxyHeight();
	}

	return ItemHeight;
}

/*
UObject* WxPropertyWindow_Base::GetParentObject()
{
	return ParentItem->GetParentObject();
}
*/

// Returns a string representation of the contents of the property.
FString WxPropertyWindow_Base::GetPropertyText()
{
	FString	Result;

	TArray<BYTE*> ReadAddresses;
	const UBOOL bSuccess = GetReadAddress( this, bSingleSelectOnly, ReadAddresses );
	if ( bSuccess )
	{
		// Export the first value into the result string.
		BYTE* Data = ReadAddresses(0);
		Property->ExportText( 0, Result, Data - Property->Offset, Data - Property->Offset, NULL, PPF_Localized );
#if 0
		// If there are multiple objects selected, compare subsequent values against the first.
		if ( ReadAddresses.Num() > 1 )
		{
			FString CurResult;
			for ( INT i = 0 ; i < ReadAddresses.Num() ; ++i )
			{
				BYTE* Data = ReadAddresses(i);
				Property->ExportText( 0, CurResult, Data - Property->Offset, Data - Property->Offset, NULL, PPF_Localized );
				if ( CurResult != Result )
				{
					return FString();
				}
			}
		}
#endif
	}

	return Result;
}

void WxPropertyWindow_Base::SetCustomControlSupport( UBOOL bShouldSupportCustomControls )
{
	bSupportsCustomControls = bShouldSupportCustomControls;
	for ( INT i = 0; i < ChildItems.Num(); i++ )
	{
		ChildItems(i)->SetCustomControlSupport(bShouldSupportCustomControls);
	}
}

// Returns the property window this node belongs to.  The returned pointer is always valid.
WxPropertyWindow* WxPropertyWindow_Base::GetPropertyWindow()
{
	return TopPropertyWindow;
}

/**
 * Sets the target property value for property-based coloration to the value of
 * this property window node's value.
 */
void WxPropertyWindow_Base::SetPropertyColorationTarget()
{
	const FString Value( GetPropertyText() );
	if ( Value.Len() )
	{
		UClass* CommonBaseClass = GetPropertyWindow()->GetRoot()->GetBaseClass();
		FEditPropertyChain* PropertyChain = BuildPropertyChain( this, Property );
		GEditor->SetPropertyColorationTarget( Value, Property, CommonBaseClass, PropertyChain );
	}
}

void WxPropertyWindow_Base::OnLeftClick( wxMouseEvent& In )
{
	UBOOL bShouldSetFocus = TRUE;
	if( IsOnSplitter( In.GetX() ) )
	{
		// Set up to draw the window splitter
		GetPropertyWindow()->StartDraggingSplitter();
		LastX = In.GetX();
		CaptureMouse();
		bShouldSetFocus = FALSE;
	}
	else
	{
		if ( In.ShiftDown() )
		{
			// Shift-clicking on a property sets the target property value for property-based coloration.
			SetPropertyColorationTarget();
			bShouldSetFocus = FALSE;
		}
		else if( !InputProxy->LeftClick( this, In.GetX(), In.GetY() ) )
		{
			bShouldSetFocus = ClickedPropertyItem(In);
		}
	}
	In.Skip( bShouldSetFocus == TRUE );
}

void WxPropertyWindow_Base::OnRightClick( wxMouseEvent& In )
{
	// Right clicking on an item expands all items rooted at this property window node.
	UBOOL bShouldSetFocus = TRUE;

	const UBOOL bProxyHandledClick = InputProxy->RightClick( this, In.GetX(), In.GetY() );
	if( !bProxyHandledClick )
	{
		// The proxy didn't handle the click -- toggle expansion state?
		if( bCanBeExpanded )
		{
			if( bExpanded )
			{
				Collapse();
			}
			else
			{
				ExpandAllItems();
			}
			bShouldSetFocus = FALSE;
		}
	}

	if ( bShouldSetFocus )
	{
		// Item focus is changing so, make sure the property has
		// been updated with the input proxy's state.
		InputProxy->SendToObjects( this );

		// Flag the parent for updating.
		if( ParentItem )
		{
			ParentItem->Refresh();
		}
	}

	In.Skip(bShouldSetFocus==TRUE);
}

/**
 * Called when an property window item receives a left-mouse-button press which wasn't handled by the input proxy.  Typical response is to gain focus
 * and (if the property window item is expandable) to toggle expansion state.
 *
 * @param	Event	the mouse click input that generated the event
 *
 * @return	TRUE if this property window item should gain focus as a result of this mouse input event.
 */
UBOOL WxPropertyWindow_Base::ClickedPropertyItem( wxMouseEvent& Event )
{
	UBOOL bShouldGainFocus = TRUE;

	if( bCanBeExpanded )
	{
		// Only expand if clicking on the left side of the splitter
		if( Event.GetX() < GetPropertyWindow()->GetSplitterPos() || (Property && (Property->PropertyFlags & CPF_EditConst)))
		{
			if( bExpanded )
			{
				Collapse();
			}
			else
			{
				Expand();
			}

			// if we're just changing expansion state, don't take focus
			bShouldGainFocus = FALSE;
		}
	}
	else
	{
		// Item focus is changing so, make sure the property has
		// been updated with the input proxy's state.
		InputProxy->SendToObjects( this );

		// Flag the parent for updating.
		if( ParentItem )
		{
			ParentItem->Refresh();
		}
	}

	return bShouldGainFocus;
}

// Called by Collapse(), deletes any child items for any properties within this item.
void WxPropertyWindow_Base::DeleteChildItems()
{
	for( INT x = 0 ; x < ChildItems.Num() ; ++x )
	{
		ChildItems(x)->Destroy();
	}

	ChildItems.Empty();

	Next = NULL;
}

void WxPropertyWindow_Base::Expand()
{
	if( !bExpanded )
	{
		bExpanded = 1;

		const UBOOL SavedShowCategories = TopPropertyWindow->ShouldShowCategories();

		TopPropertyWindow->Freeze();

		CreateChildItems();

		GetPropertyWindow()->PositionChildren();

		TopPropertyWindow->SetShowCategories(SavedShowCategories);

		TopPropertyWindow->Thaw();

		Refresh();
	}
}

void WxPropertyWindow_Base::Collapse()
{
	if( bExpanded )
	{
		bExpanded = 0;

		TopPropertyWindow->Freeze();

		DeleteChildItems();

		GetPropertyWindow()->PositionChildren();

		TopPropertyWindow->Thaw();

		Refresh();
	}
}

/**
 * Recursively searches through children for a property named PropertyName and collapses it.
 * If it's a UArrayProperty, the propery's InArrayIndex'th item is also collapsed.
 */
void WxPropertyWindow_Base::CollapseItem( const FString& PropertyName, INT InArrayIndex )
{
	// If the name of the property stored here matches the input name . . .
	if( Property && PropertyName == Property->GetName() )
	{
		// If the property is an array, collapse the InArrayIndex'th item.
		if( Property->IsA(UArrayProperty::StaticClass()) && 
			InArrayIndex != INDEX_NONE && 
			InArrayIndex >= 0 && InArrayIndex < ChildItems.Num() )
		{
			ChildItems(InArrayIndex)->Collapse();
		}
		else
		{
			// . . . collapse it!
			Collapse();
		}
	}
	else
	{
		// Extend the search down into children.
		for(INT i=0; i<ChildItems.Num(); i++)
		{
			ChildItems(i)->CollapseItem( PropertyName, InArrayIndex );
		}
	}
}

void WxPropertyWindow_Base::OnLeftUnclick( wxMouseEvent& In )
{
	// Stop dragging the splitter
	if( GetPropertyWindow()->IsDraggingSplitter() )
	{
		GetPropertyWindow()->StopDraggingSplitter();
		LastX = 0;
		ReleaseMouse();
		wxSizeEvent se;
		GetPropertyWindow()->OnSize( se );
	}
}

void WxPropertyWindow_Base::OnMouseMove( wxMouseEvent& In )
{
	if( GetPropertyWindow()->IsDraggingSplitter() )
	{
		GetPropertyWindow()->MoveSplitterPos( -(LastX - In.GetX()) );
		LastX = In.GetX();
		SetCursor( wxCursor( wxCURSOR_SIZEWE ) );
	}

	if( IsOnSplitter( In.GetX() ) )
	{
		SetCursor( wxCursor( wxCURSOR_SIZEWE ) );
	}
	else
	{
		SetCursor( wxCursor( wxCURSOR_ARROW ) );
	}
}

void WxPropertyWindow_Base::OnLeftDoubleClick( wxMouseEvent& In )
{
	PrivateLeftDoubleClick();
}

// Implements OnLeftDoubleClick().  This is a standalone function so that double click events can occur apart from wx callbacks.
void WxPropertyWindow_Base::PrivateLeftDoubleClick()
{
	FString Value = GetPropertyText();
	UBOOL bForceValue = FALSE;
	if( !Value.Len() )
	{
		Value = GTrue;
		bForceValue = TRUE;
	}

	// Assemble a list of objects and their addresses.
	TArray<FObjectBaseAddress> ObjectsToModify;

	WxPropertyWindow_Objects* ObjectNode = FindObjectItemParent();
	for( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
	{
		UObject*	Object = *Itor;
		BYTE*		Addr = GetBase( (BYTE*) Object );
		ObjectsToModify.AddItem( FObjectBaseAddress( Object, Addr ) );
	}

	// Process the double click.
	InputProxy->DoubleClick( this, ObjectsToModify, *Value, bForceValue );

	Refresh();
}

void WxPropertyWindow_Base::OnSetFocus( wxFocusEvent& In )
{
	if (TopPropertyWindow->IsReadOnly())
	{
		return;
	}

	TopPropertyWindow->Freeze();

	// Update the previously focused item.
	if( TopPropertyWindow->GetLastFocused() )
	{
		TopPropertyWindow->GetLastFocused()->InputProxy->SendToObjects( TopPropertyWindow->GetLastFocused() );
		
		if(!GPropertyWindowManager->GetShowAllItemButtons())
		{
			TopPropertyWindow->GetLastFocused()->InputProxy->DeleteControls();
		}
		else
		{
			TopPropertyWindow->GetLastFocused()->InputProxy->OnKillFocus(this);
		}
	}

	// Create any input controls this newly focused item needs.
	
	if(!GPropertyWindowManager->GetShowAllItemButtons())
	{
		const wxRect rc = GetClientRect();
		InputProxy->CreateControls( this,
									FindObjectItemParent()->GetBaseClass(),
									wxRect( rc.x + TopPropertyWindow->GetSplitterPos(), rc.y,
											rc.width - TopPropertyWindow->GetSplitterPos(), GetPropertyHeight() ),
									*GetPropertyText() );
	}

	InputProxy->OnSetFocus(this);

	// Set this node as being in focus.
	TopPropertyWindow->SetLastFocused( this );
	TopPropertyWindow->Thaw();
	TopPropertyWindow->Refresh();
}

/**
 * Creates the appropriate editing control for the property specified.
 *
 * @param	InProperty		the property that will use the new property item
 * @param	InArrayIndex	specifies which element of an array property that this property window will represent.  Only valid
 *							when creating property window items for individual elements of an array.
 * @param	ParentItem		specified the property window item that will contain this new property window item.  Only
 *							valid when creating property window items for individual array elements or struct member properties
 */
WxPropertyWindow_Item* WxPropertyWindow_Base::CreatePropertyItem( UProperty* InProperty, INT InArrayIndex/*=INDEX_NONE*/, WxPropertyWindow_Item* ParentItem/*=NULL*/ )
{
	check(InProperty);

	WxPropertyWindow_Item* Result = NULL;
	if ( SupportsCustomControls() )
	{
		UCustomPropertyItemBindings* Bindings = UCustomPropertyItemBindings::StaticClass()->GetDefaultObject<UCustomPropertyItemBindings>();
		check(Bindings);

		// if we support custom controls, see if this property has a custom editing control class setup
		Result = Bindings->GetCustomPropertyItem(InProperty,InArrayIndex,ParentItem);
	}

	if ( Result == NULL )
	{
		// don't support custom properties or couldn't load the custom editing control class, so just create a normal item
		Result = new WxPropertyWindow_Item;
	}

	return Result;
}

// Called by Expand(), creates any child items for any properties within this item.
void WxPropertyWindow_Base::CreateChildItems()
{
}

// Creates a path to the current item.
FString WxPropertyWindow_Base::GetPath() const
{
	FString Name;

	if( ParentItem )
	{
		Name += ParentItem->GetPath();
		Name += TEXT(".");
	}

	if( Property )
	{
		Name += Property->GetName();
	}

	return Name;
}


/**
 * Appends my path, including an array index (where appropriate)
 */
void WxPropertyWindow_Base::AppendPathWithArrayIndex( FString& PathPlusIndex ) const
{
	if( ParentItem )
	{
		ParentItem->AppendPathWithArrayIndex(PathPlusIndex);
		PathPlusIndex += TEXT(".");
	}

	if( Property )
	{
		Property->AppendName(PathPlusIndex);
	}

	if ( ArrayIndex != INDEX_NONE )
	{
		PathPlusIndex += TEXT("[");
		appItoaAppend(ArrayIndex,PathPlusIndex);
		PathPlusIndex += TEXT("]");
	}
}


void WxPropertyWindow_Base::RememberExpandedItems(TArray<FString>& InExpandedItems)
{
	if( bExpanded )
	{
		FString Path;
		Path.Empty(128);
		AppendPathWithArrayIndex(Path);
		new( InExpandedItems )FString( Path );

		for( INT x = 0 ; x < ChildItems.Num() ; ++x )
		{
			ChildItems(x)->RememberExpandedItems( InExpandedItems );
		}
	}
}

void WxPropertyWindow_Base::RestoreExpandedItems(const TArray<FString>& InExpandedItems)
{
	// Expand this property window if the current item's name exists in the list of expanded items.
	FString Path;
	Path.Empty(128);
	AppendPathWithArrayIndex(Path);
	if( InExpandedItems.FindItemIndex( Path ) != INDEX_NONE )
	{
		Expand();

		for( INT x = 0 ; x < ChildItems.Num() ; ++x )
		{
			ChildItems(x)->RestoreExpandedItems( InExpandedItems );
		}
	}
}

/**
 * Recursively searches through children for a property named PropertyName and expands it.
 * If it's a UArrayProperty, the propery's InArrayIndex'th item is also expanded.
 */
void WxPropertyWindow_Base::ExpandItem( const FString& PropertyName, INT InArrayIndex )
{
	// If the name of the property stored here matches the input name . . .
	if( Property && PropertyName == Property->GetName() )
	{
		// . . . expand it!
		Expand();
	
		// If the property is an array, expand the InArrayIndex'th item.
		if( Property->IsA(UArrayProperty::StaticClass()) && 
			InArrayIndex != INDEX_NONE && 
			InArrayIndex >= 0 && InArrayIndex < ChildItems.Num() )
		{
			ChildItems(InArrayIndex)->Expand();
		}
	}
	else
	{
		// Extend the search down into children.
		for(INT i=0; i<ChildItems.Num(); i++)
		{
			ChildItems(i)->ExpandItem( PropertyName, InArrayIndex );
		}
	}
}

/**
 * Expands all items rooted at the property window node.
 */
void WxPropertyWindow_Base::ExpandAllItems()
{
	if(bCanBeExpanded)
	{
		// Expand this item . . 
		Expand();

		// . . . and recurse.
		for( INT ChildIndex = 0 ; ChildIndex < ChildItems.Num() ; ++ChildIndex )
		{
			ChildItems( ChildIndex )->ExpandAllItems();
		}
	}
}

// Follows the chain of items upwards until it finds the object window that houses this item.
WxPropertyWindow_Objects* WxPropertyWindow_Base::FindObjectItemParent()
{
	wxWindow* Cur = this;
	WxPropertyWindow_Objects* Found;

	while ( true )
	{
		Found = wxDynamicCast( Cur, WxPropertyWindow_Objects );
		if ( Found )
		{
			break;
		}
		Cur = Cur->GetParent();
	}

	return Found;
}

UBOOL WxPropertyWindow_Base::GetReadAddress(WxPropertyWindow_Base* InItem,
											UBOOL InRequiresSingleSelection,
											TArray<BYTE*>& OutAddresses,
											UBOOL bComparePropertyContents,
											UBOOL bObjectForceCompare,
											UBOOL bArrayPropertiesCanDifferInSize)
{
	// The default behaviour is to defer to the parent node, if one exists.
	return ParentItem ? ParentItem->GetReadAddress( InItem, InRequiresSingleSelection, OutAddresses, bComparePropertyContents, bObjectForceCompare, bArrayPropertiesCanDifferInSize ) : FALSE;
}

/**
 * fills in the OutAddresses array with the addresses of all of the available objects.
 * @param InItem		The property to get objects from.
 * @param OutAddresses	Storage array for all of the objects' addresses.
 */
UBOOL WxPropertyWindow_Base::GetReadAddress(WxPropertyWindow_Base* InItem,
							 TArray<BYTE*>& OutAddresses)
{
	// The default behaviour is to defer to the parent node, if one exists.
	return ParentItem ? ParentItem->GetReadAddress( InItem, OutAddresses ) : FALSE;
}

void WxPropertyWindow_Base::OnPropItemButton( wxCommandEvent& In )
{
	this->SetFocus();

	if ( GApp )
	{
		// Note the current property window so that CALLBACK_ObjectPropertyChanged doesn't
		// destroy the window out from under us.
		GApp->CurrentPropertyWindow = GetPropertyWindow();

		switch( In.GetId() )
		{
			case ID_PROP_PB_ADD:
				InputProxy->ButtonClick( this, UPropertyInputProxy::PB_Add );
				break;
			case ID_PROP_PB_EMPTY:
				InputProxy->ButtonClick( this, UPropertyInputProxy::PB_Empty );
				break;
			case ID_PROP_PB_INSERT:
				InputProxy->ButtonClick( this, UPropertyInputProxy::PB_Insert );
				break;
			case ID_PROP_PB_DELETE:
				InputProxy->ButtonClick( this, UPropertyInputProxy::PB_Delete );
				break;
			case ID_PROP_PB_BROWSE:
				InputProxy->ButtonClick( this, UPropertyInputProxy::PB_Browse );
				break;
			case ID_PROP_PB_PICK:
				InputProxy->ButtonClick( this, UPropertyInputProxy::PB_Pick );
				break;
			case ID_PROP_PB_CLEAR:
				InputProxy->ButtonClick( this, UPropertyInputProxy::PB_Clear );
				break;
			case ID_PROP_PB_FIND:
				InputProxy->ButtonClick( this, UPropertyInputProxy::PB_Find );
				break;
			case ID_PROP_PB_USE:
				InputProxy->ButtonClick( this, UPropertyInputProxy::PB_Use );
				break;
			case ID_PROP_PB_NEWOBJECT:
				InputProxy->ButtonClick( this, UPropertyInputProxy::PB_NewObject );
				break;
			case ID_PROP_PB_DUPLICATE:
				InputProxy->ButtonClick( this, UPropertyInputProxy::PB_Duplicate );
				break;
			default:
				check( 0 );
				break;
		}

		// Unset, effectively making this property window updatable by CALLBACK_ObjectPropertyChanged.
		GApp->CurrentPropertyWindow = NULL;
	}
}

void WxPropertyWindow_Base::OnEditInlineNewClass( wxCommandEvent& In )
{
	UClass* NewClass = *ClassMap.Find( In.GetId() );
	if( !NewClass )
	{
		debugf(TEXT("WARNING -- couldn't find match EditInlineNewClass"));
		return;
	}

	UBOOL bDoReplace = TRUE;
	FString CurrValue = this->GetPropertyText(); // this is used to check for a Null component and if we find one we do not want to pop up the dialog

	if( CurrValue != TEXT("None") )
	{
		// Prompt the user that this action will eliminate existing data.
		// DistributionFloat derived classes aren't prompted because they're often changed (eg for particles).
		const UBOOL bIsADistribution = NewClass->IsChildOf( UDistributionFloat::StaticClass() ) || NewClass->IsChildOf( UDistributionVector::StaticClass() );
		bDoReplace = bIsADistribution || appMsgf( AMT_YesNo, *LocalizeUnrealEd("EditInlineNewAreYouSure") );
	}

	if( bDoReplace )
	{
		Collapse();

		TArray<FObjectBaseAddress> ObjectsToModify;
		TArray<FString> Values;

		// Create a new object for all objects for which properties are being edited.
		for ( TPropObjectIterator Itor( FindObjectItemParent()->ObjectIterator() ) ; Itor ; ++Itor )
		{
			UObject*		Parent				= *Itor;
			BYTE*			Addr =				GetBase( (BYTE*) Parent );
			UObject*		UseOuter			= ( NewClass->IsChildOf( UClass::StaticClass() ) ? Cast<UClass>(Parent)->GetDefaultObject() : Parent );
			EObjectFlags	MaskedOuterFlags	= UseOuter ? UseOuter->GetMaskedFlags(RF_PropagateToSubObjects) : 0;
			UObject*		NewObject			= GEngine->StaticConstructObject( NewClass, UseOuter, NAME_None, MaskedOuterFlags | ((NewClass->ClassFlags&CLASS_Localized) ? RF_PerObjectLocalized : 0) );

			// If the object was created successfully, send its name to the parent object.
			if( NewObject )
			{
				ObjectsToModify.AddItem( FObjectBaseAddress( Parent, Addr ) );
				Values.AddItem( *NewObject->GetPathName() );
			}

			//if( GetPropertyWindow()->NotifyHook )
			//	GetPropertyWindow()->NotifyHook->NotifyExec( this, *FString::Printf(TEXT("NEWNEWOBJECT CLASS=%s PARENT=%s PARENTCLASS=%s"), *NewClass->GetName(), *(*Itor)->GetPathName(), *(*Itor)->GetClass()->GetName() ) );
		}

		if ( ObjectsToModify.Num() )
		{
			WxPropertyWindow_Objects* ObjectNode = FindObjectItemParent();

			// If more than one object is selected, an empty field indicates their values for this property differ.
			// Don't send it to the objects value in this case (if we did, they would all get set to None which isn't good).
			if( ObjectNode->GetNumObjects() > 1 && !Values(0).Len() )
			{
				check(0);
				return;
			}
			InputProxy->ImportText( this, ObjectsToModify, Values );
		}

		Expand();
	}
}

// Checks to see if an X position is on top of the property windows splitter bar.
UBOOL WxPropertyWindow_Base::IsOnSplitter( INT InX )
{
	return abs( InX - GetPropertyWindow()->GetSplitterPos() ) < 3;
}

// Calls through to the enclosing property window's NotifyPreChange.
void WxPropertyWindow_Base::NotifyPreChange(UProperty* PropertyAboutToChange, UObject* Object)
{
	GetPropertyWindow()->NotifyPreChange( this, PropertyAboutToChange, Object );
}

// Calls through to the enclosing property window's NotifyPreChange.
void WxPropertyWindow_Base::NotifyPostChange(UProperty* PropertyThatChanged, UObject* Object)
{
	GetPropertyWindow()->NotifyPostChange( this, PropertyThatChanged, Object );
}

// Expands all category items belonging to this window.
void WxPropertyWindow_Base::ExpandCategories()
{
	// this is the class we'll check for AutoCollapseCategories entries
	UClass* BaseClass = NULL;

	// find the object window that contains this property window (we might be the object window)
	WxPropertyWindow_Objects* ParentObjectWindow = FindObjectItemParent();
	if ( ParentObjectWindow != NULL )
	{
		BaseClass = ParentObjectWindow->GetBaseClass();
	}

	// Get a list of properties that are in the same category
	for( INT x = 0 ; x < ChildItems.Num() ; ++x )
	{
		WxPropertyWindow_Category* CategoryChildItem = wxDynamicCast(ChildItems(x), WxPropertyWindow_Category);
		if ( CategoryChildItem != NULL )
		{
			if ( BaseClass == NULL || !BaseClass->AutoCollapseCategories.ContainsItem(CategoryChildItem->GetCategoryName()) )
			{
				CategoryChildItem->Expand();
			}
		}
	}
}

void WxPropertyWindow_Base::OnChar( wxKeyEvent& In )
{
	// Make sure the property has been updated with the input proxy's state.
	InputProxy->SendToObjects( this );

	// Flag the parent for updating.
	if( ParentItem )
	{
		ParentItem->Refresh();
	}

	/////////////////////////////////////////////////////////
	// Parse the key and set flags for requested actions.

	UBOOL bToggleRequested = FALSE;		// Flag indicating whether or not we should toggle (expand or collapse) this property.
	UBOOL bExpandRequested = FALSE;		// Flag indicating whether or not we should try *just* expanding this property.
	UBOOL bCollapseRequested = FALSE;	// Flag indicating whether or not we should try *just* collapsing this property.

	switch( In.GetKeyCode() )
	{
		case WXK_DOWN:
			// Move the selection down.
			TopPropertyWindow->MoveFocusToNextItem( this );
			break;

		case WXK_UP:
			// Move the selection up.
			TopPropertyWindow->MoveFocusToPrevItem( this );
			break;

		case WXK_TAB:
			if( In.ShiftDown() )
			{
				// SHIFT-Tab moves the selection up.
				TopPropertyWindow->MoveFocusToPrevItem( this );
			}
			else
			{
				// Tab moves the selection down.
				TopPropertyWindow->MoveFocusToNextItem( this );
			}
			break;

		case WXK_LEFT:
			// Pressing LEFT on a struct header item will collapse it.
			bCollapseRequested = TRUE;
			break;

		case WXK_RIGHT:
			// Pressing RIGHT on a struct header item will expand it.
			bExpandRequested = TRUE;
			break;

		case WXK_RETURN:
			if ( Cast<UPropertyInputBool>( InputProxy ) )
			{
				PrivateLeftDoubleClick();
			}
			// Pressing ENTER on a struct header item or object reference will toggle it's expanded status.
			bToggleRequested = TRUE;
			break;
	}

	/////////////////////////////////////////////////////////
	// We've got our requested actions.  Now, check if the focused item is exapndable/collapsable.
	// If it is, executed requested expand/collapse action.

	// The property is expandable if it's a struct property, or if it's a valid object reference.
	const UBOOL bIsStruct = ( Cast<UStructProperty>(Property,CLASS_IsAUStructProperty) ? TRUE : FALSE );

	TArray<BYTE*> Addresses;
	const UBOOL bIsValidObjectRef = Cast<UObjectProperty>(Property,CLASS_IsAUObjectProperty) && GetReadAddress( this, bSingleSelectOnly, Addresses, FALSE );

	if ( bIsStruct || bIsValidObjectRef )
	{
		if ( bToggleRequested )
		{
			// Toggle the expand/collapse state.
			if( bExpanded )
			{
				Collapse();
			}
			else
			{
				Expand();
			}
		}
		else if ( bExpandRequested )
		{
			Expand();
		}
		else if ( bCollapseRequested )
		{
			Collapse();
		}
	}
}

IMPLEMENT_COMPARE_POINTER( WxPropertyWindow_Base, XWindowProperties, { if( wxDynamicCast( A, WxPropertyWindow_Category ) ) return appStricmp( *((WxPropertyWindow_Category*)A)->GetCategoryName().ToString(), *((WxPropertyWindow_Category*)B)->GetCategoryName().ToString() ); else return appStricmp( *A->Property->GetName(), *B->Property->GetName() ); } )

void WxPropertyWindow_Base::SortChildren()
{
	if( IsSorted() )
	{
		Sort<USE_COMPARE_POINTER(WxPropertyWindow_Base,XWindowProperties)>( &ChildItems(0), ChildItems.Num() );
	}
}

void WxPropertyWindow_Base::Serialize( FArchive& Ar )
{
	// Serialize input and draw proxies.
	Ar << InputProxy << DrawProxy;

	// Recursively serialize children
	for( INT x = 0 ; x < ChildItems.Num() ; ++x )
	{
		ChildItems(x)->Serialize( Ar );
	}
}

/**
 * Returns the draw proxy class for the property contained by this item.
 */
UClass* WxPropertyWindow_Base::GetDrawProxyClass() const
{
	UClass* Result = UPropertyDrawProxy::StaticClass();
	if ( Property != NULL )
	{
		if ( SupportsCustomControls() )
		{
			UCustomPropertyItemBindings* Bindings = UCustomPropertyItemBindings::StaticClass()->GetDefaultObject<UCustomPropertyItemBindings>();
			check(Bindings);
			
			// if we support custom controls, see if this property has a custom draw proxy control class setup
			UClass* CustomPropertyClass = Bindings->GetCustomDrawProxy(this, ArrayIndex);
			if ( CustomPropertyClass != NULL )
			{
				Result = CustomPropertyClass;
			}
		}

		if ( Result == UPropertyDrawProxy::StaticClass() )
		{
			TArray<UPropertyDrawProxy*> StandardDrawProxies;
			StandardDrawProxies.AddItem( UPropertyDrawRotationHeader::StaticClass()->GetDefaultObject<UPropertyDrawProxy>() );
			StandardDrawProxies.AddItem( UPropertyDrawRotation::StaticClass()->GetDefaultObject<UPropertyDrawProxy>()		);
			StandardDrawProxies.AddItem( UPropertyDrawNumeric::StaticClass()->GetDefaultObject<UPropertyDrawProxy>()		);
			StandardDrawProxies.AddItem( UPropertyDrawColor::StaticClass()->GetDefaultObject<UPropertyDrawProxy>()			);
			StandardDrawProxies.AddItem( UPropertyDrawBool::StaticClass()->GetDefaultObject<UPropertyDrawProxy>()			);
			StandardDrawProxies.AddItem( UPropertyDrawArrayHeader::StaticClass()->GetDefaultObject<UPropertyDrawProxy>()	);

			for( INT i=0; i<StandardDrawProxies.Num(); i++ )
			{
				check(StandardDrawProxies(i));
				if( StandardDrawProxies(i)->Supports( this, ArrayIndex ) )
				{
					Result = StandardDrawProxies(i)->GetClass();
				}
			}
		}
	}

	return Result;
}

/**
 * Returns the input proxy class for the property contained by this item.
 */
UClass* WxPropertyWindow_Base::GetInputProxyClass() const
{
	UClass* Result = UPropertyInputProxy::StaticClass();
	if ( Property != NULL )
	{
		if ( SupportsCustomControls() )
		{
			UCustomPropertyItemBindings* Bindings = UCustomPropertyItemBindings::StaticClass()->GetDefaultObject<UCustomPropertyItemBindings>();
			check(Bindings);

			// if we support custom controls, see if this property has a custom draw proxy control class setup
			UClass* CustomPropertyClass = Bindings->GetCustomInputProxy(this, ArrayIndex);
			if ( CustomPropertyClass != NULL )
			{
				Result = CustomPropertyClass;
			}
		}

		if ( Result == UPropertyInputProxy::StaticClass() )
		{
			TArray<UPropertyInputProxy*> StandardInputProxies;

			/** This list is order dependent, properties at the top are checked AFTER properties at the bottom. */
			StandardInputProxies.AddItem( UPropertyInputArrayItem::StaticClass()->GetDefaultObject<UPropertyInputProxy>()	);
			StandardInputProxies.AddItem( UPropertyInputColor::StaticClass()->GetDefaultObject<UPropertyInputProxy>()		);
			StandardInputProxies.AddItem( UPropertyInputBool::StaticClass()->GetDefaultObject<UPropertyInputProxy>()		);
			StandardInputProxies.AddItem( UPropertyInputNumeric::StaticClass()->GetDefaultObject<UPropertyInputProxy>()		);
			StandardInputProxies.AddItem( UPropertyInputInteger::StaticClass()->GetDefaultObject<UPropertyInputProxy>()	);
			StandardInputProxies.AddItem( UPropertyInputText::StaticClass()->GetDefaultObject<UPropertyInputProxy>()		);
			StandardInputProxies.AddItem( UPropertyInputRotation::StaticClass()->GetDefaultObject<UPropertyInputProxy>()	);
			StandardInputProxies.AddItem( UPropertyInputEditInline::StaticClass()->GetDefaultObject<UPropertyInputProxy>()	);
			StandardInputProxies.AddItem( UPropertyInputCombo::StaticClass()->GetDefaultObject<UPropertyInputProxy>()		);
			StandardInputProxies.AddItem( UPropertyInputArray::StaticClass()->GetDefaultObject<UPropertyInputProxy>()		);

			for( INT i=0; i<StandardInputProxies.Num(); i++ )
			{
				check(StandardInputProxies(i));
				if( StandardInputProxies(i)->Supports( this, ArrayIndex ) )
				{
					Result = StandardInputProxies(i)->GetClass();
				}
			}
		}
	}

	return Result;
}

/*-----------------------------------------------------------------------------
	WxPropertyWindow_Objects
-----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC_CLASS(WxPropertyWindow_Objects,WxPropertyWindow_Base);

BEGIN_EVENT_TABLE( WxPropertyWindow_Objects, WxPropertyWindow_Base )
	EVT_PAINT( WxPropertyWindow_Objects::OnPaint )
	EVT_ERASE_BACKGROUND( WxPropertyWindow_Objects::OnEraseBackground )
END_EVENT_TABLE()

void WxPropertyWindow_Objects::Create(wxWindow* InParent,
									   WxPropertyWindow_Base* InParentItem,
									   WxPropertyWindow* InTopPropertyWindow,
									   UProperty* InProperty,
									   INT InPropertyOffset,
									   INT InArrayIdx,
									   UBOOL bInSupportsCustomControls/*=FALSE*/ )
{
	StoredProperty = InProperty;
	InProperty = NULL;
	WxPropertyWindow_Base::Create(InParent, InParentItem, InTopPropertyWindow, InProperty, InPropertyOffset, InArrayIdx, bInSupportsCustomControls);
	BaseClass = NULL;

	FinishSetup( InProperty );

	// Copy IsSorted() flag from parent.  Default sorted to TRUE if no parent exists.
	SetSorted( InParentItem ? InParentItem->IsSorted() : 1 );
	ChildHeight = 0;
}

// @todo DB: remove this method which is just a hack for UUnrealEdEngine::UpdatePropertyWindows.
void WxPropertyWindow_Objects::RemoveActor(AActor* Actor)
{
	Objects.RemoveItem( Actor );
}

// Called by Expand(), creates any child items for any properties within this item.
void WxPropertyWindow_Objects::CreateChildItems()
{
	if( GetPropertyWindow()->ShouldShowCategories() )
	{
		ChildHeight = ChildSpacer = 0;
	}
	else
	{
		ChildHeight = ChildSpacer = PROP_CategorySpacer;
		SetWindowStyleFlag( wxCLIP_CHILDREN|wxCLIP_SIBLINGS );
	}

	// Clear out child items.
	//Collapse();
	DeleteChildItems();

	if( bEditInlineUse )
	{
		bCanBeExpanded = 0;
		return;
	}

	//////////////////////////////////////////
	// Assemble a list of category names by iterating over all fields of BaseClass.

	TArray<FName> Categories;
	for( TFieldIterator<UProperty,CASTCLASS_UProperty> It(BaseClass); It; ++It )
	{
		if( GetPropertyWindow()->ShouldShowNonEditable() || (It->HasAnyPropertyFlags(CPF_Edit) && !BaseClass->HideCategories.ContainsItem(It->Category)) )
		{
			if(GPropertyWindowManager->GetShowOnlyModifiedItems())
			{
				for(WxPropertyWindow_Objects::TObjectIterator Iter = ObjectIterator(); Iter; ++Iter)
				{
					BYTE *DefaultObj = (*Iter)->GetClass()->GetDefaults();
					if(!It->Matches(*Iter, DefaultObj, 0, FALSE, PPF_DeepComparison))
					{
						Categories.AddUniqueItem( It->Category );
					}
				}
			}
			else
			{
				Categories.AddUniqueItem( It->Category );
			}
		}
	}

	//////////////////////////////////////////
	// Add the category headers and the child items that belong inside of them.

	// Only show category headers if this is the top level object window and the parent window allows headers.
	if( /*ParentItem == NULL &&*/ GetPropertyWindow()->ShouldShowCategories() )
	{
		// This is the main WxPropertyWindow_Objects node for the window and categories are allowed.
		for( INT x = 0 ; x < Categories.Num() ; ++x )
		{
			const FName& CategoryName = Categories(x);

			WxPropertyWindow_Category* pwc = new WxPropertyWindow_Category;
			pwc->Create(	CategoryName,		// FName InCategoryName
							this,				// wxWindow* InParent
							this,				// WxPropertyWindow_Base* InParentItem
							TopPropertyWindow,	// WxPropertyWindow* InTopPropertyWindow
							NULL,				// UProperty* InProperty
							OFFSET_None,		// INT InPropertyOffset
							INDEX_NONE, 		// INT InArrayIdx
							bSupportsCustomControls);


			ChildItems.AddItem( pwc );

			// Recursively expand category properties if the category has been flagged for auto-expansion.
			if ( BaseClass->AutoExpandCategories.ContainsItem(CategoryName)
			&&	!BaseClass->AutoCollapseCategories.ContainsItem(CategoryName) )
			{
				pwc->Expand();
			}
		}
	}
	else
	{
		// Iterate over all fields, creating items.
		for( TFieldIterator<UProperty,CASTCLASS_UProperty> It(BaseClass); It; ++It )
		{
			if( GetPropertyWindow()->ShouldShowNonEditable() || (It->PropertyFlags&CPF_Edit && BaseClass->HideCategories.FindItemIndex(It->Category)==INDEX_NONE) )
			{
				if(GPropertyWindowManager->GetShowOnlyModifiedItems())
				{
					for(WxPropertyWindow_Objects::TObjectIterator Iter = ObjectIterator(); Iter; ++Iter)
					{
						BYTE *DefaultObj = (*Iter)->GetClass()->GetDefaults();
						if(!It->Matches(*Iter, DefaultObj, 0, FALSE, PPF_DeepComparison))
						{
							UProperty* CurProp = *It;
							WxPropertyWindow_Item* pwi = CreatePropertyItem(CurProp);
							pwi->Create(	this,				// wxWindow* InParent
								this,				// WxPropertyWindow_Base* InParentItem
								TopPropertyWindow,	// WxPropertyWindow* InTopPropertyWindow
								CurProp,			// UProperty* InProperty
								CurProp->Offset,	// INT InPropertyOffset
								INDEX_NONE,			// INT InArrayIdx
								bSupportsCustomControls );

							ChildItems.AddItem(pwi);
							break;
						}
					}
				}
				else
				{
					UProperty* CurProp = *It;
					WxPropertyWindow_Item* pwi = CreatePropertyItem(CurProp);
					pwi->Create(	this,				// wxWindow* InParent
						this,				// WxPropertyWindow_Base* InParentItem
						TopPropertyWindow,	// WxPropertyWindow* InTopPropertyWindow
						CurProp,			// UProperty* InProperty
						CurProp->Offset,	// INT InPropertyOffset
						INDEX_NONE,			// INT InArrayIdx
						bSupportsCustomControls );

					ChildItems.AddItem(pwi);
				}
			}
		}
	}

	SortChildren();
}

// Adds a new object to the list.
void WxPropertyWindow_Objects::AddObject( UObject* InObject )
{
	Objects.AddUniqueItem( InObject );
}

// Removes an object from the list.
void WxPropertyWindow_Objects::RemoveObject( UObject* InObject )
{
	const INT idx = Objects.FindItemIndex( InObject );

	if( idx != INDEX_NONE )
	{
		Objects.Remove( idx, 1 );
	}

	if( !Objects.Num() )
	{
		Collapse();
	}
}

// Returns TRUE if the specified object is in this node's object list.
UBOOL WxPropertyWindow_Objects::HandlesObject(UObject* InObject) const
{
	return ( Objects.FindItemIndex( InObject ) != INDEX_NONE );
}

// Removes all objects from the list.
void WxPropertyWindow_Objects::RemoveAllObjects()
{
	while( Objects.Num() )
	{
		RemoveObject( Objects(0) );
	}
}

// Called when the object list is finalized, Finalize() finishes the property window setup.
void WxPropertyWindow_Objects::Finalize()
{
	// May be NULL...
	UClass* OldBase = GetBaseClass();

	// Find an appropriate base class.
	SetBestBaseClass();
	if (BaseClass && BaseClass->HasAnyClassFlags(CLASS_CollapseCategories) )
	{
		GetPropertyWindow()->SetShowCategories( FALSE );
	}
	Expand();

	// Only automatically expand and position the child windows if this is the top level object window.
	if( this == GetPropertyWindow()->Root )
	{
		bExpanded = 1;
		// Only reset thumb pos if base class has changed
		if(OldBase != GetBaseClass())
			GetPropertyWindow()->ThumbPos = 0;
		GetPropertyWindow()->PositionChildren();
	}

	// Finally, try to update the window title.
	WxPropertyWindowFrame* ParentFrame = wxDynamicCast(GetParent()->GetParent(),WxPropertyWindowFrame);
	if( ParentFrame )
	{
		ParentFrame->UpdateTitle();
	}
}

// Looks at the Objects array and returns the best base class.  Called by
// Finalize(); that is, when the list of selected objects is being finalized.
void WxPropertyWindow_Objects::SetBestBaseClass()
{
	BaseClass = NULL;

	for( INT x = 0 ; x < Objects.Num() ; ++x )
	{
		UObject* Obj = Objects(x);
		UClass* ObjClass = Obj->GetClass() == UClass::StaticClass() ? Cast<UClass>(Obj) : Obj->GetClass();

		check( Obj );
		check( ObjClass );

		// Initialize with the class of the first object we encounter.
		if( BaseClass == NULL )
		{
			BaseClass = ObjClass;
		}

		// If we've encountered an object that's not a subclass of the current best baseclass,
		// climb up a step in the class hierarchy.
		while( !ObjClass->IsChildOf( BaseClass ) )
		{
			BaseClass = BaseClass->GetSuperClass();
		}
	}
}

UBOOL WxPropertyWindow_Objects::GetReadAddress(WxPropertyWindow_Base* InItem,
											   UBOOL InRequiresSingleSelection,
											   TArray<BYTE*>& OutAddresses,
											   UBOOL bComparePropertyContents,
											   UBOOL bObjectForceCompare,
											   UBOOL bArrayPropertiesCanDifferInSize)
{
	// Are any objects selected for property editing?
	if( !Objects.Num() )
	{
		return FALSE;
	}

	// Is there a property bound to the property window?
	if( !InItem->Property )
	{
		return FALSE;
	}

	// Requesting a single selection?
	if( InRequiresSingleSelection && Objects.Num() > 1 )
	{
		// Fail: we're editing properties for multiple objects.
		return FALSE;
	}

	//////////////////////////////////////////

	// If this item is the child of an array, return NULL if there is a different number
	// of items in the array in different objects, when multi-selecting.

	if( Cast<UArrayProperty>(InItem->Property->GetOuter()) )
	{
		const INT Num = ((FScriptArray*)InItem->ParentItem->GetBase((BYTE*)Objects(0)))->Num();
		for( INT i = 1 ; i < Objects.Num() ; i++ )
		{
			if( Num != ((FScriptArray*)InItem->ParentItem->GetBase((BYTE*)Objects(i)))->Num() )
			{
				return FALSE;
			}
		}
	}

	BYTE* Base = InItem->GetBase( (BYTE*)(Objects(0)) );

	// If the item is an array itself, return NULL if there are a different number of
	// items in the array in different objects, when multi-selecting.

	if( Cast<UArrayProperty>(InItem->Property) )
	{
		// This flag is an override for array properties which want to display e.g. the "Clear" and "Empty"
		// buttons, even though the array properties may differ in the number of elements.
		if ( !bArrayPropertiesCanDifferInSize )
		{
			INT const Num = ((FScriptArray*)InItem->GetBase( (BYTE*)Objects(0) ))->Num();
			for( INT i = 1 ; i < Objects.Num() ; i++ )
			{
				if( Num != ((FScriptArray*)InItem->GetBase((BYTE*)Objects(i)))->Num() )
				{
					return FALSE;
				}
			}
		}
	}
	else
	{
		if ( bComparePropertyContents || !Cast<UObjectProperty>(InItem->Property,CLASS_IsAUObjectProperty) || bObjectForceCompare )
		{
			// Make sure the value of this property is the same in all selected objects.
			for( INT i = 1 ; i < Objects.Num() ; i++ )
			{
				if( !InItem->Property->Identical( Base, InItem->GetBase( (BYTE*)Objects(i) ) ) )
				{
					return FALSE;
				}
			}
		}
		else
		{
			if ( Cast<UObjectProperty>(InItem->Property,CLASS_IsAUObjectProperty) )
			{
				// We don't want to exactly compare component properties.  However, we
				// need to be sure that all references are either valid or invalid.

				// If BaseObj is NON-NULL, all other objects' properties should also be non-NULL.
				// If BaseObj is NULL, all other objects' properties should also be NULL.
				UObject* BaseObj = *(UObject**) Base;

				for( INT i = 1 ; i < Objects.Num() ; i++ )
				{
					UObject* CurObj = *(UObject**) InItem->GetBase( (BYTE*)Objects(i) );
					if (   ( !BaseObj && CurObj )			// BaseObj is NULL, but this property is non-NULL!
						|| ( BaseObj && !CurObj ) )			// BaseObj is non-NULL, but this property is NULL!
					{
						
						return FALSE;
					}
				}
			}
		}
	}

	// Write addresses to the output.
	for ( INT i = 0 ; i < Objects.Num() ; ++i )
	{
		OutAddresses.AddItem( InItem->GetBase( (BYTE*)(Objects(i)) ) );
	}

	// Everything checked out and we have usable addresses.
	return TRUE;
}

/**
 * fills in the OutAddresses array with the addresses of all of the available objects.
 * @param InItem		The property to get objects from.
 * @param OutAddresses	Storage array for all of the objects' addresses.
 */
UBOOL WxPropertyWindow_Objects::GetReadAddress(WxPropertyWindow_Base* InItem,
							 TArray<BYTE*>& OutAddresses)
{
	// Are any objects selected for property editing?
	if( !Objects.Num() )
	{
		return FALSE;
	}

	// Is there a property bound to the property window?
	if( !InItem->Property )
	{
		return FALSE;
	}


	// Write addresses to the output.
	for ( INT ObjectIdx = 0 ; ObjectIdx < Objects.Num() ; ++ObjectIdx )
	{
		OutAddresses.AddItem( InItem->GetBase( (BYTE*)(Objects(ObjectIdx)) ) );
	}

	// Everything checked out and we have usable addresses.
	return TRUE;
}

BYTE* WxPropertyWindow_Objects::GetBase( BYTE* Base )
{
	UObject* Object = (UObject*)Base;
	UClass* ClassObject = Cast<UClass>(Object);
	if ( ClassObject != NULL )
		Base = ClassObject->GetDefaults();

	return Base;
}

/** Does nothing to avoid flicker when we redraw the screen. */
void WxPropertyWindow_Objects::OnEraseBackground( wxEraseEvent& Event )
{
	wxDC* DC = Event.GetDC();

	if(DC)
	{
		// Clear background
		DC->SetBackground( wxBrush( wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE), wxSOLID ) );
		DC->Clear();
	}
}

void WxPropertyWindow_Objects::OnPaint( wxPaintEvent& In )
{
	if( GetPropertyWindow()->ShouldShowCategories() || GetPropertyWindow()->Root != this )
	{
		In.Skip();
		return;
	}

	wxBufferedPaintDC dc( this );
	const wxRect rc = GetClientRect();

	// Clear background
	dc.SetBackground( wxBrush( wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE), wxSOLID ) );
	dc.Clear();

	dc.SetBrush( wxBrush( wxColour(64,64,64), wxSOLID ) );
	dc.SetPen( *wxMEDIUM_GREY_PEN );
	dc.DrawRoundedRectangle( rc.x+1,rc.y+1, rc.width-2,rc.height-2, 5 );
}

// Creates a path to the current item.
FString WxPropertyWindow_Objects::GetPath() const
{
	FString Name;

	if( ParentItem )
	{
		Name += ParentItem->GetPath();
		Name += TEXT(".");
	}

	Name += TEXT("Object");

	return Name;
}


/**
 * Appends my path, including an array index (where appropriate)
 */
void WxPropertyWindow_Objects::AppendPathWithArrayIndex(FString& PathPlusIndex ) const
{
	if( ParentItem )
	{
		ParentItem->AppendPathWithArrayIndex(PathPlusIndex);
		PathPlusIndex += TEXT(".");
	}
	PathPlusIndex += TEXT("Object");
}

/*-----------------------------------------------------------------------------
	WxPropertyWindow_Category
-----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC_CLASS(WxPropertyWindow_Category,WxPropertyWindow_Base);

BEGIN_EVENT_TABLE( WxPropertyWindow_Category, WxPropertyWindow_Base )
	EVT_LEFT_DOWN( WxPropertyWindow_Category::OnLeftClick )
	EVT_RIGHT_DOWN( WxPropertyWindow_Category::OnRightClick )
	EVT_PAINT( WxPropertyWindow_Category::OnPaint )
	EVT_ERASE_BACKGROUND( WxPropertyWindow_Category::OnEraseBackground )
	EVT_MOTION( WxPropertyWindow_Category::OnMouseMove )
END_EVENT_TABLE()

void WxPropertyWindow_Category::Create(FName InCategoryName,
										wxWindow* InParent,
										WxPropertyWindow_Base* InParentItem,
										WxPropertyWindow* InTopPropertyWindow,
										UProperty* InProperty,
										INT InPropertyOffset,
										INT InArrayIdx,
										UBOOL bInSupportsCustomControls/*=FALSE*/)
{
	WxPropertyWindow_Base::Create( InParent, InParentItem, InTopPropertyWindow, InProperty, InPropertyOffset, InArrayIdx, bInSupportsCustomControls );
	CategoryName = InCategoryName;

	FinishSetup( InProperty );

	ChildHeight = PROP_CategoryHeight;
	ChildSpacer = PROP_CategorySpacer;

	SetWindowStyleFlag( wxCLIP_CHILDREN|wxCLIP_SIBLINGS );
	this->SetToolTip( *FString::Printf(TEXT("Click to expand/collapse %s category"),*CategoryName.ToString()) );
}

void WxPropertyWindow_Category::OnPaint( wxPaintEvent& In )
{
	wxBufferedPaintDC dc( this );

	wxRect rc = GetClientRect();

	// Clear background
	dc.SetBackground( wxBrush( wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE), wxSOLID ) );
	dc.Clear();

	dc.SetFont( *wxNORMAL_FONT );
	dc.SetPen( *wxMEDIUM_GREY_PEN );
	wxFont Font = dc.GetFont();
	Font.SetWeight( wxBOLD );
	dc.SetFont( Font );

	// Top level categories have darker edge borders.
	if( GetParent() == GetPropertyWindow()->Root )
	{
		dc.SetBrush( wxBrush( wxColour(64,64,64), wxSOLID ) );
	}
	else
	{
		dc.SetBrush( wxBrush( wxColour(96,96,96), wxSOLID ) );
	}
	dc.DrawRoundedRectangle( rc.x+IndentX+1,rc.y+1, rc.width-2,rc.height-2, 5 );

	dc.SetTextForeground( *wxWHITE );
	dc.DrawText( *CategoryName.ToString(), rc.x+IndentX+4+ARROW_Width,rc.y+3 );
}

// Called by Expand(), creates any child items for any properties within this item.
void WxPropertyWindow_Category::CreateChildItems()
{
	TArray<UProperty*> Properties;

	// The parent of a category window has to be an object window.
	WxPropertyWindow_Objects* itemobj = FindObjectItemParent();

	// Get a list of properties that are in the same category
	for( TFieldIterator<UProperty,CASTCLASS_UProperty> It(itemobj->GetBaseClass()); It; ++It )
	{
		if ( (GetPropertyWindow()->ShouldShowNonEditable() && CategoryName == NAME_None && It->Category == NAME_None) ||
			(It->Category == CategoryName && (GetPropertyWindow()->ShouldShowNonEditable() || (It->PropertyFlags & CPF_Edit))) )
		{
			if(GPropertyWindowManager->GetShowOnlyModifiedItems())
			{
				for(WxPropertyWindow_Objects::TObjectIterator Iter = itemobj->ObjectIterator(); Iter; ++Iter)
				{
					BYTE *DefaultObj = (*Iter)->GetClass()->GetDefaults();
					if(!It->Matches(*Iter, DefaultObj, 0, FALSE, PPF_DeepComparison))
					{
						Properties.AddItem( *It );
						break;
					}
				}
			}
			else
			{
				Properties.AddItem( *It );
			}
		}
	}

	for( INT x = 0 ; x < Properties.Num() ; ++x )
	{
		WxPropertyWindow_Item* pwi = CreatePropertyItem(Properties(x));
		pwi->Create(	this,					// wxWindow* InParent
						this,					// WxPropertyWindow_Base* InParentItem
						TopPropertyWindow,		// WxPropertyWindow* InTopPropertyWindow
						Properties(x),			// UProperty* InProperty
						Properties(x)->Offset,	// INT InPropertyOffset
						INDEX_NONE,				// INT InArrayIdx
						bSupportsCustomControls );

		ChildItems.AddItem( pwi );
	}

	SortChildren();
}

void WxPropertyWindow_Category::OnLeftClick( wxMouseEvent& In )
{
	if( bExpanded )
	{
		Collapse();
	}
	else
	{
		Expand();
	}

	In.Skip();
}

void WxPropertyWindow_Category::OnRightClick( wxMouseEvent& In )
{
	// Right clicking on an item expands all items rooted at this property window node.
	if( bExpanded )
	{
		Collapse();
	}
	else
	{
		ExpandAllItems();
	}

	In.Skip();
}

void WxPropertyWindow_Category::OnMouseMove( wxMouseEvent& In )
{
	// Blocks message from the base class
	In.Skip();
}

// Creates a path to the current item.
FString WxPropertyWindow_Category::GetPath() const
{
	FString Name;

	if( ParentItem )
	{
		Name += ParentItem->GetPath();
		Name += TEXT(".");
	}

	Name += CategoryName.ToString();

	return Name;
}



/**
 * Appends my path, including an array index (where appropriate)
 */
void WxPropertyWindow_Category::AppendPathWithArrayIndex( FString& PathPlusIndex ) const
{
	if( ParentItem )
	{
		ParentItem->AppendPathWithArrayIndex(PathPlusIndex);
		PathPlusIndex += TEXT(".");
	}
	CategoryName.AppendString(PathPlusIndex);
}

/*-----------------------------------------------------------------------------
	WxPropertyWindow_Item
-----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC_CLASS( WxPropertyWindow_Item, WxPropertyWindow_Base )

BEGIN_EVENT_TABLE( WxPropertyWindow_Item, WxPropertyWindow_Base )
	EVT_PAINT( WxPropertyWindow_Item::OnPaint )
	EVT_ERASE_BACKGROUND( WxPropertyWindow_Item::OnEraseBackground )
	EVT_TEXT_ENTER( ID_PROPWINDOW_EDIT, WxPropertyWindow_Item::OnInputTextEnter )
	EVT_TEXT( ID_PROPWINDOW_EDIT, WxPropertyWindow_Item::OnInputTextChanged )
	EVT_COMBOBOX( ID_PROPWINDOW_COMBOBOX, WxPropertyWindow_Item::OnInputComboBox )
END_EVENT_TABLE()


void WxPropertyWindow_Item::Create(wxWindow* InParent,
								   WxPropertyWindow_Base* InParentItem,
								   WxPropertyWindow* InTopPropertyWindow,
								   UProperty* InProperty,
								   INT InPropertyOffset,
								   INT InArrayIdx,
								   UBOOL bInSupportsCustomControls/*=FALSE*/)
{
	WxPropertyWindow_Base::Create( InParent, InParentItem, InTopPropertyWindow, InProperty, InPropertyOffset, InArrayIdx, bInSupportsCustomControls );

	FinishSetup( InProperty );

	bCanBeExpanded = FALSE;

	TArray<BYTE*> Addresses;
	if(	Cast<UStructProperty>(InProperty,CLASS_IsAUStructProperty)
		||	( Cast<UArrayProperty>(InProperty) && GetReadAddress(this,FALSE,Addresses) )
		||  bEditInline
		||	( InProperty->ArrayDim > 1 && InArrayIdx == -1 ) )
	{
		bCanBeExpanded = TRUE;
	}

	// Let the input/draw proxy override whether or not the property can expand.
	if(bCanBeExpanded == TRUE && InputProxy != NULL && DrawProxy != NULL)
	{
		if(InputProxy->LetPropertyExpand() == FALSE || DrawProxy->LetPropertyExpand() == FALSE )
		{
			bCanBeExpanded = FALSE;
		}
	}

	// auto-expand distribution structs
	if (Property->IsA(UStructProperty::StaticClass()))
	{
		FName StructName = ((UStructProperty*)Property)->Struct->GetFName();
		if (StructName == NAME_RawDistributionFloat || StructName == NAME_RawDistributionVector)
		{
			ExpandAllItems();
		}
	}
}

#if 0
static UBOOL ComesFromArray(UObject* InProperty)
{
	while ( InProperty )
	{
		if ( Cast<UArrayProperty>( InProperty ) )
		{
			return TRUE;
		}
		InProperty = InProperty->GetOuter();
	}

	return FALSE;
}
#endif

UBOOL WxPropertyWindow_Item::ComesFromArray() const
{
	const WxPropertyWindow_Base* CurItem = this;
	while ( CurItem )
	{
		if ( Cast<UArrayProperty>( CurItem->Property ) )
		{
			return TRUE;
		}
		CurItem = CurItem->ParentItem;
	}

	return FALSE;
}

UBOOL WxPropertyWindow_Item::DiffersFromDefault(UObject* InObject, UProperty* InProperty)
{
	check( InObject );
	check( InProperty );

	// For the moment, consider array properties or properties with an array outer to be different.
#if 0
	if ( ComesFromArray( InProperty ) )
#else
	if ( ComesFromArray() )
#endif
	{
		return TRUE;
	}

	UBOOL bDiffersFromDefault = FALSE;

	UObject* ParentObject = InObject;			// The first enclosing object.
	BYTE* ObjectData = (BYTE*)ParentObject;

	//////////////////////////
	// Select the appropriate defaults for the enclosing object.

	UObject* ParentDefault=NULL;
	UComponent* ParentComponent = Cast<UComponent>( ParentObject );

	if ( ParentComponent )
	{
		ParentDefault = ParentComponent->ResolveSourceDefaultObject();
		if ( !ParentDefault )
		{
			ParentDefault = ParentComponent->GetArchetype();
		}
	}
	else if ( ParentObject->GetClass() == UClass::StaticClass() )
	{
		ParentDefault = Cast<UClass>(ParentObject)->GetDefaultObject();
		ObjectData = (BYTE*)ParentDefault;
	}
	else
	{
		ParentDefault = ParentObject->GetArchetype();
	}

	check( ParentDefault );

	//////////////////////////
	// Check the property against its default.
	// If the property is an object property, we have to take special measures.

	UObjectProperty* ObjectProperty = Cast<UObjectProperty>( InProperty,CLASS_IsAUObjectProperty );
	UArrayProperty* ArrayProperty = Cast<UArrayProperty>( InProperty );
	UArrayProperty* OuterArrayProperty = Cast<UArrayProperty>( InProperty->GetOuter() );

	if ( ObjectProperty )
	{
		// The object is an object property.  It differs if any of its internals differ from their default.
		UComponentProperty* ComponentProperty = Cast<UComponentProperty>( ObjectProperty );
		if ( ComponentProperty )
		{
			// The address ParentObject.Property
			BYTE* PropAddr = GetBase( ObjectData );
			UComponent* Component = *(UComponent**)PropAddr;
			if ( Component )
			{
				// Get the defaults this component.
				UObject* ObjDefault = Component->ResolveSourceDefaultObject();
				if ( !ObjDefault )
				{
					// If none exist, because the object was created editinline, use the component class' defaults.
					ObjDefault = Component->GetArchetype();
				}
				check( ObjDefault );

				// Get the component object . . .
				UObject* Obj = *(UObject**)PropAddr;
				check( Obj );

				// . . . and its class.
				UClass* ObjClass = Obj->GetClass();
				check( ObjClass );

				// Iterate over all properties of the object.  If any differ, the component property differs.
				for( UProperty* CurProperty = ObjClass->PropertyLink ;
					CurProperty && !bDiffersFromDefault ;
					CurProperty = CurProperty->PropertyLinkNext )
				{
					for( INT Index = 0 ; Index < CurProperty->ArrayDim ; ++Index )
					{
						if( !CurProperty->Matches( Obj, ObjDefault, Index, FALSE, PPF_DeepComparison ) )
						{
							bDiffersFromDefault = TRUE;
							break;
						}
					}
				}
			}
		}
	}
	else if ( ArrayProperty )
	{
		// @todo DB: implement array property comparison
		checkMsg( 0, "Untrapped array property" );
	}
	else if ( OuterArrayProperty )
	{
		// @todo DB: implement array property comparison
		checkMsg( 0, "Untrapped outer array property" );
	}
	else
	{
		// The property is a simple field.  Compare it against the enclosing object's default for that property.

		// The address ParentObject.Property
		BYTE* PropAddr = GetBase( ObjectData );

		// The offset from ParentObject to ParentObject.Property
		const INT OffsetFromObject = (PropAddr - (BYTE*)ObjectData);

		// Compare the property by offsetting from the objects to compare.
		if( !InProperty->Matches( ObjectData + OffsetFromObject,
								  (BYTE*)ParentDefault + OffsetFromObject, 0, TRUE, PPF_DeepComparison ) )
		{
			bDiffersFromDefault = TRUE;
		}
	}

	return bDiffersFromDefault;
}

void WxPropertyWindow_Item::OnPaint( wxPaintEvent& In )
{
	wxBufferedPaintDC dc( this );
	const wxRect rc = GetClientRect();

	// Colors will be derived from the base color.
	const wxColour BaseColor = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);

	const INT SplitterPos = GetPropertyWindow()->GetSplitterPos()-2;
	UBOOL bIsEditConst = (Property->PropertyFlags&CPF_EditConst) ? TRUE : FALSE;
	// If this is the focused item, draw a greenish selection background.
	UBOOL bIsFocused = GetPropertyWindow()->GetLastFocused() == this;
	const INT ItemHeight = GetPropertyHeight();

	// Clear background
	dc.SetBackground( wxBrush( wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE), wxSOLID ) );
	dc.Clear();


	if( bIsFocused )
	{
		const wxColour HighlightClr = wxColour( BaseColor.Red(), 255, BaseColor.Blue() );
		dc.SetBrush( wxBrush( HighlightClr, wxSOLID ) );
		dc.SetPen( wxPen( HighlightClr, 1 /* width */, wxSOLID ) );
		if ( bIsEditConst )
		{
			// only highlight left of the splitter for edit const
			dc.DrawRectangle( wxRect( rc.x, rc.y, SplitterPos, ItemHeight ) );
		}
		else
		{
			// highlight the entire area
			dc.DrawRectangle( wxRect( rc.x, rc.y, rc.GetWidth(), ItemHeight ) );
		}
	}
	// if this is edit const then change the background color
	if ( bIsEditConst )
	{
		const wxColour HighlightClr = wxColour( BaseColor.Red() - 64, BaseColor.Green() - 64, BaseColor.Blue() - 64 );
		dc.SetBrush( wxBrush( HighlightClr, wxSOLID ) );
		dc.SetPen( wxPen( HighlightClr, 1 /* width */, wxSOLID ) );
		if ( bIsFocused )
		{
			// only highlight right of the splitter if focused
			dc.DrawRectangle( wxRect( SplitterPos, rc.y, rc.GetWidth() - SplitterPos, ItemHeight ) );
		}
		else
		{
			// highlight the entire area
			dc.DrawRectangle( wxRect( rc.x, rc.y, rc.GetWidth(), ItemHeight ) );
		}
	}

	// Set the font.
	dc.SetFont( *wxNORMAL_FONT );

	// Draw an expansion arrow on the left if this node can be expanded.
	if( bCanBeExpanded )
	{
		const wxColour LightColor( BaseColor.Red()-32, BaseColor.Green()-32, BaseColor.Blue()-32 );
		dc.SetTextBackground( LightColor );

		dc.SetBrush( wxBrush( LightColor, wxSOLID ) );
		dc.SetPen( wxPen( LightColor, 1, wxSOLID ) );
		const wxRect wkrc( rc.x + IndentX, rc.y, rc.width - IndentX, rc.height );
		dc.DrawRectangle( wkrc );

		// Draw the down or right arrow, depending on whether the node is expanded.
		dc.DrawBitmap(  bExpanded ? GPropertyWindowManager->ArrowDownB : GPropertyWindowManager->ArrowRightB,
						IndentX, wkrc.y+((PROP_DefaultItemHeight - GPropertyWindowManager->ArrowDownB.GetHeight())/2), true );
	}

	// Thin line on top to act like separator.
	const wxColour LighterColor( BaseColor.Red()-24, BaseColor.Green()-24, BaseColor.Blue()-24 );
	dc.SetPen( wxPen( LighterColor, 1, wxSOLID ) );

	dc.DrawLine( 0,0, rc.GetWidth(),0 );

	dc.DrawLine( SplitterPos,0, SplitterPos,rc.GetHeight() );

	///////////////////////////////////////////////////////////////////////////
	//
	// LEFT SIDE OF SPLITTER
	//
	///////////////////////////////////////////////////////////////////////////

	// Mask out the region left of the splitter.
	dc.SetClippingRegion( rc.x,rc.y, SplitterPos-2,rc.GetHeight() );

	// Check to see if the property value differs from default.
	UBOOL bDiffersFromDefault = FALSE;

	// Get an iterator for the enclosing objects.
	WxPropertyWindow_Objects* ObjectNode = FindObjectItemParent();
	for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
	{
		if ( DiffersFromDefault( *Itor, Property ) )
		{
			bDiffersFromDefault = TRUE;
			break;
		}
	}

	// If the property differs from the default, draw the text in bold.
	if ( bDiffersFromDefault )
	{
		wxFont Font = dc.GetFont();
		Font.SetWeight( wxBOLD );
		dc.SetFont( Font );
	}

	// Draw the text
	RenderItemName( dc, rc );

	///////////////////////////////////////////////////////////////////////////
	//
	// RIGHT SIDE OF SPLITTER
	//
	///////////////////////////////////////////////////////////////////////////

	// Set up a proper clipping area and tell the draw proxy to draw the value

	dc.SetClippingRegion( SplitterPos+2,rc.y, rc.GetWidth()-(SplitterPos+2),rc.GetHeight() );

	BYTE* ValueAddress = NULL;
	TArray<BYTE*> Addresses;
	if ( GetReadAddress( this, bSingleSelectOnly, Addresses, FALSE, TRUE ) )
	{
		ValueAddress = Addresses(0);
	}

	const wxRect ValueRect( SplitterPos+2,					// x
							0,								// y
							rc.GetWidth()-(SplitterPos+2),	// width
							ItemHeight );					// height

	if( ValueAddress )
	{
		DrawProxy->Draw( &dc, ValueRect, ValueAddress, Property, InputProxy );
	}
	else
	{
		DrawProxy->DrawUnknown( &dc, ValueRect );
	}

	dc.DestroyClippingRegion();

	// Clean up

	dc.SetBrush( wxNullBrush );
	dc.SetPen( wxNullPen );
}

/**
 * Renders the left side of the property window item.
 *
 * @param	RenderDeviceContext		the device context to use for rendering the item name
 * @param	ClientRect				the bounding region of the property window item
 */
void WxPropertyWindow_Item::RenderItemName( wxBufferedPaintDC& RenderDeviceContext, const wxRect& ClientRect )
{
	FString LeftText;
	if( ArrayIndex==-1 )
	{
		LeftText = Property->GetMetaData(TEXT("DisplayName"));
		if ( LeftText.Len() == 0 )
		{
			LeftText = Property->GetName();
		}
	}
	else
	{
		if ( Property == NULL || Property->ArraySizeEnum == NULL )
		{
			LeftText = *FString::Printf(TEXT("[%i]"), ArrayIndex );
		}
		else
		{
			LeftText = *FString::Printf(TEXT("[%s]"), *Property->ArraySizeEnum->GetEnum(ArrayIndex).ToString());
		}
	}

	INT W, H;
	RenderDeviceContext.GetTextExtent( *LeftText, &W, &H );

	const INT YOffset = (PROP_DefaultItemHeight - H) / 2;
	RenderDeviceContext.DrawText( *LeftText, ClientRect.x+IndentX+( bCanBeExpanded ? ARROW_Width : 0 ),ClientRect.y+YOffset );

	RenderDeviceContext.DestroyClippingRegion();
}


void WxPropertyWindow_Item::OnInputTextChanged( wxCommandEvent& In )
{
	Refresh();
}

void WxPropertyWindow_Item::OnInputTextEnter( wxCommandEvent& In )
{
	InputProxy->SendToObjects( this );
	ParentItem->Refresh();
	check( Cast<UPropertyInputText>(InputProxy) );
    static_cast<UPropertyInputText*>( InputProxy )->ClearTextSelection();

	Refresh();
}

void WxPropertyWindow_Item::OnInputComboBox( wxCommandEvent& In )
{
	InputProxy->SendToObjects( this );
	Refresh();
}

// Called by Expand(), creates any child items for any properties within this item.
void WxPropertyWindow_Item::CreateChildItems()
{
	UStructProperty* StructProperty = Cast<UStructProperty>(Property,CLASS_IsAUStructProperty);
	UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property);
	UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property,CLASS_IsAUObjectProperty);

	// Copy IsSorted() flag from parent.  Default sorted to TRUE if no parent exists.
	SetSorted( ParentItem ? ParentItem->IsSorted() : 1 );

	if( Property->ArrayDim > 1 && ArrayIndex == -1 )
	{
		// Expand array.
		SetSorted( 0 );
		for( INT i = 0 ; i < Property->ArrayDim ; i++ )
		{
			WxPropertyWindow_Item* pwi = CreatePropertyItem(Property,i,this);
			pwi->Create( this, this, TopPropertyWindow, Property, i*Property->ElementSize, i, bSupportsCustomControls );
			ChildItems.AddItem(pwi);
		}
	}
	else if( ArrayProperty )
	{
		// Expand array.
		SetSorted( 0 );

		FScriptArray* Array = NULL;
		TArray<BYTE*> Addresses;
		if ( GetReadAddress( this, bSingleSelectOnly, Addresses ) )
		{
			Array = (FScriptArray*)Addresses(0);
		}

		if( Array )
		{
			for( INT i = 0 ; i < Array->Num() ; i++ )
			{
				WxPropertyWindow_Item* pwi = CreatePropertyItem(ArrayProperty,i,this);
				pwi->Create( this, this, TopPropertyWindow, ArrayProperty->Inner, i*ArrayProperty->Inner->ElementSize, i, bSupportsCustomControls );
				ChildItems.AddItem(pwi);
			}
		}
	}
	else if( StructProperty )
	{
		// Expand struct.
		for( TFieldIterator<UProperty,CASTCLASS_UProperty> It(StructProperty->Struct); It; ++It )
		{
			UProperty* StructMember = *It;
			if( GetPropertyWindow()->ShouldShowNonEditable() || (StructMember->PropertyFlags&CPF_Edit) )
			{
				WxPropertyWindow_Item* pwi = CreatePropertyItem( StructMember,INDEX_NONE,this);
				pwi->Create( this, this, TopPropertyWindow, StructMember, StructMember->Offset, INDEX_NONE, bSupportsCustomControls );
				ChildItems.AddItem(pwi);
			}
		}
	}
	else if( ObjectProperty )
	{
		BYTE* ReadValue = NULL;

		TArray<BYTE*> Addresses;
		if( GetReadAddress( this, bSingleSelectOnly, Addresses, FALSE ) )
		{
			// We've got some addresses, and we know they're all NULL or non-NULL.
			// Have a peek at the first one, and only build an objects node if we've got addresses.
			UObject* obj = *(UObject**)Addresses(0);
			if( obj )
			{
				WxPropertyWindow_Objects* NewNode = new WxPropertyWindow_Objects;
				NewNode->Create( this, this, TopPropertyWindow, Property, OFFSET_None, INDEX_NONE, bSupportsCustomControls );
				for ( INT i = 0 ; i < Addresses.Num() ; ++i )
				{
					NewNode->AddObject( *(UObject**) Addresses(i) );
				}
				NewNode->Finalize();
				ChildItems.AddItem( NewNode );
			}
			else
			{
				PropertyClass = ObjectProperty->PropertyClass;
			}
		}
	}

	SortChildren();
}

BYTE* WxPropertyWindow_Item::GetBase( BYTE* Base )
{
	return ParentItem->GetContents(Base) + PropertyOffset;
}

BYTE* WxPropertyWindow_Item::GetContents( BYTE* Base )
{
	Base = GetBase(Base);
	UArrayProperty* ArrayProperty;
	if( (ArrayProperty=Cast<UArrayProperty>(Property))!=NULL )
	{
		Base = (BYTE*)((FScriptArray*)Base)->GetData();
	}
	return Base;
}

/* ==========================================================================================================
	UCustomPropertyItemBindings
========================================================================================================== */
IMPLEMENT_CLASS(UCustomPropertyItemBindings);

/**
 * Returns the custom draw proxy class that should be used for the property associated with
 * the WxPropertyWindow_Base specified.
 *
 * @param	ProxyOwnerItem	the property window item that will be using this draw proxy
 * @param	InArrayIndex	specifies which element of an array property that this property window will represent.  Only valid
 *							when creating property window items for individual elements of an array.
 *
 * @return	a pointer to a child of UPropertyDrawProxy that should be used as the draw proxy
 *			for the specified property, or NULL if there is no custom draw proxy configured for
 *			the property.
 */
UClass* UCustomPropertyItemBindings::GetCustomDrawProxy( const WxPropertyWindow_Base* ProxyOwnerItem, INT InArrayIndex/*=INDEX_NONE*/ )
{
	check(ProxyOwnerItem);

	UProperty* InProperty = ProxyOwnerItem->Property;
	check(InProperty);

	UClass* Result = NULL;
	UStructProperty* StructProp = Cast<UStructProperty>(InProperty);
	const UBOOL bIsArrayProperty = InProperty->ArrayDim > 1 || (StructProp == NULL && InProperty->IsA(UArrayProperty::StaticClass()));

	const FString PropertyPathName = InProperty->GetPathName();
	const FString StructPathName = StructProp ? StructProp->Struct->GetPathName() : TEXT("");

	// first, see if we have a custom draw proxy class associated with this specific property
	for ( INT Index = 0; Index < CustomPropertyDrawProxies.Num(); Index++ )
	{
		FPropertyItemCustomProxy& CustomProxy = CustomPropertyDrawProxies(Index);

		UBOOL bMatchesCustomProxy =
			CustomProxy.PropertyPathName == PropertyPathName ||

			// if this is a struct property, see if there is a custom editing control for the struct itself
			(StructProp && CustomProxy.PropertyPathName == StructPathName);

		if ( bMatchesCustomProxy )
		{
			// validate that it's OK to use the custom proxy for arrays
			if ( bIsArrayProperty )
			{
				if ( InArrayIndex == INDEX_NONE && !CustomProxy.bReplaceArrayHeaders )
				{
					// this custom draw proxy cannot be used for array headers
					return NULL;
				}

				if ( InArrayIndex != INDEX_NONE && CustomProxy.bIgnoreArrayElements )
				{
					// this custom draw proxy cannot be used for individual array elements
					return NULL;
				}
			}
			// found an entry for this property - if the associated class hasn't yet been loaded, load it now
			if ( CustomProxy.PropertyItemClass == NULL )
			{
				if ( CustomProxy.PropertyItemClassName.Len() == 0 )
				{
					debugf(NAME_Error, TEXT("No PropertyItemClassName specified for custom draw proxy: %s"), *CustomProxy.PropertyPathName);
				}
				else
				{
					UClass* CustomProxyClass = LoadClass<UPropertyDrawProxy>(NULL, *CustomProxy.PropertyItemClassName, NULL, LOAD_None, NULL);
					if ( CustomProxyClass != NULL )
					{
						CustomProxy.PropertyItemClass = CustomProxyClass;
					}
				}
			}

			if ( CustomProxy.PropertyItemClass != NULL &&
				 CustomProxy.PropertyItemClass->GetDefaultObject<UPropertyDrawProxy>()->Supports(ProxyOwnerItem, ProxyOwnerItem->ArrayIndex))
			{
				Result = CustomProxy.PropertyItemClass;
				break;
			}
		}
	}

	// If we haven't found a matching item so far, check to see if we have a custom proxy class associated with this property type
	if ( Result == NULL && StructProp == NULL )
	{
		UObjectProperty* ObjectProp = Cast<UObjectProperty>(InProperty);
		if ( ObjectProp != NULL )
		{
			const FString ObjectClassPathName = ObjectProp->PropertyClass->GetPathName();
			for ( INT Index = 0; Index < CustomPropertyTypeDrawProxies.Num(); Index++ )
			{
				FPropertyTypeCustomProxy& CustomProxy = CustomPropertyTypeDrawProxies(Index);
				if ( CustomProxy.PropertyName == InProperty->GetClass()->GetFName() )
				{
					if ( CustomProxy.PropertyObjectClassPathName.Len() == 0 )
					{
						debugf(NAME_Error, TEXT("No PropertyObjectClassPathName specified for custom draw proxy (type): %s"), *CustomProxy.PropertyName.ToString());
					}
					else if ( CustomProxy.PropertyObjectClassPathName == ObjectClassPathName )
					{
						// validate that it's OK to use the custom proxy for arrays
						if ( bIsArrayProperty )
						{
							if ( InArrayIndex == INDEX_NONE && !CustomProxy.bReplaceArrayHeaders )
							{
								// this custom draw proxy cannot be used for array headers
								return NULL;
							}

							if ( InArrayIndex != INDEX_NONE && CustomProxy.bIgnoreArrayElements )
							{
								// this custom draw proxy cannot be used for individual array elements
								return NULL;
							}
						}
						if ( CustomProxy.PropertyItemClass == NULL )
						{
							if ( CustomProxy.PropertyItemClassName.Len() == 0 )
							{
								debugf(NAME_Error, TEXT("No PropertyItemClassName specified for custom draw proxy item class: %s"), *CustomProxy.PropertyName.ToString());
							}
							else
							{
								UClass* CustomProxyClass = LoadClass<UPropertyDrawProxy>(NULL, *CustomProxy.PropertyItemClassName, NULL, LOAD_None, NULL);
								if ( CustomProxyClass != NULL )
								{
									CustomProxy.PropertyItemClass = CustomProxyClass;
								}
							}
						}

						if ( CustomProxy.PropertyItemClass != NULL &&
							 CustomProxy.PropertyItemClass->GetDefaultObject<UPropertyDrawProxy>()->Supports(ProxyOwnerItem, ProxyOwnerItem->ArrayIndex) )
						{
							Result = CustomProxy.PropertyItemClass;
							break;
						}
					}
				}
			}
		}
	}

	return Result;
}

/**
 * Returns the custom input proxy class that should be used for the property associated with
 * the WxPropertyWindow_Base specified.
 *
 * @param	ProxyOwnerItem	the property window item that will be using this input proxy
 * @param	InArrayIndex	specifies which element of an array property that this property window will represent.  Only valid
 *							when creating property window items for individual elements of an array.
 *
 * @return	a pointer to a child of UPropertyInputProxy that should be used as the input proxy
 *			for the specified property, or NULL if there is no custom input proxy configured for
 *			the property.
 */
UClass* UCustomPropertyItemBindings::GetCustomInputProxy( const WxPropertyWindow_Base* ProxyOwnerItem, INT InArrayIndex/*=INDEX_NONE*/ )
{
	check(ProxyOwnerItem);

	UProperty* InProperty = ProxyOwnerItem->Property;
	check(InProperty);

	UClass* Result = NULL;
	UStructProperty* StructProp = Cast<UStructProperty>(InProperty);
	const UBOOL bIsArrayProperty = InProperty->ArrayDim > 1 || (StructProp == NULL && InProperty->IsA(UArrayProperty::StaticClass()));

	const FString PropertyPathName = InProperty->GetPathName();
	const FString StructPathName = StructProp ? StructProp->Struct->GetPathName() : TEXT("");

	// first, see if we have a custom input proxy class associated with this specific property
	for ( INT Index = 0; Index < CustomPropertyInputProxies.Num(); Index++ )
	{
		FPropertyItemCustomProxy& CustomProxy = CustomPropertyInputProxies(Index);

		UBOOL bMatchesCustomProxy =
			CustomProxy.PropertyPathName == PropertyPathName ||

			// if this is a struct property, see if there is a custom editing control for the struct itself
			(StructProp && CustomProxy.PropertyPathName == StructPathName);

		if ( bMatchesCustomProxy )
		{
			// validate that it's OK to use the custom proxy for arrays
			if ( bIsArrayProperty )
			{
				if ( InArrayIndex == INDEX_NONE && !CustomProxy.bReplaceArrayHeaders )
				{
					// this custom input proxy cannot be used for array headers
					return NULL;
				}

				if ( InArrayIndex != INDEX_NONE && CustomProxy.bIgnoreArrayElements )
				{
					// this custom input proxy cannot be used for individual array elements
					return NULL;
				}
			}
			// found an entry for this property - if the associated class hasn't yet been loaded, load it now
			if ( CustomProxy.PropertyItemClass == NULL )
			{
				if ( CustomProxy.PropertyItemClassName.Len() == 0 )
				{
					debugf(NAME_Error, TEXT("No PropertyItemClassName specified for custom input proxy: %s"), *CustomProxy.PropertyPathName);
				}
				else
				{
					UClass* CustomProxyClass = LoadClass<UPropertyInputProxy>(NULL, *CustomProxy.PropertyItemClassName, NULL, LOAD_None, NULL);
					if ( CustomProxyClass != NULL )
					{
						CustomProxy.PropertyItemClass = CustomProxyClass;
					}
				}
			}

			if ( CustomProxy.PropertyItemClass != NULL &&
				 CustomProxy.PropertyItemClass->GetDefaultObject<UPropertyInputProxy>()->Supports(ProxyOwnerItem, ProxyOwnerItem->ArrayIndex) )
			{
				Result = CustomProxy.PropertyItemClass;
                break;
			}
		}
	}

	// If we haven't found a matching item so far, check to see if we have a custom proxy class associated with this property type
	if ( Result == NULL && StructProp == NULL )
	{
		UObjectProperty* ObjectProp = Cast<UObjectProperty>(InProperty);
		if ( ObjectProp != NULL )
		{
			const FString ObjectClassPathName = ObjectProp->PropertyClass->GetPathName();
			for ( INT Index = 0; Index < CustomPropertyTypeInputProxies.Num(); Index++ )
			{
				FPropertyTypeCustomProxy& CustomProxy = CustomPropertyTypeInputProxies(Index);
				if ( CustomProxy.PropertyName == InProperty->GetClass()->GetFName() )
				{
					if ( CustomProxy.PropertyObjectClassPathName.Len() == 0 )
					{
						debugf(NAME_Error, TEXT("No PropertyObjectClassPathName specified for custom input proxy: %s"), *CustomProxy.PropertyName.ToString());
					}
					else if ( CustomProxy.PropertyObjectClassPathName == ObjectClassPathName )
					{
						// validate that it's OK to use the custom proxy for arrays
						if ( bIsArrayProperty )
						{
							if ( InArrayIndex == INDEX_NONE && !CustomProxy.bReplaceArrayHeaders )
							{
								// this custom input proxy cannot be used for array headers
								return NULL;
							}

							if ( InArrayIndex != INDEX_NONE && CustomProxy.bIgnoreArrayElements )
							{
								// this custom input proxy cannot be used for individual array elements
								return NULL;
							}
						}
						if ( CustomProxy.PropertyItemClass == NULL )
						{
							if ( CustomProxy.PropertyItemClassName.Len() == 0 )
							{
								debugf(NAME_Error, TEXT("No PropertyItemClassName specified for custom input proxy item class: %s"), *CustomProxy.PropertyName.ToString());
							}
							else
							{
								UClass* CustomProxyClass = LoadClass<UPropertyInputProxy>(NULL, *CustomProxy.PropertyItemClassName, NULL, LOAD_None, NULL);
								if ( CustomProxyClass != NULL )
								{
									CustomProxy.PropertyItemClass = CustomProxyClass;
								}
							}
						}

						if ( CustomProxy.PropertyItemClass != NULL &&
							 CustomProxy.PropertyItemClass->GetDefaultObject<UPropertyInputProxy>()->Supports(ProxyOwnerItem, ProxyOwnerItem->ArrayIndex) )
						{
							Result = CustomProxy.PropertyItemClass;
							break;
						}
					}
				}
			}
		}
	}

	return Result;
}

/**
 * Returns the custom property item class that should be used for the property specified.
 *
 * @param	InProperty	the property that will use the custom property item
 *
 * @return	a pointer to a child of WxPropertyWindow_Item that should be used as the property
 *			item for the specified property, or NULL if there is no custom property item configured
 * 			for the property.
 */
WxPropertyWindow_Item* UCustomPropertyItemBindings::GetCustomPropertyItem( UProperty* InProperty, INT InArrayIndex/*=INDEX_NONE*/, WxPropertyWindow_Item* ParentItem/*=NULL*/ )
{
	check(InProperty);

	WxPropertyWindow_Item* Result = NULL;
	UStructProperty* StructProp = Cast<UStructProperty>(InProperty);
	const UBOOL bIsArrayProperty = InProperty->ArrayDim > 1 || (StructProp == NULL && InProperty->IsA(UArrayProperty::StaticClass()));

	const FString PropertyPathName = InProperty->GetPathName();
	const FString StructPathName = StructProp ? StructProp->Struct->GetPathName() : TEXT("");
    
    // use this to make certain that the correct values are being used for the UCustomPropertyItemBindings look up code
	//debugf( TEXT( "PropertyPathName: %s  StructPathName: %s" ), *PropertyPathName, *StructPathName );

	// first, see if we have a custom control associated with this specific property
	for ( INT Index = 0; Index < CustomPropertyClasses.Num(); Index++ )
	{
		FPropertyItemCustomClass& CustomClass = CustomPropertyClasses(Index);

		UBOOL bMatchesCustomClass =
			CustomClass.PropertyPathName == PropertyPathName ||

			// if this is a struct property, see if there is a custom editing control for the struct itself
			(StructProp && CustomClass.PropertyPathName == StructPathName);

		if ( bMatchesCustomClass )
		{
			// found an entry for this property - if the associated class hasn't yet been loaded, load it now
			if ( CustomClass.WxPropertyItemClass == NULL )
			{
				if ( CustomClass.PropertyItemClassName.Len() == 0 )
				{
					debugf(NAME_Error, TEXT("No PropertyItemClassName specified for custom property item class: %s"), *CustomClass.PropertyPathName);
				}
				else
				{
					CustomClass.WxPropertyItemClass = wxClassInfo::FindClass(*CustomClass.PropertyItemClassName);
				}
			}

			if ( CustomClass.WxPropertyItemClass != NULL )
			{
				// validate that it's OK to use the custom property control for arrays
				if ( bIsArrayProperty )
				{
					if ( InArrayIndex == INDEX_NONE && !CustomClass.bReplaceArrayHeaders )
					{
						// this custom property class cannot be used for array headers
						return NULL;
					}

					if ( InArrayIndex != INDEX_NONE && CustomClass.bIgnoreArrayElements )
					{
						// this custom property class cannot be used for individual array elements
						return NULL;
					}
				}
				Result = wxDynamicCast(CustomClass.WxPropertyItemClass->CreateObject(),WxPropertyWindow_Item);
				break;
			}
		}
	}

	// If we haven't found a matching item so far, check to see if we have a custom property item class associated with this property type
	if ( Result == NULL && StructProp == NULL )
	{
		UObjectProperty* ObjectProp = Cast<UObjectProperty>(InProperty);
		if ( ObjectProp != NULL )
		{
			const FString ObjectClassPathName = ObjectProp->PropertyClass->GetPathName();

			// next, check to see if we have a custom proxy class associated with this property type
			for ( INT Index = 0; Index < CustomPropertyTypeClasses.Num(); Index++ )
			{
				FPropertyTypeCustomClass& CustomClass = CustomPropertyTypeClasses(Index);
				if ( CustomClass.PropertyName == InProperty->GetClass()->GetFName() )
				{
					if ( CustomClass.PropertyObjectClassPathName.Len() == 0 )
					{
						debugf(NAME_Error, TEXT("No PropertyObjectClassPathName specified for custom property item class: %s"), *CustomClass.PropertyName.ToString());
					}
					else if ( CustomClass.PropertyObjectClassPathName == ObjectClassPathName )
					{
						if ( CustomClass.WxPropertyItemClass == NULL )
						{
							if ( CustomClass.PropertyItemClassName.Len() == 0 )
							{
								debugf(NAME_Error, TEXT("No PropertyItemClassName specified for custom property item class: %s"), *CustomClass.PropertyName.ToString());
							}
							else
							{
								CustomClass.WxPropertyItemClass = wxClassInfo::FindClass(*CustomClass.PropertyItemClassName);
							}
						}

						if ( CustomClass.WxPropertyItemClass != NULL )
						{
							// validate that it's OK to use the custom property control for arrays
							if ( bIsArrayProperty )
							{
								if ( InArrayIndex == INDEX_NONE && !CustomClass.bReplaceArrayHeaders )
								{
									// this custom property class cannot be used for array headers
									return NULL;
								}

								if ( InArrayIndex != INDEX_NONE && CustomClass.bIgnoreArrayElements )
								{
									// this custom property class cannot be used for individual array elements
									return NULL;
								}
							}
							Result = wxDynamicCast(CustomClass.WxPropertyItemClass->CreateObject(), WxPropertyWindow_Item);
							break;
						}
					}
				}
			}
		}
	}

	return Result;
}
