/*=============================================================================
	DlgProgress.h: UnrealEd dialog for displaying progress for slow operations.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DLGPROGRESS_H__
#define __DLGPROGRESS_H__

/**
  * UnrealEd dialog for displaying progress during slow operations.
  */
class WxDlgProgress : public wxDialog
{
public:
	WxDlgProgress( wxWindow* InParent );
	virtual ~WxDlgProgress();

	/**
	 * Displays the progress dialog.
	 */
	void ShowDialog();
	
	/**
	 * Sets the text that describes what part of the we are currently on.
	 *
	 * @param InStatusText	Text to set the status label to.
	 */
	void SetStatusText( const TCHAR* InStatusText );

	/**
	 * Sets the progress bar percentage.
	 *
	 *	@param ProgressNumerator		Numerator for the progress meter (its current value).
	 *	@param ProgressDenominitator	Denominator for the progress meter (its range).
	 */
	void SetProgressPercent( INT ProgressNumerator, INT ProgressDenominator );

	/**
	 * Records the application time in seconds; used in display of elapsed time.
	 */
	void MarkStartTime();

	/**
	 * Assembles a string containing the elapsed time.
	 */
	FString BuildElapsedTimeString() const;

private:

	/** Progress bar that shows how much of the operation has finished. */
	wxGauge*		Progress;

	/** Displays some status info about the operation. */
	wxStaticText*	StatusText;

	/** Application time in seconds at which the operation began. */
	DOUBLE			StartTime;

	DECLARE_EVENT_TABLE()
};

#endif // __DLGPROGRESS_H__
