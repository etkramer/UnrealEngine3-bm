/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "DlgActorSearch.h"
#include "ScopedTransaction.h"
#include "BusyCursor.h"

namespace
{
	enum EActorSearchFields
	{
		ASF_Name,
		ASF_Level,
		ASF_Group,
		ASF_PathName,
		ASF_Tag
	};
}

BEGIN_EVENT_TABLE( WxDlgActorSearch, wxDialog )
	EVT_TIMER( ACTORSEARCH_TIMER_ID, WxDlgActorSearch::OnTimer )
END_EVENT_TABLE()

WxDlgActorSearch::WxDlgActorSearch( wxWindow* InParent )
{
	const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, InParent, TEXT("ID_DLG_ACTOR_SEARCH") );
	check( bSuccess );

	SearchForEdit = (wxTextCtrl*)FindWindow( XRCID( "ID_SEARCH_FOR" ) );
	check( SearchForEdit != NULL );
	StartsWithRadio = (wxRadioButton*)FindWindow( XRCID( "ID_STARTING_WITH" ) );
	check( StartsWithRadio != NULL );
	ResultsList = (wxListCtrl*)FindWindow( XRCID( "ID_RESULTS" ) );
	check( ResultsList != NULL );
	ResultsLabel = (wxStaticText*)FindWindow( XRCID( "ID_RESULTS_TEXT" ) );
	check( ResultsLabel != NULL );
	InsideOfCombo = (wxComboBox*)FindWindow( XRCID( "IDCB_INSIDEOF" ) );
	check( InsideOfCombo != NULL );

	InsideOfCombo->Append( *LocalizeUnrealEd("Name") );
	InsideOfCombo->Append( *LocalizeUnrealEd("Level") );
	InsideOfCombo->Append( *LocalizeUnrealEd("Group") );
	InsideOfCombo->Append( *LocalizeUnrealEd("PathName") );
	InsideOfCombo->Append( *LocalizeUnrealEd("Tag") );
	InsideOfCombo->SetSelection( 0 );

	StartsWithRadio->SetValue( 1 );

	ResultsList->InsertColumn( ASF_Name, *LocalizeUnrealEd("Name"), wxLIST_FORMAT_LEFT, 128 );
	ResultsList->InsertColumn( ASF_Level, *LocalizeUnrealEd("Level"), wxLIST_FORMAT_LEFT, 128 );
	ResultsList->InsertColumn( ASF_Group, *LocalizeUnrealEd("Group"), wxLIST_FORMAT_LEFT, 128 );
	ResultsList->InsertColumn( ASF_PathName, *LocalizeUnrealEd("PathName"), wxLIST_FORMAT_LEFT, 300 );
	ResultsList->InsertColumn( ASF_Tag, *LocalizeUnrealEd("Tag"), wxLIST_FORMAT_LEFT, 128 );

	ADDEVENTHANDLER( XRCID("ID_SEARCH_FOR"), wxEVT_COMMAND_TEXT_UPDATED, &WxDlgActorSearch::OnSearchTextChanged );
	ADDEVENTHANDLER( XRCID("ID_STARTING_WITH"), wxEVT_COMMAND_RADIOBUTTON_SELECTED, &WxDlgActorSearch::OnSearchTextChanged );
	ADDEVENTHANDLER( XRCID("ID_CONTAINING"), wxEVT_COMMAND_RADIOBUTTON_SELECTED, &WxDlgActorSearch::OnSearchTextChanged );
	wxEvtHandler* eh = GetEventHandler();
	eh->Connect( XRCID("ID_RESULTS"), wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler(WxDlgActorSearch::OnColumnClicked) );
	eh->Connect( XRCID("ID_RESULTS"), wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler(WxDlgActorSearch::OnItemActivated) );
	ADDEVENTHANDLER( XRCID("IDPB_GOTOACTOR"), wxEVT_COMMAND_BUTTON_CLICKED, &WxDlgActorSearch::OnGoTo )
	ADDEVENTHANDLER( XRCID("IDPB_DELETEACTOR"), wxEVT_COMMAND_BUTTON_CLICKED, &WxDlgActorSearch::OnDelete )
	ADDEVENTHANDLER( XRCID("IDPB_PROPERTIES"), wxEVT_COMMAND_BUTTON_CLICKED, &WxDlgActorSearch::OnProperties )
	eh->Connect( this->GetId(), wxEVT_ACTIVATE, wxActivateEventHandler(WxDlgActorSearch::OnActivate) );

	FWindowUtil::LoadPosSize( TEXT("DlgActorSearch"), this, 256, 256, 450, 400 );

	UpdateResults();

	FLocalizeWindow( this );

	_updateResultsTimer.SetOwner(this,ACTORSEARCH_TIMER_ID);
}

WxDlgActorSearch::~WxDlgActorSearch()
{
	FWindowUtil::SavePosSize( TEXT("DlgActorSearch"), this );
	_updateResultsTimer.Stop();
}

void WxDlgActorSearch::OnTimer( wxTimerEvent& event )
{
	UpdateResults();
}

void WxDlgActorSearch::OnSearchTextChanged( wxCommandEvent& In )
{
	// if the timer was already running, reset the timer and start over
	if(_updateResultsTimer.IsRunning())
	{
		_updateResultsTimer.Stop();		
	}
	_updateResultsTimer.Start(SEARCH_TEXT_UPDATE_DELAY,wxTIMER_ONE_SHOT);
}

void WxDlgActorSearch::OnColumnClicked( wxListEvent& In )
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

void WxDlgActorSearch::OnItemActivated( wxListEvent& In )
{
	wxCommandEvent Event;
	OnGoTo( Event );
}

/**
 * Empties the search list, releases actor references, etc.
 */
void WxDlgActorSearch::Clear()
{
	ResultsList->DeleteAllItems();
	ReferencedActors.Empty();
	ResultsLabel->SetLabel( *FString::Printf( LocalizeSecure(LocalizeUnrealEd("F_ObjectsFound"), 0) ) );
}

/**
 * Global temp storage mapping actors to the strings that should be used for sorting.
 * Non-NULL only during an actor search update, while actors are being sorted.
 */
static TMap<AActor*, FString> *GActorString = NULL;

/** Used by WxActorResultsListSort.  A global flag indicating whether actor search results should be sorted ascending or descending. */
static UBOOL GSortAscending = FALSE;

/**
 * Orders items in the actor search dialog's list view.
 */
static int wxCALLBACK WxActorResultsListSort(long InItem1, long InItem2, long InSortData)
{
	AActor* A = (AActor*)InItem1;
	AActor* B = (AActor*)InItem2;

	const FString* StringA = GActorString->Find( A );
	const FString* StringB = GActorString->Find( B );

	return appStricmp( *(*StringA), *(*StringB) ) * (GSortAscending ? 1 : -1);
}

/**
 * Updates the contents of the results list.
 */
void WxDlgActorSearch::UpdateResults()
{
	const FScopedBusyCursor BusyCursor;
	ResultsList->Freeze();
	{
		// Empties the search list, releases actor references, etc.
		Clear();

		// Get the text to search with.  If it's empty, leave.

		FString SearchText = (const TCHAR*)SearchForEdit->GetValue();
		SearchText = SearchText.ToUpper();

		// Looks through all actors and see which ones meet the search criteria
		const INT SearchFields = InsideOfCombo->GetSelection();

		TMap<AActor*, FString> ActorStringMap;
		for( FActorIterator It; It; ++It )
		{
			AActor* Actor = *It;
			// skip transient actors (path building scout, etc)
			if ( !Actor->HasAnyFlags(RF_Transient) )
			{
				UBOOL bFoundItem = FALSE;

				FString CompString;
				switch( SearchFields )
				{
				case ASF_Name:		// Name
					CompString = Actor->GetName();
					break;
				case ASF_Level:
					CompString = Actor->GetOutermost()->GetName();
					break;
				case ASF_Tag:		// Tag
					CompString = Actor->Tag.ToString();
					break;
				case ASF_PathName:	// Path Name
					CompString = Actor->GetPathName();
					break;
				case ASF_Group:		// Group
					CompString = Actor->Group.ToString();
					break;
				default:
					break;
				}

				if( SearchText.Len() == 0 )
				{
					// If there is no search text, show all actors.
					bFoundItem = TRUE;
				}
				else
				{
					// Starts with/contains.
					if( StartsWithRadio->GetValue() )
					{
						if( CompString.ToUpper().StartsWith( SearchText ) )
						{
							bFoundItem = TRUE;
						}
					}
					else
					{
						if( CompString.ToUpper().InStr( SearchText ) != INDEX_NONE )
						{
							bFoundItem = TRUE;
						}
					}
				}

				// If the actor matches the criteria, add it to the list.
				if( bFoundItem )
				{
					ActorStringMap.Set( Actor, *CompString );
					const LONG Idx = ResultsList->InsertItem( 0, *Actor->GetName() );
					ResultsList->SetItem( Idx, ASF_Level, *Actor->GetOutermost()->GetName() );
					ResultsList->SetItem( Idx, ASF_PathName, *Actor->GetPathName() );
					ResultsList->SetItem( Idx, ASF_Group, *Actor->Group.ToString() );
					ResultsList->SetItem( Idx, ASF_Tag, *Actor->Tag.ToString() );
					ResultsList->SetItemData( Idx, (LONG)(void*)Actor );
					ReferencedActors.AddItem( Actor );
				}
			}
		}

		// Sort based on the current options
		GActorString = &ActorStringMap;
		GSortAscending = SearchOptions.bSortAscending;
		ResultsList->SortItems( WxActorResultsListSort, (LONG)&SearchOptions );
		GActorString = NULL;
	}
	ResultsList->Thaw();

	ResultsLabel->SetLabel( *FString::Printf( LocalizeSecure(LocalizeUnrealEd("F_ObjectsFound"), ResultsList->GetItemCount()) ) );
}

/** Serializes object references to the specified archive. */
void WxDlgActorSearch::Serialize(FArchive& Ar)
{
	Ar << ReferencedActors;
}

void WxDlgActorSearch::OnGoTo( wxCommandEvent& In )
{
	long ItemIndex = ResultsList->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	AActor* Actor = (AActor*)(ResultsList->GetItemData( ItemIndex ));

	const FScopedTransaction Transaction( *LocalizeUnrealEd("ActorSearchGoto") );

	// Deselect all actors, then focus on the first actor in the list, and simply select the rest.
	GEditor->Exec(TEXT("ACTOR SELECT NONE"));

	while( ItemIndex != -1 )
	{
		Actor = (AActor*)(ResultsList->GetItemData( ItemIndex ));
		if ( Actor )
		{
			GEditor->SelectActor( Actor, TRUE, NULL, FALSE, TRUE );
		}

		// Advance list iterator.
		ItemIndex = ResultsList->GetNextItem(ItemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	}

	GEditor->NoteSelectionChange();
	GEditor->Exec( TEXT("CAMERA ALIGN") );
}

void WxDlgActorSearch::OnDelete( wxCommandEvent& In )
{
	GEditor->Exec(TEXT("ACTOR SELECT NONE"));

	long ItemIndex = ResultsList->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );

	while( ItemIndex != -1 )
	{
		AActor* Actor = (AActor*)ResultsList->GetItemData( ItemIndex );
		if ( Actor )
		{
			GEditor->SelectActor( Actor, TRUE, NULL, FALSE, TRUE );
		}
		ItemIndex = ResultsList->GetNextItem( ItemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	}

	GEditor->Exec( TEXT("DELETE") );
	UpdateResults();
}

void WxDlgActorSearch::OnProperties(wxCommandEvent& In )
{
	long ItemIndex = ResultsList->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	AActor* Actor = (AActor*)(ResultsList->GetItemData( ItemIndex ));

	const FScopedTransaction Transaction( *LocalizeUnrealEd("ActorSearchSelectActors") );

	// Deselect all actors, then select the actors that are selected in our list, then 
	GEditor->Exec(TEXT("ACTOR SELECT NONE"));

	while( ItemIndex != -1 )
	{
		Actor = (AActor*)(ResultsList->GetItemData( ItemIndex ));
		if ( Actor )
		{
			GEditor->SelectActor( Actor, TRUE, NULL, FALSE, TRUE );
		}

		// Advance list iterator.
		ItemIndex = ResultsList->GetNextItem(ItemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	}

	GEditor->NoteSelectionChange();
	GEditor->Exec( TEXT("EDCALLBACK ACTORPROPS") );
}

/** Refreshes the results list when the window gets focus. */
void WxDlgActorSearch::OnActivate(wxActivateEvent& In)
{
	const UBOOL bActivated = In.GetActive();
	if(bActivated == TRUE)
	{
		UpdateResults();
	}
}
