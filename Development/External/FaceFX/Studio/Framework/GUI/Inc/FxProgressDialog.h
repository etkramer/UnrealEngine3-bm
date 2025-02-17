//------------------------------------------------------------------------------
// Simple progress dialog implementing FaceFx-style progress callback.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------


#ifndef FxProgressDialog_H__
#define FxProgressDialog_H__

#include "FxPlatform.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/dialog.h"
#endif

namespace OC3Ent
{

namespace Face
{

// FaceFx progress dialog based partially off of wxProgressDialog.
class FxProgressDialog : public wxDialog
{
public:
	WX_DECLARE_DYNAMIC_CLASS(FxProgressDialog)

	// Constructor.
	FxProgressDialog( const wxString& message, wxWindow* parent = NULL, FxBool isCancellable = FxFalse );
	// Destructor.
	virtual ~FxProgressDialog();
	
	// FaceFx progress callback function.
	void Update( FxReal percentComplete );

	// Show the progress dialog.
	bool Show( bool show = TRUE );

	// Checks if the progess dialog has been cancelled.
	FxBool HasBeenCancelled();
	// Sets the dialog text
	void SetProgressText(const wxString& progressText);

	// Internal use only.  This is set to FxTrue if the application is currently
	// in console mode.
	static FxBool IsConsoleMode;

protected:
	// Disable window closing.
	void OnClose( wxCloseEvent& event );

	// Handle the cancel button press
	void OnCancel( wxCommandEvent& event );

	// Releases the window disabler to re-enable all other windows.
	void ReleaseWindowDisabler( void );

private:
	DECLARE_EVENT_TABLE()

	// The state of the progress dialog.
	enum eProgressState
	{
		updating,
		finished
	};
	eProgressState _state;

	// All FaceFx progress dialogs range from 0 to _range (100%).
	FxInt32 _range;

	// The progress bar.
	wxGauge* _pProgressBar;

	// The cancel button.
	wxButton* _cancelButton;

	// Whether the progress dialog has been cancelled.
	FxBool _hasBeenCancelled;

	// The percentage complete text label.
	wxStaticText* _pPercentCompleteLabel;
	// The message label.
	wxStaticText* _pMessageLabel;
	// The message
	wxString _message;

	// All FaceFx progress dialogs are "app modal."
	class WXDLLEXPORT wxWindowDisabler* _pWinDisabler;
};

// Uses the progress dialog to show progress while loading archives.
class FxArchiveProgressDisplay
{
public:
	// Call before beginning loading from an archive.
	static void Begin( wxString message, wxWindow* parent );
	// Call after loading from an archive.
	static void End( void );

	// Internal use only.
	static void UpdateProgress( FxReal progress );
protected:
	static FxProgressDialog* _progressDlg;
};

} // namespace Face

} // namespace OC3Ent

#endif
