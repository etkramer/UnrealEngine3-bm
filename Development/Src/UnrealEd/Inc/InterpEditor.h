/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __INTERPEDITOR_H__
#define __INTERPEDITOR_H__

#include "UnrealEd.h"
#include "CurveEd.h"
#include "TrackableWindow.h"
#include "UnEdTran.h"

/*-----------------------------------------------------------------------------
	Editor-specific hit proxies.
-----------------------------------------------------------------------------*/

struct HInterpEdTrackBkg : public HHitProxy
{
	DECLARE_HIT_PROXY(HInterpEdTrackBkg,HHitProxy);
	HInterpEdTrackBkg(): HHitProxy(HPP_UI) {}
};

struct HInterpEdGroupTitle : public HHitProxy
{
	DECLARE_HIT_PROXY(HInterpEdGroupTitle,HHitProxy);

	class UInterpGroup* Group;

	HInterpEdGroupTitle(class UInterpGroup* InGroup) :
		HHitProxy(HPP_UI),
		Group(InGroup)
	{}
};

struct HInterpEdGroupCollapseBtn : public HHitProxy
{
	DECLARE_HIT_PROXY(HInterpEdGroupCollapseBtn,HHitProxy);

	class UInterpGroup* Group;

	HInterpEdGroupCollapseBtn(class UInterpGroup* InGroup) :
		HHitProxy(HPP_UI),
		Group(InGroup)
	{}
};

struct HInterpEdGroupLockCamBtn : public HHitProxy
{
	DECLARE_HIT_PROXY(HInterpEdGroupLockCamBtn,HHitProxy);

	class UInterpGroup* Group;

	HInterpEdGroupLockCamBtn(class UInterpGroup* InGroup) :
		HHitProxy(HPP_UI),
		Group(InGroup)
	{}
};

struct HInterpEdTrackTitle : public HHitProxy
{
	DECLARE_HIT_PROXY(HInterpEdTrackTitle,HHitProxy);

	class UInterpGroup* Group;
	INT TrackIndex;

	HInterpEdTrackTitle(class UInterpGroup* InGroup, INT InTrackIndex) :
		HHitProxy(HPP_UI),
		Group(InGroup),
		TrackIndex(InTrackIndex)
	{}
};

struct HInterpEdTrackTrajectoryButton : public HHitProxy
{
	DECLARE_HIT_PROXY(HInterpEdTrackTrajectoryButton,HHitProxy);

	class UInterpGroup* Group;
	INT TrackIndex;

	HInterpEdTrackTrajectoryButton(class UInterpGroup* InGroup, INT InTrackIndex) :
		HHitProxy(HPP_UI),
		Group(InGroup),
		TrackIndex(InTrackIndex)
	{}
};

struct HInterpEdTrackGraphPropBtn : public HHitProxy
{
	DECLARE_HIT_PROXY(HInterpEdTrackGraphPropBtn,HHitProxy);

	class UInterpGroup* Group;
	INT TrackIndex;

	HInterpEdTrackGraphPropBtn(class UInterpGroup* InGroup, INT InTrackIndex) :
		HHitProxy(HPP_UI),
		Group(InGroup),
		TrackIndex(InTrackIndex)
	{}
};

struct HInterpEdTrackDisableTrackBtn : public HHitProxy
{
	DECLARE_HIT_PROXY(HInterpEdTrackDisableTrackBtn,HHitProxy);

	class UInterpGroup* Group;
	INT TrackIndex;

	HInterpEdTrackDisableTrackBtn(class UInterpGroup* InGroup, INT InTrackIndex) :
		HHitProxy(HPP_UI),
		Group(InGroup),
		TrackIndex(InTrackIndex)
	{}
};

enum EInterpEdEventDirection
{
	IED_Forward,
	IED_Backward
};

struct HInterpEdEventDirBtn : public HHitProxy
{
	DECLARE_HIT_PROXY(HInterpEdEventDirBtn,HHitProxy);

	class UInterpGroup* Group;
	INT TrackIndex;
	EInterpEdEventDirection Dir;

	HInterpEdEventDirBtn(class UInterpGroup* InGroup, INT InTrackIndex, EInterpEdEventDirection InDir) :
		HHitProxy(HPP_UI),
		Group(InGroup),
		TrackIndex(InTrackIndex),
		Dir(InDir)
	{}
};

struct HInterpEdTimelineBkg : public HHitProxy
{
	DECLARE_HIT_PROXY(HInterpEdTimelineBkg,HHitProxy);
	HInterpEdTimelineBkg(): HHitProxy(HPP_UI) {}
};

struct HInterpEdNavigatorBackground : public HHitProxy
{
	DECLARE_HIT_PROXY(HInterpEdNavigatorBackground,HHitProxy);
	HInterpEdNavigatorBackground(): HHitProxy(HPP_UI) {}
};

struct HInterpEdNavigator : public HHitProxy
{
	DECLARE_HIT_PROXY(HInterpEdNavigator,HHitProxy);
	HInterpEdNavigator(): HHitProxy(HPP_UI) {}
};

enum EInterpEdMarkerType
{
	ISM_SeqStart,
	ISM_SeqEnd,
	ISM_LoopStart,
	ISM_LoopEnd
};

struct HInterpEdMarker : public HHitProxy
{
	DECLARE_HIT_PROXY(HInterpEdMarker,HHitProxy);

	EInterpEdMarkerType Type;

	HInterpEdMarker(EInterpEdMarkerType InType) :
		HHitProxy(HPP_UI),
		Type(InType)
	{}
};

/** Hitproxy for when the user clicks on a filter tab in matinee. */
struct HInterpEdTab : public HHitProxy
{
	/** Pointer to the interp filter for this hit proxy. */
	class UInterpFilter* Filter;

	DECLARE_HIT_PROXY(HInterpEdTab,HHitProxy);
	HInterpEdTab(UInterpFilter* InFilter): HHitProxy(HPP_UI), Filter(InFilter) {}
};

/*-----------------------------------------------------------------------------
	UInterpEdTransBuffer / FInterpEdTransaction
-----------------------------------------------------------------------------*/

class UInterpEdTransBuffer : public UTransBuffer
{
	DECLARE_CLASS(UInterpEdTransBuffer,UTransBuffer,CLASS_Transient,UnrealEd)
	NO_DEFAULT_CONSTRUCTOR(UInterpEdTransBuffer)
public:

	UInterpEdTransBuffer(SIZE_T InMaxMemory)
		:	UTransBuffer( InMaxMemory )
	{}

	/**
	 * Begins a new undo transaction.  An undo transaction is defined as all actions
	 * which take place when the user selects "undo" a single time.
	 * If there is already an active transaction in progress, increments that transaction's
	 * action counter instead of beginning a new transaction.
	 * 
	 * @param	SessionName		the name for the undo session;  this is the text that 
	 *							will appear in the "Edit" menu next to the Undo item
	 *
	 * @return	Number of active actions when Begin() was called;  values greater than
	 *			0 indicate that there was already an existing undo transaction in progress.
	 */
	virtual INT Begin(const TCHAR* SessionName)
	{
		return 0;
	}

	/**
	 * Attempts to close an undo transaction.  Only successful if the transaction's action
	 * counter is 1.
	 * 
	 * @return	Number of active actions when End() was called; a value of 1 indicates that the
	 *			transaction was successfully closed
	 */
	virtual INT End()
	{
		return 1;
	}

	/**
	 * Cancels the current transaction, no longer capture actions to be placed in the undo buffer.
	 *
	 * @param	StartIndex	the value of ActiveIndex when the transaction to be cancelled was began. 
	 */
	virtual void Cancel(INT StartIndex = 0)
	{}

	virtual void BeginSpecial(const TCHAR* SessionName);
	virtual void EndSpecial();
};

class FInterpEdTransaction : public FTransaction
{
public:
	FInterpEdTransaction( const TCHAR* InTitle=NULL, UBOOL InFlip=0 )
	:	FTransaction(InTitle, InFlip)
	{}

	virtual void SaveObject( UObject* Object );
	virtual void SaveArray( UObject* Object, FScriptArray* Array, INT Index, INT Count, INT Oper, INT ElementSize, STRUCT_AR Serializer, STRUCT_DTOR Destructor );
};

/*-----------------------------------------------------------------------------
	FInterpEdViewportClient
-----------------------------------------------------------------------------*/

class FInterpEdViewportClient : public FEditorLevelViewportClient
{
public:
	FInterpEdViewportClient( class WxInterpEd* InInterpEd );
	~FInterpEdViewportClient();

	void DrawTimeline(FViewport* Viewport,FCanvas* Canvas);
	void DrawMarkers(FViewport* Viewport,FCanvas* Canvas);
	void DrawGrid(FViewport* Viewport,FCanvas* Canvas, UBOOL bDrawTimeline);
	void DrawTabs(FViewport* Viewport,FCanvas* Canvas);
	FVector2D DrawTab(FViewport* Viewport, FCanvas* Canvas, INT &TabOffset, UInterpFilter* Filter);
	
	virtual void Draw(FViewport* Viewport,FCanvas* Canvas);

	virtual UBOOL InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed = 1.f,UBOOL bGamepad=FALSE);
	virtual void MouseMove(FViewport* Viewport, INT X, INT Y);
	virtual UBOOL InputAxis(FViewport* Viewport, INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime, UBOOL bGamepad=FALSE);
	virtual EMouseCursor GetCursor(FViewport* Viewport,INT X,INT Y);

	virtual void Tick(FLOAT DeltaSeconds);

	virtual void Serialize(FArchive& Ar);

	/** Exec handler */
	virtual void Exec(const TCHAR* Cmd);

	/** 
	 * Returns the vertical size of the entire group list for this viewport, in pixels
	 */
	INT ComputeGroupListContentHeight() const;

	/** 
	 * Returns the height of the viewable group list content box in pixels
	 *
	 *  @param ViewportHeight The size of the viewport in pixels
	 *
	 *  @return The height of the viewable content box (may be zero!)
	 */
	INT ComputeGroupListBoxHeight( const INT ViewportHeight ) const;


	/**
	 * Selects a color for the specified group (bound to the given group actor)
	 *
	 * @param Group The group to select a label color for
	 * @param GroupActorOrNull The actor currently bound to the specified, or NULL if none is bounds
	 *
	 * @return The color to use to draw the group label
	 */
	FColor ChooseLabelColorForGroupActor( UInterpGroup* Group, AActor* GroupActorOrNull ) const;


public:

	/** True if this window is the 'director tracks' window and should only draw director track groups */
	UBOOL bIsDirectorTrackWindow;

	/** True if we want filter tabs to be rendered and interactive for this window */
	UBOOL bWantFilterTabs;

	/** True if we want the animation timeline bar to be rendered and interactive for this window */
	UBOOL bWantTimeline;

	/** Scroll bar thumb position (actually, this is the negated thumb position.) */
	INT	ThumbPos_Vert;

	INT OldMouseX, OldMouseY;
	INT BoxStartX, BoxStartY;
	INT BoxEndX, BoxEndY;
	INT DistanceDragged;

	/** Used to accumulate velocity for autoscrolling when the user is dragging items near the edge of the viewport. */
	FVector2D ScrollAccum;

	class WxInterpEd* InterpEd;

	/** The object and data we are currently dragging, if NULL we are not dragging any objects. */
	FInterpEdInputData DragData;
	FInterpEdInputInterface* DragObject;

	UBOOL	bPanning;
	UBOOL   bMouseDown;
	UBOOL	bGrabbingHandle;
	UBOOL	bNavigating;
	UBOOL	bBoxSelecting;
	UBOOL	bTransactionBegun;
	UBOOL	bGrabbingMarker;
};

/*-----------------------------------------------------------------------------
	WxInterpEdVCHolder
-----------------------------------------------------------------------------*/

class WxInterpEdVCHolder : public wxWindow
{
public:
	WxInterpEdVCHolder( wxWindow* InParent, wxWindowID InID, class WxInterpEd* InInterpEd );
	virtual ~WxInterpEdVCHolder();

	/**
	 * Destroys the viewport held by this viewport holder, disassociating it from the engine, etc.  Rentrant.
	 */
	void DestroyViewport();

	/** Scroll bar */
	wxScrollBar* ScrollBar_Vert;

	/**
	 * Updates the scroll bar for the current state of the window's size and content layout.  This should be called
	 *  when either the window size changes or the vertical size of the content contained in the window changes.
	 */
	void AdjustScrollBar();

	/**
	 * Updates layout of the track editor window's content layout.  This is usually called in response to a window size change
	 */
	void UpdateWindowLayout();

	void OnSize( wxSizeEvent& In );

	FInterpEdViewportClient* InterpEdVC;

	DECLARE_EVENT_TABLE()
};


/*-----------------------------------------------------------------------------
	WxInterpEdToolBar
-----------------------------------------------------------------------------*/

class WxInterpEdToolBar : public wxToolBar
{
public:
	WxMaskedBitmap AddB, PlayReverseB, PlayB, LoopSectionB, StopB, UndoB, RedoB, CurveEdB, SnapB, FitSequenceB, FitToSelectedB, FitLoopB, FitLoopSequenceB;
	WxMaskedBitmap Speed1B, Speed10B, Speed25B, Speed50B, Speed100B, SnapTimeToFramesB, FixedTimeStepPlaybackB, GorePreviewB;
	wxComboBox* SnapCombo;

	/** Combo box that allows the user to select the initial curve interpolation mode (EInterpCurveMode) for newly
	  * created key frames. */
	wxComboBox* InitialInterpModeComboBox;

	WxInterpEdToolBar( wxWindow* InParent, wxWindowID InID );
	~WxInterpEdToolBar();
};

/*-----------------------------------------------------------------------------
	WxInterpEdMenuBar
-----------------------------------------------------------------------------*/

class WxInterpEdMenuBar : public wxMenuBar
{
public:
	wxMenu	*FileMenu, *EditMenu, *ViewMenu;

	WxInterpEdMenuBar(WxInterpEd* InEditor);
	~WxInterpEdMenuBar();
};

/*-----------------------------------------------------------------------------
	WxInterpEd
-----------------------------------------------------------------------------*/

static const FLOAT InterpEdSnapSizes[5] = { 0.01f, 0.05f, 0.1f, 0.5f, 1.0f };

static const FLOAT InterpEdFPSSnapSizes[9] =
{
	1.0f / 15.0f,
	1.0f / 24.0f,
	1.0f / 25.0f,
	1.0f / ( 30.0f / 1.001f ),	// 1.0f / 29.97...
	1.0f / 30.0f,
	1.0f / 50.0f,
	1.0f / ( 60.0f / 1.001f ),	// 1.0f / 59.94...
	1.0f / 60.0f,
	1.0f / 120.0f,
};

static const TCHAR* InterpEdFPSSnapSizeLocNames[9] =
{
	TEXT( "InterpEd_FrameRate_15_fps" ),
	TEXT( "InterpEd_FrameRate_24_fps" ),
	TEXT( "InterpEd_FrameRate_25_fps" ),
	TEXT( "InterpEd_FrameRate_29_97_fps" ),
	TEXT( "InterpEd_FrameRate_30_fps" ),
	TEXT( "InterpEd_FrameRate_50_fps" ),
	TEXT( "InterpEd_FrameRate_59_94_fps" ),
	TEXT( "InterpEd_FrameRate_60_fps" ),
	TEXT( "InterpEd_FrameRate_120_fps" )
};

class WxInterpEd : public WxTrackableFrame, public FNotifyHook, public FSerializableObject, public FCurveEdNotifyInterface, public FDockingParent
{
public:
	WxInterpEd( wxWindow* InParent, wxWindowID InID, class USeqAct_Interp* InInterp  );
	virtual	~WxInterpEd();

	/**
	 * This function is called when the window has been selected from within the ctrl + tab dialog.
	 */
	virtual void OnSelected();

	void OnSize( wxSizeEvent& In );
	virtual void OnClose( wxCloseEvent& In );

	// FNotify interface

	void NotifyDestroy( void* Src );
	void NotifyPreChange( void* Src, UProperty* PropertyAboutToChange );
	void NotifyPostChange( void* Src, UProperty* PropertyThatChanged );
	void NotifyExec( void* Src, const TCHAR* Cmd );

	// FCurveEdNotifyInterface
	virtual void PreEditCurve(TArray<UObject*> CurvesAboutToChange);
	virtual void PostEditCurve();
	virtual void MovedKey();
	virtual void DesireUndo();
	virtual void DesireRedo();

	/**
	 * FCurveEdNotifyInterface: Called by the Curve Editor when a Curve Label is clicked on
	 *
	 * @param	CurveObject	The curve object whose label was clicked on
	 */
	void OnCurveLabelClicked( UObject* CurveObject );

	// FSerializableObject
	void Serialize(FArchive& Ar);

	/** 
	 * Starts playing the current sequence. 
	 * @param bPlayLoop		Whether or not we should play the looping section.
	 * @param bPlayForward	TRUE if we should play forwards, or FALSE for reverse
	 */
	void StartPlaying( UBOOL bPlayLoop, UBOOL bPlayForward );

	/** Stops playing the current sequence. */
	void StopPlaying();

	// Menu handlers
	void OnScroll(wxScrollEvent& In);
	void OnMenuAddKey( wxCommandEvent& In );
	void OnMenuPlay( wxCommandEvent& In );
	void OnMenuStop( wxCommandEvent& In );
	void OnChangePlaySpeed( wxCommandEvent& In );
	void OnMenuInsertSpace( wxCommandEvent& In );
	void OnMenuStretchSection( wxCommandEvent& In );
	void OnMenuDeleteSection( wxCommandEvent& In );
	void OnMenuSelectInSection( wxCommandEvent& In );
	void OnMenuDuplicateSelectedKeys( wxCommandEvent& In );
	void OnSavePathTime( wxCommandEvent& In );
	void OnJumpToPathTime( wxCommandEvent& In );
	void OnViewHide3DTracks( wxCommandEvent& In );
	void OnViewZoomToScrubPos( wxCommandEvent& In );

	/**
	 * Shows or hides all movement track trajectories in the Matinee sequence
	 */
	void OnViewShowOrHideAll3DTrajectories( wxCommandEvent& In );

	/** Toggles 'capture mode' for particle replay tracks */
	void OnParticleReplayTrackContext_ToggleCapture( wxCommandEvent& In );

	/** Called when the "Toggle Gore Preview" button is pressed */
	void OnToggleGorePreview( wxCommandEvent& In );

	/** Called when the "Toggle Gore Preview" UI should be updated */
	void OnToggleGorePreview_UpdateUI( wxUpdateUIEvent& In );

	void OnToggleViewportFrameStats( wxCommandEvent& In );
	void OnToggleCurveEd( wxCommandEvent& In );
	void OnGraphSplitChangePos( wxSplitterEvent& In );

	void OnToggleSnap( wxCommandEvent& In );
	void OnToggleSnap_UpdateUI( wxUpdateUIEvent& In );

	/** Called when the 'snap time to frames' command is triggered from the GUI */
	void OnToggleSnapTimeToFrames( wxCommandEvent& In );
	void OnToggleSnapTimeToFrames_UpdateUI( wxUpdateUIEvent& In );


	/** Called when the 'fixed time step playback' command is triggered from the GUI */
	void OnFixedTimeStepPlaybackCommand( wxCommandEvent& In );

	/** Updates UI state for 'fixed time step playback' option */
	void OnFixedTimeStepPlaybackCommand_UpdateUI( wxUpdateUIEvent& In );


	/** Called when the 'prefer frame numbers' command is triggered from the GUI */
	void OnPreferFrameNumbersCommand( wxCommandEvent& In );

	/** Updates UI state for 'prefer frame numbers' option */
	void OnPreferFrameNumbersCommand_UpdateUI( wxUpdateUIEvent& In );


	/** Updates UI state for 'show time cursor pos for all keys' option */
	void OnShowTimeCursorPosForAllKeysCommand( wxCommandEvent& In );

	/** Called when the 'show time cursor pos for all keys' command is triggered from the GUI */
	void OnShowTimeCursorPosForAllKeysCommand_UpdateUI( wxUpdateUIEvent& In );


	void OnChangeSnapSize( wxCommandEvent& In );

	/**
	 * Called when the initial curve interpolation mode for newly created keys is changed
	 */
	void OnChangeInitialInterpMode( wxCommandEvent& In );

	void OnViewFitSequence( wxCommandEvent& In );
	void OnViewFitToSelected( wxCommandEvent& In );
	void OnViewFitLoop( wxCommandEvent& In );
	void OnViewFitLoopSequence( wxCommandEvent& In );

	void OnOpenBindKeysDialog( wxCommandEvent &In );

	/**
	 * Called when a docking window state has changed
	 */
	virtual void OnWindowDockingLayoutChanged();

	/**
	 * Called when the user selects the 'Expand All Groups' option from a menu.  Expands every group such that the
	 * entire hierarchy of groups and tracks are displayed.
	 */
	void OnExpandAllGroups( wxCommandEvent& In );

	/**
	 * Called when the user selects the 'Collapse All Groups' option from a menu.  Collapses every group in the group
	 * list such that no tracks are displayed.
	 */
	void OnCollapseAllGroups( wxCommandEvent& In );

	void OnContextNewTrack( wxCommandEvent& In );
	void OnContextNewGroup( wxCommandEvent& In );
	void OnContextTrackRename( wxCommandEvent& In );
	void OnContextTrackDelete( wxCommandEvent& In );
	void OnContextTrackChangeFrame( wxCommandEvent& In );
	
	/**
	 * Toggles visibility of the trajectory for the selected movement track
	 */
	void OnContextTrackShow3DTrajectory( wxCommandEvent& In );
	
	void OnContextGroupRename( wxCommandEvent& In );
	void OnContextGroupDelete( wxCommandEvent& In );
	void OnContextGroupCreateTab( wxCommandEvent& In );
	void OnContextGroupSendToTab( wxCommandEvent& In );
	void OnContextGroupRemoveFromTab( wxCommandEvent& In );
	void OnContextDeleteGroupTab( wxCommandEvent& In );

	/** Called when the user selects to move a group to another group folder */
	void OnContextGroupChangeGroupFolder( wxCommandEvent& In );

	void OnContextKeyInterpMode( wxCommandEvent& In );
	void OnContextRenameEventKey( wxCommandEvent& In );
	void OnContextSetKeyTime( wxCommandEvent& In );
	void OnContextSetValue( wxCommandEvent& In );

	/** Pops up a menu and lets you set the color for the selected key. Not all track types are supported. */
	void OnContextSetColor( wxCommandEvent& In );

	/** Pops up menu and lets the user set a group to use to lookup transform info for a movement keyframe. */
	void OnSetMoveKeyLookupGroup( wxCommandEvent& In );

	/** Clears the lookup group for a currently selected movement key. */
	void OnClearMoveKeyLookupGroup( wxCommandEvent& In );

	void OnSetAnimKeyLooping( wxCommandEvent& In );
	void OnSetAnimOffset( wxCommandEvent& In );
	void OnSetAnimPlayRate( wxCommandEvent& In );

	/** Handler for the toggle animation reverse menu item. */
	void OnToggleReverseAnim( wxCommandEvent& In );

	/** Handler for UI update requests for the toggle anim reverse menu item. */
	void OnToggleReverseAnim_UpdateUI( wxUpdateUIEvent& In );

	/** Handler for the save as camera animation menu item. */
	void OnContextSaveAsCameraAnimation( wxCommandEvent& In );

	void OnSetSoundVolume(wxCommandEvent& In);
	void OnSetSoundPitch(wxCommandEvent& In);
	void OnContextDirKeyTransitionTime( wxCommandEvent& In );
	void OnFlipToggleKey(wxCommandEvent& In);

	/** Called when a new key condition is selected in a track keyframe context menu */
	void OnKeyContext_SetCondition( wxCommandEvent& In );

	/** Syncs the generic browser to the currently selected sound track key */
	void OnKeyContext_SyncGenericBrowserToSoundCue( wxCommandEvent& In );

	/** Called when the user wants to set the master volume on Audio Master track keys */
	void OnKeyContext_SetMasterVolume( wxCommandEvent& In );

	/** Called when the user wants to set the master pitch on Audio Master track keys */
	void OnKeyContext_SetMasterPitch( wxCommandEvent& In );

	/** Called when the user wants to set the clip ID number for Particle Replay track keys */
	void OnParticleReplayKeyContext_SetClipIDNumber( wxCommandEvent& In );

	/** Called when the user wants to set the duration of Particle Replay track keys */
	void OnParticleReplayKeyContext_SetDuration( wxCommandEvent& In );

	/** Called to delete the currently selected keys */
	void OnDeleteSelectedKeys( wxCommandEvent& In );

	void OnMenuUndo( wxCommandEvent& In );
	void OnMenuRedo( wxCommandEvent& In );
	void OnMenuCut( wxCommandEvent& In );
	void OnMenuCopy( wxCommandEvent& In );
	void OnMenuPaste( wxCommandEvent& In );

	void OnMenuEdit_UpdateUI( wxUpdateUIEvent& In );

	void OnMenuImport( wxCommandEvent& In );
	void OnMenuExport( wxCommandEvent& In );
	void OnMenuReduceKeys( wxCommandEvent& In );

	/** Called when the 'Export Sound Cue Info' command is issued */
	void OnExportSoundCueInfoCommand( wxCommandEvent& In );


	// Selection
	void SetSelectedFilter(class UInterpFilter* InFilter);
	void SetActiveTrack(class UInterpGroup* InGroup, INT InTrackIndex);

	/**
	 * Locates the director group in our list of groups (if there is one)
	 *
	 * @param OutDirGroupIndex	The index of the director group in the list (if it was found)
	 *
	 * @return Returns true if a director group was found
	 */
	UBOOL FindDirectorGroup( INT& OutDirGroupIndex );

	/**
	 * Remaps the specified group index such that the director's group appears as the first element
	 *
	 * @param DirGroupIndex	The index of the 'director group' in the group list
	 * @param ElementIndex	The original index into the group list
	 *
	 * @return Returns the reordered element index for the specified element index
	 */
	INT RemapGroupIndexForDirGroup( const INT DirGroupIndex, const INT ElementIndex );

	/**
	 * Scrolls the view to the specified group if it is visible, otherwise it scrolls to the top of the screen.
	 *
	 * @param InGroup	Group to scroll the view to.
	 */
	void ScrollToGroup(class UInterpGroup* InGroup);

	/**
	 * Expands or collapses all visible groups in the track editor
	 *
	 * @param bExpand TRUE to expand all groups, or FALSE to collapse them all
	 */
	void ExpandOrCollapseAllVisibleGroups( const UBOOL bExpand );

	/**
	 * Updates the track window list scroll bar's vertical range to match the height of the window's content
	 */
	void UpdateTrackWindowScrollBars();

	/**
	 * Dirty the contents of the track window viewports
	 */
	void InvalidateTrackWindowViewports();

	/**
	 * Either shows or hides the director track window by splitting/unsplitting the parent window
	 */
	void UpdateDirectorTrackWindowVisibility();

	/**
	 * Creates a string with timing/frame information for the specified time value in seconds
	 *
	 * @param InTime The time value to create a timecode for
	 * @param bIncludeMinutes TRUE if the returned string should includes minute information
	 *
	 * @return The timecode string
	 */
	FString MakeTimecodeString( FLOAT InTime, UBOOL bIncludeMinutes = TRUE ) const;

	/**
	 * Locates the specified group's parent group folder, if it has one
	 *
	 * @param ChildGroup The group who's parent we should search for
	 *
	 * @return Returns the parent group pointer or NULL if one wasn't found
	 */
	UInterpGroup* FindParentGroupFolder( UInterpGroup* ChildGroup );

	/**
	 * Counts the number of children that the specified group folder has
	 *
	 * @param GroupFolder The group who's children we should count
	 *
	 * @return Returns the number of child groups
	 */
	INT CountGroupFolderChildren( UInterpGroup* const GroupFolder ) const;

	/**
	 * Fixes up any problems in the folder/group hierarchy caused by bad parenting in previous builds
	 */
	void RepairHierarchyProblems();


	UBOOL KeyIsInSelection(class UInterpGroup* InGroup, INT InTrackIndex, INT InKeyIndex);
	void AddKeyToSelection(class UInterpGroup* InGroup, INT InTrackIndex, INT InKeyIndex, UBOOL bAutoWind);
	void RemoveKeyFromSelection(class UInterpGroup* InGroup, INT InTrackIndex, INT InKeyIndex);
	void ClearKeySelection();
	void CalcSelectedKeyRange(FLOAT& OutStartTime, FLOAT& OutEndTime);
	void SelectKeysInLoopSection();

	// Utils
	void DeleteSelectedKeys(UBOOL bDoTransaction=false);
	void DuplicateSelectedKeys();
	void BeginMoveSelectedKeys();
	void EndMoveSelectedKeys();
	void MoveSelectedKeys(FLOAT DeltaTime);
	void AddKey();
	void SplitAnimKey();
	void ReduceKeys();

	void ViewFitSequence();
	void ViewFitToSelected();
	void ViewFitLoop();
	void ViewFitLoopSequence();
	void ViewFit(FLOAT StartTime, FLOAT EndTime);

	void ChangeKeyInterpMode(EInterpCurveMode NewInterpMode=CIM_Unknown);

	/**
	 * Copies the currently selected group/group.
	 *
	 * @param bCut	Whether or not we should cut instead of simply copying the group/track.
	 */
	void CopySelectedGroupOrTrack(UBOOL bCut);

	/**
	 * Pastes the previously copied group/track.
	 */
	void PasteSelectedGroupOrTrack();

	/**
	 * @return Whether or not we can paste a group/track.
	 */
	UBOOL CanPasteGroupOrTrack();

	/** Deletes the currently active track. */
	void DeleteSelectedTrack();

	/** Deletes the currently active group. */
	void DeleteSelectedGroup();

	/**
	 * Duplicates the specified group
	 *
	 * @param GroupToDuplicate		Group we are going to duplicate.
	 */
	void DuplicateGroup(UInterpGroup* GroupToDuplicate);

	/**
	 * Adds a new track to the specified group.
	 *
	 * @param Group The group to add a track to
	 * @param TrackClass The class of track object we are going to add.
	 * @param TrackToCopy A optional track to copy instead of instantiating a new one.
	 * @param bAllowPrompts TRUE if we should allow a dialog to be summoned to ask for initial information
	 * @param OutNewTrackIndex [Out] The index of the newly created track in its parent group
	 *
	 * @return Returns newly created track (or NULL if failed)
	 */
	UInterpTrack* AddTrackToGroup( UInterpGroup* Group, UClass* TrackClass, UInterpTrack* TrackToCopy, UBOOL bAllowPrompts, INT& OutNewTrackIndex );

	/**
	 * Adds a new track to the selected group.
	 *
	 * @param TrackClass		The class of the track we are adding.
	 * @param TrackToCopy		A optional track to copy instead of instatiating a new one.  If NULL, a new track will be instatiated.
	 */
	void AddTrackToSelectedGroup(UClass* TrackClass, UInterpTrack* TrackToCopy=NULL);

	/** 
	 * Crops the anim key in the currently selected track. 
	 *
	 * @param	bCropBeginning		Whether to crop the section before the position marker or after.
	 */
	void CropAnimKey(UBOOL bCropBeginning);

	/**
	 * Sets the global property name to use for newly created property tracks
	 *
	 * @param NewName The property name
	 */
	static void SetTrackAddPropName( const FName NewName );


	void UpdateMatineeActionConnectors();
	void LockCamToGroup(class UInterpGroup* InGroup);
	class AActor* GetViewedActor();
	virtual void UpdateCameraToGroup();
	void UpdateCamColours();

	void SyncCurveEdView();
	void AddTrackToCurveEd(class UInterpGroup* InGroup, INT InTrackIndex);

	void SetInterpPosition(FLOAT NewPosition);

	/** Refresh the Matinee position marker and viewport state */
	void RefreshInterpPosition();

	/** Make sure particle replay tracks have up-to-date editor-only transient state */
	void UpdateParticleReplayTracks();

	void SelectActiveGroupParent();

	/** Increments the cursor or selected keys by 1 interval amount, as defined by the toolbar combo. */
	void IncrementSelection();

	/** Decrements the cursor or selected keys by 1 interval amount, as defined by the toolbar combo. */
	void DecrementSelection();

	void SelectNextKey();
	void SelectPreviousKey();

	/**
	 * Zooms the curve editor and track editor in or out by the specified amount
	 *
	 * @param ZoomAmount			Amount to zoom in or out
	 * @param bZoomToTimeCursorPos	True if we should zoom to the time cursor position, otherwise mouse cursor position
	 */
	void ZoomView( FLOAT ZoomAmount, UBOOL bZoomToTimeCursorPos );

	/** Toggles fixed time step playback mode */
	void SetFixedTimeStepPlayback( UBOOL bInValue );

	/** Updates 'fixed time step' mode based on current playback state and user preferences */
	void UpdateFixedTimeStepPlayback();

	/** Toggles 'prefer frame number' setting */
	void SetPreferFrameNumbers( UBOOL bInValue );

	/** Toggles 'show time cursor pos for all keys' setting */
	void SetShowTimeCursorPosForAllKeys( UBOOL bInValue );

	void SetSnapEnabled(UBOOL bInSnapEnabled);
	
	/** Toggles snapping the current timeline position to 'frames' in Matinee. */
	void SetSnapTimeToFrames( UBOOL bInValue );

	/** Snaps the specified time value to the closest frame */
	FLOAT SnapTimeToNearestFrame( FLOAT InTime ) const;

	FLOAT SnapTime(FLOAT InTime, UBOOL bIgnoreSelectedKeys);

	void BeginMoveMarker();
	void EndMoveMarker();
	void SetInterpEnd(FLOAT NewInterpLength);
	void MoveLoopMarker(FLOAT NewMarkerPos, UBOOL bIsStart);

	void BeginDrag3DHandle(UInterpGroup* Group, INT TrackIndex);
	void Move3DHandle(UInterpGroup* Group, INT TrackIndex, INT KeyIndex, UBOOL bArriving, const FVector& Delta);
	void EndDrag3DHandle();
	void MoveInitialPosition(const FVector& Delta, const FRotator& DeltaRot);

	void ActorModified();
	void ActorSelectionChange();
	void CamMoved(const FVector& NewCamLocation, const FRotator& NewCamRotation);
	UBOOL ProcessKeyPress(FName Key, UBOOL bCtrlDown, UBOOL bAltDown);

	void InterpEdUndo();
	void InterpEdRedo();

	void MoveActiveBy(INT MoveBy);
	void MoveActiveUp();
	void MoveActiveDown();

	void DrawTracks3D(const FSceneView* View, FPrimitiveDrawInterface* PDI);
	void DrawModeHUD(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,const FSceneView* View,FCanvas* Canvas);

	void TickInterp(FLOAT DeltaSeconds);

	/** Constrains the maximum frame rate to the fixed time step rate when playing back in that mode */
	void ConstrainFixedTimeStepFrameRate();

	static void UpdateAttachedLocations(AActor* BaseActor);
	static void InitInterpTrackClasses();

	WxInterpEdToolBar* ToolBar;
	WxInterpEdMenuBar* MenuBar;

	wxSplitterWindow* GraphSplitterWnd; // Divides the graph from the track view.
	INT GraphSplitPos;

	/** The property window (dockable) */
	WxPropertyWindow* PropertyWindow;

	/** The curve editor window (dockable) */
	WxCurveEditor* CurveEd;

	/** Director track editor window (upper split of main pane) */
	WxInterpEdVCHolder* DirectorTrackWindow;

	/** Main track editor window (lower split of main pane) */
	WxInterpEdVCHolder* TrackWindow;

	UTexture2D*	BarGradText;
	FColor PosMarkerColor;
	FColor RegionFillColor;
	FColor RegionBorderColor;

	class USeqAct_Interp* Interp;
	class UInterpData* IData;

	// Only 1 track can be 'Active' at a time. This will be used for new keys etc.
	// You may have a Group selected but no Tracks (eg. empty group)
	class UInterpGroup* ActiveGroup;
	INT ActiveTrackIndex;

	// If we are connecting the camera to a particular group, this is it. If not, its NULL;
	class UInterpGroup* CamViewGroup;

	// Editor-specific Object, containing preferences and selection set to be serialised/undone.
	UInterpEdOptions* Opt;

	// Are we currently editing the value of a keyframe. This should only be true if there is one keyframe selected and the time is currently set to it.
	UBOOL bAdjustingKeyframe;

	// If we are looping 
	UBOOL bLoopingSection;

	/** The real-time that we started playback last */
	DOUBLE PlaybackStartRealTime;

	/** Number of continuous fixed time step frames we've played so far without any change in play back state,
	    such as time step, reverse mode, etc. */
	UINT NumContinuousFixedTimeStepFrames;

	// Currently moving a curve handle in the 3D viewport.
	UBOOL bDragging3DHandle;

	// Multiplier for preview playback of sequence
	FLOAT PlaybackSpeed;

	// Whether to draw the 3D version of any tracks.
	UBOOL bHide3DTrackView;

	/** Indicates if zoom should auto-center on the current scrub position. */
	UBOOL bZoomToScrubPos;

	/** Window menu item for toggling the curve editor */
	wxMenuItem* CurveEdToggleMenuItem;

	/** Snap settings. */
	UBOOL bSnapEnabled;
	UBOOL bSnapToKeys;
	UBOOL bSnapToFrames;
	FLOAT SnapAmount;

	/** True if the interp timeline position should be be snapped to the Matinee frame rate */
	UBOOL bSnapTimeToFrames;

	/** True if fixed time step playback is enabled */
	UBOOL bFixedTimeStepPlayback;
	
	/** True if the user prefers frame numbers to be drawn on track key labels (instead of time values) */
	UBOOL bPreferFrameNumbers;

	/** True if we should draw the position of the time cursor relative to the start of each key right
	    next to time cursor in the track view */
	UBOOL bShowTimeCursorPosForAllKeys;

	/** Initial curve interpolation mode for newly created keys.  This is loaded and saved to/from the user's
	  * editor preference file. */
	EInterpCurveMode InitialInterpMode;

	UTransactor* NormalTransactor;
	UInterpEdTransBuffer* InterpEdTrans;

	/** Set to TRUE in OnClose, at which point the editor is no longer ticked. */
	UBOOL	bClosed;

	/** If TRUE, the editor is modifying a CameraAnim, and functionality is tweaked appropriately */
	UBOOL	bEditingCameraAnim;

	// Static list of all InterpTrack subclasses.
	static TArray<UClass*>	InterpTrackClasses;
	static UBOOL			bInterpTrackClassesInitialized;

	// Used to convert between seconds and size on the timeline
	INT		TrackViewSizeX;
	FLOAT	PixelsPerSec;
	FLOAT	NavPixelsPerSecond;

	FLOAT	ViewStartTime;
	FLOAT	ViewEndTime;

	EInterpEdMarkerType	GrabbedMarkerType;

	UBOOL	bDrawSnappingLine;
	FLOAT	SnappingLinePosition;
	FLOAT	UnsnappedMarkerPos;

	/** Width of track editor labels on left hand side */
	INT LabelWidth;

	/** Creates a popup context menu based on the item under the mouse cursor.
	* @param	Viewport	FViewport for the FInterpEdViewportClient.
	* @param	HitResult	HHitProxy returned by FViewport::GetHitProxy( ).
	* @return	A new wxMenu with context-appropriate menu options or NULL if there are no appropriate menu options.
	*/
	virtual wxMenu	*CreateContextMenu( FViewport *Viewport, const HHitProxy *HitResult );

	/** Returns TRUE if Matinee is fully initialized */
	UBOOL IsInitialized() const
	{
		return bIsInitialized;
	}

	/** Returns TRUE if viewport frame stats are currently enabled */
	UBOOL IsViewportFrameStatsEnabled() const
	{
		return bViewportFrameStatsEnabled;
	}


protected:
	
	/** TRUE if Matinee is fully initialized */
	UBOOL bIsInitialized;

	/** TRUE if viewport frame stats are currently enabled */
	UBOOL bViewportFrameStatsEnabled;


	/**
	 *	This function returns the name of the docking parent.  This name is used for saving and loading the layout files.
	 *  @return A string representing a name to use for this docking parent.
	 */
	virtual const TCHAR* GetDockingParentName() const;

	/**
	 * @return The current version of the docking parent, this value needs to be increased every time new docking windows are added or removed.
	 */
	virtual const INT GetDockingParentVersion() const;

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxMBInterpEdTabMenu
-----------------------------------------------------------------------------*/

class WxMBInterpEdTabMenu : public wxMenu
{
public:
	WxMBInterpEdTabMenu(WxInterpEd* InterpEd);
	~WxMBInterpEdTabMenu();
};


/*-----------------------------------------------------------------------------
	WxMBInterpEdGroupMenu
-----------------------------------------------------------------------------*/

class WxMBInterpEdGroupMenu : public wxMenu
{
public:
	WxMBInterpEdGroupMenu(WxInterpEd* InterpEd);
	~WxMBInterpEdGroupMenu();
};

/*-----------------------------------------------------------------------------
	WxMBInterpEdTrackMenu
-----------------------------------------------------------------------------*/

class WxMBInterpEdTrackMenu : public wxMenu
{
public:
	WxMBInterpEdTrackMenu(WxInterpEd* InterpEd);
	~WxMBInterpEdTrackMenu();
};

/*-----------------------------------------------------------------------------
	WxMBInterpEdBkgMenu
-----------------------------------------------------------------------------*/

class WxMBInterpEdBkgMenu : public wxMenu
{
public:
	WxMBInterpEdBkgMenu(WxInterpEd* InterpEd);
	~WxMBInterpEdBkgMenu();
};

/*-----------------------------------------------------------------------------
	WxMBInterpEdKeyMenu
-----------------------------------------------------------------------------*/

class WxMBInterpEdKeyMenu : public wxMenu
{
public:
	WxMBInterpEdKeyMenu( WxInterpEd* InterpEd );
	~WxMBInterpEdKeyMenu();
};


/*-----------------------------------------------------------------------------
	WxMBInterpEdCollapseExpandMenu
-----------------------------------------------------------------------------*/

class WxMBInterpEdCollapseExpandMenu : public wxMenu
{
public:
	WxMBInterpEdCollapseExpandMenu(WxInterpEd* InterpEd);
	virtual ~WxMBInterpEdCollapseExpandMenu();
};



/*-----------------------------------------------------------------------------
	WxCameraAnimEd
-----------------------------------------------------------------------------*/

/** A specialized version of WxInterpEd used for CameraAnim editing.  Tangential features of Matinee are disabled. */
class WxCameraAnimEd : public WxInterpEd
{
public:
	WxCameraAnimEd( wxWindow* InParent, wxWindowID InID, class USeqAct_Interp* InInterp );
	virtual ~WxCameraAnimEd();

	virtual void	OnClose( wxCloseEvent& In );

	/** Creates a popup context menu based on the item under the mouse cursor.
	* @param	Viewport	FViewport for the FInterpEdViewportClient.
	* @param	HitResult	HHitProxy returned by FViewport::GetHitProxy( ).
	* @return	A new wxMenu with context-appropriate menu options or NULL if there are no appropriate menu options.
	*/
	virtual wxMenu	*CreateContextMenu( FViewport *Viewport, const HHitProxy *HitResult );

	virtual void UpdateCameraToGroup();
};


/*-----------------------------------------------------------------------------
	WxMBCameraAnimEdGroupMenu
-----------------------------------------------------------------------------*/

class WxMBCameraAnimEdGroupMenu : public wxMenu
{
public:
	WxMBCameraAnimEdGroupMenu(WxCameraAnimEd* CamAnimEd);
	~WxMBCameraAnimEdGroupMenu();
};

#endif // __INTERPEDITOR_H__
