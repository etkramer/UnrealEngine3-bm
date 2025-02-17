/*=============================================================================
	DlgMapCheck.cpp: UnrealEd dialog for displaying map errors.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "DlgMapCheck.h"
#include "EngineSequenceClasses.h"
#include "BusyCursor.h"
#include "ScopedTransaction.h"

BEGIN_EVENT_TABLE(WxDlgMapCheck, WxTrackableDialog)
	EVT_BUTTON(wxID_CANCEL,						WxDlgMapCheck::OnClose)
	EVT_BUTTON(IDPB_REFRESH,					WxDlgMapCheck::OnRefresh)
	EVT_BUTTON(IDPB_GOTOACTOR,					WxDlgMapCheck::OnGoTo)
	EVT_BUTTON(IDPB_DELETEACTOR,				WxDlgMapCheck::OnDelete)
	EVT_BUTTON(IDPB_DELETEALLACTORS,			WxDlgMapCheck::OnDeleteAll)
	EVT_BUTTON(IDPB_SHOWHELPPAGE,				WxDlgMapCheck::OnShowHelpPage)
	EVT_LIST_ITEM_ACTIVATED(IDLC_ERRORWARNING,	WxDlgMapCheck::OnItemActivated)

	EVT_UPDATE_UI(IDPB_GOTOACTOR,				WxDlgMapCheck::OnUpdateUI)
	EVT_UPDATE_UI(IDPB_DELETEACTOR,				WxDlgMapCheck::OnUpdateUI)
	EVT_UPDATE_UI(IDPB_SHOWHELPPAGE,			WxDlgMapCheck::OnUpdateUI)
END_EVENT_TABLE()

WxDlgMapCheck::WxDlgMapCheck(wxWindow* InParent) : 
WxTrackableDialog(InParent, wxID_ANY, (wxString)*LocalizeUnrealEd("MapCheck"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	wxBoxSizer* MainSizer = new wxBoxSizer(wxHORIZONTAL);
	{
		// Warning List
		wxBoxSizer* ListSizer = new wxBoxSizer(wxVERTICAL);
		{
			ErrorWarningList = new wxListCtrl( this, IDLC_ERRORWARNING, wxDefaultPosition, wxSize(350, 200), wxLC_REPORT );
			ListSizer->Add(ErrorWarningList, 1, wxGROW|wxALL, 5);
		}
		MainSizer->Add(ListSizer, 1, wxGROW|wxALL, 5);		

		// Add buttons
		wxBoxSizer* ButtonSizer = new wxBoxSizer(wxVERTICAL);
		{
			{
				wxButton* CloseButton = new wxButton( this, wxID_OK, *LocalizeUnrealEd("Close"), wxDefaultPosition, wxDefaultSize, 0 );
				ButtonSizer->Add(CloseButton, 0, wxEXPAND|wxALL, 5);

				wxButton* RefreshButton = new wxButton( this, IDPB_REFRESH, *LocalizeUnrealEd("RefreshF5"), wxDefaultPosition, wxDefaultSize, 0 );
				ButtonSizer->Add(RefreshButton, 0, wxEXPAND|wxALL, 5);
			}


			ButtonSizer->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

			{
				wxButton* GotoButton = new wxButton( this, IDPB_GOTOACTOR, *LocalizeUnrealEd("GoTo"), wxDefaultPosition, wxDefaultSize, 0 );
				ButtonSizer->Add(GotoButton, 0, wxEXPAND|wxALL, 5);

				wxButton* DeleteButton = new wxButton( this, IDPB_DELETEACTOR, *LocalizeUnrealEd("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
				ButtonSizer->Add(DeleteButton, 0, wxEXPAND|wxALL, 5);

				wxButton* DeleteAllButton = new wxButton( this, IDPB_DELETEALLACTORS, *LocalizeUnrealEd("DeleteAll"), wxDefaultPosition, wxDefaultSize, 0 );
				ButtonSizer->Add(DeleteAllButton, 0, wxEXPAND|wxALL, 5);
			}

			ButtonSizer->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

			wxButton* HelpButton = new wxButton(this, IDPB_SHOWHELPPAGE, *LocalizeUnrealEd("UDNWarningErrorHelpF1"));
			ButtonSizer->Add(HelpButton, 0, wxEXPAND|wxALL, 5);
		}
		MainSizer->Add(ButtonSizer, 0, wxALIGN_TOP|wxALL, 5);

	}
	SetSizer(MainSizer);

	// Set an accelerator table to handle hotkey presses for the window.
	wxAcceleratorEntry Entries[3];
	Entries[0].Set(wxACCEL_NORMAL,  WXK_F5,			IDPB_REFRESH);
	Entries[1].Set(wxACCEL_NORMAL,  WXK_DELETE,		IDPB_DELETEACTOR);
	Entries[2].Set(wxACCEL_NORMAL,  WXK_F1,			IDPB_SHOWHELPPAGE);

	wxAcceleratorTable AcceleratorTable(3, Entries);
	SetAcceleratorTable(AcceleratorTable);

	// Columns are, from left to right: Level, Actor, Message.
	ErrorWarningList->InsertColumn( 0, *LocalizeUnrealEd("Message"), wxLIST_FORMAT_LEFT, 700 );
	ErrorWarningList->InsertColumn( 0, *LocalizeUnrealEd("Actor"), wxLIST_FORMAT_LEFT, 100 );
	ErrorWarningList->InsertColumn( 0, *LocalizeUnrealEd("Level"), wxLIST_FORMAT_LEFT, 100 );

	// Entries in the imagelist must match the ordering of the MCTYPE_* enum in Core.h
	ImageList = new wxImageList( 16, 15 );
	ImageList->Add( WxBitmap( "Error" ), wxColor( 192,192,192 ) );
	ImageList->Add( WxBitmap( "Warning" ), wxColor( 192,192,192 ) );
	ImageList->Add( WxBitmap( "Note" ), wxColor( 192,192,192 ) );
	ImageList->Add( WxBitmap( "Kismet" ), wxColor( 192,192,192 ) );
	ImageList->Add( WxBitmap( "Info" ), wxColor( 192,192,192 ) ); 
	ErrorWarningList->AssignImageList( ImageList, wxIMAGE_LIST_SMALL );

	// Load window position.
	FWindowUtil::LoadPosSize(TEXT("DlgMapCheck"), this, -1, -1, 800, 400);
}

WxDlgMapCheck::~WxDlgMapCheck()
{
	// Save window position.
	FWindowUtil::SavePosSize(TEXT("DlgMapCheck"), this);
}

/**
 * This function is called when the window has been selected from within the ctrl + tab dialog.
 */
void WxDlgMapCheck::OnSelected()
{
	if(!IsShown())
	{
		Show();
	}

	WxTrackableDialog::OnSelected();
}

bool WxDlgMapCheck::Show( bool show )
{
	if( show )
	{
		LoadListCtrl();
	}

	return wxDialog::Show(show);
}

/**
 * Shows the dialog only if there are messages to display.
 */
void WxDlgMapCheck::ShowConditionally()
{
	LoadListCtrl();

	if( ErrorWarningList->GetItemCount() > 0 )
	{
		Show( true );
	}
}

/** Clears out the list of messages appearing in the window. */
void WxDlgMapCheck::ClearMessageList()
{
	ErrorWarningList->DeleteAllItems();
	GErrorWarningInfoList.Empty();
	ReferencedActors.Empty();
}

/**
 * Freezes the message list.
 */
void WxDlgMapCheck::FreezeMessageList()
{
	ErrorWarningList->Freeze();
}

/**
 * Thaws the message list.
 */
void WxDlgMapCheck::ThawMessageList()
{
	ErrorWarningList->Thaw();
}

/**
 * Adds a message to the map check dialog, to be displayed when the dialog is shown.
 *
 * @param	InType					The type of message.
 * @param	InActor					Actor associated with the message; can be NULL.
 * @param	InMessage				The message to display.
 * @param	InRecommendedAction		The recommended course of action to take.
 * @param	InUDNPage				UDN Page to visit if the user needs more info on the warning.  This will send the user to https://udn.epicgames.com/Three/MapErrors#InUDNPage. 
 */
void WxDlgMapCheck::AddItem(MapCheckType InType, AActor* InActor, const TCHAR* InMessage, MapCheckAction InRecommendedAction, const TCHAR* InUDNPage)
{
	// Columns are, from left to right: Level, Actor, Message.

	FErrorWarningInfo ErrorWarningInfo;

	ErrorWarningInfo.RecommendedAction = InRecommendedAction;
	ErrorWarningInfo.UDNHelpString = InUDNPage;
	ErrorWarningInfo.Actor = InActor;
	ErrorWarningInfo.Type = InType;
	ErrorWarningInfo.Message = InMessage;
	ErrorWarningInfo.UDNPage = InUDNPage;

	GErrorWarningInfoList.AddItem( ErrorWarningInfo );
}

/**
* Loads the list control with the contents of the GErrorWarningInfoList array.
*/
void WxDlgMapCheck::LoadListCtrl()
{
	FString ActorName( TEXT("<None>") );
	FString LevelName( TEXT("<None>") );
	UBOOL bReferencedByKismet = FALSE;

	ErrorWarningList->DeleteAllItems();
	ReferencedActors.Empty();

	for( int x = 0 ; x < GErrorWarningInfoList.Num() ; ++x )
	{
		FErrorWarningInfo* ewi = &GErrorWarningInfoList( x );

		if ( ewi->Actor )
		{
			ULevel* Level = ewi->Actor->GetLevel();
			UPackage* LevelPackage = Level->GetOutermost();

			ActorName = ewi->Actor->GetName();
			LevelName = LevelPackage->GetName();

			if ( ewi->Type != MCTYPE_KISMET )
			{
				// Determine if the actor is referenced by a kismet sequence.
				USequence* RootSeq = GWorld->GetGameSequence( Level );
				if( RootSeq )
				{
					bReferencedByKismet = RootSeq->ReferencesObject( ewi->Actor );
				}
			}
		}

		const LONG Index = ErrorWarningList->InsertItem( x, *LevelName, bReferencedByKismet ? MCTYPE_KISMET : ewi->Type );
		ErrorWarningList->SetItem( Index, 1, *ActorName );
		ErrorWarningList->SetItem( Index, 2, *ewi->Message );
		ErrorWarningList->SetItemData( Index, (LONG)ewi->Actor );
		if ( ewi->Actor )
		{
			ReferencedActors.AddUniqueItem( ewi->Actor );
		}
	}
}

void WxDlgMapCheck::Serialize(FArchive& Ar)
{
	Ar << ReferencedActors;
}

/**
 * Removes all items from the map check dialog that pertain to the specified actor.
 *
 * @param	Actor	The actor to match when removing items.
 */
void WxDlgMapCheck::RemoveActorItems(AActor* Actor)
{
	// Remove actor from the referenced actor's array.
	ReferencedActors.RemoveItem(Actor);

	// Loop through all of the items in our warning list and remove them from the list view if their client data matches the actor we are removing,
	// make sure to iterate through the list backwards so we do not modify ids as we are removing items.
	INT ListCount = ErrorWarningList->GetItemCount();
	for(long ItemIdx=ListCount-1; ItemIdx>=0;ItemIdx--)
	{
		const AActor* ItemActor = (AActor*)ErrorWarningList->GetItemData(ItemIdx);
		if ( ItemActor == Actor )
		{
			GErrorWarningInfoList.Remove((INT)ItemIdx);
			ErrorWarningList->DeleteItem(ItemIdx);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Event handlers.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Event handler for when the close button is clicked on. */
void WxDlgMapCheck::OnClose(wxCommandEvent& In)
{
	Show(0);
}

/** Event handler for when the refresh button is clicked on. */
void WxDlgMapCheck::OnRefresh(wxCommandEvent& In)
{
	GEditor->Exec( TEXT("MAP CHECK") );
	LoadListCtrl();
}

/** Event handler for when the goto button is clicked on. */
void WxDlgMapCheck::OnGoTo(wxCommandEvent& In)
{
	const INT NumSelected = ErrorWarningList->GetSelectedItemCount();

	if( NumSelected > 0 )
	{
		const FScopedBusyCursor BusyCursor;
		TArray<AActor*> SelectedActors;
		long ItemIndex = ErrorWarningList->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
		while( ItemIndex != -1 )
		{
			AActor* Actor = (AActor*)ErrorWarningList->GetItemData(ItemIndex);
			if ( Actor )
			{
				SelectedActors.AddItem( Actor );
			}
			ItemIndex = ErrorWarningList->GetNextItem( ItemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
		}

		if ( SelectedActors.Num() > 0 )
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd("MapCheckGoto") );
			GEditor->SelectNone( FALSE, TRUE );
			for ( INT ActorIndex = 0 ; ActorIndex < SelectedActors.Num() ; ++ActorIndex )
			{
				AActor* Actor = SelectedActors(ActorIndex);
				GEditor->SelectActor( Actor, TRUE, NULL, FALSE, TRUE );
			}
			GEditor->NoteSelectionChange();
			GEditor->MoveViewportCamerasToActor( SelectedActors, FALSE );
		}
	}
}

/** Event handler for when the delete button is clicked on. */
void WxDlgMapCheck::OnDelete(wxCommandEvent& In)
{
	const FScopedBusyCursor BusyCursor;
	GEditor->Exec(TEXT("ACTOR SELECT NONE"));

	long ItemIndex = ErrorWarningList->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	TArray<AActor*> RemoveList;

	while( ItemIndex != -1 )
	{
		AActor* Actor = (AActor*)ErrorWarningList->GetItemData(ItemIndex);
		if ( Actor )
		{
			GEditor->SelectActor( Actor, TRUE, NULL, FALSE, TRUE );

			if(Actor != NULL)
			{
				RemoveList.AddUniqueItem(Actor);
			}
		}
		ItemIndex = ErrorWarningList->GetNextItem( ItemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	}

	GEditor->Exec( TEXT("DELETE") );
	
	// Loop through all of the actors we deleted and remove any items that reference it.
	for(INT RemoveIdx=0; RemoveIdx < RemoveList.Num(); RemoveIdx++)
	{
		// only remove the item if the delete was successful
		AActor* Actor = RemoveList(RemoveIdx);
		if ( Actor->IsPendingKill() )
		{
			RemoveActorItems( Actor );
		}
	}
}

/** Event handler for when the "delete all" button is clicked on. */
void WxDlgMapCheck::OnDeleteAll(wxCommandEvent& In)
{
	const FScopedBusyCursor BusyCursor;
	GEditor->Exec(TEXT("ACTOR SELECT NONE"));

	TArray<AActor*> RemoveList;
	long ItemIndex = ErrorWarningList->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE );
	while( ItemIndex != -1 )
	{
		AActor* Actor = (AActor*)ErrorWarningList->GetItemData(ItemIndex);
		if ( Actor )
		{
			const MapCheckAction RecommendedAction = GErrorWarningInfoList(ItemIndex).RecommendedAction;
			if ( RecommendedAction == MCACTION_DELETE )
			{
				GEditor->SelectActor( Actor, TRUE, NULL, FALSE, TRUE );

				if(Actor != NULL)
				{
					RemoveList.AddUniqueItem(Actor);
				}
			}
		}
		ItemIndex = ErrorWarningList->GetNextItem( ItemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE );
	}

	GEditor->Exec( TEXT("DELETE") );

	// Loop through all of the actors we deleted and remove any items that reference it.
	for(INT RemoveIdx=0; RemoveIdx < RemoveList.Num(); RemoveIdx++)
	{
		// only remove the item if the delete was successful
		AActor* Actor = RemoveList(RemoveIdx);
		if ( Actor->IsPendingKill() )
		{
			RemoveActorItems( Actor );
		}
	}
}

/** Event handler for when a message is clicked on. */
void WxDlgMapCheck::OnItemActivated(wxListEvent& In)
{
	const long ItemIndex	= In.GetIndex();
	UObject* Obj			= (UObject*)ErrorWarningList->GetItemData(ItemIndex);
	AActor* Actor			= Cast<AActor>( Obj );
	if ( Actor )
	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd("MapCheckGoto") );
		GEditor->SelectNone( TRUE, TRUE );
		GEditor->SelectActor( Actor, TRUE, NULL, TRUE, TRUE );
		GEditor->MoveViewportCamerasToActor( *Actor, FALSE );
	}
}

/** Event handler for when the "Show Help" button is clicked on. */
void WxDlgMapCheck::OnShowHelpPage(wxCommandEvent& In)
{
#if SHIPPING_PC_GAME
	// End users don't have access to the secure parts of UDN; send them to the UDN homepage.
	wxLaunchDefaultBrowser(TEXT("http://udn.epicgames.com/Main/WebHome.html"));
#else
	// Send developers, who have access to the secure parts of UDN, to the context-specific page.
	const FString BaseURLString = "https://udn.epicgames.com/Three/MapErrors";

	// Loop through all selected items and launch browser pages for each item.
	long ItemIndex = ErrorWarningList->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	while( ItemIndex != -1 )
	{
		FErrorWarningInfo &Info = GErrorWarningInfoList(ItemIndex);
		
		if(Info.UDNHelpString.Len())
		{
			wxLaunchDefaultBrowser(*FString::Printf(TEXT("%s#%s"), *BaseURLString, *Info.UDNHelpString));
		}
		else
		{
			wxLaunchDefaultBrowser(*BaseURLString);
		}

		ItemIndex = ErrorWarningList->GetNextItem( ItemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	}
#endif
}

/** Event handler for when wx wants to update UI elements. */
void WxDlgMapCheck::OnUpdateUI(wxUpdateUIEvent& In)
{
	switch(In.GetId())
	{
	case IDPB_SHOWHELPPAGE:case IDPB_GOTOACTOR: case IDPB_DELETEACTOR:
		{
			const UBOOL bItemSelected = ErrorWarningList->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED ) != -1;
			In.Enable(bItemSelected == TRUE);
		}
		break;
	}
}



