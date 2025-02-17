/*=============================================================================
	DlgGenericBrowserSearch.cpp: Generic Browser search dialog implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "DlgGenericBrowserSearch.h"
#include "GenericBrowser.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Static helpers.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {
static int wxCALLBACK ResultsListSort(long InItem1, long InItem2, long InSortData)
{
	UObject* A = (UObject*)InItem1;
	UObject* B = (UObject*)InItem2;
	FListViewSortOptions* so = (FListViewSortOptions*)InSortData;

	FString CompA, CompB, wk;

	// Generate a string to run the compares against for each object based on
	// the current column.
	switch( so->Column )
	{
	case 0:			// Object name
		CompA = A->GetName();
		CompB = B->GetName();
		break;

	case 1:			// Package+Groups name
		wk = A->GetFullGroupName(1);
		if( wk.Len() )
		{
			CompA = FString::Printf( TEXT("%s.%s"), *A->GetOutermost()->GetName(), *wk );
		}
		else
		{
			CompA = FString::Printf( TEXT("%s"), *A->GetOutermost()->GetName() );
		}

		wk = B->GetFullGroupName(1);
		if( wk.Len() )
		{
			CompB = FString::Printf( TEXT("%s.%s"), *B->GetOutermost()->GetName(), *wk );
		}
		else
		{
			CompB = FString::Printf( TEXT("%s"), *B->GetOutermost()->GetName() );
		}
		break;

	default:
		check(0);	// Invalid column!
		break;
	}

	// Start with a base string compare.
	int Ret = appStricmp( *CompA, *CompB );

	// If we are sorting backwards, invert the string comparison result.
	if( !so->bSortAscending )
	{
		Ret *= -1;
	}

	return Ret;
}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxDlgGenericBrowserSearch
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

WxDlgGenericBrowserSearch::WxDlgGenericBrowserSearch()
{
	const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, GApp->EditorFrame, TEXT("ID_DLG_GB_SEARCH") );
	check( bSuccess );

	SearchForEdit = (wxTextCtrl*)FindWindow( XRCID( "ID_SEARCH_FOR" ) );
	check( SearchForEdit != NULL );
	StartsWithRadio = (wxRadioButton*)FindWindow( XRCID( "ID_STARTING_WITH" ) );
	check( StartsWithRadio != NULL );
	ResultsList = (wxListCtrl*)FindWindow( XRCID( "ID_RESULTS" ) );
	check( ResultsList != NULL );
	ResultsLabel = (wxStaticText*)FindWindow( XRCID( "ID_RESULTS_TEXT" ) );
	check( ResultsLabel != NULL );

	StartsWithRadio->SetValue( 1 );

	ResultsList->InsertColumn( 0, *LocalizeUnrealEd("Package"), wxLIST_FORMAT_LEFT, 300 );
	ResultsList->InsertColumn( 0, *LocalizeUnrealEd("Name"), wxLIST_FORMAT_LEFT, 128 );

	ADDEVENTHANDLER( XRCID("ID_SEARCH_FOR"), wxEVT_COMMAND_TEXT_UPDATED, &WxDlgGenericBrowserSearch::OnSearchTextChanged );
	ADDEVENTHANDLER( XRCID("ID_STARTING_WITH"), wxEVT_COMMAND_RADIOBUTTON_SELECTED, &WxDlgGenericBrowserSearch::OnSearchTextChanged );
	ADDEVENTHANDLER( XRCID("ID_CONTAINING"), wxEVT_COMMAND_RADIOBUTTON_SELECTED, &WxDlgGenericBrowserSearch::OnSearchTextChanged );
	ADDEVENTHANDLER( XRCID("ID_RESULTS"), wxEVT_COMMAND_LIST_COL_CLICK, &WxDlgGenericBrowserSearch::OnColumnClicked );
	ADDEVENTHANDLER( XRCID("ID_RESULTS"), wxEVT_COMMAND_LIST_ITEM_ACTIVATED, &WxDlgGenericBrowserSearch::OnItemActivated );

	FWindowUtil::LoadPosSize( TEXT("DlgGBSearch"), this, -1, -1, 400, 300 );
	FLocalizeWindow( this );
}

WxDlgGenericBrowserSearch::~WxDlgGenericBrowserSearch()
{
	FWindowUtil::SavePosSize( TEXT("DlgGBSearch"), this );
}

void WxDlgGenericBrowserSearch::OnSearchTextChanged( wxCommandEvent& In )
{
	UpdateResults();
}

void WxDlgGenericBrowserSearch::OnColumnClicked(wxListEvent& In)
{
	const int Column = In.GetColumn();
	if( Column > -1 )
	{
		if( Column == SearchOptions.Column )
		{
			// Clicking on the same column will flip the sort order
			SearchOptions.bSortAscending = !SearchOptions.bSortAscending;
		}
		else
		{
			// Clicking on a new column will set that column as current and reset the sort order.
			SearchOptions.Column = In.GetColumn();
			SearchOptions.bSortAscending = TRUE;
		}
	}

	UpdateResults();
}

void WxDlgGenericBrowserSearch::OnItemActivated(wxListEvent& In)
{
	TArray<UObject*> Objects;
	Objects.AddItem( (UObject*) In.GetItem().GetData() );

	WxGenericBrowser* GenericBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
	if ( GenericBrowser )
	{
		// Make sure the window is visible.  The window needs to be visible *before*
		// the browser is sync'd to objects so that redraws actually happen!
		GUnrealEd->GetBrowserManager()->ShowWindow( GenericBrowser->GetDockID(), TRUE );

		// Sync.
		GenericBrowser->SyncToObjects( Objects );
	}
}

/**
 * Updates the contents of the results list.
 */
void WxDlgGenericBrowserSearch::UpdateResults()
{
	ResultsList->Freeze();

	// Remove all existing items from the tree
	ResultsList->DeleteAllItems();

	// Get the text to search with.  If it's empty, leave.
	FString SearchText = (const TCHAR *)SearchForEdit->GetValue();
	if( !SearchText.Len() )
	{
		ResultsList->Thaw();
		return;
	}

	SearchText = SearchText.ToUpper();

	// We the current filter to determine which results are applicable to our search, if no generic browser is available,
	// we just use the list of all possible types.
	UGenericBrowserType* CurrentFilter;
	WxGenericBrowser* GenericBrowserPtr = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
	if( GenericBrowserPtr )
	{
		CurrentFilter = GenericBrowserPtr->LeftContainer->CurrentResourceType;
	}
	else
	{
		CurrentFilter = ConstructObject<UGenericBrowserType>( UGenericBrowserType_All::StaticClass() );
		CurrentFilter->Init();
	}
	

	FString ObjName;
	UBOOL bFoundItem;
	LONG idx;

	// Loop through all objects in the system, adding any ones that match our criteria into the list control.
	for( TObjectIterator<UObject> It; It; ++It )
	{
		ObjName = It->GetName();
		bFoundItem = 0;

		if( CurrentFilter->Supports( *It ) )
		{
			if( StartsWithRadio->GetValue() )
			{
				if( ObjName.ToUpper().StartsWith( SearchText ) )
				{
					bFoundItem = 1;
				}
			}
			else
			{
				if( ObjName.ToUpper().InStr( SearchText ) != INDEX_NONE )
				{
					bFoundItem = 1;
				}
			}

			if( bFoundItem )
			{
				idx = ResultsList->InsertItem( 0, *ObjName );
				const FString GroupName = It->GetFullGroupName(1);
				if( GroupName.Len() )
				{
					ResultsList->SetItem( idx, 1, *FString::Printf( TEXT("%s.%s"), *It->GetOutermost()->GetName(), *GroupName ) );
				}
				else
				{
					ResultsList->SetItem( idx, 1, *FString::Printf( TEXT("%s"), *It->GetOutermost()->GetName() ) );
				}

				UObject* obj = *It;
				const LONG Data = (LONG)obj;
				ResultsList->SetItemData( idx, Data );
			}
		}
	}

	// Sort based on the current options.
	ResultsList->SortItems( ResultsListSort, (LONG)&SearchOptions );

	// Misc/clean up.
	ResultsList->Thaw();
	ResultsLabel->SetLabel( *FString::Printf( LocalizeSecure(LocalizeUnrealEd("F_ObjectsFound"), ResultsList->GetItemCount()) ) );
}
