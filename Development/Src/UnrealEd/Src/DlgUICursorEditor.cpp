/*=============================================================================
	DlgUICursorEditor.cpp : This class lets the user edit cursors for the UI system, it should only be spawned by the UI Skin Editor.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "PropertyWindowManager.h"
#include "ScopedTransaction.h"

#include "EngineUIPrivateClasses.h"
#include "UnrealEdPrivateClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

#include "UnObjectEditor.h"
#include "UnLinkedObjEditor.h"
#include "Kismet.h"
#include "UnUIEditor.h"
#include "UnUIStyleEditor.h"

#include "DlgUISkinEditor.h"
#include "DlgUICursorEditor.h"

/**
 * This class lets the user edit cursors for the UI system, it should only be spawned by the UI Skin Editor.
 */

BEGIN_EVENT_TABLE(WxDlgUICursorEditor, wxDialog)
	EVT_BUTTON(ID_PROP_PB_BROWSE,	WxDlgUICursorEditor::OnShowBrowser)
	EVT_BUTTON(ID_PROP_PB_USE,		WxDlgUICursorEditor::OnUseSelected)

	EVT_BUTTON(wxID_OK,				WxDlgUICursorEditor::OnOK)
END_EVENT_TABLE()

WxDlgUICursorEditor::WxDlgUICursorEditor(WxDlgUISkinEditor* InParent, UUISkin* InParentSkin, const TCHAR* InCursorName, FUIMouseCursor* InCursor) : 
wxDialog(InParent, wxID_ANY, *LocalizeUI(TEXT("DlgUICursorEditor_Title")), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
Cursor(InCursor),
ParentSkin(InParentSkin),
SkinEditor(InParent)
{
	SetMinSize( wxSize(600, -1) );

	wxBoxSizer* MainSizer = new wxBoxSizer(wxHORIZONTAL);
	{
		wxBoxSizer* TextBoxSizer = new wxBoxSizer(wxVERTICAL);
		{
			// Cursor Name
			wxStaticText* CursorNameText = new wxStaticText(this, wxID_ANY, *FString::Printf(LocalizeSecure(LocalizeUI(TEXT("DlgUICursorEditor_CursorName")), InCursorName)));
			TextBoxSizer->Add(CursorNameText, 0, wxEXPAND | wxALL, 5);

			// Texture
			wxStaticBoxSizer* TextureSizer = new wxStaticBoxSizer(wxVERTICAL, this, *LocalizeUI(TEXT("DlgUICursorEditor_CursorTexture")));
			{
				wxSizer* TextHSizer = new wxBoxSizer(wxHORIZONTAL);
				{
					// Cursor texture text box.
					{
						CursorTexture = new wxTextCtrl(this, wxID_ANY);
						TextHSizer->Add(CursorTexture, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

						if(Cursor->Cursor)
						{
							USurface *Surface = Cursor->Cursor->ImageTexture;
							CursorTexture->SetValue(*Surface->GetPathName());
						}
					}


					wxButton* ButtonUse = new wxBitmapButton( this, ID_PROP_PB_USE, GPropertyWindowManager->Prop_UseCurrentBrowserSelectionB, wxDefaultPosition, wxSize(24,24), wxBU_AUTODRAW|wxBU_EXACTFIT );
					TextHSizer->Add(ButtonUse, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

					wxButton* ButtonBrowse = new wxBitmapButton( this, ID_PROP_PB_BROWSE, GPropertyWindowManager->Prop_ShowGenericBrowserB, wxDefaultPosition, wxSize(24,24), wxBU_AUTODRAW|wxBU_EXACTFIT );
					TextHSizer->Add(ButtonBrowse, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5);
				}
				TextureSizer->Add(TextHSizer, 0, wxEXPAND | wxALL, 5);
			}
			TextBoxSizer->Add(TextureSizer, 0, wxEXPAND | wxALL, 3);

			// Style
			wxStaticBoxSizer* StyleSizer = new wxStaticBoxSizer(wxVERTICAL, this, *LocalizeUI(TEXT("DlgUICursorEditor_CursorStyle")));
			{
				CursorStyle = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);

				// Loop through all of the image styles in this skin and add them to the combo box.
				CursorStyle->Freeze();
				{
					for(TMap<FName, UUIStyle*>::TIterator It(ParentSkin->StyleNameMap); It; ++It)
					{
						const UUIStyle* Style = It.Value();

						if(Style->StyleDataClass->IsChildOf(UUIStyle_Image::StaticClass()) == TRUE)
						{
							CursorStyle->Append(*It.Key().ToString());
						}
					}

					// Set default combo box selection to the cursor's style.
					if(CursorStyle->GetCount())
					{
						CursorStyle->SetSelection(0);
						CursorStyle->SetValue(*Cursor->CursorStyle.ToString());
					}
				}
				CursorStyle->Thaw();

				StyleSizer->Add(CursorStyle, 0, wxEXPAND | wxALL, 5);
			}
			TextBoxSizer->Add(StyleSizer, 0, wxEXPAND | wxALL, 3);
		}
		MainSizer->Add(TextBoxSizer, 1, wxALL | wxEXPAND, 5);


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
	Layout();
	Show();

	CursorTexture->SetSelection(0,0);
}

/** Callback for when the user clicks the OK button, applies the new settings to the cursor resource. */
void WxDlgUICursorEditor::OnOK(wxCommandEvent& event)
{
	wxString TextureName = CursorTexture->GetValue();
	UUITexture *Texture = NULL;
	USurface *Surface = FindObject<USurface>(NULL, TextureName);

	if(Surface)
	{
		// Create a UUITexture for this cursor.
		Texture = ConstructObject<UUITexture>(UUITexture::StaticClass(), ParentSkin);
		Texture->ImageTexture = Surface;
	}

	// Error if we were unable to create a texture
	if(Texture == NULL)
	{
		wxMessageBox(*LocalizeUI(TEXT("DlgUICursorEditor_InvalidTexture")), *LocalizeUI(TEXT("DlgUICursorEditor_Error")), wxICON_ERROR);
	}


	// Make sure they picked a valid style for this cursor.
	FName StyleName = FName(CursorStyle->GetValue());
	UUIStyle** Style = ParentSkin->StyleNameMap.Find(StyleName);

	if(Style == NULL)
	{
		wxMessageBox(*LocalizeUI(TEXT("DlgUICursorEditor_InvalidStyle")), *LocalizeUI(TEXT("DlgUICursorEditor_Error")), wxICON_ERROR);
	}

	if(Style && Texture)
	{
		Cursor->Cursor = Texture;
		Cursor->CursorStyle = StyleName;
		SkinEditor->UpdateSkinProperties();
		Hide();
	}
}


/** Callback for when the user clicks the show browser button, shows the generic browser. */
void WxDlgUICursorEditor::OnShowBrowser(wxCommandEvent& event)
{
	WxBrowser* Browser = GUnrealEd->GetBrowserManager()->GetBrowserPane(TEXT("GenericBrowser"));
	if ( Browser )
	{
		GUnrealEd->GetBrowserManager()->ShowWindow(Browser->GetDockID(),TRUE);
	}
}

/** Callback for when the user clicks the use selected button, uses the currently selected texture from the generic browser if one exists. */
void WxDlgUICursorEditor::OnUseSelected(wxCommandEvent& event)
{
	USurface* SelectedSurface = GEditor->GetSelectedObjects()->GetTop<USurface>();
	if ( SelectedSurface != NULL )
	{
		CursorTexture->SetValue( *SelectedSurface->GetPathName() );
	}
}

