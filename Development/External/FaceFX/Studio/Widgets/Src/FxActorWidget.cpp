//------------------------------------------------------------------------------
// A widget to view data in an FxActor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxActorWidget.h"
#include "FxAnimUserData.h"
#include "FxColourMapping.h"
#include "FxStudioApp.h"
#include "FxUtilityFunctions.h"
#include "FxFaceGraphNodeGroupManager.h"
#include "FxCurveManagerDialog.h"
#include "FxCurvePropertiesDialog.h"
#include "FxVM.h"
#include "FxTearoffWindow.h"
#include "FxAnimController.h"
#include "FxSessionProxy.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/fileconf.h"
#endif


namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// The bone view.
//------------------------------------------------------------------------------

WX_IMPLEMENT_CLASS(FxActorBoneView, FxExpandableWindow)

BEGIN_EVENT_TABLE(FxActorBoneView, FxExpandableWindow)
	EVT_LIST_COL_CLICK(ControlID_ActorWidgetBoneViewListBox, FxActorBoneView::OnColumnClick )
END_EVENT_TABLE()

FxActorBoneView::FxActorBoneView( wxWindow* parent, FxWidgetMediator* mediator )
	: Super(parent)
	, FxWidget(mediator)
	, _actor(NULL)
	, _actorName(NULL)
	, _boneListBox(NULL)
{
	SetName(_("Actor Info"));
}

FxActorBoneView::~FxActorBoneView()
{
}

void FxActorBoneView::CreateChildren( void )
{
	wxString actorName;
	if( _actor )
	{
		actorName = wxString::FromAscii(_actor->GetNameAsCstr());
	}
	else
	{
		actorName = _("No actor loaded");
	}
	_actorName = new wxStaticText(this, wxID_DEFAULT, _("Name: ")+actorName,
		wxPoint(kChildIndent,kCollapsedHeight),
		wxSize(GetClientRect().width-5-kChildIndent,15));
	_children.PushBack(_actorName);
	wxStaticText* boneListLabel = new wxStaticText(this, wxID_DEFAULT, _("Bones in Reference Pose"),
		wxPoint(kChildIndent,kCollapsedHeight+20),
		wxSize( GetClientRect().width - 5 - kChildIndent, 15) );
	_children.PushBack(boneListLabel);
	_boneListBox = new wxListCtrl(this, ControlID_ActorWidgetBoneViewListBox,
		wxPoint(kChildIndent,kCollapsedHeight+35),
		wxSize(GetClientRect().width - 5 - kChildIndent,200), wxLC_REPORT );
	_children.PushBack(_boneListBox);

	if( _actor )
	{
		wxListItem columnInfo;
		columnInfo.m_mask = wxLIST_MASK_TEXT|wxLIST_MASK_WIDTH;
		columnInfo.m_text = _("Bone Name");
		columnInfo.m_width = GetClientRect().width;
		_boneListBox->InsertColumn(0, columnInfo);

		for( FxSize i = 0; i < _actor->GetMasterBoneList().GetNumRefBones(); ++i )
		{
			_boneListBox->InsertItem(i, wxString(_actor->GetMasterBoneList().GetRefBone(i).GetNameAsCstr(), wxConvLibc));
		}
	}
	else
	{
		_boneListBox->Enable(FALSE);
	}
}

void FxActorBoneView::DestroyChildren( void )
{
	// Destroy the windows.
	Super::DestroyChildren();
	_boneListBox = NULL;
}

void FxActorBoneView::OnActorChanged( FxWidget* FxUnused(sender), FxActor* actor )
{
	_actor = actor;
	if( _isExpanded )
	{
		DestroyChildren();
		CreateChildren();
		_container->PositionExapandableWindows();
	}
}

void FxActorBoneView::OnActorInternalDataChanged( FxWidget* FxUnused(sender), FxActorDataChangedFlag FxUnused(flags) )
{
}

void FxActorBoneView::OnActorRenamed( FxWidget* FxUnused(sender) )
{
	if( _isExpanded && _actor )
	{
		wxString currentName = _actorName->GetLabel().AfterFirst(wxT(':'));
		currentName = currentName.AfterFirst(wxT(' '));
		wxString newName(_actor->GetNameAsCstr(), wxConvLibc);
		if( currentName != newName )
		{
			_actorName->SetLabel(_("Name: ") + newName);
			Refresh(FALSE);
		}
	}
}

void FxActorBoneView::OnColumnClick( wxListEvent& FxUnused(event) )
{
	static FxBool sortAscending = FxTrue;

	if( _actor )
	{
		// Tag the bones.
		FxInt32 index = -1;
		for( ;; )
		{
			index = _boneListBox->GetNextItem(index);

			if( index == -1 ) break;

			FxName boneName = FxString(_boneListBox->GetItemText(index).mb_str(wxConvLibc)).GetData();
			FxSize boneIndex = FxInvalidIndex;
			for( FxSize bone = 0; bone < _actor->GetMasterBoneList().GetNumRefBones(); ++bone )
			{
				if( boneName == _actor->GetMasterBoneList().GetRefBone(bone).GetName() )
				{
					boneIndex = bone;
				}
			}

			_boneListBox->SetItemData(index, reinterpret_cast<long>(&_actor->GetMasterBoneList().GetRefBone(boneIndex).GetName()));
		}

		_boneListBox->SortItems(CompareNames, static_cast<long>(sortAscending));
		sortAscending = !sortAscending;
	}
}

//------------------------------------------------------------------------------
// The face graph view.
//------------------------------------------------------------------------------
#define kNoPropagateMessage 5676

WX_IMPLEMENT_CLASS(FxActorFaceGraphView, FxExpandableWindow)

BEGIN_EVENT_TABLE(FxActorFaceGraphView, FxExpandableWindow)
	EVT_CHOICE(ControlID_ActorWidgetFaceGraphViewNodeFilterCombo, FxActorFaceGraphView::OnFaceGraphNodeGroupChoiceSelChanged)
	EVT_LIST_ITEM_FOCUSED(ControlID_ActorWidgetFaceGraphViewNodeListBox, FxActorFaceGraphView::OnFaceGraphNodeListBoxSelChanged)
	EVT_COMMAND_SCROLL(ControlID_ActorWidgetFaceGraphViewQuickPreviewSlider, FxActorFaceGraphView::OnPreviewSliderMoved)
	EVT_LIST_COL_CLICK(ControlID_ActorWidgetFaceGraphViewNodeListBox, FxActorFaceGraphView::OnColumnClick)
END_EVENT_TABLE()

FxActorFaceGraphView::FxActorFaceGraphView( wxWindow* parent, FxWidgetMediator* mediator )
	: Super(parent)
	, FxWidget(mediator)
	, _actor(NULL)
	, _faceGraphNodeGroupDropdown(NULL)
	, _faceGraphNodeListBox(NULL)
	, _quickPreviewSlider(NULL)
	, _noDispatchSelection(FxFalse)
{
	SetName(_("Face Graph"));
}

FxActorFaceGraphView::~FxActorFaceGraphView()
{
}

// Handles right clicks on the quick preview slider.
class FxQuickPreviewEvtHandler : public wxEvtHandler
{
	DECLARE_EVENT_TABLE()
	typedef wxEvtHandler Super;
public:
	FxQuickPreviewEvtHandler(FxActorFaceGraphView* pFGView)
		: _fgView(pFGView)
	{
		// Bind the right click event to FxQuickPreviewEvtHandler::OnRightDown.
		//Connect(wxID_ANY, wxEVT_RIGHT_DOWN, (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxMouseEventFunction, FxQuickPreviewEvtHandler::OnRightDown));
	}

protected:
	void OnRightDown( wxMouseEvent& FxUnused(event) )
	{
		// Notify the face graph view to reset the slider.
		if( _fgView )
		{
			_fgView->ResetQuickPreviewSlider();
		}
	}

	FxActorFaceGraphView* _fgView;
};

BEGIN_EVENT_TABLE(FxQuickPreviewEvtHandler, wxEvtHandler)
	EVT_RIGHT_DOWN(FxQuickPreviewEvtHandler::OnRightDown)
END_EVENT_TABLE()


void FxActorFaceGraphView::CreateChildren( void )
{
	wxStaticText* nodeFilterLabel = new wxStaticText(this, wxID_DEFAULT, _("Node Group"),
		wxPoint(kChildIndent, kCollapsedHeight),
		wxSize(GetClientRect().width - 5 - kChildIndent,15));
	_children.PushBack(nodeFilterLabel);
	_faceGraphNodeGroupDropdown = new wxChoice(this, ControlID_ActorWidgetFaceGraphViewNodeFilterCombo, 
		wxPoint(kChildIndent,kCollapsedHeight+15),
		wxSize(GetClientRect().width - 5 - kChildIndent,20));
	_children.PushBack(_faceGraphNodeGroupDropdown);

	wxStaticText* nodeListLabel = new wxStaticText(this, wxID_DEFAULT, _("Face Graph Nodes"),
		wxPoint(kChildIndent,kCollapsedHeight+40),
		wxSize(GetClientRect().width - 5 - kChildIndent,15));
	_children.PushBack(nodeListLabel);
	_faceGraphNodeListBox = new wxListCtrl(this, ControlID_ActorWidgetFaceGraphViewNodeListBox,
		wxPoint(kChildIndent,kCollapsedHeight+55),
		wxSize(GetClientRect().width - 5 - kChildIndent,300), wxLC_REPORT);
	_children.PushBack(_faceGraphNodeListBox);

	_quickPreviewSlider = new FxSlider(this, ControlID_ActorWidgetFaceGraphViewQuickPreviewSlider, 0, 0, 100,
		wxPoint(kChildIndent, kCollapsedHeight+360),
		wxSize(GetClientRect().width - 5 - kChildIndent, 20), 0);
	_quickPreviewSlider->Enable(FxFalse);
	_quickPreviewSlider->PushEventHandler(new FxQuickPreviewEvtHandler(this));
	_children.PushBack(_quickPreviewSlider);
	
	if( _actor )
	{
		_faceGraphNodeGroupDropdown->Append(wxString(FxFaceGraphNodeGroupManager::GetAllNodesGroupName().GetData(), wxConvLibc));
		for( FxSize i = 0; i < FxFaceGraphNodeGroupManager::GetNumGroups(); ++i )
		{
			_faceGraphNodeGroupDropdown->Append(
				wxString(FxFaceGraphNodeGroupManager::GetGroup(i)->GetNameAsCstr(), wxConvLibc));
		}
		_setNodeGroupAndNodeSelection();
		_setNodeListSelection();
	}
	else
	{
		_faceGraphNodeGroupDropdown->Enable(FALSE);
		_faceGraphNodeListBox->Enable(FALSE);
	}
}

void FxActorFaceGraphView::DestroyChildren( void )
{
	// Destroy the windows.
	Super::DestroyChildren();
	_faceGraphNodeGroupDropdown = NULL;
	_faceGraphNodeListBox = NULL;
	_quickPreviewSlider = NULL;
}

void FxActorFaceGraphView::OnActorChanged( FxWidget* FxUnused(sender), FxActor* actor )
{
	_actor = actor;
	if( _isExpanded )
	{
		DestroyChildren();
		CreateChildren();
		_container->PositionExapandableWindows();
	}
}

void FxActorFaceGraphView::OnFaceGraphNodeGroupSelChanged( FxWidget* FxUnused(sender), const FxName& selGroup )
{
	_selFaceGraphNodeGroup = selGroup;
	if( _isExpanded )
	{
		// Select the new group.
		wxString groupName(_selFaceGraphNodeGroup.GetAsCstr(), wxConvLibc);
		FxInt32 index = _faceGraphNodeGroupDropdown->FindString(groupName);
		if( index != -1 )
		{
			_faceGraphNodeGroupDropdown->SetSelection(index);
			// Force a refresh of the node list.
			wxCommandEvent event;
			event.SetClientData(reinterpret_cast<void*>(kNoPropagateMessage));
			OnFaceGraphNodeGroupChoiceSelChanged(event);
		}
	}
}

void FxActorFaceGraphView::OnActorInternalDataChanged( FxWidget* FxUnused(sender), FxActorDataChangedFlag flags )
{
	FxBool shouldSet = FxFalse;
	if( _isExpanded && flags & ADCF_FaceGraphChanged )
	{
		shouldSet = FxTrue;
	}
	if( _faceGraphNodeGroupDropdown && flags & ADCF_NodeGroupsChanged )
	{
		_faceGraphNodeGroupDropdown->Freeze();
		_faceGraphNodeGroupDropdown->Clear();
		_faceGraphNodeGroupDropdown->Append(wxString(FxFaceGraphNodeGroupManager::GetAllNodesGroupName().GetData(), wxConvLibc));
		for( FxSize i = 0; i < FxFaceGraphNodeGroupManager::GetNumGroups(); ++i )
		{
			_faceGraphNodeGroupDropdown->Append(
				wxString(FxFaceGraphNodeGroupManager::GetGroup(i)->GetNameAsCstr(), wxConvLibc));
		}
		_faceGraphNodeGroupDropdown->Thaw();
		shouldSet = FxTrue;
	}

	if( shouldSet )
	{
		_setNodeGroupAndNodeSelection();
		_setNodeListSelection();
	}
}

void FxActorFaceGraphView::OnFaceGraphNodeSelChanged(FxWidget* FxUnused(sender), const FxArray<FxName>& selFaceGraphNodeNames, FxBool FxUnused(zoomToSelection))
{
	_selFaceGraphNodes = selFaceGraphNodeNames;
	if( _faceGraphNodeListBox )
	{
		// Select the correct face graph nodes.
		for( FxSize i = 0; i < _selFaceGraphNodes.Length(); ++i )
		{
			FxInt32 item = _faceGraphNodeListBox->FindItem(-1, wxString::FromAscii(_selFaceGraphNodes[i].GetAsCstr()));
			if( item == -1 )
			{
				_setNodeGroupAndNodeSelection();
				break;
			}
		}
		_setNodeListSelection();
	}

	if( _quickPreviewSlider )
	{
		_quickPreviewSlider->Enable(_selFaceGraphNodes.Length() == 1);
		if( _actor && _quickPreviewSlider->IsEnabled() )
		{
			_linkedNode = _actor->GetDecompiledFaceGraph().FindNode(_selFaceGraphNodes.Front());
			if( _linkedNode )
			{
				_quickPreviewSlider->SetRange(_linkedNode->GetMin() * 100, _linkedNode->GetMax() * 100);
				FxReal currentValue = 0.0f;
				FxSessionProxy::GetRegister(_linkedNode->GetName(), currentValue);
				_quickPreviewSlider->SetValue(FxClamp(_linkedNode->GetMin() * 100, currentValue * 100, _linkedNode->GetMax() * 100));
				_quickPreviewSlider->SetToolTip(wxString(_("Test node's effect: ")) + wxString::FromAscii(_linkedNode->GetNameAsCstr()));
			}
		}
		else
		{
			_linkedNode = NULL;
		}

		Refresh(FALSE);
	}
}

void FxActorFaceGraphView::ResetQuickPreviewSlider( void )
{
	if( _linkedNode )
	{
		FxReal val = 0.f;
		val = FxClamp(_linkedNode->GetMin(), val, _linkedNode->GetMax()) * 100.f;
		// Reset the slider.
		_quickPreviewSlider->SetValue(val);
		// Update the face graph.
		wxScrollEvent toPost(wxEVT_SCROLL_CHANGED, _quickPreviewSlider->GetId(), val);
		ProcessEvent(toPost);
	}
}

void FxActorFaceGraphView::OnFaceGraphNodeGroupChoiceSelChanged( wxCommandEvent& event )
{
	if( _actor && _faceGraphNodeGroupDropdown && _faceGraphNodeListBox )
	{
		_faceGraphNodeListBox->Freeze();
		_faceGraphNodeListBox->ClearAll();
		wxListItem columnInfo;
		columnInfo.m_mask = wxLIST_MASK_TEXT|wxLIST_MASK_WIDTH;
		columnInfo.m_text = _("Node Name");
		columnInfo.m_width = GetClientRect().width;
		_faceGraphNodeListBox->InsertColumn(0, columnInfo);

		wxString nodeGroup = _faceGraphNodeGroupDropdown->GetString(_faceGraphNodeGroupDropdown->GetSelection());
		if( FxFaceGraphNodeGroupManager::GetAllNodesGroupName() == FxString(nodeGroup.mb_str(wxConvLibc)) )
		{
			FxFaceGraph::Iterator fgNodeIter    = _actor->GetDecompiledFaceGraph().Begin(FxFaceGraphNode::StaticClass());
			FxFaceGraph::Iterator fgNodeEndIter = _actor->GetDecompiledFaceGraph().End(FxFaceGraphNode::StaticClass());
			FxInt32 count = 0;
			for( ; fgNodeIter != fgNodeEndIter; ++fgNodeIter )
			{
				FxFaceGraphNode* pNode = fgNodeIter.GetNode();
				if( pNode )
				{
					_faceGraphNodeListBox->InsertItem(count, wxString(pNode->GetNameAsCstr(), wxConvLibc));
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
					_faceGraphNodeListBox->InsertItem(count, wxString(pGroup->GetNode(i).GetAsCstr(), wxConvLibc));
					count++;
				}
			}
		}
		_faceGraphNodeListBox->Thaw();

		if( event.GetClientData() != reinterpret_cast<void*>(kNoPropagateMessage) )
		{
			wxString command = wxString::Format(wxT("select -type \"nodegroup\" -names \"%s\""), nodeGroup);
			FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
		}
	}
	event.Skip();
}

void FxActorFaceGraphView::OnFaceGraphNodeListBoxSelChanged( wxListEvent& event )
{
	if( _actor && _faceGraphNodeListBox )
	{
		wxString selFaceGraphNodeNames;
		FxInt32 index = -1;
		for( ;; )
		{
			index = _faceGraphNodeListBox->GetNextItem(index, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

			if( index == -1 )
				break;

			selFaceGraphNodeNames += _faceGraphNodeListBox->GetItemText(index) + wxT("|");
		}
		if( !_noDispatchSelection )
		{
			if( selFaceGraphNodeNames.Length() > 0 )
			{
				selFaceGraphNodeNames.RemoveLast();
				wxString command = wxString::Format(wxT("select -type \"node\" -names \"%s\""),
					selFaceGraphNodeNames);
				FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
			}
			else
			{
				FxVM::ExecuteCommand("select -type \"node\" -clear");
			}
		}
	}
	event.Skip();
}

// Handle the slider movement.
void FxActorFaceGraphView::OnPreviewSliderMoved( wxScrollEvent& event )
{
	FxReal value = event.GetPosition()/100.0f;
	if( _linkedNode )
	{
		FxSessionProxy::SetRegister(_linkedNode->GetName(), value, RO_Add);
		FxSessionProxy::UpdateActorInstance();
	}

	// Tell all of the other widgets to refresh.
	if( _mediator )
	{
		_mediator->OnRefresh(this);
	}
}

void FxActorFaceGraphView::_setNodeGroupAndNodeSelection( void )
{
	if( _actor && _faceGraphNodeGroupDropdown && _faceGraphNodeListBox )
	{
		// Select the correct group.
		wxString groupName(_selFaceGraphNodeGroup.GetAsCstr(), wxConvLibc);
		FxInt32 index = _faceGraphNodeGroupDropdown->FindString(groupName);
		if( index != -1 )
		{
			_faceGraphNodeGroupDropdown->SetSelection(index);
			// Force a refresh of the node list.
			wxCommandEvent event;
			event.SetClientData(reinterpret_cast<void*>(kNoPropagateMessage));
			OnFaceGraphNodeGroupChoiceSelChanged(event);
		}
		else
		{
			// Select the all nodes group.
			_faceGraphNodeGroupDropdown->SetStringSelection(wxString(FxFaceGraphNodeGroupManager::GetAllNodesGroupName().GetData(), wxConvLibc));
		}
		// Force the list box to be filled out.
		wxCommandEvent event;
		event.SetClientData(reinterpret_cast<void*>(kNoPropagateMessage));
		OnFaceGraphNodeGroupChoiceSelChanged(event);
	}
}

void FxActorFaceGraphView::_setNodeListSelection( void )
{
	if( _actor && _faceGraphNodeGroupDropdown && _faceGraphNodeListBox )
	{
		if( _faceGraphNodeListBox->GetItemCount() > 0 )
		{
			// Clear the selection.
			_noDispatchSelection = FxTrue;
			_faceGraphNodeListBox->Freeze();
			FxInt32 numItems = _faceGraphNodeListBox->GetItemCount();
			for( FxInt32 i = 0; i < numItems; i++ )
			{
				_faceGraphNodeListBox->SetItemState(i, 0, wxLIST_STATE_SELECTED);
			}

			// Select the correct curves.
			for( FxSize i = 0; i < _selFaceGraphNodes.Length(); ++i )
			{
				FxInt32 item = _faceGraphNodeListBox->FindItem(-1, wxString::FromAscii(_selFaceGraphNodes[i].GetAsCstr()));
				if( item != -1 )
				{
					_faceGraphNodeListBox->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
				}
			}
			_faceGraphNodeListBox->Thaw();
			_noDispatchSelection = FxFalse;
		}
	}
}

// Handles column clicks in the face graph node list control.
void FxActorFaceGraphView::OnColumnClick( wxListEvent& FxUnused(event) )
{
	static FxBool sortAscending = FxTrue;

	if( _actor && _faceGraphNodeListBox )
	{
		FxArray<FxString*> itemStrings;
		// Tag the nodes.
		FxInt32 index = -1;
		for( ;; )
		{
			index = _faceGraphNodeListBox->GetNextItem(index);
			if( index == -1 ) break;
			FxString* itemString = new FxString(_faceGraphNodeListBox->GetItemText(index).mb_str(wxConvLibc));
			itemStrings.PushBack(itemString);
			_faceGraphNodeListBox->SetItemData(index, reinterpret_cast<long>(itemString));
		}
		_faceGraphNodeListBox->SortItems(CompareStrings, static_cast<long>(sortAscending));
		for( FxSize i = 0; i < itemStrings.Length(); ++i )
		{
			delete itemStrings[i];
			itemStrings[i] = NULL;
		}
		itemStrings.Clear();
		sortAscending = !sortAscending;
	}
}

//------------------------------------------------------------------------------
// The animations view.
//------------------------------------------------------------------------------
WX_IMPLEMENT_CLASS(FxActorAnimationsView, FxExpandableWindow)

BEGIN_EVENT_TABLE(FxActorAnimationsView, FxExpandableWindow)
	EVT_CHOICE(ControlID_ActorWidgetAnimationViewAnimGroupCombo, FxActorAnimationsView::OnAnimGroupSelChanged)
	EVT_CHOICE(ControlID_ActorWidgetAnimationViewAnimCombo, FxActorAnimationsView::OnAnimSelChanged)
	EVT_LIST_ITEM_FOCUSED(ControlID_ActorWidgetAnimationViewAnimCurveListBox, FxActorAnimationsView::OnAnimCurveListBoxSelChanged)
	EVT_LIST_ITEM_RIGHT_CLICK(ControlID_ActorWidgetAnimationViewAnimCurveListBox, FxActorAnimationsView::OnAnimCurveListBoxItemRightClicked)
	EVT_LIST_COL_CLICK(ControlID_ActorWidgetAnimationViewAnimCurveListBox, FxActorAnimationsView::OnAnimCurveListBoxColumnClick)

	EVT_MENU(MenuID_ActorWidgetAnimationViewCurveManager, FxActorAnimationsView::OnMenuCurveManager)
	EVT_MENU(MenuID_ActorWidgetAnimationViewCurveProperties, FxActorAnimationsView::OnMenuCurveProperties)
END_EVENT_TABLE()

FxActorAnimationsView::FxActorAnimationsView( wxWindow* parent, FxWidgetMediator* mediator )
	: Super(parent)
	, FxWidget(mediator)
	, _actor(NULL)
	, _animGroupDropdown(NULL)
	, _animDropdown(NULL)
	, _animCurveListBox(NULL)
	, _imageList(NULL)
	, _lockedBitmap(wxNullBitmap)
	, _unlockedBitmap(wxNullBitmap)
	, _noDispatchSelection(FxFalse)
{
	_selAnimGroup = FxAnimGroup::Default;
	_selAnim      = FxName("<No animation selected>");
	SetName(_("Animations"));
}

FxActorAnimationsView::~FxActorAnimationsView()
{
	if( _imageList )
	{
		delete _imageList;
		_imageList = NULL;
	}
}

void FxActorAnimationsView::CreateChildren( void )
{
	wxStaticText* groupLabel = new wxStaticText(this, wxID_DEFAULT, _("Group"),
		wxPoint(kChildIndent,kCollapsedHeight),
		wxSize(GetClientRect().width-5-kChildIndent,15));
	_children.PushBack(groupLabel);
	_animGroupDropdown = new wxChoice(this, ControlID_ActorWidgetAnimationViewAnimGroupCombo, 
		wxPoint(kChildIndent,kCollapsedHeight+15),
		wxSize(GetClientRect().width - 5 - kChildIndent,20));
	_children.PushBack(_animGroupDropdown);

	wxStaticText* animLabel = new wxStaticText(this, wxID_DEFAULT, _("Animation"),
		wxPoint(kChildIndent,kCollapsedHeight+40),
		wxSize(GetClientRect().width-5-kChildIndent,15));
	_children.PushBack(animLabel);
	_animDropdown = new wxChoice(this, ControlID_ActorWidgetAnimationViewAnimCombo, 
		wxPoint(kChildIndent,kCollapsedHeight+55),
		wxSize(GetClientRect().width - 5 - kChildIndent,20));
	_children.PushBack(_animDropdown);

	wxStaticText* curveLabel = new wxStaticText(this, wxID_DEFAULT, _("Curves"),
		wxPoint(kChildIndent,kCollapsedHeight+80),
		wxSize(GetClientRect().width-5-kChildIndent,15));
	_children.PushBack(curveLabel);
	_animCurveListBox = new wxListCtrl(this, ControlID_ActorWidgetAnimationViewAnimCurveListBox,
		wxPoint(kChildIndent,kCollapsedHeight+95),
		wxSize(GetClientRect().width - 5 - kChildIndent,300), wxLC_REPORT);
	_children.PushBack(_animCurveListBox);

	if( _actor )
	{
		for( FxSize i = 0; i < _actor->GetNumAnimGroups(); ++i )
		{
			_animGroupDropdown->Append(wxString(_actor->GetAnimGroup(i).GetNameAsCstr(), wxConvLibc));
		}
		_animGroupDropdown->SetSelection(_animGroupDropdown->FindString(wxString(_selAnimGroup.GetAsString().GetData(), wxConvLibc)));

		// Force the animation drop down to be filled out.
		_fillAnimDropdown();
		// Force the curve list box to be filled out.
		_fillCurveListBox();
		_setAnimViewSelection();
	}
	else
	{
		_animGroupDropdown->Enable(FALSE);
		_animDropdown->Enable(FALSE);
		_animCurveListBox->Enable(FALSE);
	}
}

void FxActorAnimationsView::DestroyChildren( void )
{
	// Destroy the windows.
	Super::DestroyChildren();
	_animGroupDropdown = NULL;
	_animDropdown = NULL;
	_animCurveListBox = NULL;
}

void FxActorAnimationsView::OnActorChanged( FxWidget* FxUnused(sender), FxActor* actor )
{
	_actor = actor;
	if( _isExpanded )
	{
		DestroyChildren();
		CreateChildren();
		_container->PositionExapandableWindows();
	}
}

void FxActorAnimationsView::OnActorInternalDataChanged( FxWidget* FxUnused(sender), FxActorDataChangedFlag flags )
{
	if( _isExpanded && _actor && _animGroupDropdown &&
		(flags & ADCF_AnimationsChanged || flags & ADCF_CurvesChanged || flags & ADCF_AnimationGroupsChanged) )
	{
		Freeze();
		_animGroupDropdown->Clear();
		for( FxSize i = 0; i < _actor->GetNumAnimGroups(); ++i )
		{
			_animGroupDropdown->Append(wxString(_actor->GetAnimGroup(i).GetNameAsCstr(), wxConvLibc));
		}
		_animGroupDropdown->SetSelection(_animGroupDropdown->FindString(wxString(_selAnimGroup.GetAsString().GetData(), wxConvLibc)));
		// Force the animation drop down to be filled out.
		_fillAnimDropdown();
		// Force the animation curve list box to be filled out.
		_fillCurveListBox();
		_setAnimViewSelection();
		Thaw();
	}
}

void FxActorAnimationsView::
OnAnimChanged( FxWidget* FxUnused(sender), const FxName& animGroupName, FxAnim* anim )
{
	_anim = anim;
	_selAnimGroup = animGroupName;
	_selAnim      = _anim ? _anim->GetName() : FxName();

	if( _isExpanded && _actor && _animGroupDropdown )
	{
		_animGroupDropdown->Clear();
		for( FxSize i = 0; i < _actor->GetNumAnimGroups(); ++i )
		{
			_animGroupDropdown->Append(wxString(_actor->GetAnimGroup(i).GetNameAsCstr(), wxConvLibc));
		}
		_animGroupDropdown->SetSelection(_animGroupDropdown->FindString(wxString(_selAnimGroup.GetAsString().GetData(), wxConvLibc)));
		// Force the animation drop down to be filled out.
		_fillAnimDropdown();
		// Force the curve list box to be filled out.
		_fillCurveListBox();
	}
}

void FxActorAnimationsView::
OnAnimCurveSelChanged( FxWidget* FxUnused(sender), const FxArray<FxName>& selAnimCurveNames )
{
	_selAnimCurves = selAnimCurveNames;
	_setAnimViewSelection();
}

void FxActorAnimationsView::OnAnimGroupSelChanged( wxCommandEvent& event )
{
	if( _actor && _animGroupDropdown )
	{
		wxString animGroupString = _animGroupDropdown->GetString(_animGroupDropdown->GetSelection());
		wxString command = wxString::Format(wxT("select -type \"animgroup\" -names \"%s\""), animGroupString);
		FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
		_fillAnimDropdown();
		_fillCurveListBox();
	}
	event.Skip();
}

void FxActorAnimationsView::OnAnimSelChanged( wxCommandEvent& event )
{
	if( _actor && _animGroupDropdown && _animDropdown )
	{
		wxString animGroupString = _animGroupDropdown->GetString(_animGroupDropdown->GetSelection());
		FxSize animGroupIndex = _actor->FindAnimGroup(_selAnimGroup);
		if( animGroupIndex != FxInvalidIndex )
		{
			FxAnimGroup& animGroup = _actor->GetAnimGroup(animGroupIndex);
			wxString selAnimString = _animDropdown->GetString(_animDropdown->GetSelection());
			if( selAnimString != _("<No animation selected>") )
			{
				_selAnim = FxName(FxString(selAnimString.mb_str(wxConvLibc)).GetData());
				FxSize animIndex = animGroup.FindAnim(_selAnim);
				if( animIndex != FxInvalidIndex )
				{
					_animDropdown->Delete(_animDropdown->FindString(_("<No animation selected>")));
				}
				wxString command = wxString::Format(wxT("select -type \"anim\" -names \"%s\""), selAnimString);
				FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
			}
		}
	}
	event.Skip();
}

void FxActorAnimationsView::OnAnimCurveListBoxSelChanged( wxListEvent& event )
{
	if( _actor && _animCurveListBox )
	{
		FxArray<FxName> selectedCurves = _getSelectedCurveNamesFromListCtrl();
		
		wxString selCurves;
		for( FxSize i = 0; i < selectedCurves.Length(); ++i )
		{
			selCurves += wxString::FromAscii(selectedCurves[i].GetAsCstr()) + wxT("|");
		}
		if( selCurves.Length() > 0 )
		{
			selCurves.RemoveLast();
		}
		if( !_noDispatchSelection )
		{
			wxString command;
			//@todo Revisit this when animation / curve commands are working.
			if( selCurves.Length() == 0 )
			{
				command = wxT("select -type \"curve\" -clear");
			}
			else
			{
				command = wxString::Format(wxT("select -type \"curve\" -names \"%s\""), selCurves);
			}
			FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
		}
	}
	event.Skip();
}

void FxActorAnimationsView::OnAnimCurveListBoxItemRightClicked( wxListEvent& event )
{
	wxMenu popup;
	popup.Append(MenuID_ActorWidgetAnimationViewCurveProperties, _("Curve Properties..."), _("Displays the curve properties dialog."));
	popup.AppendSeparator();
	popup.Append(MenuID_ActorWidgetAnimationViewCurveManager, _("Curve Manager..."));

	//@hack trying to make this appear positioned correctly.
	PopupMenu(&popup, wxPoint(event.GetPoint().x+5, event.GetPoint().y+70)); 
}

void FxActorAnimationsView::OnAnimCurveListBoxColumnClick( wxListEvent& FxUnused(event) )
{
	static FxBool sortAscending = FxTrue;

	if( _actor && _animCurveListBox )
	{
		FxAnimGroup& currAnimGroup = _actor->GetAnimGroup(_actor->FindAnimGroup(_selAnimGroup));
		FxSize animIndex = currAnimGroup.FindAnim(_selAnim);
		if( animIndex != FxInvalidIndex )
		{
			FxAnim* currAnim = currAnimGroup.GetAnimPtr(animIndex);
			if( currAnim )
			{
				// Tag the curves.
				FxInt32 index = -1;
				for( ;; )
				{
					index = _animCurveListBox->GetNextItem(index);

					if( index == -1 ) break;

					FxName curveName(FxString(_animCurveListBox->GetItemText(index).mb_str(wxConvLibc)).GetData());
					FxAnimCurve& animCurve = currAnim->GetAnimCurveM(currAnim->FindAnimCurve(curveName));
					_animCurveListBox->SetItemData(index, reinterpret_cast<long>(&animCurve.GetName()));
				}

				_animCurveListBox->SortItems(CompareNames, static_cast<long>(sortAscending));
				sortAscending = !sortAscending;
			}
		}
	}
}

void FxActorAnimationsView::OnMenuCurveManager( wxCommandEvent& FxUnused(event) )
{
	if( _actor )
	{
		FxCurveManagerDialog curveManager;
		curveManager.Initialize(_actor, _mediator, _selAnimGroup, _selAnim);
		curveManager.Create(this);
		FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
		curveManager.ShowModal();
		FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
		// Recreate the curve list box, since the contents of the animation
		// may have changed.
		_fillCurveListBox();
	}
}

void FxActorAnimationsView::OnMenuCurveProperties( wxCommandEvent& FxUnused(event) )
{
	if( _animCurveListBox )
	{
		FxAnim* anim = _getCurrentAnim();
		if( anim )
		{
			FxInt32 i = _animCurveListBox->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			wxColour startingColour(*wxBLACK);
			FxName animCurveName;
			if( i != -1 )
			{	
				wxString animCurveString = _animCurveListBox->GetItemText(i);
				animCurveName = FxName(FxString(animCurveString.mb_str(wxConvLibc)).GetData());
				startingColour = FxColourMap::Get(animCurveName);
			}

			FxCurvePropertiesDialog propertiesDialog(this);
			propertiesDialog.SetCurveColour(startingColour);

			FxBool ownedByAnalysis = FxFalse;
			if( anim->GetUserData() )
			{
				ownedByAnalysis = static_cast<FxAnimUserData*>(anim->GetUserData())->IsCurveOwnedByAnalysis(animCurveName);
			}
			propertiesDialog.SetIsCurveOwnedByAnalysis(ownedByAnalysis);

			FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
			if( propertiesDialog.ShowModal() == wxID_OK )
			{
				FxBool ownedByAnalysis = propertiesDialog.GetIsCurveOwnedByAnalysis();

				FxVM::ExecuteCommand("batch");
				// Find the selected curves and set their colors to the chosen colour.
				FxInt32 item = -1;
				for( ;; )
				{
					item = _animCurveListBox->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

					if( item == -1 )
					{
						// We've reached the end.
						break;
					}

					wxString animCurveString = _animCurveListBox->GetItemText(item);
					FxName animCurveName(FxString(animCurveString.mb_str(wxConvLibc)).GetData());
					FxColourMap::Set(animCurveName, propertiesDialog.GetCurveColour());
					
					FxString ownershipCommand("curve -edit -group \"");
					ownershipCommand += _selAnimGroup.GetAsString();
					ownershipCommand += "\" -anim \"";
					ownershipCommand += _selAnim.GetAsString();
					ownershipCommand += "\" -name \"";
					ownershipCommand += animCurveName.GetAsString();
					if( ownedByAnalysis )
					{
						ownershipCommand += "\" -owner \"analysis\"";
					}
					else
					{
						ownershipCommand += "\" -owner \"user\"";
					}
					FxVM::ExecuteCommand(ownershipCommand);
				}
				FxVM::ExecuteCommand("execBatch -editedcurves");
			}

			// Update the image list.
			_createImageList();

			if( _mediator )
			{
				// Tell the widgets to update themselves.
				_mediator->OnRefresh(this);
			}
		}
		FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
	}
}

void FxActorAnimationsView::_setAnimViewSelection( void )
{
	if( _actor && _animGroupDropdown && _animDropdown && _animCurveListBox )
	{
		if( _animCurveListBox->GetItemCount() > 0 )
		{
			// Clear the selection.
			_noDispatchSelection = FxTrue;
			_animCurveListBox->Freeze();
			FxInt32 numItems = _animCurveListBox->GetItemCount();
			for( FxInt32 i = 0; i < numItems; i++ )
			{
				_animCurveListBox->SetItemState(i, 0, wxLIST_STATE_SELECTED);
			}
			
			// Select the correct curves.
			for( FxSize i = 0; i < _selAnimCurves.Length(); ++i )
			{
				FxInt32 item = _animCurveListBox->FindItem(-1, wxString::FromAscii(_selAnimCurves[i].GetAsCstr()));
				if( item != -1 )
				{
					_animCurveListBox->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
				}
			}
			_animCurveListBox->Thaw();
			_noDispatchSelection = FxFalse;
		}
	}
}

FxArray<FxName> FxActorAnimationsView::_getSelectedCurveNamesFromListCtrl( void )
{
	FxArray<FxName> retval;
	if( _animCurveListBox )
	{
		FxInt32 item = -1;
		for( ;; )
		{
			item = _animCurveListBox->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

			if( item == -1 )
			{
				// We've reached the end.
				break;
			}

			wxString animCurveString = _animCurveListBox->GetItemText(item);
			FxName animCurveName(FxString(animCurveString.mb_str(wxConvLibc)).GetData());
			retval.PushBack(animCurveName);
		}
	}
	return retval;
}

void FxActorAnimationsView::_fillAnimDropdown( void )
{
	if( _actor && _animDropdown && _animGroupDropdown )
	{
		_animDropdown->Freeze();
		_animDropdown->Clear();
		_animDropdown->AppendString(_("<No animation selected>"));
		wxString animGroupString = _animGroupDropdown->GetString(_animGroupDropdown->GetSelection());
		_selAnimGroup = FxName(FxString(animGroupString.mb_str(wxConvLibc)).GetData());
		FxSize animGroupIndex = _actor->FindAnimGroup(_selAnimGroup);
		if( animGroupIndex != FxInvalidIndex )
		{
			wxArrayString choices;
			FxAnimGroup& animGroup = _actor->GetAnimGroup(animGroupIndex);
			for( FxSize i = 0; i < animGroup.GetNumAnims(); ++i )
			{
				choices.Add(wxString(animGroup.GetAnim(i).GetNameAsCstr(), wxConvLibc));
			}
			choices.Sort();
			_animDropdown->Append(choices);
		}
		FxInt32 animSel = _animDropdown->FindString(wxString(_selAnim.GetAsString().GetData(), wxConvLibc));
		if( animSel < 0 )
		{
			_animDropdown->SetSelection(0);
		}
		else
		{
			if( _animDropdown->GetString(animSel) != _("<No animation selected>") )
			{
				_animDropdown->Delete(_animDropdown->FindString(_("<No animation selected>")));
				animSel--;
			}
			_animDropdown->SetSelection(animSel);
		}
		_animDropdown->Thaw();
	}
}

void FxActorAnimationsView::_fillCurveListBox( void )
{
	if( _actor && _animCurveListBox && _animDropdown && _animGroupDropdown )
	{
		_animCurveListBox->Freeze();
		_animCurveListBox->ClearAll();

		wxListItem columnInfo;
		columnInfo.m_mask = wxLIST_MASK_TEXT|wxLIST_MASK_WIDTH;
		columnInfo.m_text = _("Curve Name");
		columnInfo.m_width = GetClientRect().width;
		_animCurveListBox->InsertColumn(0, columnInfo);

		wxString animGroupString = _animGroupDropdown->GetString(_animGroupDropdown->GetSelection());
		FxSize animGroupIndex = _actor->FindAnimGroup(_selAnimGroup);
		if( animGroupIndex != FxInvalidIndex )
		{
			FxAnimGroup& animGroup = _actor->GetAnimGroup(animGroupIndex);
			wxString animString = _animDropdown->GetString(_animDropdown->GetSelection());
			_selAnim = FxName(FxString(animString.mb_str(wxConvLibc)).GetData());
			FxSize animIndex = animGroup.FindAnim(_selAnim);
			if( animIndex != FxInvalidIndex )
			{
				const FxAnim& anim = animGroup.GetAnim(animIndex);
				for( FxSize i = 0; i < anim.GetNumAnimCurves(); ++i )
				{
					wxListItem item;
					item.SetText(wxString(anim.GetAnimCurve(i).GetNameAsCstr(), wxConvLibc));
					item.SetId(i);
					item.m_mask = wxLIST_MASK_TEXT;
					_animCurveListBox->InsertItem(item);
				}
				_createImageList();
			}
		}
		_animCurveListBox->Thaw();
	}
}

void FxActorAnimationsView::_createImageList( void )
{
	if( !_lockedBitmap.IsOk() )
	{
		wxString file = FxStudioApp::GetBitmapPath(wxT("Lock.bmp"));
		_lockedBitmap = wxBitmap(file, wxBITMAP_TYPE_BMP);
	}
	if( !_unlockedBitmap.IsOk() )
	{
		wxString file = FxStudioApp::GetBitmapPath(wxT("Unlock.bmp"));
		_unlockedBitmap = wxBitmap(file, wxBITMAP_TYPE_BMP);
	}

	const wxSize imageSize(32,16);
	if( _animCurveListBox )
	{
		FxAnim* anim = _getCurrentAnim();
		if( anim )
		{
			_animCurveListBox->SetImageList(NULL, wxIMAGE_LIST_SMALL);

			if( _imageList )
			{
				delete _imageList;
				_imageList = NULL;
			}

			wxMemoryDC lockedDC;
			lockedDC.SelectObject(_lockedBitmap);
			wxMemoryDC unlockedDC;
			unlockedDC.SelectObject(_unlockedBitmap);

			_imageList = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());
			wxBitmap colourBox(imageSize.GetWidth(), imageSize.GetHeight());
			wxMemoryDC blackUnlockedDC;
			// Black unlocked is always at imagelist[0].
			blackUnlockedDC.SelectObject(colourBox);
			blackUnlockedDC.SetPen(*wxWHITE_PEN);
			blackUnlockedDC.SetBrush(*wxBLACK_BRUSH);
			blackUnlockedDC.DrawRectangle(wxPoint(0,0), wxSize(imageSize.GetWidth()/2, imageSize.GetHeight()));
			blackUnlockedDC.Blit(wxPoint(16,0), unlockedDC.GetSize(), &unlockedDC, unlockedDC.GetLogicalOrigin());
			blackUnlockedDC.SelectObject(wxNullBitmap);
			_imageList->Add(colourBox);
			// Black locked is always at imagelist[1].
			wxMemoryDC blackLockedDC;
			blackLockedDC.SelectObject(colourBox);
			blackLockedDC.SetPen(*wxWHITE_PEN);
			blackLockedDC.SetBrush(*wxBLACK_BRUSH);
			blackLockedDC.DrawRectangle(wxPoint(0,0), wxSize(imageSize.GetWidth()/2, imageSize.GetHeight()));
			blackLockedDC.Blit(wxPoint(16,0), lockedDC.GetSize(), &lockedDC, lockedDC.GetLogicalOrigin());
			blackLockedDC.SelectObject(wxNullBitmap);
			_imageList->Add(colourBox);

			FxInt32 index = -1;
			FxInt32 count = 2;
			for( ;; )
			{
				index = _animCurveListBox->GetNextItem(index, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
				if( index == -1 )
					break;

				FxName curveName(FxString(_animCurveListBox->GetItemText(index).mb_str(wxConvLibc)).GetData());

				wxColour curveColour = FxColourMap::Get(curveName);
				if( curveColour != *wxBLACK )
				{
					wxMemoryDC colourDC;
					colourDC.SelectObject(colourBox);
					colourDC.SetPen(*wxWHITE_PEN);
					colourDC.SetBrush(wxBrush(curveColour, wxSOLID));
					colourDC.DrawRectangle(wxPoint(0,0), wxSize(imageSize.GetWidth()/2, imageSize.GetHeight()));

					if( anim->GetUserData() )
					{
						if( static_cast<FxAnimUserData*>(anim->GetUserData())->IsCurveOwnedByAnalysis(curveName))
						{
							colourDC.Blit(wxPoint(16,0), lockedDC.GetSize(), &lockedDC, lockedDC.GetLogicalOrigin());
						}
						else
						{
							colourDC.Blit(wxPoint(16,0), unlockedDC.GetSize(), &unlockedDC, unlockedDC.GetLogicalOrigin());
						}
					}
					else
					{
						colourDC.Blit(wxPoint(16,0), unlockedDC.GetSize(), &unlockedDC, unlockedDC.GetLogicalOrigin());
					}
					colourDC.SelectObject(wxNullBitmap);
					_imageList->Add(colourBox);
					_animCurveListBox->SetItemImage(index, count, count);
					count++;
				}
				else
				{
					if( anim->GetUserData() )
					{
						if( static_cast<FxAnimUserData*>(anim->GetUserData())->IsCurveOwnedByAnalysis(curveName) )
						{
							// Black locked is always 1.
							_animCurveListBox->SetItemImage(index, 1, 1);
						}
						else
						{
							// Black unlocked is always 0.
							_animCurveListBox->SetItemImage(index, 0, 0);
						}
					}
					else
					{
						// Black unlocked is always 0.
						_animCurveListBox->SetItemImage(index, 0, 0);
					}
				}
			}

			_animCurveListBox->SetImageList(_imageList, wxIMAGE_LIST_SMALL);
		}
	}
}

// Returns a pointer to the currently selected animation.
FxAnim* FxActorAnimationsView::_getCurrentAnim( void )
{
	if( _actor )
	{
		FxSize animGroupIndex = _actor->FindAnimGroup(_selAnimGroup);
		if( animGroupIndex != FxInvalidIndex )
		{
			FxAnimGroup& group = _actor->GetAnimGroup(animGroupIndex);
			FxSize animIndex = group.FindAnim(_selAnim);
			if( animIndex != FxInvalidIndex )
			{
				return group.GetAnimPtr(animIndex);
			}
		}
	}
	return NULL;
}

//------------------------------------------------------------------------------
// An actor widget.
//------------------------------------------------------------------------------
WX_IMPLEMENT_DYNAMIC_CLASS(FxActorWidget, FxExpandableWindowContainer)

BEGIN_EVENT_TABLE(FxActorWidget, FxExpandableWindowContainer)
	EVT_HELP_RANGE(wxID_ANY, wxID_HIGHEST, FxActorWidget::OnHelp)
END_EVENT_TABLE()

FxActorWidget::FxActorWidget( wxWindow* parent, FxWidgetMediator* mediator )
	: Super(parent)
	, FxWidget(mediator)
	, _actor(NULL)
{
	FxActorBoneView* boneView = new FxActorBoneView(this, _mediator);
	AddExpandableWindow(boneView);
	FxActorFaceGraphView* faceGraphView = new FxActorFaceGraphView(this, _mediator);
	AddExpandableWindow(faceGraphView);
	FxActorAnimationsView* animationsView = new FxActorAnimationsView(this, _mediator);
	AddExpandableWindow(animationsView);
}

FxActorWidget::~FxActorWidget()
{
}

void FxActorWidget::OnActorChanged( FxWidget* FxUnused(sender), FxActor* actor )
{
	_actor = actor;
}

void FxActorWidget::OnActorInternalDataChanged( FxWidget* FxUnused(sender), FxActorDataChangedFlag FxUnused(flags) )
{
}

void FxActorWidget::OnHelp(wxHelpEvent& FxUnused(event))
{
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().LoadFile();
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().DisplaySection(wxT("Actor Panel"));
}

void FxActorWidget::OnSerializeOptions(FxWidget* FxUnused(sender), wxFileConfig* config, FxBool isLoading)
{
	//@todo hackish use of hard-coded array indices that might change.
	if( isLoading )
	{
		FxBool boneViewExpanded = FxFalse;
		FxBool faceGraphViewExpanded = FxFalse;
		FxBool animationViewExpanded = FxFalse;
		config->Read(_("/ActorWidget/BoneViewExpanded"), &boneViewExpanded, FxFalse);
		config->Read(_("/ActorWidget/FaceGraphViewExpanded"), &faceGraphViewExpanded, FxFalse);
		config->Read(_("/ActorWidget/AnimationViewExpanded"), &animationViewExpanded, FxTrue);

		if( _expandableWindows.Length() >= 3 )
		{
			if( boneViewExpanded )
			{
				_expandableWindows[0]->Expand();
			}
			if( faceGraphViewExpanded )
			{
				_expandableWindows[1]->Expand();
			}
			if( animationViewExpanded )
			{
				_expandableWindows[2]->Expand();
			}
		}
	}
	else
	{
		config->Write(_("/ActorWidget/BoneViewExpanded"), _expandableWindows[0]->IsExpanded());
		config->Write(_("/ActorWidget/FaceGraphViewExpanded"), _expandableWindows[1]->IsExpanded());
		config->Write(_("/ActorWidget/AnimationViewExpanded"), _expandableWindows[2]->IsExpanded());
	}
}

} // namespace Face

} // namespace OC3Ent
