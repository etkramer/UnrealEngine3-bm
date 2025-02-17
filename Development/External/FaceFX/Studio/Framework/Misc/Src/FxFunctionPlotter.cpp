//------------------------------------------------------------------------------
// A class to plot functions to a wxBitmap
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxLinkFn.h"
#include "FxFunctionPlotter.h"
#include "FxStudioOptions.h"

namespace OC3Ent
{

namespace Face
{

wxBitmap FxFunctionPlotter::Plot(  FxLinkFnType lfType, const FxLinkFnParameters& parameters, 
								   wxSize dimension, FxReal minX, FxReal maxX, 
								   wxColour backgroundColour,
								   FxReal minY, FxReal maxY, FxBool drawGrid,
								   wxColour gridColour, wxColour plotColour, wxColour labelColour,
								   wxString xLabel, wxString yLabel, wxRect* plotRect  )
{
	wxBitmap retval( dimension.GetWidth(), dimension.GetHeight() );
	wxMemoryDC dc;
	dc.SelectObject( retval );
	dc.SetBackground( wxBrush( backgroundColour, wxSOLID ) );
	dc.Clear();

	if( drawGrid && (dimension.GetWidth() < 250 || dimension.GetHeight() < 100) )
	{
		drawGrid = FxFalse;
	}

	wxRect clientRect( wxPoint(0, 0), dimension );
	wxRect totalRect(clientRect);
	if( drawGrid )
	{
		clientRect.Deflate(12,12);
		clientRect.SetLeft(12);
		clientRect.SetTop(12);
	}
	if( plotRect )
	{
		*plotRect = clientRect;
	}

	wxFont smallFont( 8, wxSWISS, wxNORMAL, wxBOLD );
	dc.SetFont( smallFont );

	// If we're supposed to figure out the range ourselves
	if( minY == FxInvalidValue || maxY == FxInvalidValue )
	{
		FxReal minimum = FX_REAL_MAX;
		FxReal maximum = FX_REAL_MIN;
		for( FxInt32 x = clientRect.GetLeft(); x <= clientRect.GetRight(); ++x )
		{
			FxReal time  = CoordToTime( x, clientRect, minX, maxX );
			FxReal value = FxLinkFnEval( lfType, time, parameters );
			minimum = FxMin( minimum, value );
			maximum = FxMax( maximum, value );
		}

		if( minY == FxInvalidValue )
		{
			minY = minimum;
		}
		if( maxY == FxInvalidValue )
		{
			maxY = maximum;
		}
	}

	FxInt32 zeroTime  = TimeToCoord( 0.0f, clientRect, minX, maxX );
	FxInt32 zeroValue = ValueToCoord( 0.0f, clientRect, minY, maxY );

	wxPoint* samples = new wxPoint[clientRect.GetWidth()];
	for( FxInt32 x = clientRect.GetLeft(); x <= clientRect.GetRight(); ++x )
	{
		FxReal time  = CoordToTime( x, clientRect, minX, maxX );
		FxReal value = FxClamp( minY, FxLinkFnEval( lfType, time, parameters ), maxY );
		FxInt32 y = FxClamp( clientRect.GetTop(), ValueToCoord( value, clientRect, minY, maxY ), clientRect.GetBottom() );
		FxInt32 zero = FxClamp( clientRect.GetTop(), ValueToCoord( 0.0f, clientRect, minY, maxY ), clientRect.GetBottom() );

		dc.SetPen( wxPen(InterpColour(minY, value, maxY), 1, wxSOLID) );
		dc.DrawLine( x, zero, x, y );

		samples[x-clientRect.GetLeft()] = wxPoint( x, y );
	}

	dc.SetPen( wxPen(plotColour, 1, wxSOLID) );
	dc.DrawLines( clientRect.GetWidth(), samples );
	delete[] samples;

	if( !drawGrid )
	{
		dc.SetPen( wxPen( gridColour, 1, wxDOT ) );
		dc.DrawLine( wxPoint( zeroTime, clientRect.GetTop() ), wxPoint( zeroTime, clientRect.GetBottom() ) );
		dc.DrawLine( wxPoint( clientRect.GetLeft(), zeroValue ), wxPoint( clientRect.GetRight(), zeroValue ) );
	}
	else
	{
		dc.SetPen( wxPen( gridColour, 1, wxSOLID ) );
		// Draw the horizontal grid
		FxReal currY = 0.0f, yStep = 1000.0f;
		FxInt32 xExtent = 0, yExtent = 0;
		wxString fmt(wxT("%.2f"));
		GetAxisPlottingInfo( minY, maxY, currY, yStep, fmt, clientRect.GetHeight() );
		while( currY < maxY + yStep )
		{
			FxInt32 y = ValueToCoord( currY, clientRect, minY, maxY );
			dc.DrawLine( wxPoint(clientRect.GetLeft(), y), wxPoint(clientRect.GetRight(), y) );
			wxString label = wxString::Format( fmt, currY );
			dc.GetTextExtent( label, &xExtent, &yExtent );
			dc.SetTextForeground(labelColour);
			// Make sure we're not hanging into dead space
			FxInt32 yCoord = y - (yExtent / 2);
			if( y - xExtent < clientRect.GetTop() ) yCoord = y;
			if( y + xExtent > clientRect.GetBottom() ) yCoord = y - xExtent;
			DrawShadowedText( dc, label, wxPoint(totalRect.GetRight()+2, yCoord), -90.0f );
			currY += yStep;
		}

		// Draw the vertical grid
		FxReal currX = 0.0f, xStep = 1000.0f;
		GetAxisPlottingInfo( minX, maxX, currX, xStep, fmt, clientRect.GetWidth() );
		while( currX < maxX + xStep )
		{
			FxInt32 x = TimeToCoord( currX, clientRect, minX, maxX );
			dc.DrawLine( wxPoint(x, clientRect.GetTop()), wxPoint(x, clientRect.GetBottom()) );
			wxString label = wxString::Format( fmt, currX );
			dc.GetTextExtent( label, &xExtent, &yExtent );
			dc.SetTextForeground(labelColour);
			// Make sure we're not hanging into dead space
			FxInt32 xCoord = x - (xExtent/2);
			if( x - xExtent < clientRect.GetLeft() ) xCoord = x;
			if( x + xExtent > clientRect.GetRight() ) xCoord = x - xExtent;
			DrawShadowedText( dc, label, wxPoint( xCoord, totalRect.GetTop() ) );
			currX += xStep;
		}

		FxInt32 left = clientRect.GetLeft(), right = clientRect.GetRight();
		FxInt32 top = clientRect.GetTop(), bottom = clientRect.GetBottom();
		for( FxInt32 x = left; x <= right; ++x )
		{
			if( x != left && x != right )
			{
				FxReal value = CoordToTime( x, clientRect, minX, maxX );
				dc.SetPen( wxPen(InterpColour(minX, value, maxX), 1, wxSOLID) );
			}
			else
			{
				dc.SetPen( *wxBLACK_PEN );
			}
			dc.DrawLine( x, clientRect.GetBottom(), x, totalRect.GetBottom() );
		}
		for( FxInt32 y = top; y <= bottom; ++y )
		{
			if( y != top && y != bottom )
			{
				FxReal value = CoordToValue( y, clientRect, minY, maxY );
				dc.SetPen( wxPen(InterpColour(minY, value, maxY), 1, wxSOLID) );
			}
			else
			{
				dc.SetPen( *wxBLACK_PEN );
			}
			dc.DrawLine( totalRect.GetLeft(), y, clientRect.GetLeft(), y );
		}
		dc.SetPen(*wxBLACK_PEN);
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawRectangle(clientRect);

		dc.GetTextExtent( xLabel, &xExtent, &yExtent );
		DrawShadowedText( dc, xLabel, wxPoint(totalRect.GetWidth()/2 - (xExtent/2), totalRect.GetBottom() - 12), 0.0f, FxStudioOptions::GetOverlayTextColour() );
		dc.GetTextExtent( yLabel, &xExtent, &yExtent );
		DrawShadowedText( dc, yLabel, wxPoint(totalRect.GetLeft(), totalRect.GetHeight()/2 + (xExtent/2)), 90.0f, FxStudioOptions::GetOverlayTextColour() );
	}
	if( !drawGrid )
	{
		dc.SetTextForeground( *wxBLACK );
		const FxLinkFn* func = FxLinkFn::FindLinkFunctionByType(lfType);
		if( func )
		{
			dc.DrawText( wxString::FromAscii(func->GetNameAsCstr()), 3, 0 );
		}
		else
		{
			dc.DrawText( wxT("Unknown Link Function!"), 3, 0 );
		}
	}

	dc.SelectObject( wxNullBitmap );
	return retval;
}

// Maps a pixel to a time
FxReal FxFunctionPlotter::CoordToTime( FxInt32 coord, const wxRect& clientRect, FxReal minTime, FxReal maxTime )
{
	return ( static_cast<FxReal>( coord - clientRect.GetLeft() ) / 
		static_cast<FxReal>( clientRect.GetWidth() ) ) * 
		( maxTime - minTime ) + minTime;
}

// Maps a time to a pixel
FxInt32 FxFunctionPlotter::TimeToCoord( FxReal time, const wxRect& clientRect, FxReal minTime, FxReal maxTime )
{
	return ( ( time - minTime ) / ( maxTime - minTime ) ) * 
		clientRect.GetWidth() + clientRect.GetLeft();
}

// Maps a pixel to a value
FxReal FxFunctionPlotter::CoordToValue( FxInt32 coord, const wxRect& clientRect, FxReal minValue, FxReal maxValue )
{
	FxReal pct = static_cast<FxReal>( coord - clientRect.GetTop() ) / 
		static_cast<FxReal>( clientRect.GetHeight() );
	return maxValue - (pct * (maxValue - minValue));
}

// Maps a value to a pixel
FxInt32 FxFunctionPlotter::ValueToCoord( FxReal value, const wxRect& clientRect, FxReal minValue, FxReal maxValue )
{
	FxReal pct = (value - minValue) / ( maxValue - minValue );
	return clientRect.GetBottom() - static_cast<FxReal>(clientRect.GetHeight()) * pct;
}

// Draws text with a drop shadow
void FxFunctionPlotter::DrawShadowedText( wxDC& dc, wxString text, wxPoint position, FxReal angle, wxColour foregroundColour )
{
	if( angle )
	{
		if( foregroundColour != *wxBLACK )
		{
			dc.SetTextForeground( *wxBLACK );
			dc.DrawRotatedText( text, wxPoint( position.x + 1, position.y + 1 ), angle );
		}
		dc.SetTextForeground( foregroundColour );
		dc.DrawRotatedText( text, position, angle );
	}
	else
	{
		if( foregroundColour != *wxBLACK )
		{
			dc.SetTextForeground( *wxBLACK );
			dc.DrawText( text, wxPoint( position.x + 1, position.y + 1 ) );
		}
		dc.SetTextForeground( foregroundColour );
		dc.DrawText( text, position );
	}
}

void FxFunctionPlotter::GetAxisPlottingInfo( FxReal minValue, FxReal maxValue, 
	FxReal& startValue, FxReal& gridIncrement, wxString& formatString, FxInt32 distance )
{
	FxReal deltaValue = maxValue - minValue;
	FxReal height = static_cast<FxReal>( distance );
	FxReal magic = height / deltaValue;
	if( magic > 45000.0f )
	{
		gridIncrement = 0.001f;
		formatString  = wxT("%0.3f");
	}
	else if( magic > 35000.0f )
	{
		gridIncrement = 0.0025f;
		formatString = wxT("%0.4f");
	}
	else if( magic > 10000.0f )
	{
		gridIncrement = 0.005f;
		formatString  = wxT("%0.3f");
	}
	else if( magic > 5000 )
	{
		gridIncrement = 0.01f;
		formatString = wxT("%0.2f");
	}
	else if( magic > 2500 )
	{
		gridIncrement = 0.05f;
		formatString = wxT("%0.2f");
	}
	else if( magic > 700.0f )
	{
		gridIncrement = 0.1f;
		formatString = wxT("%0.1f");
	}
	else if( magic > 400.f )
	{
		gridIncrement = 0.25f;
		formatString = wxT("%0.2f");
	}
	else if( magic > 175.0f )
	{
		gridIncrement = 0.5f;
		formatString = wxT("%0.1f");
	}
	else if( magic > 50.0f )
	{
		gridIncrement = 1.f;
		formatString = wxT("%0.0f");
	}
	else if( magic > 25.0f )
	{
		gridIncrement = 2.f;
		formatString = wxT("%0.0f");
	}
	else if( magic > 15.0f )
	{
		gridIncrement = 5.f;
		formatString = wxT("%0.0f");
	}
	else if( magic > 8.0f )
	{
		gridIncrement = 10.f;
		formatString = wxT("%0.0f");
	}
	else if( magic > 4.0f )
	{
		gridIncrement = 25.0f;
		formatString = wxT("%0.0f");
	}
	else if( magic > 2.0f )
	{
		gridIncrement = 50.0f;
		formatString = wxT("%0.0f");
	}
	else if( magic > 1.0f )
	{
		gridIncrement = 100.f;
		formatString = wxT("%0.0f");
	}
	else if( magic > 0.0f )
	{
		gridIncrement = 500.f;
		formatString = wxT("%0.0f");
	}
	else
	{
		gridIncrement = 1;
		formatString = _("none");
	}

	startValue = ((FxInt32)( minValue * 1000000.f ) / (FxInt32)( gridIncrement * 1000000) ) * gridIncrement;
}

wxColour FxFunctionPlotter::InterpColour( FxReal min, FxReal value, FxReal max )
{
	const wxColour minColour  = FxStudioOptions::GetMinValueColour();
	const wxColour zeroColour = FxStudioOptions::GetZeroValueColour();
	const wxColour maxColour  = FxStudioOptions::GetMaxValueColour();
	wxColour colour(zeroColour);
	if( value > 0 )
	{
		FxReal val = value/max;
		colour = wxColour((maxColour.Red()-zeroColour.Red())*val+zeroColour.Red(),
			(maxColour.Green()-zeroColour.Green())*val+zeroColour.Green(),
			(maxColour.Blue()-zeroColour.Blue())*val+zeroColour.Blue());
	}
	else if( value < 0 )
	{
		FxReal val = -value/-min;
		colour = wxColour((minColour.Red()-zeroColour.Red())*val+zeroColour.Red(),
			(minColour.Green()-zeroColour.Green())*val+zeroColour.Green(),
			(minColour.Blue()-zeroColour.Blue())*val+zeroColour.Blue());
	}
	return colour;
}

} // namespace Face

} // namespace OC3Ent
