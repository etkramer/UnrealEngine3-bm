//------------------------------------------------------------------------------
// The curve properties dialog.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxCurvePropertiesDialog.h"
#include "FxColourMapping.h"
#include "FxTearoffWindow.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/colordlg.h"
#endif

namespace OC3Ent
{

namespace Face
{

WX_IMPLEMENT_DYNAMIC_CLASS(FxCurvePropertiesDialog, wxDialog)

BEGIN_EVENT_TABLE(FxCurvePropertiesDialog,wxDialog)
	EVT_BUTTON(ControlID_CurvePropertiesDlgChangeColourButton, FxCurvePropertiesDialog::OnButtonColourChange)
	EVT_CHECKBOX(ControlID_CurvePropertiesDlgOwnedByAnalysisCheck, FxCurvePropertiesDialog::OnCheckLockState)
END_EVENT_TABLE()

FxCurvePropertiesDialog::FxCurvePropertiesDialog()
{
}

FxCurvePropertiesDialog::
FxCurvePropertiesDialog( wxWindow* parent, wxWindowID id, 
						 const wxString& caption, const wxPoint& pos, 
					     const wxSize& size, long style )
{
	Create(parent, id, caption, pos, size, style);
}

FxBool FxCurvePropertiesDialog::
Create( wxWindow* parent, wxWindowID id, const wxString& caption, 
	    const wxPoint& pos, const wxSize& size, long style )
{
	SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
	SetExtraStyle(GetExtraStyle()|wxWS_EX_VALIDATE_RECURSIVELY);
	wxDialog::Create(parent, id, caption, pos, size, style);

	CreateControls();

	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();
	return TRUE;
}

// Creates the controls and sizers.
void FxCurvePropertiesDialog::CreateControls( void )
{
	wxBoxSizer* boxSizerV = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(boxSizerV);
	this->SetAutoLayout(TRUE);

	wxStaticText* label = new wxStaticText(this, -1, _("Curve Colour"));
	boxSizerV->Add(label, 0, wxALIGN_LEFT|wxALL, 5);

	// Create the color section.
	wxBoxSizer* smallHoriz = new wxBoxSizer(wxHORIZONTAL);
	boxSizerV->Add(smallHoriz, 0, wxALIGN_LEFT|wxALL, 0);

	wxBitmap previewBmp(wxNullBitmap);
	_colourPreview = new wxStaticBitmap(this, ControlID_CurvePropertiesDlgColourPreviewStaticBmp,
		previewBmp, wxDefaultPosition, wxSize(92, 20), wxBORDER_SIMPLE);
	smallHoriz->Add(_colourPreview, 0, wxALIGN_LEFT|wxALL, 5);
	_colourPreview->SetBackgroundColour(_curveColour);
	wxButton* changeColourBtn = new wxButton(this, ControlID_CurvePropertiesDlgChangeColourButton,
		_("Change..."));
	smallHoriz->Add(changeColourBtn, 0, wxALIGN_LEFT|wxALL, 5);

	// Create the is locked section.
	_lockedCheck = new wxCheckBox(this, ControlID_CurvePropertiesDlgOwnedByAnalysisCheck, _("Owned by Analysis"));
	boxSizerV->Add(_lockedCheck, 0, wxALIGN_LEFT|wxALL, 5);

	// Create the buttons at the bottom.
	wxBoxSizer* boxSizerH = new wxBoxSizer(wxHORIZONTAL);
	boxSizerV->Add(boxSizerH, 0, wxALIGN_LEFT|wxALL, 5);

	wxButton* okButton = new wxButton(this, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0);
	okButton->SetDefault();
	boxSizerH->Add(okButton, 0, wxALIGN_RIGHT|wxALL, 5);
	wxButton* cancelButton = new wxButton(this, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0);
	boxSizerH->Add(cancelButton, 0, wxALIGN_RIGHT|wxALL, 5);

	Refresh(FALSE);
}

FxBool FxCurvePropertiesDialog::ShowToolTips( void )
{
	return FxTrue;
}

void FxCurvePropertiesDialog::SetCurveColour( const wxColour& colour )
{
	_curveColour = colour;
	_colourPreview->SetBackgroundColour(_curveColour);
	Refresh(FALSE);
}

wxColour FxCurvePropertiesDialog::GetCurveColour( void ) const
{
	return _curveColour;
}

void FxCurvePropertiesDialog::SetIsCurveOwnedByAnalysis( FxBool isLocked )
{
	_isOwnedByAnalysis = isLocked;
	_lockedCheck->SetValue(_isOwnedByAnalysis);
	Refresh(FALSE);
}

FxBool FxCurvePropertiesDialog::GetIsCurveOwnedByAnalysis( void ) const
{
	return _isOwnedByAnalysis;
}

// Called when the user pressed the Change... button for the curve colour.
void FxCurvePropertiesDialog::OnButtonColourChange( wxCommandEvent& FxUnused(event) )
{
	wxColourDialog colorPickerDialog(this);
	colorPickerDialog.GetColourData().SetChooseFull(true);
	colorPickerDialog.GetColourData().SetColour(_curveColour);
	// Remember the user's custom colors.
	for( FxSize i = 0; i < FxColourMap::GetNumCustomColours(); ++i )
	{
		colorPickerDialog.GetColourData().SetCustomColour(i, FxColourMap::GetCustomColours()[i]);
	}
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	if( colorPickerDialog.ShowModal() == wxID_OK )
	{
		_curveColour = colorPickerDialog.GetColourData().GetColour();
	}
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
	for( FxSize i = 0; i < FxColourMap::GetNumCustomColours(); ++i )
	{
		FxColourMap::GetCustomColours()[i] = colorPickerDialog.GetColourData().GetCustomColour(i);
	}
	_colourPreview->SetBackgroundColour(_curveColour);
	Refresh(FALSE);
}

// Called when the user toggles the lock state check box
void FxCurvePropertiesDialog::OnCheckLockState( wxCommandEvent& FxUnused(event) )
{
	_isOwnedByAnalysis = _lockedCheck->GetValue();
}

} // namespace Face

} // namespace OC3Ent
