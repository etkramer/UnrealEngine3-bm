//------------------------------------------------------------------------------
// Useful grid cell renderers and editors.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxGridTools_H__
#define FxGridTools_H__

#include "FxStudioOptions.h"

namespace OC3Ent
{

namespace Face
{

// Renders colour-coded float cells.
class FxGridCellFloatRenderer : public wxGridCellFloatRenderer
{
public:
	FxGridCellFloatRenderer( int width = -1, int precision = -1 )
		: wxGridCellFloatRenderer(width, precision)
		, _width(width)
		, _precision(precision)
	{
	}

	// Overload the way we draw floats to tint the background colour to
	// indicate weighting.
	virtual void Draw( wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, 
		const wxRect& rectCell, int row, int col, bool isSelected )
	{
		dc.SetBackgroundMode(wxSOLID);

		// Grey out fields if the grid is disabled.
		if( grid.IsEnabled() )
		{
			FxReal val = FxAtof(grid.GetCellValue(row,col).mb_str(wxConvLibc));
			val = FxClamp(-1.0f, static_cast<FxReal>(val), 1.0f);
			if ( isSelected && FxRealEquality(val, 0.0f) )
			{
				dc.SetBrush(wxBrush(grid.GetSelectionBackground(), wxSOLID));
			}
			else
			{
				wxColour minColour  = FxStudioOptions::GetMinValueColour();
				wxColour zeroColour = FxStudioOptions::GetZeroValueColour();
				wxColour maxColour  = FxStudioOptions::GetMaxValueColour();
				wxColour colour(zeroColour);
				if( val > 0.0f )
				{
					colour = wxColour((maxColour.Red()-zeroColour.Red())*val+zeroColour.Red(),
						(maxColour.Green()-zeroColour.Green())*val+zeroColour.Green(),
						(maxColour.Blue()-zeroColour.Blue())*val+zeroColour.Blue());
				}
				else if( val < 0.0f )
				{
					val *= -1;
					colour = wxColour((minColour.Red()-zeroColour.Red())*val+zeroColour.Red(),
						(minColour.Green()-zeroColour.Green())*val+zeroColour.Green(),
						(minColour.Blue()-zeroColour.Blue())*val+zeroColour.Blue());
				}
				dc.SetBrush(wxBrush(colour, wxSOLID));
			}
		}
		else
		{
			dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE), wxSOLID));
		}
		attr.SetTextColour(FxStudioOptions::GetOverlayTextColour());
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.DrawRectangle(rectCell);

		SetTextColoursAndFont(grid, attr, dc, isSelected);

		// Draw the text right aligned by default.
		int hAlign, vAlign;
		attr.GetAlignment(&hAlign, &vAlign);
		hAlign = wxALIGN_RIGHT;

		wxRect rect = rectCell;
		rect.Inflate(-1);

		grid.DrawTextRectangle(dc, GetString(grid, row, col), rect, hAlign, vAlign);
	}

	virtual wxGridCellRenderer* Clone( void ) const
	{
		return new FxGridCellFloatRenderer(_width, _precision);
	}

protected:
	FxInt32 _width;
	FxInt32 _precision;
};

// Overload the starting key of the float editor to correctly handle '.'.
class FxGridCellFloatEditor : public wxGridCellFloatEditor
{
public:
	FxGridCellFloatEditor( int width = -1, int precision = -1 )
		:wxGridCellFloatEditor(width, precision)
		, _width(width)
		, _precision(precision)
	{
	}

	virtual void StartingKey(wxKeyEvent& event)
	{
		int keycode = (int)event.GetKeyCode();
		if ( isdigit(keycode) || keycode == '+' || keycode == '-' || keycode == '.'
			|| keycode ==  WXK_NUMPAD0
			|| keycode ==  WXK_NUMPAD1
			|| keycode ==  WXK_NUMPAD2
			|| keycode ==  WXK_NUMPAD3
			|| keycode ==  WXK_NUMPAD4
			|| keycode ==  WXK_NUMPAD5
			|| keycode ==  WXK_NUMPAD6
			|| keycode ==  WXK_NUMPAD7
			|| keycode ==  WXK_NUMPAD8
			|| keycode ==  WXK_NUMPAD9
			|| keycode ==  WXK_ADD
			|| keycode ==  WXK_NUMPAD_ADD
			|| keycode ==  WXK_SUBTRACT
			|| keycode ==  WXK_NUMPAD_SUBTRACT
			|| keycode ==  WXK_NUMPAD_DECIMAL )
		{
			wxGridCellTextEditor::StartingKey(event);

			// Skip Skip() below.
			return;
		}

		event.Skip();
	}

	virtual wxGridCellEditor* Clone() const
	{
		return new FxGridCellFloatEditor(_width, _precision);
	}

protected:
	FxInt32 _width;
	FxInt32 _precision;
};

} // namespace Face

} // namespace OC3Ent

#endif
