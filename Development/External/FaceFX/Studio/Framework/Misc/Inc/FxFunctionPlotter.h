//------------------------------------------------------------------------------
// A class to plot functions to a wxBitmap
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFunctionPlotter_H__
#define FxFunctionPlotter_H__

#include "FxPlatform.h"
#include "FxArray.h"

namespace OC3Ent
{

namespace Face
{

class FxFunctionPlotter
{
public:
	static wxBitmap Plot( FxLinkFnType ftType, const FxLinkFnParameters& parameters, 
		wxSize dimension, FxReal minX, FxReal maxX, wxColour backgroundColour = wxColour(192,192,192),
		FxReal minY = FxInvalidValue, FxReal maxY = FxInvalidValue, FxBool drawGrid = FxFalse,
		wxColour gridColour = *wxBLACK, wxColour plotColour = *wxBLACK, wxColour labelColour = *wxWHITE,
		wxString xLabel = wxT("x"), wxString yLabel = wxT("y"), wxRect* plotRect = 0 );

	// Maps a pixel to a time
	static FxReal CoordToTime( FxInt32 coord, const wxRect& clientRect, FxReal minTime, FxReal maxTime );
	// Maps a time to a pixel
	static FxInt32 TimeToCoord( FxReal time, const wxRect& clientRect, FxReal minTime, FxReal maxTime );
	// Maps a pixel to a value
	static FxReal CoordToValue( FxInt32 coord, const wxRect& clientRect, FxReal minValue, FxReal maxValue );
	// Maps a value to a pixel
	static FxInt32 ValueToCoord( FxReal value, const wxRect& clientRect, FxReal minValue, FxReal maxValue );

	// Draws text with a drop shadow
	static void DrawShadowedText( wxDC& dc, wxString text, wxPoint position, 
		FxReal angle = 0.0f, wxColour foregroundColour = *wxWHITE );

	// Gets axis stuff
	static void GetAxisPlottingInfo( FxReal minValue, FxReal maxValue, 
		FxReal& startValue, FxReal& gridIncrement, wxString& formatString, FxInt32 distance );

	// Interpolates the correct colour between min and max.
	static wxColour InterpColour( FxReal min, FxReal value, FxReal max );
};

} // namespace Face

} // namespace OC3Ent

#endif
