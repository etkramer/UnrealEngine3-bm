/*=============================================================================
	DlgUIEvent_MetaObject.cpp : Specialized dialog for the UIEvent_MetaObject sequence object.  Lets the user bind keys to the object, exposing them to kismet.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"

#include "EngineUIPrivateClasses.h"
#include "UnrealEdPrivateClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

#include "UnObjectEditor.h"
#include "UnLinkedObjEditor.h"
#include "Kismet.h"
#include "UnUIEditor.h"

#include "ScopedTransaction.h"
#include "ScopedObjectStateChange.h"

#include "DlgUIEvent_MetaObject.h"

// Callback for the wxListView sorting function.
namespace
{
	int wxCALLBACK WxDlgUIEvent_MetaObject_ListSort( long InItem1, long InItem2, long InSortData )
	{
		WxDlgUIEvent_MetaObject::FSortData* A = (WxDlgUIEvent_MetaObject::FSortData*)InItem1;
		WxDlgUIEvent_MetaObject::FSortData* B = (WxDlgUIEvent_MetaObject::FSortData*)InItem2;
		WxDlgUIEvent_MetaObject::FSortOptions* SortOptions = (WxDlgUIEvent_MetaObject::FSortOptions*)InSortData;

		FString CompA, CompB;

		// Generate a string to run the compares against for each object based on
		// the current column.

		switch( SortOptions->Column )
		{
		case WxDlgUIEvent_MetaObject::Field_Key:				
			CompA = A->KeyName.ToString();
			CompB = B->KeyName.ToString();
			break;

		case WxDlgUIEvent_MetaObject::Field_Event:		
			CompA = A->EventName.ToString();
			CompB = B->EventName.ToString();
			break;

		default:
			check(0);	// Invalid column!
			break;
		}

		// Start with a base string compare.
		INT Ret = appStricmp( *CompA, *CompB );

		// If we are sorting backwards, invert the string comparison result.

		if( !SortOptions->bSortAscending )
		{
			Ret *= -1;
		}

		return Ret;
	}
}



BEGIN_EVENT_TABLE(WxDlgUIEvent_MetaObject, wxDialog)
	EVT_BUTTON(IDB_UI_SHIFTRIGHT, WxDlgUIEvent_MetaObject::OnShiftRight)
	EVT_BUTTON(IDB_UI_SHIFTLEFT, WxDlgUIEvent_MetaObject::OnShiftLeft)
	EVT_BUTTON(wxID_OK, WxDlgUIEvent_MetaObject::OnOK)
	EVT_LIST_ITEM_ACTIVATED(ID_UI_AVAILABLELIST, WxDlgUIEvent_MetaObject::OnAvailableListActivated)
	EVT_LIST_ITEM_ACTIVATED(ID_UI_BOUNDLIST, WxDlgUIEvent_MetaObject::OnBoundListActivated)
	EVT_LIST_COL_CLICK(ID_UI_AVAILABLELIST, WxDlgUIEvent_MetaObject::OnClickListColumn)
	EVT_LIST_COL_CLICK(ID_UI_BOUNDLIST, WxDlgUIEvent_MetaObject::OnClickListColumn)
END_EVENT_TABLE()


WxDlgUIEvent_MetaObject::WxDlgUIEvent_MetaObject(const class WxKismet* InEditor, USequenceObject* InObject) : 
wxDialog((wxWindow*)InEditor, wxID_ANY, *LocalizeUI(TEXT("DlgUIEvent_MetaObject_Title")), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX | wxSYSTEM_MENU | wxRESIZE_BORDER ),
SeqObj(InObject),
Editor(InEditor)
{
	SetMinSize(wxSize(750,400));


	// Set default sort options.
	for(INT SortIdx = 0; SortIdx < 2; SortIdx++)
	{
		SortOptions[SortIdx].bSortAscending = TRUE;
		SortOptions[SortIdx].Column = 0;
	}

	// Create controls
	wxSizer *MainSizer = new wxBoxSizer(wxHORIZONTAL);
	{
		// Available List
		wxStaticBoxSizer* AvailableSizer = new wxStaticBoxSizer(wxVERTICAL, this, *LocalizeUI(TEXT("DlgUIEvent_MetaObject_AvailableKeys")));
		{
			ListAvailable = new WxListView(this, ID_UI_AVAILABLELIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
			AvailableSizer->Add(ListAvailable, 1, wxEXPAND | wxALL, 5);
		}
		MainSizer->Add(AvailableSizer, 1, wxEXPAND | wxALL, 5);

		// List shift buttons
		wxBoxSizer* ShiftButtonSizer = new wxBoxSizer(wxVERTICAL);
		{
			{
				wxSizer* HSizer = new wxBoxSizer(wxHORIZONTAL);
				wxButton* ButtonRight = new wxButton(this, IDB_UI_SHIFTRIGHT, TEXT(">>"));
				HSizer->Add(ButtonRight, 0, wxALIGN_CENTER);
				ShiftButtonSizer->Add(HSizer, 1, wxALIGN_CENTER);
			}

			ShiftButtonSizer->AddSpacer(10);

			{
				wxSizer* HSizer = new wxBoxSizer(wxHORIZONTAL);
				wxButton* ButtonLeft = new wxButton(this, IDB_UI_SHIFTLEFT, TEXT("<<"));
				HSizer->Add(ButtonLeft, 0, wxALIGN_CENTER);
				ShiftButtonSizer->Add(HSizer, 1, wxALIGN_CENTER);
			}
		}
		MainSizer->Add(ShiftButtonSizer, 0, wxEXPAND | wxALL, 5);

		// Bound List
		wxStaticBoxSizer* BoundSizer = new wxStaticBoxSizer(wxVERTICAL, this, *LocalizeUI(TEXT("DlgUIEvent_MetaObject_BoundKeys")));
		{
			ListBound = new WxListView(this, ID_UI_BOUNDLIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
			BoundSizer->Add(ListBound, 1, wxEXPAND | wxALL, 5);
		}
		MainSizer->Add(BoundSizer, 1, wxEXPAND | wxALL, 5);

		// OK/Cancel Buttons
		wxSizer *ButtonSizer = new wxBoxSizer(wxVERTICAL);
		{
			wxButton* ButtonOK = new wxButton(this, wxID_OK, *LocalizeUnrealEd("OK"));
			ButtonOK->SetDefault();

			ButtonSizer->Add(ButtonOK, 0, wxEXPAND | wxALL, 5);

			wxButton* ButtonCancel = new wxButton(this, wxID_CANCEL, *LocalizeUnrealEd("Cancel"));
			ButtonSizer->Add(ButtonCancel, 0, wxEXPAND | wxALL, 5);
		}
		MainSizer->Add(ButtonSizer, 0, wxALL, 5);
	}
	SetSizer(MainSizer);
	MainSizer->Fit(this);


	// Need to insert columns otherwise items will not show up.
	ListAvailable->InsertColumn(Field_Key, *LocalizeUI(TEXT("DlgUIEvent_MetaObject_Key")));
	ListAvailable->SetColumnWidth(Field_Key, 100);

	ListAvailable->InsertColumn(Field_Event, *LocalizeUI(TEXT("DlgUIEvent_MetaObject_Event")));
	ListAvailable->SetColumnWidth(Field_Event, 100);

	ListBound->InsertColumn(Field_Key, *LocalizeUI(TEXT("DlgUIEvent_MetaObject_Key")));
	ListBound->SetColumnWidth(Field_Key, 100);

	ListBound->InsertColumn(Field_Event, *LocalizeUI(TEXT("DlgUIEvent_MetaObject_Event")));
	ListBound->SetColumnWidth(Field_Event, 100);



	// Add all of the sequence object's output links to the bound list and remove them from the available list.
	UUIEvent_MetaObject* MetaObject = Cast<UUIEvent_MetaObject>(SeqObj);
	if ( MetaObject )
	{
		UUIState* OwnerState = MetaObject->GetOwnerState();

		const INT NumLinks = OwnerState->StateInputActions.Num();
		for(INT LinkIdx = 0; LinkIdx < NumLinks; LinkIdx++)
		{
			FInputKeyAction& Action = OwnerState->StateInputActions(LinkIdx);

			// Add the item to the bound list.
			long ItemIdx = ListBound->InsertItem(ListBound->GetItemCount(), *Action.InputKeyName.ToString());
			ListBound->SetItem(ItemIdx, Field_Event, *UUIRoot::GetInputEventText(Action.InputKeyState));
		}
	}

	// Add all available keys, this must be done AFTER we add bound keys so we do not add the same items to both lists.
	AddAvailableKeys();

	// initial sort
	if ( ListAvailable->GetItemCount() > 0 )
	{
		ListAvailable->SortItems(WxDlgUIEvent_MetaObject_ListSort, (LONG)&SortOptions[0]);
	}

	if ( ListBound->GetItemCount() > 0 )
	{
		ListBound->SortItems(WxDlgUIEvent_MetaObject_ListSort, (LONG)&SortOptions[1]);
	}
}

namespace
{
	/** Struct that holds info about the key name and which events are supported by it. */
	struct FKeyDefinition
	{
		FKeyDefinition(FName& Key, ESupportedInputEvents Events)
		: KeyName(Key), KeyEvents(Events)
	{

	}

	FName KeyName;
	ESupportedInputEvents KeyEvents;
	};

	// Note that this list is ordered the same as the list above, AND must end with a IE_MAX to signal the end of the list.
	static const EInputEvent KeyMapping_Keyboard [] =			{IE_Pressed, IE_Released, IE_Repeat, IE_MAX};
	static const EInputEvent KeyMapping_PressedReleased [] =	{IE_Pressed, IE_Released, IE_DoubleClick, IE_MAX};
	static const EInputEvent KeyMapping_PressedOnly [] =		{IE_Pressed, IE_MAX};
	static const EInputEvent KeyMapping_Axis [] =				{IE_Axis, IE_MAX};


	static const EInputEvent* KeyMappings [SIE_Number]= 
	{
		KeyMapping_Keyboard, KeyMapping_PressedReleased,
		KeyMapping_PressedOnly, KeyMapping_Axis
	};
}

void WxDlgUIEvent_MetaObject::AddAvailableKeys()
{
	TArray<FKeyDefinition> KeyList;

	// Do some preprocessor trickery to get the keys we need into a array.  
	{
#define DEFINE_KEY(Name, SupportedEvent) \
	KeyList.AddItem(FKeyDefinition(KEY_##Name, SupportedEvent));					

#include "UnKeys.h"

#undef DEFINE_KEY
	}

	// Count the number of array spaces we will need to allocate for all of the key actions.
	const INT NumKeys = KeyList.Num();
	INT NumActions = 0;

	for(INT KeyIdx = 0; KeyIdx < NumKeys; KeyIdx++)
	{
		const FKeyDefinition Key = KeyList(KeyIdx);
		const EInputEvent* SupportedEvents = KeyMappings[Key.KeyEvents];

		while ( *SupportedEvents++ < IE_MAX )
		{
			NumActions++;
		}
	}

	KeyActions.Add(NumActions);

	// Loop through the array and add items to the table, one for each event a key supports.  
	// Items are added only if the key isn't already bound.
	INT ActionIdx = 0;
	for( INT KeyIdx = 0; KeyIdx < NumKeys; KeyIdx++ )
	{
		const FKeyDefinition Key = KeyList(KeyIdx);
		const EInputEvent* SupportedEvents = KeyMappings[Key.KeyEvents];

		while( *SupportedEvents != IE_MAX )
		{
			// Add the key/event pair to a local array so we can sort the list (wx requirement).
			wxString EnumName = *UUIRoot::GetInputEventText(*SupportedEvents);

			// Creates a sort data item for this specific key/event pair.
			FSortData& Action = KeyActions(ActionIdx++);
			Action.KeyName = Key.KeyName;
			Action.EventName = FName(EnumName);

			// Loop through the bound list and make sure that the key isn't already there.
			UBOOL bAddItem = TRUE;

			long ListIdx = ListBound->FindItem(0, *Key.KeyName.ToString());

			while ( ListIdx != -1 )
			{
				// this is the right key name, but need to check that we have the right InputEvent state
				if(ListBound->GetColumnItemText(ListIdx, 1) == EnumName )
				{
					ListBound->SetItemData(ListIdx, (LONG)&Action);
					bAddItem = FALSE;
					break;
				}
				else
				{
					ListIdx = ListBound->FindItem(ListIdx + 1, *Key.KeyName.ToString());
				}
			}


			if( bAddItem )
			{
				long ItemIdx = ListAvailable->InsertItem(ListAvailable->GetItemCount(), *Key.KeyName.ToString());
				ListAvailable->SetItem(ItemIdx, Field_Event, EnumName);
				ListAvailable->SetItemData(ItemIdx, (LONG)&Action);
			}

			SupportedEvents++;
		}
	}
}

/** Moves selected items from the source list to the destlist. */
void WxDlgUIEvent_MetaObject::ListMove(WxListView* SourceList, WxListView* DestList)
{
	SourceList->Freeze();
	DestList->Freeze();
	{
		long SelectionIdx = SourceList->GetFirstSelected();

		// Add all of the items to the other list first, and build a list of items to remove.
		while(SelectionIdx != -1)
		{

			const wxString StateText = SourceList->GetColumnItemText(SelectionIdx, 1);
			const wxString SelectionText = SourceList->GetItemText(SelectionIdx);
			long ItemIdx = DestList->InsertItem(DestList->GetItemCount(), SelectionText);
			DestList->SetItem(ItemIdx, 1, StateText);
			DestList->SetItemData(ItemIdx, SourceList->GetItemData(SelectionIdx));

			SelectionIdx = SourceList->GetNextSelected(SelectionIdx);
		}

		// Remove all selected items from the listview.
		SelectionIdx = SourceList->GetFirstSelected();

		while(SelectionIdx != -1)
		{
			SourceList->DeleteItem(SelectionIdx);	
			SelectionIdx = SourceList->GetFirstSelected();
		}
	}
	SourceList->Thaw();
	DestList->Thaw();
}

/** Moves selected items from the bound list to the available list. */
void WxDlgUIEvent_MetaObject::OnShiftLeft(wxCommandEvent &Event)
{
	ListMove(ListBound, ListAvailable);
}

/** Moves selected items from the available list to the bound list. */
void WxDlgUIEvent_MetaObject::OnShiftRight(wxCommandEvent &Event)
{
	ListMove(ListAvailable, ListBound);
}

/** Moves selected items from the bound list to the available list. */
void WxDlgUIEvent_MetaObject::OnBoundListActivated(wxListEvent &Event)
{
	ListMove(ListBound, ListAvailable);
}

/** Moves selected items from the available list to the bound list. */
void WxDlgUIEvent_MetaObject::OnAvailableListActivated(wxListEvent &Event)
{
	ListMove(ListAvailable, ListBound);
}

/** Sorts the list based on which column was clicked. */
void WxDlgUIEvent_MetaObject::OnClickListColumn(wxListEvent &Event)
{
	WxListView* List = wxDynamicCast(Event.GetEventObject(), WxListView);

	INT SortIdx = 0;

	if(Event.GetId() == ID_UI_AVAILABLELIST)
	{
		SortIdx = 0;
	}
	else
	{
		SortIdx = 1;
	}

	// Make it so that if the user clicks on the same column twice, that it reverses the way the list is sorted.
	FSortOptions* Options = &SortOptions[SortIdx];

	if(Options->Column == Event.GetColumn())
	{
		Options->bSortAscending = !Options->bSortAscending;
	}
	else
	{
		Options->Column = Event.GetColumn();
	}

	List->SortItems(WxDlgUIEvent_MetaObject_ListSort, (LONG)Options);
}

/** When the user clicks the OK button, we create/remove output links for the sequence object based on the state of the lists. */
void WxDlgUIEvent_MetaObject::OnOK(wxCommandEvent &Event)
{
	UUIEvent_MetaObject* MetaObject = Cast<UUIEvent_MetaObject>(SeqObj);
	static UEnum* InputEventEnum = FindObject<UEnum>(UObject::StaticClass(), TEXT("EInputEvent"), TRUE);

	UBOOL bInputKeysModified = FALSE;
	if ( MetaObject  )
	{
		UUIState* CurrentState = MetaObject->GetOwnerState();
		if ( CurrentState != NULL )
		{
			FScopedTransaction Transaction(*LocalizeUI(TEXT("TransEditModifyInputEvents")));
			FScopedObjectStateChange StateChangeNotifier(CurrentState);

			// Save the changes made to the object's links to the state array before generating a new actions table.
			bInputKeysModified = MetaObject->SaveInputActionsToState();

			// Loop through all items in the list to create a new list of actions.  For each item in the list, we will try to see if an action already exists for that
			// key.  If it does, then just copy over the settings from that action instead of creating a default action.
			TArray<FInputKeyAction> NewActions;

			const INT NumActions = CurrentState->StateInputActions.Num();
			const INT NumListItems = ListBound->GetItemCount();

			if (NumListItems )
			{
				// Create space for our new links.
				NewActions.Empty(NumListItems);
				for(INT ListIdx = 0; ListIdx < NumListItems; ListIdx++)
				{
					FName KeyName(ListBound->GetItemText(ListIdx));

					// Get the enum for this list item.
					FName EnumName(ListBound->GetColumnItemText(ListIdx, 1));

					EInputEvent KeyState = (EInputEvent)InputEventEnum->FindEnumIndex(EnumName);

					new(NewActions) FInputKeyAction(KeyName, KeyState);
				}
			}

			// now remove all input keys that should no longer be in the state's list of supported keys
			for ( INT StateInputIndex = CurrentState->StateInputActions.Num() - 1; StateInputIndex >= 0; StateInputIndex-- )
			{
				FInputKeyAction& StateInputAction = CurrentState->StateInputActions(StateInputIndex);

				//@todo ronp - this relies on FInputKeyAction::operator== not checking the TriggeredOps array
				if ( !NewActions.ContainsItem(StateInputAction) )
				{
					bInputKeysModified = TRUE;
					CurrentState->RemoveInputAction(StateInputAction);
				}
			}

			// then add any input keys that are new
			for ( INT NewActionIndex = 0; NewActionIndex < NewActions.Num(); NewActionIndex++ )
			{
				FInputKeyAction& NewAction = NewActions(NewActionIndex);

				//@todo ronp - this relies on FInputKeyAction::operator== not checking the TriggeredOps array
				if ( !CurrentState->StateInputActions.ContainsItem(NewAction) )
				{
					bInputKeysModified = TRUE;
					CurrentState->AddInputAction(NewAction);
				}
			}

			// Set the metaobject's link array to our new list of links.
			MetaObject->RebuildOutputLinks();

			// if we didn't actually modify anything, cancel all propagation and transaction stuff
			if ( !bInputKeysModified )
			{
// 				InputKeyChangeNotification.CancelEdit();
				Transaction.Cancel();
			}
		}
	}

	if ( bInputKeysModified )
	{
		EndModal(wxID_OK);
	}
	else
	{
		EndModal(wxID_CANCEL);
	}
}

