//------------------------------------------------------------------------------
// Curve manager dialog.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxCurveManagerDialog.h"
#include "FxFaceGraph.h"
#include "FxVM.h"
#include "FxSessionProxy.h"
#include "FxAnimUserData.h"

namespace OC3Ent
{

namespace Face
{

WX_IMPLEMENT_DYNAMIC_CLASS(FxCurveManagerDialog, wxDialog)

BEGIN_EVENT_TABLE(FxCurveManagerDialog, wxDialog)
	EVT_CHOICE(ControlID_CurveManagerAnimGroupChoice, FxCurveManagerDialog::OnChoiceAnimGroupChanged)
	EVT_CHOICE(ControlID_CurveManagerAnimationChoice, FxCurveManagerDialog::OnChoiceAnimationChanged)
	EVT_CHOICE(ControlID_CurveManagerTopFilterChoice, FxCurveManagerDialog::OnChoiceTopFilterChanged)
	EVT_CHOICE(ControlID_CurveManagerBottomFilterChoice, FxCurveManagerDialog::OnChoiceBottomFilterChanged)

	EVT_LIST_ITEM_SELECTED(ControlID_CurveManagerSourceNamesListCtrl, FxCurveManagerDialog::OnListSelectionChanged)
	EVT_LIST_ITEM_DESELECTED(ControlID_CurveManagerSourceNamesListCtrl, FxCurveManagerDialog::OnListSelectionChanged)
	EVT_LIST_ITEM_SELECTED(ControlID_CurveManagerTargetNamesListCtrl, FxCurveManagerDialog::OnListSelectionChanged)
	EVT_LIST_ITEM_DESELECTED(ControlID_CurveManagerTargetNamesListCtrl, FxCurveManagerDialog::OnListSelectionChanged)

	EVT_BUTTON(ControlID_CurveManagerRemoveCurvesButton, FxCurveManagerDialog::OnButtonRemoveCurves)
	EVT_BUTTON(ControlID_CurveManagerAddCurrAnimButon, FxCurveManagerDialog::OnButtonAddCurrentAnim)
	EVT_BUTTON(ControlID_CurveManagerAddCurrGroupButton, FxCurveManagerDialog::OnButtonAddCurrentGroup)
	EVT_BUTTON(ControlID_CurveManagerAddAllAnimsButton, FxCurveManagerDialog::OnButtonAddAllGroups)
	EVT_BUTTON(ControlID_CurveManagerOKButton, FxCurveManagerDialog::OnButtonOK)

	EVT_LIST_COL_CLICK(ControlID_CurveManagerTargetNamesListCtrl, FxCurveManagerDialog::OnListTargetNamesColumnClick)
	EVT_LIST_COL_CLICK(ControlID_CurveManagerSourceNamesListCtrl, FxCurveManagerDialog::OnListSourceNamesColumnClick)
END_EVENT_TABLE()

// Constructors.
FxCurveManagerDialog::FxCurveManagerDialog()
	: _pActor(NULL)
	, _pMediator(NULL)
{
}

FxCurveManagerDialog::FxCurveManagerDialog( wxWindow* parent, wxWindowID id,
	const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
	: _pActor(NULL)
	, _pMediator(NULL)
{
	Create( parent, id, caption, pos, size, style );
}

// Creation.
FxBool FxCurveManagerDialog::Create( wxWindow* parent, wxWindowID id, 
	const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
	_choiceAnimGroup		= NULL;
	_choiceAnimation		= NULL;
	_choiceTopFilter		= NULL;
	_choiceBottomFilter		= NULL;
	_listSourceCurves		= NULL;
	_listTargetCurves		= NULL;
	_buttonRemoveCurves		= NULL;
	_buttonAddCurrentAnim	= NULL;
	_buttonAddCurrentGroup	= NULL;
	_buttonAddAllGroups		= NULL;

	SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
	SetExtraStyle(GetExtraStyle()|wxWS_EX_VALIDATE_RECURSIVELY);
	wxDialog::Create(parent, id, caption, pos, size, style);

	CreateControls();
	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();
	return TRUE;
}

// Initialization.
void FxCurveManagerDialog::Initialize( FxActor* actor, FxWidgetMediator* mediator,
								       FxName animGroup, FxName animation )
{
	_pActor = actor;
	_pMediator = mediator;
	_animGroup = animGroup;
	_animation = animation;
}

// Creates the controls and sizers.
void FxCurveManagerDialog::CreateControls( void )
{
	wxBoxSizer* boxSizerV = new wxBoxSizer(wxVERTICAL);
	SetSizer(boxSizerV);
	SetAutoLayout(TRUE);
	
	wxBoxSizer* boxSizerH = new wxBoxSizer(wxHORIZONTAL);
	boxSizerV->Add(boxSizerH);

	// Left column
	wxStaticBox* curveTargetBox = new wxStaticBox(this, wxID_ANY, _("Curve Target"));
	wxStaticBoxSizer* curveTargetSizer = new wxStaticBoxSizer(curveTargetBox, wxVERTICAL);
	boxSizerH->Add(curveTargetSizer, 0, wxGROW|wxALL, 5);

	wxStaticText* animGroupLabel = new wxStaticText(this, wxID_ANY, _("Animation Group"));
	curveTargetSizer->Add(animGroupLabel, 0, wxLEFT|wxRIGHT|wxTOP, 5);
	_choiceAnimGroup = new wxChoice(this, ControlID_CurveManagerAnimGroupChoice, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	curveTargetSizer->Add(_choiceAnimGroup, 0, wxGROW|wxALL, 5);

	wxStaticText* animLabel = new wxStaticText(this, wxID_ANY, _("Animation"));
	curveTargetSizer->Add(animLabel, 0, wxLEFT|wxRIGHT, 5);
	_choiceAnimation = new wxChoice(this, ControlID_CurveManagerAnimationChoice, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	curveTargetSizer->Add(_choiceAnimation, 0, wxGROW|wxALL, 5);

	_listTargetCurves = new wxListCtrl(this, ControlID_CurveManagerTargetNamesListCtrl, wxDefaultPosition, wxSize(225, 300), wxLC_REPORT);
	curveTargetSizer->Add(_listTargetCurves, 0, wxGROW|wxLEFT|wxRIGHT, 5);

	_buttonRemoveCurves = new wxButton(this, ControlID_CurveManagerRemoveCurvesButton, _("Remove Selected Curves"));
	curveTargetSizer->Add(_buttonRemoveCurves, 0, wxGROW|wxALL, 5);

	// Middle column
	wxBoxSizer* centerBoxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizerH->Add(centerBoxSizer, 0, wxGROW|wxALL|wxALIGN_CENTER_VERTICAL, 2);
	centerBoxSizer->AddSpacer(150);

	_buttonAddCurrentAnim = new wxButton(this, ControlID_CurveManagerAddCurrAnimButon, _("<< Current Anim"));
	centerBoxSizer->Add(_buttonAddCurrentAnim, 0, wxGROW|wxALL, 5);
	_buttonAddCurrentGroup = new wxButton(this, ControlID_CurveManagerAddCurrGroupButton, _("<< Current Group"));
	centerBoxSizer->Add(_buttonAddCurrentGroup, 0, wxGROW|wxLEFT|wxRIGHT, 5);
	_buttonAddAllGroups = new wxButton(this, ControlID_CurveManagerAddAllAnimsButton, _("<< All Groups"));
	centerBoxSizer->Add(_buttonAddAllGroups, 0, wxGROW|wxALL, 5);


	// Right column
	wxStaticBox* curveSourceBox = new wxStaticBox(this, wxID_ANY, _("Curve Source"));
	wxStaticBoxSizer* curveSourceSizer = new wxStaticBoxSizer(curveSourceBox, wxVERTICAL);
	boxSizerH->Add(curveSourceSizer, 0, wxGROW|wxALL, 5);

	wxStaticText* topFilterLabel = new wxStaticText(this, wxID_ANY, _("Top Level Filter"));
	curveSourceSizer->Add(topFilterLabel, 0, wxLEFT|wxRIGHT|wxTOP, 5);
	_choiceTopFilter = new wxChoice(this, ControlID_CurveManagerTopFilterChoice, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	curveSourceSizer->Add(_choiceTopFilter, 0, wxGROW|wxALL, 5);

	wxStaticText* secondFilterLabel = new wxStaticText(this, wxID_ANY, _("Second Level Filter"));
	curveSourceSizer->Add(secondFilterLabel, 0, wxLEFT|wxRIGHT, 5);
	_choiceBottomFilter = new wxChoice(this, ControlID_CurveManagerBottomFilterChoice, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	curveSourceSizer->Add(_choiceBottomFilter, 0, wxGROW|wxALL, 5);

	_listSourceCurves = new wxListCtrl(this, ControlID_CurveManagerSourceNamesListCtrl, wxDefaultPosition, wxSize(225, 300), wxLC_REPORT);
	curveSourceSizer->Add(_listSourceCurves, 0, wxGROW|wxLEFT|wxRIGHT, 5);

	curveSourceSizer->AddSpacer(33);

	// Bottom row
	wxButton* okButton = new wxButton(this, ControlID_CurveManagerOKButton, _("&OK"));
	boxSizerV->Add(okButton, 0, wxALL|wxALIGN_RIGHT, 5);

	// Create an invisible cancel button.
	// This will probably land me in programmer's hell at some point.
	// I just want escape to make the dumb dialog go away.
	wxButton* cancelButton = new wxButton(this, wxID_CANCEL, _("Cancel"), wxPoint(0,0), wxSize(0,0));
	cancelButton;

	_FillChoiceAnimGroup();
	if( _animGroup != FxName::NullName && _animation != FxName::NullName )
	{
		_choiceAnimGroup->SetSelection(_choiceAnimGroup->FindString(wxString::FromAscii(_animGroup.GetAsCstr())));
		_FillChoiceAnimation();
		_choiceAnimation->SetSelection(_choiceAnimation->FindString(wxString::FromAscii(_animation.GetAsCstr())));
		_FillListTarget();
	}
	else
	{
		_choiceAnimGroup->SetSelection(0);
		wxCommandEvent temp;
		OnChoiceAnimGroupChanged(temp);
	}
	_FillChoiceTopFilter();
	_choiceTopFilter->SetSelection(0);
	wxCommandEvent temp;
	OnChoiceTopFilterChanged(temp);

	_CheckControlState();

}

// Handles a change in the animation group.
void FxCurveManagerDialog::OnChoiceAnimGroupChanged( wxCommandEvent& FxUnused(event) )
{
	_FillChoiceAnimation();
	_choiceAnimation->SetSelection(0);
	_FillListTarget();
	_CheckControlState();
}

// Handles a change in the animation.
void FxCurveManagerDialog::OnChoiceAnimationChanged( wxCommandEvent& FxUnused(event) )
{
	_FillListTarget();
	_CheckControlState();
}

// Handles a change in the top filter.
void FxCurveManagerDialog::OnChoiceTopFilterChanged( wxCommandEvent& FxUnused(event) )
{
	_FillChoiceBottomFilter();
	_choiceBottomFilter->SetSelection(0);
	_FillListSource();
	_CheckControlState();
}

// Handles a change in the second filter.
void FxCurveManagerDialog::OnChoiceBottomFilterChanged( wxCommandEvent& FxUnused(event) )
{
	_FillListSource();
	_CheckControlState();
}

// Handles a selection change in either list
void FxCurveManagerDialog::OnListSelectionChanged( wxListEvent& FxUnused(event) )
{
	_CheckControlState();
}

// Handles a request to remove curves from the target animation.
void FxCurveManagerDialog::OnButtonRemoveCurves( wxCommandEvent& FxUnused(event) )
{
	FxArray<wxString> selectedCurves = _GetSelectedItems(_listTargetCurves);

	if( selectedCurves.Length() > 0 && _choiceAnimGroup && _choiceAnimation &&
		_choiceAnimGroup->GetSelection() != -1 && 
		_choiceAnimation->GetSelection() != -1 )
	{
		// Get the animation group and animation.
		wxString animGroup = _choiceAnimGroup->GetString(_choiceAnimGroup->GetSelection());
		wxString animation = _choiceAnimation->GetString(_choiceAnimation->GetSelection());

		// Batch the commands if necessary.
		if( selectedCurves.Length() > 1 )
		{
			FxVM::ExecuteCommand("batch");
		}
		for( FxSize i = 0; i < selectedCurves.Length(); ++i )
		{
			wxString command = wxString::Format(wxT("curve -group \"%s\" -anim \"%s\" -remove -name \"%s\""),
				animGroup, animation, selectedCurves[i]);
			FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
		}
		if( selectedCurves.Length() > 1 )
		{
			FxVM::ExecuteCommand("execBatch -removedcurves");
		}
	}
	// Rebuild the target list.
	_FillListTarget();
	_CheckControlState();
}

// Handles a request to add a curve to the current target animation.
void FxCurveManagerDialog::OnButtonAddCurrentAnim( wxCommandEvent& FxUnused(event) )
{
	if( _choiceAnimGroup && _choiceAnimation && _listSourceCurves && 
		_choiceAnimGroup->GetSelection() != -1 &&
		_choiceAnimation->GetSelection() != -1 )
	{
		FxString animGroupStr(_choiceAnimGroup->GetString(_choiceAnimGroup->GetSelection()).mb_str());
		FxName animGroup(animGroupStr.GetData());
		FxString animationStr(_choiceAnimation->GetString(_choiceAnimation->GetSelection()).mb_str());
		FxName animation(animationStr.GetData());
		FxArray<wxString> selectedCurves = _GetSelectedItems(_listSourceCurves);
		FxArray<FxBool> selectedCurveOwner = _GetSelectedItemsOwners();
		if( selectedCurveOwner.Length() == selectedCurves.Length() )
		{
			if( selectedCurves.Length() > 1 )
			{
				FxVM::ExecuteCommand("batch");
			}
			for( FxSize i = 0; i < selectedCurves.Length(); ++i )
			{
				_AddCurveToAnim(animGroup, animation, FxString(selectedCurves[i].mb_str(wxConvLibc)).GetData(), selectedCurveOwner[i]);
			}
			if( selectedCurves.Length() > 1 )
			{
				FxVM::ExecuteCommand("execBatch -addedcurves");
			}
			_FillListTarget();
			_CheckControlState();
		}
	}
}

// Handles a request to add a curve to the current animation group.
void FxCurveManagerDialog::OnButtonAddCurrentGroup( wxCommandEvent& FxUnused(event) )
{
	if( _choiceAnimGroup && _choiceAnimation && _listSourceCurves && 
		_choiceAnimGroup->GetSelection() != -1 &&
		_choiceAnimation->GetSelection() != -1 )
	{
		FxString animGroupStr(_choiceAnimGroup->GetString(_choiceAnimGroup->GetSelection()).mb_str());
		FxName animGroup(animGroupStr.GetData());
		FxArray<wxString> selectedCurves = _GetSelectedItems(_listSourceCurves);
		FxArray<FxBool> selectedCurveOwner = _GetSelectedItemsOwners();
		if( selectedCurveOwner.Length() == selectedCurves.Length() )
		{
			FxVM::ExecuteCommand("batch");
			for( FxSize i = 0; i < selectedCurves.Length(); ++i )
			{
				_AddCurveToGroup(animGroup, FxString(selectedCurves[i].mb_str(wxConvLibc)).GetData(), selectedCurveOwner[i]);
			}
			FxVM::ExecuteCommand("execBatch -addedcurves");
			_FillListTarget();
			_CheckControlState();
		}
	}
}

// Handles a request to add a curve to all animation groups.
void FxCurveManagerDialog::OnButtonAddAllGroups( wxCommandEvent& FxUnused(event) )
{
	if( _choiceAnimGroup && _choiceAnimation && _listSourceCurves && 
		_choiceAnimGroup->GetSelection() != -1 &&
		_choiceAnimation->GetSelection() != -1 )
	{
		FxArray<wxString> selectedCurves = _GetSelectedItems(_listSourceCurves);
		FxArray<FxBool> selectedCurveOwner = _GetSelectedItemsOwners();
		if( selectedCurveOwner.Length() == selectedCurves.Length() )
		{
			FxVM::ExecuteCommand("batch");
			for( FxSize i = 0; i < selectedCurves.Length(); ++i )
			{
				_AddCurveToAllGroups(FxString(selectedCurves[i].mb_str(wxConvLibc)).GetData(), selectedCurveOwner[i]);
			}
			FxVM::ExecuteCommand("execBatch -addedcurves");
			_FillListTarget();
			_CheckControlState();
		}
	}
}

// Handles the OK button press.
void FxCurveManagerDialog::OnButtonOK( wxCommandEvent& FxUnused(event) )
{
	EndDialog(wxID_OK);
}

// Handles a column click in the target names list
void FxCurveManagerDialog::OnListTargetNamesColumnClick( wxListEvent& FxUnused(event) )
{
	static FxBool sortAscending = FxTrue;

	if( _pActor && _listTargetCurves )
	{
		// Tag the animations.
		FxInt32 index = -1;
		for( ;; )
		{
			index = _listTargetCurves->GetNextItem(index);

			if( index == -1 ) break;

			FxString animGroupStr(_choiceAnimGroup->GetString(_choiceAnimGroup->GetSelection()).mb_str());
			FxString animationStr(_choiceAnimation->GetString(_choiceAnimation->GetSelection()).mb_str());
			FxSize animGroupIndex = _pActor->FindAnimGroup(animGroupStr.GetData());

			if( FxInvalidIndex != animGroupIndex )
			{
				FxAnimGroup& animGroup = _pActor->GetAnimGroup(animGroupIndex);
				FxSize animIndex = animGroup.FindAnim(animationStr.GetData());
				if( FxInvalidIndex != animGroupIndex )
				{
					const FxAnim& anim = animGroup.GetAnim(animIndex);
					FxName curveName(FxString(_listTargetCurves->GetItemText(index).mb_str(wxConvLibc)).GetData());
					FxSize curveIndex = anim.FindAnimCurve(curveName);
					if (FxInvalidIndex != curveIndex )
					{
						_listTargetCurves->SetItemData(index, reinterpret_cast<long>(&anim.GetAnimCurve(curveIndex).GetName()));
					}
				}
			}
		}
		_listTargetCurves->SortItems(CompareNames, static_cast<long>(sortAscending));
		sortAscending = !sortAscending;
	}
}


// Handles a column click in the source names list
void FxCurveManagerDialog::OnListSourceNamesColumnClick( wxListEvent& FxUnused(event) )
{
	static FxBool sortAscending = FxTrue;

	if( _pActor && _listSourceCurves )
	{
		if( _choiceTopFilter->GetSelection() > 0 )
		{
			// Tag the animations.
			FxInt32 index = -1;
			for( ;; )
			{
				index = _listSourceCurves->GetNextItem(index);

				if( index == -1 ) break;

				FxString animGroupStr(_choiceTopFilter->GetString(_choiceTopFilter->GetSelection()).mb_str());
				animGroupStr = animGroupStr.AfterFirst(':'); // Trim the "Anim Group: "
				animGroupStr = animGroupStr.AfterFirst(' ');
				FxSize animGroupIndex = _pActor->FindAnimGroup(animGroupStr.GetData());
				if( FxInvalidIndex != animGroupIndex )
				{
					FxAnimGroup& animGroup = _pActor->GetAnimGroup(animGroupIndex);
					FxString animationStr(_choiceBottomFilter->GetString(_choiceBottomFilter->GetSelection()).mb_str());
					FxSize   animIndex = animGroup.FindAnim(animationStr.GetData());
					if( FxInvalidIndex != animIndex )
					{
						const FxAnim& anim = animGroup.GetAnim(animIndex);
						FxName curveName(FxString(_listSourceCurves->GetItemText(index).mb_str(wxConvLibc)).GetData());
						FxSize curveIndex = anim.FindAnimCurve(curveName);
						if (FxInvalidIndex != curveIndex )
						{
							_listSourceCurves->SetItemData(index, reinterpret_cast<long>(&anim.GetAnimCurve(curveIndex).GetName()));
						}
					}
				}				
			}
			_listSourceCurves->SortItems(CompareNames, static_cast<long>(sortAscending));
		}
		else if( _choiceTopFilter->GetSelection() == 0 )
		{
			FxArray<FxString*> itemStrings;
			itemStrings.Reserve(_listSourceCurves->GetItemCount());
			// Tag the nodes.
			FxInt32 index = -1;
			for( ;; )
			{
				index = _listSourceCurves->GetNextItem(index);
				if( index == -1 ) break;
				FxString* itemString = new FxString(_listSourceCurves->GetItemText(index).mb_str(wxConvLibc));
				itemStrings.PushBack(itemString);
				_listSourceCurves->SetItemData(index, reinterpret_cast<long>(itemString));
			}
			_listSourceCurves->SortItems(CompareStrings, static_cast<long>(sortAscending));
			for( FxSize i = 0; i < itemStrings.Length(); ++i )
			{
				delete itemStrings[i];
				itemStrings[i] = NULL;
			}
			itemStrings.Clear();
		}

		sortAscending = !sortAscending;
	}
}

// Fills out the animation group choice
void FxCurveManagerDialog::_FillChoiceAnimGroup( void )
{
	if( _pActor && _choiceAnimGroup )
	{
		_choiceAnimGroup->Freeze();
		_choiceAnimGroup->Clear();
		for( FxSize i = 0; i < _pActor->GetNumAnimGroups(); ++i )
		{
			_choiceAnimGroup->Append(wxString(_pActor->GetAnimGroup(i).GetNameAsCstr(), wxConvLibc));
		}
		_choiceAnimGroup->Thaw();
	}
}

// Fills out the animation choice
void FxCurveManagerDialog::_FillChoiceAnimation( void )
{
	if( _pActor && _choiceAnimGroup && _choiceAnimation &&
		_choiceAnimGroup->GetSelection() != -1 )
	{
		FxString currentGroup(_choiceAnimGroup->GetString(_choiceAnimGroup->GetSelection()).mb_str(wxConvLibc));
		FxSize groupIndex = _pActor->FindAnimGroup(currentGroup.GetData());
		if( groupIndex != FxInvalidIndex )
		{
			FxAnimGroup& animGroup = _pActor->GetAnimGroup(groupIndex);
			_choiceAnimation->Freeze();
			_choiceAnimation->Clear();
			wxArrayString choices;
			for( FxSize i = 0; i < animGroup.GetNumAnims(); ++i )
			{
				choices.Add(wxString(animGroup.GetAnim(i).GetNameAsCstr(), wxConvLibc));
			}
			choices.Sort();
			_choiceAnimation->Append(choices);
			_choiceAnimation->Thaw();
		}
	}
}

// Fills out the top level filter choice
void FxCurveManagerDialog::_FillChoiceTopFilter( void )
{
	if( _pActor && _choiceTopFilter )
	{
		_choiceTopFilter->Freeze();
		_choiceTopFilter->Clear();
		_choiceTopFilter->Append(_("Face Graph"));
		for( FxSize i = 0; i < _pActor->GetNumAnimGroups(); ++i )
		{
			_choiceTopFilter->Append(_("Anim Group: ")+wxString(_pActor->GetAnimGroup(i).GetNameAsCstr(), wxConvLibc));
		}
		_choiceTopFilter->Thaw();
	}
}

// Fills out the second level filter choice
void FxCurveManagerDialog::_FillChoiceBottomFilter( void )
{
	if( _pActor && _choiceTopFilter && _choiceBottomFilter && 
		_choiceTopFilter->GetSelection() != -1 )
	{
		_choiceBottomFilter->Freeze();
		_choiceBottomFilter->Clear();
		FxInt32 selection = _choiceTopFilter->GetSelection();
		if( selection == 0 )
		{
			_choiceBottomFilter->Append(_("All Nodes"));
			_choiceBottomFilter->Append(_("All Nodes (Root only)"));
		}
		else
		{
			FxString animGroupStr(_choiceTopFilter->GetString(selection).mb_str());
			animGroupStr = animGroupStr.AfterFirst(':'); // Trim the "Anim Group: "
			animGroupStr = animGroupStr.AfterFirst(' ');
			FxSize   animGroupIndex = _pActor->FindAnimGroup(animGroupStr.GetData());
			if( FxInvalidIndex != animGroupIndex )
			{
				FxAnimGroup& animGroup = _pActor->GetAnimGroup(animGroupIndex);
				wxArrayString choices;
				for( FxSize i = 0; i < animGroup.GetNumAnims(); ++i )
				{
					choices.Add(wxString(animGroup.GetAnim(i).GetNameAsCstr(), wxConvLibc));
				}
				choices.Sort();
				_choiceBottomFilter->Append(choices);
			}
		}
		_choiceBottomFilter->Thaw();
	}
}

// Fills out the target name list
void FxCurveManagerDialog::_FillListTarget( void )
{
	// Get the current group and animation.
	if( _pActor && _choiceAnimGroup && _choiceAnimation &&
		_choiceAnimGroup->GetSelection() != -1 &&
		_choiceAnimation->GetSelection() != -1 )
	{
		FxString selectedAnimGroup(_choiceAnimGroup->GetString(_choiceAnimGroup->GetSelection()).mb_str(wxConvLibc));
		FxSize animGroupIndex = _pActor->FindAnimGroup(selectedAnimGroup.GetData());
		if( animGroupIndex != FxInvalidIndex )
		{
			FxAnimGroup& group = _pActor->GetAnimGroup(animGroupIndex);
			FxString selectedAnim(_choiceAnimation->GetString(_choiceAnimation->GetSelection()).mb_str(wxConvLibc));
			FxSize animIndex = group.FindAnim(selectedAnim.GetData());
			if( animIndex != FxInvalidIndex )
			{
				const FxAnim& anim = group.GetAnim(animIndex);
				_listTargetCurves->Freeze();
				_listTargetCurves->ClearAll();

				wxListItem columnInfo;
				columnInfo.m_mask = wxLIST_MASK_TEXT|wxLIST_MASK_WIDTH;
				columnInfo.m_text = _("Curve Name");
				columnInfo.m_width = GetClientRect().width;
				_listTargetCurves->InsertColumn(0, columnInfo);

				for( FxSize i = 0; i < anim.GetNumAnimCurves(); ++i )
				{
					_listTargetCurves->InsertItem(i, wxString::FromAscii(anim.GetAnimCurve(i).GetNameAsCstr()));
				}

				_listTargetCurves->Thaw();
			}
		}
	}
}

// Fills out the source name list
void FxCurveManagerDialog::_FillListSource()
{
	// Get the current filter type.
	if( _pActor && _choiceBottomFilter && _choiceTopFilter &&
		_choiceTopFilter->GetSelection() != -1 &&
		_choiceBottomFilter->GetSelection() != -1 )
	{
		_listSourceCurves->Freeze();
		_listSourceCurves->ClearAll();

		wxListItem columnInfo;
		columnInfo.m_mask = wxLIST_MASK_TEXT|wxLIST_MASK_WIDTH;
		columnInfo.m_text = _("Curve Name");
		columnInfo.m_width = GetClientRect().width;
		_listSourceCurves->InsertColumn(0, columnInfo);

		if( _choiceTopFilter->GetSelection() == 0 )
		{
			// Face graph
			FxFaceGraph& faceGraph = _pActor->GetDecompiledFaceGraph();
			FxSize count = 0;
			if( _choiceBottomFilter->GetSelection() == 0 )
			{
				// All nodes
				for( FxFaceGraph::Iterator curr = faceGraph.Begin(FxFaceGraphNode::StaticClass());
					 curr != faceGraph.End(FxFaceGraphNode::StaticClass());
					 ++curr)
				{
					_listSourceCurves->InsertItem(count++, wxString::FromAscii(curr.GetNode()->GetNameAsCstr()));
				}
			}
			else if( _choiceBottomFilter->GetSelection() == 1 )
			{
				// All root nodes
				for( FxFaceGraph::Iterator curr = faceGraph.Begin(FxFaceGraphNode::StaticClass());
					curr != faceGraph.End(FxFaceGraphNode::StaticClass());
					++curr)
				{
					if( curr.GetNode()->GetNumInputLinks() == 0 )
					{
						_listSourceCurves->InsertItem(count++, wxString::FromAscii(curr.GetNode()->GetNameAsCstr()));
					}
				}
			}
		}
		else
		{
			// Animation group
			FxString animGroupStr(_choiceTopFilter->GetString(_choiceTopFilter->GetSelection()).mb_str());
			animGroupStr = animGroupStr.AfterFirst(':'); // Trim the "Anim Group: "
			animGroupStr = animGroupStr.AfterFirst(' ');
			FxSize   animGroupIndex = _pActor->FindAnimGroup(animGroupStr.GetData());
			if( FxInvalidIndex != animGroupIndex )
			{
				FxAnimGroup& group = _pActor->GetAnimGroup(animGroupIndex);
				FxString animStr(_choiceBottomFilter->GetString(_choiceBottomFilter->GetSelection()).mb_str());
				FxSize   animIndex = group.FindAnim(animStr.GetData());
				if( FxInvalidIndex != animIndex )
				{
					const FxAnim& anim = group.GetAnim(animIndex);
					for( FxSize i = 0; i < anim.GetNumAnimCurves(); ++i )
					{
						_listSourceCurves->InsertItem(i, wxString::FromAscii(anim.GetAnimCurve(i).GetNameAsCstr()));
					}
				}
			}
		}

		_listSourceCurves->Thaw();
	}
}

// Properly enables/disables the controls on the dialog.
void FxCurveManagerDialog::_CheckControlState()
{
	FxInt32 numTargetSelected = _listTargetCurves->GetSelectedItemCount();
	FxInt32 numSourceSelected = _listSourceCurves->GetSelectedItemCount();
	_buttonRemoveCurves->Enable(numTargetSelected > 0);
	_buttonAddCurrentAnim->Enable(numSourceSelected > 0);
	_buttonAddCurrentGroup->Enable(numSourceSelected > 0);
	_buttonAddAllGroups->Enable(numSourceSelected > 0);
}

// Returns an array of the currently selected strings in a list control.
FxArray<wxString> FxCurveManagerDialog::_GetSelectedItems( wxListCtrl* pList )
{
	FxArray<wxString> retval;
	if( pList )
	{
		FxInt32 item = -1;
		for( ;; )
		{
			item = pList->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if( item == -1 )
				break;
			retval.PushBack( pList->GetItemText(item) );
		}
	}
	return retval;
}

// Returns an array of the currently selected string's owners (only for source)
FxArray<FxBool> FxCurveManagerDialog::_GetSelectedItemsOwners()
{
	FxArray<FxBool> retval;
	if( _pActor && _choiceBottomFilter && _choiceTopFilter && _listSourceCurves &&
		_choiceTopFilter->GetSelection() != -1 &&
		_choiceBottomFilter->GetSelection() != -1 )
	{
		FxArray<wxString> selectedItems = _GetSelectedItems(_listSourceCurves);
		retval.Reserve(selectedItems.Length());

		FxBool fromFaceGraph = _choiceTopFilter->GetSelection() == 0;
		FxAnimUserData* pUserData = NULL;
		if( !fromFaceGraph )
		{
			FxString animGroupStr(_choiceTopFilter->GetString(_choiceTopFilter->GetSelection()).mb_str());
			animGroupStr = animGroupStr.AfterFirst(':'); // Trim the "Anim Group: "
			animGroupStr = animGroupStr.AfterFirst(' ');
			FxSize   animGroupIndex = _pActor->FindAnimGroup(animGroupStr.GetData());
			if( FxInvalidIndex != animGroupIndex )
			{
				FxAnimGroup& group = _pActor->GetAnimGroup(animGroupIndex);
				FxString animStr(_choiceBottomFilter->GetString(_choiceBottomFilter->GetSelection()).mb_str());
				FxSize   animIndex = group.FindAnim(animStr.GetData());
				if( FxInvalidIndex != animIndex )
				{
					FxAnim* pAnim = group.GetAnimPtr(animIndex);
					pUserData = static_cast<FxAnimUserData*>(pAnim->GetUserData());
				}
			}
		}

		for( FxSize i = 0; i < selectedItems.Length(); ++i )
		{
			FxBool owner = FxFalse;
			if( fromFaceGraph )
			{
				// Set the owner to whether or not the given node is in the mapping.
				// Because I don't have access to the session from here, I'll use the proxy.
				owner = FxSessionProxy::IsTrackInMapping(FxString(selectedItems[i].mb_str(wxConvLibc)));
			}
			else if( pUserData )
			{
				// Otherwise, copy the owner from the selected track in the
				// source animation.
				owner = pUserData->IsCurveOwnedByAnalysis(FxString(selectedItems[i].mb_str(wxConvLibc)).GetData());
			}
			retval.PushBack(owner);
		}
	}
	return retval;
}

// Adds a curve to a specified animation.
void FxCurveManagerDialog::_AddCurveToAnim( const FxName& animGroup, const FxName& animation, const FxName& curve, FxBool ownedByAnalysis )
{
	if( _choiceTopFilter->GetSelection() > 0 )
	{
		FxString sourceAnimGroup(_choiceTopFilter->GetString(_choiceTopFilter->GetSelection()).mb_str());
		sourceAnimGroup = sourceAnimGroup.AfterFirst(':'); // Trim the "Anim Group: "
		sourceAnimGroup = sourceAnimGroup.AfterFirst(' ');
		FxSize   animGroupIndex = _pActor->FindAnimGroup(sourceAnimGroup.GetData());
		if( FxInvalidIndex != animGroupIndex )
		{
			FxAnimGroup& group = _pActor->GetAnimGroup(animGroupIndex);
			FxString sourceAnim(_choiceBottomFilter->GetString(_choiceBottomFilter->GetSelection()).mb_str());
			FxSize   animIndex = group.FindAnim(sourceAnim.GetData());
			if( FxInvalidIndex != animIndex )
			{
				wxString command = wxString::Format(
					wxT("curve -copy -sourcegroup \"%s\" -sourceanim \"%s\" -sourcename \"%s\" -group \"%s\" -anim \"%s\" -name \"%s\""),
					wxString(sourceAnimGroup.GetData(), wxConvLibc),
					wxString(sourceAnim.GetData(), wxConvLibc),
					wxString(curve.GetAsCstr(), wxConvLibc),
					wxString(animGroup.GetAsCstr(), wxConvLibc), 
					wxString(animation.GetAsCstr(), wxConvLibc),
					wxString(curve.GetAsCstr(), wxConvLibc));
				FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
			}
		}
	}
	else
	{
		wxString owner = ownedByAnalysis ? wxT("analysis") : wxT("user");
		wxString command = wxString::Format(wxT("curve -group \"%s\" -anim \"%s\" -add -name \"%s\" -owner \"%s\""),
			wxString(animGroup.GetAsCstr(), wxConvLibc), wxString(animation.GetAsCstr(), wxConvLibc),
			wxString(curve.GetAsCstr(), wxConvLibc), owner );
		FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
	}
}

// Adds a curve to a specified group.
void FxCurveManagerDialog::_AddCurveToGroup( const FxName& animGroup, const FxName& curve, FxBool ownedByAnalysis )
{
	if( _pActor )
	{
		FxSize groupIndex = _pActor->FindAnimGroup(animGroup);
		if( groupIndex != FxInvalidIndex )
		{
			FxAnimGroup& currAnimGroup = _pActor->GetAnimGroup(groupIndex);
			for( FxSize anim = 0; anim < currAnimGroup.GetNumAnims(); ++anim )
			{
				_AddCurveToAnim(animGroup, currAnimGroup.GetAnim(anim).GetName(), curve, ownedByAnalysis);
			}
		}
	}
}

// Adds a curve to each group.
void FxCurveManagerDialog::_AddCurveToAllGroups( const FxName& curve, FxBool ownedByAnalysis )
{
	if( _pActor )
	{
		for( FxSize group = 0; group < _pActor->GetNumAnimGroups(); ++group )
		{
			_AddCurveToGroup(_pActor->GetAnimGroup(group).GetName() , curve, ownedByAnalysis);
		}
	}
}

} // namespace Face

} // namespace OC3Ent
