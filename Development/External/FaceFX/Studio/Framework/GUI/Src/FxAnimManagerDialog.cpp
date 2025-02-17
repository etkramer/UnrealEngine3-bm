//------------------------------------------------------------------------------
// Face Graph node group manager dialog.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxAnimManagerDialog.h"
#include "FxUtilityFunctions.h"
#include "FxCurveManagerDialog.h"
#include "FxStudioSession.h"
#include "FxVM.h"
#include "FxTearoffWindow.h"
#include "FxCreateAnimWizard.h"
#include "FxAnimSetManager.h"

namespace OC3Ent
{

namespace Face
{

WX_IMPLEMENT_DYNAMIC_CLASS(FxAnimManagerDialog, wxDialog)

BEGIN_EVENT_TABLE(FxAnimManagerDialog, wxDialog)
	EVT_CHOICE(ControlID_AnimManagerDlgPrimaryAnimGroupChoice, FxAnimManagerDialog::OnPrimaryAnimGroupChanged)
	EVT_CHOICE(ControlID_AnimManagerDlgCopyFromPrimaryToSecondaryButton, FxAnimManagerDialog::OnSecondaryAnimGroupChanged)
	EVT_LIST_COL_CLICK(ControlID_AnimManagerDlgPrimaryAnimListCtrl, FxAnimManagerDialog::OnPrimaryAnimListCtrlColumnClick)
	EVT_LIST_COL_CLICK(ControlID_AnimManagerDlgSecondaryAnimListCtrl, FxAnimManagerDialog::OnSecondaryAnimListCtrlColumnClick)
	EVT_LIST_ITEM_SELECTED(ControlID_AnimManagerDlgPrimaryAnimListCtrl, FxAnimManagerDialog::OnPrimaryAnimListCtrlSelChanged)
	EVT_LIST_ITEM_DESELECTED(ControlID_AnimManagerDlgPrimaryAnimListCtrl, FxAnimManagerDialog::OnPrimaryAnimListCtrlSelChanged)
	EVT_LIST_ITEM_SELECTED(ControlID_AnimManagerDlgSecondaryAnimListCtrl, FxAnimManagerDialog::OnSecondaryAnimListCtrlSelChanged)
	EVT_LIST_ITEM_DESELECTED(ControlID_AnimManagerDlgSecondaryAnimListCtrl, FxAnimManagerDialog::OnSecondaryAnimListCtrlSelChanged)
		
	EVT_BUTTON(ControlID_AnimManagerDlgNewGroupButton, FxAnimManagerDialog::OnNewGroup)
	EVT_BUTTON(ControlID_AnimManagerDlgDeleteGroupButton, FxAnimManagerDialog::OnDeleteGroup)
	EVT_BUTTON(ControlID_AnimManagerDlgDeleteAnimButton, FxAnimManagerDialog::OnDeleteAnim)
	EVT_BUTTON(ControlID_AnimManagerDlgCreateAnimButton, FxAnimManagerDialog::OnCreateAnim)
	EVT_BUTTON(ControlID_AnimManagerDlgCurveManagerButton, FxAnimManagerDialog::OnCurveManager)
	EVT_BUTTON(ControlID_AnimManagerDlgMoveFromSecondaryToPrimaryButton, FxAnimManagerDialog::OnMoveFromSecondaryToPrimary)
	EVT_BUTTON(ControlID_AnimManagerDlgMoveFromPrimaryToSecondaryButton, FxAnimManagerDialog::OnMoveFromPrimaryToSecondary)
	EVT_BUTTON(ControlID_AnimManagerDlgCopyFromSecondaryToPrimaryButton, FxAnimManagerDialog::OnCopyFromSecondaryToPrimary)
	EVT_BUTTON(ControlID_AnimManagerDlgCopyFromPrimaryToSecondaryButton, FxAnimManagerDialog::OnCopyFromPrimaryToSecondary)
	EVT_BUTTON(ControlID_AnimManagerDlgOKButton, FxAnimManagerDialog::OnOK)

	EVT_CLOSE(FxAnimManagerDialog::OnClose)
END_EVENT_TABLE()

FxAnimManagerDialog::FxAnimManagerDialog()
	: _pActor(NULL)
	, _pMediator(NULL)
{
}

FxAnimManagerDialog::FxAnimManagerDialog( wxWindow* parent, wxWindowID id, 
	                                      const wxString& caption, 
										  const wxPoint& pos, 
										  const wxSize& size, long style )
										  : _pActor(NULL)
										  , _pMediator(NULL)
{
	Create(parent, id, caption, pos, size, style);
}

FxBool FxAnimManagerDialog::Create( wxWindow* parent, wxWindowID id, 
								    const wxString& caption, const wxPoint& pos, 
									const wxSize& size, long style )
{
	_primaryAnimGroupDropdown	= NULL;
	_newGroupButton				= NULL;
	_deleteGroupButton			= NULL;
	_primaryAnimListCtrl		= NULL;
	_deleteAnimButton			= NULL;
	_createAnimButton		    = NULL;
	_curveManagerButton			= NULL;
	_moveFromSecondaryToPrimaryButton = NULL;
	_moveFromPrimaryToSecondaryButton = NULL;
	_copyFromSecondaryToPrimaryButton = NULL;
	_copyFromPrimaryToSecondaryButton = NULL;
	_secondaryAnimGroupDropdown = NULL;
	_secondaryAnimListCtrl		= NULL;
	_okButton					= NULL;

	_primaryAnimsSelected   = FxFalse;
	_secondaryAnimsSelected = FxFalse;

	SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
	SetExtraStyle(GetExtraStyle()|wxWS_EX_VALIDATE_RECURSIVELY);
	wxDialog::Create(parent, id, caption, pos, size, style);

	CreateControls();
	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();

	return TRUE;
}

void FxAnimManagerDialog::Initialize( FxActor* actor, FxWidgetMediator* mediator )
{
	_pActor    = actor;
	_pMediator = mediator;
}

void FxAnimManagerDialog::CreateControls( void )
{
	wxBoxSizer* boxSizerV = new wxBoxSizer(wxVERTICAL);
	SetSizer(boxSizerV);
	SetAutoLayout(TRUE);

	wxBoxSizer* boxSizerH = new wxBoxSizer(wxHORIZONTAL);
	boxSizerV->Add(boxSizerH, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	wxStaticBox* primaryStaticText = new wxStaticBox(this, wxID_ANY, _("Primary"));
	wxStaticBoxSizer* primaryGroupBox = new wxStaticBoxSizer(primaryStaticText, wxVERTICAL);
	boxSizerH->Add(primaryGroupBox, 0, wxGROW|wxALL, 5);

	wxStaticText* primaryAnimGroupLabel = new wxStaticText(this, wxID_STATIC, _("Animation Group"), wxDefaultPosition, wxDefaultSize, 0);
	primaryGroupBox->Add(primaryAnimGroupLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

	_primaryAnimGroupDropdown = new wxChoice(this, ControlID_AnimManagerDlgPrimaryAnimGroupChoice, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	_primaryAnimGroupDropdown->SetToolTip(_("Primary animation group."));
	primaryGroupBox->Add(_primaryAnimGroupDropdown, 0, wxGROW|wxALL, 5);

	wxBoxSizer* groupButtonsBoxSizerH = new wxBoxSizer(wxHORIZONTAL);
	primaryGroupBox->Add(groupButtonsBoxSizerH, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	_newGroupButton = new wxButton(this, ControlID_AnimManagerDlgNewGroupButton, _("New Group..."), wxDefaultPosition, wxDefaultSize, 0);
	_newGroupButton->SetToolTip(_("Create a new animation group."));
	groupButtonsBoxSizerH->Add(_newGroupButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	_deleteGroupButton = new wxButton(this, ControlID_AnimManagerDlgDeleteGroupButton, _("Delete Group"), wxDefaultPosition, wxDefaultSize, 0);
	_deleteGroupButton->SetToolTip(_("Delete the selected animation group."));
	groupButtonsBoxSizerH->Add(_deleteGroupButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* primaryAnimationsLabel = new wxStaticText(this, wxID_STATIC, _("Animations"), wxDefaultPosition, wxDefaultSize, 0);
	primaryGroupBox->Add(primaryAnimationsLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

	_primaryAnimListCtrl = new wxListCtrl(this, ControlID_AnimManagerDlgPrimaryAnimListCtrl, wxDefaultPosition, wxSize(225, 300), wxLC_REPORT);
	_primaryAnimListCtrl->SetToolTip(_("Animations in the primary group."));
	primaryGroupBox->Add(_primaryAnimListCtrl, 0, wxGROW|wxALL, 5);

	_deleteAnimButton = new wxButton(this, ControlID_AnimManagerDlgDeleteAnimButton, _("&Delete Animation(s)"), wxDefaultPosition, wxDefaultSize, 0);
	_deleteAnimButton->SetToolTip(_("Delete the selected animation(s)."));
	primaryGroupBox->Add(_deleteAnimButton, 0, wxGROW|wxALL, 0);

	_createAnimButton = new wxButton(this, ControlID_AnimManagerDlgCreateAnimButton, _("&Create Animation..."), wxDefaultPosition, wxDefaultSize, 0);
	_createAnimButton->SetToolTip(_("Create a new animation and put it in the current animation group."));
	primaryGroupBox->Add(_createAnimButton, 0, wxGROW|wxALL, 0);

	_curveManagerButton = new wxButton(this, ControlID_AnimManagerDlgCurveManagerButton, _("Curve &Manager..."), wxDefaultPosition, wxDefaultSize, 0);
	_curveManagerButton->SetToolTip(_("Display the curve manager for the selected animation."));
	primaryGroupBox->Add(_curveManagerButton, 0, wxGROW|wxALL, 0);

	wxBoxSizer* centerBoxSizerV = new wxBoxSizer(wxVERTICAL);
	boxSizerH->Add(centerBoxSizerV, 0, wxGROW|wxALL, 5);

	// Spacers.
	centerBoxSizerV->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	centerBoxSizerV->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	centerBoxSizerV->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	centerBoxSizerV->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	centerBoxSizerV->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	centerBoxSizerV->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	centerBoxSizerV->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	centerBoxSizerV->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	centerBoxSizerV->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	centerBoxSizerV->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	centerBoxSizerV->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	centerBoxSizerV->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	centerBoxSizerV->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	centerBoxSizerV->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	centerBoxSizerV->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	centerBoxSizerV->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	centerBoxSizerV->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	_moveFromSecondaryToPrimaryButton = new wxButton(this, ControlID_AnimManagerDlgMoveFromSecondaryToPrimaryButton, _("<- Move"), wxDefaultPosition, wxDefaultSize, 0);
	_moveFromSecondaryToPrimaryButton->SetToolTip(_("Move the selected animations from the secondary group to the primary group."));
	centerBoxSizerV->Add(_moveFromSecondaryToPrimaryButton, 0, wxGROW|wxALL, 5);

	_moveFromPrimaryToSecondaryButton = new wxButton(this, ControlID_AnimManagerDlgMoveFromPrimaryToSecondaryButton, _("Move ->"), wxDefaultPosition, wxDefaultSize, 0);
	_moveFromPrimaryToSecondaryButton->SetToolTip(_("Move the selected animations from the primary group to the secondary group."));
	centerBoxSizerV->Add(_moveFromPrimaryToSecondaryButton, 0, wxGROW|wxALL, 5);

	_copyFromSecondaryToPrimaryButton = new wxButton(this, ControlID_AnimManagerDlgCopyFromSecondaryToPrimaryButton, _("<- Copy"), wxDefaultPosition, wxDefaultSize, 0);
	_copyFromSecondaryToPrimaryButton->SetToolTip(_("Copy the selected animations from the secondary group to the primary group."));
	centerBoxSizerV->Add(_copyFromSecondaryToPrimaryButton, 0, wxGROW|wxALL, 5);

	_copyFromPrimaryToSecondaryButton = new wxButton(this, ControlID_AnimManagerDlgCopyFromPrimaryToSecondaryButton, _("Copy ->"), wxDefaultPosition, wxDefaultSize, 0);
	_copyFromPrimaryToSecondaryButton->SetToolTip(_("Copy the selected animations from the primary group to the secondary group."));
	centerBoxSizerV->Add(_copyFromPrimaryToSecondaryButton, 0, wxGROW|wxALL, 5);

	wxStaticBox* secondaryStaticText = new wxStaticBox(this, wxID_ANY, _("Secondary"));
	wxStaticBoxSizer* secondaryGroupBox = new wxStaticBoxSizer(secondaryStaticText, wxVERTICAL);
	boxSizerH->Add(secondaryGroupBox, 0, wxGROW|wxALL, 5);

	wxStaticText* secondaryAnimGroupLabel = new wxStaticText(this, wxID_STATIC, _("Animation Group"), wxDefaultPosition, wxDefaultSize, 0);
	secondaryGroupBox->Add(secondaryAnimGroupLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

	_secondaryAnimGroupDropdown = new wxChoice(this, ControlID_AnimManagerDlgCopyFromPrimaryToSecondaryButton, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	_secondaryAnimGroupDropdown->SetToolTip(_("Secondary animation group."));
	secondaryGroupBox->Add(_secondaryAnimGroupDropdown, 0, wxGROW|wxALL, 5);

	// Spacers.
	secondaryGroupBox->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	secondaryGroupBox->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	secondaryGroupBox->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	wxStaticText* secondaryAnimationsLabel = new wxStaticText(this, wxID_STATIC, _("Animations"), wxDefaultPosition, wxDefaultSize, 0);
	secondaryGroupBox->Add(secondaryAnimationsLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

	_secondaryAnimListCtrl = new wxListCtrl(this, ControlID_AnimManagerDlgSecondaryAnimListCtrl, wxDefaultPosition, wxSize(225, 300), wxLC_REPORT);
	_secondaryAnimListCtrl->SetToolTip(_("Animations in the secondary group."));
	secondaryGroupBox->Add(_secondaryAnimListCtrl, 0, wxGROW|wxALL, 5);

	_okButton = new wxButton(this, ControlID_AnimManagerDlgOKButton, _("&OK"), wxDefaultPosition, wxDefaultSize, 0);
	_okButton->SetToolTip(_("Dismiss the dialog."));
	boxSizerV->Add(_okButton, 0, wxALIGN_RIGHT|wxALL, 5);

	// Create an invisible cancel button.
	// This will probably land me in programmer's hell at some point.
	// I just want escape to make the dumb dialog go away.
	wxButton* cancelButton = new wxButton(this, wxID_CANCEL, _("Cancel"), wxPoint(0,0), wxSize(0,0));
	cancelButton;

	_deleteGroupButton->Enable(FALSE);
	_deleteAnimButton->Enable(FALSE);
	_curveManagerButton->Enable(FALSE);
	_moveFromPrimaryToSecondaryButton->Enable(FALSE);
	_moveFromSecondaryToPrimaryButton->Enable(FALSE);
	_copyFromPrimaryToSecondaryButton->Enable(FALSE);
	_copyFromSecondaryToPrimaryButton->Enable(FALSE);
	
	if( _pActor )
	{
		_fillGroupDropdowns();
		_primaryAnimGroupDropdown->SetSelection(0);
		_secondaryAnimGroupDropdown->SetSelection(0);
		_primaryGroup   = _primaryAnimGroupDropdown->GetString(0);
		_secondaryGroup = _secondaryAnimGroupDropdown->GetString(0);
		_fillPrimaryAnimListCtrl();
		_fillSecondaryAnimListCtrl();
	}
	else
	{
		_primaryAnimGroupDropdown->Enable(FALSE);
		_newGroupButton->Enable(FALSE);
		_primaryAnimListCtrl->Enable(FALSE);
		_secondaryAnimGroupDropdown->Enable(FALSE);
		_secondaryAnimListCtrl->Enable(FALSE);
	}
}

FxBool FxAnimManagerDialog::ShowToolTips( void )
{
	return FxTrue;
}

void FxAnimManagerDialog::OnPrimaryAnimGroupChanged( wxCommandEvent& FxUnused(event) )
{
	if( _primaryAnimGroupDropdown )
	{
		_primaryGroup = _primaryAnimGroupDropdown->GetString(_primaryAnimGroupDropdown->GetSelection());
		_primaryAnimsSelected = FxFalse;

		// Enable the delete group button?
		if( _deleteGroupButton )
		{
			if( _primaryGroup != wxString(FxAnimGroup::Default.GetAsCstr(), wxConvLibc) &&
				FxInvalidIndex == FxAnimSetManager::FindMountedAnimSet(FxString(_primaryGroup.mb_str(wxConvLibc)).GetData()) )
			{
				_deleteGroupButton->Enable(TRUE);	
			}
			else
			{
				_deleteGroupButton->Enable(FALSE);
			}
		}

		// Enable the transfer buttons?
		_enableTransferButtons();

		// Fill out the primary animation list control.
		_fillPrimaryAnimListCtrl();

		// Disable the delete animations and curve manager buttons.
		if( _deleteAnimButton && _curveManagerButton )
		{
			_deleteAnimButton->Enable(FALSE);
			_curveManagerButton->Enable(FALSE);
		}
	}
}

void FxAnimManagerDialog::OnSecondaryAnimGroupChanged( wxCommandEvent& FxUnused(event) )
{
	if( _secondaryAnimGroupDropdown )
	{
		_secondaryGroup = _secondaryAnimGroupDropdown->GetString(_secondaryAnimGroupDropdown->GetSelection());
		_secondaryAnimsSelected = FxFalse;

		// Enable the transfer buttons?
		_enableTransferButtons();

		// Fill out the secondary animation list control.
		_fillSecondaryAnimListCtrl();

		// Disable the delete animations and curve manager buttons.
		if( _deleteAnimButton && _curveManagerButton )
		{
			_deleteAnimButton->Enable(FALSE);
			_curveManagerButton->Enable(FALSE);
		}
	}
}

void FxAnimManagerDialog::OnPrimaryAnimListCtrlColumnClick( wxListEvent& FxUnused(event) )
{
	static FxBool sortAscending = FxTrue;

	if( _pActor && _primaryAnimListCtrl )
	{
		// Tag the animations.
		FxInt32 index = -1;
		for( ;; )
		{
			index = _primaryAnimListCtrl->GetNextItem(index);

			if( index == -1 ) break;

			FxName animName(FxString(_primaryAnimListCtrl->GetItemText(index).mb_str(wxConvLibc)).GetData());
			FxSize animGroupIndex = _pActor->FindAnimGroup(FxString(_primaryGroup.mb_str(wxConvLibc)).GetData());
			if( FxInvalidIndex != animGroupIndex )
			{
				FxAnimGroup& animGroup = _pActor->GetAnimGroup(animGroupIndex);
				FxSize animIndex = animGroup.FindAnim(animName);
				if( FxInvalidIndex != animIndex )
				{
					const FxAnim& anim = animGroup.GetAnim(animIndex);
					_primaryAnimListCtrl->SetItemData(index, reinterpret_cast<long>(&anim.GetName()));
				}
			}
		}
		_primaryAnimListCtrl->SortItems(CompareNames, static_cast<long>(sortAscending));
		sortAscending = !sortAscending;
	}
}

void FxAnimManagerDialog::OnSecondaryAnimListCtrlColumnClick( wxListEvent& FxUnused(event) )
{
	static FxBool sortAscending = FxTrue;

	if( _pActor && _secondaryAnimListCtrl )
	{
		// Tag the animations.
		FxInt32 index = -1;
		for( ;; )
		{
			index = _secondaryAnimListCtrl->GetNextItem(index);

			if( index == -1 ) break;

			FxName animName(FxString(_secondaryAnimListCtrl->GetItemText(index).mb_str(wxConvLibc)).GetData());
			FxSize animGroupIndex = _pActor->FindAnimGroup(FxString(_secondaryGroup.mb_str(wxConvLibc)).GetData());
			if( FxInvalidIndex != animGroupIndex )
			{
				FxAnimGroup& animGroup = _pActor->GetAnimGroup(animGroupIndex);
				FxSize animIndex = animGroup.FindAnim(animName);
				if( FxInvalidIndex != animIndex )
				{
					const FxAnim& anim = animGroup.GetAnim(animIndex);
					_secondaryAnimListCtrl->SetItemData(index, reinterpret_cast<long>(&anim.GetName()));
				}
			}
		}
		_secondaryAnimListCtrl->SortItems(CompareNames, static_cast<long>(sortAscending));
		sortAscending = !sortAscending;
	}
}

void FxAnimManagerDialog::OnPrimaryAnimListCtrlSelChanged( wxListEvent& FxUnused(event) )
{
	// Check that there is at least one item selected.
	if( _primaryAnimListCtrl )
	{
		_primaryAnimsSelected = FxFalse;
		FxInt32 item = -1;
		for( ;; )
		{
			item = _primaryAnimListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if( item == -1 )
				break;
			_primaryAnimsSelected = FxTrue;
			break;
		}

		// Enable the transfer from primary group to secondary group button.
		_enableTransferButtons();

		// Enable the delete animation and curve manager buttons?
		if( _deleteAnimButton && _curveManagerButton )
		{
			if( _primaryAnimsSelected )
			{
				_deleteAnimButton->Enable(TRUE);
				_curveManagerButton->Enable(TRUE);
			}
			else
			{
				_deleteAnimButton->Enable(FALSE);
				_curveManagerButton->Enable(FALSE);
			}
		}
	}
}

void FxAnimManagerDialog::OnSecondaryAnimListCtrlSelChanged( wxListEvent& FxUnused(event) )
{
	// Check that there is at least one item selected.
	if( _secondaryAnimListCtrl )
	{
		_secondaryAnimsSelected = FxFalse;
		FxInt32 item = -1;
		for( ;; )
		{
			item = _secondaryAnimListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if( item == -1 )
				break;
			_secondaryAnimsSelected = FxTrue;
			break;
		}

		// Enable the transfer from primary group to secondary group button.
		_enableTransferButtons();
	}
}

void FxAnimManagerDialog::OnNewGroup( wxCommandEvent& FxUnused(event) )
{
	wxString newAnimGroupName = ::wxGetTextFromUser(_("New animation group name"), _("Create animation group"), wxEmptyString, this);

	if( newAnimGroupName != wxEmptyString )
	{
		wxString command = wxT("animGroup -create -group \"");
		command += newAnimGroupName;
		command += wxT("\"");
		if( CE_Success == FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc))) )
		{
			// Fill out the primary and secondary group drop downs.
			_fillGroupDropdowns();
			// Select the new group as the primary group.
			_primaryGroup = newAnimGroupName;
			_primaryAnimsSelected = FxFalse;
			if( _primaryAnimGroupDropdown )
			{
				_primaryAnimGroupDropdown->SetStringSelection(newAnimGroupName);
				_fillPrimaryAnimListCtrl();
			}
			// Select the old secondary group.
			_secondaryAnimsSelected = FxFalse;
			if( _secondaryAnimGroupDropdown )
			{
				_secondaryAnimGroupDropdown->SetStringSelection(_secondaryGroup);
				_fillSecondaryAnimListCtrl();
			}
			// Enable the transfer buttons?
			_enableTransferButtons();
		}
	}
}

void FxAnimManagerDialog::OnDeleteGroup( wxCommandEvent& FxUnused(event) )
{
	FxBool batched = FxFalse;
	if( _primaryAnimListCtrl )
	{
		// This should never be true, but prevent it and notify the user just
		// in case.
		if( _primaryGroup == wxString(FxAnimGroup::Default.GetAsCstr(), wxConvLibc) )
		{
			FxMessageBox(_("You cannot delete the default animation group!"), _("Logic Error!"));
			return;
		}
		// Ditto.
		if( FxInvalidIndex != FxAnimSetManager::FindMountedAnimSet(FxString(_primaryGroup.mb_str(wxConvLibc)).GetData()) )
		{
			FxMessageBox(_("You cannot delete a mounted animation group!"), _("Logic Error!"));
			return;
		}

		// If the group is not empty, prompt the user to remove all animations
		// in the group before removing the group.
		if( _primaryAnimListCtrl->GetItemCount() > 0 )
		{
			if( wxYES == FxMessageBox(_("This will delete all of the animations in the group.  Are you sure?"), _("Confirmation"), wxYES_NO) )
			{
				// If the user selected yes, batch up the entire series of commands
				// to remove all of the animations in the group and then the
				// group itself.
				batched = FxTrue;
				FxVM::ExecuteCommand("batch");

				FxInt32 item = -1;
				for( ;; )
				{
					item = _primaryAnimListCtrl->GetNextItem(item, wxLIST_NEXT_ALL);
					if( item == -1 )
						break;
					wxString command = wxT("anim -remove -group \"");
					command += _primaryGroup;
					command += wxT("\" -name \"");
					command += _primaryAnimListCtrl->GetItemText(item);
					command += wxT("\"");
					FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
				}
			}
			else
			{
				// If the user selected no, do nothing and return.
				return;
			}
		}
	}

	wxString command = wxT("animGroup -remove -group \"");
	command += _primaryGroup;
	command += wxT("\"");
	FxCommandError returnCode = CE_Failure;

	if( batched )
	{
		FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
		//@todo Fix this -editednodes flag hack!!
		returnCode = FxVM::ExecuteCommand("execBatch -editednodes");
	}
	else
	{
		returnCode = FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
	}

	if( CE_Success == returnCode )
	{
		// Fill out the primary and secondary group drop downs.
		_fillGroupDropdowns();
		if( _primaryAnimGroupDropdown && _secondaryAnimGroupDropdown )
		{
			// If the removed group was selected in the secondary group drop down,
			// reset that selection to the default group.
			if( _secondaryGroup == _primaryGroup )
			{
				_secondaryAnimGroupDropdown->SetSelection(0);
				_secondaryGroup = _secondaryAnimGroupDropdown->GetString(0);
			}
			else
			{
				_secondaryAnimGroupDropdown->SetStringSelection(_secondaryGroup);
			}
			_secondaryAnimsSelected = FxFalse;
			_fillSecondaryAnimListCtrl();
			// Select the default group in the primary drop down.
			_primaryAnimGroupDropdown->SetSelection(0);
			_primaryGroup = _primaryAnimGroupDropdown->GetString(0);
			_primaryAnimsSelected = FxFalse;
			_fillPrimaryAnimListCtrl();
			// Enable the delete group button?
			if( _deleteGroupButton )
			{
				if( _primaryGroup != wxString(FxAnimGroup::Default.GetAsCstr(), wxConvLibc) &&
					FxInvalidIndex == FxAnimSetManager::FindMountedAnimSet(FxString(_primaryGroup.mb_str(wxConvLibc)).GetData()) )
				{
					_deleteGroupButton->Enable(TRUE);	
				}
				else
				{
					_deleteGroupButton->Enable(FALSE);
				}
			}
			// Enable the transfer buttons?
			_enableTransferButtons();
		}
	}
}

void FxAnimManagerDialog::OnDeleteAnim( wxCommandEvent& FxUnused(event) )
{
	if( _primaryAnimListCtrl )
	{
		FxBool AreItemsSelected = FxFalse;
		FxInt32 item = -1;
		for( ;; )
		{
			item = _primaryAnimListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if( item == -1 )
				break;
			if( !AreItemsSelected )
			{
				AreItemsSelected = FxTrue;
				FxVM::ExecuteCommand("batch");
			}
			wxString command = wxT("anim -remove -group \"");
			command += _primaryGroup;
			command += wxT("\" -name \"");
			command += _primaryAnimListCtrl->GetItemText(item);
			command += wxT("\"");
			FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
		}

		if( AreItemsSelected )
		{
			//@todo Fix this -editednodes flag hack!!
			FxVM::ExecuteCommand("execBatch -editednodes");
		}

		_fillPrimaryAnimListCtrl();

		// Disable the transfer from primary group to secondary group button.
		_primaryAnimsSelected = FxFalse;
		_enableTransferButtons();

		// Enable the delete animation and curve manager buttons?
		if( _deleteAnimButton && _curveManagerButton )
		{
			_deleteAnimButton->Enable(FALSE);
			_curveManagerButton->Enable(FALSE);
		}
	}
}

void FxAnimManagerDialog::OnCreateAnim( wxCommandEvent& FxUnused(event) )
{
	if( FxRunCreateAnimWizard(this, FxString(_primaryGroup.mb_str(wxConvLibc))) )
	{
		_fillGroupDropdowns();

		// Reselect the groups.
		_primaryAnimGroupDropdown->SetStringSelection(_primaryGroup);
		_secondaryAnimGroupDropdown->SetStringSelection(_secondaryGroup);

		_primaryAnimsSelected   = FxFalse;
		_secondaryAnimsSelected = FxFalse;
		_fillPrimaryAnimListCtrl();
		_fillSecondaryAnimListCtrl();
		_enableTransferButtons();
	}
}

void FxAnimManagerDialog::OnCurveManager( wxCommandEvent& FxUnused(event) )
{
	if( _primaryAnimGroupDropdown && _primaryAnimListCtrl &&
		_primaryAnimGroupDropdown->GetSelection() != -1 &&
		_primaryAnimListCtrl->GetSelectedItemCount() > 0 )
	{
		FxString animGroup(_primaryAnimGroupDropdown->GetString(_primaryAnimGroupDropdown->GetSelection()).mb_str(wxConvLibc));
		int index = _primaryAnimListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		FxString animation(_primaryAnimListCtrl->GetItemText(index).mb_str(wxConvLibc));
		FxCurveManagerDialog curveManager;
		curveManager.Initialize(_pActor, _pMediator, animGroup.GetData(), animation.GetData());
		curveManager.Create(this);
		FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
		curveManager.ShowModal();
		FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
	}
}

void FxAnimManagerDialog::OnMoveFromSecondaryToPrimary( wxCommandEvent& FxUnused(event) )
{
	if( _secondaryAnimListCtrl )
	{
		FxInt32 item = -1;
		for( ;; )
		{
			item = _secondaryAnimListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if( item == -1 )
				break;
			wxString command = wxT("anim -move -from \"");
			command += _secondaryGroup;
			command += wxT("\" -to \"");
			command += _primaryGroup;
			command += wxT("\" -name \"");
			command += _secondaryAnimListCtrl->GetItemText(item);
			command += wxT("\"");
			// Do not try to batch these.  The user data stuff inside of the 
			// command depends on ActorDataChanged() being called because
			// adding or removing animations can potentially cause a reallocation.
			FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
		}

		_fillPrimaryAnimListCtrl();
		_fillSecondaryAnimListCtrl();

		_primaryAnimsSelected   = FxFalse;
		_secondaryAnimsSelected = FxFalse;
		_enableTransferButtons();

		// Enable the delete animation and curve manager buttons?
		if( _deleteAnimButton && _curveManagerButton )
		{
			_deleteAnimButton->Enable(FALSE);
			_curveManagerButton->Enable(FALSE);
		}
	}
}

void FxAnimManagerDialog::OnMoveFromPrimaryToSecondary( wxCommandEvent& FxUnused(event) )
{
	if( _primaryAnimListCtrl )
	{
		FxInt32 item = -1;
		for( ;; )
		{
			item = _primaryAnimListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if( item == -1 )
				break;
			wxString command = wxT("anim -move -from \"");
			command += _primaryGroup;
			command += wxT("\" -to \"");
			command += _secondaryGroup;
			command += wxT("\" -name \"");
			command += _primaryAnimListCtrl->GetItemText(item);
			command += wxT("\"");
			// Do not try to batch these.  The user data stuff inside of the 
			// command depends on ActorDataChanged() being called because
			// adding or removing animations can potentially cause a reallocation.
			FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
		}

		_fillPrimaryAnimListCtrl();
		_fillSecondaryAnimListCtrl();

		_primaryAnimsSelected   = FxFalse;
		_secondaryAnimsSelected = FxFalse;
		_enableTransferButtons();

		// Enable the delete animation and curve manager buttons?
		if( _deleteAnimButton && _curveManagerButton )
		{
			_deleteAnimButton->Enable(FALSE);
			_curveManagerButton->Enable(FALSE);
		}
	}
}

void FxAnimManagerDialog::OnCopyFromSecondaryToPrimary( wxCommandEvent& FxUnused(event) )
{
	if( _secondaryAnimListCtrl )
	{
		FxInt32 item = -1;
		for( ;; )
		{
			item = _secondaryAnimListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if( item == -1 )
				break;
			wxString command = wxT("anim -copy -from \"");
			command += _secondaryGroup;
			command += wxT("\" -to \"");
			command += _primaryGroup;
			command += wxT("\" -name \"");
			command += _secondaryAnimListCtrl->GetItemText(item);
			command += wxT("\"");
			// Do not try to batch these.  The user data stuff inside of the 
			// command depends on ActorDataChanged() being called because
			// adding or removing animations can potentially cause a reallocation.
			FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
		}

		_fillPrimaryAnimListCtrl();
		_fillSecondaryAnimListCtrl();

		_primaryAnimsSelected   = FxFalse;
		_secondaryAnimsSelected = FxFalse;
		_enableTransferButtons();

		// Enable the delete animation and curve manager buttons?
		if( _deleteAnimButton && _curveManagerButton )
		{
			_deleteAnimButton->Enable(FALSE);
			_curveManagerButton->Enable(FALSE);
		}
	}
}

void FxAnimManagerDialog::OnCopyFromPrimaryToSecondary( wxCommandEvent& FxUnused(event) )
{
	if( _primaryAnimListCtrl )
	{
		FxInt32 item = -1;
		for( ;; )
		{
			item = _primaryAnimListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if( item == -1 )
				break;
			wxString command = wxT("anim -copy -from \"");
			command += _primaryGroup;
			command += wxT("\" -to \"");
			command += _secondaryGroup;
			command += wxT("\" -name \"");
			command += _primaryAnimListCtrl->GetItemText(item);
			command += wxT("\"");
			// Do not try to batch these.  The user data stuff inside of the 
			// command depends on ActorDataChanged() being called because
			// adding or removing animations can potentially cause a reallocation.
			FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
		}

		_fillPrimaryAnimListCtrl();
		_fillSecondaryAnimListCtrl();

		_primaryAnimsSelected   = FxFalse;
		_secondaryAnimsSelected = FxFalse;
		_enableTransferButtons();

		// Enable the delete animation and curve manager buttons?
		if( _deleteAnimButton && _curveManagerButton )
		{
			_deleteAnimButton->Enable(FALSE);
			_curveManagerButton->Enable(FALSE);
		}
	}
}

void FxAnimManagerDialog::OnOK( wxCommandEvent& event )
{
	Close();
	event.Skip();
}

void FxAnimManagerDialog::OnClose( wxCloseEvent& event )
{
	event.Skip();
}

void FxAnimManagerDialog::_fillGroupDropdowns( void )
{
	if( _pActor && _primaryAnimGroupDropdown && _secondaryAnimGroupDropdown )
	{
		_primaryAnimGroupDropdown->Freeze();
		_secondaryAnimGroupDropdown->Freeze();
		_primaryAnimGroupDropdown->Clear();
		_secondaryAnimGroupDropdown->Clear();
		for( FxSize i = 0; i < _pActor->GetNumAnimGroups(); ++i )
		{
			_primaryAnimGroupDropdown->Append(wxString(_pActor->GetAnimGroup(i).GetNameAsCstr(), wxConvLibc));
			_secondaryAnimGroupDropdown->Append(wxString(_pActor->GetAnimGroup(i).GetNameAsCstr(), wxConvLibc));
		}
		_primaryAnimGroupDropdown->Thaw();
		_secondaryAnimGroupDropdown->Thaw();
	}
}

void FxAnimManagerDialog::_fillPrimaryAnimListCtrl( void )
{
	if( _pActor && _primaryAnimListCtrl )
	{
		_primaryAnimListCtrl->Freeze();
		_primaryAnimListCtrl->ClearAll();

		wxListItem columnInfo;
		columnInfo.m_mask = wxLIST_MASK_TEXT|wxLIST_MASK_WIDTH;
		columnInfo.m_text = _("Animation Name");
		columnInfo.m_width = GetClientRect().width;
		_primaryAnimListCtrl->InsertColumn(0, columnInfo);

		FxSize animGroupIndex = _pActor->FindAnimGroup(FxString(_primaryGroup.mb_str(wxConvLibc)).GetData());
		if( FxInvalidIndex != animGroupIndex )
		{
			FxAnimGroup& animGroup = _pActor->GetAnimGroup(animGroupIndex);
			for( FxSize i = 0; i < animGroup.GetNumAnims(); ++i )
			{
				const FxAnim& anim = animGroup.GetAnim(i);
				_primaryAnimListCtrl->InsertItem(i, wxString(anim.GetNameAsCstr(), wxConvLibc));
			}
		}
		_primaryAnimListCtrl->Thaw();
	}
}

void FxAnimManagerDialog::_fillSecondaryAnimListCtrl( void )
{
	if( _pActor && _secondaryAnimListCtrl )
	{
		_secondaryAnimListCtrl->Freeze();
		_secondaryAnimListCtrl->ClearAll();

		wxListItem columnInfo;
		columnInfo.m_mask = wxLIST_MASK_TEXT|wxLIST_MASK_WIDTH;
		columnInfo.m_text = _("Animation Name");
		columnInfo.m_width = GetClientRect().width;
		_secondaryAnimListCtrl->InsertColumn(0, columnInfo);

		FxSize animGroupIndex = _pActor->FindAnimGroup(FxString(_secondaryGroup.mb_str(wxConvLibc)).GetData());
		if( FxInvalidIndex != animGroupIndex )
		{
			FxAnimGroup& animGroup = _pActor->GetAnimGroup(animGroupIndex);
			for( FxSize i = 0; i < animGroup.GetNumAnims(); ++i )
			{
				const FxAnim& anim = animGroup.GetAnim(i);
				_secondaryAnimListCtrl->InsertItem(i, wxString(anim.GetNameAsCstr(), wxConvLibc));
			}
		}
		_secondaryAnimListCtrl->Thaw();
	}
}

void FxAnimManagerDialog::_enableTransferButtons( void )
{
	if( _moveFromPrimaryToSecondaryButton && _moveFromSecondaryToPrimaryButton &&
		_copyFromPrimaryToSecondaryButton && _copyFromSecondaryToPrimaryButton )
	{
		if( _primaryGroup != _secondaryGroup )
		{
			if( _primaryAnimsSelected )
			{
				_moveFromPrimaryToSecondaryButton->Enable(TRUE);
				_copyFromPrimaryToSecondaryButton->Enable(TRUE);
			}
			else
			{
				_moveFromPrimaryToSecondaryButton->Enable(FALSE);
				_copyFromPrimaryToSecondaryButton->Enable(FALSE);
			}

			if( _secondaryAnimsSelected )
			{
				_moveFromSecondaryToPrimaryButton->Enable(TRUE);
				_copyFromSecondaryToPrimaryButton->Enable(TRUE);
			}
			else
			{
				_moveFromSecondaryToPrimaryButton->Enable(FALSE);
				_copyFromSecondaryToPrimaryButton->Enable(FALSE);
			}
		}
		else
		{
			_moveFromPrimaryToSecondaryButton->Enable(FALSE);
			_moveFromSecondaryToPrimaryButton->Enable(FALSE);
			_copyFromPrimaryToSecondaryButton->Enable(FALSE);
			_copyFromSecondaryToPrimaryButton->Enable(FALSE);
		}
	}
}

} // namespace Face

} // namespace OC3Ent
