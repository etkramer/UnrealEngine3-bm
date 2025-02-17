//------------------------------------------------------------------------------
// Allows a user to choose a name for a curve.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxAnimCurveNameSelDialog.h"

namespace OC3Ent
{

namespace Face
{

BEGIN_EVENT_TABLE(FxAnimCurveNameSelectionDlg,wxDialog)
	EVT_CHOICE(ControlID_AnimCurveNameSelDlgNodeGroupChoice, FxAnimCurveNameSelectionDlg::OnFilterSelChanged)
	EVT_BUTTON(wxID_OK, FxAnimCurveNameSelectionDlg::OnOK)
END_EVENT_TABLE()

FxAnimCurveNameSelectionDlg::FxAnimCurveNameSelectionDlg( wxWindow* parent, FxFaceGraph* faceGraph )
	: wxDialog(parent, -1, wxString(wxT("Select Node to Drive, or Type New Curve Name")), 
	wxDefaultPosition, wxSize(350, 135), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
	, selectedTargetName(FxName::NullName)
	, _filterChoice(NULL)
	, _targetNameCombo(NULL)
	, _faceGraph(faceGraph)
	, _nodeFilter(FxFaceGraphNode::StaticClass())
{
	SetSizer(new wxBoxSizer(wxVERTICAL));
	_filterChoice = new wxChoice(this, ControlID_AnimCurveNameSelDlgNodeGroupChoice);
	GetSizer()->Add(_filterChoice, 0, wxALL|wxEXPAND, 5);
	_targetNameCombo = new wxComboBox(this, ControlID_AnimCruveNameSelDlgTargetNameCombo);
	GetSizer()->Add(_targetNameCombo, 0, wxALL|wxEXPAND, 5);

	wxSizer* buttonSizer = CreateButtonSizer(wxOK|wxCANCEL);
	GetSizer()->Add(buttonSizer, 0, wxALL|wxEXPAND, 5);

	if( faceGraph )
	{
		_filterChoice->Append(_("All"));
		_filterChoice->Append(_("All: Root only"));
		for( FxSize i = 0; i < faceGraph->GetNumNodeTypes(); ++i )
		{
			wxString nodeName = wxString::FromAscii(
				faceGraph->GetNodeType(i)->GetName().GetAsString().GetData());
			_filterChoice->Append(nodeName);
			_filterChoice->Append(nodeName + _(": Root only"));
		}
		SetSelection();
	}
	else
	{
		_filterChoice->Enable(FALSE);
	}
}

void FxAnimCurveNameSelectionDlg::InitDialog()
{
	wxDialog::InitDialog();
	_targetNameCombo->SetFocus();
}

// Called when the filter selection changes.
void FxAnimCurveNameSelectionDlg::OnFilterSelChanged( wxCommandEvent& event )
{
	if( _faceGraph && _filterChoice && _targetNameCombo )
	{
		_targetNameCombo->Clear();
		wxString nodeFilter = _filterChoice->GetString(_filterChoice->GetSelection());
		FxBool rootOnly = nodeFilter.Find(wxT(':')) != -1;
		nodeFilter = nodeFilter.BeforeFirst(wxT(':'));
		if( wxT("All") == nodeFilter )
		{
			nodeFilter = wxT("FxFaceGraphNode");
		}
		FxString nodeTypeString(nodeFilter.mb_str(wxConvLibc));
		_nodeFilter = FxClass::FindClassDesc(nodeTypeString.GetData());
		if( _nodeFilter )
		{
			FxFaceGraph::Iterator fgNodeIter    = _faceGraph->Begin(_nodeFilter);
			FxFaceGraph::Iterator fgNodeEndIter = _faceGraph->End(_nodeFilter);
			for( ; fgNodeIter != fgNodeEndIter; ++fgNodeIter )
			{
				FxFaceGraphNode* pNode = fgNodeIter.GetNode();
				if( pNode && ((rootOnly && pNode->GetNumInputLinks() == 0) || !rootOnly) )
				{
					_targetNameCombo->Append(wxString(pNode->GetNameAsCstr(), wxConvLibc));
				}
			}
		}
	}
	event.Skip();
} 

// Called when the user presses OK.
void FxAnimCurveNameSelectionDlg::OnOK( wxCommandEvent& event )
{
	wxString selection = _targetNameCombo->GetValue();
	selectedTargetName = FxName(FxString(selection.mb_str(wxConvLibc)).GetData());
	event.Skip();
}

// Initialize the selection
void FxAnimCurveNameSelectionDlg::SetSelection( void )
{
	if( _faceGraph && _filterChoice && _nodeFilter )
	{
		if( _nodeFilter == FxFaceGraphNode::StaticClass() )
		{
			// The "All" filter.
			_filterChoice->SetSelection(
				_filterChoice->FindString(_("All")));
		}
		else
		{
			// Some other filter.
			_filterChoice->SetSelection(
				_filterChoice->FindString(
				wxString(_nodeFilter->GetName().GetAsString().GetData(), wxConvLibc)));
		}

		// Force the list box to be filled out.
		wxCommandEvent temp;
		OnFilterSelChanged(temp);
	}
}

} // namespace Face

} // namespace OC3Ent
