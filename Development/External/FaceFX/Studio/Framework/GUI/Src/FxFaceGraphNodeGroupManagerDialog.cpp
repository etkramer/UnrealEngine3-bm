//------------------------------------------------------------------------------
// Face Graph node group manager dialog.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxFaceGraphNodeGroupManagerDialog.h"
#include "FxUtilityFunctions.h"
#include "FxFaceGraphNodeGroupManager.h"
#include "FxVM.h"

namespace OC3Ent
{

namespace Face
{

WX_IMPLEMENT_DYNAMIC_CLASS(FxFaceGraphNodeGroupManagerDialog, wxDialog)

BEGIN_EVENT_TABLE(FxFaceGraphNodeGroupManagerDialog, wxDialog)
	EVT_CHOICE(ControlID_FaceGraphNodeGroupManagerDlgNodeTypeFilterChoice, FxFaceGraphNodeGroupManagerDialog::OnFaceGraphNodeFilterSelChanged)
	EVT_LIST_COL_CLICK(ControlID_FaceGraphNodeGroupManagerDlgFaceGraphNodesListCtrl, FxFaceGraphNodeGroupManagerDialog::OnFaceGraphNodeColumnClick)
	EVT_BUTTON(ControlID_FaceGraphNodeGroupManagerDlgAddToGroupButton, FxFaceGraphNodeGroupManagerDialog::OnAddToGroup)

	EVT_CHOICE(ControlID_FaceGraphNodeGroupManagerDlgGroupChoice, FxFaceGraphNodeGroupManagerDialog::OnGroupSelChanged)
	EVT_BUTTON(ControlID_FaceGraphNodeGroupManagerDlgNewGroupButton, FxFaceGraphNodeGroupManagerDialog::OnNewGroup)
	EVT_BUTTON(ControlID_FaceGraphNodeGroupManagerDlgDeleteGroupButton, FxFaceGraphNodeGroupManagerDialog::OnDeleteGroup)
	EVT_LIST_COL_CLICK(ControlID_FaceGraphNodeGroupManagerDlgGroupNodesListCtrl, FxFaceGraphNodeGroupManagerDialog::OnGroupFaceGraphNodeColumnClick)
	EVT_BUTTON(ControlID_FaceGraphNodeGroupManagerDlgRemoveFromGroupButton, FxFaceGraphNodeGroupManagerDialog::OnRemoveFromGroup)

	EVT_LIST_ITEM_SELECTED(ControlID_FaceGraphNodeGroupManagerDlgFaceGraphNodesListCtrl, FxFaceGraphNodeGroupManagerDialog::OnSelectionChanged)
	EVT_LIST_ITEM_DESELECTED(ControlID_FaceGraphNodeGroupManagerDlgFaceGraphNodesListCtrl, FxFaceGraphNodeGroupManagerDialog::OnSelectionChanged)
	EVT_LIST_ITEM_SELECTED(ControlID_FaceGraphNodeGroupManagerDlgGroupNodesListCtrl, FxFaceGraphNodeGroupManagerDialog::OnSelectionChanged)
	EVT_LIST_ITEM_DESELECTED(ControlID_FaceGraphNodeGroupManagerDlgGroupNodesListCtrl, FxFaceGraphNodeGroupManagerDialog::OnSelectionChanged)

	EVT_BUTTON(ControlID_FaceGraphNodeGroupManagerDlgOKButton, FxFaceGraphNodeGroupManagerDialog::OnOK)
	EVT_CLOSE(FxFaceGraphNodeGroupManagerDialog::OnClose)
END_EVENT_TABLE()

FxFaceGraphNodeGroupManagerDialog::FxFaceGraphNodeGroupManagerDialog()
{
}

FxFaceGraphNodeGroupManagerDialog::
FxFaceGraphNodeGroupManagerDialog( wxWindow* parent, wxWindowID id, 
	const wxString& caption, const wxPoint& pos, 
	const wxSize& size, long style )
{
	Create(parent, id, caption, pos, size, style);
}

FxBool FxFaceGraphNodeGroupManagerDialog::
Create( wxWindow* parent, wxWindowID id, const wxString& caption, 
	const wxPoint& pos, const wxSize& size, long style )
{
	_actor					= NULL;
	_mediator				= NULL;
	_filterDropdown         = NULL;
	_faceGraphNodesListCtrl = NULL;
	_addToGroupButton       = NULL;
	_groupDropdown          = NULL;
	_newGroupButton         = NULL;
	_deleteGroupButton      = NULL;
	_nodesInGroupListCtrl   = NULL;
	_removeFromGroupButton  = NULL;
	_okButton               = NULL;
	
	SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
	SetExtraStyle(GetExtraStyle()|wxWS_EX_VALIDATE_RECURSIVELY);
	wxDialog::Create(parent, id, caption, pos, size, style);

	CreateControls();
	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();
	return TRUE;
}

void FxFaceGraphNodeGroupManagerDialog::
Initialize( FxActor* actor, FxWidgetMediator* mediator )
{
	_actor    = actor;
	_mediator = mediator;

	if( _actor )
	{
		_filterDropdown->Enable(TRUE);
		_faceGraphNodesListCtrl->Enable(TRUE);
		_addToGroupButton->Enable(TRUE);
		_groupDropdown->Enable(TRUE);
		_newGroupButton->Enable(TRUE);
		_deleteGroupButton->Enable(TRUE);
		_nodesInGroupListCtrl->Enable(TRUE);
		_removeFromGroupButton->Enable(TRUE);

		_filterDropdown->Append(wxT("All"));
		for( FxSize i = 0; i < _actor->GetDecompiledFaceGraph().GetNumNodeTypes(); ++i )
		{
			_filterDropdown->Append(
				wxString(_actor->GetDecompiledFaceGraph().GetNodeType(i)->GetName().GetAsString().GetData(), wxConvLibc));
		}
        _filterDropdown->SetSelection(0);
		// Force the list control to be filled out.
		wxCommandEvent temp;
		OnFaceGraphNodeFilterSelChanged(temp);

		FxSize numGroups = FxFaceGraphNodeGroupManager::GetNumGroups();
		if( 0 == numGroups )
		{
			_groupDropdown->Append(wxT("<No Groups>"));
		}
		else
		{
			for( FxSize i = 0; i < numGroups; ++i )
			{
				FxFaceGraphNodeGroup* pGroup = FxFaceGraphNodeGroupManager::GetGroup(i);
				if( pGroup )
				{
					_groupDropdown->Append(wxString(pGroup->GetNameAsCstr(), wxConvLibc));
				}
			}
		}
		_groupDropdown->SetSelection(0);
		// Force the list control to be filled out.
		OnGroupSelChanged(temp);
	}
}

void FxFaceGraphNodeGroupManagerDialog::CreateControls( void )
{
	wxBoxSizer* boxSizerV = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(boxSizerV);
	this->SetAutoLayout(TRUE);

	wxBoxSizer* boxSizerH = new wxBoxSizer(wxHORIZONTAL);
	boxSizerV->Add(boxSizerH, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	wxStaticBox* faceGraphLabel = new wxStaticBox(this, wxID_ANY, _("Face Graph"));
	wxStaticBoxSizer* faceGraphStaticBoxSizer = new wxStaticBoxSizer(faceGraphLabel, wxVERTICAL);
	boxSizerH->Add(faceGraphStaticBoxSizer, 0, wxGROW|wxALL, 5);

	wxBoxSizer* faceGraphBoxSizerV = new wxBoxSizer(wxVERTICAL);
	faceGraphStaticBoxSizer->Add(faceGraphBoxSizerV, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	wxStaticText* filterLabel = new wxStaticText(this, wxID_STATIC, _("Node Type Filter"), wxDefaultPosition, wxDefaultSize, 0);
	faceGraphBoxSizerV->Add(filterLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

	_filterDropdown = new wxChoice(this, ControlID_FaceGraphNodeGroupManagerDlgNodeTypeFilterChoice, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	_filterDropdown->SetToolTip(_("Filter nodes in the Face Graph based on node type."));
	faceGraphBoxSizerV->Add(_filterDropdown, 0, wxGROW|wxALL, 5);

	wxBoxSizer* faceGraphBoxSizerH = new wxBoxSizer(wxHORIZONTAL);
	faceGraphBoxSizerV->Add(faceGraphBoxSizerH, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	// Spacers.
	faceGraphBoxSizerH->Add(75, 25, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	faceGraphBoxSizerH->Add(75, 25, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* faceGraphNodesLabel = new wxStaticText(this, wxID_STATIC, _("Face Graph Nodes"), wxDefaultPosition, wxDefaultSize, 0);
	faceGraphBoxSizerV->Add(faceGraphNodesLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

	_faceGraphNodesListCtrl = new wxListCtrl(this, ControlID_FaceGraphNodeGroupManagerDlgFaceGraphNodesListCtrl, wxDefaultPosition, wxSize(225, 300), wxLC_REPORT);
	_faceGraphNodesListCtrl->SetToolTip(_("Nodes in the Face Graph."));
	faceGraphBoxSizerV->Add(_faceGraphNodesListCtrl, 0, wxGROW|wxALL, 5);

	_addToGroupButton = new wxButton(this, ControlID_FaceGraphNodeGroupManagerDlgAddToGroupButton, _("Add to Current Group"), wxDefaultPosition, wxDefaultSize, 0);
	_addToGroupButton->SetToolTip(_("Add the selected nodes to the selected group."));
	faceGraphBoxSizerV->Add(_addToGroupButton, 0, wxGROW|wxALL, 5);

	wxStaticBox* nodeGroupsLabel = new wxStaticBox(this, wxID_ANY, _("Node Groups"));
	wxStaticBoxSizer* nodeGroupsStaticBoxSizerV = new wxStaticBoxSizer(nodeGroupsLabel, wxVERTICAL);
	boxSizerH->Add(nodeGroupsStaticBoxSizerV, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxBoxSizer* nodeGroupsBoxSizerV = new wxBoxSizer(wxVERTICAL);
	nodeGroupsStaticBoxSizerV->Add(nodeGroupsBoxSizerV, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	wxStaticText* groupLabel = new wxStaticText(this, wxID_STATIC, _("Group"), wxDefaultPosition, wxDefaultSize, 0);
	nodeGroupsBoxSizerV->Add(groupLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

	_groupDropdown = new wxChoice(this, ControlID_FaceGraphNodeGroupManagerDlgGroupChoice, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	_groupDropdown->SetToolTip(_("Current node group."));
	nodeGroupsBoxSizerV->Add(_groupDropdown, 0, wxGROW|wxALL, 5);

	wxBoxSizer* nodeGroupsBoxSizerH = new wxBoxSizer(wxHORIZONTAL);
	nodeGroupsBoxSizerV->Add(nodeGroupsBoxSizerH, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	_newGroupButton = new wxButton(this, ControlID_FaceGraphNodeGroupManagerDlgNewGroupButton, _("New Group..."), wxDefaultPosition, wxDefaultSize, 0);
	_newGroupButton->SetToolTip(_("Create a new node group."));
	nodeGroupsBoxSizerH->Add(_newGroupButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	_deleteGroupButton = new wxButton(this, ControlID_FaceGraphNodeGroupManagerDlgDeleteGroupButton, _("Delete Group"), wxDefaultPosition, wxDefaultSize, 0);
	_deleteGroupButton->SetToolTip(_("Delete the selected node group."));
	nodeGroupsBoxSizerH->Add(_deleteGroupButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* nodesInGroupLabel = new wxStaticText(this, wxID_STATIC, _("Nodes In Group"), wxDefaultPosition, wxDefaultSize, 0);
	nodeGroupsBoxSizerV->Add(nodesInGroupLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

	_nodesInGroupListCtrl = new wxListCtrl(this, ControlID_FaceGraphNodeGroupManagerDlgGroupNodesListCtrl, wxDefaultPosition, wxSize(225, 300), wxLC_REPORT);
	_nodesInGroupListCtrl->SetToolTip(_("Nodes in the current group."));
	nodeGroupsBoxSizerV->Add(_nodesInGroupListCtrl, 0, wxGROW|wxALL, 5);

	_removeFromGroupButton = new wxButton(this, ControlID_FaceGraphNodeGroupManagerDlgRemoveFromGroupButton, _("Remove From Group"), wxDefaultPosition, wxDefaultSize, 0);
	_removeFromGroupButton->SetToolTip(_("Remove the selected nodes from the selected group."));
	nodeGroupsBoxSizerV->Add(_removeFromGroupButton, 0, wxGROW|wxALL, 5);

	_okButton = new wxButton(this, ControlID_FaceGraphNodeGroupManagerDlgOKButton, _("OK"), wxDefaultPosition, wxDefaultSize, 0);
	_okButton->SetToolTip(_("Dismiss the dialog."));
	boxSizerV->Add(_okButton, 0, wxALIGN_RIGHT|wxALL, 5);

	// Create an invisible cancel button.
	// This will probably land me in programmer's hell at some point.
	// I just want escape to make the dumb dialog go away.
	wxButton* cancelButton = new wxButton(this, wxID_CANCEL, _("Cancel"), wxPoint(0,0), wxSize(0,0));
	cancelButton;

	_filterDropdown->Enable(FALSE);
	_faceGraphNodesListCtrl->Enable(FALSE);
	_addToGroupButton->Enable(FALSE);
	_groupDropdown->Enable(FALSE);
	_newGroupButton->Enable(FALSE);
	_deleteGroupButton->Enable(FALSE);
	_nodesInGroupListCtrl->Enable(FALSE);
	_removeFromGroupButton->Enable(FALSE);
}

FxBool FxFaceGraphNodeGroupManagerDialog::ShowToolTips( void )
{
	return FxTrue;
}

void FxFaceGraphNodeGroupManagerDialog::
OnFaceGraphNodeFilterSelChanged( wxCommandEvent& event )
{
	if( _actor && _filterDropdown && _faceGraphNodesListCtrl )
	{
		_faceGraphNodesListCtrl->ClearAll();
		wxListItem columnInfo;
		columnInfo.m_mask = wxLIST_MASK_TEXT|wxLIST_MASK_WIDTH;
		columnInfo.m_text = _("Node Name");
		columnInfo.m_width = GetClientRect().width;
		_faceGraphNodesListCtrl->InsertColumn(0, columnInfo);

		wxString nodeFilter = _filterDropdown->GetString(_filterDropdown->GetSelection());
		if( wxT("All") == nodeFilter )
		{
			nodeFilter = wxT("FxFaceGraphNode");
		}
		FxString nodeTypeString(nodeFilter.mb_str(wxConvLibc));
		const FxClass* pNodeFilter = FxClass::FindClassDesc(nodeTypeString.GetData());
		if( pNodeFilter )
		{
			FxFaceGraph::Iterator fgNodeIter    = _actor->GetDecompiledFaceGraph().Begin(pNodeFilter);
			FxFaceGraph::Iterator fgNodeEndIter = _actor->GetDecompiledFaceGraph().End(pNodeFilter);
			FxInt32 count = 0;
			for( ; fgNodeIter != fgNodeEndIter; ++fgNodeIter )
			{
				FxFaceGraphNode* pNode = fgNodeIter.GetNode();
				if( pNode )
				{
					_faceGraphNodesListCtrl->InsertItem(count, wxString(pNode->GetNameAsCstr(), wxConvLibc));
					count++;
				}
			}
		}
		CheckControlState();
	}
	event.Skip();
}

void FxFaceGraphNodeGroupManagerDialog::
OnFaceGraphNodeColumnClick( wxListEvent& FxUnused(event) )
{
	static FxBool sortAscending = FxTrue;

	if( _actor && _faceGraphNodesListCtrl )
	{
		FxArray<FxString*> itemStrings;
		// Tag the nodes.
		FxInt32 index = -1;
		for( ;; )
		{
			index = _faceGraphNodesListCtrl->GetNextItem(index);
			if( index == -1 ) break;
			FxString* itemString = new FxString(_faceGraphNodesListCtrl->GetItemText(index).mb_str(wxConvLibc));
			itemStrings.PushBack(itemString);
			_faceGraphNodesListCtrl->SetItemData(index, reinterpret_cast<long>(itemString));
		}
		_faceGraphNodesListCtrl->SortItems(CompareStrings, static_cast<long>(sortAscending));
		for( FxSize i = 0; i < itemStrings.Length(); ++i )
		{
			delete itemStrings[i];
			itemStrings[i] = NULL;
		}
		itemStrings.Clear();
		sortAscending = !sortAscending;
	}
}

void FxFaceGraphNodeGroupManagerDialog::OnAddToGroup( wxCommandEvent& FxUnused(event) )
{
	if( _groupDropdown )
	{
		wxString groupName = _groupDropdown->GetString(_groupDropdown->GetSelection());
		if( groupName != wxString(wxT("<No Groups>")) )
		{
			FxFaceGraphNodeGroup* pGroup = FxFaceGraphNodeGroupManager::GetGroup(FxString(groupName.mb_str(wxConvLibc)).GetData());
			wxString nodes = wxEmptyString;
			if( pGroup && _faceGraphNodesListCtrl )
			{
				FxInt32 item = -1;
				for( ;; )
				{
					item = _faceGraphNodesListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
					if( item == -1 )
						break;
					wxString nodeName = _faceGraphNodesListCtrl->GetItemText(item);
					nodes += nodeName + wxT("|");
				}
				if( nodes.Length() > 0 )
				{
					nodes.RemoveLast();
					wxString command = wxString::Format(wxT("nodeGroup -group \"%s\" -edit -add -nodes \"%s\""),
						groupName, nodes);
					FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));

					// Force the list control to be filled out.
					wxCommandEvent temp;
					OnGroupSelChanged(temp);
				}
			}
		}
	}
}

void FxFaceGraphNodeGroupManagerDialog::
OnGroupSelChanged( wxCommandEvent& event )
{
	if( _actor && _groupDropdown && _nodesInGroupListCtrl )
	{
		_nodesInGroupListCtrl->ClearAll();
		wxListItem columnInfo;
		columnInfo.m_mask = wxLIST_MASK_TEXT|wxLIST_MASK_WIDTH;
		columnInfo.m_text = _("Node Name");
		columnInfo.m_width = GetClientRect().width;
		_nodesInGroupListCtrl->InsertColumn(0, columnInfo);

		wxString groupName = _groupDropdown->GetString(_groupDropdown->GetSelection());
		if( groupName == wxString(wxT("<No Groups>")) )
		{
			if( _addToGroupButton && _removeFromGroupButton && _deleteGroupButton )
			{
				_addToGroupButton->Enable(FALSE);
				_removeFromGroupButton->Enable(FALSE);
				_deleteGroupButton->Enable(FALSE);
			}
		}
		else
		{
			if( _addToGroupButton && _removeFromGroupButton && _deleteGroupButton )
			{
				_addToGroupButton->Enable(TRUE);
				_removeFromGroupButton->Enable(TRUE);
				_deleteGroupButton->Enable(TRUE);
			}
			FxFaceGraphNodeGroup* pGroup = FxFaceGraphNodeGroupManager::GetGroup(FxString(groupName.mb_str(wxConvLibc)).GetData());
			if( pGroup )
			{
				FxInt32 count = 0;
				for( FxSize i = 0; i < pGroup->GetNumNodes(); ++i )
				{
					const FxName& node = pGroup->GetNode(i);
					_nodesInGroupListCtrl->InsertItem(count, wxString(node.GetAsCstr(), wxConvLibc));
					count++;
				}
			}
		}
	}
	CheckControlState();
	event.Skip();
}

void FxFaceGraphNodeGroupManagerDialog::OnNewGroup( wxCommandEvent& FxUnused(event) )
{
	if( _groupDropdown )
	{
		wxString groupName = ::wxGetTextFromUser(_("Enter new group name:"), _("Create Group"), wxEmptyString, this);
		if( groupName != wxEmptyString )
		{
			wxString command = wxString::Format(wxT("nodeGroup -create -group \"%s\""), groupName);
			FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));

			_groupDropdown->Clear();
			FxSize numGroups = FxFaceGraphNodeGroupManager::GetNumGroups();
			for( FxSize i = 0; i < numGroups; ++i )
			{
				FxFaceGraphNodeGroup* pGroup = FxFaceGraphNodeGroupManager::GetGroup(i);
				if( pGroup )
				{
					_groupDropdown->Append(wxString(pGroup->GetNameAsCstr(), wxConvLibc));
				}
			}
			_groupDropdown->SetStringSelection(groupName);
			// Force the list control to be filled out.
			wxCommandEvent temp;
			OnGroupSelChanged(temp);
		}
	}
	CheckControlState();
}

void FxFaceGraphNodeGroupManagerDialog::OnDeleteGroup( wxCommandEvent& FxUnused(event) )
{
	if( _groupDropdown )
	{
		wxString groupName = _groupDropdown->GetString(_groupDropdown->GetSelection());
		if( groupName != wxString(wxT("<No Groups>")) )
		{
			wxString command = wxString::Format(wxT("nodeGroup -edit -remove -group \"%s\""), groupName);
			FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));

			_groupDropdown->Clear();
			FxSize numGroups = FxFaceGraphNodeGroupManager::GetNumGroups();
			if( 0 == numGroups )
			{
				_groupDropdown->Append(wxT("<No Groups>"));
			}
			else
			{
				for( FxSize i = 0; i < numGroups; ++i )
				{
					FxFaceGraphNodeGroup* pGroup = FxFaceGraphNodeGroupManager::GetGroup(i);
					if( pGroup )
					{
						_groupDropdown->Append(wxString(pGroup->GetNameAsCstr(), wxConvLibc));
					}
				}
			}
			_groupDropdown->SetSelection(0);
			// Force the list control to be filled out.
			wxCommandEvent temp;
			OnGroupSelChanged(temp);
		}
	}
	CheckControlState();
}

void FxFaceGraphNodeGroupManagerDialog::
OnGroupFaceGraphNodeColumnClick( wxListEvent& FxUnused(event) )
{
	static FxBool sortAscending = FxTrue;

	if( _actor && _nodesInGroupListCtrl )
	{
		FxArray<FxString*> itemStrings;
		// Tag the nodes.
		FxInt32 index = -1;
		for( ;; )
		{
			index = _nodesInGroupListCtrl->GetNextItem(index);
			if( index == -1 ) break;
			FxString* itemString = new FxString(_faceGraphNodesListCtrl->GetItemText(index).mb_str(wxConvLibc));
			itemStrings.PushBack(itemString);
			_nodesInGroupListCtrl->SetItemData(index, reinterpret_cast<long>(itemString));
		}
		_nodesInGroupListCtrl->SortItems(CompareStrings, static_cast<long>(sortAscending));
		for( FxSize i = 0; i < itemStrings.Length(); ++i )
		{
			delete itemStrings[i];
			itemStrings[i] = NULL;
		}
		itemStrings.Clear();
		sortAscending = !sortAscending;
	}
}

void FxFaceGraphNodeGroupManagerDialog::OnRemoveFromGroup( wxCommandEvent& FxUnused(event) )
{
	if( _groupDropdown )
	{
		wxString groupName = _groupDropdown->GetString(_groupDropdown->GetSelection());
		if( groupName != wxString(wxT("<No Groups>")) )
		{
			FxFaceGraphNodeGroup* pGroup = FxFaceGraphNodeGroupManager::GetGroup(FxString(groupName.mb_str(wxConvLibc)).GetData());
			if( pGroup && _nodesInGroupListCtrl )
			{
				FxInt32 item = -1;
				wxString nodes = wxEmptyString;
				for( ;; )
				{
					item = _nodesInGroupListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
					if( item == -1 )
						break;
					wxString nodeName = _nodesInGroupListCtrl->GetItemText(item);
					nodes += nodeName + wxT("|");
				}
				if( nodes.Length() > 0 )
				{
					wxString command = wxString::Format(wxT("nodeGroup -group \"%s\" -edit -remove -nodes \"%s\""),
						groupName, nodes);
					FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));

					// Force the list control to be filled out.
					wxCommandEvent temp;
					OnGroupSelChanged(temp);
				}
			}
		}
	}
	CheckControlState();
}

void FxFaceGraphNodeGroupManagerDialog::OnOK( wxCommandEvent& event )
{
	Close();
	event.Skip();
}

void FxFaceGraphNodeGroupManagerDialog::OnClose( wxCloseEvent& event )
{
	if( _mediator )
	{
		_mediator->OnActorInternalDataChanged(NULL, ADCF_NodeGroupsChanged);
	}
	event.Skip();
}

void FxFaceGraphNodeGroupManagerDialog::OnSelectionChanged( wxListEvent& FxUnused(event) )
{
	CheckControlState();
}

void FxFaceGraphNodeGroupManagerDialog::CheckControlState( void )
{
	if( _faceGraphNodesListCtrl && _addToGroupButton )
	{
		_addToGroupButton->Enable( _faceGraphNodesListCtrl->GetSelectedItemCount() > 0 );
	}
	if( _nodesInGroupListCtrl && _removeFromGroupButton )
	{
		_removeFromGroupButton->Enable( _nodesInGroupListCtrl->GetSelectedItemCount() > 0 );
	}
}

} // namespace Face

} // namespace OC3Ent
