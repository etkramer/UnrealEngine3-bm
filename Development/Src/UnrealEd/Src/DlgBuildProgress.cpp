/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "DlgBuildProgress.h"

BEGIN_EVENT_TABLE(WxDlgBuildProgress, wxDialog)
	EVT_BUTTON( ID_DialogBuildProgress_StopBuild, WxDlgBuildProgress::OnStopBuild )
END_EVENT_TABLE()

WxDlgBuildProgress::WxDlgBuildProgress(wxWindow* InParent) : 
wxDialog(InParent, wxID_ANY, TEXT("DlgBuildProgressTitle"), wxDefaultPosition, wxDefaultSize, wxCAPTION )
{
	static const INT MinDialogWidth = 400;

	wxBoxSizer* VerticalSizer = new wxBoxSizer(wxVERTICAL);
	{
		VerticalSizer->SetMinSize(wxSize(MinDialogWidth, -1));

		wxStaticBoxSizer* BuildStatusSizer = new wxStaticBoxSizer(wxVERTICAL, this, TEXT("BuildStatus"));
		{
			BuildStatusText = new wxStaticText(this, wxID_ANY, TEXT("Default Status Text"));
			BuildStatusSizer->Add(BuildStatusText, 1, wxEXPAND | wxALL, 5);
		}
		VerticalSizer->Add(BuildStatusSizer, 0, wxEXPAND | wxALL, 5);

		wxStaticBoxSizer* BuildProgressSizer = new wxStaticBoxSizer(wxVERTICAL, this, TEXT("BuildProgress"));
		{
			BuildProgress = new wxGauge(this, wxID_ANY, 100);
			BuildProgressSizer->Add(BuildProgress, 1, wxEXPAND);
		}
		VerticalSizer->Add(BuildProgressSizer, 0, wxEXPAND | wxALL, 5);

		StopBuildButton = new wxButton(this, ID_DialogBuildProgress_StopBuild, TEXT("StopBuild"));
		VerticalSizer->Add(StopBuildButton, 0, wxCENTER | wxALL, 5);
	}
	SetSizer(VerticalSizer);

	

	FWindowUtil::LoadPosSize( TEXT("DlgBuildProgress"), this );
	FLocalizeWindow( this );
}

WxDlgBuildProgress::~WxDlgBuildProgress()
{
	FWindowUtil::SavePosSize( TEXT("DlgBuildProgress"), this );
}

/**
 * Displays the build progress dialog.
 */
void WxDlgBuildProgress::ShowDialog()
{
	// Reset progress indicators
	BuildStartTime = -1;
	StopBuildButton->Enable( TRUE );
	SetBuildStatusText(TEXT(""));
	SetBuildProgressPercent( 0, 100 );
	
	Show();
}

/**
 * Assembles a string containing the elapsed build time.
 */
FString WxDlgBuildProgress::BuildElapsedTimeString() const
{
	// Display elapsed build time.
	const DOUBLE ElapsedBuildTimeSeconds	= appSeconds() - BuildStartTime;
	const INT BuildTimeHours				= ElapsedBuildTimeSeconds / 3600.0;
	const INT BuildTimeMinutes				= (ElapsedBuildTimeSeconds - BuildTimeHours*3600) / 60.0;
	const INT BuildTimeSeconds				= appTrunc( ElapsedBuildTimeSeconds - BuildTimeHours*3600 - BuildTimeMinutes*60 );

	if(BuildStartTime < 0)
	{
		return FString(TEXT("0:00"));
	}
	else
	{
		if ( BuildTimeHours > 0 )
		{
			return FString::Printf( TEXT("(%i:%02i:%02i)"), BuildTimeHours, BuildTimeMinutes, BuildTimeSeconds );
		}
		else
		{
			return FString::Printf( TEXT("(%i:%02i)"), BuildTimeMinutes, BuildTimeSeconds );
		}
	}
}

/**
 * Sets the text that describes what part of the build we are currently on.
 *
 * @param StatusText	Text to set the status label to.
 */
void WxDlgBuildProgress::SetBuildStatusText( const TCHAR* StatusText )
{
	const UBOOL bStoppingBuild = !StopBuildButton->IsEnabled();
	
	// Only update the text if we haven't cancelled the build.
	if( !bStoppingBuild )
	{
		const FString TimeAndStatus( FString::Printf( TEXT("%s  %s"), *BuildElapsedTimeString(), StatusText ) );
		const UBOOL bLabelChanged = BuildStatusText->GetLabel() != *TimeAndStatus;
		if( bLabelChanged )
		{
			BuildStatusText->SetLabel( *TimeAndStatus );
		}
	}
}


/**
* Sets the build progress bar percentage.
*
*	@param ProgressNumerator		Numerator for the progress meter (its current value).
*	@param ProgressDenominitator	Denominiator for the progress meter (its range).
*/
void WxDlgBuildProgress::SetBuildProgressPercent( INT ProgressNumerator, INT ProgressDenominator )
{
	const UBOOL bStoppingBuild = !StopBuildButton->IsEnabled();

	// Only update the progress bar if we haven't cancelled the build.
	if( !bStoppingBuild )
	{
		if(BuildProgress->GetRange() != ProgressDenominator)
		{
			BuildProgress->SetRange(ProgressDenominator);
		}
		
		BuildProgress->SetValue(ProgressNumerator);
	}
}

/**
 * Records the application time in seconds; used in display of elapsed build time.
 */
void WxDlgBuildProgress::MarkBuildStartTime()
{
	BuildStartTime = appSeconds();
}

/**
 * Callback for the Stop Build button, stops the current build.
 */
void WxDlgBuildProgress::OnStopBuild(wxCommandEvent& In )
{
	GApp->SetMapBuildCancelled( TRUE );
	
	SetBuildStatusText( *LocalizeUnrealEd("StoppingMapBuild") );

	StopBuildButton->Enable( FALSE );
}
