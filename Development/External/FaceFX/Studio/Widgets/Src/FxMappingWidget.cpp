//------------------------------------------------------------------------------
// A spreadsheet-like view of the character's mapping.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxMappingWidget.h"
#include "FxMath.h"
#include "FxStudioOptions.h"
#include "FxVM.h"
#include "FxSessionProxy.h"
#include "FxTearoffWindow.h"
#include "FxStudioApp.h"
#include "FxGridTools.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/dcbuffer.h"
#endif

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// Target selection dialog.
//------------------------------------------------------------------------------
class FxTargetSelectionDlg : public wxDialog
{
	DECLARE_EVENT_TABLE();
public:
	FxTargetSelectionDlg( wxWindow* parent, FxPhonemeMap* phonemeMap, FxFaceGraph* faceGraph )
		: wxDialog(parent, -1, wxString(wxT("Select target node, or type new curve name")), 
		           wxDefaultPosition, wxSize(200, 150), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
		, selectedTargetName(FxName::NullName)
		, _filterChoice(NULL)
		, _targetNameCombo(NULL)
		, _phonemeMap(phonemeMap)
		, _faceGraph(faceGraph)
		, _nodeFilter(FxFaceGraphNode::StaticClass())
	{
		SetSizer(new wxBoxSizer(wxVERTICAL));
		_filterChoice = new wxChoice(this, ControlID_MappingWidgetTargetSelectionDlgNodeGroupChoice);
		GetSizer()->Add(_filterChoice, 0, wxALL|wxEXPAND, 5);
		_targetNameCombo = new wxComboBox(this, ControlID_MappingWidgetTargetSelectionDlgTargetNameCombo);
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
					faceGraph->GetNodeType(i)->GetName().GetAsCstr());
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

	FxName selectedTargetName;

	virtual void InitDialog()
	{
		wxDialog::InitDialog();
		_targetNameCombo->SetFocus();
	}

protected:
	// Called when the filter selection changes.
	void OnFilterSelChanged( wxCommandEvent& event )
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
						// Ensure the name doesn't already appear in the mapping.
						FxBool canAdd = FxTrue;
						for( FxSize i = 0; i < _phonemeMap->GetNumTargets(); ++i )
						{
							canAdd &= _phonemeMap->GetTargetName(i) != pNode->GetName();
						}
						if( canAdd )
						{
							_targetNameCombo->Append(wxString(pNode->GetNameAsCstr(), wxConvLibc));
						}
					}
				}
			}
		}
		event.Skip();
	} 

	// Called when the user presses OK.
	void OnOK( wxCommandEvent& event )
	{
		wxString selection = _targetNameCombo->GetValue();
		FxName selectedName(FxString(selection.mb_str(wxConvLibc)).GetData());
		for( FxSize i = 0; i < _phonemeMap->GetNumTargets(); ++i )
		{
			if( selectedName == _phonemeMap->GetTargetName(i) )
			{
				FxMessageBox( _("That target already has an entry in the mapping table.") );
				return;
			}
		}
		selectedTargetName = selectedName;
		event.Skip();
	}

	// Initialize the selection
	void SetSelection( void )
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
					wxString(_nodeFilter->GetName().GetAsCstr(), wxConvLibc)));
			}

			// Force the list box to be filled out.
			wxCommandEvent temp;
			OnFilterSelChanged(temp);
		}
	}

	wxChoice*       _filterChoice;
	wxComboBox*     _targetNameCombo;
	FxPhonemeMap*  _phonemeMap;
	FxFaceGraph*   _faceGraph;
	const FxClass* _nodeFilter;
};

BEGIN_EVENT_TABLE(FxTargetSelectionDlg,wxDialog)
	EVT_CHOICE(ControlID_MappingWidgetTargetSelectionDlgNodeGroupChoice, FxTargetSelectionDlg::OnFilterSelChanged)
	EVT_BUTTON(wxID_OK, FxTargetSelectionDlg::OnOK)
END_EVENT_TABLE()


//------------------------------------------------------------------------------
// FxMappingWidget
//------------------------------------------------------------------------------

WX_IMPLEMENT_DYNAMIC_CLASS(FxMappingWidget, wxGrid)

BEGIN_EVENT_TABLE(FxMappingWidget, wxGrid)
	EVT_GRID_LABEL_RIGHT_CLICK(FxMappingWidget::OnLabelRightClick)

	EVT_MENU(MenuID_MappingWidgetRemoveTarget, FxMappingWidget::OnRemoveTarget)
	EVT_MENU(MenuID_MappingWidgetMakeDefault, FxMappingWidget::OnMakeDefault)
	EVT_MENU(MenuID_MappingWidgetAddTarget, FxMappingWidget::OnAddTarget)
	EVT_MENU(MenuID_MappingWidgetSyncAnim, FxMappingWidget::OnSyncAnim)
	EVT_MENU(MenuID_MappingWidgetClearMapping, FxMappingWidget::OnClearMapping)

	EVT_GRID_CELL_CHANGE(FxMappingWidget::OnCellChange)

	EVT_HELP_RANGE( wxID_ANY, wxID_HIGHEST, FxMappingWidget::OnHelp )
END_EVENT_TABLE()

// Constructor.
FxMappingWidget::FxMappingWidget( wxWindow* parent, 
	FxWidgetMediator* mediator, wxWindowID id, wxPoint position, wxSize size, 
	FxInt32 style)
	: wxGrid(parent, id, position, size, style|wxWS_EX_VALIDATE_RECURSIVELY|wxSIMPLE_BORDER)
	, FxWidget(mediator)
	, _phonemeMap(NULL)
	, _faceGraph(NULL)
	, _ignoreMappingChange(FxFalse)
{
	CreateGrid(0, 0, wxGrid::wxGridSelectCells);
	SetGridLineColour(FxStudioOptions::GetOverlayTextColour());

	RebuildGrid();
}

// Destructor.
FxMappingWidget::~FxMappingWidget()
{
}

void FxMappingWidget::OnActorChanged( FxWidget* FxUnused(sender), FxActor* actor )
{
	if( actor )
	{
		_faceGraph = &actor->GetDecompiledFaceGraph();

		FxMappingWidgetContainer* pContainer = static_cast<FxMappingWidgetContainer*>(GetParent());
		pContainer->UpdateControlStates(actor != NULL);
	}
}

void FxMappingWidget::OnPhonemeMappingChanged( FxWidget* FxUnused(sender), FxPhonemeMap* phonemeMap )
{
	if( !_ignoreMappingChange )
	{
		_phonemeMap = phonemeMap;

		FxMappingWidgetContainer* pContainer = static_cast<FxMappingWidgetContainer*>(GetParent());
		pContainer->UpdateControlStates(_phonemeMap != NULL);

		if( IsCellEditControlEnabled() )
		{
			DisableCellEditControl();
		}
		RebuildGrid();
	}
}

void FxMappingWidget::OnAnimChanged( FxWidget* FxUnused(sender), const FxName& animGroupName, FxAnim* anim )
{
	_animGroup = animGroupName;
	_anim = anim;
}

void FxMappingWidget::OnAnimPhonemeListChanged(	FxWidget* FxUnused(sender), FxAnim* FxUnused(anim), FxPhonWordList* phonemeList )
{
	_phonList = phonemeList;
}

void FxMappingWidget::OnActorInternalDataChanged( FxWidget* FxUnused(sender), FxActorDataChangedFlag FxUnused(flags) )
{
	if( IsCellEditControlEnabled() )
	{
		DisableCellEditControl();
	}
	RebuildGrid();
}

// Called when the user right-clicks on a column label.
void FxMappingWidget::OnLabelRightClick( wxGridEvent& event )
{
	if( _phonemeMap )
	{
		wxMenu popupMenu;
		
		if( 0 <= event.GetCol() && event.GetCol() < static_cast<FxInt32>(_phonemeMap->GetNumTargets()) )
		{
			_lastTarget = FxName(FxString(GetColLabelValue(event.GetCol()).mb_str(wxConvLibc)).GetData());
			wxString prompt(wxString::Format(_("Remove %s from mapping"), wxString::FromAscii(_lastTarget.GetAsCstr())));
			popupMenu.Append(MenuID_MappingWidgetRemoveTarget, prompt, _("Removes the target from the mapping"));
		}

		popupMenu.Append(MenuID_MappingWidgetAddTarget, _("Add new target to mapping..."), _("Adds a new target to the mapping"));
		popupMenu.AppendSeparator();
		popupMenu.Append(MenuID_MappingWidgetMakeDefault, _("Make default mapping"), _("Sets the mapping to the default"));

		PopupMenu(&popupMenu, ScreenToClient(wxGetMousePosition()));
	}
}

// Called when the user selects to remove a target from the popup menu.
void FxMappingWidget::OnRemoveTarget( wxCommandEvent& FxUnused(event) )
{
	if( _phonemeMap )
	{
		FxString command("map -remove -track \"");
		command = FxString::Concat( command, _lastTarget.GetAsCstr() );
		command = FxString::Concat( command, "\"" );
		FxVM::ExecuteCommand(command);
	}
}

// Called when the user selects to make the default mapping from the popup.
void FxMappingWidget::OnMakeDefault( wxCommandEvent& FxUnused(event) )
{
	if( _phonemeMap )
	{
		if( wxYES == FxMessageBox(_("This will replace the current mapping with the default mapping and is not undoable.  Continue?"),
			_("Confirm Opertaion"), wxYES_NO|wxICON_QUESTION, this) )
		{
			_phonemeMap->MakeDefaultMapping();
			RebuildGrid();
			if( _mediator && _phonemeMap )
			{
				_mediator->OnPhonemeMappingChanged(this, _phonemeMap);
			}
			FxSessionProxy::SetIsForcedDirty(FxTrue);
		}
	}
}

void FxMappingWidget::OnAddTarget( wxCommandEvent& FxUnused(event) )
{
	if( _phonemeMap )
	{
		FxTargetSelectionDlg dialog(this, _phonemeMap, _faceGraph);
		FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
		if( dialog.ShowModal() == wxID_OK )
		{
			if( dialog.selectedTargetName != FxName::NullName )
			{
				wxString command = wxString::Format(wxT("map -create -track \"%s\""),
					wxString(dialog.selectedTargetName.GetAsCstr(), wxConvLibc));
				FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
			}
		}
		FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
	}
}

void FxMappingWidget::OnSyncAnim( wxCommandEvent& FxUnused(event) )
{
	if( _phonemeMap && _anim )
	{
		// Find any curves in the animation that are owned by analysis and not
		// in the mapping (and is not a speech gesture); flag them for removal.
		FxArray<FxString> curvesToRemove;
		FxAnimUserData* animUserData = static_cast<FxAnimUserData*>(_anim->GetUserData());
		for( FxSize i = 0; i < _anim->GetNumAnimCurves(); ++i )
		{
			if( animUserData->IsCurveOwnedByAnalysis(_anim->GetAnimCurve(i).GetName()) )
			{
				if( _phonemeMap->GetTargetIndex(_anim->GetAnimCurve(i).GetName()) == FxInvalidIndex )
				{
					FxBool isSpeechGesture = FxFalse;
					for( FxSize j = 0; j < FxCGGetNumSpeechGestureTracks(); ++j )
					{
						if( FxCGGetSpeechGestureTrackName(j) == _anim->GetAnimCurve(i).GetName() )
						{
							isSpeechGesture = FxTrue;
							break;
						}
					}

					FxString animCurveName = _anim->GetAnimCurve(i).GetNameAsString();
					if( FxString("Eyebrow_Raise_Left") == animCurveName  ||
					    FxString("Eyebrow_Raise_Right") == animCurveName ||
					    FxString("Blink_Left") == animCurveName          ||
					    FxString("Blink_Right") == animCurveName         ||
					    FxString("Head_Bend") == animCurveName           ||
					    FxString("Head_Side_Side") == animCurveName      ||
					    FxString("Head_Twist") == animCurveName          ||
					    FxString("Eye_Side_Side_Left") == animCurveName  ||
					    FxString("Eye_Side_Side_Right") == animCurveName ||
					    FxString("Eye_Up_Down_Left") == animCurveName    ||
					    FxString("Eye_Up_Down_Right") == animCurveName )
					{
						isSpeechGesture = FxTrue;
					}

					if( !isSpeechGesture )
					{
						curvesToRemove.PushBack(_anim->GetAnimCurve(i).GetNameAsString());
					}
				}
			}
		}

		// Find any targets in the mapping that are not in the animation; flag
		// them for addition.
		FxArray<FxString> curvesToAdd;
		for( FxSize i = 0; i < _phonemeMap->GetNumTargets(); ++i )
		{
			if( _anim->FindAnimCurve(_phonemeMap->GetTargetName(i)) == FxInvalidIndex )
			{
				curvesToAdd.PushBack(_phonemeMap->GetTargetName(i).GetAsString());
			}
		}

		FxVM::ExecuteCommand("batch");
		for( FxSize i = 0; i < curvesToRemove.Length(); ++i )
		{
			wxString removeCommand = wxString::Format(
				wxT("curve -remove -group \"%s\" -anim \"%s\" -name \"%s\""),
				wxString::FromAscii(_animGroup.GetAsCstr()),
				wxString::FromAscii(_anim->GetNameAsCstr()),
				wxString::FromAscii(curvesToRemove[i].GetData()));
			FxVM::ExecuteCommand(FxString(removeCommand.mb_str(wxConvLibc)));
		}
		for( FxSize i = 0; i < curvesToAdd.Length(); ++i )
		{
			wxString addCommand = wxString::Format(
				wxT("curve -add -group \"%s\" -anim \"%s\" -name \"%s\" -owner \"analysis\""),
				wxString::FromAscii(_animGroup.GetAsCstr()),
				wxString::FromAscii(_anim->GetNameAsCstr()),
				wxString::FromAscii(curvesToAdd[i].GetData()));
			FxVM::ExecuteCommand(FxString(addCommand.mb_str(wxConvLibc)));
		}
		FxVM::ExecuteCommand("execBatch -addedcurves -removedcurves");

		// Force re-coarticulation.
		if( _phonList )
		{
			_mediator->OnAnimPhonemeListChanged(this, _anim, _phonList);
		}
	}
}

// Called to clear the mapping.
void FxMappingWidget::OnClearMapping( wxCommandEvent& FxUnused(event))
{
	if( _phonemeMap )
	{
		// Reserve room for all the remove commands.
		FxArray<FxString> commands(_phonemeMap->GetNumTargets());
		// Iterate backwards so when the user undoes the command the target
		// order will be the same as it is when the user executes the command.
		for( FxInt32 i = _phonemeMap->GetNumTargets() - 1; i >= 0; --i )
		{
			FxString command("map -remove -track \"");
			command += _phonemeMap->GetTargetName(i).GetAsString();
			command += "\"";
			commands.PushBack(command);
		}

		FxVM::ExecuteCommand("batch");
		for( FxSize i = 0; i < commands.Length(); ++i )
		{
			FxVM::ExecuteCommand(commands[i]);
		}
		FxVM::ExecuteCommand("execBatch -changedmapping");
	}
}

// Called when the user changes a cell in the table.
void FxMappingWidget::OnCellChange( wxGridEvent& event )
{
	if( _phonemeMap )
	{
		wxString value = GetCellValue(event.GetRow(), event.GetCol());
		FxReal val = FxAtof(value.mb_str(wxConvLibc));
		if( FxRealEquality(val, 0.0f) )
		{
			SetCellValue(wxT("0.00"), event.GetRow(), event.GetCol());
		}
		FxReal oldValue = _phonemeMap->GetMappingAmount(static_cast<FxPhoneme>(event.GetRow()), event.GetCol());
		if( val > FX_REAL_MAX )
		{
			val = FX_REAL_MAX;
		}
		if( !FxRealEquality(val, oldValue) )
		{
			wxString phoneme;
			switch( FxStudioOptions::GetPhoneticAlphabet() )
			{
			default:
			case PA_DEFAULT:
				phoneme = FxGetPhonemeInfo(event.GetRow()).talkback;
				break;
			case PA_IPA:
			case PA_SAMPA:
				phoneme = FxGetPhonemeInfo(event.GetRow()).sampa;
				break;
			};

			wxString command  = wxString::Format(wxT("map -set -phoneme \"%s\" -track \"%s\" -value %f"),
				phoneme,
				GetColLabelValue(event.GetCol()),
				val);
			_ignoreMappingChange = FxTrue;
			FxVM::ExecuteCommand( FxString(command.mb_str(wxConvLibc)) );
			_ignoreMappingChange = FxFalse;
		}
	}
}

void FxMappingWidget::OnHelp(wxHelpEvent& FxUnused(event))
{
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().LoadFile();
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().DisplaySection(wxT("Mapping Tab"));
}

void FxMappingWidget::RebuildGrid()
{
	if( IsCellEditControlShown() )
	{
		HideCellEditControl();
	}
	BeginBatch();

	const FxSize numPhonemes = NUM_PHONS;
	FxSize numTargets = 0;
	if( _phonemeMap )
	{
		numTargets = _phonemeMap->GetNumTargets();
	}

	// Size the grid
	if( numPhonemes > GetNumberRows() )
	{
		InsertRows(0, numPhonemes - GetNumberRows());
	}
	else if( numPhonemes < GetNumberRows() )
	{
		DeleteRows(0, GetNumberRows() - numPhonemes);
	}
	if( static_cast<FxInt32>(numTargets) > GetNumberCols() )
	{
		InsertCols(0, numTargets - GetNumberCols());
	}
	else if( static_cast<FxInt32>(numTargets) < GetNumberCols() )
	{
		DeleteCols(0, GetNumberCols() - numTargets);
	}

	// Set the row labels and label size.
	FxPhoneticAlphabet phoneticAlphabet = FxStudioOptions::GetPhoneticAlphabet();
	wxString rowLabel = wxEmptyString;
	SetRowLabelSize(45);
	for( FxSize i = 0; i < NUM_PHONS; ++i )
	{
		switch( phoneticAlphabet )
		{
		default:
		case PA_DEFAULT:
			rowLabel = FxGetPhonemeInfo(i).talkback;
			break;
		case PA_SAMPA:
			rowLabel = FxGetPhonemeInfo(i).sampa;
			break;
		case PA_IPA:
			rowLabel = FxGetPhonemeInfo(i).ipa;
			break;
		};

		SetRowLabelValue(i, rowLabel);
		SetLabelFont(wxFont(8.0f, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE, FxStudioOptions::GetUnicodeFontName()));
	}

	if( _phonemeMap )
	{
		SetDefaultColSize(45);

		// Set the column labels.
		for( FxSize i = 0; i < numTargets; ++i )
		{
			SetColLabelValue(i, wxString::FromAscii(_phonemeMap->GetTargetName(i).GetAsCstr()));
		}
		
		// Set the cell values
		for( FxSize row = 0; row < numPhonemes; ++row )
		{
			for( FxSize col = 0; col < numTargets; ++col )
			{
				SetCellValue( wxString::Format(wxT("%f"), _phonemeMap->GetMappingAmount( static_cast<FxPhoneme>(row), col )), row, col );
			}
		}
	}

	// Set the renderers and size the grid appropriately
	SetDefaultRenderer( new FxGridCellFloatRenderer(-1,2) );
	SetDefaultEditor( new FxGridCellFloatEditor(-1,2) );

	EndBatch();
}

//------------------------------------------------------------------------------
// FxMappingWidgetContainer
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(FxMappingWidgetContainer, wxWindow)
	EVT_PAINT(FxMappingWidgetContainer::OnPaint)
	EVT_SIZE(FxMappingWidgetContainer::OnSize)

	EVT_BUTTON(ControlID_MappingWidgetToolbarMakeDefaultMappingButton, FxMappingWidgetContainer::OnMakeDefaultMappingButton)
	EVT_BUTTON(ControlID_MappingWidgetToolbarClearMappingButton, FxMappingWidgetContainer::OnClearMappingButton)
	EVT_BUTTON(ControlID_MappingWidgetToolbarAddTargetButton, FxMappingWidgetContainer::OnAddTargetButton)
	EVT_BUTTON(ControlID_MappingWidgetToolbarSyncAnimButton, FxMappingWidgetContainer::OnSyncAnimButton)

	EVT_UPDATE_UI_RANGE(ControlID_MappingWidgetToolbarMakeDefaultMappingButton, ControlID_MappingWidgetToolbarAddTargetButton, FxMappingWidgetContainer::OnUpdateUI)
END_EVENT_TABLE()

FxMappingWidgetContainer::FxMappingWidgetContainer(wxWindow* parent, FxWidgetMediator* mediator)
	: Super(parent, -1, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE|wxCLIP_CHILDREN|wxTAB_TRAVERSAL)
{
	_mappingWidget = new FxMappingWidget(this, mediator);
	CreateToolbarControls();
}

FxMappingWidgetContainer::~FxMappingWidgetContainer()
{
}

void FxMappingWidgetContainer::UpdateControlStates(FxBool state)
{
	if( _toolbarMakeDefaultMappingButton && _toolbarClearMappingButton && _toolbarAddTargetButton )
	{
		_toolbarMakeDefaultMappingButton->Enable(state);
		_toolbarClearMappingButton->Enable(state);
		_toolbarAddTargetButton->Enable(state);
		_toolbarSyncAnimButton->Enable(state);
	}
}

void FxMappingWidgetContainer::OnPaint(wxPaintEvent& FxUnused(event))
{
	wxBufferedPaintDC dc(this);
	DrawToolbar(&dc);
}

void FxMappingWidgetContainer::DrawToolbar(wxDC* pDC)
{
	wxColour colourFace = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
	wxColour colourShadow = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);

	// Draw the toolbar background.
	pDC->SetBrush(wxBrush(colourFace));
	pDC->SetPen(wxPen(colourFace));
	pDC->DrawRectangle(_toolbarRect);

	// Shade the edges.
	pDC->SetPen(wxPen(colourShadow));
	pDC->DrawLine(_toolbarRect.GetLeft(), _toolbarRect.GetBottom(),
		_toolbarRect.GetRight(), _toolbarRect.GetBottom());

	pDC->SetPen(wxNullPen);
	pDC->SetBrush(wxNullBrush);
}

void FxMappingWidgetContainer::RefreshChildren()
{
	wxWindowList& children = GetChildren();
	wxWindowListNode* node = children.GetFirst();
	while( node )
	{
		node->GetData()->Refresh();
		node = node->GetNext();
	}
}

void FxMappingWidgetContainer::OnSize(wxSizeEvent& FxUnused(event))
{
	DoResize();
	RefreshChildren();
}

void FxMappingWidgetContainer::DoResize()
{
	wxRect clientRect(GetClientRect());
	_toolbarRect = clientRect;
	_toolbarRect.SetHeight(FxToolbarHeight);
	_widgetRect = clientRect;
	_widgetRect.SetTop(FxToolbarHeight);
	_widgetRect.SetHeight(_widgetRect.GetHeight() - FxToolbarHeight);

	if( _mappingWidget )
	{
		_mappingWidget->SetSize(_widgetRect);
		_mappingWidget->Refresh(FALSE);
	}
}

void FxMappingWidgetContainer::CreateToolbarControls()
{
	SetSizer(new wxBoxSizer(wxHORIZONTAL));

	GetSizer()->AddSpacer(5);

	_toolbarClearMappingButton = new FxButton(this,
		ControlID_MappingWidgetToolbarClearMappingButton, _("NM"), wxDefaultPosition, wxSize(20,20));
	_toolbarClearMappingButton->MakeToolbar();
	_toolbarClearMappingButton->SetToolTip(_("New mapping"));
	GetSizer()->Add(_toolbarClearMappingButton, 0, wxTOP, 2);

	_toolbarMakeDefaultMappingButton = new FxButton(this, 
		ControlID_MappingWidgetToolbarMakeDefaultMappingButton, _("DM"), wxDefaultPosition, wxSize(20,20));
	_toolbarMakeDefaultMappingButton->MakeToolbar();
	_toolbarMakeDefaultMappingButton->SetToolTip(_("New default mapping"));
	GetSizer()->Add(_toolbarMakeDefaultMappingButton, 0, wxTOP, 2);

	_toolbarAddTargetButton = new FxButton(this,
		ControlID_MappingWidgetToolbarAddTargetButton, _("AT"), wxDefaultPosition, wxSize(20,20));
	_toolbarAddTargetButton->MakeToolbar();
	_toolbarAddTargetButton->SetToolTip(_("Add target to mapping"));
	GetSizer()->Add(_toolbarAddTargetButton, 0, wxTOP, 2);
	
	_toolbarSyncAnimButton = new FxButton(this,
		ControlID_MappingWidgetToolbarSyncAnimButton, _("SA"), wxDefaultPosition, wxSize(20,20));
	_toolbarSyncAnimButton->MakeToolbar();
	_toolbarSyncAnimButton->SetToolTip(_("Sync current animation to mapping"));
	GetSizer()->Add(_toolbarSyncAnimButton, 0, wxTOP, 2);


	SetAutoLayout(TRUE);
	GetSizer()->Layout();

	LoadIcons();
}

void FxMappingWidgetContainer::OnMakeDefaultMappingButton(wxCommandEvent& FxUnused(event))
{
	wxCommandEvent toDispatch(wxEVT_COMMAND_MENU_SELECTED, MenuID_MappingWidgetMakeDefault);
	_mappingWidget->ProcessEvent(toDispatch);
}

void FxMappingWidgetContainer::OnClearMappingButton(wxCommandEvent& FxUnused(event))
{
	wxCommandEvent toDispatch(wxEVT_COMMAND_MENU_SELECTED, MenuID_MappingWidgetClearMapping);
	_mappingWidget->ProcessEvent(toDispatch);
}

void FxMappingWidgetContainer::OnAddTargetButton(wxCommandEvent& FxUnused(event))
{
	wxCommandEvent toDispatch(wxEVT_COMMAND_MENU_SELECTED, MenuID_MappingWidgetAddTarget);
	_mappingWidget->ProcessEvent(toDispatch);
}

void FxMappingWidgetContainer::OnSyncAnimButton(wxCommandEvent& FxUnused(event))
{
	wxCommandEvent toDispatch(wxEVT_COMMAND_MENU_SELECTED, MenuID_MappingWidgetSyncAnim);
	_mappingWidget->ProcessEvent(toDispatch);
}

void FxMappingWidgetContainer::OnUpdateUI(wxUpdateUIEvent& FxUnused(event))
{
	FxActor* pActor = NULL;
	FxSessionProxy::GetActor(&pActor);
	UpdateControlStates(pActor != NULL);
}

void FxMappingWidgetContainer::LoadIcons()
{
	if( _toolbarClearMappingButton && _toolbarMakeDefaultMappingButton && _toolbarAddTargetButton )
	{
		wxString clearmappingPath = FxStudioApp::GetIconPath(wxT("new.ico"));
		wxString clearmappingDisabledPath = FxStudioApp::GetIconPath(wxT("new_disabled.ico"));
		if( FxFileExists(clearmappingPath) && FxFileExists(clearmappingDisabledPath) )
		{
			wxIcon clearmappingIcon = wxIcon(wxIconLocation(clearmappingPath));
			clearmappingIcon.SetHeight(16);
			clearmappingIcon.SetWidth(16);
			wxIcon clearmappingDisabledIcon = wxIcon(wxIconLocation(clearmappingDisabledPath));
			clearmappingDisabledIcon.SetHeight(16);
			clearmappingDisabledIcon.SetWidth(16);
			_toolbarClearMappingButton->SetEnabledBitmap(clearmappingIcon);
			_toolbarClearMappingButton->SetDisabledBitmap(clearmappingDisabledIcon);
		}

		wxString makedefaultPath = FxStudioApp::GetIconPath(wxT("workspacewizard.ico"));
		wxString makedefaultDisabledPath = FxStudioApp::GetIconPath(wxT("workspacewizard_disabled.ico"));
		if( FxFileExists(makedefaultPath) && FxFileExists(makedefaultDisabledPath) )
		{
			wxIcon makedefaultIcon = wxIcon(wxIconLocation(makedefaultPath));
			makedefaultIcon.SetHeight(16);
			makedefaultIcon.SetWidth(16);
			wxIcon makedefaultDisabledIcon = wxIcon(wxIconLocation(makedefaultDisabledPath));
			makedefaultDisabledIcon.SetHeight(16);
			makedefaultDisabledIcon.SetWidth(16);
			_toolbarMakeDefaultMappingButton->SetEnabledBitmap(makedefaultIcon);
			_toolbarMakeDefaultMappingButton->SetDisabledBitmap(makedefaultDisabledIcon);
		}

		wxString addtargetPath = FxStudioApp::GetIconPath(wxT("addtarget.ico"));
		wxString addtargetDisabledPath = FxStudioApp::GetIconPath(wxT("addtarget_disabled.ico"));
		if( FxFileExists(addtargetPath) && FxFileExists(addtargetDisabledPath) )
		{
			wxIcon addtargetIcon = wxIcon(wxIconLocation(addtargetPath));
			addtargetIcon.SetHeight(16);
			addtargetIcon.SetWidth(16);
			wxIcon addtargetDisabledIcon = wxIcon(wxIconLocation(addtargetDisabledPath));
			addtargetDisabledIcon.SetHeight(16);
			addtargetDisabledIcon.SetWidth(16);
			_toolbarAddTargetButton->SetEnabledBitmap(addtargetIcon);
			_toolbarAddTargetButton->SetDisabledBitmap(addtargetDisabledIcon);
		}

		wxString syncanimPath = FxStudioApp::GetIconPath(wxT("syncanim.ico"));
		wxString syncanimDisabledPath = FxStudioApp::GetIconPath(wxT("syncanim_disabled.ico"));
		if( FxFileExists(syncanimPath) && FxFileExists(syncanimDisabledPath) )
		{
			wxIcon syncanimIcon = wxIcon(wxIconLocation(syncanimPath));
			syncanimIcon.SetHeight(16);
			syncanimIcon.SetWidth(16);
			wxIcon syncanimDisabledIcon = wxIcon(wxIconLocation(syncanimDisabledPath));
			syncanimDisabledIcon.SetHeight(16);
			syncanimDisabledIcon.SetWidth(16);
			_toolbarSyncAnimButton->SetEnabledBitmap(syncanimIcon);
			_toolbarSyncAnimButton->SetDisabledBitmap(syncanimDisabledIcon);
		}
	}
}

} // namespace Face

} // namespace OC3Ent
