//------------------------------------------------------------------------------
// A widget for editing phoneme and word timings.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxPhonemeWidget_H__
#define FxPhonemeWidget_H__

#include "FxPlatform.h"
#include "FxTimeManager.h"
#include "FxWidget.h"
#include "FxPhonWordList.h"

namespace OC3Ent
{

namespace Face
{

// Forward declare the console variable.
class FxConsoleVariable;

class FxPhonemeWidget : public wxWindow, public FxTimeSubscriber, public FxWidget
{
	typedef wxWindow Super;
	WX_DECLARE_DYNAMIC_CLASS(FxPhonemeWidget)
	DECLARE_EVENT_TABLE()

	friend class FxPhonemeWidgetOptionsDlg;

public:
	// Constructor.
	FxPhonemeWidget( wxWindow* parent = NULL, FxWidgetMediator* mediator = NULL );
	// Destructor.
	~FxPhonemeWidget();

	// Overload the FxTimeSubscriber's notification method.
	void OnNotifyTimeChange();
	FxReal GetMinimumTime();
	FxReal GetMaximumTime();
	FxInt32 GetPaintPriority();

	// FxWidget message handlers.
	virtual void OnAppShutdown( FxWidget* sender );
	virtual void OnAnimChanged( FxWidget* sender, const FxName& animGroupName, 
		FxAnim* anim);
	virtual void OnTimeChanged(FxWidget* sender, FxReal newTime );
	virtual void OnRefresh( FxWidget* sender );
	virtual void OnSerializeOptions( FxWidget* sender, wxFileConfig* config, FxBool isLoading );

protected:
	// Mouse wheel.
	void OnMouseWheel( wxMouseEvent& event );
	// Mouse movement.
	void OnMouseMove( wxMouseEvent& event );
	// Left button down
	void OnLButtonDown( wxMouseEvent& event );
	// Left button up
	void OnLButtonUp( wxMouseEvent& event );
	// Left button doubleclick
	void OnLeftDoubleClick( wxMouseEvent& event );
	// Right button down
	void OnRButtonClick( wxMouseEvent& event );
	// Middle button down
	void OnMiddleButtonDown( wxMouseEvent& event );
	// Middle button up
	void OnMiddleButtonUp( wxMouseEvent& event );

	// Painting.
	void OnPaint( wxPaintEvent& event );

	// Resets the view
	void OnResetView( wxCommandEvent& event );
	// Plays the selection
	void OnPlaySelection( wxCommandEvent& event );
	// Deletes the selection
	void OnDeleteSelection( wxCommandEvent& event );
	// Groups the selection
	void OnGroupSelection( wxCommandEvent& event );
	// Ungroups the selection
	void OnUngroupSelection( wxCommandEvent& event );

	// Responds to a change phoneme message
	void OnPhonemeMenu( wxCommandEvent& event );
	// Responds to a quick change phone
	void OnQuickChangePhoneme( wxCommandEvent& event );
	// Responds to an insert phoneme message
	void OnInsertPhoneme( wxCommandEvent& event );
	// Responds to an append phoneme message
	void OnAppendPhoneme( wxCommandEvent& event );

	// Displays the options menu
	void OnOptions( wxCommandEvent& event );

	void OnCut( wxCommandEvent& event );
	void OnCopy( wxCommandEvent& event );
	void OnPaste( wxCommandEvent& event );
	void OnUndo( wxCommandEvent& event );
	void OnRedo( wxCommandEvent& event );

	void OnZoomSelection( wxCommandEvent& event );
	void OnReanalyzeSelection( wxCommandEvent& event );

	void OnGetFocus( wxFocusEvent& event );
	void OnLoseFocus( wxFocusEvent& event );

	void OnHelp( wxHelpEvent& event );

	// Draws a labeled rectangle
	void DrawLabeledRectangle( wxDC* pDC, const wxPoint& leftTop, 
		const wxPoint& rightBottom, const wxBrush& brush, 
		const wxString& label );

	enum eZoomDir
	{
		ZOOM_NONE = 0,
		ZOOM_IN,
		ZOOM_OUT
	};
	// Zooms the view.  zoomPoint is the "focus" point of the zoom, relative
	// to the client rect.  zoomFactor is the "zoom factor" and zoomDir is 
	// either ZOOM_IN or ZOOM_OUT.
	void Zoom( const wxPoint& zoomPoint, FxReal zoomFactor, eZoomDir zoomDir );

	enum eMouseAction
	{
		MOUSE_NONE,
		MOUSE_EDIT_PHONEMEBOUNDARY,
		MOUSE_EDIT_WORDBOUNDARY,
		MOUSE_EDIT_SWAPPHONES,
		MOUSE_EDIT_SWAPWORDS
	};

	enum eSide
	{
		START,
		END
	};

	FxBool HitTestPhonBoundaries( wxPoint point, FxSize* phonIndex = NULL, eSide* phonSide = NULL );
	FxBool HitTestPhonemes( wxPoint point, FxSize* phonIndex = NULL );
	FxBool HitTestWordBoundaries( wxPoint point, FxSize* wordIndex = NULL, eSide* wordSide = NULL);
	FxBool HitTestWords( wxPoint point, FxSize* wordIndex = NULL );

	// Initializes the phoneme information list
	void Initialize();

	void DispatchPhonemesChanged();
	void FlagNoUnload();

	FxBool			 _phonsOnTop;
	FxBool			 _ungroupedPhonsFillBar;
	FxInt32			 _ySplit;

	FxReal			 _minTime;
	FxReal			 _maxTime;
	FxReal			 _currentTime;
	
	wxPoint			 _lastPos;
	FxReal			 _mouseWheelZoomFactor;
	FxReal			 _zoomFactor;

	eMouseAction     _mouseAction;
	FxSize			 _index;
	eSide			 _side;

	FxBool			 _hasFocus;

	FxAnim*         _pAnim;
	FxPhonWordList* _pPhonList;
	FxArray<FxPhonemeClassInfo> _phonClassInfo;

	static FxArray<FxPhonInfo> _copyPhoneBuffer;
	static FxArray<FxWordInfo> _copyWordBuffer;

	FxConsoleVariable* pl_realtimeupdate;
	FxConsoleVariable* a_audiomin;
	FxConsoleVariable* a_audiomax;
};

//------------------------------------------------------------------------------
// Options dialog
//------------------------------------------------------------------------------
class FxPhonemeWidgetOptionsDlg : public wxDialog
{
	WX_DECLARE_DYNAMIC_CLASS(FxPhonemeWidgetOptionsDlg)
	DECLARE_EVENT_TABLE()

public:
	// Constructors.
	FxPhonemeWidgetOptionsDlg();
	FxPhonemeWidgetOptionsDlg( wxWindow* parent, wxWindowID id = 10006, 
		const wxString& caption = _("Phoneme Bar Options"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(400,300), 
		long style = wxDEFAULT_DIALOG_STYLE );

	// Destructor
	~FxPhonemeWidgetOptionsDlg();

	// Creation.
	bool Create( wxWindow* parent, wxWindowID id = 10006, 
		const wxString& caption = _("Phoneme Bar Options"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(400,300), 
		long style = wxDEFAULT_DIALOG_STYLE );

	// Creates the controls and sizers.
	void CreateControls();

	// Should we show tooltips?
	static FxBool ShowToolTips();

	// Updates the phoneme widget with the current options.
	void SetOptions();

protected:
	// Called whenever an action "dirties" the options
	void OnDirty( wxCommandEvent& event );
	// Called when the user clicks the apply button
	void OnApply( wxCommandEvent& event );
	// Called when the user clicks the restore defaults button
	void OnRestoreDefault( wxCommandEvent& event );

	// Called when the color list box changes
	void OnColourListChange( wxCommandEvent& event );
	// Called when the user clicks the "Change..." button to change a colour
	void OnColourChange( wxCommandEvent& event );

	// Initializes the options to the current values
	void GetOptionsFromParent();

private:
	FxPhonemeWidget* _phonemeWidget;
	wxButton*		 _applyButton;
	wxListBox*		 _colourList;
	wxStaticBitmap*  _colourPreview;

	FxInt32	  _phoneticAlphabet;
	wxColour* _phonClassColours;
	FxInt32   _numCustomColours;
	FxBool    _ungroupedPhonsFillBar;
};

//------------------------------------------------------------------------------
// Phoneme change dialog
//------------------------------------------------------------------------------
class FxChangePhonemeDialog : public wxDialog
{
	WX_DECLARE_DYNAMIC_CLASS(FxPhonemeWidgetOptionsDlg)
	DECLARE_EVENT_TABLE()
public:
	FxChangePhonemeDialog(wxWindow* parent = NULL);
	FxPhoneme selectedPhoneme;

protected:

	void OnSelChange(wxCommandEvent& event);
	void OnOK(wxCommandEvent& event);

	wxStaticText* _sampleWord;
	wxListBox* _phonemeList;
};

} // namespace Face

} // namespace OC3Ent

#endif
