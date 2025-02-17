/*=============================================================================
    WxDlgAddCalloutButton.cpp: 
    Copyright 2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "EngineUIPrivateClasses.h"
#include "UnObjectEditor.h"
#include "UnUIEditor.h"
#include "ScopedTransaction.h"
#include "PropertyWindowManager.h"
#include "WxDlgAddCalloutButton.h"

/*!
 * WxDlgAddCalloutButton type definition
 */
IMPLEMENT_DYNAMIC_CLASS(WxDlgAddCalloutButton, wxDialog)

/*!
 * WxDlgAddCalloutButton event table definition
 */
BEGIN_EVENT_TABLE( WxDlgAddCalloutButton, wxDialog )
	EVT_TEXT(ID_UI_ADDCALLOUT_EDIT_TAG,WxDlgAddCalloutButton::OnTextEntered)
	EVT_TEXT(ID_UI_ADDCALLOUT_EDIT_TAG,WxDlgAddCalloutButton::OnTextEntered)
END_EVENT_TABLE()

/*!
 * WxDlgAddCalloutButton constructors
 */
WxDlgAddCalloutButton::WxDlgAddCalloutButton()
{
	Init();
}

WxDlgAddCalloutButton::WxDlgAddCalloutButton( UUICalloutButtonPanel* InOwnerPanel, wxWindow* parent, wxWindowID id/*=ID_UI_ADDCALLOUT_DLG*/ )
{
	Init();
	Create(InOwnerPanel, parent, id);
}

WxDlgAddCalloutButton::WxDlgAddCalloutButton( UUICalloutButton* InButton, wxWindow* parent, wxWindowID id/*=ID_UI_ADDCALLOUT_DLG*/ )
{
	Init();
	Create(InButton, parent, id);
}

/**
 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
 *
 * @param	InParent				the window that opened this dialog
 * @param	InID					the ID to use for this dialog
 */
void WxDlgAddCalloutButton::Create( UUICalloutButtonPanel* InOwnerPanel, wxWindow* parent, wxWindowID id/*=ID_UI_ADDCALLOUT_DLG*/ )
{
	check(InOwnerPanel);
	OwnerPanel = InOwnerPanel;

	verify(wxDialog::Create(parent, id,*LocalizeUI(TEXT("DlgAddCalloutButton_Title")),wxDefaultPosition,wxSize(350,140),wxDEFAULT_DIALOG_STYLE|wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxSUNKEN_BORDER|wxTAB_TRAVERSAL));
	SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);

	// create all controls
	CreateControls();

	// initialize the controls with valid options
	InitializeControlValues();

	// position the dialog
	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();
}

/**
 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
 *
 * @param	InButton				when this dialog is invoked to modify an existing button, the button that was selected 
 * @param	InParent				the window that opened this dialog
 * @param	InID					the ID to use for this dialog
 */
void WxDlgAddCalloutButton::Create( UUICalloutButton* InButton, wxWindow* parent, wxWindowID id/*=ID_UI_ADDCALLOUT_DLG*/ )
{
	check(InButton);

	SelectedButton = InButton;
	UUICalloutButtonPanel* ButtonOwner = Cast<UUICalloutButtonPanel>(InButton->GetOwner());
	Create(ButtonOwner, parent, id);
}

/**
 * WxDlgAddCalloutButton destructor
 */
WxDlgAddCalloutButton::~WxDlgAddCalloutButton()
{
}

/**
 * Member initialization 
 */

void WxDlgAddCalloutButton::Init()
{
	OwnerPanel = NULL;
	SelectedButton = NULL;
	cmb_InputAlias = NULL;
	txt_WidgetTag = NULL;
	cmb_ButtonClass = NULL;
	OKButton = NULL;
}

/*!
 * Control creation for WxDlgAddCalloutButton
 */

void WxDlgAddCalloutButton::CreateControls()
{    
	wxBoxSizer* BaseVSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(BaseVSizer);
	{
		wxFlexGridSizer* OptionGridSizer = new wxFlexGridSizer(2, 2, 0, 0);
		OptionGridSizer->AddGrowableCol(1);
		BaseVSizer->Add(OptionGridSizer, 1, wxGROW|wxALL, 0);
		{
			wxStaticText* lbl_InputAlias = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgAddCalloutButton_Label_InputAlias")), wxDefaultPosition, wxDefaultSize, 0 );
			OptionGridSizer->Add(lbl_InputAlias, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

			cmb_InputAlias = new wxChoice( this, ID_UI_ADDCALLOUT_COMBO_ALIAS, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_SORT );
			cmb_InputAlias->SetToolTip(*LocalizeUI(TEXT("DlgAddCalloutButton_ToolTip_InputAlias")));
			OptionGridSizer->Add(cmb_InputAlias, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

			wxStaticText* lbl_WidgetTag = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgAddCalloutButton_Label_WidgetTag")), wxDefaultPosition, wxDefaultSize, 0 );
			OptionGridSizer->Add(lbl_WidgetTag, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

			txt_WidgetTag = new WxTextCtrl( this, ID_UI_ADDCALLOUT_EDIT_TAG, TEXT(""), wxDefaultPosition, wxDefaultSize, 0 );
			txt_WidgetTag->SetToolTip(*LocalizeUI(TEXT("DlgAddCalloutButton_ToolTip_WidgetTag")));
			txt_WidgetTag->SetMaxLength(NAME_SIZE);
			OptionGridSizer->Add(txt_WidgetTag, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

			wxStaticText* lbl_ButtonClass = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgAddCalloutButton_Label_ButtonClass")), wxDefaultPosition, wxDefaultSize, 0 );
			OptionGridSizer->Add(lbl_ButtonClass, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

			cmb_ButtonClass = new wxChoice( this, ID_UI_ADDCALLOUT_COMBO_CLASS, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_SORT );
			cmb_ButtonClass->SetToolTip(*LocalizeUI(TEXT("DlgAddCalloutButton_ToolTip_ButtonClass")));
			OptionGridSizer->Add(cmb_ButtonClass, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
		}

		wxBoxSizer* ButtonHSizer = new wxBoxSizer(wxHORIZONTAL);
		BaseVSizer->Add(ButtonHSizer, 0, wxALIGN_RIGHT|wxALL, 5);
		{
			OKButton = new wxButton(this, wxID_OK, *LocalizeUnrealEd(TEXT("&OK")));
			OKButton->SetDefault();
			ButtonHSizer->Add(OKButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

			wxButton* CancelButton = new wxButton( this, wxID_CANCEL, *LocalizeUnrealEd(TEXT("&Cancel")) );
			ButtonHSizer->Add(CancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
		}
	}

	// Set validators
	txt_WidgetTag->SetValidator( WxNameTextValidator(&DesiredWidgetTag, VALIDATE_ObjectName) );
}

/**
 * Populates the combo controls with the available selections for each type.
 */
void WxDlgAddCalloutButton::InitializeControlValues()
{
	Freeze();
	{
		if ( cmb_InputAlias != NULL )
		{
			TArray<FName> InputAliases;
			OwnerPanel->GetAvailableCalloutButtonAliases(InputAliases);

			cmb_InputAlias->Freeze();

			// clear any existing elements
			cmb_InputAlias->Clear();

			wxArrayString InputAliasStrings;
			for ( INT AliasIdx = 0; AliasIdx < InputAliases.Num(); AliasIdx++ )
			{
				InputAliasStrings.Add(*InputAliases(AliasIdx).ToString());
			}

			// add the strings for the aliases available in this button panel
			cmb_InputAlias->Append(InputAliasStrings);
			if ( cmb_InputAlias->GetCount() > 0 )
			{
				// set the selection to the first item
				cmb_InputAlias->SetSelection(0);
			}

			cmb_InputAlias->Thaw();
		}

		if ( txt_WidgetTag != NULL && SelectedButton != NULL )
		{
			DesiredWidgetTag = *SelectedButton->GetName();
			txt_WidgetTag->Disable();
		}

		if ( cmb_ButtonClass != NULL )
		{
			UClass* RequiredMetaClass = UUICalloutButton::StaticClass();
			if ( SelectedButton != NULL )
			{
				RequiredMetaClass = SelectedButton->GetClass();
			}
			else if ( OwnerPanel->ButtonTemplate != NULL )
			{
				RequiredMetaClass = OwnerPanel->ButtonTemplate->GetClass();
			}

			// next, generate a list of valid callout button classes to choose from
			TArray<UClass*> ValidButtonClasses;
			for ( TObjectIterator<UClass> It; It; ++It )
			{
				if ( It->IsChildOf(UUICalloutButton::StaticClass())
				&&	!It->HasAnyClassFlags(CLASS_Abstract|CLASS_Deprecated|CLASS_HideDropDown) )
				{
					ValidButtonClasses.AddItem(*It);
				}
			}

			wxArrayString ButtonClassNames;

			INT SelectedItem = 0;
			for ( INT ClassIndex = 0; ClassIndex < ValidButtonClasses.Num(); ClassIndex++ )
			{
				if ( ValidButtonClasses(ClassIndex) == RequiredMetaClass )
				{
					SelectedItem = ClassIndex;
				}

				ButtonClassNames.Add(*ValidButtonClasses(ClassIndex)->GetDescription());
			}

			cmb_ButtonClass->Freeze();

			// clear any existing elements
			cmb_ButtonClass->Clear();

			// add the names of all valid button classes
			cmb_ButtonClass->Append(ButtonClassNames);
			if ( (UINT)SelectedItem < cmb_ButtonClass->GetCount() )
			{
				// select the one corresponding to this button panel's button template
				cmb_ButtonClass->SetSelection(SelectedItem);
			}

			// if there is only one class available, disable the combo
			if ( ButtonClassNames.GetCount() == 1 || SelectedButton != NULL )
			{
				cmb_ButtonClass->Disable();
			}
			cmb_ButtonClass->Thaw();
		}
	}
	Thaw();
}


/**
 * @return	the input alias chosen for the new button
 */
FName WxDlgAddCalloutButton::GetSelectedAlias() const
{
	FName Result = NAME_None;

	if ( cmb_InputAlias != NULL && cmb_InputAlias->GetCount() > 0 )
	{
		Result = FName(cmb_InputAlias->GetStringSelection().c_str());
	}

	return Result;
}

/**
 * @return	the tag to use for the new button.
 */
FName WxDlgAddCalloutButton::GetButtonTag() const
{
	FName Result=NAME_None;

	if ( txt_WidgetTag != NULL && txt_WidgetTag->GetLineLength(0) > 0 )
	{
		Result = FName(txt_WidgetTag->GetValue().c_str());
	}

	return Result;
}

/**
 * @return	the name of the class chosen for the new button
 */
FString WxDlgAddCalloutButton::GetSelectedClassName() const
{
	FString Result;

	if ( cmb_ButtonClass != NULL && cmb_ButtonClass->GetCount() > 0 )
	{
		Result = cmb_ButtonClass->GetStringSelection().c_str();
	}

	return Result;
}

bool WxDlgAddCalloutButton::Validate()
{
	bool bResult = wxDialog::Validate();
	if ( bResult )
	{
		FName WidgetTag = GetButtonTag();
		if ( OwnerPanel != NULL )
		{
			if ( SelectedButton == NULL && OwnerPanel->FindChild(WidgetTag) != NULL )
			{
				appMsgf(AMT_OK, *LocalizeUI(TEXT("DlgAddCalloutButton_Error_ExistingChild")));
				bResult = false;
			}
			else
			{
				FName ChosenAlias = GetSelectedAlias();
				if ( OwnerPanel->eventContainsButton(ChosenAlias) )
				{
					appMsgf(AMT_OK, *LocalizeUI(TEXT("DlgAddCalloutButton_Error_ExistingTag")));
					bResult = false;
				}
			}
		}
	}

	return bResult;
}

void WxDlgAddCalloutButton::OnTextEntered( wxCommandEvent& Event )
{
	if ( OKButton != NULL )
	{
		// only enable the OK button if both edit boxes contain text
		OKButton->Enable( txt_WidgetTag->GetLineLength(0) > 0 );
	}

	Event.Skip();
}

// EOF





