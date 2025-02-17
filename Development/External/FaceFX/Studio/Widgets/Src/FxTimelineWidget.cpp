//------------------------------------------------------------------------------
// The timeline for FaceFx Studio.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxStudioApp.h"
#include "FxTimelineWidget.h"
#include "FxTimeManager.h"
#include "FxMath.h"
#include "FxAnimUserData.h"
#include "FxButton.h"
#include "FxVM.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/dcbuffer.h"
#endif

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// Time window control
//------------------------------------------------------------------------------
WX_IMPLEMENT_DYNAMIC_CLASS( FxTimeWindowCtrl, wxWindow )

BEGIN_EVENT_TABLE( FxTimeWindowCtrl, wxWindow )
	EVT_PAINT( FxTimeWindowCtrl::OnPaint )
	EVT_MOUSE_EVENTS( FxTimeWindowCtrl::OnMouse )
END_EVENT_TABLE()

FxTimeWindowCtrl::FxTimeWindowCtrl( wxWindow* parent )
	: wxWindow(parent, wxID_DEFAULT, wxDefaultPosition, wxSize(50, 22), wxBORDER_SUNKEN)
	, _parent(NULL)
	, _minTimeExtent(FxInvalidValue)
	, _maxTimeExtent(FxInvalidValue)
	, _minTimeWindow(FxInvalidValue)
	, _maxTimeWindow(FxInvalidValue)
	, _leftSelected( FxFalse )
	, _rightSelected( FxFalse )
	, _mouse(MOUSE_NOTHING)
{
	_parent = static_cast<FxTimelineTopPanel*>(parent);
}

void FxTimeWindowCtrl::SetTimeExtent( FxReal minTime, FxReal maxTime )
{
	_minTimeExtent = minTime;
	_maxTimeExtent = maxTime;

	Refresh(FALSE);
}

void FxTimeWindowCtrl::SetTimeWindow( FxReal minTime, FxReal maxTime )
{
	_minTimeWindow = minTime;
	_maxTimeWindow = maxTime;

	Refresh(FALSE);
}

void FxTimeWindowCtrl::GridRefresh(FxInt32 FxUnused(w), FxInt32 FxUnused(h))
{
	// Here just in case for now.
}

void FxTimeWindowCtrl::OnPaint( wxPaintEvent& FxUnused(event) )
{
	wxRect clientRect = GetClientRect();

	wxBufferedPaintDC dc(this);
	dc.SetBackground( wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE), wxSOLID) );
	dc.Clear();
	if( _minTimeWindow != FxInvalidValue && _maxTimeWindow != FxInvalidValue )
	{
		// Get some colours we'll need to draw with.
		wxColour faceColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
		wxColour highlightColour =wxSystemSettings::GetColour(wxSYS_COLOUR_3DHIGHLIGHT);
		wxColour shadowColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
		wxColour selectedColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT);
		dc.SetBrush( wxBrush(faceColour, wxSOLID) );
		dc.SetPen( *wxTRANSPARENT_PEN );

		wxRect timeWindowRect = GetWindowBodyRect();
		wxRect timeWindowLeftHandle = GetLeftHandleRect();
		wxRect timeWindowRightHandle = GetRightHandleRect();
		
		// Draw the time window slider as 3d object.
		dc.DrawRectangle( timeWindowRect );

		if( _leftSelected )
		{
			dc.SetBrush( wxBrush(selectedColour, wxSOLID) );
		}
		else
		{
			dc.SetBrush( wxBrush(faceColour, wxSOLID) );
		}
		dc.DrawRectangle( timeWindowLeftHandle );

		if( _rightSelected )
		{
			dc.SetBrush( wxBrush(selectedColour, wxSOLID) );
		}
		else
		{
			dc.SetBrush( wxBrush(faceColour, wxSOLID) );
		}
		dc.DrawRectangle( timeWindowRightHandle );

		// Partition off the handles
		dc.SetPen( wxPen(shadowColour, 1, wxSOLID) );
		dc.DrawLine( timeWindowLeftHandle.GetRight(), timeWindowLeftHandle.GetTop(),
					 timeWindowLeftHandle.GetRight(), timeWindowLeftHandle.GetBottom() );
		dc.DrawLine( timeWindowRightHandle.GetLeft(), timeWindowRightHandle.GetTop(),
					 timeWindowRightHandle.GetLeft(), timeWindowRightHandle.GetBottom() );

		// Shade the slider
		dc.SetPen( wxPen(highlightColour, 1, wxSOLID) );
		dc.DrawLine( timeWindowLeftHandle.GetLeft(), timeWindowLeftHandle.GetBottom(),
					 timeWindowLeftHandle.GetLeft(), timeWindowLeftHandle.GetTop() );
		dc.DrawLine( timeWindowLeftHandle.GetLeft(), timeWindowLeftHandle.GetTop(),
					 timeWindowRightHandle.GetRight(), timeWindowRightHandle.GetTop() );
		dc.SetPen( wxPen(shadowColour, 1, wxSOLID) );
		dc.DrawLine( timeWindowRightHandle.GetRight(), timeWindowRightHandle.GetTop(),
					 timeWindowRightHandle.GetRight(), timeWindowRightHandle.GetBottom() );
		dc.DrawLine( timeWindowRightHandle.GetRight(), timeWindowRightHandle.GetBottom(),
					 timeWindowLeftHandle.GetLeft(), timeWindowLeftHandle.GetBottom() );

		// @todo draw the icon on the slider handles.

		// Label the slider if we're in a dragging mode
		if( _mouse != MOUSE_NOTHING )
		{
			wxString leftLabel  = FxFormatTime(_minTimeWindow, FxStudioOptions::GetTimelineGridFormat());
			wxPoint leftLabelPt = wxPoint( timeWindowRect.GetLeft() + 1, timeWindowRect.GetTop() + 1 );

			FxInt32 width = 0, height = 0;

			dc.SetFont( wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT) );
			dc.GetTextExtent( leftLabel, &width, &height );
			if( leftLabelPt.x + width <= timeWindowRect.GetRight() )
			{
				dc.DrawText( leftLabel, leftLabelPt );

				wxString rightLabel = FxFormatTime(_maxTimeWindow, FxStudioOptions::GetTimelineGridFormat());
				dc.GetTextExtent( rightLabel, &width, &height );
				wxPoint rightLabelPt= wxPoint( timeWindowRect.GetRight() - width -1 , timeWindowRect.GetTop() + 1);
				if( rightLabelPt.x >= leftLabelPt.x + width )
				{
					dc.DrawText( rightLabel, rightLabelPt );
				}
			}
		}
	}
}

void FxTimeWindowCtrl::OnMouse( wxMouseEvent& event )
{
	static wxPoint lastPos(0,0);
	wxRect leftHandle  = GetLeftHandleRect();
	wxRect body        = GetWindowBodyRect();
	wxRect rightHandle = GetRightHandleRect();
	if( event.LeftDown() )
	{
		SetFocus();
		if( !HasCapture() )
		{
			CaptureMouse();
		}

		if( body.Contains(event.GetPosition()) )
		{
			_mouse = MOUSE_PANBODY;
		}
		else if( leftHandle.Contains(event.GetPosition()) )
		{
			_mouse = MOUSE_DRAGLEFTHANDLE;
		}
		else if( rightHandle.Contains(event.GetPosition()) )
		{
			_mouse = MOUSE_DRAGRIGHTHANDLE;
		}
		if( _mouse != MOUSE_NOTHING )
		{
			Refresh(FALSE);
			Update();
		}
	}
	else if( event.LeftUp() )
	{
		if( HasCapture() )
		{
			ReleaseMouse();
		}
		_mouse = MOUSE_NOTHING;
		Refresh( FALSE );
	}
	else if( event.Dragging() )
	{
		FxReal deltaTime = CoordToTime(event.GetPosition().x) - CoordToTime(lastPos.x);

		if( _mouse == MOUSE_PANBODY )
		{
			if( _maxTimeWindow + deltaTime > _maxTimeExtent )
			{
				FxReal duration = _maxTimeWindow - _minTimeWindow;
				_maxTimeWindow = _maxTimeExtent;
				_minTimeWindow = _maxTimeWindow - duration;
			}
			else if( _minTimeWindow + deltaTime < _minTimeExtent )
			{
				FxReal duration = _maxTimeWindow - _minTimeWindow;
				_minTimeWindow = _minTimeExtent;
				_maxTimeWindow = _minTimeWindow + duration;
			}
			else
			{
				_minTimeWindow = _minTimeWindow + deltaTime;
				_maxTimeWindow = _maxTimeWindow + deltaTime;
			}
		}
		else if( _mouse == MOUSE_DRAGLEFTHANDLE )
		{
			if( _minTimeWindow + deltaTime <= _maxTimeWindow )
			{
				_minTimeWindow = FxMax( _minTimeExtent, _minTimeWindow + deltaTime );
			}
		}
		else if( _mouse == MOUSE_DRAGRIGHTHANDLE )
		{
			if( _maxTimeWindow + deltaTime >= _minTimeWindow )
			{
				_maxTimeWindow = FxMin( _maxTimeExtent, _maxTimeWindow + deltaTime );
			}
		}

		if( _mouse != MOUSE_NOTHING )
		{
			Refresh(FALSE);
			Update();
			_parent->UpdateWindowFromChild( _minTimeWindow, _maxTimeWindow );
		}
	}
	else if( event.Moving() )
	{
		_leftSelected = FxFalse;
		_rightSelected = FxFalse;
		if( leftHandle.Contains(event.GetPosition()) )
		{
			_leftSelected = FxTrue;
		}
		else if( rightHandle.Contains(event.GetPosition()) )
		{
			_rightSelected = FxTrue;
		}
		Refresh(FALSE);
	}
	else if( event.Leaving() )
	{
		_leftSelected = FxFalse;
		_rightSelected = FxFalse;
		Refresh(FALSE);
	}
	lastPos = event.GetPosition();
}

FxInt32 FxTimeWindowCtrl::TimeToCoord( FxReal time )
{
	wxRect validRect = GetValidRect();
	return ( ( time - _minTimeExtent ) / ( _maxTimeExtent - _minTimeExtent) ) * 
		validRect.GetWidth() + validRect.GetLeft();
}

FxReal FxTimeWindowCtrl::CoordToTime( FxInt32 coord )
{
	wxRect validRect = GetValidRect();
	return ( static_cast<FxReal>( coord - validRect.GetLeft() ) / 
		static_cast<FxReal>( validRect.GetWidth() ) ) * 
		( _maxTimeExtent - _minTimeExtent ) + _minTimeExtent;
}

wxRect FxTimeWindowCtrl::GetValidRect()
{
	wxRect validRect = GetClientRect();
	validRect.SetWidth( validRect.GetWidth() - 30 );
	validRect.SetLeft( validRect.GetLeft() + 15 );
	return validRect;
}

wxRect FxTimeWindowCtrl::GetLeftHandleRect()
{
	wxRect bodyRect = GetWindowBodyRect();
	return wxRect( wxPoint(bodyRect.GetLeft()-14, 0), wxSize(14,bodyRect.GetHeight()) );
}

wxRect FxTimeWindowCtrl::GetWindowBodyRect()
{
	wxRect clientRect = GetClientRect();
	wxPoint topLeft(TimeToCoord(_minTimeWindow), clientRect.GetTop());
	wxPoint bottomRight(TimeToCoord(_maxTimeWindow), clientRect.GetBottom()-1);
	return wxRect(topLeft, bottomRight);
}

wxRect FxTimeWindowCtrl::GetRightHandleRect()
{
	wxRect bodyRect = GetWindowBodyRect();
	return wxRect( wxPoint(bodyRect.GetRight(), 0), wxSize(14,bodyRect.GetHeight()) );
}

//------------------------------------------------------------------------------
// Timeline top panel
//------------------------------------------------------------------------------

WX_IMPLEMENT_DYNAMIC_CLASS( FxTimelineTopPanel, wxWindow )
BEGIN_EVENT_TABLE( FxTimelineTopPanel, wxWindow )
	EVT_SIZE( FxTimelineTopPanel::OnSize )
	EVT_LEFT_UP( FxTimelineTopPanel::OnLeftClick )
	EVT_PAINT( FxTimelineTopPanel::OnPaint )

	EVT_TEXT_ENTER( ControlID_TimelineWidgetTopPanelMinTimeWindow, FxTimelineTopPanel::UpdateMinMaxWindow )
	EVT_TEXT_ENTER( ControlID_TimelineWidgetTopPanelMaxTimeWindow, FxTimelineTopPanel::UpdateMinMaxWindow )
	EVT_TEXT_ENTER( ControlID_TimelineWidgetTopPanelMinTimeExtent, FxTimelineTopPanel::UpdateMinMaxExtent )
	EVT_TEXT_ENTER( ControlID_TimelineWidgetTopPanelMaxTimeExtent, FxTimelineTopPanel::UpdateMinMaxExtent )
END_EVENT_TABLE()

FxTimelineTopPanel::FxTimelineTopPanel()
	: _parent(0)
	, _collapsed(FxFalse)
	, _mainSizer(0)
	, _minTimeExtentText(0)
	, _minTimeWindowText(0)
	, _timeWindowCtrl(0)
	, _maxTimeWindowText(0)
	, _maxTimeExtentText(0)
{
}

FxTimelineTopPanel::FxTimelineTopPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos /* = wxDefaultPosition */, const wxSize& size /* = wxDefaultSize */, long style /* = 0  */ )
	: _parent(0)
	, _collapsed(FxFalse)
	, _mainSizer(0)
	, _minTimeExtentText(0)
	, _minTimeWindowText(0)
	, _timeWindowCtrl(0)
	, _maxTimeWindowText(0)
	, _maxTimeExtentText(0)
	, _minTimeExtent(FxInvalidValue)
	, _minTimeWindow(FxInvalidValue)
	, _maxTimeWindow(FxInvalidValue)
	, _maxTimeExtent(FxInvalidValue)
{
	Create( parent, id, pos, size, style );
}

void FxTimelineTopPanel::Create(wxWindow *parent, wxWindowID id, const wxPoint& pos /* = wxDefaultPosition */, const wxSize& size /* = wxDefaultSize */, long style /* = 0  */ )
{
	wxWindow::Create( parent, id, pos, size, style );

	_parent = static_cast<FxTimelineWidget*>(parent);

	_mainSizer  = new wxBoxSizer(wxHORIZONTAL);
	SetSizer( _mainSizer );
	SetAutoLayout( FALSE );

	_mainSizer->AddSpacer(12);
	// Create the left side text control
	_minTimeExtentText = new wxTextCtrl( this, ControlID_TimelineWidgetTopPanelMinTimeExtent, wxEmptyString, wxDefaultPosition, wxSize( 40, 20 ) );
	_minTimeExtentText->SetToolTip( _("Minimum animation time") );
	_mainSizer->Add( _minTimeExtentText, 0, wxLEFT, 2 );
	_minTimeWindowText = new wxTextCtrl( this, ControlID_TimelineWidgetTopPanelMinTimeWindow, wxEmptyString, wxDefaultPosition, wxSize( 40, 20 ) );
	_minTimeWindowText->SetToolTip( _("Minimum time range") );
	_mainSizer->Add( _minTimeWindowText, 0, wxLEFT|wxRIGHT, 2 );

	// Create the centre panel
	_timeWindowCtrl = new FxTimeWindowCtrl( this );
	_mainSizer->Add( _timeWindowCtrl, 1, 0, 0 );

	// Create the right side text control
	_maxTimeWindowText = new wxTextCtrl( this, ControlID_TimelineWidgetTopPanelMaxTimeWindow, wxEmptyString, wxDefaultPosition, wxSize( 40, 20 ) );
	_maxTimeWindowText->SetToolTip( _("Maximum time range") );
	_mainSizer->Add( _maxTimeWindowText, 0, wxLEFT|wxRIGHT, 2 );
	_maxTimeExtentText = new wxTextCtrl( this, ControlID_TimelineWidgetTopPanelMaxTimeExtent, wxEmptyString, wxDefaultPosition, wxSize( 40, 20 ) );
	_maxTimeExtentText->SetToolTip( _("Maximum animation time") );
	_mainSizer->Add( _maxTimeExtentText, 0, wxRIGHT, 2 );

	SetMinMaxWindow( FxInvalidValue, FxInvalidValue );
	SetMinMaxExtent( FxInvalidValue, FxInvalidValue );

	Layout();
}

void FxTimelineTopPanel::SetMinMaxWindow( FxReal min, FxReal max, FxBool updateChild )
{
	_minTimeWindow = min;
	_maxTimeWindow = max;

	if( _timeWindowCtrl && updateChild )
	{
		_timeWindowCtrl->SetTimeWindow(_minTimeWindow, _maxTimeWindow );
	}

	FxReal minExtent = FxMin( _minTimeExtent, _minTimeWindow );
	FxReal maxExtent = FxMax( _maxTimeExtent, _maxTimeWindow );
	SetMinMaxExtent( minExtent, maxExtent );

	if( _minTimeWindowText && _maxTimeWindowText )
	{
		if( _minTimeWindow != FxInvalidValue && _maxTimeWindow != FxInvalidValue )
		{
			_minTimeWindowText->Enable( TRUE );
			_maxTimeWindowText->Enable( TRUE );
			_minTimeWindowText->SetValue( FxFormatTime(_minTimeWindow, FxStudioOptions::GetTimelineGridFormat()) );
			_maxTimeWindowText->SetValue( FxFormatTime(_maxTimeWindow, FxStudioOptions::GetTimelineGridFormat()) );
		}
		else
		{
			_minTimeWindowText->Enable( FALSE );
			_maxTimeWindowText->Enable( FALSE );
			_minTimeWindowText->SetValue( wxEmptyString );
			_maxTimeWindowText->SetValue( wxEmptyString );
		}
	}
}

void FxTimelineTopPanel::SetMinMaxExtent( FxReal min, FxReal max, FxBool updateChild )
{
	_minTimeExtent = min;
	_maxTimeExtent = max;
	if( _timeWindowCtrl && updateChild )
	{
		_timeWindowCtrl->SetTimeExtent(_minTimeExtent, _maxTimeExtent );
	}

	if( _minTimeExtentText && _maxTimeExtentText )
	{
		if( _minTimeExtent != FxInvalidValue && _maxTimeExtent != FxInvalidValue )
		{
			_minTimeExtentText->Enable( TRUE );
			_maxTimeExtentText->Enable( TRUE );
			_minTimeExtentText->SetValue( FxFormatTime(_minTimeExtent, FxStudioOptions::GetTimelineGridFormat()) );
			_maxTimeExtentText->SetValue( FxFormatTime(_maxTimeExtent, FxStudioOptions::GetTimelineGridFormat()) );
		}
		else
		{
			_minTimeExtentText->Enable( FALSE );
			_maxTimeExtentText->Enable( FALSE );
			_minTimeExtentText->SetValue( wxEmptyString );
			_maxTimeExtentText->SetValue( wxEmptyString );
		}
	}
}

void FxTimelineTopPanel::GetMinMaxExtent( FxReal& min, FxReal& max )
{
	min = _minTimeExtent;
	max = _maxTimeExtent;
}

void FxTimelineTopPanel::GridRefresh(FxInt32 w, FxInt32 h)
{
	SetMinMaxWindow(_minTimeWindow, _maxTimeWindow, FxFalse);
	SetMinMaxExtent(_minTimeExtent, _maxTimeExtent, FxFalse);
	_timeWindowCtrl->GridRefresh(w, h);
}

void FxTimelineTopPanel::UpdateMinMaxWindow( wxCommandEvent& FxUnused(event) )
{
	FxReal minTimeWindow = FxUnformatTime(_minTimeWindowText->GetValue(), FxStudioOptions::GetTimelineGridFormat());
	FxReal maxTimeWindow = FxUnformatTime(_maxTimeWindowText->GetValue(), FxStudioOptions::GetTimelineGridFormat());
	if( minTimeWindow > maxTimeWindow )
	{
		maxTimeWindow = minTimeWindow + 1.0f;
	}
	FxBool updateExtents = FxFalse;
	if( minTimeWindow < _minTimeExtent )
	{
		updateExtents = FxTrue;
		_minTimeExtent = minTimeWindow;
	}
	else if( maxTimeWindow > _maxTimeExtent )
	{
		updateExtents = FxTrue;
		_maxTimeExtent = maxTimeWindow;
	}
	if( updateExtents )
	{
		SetMinMaxExtent( _minTimeExtent, _maxTimeExtent );
	}
	_minTimeWindow = minTimeWindow;
	_maxTimeWindow = maxTimeWindow;
	_timeWindowCtrl->SetTimeWindow( _minTimeWindow, _maxTimeWindow );
	_parent->SetSubscriberTimeRange( _minTimeWindow, _maxTimeWindow );
}

void FxTimelineTopPanel::UpdateMinMaxExtent( wxCommandEvent& FxUnused(event) )
{
	FxReal minTimeExtent = FxUnformatTime(_minTimeExtentText->GetValue(), FxStudioOptions::GetTimelineGridFormat());
	FxReal maxTimeExtent = FxUnformatTime(_maxTimeExtentText->GetValue(), FxStudioOptions::GetTimelineGridFormat());
	if( minTimeExtent > maxTimeExtent )
	{
		maxTimeExtent = minTimeExtent + 1.0f;
	}

	SetMinMaxExtent( minTimeExtent, maxTimeExtent );
	if( minTimeExtent > _minTimeWindow )
	{
		_minTimeWindow = minTimeExtent;
		if( _minTimeWindow > _maxTimeWindow )
		{
			_maxTimeWindow = _minTimeWindow + 1.0f;

		}
		_timeWindowCtrl->SetTimeWindow( _minTimeWindow, _maxTimeWindow );
		_parent->SetSubscriberTimeRange( _minTimeWindow, _maxTimeWindow );
	}
	if( maxTimeExtent < _maxTimeWindow )
	{
		_maxTimeWindow = maxTimeExtent;
		if( _maxTimeWindow < _minTimeWindow )
		{
			_minTimeWindow = _maxTimeWindow - 1.0f;
		}
		_timeWindowCtrl->SetTimeWindow( _minTimeWindow, _maxTimeWindow );
		_parent->SetSubscriberTimeRange( _minTimeWindow, _maxTimeWindow );
	}
}

void FxTimelineTopPanel::UpdateWindowFromChild( FxReal min, FxReal max )
{
	SetMinMaxWindow( min, max, FxFalse );
	_parent->SetSubscriberTimeRange( _minTimeWindow, _maxTimeWindow );
}

wxSize FxTimelineTopPanel::DoGetBestSize() const
{
	if( !_collapsed )
	{
		return wxSize( 0, 22 );
	}
	else
	{
		return wxSize( 0, 5 );
	}
}

void FxTimelineTopPanel::OnSize( wxSizeEvent& FxUnused(event) )
{
	Layout();
}

void FxTimelineTopPanel::OnLeftClick( wxMouseEvent& event )
{
	SetFocus();
	if( (_collapsed && event.GetX() <= 23) ||
		(!_collapsed && event.GetX() <= 10 ) )
	{
		_collapsed = !_collapsed;

		if( _collapsed )
		{
			_mainSizer->Show( 1, false );
			_mainSizer->Show( 2, false );
			_mainSizer->Show( 3, false );
			_mainSizer->Show( 4, false );
			_mainSizer->Show( 5, false );
		}
		else
		{
			_mainSizer->Show( 1, true );
			_mainSizer->Show( 2, true );
			_mainSizer->Show( 3, true );
			_mainSizer->Show( 4, true );
			_mainSizer->Show( 5, true );
		}

		if( GetGrandParent() )
		{
			GetGrandParent()->Layout();
		}
		Layout();
	}
}

void FxTimelineTopPanel::OnPaint( wxPaintEvent& event )
{
	wxPaintDC dc(this);
	wxColour buttonColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
	wxColour buttonHighlight = wxSystemSettings::GetColour(wxSYS_COLOUR_3DHIGHLIGHT);
	wxColour buttonShadow = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
	if( _collapsed )
	{
		dc.SetBrush(wxBrush(buttonColour,wxSOLID));
		dc.SetPen(wxPen(buttonShadow,1,wxSOLID));
		dc.DrawRectangle(1, 0, 23, 5);
		dc.SetPen(wxPen(buttonHighlight,1,wxSOLID));
		dc.DrawLine(1,5,1,0);
		dc.DrawLine(1,0,23,0);

		wxPoint lines[] = {wxPoint(0,5), wxPoint(2,0), wxPoint(4,5)};
		dc.SetPen(wxPen(wxColour(75,75,75),1,wxSOLID));
		dc.DrawLines( WXSIZEOF(lines), lines, 3 );
		dc.DrawLines( WXSIZEOF(lines), lines, 9 );
		dc.DrawLines( WXSIZEOF(lines), lines, 15 );
	}
	else
	{
		dc.SetBrush(wxBrush(buttonColour,wxSOLID));
		dc.SetPen(wxPen(buttonShadow,1,wxSOLID));
		dc.DrawRectangle(1, 0, 11, 20);
		dc.SetPen(wxPen(buttonHighlight,1,wxSOLID));
		dc.DrawLine(1,20,1,0);
		dc.DrawLine(1,0,11,0);

		wxPoint lines[] = {wxPoint(0,0), wxPoint(2,5), wxPoint(4,0)};
		dc.SetPen(wxPen(wxColour(75,75,75),1,wxSOLID));
		dc.DrawLines( WXSIZEOF(lines), lines, 4, 1 );
		dc.DrawLines( WXSIZEOF(lines), lines, 4, 7 );
		dc.DrawLines( WXSIZEOF(lines), lines, 4, 13 );

		event.Skip();
	}
}

//------------------------------------------------------------------------------
// Current time control
//------------------------------------------------------------------------------
WX_IMPLEMENT_DYNAMIC_CLASS( FxCurrentTimeCtrl, wxWindow )
BEGIN_EVENT_TABLE( FxCurrentTimeCtrl, wxWindow )
	EVT_PAINT( FxCurrentTimeCtrl::OnPaint )
	EVT_MOUSE_EVENTS( FxCurrentTimeCtrl::OnMouse )
END_EVENT_TABLE()

FxCurrentTimeCtrl::FxCurrentTimeCtrl( wxWindow* parent )
	: wxWindow(parent, wxID_DEFAULT, wxDefaultPosition, wxSize(50, 27), wxBORDER_SUNKEN)
	, _parent(NULL)
	, _minTime(FxInvalidValue)
	, _maxTime(FxInvalidValue)
	, _currentTime(FxInvalidValue)
	, _mousing(FxFalse)
{
	_parent = static_cast<FxTimelineBottomPanel*>(parent);
}

void FxCurrentTimeCtrl::SetTimeExtent( FxReal min, FxReal max )
{
	_minTime = min;
	_maxTime = max;

	FxTimeManager::GetGridInfos(_gridInfos, GetClientRect(), FxDefaultGroup, FxStudioOptions::GetTimelineGridFormat(), _minTime, _maxTime);
	
	Refresh(FALSE);
}

void FxCurrentTimeCtrl::SetCurrentTime( FxReal current )
{
	_currentTime = current;
	Refresh(FALSE);
}

FxReal FxCurrentTimeCtrl::GetMarkerTimeDelta()
{
	FxInt32 minCoord = GetClientRect().GetLeft();
	return CoordToTime( minCoord + 1 ) - CoordToTime( minCoord );
}

void FxCurrentTimeCtrl::GridRefresh(FxInt32 w, FxInt32 h)
{
	wxRect newClientRect(0, 0, w, h);
	FxTimeManager::GetGridInfos(_gridInfos, newClientRect, FxDefaultGroup, FxStudioOptions::GetTimelineGridFormat(), _minTime, _maxTime);
	
	Refresh(FALSE);
}

void FxCurrentTimeCtrl::OnPaint( wxPaintEvent& FxUnused(event) )
{
	wxRect clientRect = GetClientRect();

	wxBufferedPaintDC dc(this);
	if( _minTime == FxInvalidValue || _maxTime == FxInvalidValue )
	{
		dc.SetBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
	}
	else
	{
		dc.SetBackground( *wxWHITE_BRUSH );
	}
	dc.Clear();

	if( _minTime != FxInvalidValue && _maxTime != FxInvalidValue && _currentTime != FxInvalidValue )
	{
		dc.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
		dc.SetPen(*wxBLACK_PEN);
		dc.SetTextForeground(*wxBLACK);

		FxSize numGridInfos = _gridInfos.Length();
		for( FxSize i = 0; i < numGridInfos; ++i )
		{
			if( _minTime <= _gridInfos[i].time && _gridInfos[i].time <= _maxTime )
			{
				FxInt32 x = TimeToCoord( _gridInfos[i].time );
				FxInt32 y = clientRect.GetHeight()/2;
				dc.DrawLine( x, clientRect.GetBottom(), x, clientRect.GetBottom() - y );
				dc.DrawText( _gridInfos[i].label, x, clientRect.GetTop());
			}
		}

		// Draw the current time marker, and label it if we're actively changing it.
		if( _minTime <= _currentTime && _currentTime <= _maxTime )
		{
			FxInt32 currentTimeCoord = TimeToCoord(_currentTime);
			FxInt32 xExtent = 0, yExtent = 0;
			dc.SetPen( wxPen(*wxBLACK, 2, wxSOLID) );
			dc.DrawLine( currentTimeCoord, clientRect.GetTop(), currentTimeCoord, clientRect.GetBottom() );
			if( _mousing )
			{
				wxString currTimeLabel = FxFormatTime(_currentTime, FxStudioOptions::GetTimelineGridFormat());
				dc.GetTextExtent( currTimeLabel, &xExtent, &yExtent );
				if( currentTimeCoord + xExtent + 1 < clientRect.GetRight() )
				{
					dc.DrawText( currTimeLabel, currentTimeCoord + 1, clientRect.GetBottom() - yExtent + 2 );
				}
				else
				{
					dc.DrawText( currTimeLabel, currentTimeCoord - xExtent - 2, clientRect.GetBottom() - yExtent + 2 );
				}
			}
		}
	}
}

void FxCurrentTimeCtrl::OnMouse( wxMouseEvent& event )
{
	if( event.LeftDown() )
	{
		SetFocus();
		if( !HasCapture() )
		{
			CaptureMouse();
		}
		_parent->StartScrubbing();
		_mousing = FxTrue;
		Refresh(FALSE);
		Update();
	}
	if( _mousing && (event.LeftDown() || event.LeftUp() || (event.Dragging() && event.LeftIsDown())) )
	{
		_currentTime = CoordToTime( event.GetPosition().x );
		_currentTime = FxClamp( _minTime, _currentTime, _maxTime );
		Refresh(FALSE);
		Update();
		_parent->UpdateTimeFromChild( _currentTime );
	}

	if( _mousing && event.LeftUp() )
	{
		_mousing = FxFalse;
		_parent->StopScrubbing();
		if( HasCapture() )
		{
			ReleaseMouse();
		}
	}
}

FxInt32 FxCurrentTimeCtrl::TimeToCoord( FxReal time )
{
	wxRect rect = GetClientRect();
	return ( ( time - _minTime ) / ( _maxTime - _minTime) ) * 
		rect.GetWidth() + rect.GetLeft();
}

FxReal FxCurrentTimeCtrl::CoordToTime( FxInt32 coord )
{
	wxRect rect = GetClientRect();
	return ( static_cast<FxReal>( coord - rect.GetLeft() ) / 
		static_cast<FxReal>( rect.GetWidth() ) ) * 
		( _maxTime - _minTime ) + _minTime;
}

//------------------------------------------------------------------------------
// Timeline bottom panel
//------------------------------------------------------------------------------

WX_IMPLEMENT_DYNAMIC_CLASS( FxTimelineBottomPanel, wxWindow )
BEGIN_EVENT_TABLE( FxTimelineBottomPanel, wxWindow )
	EVT_SIZE( FxTimelineBottomPanel::OnSize )
	EVT_LEFT_UP( FxTimelineBottomPanel::OnLeftClick )
	EVT_PAINT( FxTimelineBottomPanel::OnPaint )

	EVT_TEXT_ENTER(ControlID_TimelineWidgetBottomPanelCurrTimeText, FxTimelineBottomPanel::UpdateCurrentTime)
	EVT_BUTTON(ControlID_TimelineWidgetBottomPanelPlayButton, FxTimelineBottomPanel::OnPlayButton)
	EVT_COMMAND(ControlID_TimelineWidgetBottomPanelPlayButton, wxEVT_COMMAND_BUTTON_DROP_CLICKED, FxTimelineBottomPanel::OnPlayButtonDrop)
	EVT_BUTTON(ControlID_TimelineWidgetBottomPanelResetButton, FxTimelineBottomPanel::OnResetButton)
	EVT_BUTTON(ControlID_TimelineWidgetBottomPanelLoopToggle, FxTimelineBottomPanel::OnLoopToggle)

	EVT_MENU_RANGE(MenuID_TimelineWidgetBottomPanelVerySlow, MenuID_TimelineWidgetBottomPanelVeryFast, FxTimelineBottomPanel::OnPlaybackSpeedChange)
END_EVENT_TABLE()

FxTimelineBottomPanel::FxTimelineBottomPanel()
	: _parent(0)
	, _mainSizer(0)
	, _collapsed(FxFalse)
	, _loopButton(0)
	, _resetButton(0)
	, _playButton(0)
	, _currentTimeText(0)
	, _currentTimeCtrl(0)
	, _isPlaying(FxFalse)
	, _currentTime(FxInvalidValue)
{
}

FxTimelineBottomPanel::FxTimelineBottomPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos /* = wxDefaultPosition */, const wxSize& size /* = wxDefaultSize */, long style /* = 0  */ )
	: _parent(0)
	, _mainSizer(0)
	, _collapsed(FxFalse)
	, _loopButton(0)
	, _resetButton(0)
	, _playButton(0)
	, _currentTimeText(0)
	, _currentTimeCtrl(0)
	, _isPlaying(FxFalse)
	, _currentTime(FxInvalidValue)
{
	Create( parent, id, pos, size, style );
}

void FxTimelineBottomPanel::Create(wxWindow *parent, wxWindowID id, const wxPoint& pos /* = wxDefaultPosition */, const wxSize& size /* = wxDefaultSize */, long style /* = 0  */ )
{
	wxWindow::Create( parent, id, pos, size, style );

	_parent = static_cast<FxTimelineWidget*>(parent);

	_mainSizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer( _mainSizer );
	SetAutoLayout( FALSE );
	_mainSizer->AddSpacer(12);

	_playEnabled = wxIcon(wxIconLocation(FxStudioApp::GetIconPath(wxT("play.ico"))));
	_playEnabled.SetWidth(16);
	_playEnabled.SetHeight(16);
	_playDisabled = wxIcon(wxIconLocation(FxStudioApp::GetIconPath(wxT("play_disabled.ico"))));
	_playDisabled.SetWidth(16);
	_playDisabled.SetHeight(16);

	_stopEnabled = wxIcon(wxIconLocation(FxStudioApp::GetIconPath(wxT("stop.ico"))));
	_stopEnabled.SetWidth(16);
	_stopEnabled.SetHeight(16);
	_stopDisabled = wxIcon(wxIconLocation(FxStudioApp::GetIconPath(wxT("stop_disabled.ico"))));
	_stopDisabled.SetWidth(16);
	_stopDisabled.SetHeight(16);

	wxIcon loopEnabled = wxIcon(wxIconLocation(FxStudioApp::GetIconPath(wxT("loop.ico"))));
	loopEnabled.SetWidth(16);
	loopEnabled.SetHeight(16);
	wxIcon loopDisabled = wxIcon(wxIconLocation(FxStudioApp::GetIconPath(wxT("loop_disabled.ico"))));
	loopDisabled.SetWidth(16);
	loopDisabled.SetHeight(16);

	wxIcon rewindEnabled = wxIcon(wxIconLocation(FxStudioApp::GetIconPath(wxT("rewind.ico"))));
	rewindEnabled.SetWidth(16);
	rewindEnabled.SetHeight(16);
	wxIcon rewindDisabled = wxIcon(wxIconLocation(FxStudioApp::GetIconPath(wxT("rewind_disabled.ico"))));
	rewindDisabled.SetWidth(16);
	rewindDisabled.SetHeight(16);

	// Create the control group on the left.
	wxPanel* leftPanel = new wxPanel(this);
	leftPanel->SetSizer( new wxBoxSizer(wxHORIZONTAL) );
	leftPanel->SetAutoLayout(FxTrue);
	_loopButton = new FxButton(leftPanel, ControlID_TimelineWidgetBottomPanelLoopToggle, wxT("@"), wxDefaultPosition, wxSize(20,20));
	_loopButton->SetEnabledBitmap( loopEnabled );
	_loopButton->SetDisabledBitmap(loopDisabled);
	_loopButton->MakeToggle();
	_loopButton->SetToolTip( _("Toggle looped playback") );
	_loopButton->Enable(FxFalse);
	leftPanel->GetSizer()->Add(_loopButton, 0, wxALL|wxEXPAND|wxALIGN_BOTTOM, 1);
	_resetButton = new FxButton(leftPanel, ControlID_TimelineWidgetBottomPanelResetButton, wxT("|<<"), wxDefaultPosition, wxSize(20,20));
	_resetButton->SetEnabledBitmap( rewindEnabled );
	_resetButton->SetDisabledBitmap(rewindDisabled);
	_resetButton->SetToolTip( _("Reset the cursor to the beginning of the time range") );
	_resetButton->Enable(FxFalse);
	leftPanel->GetSizer()->Add(_resetButton, 0, wxALL|wxEXPAND|wxALIGN_BOTTOM, 1);
	_playButton = new FxDropButton(leftPanel, ControlID_TimelineWidgetBottomPanelPlayButton, wxT(">"), wxDefaultPosition, wxSize(30,20));
	_playButton->SetToolTip( _("Play the current time range") );
	_playButton->Enable(FxFalse);
	leftPanel->GetSizer()->Add(_playButton, 0, wxALL|wxEXPAND|wxALIGN_BOTTOM, 1);
	_currentTimeText = new wxTextCtrl( leftPanel, ControlID_TimelineWidgetBottomPanelCurrTimeText, wxEmptyString, wxDefaultPosition, wxSize(40,20));
	_currentTimeText->SetToolTip( _("Current Time") );
	_currentTimeText->Enable(FxFalse);
	leftPanel->GetSizer()->Add(_currentTimeText, 0, wxALL|wxEXPAND|wxALIGN_BOTTOM, 1);

	_mainSizer->Add(leftPanel, 0, 0, 0);

	// Create the centre panel.
	_currentTimeCtrl = new FxCurrentTimeCtrl( this );
	_mainSizer->Add( _currentTimeCtrl, 1, 0, 0 );

	SetIcons( FxFalse );

	Layout();
}

wxSize FxTimelineBottomPanel::DoGetBestSize() const
{
	if( !_collapsed )
	{
		return wxSize( FX_INT32_MAX, 27 );
	}
	else
	{
		return wxSize( FX_INT32_MAX, 5 );
	}
}

void FxTimelineBottomPanel::SetTimeExtent( FxReal minTime, FxReal maxTime )
{
	if( minTime != FxInvalidValue && maxTime != FxInvalidValue )
	{
		_currentTimeText->Enable(FxTrue);
		_loopButton->Enable(FxTrue);
		_resetButton->Enable(FxTrue);
		_playButton->Enable(FxTrue);
	}
	else
	{
		_currentTimeText->Enable(FxFalse);
		_loopButton->Enable(FxFalse);
		_resetButton->Enable(FxFalse);
		_playButton->Enable(FxFalse);
	}
	_currentTimeCtrl->SetTimeExtent( minTime, maxTime );
}

void FxTimelineBottomPanel::SetCurrentTime( FxReal currentTime )
{
	_currentTime = currentTime;
	_currentTimeCtrl->SetCurrentTime( _currentTime );
	if( currentTime != FxInvalidValue )
	{
		_currentTimeText->Enable(FxTrue);
		_currentTimeText->SetValue( FxFormatTime(_currentTime, FxStudioOptions::GetTimelineGridFormat()) );
	}
	else
	{
		_currentTimeText->SetValue( wxEmptyString );
		_currentTimeText->Enable(FALSE);
	}
}

FxReal FxTimelineBottomPanel::GetMarkerTimeDelta()
{
	return _currentTimeCtrl->GetMarkerTimeDelta();
}

void FxTimelineBottomPanel::SetIcons( FxBool playing )
{
	_isPlaying = playing;
	if( !_isPlaying )
	{
		_playButton->SetEnabledBitmap(_playEnabled);
		_playButton->SetDisabledBitmap(_playDisabled);
	}
	else
	{
		_playButton->SetEnabledBitmap(_stopEnabled);
		_playButton->SetDisabledBitmap(_stopDisabled);
	}
	_playButton->Refresh();

}

void FxTimelineBottomPanel::Redraw()
{
	_currentTimeCtrl->Refresh(FALSE);
	_currentTimeCtrl->Update();
}

void FxTimelineBottomPanel::GridRefresh(FxInt32 w, FxInt32 h)
{
	_currentTimeCtrl->GridRefresh(w, h);
	SetCurrentTime(_currentTime);
}

void FxTimelineBottomPanel::OnSize( wxSizeEvent& FxUnused(event) )
{
	Layout();
}

void FxTimelineBottomPanel::OnLeftClick( wxMouseEvent& event )
{
	SetFocus();
	if( (_collapsed && event.GetX() <= 23) ||
		(!_collapsed && event.GetX() <= 10 ) )
	{
		_collapsed = !_collapsed;

		if( _collapsed )
		{
			_mainSizer->Show( 1, false );
			_mainSizer->Show( 2, false );
		}
		else
		{
			_mainSizer->Show( 1, true );
			_mainSizer->Show( 2, true );
		}

		if( GetGrandParent() )
		{
			GetGrandParent()->Layout();
		}
		Layout();
	}
}

void FxTimelineBottomPanel::OnPaint( wxPaintEvent& event )
{
	wxPaintDC dc(this);
	wxColour buttonColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
	wxColour buttonHighlight = wxSystemSettings::GetColour(wxSYS_COLOUR_3DHIGHLIGHT);
	wxColour buttonShadow = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
	if( _collapsed )
	{
		dc.SetBrush(wxBrush(buttonColour,wxSOLID));
		dc.SetPen(wxPen(buttonShadow,1,wxSOLID));
		dc.DrawRectangle(1, 0, 23, 5);
		dc.SetPen(wxPen(buttonHighlight,1,wxSOLID));
		dc.DrawLine(1,5,1,0);
		dc.DrawLine(1,0,23,0);

		wxPoint lines[] = {wxPoint(0,5), wxPoint(2,0), wxPoint(4,5)};
		dc.SetPen(wxPen(wxColour(75,75,75),1,wxSOLID));
		dc.DrawLines( WXSIZEOF(lines), lines, 3 );
		dc.DrawLines( WXSIZEOF(lines), lines, 9 );
		dc.DrawLines( WXSIZEOF(lines), lines, 15 );
	}
	else
	{
		dc.SetBrush(wxBrush(buttonColour,wxSOLID));
		dc.SetPen(wxPen(buttonShadow,1,wxSOLID));
		dc.DrawRectangle(1, 0, 11, 26);
		dc.SetPen(wxPen(buttonHighlight,1,wxSOLID));
		dc.DrawLine(1,26,1,0);
		dc.DrawLine(1,0,11,0);

		wxPoint lines[] = {wxPoint(0,0), wxPoint(2,5), wxPoint(4,0)};
		dc.SetPen(wxPen(wxColour(75,75,75),1,wxSOLID));
		dc.DrawLines( WXSIZEOF(lines), lines, 4, 5 );
		dc.DrawLines( WXSIZEOF(lines), lines, 4, 11 );
		dc.DrawLines( WXSIZEOF(lines), lines, 4, 17 );

		event.Skip();
	}
}

void FxTimelineBottomPanel::UpdateCurrentTime( wxCommandEvent& FxUnused(event) )
{
	FxReal currentTime = FxUnformatTime(_currentTimeText->GetValue(), FxStudioOptions::GetTimelineGridFormat());
	SetCurrentTime(currentTime);
	_parent->SetCurrentTime( currentTime );
}

void FxTimelineBottomPanel::OnPlayButton( wxCommandEvent& FxUnused(event) )
{
	if( !_isPlaying )
	{
		_parent->PlayAudio( _loopButton->GetToggleValue() );
	}
	else
	{
		_parent->StopAudio();
	}
}

void FxTimelineBottomPanel::OnPlayButtonDrop( wxCommandEvent& FxUnused(event) )
{
	wxMenu popup;
	popup.AppendCheckItem( MenuID_TimelineWidgetBottomPanelVerySlow, _("Very Slow") );
	popup.AppendCheckItem( MenuID_TimelineWidgetBottomPanelSlow, _("Slow") );
	popup.AppendCheckItem( MenuID_TimelineWidgetBottomPanelNormal, _("Normal") );
	popup.AppendCheckItem( MenuID_TimelineWidgetBottomPanelFast, _("Fast") );
	popup.AppendCheckItem( MenuID_TimelineWidgetBottomPanelVeryFast, _("Very Fast") );

	FxConsoleVariable* g_playbackspeed = FxVM::FindConsoleVariable("g_playbackspeed");
	if( g_playbackspeed )
	{
		FxReal playbackSpeed = g_playbackspeed->GetFloat();

		if( FxRealEquality(playbackSpeed, 0.5f) )
		{
			popup.Check(MenuID_TimelineWidgetBottomPanelVerySlow, FxTrue);
		}
		else if( FxRealEquality(playbackSpeed, 0.75f) )
		{
			popup.Check(MenuID_TimelineWidgetBottomPanelSlow, FxTrue);
		}
		else if( FxRealEquality(playbackSpeed, 1.0f) )
		{
			popup.Check(MenuID_TimelineWidgetBottomPanelNormal, FxTrue);
		}
		else if( FxRealEquality(playbackSpeed, 1.50f) )
		{
			popup.Check(MenuID_TimelineWidgetBottomPanelFast, FxTrue);
		}
		else if( FxRealEquality(playbackSpeed, 2.0f) )
		{
			popup.Check(MenuID_TimelineWidgetBottomPanelVeryFast, FxTrue);
		}
	}

	PopupMenu( &popup, wxPoint(_playButton->GetRect().GetRight()+3, _playButton->GetRect().GetBottom()) );
}

void FxTimelineBottomPanel::OnResetButton( wxCommandEvent& FxUnused(event) )
{
	_parent->StopAudio();
	SetCurrentTime( _currentTimeCtrl->_minTime );
	_parent->SetCurrentTime( _currentTimeCtrl->_minTime );
}

void FxTimelineBottomPanel::OnLoopToggle( wxCommandEvent& FxUnused(event) )
{
	// note: because the value hasn't changed yet, we have to update the parent
	// to what the state *will* be, hence the negation.
	_parent->SetLooping( !_loopButton->GetToggleValue() );
}

void FxTimelineBottomPanel::OnPlaybackSpeedChange( wxCommandEvent& event)
{
	_parent->StopAudio();
	switch( event.GetId() )
	{
	case MenuID_TimelineWidgetBottomPanelVerySlow:
		FxVM::ExecuteCommand("set -name \"g_playbackspeed\" -value \"0.5\"");
		break;
	case MenuID_TimelineWidgetBottomPanelSlow:
		FxVM::ExecuteCommand("set -name \"g_playbackspeed\" -value \"0.75\"");
		break;
	default:
	case MenuID_TimelineWidgetBottomPanelNormal:
		FxVM::ExecuteCommand("set -name \"g_playbackspeed\" -value \"1.0\"");
		break;
	case MenuID_TimelineWidgetBottomPanelFast:
		FxVM::ExecuteCommand("set -name \"g_playbackspeed\" -value \"1.5\"");
		break;
	case MenuID_TimelineWidgetBottomPanelVeryFast:
		FxVM::ExecuteCommand("set -name \"g_playbackspeed\" -value \"2.0\"");
		break;
	}
}

void FxTimelineBottomPanel::StartScrubbing()
{
	_parent->StartScrubbing();
}

void FxTimelineBottomPanel::UpdateTimeFromChild( FxReal current )
{
	if( current != FxInvalidValue )
	{
		if( _isPlaying )
		{
			_parent->StopAudio();
		}
		_parent->SetCurrentTime( current, FxTrue );
		_currentTimeText->SetValue(FxFormatTime(current, FxStudioOptions::GetTimelineGridFormat()));
	}
}

void FxTimelineBottomPanel::StopScrubbing()
{
	_parent->StopScrubbing();
}

//------------------------------------------------------------------------------
// Timeline widget
//------------------------------------------------------------------------------
WX_IMPLEMENT_DYNAMIC_CLASS( FxTimelineWidget, wxWindow )
BEGIN_EVENT_TABLE( FxTimelineWidget, wxWindow )
	EVT_SIZE(FxTimelineWidget::OnSize)
	EVT_HELP_RANGE(wxID_ANY, wxID_HIGHEST, FxTimelineWidget::OnHelp)
	EVT_MENU(MenuID_TimelineWidgetResetView, FxTimelineWidget::OnResetView)
	EVT_MENU(MenuID_TimelineWidgetPlayAudio, FxTimelineWidget::OnPlayAudio)
END_EVENT_TABLE()

FxTimelineWidget::FxTimelineWidget( wxWindow* parent, FxWidgetMediator* mediator )
	: wxWindow(parent, -1, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN|wxFULL_REPAINT_ON_RESIZE)
	, FxWidget(mediator)
	, _pActor(NULL)
	, _timeWindowPanel(0)
	, _currTimePanel(0)
	, _anim(NULL)
	, _boundGroupId(-1)
	, _boundGroupMinTime(FxInvalidValue)
	, _boundGroupMaxTime(FxInvalidValue)
	, _currentTime( FxInvalidValue)
	, _previousTime(0.0f)
	, _audioScrubbingAllowed(FxTrue)
	, _scrubStartTime(FxInvalidValue)
{
	static const int numEntries = 2;
	wxAcceleratorEntry entries[numEntries];
	entries[0].Set( wxACCEL_CTRL, (FxInt32)'R', MenuID_TimelineWidgetResetView );
	entries[1].Set( wxACCEL_NORMAL, WXK_SPACE, MenuID_TimelineWidgetPlayAudio );
	// Note to self: when adding an accelerator entry, update numEntries. It works better.
	wxAcceleratorTable accelerator( numEntries, entries );
	SetAcceleratorTable( accelerator );

	SetSizer( new wxBoxSizer(wxVERTICAL) );
	SetAutoLayout( FALSE );
	_timeWindowPanel = new FxTimelineTopPanel( this, ControlID_TimelineWidgetMainSliderWindow, wxDefaultPosition, wxDefaultSize );
	GetSizer()->Add( _timeWindowPanel, 0, wxGROW, 0 );
	_currTimePanel = new FxTimelineBottomPanel( this, ControlID_TimelineWidgetMainTimelineWindow, wxDefaultPosition, wxDefaultSize );
	GetSizer()->Add( _currTimePanel, 0, wxGROW, 0 );
	Layout();
}

FxTimelineWidget::~FxTimelineWidget()
{
}

// Time subscriber stuff
void FxTimelineWidget::OnNotifyTimeChange()
{
	if( _anim )
	{
		FxTimeManager::GetTimes( _boundGroupMinTime, _boundGroupMaxTime, _boundGroupId );
		_timeWindowPanel->SetMinMaxWindow( _boundGroupMinTime, _boundGroupMaxTime );
		_currTimePanel->SetTimeExtent( _boundGroupMinTime, _boundGroupMaxTime );
	}
}

FxReal FxTimelineWidget::GetMinimumTime()
{
	FxReal minExtent, maxExtent;
	_timeWindowPanel->GetMinMaxExtent(minExtent, maxExtent);
	if( _anim && _anim->GetStartTime() < minExtent )
	{
		_timeWindowPanel->SetMinMaxExtent(_anim->GetStartTime(), maxExtent);
		Refresh(FALSE);
	}
	return _anim ? _anim->GetStartTime() : FX_REAL_MAX;
}

FxReal FxTimelineWidget::GetMaximumTime()
{
	FxReal minExtent, maxExtent;
	_timeWindowPanel->GetMinMaxExtent(minExtent, maxExtent);
	if( _anim && _anim->GetEndTime() > maxExtent )
	{
		_timeWindowPanel->SetMinMaxExtent(minExtent, _anim->GetEndTime());
		Refresh(FALSE);
	}
	return _anim ? _anim->GetEndTime() : FX_REAL_MIN;
}

FxInt32 FxTimelineWidget::GetPaintPriority()
{
	return 1;
}

void FxTimelineWidget::VerifyPendingTimeChange( FxReal& min, FxReal& max )
{
	FxReal minExtent, maxExtent;
	_timeWindowPanel->GetMinMaxExtent( minExtent, maxExtent );
	if( min < minExtent && max > maxExtent )
	{
		min = minExtent;
		max = maxExtent;
	}
	else if( min < minExtent && max < _boundGroupMaxTime )
	{
		min = minExtent;
		max = _boundGroupMaxTime;
	}
	else if( min < minExtent && max > _boundGroupMaxTime )
	{
		min = minExtent;
	}
	else if( max > maxExtent && min > _boundGroupMinTime )
	{
		min = _boundGroupMinTime;
		max = maxExtent;
	}
	else if( max > maxExtent && min < _boundGroupMinTime )
	{
		max = maxExtent;
	}
}

void FxTimelineWidget::BindToTimeSubscriberGroup( FxInt32 groupId )
{
	if( _boundGroupId != groupId )
	{
		if( _boundGroupId >= 0 )
		{
			FxTimeManager::UnregisterSubscriber( this, _boundGroupId );
		}

		_boundGroupId = groupId;
		FxTimeManager::RegisterSubscriber( this, _boundGroupId );
	}
}

// Sets whether or not to scrub audio.
void FxTimelineWidget::SetAudioScrubbingAllowed( FxBool scrubbingAllowed )
{
	_audioScrubbingAllowed = scrubbingAllowed;
}

// Called when the application is shutting down.
void FxTimelineWidget::OnAppShutdown( FxWidget* FxUnused(sender) )
{
	if( _boundGroupId >= 0 )
	{
		FxTimeManager::UnregisterSubscriber( this, _boundGroupId );
	}
}

// Called when the actor pointer changes.
void FxTimelineWidget::OnActorChanged( FxWidget* FxUnused(sender), FxActor* actor )
{
	_pActor = actor;
	_anim = NULL;
	DoResetView();
}

// Called to refresh the widget
void FxTimelineWidget::OnRefresh( FxWidget* FxUnused(sender) )
{
	Refresh( FALSE );
}

// Called when the anim changes
void FxTimelineWidget::OnAnimChanged(FxWidget* FxUnused(sender), 
									 const FxName& FxUnused(animGroupName), 
									 FxAnim* anim )
{
	_anim = anim;
	DoResetView();
	Refresh(FALSE);
}

// Called when a widget changes the time
void FxTimelineWidget::OnTimeChanged( FxWidget* FxUnused(sender), FxReal newTime )
{
	_currentTime = newTime;
	if( _currTimePanel )
	{
		_currTimePanel->SetCurrentTime( _currentTime );
	}
}

// Called when the grid needs to be recalculated
void FxTimelineWidget::OnRecalculateGrid( FxWidget* FxUnused(sender) )
{
	FxInt32 w = GetClientRect().GetWidth();
	FxInt32 h = GetClientRect().GetHeight();
	_timeWindowPanel->GridRefresh(w, h);
	_currTimePanel->GridRefresh(w, h);
}

// Called to reset the view to the animation extents.
void FxTimelineWidget::OnResetView( wxCommandEvent& FxUnused(event) )
{
	DoResetView();
}

// Called to play the audio
void FxTimelineWidget::OnPlayAudio( wxCommandEvent& FxUnused(event) )
{
	// Simulate a play button press.
	wxCommandEvent playPressEvent(wxEVT_COMMAND_BUTTON_CLICKED, ControlID_TimelineWidgetBottomPanelPlayButton);
	_currTimePanel->ProcessEvent(playPressEvent);
}

// Sets the time range for the bound subscribers
void FxTimelineWidget::SetSubscriberTimeRange( FxReal min, FxReal max )
{
	_currTimePanel->SetTimeExtent( _boundGroupMinTime, _boundGroupMaxTime );
	_currTimePanel->Redraw();
	FxTimeManager::RequestTimeChange( min, max, _boundGroupId );
}

// Sets the current time
void FxTimelineWidget::SetCurrentTime( FxReal currentTime, FxBool scrubbing, FxBool isAutoRewind )
{
	static const FxReal kDefaultAudioScrubTimeDelta = 0.2f;
	_previousTime = _currentTime;
	_currentTime = currentTime;

	if( _scrubStartTime != FxInvalidValue && _currentTime != FxInvalidValue )
	{
		if( _mediator )
		{
			_mediator->OnTimeChanged( NULL, _currentTime );
		}
	}
	else
	{
		if( isAutoRewind && _currentTime != FxInvalidValue )
		{
			if( _mediator )
			{
				_mediator->OnTimeChanged( NULL, _currentTime );
			}
		}
		else
		{
			if( _pActor && _anim && _currentTime != FxInvalidValue )
			{
				wxString command = wxString::Format(wxT("currentTime -new %f"), _currentTime);
				FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
			}
		}
	}

	if( scrubbing && _audioScrubbingAllowed )
	{
		// Calculate how much audio should be played for the scrub.
		// This is calculated in place, but as a minor speedup this
		// could be calculated throughout the other places in the 
		// timeline widget where the marker time delta could possibly
		// change (there are quite a few and the speed hit is very
		// minimal, so rather than possibly forgetting to put this 
		// somewhere, I left it here).
		FxReal audioScrubTimeDelta = FxMax(kDefaultAudioScrubTimeDelta, _currTimePanel->GetMarkerTimeDelta());
		// How much time is between the current time selection 
		// and the previous one?
		FxReal timeDelta = FxAbs(_currentTime - _previousTime);
		// The duration of the of the entire animation.
		FxReal duration = FxTimeManager::GetMaximumTime() - FxTimeManager::GetMinimumTime();
		// The percent of the duration that the timeDelta represents.
		FxReal pct = timeDelta / duration;
		// The "velocity" of the time change.  Velocity is passed as
		// pitch to the play audio message and is initialized to
		// 1.0f (normal pitch) and updated if the percentage 
		// warrants it to produce a nice scrubbing effect.
		FxReal velocity = 1.0f;
		if( pct >= 0.05f )
		{
			velocity = 6.0f;
		}
		else if( pct >= 0.04f )
		{
			velocity = 5.0f;
		}
		else if( pct >= 0.03f )
		{
			velocity = 4.0f;
		}
		else if( pct >= 0.02f )
		{
			velocity = 3.0f;
		}
		else if( pct >= 0.01f )
		{
			velocity = 2.0f;
		}
		FxStudioAnimPlayer* animPlayer = FxStudioAnimPlayer::Instance();
		if( animPlayer )
		{
			animPlayer->ScrubAudio(_currentTime, audioScrubTimeDelta, velocity);
		}
	}
}

// Starts scrubbing
void FxTimelineWidget::StartScrubbing()
{
	_scrubStartTime = _currentTime;
}

// Stops scrubbing
void FxTimelineWidget::StopScrubbing()
{
	// If audio scrubbing is enabled, make sure the audio stops.
	if( _audioScrubbingAllowed )
	{
		StopAudio();
	}
	if( !FxRealEquality(_currentTime, _scrubStartTime) &&
		!FxRealEquality(_scrubStartTime, FxInvalidValue) )
	{
		if( _pActor && _anim )
		{
			// Issue the time changed command.
			wxString command = wxString::Format(wxT("currentTime -new %f -prev %f"), _currentTime, _scrubStartTime);
			FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
			_scrubStartTime = FxInvalidValue;
		}
	}
}

// Sets whether or not to loop
void FxTimelineWidget::SetLooping( FxBool loop )
{
	FxStudioAnimPlayer* animPlayer = FxStudioAnimPlayer::Instance();
	if( animPlayer )
	{
		animPlayer->SetLooping(loop);
	}
}

// Plays the audio
void FxTimelineWidget::PlayAudio( FxBool loop )
{
	FxStudioAnimPlayer* animPlayer = FxStudioAnimPlayer::Instance();
	if( animPlayer )
	{
		animPlayer->Play(loop);
	}
}

// Stops the audio
void FxTimelineWidget::StopAudio()
{
	FxStudioAnimPlayer* animPlayer = FxStudioAnimPlayer::Instance();
	if( animPlayer )
	{
		if( animPlayer->IsPlaying() )
		{
			animPlayer->Stop();
		}
		animPlayer->StopScrubbing();
	}
}

void FxTimelineWidget::DoResetView()
{
	if( _anim && _timeWindowPanel )
	{
		FxReal minTime = FxMin(0.0f,_anim->GetStartTime());
		FxReal maxTime = _anim->GetEndTime();
		if( _anim->GetUserData() )
		{
			FxPhonWordList* pPhons = reinterpret_cast<FxAnimUserData*>(_anim->GetUserData())->GetPhonemeWordListPointer();
			if( pPhons && pPhons->GetNumPhonemes() > 0)
			{
				minTime = FxMin( 0.0f, FxMin(pPhons->GetPhonemeStartTime(0), _anim->GetStartTime()) );
				maxTime = FxMax( pPhons->GetPhonemeEndTime(pPhons->GetNumPhonemes()-1), _anim->GetEndTime() );
			}
		}
		_timeWindowPanel->SetMinMaxExtent( minTime, maxTime );
		FxTimeManager::RequestTimeChange( minTime, maxTime );
	}
	else if( _timeWindowPanel )
	{
		_timeWindowPanel->SetMinMaxExtent( FxInvalidValue, FxInvalidValue );
		_timeWindowPanel->SetMinMaxWindow( FxInvalidValue, FxInvalidValue );
		_currTimePanel->SetTimeExtent( FxInvalidValue, FxInvalidValue );
		_currTimePanel->SetCurrentTime( FxInvalidValue );
	}
}

void FxTimelineWidget::OnSize( wxSizeEvent& event )
{
	FxInt32 w = event.GetSize().GetWidth();
	FxInt32 h = event.GetSize().GetHeight();
	GetSizer()->SetDimension( 0, 0, w, h );
	_timeWindowPanel->GridRefresh(w, h);
	_currTimePanel->GridRefresh(w, h);
}

void FxTimelineWidget::OnHelp( wxHelpEvent& FxUnused(event) )
{
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().LoadFile();
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().DisplaySection(wxT("Scrub Bar, Timeline, Command Line"));
}

// Gets the start of the time range.
FxReal FxTimelineWidget::GetTimeRangeStart()
{
	return _boundGroupMinTime;
}

// Gets the end of the time range.
FxReal FxTimelineWidget::GetTimeRangeEnd()
{
	return _boundGroupMaxTime;
}

// Gets the current time.
FxReal FxTimelineWidget::GetCurrentTime()
{
	return _currentTime;
}

// Called when the animation starts.
void FxTimelineWidget::OnPlaybackStarted()
{
	_currTimePanel->SetIcons(FxTrue);
}

// Called when the animation is done playing
void FxTimelineWidget::OnPlaybackFinished()
{
	_currTimePanel->SetIcons(FxFalse);
}

} // namespace Face

} // namespace OC3Ent
