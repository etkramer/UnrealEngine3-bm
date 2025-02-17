//------------------------------------------------------------------------------
// The timeline for FaceFx Studio.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxTimelineWidget_H__
#define FxTimelineWidget_H__

#include "FxPlatform.h"
#include "FxWidget.h"
#include "FxTimeManager.h"
#include "FxButton.h"
#include "FxDropButton.h"
#include "FxStudioAnimPlayer.h"

namespace OC3Ent
{

namespace Face
{

class FxTimelineTopPanel;
class FxTimelineBottomPanel;
class FxTimelineWidget;

//------------------------------------------------------------------------------
// Time window control
//------------------------------------------------------------------------------
class FxTimeWindowCtrl : public wxWindow
{
	typedef wxWindow Super;
	WX_DECLARE_DYNAMIC_CLASS( FxTimeWindowCtrl )
	DECLARE_EVENT_TABLE()

public:
	FxTimeWindowCtrl( wxWindow* parent = NULL );

	void SetTimeExtent( FxReal minTime, FxReal maxTime );
	void SetTimeWindow( FxReal minTime, FxReal maxTime );

	void GridRefresh(FxInt32 w, FxInt32 h);

protected:
	void OnPaint( wxPaintEvent& event );
	void OnMouse( wxMouseEvent& event );

	inline FxInt32 TimeToCoord( FxReal time );
	inline FxReal CoordToTime( FxInt32 coord );

	inline wxRect GetValidRect();

	inline wxRect GetLeftHandleRect();
	inline wxRect GetWindowBodyRect();
	inline wxRect GetRightHandleRect();

	FxTimelineTopPanel* _parent;

	FxReal _minTimeExtent;
	FxReal _maxTimeExtent;

	FxReal _minTimeWindow;
	FxReal _maxTimeWindow;

	FxBool _leftSelected;
	FxBool _rightSelected;

	enum MouseAction
	{
		MOUSE_NOTHING,
		MOUSE_DRAGLEFTHANDLE,
		MOUSE_DRAGRIGHTHANDLE,
		MOUSE_PANBODY
	};
	MouseAction _mouse;
};

//------------------------------------------------------------------------------
// Timeline top panel
//------------------------------------------------------------------------------
class FxTimelineTopPanel : public wxWindow
{
	friend class FxTimeWindowCtrl;
	typedef wxWindow Super;
	WX_DECLARE_DYNAMIC_CLASS( FxTimelineTopPanel )
	DECLARE_EVENT_TABLE()

public:
	FxTimelineTopPanel();
	FxTimelineTopPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, long style = 0 );

	void Create(wxWindow *parent, wxWindowID id, const wxPoint& pos  = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0 );
	
	wxSize DoGetBestSize() const;

	void SetMinMaxWindow( FxReal min, FxReal max, FxBool updateChild = FxTrue );
	void SetMinMaxExtent( FxReal min, FxReal max, FxBool updateChild = FxTrue );

	void GetMinMaxExtent( FxReal& min, FxReal& max );
	
	void GridRefresh(FxInt32 w, FxInt32 h);

protected:
	void UpdateMinMaxWindow( wxCommandEvent& event );
	void UpdateMinMaxExtent( wxCommandEvent& event );

	void UpdateWindowFromChild( FxReal min, FxReal max );

	void OnSize( wxSizeEvent& event );
	void OnLeftClick( wxMouseEvent& event );
	void OnPaint( wxPaintEvent& event );

	FxTimelineWidget* _parent;

	wxSizer* _mainSizer;
	FxBool   _collapsed;

	wxTextCtrl* _minTimeExtentText;
	wxTextCtrl* _minTimeWindowText;
	FxTimeWindowCtrl* _timeWindowCtrl;
	wxTextCtrl* _maxTimeWindowText;
	wxTextCtrl* _maxTimeExtentText;

	FxReal _minTimeExtent;
	FxReal _minTimeWindow;
	FxReal _maxTimeWindow;
	FxReal _maxTimeExtent;
};

//------------------------------------------------------------------------------
// Current time control
//------------------------------------------------------------------------------
class FxCurrentTimeCtrl : public wxWindow
{
	friend class FxTimelineBottomPanel;
	typedef wxWindow Super;
	WX_DECLARE_DYNAMIC_CLASS( FxCurrentTimeCtrl )
	DECLARE_EVENT_TABLE()

public:
	FxCurrentTimeCtrl( wxWindow* parent = NULL );

	void SetTimeExtent( FxReal min, FxReal max );
	void SetCurrentTime( FxReal current );

	FxReal GetMarkerTimeDelta();

	void GridRefresh(FxInt32 w, FxInt32 h);

protected:
	void OnPaint( wxPaintEvent& event );
	void OnMouse( wxMouseEvent& event );

	FxReal CoordToTime( FxInt32 coord );
	FxInt32 TimeToCoord( FxReal time );

	FxTimelineBottomPanel* _parent;

	FxReal _minTime;
	FxReal _maxTime;
	FxReal _currentTime;

	FxArray<FxTimeGridInfo> _gridInfos;

	FxBool _mousing;
};

//------------------------------------------------------------------------------
// Timeline bottom panel
//------------------------------------------------------------------------------
class FxTimelineBottomPanel : public wxWindow
{
	friend class FxCurrentTimeCtrl;
	typedef wxWindow Super;
	WX_DECLARE_DYNAMIC_CLASS( FxTimelineBottomPanel )
	DECLARE_EVENT_TABLE()

public:
	FxTimelineBottomPanel();
	FxTimelineBottomPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, long style = 0 );

	void Create(wxWindow *parent, wxWindowID id, const wxPoint& pos  = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0 );

	wxSize DoGetBestSize() const;

	void SetTimeExtent( FxReal minTime, FxReal maxTime );
	void SetCurrentTime( FxReal currentTime );
	FxReal GetMarkerTimeDelta();

	void SetIcons( FxBool playing );

	void Redraw();

	void GridRefresh(FxInt32 w, FxInt32 h);

protected:
	void OnSize( wxSizeEvent& event );
	void OnLeftClick( wxMouseEvent& event );
	void OnPaint( wxPaintEvent& event );

	void UpdateCurrentTime( wxCommandEvent& event );
	void OnPlayButton( wxCommandEvent& event );
	void OnPlayButtonDrop( wxCommandEvent& event);
	void OnResetButton( wxCommandEvent& event );
	void OnLoopToggle( wxCommandEvent& event );

	void OnPlaybackSpeedChange( wxCommandEvent& event);

	void StartScrubbing();
	void UpdateTimeFromChild( FxReal current );
	void StopScrubbing();

	FxTimelineWidget* _parent;

	wxSizer* _mainSizer;
	FxBool	 _collapsed;
	FxBool   _isPlaying;

	FxButton* _loopButton;
	FxButton* _resetButton;
	FxDropButton* _playButton;
	wxIcon    _playEnabled;
	wxIcon    _playDisabled;
	wxIcon    _stopEnabled;
	wxIcon    _stopDisabled;
	wxTextCtrl* _currentTimeText;
	FxCurrentTimeCtrl* _currentTimeCtrl;

	FxReal _currentTime;
};


//------------------------------------------------------------------------------
// Timeline widget
//------------------------------------------------------------------------------
class FxTimelineWidget : public wxWindow, public FxWidget, public FxTimeSubscriber
{
	typedef wxWindow Super;
	WX_DECLARE_DYNAMIC_CLASS( FxTimelineWidget )
	DECLARE_EVENT_TABLE()

public:

	// Constructor.
	FxTimelineWidget( wxWindow* parent = NULL, FxWidgetMediator* mediator = NULL );
	// Destructor.
	~FxTimelineWidget();

	// Time subscriber stuff.
	void OnNotifyTimeChange();
	FxReal GetMinimumTime();
	FxReal GetMaximumTime();
	FxInt32 GetPaintPriority();
	void VerifyPendingTimeChange( FxReal& min, FxReal& max );

	// Binds the timeline widget to a group of related controls.
	void BindToTimeSubscriberGroup( FxInt32 groupId );

	// Sets whether or not to scrub audio.
	void SetAudioScrubbingAllowed( FxBool scrubbingAllowed );

	// FxWidget message handlers.
	// Called when the application is shutting down.
	virtual void OnAppShutdown( FxWidget* sender );
	// Called when the actor pointer changes.
	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	// Called to refresh the widget.
	virtual void OnRefresh( FxWidget* sender );
	// Called when the anim changes.
	virtual void OnAnimChanged(FxWidget* sender, const FxName& animGroupName, FxAnim* anim );
	// Called when a widget changes the time.
	virtual void OnTimeChanged( FxWidget* sender, FxReal newTime );
	// Called when the grid needs to be recalculated
	virtual void OnRecalculateGrid( FxWidget* sender );

	// Called to reset the view to the animation extents.
	void OnResetView( wxCommandEvent& event );
	// Called to play the audio
	void OnPlayAudio( wxCommandEvent& event );

	// Sets the time range for the bound subscribers
	void SetSubscriberTimeRange( FxReal min, FxReal max );
	// Sets the current time
	void SetCurrentTime( FxReal currentTime, FxBool scrubbing = FxFalse, FxBool isAutoRewind = FxFalse );
	// Starts scrubbing
	void StartScrubbing();
	// Stops scrubbing
	void StopScrubbing();
	// Sets whether or not to loop
	void SetLooping( FxBool loop );

	// Plays the audio
	void PlayAudio( FxBool loop = FxFalse );
	// Stops the audio
	void StopAudio();

	// Gets the start of the time range.
	FxReal GetTimeRangeStart();
	// Gets the end of the time range.
	FxReal GetTimeRangeEnd();
	// Gets the current time.
	FxReal GetCurrentTime();

	// Called when the animation starts.
	void OnPlaybackStarted();
	// Called when the animation is done playing
	void OnPlaybackFinished();

protected:
	void DoResetView();
	void OnSize( wxSizeEvent& event );
	void OnHelp( wxHelpEvent& event );

	FxActor* _pActor;

	FxTimelineTopPanel* _timeWindowPanel;
	FxTimelineBottomPanel* _currTimePanel;

	FxAnim* _anim;

	FxInt32 _boundGroupId;
	FxReal _boundGroupMinTime;
	FxReal _boundGroupMaxTime;	

	FxReal _currentTime;
	FxReal _previousTime;
	FxBool _audioScrubbingAllowed;

	FxReal _scrubStartTime;
};

} // namespace Face

} // namespace OC3Ent

#endif

