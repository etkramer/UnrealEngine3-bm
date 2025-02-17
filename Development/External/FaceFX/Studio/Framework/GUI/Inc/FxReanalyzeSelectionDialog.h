//------------------------------------------------------------------------------
// A reanalyze selection dialog.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxReanalyzeSelectionDialog_H__
#define FxReanalyzeSelectionDialog_H__

#include "FxPlatform.h"
#include "FxWidget.h"
#include "FxButton.h"
#include "FxPhonWordList.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#endif

namespace OC3Ent
{

namespace Face
{

#ifndef wxCLOSE_BOX
	#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
	#define wxFIXED_MINSIZE 0
#endif

// Forward declare the reanalyze selection dialog.
class FxReanalyzeSelectionDialog;

// A stub widget for the reanalyze selection dialog that is only used to receive 
// the animation stopped message, the play, and the stop animation messages.
class FxRSDStubWidget : public FxWidget
{
public:
	// Constructor.
	FxRSDStubWidget( FxWidgetMediator* mediator = NULL, 
		             FxReanalyzeSelectionDialog* pRSD = NULL );	
	// Destructor.
	virtual ~FxRSDStubWidget();

	// FxWidget message handlers.
	virtual void OnPlayAnimation( FxWidget* sender, FxReal startTime = FxInvalidValue,
		FxReal endTime = FxInvalidValue, FxBool loop = FxFalse );
	virtual void OnStopAnimation( FxWidget* sender );
	virtual void OnAnimationStopped( FxWidget* sender );

protected:
	FxReanalyzeSelectionDialog* _pRSD;
};

// A reanalyze selection dialog.
class FxReanalyzeSelectionDialog : public wxDialog
{
	WX_DECLARE_DYNAMIC_CLASS(FxReanalyzeSelectionDialog)
	DECLARE_EVENT_TABLE()
	typedef wxDialog Super;

public:
	// Constructors.
	FxReanalyzeSelectionDialog();
	FxReanalyzeSelectionDialog( wxWindow* parent, wxWindowID id = 10005, 
		                        const wxString& caption = _("Reanalyze Selection"), 
		                        const wxPoint& pos = wxDefaultPosition, 
		                        const wxSize& size = wxSize(400,300), 
		                        long style = wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX );
	// Destructor.
	virtual ~FxReanalyzeSelectionDialog();

	// Creation.
	FxBool Create( wxWindow* parent, wxWindowID id = 10005, 
		           const wxString& caption = _("Reanalyze Selection"), 
		           const wxPoint& pos = wxDefaultPosition, 
		           const wxSize& size = wxSize(400,300), 
		           long style = wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX );

	// Initialization.  Must be called after Create() but before 
	// ShowModal().
	void Initialize( FxWidgetMediator* mediator, FxPhonWordList* pPhonemeWordList );

	// Creates the controls and sizers.
	void CreateControls( void );

	// Sets the state of the play button icons.
	void SetIcons( FxBool isPlaying );

	// Returns the text that was in the text edit box after the reanalyze
	// button was pressed.
	const FxWString& GetAnalysisText( void ) const;
	// Sets the text in the text edit box.  Must be called before Create().
	void SetAnalysisText( const FxWString& analysisText );

	// Should we show tooltips?
	static FxBool ShowToolTips( void );

protected:
	// The mediator.
	FxWidgetMediator* _pMediator;
	// The phoneme word list.
	FxPhonWordList* _pPhonemeWordList;
	// The stub widget.
	FxRSDStubWidget* _pStubWidget;
	// FxTrue if audio is currently playing.
	FxBool _isPlaying;
	// The analysis text.
	FxWString _analysisText;

	// The animation text edit box.
	wxTextCtrl* _animTextEditBox;
	// The text browse button.
	wxButton* _browseForTextButton;
	// The play audio button.
	FxButton* _playAudioButton;
	// The reanalyze button.
	wxButton* _reanalyzeButton;
	// The cancel button.
	wxButton* _cancelButton;

	wxIcon _playEnabled;
	wxIcon _playDisabled;
	wxIcon _stopEnabled;
	wxIcon _stopDisabled;

	// Event handlers.
	void OnBrowseForTextButtonPressed( wxCommandEvent& event );
	void OnPlayAudioButtonPressed( wxCommandEvent& event );
	void OnReanalyzeButtonPressed( wxCommandEvent& event );
	void OnCancelButtonPressed( wxCommandEvent& event );
};

} // namespace Face

} // namespace OC3Ent

#endif
