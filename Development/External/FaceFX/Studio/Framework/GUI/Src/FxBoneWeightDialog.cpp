//------------------------------------------------------------------------------
// A dialog for modifying bone weights.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxBoneWeightDialog.h"
#include "FxGridTools.h"

namespace OC3Ent
{

namespace Face
{

WX_IMPLEMENT_DYNAMIC_CLASS(FxBoneWeightDialog, wxDialog)

BEGIN_EVENT_TABLE(FxBoneWeightDialog, wxDialog)
	EVT_NAVIGATION_KEY(FxBoneWeightDialog::OnNavKey)
	EVT_BUTTON(ControlID_BoneWeightDlgOKButton, FxBoneWeightDialog::OnOK)
END_EVENT_TABLE()

FxBoneWeightDialog::FxBoneWeightDialog( wxWindow* parent, wxString title )
	: wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(300,500), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
	, _weightGrid(NULL)
{
	// Create the controls.
	SetSizer(new wxBoxSizer(wxVERTICAL));
	_weightGrid = new wxGrid(this, ControlID_BoneWeightDlgWeightGrid );
	wxButton* okButton = new wxButton(this, ControlID_BoneWeightDlgOKButton, _("OK"));
	wxButton* cancelButton = new wxButton(this, wxID_CANCEL, _("Cancel"));

	// Arrange the controls.
	GetSizer()->Add(_weightGrid, 1, wxGROW|wxALL, 5);
	wxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);
	buttonRow->Add(okButton, 0, wxALL, 5);
	buttonRow->Add(cancelButton, 0, wxALL, 5);
	GetSizer()->Add(buttonRow);
}

void FxBoneWeightDialog::SetInitialBoneWeights( const FxArray<FxAnimBoneWeight>& boneWeights )
{
	_weightGrid->CreateGrid(boneWeights.Length(), 2);
	_weightGrid->SetRowLabelSize(0);
	_weightGrid->SetColLabelValue(0, _("Bone Name"));
	_weightGrid->SetColLabelValue(1, _("Weight"));
	for( FxSize i = 0; i < boneWeights.Length(); ++i )
	{
		_weightGrid->SetCellValue(
			wxString::FromAscii(boneWeights[i].boneName.GetAsCstr()),
			i, 0);
		_weightGrid->SetReadOnly(i, 0);
		_weightGrid->SetCellValue(
			wxString::Format(wxT("%f"), boneWeights[i].boneWeight),
			i, 1);
		_weightGrid->SetCellRenderer(i, 1, new FxGridCellFloatRenderer(-1, 2));
		_weightGrid->SetCellEditor(i, 1, new wxGridCellFloatEditor(-1, 2));
	}
	_weightGrid->AutoSizeColumns();
}

FxArray<FxAnimBoneWeight> FxBoneWeightDialog::GetFinalBoneWeights()
{
	FxArray<FxAnimBoneWeight> retval;
	retval.Reserve(_weightGrid->GetNumberRows());
	for( FxInt32 i = 0; i < _weightGrid->GetNumberRows(); ++i )
	{
		FxAnimBoneWeight curr;
		curr.boneName = FxName(FxString(_weightGrid->GetCellValue(i, 0).mb_str(wxConvLibc)).GetData());
		curr.boneWeight = FxAtof(_weightGrid->GetCellValue(i, 1).mb_str(wxConvLibc));
		retval.PushBack(curr);
	}
	return retval;
}

// Called when the user presses a navigation key.
void FxBoneWeightDialog::OnNavKey( wxNavigationKeyEvent& event )
{
	if( _weightGrid->IsCellEditControlShown() )
	{
		_weightGrid->SaveEditControlValue();
		_weightGrid->HideCellEditControl();
	}
	event.Skip();
}

void FxBoneWeightDialog::OnOK( wxCommandEvent& FxUnused(event) )
{
	//@todo process the return values.

	EndDialog(wxID_OK);
}

} // namespace Face

} // namespace OC3Ent

