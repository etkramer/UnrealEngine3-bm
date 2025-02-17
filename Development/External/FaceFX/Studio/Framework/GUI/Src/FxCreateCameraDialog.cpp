//------------------------------------------------------------------------------
// Dialog used for creating viewport cameras.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxUtilityFunctions.h"
#include "FxCreateCameraDialog.h"
#ifdef IS_FACEFX_STUDIO
#include "FxSessionProxy.h"
#endif

namespace OC3Ent
{

namespace Face
{

WX_IMPLEMENT_DYNAMIC_CLASS(FxCreateCameraDialog, wxDialog)

BEGIN_EVENT_TABLE(FxCreateCameraDialog, wxDialog)
	EVT_CHECKBOX(ControlID_CreateCameraDlgBindToBoneCheck, FxCreateCameraDialog::OnBindToBoneChecked)
	EVT_LIST_COL_CLICK(ControlID_CreateCameraDlgBoneListBox, FxCreateCameraDialog::OnColumnClick)
	EVT_BUTTON(ControlID_CreateCameraDlgCreateButton, FxCreateCameraDialog::OnCreateButtonPressed)
	EVT_BUTTON(ControlID_CreateCameraDlgCancelButton, FxCreateCameraDialog::OnCancelButtonPressed)
END_EVENT_TABLE()

FxCreateCameraDialog::FxCreateCameraDialog()
	: _cameraName(NULL)
	, _usesFixedAspectRatio(NULL)
	, _isBoundToBone(NULL)
	, _boneName(NULL)
{
}

FxBool FxCreateCameraDialog::
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

void FxCreateCameraDialog::CreateControls( void )
{
	wxBoxSizer* boxSizerV = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(boxSizerV);
	this->SetAutoLayout(TRUE);

	wxStaticText* staticText = new wxStaticText(this, wxID_DEFAULT, _("Camera Name:"));
	boxSizerV->Add(staticText, 0, wxALL|wxALIGN_LEFT|wxEXPAND, 5);

	_cameraNameEditBox = new wxTextCtrl(this, wxID_DEFAULT, _("NewCamera"));
	boxSizerV->Add(_cameraNameEditBox, 0, wxALL|wxALIGN_LEFT|wxEXPAND, 5);

	_usesFixedAspectRatioCheckBox = new wxCheckBox(this, wxID_DEFAULT, _("Use Fixed Aspect Ratio"));
	boxSizerV->Add(_usesFixedAspectRatioCheckBox, 0, wxALL|wxALIGN_LEFT|wxEXPAND, 5);

	_bindToBoneCheckBox = new wxCheckBox(this, ControlID_CreateCameraDlgBindToBoneCheck, _("Bind To Bone"));
	boxSizerV->Add(_bindToBoneCheckBox, 0, wxALL|wxALIGN_LEFT|wxEXPAND, 5);

#ifdef IS_FACEFX_STUDIO
	FxRenderWidgetCaps renderWidgetCaps;
	FxSessionProxy::QueryRenderWidgetCaps(renderWidgetCaps);

	if( renderWidgetCaps.supportsFixedAspectRatioCameras )
	{
		_usesFixedAspectRatioCheckBox->SetValue(TRUE);
	}
	else
	{
		_usesFixedAspectRatioCheckBox->Enable(FALSE);
	}

	if( !renderWidgetCaps.supportsBoneLockedCameras )
	{
		_bindToBoneCheckBox->Enable(FALSE);
	}
#else
	_usesFixedAspectRatioCheckBox->Enable(FALSE);
	_bindToBoneCheckBox->Enable(FALSE);
#endif
    
	_boneListBox = new wxListCtrl(this, ControlID_CreateCameraDlgBoneListBox, wxDefaultPosition, wxSize(200,200), wxLC_REPORT|wxLC_SINGLE_SEL);
	wxListItem columnInfo;
	columnInfo.m_mask = wxLIST_MASK_TEXT|wxLIST_MASK_WIDTH;
	columnInfo.m_text = _("Bone Name");
	columnInfo.m_width = GetClientRect().width;
	_boneListBox->InsertColumn(0, columnInfo);
	if( _actor )
	{
		FxMasterBoneList& masterBoneList = _actor->GetMasterBoneList();
		for( FxSize i = 0; i < masterBoneList.GetNumRefBones(); ++i )
		{
			_boneListBox->InsertItem(i, wxString(masterBoneList.GetRefBone(i).GetNameAsCstr(), wxConvLibc));
		}
	}
	_boneListBox->Enable(FALSE);
	boxSizerV->Add(_boneListBox, 0, wxALL|wxALIGN_LEFT|wxEXPAND, 5);

	wxBoxSizer* bottomButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	boxSizerV->Add(bottomButtonSizer, 0, wxALIGN_RIGHT, 5);

	_createButton = new wxButton(this, ControlID_CreateCameraDlgCreateButton, _("Create"), wxDefaultPosition, wxDefaultSize);
	bottomButtonSizer->Add(_createButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	_cancelButton = new wxButton(this, ControlID_CreateCameraDlgCancelButton, _("Cancel"), wxDefaultPosition, wxDefaultSize);
	bottomButtonSizer->Add(_cancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

void FxCreateCameraDialog::
Initialize( FxActor* pActor, FxString* cameraName, FxBool* usesFixedAspectRatio, FxBool* isBoundToBone, FxString* boneName )
{
	_actor = pActor;
	_cameraName = cameraName;
	_usesFixedAspectRatio = usesFixedAspectRatio;
	_isBoundToBone = isBoundToBone;
	_boneName = boneName;
}

FxBool FxCreateCameraDialog::ShowToolTips( void )
{
	return FxTrue;
}

void FxCreateCameraDialog::OnBindToBoneChecked( wxCommandEvent& FxUnused(event) )
{
	if( _bindToBoneCheckBox && _boneListBox )
	{
		if( _bindToBoneCheckBox->IsChecked() )
		{
			_boneListBox->Enable(TRUE);
		}
		else
		{
			_boneListBox->Enable(FALSE);
		}
	}
}

void FxCreateCameraDialog::OnColumnClick( wxListEvent& FxUnused(event) )
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

			FxName boneName(FxString(_boneListBox->GetItemText(index).mb_str(wxConvLibc)).GetData());
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

void FxCreateCameraDialog::OnCreateButtonPressed( wxCommandEvent& FxUnused(event) )
{
	if( _cameraNameEditBox && _usesFixedAspectRatioCheckBox && 
		_bindToBoneCheckBox && _boneListBox && _cameraName && 
		_usesFixedAspectRatio && _isBoundToBone && _boneName )
	{
		*_cameraName = _cameraNameEditBox->GetValue().mb_str(wxConvLibc);

		if( _usesFixedAspectRatioCheckBox->IsChecked() )
		{
			*_usesFixedAspectRatio = FxTrue;
		}
		else
		{
			*_usesFixedAspectRatio = FxFalse;
		}

		if( _bindToBoneCheckBox->IsChecked() )
		{
			*_isBoundToBone = FxTrue;
			FxInt32 index = _boneListBox->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if( index != -1 )
			{
				*_boneName = _boneListBox->GetItemText(index).mb_str(wxConvLibc);
			}
			else
			{
				*_boneName = FxString();
			}
		}
		else
		{
			*_isBoundToBone = FxFalse;
		}
	}

	EndDialog(wxID_OK);
}

void FxCreateCameraDialog::OnCancelButtonPressed( wxCommandEvent& FxUnused(event) )
{
	EndDialog(wxID_CANCEL);
}

} // namespace Face

} // namespace OC3Ent
