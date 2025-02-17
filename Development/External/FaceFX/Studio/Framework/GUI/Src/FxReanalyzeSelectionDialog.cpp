//------------------------------------------------------------------------------
// A reanalyze selection dialog.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxStudioApp.h"
#include "FxReanalyzeSelectionDialog.h"
#include "FxTearoffWindow.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/textfile.h"
#endif

namespace OC3Ent
{

namespace Face
{

FxRSDStubWidget::
FxRSDStubWidget( FxWidgetMediator* mediator, FxReanalyzeSelectionDialog* pRSD )
	: FxWidget(mediator)
	, _pRSD(pRSD)
{
}

FxRSDStubWidget::~FxRSDStubWidget()
{
}

void FxRSDStubWidget::
OnPlayAnimation( FxWidget* FxUnused(sender), FxReal FxUnused(startTime), 
				 FxReal FxUnused(endTime), FxBool FxUnused(loop) )
{
	if( _pRSD )
	{
		_pRSD->SetIcons(FxTrue);
	}
}

void FxRSDStubWidget::OnStopAnimation( FxWidget* FxUnused(sender) )
{
	if( _pRSD )
	{
		_pRSD->SetIcons(FxFalse);
	}
}

void FxRSDStubWidget::OnAnimationStopped( FxWidget* FxUnused(sender) )
{
	if( _pRSD )
	{
		_pRSD->SetIcons(FxFalse);
	}
}

WX_IMPLEMENT_DYNAMIC_CLASS(FxReanalyzeSelectionDialog, wxDialog)

BEGIN_EVENT_TABLE(FxReanalyzeSelectionDialog, wxDialog)
	EVT_BUTTON(ControlID_ReanalyzeSelectionDlgBrowseForTextButton, FxReanalyzeSelectionDialog::OnBrowseForTextButtonPressed)
	EVT_BUTTON(ControlID_ReanalyzeSelectionDlgPlayAudioButton, FxReanalyzeSelectionDialog::OnPlayAudioButtonPressed)
	EVT_BUTTON(ControlID_ReanalyzeSelectionDlgReanalyzeButton, FxReanalyzeSelectionDialog::OnReanalyzeButtonPressed)
	EVT_BUTTON(ControlID_ReanalyzeSelectionDlgCancelButton, FxReanalyzeSelectionDialog::OnCancelButtonPressed)
END_EVENT_TABLE()

FxReanalyzeSelectionDialog::FxReanalyzeSelectionDialog()
	: _pMediator(0)
	, _pPhonemeWordList(0)
	, _pStubWidget(0)
	, _isPlaying(FxFalse)
	, _animTextEditBox(0)
	, _browseForTextButton(0)
	, _playAudioButton(0)
	, _reanalyzeButton(0)
	, _cancelButton(0)
{
}

FxReanalyzeSelectionDialog::
FxReanalyzeSelectionDialog( wxWindow* parent, wxWindowID id, const wxString& caption, 
							const wxPoint& pos, const wxSize& size, long style )
							: _pMediator(0)
							, _pPhonemeWordList(0)
							, _pStubWidget(0)
							, _isPlaying(FxFalse)
							, _animTextEditBox(0)
							, _browseForTextButton(0)
							, _playAudioButton(0)
							, _reanalyzeButton(0)
							, _cancelButton(0)
{
	Create(parent, id, caption, pos, size, style);
}

FxReanalyzeSelectionDialog::~FxReanalyzeSelectionDialog()
{
	if( _pStubWidget )
	{
		delete _pStubWidget;
		_pStubWidget = 0;
	}
}

FxBool FxReanalyzeSelectionDialog::
Create( wxWindow* parent, wxWindowID id, const wxString& caption, 
	    const wxPoint& pos, const wxSize& size, long style )
{
	SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
	SetExtraStyle(GetExtraStyle()|wxWS_EX_VALIDATE_RECURSIVELY);
	wxDialog::Create(parent, id, caption, pos, size, style);

	CreateControls();

	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();
	return TRUE;
}

void FxReanalyzeSelectionDialog::
Initialize( FxWidgetMediator* mediator, FxPhonWordList* pPhonemeWordList )
{
	_pMediator = mediator;
	_pPhonemeWordList = pPhonemeWordList;
	_pStubWidget = new FxRSDStubWidget(_pMediator, this);
}

void FxReanalyzeSelectionDialog::CreateControls( void )
{
	wxBoxSizer* boxSizerV = new wxBoxSizer(wxVERTICAL);
	SetSizer(boxSizerV);
	SetAutoLayout(TRUE);

	_playEnabled = wxIcon(wxIconLocation(FxStudioApp::GetIconPath(wxT("play.ico"))));
	_playEnabled.SetWidth(16);
	_playEnabled.SetHeight(16);
	_playDisabled = wxIcon(wxIconLocation(FxStudioApp::GetIconPath(wxT("play_disabled.ico"))));
	_playDisabled.SetWidth(16);
	_playDisabled.SetHeight(16);

	_stopEnabled = wxIcon(wxIconLocation(FxStudioApp::GetIconPath(wxT("stop.ico"))));
	_stopEnabled.SetWidth(16);
	_stopEnabled.SetHeight(16);
	_stopDisabled = wxIcon(wxIconLocation(FxStudioApp::GetIconPath(wxT("stop_disabled.ico"))));
	_stopDisabled.SetWidth(16);
	_stopDisabled.SetHeight(16);

	wxStaticBox* reanalyzeParametersGroupBox = new wxStaticBox(this, wxID_STATIC, _("Reanalyze parameters"));
	wxStaticBoxSizer* reanalyzeParametersSizer = new wxStaticBoxSizer(reanalyzeParametersGroupBox, wxVERTICAL);
	boxSizerV->Add(reanalyzeParametersSizer, 0, wxGROW|wxALL, 5);

	wxStaticText* textLabel = new wxStaticText(this, wxID_STATIC, _("Animation text"), wxDefaultPosition, wxDefaultSize);
	reanalyzeParametersSizer->Add(textLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

	wxBoxSizer* animTextSizer = new wxBoxSizer(wxHORIZONTAL);
	reanalyzeParametersSizer->Add(animTextSizer, 0, wxALIGN_LEFT, 5);

	_animTextEditBox = new wxTextCtrl(this, ControlID_ReanalyzeSelectionDlgAnimTextEditBox, _T(""), wxDefaultPosition, wxSize(300, 100), wxTE_MULTILINE);
	_animTextEditBox->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, FxStudioOptions::GetUnicodeFontName(), wxFONTENCODING_SYSTEM));

	animTextSizer->Add(_animTextEditBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	if( !_analysisText.IsEmpty() )
	{
		_animTextEditBox->SetValue(wxString(_analysisText.GetData()));
	}

	wxBoxSizer* browseSizer = new wxBoxSizer(wxVERTICAL);
	animTextSizer->Add(browseSizer, 0, wxALIGN_LEFT, 5);

	_browseForTextButton = new wxButton(this, ControlID_ReanalyzeSelectionDlgBrowseForTextButton, _("Browse..."), wxDefaultPosition, wxDefaultSize);
	browseSizer->Add(_browseForTextButton, 0, wxALIGN_TOP|wxALL, 5);

	_playAudioButton = new FxButton(this, ControlID_ReanalyzeSelectionDlgPlayAudioButton, wxT(">"), wxDefaultPosition, wxSize(20,20));
	_playAudioButton->SetToolTip(_("Play the current time range"));
	browseSizer->Add(_playAudioButton, 0, wxALIGN_TOP|wxALL, 5);

	wxBoxSizer* bottomButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	boxSizerV->Add(bottomButtonSizer, 0, wxALIGN_RIGHT, 5);

	_reanalyzeButton = new wxButton(this, ControlID_ReanalyzeSelectionDlgReanalyzeButton, _("Reanalyze"), wxDefaultPosition, wxDefaultSize);
	bottomButtonSizer->Add(_reanalyzeButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	
	_cancelButton = new wxButton(this, ControlID_ReanalyzeSelectionDlgCancelButton, _("Cancel"), wxDefaultPosition, wxDefaultSize);
	bottomButtonSizer->Add(_cancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	SetIcons(FxFalse);
}

void FxReanalyzeSelectionDialog::SetIcons( FxBool isPlaying )
{
	_isPlaying = isPlaying;
	if( !_isPlaying )
	{
		_playAudioButton->SetEnabledBitmap(_playEnabled);
		_playAudioButton->SetDisabledBitmap(_playDisabled);
	}
	else
	{
		_playAudioButton->SetEnabledBitmap(_stopEnabled);
		_playAudioButton->SetDisabledBitmap(_stopDisabled);
	}
	_playAudioButton->Refresh();
}

const FxWString& FxReanalyzeSelectionDialog::GetAnalysisText( void ) const
{
	return _analysisText;
}

void FxReanalyzeSelectionDialog::SetAnalysisText( const FxWString& analysisText )
{
	_analysisText = analysisText;
}

FxBool FxReanalyzeSelectionDialog::ShowToolTips( void )
{
	return FxTrue;
}

void FxReanalyzeSelectionDialog::OnBrowseForTextButtonPressed( wxCommandEvent& FxUnused(event) )
{
	wxFileDialog fileDlg(this, _("Select a text file"), wxEmptyString, wxEmptyString, _("Text files (*.txt)|*.txt|All Files (*.*)|*.*"), wxFD_OPEN);
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	if( wxID_OK == fileDlg.ShowModal() )
	{
		if( _animTextEditBox )
		{
			wxTextFile textFile(fileDlg.GetPath());
			if( textFile.Exists() )
			{
				if( textFile.Open(fileDlg.GetPath()) )
				{
					_animTextEditBox->Clear();
					FxString analysisText;
					for( FxSize i = 0; i < textFile.GetLineCount(); ++i )
					{
						analysisText = FxString::Concat(analysisText, FxString(textFile.GetLine(i).mb_str(wxConvLibc)));
						analysisText = FxString::Concat(analysisText, " ");
					}
					_animTextEditBox->SetValue(wxString(analysisText.GetData(), wxConvLibc));
				}
			}
			else
			{
				_animTextEditBox->Clear();
			}
		}
	}
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
}

void FxReanalyzeSelectionDialog::OnPlayAudioButtonPressed( wxCommandEvent& FxUnused(event) )
{
	if( _pMediator && _pPhonemeWordList )
	{
		if( !_isPlaying )
		{
			FxReal selectionStart = FX_REAL_MAX;
			FxReal selectionEnd   = FX_REAL_MIN;
			// Calculate the extents of the selection.
			for( FxSize i = 0; i < _pPhonemeWordList->GetNumPhonemes(); ++i )
			{
				if( _pPhonemeWordList->GetPhonemeSelected(i) )
				{
					selectionStart = FxMin(selectionStart, _pPhonemeWordList->GetPhonemeStartTime(i));
					selectionEnd   = FxMax(selectionEnd, _pPhonemeWordList->GetPhonemeEndTime(i));
				}
			}

			if( selectionEnd > selectionStart )
			{
				// Play the current selection range of audio.
				_pMediator->OnPlayAnimation(NULL, selectionStart, selectionEnd);
			}
			else if( selectionStart == FX_REAL_MAX && selectionEnd == FX_REAL_MIN )
			{
				// Play the full range of audio.
				_pMediator->OnPlayAnimation(NULL);
			}
		}
		else
		{
			_pMediator->OnStopAnimation(NULL);
		}
	}
}

void FxReanalyzeSelectionDialog::OnReanalyzeButtonPressed( wxCommandEvent& FxUnused(event) )
{
	if( _animTextEditBox )
	{
		// If there is text in the animation text edit box, use it
		// as the optional analysis text.
		wxString animText = _animTextEditBox->GetValue();
		animText.Trim();
		if( wxEmptyString != animText )
		{
			_analysisText = FxWString(animText.GetData());	
		}
		else
		{
			_analysisText = L"";
		}
	}
	EndDialog(wxID_OK);
}

void FxReanalyzeSelectionDialog::OnCancelButtonPressed( wxCommandEvent& FxUnused(event) )
{
	EndDialog(wxID_CANCEL);
}

} // namespace Face

} // namespace OC3Ent
