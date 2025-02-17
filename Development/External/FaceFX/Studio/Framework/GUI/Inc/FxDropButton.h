//------------------------------------------------------------------------------
// An simple owner-drawn button with a dropdown button paired to it.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxDropButton_H__
#define FxDropButton_H__

BEGIN_DECLARE_EVENT_TYPES()
	DECLARE_LOCAL_EVENT_TYPE(wxEVT_COMMAND_BUTTON_DROP_CLICKED, -1)
END_DECLARE_EVENT_TYPES()

namespace OC3Ent
{

namespace Face
{

class FxDropButton : public wxWindow
{
	WX_DECLARE_DYNAMIC_CLASS(FxDropButton)
	DECLARE_EVENT_TABLE()
public:
	// Constructors
	FxDropButton();
	FxDropButton( wxWindow* parent, wxWindowID id, const wxString& label = wxEmptyString,
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, 
		long style = 0 );

	bool Create( wxWindow* parent, wxWindowID id, const wxString& label = wxEmptyString,
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, 
		long style = 0 );

	// Sets the toolbar style.
	void MakeToolbar(FxBool sendMenuEvent = FxFalse ) { _toolbar = FxTrue; _sendMenuEvent = sendMenuEvent; }

	void MakeToggle();
	FxBool GetValue();
	void SetValue( FxBool value );
	FxBool Enable( FxBool enable = true );

	void SetEnabledBitmap( const wxIcon& bitmap );
	void SetDisabledBitmap( const wxIcon& bitmap );

	void SetCustomBackgroundColour( const wxColour& bgColour );
	FxBool GetUseCustomBackgroundColour( void ) const;
	void SetUseCustomBackgroundColour( FxBool useCustomBackgroundColour );

protected:
	// Colour change event.
	void OnSysColourChanged( wxSysColourChangedEvent& event );
	// Paint handler.
	void OnPaint( wxPaintEvent& event );
	// Mouse handler.
	void OnMouse( wxMouseEvent& event );

	wxBrush  _buttonNormalFaceBrush;
	wxBrush  _buttonColourBrush;
	wxBrush  _customBackgroundColourBrush;
	wxBrush  _buttonMouseOverFaceBrush;
	wxBrush  _buttonDepressedFaceBrush;
	wxPen	 _buttonShadowPen;
	wxPen    _inactiveBorderPen;
	wxFont	 _buttonTextFont;

	FxBool   _mouseOver;
	FxBool   _depressed;
	FxBool   _value;
	FxBool   _toggle;
	FxBool	 _toolbar;
	FxBool   _useCustomBackgroundColour;
	FxBool   _sendMenuEvent;

	wxIcon	 _enabledBitmap;
	wxIcon   _disabledBitmap;
	wxString _label;
};

} // namespace Face

} // namespace OC3Ent

#endif
