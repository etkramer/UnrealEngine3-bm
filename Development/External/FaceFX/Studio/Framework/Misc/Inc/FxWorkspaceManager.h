//------------------------------------------------------------------------------
// A manager for the various workspaces in the system.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxWorkspaceManager_H__
#define FxWorkspaceManager_H__

// Includes
#include "FxList.h"
#include "FxWorkspaceElements.h"
#include "FxNamedObject.h"
#include "FxActor.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	// wx-specific includes
#endif

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// FxWorkspace
//
// A workspace definition.
//------------------------------------------------------------------------------
class FxWorkspace : public FxNamedObject
{
	// Declare the class.
	FX_DECLARE_CLASS(FxWorkspace, FxNamedObject);

public:
//------------------------------------------------------------------------------
// Construction / Destruction
//------------------------------------------------------------------------------
	FxWorkspace();
	FxWorkspace(const FxName& workspaceName);
	FxWorkspace(const FxWorkspace& other);
	FxWorkspace& operator=(const FxWorkspace& other);
	~FxWorkspace();

//------------------------------------------------------------------------------
// Accessors.
//------------------------------------------------------------------------------
	// Set the rect to use as the bounds of the workspace.
	void SetWorkspaceRect(const wxRect& workspaceRect);
	// Returns the rect used as the bounds of the workspace.
	const wxRect& GetWorkspaceRect() const;
	// Sets the actor to use.
	void SetActor(FxActor* pActor);
	// Sets the animation to use.
	void SetAnim(FxAnim* pAnim);
	// Sets the anim controller.
	void SetAnimController(FxAnimController* pAnimController);
	// Set the current time.
	void SetCurrentTime(FxReal currentTime);

	// Returns the number of sliders.
	FxSize GetNumSliders();
	// Returns the sliders at index.
	FxWorkspaceElementSlider* GetSlider(FxSize index);
	// Returns the number of viewports.
	FxSize GetNumViewports();
	// Returns the viewport at inde.
	FxWorkspaceElementViewport* GetViewport(FxSize index);

	// Returns the currently selected element.
	FxWorkspaceElement* GetSelectedElement();
	// Returns the currently selected slider, 
	// or NULL if a slider isn't currently selected.
	FxWorkspaceElementSlider* GetSelectedSlider();

	// Returns the workspace mode.
	FxWorkspaceMode GetMode();
	// Sets the workspace mode.
	void SetMode(FxWorkspaceMode mode);
	// Sets the workspace options.
	void SetOption(FxBool autoKey, FxBool autoSelectCurve);
	// Returns the auto-key option.
	FxBool GetAutoKey() { return _autoKey; }
	// Returns the auto-select curve option.
	FxBool GetAutoSelectCurve() { return _autoSelectCurve; }

	// Clears the dirty flag.
	void ClearDirty() { _isDirty = FxFalse; }
	// Gets the dirty flag.
	FxBool IsDirty() { return _isDirty; }

	// Gets the text colour.
	wxColour GetTextColour();
	// Sets the text colour.
	void SetTextColour(const wxColour& colour);

	// Begins set tab order mode.
	void BeginSetTabOrder();
	// Returns true if the workspace is setting its tab order.
	FxBool IsSettingTabOrder();
	// Returns true if the tab order list is full.
	FxBool IsTabOrderComplete();
	// Ends set tab order mode.
	void EndSetTabOrder();

	// Goes to the next selected slider based on tab order.
	void GoToNextSlider();
	// Goes to the previous selected slider based on tab order.
	void GoToPrevSlider();

//------------------------------------------------------------------------------
// Element management.
//------------------------------------------------------------------------------
	// Adds a slider to the workspace.
	void AddSlider(const FxWorkspaceElementSlider& slider);
	// Adds a background to the workspace.
	void AddBackground(const FxWorkspaceElementBackground& background);
	// Adds a viewport to a workspace.
	void AddViewport(const FxWorkspaceElementViewport& viewport);

	// Removes an element.
	FxBool RemoveElement(FxWorkspaceElement* pElem);

//------------------------------------------------------------------------------
// User interaction.
//------------------------------------------------------------------------------
	// Draw the workspace, using the workspace rect as the bounds.
	void DrawOn(wxDC* pDC, FxBool editMode = FxFalse, FxBool drawGrid = FxFalse);

	// Process a mouse event.
	void ProcessMouseEvent(wxMouseEvent& event, FxInt32& needsRefresh, FxBool editMode = FxFalse);
	// Process a key press.
	FxBool ProcessKeyDown(wxKeyEvent& event, FxInt32& needsRefresh, FxBool editMode = FxFalse);

	// Updates all the sliders to their correct state.
	void UpdateAllSliders();

//------------------------------------------------------------------------------
// Serialization
//------------------------------------------------------------------------------
	// Serialize the workspace to an archive.
	virtual void Serialize( FxArchive& arc );

private:
//------------------------------------------------------------------------------
// Member functions
//------------------------------------------------------------------------------
	void SelectCursor(FxWorkspaceElement::HitResult res, FxReal rot, const FxVec2& scale);

//------------------------------------------------------------------------------
// Member variables
//------------------------------------------------------------------------------
	// The list of bitmapped backgrounds in the workspace.
	FxList<FxWorkspaceElementBackground> _backgrounds;
	// The list of sliders in the workspace.
	FxList<FxWorkspaceElementSlider> _sliders;
	// The list of viewports in the workspace.
	FxList<FxWorkspaceElementViewport> _viewports;

	wxRect				_workspaceRect;	// The bounding rect for the workspace.
	FxActor*			_actor;			// The actor to work on.
	FxAnim*				_anim;			// The anim to work on.
	FxAnimController*	_animController;// The anim controller.
	FxReal				_currentTime;	// The current time.

	// The last element to pass the hit test.
	FxWorkspaceElement* _focusedElement;
	// The element that has the capture.
	FxWorkspaceElement* _captureElement;
	// The selected element.
	FxWorkspaceElement* _selectedElement;

	// The focused slider.
	FxWorkspaceElementSlider* _focusedSlider;

	// The workspace's mode.
	FxWorkspaceMode		_mode;
	// Some workspace options.
	FxBool				_autoKey;
	FxBool				_autoSelectCurve;

	// The workspace dirty flag. Only important in edit mode.
	FxBool				_isDirty;

	// The text colour in the workspace.
	wxColour			_textColour;

	// The setting tab order flag. Only important in edit mode.
	FxBool				_isSettingTabOrder;
	// The tab order of the sliders. Only important while setting tab order.
	FxList<FxWorkspaceElementSlider*> _sliderTabOrder;

};

//------------------------------------------------------------------------------
// FxWorkspaceContainer
//
// Any class containing a workspace needs to derive from this class to receive
// important notifications about the status of the contained workspace.
//------------------------------------------------------------------------------
class FxWorkspaceContainer
{
public:
	// A workspace was added to the system.
	virtual void OnWorkspaceAdded(const FxName& newWorkspaceName) = 0;
	// A workspace is about to be removed from the system.
	virtual void OnWorkspacePreRemove(const FxName& workspaceToRemove) = 0;
	// A workspace has just been removed from the system.
	virtual void OnWorkspacePostRemove(const FxName& removedWorkspace) = 0;
	// A workspace has been renamed.
	virtual void OnWorkspaceRenamed(const FxName& oldName, const FxName newName) = 0;
	// A workspace's contents have been modified.
	virtual void OnWorkspaceModified(const FxName& modifiedWorkspace) = 0;
	// A workspace should be updated.
	virtual void OnWorkspaceNeedsUpdate() = 0;
	// A workspace's mode should be changed.
	virtual void OnWorkspaceModeSwitch(FxWorkspaceMode newMode) = 0;
	// A workspace's option has changed.
	virtual void OnWorkspaceOptionChanged(FxBool autoKey, FxBool autoSelectCurve) = 0;
	// A workspace container should be closed.
	virtual void OnWorkspaceClose() = 0;
};

//------------------------------------------------------------------------------
// FxWorkspaceManager
//
// The workspace manager.
//------------------------------------------------------------------------------
class FxWorkspaceManager
{
public:
	static void Startup();
	static FxWorkspaceManager* Instance();
	static void Shutdown();

	// Clear the workspaces in the manager.
	void Clear();
	// Add a workspace to the manager.
	void Add(const FxWorkspace& workspace);
	// Removes a workspace by name.
	// Returns true if we removed the last workspace and added the default.
	FxBool Remove(const FxName& workspaceName);
	// Renames a workspace.
	void Rename(const FxName& originalName, const FxName& newName);
	// Modifies a workspace.
	void Modify(const FxName& workspaceName, const FxWorkspace& newValue);

	// Flag that workspaces need to be updated.
	void FlagWorkspacesNeedUpdate();
	// Tell the workspaces to change their modes.
	void ChangeMode(FxWorkspaceMode newMode);
	// Tell the workspaces to change an option.
	void ChangeOption(FxBool autoKey, FxBool autoSelectCurve);

	// Returns whether workspaces should always display names.
	FxBool GetDisplayNames();
	// Sets whether workspaces should always display names.
	void SetDisplayNames(FxBool displayNames);

	// Find a workspace.
	const FxWorkspace& Find(const FxName& workspaceName);

	// Gets the number of workspaces.
	FxSize GetNumWorkspaces() const;
	// Gets the workspace at index.
	const FxWorkspace& GetWorkspace(FxSize index);

	// Adds a workspace container.
	void AddWorkspaceContainer(FxWorkspaceContainer* pContainer);
	// Removes a workspace container.
	void RemoveWorkspaceContainer(FxWorkspaceContainer* pContainer);

	// Serialize the workspaces to/from an archive.
	void Serialize(FxArchive& arc);

private:
	// Hide constructor/destructor.
	FxWorkspaceManager();
	~FxWorkspaceManager();

	// Dispatch a workspace added message.
	void DispatchWorkspaceAdded(const FxName& newWorkspaceName);
	// Dispatch a pre-remove workspace message.
	void DispatchWorkspacePreRemove(const FxName& workspaceToRemove);
	// Dispatch a post-remove workspace message.
	void DispatchWorkspacePostRemove(const FxName& removedWorkspace);
	// Dispatch a workspace renamed message.
	void DispatchWorkspaceRenamed(const FxName& oldName, const FxName& newName);
	// Dispatch a workspace modified message.
	void DispatchWorkspaceModified(const FxName& modifiedWorkspace);
	// Dispatch a workspace close message.
	void DispatchWorkspaceClose();

	static FxWorkspaceManager* _instance;
	FxList<FxWorkspace> _workspaces;
	FxList<FxWorkspaceContainer*> _workspaceContainers;
	FxBool _shouldDisplayNames;
};

} // namespace Face

} // namespace OC3Ent

#endif
