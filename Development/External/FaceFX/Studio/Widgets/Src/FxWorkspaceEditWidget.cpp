//------------------------------------------------------------------------------
// The widget for editing slider workspaces.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxWorkspaceEditWidget.h"
#include "FxFaceGraphNodeGroupManager.h"
#include "FxTearoffWindow.h"
#include "FxAnimController.h"
#include "FxStudioApp.h"
#include "FxCameraManager.h"
#include "FxNewWorkspaceWizard.h"
#include "FxSessionProxy.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/dcbuffer.h" // buffered paint dc
	#include "wx/colordlg.h" // wxGetColourFromUser
#endif

namespace OC3Ent
{

namespace Face
{

BEGIN_EVENT_TABLE(FxWidgetSwitcher, wxWindow)
	EVT_SIZE(FxWidgetSwitcher::OnSize)
END_EVENT_TABLE()

//------------------------------------------------------------------------------
// Workspace edit widget drop target
//------------------------------------------------------------------------------
class FxWorkspaceEditDropTarget : public wxDropTarget
{
public:
	FxWorkspaceEditDropTarget(wxDataObject* data = NULL, wxWindow* parent = NULL)
		: wxDropTarget(data)
		, _lastXPos(FxInvalidValue)
		, _lastYPos(FxInvalidValue)
		, _parent(parent)
	{
	}

	void SetRects(const wxRect& editPaneRect, const wxRect& xAxisRect, const wxRect& yAxisRect)
	{
		_rectEditPane = editPaneRect;
		_rectXAxis = xAxisRect;
		_rectYAxis = yAxisRect;
	}

	virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult FxUnused(def))
	{		
		if( _rectXAxis.Contains(x,y) )
		{
			wxClientDC dc(_parent);
			dc.SetPen(*wxRED_PEN);
			dc.SetBrush(*wxWHITE_BRUSH);
			dc.DrawRectangle(_rectXAxis);
			return wxDragCopy;
		}
		else if( _rectYAxis.Contains(x,y) )
		{
			wxClientDC dc(_parent);
			dc.SetPen(*wxRED_PEN);
			dc.SetBrush(*wxWHITE_BRUSH);
			dc.DrawRectangle(_rectYAxis);
			return wxDragCopy;
		}
		else if( _rectEditPane.Contains(x,y) )
		{
			FxWorkspaceElementSlider slider;
			slider.SetWorkspaceRect(_rectEditPane);
			wxClientDC dc(_parent);
			// Clear the previous draw
			if( _lastXPos != FxInvalidValue )
			{
				slider.SetPosition(_lastXPos, _lastYPos);
				slider.DrawOn(&dc);
			}
			FxReal xPos = static_cast<FxReal>(x - _rectEditPane.GetLeft()) / static_cast<FxReal>(_rectEditPane.GetWidth());
			FxReal yPos = static_cast<FxReal>(y - _rectEditPane.GetTop()) / static_cast<FxReal>(_rectEditPane.GetHeight());
			slider.SetPosition(xPos, yPos);
			slider.DrawOn(&dc);
			_lastXPos = xPos;
			_lastYPos = yPos;
			return wxDragMove;
		}
		else
		{
			_parent->Refresh(FALSE);
			_lastXPos = FxInvalidValue;
			_lastYPos = FxInvalidValue;
		}
		return wxDragNone;
	}

	virtual wxDragResult OnData(wxCoord x ,wxCoord y ,wxDragResult FxUnused(def))
	{
		FxWorkspaceEditWidget* pWidget = static_cast<FxWorkspaceEditWidget*>(_parent);
		GetData();
		wxDragResult result = wxDragNone;

		if( _rectXAxis.Contains(x,y) )
		{
			wxString xAxis = static_cast<wxTextDataObject*>(m_dataObject)->GetText();
			pWidget->SetXAxisNodeName(xAxis);
			result = wxDragCopy;
		}
		else if( _rectYAxis.Contains(x,y) )
		{
			wxString yAxis = static_cast<wxTextDataObject*>(m_dataObject)->GetText();
			pWidget->SetYAxisNodeName(yAxis);
			result = wxDragCopy;
		}
		else if( _rectEditPane.Contains(x,y) )
		{
			wxString slider = static_cast<wxTextDataObject*>(m_dataObject)->GetText();
			pWidget->AddSliderFromDragdrop(slider, wxPoint(x,y));
			result = wxDragMove;
		}
		
		if( result == wxDragCopy || result == wxDragMove )
		{
			_parent->Refresh(FALSE);
		}

		_lastXPos = FxInvalidValue;
		_lastYPos = FxInvalidValue;

		return result;
	}

	virtual void OnLeave()
	{
		_parent->Refresh(FALSE);
		_lastXPos = FxInvalidValue;
		_lastYPos = FxInvalidValue;
	}

protected:
	wxRect _rectEditPane;
	wxRect _rectXAxis;
	wxRect _rectYAxis;
	wxWindow* _parent;
	FxReal _lastXPos;
	FxReal _lastYPos;
};

//------------------------------------------------------------------------------
// Event table.
//------------------------------------------------------------------------------

WX_IMPLEMENT_DYNAMIC_CLASS(FxWorkspaceEditWidget, wxWindow)

BEGIN_EVENT_TABLE(FxWorkspaceEditWidget, wxWindow)
	EVT_SIZE(FxWorkspaceEditWidget::OnSize)
	EVT_PAINT(FxWorkspaceEditWidget::OnPaint)

	EVT_CHOICE(ControlID_WorkspaceEditWidgetSidebarNodeGroupChoice, FxWorkspaceEditWidget::OnSidebarNodeGroupChoiceChange)
	EVT_LIST_BEGIN_DRAG(ControlID_WorkspaceEditWidgetSidebarNodeNameList, FxWorkspaceEditWidget::OnSidebarNodeNameListDragStart)
	EVT_LIST_COL_CLICK(ControlID_WorkspaceEditWidgetSidebarNodeNameList, FxWorkspaceEditWidget::OnSidebarNodeNameListColumnClick)
	EVT_BUTTON(ControlID_WorkspaceEditWidgetSidebarXAxisRemoveButton, FxWorkspaceEditWidget::OnSidebarXAxisRemoveButton)
	EVT_BUTTON(ControlID_WorkspaceEditWidgetSidebarYAxisRemoveButton, FxWorkspaceEditWidget::OnSidebarYAxisRemoveButton)

	EVT_CHOICE(ControlID_WorkspaceEditWidgetToolbarWorkspaceChoice, FxWorkspaceEditWidget::OnToolbarWorkspaceChoiceChange)
	EVT_BUTTON(ControlID_WorkspaceEditWidgetToolbarNewWorkspaceButton, FxWorkspaceEditWidget::OnToolbarNewWorkspaceButton)
	EVT_BUTTON(ControlID_WorkspaceEditWidgetToolbarNewWorkspaceWizardButton, FxWorkspaceEditWidget::OnToolbarNewWorkspaceWizardButton)
	EVT_BUTTON(ControlID_WorkspaceEditWidgetToolbarSaveWorkspaceButton, FxWorkspaceEditWidget::OnToolbarSaveWorkspaceButton)
	EVT_BUTTON(ControlID_WorkspaceEditWidgetToolbarRenameWorkspaceButton, FxWorkspaceEditWidget::OnToolbarRenameWorkspaceButton)
	EVT_BUTTON(ControlID_WorkspaceEditWidgetToolbarRemoveWorkspaceButton, FxWorkspaceEditWidget::OnToolbarRemoveWorkspaceButton)
	EVT_BUTTON(ControlID_WorkspaceEditWidgetToolbarAddBackgroundButton, FxWorkspaceEditWidget::OnToolbarAddBackgroundButton)
	EVT_BUTTON(ControlID_WorkspaceEditWidgetToolbarAddViewportButton, FxWorkspaceEditWidget::OnToolbarAddViewportButton)
	EVT_BUTTON(ControlID_WorkspaceEditWidgetToolbarRemoveSelectedButton, FxWorkspaceEditWidget::OnToolbarRemoveSelectedButton)
	EVT_BUTTON(ControlID_WorkspaceEditWidgetToolbarCloseButton, FxWorkspaceEditWidget::OnToolbarCloseButton)
	EVT_BUTTON(ControlID_WorkspaceEditWidgetToolbarChangeTextColourButton, FxWorkspaceEditWidget::OnToolbarChangeTextColourButton)
	EVT_BUTTON(ControlID_WorkspaceEditWidgetToolbarSetTabOrderButton, FxWorkspaceEditWidget::OnToolbarSetTabOrderButton)

	EVT_LEFT_DOWN(FxWorkspaceEditWidget::OnLeftDown)
	EVT_MOTION(FxWorkspaceEditWidget::OnMouseMove)
	EVT_LEFT_UP(FxWorkspaceEditWidget::OnLeftUp)

	EVT_KEY_DOWN(FxWorkspaceEditWidget::OnKeyDown)
	EVT_KEY_UP(FxWorkspaceEditWidget::OnKeyUp)

	EVT_SYS_COLOUR_CHANGED(FxWorkspaceEditWidget::OnSysColourChanged)
	EVT_HELP_RANGE(wxID_ANY, wxID_HIGHEST, FxWorkspaceEditWidget::OnHelp)
END_EVENT_TABLE()

//------------------------------------------------------------------------------
// Construction/Destruction
//------------------------------------------------------------------------------

#define kMinSidebarWidth 150

FxWorkspaceEditWidget::FxWorkspaceEditWidget(wxWindow* parent, FxWidgetMediator* mediator)
	: Super(parent, -1, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE|wxCLIP_CHILDREN|wxTAB_TRAVERSAL)
	, FxWidget(mediator)
	, _actor(NULL)
	, _anim(NULL)
	, _animController(NULL)
	, _currentSlider(NULL)
	, _canDeleteCurrentSlider(FxFalse)
	, _currentSliderHasCapture(FxFalse)
	, _sliderXAxisNodeName(wxEmptyString)
	, _sliderYAxisNodeName(wxEmptyString)
	, _sidebarWidth(kMinSidebarWidth)
	, _mouseAction(MouseNone)
	, _highlightCreateSlider(FxFalse)
	, _sidebarNodeGroupChoice(NULL)
	, _sidebarNodeNameList(NULL)
	, _valid(FxFalse)
	, _workspaceIsModified(FxFalse)
	, _drawGrid(FxFalse)
	, _canDoViewports(FxFalse)
{
	// Query to see if the renderer supports the necessary features to allow
	// viewport rendering.
	if( _mediator )
	{
		FxRenderWidgetCaps caps;
		_mediator->OnQueryRenderWidgetCaps(this, caps);
		if( caps.renderWidgetName != "Undefined" )
		{
			_canDoViewports = caps.supportsMultipleCameras && caps.supportsOffscreenRenderTargets;
		}
	}
	RefreshSystemColours();
	CreateToolbarControls();
	CreateSidebarControls();
	SetControlStates();
	SetDropTarget(new FxWorkspaceEditDropTarget(new wxTextDataObject, this));
	ResizeRects();
}

FxWorkspaceEditWidget::~FxWorkspaceEditWidget()
{
	ClearCurrentSlider();
}

void FxWorkspaceEditWidget::SetActiveWorkspace( const FxName& activeWorkspace )
{
	FxInt32 index = _toolbarWorkspaceChoice->FindString(wxString::FromAscii(activeWorkspace.GetAsCstr()));
	if( index != -1 )
	{
		_toolbarWorkspaceChoice->SetSelection(index);
		DoLoadSelectedWorkspace();
	}
}

//------------------------------------------------------------------------------
// FxWidget message handlers
//------------------------------------------------------------------------------

void FxWorkspaceEditWidget::OnActorChanged(FxWidget* FxUnused(sender), FxActor* actor)
{
	_actor = actor;
	_valid = _actor ? FxTrue : FxFalse;
	ClearCurrentSlider();
	FillNodeGroupChoice();
	SetControlStates();
	FillWorkspaceChoice();
	_currentWorkspace.SetActor(_actor);
	Refresh(FALSE, &_editPaneRect);		
}

void FxWorkspaceEditWidget::OnFaceGraphNodeGroupSelChanged(FxWidget* FxUnused(sender), const FxName& selGroup)
{
	_selFaceGraphNodeGroup = selGroup;
	SetNodeGroupSelection();
	FillNodeNameList();
}

void FxWorkspaceEditWidget::OnActorInternalDataChanged(FxWidget* FxUnused(sender), FxActorDataChangedFlag flags)
{
	if( flags & ADCF_FaceGraphChanged )
	{
		FillNodeGroupChoice();
		// Force a relinking of the sliders.
		_currentWorkspace.SetActor(_actor);
		Refresh(FALSE, &_editPaneRect);		
	}
	if( flags & ADCF_NodeGroupsChanged )
	{
		FillNodeGroupChoice();
	}
	if( flags & ADCF_WorkspacesChanged )
	{
		FillWorkspaceChoice();
	}
}

void FxWorkspaceEditWidget::OnAnimChanged(FxWidget* FxUnused(sender), const FxName& FxUnused(animGroupName), FxAnim* anim)
{
	_anim = anim;
	_currentWorkspace.SetAnim(_anim);
	Refresh(FALSE, &_editPaneRect);
}

void FxWorkspaceEditWidget::OnTimeChanged(FxWidget* FxUnused(sender), FxReal newTime)
{
	_currentTime = newTime;
	_currentWorkspace.SetCurrentTime(newTime);
	Refresh(FALSE, &_editPaneRect);
}

void FxWorkspaceEditWidget::OnRefresh(FxWidget* FxUnused(sender))
{
	if( _animController )
	{
		Refresh(FALSE, &_editPaneRect);
		Update();
	}
}

void FxWorkspaceEditWidget::OnAppStartup(FxWidget* FxUnused(sender))
{
	_valid = FxTrue;
}

void FxWorkspaceEditWidget::OnAppShutdown(FxWidget* FxUnused(sender))
{
	ClearCurrentSlider();
	_valid = FxFalse;
}

void FxWorkspaceEditWidget::OnLoadActor(FxWidget* FxUnused(sender), const FxString& FxUnused(actorPath))
{
	FillWorkspaceChoice();	
}

void FxWorkspaceEditWidget::OnInteractEditWidget(FxWidget* FxUnused(sender), FxBool isQuery, FxBool& shouldSave)
{
	if( isQuery )
	{
		shouldSave = _workspaceIsModified;
	}
	else if( shouldSave )
	{
		wxCommandEvent trash;
		OnToolbarSaveWorkspaceButton(trash);
	}
}

//------------------------------------------------------------------------------
// Command handlers
//------------------------------------------------------------------------------

void FxWorkspaceEditWidget::OnSidebarNodeGroupChoiceChange(wxCommandEvent& FxUnused(event))
{
	FillNodeNameList();
}

void FxWorkspaceEditWidget::OnSidebarNodeNameListDragStart(wxListEvent& event)
{
	// Set the list control up as a drag source.
	wxTextDataObject data(_sidebarNodeNameList->GetItemText(event.GetIndex()));

	wxDropSource dragSource(data, this);
	//@todo handle the result.
	/*wxDragResult result = dragSource.DoDragDrop(TRUE);*/
	dragSource.DoDragDrop(TRUE);
}

void FxWorkspaceEditWidget::OnSidebarNodeNameListColumnClick(wxListEvent& FxUnused(event))
{
	static FxBool sortAscending = FxTrue;

	if( _actor && _sidebarNodeNameList )
	{
		FxArray<FxString*> itemStrings;
		itemStrings.Reserve(_sidebarNodeNameList->GetItemCount());
		// Tag the nodes.
		FxInt32 index = -1;
		for( ;; )
		{
			index = _sidebarNodeNameList->GetNextItem(index);
			if( index == -1 ) break;
			FxString* itemString = new FxString(_sidebarNodeNameList->GetItemText(index).mb_str(wxConvLibc));
			itemStrings.PushBack(itemString);
			_sidebarNodeNameList->SetItemData(index, reinterpret_cast<long>(itemString));
		}
		_sidebarNodeNameList->SortItems(CompareStrings, static_cast<long>(sortAscending));
		for( FxSize i = 0; i < itemStrings.Length(); ++i )
		{
			delete itemStrings[i];
			itemStrings[i] = NULL;
		}
		itemStrings.Clear();
		sortAscending = !sortAscending;
	}
}

void FxWorkspaceEditWidget::OnSidebarXAxisRemoveButton(wxCommandEvent& FxUnused(event))
{
	if( _currentSlider )
	{
		_currentSlider->SetNodeNameXAxis("");

		if( _currentSlider->GetNodeNameYAxis().IsEmpty() )
		{
			ClearCurrentSlider();
		}

		if( !_canDeleteCurrentSlider )
		{
			FlagWorkspaceModified();
		}

		SetControlStates();
		Refresh(FALSE);
	}
}

void FxWorkspaceEditWidget::OnSidebarYAxisRemoveButton(wxCommandEvent& FxUnused(event))
{
	if( _currentSlider )
	{
		_currentSlider->SetNodeNameYAxis("");

		if( _currentSlider->GetNodeNameXAxis().IsEmpty() )
		{
			ClearCurrentSlider();
		}

		if( !_canDeleteCurrentSlider )
		{
			FlagWorkspaceModified();
		}

		SetControlStates();
		Refresh(FALSE);
	}
}

void FxWorkspaceEditWidget::OnToolbarWorkspaceChoiceChange(wxCommandEvent& FxUnused(event))
{
	PromptToSaveIfNeeded();
	DoLoadSelectedWorkspace();
}

void FxWorkspaceEditWidget::OnToolbarNewWorkspaceButton(wxCommandEvent& event)
{
	PromptToSaveIfNeeded();

	FxString newName;
	for( FxSize i = 1; i < 1000; ++i )
	{
		FxString newNameStr("Workspace ");
		newNameStr << i;
		// This is slightly strange looking because we have to take into account
		// the cases where FX_DISALLOW_SPACES_IN_NAMES is defined, such as in
		// Unreal Engine 3.
		newName = FxName(newNameStr.GetData()).GetAsString();
		FxBool currentTryExists = FxFalse;
		for( FxSize j = 0; j < FxWorkspaceManager::Instance()->GetNumWorkspaces(); ++j )
		{
			currentTryExists |= FxWorkspaceManager::Instance()->GetWorkspace(j).GetNameAsString() == newName;
		}
		if( !currentTryExists )
		{
			break;
		}
	}

	FxWorkspace newWorkspace(newName.GetData());
	FxWorkspaceManager::Instance()->Add(newWorkspace);
	if( _toolbarWorkspaceChoice )
	{
		FillWorkspaceChoice();
		_toolbarWorkspaceChoice->SetStringSelection(wxString::FromAscii(*newName));
		DoLoadSelectedWorkspace();
		OnToolbarRenameWorkspaceButton(event); // Prompt the user to rename the new ws.
	}
}

void FxWorkspaceEditWidget::OnToolbarNewWorkspaceWizardButton(wxCommandEvent& FxUnused(event))
{
	PromptToSaveIfNeeded();

	wxBitmap wizardBmp;
	wxString bmpPath = FxStudioApp::GetBitmapPath(wxT("Wizard.bmp"));
	if( FxFileExists(bmpPath) )
	{
		wizardBmp.LoadFile(bmpPath, wxBITMAP_TYPE_BMP);
	}
	
	wxWizard* newWorkspaceWizard = new wxWizard(this, wxID_ANY, _("Create a new workspace"), wizardBmp);
	FxWizardPageSelectNodes* nodeSelectPage = new FxWizardPageSelectNodes(newWorkspaceWizard);
	newWorkspaceWizard->GetPageAreaSizer()->Add(nodeSelectPage);
	FxWizardPageSelectLayout* layoutSelectPage = new FxWizardPageSelectLayout(newWorkspaceWizard);
	wxWizardPageSimple::Chain(nodeSelectPage, layoutSelectPage);
	FxWizardPageWorkspaceOptions* optionsPage = new FxWizardPageWorkspaceOptions(newWorkspaceWizard);
	wxWizardPageSimple::Chain(layoutSelectPage, optionsPage);

	if( newWorkspaceWizard->RunWizard(nodeSelectPage) )
	{
		FxArray<FxString> sliders;
		nodeSelectPage->GetSliderNames(sliders);
		FxInt32 sliderOrientation = layoutSelectPage->GetSliderOrientation();
		FxInt32 numCols = layoutSelectPage->GetNumCols();
		FxInt32 colSpacing = layoutSelectPage->GetColSpacing();

		FxInt32 slidersPerCol = static_cast<FxInt32>(FxCeil(static_cast<FxReal>(sliders.Length()) / static_cast<FxReal>(numCols)));
		FxInt32 currCol = 0;
		FxInt32 sliderInCol = 0;
		FxWorkspace newWorkspace(FxString(optionsPage->GetWorkspaceName().mb_str(wxConvLibc)).GetData());
		newWorkspace.SetActor(_actor);
		newWorkspace.SetAnimController(_animController);
		newWorkspace.SetAnim(_anim);
		for( FxSize i = 0; i < sliders.Length(); ++i )
		{
			// Handle leftover sliders.
			FxInt32 numSlidersInCurrCol = FxMin<FxInt32>(sliders.Length() - (currCol * slidersPerCol), slidersPerCol);
			FxReal colWidth = 1.0f / numCols;
			FxReal colSpacingPct = colSpacing / 100.f;
			FxReal sliderHeight = (1.0f - 2 * colSpacingPct) / numSlidersInCurrCol;

			FxWorkspaceElementSlider currSlider;
			if( sliderOrientation == 0 )
			{
				// Handle horizontal orientations
				currSlider.SetNodeNameXAxis(sliders[i]);
				currSlider.SetScale(colWidth - 2 * colSpacingPct, sliderHeight - colSpacingPct);
				currSlider.SetPosition(colWidth * currCol + (colWidth * 0.5) + 0.5 * colSpacingPct, sliderHeight * sliderInCol + (sliderHeight * 0.5) + 0.5 * colSpacingPct);
			}
			else
			{
				// Handle vertical orientations.
				currSlider.SetNodeNameYAxis(sliders[i]);
				currSlider.SetScale(sliderHeight - colSpacingPct, colWidth - 2 * colSpacingPct);
				currSlider.SetPosition(sliderHeight * sliderInCol + (sliderHeight * 0.5) + 0.5 * colSpacingPct, colWidth * currCol + (colWidth * 0.5) + 0.5 * colSpacingPct);
			}
			newWorkspace.AddSlider(currSlider);

			++sliderInCol;
			if( sliderInCol == slidersPerCol )
			{
				sliderInCol = 0;
				++currCol;
			}
		}

		FxWorkspaceManager::Instance()->Add(newWorkspace);
		if( _toolbarWorkspaceChoice )
		{
			FillWorkspaceChoice();
			_toolbarWorkspaceChoice->SetStringSelection(wxString::FromAscii(newWorkspace.GetNameAsCstr()));
			DoLoadSelectedWorkspace();
		}
	}

	newWorkspaceWizard->Destroy();
}

void FxWorkspaceEditWidget::OnToolbarSaveWorkspaceButton(wxCommandEvent& FxUnused(event))
{
	FxWorkspaceManager::Instance()->Modify(_currentWorkspace.GetName(), _currentWorkspace);
	_workspaceIsModified = FxFalse;
	_currentWorkspace.ClearDirty();
	FxSessionProxy::SetIsForcedDirty(FxTrue);
	SetControlStates();
}

void FxWorkspaceEditWidget::OnToolbarRenameWorkspaceButton(wxCommandEvent& FxUnused(event))
{
	wxTextEntryDialog textDialog(this, _("Rename workspace: "), _("Rename workspace"), 
		wxString::FromAscii(_currentWorkspace.GetNameAsCstr()));
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	if( textDialog.ShowModal() == wxID_OK )
	{
		FxString newName = FxString(textDialog.GetValue().mb_str(wxConvLibc));
		if( newName != _currentWorkspace.GetNameAsString() )
		{
			// Make sure the name doesn't already exist.
			for( FxSize j = 0; j < FxWorkspaceManager::Instance()->GetNumWorkspaces(); ++j )
			{
				if( FxWorkspaceManager::Instance()->GetWorkspace(j).GetNameAsString() == newName )
				{
					FxMessageBox(_("A workspace with that name already exists."));
					return;
				}
			}

			ClearCurrentSlider();

			FxWorkspaceManager::Instance()->Rename(_currentWorkspace.GetName(), newName.GetData());
			_currentWorkspace.SetName(newName.GetData());
			FillWorkspaceChoice();
			Refresh(FALSE, &_editPaneRect);
		}
	}
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
}

void FxWorkspaceEditWidget::OnToolbarRemoveWorkspaceButton(wxCommandEvent& FxUnused(event))
{
	// Confirm the workspace deletion with the user.
	if( FxMessageBox(_("Delete current workspace?"), _("Delete workspace?"),
		wxYES_NO|wxCENTER|wxICON_QUESTION, this) == wxYES )
	{
		ClearCurrentSlider();

		FxName wsName = _currentWorkspace.GetName();
		FxWorkspaceManager::Instance()->Remove(wsName);
		if( _toolbarWorkspaceChoice )
		{
			_toolbarWorkspaceChoice->SetSelection(0);
			FillWorkspaceChoice();
		}
	}
}

void FxWorkspaceEditWidget::OnToolbarAddBackgroundButton(wxCommandEvent& FxUnused(event))
{
	wxFileDialog fileDlg(this, _("Select a bitmap for background"), wxEmptyString, wxEmptyString, _("BMP files (*.bmp)|*.bmp|All Files (*.*)|*.*"), wxFD_OPEN);
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	if ( fileDlg.ShowModal() == wxID_OK )
	{
		wxImage backgroundImage(fileDlg.GetPath());
		if( backgroundImage.Ok() )
		{
			FxWorkspaceElementBackground newBackgroundElement(backgroundImage);
			newBackgroundElement.SetWorkspaceRect(_editPaneRect);
			newBackgroundElement.SetPosition(0.5f, 0.5f);
			newBackgroundElement.SetScale(1.0f, 1.0f);
			_currentWorkspace.AddBackground(newBackgroundElement);
			FlagWorkspaceModified();
			Refresh(FALSE, &_editPaneRect);
		}
		else
		{
			FxMessageBox(_("Invalid image format."));
		}
	}
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
}

void FxWorkspaceEditWidget::OnToolbarAddViewportButton(wxCommandEvent& FxUnused(event))
{
	FxViewportCameraChooserDialog chooser(this);
	if( chooser.ShowModal() == wxID_OK )
	{
		wxBusyCursor wait;
		FxWorkspaceElementViewport viewport;
		viewport.SetWorkspaceRect(_editPaneRect);
		viewport.SetPosition(0.5f, 0.5f);
		viewport.SetScale(1.0f, 1.0f);
		viewport.SetCameraName(chooser.GetSelectedName());
		_currentWorkspace.AddViewport(viewport);
		FlagWorkspaceModified();
		Refresh(FALSE);
	}
}

void FxWorkspaceEditWidget::OnToolbarRemoveSelectedButton(wxCommandEvent& FxUnused(event))
{
	DoRemoveSelectedElement();
}

void FxWorkspaceEditWidget::OnToolbarCloseButton(wxCommandEvent& FxUnused(event))
{
	PromptToSaveIfNeeded();
	static_cast<FxWidgetSwitcher*>(GetParent())->ToggleChildren(_currentWorkspace.GetName());
}

void FxWorkspaceEditWidget::OnToolbarChangeTextColourButton(wxCommandEvent& FxUnused(event))
{
	wxColour textColour = ::wxGetColourFromUser(this, _currentWorkspace.GetTextColour());
	if( textColour.Ok() )
	{
		_currentWorkspace.SetTextColour(textColour);
		FlagWorkspaceModified();
		Refresh(FALSE);
	}
}

void FxWorkspaceEditWidget::OnToolbarSetTabOrderButton(wxCommandEvent& FxUnused(event))
{
	if( !_toolbarSetTabOrderButton->GetToggleValue() )
	{
		FxMessageBox(_("Click the sliders in the order you want the Tab key to move through them."), _("Create Tab Order"));
		_currentWorkspace.BeginSetTabOrder();

		// Go ahead an switch the control states.
		_toolbarWorkspaceChoice->Enable(FxFalse);
		_toolbarNewWorkspaceButton->Enable(FxFalse);
		_toolbarNewWorkspaceWizardButton->Enable(FxFalse);
		_toolbarSaveWorkspaceButton->Enable(FxFalse);
		_toolbarRenameWorkspaceButton->Enable(FxFalse);
		_toolbarDeleteWorkspaceButton->Enable(FxFalse);
		_toolbarAddBackgroundButton->Enable(FxFalse);
		_toolbarAddViewportButton->Enable(FxFalse);
		_toolbarRemoveSelectedButton->Enable(FxFalse);
		_toolbarChangeTextColourButton->Enable(FxFalse);
	}
	else
	{
		_currentWorkspace.EndSetTabOrder();
		FxMessageBox(_("Your slider tab order has been saved."), _("Tab Order Complete"));
		FlagWorkspaceModified();

		// Go ahead an switch the control states.
		_toolbarWorkspaceChoice->Enable(FxTrue);
		_toolbarNewWorkspaceButton->Enable(FxTrue);
		_toolbarNewWorkspaceWizardButton->Enable(FxTrue);
		_toolbarSaveWorkspaceButton->Enable(_workspaceIsModified);
		_toolbarRenameWorkspaceButton->Enable(!_workspaceIsModified);
		_toolbarDeleteWorkspaceButton->Enable(FxTrue);
		_toolbarAddBackgroundButton->Enable(FxTrue);
		_toolbarAddViewportButton->Enable(FxTrue);
		_toolbarRemoveSelectedButton->Enable(_currentWorkspace.GetSelectedElement() != NULL);
		_toolbarChangeTextColourButton->Enable(FxTrue);
	}
	Refresh(FALSE);
}

//------------------------------------------------------------------------------
// Event Handlers
//------------------------------------------------------------------------------

void FxWorkspaceEditWidget::OnSize(wxSizeEvent& FxUnused(event))
{
	ResizeRects();
	if( _valid )
	{
		RefreshChildren();
	}
}

void FxWorkspaceEditWidget::OnPaint(wxPaintEvent& FxUnused(event))
{
	wxBufferedPaintDC dc(this);
	dc.SetBackground(*wxRED_BRUSH);
	dc.Clear();

	DrawToolbar(&dc);
	DrawSidebar(&dc);
	DrawEditPane(&dc);
}

void FxWorkspaceEditWidget::OnLeftDown(wxMouseEvent& event)
{
	if( _valid )
	{
		if( !HasCapture() )
		{
			SetFocus();
			CaptureMouse();
		}

		if( _currentSliderHasCapture )
		{
			DoTestSlider(event);
			return;
		}

		if( _mouseAction == MouseNone )
		{
			if( event.GetY() >= _sidebarRect.GetTop() && 
				_sidebarRect.GetLeft() <= event.GetX() &&
				event.GetX() <= _sidebarRect.GetLeft() + 5)
			{
				_mouseAction = MouseResizeSidebar;

				// Draw the ghosted sash
				wxWindowDC dc(this);
				DrawSidebarGhostSash(&dc);
			}
			else if( _sidebarCreateSliderRect.Contains(event.GetPosition()) )
			{
				_mouseAction = MouseCreateSliderFromPanel;
			}
			else if( _currentSlider && _sidebarTestSliderRect.Contains(event.GetPosition()) )
			{
				DoTestSlider(event);
			}
			else if( _editPaneRect.Contains(event.GetPosition()) )
			{
				FxInt32 refresh = WRF_None;
				_currentWorkspace.ProcessMouseEvent(event, refresh, FxTrue);

				if( _currentWorkspace.IsSettingTabOrder() )
				{
					if( _currentWorkspace.IsTabOrderComplete() )
					{
						Refresh(FxFalse, &_editPaneRect);
						Update();
						_toolbarSetTabOrderButton->SetValue(FxFalse);
						_currentWorkspace.EndSetTabOrder();
						FxMessageBox(_("Your slider tab order has been saved."), _("Tab Order Complete"));
						FlagWorkspaceModified();
					}
				}
				
				SetControlStates();

				if( refresh & WRF_Workspaces )
				{
					Refresh(FxFalse, &_editPaneRect);
				}

				_mouseAction = MouseInteractWorkspace;
			}
			// @todo handle drag starts from the drop rects.
		}
	}
}

void FxWorkspaceEditWidget::OnMouseMove(wxMouseEvent& event)
{
	if( _valid )
	{
		if( _currentSliderHasCapture )
		{
			DoTestSlider(event);
			return;
		}

		if( _mouseAction == MouseNone || _mouseAction == MouseInteractWorkspace )
		{
			if( event.GetY() >= _sidebarRect.GetTop() && 
				_sidebarRect.GetLeft() <= event.GetX() &&
				event.GetX() <= _sidebarRect.GetLeft() + 5)
			{
				::wxSetCursor(wxCursor(wxCURSOR_SIZEWE));
			}
			else
			{
				if( _currentSlider && _sidebarTestSliderRect.Contains(event.GetPosition()) &&
					_mouseAction != MouseInteractWorkspace )
				{
					DoTestSlider(event);
				}

				FxBool wantHighlight = _sidebarCreateSliderRect.Contains(event.GetPosition());
				if( _currentSlider && _highlightCreateSlider != wantHighlight )
				{
					_highlightCreateSlider = wantHighlight;
					Refresh(FALSE, &_sidebarCreateSliderRect);
				}

				if( _editPaneRect.Contains(event.GetPosition()) )
				{
					::wxSetCursor(wxCursor(wxCURSOR_ARROW));
					FxInt32 refresh = WRF_None;
					_currentWorkspace.ProcessMouseEvent(event, refresh, FxTrue);
					if( refresh & WRF_Workspaces )
					{
						if( !_workspaceIsModified && _currentWorkspace.IsDirty() )
						{
							FlagWorkspaceModified();
						}
						
						Refresh(FxFalse, &_editPaneRect);
					}			
				}
				else if( _mouseAction == MouseInteractWorkspace )
				{
					if( _sidebarTestSliderRect.Contains(event.GetPosition()) )
					{
						//@todo where the hell are the drag/drop cursors.
						::wxSetCursor(wxCursor(wxCURSOR_ARROW));
					}
					else
					{
						::wxSetCursor(wxCursor(wxCURSOR_NO_ENTRY));
					}
				}
				else
				{
					::wxSetCursor(wxCursor(wxCURSOR_ARROW));
				}
			}
		}
		else if( _mouseAction == MouseResizeSidebar )
		{
			wxWindowDC dc(this);
			// Erase the previous ghosted sash.
			DrawSidebarGhostSash(&dc);
			_sidebarWidth = FxClamp(kMinSidebarWidth, _clientRect.GetRight() - event.GetX(), _clientRect.GetWidth());
			// Draw the new ghosted sash.
			DrawSidebarGhostSash(&dc);
		}
		else if( _mouseAction == MouseCreateSliderFromPanel )
		{
			if( _currentSlider )
			{
				// Set the list control up as a drag source.
				wxTextDataObject data(wxString((_currentSlider->GetNodeNameXAxis() + "|" + _currentSlider->GetNodeNameYAxis()).GetData(), wxConvLibc));

				wxDropSource dragSource(data, this);
				//@todo handle the result.
				wxDragResult result = dragSource.DoDragDrop(TRUE);
				if( result == wxDragMove )
				{
					ClearCurrentSlider();
					Refresh(FALSE);
				}

				_mouseAction = MouseNone;
			}
		}
	}
}

void FxWorkspaceEditWidget::OnLeftUp(wxMouseEvent& event)
{
	if( _valid )
	{
		if( _currentSliderHasCapture )
		{
			DoTestSlider(event);
			return;
		}

		if( HasCapture() )
		{
			ReleaseMouse();
		}

		if( _mouseAction == MouseResizeSidebar )
		{
			_sidebarWidth = FxClamp(kMinSidebarWidth, _clientRect.GetRight() - event.GetX(), _clientRect.GetWidth());
			wxWindowDC dc(this);
			// Erase the previous ghosted sash.
			DrawSidebarGhostSash(&dc);
			ResizeRects();
			Refresh(FALSE);
		}

		if( _currentSlider && _sidebarTestSliderRect.Contains(event.GetPosition()) && 
			_mouseAction != MouseInteractWorkspace )
		{
			DoTestSlider(event);
		}
		else if( _editPaneRect.Contains(event.GetPosition()) )
		{
			FxInt32 refresh = WRF_None;
			_currentWorkspace.ProcessMouseEvent(event, refresh, FxTrue);
			if( refresh & WRF_Workspaces )
			{
				Refresh(FxFalse, &_editPaneRect);
			}	
		}
		else if(_mouseAction == MouseInteractWorkspace && 
			_sidebarTestSliderRect.Contains(event.GetPosition()) )
		{
			FxWorkspaceElement* selectedElement = _currentWorkspace.GetSelectedElement();
			FxWorkspaceElementSlider* selSlider = NULL;
			for( FxSize i = 0; i < _currentWorkspace.GetNumSliders(); ++i )
			{
				if( _currentWorkspace.GetSlider(i) == selectedElement )
				{
					selSlider = _currentWorkspace.GetSlider(i);
					break;
				}
			}

			if( selSlider )
			{
				SetXAxisNodeName(wxString::FromAscii(selSlider->GetNodeNameXAxis().GetData()));
				SetYAxisNodeName(wxString::FromAscii(selSlider->GetNodeNameYAxis().GetData()));
				_currentWorkspace.RemoveElement(selSlider);
				_workspaceIsModified = FxTrue;
				SetControlStates();
				Refresh(FALSE);
			}
		}

		_mouseAction = MouseNone;
	}
}

void FxWorkspaceEditWidget::OnKeyDown(wxKeyEvent& event)
{
	if( _valid && (event.GetKeyCode() == WXK_DELETE || event.GetKeyCode() == WXK_NUMPAD_DECIMAL) )
	{
		DoRemoveSelectedElement();
	}

	FxBool cachedDrawGrid = _drawGrid;
	_drawGrid = event.ControlDown();
	if( cachedDrawGrid != _drawGrid )
	{
		Refresh(FxFalse, &_editPaneRect);
	}
}

void FxWorkspaceEditWidget::OnKeyUp(wxKeyEvent& event)
{
	FxBool cachedDrawGrid = _drawGrid;
	_drawGrid = event.ControlDown();
	if( cachedDrawGrid != _drawGrid )
	{
		Refresh(FxFalse, &_editPaneRect);
	}
}

void FxWorkspaceEditWidget::OnSysColourChanged(wxSysColourChangedEvent& FxUnused(event))
{
	RefreshSystemColours();
	Refresh(FALSE);
}

void FxWorkspaceEditWidget::OnHelp(wxHelpEvent& FxUnused(event))
{
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().LoadFile();
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().DisplaySection(wxT("Workspace Tab"));
}

//------------------------------------------------------------------------------
// Public methods.
//------------------------------------------------------------------------------

void FxWorkspaceEditWidget::SetAnimController(FxAnimController* animController)
{
	_animController = animController;
	_currentWorkspace.SetAnimController(_animController);
}

void FxWorkspaceEditWidget::SetXAxisNodeName(const wxString& xAxisNodeName)
{
	if( !_currentSlider )
	{
		_currentSlider = new FxWorkspaceElementSlider();
		_canDeleteCurrentSlider = FxTrue;
		_currentSlider->LinkToAnimController(_animController);
		_currentSlider->SetCurrentTime(_currentTime);
		_currentSlider->SetScale(0.9f, 0.4f);
	}

	wxWindowDC dc(this);
	dc.SetBrush(*wxWHITE_BRUSH);
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(_sidebarXDropRect);
	dc.SetPen(*wxBLACK_PEN);
	dc.SetLogicalFunction(wxINVERT);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW));
	wxRect temp(_sidebarXDropRect);
	temp.Inflate(5,5);
	dc.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	dc.DrawText(xAxisNodeName, _sidebarXDropRect.GetLeft() + 2, _sidebarXDropRect.GetTop() + 2);
	dc.DrawRectangle(temp);
	for( FxSize i = 0; i < 5; ++i )
	{
		dc.DrawRectangle(temp);
		temp.Deflate(1,1);
		dc.DrawRectangle(temp);
		::wxMilliSleep(20);
	}
	dc.DrawRectangle(temp);

	_currentSlider->SetNodeNameXAxis(FxString(xAxisNodeName.mb_str(wxConvLibc)));
	_currentSlider->LinkToNodes(&_actor->GetDecompiledFaceGraph());
	_currentSlider->LinkToAnim(_anim);

	if( !_canDeleteCurrentSlider )
	{
		FlagWorkspaceModified();
	}

	SetControlStates();

	Refresh(FALSE, &_sidebarRect);
}

void FxWorkspaceEditWidget::SetYAxisNodeName(const wxString& yAxisNodeName)
{
	if( !_currentSlider )
	{
		_currentSlider = new FxWorkspaceElementSlider();
		_canDeleteCurrentSlider = FxTrue;
		_currentSlider->LinkToAnim(_anim);
		_currentSlider->LinkToAnimController(_animController);
		_currentSlider->SetCurrentTime(_currentTime);
		_currentSlider->SetScale(0.4f, 0.9f);
	}

	wxWindowDC dc(this);
	dc.SetBrush(*wxWHITE_BRUSH);
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(_sidebarYDropRect);
	dc.SetPen(*wxBLACK_PEN);
	dc.SetLogicalFunction(wxINVERT);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW));
	wxRect temp(_sidebarYDropRect);
	temp.Inflate(5,5);
	dc.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	dc.DrawText(yAxisNodeName, _sidebarYDropRect.GetLeft() + 2, _sidebarYDropRect.GetTop() + 2);
	dc.DrawRectangle(temp);
	for( FxSize i = 0; i < 5; ++i )
	{
		dc.DrawRectangle(temp);
		temp.Deflate(1,1);
		dc.DrawRectangle(temp);
		::wxMilliSleep(20);
	}
	dc.DrawRectangle(temp);

	_currentSlider->SetNodeNameYAxis(FxString(yAxisNodeName.mb_str(wxConvLibc)));
	_currentSlider->LinkToNodes(&_actor->GetDecompiledFaceGraph());
	_currentSlider->LinkToAnim(_anim);

	if( !_canDeleteCurrentSlider )
	{
		FlagWorkspaceModified();
	}

	SetControlStates();

	Refresh(FALSE, &_sidebarRect);
}

void FxWorkspaceEditWidget::AddSliderFromDragdrop(const wxString& sliderDesc, const wxPoint& loc)
{
	// Deselect the currently selected slider.
	ClearCurrentSlider();

	// Parse the slider description.
	FxString description(sliderDesc.mb_str(wxConvLibc));
	FxString xAxis = description.BeforeFirst('|');
	FxString yAxis = description.AfterFirst('|');

	// Create the new slider.
	FxWorkspaceElementSlider newSlider;
	newSlider.SetWorkspaceRect(_editPaneRect);
	newSlider.SetNodeNameXAxis(xAxis);
	newSlider.SetNodeNameYAxis(yAxis);
	newSlider.SetPosition(
		static_cast<FxReal>(loc.x - _editPaneRect.GetLeft()) / static_cast<FxReal>(_editPaneRect.GetWidth()),
		static_cast<FxReal>(loc.y - _editPaneRect.GetTop()) / static_cast<FxReal>(_editPaneRect.GetHeight()));
	newSlider.SetScale(
		xAxis.IsEmpty() ? 0.05f : 0.1f,
		yAxis.IsEmpty() ? 0.05f : 0.1f);
//	newSlider.SetCaption(sliderDesc);
	_currentWorkspace.AddSlider(newSlider);

	FlagWorkspaceModified();
}

//------------------------------------------------------------------------------
// FxWidget message helper functions
//------------------------------------------------------------------------------

void FxWorkspaceEditWidget::SetControlStates()
{
	FxBool controlState = _actor != NULL && !_toolbarSetTabOrderButton->GetToggleValue();
	if( _sidebarNodeGroupChoice && _sidebarNodeNameList )
	{
		_sidebarNodeGroupChoice->Enable(controlState);
		_sidebarNodeNameList->Enable(controlState);
		_sidebarXAxisRemoveButton->Enable(controlState && _currentSlider && !_currentSlider->GetNodeNameXAxis().IsEmpty());
		_sidebarYAxisRemoveButton->Enable(controlState && _currentSlider && !_currentSlider->GetNodeNameYAxis().IsEmpty());
	}

	if( _toolbarWorkspaceChoice )
	{
		_toolbarWorkspaceChoice->Enable(controlState);
		_toolbarNewWorkspaceButton->Enable(controlState);
		_toolbarNewWorkspaceWizardButton->Enable(controlState);
		_toolbarSaveWorkspaceButton->Enable(controlState && _workspaceIsModified);
		_toolbarRenameWorkspaceButton->Enable(controlState && !_workspaceIsModified);
		_toolbarDeleteWorkspaceButton->Enable(controlState);
		_toolbarAddBackgroundButton->Enable(controlState);
		_toolbarAddViewportButton->Enable(controlState && _canDoViewports);
		_toolbarRemoveSelectedButton->Enable(controlState && _currentWorkspace.GetSelectedElement() != NULL);
		_toolbarChangeTextColourButton->Enable(controlState);
		_toolbarSetTabOrderButton->Enable(controlState || _actor != NULL);
	}
}

void FxWorkspaceEditWidget::FillNodeGroupChoice()
{
	if( _sidebarNodeGroupChoice )
	{
		_sidebarNodeGroupChoice->Freeze();
		_sidebarNodeGroupChoice->Clear();
		if( _actor )
		{
			_sidebarNodeGroupChoice->Append(wxString(FxFaceGraphNodeGroupManager::GetAllNodesGroupName().GetData(), wxConvLibc));
			for( FxSize i = 0; i < FxFaceGraphNodeGroupManager::GetNumGroups(); ++i )
			{
				_sidebarNodeGroupChoice->Append(
					wxString(FxFaceGraphNodeGroupManager::GetGroup(i)->GetNameAsCstr(), wxConvLibc));
			}
		}
		_sidebarNodeGroupChoice->Thaw();
	}

	SetNodeGroupSelection();
}

void FxWorkspaceEditWidget::SetNodeGroupSelection()
{
	if( _actor && _sidebarNodeGroupChoice )
	{
		// Select the correct group.
		wxString groupName(_selFaceGraphNodeGroup.GetAsCstr(), wxConvLibc);
		FxInt32 index = _sidebarNodeGroupChoice->FindString(groupName);
		if( index != -1 )
		{
			_sidebarNodeGroupChoice->SetSelection(index);
		}
		else
		{
			// Select the all nodes group.
			_sidebarNodeGroupChoice->SetStringSelection(wxString(FxFaceGraphNodeGroupManager::GetAllNodesGroupName().GetData(), wxConvLibc));
		}

		// Force the node name list box to be filled out.
		FillNodeNameList();
	}
}

void FxWorkspaceEditWidget::FillNodeNameList()
{
	if( _actor && _sidebarNodeGroupChoice && _sidebarNodeNameList )
	{
		_sidebarNodeNameList->Freeze();
		_sidebarNodeNameList->ClearAll();

		wxListItem columnInfo;
		columnInfo.m_mask = wxLIST_MASK_TEXT;
		columnInfo.m_text = _("Node Name");
		columnInfo.m_width = wxLIST_AUTOSIZE;
		_sidebarNodeNameList->InsertColumn(0, columnInfo);

		wxString nodeGroup = _sidebarNodeGroupChoice->GetString(_sidebarNodeGroupChoice->GetSelection());
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
					_sidebarNodeNameList->InsertItem(count, wxString(pNode->GetNameAsCstr(), wxConvLibc));
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
					_sidebarNodeNameList->InsertItem(count, wxString(pGroup->GetNode(i).GetAsCstr(), wxConvLibc));
					count++;
				}
			}
		}
		_sidebarNodeNameList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
		_sidebarNodeNameList->Thaw();
	}
}

void FxWorkspaceEditWidget::FillWorkspaceChoice()
{
	if( _toolbarWorkspaceChoice )
	{
		FxInt32 currentSel = _toolbarWorkspaceChoice->GetSelection();

		_toolbarWorkspaceChoice->Freeze();
		_toolbarWorkspaceChoice->Clear();
		if( _actor )
		{
			for( FxSize i = 0; i < FxWorkspaceManager::Instance()->GetNumWorkspaces(); ++i )
			{
				_toolbarWorkspaceChoice->Append(
					wxString(FxWorkspaceManager::Instance()->GetWorkspace(i).GetNameAsCstr(), wxConvLibc));
			}
		}
		_toolbarWorkspaceChoice->Thaw();

		// Set the current selection.
		_toolbarWorkspaceChoice->SetSelection(currentSel == -1 ? 0 : currentSel);

		DoLoadSelectedWorkspace();
	}
}

void FxWorkspaceEditWidget::DoLoadSelectedWorkspace()
{
	ClearCurrentSlider();
	if( _actor && _toolbarWorkspaceChoice )
	{
		wxString workspace = _toolbarWorkspaceChoice->GetString(_toolbarWorkspaceChoice->GetSelection());
		FxName workspaceName(FxString(workspace.mb_str(wxConvLibc)).GetData());
		_currentWorkspace = FxWorkspaceManager::Instance()->Find(workspaceName);;
		_currentWorkspace.SetActor(_actor);
		_currentWorkspace.SetAnim(_anim);
		_currentWorkspace.SetAnimController(_animController);
		_currentWorkspace.SetCurrentTime(_currentTime);
		_currentWorkspace.SetWorkspaceRect(_editPaneRect);
		Refresh(FALSE, &_editPaneRect);
	}
	else
	{
		_currentWorkspace = FxWorkspace();
	}
	_workspaceIsModified = FxFalse;
	_currentWorkspace.ClearDirty();
	SetControlStates();
}

void FxWorkspaceEditWidget::DoTestSlider(wxMouseEvent& event)
{
	FxInt32 refresh = WRF_None;

	wxRect cachedWorkspaceRect = _currentSlider->GetWorkspaceRect();
	FxVec2 cachedPos   = _currentSlider->GetPosition();
	FxVec2 cachedScale = _currentSlider->GetScale();
	FxReal cachedRot    = _currentSlider->GetRotation();
	_currentSlider->SetWorkspaceRect(_sidebarTestSliderRect);
	_currentSlider->SetPosition(FxVec2(0.5f, 0.5f));
	_currentSlider->SetScale(FxVec2(0.9f, 0.9f));
	_currentSlider->SetRotation(0.0f);
	_currentSliderHasCapture = _currentSlider->ProcessMouse(event, refresh);
	_currentSlider->SetPosition(cachedPos);
	_currentSlider->SetScale(cachedScale);
	_currentSlider->SetRotation(cachedRot);
	_currentSlider->SetWorkspaceRect(cachedWorkspaceRect);

	if( refresh & WRF_Workspaces || refresh & WRF_App )
	{
		// Tell the widgets to update.
		if( _mediator )
		{
			_mediator->OnRefresh(NULL);
		}

		Refresh(FALSE, &_sidebarTestSliderRect);
		Refresh(FALSE, &_editPaneRect);
		Update();
	}
}

void FxWorkspaceEditWidget::DoRemoveSelectedElement()
{
	if( _currentWorkspace.GetSelectedElement() )
	{
		_currentWorkspace.RemoveElement(_currentWorkspace.GetSelectedElement());

		// Clear the current slider, if it's a pointer to something in the workspace.
		if( !_canDeleteCurrentSlider )
		{
			_currentSlider = NULL;
		}

		FlagWorkspaceModified();
		Refresh(FALSE);
	}
}

//------------------------------------------------------------------------------
// Window position management
//------------------------------------------------------------------------------

void FxWorkspaceEditWidget::ResizeRects()
{
	_clientRect = GetClientRect();
	_sidebarWidth = FxClamp(kMinSidebarWidth, _sidebarWidth, _clientRect.GetWidth());

	// Gross window controls.
	_toolbarRect = wxRect(_clientRect.GetLeft(), 
		_clientRect.GetTop(), 
		_clientRect.GetWidth() + 1, 
		FxToolbarHeight);
	_sidebarRect = wxRect(_clientRect.GetRight() - _sidebarWidth, 
		_clientRect.GetTop() + FxToolbarHeight, 
		_sidebarWidth + 1, 
		_clientRect.GetHeight() - FxToolbarHeight + 1);
	_editPaneRect = wxRect(_clientRect.GetLeft(), 
		_toolbarRect.GetBottom() + 1, 
		_clientRect.GetWidth() - _sidebarRect.GetWidth(), 
		_clientRect.GetHeight() - _toolbarRect.GetHeight() + 1);

	MoveControls();
}

void FxWorkspaceEditWidget::MoveControls()
{
	if( _sidebarNodeGroupChoice && _sidebarNodeNameList )
	{
		static const FxInt32 dropRectHeight = 20;
		static const FxInt32 sliderTestHeight = 100;
		static const FxInt32 pad = 5;

		_sidebarNodeGroupChoice->SetSize(wxRect(
			_sidebarRect.GetLeft() + 10, _sidebarRect.GetTop() + 5,
			_sidebarRect.GetWidth() - 15, 20));

		FxInt32 remainingControlHeight = sliderTestHeight + 3 * dropRectHeight + 4 * pad;

		_sidebarNodeNameList->SetSize(wxRect(
			_sidebarRect.GetLeft() + 10, _sidebarRect.GetTop() + 30,
			_sidebarRect.GetWidth() - 15, _sidebarRect.GetHeight() - dropRectHeight - 2 * pad - remainingControlHeight));

		wxSize dropRectButtonSize(20,20);

		_sidebarXDropRect = wxRect(_sidebarRect.GetLeft() + 10,
			_sidebarNodeNameList->GetRect().GetBottom() + pad,
			_sidebarRect.GetWidth() - 15 - dropRectButtonSize.GetWidth(), dropRectHeight);
		_sidebarYDropRect = wxRect(_sidebarRect.GetLeft() + 10,
			_sidebarXDropRect.GetBottom() + pad,
			_sidebarRect.GetWidth() - 15 - dropRectButtonSize.GetWidth(), dropRectHeight);

		_sidebarXAxisRemoveButton->SetSize(wxRect(
			_sidebarXDropRect.GetLeft() + _sidebarXDropRect.GetWidth(), _sidebarXDropRect.GetTop(),
			dropRectButtonSize.GetWidth(), dropRectButtonSize.GetHeight()));
		_sidebarYAxisRemoveButton->SetSize(wxRect(
			_sidebarYDropRect.GetLeft() + _sidebarYDropRect.GetWidth(), _sidebarYDropRect.GetTop(),
			dropRectButtonSize.GetWidth(), dropRectButtonSize.GetHeight()));

		_sidebarTestSliderRect = wxRect(_sidebarRect.GetLeft() + 10,
			_sidebarYDropRect.GetBottom() + pad,
			_sidebarRect.GetWidth() - 15, sliderTestHeight);
		_sidebarCreateSliderRect = wxRect(_sidebarRect.GetLeft() + 10,
			_sidebarTestSliderRect.GetBottom(),
			_sidebarRect.GetWidth() - 15, dropRectHeight);

		FxWorkspaceEditDropTarget* dropTarget = static_cast<FxWorkspaceEditDropTarget*>(GetDropTarget());
		dropTarget->SetRects(_editPaneRect, _sidebarXDropRect, _sidebarYDropRect);

		if( _valid )
		{
			_currentWorkspace.SetWorkspaceRect(_editPaneRect);
		}
	}
}

//------------------------------------------------------------------------------
// Painting
//------------------------------------------------------------------------------

void FxWorkspaceEditWidget::DrawToolbar(wxDC* pDC)
{
	// Draw the toolbar background.
	pDC->SetBrush(wxBrush(_colourFace));
	pDC->SetPen(wxPen(_colourFace));
	pDC->DrawRectangle(_toolbarRect);

	// Shade the edges.
	pDC->SetPen(wxPen(_colourShadow));
	pDC->DrawLine(_toolbarRect.GetLeft(), _toolbarRect.GetBottom(),
				  _toolbarRect.GetRight(), _toolbarRect.GetBottom());

	// Draw some dividing lines.
	pDC->SetPen(wxPen(_colourEngrave));
	FxInt32 line0x = _toolbarCloseButton->GetRect().GetRight() + 3;
	FxInt32 line1x = _toolbarDeleteWorkspaceButton->GetRect().GetRight() + 3;
	FxInt32 line2x = _toolbarAddViewportButton->GetRect().GetRight() + 3;
	pDC->DrawLine(line0x, _toolbarRect.GetTop() + 1, line0x, _toolbarRect.GetBottom() - 1);
	pDC->DrawLine(line1x, _toolbarRect.GetTop() + 1, line1x, _toolbarRect.GetBottom() - 1);
	pDC->DrawLine(line2x, _toolbarRect.GetTop() + 1, line2x, _toolbarRect.GetBottom() - 1);

	// Clean up.
	pDC->SetBrush(wxNullBrush);
	pDC->SetPen(wxNullPen);
}

void FxWorkspaceEditWidget::DrawSidebar(wxDC* pDC)
{
	// Draw the sidebar background.
	pDC->SetBrush(wxBrush(_colourFace));
	pDC->SetPen(wxPen(_colourFace));
	pDC->DrawRectangle(_sidebarRect);

	// Draw the sidebar resize sash.
	pDC->SetPen(wxPen(_colourShadow));
	pDC->DrawLine(_sidebarRect.GetLeft()+4, _sidebarRect.GetTop(), _sidebarRect.GetLeft()+4, _sidebarRect.GetBottom());
	pDC->SetPen(wxPen(_colourHighlight));
	pDC->DrawLine(_sidebarRect.GetLeft()+1, _sidebarRect.GetTop(), _sidebarRect.GetLeft()+1, _sidebarRect.GetBottom());
	pDC->SetPen(wxPen(_colourDarkShadow));
	pDC->DrawLine(_sidebarRect.GetLeft()+5, _sidebarRect.GetTop(), _sidebarRect.GetLeft()+5, _sidebarRect.GetBottom());

	DrawSidebarDropTargets(pDC);
	
	pDC->SetBrush(*wxWHITE_BRUSH);
	pDC->SetPen(wxPen(_colourShadow));
	pDC->DrawRectangle(_sidebarTestSliderRect);

	if( _currentSlider )
	{
		wxRect cachedWorkspaceRect = _currentSlider->GetWorkspaceRect();
		FxVec2 cachedPos   = _currentSlider->GetPosition();
		FxVec2 cachedScale = _currentSlider->GetScale();
		FxReal cachedRot   = _currentSlider->GetRotation();
		_currentSlider->SetWorkspaceRect(_sidebarTestSliderRect);
		_currentSlider->SetPosition(FxVec2(0.5f, 0.5f));
		_currentSlider->SetScale(FxVec2(0.9f, 0.9f));
		_currentSlider->SetRotation(0.0f);
		_currentSlider->DrawOn(pDC);
		_currentSlider->SetPosition(cachedPos);
		_currentSlider->SetScale(cachedScale);
		_currentSlider->SetRotation(cachedRot);
		_currentSlider->SetWorkspaceRect(cachedWorkspaceRect);
	}

	// Clean up.
	pDC->SetPen(wxNullPen);
	pDC->SetBrush(wxNullBrush);
}

void FxWorkspaceEditWidget::DrawSidebarGhostSash(wxDC* pDC)
{
	pDC->SetLogicalFunction(wxINVERT);
	pDC->DrawRectangle(wxRect(
		wxPoint(_clientRect.GetRight() - _sidebarWidth, _sidebarRect.GetTop()),
		wxSize(4, _sidebarRect.GetHeight())));
	pDC->SetLogicalFunction(wxCOPY);
}

void FxWorkspaceEditWidget::DrawSidebarDropTargets(wxDC* pDC)
{
	// Draw the drop target rects.
	pDC->SetPen(wxPen(_colourShadow));
	pDC->SetBrush(*wxWHITE_BRUSH);
	pDC->DrawRectangle(_sidebarXDropRect);
	pDC->DrawRectangle(_sidebarYDropRect);
	if( _currentSlider )
	{
		if( _highlightCreateSlider )
		{
			pDC->SetBrush(wxBrush(wxColour(240, 225, 150)));
		}
		pDC->DrawRectangle(_sidebarCreateSliderRect);
	}
	pDC->SetTextForeground(_colourShadow);

	if( _valid )
	{
		// Save the current clipping box.
		long x = 0, y = 0, w = 0, h = 0;
		pDC->GetClippingBox(&x,&y,&w,&h);

		// Create the fonts to use.
		wxFont systemFont = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
		wxFont systemFontItalic = systemFont;
		systemFontItalic.SetStyle(wxSLANT);

		// Set the clipping region for the x-axis drop rect and draw the text.
		pDC->DestroyClippingRegion();
		pDC->SetClippingRegion(_sidebarXDropRect);
		if( _currentSlider && !_currentSlider->GetNodeNameXAxis().IsEmpty() )
		{
			pDC->SetFont(systemFont);
			pDC->DrawText(wxString::FromAscii(_currentSlider->GetNodeNameXAxis().GetData()),
				_sidebarXDropRect.GetLeft() + 2, _sidebarXDropRect.GetTop() + 2);
		}
		else
		{
			pDC->SetFont(systemFontItalic);
			pDC->DrawText(_("Drop x-axis"), _sidebarXDropRect.GetLeft() + 2, _sidebarXDropRect.GetTop() + 2);
		}

		// Set the clipping region for the y-axis drop rect and draw the text.
		pDC->DestroyClippingRegion();
		pDC->SetClippingRegion(_sidebarYDropRect);
		if( _currentSlider && !_currentSlider->GetNodeNameYAxis().IsEmpty() )
		{
			pDC->SetFont(systemFont);
			pDC->DrawText(wxString::FromAscii(_currentSlider->GetNodeNameYAxis().GetData()),
				_sidebarYDropRect.GetLeft() + 2, _sidebarYDropRect.GetTop() + 2);
		}
		else
		{
			pDC->SetFont(systemFontItalic);
			pDC->DrawText(_("Drop y-axis"), _sidebarYDropRect.GetLeft() + 2, _sidebarYDropRect.GetTop() + 2);
		}

		// Restore the original clipping region.
		pDC->DestroyClippingRegion();

		pDC->SetClippingRegion(_sidebarCreateSliderRect);
		if( _currentSlider )
		{
			pDC->SetFont(systemFontItalic);
			pDC->DrawText(_("Drag to create"), _sidebarCreateSliderRect.GetLeft() + 2, _sidebarCreateSliderRect.GetTop() + 2);
		}
		pDC->DestroyClippingRegion();

		if( w != 0 && h != 0 )
		{
			pDC->SetClippingRegion(x,y,w,h);
		}
	}
}

void FxWorkspaceEditWidget::DrawEditPane(wxDC* pDC)
{
	if( _valid )
	{
		// Allow the elements to draw themselves.
		_currentWorkspace.DrawOn(pDC, FxTrue, _drawGrid);
	}
	else
	{
		pDC->SetPen(*wxTRANSPARENT_PEN);
		pDC->SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE)));
		pDC->DrawRectangle(_editPaneRect);
	}
}

void FxWorkspaceEditWidget::RefreshChildren()
{
	wxWindowList& children = GetChildren();
	wxWindowListNode* node = children.GetFirst();
	while( node )
	{
		node->GetData()->Refresh();
		node = node->GetNext();
	}
}

void FxWorkspaceEditWidget::RefreshSystemColours()
{
	_colourFace = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
	_colourHighlight = wxSystemSettings::GetColour(wxSYS_COLOUR_3DHIGHLIGHT);
	_colourShadow = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
	_colourDarkShadow = wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW);
	_colourEngrave = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
}

//------------------------------------------------------------------------------
// Control Creation
//------------------------------------------------------------------------------

void FxWorkspaceEditWidget::CreateToolbarControls()
{
	FxInt32 xPos = _toolbarRect.GetLeft() + 5;
	_toolbarWorkspaceChoice = new wxChoice(this, ControlID_WorkspaceEditWidgetToolbarWorkspaceChoice, wxPoint(xPos, _toolbarRect.GetTop() + 2), wxSize(100,20));
	xPos += 103;
	_toolbarCloseButton = new FxButton(this, ControlID_WorkspaceEditWidgetToolbarCloseButton, _("X"), wxPoint(xPos, _toolbarRect.GetTop() + 2), wxSize(20,20));
	_toolbarCloseButton->SetToolTip(_("Leave edit mode"));
	_toolbarCloseButton->MakeToolbar();
	xPos += 26;
	_toolbarNewWorkspaceButton = new FxButton(this, ControlID_WorkspaceEditWidgetToolbarNewWorkspaceButton, _("NW"), wxPoint(xPos, _toolbarRect.GetTop() + 2), wxSize(20,20));
	_toolbarNewWorkspaceButton->MakeToolbar();
	_toolbarNewWorkspaceButton->SetToolTip(_("Create a new workspace"));
	xPos += 23;
	_toolbarNewWorkspaceWizardButton = new FxButton(this, ControlID_WorkspaceEditWidgetToolbarNewWorkspaceWizardButton, _("WW"), wxPoint(xPos, _toolbarRect.GetTop() + 2), wxSize(20,20));
	_toolbarNewWorkspaceWizardButton->MakeToolbar();
	_toolbarNewWorkspaceWizardButton->SetToolTip(_("Run the workspace creation wizard"));
	xPos += 23;
	_toolbarSaveWorkspaceButton = new FxButton(this, ControlID_WorkspaceEditWidgetToolbarSaveWorkspaceButton, _("SW"), wxPoint(xPos, _toolbarRect.GetTop() + 2), wxSize(20,20));
	_toolbarSaveWorkspaceButton->MakeToolbar();
	_toolbarSaveWorkspaceButton->SetToolTip(_("Apply changes to this workspace"));
	xPos += 23;
	_toolbarRenameWorkspaceButton = new FxButton(this, ControlID_WorkspaceEditWidgetToolbarRenameWorkspaceButton, _("RW"), wxPoint(xPos, _toolbarRect.GetTop() + 2), wxSize(20,20));
	_toolbarRenameWorkspaceButton->MakeToolbar();
	_toolbarRenameWorkspaceButton->SetToolTip(_("Rename this workspace"));
	xPos += 23;
	_toolbarDeleteWorkspaceButton = new FxButton(this, ControlID_WorkspaceEditWidgetToolbarRemoveWorkspaceButton, _("DW"), wxPoint(xPos, _toolbarRect.GetTop() + 2), wxSize(20,20));
	_toolbarDeleteWorkspaceButton->MakeToolbar();
	_toolbarDeleteWorkspaceButton->SetToolTip(_("Delete this workspace"));
	xPos += 26;
	_toolbarAddBackgroundButton = new FxButton(this, ControlID_WorkspaceEditWidgetToolbarAddBackgroundButton, _("AB"), wxPoint(xPos, _toolbarRect.GetTop() + 2), wxSize(20,20));
	_toolbarAddBackgroundButton->MakeToolbar();
	_toolbarAddBackgroundButton->SetToolTip(_("Add a background bitmap"));
	xPos += 23;
	_toolbarAddViewportButton = new FxButton(this, ControlID_WorkspaceEditWidgetToolbarAddViewportButton, _("AV"), wxPoint(xPos, _toolbarRect.GetTop() + 2), wxSize(20,20));
	_toolbarAddViewportButton->MakeToolbar();
	_toolbarAddViewportButton->SetToolTip(_("Add a viewport"));
	xPos += 26;
	_toolbarRemoveSelectedButton = new FxButton(this, ControlID_WorkspaceEditWidgetToolbarRemoveSelectedButton, _("RS"), wxPoint(xPos, _toolbarRect.GetTop() + 2), wxSize(20,20));
	_toolbarRemoveSelectedButton->MakeToolbar();
	_toolbarRemoveSelectedButton->SetToolTip(_("Remove the selected workspace element"));
	xPos += 23;
	_toolbarChangeTextColourButton = new FxButton(this, ControlID_WorkspaceEditWidgetToolbarChangeTextColourButton, _("TC"), wxPoint(xPos, _toolbarRect.GetTop() + 2), wxSize(20,20));
	_toolbarChangeTextColourButton->MakeToolbar();
	_toolbarChangeTextColourButton->SetToolTip(_("Change the workspace's text color"));
	xPos += 23;
	_toolbarSetTabOrderButton = new FxButton(this, ControlID_WorkspaceEditWidgetToolbarSetTabOrderButton, _("TO"), wxPoint(xPos, _toolbarRect.GetTop() + 2), wxSize(20,20));
	_toolbarSetTabOrderButton->MakeToolbar();
	_toolbarSetTabOrderButton->MakeToggle();
	_toolbarSetTabOrderButton->SetToolTip(_("Set the workspace's tab order"));
	xPos += 23;

	LoadIcons();
}

void FxWorkspaceEditWidget::CreateSidebarControls()
{
	_sidebarNodeGroupChoice = new wxChoice(this, ControlID_WorkspaceEditWidgetSidebarNodeGroupChoice);
	_sidebarNodeNameList = new wxListCtrl(this, ControlID_WorkspaceEditWidgetSidebarNodeNameList,
		wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_HRULES);
	_sidebarXAxisRemoveButton = new FxButton(this, ControlID_WorkspaceEditWidgetSidebarXAxisRemoveButton, _("x"));
	_sidebarXAxisRemoveButton->MakeToolbar();
	_sidebarYAxisRemoveButton = new FxButton(this, ControlID_WorkspaceEditWidgetSidebarYAxisRemoveButton, _("x"));
	_sidebarYAxisRemoveButton->MakeToolbar();
}

void FxWorkspaceEditWidget::ClearCurrentSlider()
{
	if( _canDeleteCurrentSlider )
	{
		delete _currentSlider;
	}
	_currentSlider = NULL;
	_canDeleteCurrentSlider = FxFalse;
}

void FxWorkspaceEditWidget::FlagWorkspaceModified()
{
	_workspaceIsModified = FxTrue;
	FxSessionProxy::SetIsForcedDirty(FxTrue);
	SetControlStates();
}

void FxWorkspaceEditWidget::PromptToSaveIfNeeded()
{
	// If we're editing tab order, close out of it
	if( _toolbarSetTabOrderButton->GetToggleValue() )
	{
		wxCommandEvent event;
		OnToolbarSetTabOrderButton(event);
		_toolbarSetTabOrderButton->SetValue(FxFalse);
	}

	if( _workspaceIsModified )
	{
		if( FxMessageBox(_("Save changes to current workspace?"), _("Save Workspace?"),
			wxYES_NO|wxCENTER|wxICON_QUESTION, this) == wxYES )
		{
			// Simulate a save button press.
			wxCommandEvent event;
			OnToolbarSaveWorkspaceButton(event);
		}
		else
		{
			// Reload the current workspace from the manager.
			DoLoadSelectedWorkspace();
		}
	}
}

void FxWorkspaceEditWidget::LoadIcons()
{
	wxString endEditWorkspacePath = FxStudioApp::GetIconPath(wxT("endeditworkspace.ico"));
	wxString endEditWorkspaceDisabledPath = FxStudioApp::GetIconPath(wxT("endeditworkspace_disabled.ico"));
	if( FxFileExists(endEditWorkspacePath) && FxFileExists(endEditWorkspaceDisabledPath) )
	{
		wxIcon endEditWorkspaceIcon = wxIcon(wxIconLocation(endEditWorkspacePath));
		endEditWorkspaceIcon.SetHeight(16);
		endEditWorkspaceIcon.SetWidth(16);
		wxIcon endEditWorkspaceDisabledIcon = wxIcon(wxIconLocation(endEditWorkspaceDisabledPath));
		endEditWorkspaceDisabledIcon.SetHeight(16);
		endEditWorkspaceDisabledIcon.SetWidth(16);

		_toolbarCloseButton->SetEnabledBitmap(endEditWorkspaceIcon);
		_toolbarCloseButton->SetDisabledBitmap(endEditWorkspaceDisabledIcon);
	}

	wxString newPath = FxStudioApp::GetIconPath(wxT("new.ico"));
	wxString newDisabledPath = FxStudioApp::GetIconPath(wxT("new_disabled.ico"));
	if( FxFileExists(newPath) && FxFileExists(newDisabledPath) )
	{
		wxIcon newIcon = wxIcon(wxIconLocation(newPath));
		newIcon.SetHeight(16);
		newIcon.SetWidth(16);
		wxIcon newDisabledIcon = wxIcon(wxIconLocation(newDisabledPath));
		newDisabledIcon.SetHeight(16);
		newDisabledIcon.SetWidth(16);

		_toolbarNewWorkspaceButton->SetEnabledBitmap(newIcon);
		_toolbarNewWorkspaceButton->SetDisabledBitmap(newDisabledIcon);
	}

	wxString workspacewizardPath = FxStudioApp::GetIconPath(wxT("workspacewizard.ico"));
	wxString workspacewizardDisabledPath = FxStudioApp::GetIconPath(wxT("workspacewizard_disabled.ico"));
	if( FxFileExists(workspacewizardPath) && FxFileExists(workspacewizardDisabledPath) )
	{
		wxIcon workspacewizardIcon = wxIcon(wxIconLocation(workspacewizardPath));
		workspacewizardIcon.SetHeight(16);
		workspacewizardIcon.SetWidth(16);
		wxIcon workspacewizardDisabledIcon = wxIcon(wxIconLocation(workspacewizardDisabledPath));
		workspacewizardDisabledIcon.SetHeight(16);
		workspacewizardDisabledIcon.SetWidth(16);

		_toolbarNewWorkspaceWizardButton->SetEnabledBitmap(workspacewizardIcon);
		_toolbarNewWorkspaceWizardButton->SetDisabledBitmap(workspacewizardDisabledIcon);
	}

	wxString savePath = FxStudioApp::GetIconPath(wxT("apply.ico"));
	wxString saveDisabledPath = FxStudioApp::GetIconPath(wxT("apply_disabled.ico"));
	if( FxFileExists(savePath) && FxFileExists(saveDisabledPath) )
	{
		wxIcon saveIcon = wxIcon(wxIconLocation(savePath));
		saveIcon.SetHeight(16);
		saveIcon.SetWidth(16);
		wxIcon saveDisabledIcon = wxIcon(wxIconLocation(saveDisabledPath));
		saveDisabledIcon.SetHeight(16);
		saveDisabledIcon.SetWidth(16);

		_toolbarSaveWorkspaceButton->SetEnabledBitmap(saveIcon);
		_toolbarSaveWorkspaceButton->SetDisabledBitmap(saveDisabledIcon);
	}

	wxString renamePath = FxStudioApp::GetIconPath(wxT("rename.ico"));
	wxString renameDisabledPath = FxStudioApp::GetIconPath(wxT("rename_disabled.ico"));
	if( FxFileExists(renamePath) && FxFileExists(renameDisabledPath) )
	{
		wxIcon renameIcon = wxIcon(wxIconLocation(renamePath));
		renameIcon.SetHeight(16);
		renameIcon.SetWidth(16);
		wxIcon renameDisabledIcon = wxIcon(wxIconLocation(renameDisabledPath));
		renameDisabledIcon.SetHeight(16);
		renameDisabledIcon.SetWidth(16);

		_toolbarRenameWorkspaceButton->SetEnabledBitmap(renameIcon);
		_toolbarRenameWorkspaceButton->SetDisabledBitmap(renameDisabledIcon);
	}

	wxString removeworkspacePath = FxStudioApp::GetIconPath(wxT("removeworkspace.ico"));
	wxString removeworkspaceDisabledPath = FxStudioApp::GetIconPath(wxT("removeworkspace_disabled.ico"));
	if( FxFileExists(removeworkspacePath) && FxFileExists(removeworkspaceDisabledPath) )
	{
		wxIcon removeworkspaceIcon = wxIcon(wxIconLocation(removeworkspacePath));
		removeworkspaceIcon.SetHeight(16);
		removeworkspaceIcon.SetWidth(16);
		wxIcon removeworkspaceDisabledIcon = wxIcon(wxIconLocation(removeworkspaceDisabledPath));
		removeworkspaceDisabledIcon.SetHeight(16);
		removeworkspaceDisabledIcon.SetWidth(16);

		_toolbarDeleteWorkspaceButton->SetEnabledBitmap(removeworkspaceIcon);
		_toolbarDeleteWorkspaceButton->SetDisabledBitmap(removeworkspaceDisabledIcon);
	}

	wxString addbackgroundPath = FxStudioApp::GetIconPath(wxT("addbackground.ico"));
	wxString addbackgroundDisabledPath = FxStudioApp::GetIconPath(wxT("addbackground_disabled.ico"));
	if( FxFileExists(addbackgroundPath) && FxFileExists(addbackgroundDisabledPath) )
	{
		wxIcon addbackgroundIcon = wxIcon(wxIconLocation(addbackgroundPath));
		addbackgroundIcon.SetHeight(16);
		addbackgroundIcon.SetWidth(16);
		wxIcon addbackgroundDisabledIcon = wxIcon(wxIconLocation(addbackgroundDisabledPath));
		addbackgroundDisabledIcon.SetHeight(16);
		addbackgroundDisabledIcon.SetWidth(16);

		_toolbarAddBackgroundButton->SetEnabledBitmap(addbackgroundIcon);
		_toolbarAddBackgroundButton->SetDisabledBitmap(addbackgroundDisabledIcon);
	}

	wxString addviewportPath = FxStudioApp::GetIconPath(wxT("addviewport.ico"));
	wxString addviewportDisabledPath = FxStudioApp::GetIconPath(wxT("addviewport_disabled.ico"));
	if( FxFileExists(addviewportPath) && FxFileExists(addviewportDisabledPath) )
	{
		wxIcon addviewportIcon = wxIcon(wxIconLocation(addviewportPath));
		addviewportIcon.SetHeight(16);
		addviewportIcon.SetWidth(16);
		wxIcon addviewportDisabledIcon = wxIcon(wxIconLocation(addviewportDisabledPath));
		addviewportDisabledIcon.SetHeight(16);
		addviewportDisabledIcon.SetWidth(16);

		_toolbarAddViewportButton->SetEnabledBitmap(addviewportIcon);
		_toolbarAddViewportButton->SetDisabledBitmap(addviewportDisabledIcon);
	}

	wxString removeselectedPath = FxStudioApp::GetIconPath(wxT("remove.ico"));
	wxString removeselectedDisabledPath = FxStudioApp::GetIconPath(wxT("remove_disabled.ico"));
	if( FxFileExists(removeselectedPath) && FxFileExists(removeselectedDisabledPath) )
	{
		wxIcon removeselectedIcon = wxIcon(wxIconLocation(removeselectedPath));
		removeselectedIcon.SetHeight(16);
		removeselectedIcon.SetWidth(16);
		wxIcon removeselectedDisabledIcon = wxIcon(wxIconLocation(removeselectedDisabledPath));
		removeselectedDisabledIcon.SetHeight(16);
		removeselectedDisabledIcon.SetWidth(16);

		_toolbarRemoveSelectedButton->SetEnabledBitmap(removeselectedIcon);
		_toolbarRemoveSelectedButton->SetDisabledBitmap(removeselectedDisabledIcon);
	}

	wxString colourPath = FxStudioApp::GetIconPath(wxT("colour.ico"));
	wxString colourDisabledPath = FxStudioApp::GetIconPath(wxT("colour_disabled.ico"));
	if( FxFileExists(colourPath) && FxFileExists(colourDisabledPath) )
	{
		wxIcon colourIcon = wxIcon(wxIconLocation(colourPath));
		colourIcon.SetHeight(16);
		colourIcon.SetWidth(16);
		wxIcon colourDisabledIcon = wxIcon(wxIconLocation(colourDisabledPath));
		colourDisabledIcon.SetHeight(16);
		colourDisabledIcon.SetWidth(16);

		_toolbarChangeTextColourButton->SetEnabledBitmap(colourIcon);
		_toolbarChangeTextColourButton->SetDisabledBitmap(colourDisabledIcon);
	}

	wxString tabOrderPath = FxStudioApp::GetIconPath(wxT("taborder.ico"));
	wxString tabOrderDisabledPath = FxStudioApp::GetIconPath(wxT("taborder_disabled.ico"));
	if( FxFileExists(tabOrderPath) && FxFileExists(tabOrderDisabledPath) )
	{
		wxIcon tabOrderIcon = wxIcon(wxIconLocation(tabOrderPath));
		tabOrderIcon.SetHeight(16);
		tabOrderIcon.SetWidth(16);
		wxIcon tabOrderDisabledIcon = wxIcon(wxIconLocation(tabOrderDisabledPath));
		tabOrderDisabledIcon.SetHeight(16);
		tabOrderDisabledIcon.SetWidth(16);

		_toolbarSetTabOrderButton->SetEnabledBitmap(tabOrderIcon);
		_toolbarSetTabOrderButton->SetDisabledBitmap(tabOrderDisabledIcon);
	}
}

//------------------------------------------------------------------------------
// A dialog for choosing a camera for a viewport
//------------------------------------------------------------------------------
WX_IMPLEMENT_DYNAMIC_CLASS(FxViewportCameraChooserDialog, wxDialog)

BEGIN_EVENT_TABLE(FxViewportCameraChooserDialog, wxDialog)
	EVT_PAINT(FxViewportCameraChooserDialog::OnPaint)
	EVT_LISTBOX(ControlID_WorkspaceEditWidgetViewportCameraChooserDlgCameraListBox, FxViewportCameraChooserDialog::OnListboxSelectionChanged)
END_EVENT_TABLE()

// Constructors.
FxViewportCameraChooserDialog::FxViewportCameraChooserDialog()
{
}

FxViewportCameraChooserDialog::FxViewportCameraChooserDialog(
	wxWindow* parent, wxWindowID id, const wxString& caption, 
	const wxPoint& pos, const wxSize& size, long style)
{
	Create(parent, id, caption, pos, size, style);
}

FxBool FxViewportCameraChooserDialog::Create(
	wxWindow* parent, wxWindowID id, const wxString& caption, 
	const wxPoint& pos, const wxSize& size, long style)
{
	wxDialog::Create(parent, id, caption, pos, size, style);

	CreateControls();
	GetSizer()->Fit(this);
	return FxTrue;
}

void FxViewportCameraChooserDialog::CreateControls()
{
	wxBusyCursor wait;

	// Get the camera names.
	FxSize numChoices = FxCameraManager::GetNumCameras() + 1;
	wxString* choices = new wxString[numChoices];
	choices[0] = wxString(_("Default"));
	for( FxSize i = 0; i < numChoices - 1; ++i )
	{
		choices[i+1] = wxString::FromAscii(FxCameraManager::GetCamera(i)->GetNameAsCstr());
	}

	wxRect clientRect = GetClientRect();

	wxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	wxSizer* topRow = new wxBoxSizer(wxHORIZONTAL);
	_cameraList = new wxListBox(this, ControlID_WorkspaceEditWidgetViewportCameraChooserDlgCameraListBox,
		wxDefaultPosition, wxSize(200, 200), numChoices, choices, wxLB_SINGLE);
	topRow->Add(_cameraList, 1, wxEXPAND|wxALL, 5);
	topRow->AddStretchSpacer(1);
	mainSizer->Add(topRow);

	wxSizer* bottomRow = new wxBoxSizer(wxHORIZONTAL);
	// Bottom row
	wxButton* okButton = new wxButton(this, wxID_OK, _("&OK"));
	bottomRow->Add(okButton, 0, wxALL|wxALIGN_RIGHT, 5);
	wxButton* cancelButton = new wxButton(this, wxID_CANCEL, _("&Cancel"));
	bottomRow->Add(cancelButton, 0, wxALL|wxALIGN_RIGHT, 5);
	mainSizer->Add(bottomRow, 0, wxALIGN_RIGHT, 0);

	SetSizer(mainSizer);
	SetAutoLayout(TRUE);
	Layout();

	_workspace.SetWorkspaceRect(wxRect(wxPoint(215, 5), wxSize(200, 200)));
	FxWorkspaceElementViewport viewport;
	viewport.SetWorkspaceRect(wxRect(wxPoint(215, 5), wxSize(200, 200)));
	viewport.SetPosition(0.5f, 0.5f);
	viewport.SetScale(1.0f, 1.0f);
	_workspace.AddViewport(viewport);

	// Set the initial selection.
	_cameraList->SetSelection(0);

	delete[] choices;
	choices = NULL;
}

// Returns the selected name.
const FxName& FxViewportCameraChooserDialog::GetSelectedName()
{
	return _workspace.GetViewport(0)->GetCameraName();
}

void FxViewportCameraChooserDialog::OnPaint(wxPaintEvent& FxUnused(event))
{
	wxBufferedPaintDC dc(this);
	dc.SetBackground(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE)));
	dc.Clear();

	_workspace.DrawOn(&dc);
}

void FxViewportCameraChooserDialog::OnListboxSelectionChanged(wxCommandEvent& FxUnused(event))
{
	// Workspace is *guaranteed* to have one element - the viewport.
	FxWorkspaceElementViewport* pViewport = _workspace.GetViewport(0);

	// Get the name of the selected element.
	wxString selectedElement = _cameraList->GetStringSelection();
	if( !selectedElement.IsEmpty() )
	{
		if( selectedElement == _("Default") )
		{
			// Set the null name.
			pViewport->SetCameraName(FxName::NullName);
		}
		else
		{
			pViewport->SetCameraName(FxString(selectedElement.mb_str(wxConvLibc)).GetData());
		}

		Refresh(FALSE);
	}
}

} // namespace Face

} // namespace OC3Ent
