//------------------------------------------------------------------------------
// The curve properties dialog.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCurvePropertiesDialog_H__
#define FxCurvePropertiesDialog_H__

namespace OC3Ent
{

namespace Face
{

// The curve properties dialog.
class FxCurvePropertiesDialog : public wxDialog
{
	WX_DECLARE_DYNAMIC_CLASS(FxCurvePropertiesDialog)
	DECLARE_EVENT_TABLE()

public:
	// Constructors.
	FxCurvePropertiesDialog();
	FxCurvePropertiesDialog( wxWindow* parent, wxWindowID id = 10005, 
		const wxString& caption = _("Curve Properties"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(400,300), 
		long style = wxDEFAULT_DIALOG_STYLE );

	// Creation.
	FxBool Create( wxWindow* parent, wxWindowID id = 10005, 
		const wxString& caption = _("Curve Properties"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(400,300), 
		long style = wxDEFAULT_DIALOG_STYLE );

	// Creates the controls and sizers.
	void CreateControls( void );

	// Should we show tool tips?
	static FxBool ShowToolTips( void );

	void SetCurveColour( const wxColour& colour );
	wxColour GetCurveColour( void ) const;

	void SetIsCurveOwnedByAnalysis( FxBool isOwnedByAnalysis );
	FxBool GetIsCurveOwnedByAnalysis( void ) const;

protected:
	// Called when the user pressed the Change.. button for the curve colour.
	void OnButtonColourChange( wxCommandEvent& event );
	// Called when the user toggles the lock state check box.
	void OnCheckLockState( wxCommandEvent& event );

	wxColour _curveColour;
	FxBool   _isOwnedByAnalysis;

private:
	wxButton*         _applyButton;
	wxStaticBitmap*   _colourPreview;
	wxCheckBox*       _lockedCheck;
};

} // namespace Face

} // namespace OC3Ent

#endif
