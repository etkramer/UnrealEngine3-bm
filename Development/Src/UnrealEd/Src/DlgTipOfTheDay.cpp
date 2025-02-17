/*=============================================================================
DlgTipOfTheDay.cpp: Tip of the day dialog
Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "FConfigCacheIni.h"
#include "DlgTipOfTheDay.h"

/*-----------------------------------------------------------------------------
WxLocalizedTipProvider
-----------------------------------------------------------------------------*/
WxLocalizedTipProvider::WxLocalizedTipProvider() : 
TipSection(NULL)
{
	// We allow various .inis to contribute multiple paths for localization files.
	UBOOL bFoundFile =  FALSE;
	for (INT LangIndex = 0; LangIndex < 2 && !bFoundFile; LangIndex++)
	{
		// first pass use use language, second pass use english (only does second pass if first wasn't found)
		const TCHAR* LangExt = LangIndex ? TEXT("int") : UObject::GetLanguage();

		for( INT PathIndex=0; PathIndex < GSys->LocalizationPaths.Num(); PathIndex++ )
		{
			FString LocPath = FString::Printf(TEXT("%s") PATH_SEPARATOR TEXT("%s"), *GSys->LocalizationPaths(PathIndex), LangExt);
			FFilename SearchPath = FString::Printf(TEXT("%s") PATH_SEPARATOR TEXT("EditorTips.%s"), *LocPath, LangExt);

			TArray<FString> LocFileNames;
			GFileManager->FindFiles(LocFileNames, *SearchPath, TRUE, FALSE);

			if(LocFileNames.Num())
			{
				TipFilename = LocPath * LocFileNames(0);
				GConfig->LoadFile(*TipFilename);
				bFoundFile = TRUE;
				break;
			}
		}
	}
}

wxString WxLocalizedTipProvider::GetTip(INT &TipNumber)
{
	wxString Tip;

	if(TipFilename != TEXT(""))
	{
		FConfigFile* TipFile = GConfig->FindConfigFile( *TipFilename);
		if(TipFile != NULL)
		{
			TipSection = TipFile->Find(TEXT("Tips"));
		}
	}


	if(TipSection != NULL)
	{
		INT NumTips = TipSection->Num();
		TipNumber = TipNumber % NumTips;

		FConfigSection::TIterator TipIterator(*TipSection);

		for(INT Idx=0;Idx<TipNumber;Idx++)
		{
			++TipIterator;
		}

		Tip = *Localize(TEXT("Tips"),*TipIterator.Key(), TEXT("EditorTips"));

		TipNumber++;
	}

	return Tip;
}


/*-----------------------------------------------------------------------------
WxDlgTipOfTheDay
-----------------------------------------------------------------------------*/
BEGIN_EVENT_TABLE(WxDlgTipOfTheDay, wxDialog)
	EVT_BUTTON(IDB_NEXT_TIP, WxDlgTipOfTheDay::OnNextTip)
END_EVENT_TABLE()

WxDlgTipOfTheDay::WxDlgTipOfTheDay(wxWindow* Parent) : 
wxDialog(Parent, wxID_ANY, *LocalizeUnrealEd("DlgTipOfTheDay_Title"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP),
CurrentTip(0)
{
	SetMinSize(wxSize(400,300));

	wxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
	{
		wxSizer* HSizer = new wxBoxSizer(wxHORIZONTAL);
		{
			// Bitmap Panel
			wxPanel* BitmapPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER);
			{
				wxSizer* BitmapSizer = new wxBoxSizer(wxHORIZONTAL);
				{
					wxStaticBitmap *Bitmap = new wxStaticBitmap(BitmapPanel, wxID_ANY, WxBitmap(TEXT("TipOfTheDay")));
					BitmapSizer->Add(Bitmap, 0, wxALIGN_BOTTOM);
				}
				BitmapPanel->SetSizer(BitmapSizer);
				BitmapPanel->SetBackgroundColour(wxColour(187,2,1));
			}
			HSizer->Add(BitmapPanel, 0, wxEXPAND | wxALL, 5);

			// Textboxes
			wxSizer* TextSizer = new wxBoxSizer(wxVERTICAL);
			{
				// Static Label
				wxStaticText *Label = new wxStaticText(this, wxID_ANY, *LocalizeUnrealEd("DlgTipOfTheDay_TipOfTheDay"));
				wxFont Font = Label->GetFont();
				Font.SetPointSize(16);
				Font.SetWeight(wxFONTWEIGHT_BOLD);
				Label->SetFont(Font);
				TextSizer->Add(Label, 0, wxEXPAND | wxBOTTOM, 5);

				// Tip Text
				TipText = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
				TipText->SetEditable(false);

				TextSizer->Add(TipText, 1, wxEXPAND | wxTOP, 5);
			}
			HSizer->Add(TextSizer, 1, wxEXPAND | wxALL, 5);
		}
		MainSizer->Add(HSizer, 1, wxEXPAND | wxALL, 5);

		wxSizer* ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
		{
			CheckStartupShow = new wxCheckBox(this, wxID_ANY, *LocalizeUnrealEd("DlgTipOfTheDay_ShowTipsAtStartup"));
			ButtonSizer->Add(CheckStartupShow, 1, wxALIGN_LEFT | wxEXPAND);

			NextTip = new wxButton(this, IDB_NEXT_TIP, *LocalizeUnrealEd("DlgTipOfTheDay_NextTip"));
			ButtonSizer->Add(NextTip, 0, wxEXPAND | wxLEFT, 5);

			wxButton* OKButton = new wxButton(this, wxID_OK,*LocalizeUnrealEd("OK"));
			ButtonSizer->Add(OKButton, 0, wxEXPAND | wxLEFT, 5);
		}
		MainSizer->Add(ButtonSizer, 0, wxEXPAND | wxALL, 5);
	}
	SetSizer(MainSizer);

	// Load options for the dialog.
	LoadOptions();

	// Display a tip.
	UpdateTip();
}

WxDlgTipOfTheDay::~WxDlgTipOfTheDay()
{
	SaveOptions();
}

/** Saves options for the tip box. */
void WxDlgTipOfTheDay::SaveOptions() const
{
	// Window position/size
	FWindowUtil::SavePosSize(TEXT("DlgTipOfTheDay"), this);

	// Show at startup.
	GConfig->SetBool(TEXT("DlgTipOfTheDay"), TEXT("ShowAtStartup"), CheckStartupShow->GetValue(), GEditorUserSettingsIni);

	// Current Tip
	GConfig->SetInt(TEXT("DlgTipOfTheDay"), TEXT("CurrentTip"), CurrentTip, GEditorUserSettingsIni);
}

/** Loads options for the tip box. */
void WxDlgTipOfTheDay::LoadOptions()
{
	// Window position/size.
	FWindowUtil::LoadPosSize(TEXT("DlgTipOfTheDay"), this, 640, 480, -1,-1);

	// Show at startup.
	UBOOL bShowAtStartup = TRUE;
	GConfig->GetBool(TEXT("DlgTipOfTheDay"), TEXT("ShowAtStartup"), bShowAtStartup, GEditorUserSettingsIni);
	CheckStartupShow->SetValue(bShowAtStartup == TRUE);

	// Current Tip
	UBOOL bLoadedTipCount = GConfig->GetInt(TEXT("DlgTipOfTheDay"), TEXT("CurrentTip"), CurrentTip, GEditorUserSettingsIni);

	if(bLoadedTipCount == FALSE)
	{
		CurrentTip = 0;
	}
}

/** Updates the currently displayed tip with a new randomized tip. */
void WxDlgTipOfTheDay::UpdateTip()
{
	TipText->SetValue(TipProvider.GetTip(CurrentTip));
}

/** Updates the tip text with a new tip. */
void WxDlgTipOfTheDay::OnNextTip(wxCommandEvent &Event)
{
	UpdateTip();
}




