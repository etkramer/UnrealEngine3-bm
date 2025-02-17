//------------------------------------------------------------------------------
// The link function editor dialog.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxLinkFnEditDlg.h"
#include "FxLinkFn.h"
#include "FxMath.h"
#include "FxVM.h"

namespace OC3Ent
{

namespace Face
{

WX_IMPLEMENT_DYNAMIC_CLASS(FxLinkFnEditDlg, wxDialog)

BEGIN_EVENT_TABLE(FxLinkFnEditDlg, wxDialog)
	EVT_NAVIGATION_KEY(FxLinkFnEditDlg::OnNavKey)

	EVT_COMBOBOX(ControlID_LinkFunctionEditDlgLinkFunctionChoice, FxLinkFnEditDlg::OnLinkFunctionChanged)
	EVT_GRID_CELL_CHANGE(FxLinkFnEditDlg::OnCellChanged)

	EVT_BUTTON(ControlID_LinkFunctionEditDlgOKButton, FxLinkFnEditDlg::OnButtonOK)
	EVT_BUTTON(ControlID_LinkFunctionEditDlgCancelButton, FxLinkFnEditDlg::OnButtonCancel)
END_EVENT_TABLE()

FxLinkFnEditDlg::FxLinkFnEditDlg()
{
}

FxLinkFnEditDlg::FxLinkFnEditDlg( wxWindow* parent, FxWidgetMediator* mediator, 
							    FxFaceGraph* pFaceGraph, 
								const FxName& inputNode, const FxName& outputNode)	
	: Super(parent, -1, _("Edit link function"), wxDefaultPosition, wxDefaultSize, 
	        wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxFULL_REPAINT_ON_RESIZE)
	, _mediator(mediator)
	, _linkFunctionEditor(NULL)
	, _linkFunctionCombo(NULL)
	, _parameterGrid(NULL)
	, _pGraph(pFaceGraph)
	, _inputNode(inputNode)
	, _outputNode(outputNode)
	, _active(FxFalse)
{
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
	CreateControls();
	OnLinkSelChanged( NULL, _inputNode, _outputNode );
}

FxLinkFnEditDlg::~FxLinkFnEditDlg()
{
}

// Called when the selected link changes.
void FxLinkFnEditDlg::
OnLinkSelChanged( FxWidget* FxUnused(sender), const FxName& inputNode, const FxName& outputNode )
{
	if( _pGraph )
	{
		FxFaceGraphNode* pInputNode = NULL;
		FxFaceGraphNode* pOutputNode = NULL;
		_pGraph->DetermineLinkDirection(inputNode, outputNode, pInputNode, pOutputNode);
		if( pInputNode && pOutputNode )
		{
			_inputNode = pInputNode->GetName();
			_outputNode = pOutputNode->GetName();
		}

		_active = _pGraph->GetLink(inputNode, outputNode, _link);
		if( _active )
		{
			wxString linkFnName = wxString::FromAscii(_link.GetLinkFnName().GetAsString().GetData());
			_linkFunctionCombo->SetSelection(_linkFunctionCombo->FindString(linkFnName));
			_linkFunctionEditor->SetNodeNames(_inputNode, _outputNode);
			_linkFunctionEditor->SetLinkFunction(FxLinkFn::FindLinkFunctionByType(_link.GetLinkFnType()), _link.GetLinkFnParams());

			wxCommandEvent temp;
			OnLinkFunctionChanged(temp);
		}

		UpdateLabel();
		SetActive(_active);
	}
}

// Called when the selection in the link function combo changes.
void FxLinkFnEditDlg::OnLinkFunctionChanged( wxCommandEvent& FxUnused(event) )
{
	// Find the link function
	FxString selection(_linkFunctionCombo->GetString(_linkFunctionCombo->GetSelection()).mb_str(wxConvLibc));
	PopulateParameterGrid(selection);
	UpdateFunctionEditor();
	UpdateLabel();
}

// Called when a cell in the parameter grid changes.
void FxLinkFnEditDlg::OnCellChanged( wxGridEvent& FxUnused(event) )
{
	UpdateFunctionEditor();
	UpdateLabel();
}

// Called when the user presses apply.
void FxLinkFnEditDlg::OnButtonApply( wxCommandEvent& FxUnused(event) )
{
	if( _pGraph )
	{
		if( _parameterGrid->IsCellEditControlEnabled() )
		{
			_parameterGrid->SaveEditControlValue();
			_parameterGrid->HideCellEditControl();
			_parameterGrid->DisableCellEditControl();
			Refresh();
		}

		FxString linkFnName(_linkFunctionCombo->GetString(_linkFunctionCombo->GetSelection()).mb_str(wxConvLibc));
		const FxLinkFn* linkFn = FxLinkFn::FindLinkFunction(linkFnName.GetData());
		_link.SetLinkFn(linkFn);

		FxLinkFnParameters params;
		
		wxString parameterString;
		for( FxSize i = 0; i < linkFn->GetNumParams(); ++i )
		{
			params.parameters.PushBack(FxAtof(_parameterGrid->GetCellValue(i, 1).mb_str(wxConvLibc)));
			parameterString += _parameterGrid->GetCellValue(i, 0) + wxT("=") + 
							   _parameterGrid->GetCellValue(i, 1) + wxT("|");
		}
		if( parameterString.Length() > 0 )
		{
			parameterString.RemoveLast();
		}

		// Use the default parameters if the user hasn't typed in custom ones.
		FxBool allEqual = FxTrue;
		for( FxSize i = 0; i < linkFn->GetNumParams(); ++i )
		{
			allEqual &= FxRealEquality(params.parameters[i], linkFn->GetParamDefaultValue(i));
		}
		if( allEqual )
		{
			params.parameters.Clear();
			parameterString = wxEmptyString;
		}

		wxString command = wxString::Format( wxT("graph -editlink -from \"%s\" -to \"%s\" -linkfn \"%s\""),
			wxString::FromAscii(_outputNode.GetAsCstr()),
			wxString::FromAscii(_inputNode.GetAsCstr()),
			wxString::FromAscii(linkFnName.GetData()) );
		if( parameterString.Length() > 0 )
		{
			command += wxT(" -linkfnparams \"") + parameterString + wxT("\"");
		}
		FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
	}
}

// Called when the user presses OK
void FxLinkFnEditDlg::OnButtonOK( wxCommandEvent& event )
{
	if( _parameterGrid->IsCellEditControlShown() )
	{
		_parameterGrid->SaveEditControlValue();
		_parameterGrid->HideCellEditControl();
	}

	OnButtonApply( event );
	EndDialog(wxID_OK);
}

// Called when the user presses OK
void FxLinkFnEditDlg::OnButtonCancel( wxCommandEvent& FxUnused(event) )
{
	EndDialog(wxID_CANCEL);
}

// Called when the user presses a navigation key.
void FxLinkFnEditDlg::OnNavKey( wxNavigationKeyEvent& event )
{
	if( _parameterGrid->IsCellEditControlShown() )
	{
		_parameterGrid->SaveEditControlValue();
		_parameterGrid->HideCellEditControl();
	}
	event.Skip();
}

// Creates the controls.
void FxLinkFnEditDlg::CreateControls( void )
{
	wxSizer* topLevelSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(topLevelSizer);
	SetAutoLayout(TRUE);

	_linkFunctionLabel = new wxStaticText(this, ControlID_LinkFunctionEditDlgLabelStaticText, _("node -> other node"));
	topLevelSizer->Add(_linkFunctionLabel, 0, wxGROW|wxALL, 5);

	_linkFunctionCombo = new wxComboBox(this, ControlID_LinkFunctionEditDlgLinkFunctionChoice, 
		wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	topLevelSizer->Add(_linkFunctionCombo, 0, wxGROW|wxALL, 5);

	_linkFunctionEditor = new FxFunctionEditor(this, -1, wxDefaultPosition, wxSize(300, 300));
	topLevelSizer->Add(_linkFunctionEditor, 3, wxGROW|wxTOP|wxLEFT|wxRIGHT, 5);

	_parameterGrid = new wxGrid(this, ControlID_LinkFunctionEditDlgParameterGrid, 
		wxDefaultPosition, wxSize(300, 100), wxWANTS_CHARS|wxSTATIC_BORDER);
	_parameterGrid->CreateGrid(0, 2);
	_parameterGrid->SetColLabelValue(0, _("Name"));
	_parameterGrid->SetColLabelValue(1, _("Value"));
	_parameterGrid->SetColSize(0, 95);
	_parameterGrid->SetRowLabelSize(0);
	_parameterGrid->SetColLabelSize(20);
	topLevelSizer->Add(_parameterGrid, 1, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

	wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton* okButton = new wxButton(this, ControlID_LinkFunctionEditDlgOKButton, _("OK"));
	buttonSizer->Add(okButton, 1, wxGROW|wxALL, 5);
	wxButton* cancelButton = new wxButton(this, ControlID_LinkFunctionEditDlgCancelButton, _("Cancel"));
	buttonSizer->Add(cancelButton, 1, wxGROW|wxALL, 5);
	topLevelSizer->Add(buttonSizer, 0, wxGROW|wxALL, 5);

	PopulateLinkFunctionCombo();
	UpdateSizers();
}

// Populates the combo box with the names of the link functions.
void FxLinkFnEditDlg::PopulateLinkFunctionCombo( void )
{
	_linkFunctionCombo->Clear();

	FxSize numLinkFns = FxLinkFn::GetNumLinkFunctions();
	for( FxSize i = 0; i < numLinkFns; ++i )
	{
		const FxLinkFn* linkFn = FxLinkFn::GetLinkFunction(i);
		if( linkFn->GetType() != LFT_Null )
		{
			_linkFunctionCombo->Append(wxString::FromAscii(linkFn->GetNameAsCstr()));
		}
	}
	_linkFunctionCombo->SetSelection(0);

	FxString selection(_linkFunctionCombo->GetString(_linkFunctionCombo->GetSelection()).mb_str(wxConvLibc));
	PopulateParameterGrid(selection); 
	UpdateFunctionEditor();
}

// Populates the grid with the names of the parameters for a given link function.
void FxLinkFnEditDlg::PopulateParameterGrid( const FxString& linkFunctionName )
{
	// Clear the grid.
	if( _parameterGrid->GetNumberRows() )
	{
		_parameterGrid->DeleteRows(0, _parameterGrid->GetNumberRows());
	}

	const FxLinkFn* linkFn = FxLinkFn::FindLinkFunction(linkFunctionName.GetData());

	if( linkFn )
	{
		// Add the new parameters.
		FxSize numParams = linkFn->GetNumParams();
		_parameterGrid->AppendRows(numParams);

		for( FxSize i = 0; i < numParams; ++i )
		{
			_parameterGrid->SetCellValue(wxString::FromAscii(linkFn->GetParamName(i).GetData()), i, 0);
			_parameterGrid->SetReadOnly(i, 0);
			_parameterGrid->SetCellRenderer(i, 1, new wxGridCellFloatRenderer(-1, 2));
			_parameterGrid->SetCellEditor(i, 1, new wxGridCellFloatEditor(-1,2));
			wxString paramValue = wxEmptyString;
			if( _active )
			{
				FxLinkFnParameters params = _link.GetLinkFnParams();
				if( params.parameters.Length() == numParams )
				{
					paramValue = wxString::Format(wxT("%f"), params.parameters[i]);
				}
			}
			if( paramValue == wxEmptyString )
			{
				paramValue = wxString::Format(wxT("%f"), linkFn->GetParamDefaultValue(i));
			}
			_parameterGrid->SetCellValue(paramValue, i, 1);
		}
	}
}

// Updates the function editor.
void FxLinkFnEditDlg::UpdateFunctionEditor( void )
{
	if( _pGraph )
	{
		FxString linkFunctionName(_linkFunctionCombo->GetString(_linkFunctionCombo->GetSelection()).mb_str(wxConvLibc));
		const FxLinkFn* linkFn = FxLinkFn::FindLinkFunction(linkFunctionName.GetData());
		if( linkFn )
		{
			FxLinkFnParameters params = _link.GetLinkFnParams();
			params.parameters.Clear();
			for( FxSize i = 0; i < linkFn->GetNumParams(); ++i )
			{
				FxReal parameterValue = FxAtof(_parameterGrid->GetCellValue(i, 1).mb_str(wxConvLibc));
				params.parameters.PushBack(parameterValue);
			}

			// Set the domain and range
			if( _inputNode != FxName::NullName && _outputNode != FxName::NullName )
			{
				FxFaceGraphNode* pInput  = _pGraph->FindNode(_inputNode);
				FxFaceGraphNode* pOutput = _pGraph->FindNode(_outputNode);
				_linkFunctionEditor->SetDomainRange(pOutput->GetMin(), pOutput->GetMax(),
					pInput->GetMin(), pInput->GetMax());
			}
			_linkFunctionEditor->SetLinkFunction(linkFn, params);
			_linkFunctionEditor->Refresh();
		}
	}
}

// Updates the sizes.
void FxLinkFnEditDlg::UpdateSizers( void )
{
	Fit();
}

// Updates the label.
void FxLinkFnEditDlg::UpdateLabel( void )
{
	if( _active )
	{
		wxString outputNodeName(_outputNode.GetAsString().GetData(), wxConvLibc);
		wxString inputNodeName( _inputNode.GetAsString().GetData(), wxConvLibc);
		FxString linkFunctionName(_linkFunctionCombo->GetString(_linkFunctionCombo->GetSelection()).mb_str(wxConvLibc));
		const FxLinkFn* linkFn = FxLinkFn::FindLinkFunction(linkFunctionName.GetData());
		if( linkFn )
		{
			wxString functionDesc(linkFn->GetFnDesc().GetData(), wxConvLibc);

			for( FxSize i = 0; i < linkFn->GetNumParams(); ++i )
			{
				wxString parameterName = _parameterGrid->GetCellValue(i, 0);
				FxReal  parameterValueReal = FxAtof(_parameterGrid->GetCellValue(i, 1).mb_str(wxConvLibc));
				wxString parameterValue = wxString::Format(wxT("%.2f"), parameterValueReal);
				functionDesc.Replace(parameterName, parameterValue);
			}

			wxString space(wxT(" "));
			functionDesc.Replace(wxT(" x "), space + outputNodeName + space);
			functionDesc.Replace(wxT(" y "), space + inputNodeName + space);

			_linkFunctionLabel->SetLabel(functionDesc);
		}
		else
		{
			wxString linkFnName = wxString::FromAscii(_link.GetLinkFnName().GetAsString().GetData());
			_linkFunctionLabel->SetLabel(wxString::Format(_("Deprecated link function: %s"), linkFnName));
		}
	}
	else
	{
		_linkFunctionLabel->SetLabel(_("No link selected."));
	}
}

// Sets whether or not the controls are active.
void FxLinkFnEditDlg::SetActive( FxBool isActive )
{
	if( !isActive )
	{
		FxLinkFnParameters params;
		_linkFunctionEditor->SetLinkFunction(NULL, params);
	}

	_linkFunctionEditor->Enable(isActive);
	_linkFunctionCombo->Enable(isActive);
	_parameterGrid->EnableEditing(isActive);

	Refresh(FALSE);
}

} // namespace Face

} // namespace OC3Ent
