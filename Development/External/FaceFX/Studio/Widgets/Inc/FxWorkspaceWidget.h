//------------------------------------------------------------------------------
// The widget for interacting with slider workspaces.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxWorkspaceWidget_H__
#define FxWorkspaceWidget_H__

#include "FxWidget.h"
#include "FxWorkspaceManager.h"
#include "FxButton.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
#endif

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// FxWorkspaceWidget
//
// The parent workspace window, with the toolbar and related controls.
//------------------------------------------------------------------------------

class FxWorkspaceWidget : public wxWindow, public FxWidget, public FxWorkspaceContainer
{
	typedef wxWindow Super;
	WX_DECLARE_DYNAMIC_CLASS(FxWorkspaceWidget)
	DECLARE_EVENT_TABLE()

public:
	FxWorkspaceWidget(wxWindow* parent = NULL, FxWidgetMediator* mediator = NULL);
	~FxWorkspaceWidget();

	// Sets the active workspace.
	void SetActiveWorkspace( const FxName& activeWorkspace );

//------------------------------------------------------------------------------
// Widget messages.
//------------------------------------------------------------------------------
	virtual void OnActorChanged(FxWidget* sender, FxActor* actor);
	virtual void OnActorInternalDataChanged(FxWidget* sender, FxActorDataChangedFlag flags);
	virtual void OnAnimChanged(FxWidget* sender, const FxName& animGroupName, FxAnim* anim);
	virtual void OnPhonemeMappingChanged(FxWidget* sender, FxPhonemeMap* phonemeMap);
	virtual void OnTimeChanged(FxWidget* sender, FxReal newTime);
	virtual void OnRefresh(FxWidget* sender);

	virtual void OnAppStartup(FxWidget* sender);
	virtual void OnAppShutdown(FxWidget* sender);

//------------------------------------------------------------------------------
// Event handlers.
//------------------------------------------------------------------------------
	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnMouse(wxMouseEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnSystemColoursChanged(wxSysColourChangedEvent& event);
	void OnHelp(wxHelpEvent& event);

	void OnToolbarWorkspaceChoiceChange(wxCommandEvent& event);
	void OnToolbarEditWorkspaceButton(wxCommandEvent& event);
	void OnToolbarModeChoiceChange(wxCommandEvent& event);
	void OnToolbarDisplayNamesToggle(wxCommandEvent& event);
	void OnToolbarWorkspaceOptionChange(wxCommandEvent& event);
	void OnToolbarAnimModeKeyAllButton(wxCommandEvent& event);
	void OnToolbarAnimModePrevKeyButton(wxCommandEvent& event);
	void OnToolbarAnimModeNextKeyButton(wxCommandEvent& event);
	void OnToolbarSharedModeClearButton(wxCommandEvent& event);
	void OnToolbarSharedModeLoadCurrentFrameButton(wxCommandEvent& event);
	void OnToolbarCombinerModeCreateButton(wxCommandEvent& event);
	void OnToolbarMappingModeApplyButton(wxCommandEvent& event);
	void OnToolbarMappingModePhonemeChoiceChange(wxCommandEvent& event);
	void OnToolbarInstanceWorkspaceButton(wxCommandEvent& event);

//------------------------------------------------------------------------------
// Public methods
//------------------------------------------------------------------------------
	void SetWorkspace(FxWorkspace* workspace);
	void SetAnimController(FxAnimController* animController);

//------------------------------------------------------------------------------
// Workspace container messages.
//------------------------------------------------------------------------------
	virtual void OnWorkspaceAdded(const FxName& newWorkspaceName);
	virtual void OnWorkspacePreRemove(const FxName& workspaceToRemove);
	virtual void OnWorkspacePostRemove(const FxName& removedWorkspace);
	virtual void OnWorkspaceRenamed(const FxName& oldName, const FxName newName);
	virtual void OnWorkspaceModified(const FxName& modifiedWorkspace);
	virtual void OnWorkspaceNeedsUpdate();
	virtual void OnWorkspaceModeSwitch(FxWorkspaceMode newMode);
	virtual void OnWorkspaceOptionChanged(FxBool autoKey, FxBool autoSelectCurve);
	virtual void OnWorkspaceClose();

protected:
//------------------------------------------------------------------------------
// Member functions.
//------------------------------------------------------------------------------
	// Message helper functions.
	void DoResizeRects();
	void DoRefreshSystemColours();
	void DoFillWorkspaceChoice();
	void DoLoadSelectedWorkspace();
	void DoCreateModeControls(FxWorkspaceMode mode);
	void DoFillPhonemeChoice();
	void DoLoadCurrentPhonemeMappingToSliders();

	// Painting.
	void DrawToolbar(wxDC* pDC);

	// Control management.
	void CreateToolbarControls();
	void SetControlStates();
	void MoveControls();

	// Workspace initialization.
	void InitWorkspace();

	// Dispatch a refresh message.
	void DispatchRefreshMessage();

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
	FxPhonemeMap* _phonemeMap;
	FxBool	 _valid;

	// Current workspace.
	FxWorkspace _currentWorkspace;
	FxBool		_autoKey;
	FxBool		_autoSelectCurve;
	FxWorkspaceMode _mode;

	// Sub-window layout.
	wxRect	 _toolbarRect;
	wxRect	 _workspaceRect;

	// Relevant colours.
	wxColour _colourFace;
	wxColour _colourShadow;
	wxColour _colourEngrave;

	// Toolbar controls.
	wxChoice* _toolbarWorkspaceChoice;
	FxButton* _toolbarEditWorkspaceButton;
	FxButton* _toolbarInstanceWorkspaceButton;
	wxChoice* _toolbarModeChoice;
	FxButton* _toolbarAutoselectCurvesToggle;
	FxButton* _toolbarDisplayNamesToggle;

	// Animation mode controls.
	FxButton* _toolbarAnimModeAutokeyToggle;
	FxButton* _toolbarAnimModeKeyAllButton;
	FxButton* _toolbarAnimModeNextKeyButton;
	FxButton* _toolbarAnimModePrevKeyButton;
	// Shared Combiner/Mapping controls.
	FxButton* _toolbarSharedModeClearButton;
	FxButton* _toolbarSharedModeLoadCurrentFrameButton;
	// Combiner mode controls.
	FxButton* _toolbarCombinerModeCreateButton;
	// Mapping mode controls.
	wxChoice* _toolbarMappingModePhonemeChoice;
	wxStaticText* _toolbarMappingModeSampleWord;
	FxButton* _toolbarMappingModeApplyButton;

	// Current mode-based controls.
	FxArray<wxWindow*> _modeControls;
	// Mapping-related data.
	FxPhoneticAlphabet _alphabet;
};

//------------------------------------------------------------------------------
// FxWorkspaceInstanceWidget
//
// Child torn-off workspace instances.
//------------------------------------------------------------------------------

class FxWorkspaceInstanceWidget : public wxWindow, public FxWidget, public FxWorkspaceContainer
{
	typedef wxWindow Super;
	WX_DECLARE_DYNAMIC_CLASS(FxWorkspaceInstanceWidget)
	DECLARE_EVENT_TABLE()

public:
	FxWorkspaceInstanceWidget(wxWindow* parent = NULL, FxWidgetMediator* mediator = NULL);
	~FxWorkspaceInstanceWidget();

	//------------------------------------------------------------------------------
	// Widget messages.
	//------------------------------------------------------------------------------
	virtual void OnActorChanged(FxWidget* sender, FxActor* actor);
	virtual void OnActorInternalDataChanged(FxWidget* sender, FxActorDataChangedFlag flags);
	virtual void OnAnimChanged(FxWidget* sender, const FxName& animGroupName, FxAnim* anim);
	virtual void OnTimeChanged(FxWidget* sender, FxReal newTime);
	virtual void OnRefresh(FxWidget* sender);

	virtual void OnAppStartup(FxWidget* sender);
	virtual void OnAppShutdown(FxWidget* sender);

	//------------------------------------------------------------------------------
	// Event handlers.
	//------------------------------------------------------------------------------
	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnMouse(wxMouseEvent& event);
	void OnKeyDown(wxKeyEvent& event);

	void OnHelp(wxHelpEvent& event);

	void OnUndo(wxCommandEvent& event);
	void OnRedo(wxCommandEvent& event);

	void OnPrevFrame(wxCommandEvent& event);
	void OnNextFrame(wxCommandEvent& event);

	//------------------------------------------------------------------------------
	// Public methods
	//------------------------------------------------------------------------------
	void SetWorkspace(FxWorkspace* workspace);
	void SetAnimController(FxAnimController* animController);

	//------------------------------------------------------------------------------
	// Workspace container messages.
	//------------------------------------------------------------------------------
	virtual void OnWorkspaceAdded(const FxName& newWorkspaceName);
	virtual void OnWorkspacePreRemove(const FxName& workspaceToRemove);
	virtual void OnWorkspacePostRemove(const FxName& removedWorkspace);
	virtual void OnWorkspaceRenamed(const FxName& oldName, const FxName newName);
	virtual void OnWorkspaceModified(const FxName& modifiedWorkspace);
	virtual void OnWorkspaceNeedsUpdate();
	virtual void OnWorkspaceModeSwitch(FxWorkspaceMode newMode);
	virtual void OnWorkspaceOptionChanged(FxBool autoKey, FxBool autoSelectCurve);
	virtual void OnWorkspaceClose();

protected:
	// Workspace initialization.
	void InitWorkspace();
	// Dispatch a refresh message.
	void DispatchRefreshMessage();

	//------------------------------------------------------------------------------
	// Member variables.
	//------------------------------------------------------------------------------
	// Actor data.
	FxActor* _actor;
	FxAnim*  _anim;
	FxReal	 _currentTime;
	FxName   _selFaceGraphNodeGroup;
	FxAnimController* _animController;
	FxBool	 _valid;

	// Current workspace.
	FxWorkspace _currentWorkspace;
	FxBool		_autoKey;
	FxBool		_autoSelectCurve;
	FxWorkspaceMode _mode;
};

} // namespace Face

} // namespace OC3Ent

#endif
