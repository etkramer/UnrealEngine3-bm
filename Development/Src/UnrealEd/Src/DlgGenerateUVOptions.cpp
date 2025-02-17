/*=============================================================================
Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "DlgGenerateUVOptions.h"

BEGIN_EVENT_TABLE(WxDlgGenerateUVOptions, wxDialog)
EVT_BUTTON( ID_DialogGenerateUVOptions_OK, WxDlgGenerateUVOptions::OnOk )
EVT_BUTTON( ID_DialogGenerateUVOptions_Cancel, WxDlgGenerateUVOptions::OnCancel )
EVT_COMBOBOX( ID_DialogGenerateUVOptions_LODCombo, WxDlgGenerateUVOptions::OnChangeLODCombo )
END_EVENT_TABLE()

WxDlgGenerateUVOptions::WxDlgGenerateUVOptions(
	wxWindow* Parent, 
	const TArray<FString>& LODComboOptions, 
	const TArray<INT>& InLODNumTexcoords,
	FLOAT MaxDesiredStretchDefault)
: wxDialog(Parent, wxID_ANY, *LocalizeUnrealEd("GenerateUVsTitle"), wxDefaultPosition, wxSize(400, 240), wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP)
{
	LODNumTexcoords = InLODNumTexcoords;
	ChosenLODIndex = 0;
	ChosenTexIndex = 0;
	MaxStretch = MaxDesiredStretchDefault;
	bKeepExistingUVs = TRUE;

	//the base sizer
	wxBoxSizer* VerticalSizer = new wxBoxSizer(wxVERTICAL);
	{
		//sizer for input LOD related controls
		wxBoxSizer* InputLODSizer = new wxBoxSizer(wxHORIZONTAL);
		{
			wxStaticText* InputLODText = new wxStaticText(this, wxID_ANY, *LocalizeUnrealEd("LODChooseText"));
			InputLODSizer->Add(InputLODText, 2, wxEXPAND | wxALL, 5);

			LODComboBox = new wxComboBox( this, ID_DialogGenerateUVOptions_LODCombo, TEXT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
			//populate the combo box with available LOD's
			for( INT OptionIndex = 0 ; OptionIndex < LODComboOptions.Num(); OptionIndex++)
			{
				const FString& Option = LODComboOptions( OptionIndex );
				LODComboBox->Append( *Option );
			}
			LODComboBox->SetSelection(0);
			InputLODSizer->Add(LODComboBox, 1, wxEXPAND | wxALL, 5);
		}
		VerticalSizer->Add(InputLODSizer, 0, wxEXPAND | wxALL, 5);

		//sizer for input uv index related controls
		wxBoxSizer* InputTexIndexSizer = new wxBoxSizer(wxHORIZONTAL);
		{
			wxStaticText* InputTexIndexText = new wxStaticText(this, wxID_ANY, *LocalizeUnrealEd("UVChannelText"));
			InputTexIndexSizer->Add(InputTexIndexText, 2, wxEXPAND | wxALL, 5);

			TexIndexComboBox = new wxComboBox( this, ID_DialogGenerateUVOptions_TexIndexCombo, TEXT("0"), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
			//populate combo with available UV indices, clamp to the maximum number of tex coord channels allowed
			for( INT OptionIndex = 0 ; OptionIndex < LODNumTexcoords(0) + 1 && OptionIndex < MAX_TEXCOORDS; OptionIndex++)
			{
				TexIndexComboBox->Append(*FString::Printf( TEXT("%d"), OptionIndex ));
			}
			TexIndexComboBox->SetSelection(0);

			InputTexIndexSizer->Add(TexIndexComboBox, 1, wxEXPAND | wxALL, 5);
		}
		VerticalSizer->Add(InputTexIndexSizer, 0, wxEXPAND | wxALL, 5);

		//sizer for input max stretch related controls
		wxBoxSizer* InputMaxStretchSizer = new wxBoxSizer(wxHORIZONTAL);
		{
			wxStaticText* InputMaxStretchText = new wxStaticText(this, wxID_ANY, *LocalizeUnrealEd("MaxUVStretchingText"));
			InputMaxStretchSizer->Add(InputMaxStretchText, 2, wxEXPAND | wxALL, 5);

			FString MaxDesiredStretchString = FString::Printf(TEXT("%f"), MaxDesiredStretchDefault);
			MaxStretchEntry = new wxTextCtrl( this, ID_DialogGenerateUVOptions_MaxStretchEdit, *MaxDesiredStretchString, wxDefaultPosition, wxSize(15, -1), 0 );

			InputMaxStretchSizer->Add(MaxStretchEntry, 1, wxEXPAND | wxALL, 5);
		}
		VerticalSizer->Add(InputMaxStretchSizer, 0, wxEXPAND | wxALL, 5);

		//sizer for input max stretch related controls
		wxBoxSizer* InputKeepExistingUVSizer = new wxBoxSizer(wxHORIZONTAL);
		{
			wxStaticText* InputKeepExistingUVText = new wxStaticText(this, wxID_ANY, *LocalizeUnrealEd("KeepExistingUVText"));
			InputKeepExistingUVSizer->Add(InputKeepExistingUVText, 2, wxEXPAND | wxALL, 5);

			FString MaxDesiredStretchString = FString::Printf(TEXT("%f"), MaxDesiredStretchDefault);
			KeepExistingUVCheckBox = new wxCheckBox( this, ID_DialogGenerateUVOptions_KeepExistingUVs, TEXT(""), wxDefaultPosition, wxSize(15, -1), 0 );
			KeepExistingUVCheckBox->SetValue(bKeepExistingUVs != FALSE);

			InputKeepExistingUVSizer->Add(KeepExistingUVCheckBox, 1, wxEXPAND | wxALL, 5);
		}
		VerticalSizer->Add(InputKeepExistingUVSizer, 0, wxEXPAND | wxALL, 5);

		//sizer for button controls
		wxBoxSizer* ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
		{
			OkButton = new wxButton(this, ID_DialogGenerateUVOptions_OK, *LocalizeUnrealEd("OK"));
			ButtonSizer->Add(OkButton, 0, wxCENTER | wxALL, 5);

			CancelButton = new wxButton(this, ID_DialogGenerateUVOptions_Cancel, *LocalizeUnrealEd("Cancel"));
			ButtonSizer->Add(CancelButton, 0, wxCENTER | wxALL, 5);
		}
		VerticalSizer->Add(ButtonSizer, 0, wxALIGN_RIGHT | wxALL, 2);
		
	}
	SetSizer(VerticalSizer);

	//fit the window around the sizer
	Fit();
}

WxDlgGenerateUVOptions::~WxDlgGenerateUVOptions()
{

}

int WxDlgGenerateUVOptions::ShowModal()
{
	return wxDialog::ShowModal();
}

void WxDlgGenerateUVOptions::OnOk(wxCommandEvent& In )
{
	//parse user input, use defaults if input is invalid
	if (LODComboBox->GetSelection() >= 0)
	{
		ChosenLODIndex = LODComboBox->GetSelection();
	}

	if (TexIndexComboBox->GetSelection() >= 0)
	{
		ChosenTexIndex = TexIndexComboBox->GetSelection();
	}
	
	FLOAT ParsedMaxStretch = appAtof(MaxStretchEntry->GetValue());
	if (ParsedMaxStretch > 0.01f && ParsedMaxStretch <= 1.0f)
	{
		MaxStretch = ParsedMaxStretch;
	}

	bKeepExistingUVs = KeepExistingUVCheckBox->GetValue();

	wxDialog::AcceptAndClose();
}

void WxDlgGenerateUVOptions::OnCancel(wxCommandEvent& In )
{
	wxDialog::EndDialog( In.GetId() );
}

void WxDlgGenerateUVOptions::OnChangeLODCombo(wxCommandEvent& In )
{
	//re-populate the texture coordinate index based on the number of tex coords in the newly selected LOD
	if (LODComboBox->GetSelection() >= 0)
	{
		ChosenLODIndex = LODComboBox->GetSelection();
	}

	TexIndexComboBox->Clear();
	for( INT OptionIndex = 0 ; OptionIndex < LODNumTexcoords(ChosenLODIndex) + 1 && OptionIndex < MAX_TEXCOORDS; OptionIndex++)
	{
		TexIndexComboBox->Append(*FString::Printf( TEXT("%d"), OptionIndex));
	}
	TexIndexComboBox->SetSelection(0);
}
