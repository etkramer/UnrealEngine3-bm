//------------------------------------------------------------------------------
// An simple owner-drawn button with a dropdown button paired to it.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxDropButton.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/dcbuffer.h"
#endif

DEFINE_EVENT_TYPE(wxEVT_COMMAND_BUTTON_DROP_CLICKED)

namespace OC3Ent
{

namespace Face
{

WX_IMPLEMENT_DYNAMIC_CLASS(FxDropButton,wxWindow)

BEGIN_EVENT_TABLE(FxDropButton,wxWindow)
	EVT_SYS_COLOUR_CHANGED(FxDropButton::OnSysColourChanged)
	EVT_PAINT(FxDropButton::OnPaint)
	EVT_MOUSE_EVENTS(FxDropButton::OnMouse)
END_EVENT_TABLE()

FxDropButton::FxDropButton()
	: _mouseOver(FxFalse)
	, _depressed(FxFalse)
	, _value(FxFalse)
	, _toggle(FxFalse)
	, _toolbar(FxFalse)
	, _useCustomBackgroundColour(FxFalse)
	, _sendMenuEvent(FxFalse)
	, _enabledBitmap(wxNullIcon)
	, _disabledBitmap(wxNullIcon)
	, _label(wxEmptyString)
{
}

FxDropButton::FxDropButton( wxWindow* parent, wxWindowID id, const wxString& label,
	const wxPoint& pos, const wxSize& size, long style )
	: _mouseOver(FxFalse)
	, _depressed(FxFalse)
	, _value(FxFalse)
	, _toggle(FxFalse)
	, _toolbar(FxFalse)
	, _useCustomBackgroundColour(FxFalse)
	, _sendMenuEvent(FxFalse)
	, _enabledBitmap(wxNullIcon)
	, _disabledBitmap(wxNullIcon)
	, _label(wxEmptyString)
{
	Create( parent, id, label, pos, size, style );
}

bool FxDropButton::Create( wxWindow* parent, wxWindowID id, const wxString& label,
	const wxPoint& pos, const wxSize& size, long style )
{
	wxWindow::Create( parent, id, pos, size, style );
	_label = label;
	wxSysColourChangedEvent temp;
	OnSysColourChanged(temp);

	_customBackgroundColourBrush = _buttonColourBrush;

	return true;
}

void FxDropButton::MakeToggle()
{
	_toggle = FxTrue;
}

FxBool FxDropButton::GetValue()
{
	return _value;
}

void FxDropButton::SetValue( FxBool value )
{
	_depressed = value;
	_value = value;
	Refresh(FALSE);
}

FxBool FxDropButton::Enable( FxBool enable )
{
	FxBool retval = wxWindow::Enable(enable);
	Refresh(FALSE);
	return retval;
}

void FxDropButton::SetEnabledBitmap( const wxIcon& bitmap )
{
	_enabledBitmap = bitmap;
}

void FxDropButton::SetDisabledBitmap( const wxIcon& bitmap )
{
	_disabledBitmap = bitmap;
}

void FxDropButton::SetCustomBackgroundColour( const wxColour& bgColour )
{
	_customBackgroundColourBrush = wxBrush(bgColour, wxSOLID);
}

FxBool FxDropButton::GetUseCustomBackgroundColour( void ) const
{
	return _useCustomBackgroundColour;
}

void FxDropButton::SetUseCustomBackgroundColour( FxBool useCustomBackgroundColour )
{
	_useCustomBackgroundColour = useCustomBackgroundColour;
	if( _useCustomBackgroundColour )
	{
		_buttonColourBrush = _customBackgroundColourBrush;
	}
	else
	{
		_buttonColourBrush = _buttonNormalFaceBrush;
	}
	Refresh(FALSE);
}

void FxDropButton::OnSysColourChanged( wxSysColourChangedEvent& FxUnused(event) )
{
	_buttonNormalFaceBrush    = wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE), wxSOLID);
	_buttonMouseOverFaceBrush = wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT), wxSOLID);
	_buttonDepressedFaceBrush = wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT), wxSOLID);
	_buttonShadowPen          = wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW), 1, wxSOLID);
	_inactiveBorderPen        = wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVEBORDER),1,wxSOLID);
	_buttonTextFont           = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

	if( !_useCustomBackgroundColour )
	{
		_buttonColourBrush = _buttonNormalFaceBrush;
	}
	Refresh(FALSE);
}

void FxDropButton::OnPaint( wxPaintEvent& FxUnused(event) )
{
	wxBufferedPaintDC dc(this);
	wxRect clientRect = GetClientRect();

	FxInt32 left   = clientRect.GetLeft();
	FxInt32 right  = clientRect.GetRight() - 10;
	FxInt32 top    = clientRect.GetTop();
	FxInt32 bottom = clientRect.GetBottom();

	FxInt32 width, height;
	if( GetParent() )
	{
		dc.SetBackground( wxBrush(GetParent()->GetBackgroundColour(), wxSOLID) );
	}
	else
	{
		dc.SetBackground( _buttonNormalFaceBrush );
	}
	dc.Clear();

	if( ((!_toggle && !_mouseOver && !_depressed) || (_toggle && !_mouseOver && !_value)) && IsEnabled() )
	{
		dc.SetBrush(_buttonColourBrush);
	}
	else if( !IsEnabled() )
	{
		dc.SetBrush(_buttonColourBrush);
	}
	else if( (_mouseOver && !_depressed && !_value ) )
	{
		dc.SetBrush(_buttonMouseOverFaceBrush);
	}
	else if( (_depressed && _mouseOver) || (_toggle && _value) )
	{
		dc.SetBrush(_buttonDepressedFaceBrush);
	}
	else
	{
		dc.SetBrush(_buttonColourBrush);
	}

	if( IsEnabled() )
	{
		dc.SetPen(_buttonShadowPen);
	}
	else
	{
		dc.SetPen(_inactiveBorderPen);
	}

	if( (_toolbar && (_depressed||_mouseOver)) || !_toolbar || (_toggle && _value) )
	{
		dc.DrawRoundedRectangle(clientRect, 3);

		if( _mouseOver )
		{
			// Draw the drop section.
			dc.SetPen(_buttonShadowPen);
			dc.DrawLine(clientRect.GetRight() - 10, clientRect.GetTop(),
				clientRect.GetRight() - 10, clientRect.GetBottom());
		}
	}

	if( IsEnabled() )
	{
		dc.SetPen(*wxBLACK_PEN);
		dc.SetBrush(*wxBLACK_BRUSH);
	}
	else
	{
		dc.SetPen(*wxLIGHT_GREY_PEN);
		dc.SetBrush(*wxLIGHT_GREY_BRUSH);
	}
	wxPoint dropGlyph[] = {wxPoint(0,0), wxPoint(6,0), wxPoint(3, 3)};
	dc.DrawPolygon(3, dropGlyph, clientRect.GetRight() - 8, clientRect.GetTop() + 12);

	if( !_enabledBitmap.IsOk() && !_disabledBitmap.IsOk() )
	{
		dc.SetFont(_buttonTextFont);
		dc.GetTextExtent(_label, &width, &height);
		if( IsEnabled() )
		{
			dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
		}
		else
		{
			dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
		}
		dc.DrawText(_label, ((right-left)/2) - (width/2), ((bottom-top)/2) - (height/2));
	}
	else
	{
		if( IsEnabled() )
		{
			dc.DrawIcon( _enabledBitmap, ((right-left)/2) - (_enabledBitmap.GetWidth()/2) + 1, ((bottom-top)/2) - (_enabledBitmap.GetHeight()/2) + 1);
		}
		else
		{
			dc.DrawIcon( _disabledBitmap, ((right-left)/2) - (_disabledBitmap.GetWidth()/2) + 1, ((bottom-top)/2) - (_disabledBitmap.GetHeight()/2) + 1);
		}
	}
}

void FxDropButton::OnMouse( wxMouseEvent& event )
{
	if( event.Entering() )
	{
		_mouseOver = FxTrue;
		Refresh(FALSE);
		Update();
	}
	else if( event.Leaving() )
	{
		_mouseOver = FxFalse;
		Refresh(FALSE);
		Update();
	}
	else if( event.LeftDown() )
	{
		if( !HasCapture() )
		{
			CaptureMouse();
		}
		_depressed = FxTrue;

		Refresh(FALSE);
		Update();
	}
	else if( event.LeftUp() )
	{
		if( HasCapture() )
		{
			ReleaseMouse();
		}
		if( _mouseOver )
		{
			if( event.GetPosition().x <= GetClientRect().GetRight() - 10 )
			{
				if( _sendMenuEvent )
				{
					// Send a menu item selection.
					wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, GetId());
					event.SetEventObject(this);
					GetEventHandler()->ProcessEvent(event);
				}
				else
				{
					// Send a normal button press.
					wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
					event.SetEventObject(this);
					GetEventHandler()->ProcessEvent(event);
				}
			}
			else
			{
				wxCommandEvent event(wxEVT_COMMAND_BUTTON_DROP_CLICKED, GetId());
				event.SetEventObject(this);
				GetEventHandler()->ProcessEvent(event);
			}
		}
		if( _toggle )
		{
			_value = !_value;
		}
		_depressed = FxFalse;

		Refresh(FALSE);
		Update();
	}
}

} // namespace Face

} // namespace OC3Ent

