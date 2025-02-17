/*=============================================================================
	DlgUISoundCue.cpp : UISoundCue dialog class implementations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "PropertyWindowManager.h"
#include "EngineUIPrivateClasses.h"

#include "UnObjectEditor.h"
#include "UnUIEditor.h"

#include "DlgUISkinEditor.h"
#include "DlgUISoundCue.h"

#include "GenericBrowser.h"

BEGIN_EVENT_TABLE(WxSoundCueTextCtrl,WxTextCtrl)
	EVT_CHAR(WxSoundCueTextCtrl::OnChar)
END_EVENT_TABLE()

/** this handler listens for backspace or delete keypresses, then clears the editbox when those are received */
void WxSoundCueTextCtrl::OnChar( wxKeyEvent& Event )
{
	if ( Event.GetKeyCode() == WXK_BACK || Event.GetKeyCode() == WXK_DELETE )
	{
		Clear();
	}
	else
	{
		Event.Skip();
	}
}

IMPLEMENT_DYNAMIC_CLASS(WxDlgEditSoundCue,wxDialog);
BEGIN_EVENT_TABLE(WxDlgEditSoundCue,wxDialog)
	EVT_CHOICE(ID_UI_EDITUISOUNDCUE_COMBO_ALIAS,WxDlgEditSoundCue::OnSelectSoundCueName)
	
	EVT_TEXT(ID_UI_EDITUISOUNDCUE_TEXT_PATHNAME,WxDlgEditSoundCue::OnSoundCuePathChanged)

	EVT_BUTTON(ID_PROP_PB_USE,WxDlgEditSoundCue::OnClick_UseSelected)
	EVT_BUTTON(ID_PROP_PB_BROWSE,WxDlgEditSoundCue::OnClick_ShowInBrowser)

	EVT_BUTTON(wxID_OK, WxDlgEditSoundCue::OnClick_CloseDialog)
	EVT_BUTTON(wxID_CANCEL, WxDlgEditSoundCue::OnClick_CloseDialog)


	EVT_CLOSE(WxDlgEditSoundCue::OnClose)

END_EVENT_TABLE()

/**
 * Constructor
 * Initializes the list of available UISoundCue names
 */
WxDlgEditSoundCue::WxDlgEditSoundCue()
: ParentSkinEditor(NULL), CurrentlyActiveSkin(NULL)
, cmb_AvailableNames(NULL), txt_SoundCuePath(NULL)
, btn_UseSelected(NULL), btn_ShowBrowser(NULL)
, InitialSoundCueData(EC_EventParm), CurrentSoundCueData(EC_EventParm)
{
}

/**
 * Destructor
 * Saves the window position and size to the ini
 */

WxDlgEditSoundCue::~WxDlgEditSoundCue()
{
	FWindowUtil::SavePosSize(TEXT("EditSoundCueDialog"), this);
}

/**
 * Initialize this dialog.  Must be the first function called after creating the dialog.
 *
 * @param	ActiveSkin	the skin that is currently active
 * @param	InParent	the window that opened this dialog
 * @param	InID		the ID to use for this dialog
 */
void WxDlgEditSoundCue::Create( UUISkin* ActiveSkin, WxDlgUISkinEditor* InParent, wxWindowID InID/*=ID_UI_EDITUISOUNDCUE_DLG*/, FName InitialSoundCueName/*=NAME_None*/, USoundCue* InitialSoundCueSound/*=NULL*/ )
{
	ParentSkinEditor = InParent;
	CurrentlyActiveSkin = ActiveSkin;
	InitialSoundCueData.SoundName = InitialSoundCueName;
	InitialSoundCueData.SoundToPlay = InitialSoundCueSound;

	// copy the initial values into the working copy
	CurrentSoundCueData = InitialSoundCueData;

	SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
	verify(wxDialog::Create(
		InParent, InID,
		*LocalizeUI(TEXT("DlgUIEditSoundCue_Title")),
		wxDefaultPosition, wxDefaultSize,
		wxCAPTION|wxCLOSE_BOX|wxDIALOG_MODAL|wxCLIP_CHILDREN |wxTAB_TRAVERSAL
		));

	CreateControls();

	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();

	wxRect DefaultPos = GetRect();
	FWindowUtil::LoadPosSize( TEXT("EditSoundCueDialog"), this, DefaultPos.GetLeft(), DefaultPos.GetTop(), 400, DefaultPos.GetHeight() );
}

/**
 * Creates the controls and sizers for this dialog.
 */
void WxDlgEditSoundCue::CreateControls()
{
	wxBoxSizer* BaseVSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(BaseVSizer);

	wxStaticBox* StaticGroupBox = new wxStaticBox(this, wxID_ANY, *LocalizeUI(TEXT("DlgUIEditSoundCue_Label")));
	{
		wxStaticBoxSizer* StaticHSizer = new wxStaticBoxSizer(StaticGroupBox, wxHORIZONTAL);
		BaseVSizer->Add(StaticHSizer, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP, 5);
		{
			wxBoxSizer* ParameterVSizer = new wxBoxSizer(wxVERTICAL);
			StaticHSizer->Add(ParameterVSizer, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);
			{
				cmb_AvailableNames = new wxChoice(this, ID_UI_EDITUISOUNDCUE_COMBO_ALIAS, wxDefaultPosition, wxDefaultSize);
				cmb_AvailableNames->SetToolTip(*LocalizeUI(TEXT("DlgUIEditSoundCue_ToolTip_NameCombo")));
				ParameterVSizer->Add(cmb_AvailableNames, 0, wxGROW|wxALL, 5);

				wxBoxSizer* SoundCuePathHSizer = new wxBoxSizer(wxHORIZONTAL);
				ParameterVSizer->Add(SoundCuePathHSizer, 0, wxGROW|wxALL, 5);
				{	
					txt_SoundCuePath = new WxSoundCueTextCtrl( this, ID_UI_EDITUISOUNDCUE_TEXT_PATHNAME, TEXT(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
					txt_SoundCuePath->SetToolTip(*LocalizeUI(TEXT("DlgUIEditSoundCue_ToolTip_SoundCuePath")));
					SoundCuePathHSizer->Add(txt_SoundCuePath, 1, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);

					btn_UseSelected = new wxBitmapButton( this, ID_PROP_PB_USE, GPropertyWindowManager->Prop_UseCurrentBrowserSelectionB, wxDefaultPosition, wxSize(24,24), wxBU_AUTODRAW|wxBU_EXACTFIT );
					btn_UseSelected->SetToolTip(*LocalizeUI(TEXT("DlgUIEditSoundCue_ToolTip_SoundCueUse")));
					SoundCuePathHSizer->Add(btn_UseSelected, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

					btn_ShowBrowser = new wxBitmapButton( this, ID_PROP_PB_BROWSE, GPropertyWindowManager->Prop_ShowGenericBrowserB, wxDefaultPosition, wxSize(24, 24), wxBU_AUTODRAW|wxBU_EXACTFIT );
					btn_ShowBrowser->SetToolTip(*LocalizeUI(TEXT("DlgUIEditSoundCue_ToolTip_SoundCueBrowse")));
					SoundCuePathHSizer->Add(btn_ShowBrowser, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5);

				}
			}
		}
	}

	wxStdDialogButtonSizer* ButtonSizer = new wxStdDialogButtonSizer;
	BaseVSizer->Add(ButtonSizer, 0, wxALIGN_RIGHT|wxALL, 5);
	{
		wxButton* OKButton = new wxButton( this, wxID_OK, *LocalizeUnrealEd(TEXT("&OK")), wxDefaultPosition, wxDefaultSize, 0 );
		OKButton->SetDefault();
		ButtonSizer->AddButton(OKButton);

		wxButton* CancelButton = new wxButton( this, wxID_CANCEL, *LocalizeUnrealEd(TEXT("&Cancel")), wxDefaultPosition, wxDefaultSize, 0 );
		ButtonSizer->AddButton(CancelButton);

		ButtonSizer->Realize();
	}

	InitializeNameCombo();
	if ( CurrentSoundCueData.SoundToPlay != NULL )
	{
		txt_SoundCuePath->SetValue(*CurrentSoundCueData.SoundToPlay->GetPathName());
	}
}


/**
 * Initializes the name combo with the available names as configured in the UIInteraction's UISoundCueNames array.
 */
void WxDlgEditSoundCue::InitializeNameCombo()
{
	check(CurrentlyActiveSkin);

	UUIInteraction* DefaultUIController = UUIRoot::GetDefaultUIController();
	check(DefaultUIController);

	cmb_AvailableNames->Freeze();
	cmb_AvailableNames->Clear();

	// get the list of all names registered in the UI controller
	TArray<FName> AvailableNames = DefaultUIController->UISoundCueNames;

	// now get the list of UISoundCues that exist in the current skin
	TArray<FUISoundCue> ExistingSoundCues;
	CurrentlyActiveSkin->GetSkinSoundCues(ExistingSoundCues);

	// now iterate through our local copy of the available names, removing those that exist in the current skin
	for ( INT SoundCueIndex = 0; SoundCueIndex < ExistingSoundCues.Num(); SoundCueIndex++ )
	{
		FUISoundCue& SoundCue = ExistingSoundCues(SoundCueIndex);
		if ( SoundCue.SoundName != NAME_None && SoundCue.SoundName != CurrentSoundCueData.SoundName )
		{
			AvailableNames.RemoveItem(SoundCue.SoundName);
		}
	}

	cmb_AvailableNames->Append(*LocalizeUnrealEd(TEXT("None")));

	INT SelectionIndex = 0;
	// now add the remaining names to the combo
	for ( INT NameIndex = 0; NameIndex < AvailableNames.Num(); NameIndex++ )
	{
		if ( CurrentSoundCueData.SoundName != NAME_None && CurrentSoundCueData.SoundName == AvailableNames(NameIndex) )
		{
			// +1 because of the "None" item
			SelectionIndex = NameIndex+1;
		}
		cmb_AvailableNames->Append(*AvailableNames(NameIndex).ToString());
	}

	cmb_AvailableNames->SetSelection(SelectionIndex);
	cmb_AvailableNames->Thaw();
}

/**
 * Returns the sound cue data that the user selected in the dialog.
 */
FUISoundCue WxDlgEditSoundCue::GetSoundCueData() const
{
	return CurrentSoundCueData;
}

/** This handler is called when the user selects a name from the UISoundCue name combo box */
void WxDlgEditSoundCue::OnSelectSoundCueName( wxCommandEvent& Event )
{
	wxString Selection;
	if ( cmb_AvailableNames->GetCount() > 0 )
	{
		Selection = cmb_AvailableNames->GetStringSelection();
	}

	if ( Selection.Len() > 0 )
	{
		CurrentSoundCueData.SoundName = Selection.c_str();
	}
}

/** this handler is called when the text in the sound cue path name text control is changed */
void WxDlgEditSoundCue::OnSoundCuePathChanged( wxCommandEvent& Event )
{
	if ( txt_SoundCuePath != NULL && txt_SoundCuePath->GetValue().Len() == 0 )
	{
		CurrentSoundCueData.SoundToPlay = NULL;
	}
}

/** this handler is called when the user clicks the "use selected sound cue" button */
void WxDlgEditSoundCue::OnClick_UseSelected( wxCommandEvent& Event )
{
	USoundCue* SelectedSoundCue = GEditor->GetSelectedObjects()->GetTop<USoundCue>();
	if ( SelectedSoundCue != NULL )
	{
		txt_SoundCuePath->SetValue( *SelectedSoundCue->GetPathName() );
		CurrentSoundCueData.SoundToPlay = SelectedSoundCue;
	}
}

/** this handler is called when the user clicks the "sync generic browser" button */
void WxDlgEditSoundCue::OnClick_ShowInBrowser( wxCommandEvent& Event )
{
	USoundCue* soundcue = CurrentSoundCueData.SoundToPlay;
	
	if (soundcue)
	{
		WxGenericBrowser* GenericBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
		if ( GenericBrowser )
		{		
			// update the GB as well as new packages may have been loaded
			GenericBrowser->Update();

			// Make sure the window is visible.  The window needs to be visible *before*
			// the browser is sync'd to objects so that redraws actually happen!
			GUnrealEd->GetBrowserManager()->ShowWindow( GenericBrowser->GetDockID(), TRUE );

			// Sync.
			TArray<UObject*> SyncToThese;
			SyncToThese.AddItem(soundcue);
			GenericBrowser->SyncToObjects( SyncToThese );
		}
	}
}

/** this handler is called when the user clicks the OK or Cancel button */
void WxDlgEditSoundCue::OnClick_CloseDialog( wxCommandEvent& Event )
{
	SetReturnCode(Event.GetId());
	if ( !IsModal() )
	{
		if ( Event.GetId() == wxID_OK && Validate() && TransferDataFromWindow() )
		{
			if ( CurrentSoundCueData.SoundName != NAME_None )
			{
				GEditor->BeginTransaction(*LocalizeUI(TEXT("DlgUIEditSoundCue_MenuItem_ModifySoundCues")));

				if ( ParentSkinEditor != NULL )
				{
					ParentSkinEditor->AddSoundCue(CurrentSoundCueData.SoundName, CurrentSoundCueData.SoundToPlay, TRUE);
				}

				if ( CurrentlyActiveSkin != NULL )
				{
					CurrentlyActiveSkin->AddUISoundCue(CurrentSoundCueData.SoundName, CurrentSoundCueData.SoundToPlay);
				}

				GEditor->EndTransaction();
			}
		}

		Close();
	}
	else
	{
		Event.Skip();
	}
}

/** this handler hides the dialog */
void WxDlgEditSoundCue::OnClose( wxCloseEvent& Event )
{
	Destroy();
}





