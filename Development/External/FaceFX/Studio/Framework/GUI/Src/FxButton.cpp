//------------------------------------------------------------------------------
// An simple owner-drawn button/
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxButton.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/dcbuffer.h"
#endif

namespace OC3Ent
{

namespace Face
{

WX_IMPLEMENT_DYNAMIC_CLASS(FxButton,wxWindow)

BEGIN_EVENT_TABLE(FxButton,wxWindow)
	EVT_SYS_COLOUR_CHANGED(FxButton::OnSysColourChanged)
	EVT_PAINT(FxButton::OnPaint)
	EVT_MOUSE_EVENTS(FxButton::OnMouse)
END_EVENT_TABLE()

FxButton::FxButton()
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

FxButton::FxButton( wxWindow* parent, wxWindowID id, const wxString& label,
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

bool FxButton::Create( wxWindow* parent, wxWindowID id, const wxString& label,
	const wxPoint& pos, const wxSize& size, long style )
{
	wxWindow::Create( parent, id, pos, size, style );
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	_label = label;
	wxSysColourChangedEvent temp;
	OnSysColourChanged(temp);

	_customBackgroundColourBrush = _buttonColourBrush;

	return true;
}

void FxButton::MakeToggle()
{
	_toggle = FxTrue;
}

FxBool FxButton::GetToggleValue()
{
	return _value;
}

void FxButton::SetValue( FxBool value )
{
	_depressed = value;
	_value = value;
	Refresh(FALSE);
}

FxBool FxButton::Enable( FxBool enable )
{
	FxBool retval = wxWindow::Enable(enable);
	Refresh(FALSE);
	return retval;
}

void FxButton::SetEnabledBitmap( const wxIcon& bitmap )
{
	_enabledBitmap = bitmap;
}

void FxButton::SetDisabledBitmap( const wxIcon& bitmap )
{
	_disabledBitmap = bitmap;
}

void FxButton::SetCustomBackgroundColour( const wxColour& bgColour )
{
	_customBackgroundColourBrush = wxBrush(bgColour, wxSOLID);
}

FxBool FxButton::GetUseCustomBackgroundColour( void ) const
{
	return _useCustomBackgroundColour;
}

void FxButton::SetUseCustomBackgroundColour( FxBool useCustomBackgroundColour )
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

void FxButton::OnSysColourChanged( wxSysColourChangedEvent& FxUnused(event) )
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

void FxButton::OnPaint( wxPaintEvent& FxUnused(event) )
{
	wxBufferedPaintDC dc(this);
	wxRect clientRect = GetClientRect();

	FxInt32 left   = clientRect.GetLeft();
	FxInt32 right  = clientRect.GetRight();
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
		dc.DrawRoundedRectangle(clientRect, 1);
		//dc.GradientFillConcentric(clientRect.Deflate(1,1), _buttonColourBrush.GetColour(), dc.GetBrush().GetColour());
	}
	
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

void FxButton::OnMouse( wxMouseEvent& event )
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
			if( _toggle )
			{
				_value = !_value;
			}
			_depressed = FxFalse;

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

		Refresh(FALSE);
		Update();
	}
}

} // namespace Face

} // namespace OC3Ent
