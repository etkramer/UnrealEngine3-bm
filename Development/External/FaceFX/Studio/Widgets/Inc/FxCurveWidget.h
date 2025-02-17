//------------------------------------------------------------------------------
// A curve widget, allowing editing of curves in an FxAnim.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCurveWidget_H__
#define FxCurveWidget_H__

#include "FxPlatform.h"
#include "FxArray.h"
#include "FxName.h"
#include "FxTimeManager.h"
#include "FxAnimController.h"
#include "FxWidget.h"
#include "FxButton.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/geometry.h"
	#include "wx/image.h"
#endif

namespace OC3Ent
{

namespace Face
{

class FxAnim;
class FxCurveWidget;

class FxCurveWidgetGrid
{
public:
	FxCurveWidgetGrid( FxCurveWidget* pWidget = NULL );
	~FxCurveWidgetGrid();

	void SetWidget( FxCurveWidget* pWidget );
	void Recalc();
	void SetCurrTime(FxReal currTime);
	void DrawOn( wxDC* pDC );
	FxReal SnapTime( FxReal origTime, FxBool ignoreCurrentTime = FxFalse, FxAnimController* animController = NULL );
	FxReal SnapValue( FxReal origValue );
	FxInt32 SnapX( FxInt32 x );
	FxInt32 SnapY( FxInt32 y );

	FxInt32 group;

private:
	FxCurveWidget*  _pWidget;
	FxArray<FxTimeGridInfo> _timeGridInfos;
	FxArray<FxReal> _values;
	FxReal			  _currTime;
	FxReal			  _minTime;
	FxReal			  _maxTime;
	wxString		  _timeFormat;
	wxString		  _valueFormat;
};

class FxCurveWidget : public wxWindow, public FxTimeSubscriber, public FxWidget
{
	typedef wxWindow Super;
	WX_DECLARE_DYNAMIC_CLASS(FxCurveWidget)
	DECLARE_EVENT_TABLE()

	friend class FxCurveWidgetGrid;

public:
	// Constructor.
	FxCurveWidget( wxWindow* parent = NULL, FxWidgetMediator* mediator = NULL );
	// Destructor.
	~FxCurveWidget();

	// Overload the FxTimeSubscriber's notification method.
	void OnNotifyTimeChange();
	FxReal GetMinimumTime();
	FxReal GetMaximumTime();
	FxInt32 GetPaintPriority();

	// Gets a specific curve's visibility.
	FxBool GetCurveVisibility( const FxName& curve );
	// Sets a specific curve's visibility.
	void SetCurveVisibility( const FxName& curve, FxBool isVisible );
	// Sets all curves to not visible
	void ClearCurveVisibility();

	// Gets a specific curve's colour.
	wxColour GetCurveColour( const FxName& curve );
	// Sets a specific curve's colour.
	void SetCurveColour( const FxName& curve, const wxColour& colour );

	// Sets the animation controller.  This should be called during 
	// initialization and must be called before any calls to SetAnimPointer.
	void SetAnimControllerPointer( FxAnimController* pAnimController );
	// Sets the animation.
	void SetAnimPointer( FxAnim* pAnim );
	
	// Gets the range of the graph.
	void GetGraphRange( FxReal& minValue, FxReal& maxValue );

	// FxWidget message handlers.
	virtual void OnAppStartup( FxWidget* sender );
	virtual void OnAppShutdown( FxWidget* sender );
	virtual void OnAnimChanged( FxWidget* sender, const FxName& animGroupName, 
								FxAnim* anim );
	virtual void OnAnimCurveSelChanged( FxWidget* sender, 
								        const FxArray<FxName>& selAnimCurves );
	virtual void OnRefresh( FxWidget* sender );
	virtual void OnTimeChanged( FxWidget* sender, FxReal newTime );
	virtual void OnRecalculateGrid( FxWidget* sender );

protected:
	// Paint event notification.
	void OnPaint( wxPaintEvent& event );

	// Mouse wheel.
	void OnMouseWheel( wxMouseEvent& event );
	// Left button down.
	void OnLeftButtonDown( wxMouseEvent& event );
	// Left button up.
	void OnLeftButtonUp( wxMouseEvent& event );
	// Mouse movement.
	void OnMouseMove( wxMouseEvent& event );
	// Right button down.
	void OnRightButtonDown( wxMouseEvent& event );
	// Right button up.
	void OnRightButtonUp( wxMouseEvent& event );
	// Middle button down.
	void OnMiddleButtonDown( wxMouseEvent& event );
	// Middle button up.
	void OnMiddleButtonUp( wxMouseEvent& event );

	// Handle key presses.
	void OnKeyDown( wxKeyEvent& event );

	// Called when the window is resided.
	void OnResize( wxSizeEvent& event );

	// Resets the view to centered.
	void OnResetView( wxCommandEvent& event );
	// Selects all *visible* keys.
	void OnSelectAll( wxCommandEvent& event );
	// Toggles locking the tangents for the current key.
	void OnLockTangent( wxCommandEvent& event );
	// Unlocks the tangent handles for the current key.
	void OnUnlockTangent( wxCommandEvent& event );
	// Deletes the selected keys.
	void OnDeleteSelection( wxCommandEvent& event );
	// Edits the properties of the current edit key.
	//void OnEditKey( wxCommandEvent& event );
	// Adds a key
	void OnAddKey( wxCommandEvent& event );
	// Resets the slopes of the selected keys to zero
	void OnResetSlopes( wxCommandEvent & event );

	// Copies keys
	void OnCopy( wxCommandEvent& event );
	// Cuts keys
	void OnCut( wxCommandEvent& event );
	// Pastes keys
	void OnPaste( wxCommandEvent& event );

	void OnGetFocus( wxFocusEvent& event );
	void OnLoseFocus( wxFocusEvent& event );

	// Fits curves in time axis.
	void OnFitTime( wxCommandEvent& event );
	// Fits curves in value axis.
	void OnFitValue( wxCommandEvent& event );

	// Shows all active curves.
	void OnViewAllActiveCurves( wxCommandEvent& event );

	void OnNextKey( wxCommandEvent& event );
	void OnPrevKey( wxCommandEvent& event );

	// Updates the keys.
	void OnUpdateKeys( wxCommandEvent& event );

	void OnHelp( wxHelpEvent& event );

private:
	void CalcVerticalSpan( void );
	void DrawGrid( wxDC* pDC );
	void DrawKeys( wxDC* pDC, FxSize curveIndex );
	void DrawCurves( wxDC* pDC );

	FxReal  CoordToValue( FxInt32 coord, const wxRect& rect );
	FxInt32 ValueToCoord( FxReal value, const wxRect& rect );

	enum eZoomDir
	{
		ZOOM_NONE = 0,
		ZOOM_IN,
		ZOOM_OUT
	};
	// Zooms the view.  zoomPoint is the "focus" point of the zoom, relative
	// to the client rect.  zoomFactor is the "zoom factor" and zoomDir is 
	// either ZOOM_IN or ZOOM_OUT.
	void ZoomTime( const wxPoint& zoomPoint, FxReal zoomFactor, eZoomDir zoomDir );
	void ZoomValue( const wxPoint& zoomPoint, FxReal zoomFactor, eZoomDir zoomDir );

	// Returns the toolbar rect
	wxRect GetToolbarRect();
	// Returns the client rect
	wxRect GetWorkRect();
	// Updates the time/value displays
	void UpdateToolbarDisplays(FxBool overrideFocusCheck = FxFalse);
	// Enables/disables the toolbar buttons.
	void SetToolbarState();

	enum eMouseOperation
	{
		MOUSE_NONE = 0,
		MOUSE_DRAGSELECTION,
		MOUSE_EDITTANGENTS,
		MOUSE_MOVESELECTION,
		MOUSE_ADDKEY,
		MOUSE_MOVETIME,
		MOUSE_ONEDMOVESELECTION
	};

	enum eWhichTangent
	{
		TANGENT_NONE = 0,
		TANGENT_INCOMING = 1,
		TANGENT_OUTGOING = 2
	};

	// The zoom factor for non mouse wheel zooming.
	FxReal _zoomFactor;
	// The zoom factor for mouse wheel zooming.
	FxReal _mouseWheelZoomFactor;

	// The current mouse operation.
	eMouseOperation _mouseOp;
	// The current curve we're editing.
	FxSize _editCurve;
	// The current key we're editing.
	FxSize _editKey;
	// Which tangent we're editing.
	eWhichTangent _editTangent;
	
	// A pointer to the animation controller.
	FxAnimController* _pAnimController;
	// A pointer to the animation we're working on.
	FxAnim* _pAnim;
	// The minimum time shown in the view.
	FxReal _minTime;
	// The maximum time shown in the view.
	FxReal _maxTime;
	// The minimum value shown in the view.
	FxReal _minValue;
	// The maximum value shown in the view.
	FxReal _maxValue;

	// The current time in the application (for the hint line)
	FxReal _currentTime;

	// Manages the grid - drawing, snapping, etc.
	FxCurveWidgetGrid _grid;

	// The last position of the mouse.
	wxPoint _lastPos;
	// The rectangle of the selection (in (time,value),(time,value) ).
	wxRect2DDouble _selection;

	// One-time initialization of the control
	void Initialize();
	// The background of the control
	wxBitmap _background;

	// Stuff for the pop-up key editing.
	FxReal     _keyeditTime;
	FxReal     _keyeditValue;
	FxReal     _keyeditIncDeriv;
	FxReal     _keyeditOutDeriv;

	// The copy/paste buffer
	static FxArray< FxArray<FxAnimKey> > _copyBuffer;
	FxReal _copyMinTime;
	// Whether the window has focus
	FxBool _hasFocus;

	// True if we're doing a 1-d pan
	FxBool _oneDeePan;
	enum SingleDirection
	{
		DIR_UNCERTAIN,
		DIR_TIME,
		DIR_VALUE
	};
	SingleDirection _oneDeePanDirection;
	SingleDirection _oneDeeTransformDirection;

	void CachePivotKeyStartPos();
	// The starting time/value of the pivot key.
	wxPoint2DDouble _startPos;
	// The starting time/value of the mouse.
	wxPoint2DDouble _startMousePos;
	// The starting slopes for tangent editing
	FxReal _cachedSlopeIn;
	FxReal _cachedSlopeOut;

	FxBool _hasCurveSelection;

	// Buttons
	FxArray<FxButton*> _toolbarButtons;
	FxButton* _buttonSnapValue;
	FxButton* _buttonSnapTime;
	FxButton* _buttonFitValue;
	FxButton* _buttonFitTime;
	FxButton* _buttonViewAllActive;
	FxButton* _buttonPrevKey;
	FxButton* _buttonNextKey;
	// Text controls
	wxTextCtrl* _textTime;
	wxTextCtrl* _textValue;
	wxTextCtrl* _textIncDeriv;
	wxTextCtrl* _textOutDeriv;
	// Cached values from onmousemove
	FxReal _lastTime;
	FxReal _lastValue;

	// Cache the current time.
	FxReal _cachedCurrentTime;
};

} // namespace Face

} // namespace OC3Ent

#endif
