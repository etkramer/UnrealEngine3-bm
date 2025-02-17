/*=============================================================================
	DlgBuildProgress.h: UnrealEd dialog for displaying map build progress and cancelling builds.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DLGBUILDPROGRESS_H__
#define __DLGBUILDPROGRESS_H__

/**
  * UnrealEd dialog for displaying map build progress and cancelling builds.
  */
class WxDlgBuildProgress : public wxDialog
{
public:
	WxDlgBuildProgress( wxWindow* InParent );
	virtual ~WxDlgBuildProgress();

	/**
	 * Displays the build progress dialog.
	 */
	void ShowDialog();
	
	/**
	 * Sets the text that describes what part of the build we are currently on.
	 *
	 * @param StatusText	Text to set the status label to.
	 */
	void SetBuildStatusText( const TCHAR* StatusText );

	/**
	 * Sets the build progress bar percentage.
	 *
	 *	@param ProgressNumerator		Numerator for the progress meter (its current value).
	 *	@param ProgressDenominitator	Denominiator for the progress meter (its range).
	 */
	void SetBuildProgressPercent( INT ProgressNumerator, INT ProgressDenominator );

	/**
	 * Records the application time in seconds; used in display of elapsed build time.
	 */
	void MarkBuildStartTime();

	/**
	 * Assembles a string containing the elapsed build time.
	 */
	FString BuildElapsedTimeString() const;

private:
	/**
	 * Callback for the Stop Build button, stops the current build.
	 */
	void OnStopBuild( wxCommandEvent& In );

	/** Progress bar that shows how much of the build has finished. */
	wxGauge*		BuildProgress;

	/** Displays some status info about the build. */
	wxStaticText*	BuildStatusText;

	/** The stop build button */
	wxButton*		StopBuildButton;

	/** Application time in seconds at which the build began. */
	DOUBLE			BuildStartTime;

	DECLARE_EVENT_TABLE()
};

#endif // __DLGBUILDPROGRESS_H__
