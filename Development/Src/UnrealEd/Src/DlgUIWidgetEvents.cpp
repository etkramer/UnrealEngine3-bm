/*=============================================================================
	DlgUIWidgetEvents.h:	Dialog that lets the user enable/disable UI event key aliases for a paticular widget.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"

#include "EngineUIPrivateClasses.h"
#include "UnrealEdPrivateClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

#include "UnObjectEditor.h"
#include "UnUIEditor.h"
#include "DlgUIEventKeyBindings.h"
#include "DlgUIWidgetEvents.h"

#include "ScopedTransaction.h"
#include "ScopedObjectStateChange.h"

BEGIN_EVENT_TABLE(WxDlgUIWidgetEvents, wxDialog)
	EVT_BUTTON(wxID_OK, WxDlgUIWidgetEvents::OnOK)
	EVT_BUTTON(IDB_UI_MODIFYEVENTDEFAULTS, WxDlgUIWidgetEvents::OnModifyDefaults )
END_EVENT_TABLE()

WxDlgUIWidgetEvents::WxDlgUIWidgetEvents(wxWindow* InParent, UUIScreenObject* InWidget) : 
wxDialog(InParent, wxID_ANY, *LocalizeUI("DlgUIWidgetEvents_Title"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
Widget(InWidget)
{
	check(Widget);

	wxSizer* MainSizer = new wxBoxSizer(wxHORIZONTAL);
	{
		// Event List
		wxBoxSizer* VSizer = new wxBoxSizer(wxVERTICAL);
		{
			// Current Widget
			FString StaticTextString = FString::Printf(LocalizeSecure(LocalizeUI("DlgUIWidgetEvents_WidgetF"), *Widget->GetWidgetPathName()));
			wxStaticText* StaticText = new wxStaticText(this, wxID_ANY, *StaticTextString);
			VSizer->Add(StaticText, 0, wxEXPAND | wxALL, 5);

			// Enabled Events Box
			wxStaticBoxSizer* ListSizer = new wxStaticBoxSizer(wxVERTICAL, this, *LocalizeUI("DlgUIWidgetEvents_EnabledEvents"));
			{
				EventList = new wxCheckListBox(this, ID_UI_EVENTLIST);
				ListSizer->Add(EventList, 1, wxEXPAND | wxALL, 5);
			}
			VSizer->Add(ListSizer, 1, wxEXPAND | wxALL, 5);
		}
		MainSizer->Add(VSizer, 1, wxEXPAND);

		// OK/Cancel Buttons
		wxSizer *ButtonSizer = new wxBoxSizer(wxVERTICAL);
		{
			wxButton* ButtonOK = new wxButton(this, wxID_OK, *LocalizeUnrealEd("OK"));
			ButtonOK->SetDefault();

			ButtonSizer->Add(ButtonOK, 0, wxEXPAND | wxALL, 5);

			wxButton* ButtonCancel = new wxButton(this, wxID_CANCEL, *LocalizeUnrealEd("Cancel"));
			ButtonSizer->Add(ButtonCancel, 0, wxEXPAND | wxALL, 5);

			wxButton* ButtonModifyDefaults = new wxButton(this, IDB_UI_MODIFYEVENTDEFAULTS, *LocalizeUI("DlgUIWidgetEvents_ModifyEventDefaults"));
			ButtonSizer->Add(ButtonModifyDefaults, 0, wxEXPAND | wxALL, 5);
		}
		MainSizer->Add(ButtonSizer, 0, wxALL, 5);
	}
	SetSizer(MainSizer);

	// Generates a list of supported events, and checks them if they are enabled.
	GenerateEventList();

	FWindowUtil::LoadPosSize(TEXT("DlgUIWidgetEvents"), this, -1, -1, 600, 300);
}

WxDlgUIWidgetEvents::~WxDlgUIWidgetEvents()
{
	FWindowUtil::SavePosSize(TEXT("DlgUIWidgetEvents"), this);
}

/** Generates the list of all of the event aliases for the widget. */
void WxDlgUIWidgetEvents::GenerateEventList()
{
	// Add all of the widget's event aliases to the list and uncheck any items
	// that are in the disabled event aliases array.
	EventList->Freeze();
	{
		TArray<FName> EventAliases;

		Widget->GetSupportedUIActionKeyNames(EventAliases);

		for(INT AliasIdx = 0; AliasIdx < EventAliases.Num(); AliasIdx++)
		{
			const FName &Alias = EventAliases(AliasIdx);
			INT ItemIdx = EventList->Append(*Alias.ToString());

			if(Widget->EventProvider->DisabledEventAliases.ContainsItem(Alias) == TRUE)
			{
				EventList->Check(ItemIdx, FALSE);
			}
			else
			{
				EventList->Check(ItemIdx, TRUE);
			}
		}
	}
	EventList->Thaw();
}

/** Callback for when the user clicks the OK button. */
void WxDlgUIWidgetEvents::OnOK( wxCommandEvent& Event )
{
	checkSlow(Widget);
	checkSlow(Widget->EventProvider);

	UBOOL bActiveEventsModified=FALSE;
	const INT TotalInputEvents = EventList->GetCount();

	// first, determine whether we have actually changed anything
	// Loop through all event aliases and add any unchecked items to the disabled event aliases array.
	for(INT AliasIdx = 0; AliasIdx < TotalInputEvents; AliasIdx++)
	{
		FName EventName = FName(EventList->GetString(AliasIdx).c_str());
		const UBOOL bEventPreviouslyDisabled = Widget->EventProvider->DisabledEventAliases.ContainsItem(EventName);
		const UBOOL bEventCurrentlyDisabled = !EventList->IsChecked(AliasIdx);
		if ( bEventCurrentlyDisabled != bEventPreviouslyDisabled )
		{
			bActiveEventsModified = TRUE;
			break;
		}
	}

	if ( bActiveEventsModified )
	{
		FScopedTransaction Transaction(*LocalizeUI(TEXT("TransEditModifyDisabledInputEvents")));
		FScopedObjectStateChange InputKeyChangeNotification(Widget->EventProvider);

		Widget->EventProvider->Modify();
		Widget->EventProvider->DisabledEventAliases.Empty();

		// Loop through all event aliases and add any unchecked items to the disabled event aliases array.
		for(INT AliasIdx = 0; AliasIdx < TotalInputEvents; AliasIdx++)
		{
			if(EventList->IsChecked(AliasIdx) == FALSE)
			{
				const wxString ItemText = EventList->GetString(AliasIdx);
				INT ArrayIdx = Widget->EventProvider->DisabledEventAliases.AddZeroed(1);
				Widget->EventProvider->DisabledEventAliases(ArrayIdx) = FName(ItemText);
			}
		}

		// Send a notification that we changed the widget UI key bindings.
		GCallbackEvent->Send(CALLBACK_UIEditor_WidgetUIKeyBindingChanged);

		EndModal(wxID_OK);
	}
	else
	{
		EndModal(wxID_CANCEL);
	}
}

/** Shows the bind default event keys dialog. */
void WxDlgUIWidgetEvents::OnModifyDefaults( wxCommandEvent &Event )
{
	WxDlgUIEventKeyBindings Dlg(this);
	Dlg.ShowModal();
}








