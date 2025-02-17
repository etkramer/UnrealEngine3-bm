//------------------------------------------------------------------------------
// Simple progress dialog implementing FaceFx-style progress callback.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxProgressDialog.h"
#include "FxConsole.h"

#include <iostream>
#include <stdio.h>

namespace OC3Ent
{

namespace Face
{

BEGIN_EVENT_TABLE(FxProgressDialog, wxDialog)
	EVT_CLOSE(FxProgressDialog::OnClose)
	EVT_BUTTON(wxID_CANCEL, FxProgressDialog::OnCancel)
END_EVENT_TABLE()

WX_IMPLEMENT_CLASS(FxProgressDialog, wxDialog)

#define LAYOUT_X_MARGIN 8
#define LAYOUT_Y_MARGIN 8

FxBool FxProgressDialog::IsConsoleMode = FxFalse;

FxProgressDialog::
FxProgressDialog( const wxString& message, wxWindow* parent, FxBool isCancellable )
	: wxDialog(parent, -1, wxString(_("")))
	, _state(updating)
	, _range(100)
	, _hasBeenCancelled(FxFalse)
	, _pMessageLabel(NULL)
{
	if( IsConsoleMode )
	{
		std::cout << "// " << message.mb_str(wxConvLibc) << std::endl;
	}
	else
	{
		// Remove the title bar from the dialog.
		SetWindowStyle(0);
		// We can go away any time.
		SetExtraStyle(GetExtraStyle()|wxWS_EX_TRANSIENT);

		// The main sizer for the progress dialog.
		wxBoxSizer* dialogSizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(dialogSizer);
		SetAutoLayout(TRUE);

		_pMessageLabel = new wxStaticText(this, -1, message);
		dialogSizer->Add(_pMessageLabel, 1, wxALIGN_LEFT|wxLEFT|wxTOP|wxRIGHT|wxGROW|wxADJUST_MINSIZE, 5);

		// The bottom sizer for the progress bar and percent complete label.
		wxBoxSizer* progressSizer = new wxBoxSizer(wxHORIZONTAL);
		dialogSizer->Add(progressSizer, 1, wxALIGN_LEFT|wxGROW, 0);
		// Create the progress bar.
		_pProgressBar = new wxGauge(this, -1, _range, wxDefaultPosition, 
			wxSize(200, 20), wxGA_HORIZONTAL|wxGA_SMOOTH);
		progressSizer->Add(_pProgressBar, 1, wxALIGN_LEFT|wxALL|wxGROW, 5);
		_pProgressBar->SetValue(0);
		wxSize progressBarSize = _pProgressBar->GetSize();

		_pPercentCompleteLabel = new wxStaticText(this, -1, wxT("0%"));
		progressSizer->Add(_pPercentCompleteLabel, 0, wxALIGN_LEFT|wxTOP|wxBOTTOM|wxRIGHT, 5);
		wxSize percentCompleteLabelSize = _pPercentCompleteLabel->GetSize();

		// stupid cancel button doesn't work.
		if( isCancellable )
		{
			wxButton* cancelButton = new wxButton(this, wxID_CANCEL, _("Cancel"));
			dialogSizer->Add(cancelButton, 0, wxALL|wxALIGN_LEFT, 5);
		}
		// Force the progress bar color to be blue.
		_pProgressBar->SetForegroundColour(*wxBLUE);

		Layout();
		Fit();

		Centre(wxCENTER_FRAME|wxBOTH);
		
		if( isCancellable )
		{
			// Move the progress display down a little bit so the transient 
			// progress windows that pop up during batch analysis can be seen.
			wxRect windowRect = GetRect();
			windowRect.SetTop(windowRect.GetTop() + windowRect.GetHeight());
			SetSize(windowRect);
		}

		// Make this window "app modal."
		_pWinDisabler = new wxWindowDisabler(this);
		Show(TRUE);
		Enable(TRUE);

		// Update immediately.
		wxSafeYield();

#ifdef __WXMAC__
		//MacUpdateImmediately();
		Refresh(FALSE);
#endif
	}
}

FxProgressDialog::~FxProgressDialog()
{
	if( !IsConsoleMode )
	{
		ReleaseWindowDisabler();
	}
}

void FxProgressDialog::Update( FxReal percentComplete )
{
	if( IsConsoleMode )
	{
		static FxBool started   = FxFalse;
		static FxSize count     = 0;

		// Put percentComplete into the 0.0 to 100.0 range.
		percentComplete *= 100.0f;

		// Use some old-school printf magic to do a 'spinner' progress indicator.
		if( started )
		{
			for( FxSize i = 0; i < 20 ; ++i )
			{
				printf("\x08");
			}
		}

		started = FxTrue;

		switch( count )
		{
		case 0:
			count++;
			printf("/");
			break;
		case 1:
			count++;
			printf("-");
			break;
		case 2:
			count++;
			printf("\\");
			break;
		case 3:
			count = 0;
			printf("|");
			break;
		default:
			break;
		}

		printf(" %.2f%% complete", percentComplete);

		if( percentComplete >= 100.0f )
		{
			for( FxSize i = 0; i < 21 ; ++i )
			{
				printf("\x08");
			}
			printf("// 100.00%% complete\n");
			started = FxFalse;
			count   = 0;
		}
		// Flush cout to make sure everything is written immediately.
		std::cout << std::flush;
	}
	else
	{
		FxInt32 value = percentComplete * _range;

		if( _pProgressBar )
		{
			_pProgressBar->SetValue(value);
		}

		if( _pPercentCompleteLabel )
		{
			_pPercentCompleteLabel->SetLabel(wxString::Format(wxT("%d%%"), value));
		}

		// Auto-hide.
		if( value == _range )
		{
			ReleaseWindowDisabler();
			Hide();
		}
		else
		{
			wxSafeYield();
		}

#ifdef __WXMAC__
		//MacUpdateImmediately();
		Refresh(FALSE);
#endif
	}
}

bool FxProgressDialog::Show( bool show )
{
	if( IsConsoleMode )
	{
		return false;
	}
	else
	{
		if( !show )
		{
			ReleaseWindowDisabler();
		}
		return wxDialog::Show(show);
	}
}

FxBool FxProgressDialog::HasBeenCancelled()
{
	return _hasBeenCancelled;
}

void FxProgressDialog::SetProgressText(const wxString& progressText)
{
	_message = progressText;
	if( _pMessageLabel )
	{
		_pMessageLabel->SetLabel(_message);
		Layout();
		Fit();
	}
}

void FxProgressDialog::OnClose( wxCloseEvent& event )
{
	if( !IsConsoleMode )
	{
		if( _state == updating )
		{
			// Don't let the user close the progress dialog.
			event.Veto(TRUE);
		}
		else
		{
			// Must be finished, allow the progress dialog to close.
			event.Skip();
		}
	}
}

void FxProgressDialog::OnCancel( wxCommandEvent& FxUnused(event) )
{
	_hasBeenCancelled = FxTrue;
}

void FxProgressDialog::ReleaseWindowDisabler( void )
{
	if( _pWinDisabler )
	{
		delete _pWinDisabler;
		_pWinDisabler = NULL;
	}
}

FxProgressDialog* FxArchiveProgressDisplay::_progressDlg = 0;
void FxArchiveProgressDisplay::Begin( wxString message, wxWindow* parent )
{
	_progressDlg = new FxProgressDialog(message, parent);
}

void FxArchiveProgressDisplay::End( void )
{
	if( _progressDlg )
	{
		_progressDlg->Update(1.0f);
		delete _progressDlg;
		_progressDlg = 0;
	}
}

void FxArchiveProgressDisplay::UpdateProgress( FxReal progress )
{
	if( _progressDlg )
	{
		_progressDlg->Update(progress);
	}
}

} // namespace Face

} // namespace OC3Ent
