/*=============================================================================
	DlgUIEventKeyBindings.cpp : This is a dialog that lets the user set defaults for which keys trigger built in UI Events for each widget type.  
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "FConfigCacheIni.h"
#include "EngineUIPrivateClasses.h"
#include "UnrealEdPrivateClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

#include "SourceControlIntegration.h"
#include "UnObjectEditor.h"
#include "UnLinkedObjEditor.h"
#include "Kismet.h"
#include "UnUIEditor.h"
#include "UnConsoleSupportContainer.h"

#include "DlgUIEventKeyBindings.h"

//@note ronp: if this is enabled, it prevents the user from bindings keys to non-recommended states, such as binding Enter to the enabled state.
// I think there are too many cases where you want to be able to do this, so for now it's disabled.
#define REMOVE_UNASSOCIATED_KEYS 0

const BYTE DefaultModifierFlags = (KEYMODIFIER_AltExcluded|KEYMODIFIER_CtrlExcluded|KEYMODIFIER_ShiftExcluded);

BEGIN_EVENT_TABLE(WxDlgChooseInputKey, wxDialog)
	EVT_LISTBOX(ID_UI_AVAILABLELIST, WxDlgChooseInputKey::OnInputKeySelected)
	EVT_KEY_DOWN( WxDlgChooseInputKey::OnInputKeyPressed )
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(WxRawInputKeyHandler,wxEvtHandler)
	EVT_KEY_DOWN(WxRawInputKeyHandler::OnInputKeyPressed)
	EVT_LEFT_DOWN(WxRawInputKeyHandler::OnMouseInput)
	EVT_MIDDLE_DOWN(WxRawInputKeyHandler::OnMouseInput)
	EVT_RIGHT_DOWN(WxRawInputKeyHandler::OnMouseInput)
	EVT_MOUSEWHEEL(WxRawInputKeyHandler::OnMouseInput)
END_EVENT_TABLE()

/** Constructor */
WxRawInputKeyHandler::WxRawInputKeyHandler( WxDlgChooseInputKey* inOwnerDialog )
: wxEvtHandler(), InputKeyDialog(inOwnerDialog)
{
}

/** called when the user presses/releases a key while the list is active */
void WxRawInputKeyHandler::OnInputKeyPressed( wxKeyEvent& Event )
{
	if ( InputKeyDialog != NULL )
	{
		InputKeyDialog->OnInputKeyPressed(Event);
	}
	else
	{
		Event.Skip();
	}
}

/**
 * Called when the user presses a mouse button or scrolls the mouse wheel while
 * the list is active
 */
void WxRawInputKeyHandler::OnMouseInput( wxMouseEvent& Event )
{
	if ( InputKeyDialog != NULL )
	{
		InputKeyDialog->OnMouseInput(Event);
	}
	else
	{
		Event.Skip();
	}
}


/** Constructor */
WxDlgChooseInputKey::WxDlgChooseInputKey( wxWindow* inParent )
: wxDialog(inParent, wxID_ANY, TEXT(""), wxDefaultPosition, wxDefaultSize
, wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxWANTS_CHARS)
, KeyList(NULL), SelectedKey(NAME_None), ListInputKeyHandler(NULL) 
{
	SetExtraStyle(wxWS_EX_BLOCK_EVENTS);

	// wxDialog's ctor always sets this flag, which causes the base wx code to trap Tab and Enter input keys
	// for navigation control.  We want these keys as well, so we unset this flag here.
	SetWindowStyleFlag(GetWindowStyleFlag()& ~wxTAB_TRAVERSAL);

	wxBoxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(MainSizer);

	KeyList = new wxListBox( this, ID_UI_AVAILABLELIST, wxDefaultPosition, wxDefaultSize,
		0, NULL, wxLB_SINGLE|wxLB_SORT|wxSUNKEN_BORDER|wxWANTS_CHARS );
	KeyList->SetToolTip(*LocalizeUI(TEXT("DlgUIEventKeyBindings_Tooltip_AvailableKeys")));
	MainSizer->Add(KeyList, 1, wxGROW|wxALL, 5);

	wxBoxSizer* ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	{
		wxButton* ButtonOK = new wxButton(this, wxID_OK, *LocalizeUnrealEd("&OK"));
		ButtonOK->SetDefault();
		ButtonSizer->Add(ButtonOK, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

		wxButton* ButtonCancel = new wxButton(this, wxID_CANCEL, *LocalizeUnrealEd("&Cancel"));
		ButtonSizer->Add(ButtonCancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	}
	MainSizer->Add(ButtonSizer, 0, wxALIGN_RIGHT|wxALL, 2);

	MainSizer->SetSizeHints(this);
	Centre();

	FWindowUtil::LoadPosSize( TEXT("DlgChooseInputKey"), this );
}

WxDlgChooseInputKey::~WxDlgChooseInputKey()
{
	if ( KeyList != NULL )
	{
		KeyList->PopEventHandler(true);
	}
	ListInputKeyHandler = NULL;
	FWindowUtil::SavePosSize( TEXT("DlgChooseInputKey"), this );
}

/** Returns the key selected by the user */
FName WxDlgChooseInputKey::GetSelectedKey() const
{
	return SelectedKey;
}

/** show the dialog */
int WxDlgChooseInputKey::ShowModal( const TArray<FName>& AvailableKeys )
{
	SetTitle(*LocalizeUI(TEXT("DlgChooseNewKey_Title")));

	// populate the list of keys
	for ( INT KeyIndex = 0; KeyIndex < AvailableKeys.Num(); KeyIndex++ )
	{
		KeyList->Append(*AvailableKeys(KeyIndex).ToString());
	}

	// this is used to intercept and route key presses to the dialog's input processing method.
	ListInputKeyHandler = new WxRawInputKeyHandler(this);
	KeyList->PushEventHandler(ListInputKeyHandler);

	return wxDialog::ShowModal();
}

/** called when the user selects an item in the list */
void WxDlgChooseInputKey::OnInputKeySelected( wxCommandEvent& Event )
{
	SelectedKey = KeyList->GetStringSelection().c_str();
}


/** called when the user presses a key while the list is active */
void WxDlgChooseInputKey::OnInputKeyPressed( wxKeyEvent& Event )
{
	bool bProcessedEvent=false;

	wxWindow* FocusedControl = FindFocus();
	if ( FocusedControl == KeyList )
	{
		// if the user is holding a modifier key, temporarily disable auto-key selection
		if ( !Event.AltDown() && !Event.ShiftDown() && !Event.CmdDown() )
		{
			INT wxCharCode = Event.GetKeyCode();
			
			// wxCharCode will something like WXK_LBUTTON which doesn't map in any way to the windows virtual key code
			// We need to the windows virtual key code because this is what the WindowsClient uses for the key in its
			// map of char code <==> input key name
			bool bVirtual=false;
			WORD TranslatedCode = wxCharCodeWXToMSW(wxCharCode, &bVirtual);
			
			bProcessedEvent = InterceptInputKey(TranslatedCode);
		}
	}

	Event.Skip(!bProcessedEvent);
}

/** Called when the user clicks the mouse on the list or uses the mouse wheel */
void WxDlgChooseInputKey::OnMouseInput( wxMouseEvent& Event )
{
	bool bProcessedEvent=false;

	wxWindow* FocusedControl = FindFocus();
	if ( FocusedControl == KeyList )
	{
		// if the user is holding a modifier key, temporarily disable auto-key selection
		if ( !Event.AltDown() && !Event.ShiftDown() && !Event.CmdDown() )
		{
			// the value returned from wxMouseEvent::GetButton() is the index of the button on the mouse,
			// not the windows virtual key code for that button, so translate the code now.
			INT wxButtonCode = Event.GetButton();
			switch ( wxButtonCode )
			{
			case wxMOUSE_BTN_LEFT:
				wxButtonCode = VK_LBUTTON;
				break;
			case wxMOUSE_BTN_MIDDLE:
				wxButtonCode = VK_MBUTTON;
				break;
			case wxMOUSE_BTN_RIGHT:
				wxButtonCode = VK_RBUTTON;
				break;

			default:
				// there is no windows virtual code for mouse wheel movement, so just handle
				// the translation and selection here
				if ( Event.GetEventType() == wxEVT_MOUSEWHEEL )
				{
					FName KeyName = Event.GetWheelRotation() > 0
						? KEY_MouseScrollUp : KEY_MouseScrollDown;

					INT ListIndex = KeyList->FindString(*KeyName.ToString());
					if ( ListIndex != wxNOT_FOUND )
					{
						// select this key in the list
						SelectedKey = KeyName;
						KeyList->SetSelection( ListIndex );
					}

					bProcessedEvent = true;
				}

				wxButtonCode = wxMOUSE_BTN_NONE;
				break;
			}

			if ( wxButtonCode != wxMOUSE_BTN_NONE )
			{
				bProcessedEvent = InterceptInputKey(wxButtonCode);
			}
		}
	}

	Event.Skip(!bProcessedEvent);
}

/**
 * Determines whether the specified code corresponds to a key in the list
 * and selects the appropriate element if found.
 *
 * @param	KeyCode		a windows virtual input key code
 *
 * @return	true if the specified code maps to a key which can be bound to an input alias
 */
bool WxDlgChooseInputKey::InterceptInputKey( WORD KeyCode )
{
	bool bProcessedEvent = false;

	switch ( KeyCode )
	{
	// We don't want to allow Ctrl, Alt, or Shift to be bound to an input alias by themselves,
	// so don't translate them.  Additionally, this allows users to temporarily disable auto-key selection
	// if they need to.
	//case VK_SHIFT:      KeyCode = VK_LSHIFT;		break;
	//case VK_CONTROL:    KeyCode = VK_LCONTROL;	break;
	//case VK_MENU :      KeyCode = VK_LMENU;		break;

	// these characters aren't properly converted by wxWidgets' wxCharCodeWXToMSW method
	case TEXT(';'):		KeyCode = VK_OEM_1;		break;
	case TEXT('+'):		KeyCode = VK_OEM_PLUS;	break;
	case TEXT(','):		KeyCode = VK_OEM_COMMA;	break;
	case TEXT('-'):		KeyCode = VK_OEM_MINUS;	break;
	case TEXT('.'):		KeyCode = VK_OEM_PERIOD;break;
	case TEXT('/'):		KeyCode = VK_OEM_2;		break;
	case TEXT('~'):		KeyCode = VK_OEM_3;		break;
	case TEXT('['):		KeyCode = VK_OEM_4;		break;
	case TEXT('\\'):	KeyCode = VK_OEM_5;		break;
	case TEXT(']'):		KeyCode = VK_OEM_6;		break;
	case TEXT('\''):	KeyCode = VK_OEM_7;		break;
	}
	check(GEngine);
	check(GEngine->Client);

	FName KeyName = GEngine->Client->GetVirtualKeyName(KeyCode);

//	debugf(TEXT("WxDlgChooseInputKey::OnInputKeyReleased  CharCode: 0x%04X   KeyName: %s"), KeyCode, *KeyName.ToString());
	if ( KeyName != NAME_None )
	{
		// special case - if the Enter key is currently selected and the user pressed enter, close the dialog
		if ( SelectedKey == KEY_Enter && KeyName == KEY_Enter )
		{
			bProcessedEvent = false;
		}
		else
		{
			INT ListIndex = KeyList->FindString(*KeyName.ToString());
			if ( ListIndex != wxNOT_FOUND )
			{
				// select this key in the list
				SelectedKey = KeyName;
				KeyList->SetSelection( ListIndex );
			}

			// swallow this input so that we don't get different behavior depending on whether the key is in the list or not
			bProcessedEvent = true;
		}
	}
	// if the user pressed the numpad Enter key, close the dialog
	else if ( KeyCode == 0x174 )	
	{
		bProcessedEvent = true;
		EndModal(wxID_OK);
	}

	return bProcessedEvent;
}

// Callback for the wxListView sorting function.
namespace
{
	int wxCALLBACK WxDlgUIEventKeyBindings_ListSort( long InItem1, long InItem2, long InSortData )
	{
		FName* A = (FName*)InItem1;
		FName* B = (FName*)InItem2;
		WxDlgUIEventKeyBindings::FSortOptions* SortOptions = (WxDlgUIEventKeyBindings::FSortOptions*)InSortData;

		FString CompA, CompB;

		// Generate a string to run the compares against for each object based on
		// the current column.

		switch( SortOptions->Column )
		{
		case WxDlgUIEventKeyBindings::Field_Key:				
			CompA = A->ToString();
			CompB = B->ToString();
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

/**
 * Returns a pointer to the UIState class recommended for the specified event type.
 */
UClass* GetInputEventTypeRecommendedState( ESupportedInputEvents EventType )
{
	UClass* Result = NULL;

	switch(EventType)
	{
	case SIE_MouseButton:
		// Add mouse button clicks to the active state.
		Result = UUIState_Active::StaticClass();
		break;
	case SIE_Keyboard: 
	case SIE_PressedOnly: 
		// Add keyboard presses to the focused state.
		Result = UUIState_Focused::StaticClass();
		break;
	case SIE_Axis:
		// Add axis events to the pressed state.
		Result = UUIState_Pressed::StaticClass();
		break;
	}
	
	return Result;
}

/* ==========================================================================================================
	WxDlgUIEventKeyBindings
========================================================================================================== */


BEGIN_EVENT_TABLE(WxDlgUIEventKeyBindings, wxDialog)
	EVT_LISTBOX(ID_UI_WIDGETLIST, WxDlgUIEventKeyBindings::OnWidgetClassSelected)
	EVT_LISTBOX(ID_UI_EVENTLIST, WxDlgUIEventKeyBindings::OnAliasListSelected )

	EVT_CHECKLISTBOX(ID_UI_STATESLIST, WxDlgUIEventKeyBindings::OnStateCheckToggled)

	EVT_CHECKBOX(ID_UI_CHECK_ALTREQ, WxDlgUIEventKeyBindings::OnModifierChecked)
	EVT_CHECKBOX(ID_UI_CHECK_CTRLREQ, WxDlgUIEventKeyBindings::OnModifierChecked)
	EVT_CHECKBOX(ID_UI_CHECK_SHIFTREQ, WxDlgUIEventKeyBindings::OnModifierChecked)
	EVT_CHECKBOX(ID_UI_CHECK_ALTEXCLUDED, WxDlgUIEventKeyBindings::OnModifierChecked)
	EVT_CHECKBOX(ID_UI_CHECK_CTRLEXCLUDED, WxDlgUIEventKeyBindings::OnModifierChecked)
	EVT_CHECKBOX(ID_UI_CHECK_SHIFTEXCLUDED, WxDlgUIEventKeyBindings::OnModifierChecked)

	EVT_LIST_ITEM_SELECTED(ID_UI_BOUNDLIST, WxDlgUIEventKeyBindings::OnInputKeyList_Selected)
	EVT_LIST_ITEM_DESELECTED(ID_UI_BOUNDLIST, WxDlgUIEventKeyBindings::OnInputKeyList_Deselected)
	EVT_LIST_ITEM_RIGHT_CLICK(ID_UI_BOUNDLIST, WxDlgUIEventKeyBindings::OnInputKeyList_RightClick)
	EVT_LIST_KEY_DOWN(ID_UI_BOUNDLIST, WxDlgUIEventKeyBindings::OnInputKeyList_KeyDown)
	EVT_LIST_ITEM_ACTIVATED(ID_UI_BOUNDLIST, WxDlgUIEventKeyBindings::OnInputKeyList_ReplaceKey)

	EVT_MENU( ID_UI_BIND_NEW_INPUT_KEY, WxDlgUIEventKeyBindings::OnNewKeyButton )
	EVT_MENU( ID_UI_REMOVE_BOUND_INPUT_KEY, WxDlgUIEventKeyBindings::OnRemoveInputKey )

	EVT_BUTTON(ID_UI_BIND_NEW_INPUT_KEY, WxDlgUIEventKeyBindings::OnNewKeyButton)
	EVT_BUTTON(wxID_OK, WxDlgUIEventKeyBindings::OnOK)

	EVT_COMBOBOX(ID_UI_COMBO_CHOOSEPLATFORM, WxDlgUIEventKeyBindings::OnPlatformSelected)

	EVT_UPDATE_UI_RANGE(ID_UI_REQUIRES_VALID_KEY_START, ID_UI_REQUIRES_VALID_KEY_END, WxDlgUIEventKeyBindings::OnUpdateOptionsPanel_UI)
	EVT_UPDATE_UI(ID_UI_BIND_NEW_INPUT_KEY, WxDlgUIEventKeyBindings::OnUpdateInputKeys_UI)
	EVT_UPDATE_UI(ID_UI_BOUNDLIST, WxDlgUIEventKeyBindings::OnUpdateInputKeys_UI)
	EVT_UPDATE_UI(ID_UI_COMBO_CHOOSEPLATFORM, WxDlgUIEventKeyBindings::OnUpdateInputKeys_UI)
END_EVENT_TABLE()

WxDlgUIEventKeyBindings::WxDlgUIEventKeyBindings()
{
	Init();
}

WxDlgUIEventKeyBindings::WxDlgUIEventKeyBindings(wxWindow* InParent)
{
	Init();
	Create(InParent);
}


void WxDlgUIEventKeyBindings::Init()
{
	MainSplitter = NULL;
	LeftSplitter = NULL;
	ListWidgets = NULL;
	ListAliases = NULL;
	ListStates = NULL;
	chk_ModifierFlag[INDEX_AltReq] = NULL;
	chk_ModifierFlag[INDEX_AltExcl] = NULL;
	chk_ModifierFlag[INDEX_CtrlReq] = NULL;
	chk_ModifierFlag[INDEX_CtrlExcl] = NULL;
	chk_ModifierFlag[INDEX_ShiftReq] = NULL;
	chk_ModifierFlag[INDEX_ShiftExcl] = NULL;
	ListBoundInputKeys = NULL;
	btn_NewKey = NULL;
	combo_Platform = NULL;
	SelectedInputKey = NAME_None;
	CurrentPlatformName = CONSOLESUPPORT_NAME_PC;

	// Set default sort options.
	SortOptions.bSortAscending = TRUE;
	SortOptions.Column = -1;
}

UBOOL WxDlgUIEventKeyBindings::Create( wxWindow* InParent )
{
	wxDialog::Create(InParent, wxID_ANY, *LocalizeUI("DlgUIEventKeyBindings_Title"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX | wxSYSTEM_MENU | wxRESIZE_BORDER);

	CreateControls();

	// Build a list of available input keys.
	BuildKeyList();

	// Add all available UI Widgets to the widget combo box.
	PopulateWidgetClassList();

	// Load window position and splitter position.
	LoadDialogOptions();

	// perform the initial update of all controls by simulating the user changing the current widget class
	wxCommandEvent Event(wxEVT_COMMAND_LISTBOX_SELECTED, ID_UI_WIDGETLIST);
	ProcessEvent(Event);

	return TRUE;
}

WxDlgUIEventKeyBindings::~WxDlgUIEventKeyBindings()
{
	SaveDialogOptions();
}


void WxDlgUIEventKeyBindings::CreateControls()
{
	wxBoxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(MainSizer);
	{
		MainSplitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxSize(500, 300), wxSP_3DSASH|wxNO_BORDER );
		MainSplitter->SetMinimumPaneSize(20);
		{
			// Widget Class and Available Input Alias lists
			wxPanel* LeftContainerPanel = new wxPanel( MainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
			{
				wxBoxSizer* LeftContainerSizer = new wxBoxSizer(wxHORIZONTAL);
				LeftContainerPanel->SetSizer(LeftContainerSizer);

				LeftSplitter = new wxSplitterWindow( LeftContainerPanel, wxID_ANY, wxDefaultPosition, wxSize(100, 100), wxSP_3DSASH|wxNO_BORDER );
				LeftSplitter->SetMinimumPaneSize(20);
				{
					wxPanel* WidgetClassContainerPanel = new wxPanel( LeftSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
					wxStaticBox* WidgetClassStaticBox = new wxStaticBox(WidgetClassContainerPanel, wxID_ANY, *LocalizeUI(TEXT("DlgUIEventKeyBindings_WidgetClass")));
					wxStaticBoxSizer* WidgetClassStaticBoxSizer = new wxStaticBoxSizer(WidgetClassStaticBox, wxVERTICAL);
					WidgetClassContainerPanel->SetSizer(WidgetClassStaticBoxSizer);

					ListWidgets = new wxListBox( WidgetClassContainerPanel, ID_UI_WIDGETLIST, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_EXTENDED );
					WidgetClassStaticBoxSizer->Add(ListWidgets, 1, wxGROW|wxALL, 0);

					wxPanel* InputAliasContainerPanel = new wxPanel( LeftSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
					wxStaticBox* InputAliasStaticBox = new wxStaticBox(InputAliasContainerPanel, wxID_ANY, *LocalizeUI(TEXT("DlgUIEventKeyBindings_WidgetEvents")));
					wxStaticBoxSizer* InputAliasStaticBoxSizer = new wxStaticBoxSizer(InputAliasStaticBox, wxVERTICAL);
					InputAliasContainerPanel->SetSizer(InputAliasStaticBoxSizer);

					ListAliases = new wxListBox( InputAliasContainerPanel, ID_UI_EVENTLIST, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_EXTENDED );
					InputAliasStaticBoxSizer->Add(ListAliases, 1, wxGROW|wxALL, 0);

					LeftSplitter->SplitHorizontally(WidgetClassContainerPanel, InputAliasContainerPanel, 125);
				}
				LeftContainerSizer->Add(LeftSplitter, 1, wxGROW|wxALL, 0);
			}

			// Enabled Input Alias List, Modifier checkboxes, and bound input key list
			wxPanel* RightContainerPanel = new wxPanel( MainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
			{
				wxBoxSizer* RightContainerSizer = new wxBoxSizer(wxVERTICAL);
				RightContainerPanel->SetSizer(RightContainerSizer);
				{
					wxBoxSizer* RightContainerSizerHorz = new wxBoxSizer(wxHORIZONTAL);
					RightContainerSizer->Add(RightContainerSizerHorz, 0, wxGROW|wxALL, 0);
					{
						wxStaticBox* WidgetStatesStaticBox = new wxStaticBox(RightContainerPanel, wxID_ANY, *LocalizeUI(TEXT("DlgUIEventKeyBindings_StatesLabel")));
						wxStaticBoxSizer* WidgetStatesStaticSizer = new wxStaticBoxSizer(WidgetStatesStaticBox, wxVERTICAL);
						RightContainerSizerHorz->Add(WidgetStatesStaticSizer, 1, wxGROW|wxALL, 5);
						{
							ListStates = new wxCheckListBox( RightContainerPanel, ID_UI_STATESLIST, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE|wxSUNKEN_BORDER );
							WidgetStatesStaticSizer->Add(ListStates, 1, wxGROW|wxALL, 0);

							wxStaticBox* ModifiersStaticBox = new wxStaticBox(RightContainerPanel, wxID_ANY, *LocalizeUI(TEXT("DlgUIEventKeyBindings_ModifierLabel")));
							wxStaticBoxSizer* ModifiersStaticSizer = new wxStaticBoxSizer(ModifiersStaticBox, wxVERTICAL);
							RightContainerSizerHorz->Add(ModifiersStaticSizer, 0, wxGROW|wxALL, 5);
							{
								wxBoxSizer* ModifierHeaderSizer = new wxBoxSizer(wxHORIZONTAL);
								ModifiersStaticSizer->Add(ModifierHeaderSizer, 0, wxGROW|wxALL, 0);
								ModifierHeaderSizer->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

								wxStaticText* RequiredLabel = new wxStaticText( RightContainerPanel, wxID_STATIC, *LocalizeUI(TEXT("DlgUIEventKeyBindings_RequiredLabel")), wxDefaultPosition, wxDefaultSize, 0 );
								ModifierHeaderSizer->Add(RequiredLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

								wxStaticText* ExcludedLabel = new wxStaticText( RightContainerPanel, wxID_STATIC, *LocalizeUI(TEXT("DlgUIEventKeyBindings_ExcludedLabel")), wxDefaultPosition, wxDefaultSize, 0 );
								ModifierHeaderSizer->Add(ExcludedLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

								wxBoxSizer* AltSizer = new wxBoxSizer(wxHORIZONTAL);
								ModifiersStaticSizer->Add(AltSizer, 0, wxGROW|wxALL, 0);
								{
									wxStaticText* AltLabel = new wxStaticText( RightContainerPanel, wxID_STATIC, TEXT("Alt"), wxDefaultPosition, wxDefaultSize, 0 );
									AltSizer->Add(AltLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

									AltSizer->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

									chk_ModifierFlag[INDEX_AltReq] = new wxCheckBox( RightContainerPanel, ID_UI_CHECK_ALTREQ, TEXT("     "), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
									chk_ModifierFlag[INDEX_AltReq]->SetValue(false);
									AltSizer->Add(chk_ModifierFlag[INDEX_AltReq], 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

									chk_ModifierFlag[INDEX_AltExcl] = new wxCheckBox( RightContainerPanel, ID_UI_CHECK_ALTEXCLUDED, TEXT("           "), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
									chk_ModifierFlag[INDEX_AltExcl]->SetValue(false);
									AltSizer->Add(chk_ModifierFlag[INDEX_AltExcl], 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
								}

								wxBoxSizer* CtrlSizer = new wxBoxSizer(wxHORIZONTAL);
								ModifiersStaticSizer->Add(CtrlSizer, 0, wxGROW|wxALL, 0);
								{
									wxStaticText* CtrlLabel = new wxStaticText( RightContainerPanel, wxID_STATIC, TEXT("Ctrl"), wxDefaultPosition, wxDefaultSize, 0 );
									CtrlSizer->Add(CtrlLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

									CtrlSizer->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

									chk_ModifierFlag[INDEX_CtrlReq] = new wxCheckBox( RightContainerPanel, ID_UI_CHECK_CTRLREQ, TEXT("     "), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
									chk_ModifierFlag[INDEX_CtrlReq]->SetValue(false);
									CtrlSizer->Add(chk_ModifierFlag[INDEX_CtrlReq], 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

									chk_ModifierFlag[INDEX_CtrlExcl] = new wxCheckBox( RightContainerPanel, ID_UI_CHECK_CTRLEXCLUDED, TEXT("           "), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
									chk_ModifierFlag[INDEX_CtrlExcl]->SetValue(false);
									CtrlSizer->Add(chk_ModifierFlag[INDEX_CtrlExcl], 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
								}

								wxBoxSizer* ShiftSizer = new wxBoxSizer(wxHORIZONTAL);
								ModifiersStaticSizer->Add(ShiftSizer, 0, wxGROW|wxALL, 0);
								{
									wxStaticText* ShiftLabel = new wxStaticText( RightContainerPanel, wxID_STATIC, TEXT("Shift"), wxDefaultPosition, wxDefaultSize, 0 );
									ShiftSizer->Add(ShiftLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

									ShiftSizer->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

									chk_ModifierFlag[INDEX_ShiftReq] = new wxCheckBox( RightContainerPanel, ID_UI_CHECK_SHIFTREQ, TEXT("     "), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
									chk_ModifierFlag[INDEX_ShiftReq]->SetValue(false);
									ShiftSizer->Add(chk_ModifierFlag[INDEX_ShiftReq], 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

									chk_ModifierFlag[INDEX_ShiftExcl] = new wxCheckBox( RightContainerPanel, ID_UI_CHECK_SHIFTEXCLUDED, TEXT("           "), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
									chk_ModifierFlag[INDEX_ShiftExcl]->SetValue(false);
									ShiftSizer->Add(chk_ModifierFlag[INDEX_ShiftExcl], 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
								}
							}
						}
					}
				}
				
				wxBoxSizer* InputKeyControlsHorzSizer = new wxBoxSizer(wxHORIZONTAL);
				RightContainerSizer->Add(InputKeyControlsHorzSizer, 0, wxGROW|wxALL, 0);
				{
					btn_NewKey = new wxButton( RightContainerPanel, ID_UI_BIND_NEW_INPUT_KEY, *LocalizeUI(TEXT("DlgUIEventKeyBindings_NewKeyLabel")), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
					InputKeyControlsHorzSizer->Add(btn_NewKey, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

					wxArrayString combo_PlatformStrings;

					// if we have any console plugins, add them to the list of selectable platforms
					const INT NumSupportedConsoles = FConsoleSupportContainer::GetConsoleSupportContainer()->GetNumConsoleSupports();
					if ( NumSupportedConsoles > 0 )
					{
						// loop through all consoles (only support 20 consoles)
						INT ConsoleIndex = 0;
						for (FConsoleSupportIterator It; It && ConsoleIndex < 20; ++It, ConsoleIndex++)
						{
							combo_PlatformStrings.Add(It->GetConsoleName());
						}
					}

					combo_Platform = new WxComboBox( RightContainerPanel, ID_UI_COMBO_CHOOSEPLATFORM, *CurrentPlatformName, wxDefaultPosition, wxDefaultSize, combo_PlatformStrings, wxCB_READONLY );
					combo_Platform->SetStringSelection(*CurrentPlatformName);
					InputKeyControlsHorzSizer->Add( combo_Platform, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

					// for now, always disable.
					if ( NumSupportedConsoles == 0 )
					{
						combo_Platform->Enable(false);
					}
				}

				ListBoundInputKeys = new wxListCtrl( RightContainerPanel, ID_UI_BOUNDLIST, wxDefaultPosition, wxSize(100, 100), wxLC_REPORT|wxLC_VRULES|wxLC_HRULES|wxSUNKEN_BORDER );
				ListBoundInputKeys->InsertColumn(COLHEADER_KeyName, *LocalizeUI(TEXT("DlgUIEventKeyBindings_Key")));
				ListBoundInputKeys->InsertColumn(COLHEADER_StateNames, *LocalizeUI(TEXT("DlgUIEventKeyBindings_StatesLabel")));
				ListBoundInputKeys->InsertColumn(COLHEADER_Modifiers, *LocalizeUI(TEXT("DlgUIEventKeyBindings_ModifierLabel")));
				RightContainerSizer->Add(ListBoundInputKeys, 1, wxGROW|wxALL, 5);
			}
			MainSplitter->SplitVertically(LeftContainerPanel, RightContainerPanel, 160);
		}
		MainSizer->Add(MainSplitter, 1, wxGROW|wxALL, 5);

		wxBoxSizer* ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
		{
			wxButton* ButtonOK = new wxButton(this, wxID_OK, *LocalizeUnrealEd("&OK"));
			ButtonOK->SetDefault();

			ButtonSizer->Add(ButtonOK, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

			wxButton* ButtonCancel = new wxButton(this, wxID_CANCEL, *LocalizeUnrealEd("&Cancel"));
			ButtonSizer->Add(ButtonCancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
		}
		MainSizer->Add(ButtonSizer, 0, wxALIGN_RIGHT|wxALL, 2);
	}
}

/**
 * Loads the window position and other options for the dialog.
 */
void WxDlgUIEventKeyBindings::LoadDialogOptions()
{
	FWindowUtil::LoadPosSize(TEXT("DlgUIEventKeyBindings"), this, -1,-1, 800, 400);

	// splitter positions
	{
		INT SplitterPos = 160;
		GConfig->GetInt(TEXT("DlgUIEventKeyBindings"), TEXT("MainSplitterPos"), SplitterPos, GEditorUserSettingsIni);
		MainSplitter->SetSashPosition(SplitterPos);

		SplitterPos = 125;
		GConfig->GetInt(TEXT("DlgUIEventKeyBindings"), TEXT("LeftSplitterPos"), SplitterPos, GEditorUserSettingsIni);
		LeftSplitter->SetSashPosition(SplitterPos);
	}

	// last selected widget class
	{
		UINT LastSelectedClassIndex=0;
		FString LastSelectedClass;
		if ( GConfig->GetString(TEXT("DlgUIEventKeyBindings"), TEXT("LastSelectedClass"), LastSelectedClass, GEditorUserSettingsIni) )
		{
			LastSelectedClassIndex = ListWidgets->FindString(*LastSelectedClass);
			if ( LastSelectedClassIndex == wxNOT_FOUND )
			{
				LastSelectedClassIndex = 0;
			}
		}

		if ( LastSelectedClassIndex >= 0 && LastSelectedClassIndex < ListWidgets->GetCount() )
		{
			ListWidgets->SetSelection(LastSelectedClassIndex);
		}
	}


	// last selected platform
	{
		if ( GConfig->GetString(TEXT("DlgUIEventKeyBindings"), TEXT("LastSelectedPlatform"), CurrentPlatformName, GEditorUserSettingsIni) )
		{
			if ( !combo_Platform->SetStringSelection(*CurrentPlatformName) )
			{
				// if we couldn't activate the previously selected platform (perhaps because that platform is no longer supported on this machine)
				// just reset the CurrentPlatformName to whatever string is actually visible in the combo.
				CurrentPlatformName = combo_Platform->GetValue().c_str();
			}
		}
	}
}

/**
 * Saves the window position and other options for the dialog.
 */
void WxDlgUIEventKeyBindings::SaveDialogOptions() const
{
	// Save window position and splitter sash position.
	FWindowUtil::SavePosSize(TEXT("DlgUIEventKeyBindings"), this);
	GConfig->SetInt(TEXT("DlgUIEventKeyBindings"), TEXT("MainSplitterPos"), MainSplitter->GetSashPosition(), GEditorUserSettingsIni);
	GConfig->SetInt(TEXT("DlgUIEventKeyBindings"), TEXT("LeftSplitterPos"), LeftSplitter->GetSashPosition(), GEditorUserSettingsIni);

	// save the currently selected class
	wxArrayInt SelectedIndices;
	ListWidgets->GetSelections(SelectedIndices);
	if ( SelectedIndices.GetCount() > 0 )
	{
		GConfig->SetString(TEXT("DlgUIEventKeyBindings"), TEXT("LastSelectedClass"), ListWidgets->GetString(SelectedIndices[0]).c_str(), GEditorUserSettingsIni);
	}

	// save the currently selected platform
	GConfig->SetString(TEXT("DlgUIEventKeyBindings"), TEXT("LastSelectedPlatform"), *CurrentPlatformName, GEditorUserSettingsIni);
}

/** Builds a list of available input keys. */
void WxDlgUIEventKeyBindings::BuildKeyList()
{
	KeyList.Empty();
	KeyEventMap.Empty();

	// Do some preprocessor trickery to get the keys we need into a array.  
	{
		#define DEFINE_KEY(Name, SupportedEvent)											\
				KeyList.AddItem(KEY_##Name);												\
				KeyEventMap.Set(KEY_##Name, SupportedEvent);																							


		#include "UnKeys.h"

		#undef DEFINE_KEY
	}
}

/** 
 * Callback for the Serialize function of FSerializableObject. 
 *
 * @param	Ar	Archive to serialize to.
 */
void WxDlgUIEventKeyBindings::Serialize(FArchive& Ar)
{
	// Serialize all of the UUIObjects that we are storing references to.
	for(FClassToUIKeyMap::TIterator It(WidgetLookupMap); It; ++It)
	{
		UClass* Class = const_cast<UClass*>(It.Key());
		TArray<FUIInputAliasStateMap>& StateMappings = It.Value();
		Ar << Class << StateMappings;
	}

	// Serialize all of the UClasses that we are storing references to.
	for ( TMap<FString,UClass*>::TIterator It(StateNameToClassMap); It; ++It )
	{
		UClass* StateClass = It.Value();
		Ar << StateClass;
	}
}

/**
 * Returns the widget state alias struct given the widget class and state class to use for the search.
 *
 * @param	WidgetClass		Widget class to use for the search.
 * @param	StateClass		The state to we are looking for.
 *
 *  @return	A pointer to the UIInputAliasStateMap for the specified widget and state classes.
 */
FUIInputAliasStateMap* WxDlgUIEventKeyBindings::GetStateMapFromClass(UClass* WidgetClass, const UClass* StateClass)
{
	FUIInputAliasStateMap* Result = NULL;
	TArray<FUIInputAliasStateMap> *StateMaps = WidgetLookupMap.Find(WidgetClass);

	if(StateMaps != NULL)
	{	
		for(INT StateIdx = 0; StateIdx < StateMaps->Num(); StateIdx++)
		{
			FUIInputAliasStateMap &StateMap = (*StateMaps)(StateIdx);
			if(StateClass == StateMap.State)
			{
				Result = &StateMap;
				break;
			}
		}
	}

	return Result;
}

/** 
 * Returns the action alias map for the specified state using the selected widget class and event alias.
 *
 * @param StateClass	The state class to look in for the selected event alias struct.
 * @return A pointer to the inputactionalias struct for the state specified.
 */
TArray<FUIInputActionAlias*> WxDlgUIEventKeyBindings::GetSelectedActionAliasMaps(const UClass* StateClass)
{
	check(StateClass);
	check(StateClass->IsChildOf(UUIState::StaticClass()));

	TArray<FUIInputActionAlias*> Result;
	for ( INT WidgetIndex = 0; WidgetIndex < SelectedWidgets.Num(); WidgetIndex++ )
	{
		UUIScreenObject* SelectedWidget = SelectedWidgets(WidgetIndex);
		FUIInputAliasStateMap* StateMap = GetStateMapFromClass(SelectedWidget->GetClass(), StateClass);

		if(StateMap != NULL)
		{
			for ( INT SelectedAliasIndex = 0; SelectedAliasIndex < SelectedAliases.Num(); SelectedAliasIndex++ )
			{
				const FName& SelectedAliasName = SelectedAliases(SelectedAliasIndex);
				for(INT ActionIdx = 0; ActionIdx < StateMap->StateInputAliases.Num(); ActionIdx++)
				{
					FUIInputActionAlias& SearchAlias = StateMap->StateInputAliases(ActionIdx);
					if ( SearchAlias.InputAliasName == SelectedAliasName )
					{
						Result.AddItem(&SearchAlias);
						break;
					}
				}
			}
		}
	}

	return Result;
}


/** Generates a bitmask of modifier flags based on the currently checked modifier checkboxes */
BYTE WxDlgUIEventKeyBindings::GetCurrentModifierFlags() const
{
	BYTE ModifierFlags = 0;

	for ( INT ModifierIndex = 0; ModifierIndex < INDEX_MAX; ModifierIndex++ )
	{
		if ( chk_ModifierFlag[ModifierIndex]->IsChecked() )
		{
			ModifierFlags |= (1<<ModifierIndex);
		}
	}

	return ModifierFlags;
}

/**
 * Returns the list of states which can be contain input alias bindings for the specified widget's class.  This is usually
 * the classes in the widget's DefaultStates array, but might contain more states if the widget class is abstract.
 */
TArray<UClass*> WxDlgUIEventKeyBindings::GetSupportedStates( UUIScreenObject* WidgetCDO ) const
{
	check(WidgetCDO);
	UClass* WidgetClass = WidgetCDO->GetClass();

	// initialize the list with the states in the widget class's DefaultStates array.
	TArray<UClass*> SupportedStates = WidgetCDO->DefaultStates;
	if ( WidgetClass->HasAnyClassFlags(CLASS_Abstract) )
	{
		// if this widget class is marked abstract, we will allow the user to add keys for all states, not just the states supported by this widget
		// this is so that the user can configure "default" input alias mappings in a base class, then only set the ones that should be different in
		// the placeable classes.
		TArray<FUIStateResourceInfo> StateResources;
		GUnrealEd->GetBrowserManager()->UISceneManager->GetSupportedUIStates(StateResources, WidgetClass);
		for ( INT StateIndex = 0; StateIndex < StateResources.Num(); StateIndex++ )
		{
			FUIStateResourceInfo& ResourceInfo = StateResources(StateIndex);

			UClass* AvailableState = ResourceInfo.UIResource->GetClass();
			if ( !SupportedStates.ContainsItem(AvailableState) )
			{
				// we probably want to insert this state last, but before any of its child states.
				INT SupportedStateIndex = 0;
				for ( ;SupportedStateIndex < SupportedStates.Num(); SupportedStateIndex++ )
				{
					if ( SupportedStates(SupportedStateIndex)->IsChildOf(AvailableState) )
					{
						break;
					}
				}

				SupportedStates.InsertItem(AvailableState, SupportedStateIndex);
			}
		}
	}

	return SupportedStates;
}

/**
 * Finds all aliases in a specific state that are bound to the specified key/modifier combination.
 *
 * @param	SearchWidget		the widget to search for matching bindings
 * @param	SearchStateClass	the state to search for bound input keys
 * @param	SearchKey			the input key / modifier combination to search for
 * @param	out_Aliases			receives the list of existing mappings.
 */
UBOOL WxDlgUIEventKeyBindings::FindInputKeyMappings( UUIScreenObject* SearchWidget, UClass* SearchStateClass, const FRawInputKeyEventData& SearchKey, TArray<FName>& out_Aliases )
{
	UBOOL bResult = FALSE;

	if ( SearchWidget != NULL && SearchStateClass != NULL && SearchKey.InputKeyName != NAME_None )
	{
		out_Aliases.Empty();
		// Find the state map for this selected widget, and the state determined by the event type.
		FUIInputAliasStateMap* StateMap = GetStateMapFromClass(SearchWidget->GetClass(), SearchStateClass);
		if ( StateMap != NULL )
		{
			for ( INT AliasIdx = 0; AliasIdx < StateMap->StateInputAliases.Num(); AliasIdx++ )
			{
				// iterate through each alias in this state's alias map
				FUIInputActionAlias& Alias = StateMap->StateInputAliases(AliasIdx);
				if ( Alias.LinkedInputKeys.ContainsItem(SearchKey) )
				{
					// if a matching mapping is found, add this alias's name to the output array
					out_Aliases.AddUniqueItem(Alias.InputAliasName);
					bResult = TRUE;
				}
			}
		}
	}

	return bResult;
}

/**
 * Adds an input key to the the list of keys mapped for the currently selected input aliases in the 
 * specified state for the selected widgets.
 *
 * @param	KeyName		the input key to add to the widget's map.
 * @param	StateClass  The state class to add the key binding to.
 * @param	bUseDefaultModifierFlags
 *						if TRUE, the default modifier flag mask will be assigned to this new alias instead
 *						of the currently checked ones; useful when adding an entirely new key.
 */
void WxDlgUIEventKeyBindings::AddKeyToStateMap( const FName& KeyName, const UClass* StateClass, UBOOL bUseDefaultModifierFlags/*=FALSE*/ )
{
	UBOOL bRefreshInputKeyCache=FALSE;
	for ( INT SelectionIndex = 0; SelectionIndex < SelectedWidgets.Num(); SelectionIndex++ )
	{
		UUIScreenObject* SelectedWidget = SelectedWidgets(SelectionIndex);

		// Find the state map for this selected widget, and the state determined by the event type.
		FUIInputAliasStateMap* StateMap = GetStateMapFromClass(SelectedWidget->GetClass(), StateClass);
		if(StateMap != NULL)
		{
			// for each selected alias
			for ( INT SelectedAliasIndex = 0; SelectedAliasIndex < SelectedAliases.Num(); SelectedAliasIndex++ )
			{
				const FName& SelectedAliasName = SelectedAliases(SelectedAliasIndex);

				// See if the selected alias exists in this state yet, if not, add a entry in the alias array for the
				// selected alias.  Then add the key to the action alias struct.
				FUIInputActionAlias* ExistingAlias = NULL;
				for(INT AliasIdx = 0; AliasIdx < StateMap->StateInputAliases.Num(); AliasIdx++)
				{
					FUIInputActionAlias& Alias = StateMap->StateInputAliases(AliasIdx);
					if( Alias.InputAliasName == SelectedAliasName)
					{
						ExistingAlias = &Alias;
						break;
					}
				}

				// if the state doesn't already have this input alias in its list, add it here
				if( ExistingAlias == NULL )
				{
					ExistingAlias = new(StateMap->StateInputAliases) FUIInputActionAlias(EC_EventParm);
					ExistingAlias->InputAliasName = SelectedAliasName;
				}

				FRawInputKeyEventData InputKeyMappingData(KeyName, bUseDefaultModifierFlags ? DefaultModifierFlags : GetCurrentModifierFlags());

				// this method should only be called in response to the user adding a new input key to an input alias. If the
				// input key was already part of the alias, then we shouldn't have been able to get here, so let's assert on this
				// condition.
				check(!ExistingAlias->LinkedInputKeys.ContainsItem(InputKeyMappingData));

				// finally, add it to the input alias
				ExistingAlias->LinkedInputKeys.AddItem(InputKeyMappingData);
				bRefreshInputKeyCache = TRUE;
			}
		}
	}

	if ( bRefreshInputKeyCache )
	{
		RebuildBoundInputKeyDataCache();
	}
}

/**
 * Removes a key from all of the selected alias's input maps for all states of the currently selected widgets.
 *
 * @param	KeyName		Key to remove from the widget's maps.
 */
void WxDlgUIEventKeyBindings::RemoveKeyFromStateMaps( FName& InputKeyName )
{
	UBOOL bRefreshInputKeyCache=FALSE;
	for ( INT SelectionIndex = 0; SelectionIndex < SelectedWidgets.Num(); SelectionIndex++ )
	{
		UUIScreenObject* SelectedWidget = SelectedWidgets(SelectionIndex);
		TArray<FUIInputAliasStateMap>* StateMaps = WidgetLookupMap.Find(SelectedWidget->GetClass());
		if( StateMaps != NULL )
		{
			// for each selected alias
			for ( INT SelectedAliasIndex = 0; SelectedAliasIndex < SelectedAliases.Num(); SelectedAliasIndex++ )
			{
				const FName& SelectedAliasName = SelectedAliases(SelectedAliasIndex);

				// Loop through all states, then for each state go through every alias and see if it matches the currently
				for(INT StateIdx = 0; StateIdx < StateMaps->Num(); StateIdx++)
				{
					FUIInputAliasStateMap& StateMap = (*StateMaps)(StateIdx);
					for(INT AliasIdx = 0; AliasIdx < StateMap.StateInputAliases.Num(); AliasIdx++)
					{
						FUIInputActionAlias& Alias = StateMap.StateInputAliases(AliasIdx);
						if( Alias.InputAliasName == SelectedAliasName )
						{
							// remove any entries from this alias for the specified input key name
							for ( INT MappedKeyIndex = Alias.LinkedInputKeys.Num() - 1; MappedKeyIndex >= 0; MappedKeyIndex-- )
							{
								if ( Alias.LinkedInputKeys(MappedKeyIndex).InputKeyName == InputKeyName )
								{
									Alias.LinkedInputKeys.Remove(MappedKeyIndex);
									bRefreshInputKeyCache = TRUE;
								}
							}

							// If this alias no longer has any keys bound to it, remove it from the state map.
							if( Alias.LinkedInputKeys.Num() == 0 )
							{
								StateMap.StateInputAliases.Remove(AliasIdx);
								bRefreshInputKeyCache = TRUE;
							}
								
							break;
						}
					}
				}
			}
		}
	}

	if ( bRefreshInputKeyCache )
	{
		RebuildBoundInputKeyDataCache();
	}
}

/** Updates the ModifierFlags value for the currently selected widget/input alias/input key's UIInputActionAlias */
void WxDlgUIEventKeyBindings::SetModifierBitmask( BYTE NewModifierFlagMask )
{
	UBOOL bRefreshInputKeyCache=FALSE;

	if ( SelectedWidgets.Num() > 0 && SelectedAliases.Num() > 0 && SelectedInputKey != NAME_None )
	{
		// first, validate that the flag 
		FInputKeyMappingData* AliasMappingData = MappedInputKeyData.Find(SelectedInputKey);
		check(AliasMappingData);

		UClass* StateClass = AliasMappingData->LinkedState;
		for ( INT SelectionIndex = 0; SelectionIndex < SelectedWidgets.Num(); SelectionIndex++ )
		{
			UUIScreenObject* SelectedWidget = SelectedWidgets(SelectionIndex);

			// Find the state map for this selected widget
			FUIInputAliasStateMap* StateMap = GetStateMapFromClass(SelectedWidget->GetClass(), StateClass);
			if ( StateMap != NULL )
			{
				// for each selected alias
				for ( INT SelectedAliasIndex = 0; SelectedAliasIndex < SelectedAliases.Num(); SelectedAliasIndex++ )
				{
					const FName& SelectedAliasName = SelectedAliases(SelectedAliasIndex);

					// find the UIInputActionAlias with this InputAliasName in the state's list of alias <==> input key mappings
					FUIInputActionAlias* ExistingAlias = NULL;
					for(INT AliasIdx = 0; AliasIdx < StateMap->StateInputAliases.Num(); AliasIdx++)
					{
						FUIInputActionAlias& Alias = StateMap->StateInputAliases(AliasIdx);
						if( Alias.InputAliasName == SelectedAliasName)
						{
							ExistingAlias = &Alias;
							break;
						}
					}

					// if we have a SelectedWidget, Alias name, and input key name, then we should already have an existing alias
					check(ExistingAlias);

					// now find the correct input alias mapping
					for ( INT KeyIndex = 0; KeyIndex < ExistingAlias->LinkedInputKeys.Num(); KeyIndex++ )
					{
						FRawInputKeyEventData& InputKeyData = ExistingAlias->LinkedInputKeys(KeyIndex);
						if ( InputKeyData.InputKeyName == SelectedInputKey )
						{
							bRefreshInputKeyCache = bRefreshInputKeyCache || InputKeyData.ModifierKeyFlags != NewModifierFlagMask;
							InputKeyData.ModifierKeyFlags = NewModifierFlagMask;
							break;
						}
					}
				}
			}
		}
	}

	if ( bRefreshInputKeyCache )
	{
		FName CurrentInputKey = SelectedInputKey;

		ClearBoundInputKeyData();
		PopulateBoundInputKeys();

		// find the index of the previously selected input key
		INT SelectionIndex = ListBoundInputKeys->FindItem(-1, *CurrentInputKey.ToString());
		if ( SelectionIndex != wxNOT_FOUND )
		{
			// now reselect the previously selected item in the BoundInputKeys list
			ListBoundInputKeys->SetItemState(SelectionIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		}
	}
}

/** Simple wrapper for updating all of the selection arrays */
void WxDlgUIEventKeyBindings::UpdateSelections()
{
	UpdateSelectedWidgets();
	UpdateSelectedAliases();
	SelectedInputKey = NAME_None;
}


/** Gets the list of keys mapped to the selected input aliases for the specified widget CDO */
void WxDlgUIEventKeyBindings::GetWidgetReverseKeyMap( UUIScreenObject* WidgetCDO, TMap<FRawInputKeyEventData,FStateAliasPair>& out_MappedKeys )
{
	check(WidgetCDO);
	UClass* WidgetClass = WidgetCDO->GetClass();

	// get the list of input alias <==> input key name maps for all of this widget's supported states
	TArray<FUIInputAliasStateMap>* StateInputAliasBindings = WidgetLookupMap.Find(WidgetClass);
	check(StateInputAliasBindings);

	out_MappedKeys.Empty();
	for ( INT StateIndex = 0; StateIndex < StateInputAliasBindings->Num(); StateIndex++ )
	{
		FUIInputAliasStateMap& StateInputAliasList = (*StateInputAliasBindings)(StateIndex);

		// grab the list of input alias <==> input key name mappings for this particular state
		TArray<FUIInputActionAlias>& InputAliasMappings = StateInputAliasList.StateInputAliases;
		for ( INT AliasIndex = 0; AliasIndex < InputAliasMappings.Num(); AliasIndex++ )
		{
			FUIInputActionAlias& InputAliasMapping = InputAliasMappings(AliasIndex);
			if ( SelectedAliases.ContainsItem(InputAliasMapping.InputAliasName) )
			{
				// found one - now we add each bound key to the list of bound keys and update the modifiers
				for ( INT KeyIndex = 0; KeyIndex < InputAliasMapping.LinkedInputKeys.Num(); KeyIndex++ )
				{
					FRawInputKeyEventData& MappedInputKey = InputAliasMapping.LinkedInputKeys(KeyIndex);
					FStateAliasPair* ExistingValue = out_MappedKeys.Find(MappedInputKey);
					if ( ExistingValue == NULL )
					{
						out_MappedKeys.Set(MappedInputKey, FStateAliasPair(InputAliasMapping.InputAliasName, StateInputAliasList.State));
					}
					else
					{
						// if this combination of input key name and modifier flags is already in the map, we can't show it in the list anyway so don't add it
					}
				}
			}
		}
	}
}

/** Retrieves the currently selected widgets */
void WxDlgUIEventKeyBindings::UpdateSelectedWidgets()
{
	SelectedWidgets.Empty();
	SelectedInputKey = NAME_None;

	wxArrayInt SelectedIndices;
	if ( ListWidgets->GetSelections(SelectedIndices) > 0 )
	{
		for ( UINT SelectionIndex = 0; SelectionIndex < SelectedIndices.GetCount(); SelectionIndex++ )
		{
			UUIScreenObject* Widget = (UUIScreenObject*)ListWidgets->GetClientData(SelectedIndices[SelectionIndex]);
			if ( Widget != NULL )
			{
				SelectedWidgets.AddUniqueItem(Widget);
			}
		}
	}
}

/** Updates the list of currently selected input aliases */
void WxDlgUIEventKeyBindings::UpdateSelectedAliases()
{
	SelectedAliases.Empty();
	SelectedInputKey = NAME_None;

	wxArrayInt SelectedIndices;
	if ( ListAliases->GetSelections(SelectedIndices) > 0 )
	{
		for ( UINT SelectionIndex = 0; SelectionIndex < SelectedIndices.GetCount(); SelectionIndex++ )
		{
			const FName InputAlias = ListAliases->GetString(SelectedIndices[SelectionIndex]).c_str();
			if ( InputAlias != NAME_None )
			{
				SelectedAliases.AddUniqueItem(InputAlias);
			}
		}
	}
}

IMPLEMENT_COMPARE_POINTER( UClass, WidgetClassInputKeyDialog, { return appStricmp( *A->GetDescription(), *B->GetDescription() ); } )
/** Adds all of the widgets available to the widget combobox. */
void WxDlgUIEventKeyBindings::PopulateWidgetClassList()
{
	ListWidgets->Freeze();
	{
		UClass* RootClass = UUIScreenObject::StaticClass();
		TArray<UClass*> WidgetClasses;
		WidgetClasses.AddItem(RootClass);

		for ( FObjectIterator It; It; ++It )
		{
			if ( It->IsA(RootClass) )
			{
				UUIScreenObject* Widget = static_cast<UUIScreenObject*>(*It);

				//@todo ronp: need support for specifying "input alias => raw input key" mappings for widget archetypes, 
				// but this will require changes to UIRoot's UIInputAliasStateMap & UIInputAliasClassMap structs.

				// only consider class default objects for now; eventually we'll also support archetypes

				// By ignoring classes which are abstract or not placeable, we filter out those widget classes which are
				// not intended to be interacted with by users, such as UIPrefab.  We then iterate up the widget's parent
				// chain so that we show all of the classes which can support input keys
				const UBOOL bValidResource = Widget != NULL && Widget->HasAnyFlags(RF_ClassDefaultObject)
					&& !Widget->GetClass()->HasAnyClassFlags(CLASS_Abstract|CLASS_HideDropDown);

				if ( bValidResource )
				{
					for ( UClass* WidgetClass = Widget->GetClass(); WidgetClass && WidgetClass != RootClass; WidgetClass = WidgetClass->GetSuperClass() )
					{
						WidgetClasses.AddUniqueItem(WidgetClass);
					}
				}
			}
		}

		Sort<USE_COMPARE_POINTER(UClass,WidgetClassInputKeyDialog)>(&WidgetClasses(0), WidgetClasses.Num());
		for ( INT ClassIndex = 0; ClassIndex < WidgetClasses.Num(); ClassIndex++ )
		{
			UClass* WidgetClass = WidgetClasses(ClassIndex);
			UUIScreenObject* WidgetCDO = WidgetClass->GetDefaultObject<UUIScreenObject>();

			ListWidgets->Append(*WidgetClass->GetDescription(), WidgetCDO);
			GenerateEventArrays(WidgetCDO);
		}
	}
	ListWidgets->Thaw();
	ListWidgets->Refresh();
}

/** Populates the EventList with the input aliases supported by the selected widgets */
void WxDlgUIEventKeyBindings::PopulateSupportedAliasList()
{
	if ( SelectedWidgets.Num() > 0 )
	{
		// build a list of input aliases which are supported by all selected widget classes

		UUIScreenObject* FirstWidget = SelectedWidgets(0);
		
		// start with the first widget - grab the list of support input alias names
		TArray<FName> CombinedInputAliases;
		FirstWidget->GetSupportedUIActionKeyNames(CombinedInputAliases);
		for ( INT ClassIndex = 1; CombinedInputAliases.Num() > 0 && ClassIndex < SelectedWidgets.Num(); ClassIndex++ )
		{
			TArray<FName> SupportedAliases;
			SelectedWidgets(ClassIndex)->GetSupportedUIActionKeyNames(SupportedAliases);

			for ( INT AliasIndex = CombinedInputAliases.Num() - 1; AliasIndex >= 0; AliasIndex-- )
			{
				const FName InputAlias = CombinedInputAliases(AliasIndex);

				// if this input alias isn't supported by one of the other selected widgets, remove it from the list
				if ( !SupportedAliases.ContainsItem(InputAlias) )
				{
					CombinedInputAliases.Remove(AliasIndex);
				}
			}
		}

		if ( CombinedInputAliases.Num() > 0 )
		{
			ListAliases->Freeze();
			{
				for ( INT AliasIndex = 0; AliasIndex < CombinedInputAliases.Num(); AliasIndex++ )
				{
					const FName AliasName = CombinedInputAliases(AliasIndex);
					ListAliases->Append(*AliasName.ToString());
				}
			}
			ListAliases->Thaw();
		}

		// purge any previously selected aliases which are no longer valid
		for ( INT AliasIndex = SelectedAliases.Num() - 1; AliasIndex >= 0; AliasIndex-- )
		{
			const FName InputAlias = SelectedAliases(AliasIndex);
			if ( !CombinedInputAliases.ContainsItem(InputAlias) )
			{
				// no longer in the list, remove it from the selection
				SelectedAliases.Remove(AliasIndex);
			}
		}
	}
}

/** Populates the StateList with the names of the UIStates supported by the selected widgets */
void WxDlgUIEventKeyBindings::PopulateSupportedStateList()
{
	StateNameToClassMap.Empty();
	if ( SelectedWidgets.Num() > 0 )
	{
		// generate a list of supported states using the first selected widget
		TArray<UClass*> CombinedSupportedStates = GetSupportedStates(SelectedWidgets(0));

		// then remove all states not supported by all other selected widgets
		for ( INT SelectionIndex = 1; CombinedSupportedStates.Num() > 0 && SelectionIndex < SelectedWidgets.Num(); SelectionIndex++ )
		{
			TArray<UClass*> SupportedStates = GetSupportedStates(SelectedWidgets(SelectionIndex));

			for ( INT StateIndex = CombinedSupportedStates.Num() - 1; StateIndex >= 0; StateIndex-- )
			{
				UClass* StateClass = CombinedSupportedStates(StateIndex);
				if ( !SupportedStates.ContainsItem(StateClass) )
				{
					CombinedSupportedStates.Remove(StateIndex);
				}
			}
		}

		// at this point, CombinedStateResources should now only contain those states supported by all selected widgets
		// so populate the list box
		if ( CombinedSupportedStates.Num() > 0 )
		{
			ListStates->Freeze();
			{
				for ( INT StateIndex = 0; StateIndex < CombinedSupportedStates.Num(); StateIndex++ )
				{
					UClass* StateClass = CombinedSupportedStates(StateIndex);
					const FString FriendlyName = StateClass->GetDescription();

					StateNameToClassMap.Set(*FriendlyName, StateClass);
					ListStates->Append(*FriendlyName);

				}
			}
			ListStates->Thaw();
		}
	}
}

/** Populates the BoundInputKeysList with the input keys bound to the currently selected input aliases */
void WxDlgUIEventKeyBindings::PopulateBoundInputKeys()
{
	if ( SelectedWidgets.Num() > 0 && SelectedAliases.Num() > 0 )
	{
		RebuildBoundInputKeyDataCache();
		ListBoundInputKeys->Freeze();
		{
			// finally, populate the list with the input key bindings
			for ( TMap<FName,FInputKeyMappingData>::TIterator It(MappedInputKeyData); It; ++It )
			{
				const FName& InputKeyName = It.Key();
				FInputKeyMappingData& MappingData = It.Value();

				// now add this item to the list
				INT item = ListBoundInputKeys->InsertItem( COLHEADER_KeyName, *InputKeyName.ToString() );
				if ( item != -1 )
				{
					ListBoundInputKeys->SetItem( item, COLHEADER_StateNames, *MappingData.LinkedState->GetDescription() );

					//@todo ronp localize
					ListBoundInputKeys->SetItem( item, COLHEADER_Modifiers,
						MappingData.ModifierFlags == DefaultModifierFlags ? TEXT("Default") : TEXT("Custom") );
				}
			}
		}

		int ColAutoSizeMode = wxLIST_AUTOSIZE;
		if ( ListBoundInputKeys->GetItemCount() == 0 )
		{
			ColAutoSizeMode = wxLIST_AUTOSIZE_USEHEADER;
		}

		ListBoundInputKeys->SetColumnWidth(0, ColAutoSizeMode);
		ListBoundInputKeys->SetColumnWidth(1, ColAutoSizeMode);
		ListBoundInputKeys->SetColumnWidth(2, ColAutoSizeMode);
		ListBoundInputKeys->Thaw();
	}
}

/** Updates the checked state of the ListStates list's checkboxes */
void WxDlgUIEventKeyBindings::UpdateStateChecklistValues()
{
	ListStates->Freeze();
	if ( SelectedInputKey != NAME_None )
	{
		FInputKeyMappingData* AliasMappingData = MappedInputKeyData.Find(SelectedInputKey);
		check(AliasMappingData);

		for ( UINT StateIndex = 0; StateIndex < ListStates->GetCount(); StateIndex++ )
		{
			const FString ElementStateFriendlyName = ListStates->GetString(StateIndex).c_str();
			UClass** StateClass = StateNameToClassMap.Find(ElementStateFriendlyName);
			check(StateClass);

			ListStates->Check(
				StateIndex,
				AliasMappingData->LinkedState == *StateClass ? true : false
				);
		}
	}
	else
	{
		for ( UINT StateIndex = 0; StateIndex < ListStates->GetCount(); StateIndex++ )
		{
			ListStates->Check(StateIndex, false);
		}
	}
	ListStates->Thaw();
}


/** Updates the checked state of all modifier checkboxes according to the currently selected items */
void WxDlgUIEventKeyBindings::UpdateModifierValues()
{
	if ( SelectedInputKey != NAME_None )
	{
		FInputKeyMappingData* MappingData = MappedInputKeyData.Find(SelectedInputKey);
		check(MappingData != NULL);

		for ( INT ModifierFlagIndex = 0; ModifierFlagIndex < INDEX_MAX; ModifierFlagIndex++ )
		{
			chk_ModifierFlag[ModifierFlagIndex]->SetValue((MappingData->ModifierFlags&(1<<ModifierFlagIndex))!=0);
		}
	}
	else
	{
		for ( INT ModifierFlagIndex = 0; ModifierFlagIndex < INDEX_MAX; ModifierFlagIndex++ )
		{
			chk_ModifierFlag[ModifierFlagIndex]->SetValue(false);
		}
	}
}

/** Regenerates the MappedInputKeyData cache */
void WxDlgUIEventKeyBindings::RebuildBoundInputKeyDataCache()
{
	MappedInputKeyData.Empty();
	if ( SelectedWidgets.Num() > 0 && SelectedAliases.Num() > 0 )
	{
		// first, we get the list of bound keys for the first selected widget
		TMap<FRawInputKeyEventData,FStateAliasPair> CombinedInputKeyMappings;
		GetWidgetReverseKeyMap(SelectedWidgets(0), CombinedInputKeyMappings);

		// next get the mapped key list for the remaining selected widgets and remove any keys which aren't mapped
		// or are mapped to different states in the other widgets.
		for ( INT WidgetIndex = 1; WidgetIndex < SelectedWidgets.Num(); WidgetIndex++ )
		{
			TMap<FRawInputKeyEventData,FStateAliasPair> InputKeyMappings;
			GetWidgetReverseKeyMap(SelectedWidgets(WidgetIndex), InputKeyMappings);

			for ( TMap<FRawInputKeyEventData,FStateAliasPair>::TIterator It(CombinedInputKeyMappings); It; ++It )
			{
				FStateAliasPair* MappedData = InputKeyMappings.Find(It.Key());
				if ( MappedData == NULL || (*MappedData) != It.Value() )
				{
					It.RemoveCurrent();
				}
			}
		}

		// at this point, CombinedInputKeyMappings should now only contain those input keys mapped exactly the same
		// by all selected widgets.  Now we convert this map into one that can be used to populate the list, and update
		// the state list and modifier checkboxes when a key is selected
		for ( TMap<FRawInputKeyEventData,FStateAliasPair>::TIterator It(CombinedInputKeyMappings); It; ++It )
		{
			const FRawInputKeyEventData& InputKeyData = It.Key();
			FStateAliasPair& AliasData = It.Value();

			// the GetWidgetReverseMap function is supposed to ignore duplicates, so we shouldn't have any at this point
			checkSlow(!MappedInputKeyData.HasKey(InputKeyData.InputKeyName));

			MappedInputKeyData.Set(InputKeyData.InputKeyName,
				FInputKeyMappingData(AliasData.AliasName, AliasData.AliasState, InputKeyData.ModifierKeyFlags));
		}
		MappedInputKeyData.Shrink();
	}

	UpdateAvailableKeys();
}

/**
 * Called when the list of bound input keys is invalidated; clears the list and all other
 * cached data related to the set of input keys bound to the selected aliases.
 */
void WxDlgUIEventKeyBindings::ClearBoundInputKeyData()
{
	// clear the cache of input key mappings
	MappedInputKeyData.Empty();

	// clear the list of input key bindings
	ListBoundInputKeys->DeleteAllItems();

	SelectedInputKey = NAME_None;
}

/** Updates the list of available keys */
void WxDlgUIEventKeyBindings::UpdateAvailableKeys()
{
	// reset the list of available keys
	AvailableKeys = KeyList;

	TArray<FName> UsedKeys;
	MappedInputKeyData.GenerateKeyArray(UsedKeys);
	for ( INT UsedIndex = 0; UsedIndex < UsedKeys.Num(); UsedIndex++ )
	{
		AvailableKeys.RemoveItem(UsedKeys(UsedIndex));
	}

#if REMOVE_UNASSOCIATED_KEYS
	// then remove any keys which can't possibly be bound given the currently selected widget, alias, and state

	// first, add all supported input event types to a list
	TMap<ESupportedInputEvents,UClass*> ValidInputEventTypes;
	for ( INT EventTypeIndex = 0; EventTypeIndex < SIE_Number; EventTypeIndex++ )
	{
		UClass* SuggestedStateClass = GetInputEventTypeRecommendedState((ESupportedInputEvents)EventTypeIndex);
		check(SuggestedStateClass);

		ValidInputEventTypes.Set((ESupportedInputEvents)EventTypeIndex, SuggestedStateClass);
	}

	// next, go through the selected widgets.  if any of the selected widgets' input alias mappings don't contain 
	// the state associated with a particular input event type, remove that input event type from the list
	for ( INT SelectionIndex = 0; SelectionIndex < SelectedWidgets.Num() && ValidInputEventTypes.Num(); SelectionIndex++ )
	{
		UClass* WidgetClass = SelectedWidgets(SelectionIndex);
		for ( TMap<ESupportedInputEvents,UClass*>::TIterator It(ValidInputEventTypes); It; ++It )
		{
			// if this widget class doesn't contain the state recommended for this key, remove the event type from the list
			if ( GetStateMapFromClass(WidgetClass, It.Value()) == NULL )
			{
				It.RemoveCurrent();
			}
		}
	}

	// finally, go through the list of available keys and remove any which have input event types that are associated with states not supported in all widgets
	for ( INT KeyIndex = AvailableKeys.Num() - 1; KeyIndex >= 0; KeyIndex-- )
	{
		const FName& CurrentKey = AvailableKeys(KeyIndex);
		INT* KeyEventTypeRef = KeyEventMap.Find(CurrentKey);
		if ( KeyEventTypeRef != NULL )
		{
			ESupportedInputEvents KeyEventType = (ESupportedInputEvents)(*KeyEventTypeRef);
			if ( !ValidInputEventTypes.HasKey(KeyEventType) )
			{
				AvailableKeys.Remove(KeyIndex);
			}
		}
	}
#endif
}

/** 
 * Generates a local version of the Unreal Key to UI Event Key mapping tables for editing. 
 *
 * @param	Widget	Widget to generate the mapping tables for.
 */
void WxDlgUIEventKeyBindings::GenerateEventArrays(UUIScreenObject* Widget)
{
	check(Widget);
	UClass* WidgetClass = Widget->GetClass();

	// get the list of input aliases supported by this widget class
	TArray<FName> SupportedEvents;
	Widget->GetSupportedUIActionKeyNames(SupportedEvents);

	UUIInteraction* DefaultUIInteraction = UUIRoot::GetDefaultUIController();
	check(DefaultUIInteraction);

	// Create a mapping from the widget's class <==> list of per-state input alias mappings
	TArray<FUIInputAliasStateMap>& ExistingStateMappings = WidgetLookupMap.Set(WidgetClass, TArray<FUIInputAliasStateMap>());

	// if this widget's class had some input alias mappings set in the .ini, initialize our working list with those.
	FUIInputAliasClassMap** StateMappingPtr = DefaultUIInteraction->WidgetInputAliasLookupTable.Find(WidgetClass);
	if(StateMappingPtr != NULL)
	{
		ExistingStateMappings = (*StateMappingPtr)->WidgetStates;
	}

	// make sure that the class <==> state input alias mapping for this widget's class has a list
	// of input key <==> input alias mappings
	TArray<UClass*> SupportedStates = GetSupportedStates(Widget);

	// now iterate through the list of states which will be displayed in the bind keys dialog
	for(INT SupportedIdx = 0; SupportedIdx < SupportedStates.Num(); SupportedIdx++)
	{
		// if the widget class doesn't have an entry for this state in its state <==> input alias list map,
		// we'll add one now.
		UBOOL bFoundMapping = FALSE;

		UClass* SupportedClass = SupportedStates(SupportedIdx);
		for( INT StateIdx = 0; StateIdx < ExistingStateMappings.Num(); StateIdx++)
		{
			if ( SupportedClass == ExistingStateMappings(StateIdx).State)
			{
				ExistingStateMappings(StateIdx).StateClassName = SupportedClass->GetPathName();
				bFoundMapping = TRUE;
				break;
			}
		}

		// didn't already have one, so add it now.
		if(bFoundMapping == FALSE)
		{
			INT ItemIdx = ExistingStateMappings.AddZeroed(1);
			
			ExistingStateMappings(ItemIdx).StateClassName = SupportedClass->GetPathName();
			ExistingStateMappings(ItemIdx).State = SupportedClass;
			ExistingStateMappings(ItemIdx).StateInputAliases.Empty();
		}
	}
}


/** Sorts the list based on which column was clicked. */
void WxDlgUIEventKeyBindings::OnClickListColumn(wxListEvent &Event)
{/*
	WxListView* List = wxDynamicCast(Event.GetEventObject(), WxListView);

	if(Event.GetId() == ID_UI_AVAILABLELIST)
	{
		// Make it so that if the user clicks on the same column twice, that it reverses the way the list is sorted.
		FSortOptions* Options = &SortOptions;

		if(Options->Column == Event.GetColumn())
		{
			Options->bSortAscending = !Options->bSortAscending;
		}
		else
		{
			Options->Column = Event.GetColumn();
		}

		List->SortItems(WxDlgUIEventKeyBindings_ListSort, (LONG)Options);
	}
*/

}

/**
 * Enables / disables the state list and modifier checkboxes depending on whether a valid input key is
 * selected.
 */
void WxDlgUIEventKeyBindings::OnUpdateOptionsPanel_UI( wxUpdateUIEvent& Event )
{
	Event.Enable(SelectedInputKey != NAME_None ? true : false);
}

/**
 * Enables/disables the "New Key" button and input key list depending on whether any input aliases
 * are selected
 */
void WxDlgUIEventKeyBindings::OnUpdateInputKeys_UI( wxUpdateUIEvent& Event )
{
	bool bShouldBeEnabled = SelectedAliases.Num() > 0 ? true : false;
	if ( bShouldBeEnabled && Event.GetId() == ID_UI_COMBO_CHOOSEPLATFORM )
	{
		bShouldBeEnabled = FConsoleSupportContainer::GetConsoleSupportContainer()->GetNumConsoleSupports() > 0;
	}

	Event.Enable(bShouldBeEnabled);
}

/** Called when the selected widget changes. */
void WxDlgUIEventKeyBindings::OnWidgetClassSelected(wxCommandEvent& Event)
{
	Freeze();

	// update the list of selected widgets
	UpdateSelections();

	// clear everything
	{
		// clear the list of supported input aliases
		ListAliases->Clear();

		// clear the list of supported states
		ListStates->Clear();

		ClearBoundInputKeyData();
	}

	// Repopulate the list of support input aliases; this will also deselect any input aliases which are no longer valid
	PopulateSupportedAliasList();

	// now re-select all the aliases which were previously selected
	UBOOL bSimulateAliasSelection = FALSE;
	for ( INT AliasIndex = 0; AliasIndex < SelectedAliases.Num(); AliasIndex++ )
	{
		const FName InputAlias = SelectedAliases(AliasIndex);

		// find the index for this alias in the list
		INT Index = ListAliases->FindString(*InputAlias.ToString());
		if ( Index != wxNOT_FOUND )
		{
			ListAliases->SetSelection(Index, true);
			bSimulateAliasSelection = TRUE;
		}
	}

	// Repopulate the list of supported states
	PopulateSupportedStateList();

	// now update the rest of the controls
	if ( bSimulateAliasSelection )
	{
		PopulateBoundInputKeys();
	}

	UpdateOptionsPanel();

	Thaw();
}


/** Called when the user selects an event in the event list. */
void WxDlgUIEventKeyBindings::OnAliasListSelected(wxCommandEvent &Event)
{
	// update the global cache of selected aliases
	UpdateSelectedAliases();

	// the set of bound input keys is no longer valid, so clear out all data related to them
	ClearBoundInputKeyData();

	// then rebuild the key map cache and repopulate the input key list
	PopulateBoundInputKeys();

	if ( ListBoundInputKeys->GetItemCount() > 0 )
	{
		if ( ListBoundInputKeys->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED) )
		{
			SelectedInputKey = ListBoundInputKeys->GetItemText(0).c_str();
		}
	}

	UpdateOptionsPanel();
}

/** called when the user selects an item in the bound keys list */
void WxDlgUIEventKeyBindings::OnInputKeyList_Selected( wxListEvent& Event )
{
	SelectedInputKey = Event.GetText().c_str();
	UpdateOptionsPanel();
}

/** called when the user selects an item in the bound keys list */
void WxDlgUIEventKeyBindings::OnInputKeyList_Deselected( wxListEvent& Event )
{
	SelectedInputKey = NAME_None;
	UpdateOptionsPanel();
}

/** called when the user right-clicks on the ListBoundInputKeys list */
void WxDlgUIEventKeyBindings::OnInputKeyList_RightClick( wxListEvent& Event )
{
	wxMenu ContextMenu;

	ContextMenu.Append(ID_UI_BIND_NEW_INPUT_KEY, *LocalizeUI(TEXT("DlgUIEventKeyBindings_MenuItem_NewKey")));
	if ( SelectedInputKey != NAME_None )
	{
		ContextMenu.Append(ID_UI_REMOVE_BOUND_INPUT_KEY, *LocalizeUI(TEXT("DlgUIEventKeyBindings_MenuItem_RemoveKey")));
	}

	FTrackPopupMenu tpm(this, &ContextMenu);
	tpm.Show();
}

/** called when the user presses a key with the bound input key list focused */
void WxDlgUIEventKeyBindings::OnInputKeyList_KeyDown( wxListEvent& Event )
{
	int KeyCode = Event.GetKeyCode();
	if ( (KeyCode == WXK_DELETE || KeyCode == WXK_BACK) && SelectedInputKey != NAME_None )
	{
		RemoveKeyFromStateMaps(SelectedInputKey);
		ClearBoundInputKeyData();
		PopulateBoundInputKeys();
		UpdateOptionsPanel();
	}
	else
	{
		Event.Skip();
	}
}

/** Called when the user checks/unchecks a state in the state list */
void WxDlgUIEventKeyBindings::OnStateCheckToggled( wxCommandEvent& Event )
{
	// first, find the UClass* corresponding to the state that was checked
	const INT SelectionIndex = Event.GetInt();
	check(SelectionIndex >= 0);
	check(SelectionIndex < static_cast<INT>(ListStates->GetCount()));

	const FString StateFriendlyName = ListStates->GetString(SelectionIndex).c_str();
	UClass** pStateClass = StateNameToClassMap.Find(StateFriendlyName);
	check(pStateClass);
	const UClass* StateClass = *pStateClass;

	// Add/Remove the key for this row from the list of supported keys for this alias based on the new value of the checkbox.
	const UBOOL bSelected = ListStates->IsChecked(SelectionIndex);

	// shouldn't be here unless we actually have an input selected.
	check(SelectedInputKey != NAME_None);

	if ( bSelected )
	{
		RemoveKeyFromStateMaps(SelectedInputKey);
		AddKeyToStateMap(SelectedInputKey, StateClass);
	}
	else
	{
		UBOOL bRefreshInputKeyCache=FALSE;

		TArray<FUIInputActionAlias*> AliasMappings = GetSelectedActionAliasMaps(StateClass);
		for ( INT AliasIndex = 0; AliasIndex < AliasMappings.Num(); AliasIndex++ )
		{
			FUIInputActionAlias* Alias = AliasMappings(AliasIndex);
			for ( INT InputKeyIndex = Alias->LinkedInputKeys.Num() - 1; InputKeyIndex >= 0; InputKeyIndex-- )
			{
				if ( Alias->LinkedInputKeys(InputKeyIndex).InputKeyName == SelectedInputKey )
				{
					Alias->LinkedInputKeys.Remove(InputKeyIndex);
					bRefreshInputKeyCache = TRUE;
				}
			}
		}

		if ( bRefreshInputKeyCache )
		{
			PopulateBoundInputKeys();
		}
	}

	UpdateStateChecklistValues();
}

/** called when on of the modifier checkboxes is checked/unchecked */
void WxDlgUIEventKeyBindings::OnModifierChecked( wxCommandEvent& Event )
{
	const INT ClickedId = Event.GetId();
	const UBOOL bIsChecked = Event.IsChecked();

	check(INDEX_MAX < 8);
	for ( BYTE CheckboxIndex = 0; CheckboxIndex < INDEX_MAX; CheckboxIndex++ )
	{
		if ( chk_ModifierFlag[CheckboxIndex]->GetId() == ClickedId )
		{
			BYTE CurrentModifierFlagMask = GetCurrentModifierFlags();
			const BYTE FlagToUpdate = 1 << CheckboxIndex;

			if ( bIsChecked )
			{
				// OR in in the new flag
				CurrentModifierFlagMask |= FlagToUpdate;

				// then verify that we don't have a conflict between mutually exclusive flags
				switch ( FlagToUpdate )
				{
				case KEYMODIFIER_AltRequired:
					// if AltActive was checked, make sure we clear AltExcluded
					CurrentModifierFlagMask &= ~KEYMODIFIER_AltExcluded;
					break;
				case KEYMODIFIER_CtrlRequired:
					// if CtrlActive was checked, make sure we clear CtrlExcluded
					CurrentModifierFlagMask &= ~KEYMODIFIER_CtrlExcluded;
					break;
				case KEYMODIFIER_ShiftRequired:
					// if ShiftActive was checked, make sure we clear ShiftExcluded
					CurrentModifierFlagMask &= ~KEYMODIFIER_ShiftExcluded;
					break;
				case KEYMODIFIER_AltExcluded:
					// if AltExcluded was checked, make sure we clear AltActive
					CurrentModifierFlagMask &= ~KEYMODIFIER_AltRequired;
					break;
				case KEYMODIFIER_CtrlExcluded:
					// if CtrlExcluded was checked, make sure we clear CtrlActive
					CurrentModifierFlagMask &= ~KEYMODIFIER_CtrlRequired;
					break;
				case KEYMODIFIER_ShiftExcluded:
					// if ShiftExcluded was checked, make sure we clear ShiftActive
					CurrentModifierFlagMask &= ~KEYMODIFIER_ShiftRequired;
					break;
				}
			}
			else
			{
				CurrentModifierFlagMask &= ~FlagToUpdate;
			}

			SetModifierBitmask(CurrentModifierFlagMask);
			break;
		}
	}
}


/**
 * Returns a pointer to the state class most appropriate to handle input events for the specified input key.  If not all selected widgets support
 * the most appropriate state (and REMOVE_UNASSOCIATED_KEYS is 0), returns NULL.
 */
UClass* WxDlgUIEventKeyBindings::GetRecommendedStateClass( const FName& SelectedKey )
{
	UClass* RecommendedState = NULL;

	if ( SelectedKey != NAME_None )
	{
		// Add the key to our local state alias map depending on the type of key we are adding.
		INT* EventTypePtr = KeyEventMap.Find(SelectedKey);
		if(EventTypePtr != NULL)
		{
			ESupportedInputEvents EventType = (ESupportedInputEvents)(*EventTypePtr);

			// Depending on the input key type we received, check different states.
			switch(EventType)
			{
			case SIE_MouseButton:
				// Add mouse button clicks to the active state.
				RecommendedState = UUIState_Active::StaticClass();
				break;
			case SIE_Keyboard: 
			case SIE_PressedOnly: 
				// Add keyboard presses to the focused state.
				RecommendedState = UUIState_Focused::StaticClass();
				break;
			case SIE_Axis:
				// Add axis events to the pressed state.
				RecommendedState = UUIState_Pressed::StaticClass();
				break;
			}
		}
	}

#if !REMOVE_UNASSOCIATED_KEYS
	// make sure this state is supported by all selected widget classes
	// next, go through the selected widgets.  if any of the selected widgets' input alias mappings don't contain 
	// the state associated with a particular input event type, remove that input event type from the list
	for ( INT SelectionIndex = 0; SelectionIndex < SelectedWidgets.Num(); SelectionIndex++ )
	{
		UClass* WidgetClass = SelectedWidgets(SelectionIndex)->GetClass();
		if ( GetStateMapFromClass(WidgetClass, RecommendedState) == NULL )
		{
			// not supported by this widget class - clear the result
			RecommendedState = NULL;
		}
	}
#endif

	return RecommendedState;
}

/**
 * Displays the "Choose input key" dialog, then adds the selected key to the list of bound keys.
 *
 * @return	the key that the user selected, or NAME_None if the user didn't choose a key or the key couldn't be added to the input key list
 */
FName WxDlgUIEventKeyBindings::ShowInputKeySelectionDialog()
{
	FName Result = NAME_None;

	WxDlgChooseInputKey dlg(this);
	if ( dlg.ShowModal(AvailableKeys) == wxID_OK )
	{
		FName SelectedKey = dlg.GetSelectedKey();
		if ( SelectedKey != NAME_None )
		{
			// Add the key to our local state alias map depending on the type of key we are adding.
			UClass* StateClass = GetRecommendedStateClass(SelectedKey);

#if !REMOVE_UNASSOCIATED_KEYS
			if ( StateClass == NULL )
			{
				// if StateClass is NULL, then the recommended state for the selected key isn't supported by all widgets
				// so default back to the enabled state instead.
				StateClass = UUIState_Enabled::StaticClass();
			}
#endif

			if(StateClass != NULL)
			{
				UBOOL bCancel=FALSE;
				for ( INT SelectionIndex = 0; SelectionIndex < SelectedWidgets.Num(); SelectionIndex++ )
				{
					UUIScreenObject* WidgetCDO = SelectedWidgets(SelectionIndex);
					TArray<FName> AliasesContainingSelectedKey;
					
					if ( FindInputKeyMappings(WidgetCDO, StateClass, FRawInputKeyEventData(SelectedKey), AliasesContainingSelectedKey) )
					{
						FString AliasString;
						for ( INT AliasIndex = 0; AliasIndex < AliasesContainingSelectedKey.Num(); AliasIndex++ )
						{
							if ( AliasString.Len() > 0 )
							{
								AliasString += TEXT(", ");
							}

							AliasString += AliasesContainingSelectedKey(AliasIndex).ToString();
						}

						if ( !appMsgf(AMT_YesNo, LocalizeSecure(LocalizeUI(TEXT("DlgUIEventKeyBindings_AddKey_ExistingBindingsError")),
							*SelectedKey.ToString(), *WidgetCDO->GetClass()->GetDescription(), *AliasString)) )
						{
							bCancel = TRUE;
							break;
						}
					}
				}

				if ( !bCancel )
				{
					AddKeyToStateMap(SelectedKey, StateClass, TRUE);

					// clear out all cached data related to the currently bound input keys
					ClearBoundInputKeyData();

					// then rebuild the key map cache and repopulate the input key list
					PopulateBoundInputKeys();

					Result = SelectedKey;
				}
			}
		}
	}

	return Result;
}

/**
 * Called when the user clicks the "New Key" button - shows a dialog which allows
 * them to choose a key from the list of all available keys
 */
void WxDlgUIEventKeyBindings::OnNewKeyButton( wxCommandEvent& Event )
{
	FName SelectedKey = ShowInputKeySelectionDialog();
	if ( SelectedKey != NAME_None )
	{
		// find the index of the previously selected input key
		INT SelectionIndex = ListBoundInputKeys->FindItem(-1, *SelectedKey.ToString());
		if ( SelectionIndex != wxNOT_FOUND )
		{
			// now reselect the previously selected item in the BoundInputKeys list
			if ( ListBoundInputKeys->SetItemState(SelectionIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED) )
			{
				SelectedInputKey = SelectedKey;
			}
		}

		UpdateOptionsPanel();
	}
}

/** Called when the user chooses to remove the current input key from the alias mappings */
void WxDlgUIEventKeyBindings::OnRemoveInputKey( wxCommandEvent& Event )
{
	if ( SelectedInputKey != NAME_None )
	{
		RemoveKeyFromStateMaps(SelectedInputKey);
		
		ClearBoundInputKeyData();

		PopulateBoundInputKeys();
		UpdateOptionsPanel();
	}
	else
	{
		Event.Skip();
	}
}

/** Called when the user double-clicks an item or presses enter with an item selected */
void WxDlgUIEventKeyBindings::OnInputKeyList_ReplaceKey( wxListEvent& Event )
{
	if ( SelectedInputKey != NAME_None )
	{
		FName PreviouslySelectedKey = SelectedInputKey;
		FName SelectedKey = ShowInputKeySelectionDialog();
		if ( SelectedKey != NAME_None )
		{
			// remove this key from the widget's lists of input aliases for all states
			RemoveKeyFromStateMaps(PreviouslySelectedKey);

			// clear the input key list, the SelectedInputKey, and the input key data cache
			ClearBoundInputKeyData();

			// repopulate the input key list and the input key data cache
			PopulateBoundInputKeys();

			// find the index of the previously selected input key
			INT SelectionIndex = ListBoundInputKeys->FindItem(-1, *SelectedKey.ToString());
			if ( SelectionIndex != wxNOT_FOUND )
			{
				// now reselect the previously selected item in the BoundInputKeys list
				if ( ListBoundInputKeys->SetItemState(SelectionIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED) )
				{
					SelectedInputKey = SelectedKey;
				}
			}

			UpdateOptionsPanel();
		}
	}
}

/** Called when the user chooses a different value from the platform combo */
void WxDlgUIEventKeyBindings::OnPlatformSelected( wxCommandEvent& Event )
{
	FString NewSelectedPlatform = combo_Platform->GetValue().c_str();
	if ( NewSelectedPlatform.Len() && NewSelectedPlatform != CurrentPlatformName )
	{
		// first, save the input alias bindings for the current platform
		if ( SaveSpecifiedPlatform(CurrentPlatformName) )
		{
			// Send a notification that we changed the default widget UI key bindings.
			GCallbackEvent->Send(CALLBACK_UIEditor_WidgetUIKeyBindingChanged);
		}

		// switch the CurrentPlatformName and load its alias bindings from the corresponding ini
		if ( LoadSpecifiedPlatformValues(NewSelectedPlatform) )
		{
			CurrentPlatformName = NewSelectedPlatform;
		}
		else
		{
			combo_Platform->SetStringSelection(*CurrentPlatformName);

			appMsgf(AMT_OK, TEXT("Failed to load input configuration for platform %s"), *CurrentPlatformName);
		}

	}
}

/**
 * Searches all widget input alias bindings and searches for input keys/modifier combinations that are bound to more than one input
 * alias for a single widget's state.  If duplicates are found, prompts the user for confirmation to continue.
 *
 * @return	TRUE if there were no duplicate bindings or the user chose to ignore them.  FALSE if duplicate bindings were detected and
 *			the user wants to correct them.
 */
UBOOL WxDlgUIEventKeyBindings::VerifyUniqueBindings()
{
	UBOOL bResult = TRUE;

	// first, assemble the list of available states
	TArray<UClass*> SearchStates;
	for ( UINT StateIndex = 0; StateIndex < ListStates->GetCount(); StateIndex++ )
	{
		const FString ElementStateFriendlyName = ListStates->GetString(StateIndex).c_str();
		UClass** StateClass = StateNameToClassMap.Find(ElementStateFriendlyName);
		if ( StateClass != NULL )
		{
			SearchStates.AddItem(*StateClass);
		}
	}

	// next, iterate over all widget classes
	TMap<UUIScreenObject*, TMap<UClass*, TMultiMap<FName, FName> > > Conflicts;
	for ( UINT ClassIndex = 0; ClassIndex < ListWidgets->GetCount(); ClassIndex++ )
	{
		UUIScreenObject* WidgetCDO = static_cast<UUIScreenObject*>(ListWidgets->GetClientData(ClassIndex));
		for ( INT StateIndex = 0; StateIndex < SearchStates.Num(); StateIndex++ )
		{
			// lookup the input alias map for each available state
			UClass* StateClass = SearchStates(StateIndex);
			FUIInputAliasStateMap* StateMap = GetStateMapFromClass(WidgetCDO->GetClass(), StateClass);
			if ( StateMap != NULL )
			{
				TMultiMap<FName,FName> StateConflicts;
				// iterate through each input alias in this state's alias map
				for ( INT AliasIndex = 0; AliasIndex < StateMap->StateInputAliases.Num(); AliasIndex++ )
				{
					FUIInputActionAlias& Alias = StateMap->StateInputAliases(AliasIndex);

					// for each input key bound to this alias, 
					for ( INT KeyIndex = 0; KeyIndex < Alias.LinkedInputKeys.Num(); KeyIndex++ )
					{
						FRawInputKeyEventData& CheckKey = Alias.LinkedInputKeys(KeyIndex);
						TArray<FName> DuplicateBindings;

						// find all input aliases in this state which are currently bound to this input key
						if ( FindInputKeyMappings(WidgetCDO, StateClass, CheckKey, DuplicateBindings) )
						{
							// remove the one we're currently checking
							DuplicateBindings.RemoveItem(Alias.InputAliasName);

							// add any remaining input aliases to the list of duplicates found in this state
							for ( INT DupIndex = 0; DupIndex < DuplicateBindings.Num(); DupIndex++ )
							{
								StateConflicts.Add(CheckKey.InputKeyName, DuplicateBindings(DupIndex));
							}
						}
					}
				}

				// if this state contained duplicate input alias bindings, add that data to the global list
				TArray<FName> InputKeyNames;
				if ( StateConflicts.Num() > 0 )
				{
					TMap<UClass*, TMultiMap<FName, FName> >* WidgetConflictMap = Conflicts.Find(WidgetCDO);
					if ( WidgetConflictMap == NULL )
					{
						WidgetConflictMap = &Conflicts.Set(WidgetCDO, TMap<UClass*, TMultiMap<FName, FName> >());
					}

					TMultiMap<FName,FName>* StateConflictMap = WidgetConflictMap->Find(StateClass);
					if ( StateConflictMap == NULL )
					{
						StateConflictMap = &WidgetConflictMap->Set(StateClass, TMultiMap<FName,FName>());
					}

					// copy the state's duplicates into the master map
					*StateConflictMap = StateConflicts;
				}
			}
		}
	}

	// if duplicates were detected, build an error message containing the widget, state, and bound input aliases for
	// each input key that is assigned to multiple input aliases
	if ( Conflicts.Num() > 0 )
	{
		FString ConflictString;
		for ( TMap<UUIScreenObject*, TMap<UClass*, TMultiMap<FName, FName> > >::TIterator WidgetIt(Conflicts); WidgetIt; ++WidgetIt )
		{
			UUIScreenObject* WidgetCDO = WidgetIt.Key();

			ConflictString += WidgetCDO->GetClass()->GetDescription() + TEXT(":") + LINE_TERMINATOR;

			TMap<UClass*, TMultiMap<FName, FName> >& StateConflictMap = WidgetIt.Value();
			for ( TMap<UClass*, TMultiMap<FName, FName> >::TIterator StateIt(StateConflictMap); StateIt; ++StateIt )
			{
				UClass* StateClass = StateIt.Key();
				TMultiMap<FName, FName>& BindingMap = StateIt.Value();

				ConflictString += FString::Printf(TEXT("<< %s >>") LINE_TERMINATOR, *StateClass->GetDescription());

				TLookupMap<FName> InputKeyNames;
				BindingMap.GetKeys(InputKeyNames);

				for ( INT KeyIndex = 0; KeyIndex < InputKeyNames.Num(); KeyIndex++ )
				{
					const FName& InputKeyName = InputKeyNames(KeyIndex);
					TArray<FName> DuplicateAliasNames;
					FString DupMappingString;

					BindingMap.MultiFind(InputKeyName, DuplicateAliasNames);
					for ( INT DupIndex = 0; DupIndex < DuplicateAliasNames.Num(); DupIndex++ )
					{
						FName& AliasName = DuplicateAliasNames(DupIndex);
						if ( DupMappingString.Len() > 0 )
						{
							DupMappingString += TEXT(", ");
						}

						DupMappingString += AliasName.ToString();
					}

					ConflictString += FString::Printf(TEXT("%s: %s") LINE_TERMINATOR, *InputKeyName.ToString(), *DupMappingString);
				}

				ConflictString += LINE_TERMINATOR;
			}

			ConflictString += LINE_TERMINATOR;
		}

		FString ErrorMessage = FString::Printf(LocalizeSecure(LocalizeUI(TEXT("DlgUIEventKeyBindings_CloseDialog_ExistingBindingsError")), *ConflictString));
		debugf(*ErrorMessage);
		bResult = appMsgf(AMT_YesNo, *ErrorMessage);
	}

	return bResult;
}

/** When the user clicks the OK button, we create/remove output links for the sequence object based on the state of the lists. */
void WxDlgUIEventKeyBindings::OnOK(wxCommandEvent &Event)
{
	// verify that we don't have any conflicting alias bindings
	if ( !VerifyUniqueBindings() )
	{
		return;
	}

	FString SelectedPlatform = combo_Platform->GetValue().c_str();

	// quick sanity check to identify siutations where the value in memory is out of sync with the selected value
	checkSlow(SelectedPlatform == CurrentPlatformName);

	if ( SaveSpecifiedPlatform(CurrentPlatformName) )
	{
		// Send a notification that we changed the default widget UI key bindings.
		GCallbackEvent->Send(CALLBACK_UIEditor_WidgetUIKeyBindingChanged);
	}

	// Finish the dialog.
	EndModal(wxID_OK);
}

/**
 * Returns the filename to the default input.ini file for the specified platform.
 */
FString WxDlgUIEventKeyBindings::GetDefaultIniFilename( const FString& PlatformName )
{
	FString IniPrefix;
	if ( PlatformName == CONSOLESUPPORT_NAME_PC )
	{
		IniPrefix = PC_DEFAULT_INI_PREFIX;
	}
	else if ( PlatformName == CONSOLESUPPORT_NAME_360 )
	{
		IniPrefix = XENON_DEFAULT_INI_PREFIX;
	}
	else if ( PlatformName == TEXT("Linux") )
	{
		IniPrefix = PC_DEFAULT_INI_PREFIX;
	}
	else if ( PlatformName == CONSOLESUPPORT_NAME_PS3 )
	{
		IniPrefix = PS3_DEFAULT_INI_PREFIX;
	}
	else
	{
		appMsgf(AMT_OK, TEXT("Unhandled PlatformName in WxDlgUIEventKeyBindings::GetDefaultIniFilename: %s"), *PlatformName);
	}

	if ( IniPrefix.Len() > 0 )
	{
		return appGameConfigDir() * IniPrefix + TEXT("Input.ini");
	}

	return TEXT("");
}

/**
 * Reads the input alias mappings from the default .ini for the specified platform and applies the new mappings to the UIInputConfiguration singleton.
 *
 * @param	PlatformName	the name of the platform (as returned from the FConsoleSupport::GetConsoleName object for that platform), or PC.
 *
 * @return	TRUE if the new values were successfully loaded and applied
 */
UBOOL WxDlgUIEventKeyBindings::LoadSpecifiedPlatformValues( const FString& PlatformName )
{
	UBOOL bSuccessfulLoad = FALSE;

	// Get the default INI file name for this game for the specified platform
	FString DefaultIniFilename = GetDefaultIniFilename(PlatformName);
	if ( DefaultIniFilename.Len() > 0 )
	{
		// build a new .ini file for the specified platform
		FConfigFile PlatformInputIni;
		LoadAnIniFile(*DefaultIniFilename, PlatformInputIni, FALSE);

// 		debugf(TEXT("PlatformName: %s  DefaultIniFilename: %s"), *PlatformName, *DefaultIniFilename);
// 		PlatformInputIni.Dump(*GLog);

		// add it to the config cache so that LoadConfig() can find it
		static_cast<FConfigCacheIni*>(GConfig)->Set(TEXT("PlatformInput.ini"), PlatformInputIni);

		// get the input configuration singleton
		UUIInteraction* DefaultUIController = UUIRoot::GetDefaultUIController();
		check(DefaultUIController);

		UUIInputConfiguration* InputConfig = DefaultUIController->GetInputSettings();
		check(InputConfig);

		// tell it to reload its values from the constructed config file
		InputConfig->ReloadConfig(NULL, TEXT("PlatformInput.ini"));

		// now re-initialize everything:
		// reload the widget and state class references stored in the ini
		InputConfig->LoadInputAliasClasses();
// 		UObject::StaticExec( *FString::Printf(TEXT("OBJ DUMP %s"), *InputConfig->GetPathName()), *GLog );

		// re-initialize the UIController's runtime lookup table.
		DefaultUIController->InitializeInputAliasLookupTable();

		// rebuild the cached data used by this dialog's operation
		WidgetLookupMap.Empty();
		for ( UINT ClassIndex = 0; ClassIndex < ListWidgets->GetCount(); ClassIndex++ )
		{
			UUIScreenObject* WidgetCDO = static_cast<UUIScreenObject*>(ListWidgets->GetClientData(ClassIndex));
			GenerateEventArrays(WidgetCDO);
		}

		// update everything in the dialog by simulating the user changing the current widget class
		wxCommandEvent ResetEvent(wxEVT_COMMAND_LISTBOX_SELECTED, ID_UI_WIDGETLIST);
		ProcessEvent(ResetEvent);

		bSuccessfulLoad = TRUE;
	}

	return bSuccessfulLoad;
}

/**
 * Saves the input alias mappings to the default ini for the currently selected platform
 *
 * @param	PlatformName	the name of the platform (as returned from the FConsoleSupport::GetConsoleName object for that platform), or PC.
 *
 * @return	TRUE if the save was successful, FALSE if the file was read-only or couldn't be written for some reason
 */
UBOOL WxDlgUIEventKeyBindings::SaveSpecifiedPlatform( const FString& PlatformName )
{
	UBOOL bSaveSuccessful = FALSE;

	// Get the default INI file name for this game for the specified platform
	FString DefaultIniFilename = GetDefaultIniFilename(PlatformName);
	if ( DefaultIniFilename.Len() > 0 )
	{
		// Make sure the file is writeable
		UBOOL bReadOnly = GFileManager->IsReadOnly(*DefaultIniFilename);
		if( bReadOnly )
		{
#if HAVE_SCC
			if ( GApp->SCC )
			{
				// if not, ask the user if they want to check it out of perforce.
				if( appMsgf(AMT_YesNo, LocalizeSecure(LocalizeUnrealEd(TEXT("CheckoutPerforce")), *DefaultIniFilename)) )
				{
					// open a connection to the perforce server
					if( !GApp->SCC->IsOpen() )
					{
						GApp->SCC->Open();
					}

					if( GApp->SCC->IsOpen() )
					{
						GApp->SCC->CheckOut(*DefaultIniFilename);

						// if the check-out was successful, unset bReadOnly
						bReadOnly = GFileManager->IsReadOnly(*DefaultIniFilename);
					}
				}
			}
#endif // HAVE_SCC

			if( bReadOnly )
			{
				wxMessageBox( *FString::Printf(LocalizeSecure(LocalizeUI(TEXT("DlgUIEventKeyBindings_ReadOnlyError")), *DefaultIniFilename)), *LocalizeUnrealEd("Error"), wxICON_ERROR);
			}
		}

		if ( !bReadOnly )
		{
// =============================================================================================
// rebuild the array of class->input key alias mappings stored in the UIInputConfiguration object
//==============================================================================================

			// Copy the key mapping arrays back to the 'global' mapping array stored in UUIInteraction.
			// We loop through every state for the widget, and if it has any key aliases bound to it, copy it back
			// over to the final array.
			UUIInteraction* DefaultUIInteraction = UUIRoot::GetDefaultUIController();
			check(DefaultUIInteraction);

			UUIInputConfiguration* InputConfig = DefaultUIInteraction->GetInputSettings();
			check(InputConfig);

			InputConfig->WidgetInputAliases.Empty();
			for(FClassToUIKeyMap::TIterator It(WidgetLookupMap); It; ++It)
			{
				// Loop through all states and see if they should be added to the state map array.
				TArray<INT> ValidStateIndexes;

				const TArray<FUIInputAliasStateMap>& StateMaps = It.Value();
				for(INT StateIdx = 0; StateIdx < StateMaps.Num(); StateIdx++)
				{
					const FUIInputAliasStateMap& StateMap = StateMaps(StateIdx);
					if ( StateMap.StateInputAliases.Num() )
					{
						for ( INT AliasIndex = 0; AliasIndex < StateMap.StateInputAliases.Num(); AliasIndex++ )
						{
							if ( StateMap.StateInputAliases(AliasIndex).LinkedInputKeys.Num() > 0 )
							{
								ValidStateIndexes.AddItem(StateIdx);
								break;
							}
						}
					}
				}

				// If we have at least 1 valid state with some keys mapped to it, then go ahead and create a entry in the
				// final array for this widget class.
				if(ValidStateIndexes.Num() > 0)
				{
					// Create a entry for this widget class.
					UClass* ObjectClass = It.Key();
					INT ItemIdx = InputConfig->WidgetInputAliases.AddZeroed(1);
					FUIInputAliasClassMap& WidgetKeyMapping = InputConfig->WidgetInputAliases(ItemIdx);
					WidgetKeyMapping.WidgetClass = ObjectClass;
					WidgetKeyMapping.WidgetClassName = ObjectClass->GetPathName();
					WidgetKeyMapping.WidgetStates.AddZeroed(ValidStateIndexes.Num());

					// Finally go through and add all of the states to the StateMap for this widget.
					for(INT StateIdx = 0; StateIdx < ValidStateIndexes.Num(); StateIdx++)
					{
						const FUIInputAliasStateMap& StateMap = StateMaps(ValidStateIndexes(StateIdx));
						WidgetKeyMapping.WidgetStates(StateIdx) = StateMap;
					}
				}
			}

// =============================================================================================
// tell the UIInteraction object to refresh its alias mappings based on the new values
//==============================================================================================

			// save the current number of classes that have aliases, as InitializeInputAliasLookupTable will add empty entries for any missing classes
			const INT NumClassesToExport = InputConfig->WidgetInputAliases.Num();

			// Force the interaction to reinitialize the lookup maps.
			DefaultUIInteraction->InitializeInputAliasLookupTable();

// =============================================================================================
// save the new bindings to the default .ini file
//==============================================================================================

			// Generate a array of strings, each line being a unique item in our WidgetInputAliases array.
			UArrayProperty* DefaultEventKeyProperty = FindField<UArrayProperty>(UUIInputConfiguration::StaticClass(), TEXT("WidgetInputAliases"));
			check(DefaultEventKeyProperty);

			UScriptStruct* InputAliasStruct = CastChecked<UStructProperty>(DefaultEventKeyProperty->Inner)->Struct;
			BYTE* StructDefaults = InputAliasStruct ? InputAliasStruct->GetDefaults() : NULL;

			TArray<FString> ArrayStrings;
			ArrayStrings.AddZeroed(NumClassesToExport);

			TArray<FUIInputAliasClassMap>* Array = (TArray<FUIInputAliasClassMap>*)((BYTE*)InputConfig + DefaultEventKeyProperty->Offset);
			for(INT WidgetIdx = 0; WidgetIdx < NumClassesToExport; WidgetIdx++)
			{
				FUIInputAliasClassMap& ClassAliases = (*Array)(WidgetIdx);
				FString& OutputString = ArrayStrings(WidgetIdx);
				BYTE* ElementValue = (BYTE*)Array->GetData() + DefaultEventKeyProperty->Inner->ElementSize * WidgetIdx;

				BYTE* DefaultElementValue = NULL;
				UClass* WidgetSuperClass = ClassAliases.WidgetClass->GetSuperClass();
				if ( WidgetSuperClass->IsChildOf(UUIScreenObject::StaticClass()) )
				{
					// find the element corresponding to this class's super
					for ( INT SearchIndex = 0; SearchIndex < NumClassesToExport; SearchIndex++ )
					{
						FUIInputAliasClassMap& ClassAliases = (*Array)(WidgetIdx);
						if ( ClassAliases.WidgetClass == WidgetSuperClass )
						{
							DefaultElementValue = (BYTE*)Array->GetData() + DefaultEventKeyProperty->Inner->ElementSize * SearchIndex;
							break;
						}
					}
				}

				// otherwise, use the struct defaults.
				if ( DefaultElementValue == NULL )
				{
					DefaultElementValue = StructDefaults;
				}

				DefaultEventKeyProperty->Inner->ExportTextItem(OutputString, ElementValue, StructDefaults, InputConfig, PPF_SkipObjectProperties|PPF_Localized);
			}

			// Clear the existing WidgetInputAliases array items and then set our newly generated array.
			TMultiMap<FString, FString> *Section = GConfig->GetSectionPrivate(TEXT("Engine.UIInputConfiguration"), FALSE, FALSE, *DefaultIniFilename);
			if(Section != NULL)
			{
				Section->RemoveKey(TEXT("+WidgetInputAliases"));
			}

			if ( CurrentPlatformName != CONSOLESUPPORT_NAME_PC )
			{
				// if this isn't the PC, we need to add a single line that will remove all PC bindings from the final .ini
				GConfig->SetString(TEXT("Engine.UIInputConfiguration"), TEXT("!WidgetInputAliases"), TEXT("ClearArray"), *DefaultIniFilename);
			}
			GConfig->SetArray(TEXT("Engine.UIInputConfiguration"), TEXT("+WidgetInputAliases"), ArrayStrings, *DefaultIniFilename);
			GConfig->Flush(FALSE, *DefaultIniFilename);
			bSaveSuccessful = TRUE;
		}
	}

	return bSaveSuccessful;
}

/**
 * Saves the input alias mappings to the default ini for all platforms
 *
 * @return	TRUE if the all platforms were saved successfully, FALSE if any platforms failed
 */
UBOOL WxDlgUIEventKeyBindings::SaveAll()
{
	UBOOL bSaveSuccessful = FALSE;

	// save each platform in turn

	return bSaveSuccessful;
}
