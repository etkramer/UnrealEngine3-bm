//------------------------------------------------------------------------------
// A wizard for creating new workspaces with given layouts.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxNewWorkspaceWizard.h"

#include "FxFaceGraphNodeGroupManager.h"
#include "FxSessionProxy.h"
#include "FxStudioApp.h"
#include "FxWorkspaceManager.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/dnd.h"
	#include "wx/valgen.h"
	#include "wx/spinctrl.h"
#endif

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// Text drop target specialization for the list controls.
// Adapted from the wxwindows dnd sample.
//------------------------------------------------------------------------------

class FxDnDText : public wxTextDropTarget
{
public:
	FxDnDText(wxListCtrl* pOwner) { _owner = pOwner; }

	virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& text)
	{
		int flags;
		long index = _owner->HitTest(wxPoint(x,y), flags);

		FxInt32 count = 0;
		FxInt32 start = _owner->GetItemCount();
		if( flags & wxLIST_HITTEST_ONITEM )
		{
			start = index;
		}
		wxString rest = text;
		do 
		{
			_owner->InsertItem(start + count, rest.BeforeFirst(wxT('|')));
			rest = rest.AfterFirst(wxT('|'));
			++count;

		} while( !rest.IsEmpty() );

		_owner->Refresh();
		return FxTrue;
	}

private:
	wxListCtrl* _owner;
};

//------------------------------------------------------------------------------
// Select Nodes wizard page
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(FxWizardPageSelectNodes, wxWizardPageSimple)
	EVT_CHOICE(ControlID_NewWorkspaceWizardSelectNodePageNodeGroupChoice, FxWizardPageSelectNodes::OnNodeGroupChoiceChange)
	EVT_LIST_BEGIN_DRAG(ControlID_NewWorkspaceWizardSelectNodePageNodeNameList, FxWizardPageSelectNodes::OnNodeNameListDragStart)
	EVT_BUTTON(ControlID_NewWorkspaceWizardSelectNodePageAddButton, FxWizardPageSelectNodes::OnAddButton)
	EVT_BUTTON(ControlID_NewWorkspaceWizardSelectNodePageRemoveButton, FxWizardPageSelectNodes::OnRemoveButton)
	EVT_LIST_BEGIN_DRAG(ControlID_NewWorkspaceWizardSelectNodePageSliderNameList, FxWizardPageSelectNodes::OnSliderNameListDragStart)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, FxWizardPageSelectNodes::OnWizardPageChanging)
END_EVENT_TABLE()

FxWizardPageSelectNodes::FxWizardPageSelectNodes(wxWizard* parent)
	: wxWizardPageSimple(parent)
{
	CreateControls();
	FillNodeGroupChoice();
}

void FxWizardPageSelectNodes::GetSliderNames(FxArray<FxString>& sliderNames)
{
	sliderNames.Clear();
	if( _sliderNameList )
	{
		sliderNames.Reserve(_sliderNameList->GetItemCount());
		for( FxSize i = 0; i < sliderNames.Allocated(); ++i )
		{
			sliderNames.PushBack(FxString(_sliderNameList->GetItemText(i).mb_str(wxConvLibc)));
		}
	}
}

void FxWizardPageSelectNodes::OnNodeGroupChoiceChange(wxCommandEvent& FxUnused(event))
{
	FillNodeNameList();
}

void FxWizardPageSelectNodes::OnNodeNameListDragStart(wxListEvent& FxUnused(event))
{
	// We need to read all the selected items.
	wxString nodes;
	FxInt32 item = -1;
	for( ;; )
	{
		item = _nodeNameList->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if( item == -1 )
			break;
		wxString nodeName = _nodeNameList->GetItemText(item);
		nodes += nodeName + wxT("|");
	}

	if( nodes.Length() > 0 )
	{
		nodes.RemoveLast();

		// Set the list control up as a drag source.
		wxTextDataObject data(nodes);
		wxDropSource dragSource(data, _nodeNameList);

		//@todo handle the result.
		/*wxDragResult result = dragSource.DoDragDrop(TRUE);*/
		dragSource.DoDragDrop(TRUE);
	}
}

void FxWizardPageSelectNodes::OnAddButton(wxCommandEvent& FxUnused(event))
{
	// Add the selected items to the end of the slider list.
	FxInt32 item = -1;
	for( ;; )
	{
		item = _nodeNameList->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if( item == -1 )
			break;
		_sliderNameList->InsertItem(_sliderNameList->GetItemCount(), _nodeNameList->GetItemText(item));
	}
}

void FxWizardPageSelectNodes::OnRemoveButton(wxCommandEvent& FxUnused(event))
{
	FxInt32 item = -1;
	FxArray<FxInt32> selectedItems(_sliderNameList->GetSelectedItemCount());
	for( ;; )
	{
		item = _sliderNameList->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if( item == -1 )
			break;
		selectedItems.PushBack(item);
	}
	// Remove in reverse order to avoid changing indices of yet-to-be erased elements.
	for( FxInt32 i = selectedItems.Length() - 1; i >= 0; --i )
	{
		_sliderNameList->DeleteItem(selectedItems[i]);
	}
}

void FxWizardPageSelectNodes::OnSliderNameListDragStart(wxListEvent& FxUnused(event))
{
	// We need to read all the selected items.
	wxString nodes;
	FxInt32 item = -1;
	for( ;; )
	{
		item = _sliderNameList->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if( item == -1 )
			break;
		wxString nodeName = _sliderNameList->GetItemText(item);
		nodes += nodeName + wxT("|");
	}

	if( nodes.Length() > 0 )
	{
		nodes.RemoveLast();

		// Set the list control up as a drag source.
		wxTextDataObject data(nodes);
		wxDropSource dragSource(data, _nodeNameList);

		//@todo handle the result.
		/*wxDragResult result = dragSource.DoDragDrop(TRUE);*/
		dragSource.DoDragDrop(TRUE);
		wxCommandEvent fake;
		OnRemoveButton(fake);
	}
}

void FxWizardPageSelectNodes::OnWizardPageChanging(wxWizardEvent& event)
{
	if( _sliderNameList->GetItemCount() == 0 )
	{
		FxMessageBox(_("Please add the names of the nodes you want to control with sliders to the second list box"), _("No Sliders to Create"));
		event.Veto();
	}
}

void FxWizardPageSelectNodes::CreateControls()
{
	SetSizer(new wxBoxSizer(wxVERTICAL));
	wxStaticText* text = new wxStaticText(this, wxID_ANY,
		_("This wizard will create a new workspace with pre-arranged sliders.\n\nFirst, select the nodes you want to control using sliders.\n"));
	GetSizer()->Add(text, 0, wxALL, 5);

	wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* col = new wxBoxSizer(wxVERTICAL);
	_nodeGroupChoice = new wxChoice(this, ControlID_NewWorkspaceWizardSelectNodePageNodeGroupChoice);
	col->Add(_nodeGroupChoice, 0, wxLEFT|wxRIGHT|wxEXPAND, 5);
	_nodeNameList = new wxListCtrl(this, ControlID_NewWorkspaceWizardSelectNodePageNodeNameList,
		wxDefaultPosition, wxSize(150, 213), wxLC_REPORT);
	col->Add(_nodeNameList, 0, wxLEFT|wxRIGHT, 5);

	row->Add(col);

	col = new wxBoxSizer(wxVERTICAL);
	_addButton = new wxButton(this, ControlID_NewWorkspaceWizardSelectNodePageAddButton, _("Add >>"));
	col->Add(_addButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
	_removeButton = new wxButton(this, ControlID_NewWorkspaceWizardSelectNodePageRemoveButton, _("Remove <<"));
	col->Add(_removeButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
	row->Add(col);

	col = new wxBoxSizer(wxVERTICAL);
	col->Add(new wxStaticText(this, wxID_ANY, _("Workspace Sliders")), 0, wxLEFT|wxRIGHT|wxBOTTOM, 5);
	_sliderNameList = new wxListCtrl(this, ControlID_NewWorkspaceWizardSelectNodePageSliderNameList,
		wxDefaultPosition, wxSize(150, 220), wxLC_REPORT);

	wxListItem columnInfo;
	columnInfo.m_mask = wxLIST_MASK_TEXT;
	columnInfo.m_text = _("Slider Name");
	columnInfo.m_width = wxLIST_AUTOSIZE;
	_sliderNameList->InsertColumn(0, columnInfo);

	_sliderNameList->SetDropTarget(new FxDnDText(_sliderNameList));
	col->Add(_sliderNameList, 1, wxLEFT|wxRIGHT|wxEXPAND, 5);
	row->Add(col);

	GetSizer()->Add(row);
}

void FxWizardPageSelectNodes::FillNodeGroupChoice()
{
	if( _nodeGroupChoice )
	{
		_nodeGroupChoice->Freeze();
		_nodeGroupChoice->Clear();
		FxActor* actor = NULL;
		if( FxSessionProxy::GetActor(&actor) )
		{
			_nodeGroupChoice->Append(wxString(FxFaceGraphNodeGroupManager::GetAllNodesGroupName().GetData(), wxConvLibc));
			for( FxSize i = 0; i < FxFaceGraphNodeGroupManager::GetNumGroups(); ++i )
			{
				_nodeGroupChoice->Append(
					wxString(FxFaceGraphNodeGroupManager::GetGroup(i)->GetNameAsCstr(), wxConvLibc));
			}
		}
		_nodeGroupChoice->Thaw();
	}

	SetNodeGroupSelection();
}

void FxWizardPageSelectNodes::SetNodeGroupSelection()
{
	FxActor* actor = NULL;
	if( FxSessionProxy::GetActor(&actor) && _nodeGroupChoice )
	{
		FxString selFaceGraphNodeGroup;
		FxSessionProxy::GetSelectedFaceGraphNodeGroup(selFaceGraphNodeGroup);
		// Select the correct group.
		wxString groupName(selFaceGraphNodeGroup.GetData(), wxConvLibc);
		FxInt32 index = _nodeGroupChoice->FindString(groupName);
		if( index != -1 )
		{
			_nodeGroupChoice->SetSelection(index);
		}
		else
		{
			// Select the all nodes group.
			_nodeGroupChoice->SetStringSelection(wxString(FxFaceGraphNodeGroupManager::GetAllNodesGroupName().GetData(), wxConvLibc));
		}

		// Force the node name list box to be filled out.
		FillNodeNameList();
	}
}

void FxWizardPageSelectNodes::FillNodeNameList()
{
	FxActor* actor = NULL;
	if( FxSessionProxy::GetActor(&actor) && _nodeGroupChoice && _nodeNameList )
	{
		_nodeNameList->Freeze();
		_nodeNameList->ClearAll();

		wxListItem columnInfo;
		columnInfo.m_mask = wxLIST_MASK_TEXT;
		columnInfo.m_text = _("Node Name");
		columnInfo.m_width = wxLIST_AUTOSIZE;
		_nodeNameList->InsertColumn(0, columnInfo);

		wxString nodeGroup = _nodeGroupChoice->GetString(_nodeGroupChoice->GetSelection());
		if( FxFaceGraphNodeGroupManager::GetAllNodesGroupName() == FxString(nodeGroup.mb_str(wxConvLibc)) )
		{
			FxFaceGraph::Iterator fgNodeIter    = actor->GetDecompiledFaceGraph().Begin(FxFaceGraphNode::StaticClass());
			FxFaceGraph::Iterator fgNodeEndIter = actor->GetDecompiledFaceGraph().End(FxFaceGraphNode::StaticClass());
			FxInt32 count = 0;
			for( ; fgNodeIter != fgNodeEndIter; ++fgNodeIter )
			{
				FxFaceGraphNode* pNode = fgNodeIter.GetNode();
				if( pNode )
				{
					_nodeNameList->InsertItem(count, wxString(pNode->GetNameAsCstr(), wxConvLibc));
					count++;
				}
			}
		}
		else
		{
			FxFaceGraphNodeGroup* pGroup = FxFaceGraphNodeGroupManager::GetGroup(FxString(nodeGroup.mb_str(wxConvLibc)).GetData());
			if( pGroup )
			{
				FxInt32 count = 0;
				for( FxSize i = 0; i < pGroup->GetNumNodes(); ++i )
				{
					_nodeNameList->InsertItem(count, wxString(pGroup->GetNode(i).GetAsCstr(), wxConvLibc));
					count++;
				}
			}
		}
		_nodeNameList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
		_nodeNameList->Thaw();
	}
}

//------------------------------------------------------------------------------
// Select Layout wizard page.
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(FxWizardPageSelectLayout, wxWizardPageSimple)
	EVT_BUTTON(ControlID_NewWorkspaceWizardSelectLayoutPageSingleHorizontalButton, FxWizardPageSelectLayout::OnPresetPress)
	EVT_BUTTON(ControlID_NewWorkspaceWizardSelectLayoutPageSingleVerticalButton, FxWizardPageSelectLayout::OnPresetPress)
	EVT_BUTTON(ControlID_NewWorkspaceWizardSelectLayoutPageDoubleHorizontalButton, FxWizardPageSelectLayout::OnPresetPress)
	EVT_BUTTON(ControlID_NewWorkspaceWizardSelectLayoutPageDoubleVerticalButton, FxWizardPageSelectLayout::OnPresetPress)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, FxWizardPageSelectLayout::OnWizardPageChanging)
END_EVENT_TABLE()

FxWizardPageSelectLayout::FxWizardPageSelectLayout(wxWizard* parent)
	: wxWizardPageSimple(parent)
	, _sliderOrientation(0)
	, _numCols(1)
	, _colSpacing(2)
{
	CreateControls();
}

void FxWizardPageSelectLayout::OnPresetPress(wxCommandEvent& event)
{
	switch(event.GetId())
	{
	default:
	case ControlID_NewWorkspaceWizardSelectLayoutPageSingleHorizontalButton:
		_sliderOrientation = 0;
		_numCols = 1;
		break;
	case ControlID_NewWorkspaceWizardSelectLayoutPageSingleVerticalButton:
		_sliderOrientation = 1;
		_numCols = 1;
		break;
	case ControlID_NewWorkspaceWizardSelectLayoutPageDoubleHorizontalButton:
		_sliderOrientation = 0;
		_numCols = 2;
		break;
	case ControlID_NewWorkspaceWizardSelectLayoutPageDoubleVerticalButton:
		_sliderOrientation = 1;
		_numCols = 2;
		break;
	};
	TransferDataToWindow();
}

void FxWizardPageSelectLayout::OnWizardPageChanging(wxWizardEvent& FxUnused(event))
{
	// Record the selection.
	TransferDataFromWindow();
}

void FxWizardPageSelectLayout::CreateControls()
{
	SetSizer(new wxBoxSizer(wxVERTICAL));
	wxStaticText* text = new wxStaticText(this, wxID_ANY,
		_("Now, select what layout you would like the sliders to use.\n\nClick a button to set a pre-defined layout, or make one of your own using the controls below.\n"));
	GetSizer()->Add(text, 0, wxALL, 5);

	// Load the bitmaps.
	wxFileName bmpPath = FxStudioApp::GetAppDirectory();
	bmpPath.AppendDir(wxT("res"));
	bmpPath.AppendDir(wxT("bmp"));

	wxBitmap singleHorizontalBitmap;
	bmpPath.SetName(wxT("SingleColHorizontalSliders.bmp"));
	if( FxFileExists(bmpPath.GetFullPath()) )
	{
		singleHorizontalBitmap.LoadFile(bmpPath.GetFullPath(), wxBITMAP_TYPE_BMP);
	}
	wxBitmap singleVerticalBitmap;
	bmpPath.SetName(wxT("SingleColVerticalSliders.bmp"));
	if( FxFileExists(bmpPath.GetFullPath()) )
	{
		singleVerticalBitmap.LoadFile(bmpPath.GetFullPath(), wxBITMAP_TYPE_BMP);
	}
	wxBitmap doubleHorizontalBitmap;
	bmpPath.SetName(wxT("DoubleColHorizontalSliders.bmp"));
	if( FxFileExists(bmpPath.GetFullPath()) )
	{
		doubleHorizontalBitmap.LoadFile(bmpPath.GetFullPath(), wxBITMAP_TYPE_BMP);
	}
	wxBitmap doubleVerticalBitmap;
	bmpPath.SetName(wxT("DoubleColVerticalSliders.bmp"));
	if( FxFileExists(bmpPath.GetFullPath()) )
	{
		doubleVerticalBitmap.LoadFile(bmpPath.GetFullPath(), wxBITMAP_TYPE_BMP);
	}

	wxStaticBox* presetBox = new wxStaticBox(this, wxID_ANY, _("Preset Layouts"));
	wxStaticBoxSizer* row = new wxStaticBoxSizer(presetBox, wxHORIZONTAL);
	wxBitmapButton* singleHorizontalButton = new wxBitmapButton(this, ControlID_NewWorkspaceWizardSelectLayoutPageSingleHorizontalButton, singleHorizontalBitmap);
	row->Add(singleHorizontalButton, 0, wxALL, 5);
	wxBitmapButton* singleVerticalButton = new wxBitmapButton(this, ControlID_NewWorkspaceWizardSelectLayoutPageSingleVerticalButton, singleVerticalBitmap);
	row->Add(singleVerticalButton, 0, wxALL, 5);
	wxBitmapButton* doubleHorizontalButton = new wxBitmapButton(this, ControlID_NewWorkspaceWizardSelectLayoutPageDoubleHorizontalButton, doubleHorizontalBitmap);
	row->Add(doubleHorizontalButton, 0, wxALL, 5);
	wxBitmapButton* doubleVerticalButton = new wxBitmapButton(this, ControlID_NewWorkspaceWizardSelectLayoutPageDoubleVerticalButton, doubleVerticalBitmap);
	row->Add(doubleVerticalButton, 0, wxALL, 5);
	GetSizer()->Add(row);

	wxStaticBox* customBox = new wxStaticBox(this, wxID_ANY, _("Custom Layout Control"));
	wxStaticBoxSizer* col = new wxStaticBoxSizer(customBox, wxVERTICAL);

	wxString choices[] = {_("Horizontal"), _("Vertical")};
	wxRadioBox* sliderOrientationRadio = new wxRadioBox(this, wxID_ANY, _("Slider Orientation"),
		wxDefaultPosition, wxDefaultSize, 2, choices, 0, wxRA_HORIZONTAL, wxGenericValidator(&_sliderOrientation));
	col->Add(sliderOrientationRadio, 0, wxALL, 5);

	wxBoxSizer* spinrow = new wxBoxSizer(wxHORIZONTAL);
	spinrow->Add(new wxStaticText(this, wxID_ANY, _("Number of rows or cols:")), 0, wxLEFT|wxTOP|wxBOTTOM, 5);
	wxSpinCtrl* spinctrl = new wxSpinCtrl(this, wxID_ANY, _("1"), wxDefaultPosition, wxSize(40,20), wxSP_ARROW_KEYS, 1, 10, 1);
	spinctrl->SetValidator(wxGenericValidator(&_numCols));
	spinrow->Add(spinctrl, 0, wxALL, 5);
	col->Add(spinrow);

	wxBoxSizer* colspacingRow = new wxBoxSizer(wxHORIZONTAL);
	colspacingRow->Add(new wxStaticText(this, wxID_ANY, _("Column spacing (% of workspace):")), 0, wxLEFT|wxTOP|wxBOTTOM, 5);
	spinctrl = new wxSpinCtrl(this, wxID_ANY, _("2"), wxDefaultPosition, wxSize(40,20), wxSP_ARROW_KEYS, 1, 10, 2);
	spinctrl->SetValidator(wxGenericValidator(&_colSpacing));
	colspacingRow->Add(spinctrl, 0, wxALL, 5);
	col->Add(colspacingRow);

	GetSizer()->Add(col);
	TransferDataToWindow();
}

//------------------------------------------------------------------------------
// Workspace Options wizard page
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(FxWizardPageWorkspaceOptions, wxWizardPageSimple)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, FxWizardPageWorkspaceOptions::OnWizardPageChanging)
END_EVENT_TABLE()

FxWizardPageWorkspaceOptions::FxWizardPageWorkspaceOptions(wxWizard* parent)
	: wxWizardPageSimple(parent)
	, _workspaceName(_("New Workspace"))
{
	CreateControls();
}

void FxWizardPageWorkspaceOptions::OnWizardPageChanging(wxWizardEvent& event)
{
	TransferDataFromWindow();

	for( FxSize i = 0; i < FxWorkspaceManager::Instance()->GetNumWorkspaces(); ++i )
	{
		wxString workspaceName(FxWorkspaceManager::Instance()->GetWorkspace(i).GetNameAsCstr(), wxConvLibc);
		if( workspaceName == _workspaceName )
		{
			FxMessageBox(_("A workspace with that name already exists.  Please choose another name."), _("Duplicate name"));
			event.Veto();
		}
	}
}

void FxWizardPageWorkspaceOptions::CreateControls()
{
	SetSizer(new wxBoxSizer(wxVERTICAL));
	wxStaticText* text = new wxStaticText(this, wxID_ANY,
		_("Finally, name the workspace.\n"));
	GetSizer()->Add(text, 0, wxALL, 5);

	wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(new wxStaticText(this, wxID_ANY, _("Workspace name:")), 0, wxALL, 5);
	row->Add(
		new wxTextCtrl(this, wxID_ANY, _("New Workspace"), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&_workspaceName)),
		0, wxALL, 5);
	GetSizer()->Add(row);

	TransferDataToWindow();
}

} // namespace Face

} // namespace OC3Ent
