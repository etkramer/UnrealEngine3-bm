/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "DynamicShadowStatsBrowser.h"

BEGIN_EVENT_TABLE(WxDynamicShadowStatsBrowser,WxBrowser)
	EVT_SIZE(WxDynamicShadowStatsBrowser::OnSize)
    EVT_LIST_ITEM_ACTIVATED(ID_DYNAMICSHADOWSTATSBROWSER_LISTCONTROL, WxDynamicShadowStatsBrowser::OnItemActivated)
END_EVENT_TABLE()

WxDynamicShadowStatsBrowser::WxDynamicShadowStatsBrowser()
{
	// Register our callbacks.
	GCallbackEvent->Register( CALLBACK_RefreshEditor_DynamicShadowStatsBrowser, this );
}

/**
 * Forwards the call to our base class to create the window relationship.
 * Creates any internally used windows after that
 *
 * @param DockID the unique id to associate with this dockable window
 * @param FriendlyName the friendly name to assign to this window
 * @param Parent the parent of this window (should be a Notebook)
 */
void WxDynamicShadowStatsBrowser::Create(INT DockID,const TCHAR* FriendlyName, wxWindow* Parent)
{
	// Let our base class start up the windows
	WxBrowser::Create(DockID,FriendlyName,Parent);	

	// Add a menu bar
	MenuBar = new wxMenuBar();
	// Append the docking menu choices
	WxBrowser::AddDockingMenu(MenuBar);
	
	// Create list control
	ListControl = new wxListCtrl( this, ID_DYNAMICSHADOWSTATSBROWSER_LISTCONTROL, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_HRULES | wxLC_VRULES );

	// Insert columns.
	ListControl->InsertColumn( DSSBC_Light,				*LocalizeUnrealEd("DynamicShadowStatsBrowser_Light")			);
	ListControl->InsertColumn( DSSBC_Subject,			*LocalizeUnrealEd("DynamicShadowStatsBrowser_Subject")			);
	ListControl->InsertColumn( DSSBC_PreShadow,			*LocalizeUnrealEd("DynamicShadowStatsBrowser_PreShadow")		);
	ListControl->InsertColumn( DSSBC_PassNumber,		*LocalizeUnrealEd("DynamicShadowStatsBrowser_PassNumber")		);
	ListControl->InsertColumn( DSSBC_ClassName,			*LocalizeUnrealEd("DynamicShadowStatsBrowser_ClassName")		);	
	ListControl->InsertColumn( DSSBC_FullComponentName,	*LocalizeUnrealEd("DynamicShadowStatsBrowser_FullComponentName"));	
}

/**
 * Tells the browser to update itself
 */
void WxDynamicShadowStatsBrowser::Update()
{
	BeginUpdate();

	if( GEditor->PlayWorld )
	{
		// Insertion is sped up by temporarily hiding control.
		ListControl->Hide();

		// Clear existing items.
		ListControl->DeleteAllItems();
		ObjectNames.Empty();

		// Iterate in reverse order as we're inserting at the top.
		for( INT RowIndex=GEditor->PlayWorld->DynamicShadowStats.Num()-1; RowIndex>=0; RowIndex-- ) 
		{
			const FDynamicShadowStatsRow&	StatsRow	= GEditor->PlayWorld->DynamicShadowStats(RowIndex);
			INT								ColumnIndex = 0;
			long							ItemIndex	= 0;

			if( ColumnIndex < StatsRow.Columns.Num() )
			{
				ItemIndex = ListControl->InsertItem( 0, *StatsRow.Columns(ColumnIndex) );
				ColumnIndex++;
			}

			while( ColumnIndex < StatsRow.Columns.Num() && ColumnIndex < DSSBC_MAX )
			{
				ListControl->SetItem( ItemIndex, ColumnIndex, *StatsRow.Columns(ColumnIndex) );
				ColumnIndex++;
			}

			// Find associated object name...
			// ... and insert it into list
			ObjectNames.InsertZeroed( 0 );
			if( StatsRow.Columns.Num() )
			{
				// Try to find primitive component.
				UPrimitiveComponent* PrimitiveComponent = FindObject<UPrimitiveComponent>( NULL, *StatsRow.Columns(StatsRow.Columns.Num()-1) );
				// If found, use the component's owner for selection when double clicking.
				if( PrimitiveComponent && PrimitiveComponent->GetOwner() )
				{
					ObjectNames(0) = PrimitiveComponent->GetOwner()->GetPathName();
				}
			}
			else
			{
				ObjectNames(0) = TEXT("");
			}
		}

		// Set proper column width.
		SetAutoColumnWidth();

		// Show again.
		ListControl->Show();
	}

	EndUpdate();
}

/**
 * Sets auto column width. Needs to be called after resizing as well.
 */
void WxDynamicShadowStatsBrowser::SetAutoColumnWidth()
{
	// Set proper column width
	for( INT ColumnIndex=0; ColumnIndex<DSSBC_MAX; ColumnIndex++ )
	{
		INT Width = 0;
		ListControl->SetColumnWidth( ColumnIndex, wxLIST_AUTOSIZE );
		Width = Max( ListControl->GetColumnWidth( ColumnIndex ), Width );
		ListControl->SetColumnWidth( ColumnIndex, wxLIST_AUTOSIZE_USEHEADER );
		Width = Max( ListControl->GetColumnWidth( ColumnIndex ), Width );
		ListControl->SetColumnWidth( ColumnIndex, Width );
	}
}

/**
 * Sets the size of the list control based upon our new size
 *
 * @param In the command that was sent
 */
void WxDynamicShadowStatsBrowser::OnSize( wxSizeEvent& In )
{
	// During the creation process a sizing message can be sent so don't
	// handle it until we are initialized
	if( bAreWindowsInitialized )
	{
		ListControl->SetSize( GetClientRect() );
		SetAutoColumnWidth();
	}
}

/**
 * Handler for item activation (double click) event
 *
 * @param In the command that was sent
 */
void WxDynamicShadowStatsBrowser::OnItemActivated( wxListEvent& In )
{
	INT Index = In.GetIndex();
	if( Index >= 0 && Index < ObjectNames.Num() )
	{
		const FString& ObjectName = ObjectNames(Index);
		// Go to the actor associated with the current row.
		GEditor->Exec(TEXT("ACTOR SELECT NONE"));
		GEditor->Exec(*FString::Printf(TEXT("CAMERA ALIGN NAME=%s"), *ObjectName ) );
	}
}

