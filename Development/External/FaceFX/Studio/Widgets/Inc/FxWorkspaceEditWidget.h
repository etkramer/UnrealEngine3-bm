//------------------------------------------------------------------------------
// The widget for editing slider workspaces.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxWorkspaceEditWidget_H__
#define FxWorkspaceEditWidget_H__

#include "FxWidget.h"
#include "FxWorkspaceManager.h"
#include "FxButton.h"
#include "FxWorkspaceWidget.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/dnd.h" // drag and drop support.
	#include "wx/listctrl.h" // list control
#endif

namespace OC3Ent
{

namespace Face
{

class FxWorkspaceEditWidget : public wxWindow, public FxWidget
{
	typedef wxWindow Super;
	WX_DECLARE_DYNAMIC_CLASS(FxWorkspaceEditWidget)
	DECLARE_EVENT_TABLE()

public:
	FxWorkspaceEditWidget(wxWindow* parent = NULL, FxWidgetMediator* mediator = NULL);
	~FxWorkspaceEditWidget();

	// Sets the active workspace.
	void SetActiveWorkspace( const FxName& activeWorkspace );

//------------------------------------------------------------------------------
// Widget messages.
//------------------------------------------------------------------------------
	virtual void OnActorChanged(FxWidget* sender, FxActor* actor);
	virtual void OnFaceGraphNodeGroupSelChanged(FxWidget* sender, const FxName& selGroup); 
	virtual void OnActorInternalDataChanged(FxWidget* sender, FxActorDataChangedFlag flags);
	virtual void OnAnimChanged(FxWidget* sender, const FxName& animGroupName, FxAnim* anim);
	virtual void OnTimeChanged(FxWidget* sender, FxReal newTime);
	virtual void OnRefresh(FxWidget* sender);

	virtual void OnAppStartup(FxWidget* sender);
	virtual void OnAppShutdown(FxWidget* sender);

	virtual void OnLoadActor(FxWidget* sender, const FxString& actorPath);

	virtual void OnInteractEditWidget(FxWidget* sender, FxBool isQuery, FxBool& shouldSave);

//------------------------------------------------------------------------------
// Command handlers.
//------------------------------------------------------------------------------
	void OnSidebarNodeGroupChoiceChange(wxCommandEvent& event);
	void OnSidebarNodeNameListDragStart(wxListEvent& event);
	void OnSidebarNodeNameListColumnClick(wxListEvent& event);

	void OnSidebarXAxisRemoveButton(wxCommandEvent& event);
	void OnSidebarYAxisRemoveButton(wxCommandEvent& event);

	void OnToolbarWorkspaceChoiceChange(wxCommandEvent& event);

	void OnToolbarNewWorkspaceButton(wxCommandEvent& event);
	void OnToolbarNewWorkspaceWizardButton(wxCommandEvent& event);
	void OnToolbarSaveWorkspaceButton(wxCommandEvent& event);
	void OnToolbarRenameWorkspaceButton(wxCommandEvent& event);
	void OnToolbarRemoveWorkspaceButton(wxCommandEvent& event);
	void OnToolbarAddBackgroundButton(wxCommandEvent& event);
	void OnToolbarAddViewportButton(wxCommandEvent& event);
	void OnToolbarRemoveSelectedButton(wxCommandEvent& event);
	void OnToolbarCloseButton(wxCommandEvent& event);
	void OnToolbarChangeTextColourButton(wxCommandEvent& event);
	void OnToolbarSetTabOrderButton(wxCommandEvent& event);

//------------------------------------------------------------------------------
// Event handlers.
//------------------------------------------------------------------------------
	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);

	void OnLeftDown(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);
	void OnLeftUp(wxMouseEvent& event);

	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);

	void OnSysColourChanged(wxSysColourChangedEvent& event);

	void OnHelp(wxHelpEvent& event);

//------------------------------------------------------------------------------
// Public methods.
//------------------------------------------------------------------------------
	void SetAnimController(FxAnimController* animController);
	void SetXAxisNodeName(const wxString& xAxisNodeName);
	void SetYAxisNodeName(const wxString& yAxisNodeName);
	void AddSliderFromDragdrop(const wxString& sliderDesc, const wxPoint& loc);

protected:
//------------------------------------------------------------------------------
// Member functions.
//------------------------------------------------------------------------------
	// FxWidget message helper functions.
	void SetControlStates();
	void FillNodeGroupChoice();
	void SetNodeGroupSelection();
	void FillNodeNameList();
	void FillWorkspaceChoice();
	void DoLoadSelectedWorkspace();
	void DoTestSlider(wxMouseEvent& event);
	void DoRemoveSelectedElement();

	// Window position management
	void ResizeRects();
	void MoveControls();

	// Painting
	void DrawToolbar(wxDC* pDC);
	void DrawSidebar(wxDC* pDC);
	void DrawSidebarGhostSash(wxDC* pDC);
	void DrawSidebarDropTargets(wxDC* pDC);
	void DrawEditPane(wxDC* pDC);
	void RefreshChildren();

	// Pull the relevant colours.
	void RefreshSystemColours();

	// Create the toolbar controls.
	void CreateToolbarControls();
	// Create the sidebar controls.
	void CreateSidebarControls();

	// Clear the current slider selection.
	void ClearCurrentSlider();

	// Flag the workspace as dirty.
	void FlagWorkspaceModified();
	// Prompt to save the workspace if needed.
	void PromptToSaveIfNeeded();

	// Initialize the icons.
	void LoadIcons();

//------------------------------------------------------------------------------
// Member variables.
//------------------------------------------------------------------------------
	// Actor data.
	FxActor* _actor;
	FxAnim*  _anim;
	FxReal	 _currentTime;
	FxName   _selFaceGraphNodeGroup;
	FxAnimController* _animController;

	// Current workspace.
	FxWorkspace _currentWorkspace;

	// Current slider data.
	FxWorkspaceElementSlider* _currentSlider;
	FxBool	 _canDeleteCurrentSlider;
	FxBool	 _currentSliderHasCapture;
	wxString _sliderXAxisNodeName;
	wxString _sliderYAxisNodeName;

	// Relevant colours.
	wxColour _colourFace;
	wxColour _colourShadow;
	wxColour _colourDarkShadow;
	wxColour _colourHighlight;
	wxColour _colourEngrave;

	// Sub-window layout.
	wxRect _clientRect;
	wxRect _toolbarRect;
	wxRect _sidebarRect;
	wxRect _sidebarXDropRect;
	wxRect _sidebarYDropRect;
	wxRect _sidebarTestSliderRect;
	wxRect _sidebarCreateSliderRect;
	wxRect _editPaneRect;
	FxInt32 _sidebarWidth;

	// Mouse movement.
	enum MouseAction
	{
		MouseNone,
		MouseResizeSidebar,
		MouseCreateSliderFromPanel,
		MouseInteractWorkspace
	};
	MouseAction _mouseAction;
	FxBool		_highlightCreateSlider;

	// Toolbar controls.
	wxChoice*	_toolbarWorkspaceChoice;
	FxButton*	_toolbarNewWorkspaceButton;
	FxButton*	_toolbarNewWorkspaceWizardButton;
	FxButton*	_toolbarSaveWorkspaceButton;
	FxButton*	_toolbarRenameWorkspaceButton;
	FxButton*	_toolbarDeleteWorkspaceButton;
	FxButton*	_toolbarAddBackgroundButton;
	FxButton*	_toolbarAddViewportButton;
	FxButton*	_toolbarRemoveSelectedButton;
	FxButton*	_toolbarCloseButton;
	FxButton*	_toolbarChangeTextColourButton;
	FxButton*	_toolbarSetTabOrderButton;

	// Sidebar controls.
	wxChoice*	_sidebarNodeGroupChoice;
	wxListCtrl*	_sidebarNodeNameList;
	FxButton*	_sidebarXAxisRemoveButton;
	FxButton*	_sidebarYAxisRemoveButton;

	// Whether the control is valid.
	FxBool		_valid;
	// Whether the workspace has been modified.
	FxBool		_workspaceIsModified;
	// Whether or not to draw a grid.
	FxBool		_drawGrid;
	// Whether or not the renderer supports viewports.
	FxBool		_canDoViewports;
};

class FxWidgetSwitcher : public wxWindow
{
DECLARE_EVENT_TABLE()
public:
	FxWidgetSwitcher(wxWindow* parent)
		: wxWindow(parent, wxID_ANY)
		, _child1(NULL)
		, _child2(NULL)
		, _useChild1(FxTrue)
	{
	}

	void Initialize(FxWorkspaceWidget* child1, FxWorkspaceEditWidget* child2)
	{
		_child1 = child1;
		_child2 = child2;
		//Connect(wxID_ANY, wxEVT_SIZE, (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxSizeEventFunction, FxWidgetSwitcher::OnSize));
		_useChild1 = FxFalse;
		ToggleChildren(FxName::NullName);
	}

	void ToggleChildren(const FxName& activeWorkspace)
	{
		_useChild1 = !_useChild1;
		_child1->Show(_useChild1);
		_child1->Enable(_useChild1);
		_child2->Show(!_useChild1);
		_child2->Enable(!_useChild1);
		if( _useChild1 )
		{
			_child1->SetActiveWorkspace(activeWorkspace);
			_child1->SetFocus();
			_child1->Refresh();
		}
		else
		{
			_child2->SetActiveWorkspace(activeWorkspace);
			_child2->SetFocus();
			_child2->Refresh();
		}
	}

	void OnSize(wxSizeEvent& event)
	{
		_child1->SetSize(event.GetSize());
		_child2->SetSize(event.GetSize());
	}

protected:
	FxWorkspaceWidget* _child1;
	FxWorkspaceEditWidget* _child2;
	FxBool _useChild1;
};

//------------------------------------------------------------------------------
// A dialog for choosing a camera for a viewport.
//------------------------------------------------------------------------------

class FxViewportCameraChooserDialog : public wxDialog
{
	WX_DECLARE_DYNAMIC_CLASS(FxViewportCameraChooserDialog)
	DECLARE_EVENT_TABLE()

public:
	// Constructors.
	FxViewportCameraChooserDialog();
	FxViewportCameraChooserDialog(wxWindow* parent, wxWindowID id = wxID_ANY, 
		const wxString& caption = _("Choose Viewport Camera"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(410,280), 
		long style = wxDEFAULT_DIALOG_STYLE);

	// Creation.
	FxBool Create(wxWindow* parent, wxWindowID id = wxID_ANY, 
		const wxString& caption = _("Choose Viewport Camera"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(410,280), 
		long style = wxDEFAULT_DIALOG_STYLE);

	// Creates the controls and sizers.
	void CreateControls();

	// Returns the selected name.
	const FxName& GetSelectedName();

protected:
	void OnPaint(wxPaintEvent& event);
	void OnListboxSelectionChanged(wxCommandEvent& event);

	wxListBox*  _cameraList;
	FxWorkspace _workspace;
};

} // namespace Face

} // namespace OC3Ent

#endif

