//------------------------------------------------------------------------------
// A way of drawing sliders so they can be more "glanceable"
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxSlider.h"
#include "FxUtil.h"
#include "FxStudioOptions.h"

namespace OC3Ent
{

namespace Face
{

WX_IMPLEMENT_DYNAMIC_CLASS(FxSlider, wxWindow)
BEGIN_EVENT_TABLE(FxSlider, wxWindow)
	EVT_PAINT(FxSlider::Draw)
	EVT_MOUSE_EVENTS(FxSlider::OnMouse)
END_EVENT_TABLE()

FxSlider::FxSlider(void)
	: _minValue( 0 )
	, _maxValue( 100 )
	, _currValue( 0 )
	, _pageSize( 1 )
	, _lineSize( 1 )
	, _flags( 0 )
{
	Initialize();
}

FxSlider::FxSlider( wxWindow *parent, wxWindowID id,
					  int value, int minValue, int maxValue,
					  const wxPoint& pos, const wxSize& size, FxInt32 style )
	: wxWindow( parent, id, pos, size )
	, _minValue( minValue )
	, _maxValue( maxValue )
	, _currValue( value )
	, _pageSize( 1 )
	, _lineSize( 1 )
	, _flags( style )
{
	Initialize();
}

FxSlider::~FxSlider(void)
{
}

int FxSlider::GetValue() const
{
	return _currValue;
}

void FxSlider::SetValue(int value)
{
	_currValue = value;
	Refresh( FALSE );
	Update();
}

void FxSlider::SetRange(int minValue, int maxValue)
{
	_minValue = minValue;
	_maxValue = maxValue;
	Refresh( FALSE );
	Update();
}

int FxSlider::GetMin() const
{
	return _minValue;
}

int FxSlider::GetMax() const
{
	return _maxValue;
}

void FxSlider::SetLineSize(int lineSize)
{
	_lineSize = lineSize;
}

void FxSlider::SetPageSize(int pageSize)
{
	_pageSize = pageSize;
}

int FxSlider::GetLineSize() const
{
	return _lineSize;
}

int FxSlider::GetPageSize() const
{
	return _pageSize;
}

// these methods get/set the length of the slider pointer in pixels
void FxSlider::SetThumbLength(int WXUNUSED(lenPixels))
{
}

int FxSlider::GetThumbLength() const
{
	return _rightBorder + _leftBorder;
}

void FxSlider::Draw( wxPaintEvent& FxUnused(event) )
{
	wxRect clientRect( GetClientRect() );
	wxPaintDC front( this );

	wxColour minColour  = FxStudioOptions::GetMinValueColour();
	wxColour zeroColour = FxStudioOptions::GetZeroValueColour();
	wxColour maxColour  = FxStudioOptions::GetMaxValueColour();
	wxColour overlayColour = FxStudioOptions::GetOverlayTextColour();
	wxBrush backgroundBrush( zeroColour, wxSOLID );
	wxBrush threeDeeFaceBrush( wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE), wxSOLID );
	wxPen   highlightPen( wxSystemSettings::GetColour(wxSYS_COLOUR_3DHIGHLIGHT), 2, wxSOLID );
	wxPen   shadowPen( wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW), 2, wxSOLID );
	wxPen   darkShadowPen( wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW), 1, wxSOLID );

	if( !IsEnabled() )
	{
		// Draw in a "disabled" style.
		backgroundBrush = threeDeeFaceBrush;
	}

	// Set up the backbuffer
	wxMemoryDC dc;
	wxBitmap backbuffer( front.GetSize().GetWidth(), front.GetSize().GetHeight() );
	dc.SelectObject( backbuffer );
	dc.SetBackground( threeDeeFaceBrush  );
	dc.Clear();

	FxInt32 zero = ValueToCoord( _minValue > 0 ? _minValue : _maxValue < 0 ? _maxValue : 0 );
	FxInt32 curr = ValueToCoord( _currValue );

	wxColour colour(zeroColour);
	if( _currValue > 0 )
	{
		FxReal val = _currValue/static_cast<FxReal>(_maxValue);
		colour = wxColour((maxColour.Red()-zeroColour.Red())*val+zeroColour.Red(),
						  (maxColour.Green()-zeroColour.Green())*val+zeroColour.Green(),
						  (maxColour.Blue()-zeroColour.Blue())*val+zeroColour.Blue());
	}
	else if( _currValue < 0 )
	{
		FxReal val = -_currValue/-static_cast<FxReal>(_minValue);
		colour = wxColour((minColour.Red()-zeroColour.Red())*val+zeroColour.Red(),
						  (minColour.Green()-zeroColour.Green())*val+zeroColour.Green(),
						  (minColour.Blue()-zeroColour.Blue())*val+zeroColour.Blue());
	}

	wxRect sliderGrooveRect( wxPoint(clientRect.GetLeft() + _leftBorder, clientRect.GetTop() + 5),
							 wxPoint(clientRect.GetRight() - _rightBorder, clientRect.GetBottom() - 5) );

	dc.SetPen( shadowPen );
	dc.SetBrush( backgroundBrush );
	dc.DrawRectangle( sliderGrooveRect );
	dc.SetPen( darkShadowPen );
	dc.DrawLine( wxPoint(sliderGrooveRect.GetLeft(), sliderGrooveRect.GetBottom()),
				 wxPoint(sliderGrooveRect.GetLeft(), sliderGrooveRect.GetTop()) );
	dc.DrawLine( wxPoint(sliderGrooveRect.GetLeft(), sliderGrooveRect.GetTop()),
				 wxPoint(sliderGrooveRect.GetRight(), sliderGrooveRect.GetTop()) );
	dc.SetPen( highlightPen );
	dc.DrawLine( wxPoint(sliderGrooveRect.GetLeft(), sliderGrooveRect.GetBottom()),
				 wxPoint(sliderGrooveRect.GetRight(), sliderGrooveRect.GetBottom()) );
	dc.DrawLine( wxPoint(sliderGrooveRect.GetRight(), sliderGrooveRect.GetBottom()),
				 wxPoint(sliderGrooveRect.GetRight(), sliderGrooveRect.GetTop()) );

	dc.SetPen( *wxTRANSPARENT_PEN );
	dc.SetBrush( wxBrush( colour, wxSOLID ) );
	wxRect sliderGrooveFillRect(wxPoint(zero, sliderGrooveRect.GetTop() + 1), wxSize(curr-zero, sliderGrooveRect.GetHeight()-2));
	dc.DrawRectangle( sliderGrooveFillRect );

	dc.SetPen( wxPen(overlayColour, 1, wxSOLID) );
	dc.DrawLine( wxPoint(zero, sliderGrooveRect.GetTop()), wxPoint(zero, sliderGrooveRect.GetBottom()) );

	wxRect thumbPosRect( wxPoint(curr - _leftBorder, clientRect.GetTop() + 1),
						 wxPoint(curr + _rightBorder, clientRect.GetBottom() - 1) );
	dc.SetPen( highlightPen );
	dc.SetBrush( threeDeeFaceBrush );
	dc.DrawRectangle(thumbPosRect);
	dc.SetPen( shadowPen );
	dc.DrawLine( wxPoint(thumbPosRect.GetLeft(), thumbPosRect.GetBottom()),
		wxPoint(thumbPosRect.GetRight(), thumbPosRect.GetBottom()) );
	dc.DrawLine( wxPoint(thumbPosRect.GetRight(), thumbPosRect.GetBottom()),
		wxPoint(thumbPosRect.GetRight(), thumbPosRect.GetTop()) );
	dc.SetPen( darkShadowPen );
	dc.DrawLine( wxPoint(thumbPosRect.GetLeft(), thumbPosRect.GetBottom()),
		wxPoint(thumbPosRect.GetRight(), thumbPosRect.GetBottom()) );
	dc.DrawLine( wxPoint(thumbPosRect.GetRight(), thumbPosRect.GetBottom()),
		wxPoint(thumbPosRect.GetRight(), thumbPosRect.GetTop()) );

	// Swap to the front buffer
	front.Blit( front.GetLogicalOrigin(), front.GetSize(), &dc, dc.GetLogicalOrigin(), wxCOPY );
}

void FxSlider::OnMouse( wxMouseEvent& event )
{
	if( event.LeftDown() && !HasCapture() )
	{
		CaptureMouse();
	}
	if( event.LeftUp() && HasCapture() )
	{
		ReleaseMouse();
	}
	if( event.LeftIsDown() && HasCapture() )
	{
		_currValue = FxClamp(_minValue,CoordToValue( event.GetPosition().x ),_maxValue);
		Refresh(FALSE);
		Update();

		wxScrollEvent event(wxEVT_SCROLL_THUMBTRACK, m_windowId);
		event.SetPosition(_currValue);
		event.SetEventObject( this );
		GetEventHandler()->ProcessEvent(event);

		wxCommandEvent cevent( wxEVT_COMMAND_SLIDER_UPDATED, GetId() );
		cevent.SetInt( _currValue );
		cevent.SetEventObject( this );

		GetEventHandler()->ProcessEvent( cevent );
	}
}

void FxSlider::Initialize()
{
	_leftBorder = 2;
	_rightBorder = 2;
}

FxInt32 FxSlider::ValueToCoord( FxInt32 value )
{
	FxInt32 midWidth = GetClientRect().GetWidth() - _leftBorder - _rightBorder;
	return ( static_cast<FxReal>( value - _minValue ) / 
		     static_cast<FxReal>( _maxValue - _minValue ) ) * 
		     midWidth + _leftBorder;
}

FxInt32 FxSlider::CoordToValue( FxInt32 coord )
{
	FxInt32 midWidth = GetClientRect().GetWidth() - _leftBorder - _rightBorder;
	return ( static_cast<FxReal>( coord - _leftBorder ) / 
		     static_cast<FxReal>( midWidth ) ) * 
		     ( _maxValue - _minValue ) + _minValue;
}

} // namespace Face

} // namespace OC3Ent
