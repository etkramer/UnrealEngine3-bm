/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "DlgProgress.h"

BEGIN_EVENT_TABLE(WxDlgProgress, wxDialog)
END_EVENT_TABLE()

WxDlgProgress::WxDlgProgress(wxWindow* InParent) : 
	wxDialog(InParent, wxID_ANY, TEXT("DlgProgressTitle"), wxDefaultPosition, wxDefaultSize, wxCAPTION )
{
	static const INT MinDialogWidth = 500;

	wxBoxSizer* VerticalSizer = new wxBoxSizer(wxVERTICAL);
	{
		VerticalSizer->SetMinSize(wxSize(MinDialogWidth, -1));

		wxStaticBoxSizer* StatusSizer = new wxStaticBoxSizer(wxVERTICAL, this, TEXT("Status"));
		{
			StatusText = new wxStaticText(this, wxID_ANY, TEXT(""));
			StatusSizer->Add(StatusText, 1, wxEXPAND | wxALL, 5);
		}
		VerticalSizer->Add(StatusSizer, 0, wxEXPAND | wxALL, 5);

		wxStaticBoxSizer* ProgressSizer = new wxStaticBoxSizer(wxVERTICAL, this, TEXT("Progress"));
		{
			Progress = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL /*| wxGA_SMOOTH*/ );
			ProgressSizer->Add(Progress, 1, wxEXPAND);
		}
		VerticalSizer->Add(ProgressSizer, 0, wxEXPAND | wxALL, 5);
	}

	SetSizer(VerticalSizer);

	FWindowUtil::LoadPosSize( TEXT("DlgProgress"), this );
	FLocalizeWindow( this );
}

WxDlgProgress::~WxDlgProgress()
{
	FWindowUtil::SavePosSize( TEXT("DlgProgress"), this );
}

/**
 * Displays the progress dialog.
 */
void WxDlgProgress::ShowDialog()
{
	// Reset progress indicators
	StartTime = -1;
	MarkStartTime();

	SetStatusText(TEXT(""));
	SetProgressPercent( 0, 0 );

	Show();
	Update();
}

/**
 * Assembles a string containing the elapsed time.
 */
FString WxDlgProgress::BuildElapsedTimeString() const
{
	// Display elapsed time.
	const DOUBLE ElapsedTimeSeconds	= appSeconds() - StartTime;
	const INT TimeHours				= ElapsedTimeSeconds / 3600.0;
	const INT TimeMinutes			= (ElapsedTimeSeconds - TimeHours*3600) / 60.0;
	const INT TimeSeconds			= appTrunc( ElapsedTimeSeconds - TimeHours*3600 - TimeMinutes*60 );

	if(StartTime < 0)
	{
		return FString(TEXT("0:00"));
	}
	else
	{
		if ( TimeHours > 0 )
		{
			return FString::Printf( TEXT("(%i:%02i:%02i)"), TimeHours, TimeMinutes, TimeSeconds );
		}
		else
		{
			return FString::Printf( TEXT("(%i:%02i)"), TimeMinutes, TimeSeconds );
		}
	}
}

/**
 * Sets the text that describes what part of the operation we are currently on.
 *
 * @param InStatusText	Text to set the status label to.
 */
void WxDlgProgress::SetStatusText( const TCHAR* InStatusText )
{
	const FString TimeAndStatus( FString::Printf( TEXT("%s  %s"), *BuildElapsedTimeString(), InStatusText ) );
	const UBOOL bLabelChanged = (StatusText->GetLabel() != *TimeAndStatus);
	if( bLabelChanged )
	{
		StatusText->SetLabel( *TimeAndStatus );

		// Force a repaint so the user sees a smoother progress bar
		StatusText->Update();
	}
}

/**
* Sets the progress bar percentage.
*
*	@param ProgressNumerator		Numerator for the progress meter (its current value).
*	@param ProgressDenominitator	Denominator for the progress meter (its range).
*/
void WxDlgProgress::SetProgressPercent( INT ProgressNumerator, INT ProgressDenominator )
{
	if( ProgressDenominator == 0 )
	{
		// Marquee-style progress bars seem to require a specific range to be set
		if( Progress->GetRange() != 100 )
		{
			Progress->SetRange( 100 );
		}
		if( Progress->GetValue() != 0 )
		{
			Progress->SetValue( 0 );
		}

		// No progress denominator, so use 'indeterminate mode'
		Progress->SetIndeterminateMode();

#if _WINDOWS
		// Activate marquee progress bar
		HWND ProgressWindowHandle = ( HWND )Progress->GetHWND();
		::SendMessage( ProgressWindowHandle, (UINT)( WM_USER+10 )/* PBM_SETMARQUEE */, (WPARAM)1, (LPARAM)5 );
#endif
	}
	else
	{
		Progress->SetDeterminateMode();

#if _WINDOWS
		// Deactivate marquee progress bar
		HWND ProgressWindowHandle = ( HWND )Progress->GetHWND();
		::SendMessage( ProgressWindowHandle, (UINT)( WM_USER+10 )/* PBM_SETMARQUEE */, (WPARAM)0, (LPARAM)100 );
#endif

  		if( Progress->GetRange() != ProgressDenominator )
		{
			Progress->SetRange( ProgressDenominator );
		}
		
		Progress->SetValue( ProgressNumerator );

		// Force a repaint so the user sees a smoother progress bar
		Progress->Update();
	}
}

/**
 * Records the application time in seconds; used in display of elapsed time.
 */
void WxDlgProgress::MarkStartTime()
{
	StartTime = appSeconds();
}
