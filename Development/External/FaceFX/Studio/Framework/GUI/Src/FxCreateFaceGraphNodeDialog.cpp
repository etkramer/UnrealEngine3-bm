//------------------------------------------------------------------------------
// A face graph node creation dialog.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxCreateFaceGraphNodeDialog.h"
#include "FxFaceGraphNodePropertiesWidget.h"
#include "FxVM.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

WX_IMPLEMENT_DYNAMIC_CLASS(FxCreateFaceGraphNodeDialog, wxDialog)

BEGIN_EVENT_TABLE(FxCreateFaceGraphNodeDialog, wxDialog)
	EVT_BUTTON(ControlID_CreateFaceGraphNodeDlgCreateButton, FxCreateFaceGraphNodeDialog::OnCreateButtonPressed)
	EVT_BUTTON(ControlID_CreateFaceGraphNodeDlgCancelButton, FxCreateFaceGraphNodeDialog::OnCancelButtonPressed)
	EVT_CLOSE(FxCreateFaceGraphNodeDialog::OnClose)
END_EVENT_TABLE()

FxCreateFaceGraphNodeDialog::FxCreateFaceGraphNodeDialog()
	: _actor(NULL)
	, _node(NULL)
	, _mediator(NULL)
{
}

FxBool FxCreateFaceGraphNodeDialog::
Create( wxWindow* parent, wxWindowID id, const wxString& caption, 
	const wxPoint& pos, const wxSize& size, long style )
{
	SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
	wxDialog::Create(parent, id, caption, pos, size, style);

	CreateControls();
	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();
	return TRUE;
}

void FxCreateFaceGraphNodeDialog::CreateControls( void )
{
	wxBoxSizer* boxSizerV = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(boxSizerV);
	this->SetAutoLayout(TRUE);

	wxBoxSizer* propertiesSizer = new wxBoxSizer(wxHORIZONTAL);
	boxSizerV->Add(propertiesSizer, 1, wxALIGN_CENTER_VERTICAL|wxGROW, 5);

	_nodePropertiesWidget = new FxNodePropertiesWidget(this, _mediator, -1, wxDefaultPosition, wxSize(400,300));

	_nodePropertiesWidget->OnActorChanged(NULL, _actor);
	FxArray<FxName> selFaceGraphNodeNames;
	if( _node )
	{
		selFaceGraphNodeNames.PushBack(_node->GetName());
	}
	_nodePropertiesWidget->OnFaceGraphNodeSelChanged(NULL, selFaceGraphNodeNames);

	propertiesSizer->Add(_nodePropertiesWidget, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM|wxGROW, 5);

	_nodeLinkPropertiesWidget = 
		new FxNodeLinkPropertiesWidget(this, FxNodeLinkPropertiesWidget::NS_OutputLinks, _mediator, -1, wxDefaultPosition, wxSize(400,300));

	_nodeLinkPropertiesWidget->OnActorChanged(NULL, _actor);
	_nodeLinkPropertiesWidget->OnFaceGraphNodeSelChanged(NULL, selFaceGraphNodeNames);

	propertiesSizer->Add(_nodeLinkPropertiesWidget, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxBOTTOM|wxGROW, 5);

	wxBoxSizer* bottomButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	boxSizerV->Add(bottomButtonSizer, 0, wxALIGN_RIGHT, 5);

	_createButton = new wxButton(this, ControlID_CreateFaceGraphNodeDlgCreateButton, _("Create"), wxDefaultPosition, wxDefaultSize);
	bottomButtonSizer->Add(_createButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	_cancelButton = new wxButton(this, ControlID_CreateFaceGraphNodeDlgCancelButton, _("Cancel"), wxDefaultPosition, wxDefaultSize);
	bottomButtonSizer->Add(_cancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

void FxCreateFaceGraphNodeDialog::
Initialize( FxActor* pActor, FxFaceGraphNode* pNode, FxWidgetMediator* pMediator )
{
	_actor    = pActor;
	_node     = pNode;
	_mediator = pMediator;
}

FxBool FxCreateFaceGraphNodeDialog::ShowToolTips( void )
{
	return FxTrue;
}

void FxCreateFaceGraphNodeDialog::_removeNode( void )
{
	if( _actor && _node )
	{
		wxString command = wxString::Format(wxT("graph -removenode -name \"%s\""),
			wxString::FromAscii(_node->GetNameAsCstr()) );
		FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
	}
}

void FxCreateFaceGraphNodeDialog::OnCreateButtonPressed( wxCommandEvent& FxUnused(event) )
{
	// Similarly, I bet some users will type a value in one of the boxes and
	// not bother to save the value into the control.  Do that for them.
	if( _nodePropertiesWidget->IsCellEditControlShown() )
	{
		_nodePropertiesWidget->SaveEditControlValue();
		_nodePropertiesWidget->HideCellEditControl();
	}

	FxSessionProxy::DisableAllRegisters();

	EndDialog(wxID_OK);
}

void FxCreateFaceGraphNodeDialog::OnCancelButtonPressed( wxCommandEvent& FxUnused(event) )
{
	_removeNode();
	EndDialog(wxID_CANCEL);
}

void FxCreateFaceGraphNodeDialog::OnClose( wxCloseEvent& FxUnused(event) )
{
	_removeNode();
	EndDialog(wxID_CANCEL);
}

} // namespace Face

} // namespace OC3Ent
