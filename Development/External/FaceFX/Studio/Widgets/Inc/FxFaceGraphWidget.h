//------------------------------------------------------------------------------
// The face graph editor widget.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFaceGraphWidget_H__
#define FxFaceGraphWidget_H__

#include "FxPlatform.h"
#include "FxWidget.h"
#include "FxFaceGraphNodeUserData.h"
#include "FxButton.h"
#include "FxDropButton.h"

namespace OC3Ent
{

namespace Face
{

class FxAnimController;

//------------------------------------------------------------------------------
// Structure for holding the node icons
//------------------------------------------------------------------------------
struct ClassIconInfo
{
	const FxClass* pClass;
	wxBitmap classIcon;
};
typedef FxArray<ClassIconInfo> ClassIconList;

//------------------------------------------------------------------------------
// Face Graph Widget Interface
//------------------------------------------------------------------------------
class FxFaceGraphWidget : public wxWindow, public FxWidget
{
	typedef wxWindow Super;
	WX_DECLARE_DYNAMIC_CLASS( FxFaceGraphWidget )
	DECLARE_EVENT_TABLE()
	friend class FxFaceGraphWidgetOptionsDlg;
	friend class FxFaceGraphWidgetContainer;

public:

//------------------------------------------------------------------------------
// Construction / Destruction
//------------------------------------------------------------------------------
	// Constructor.
	FxFaceGraphWidget( wxWindow* parent = NULL, FxWidgetMediator* mediator = NULL );
	// Destructor.
	~FxFaceGraphWidget();

//------------------------------------------------------------------------------
// Public interface
//------------------------------------------------------------------------------
	// Sets the face graph pointer.
	void SetFaceGraphPointer( FxFaceGraph* pGraph );
	// Returns the face graph pointer.
	FxFaceGraph* GetFaceGraphPointer();
	// Sets the animation controller.
	void SetAnimControllerPointer( FxAnimController* pAnimController );

	// Cleans up any user data attached to the face graph
	void CleanupUserdata( FxFaceGraph* pGraph );

	// Enables/disables auto-panning.
	void SetAutoPanning( FxBool enabled );

	// Returns the popup menu for adding a node.
	wxMenu* GetAddNodePopup();

//------------------------------------------------------------------------------
// Widget interface
//------------------------------------------------------------------------------
	// Called when the application is starting up.
	virtual void OnAppStartup( FxWidget* sender );
	// Called when the application is shutting down.
	virtual void OnAppShutdown( FxWidget* sender );
	// Called when the actor is changed.
	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	// Called when the actor's internal data changes
	virtual void OnActorInternalDataChanged( FxWidget* sender, FxActorDataChangedFlag flags );
	// Called to layout the Face Graph.
	virtual void OnLayoutFaceGraph( FxWidget* sender );
	// Called when a node has been added to the Face Graph.  The node has already
	// been added when this message is sent.
	virtual void OnAddFaceGraphNode( FxWidget* sender, FxFaceGraphNode* node );
	// Called when a node is about to be removed from the Face Graph.  The node has
	// not been removed yet when this message is sent.
	virtual void OnRemoveFaceGraphNode( FxWidget* sender, FxFaceGraphNode* node );
	// Called when the faceGraph node selection changes.
	virtual void OnFaceGraphNodeSelChanged( FxWidget* sender, 
		                      const FxArray<FxName>& selFaceGraphNodeNames, FxBool zoomToSelection = FxTrue );
	// Called when the link selection changes.
	virtual void OnLinkSelChanged( FxWidget* sender, const FxName& inputNode, 
		                   const FxName& outputNode );
	// Called when the widget needs to refresh itself.
	virtual void OnRefresh( FxWidget* sender );
	// Called when the time changes
	virtual void OnTimeChanged( FxWidget* sender, FxReal newTime );
	// Called to get/set options in the face graph
	virtual void OnSerializeFaceGraphSettings( FxWidget* sender, FxBool isGetting, 
		FxArray<FxNodePositionInfo>& nodeInfos, FxInt32& viewLeft, FxInt32& viewTop,
		FxInt32& viewRight, FxInt32& viewBottom, FxReal& zoomLevel );
	// Called so serialize the face graph configuration.
	virtual void OnSerializeOptions(FxWidget* sender, wxFileConfig* config, FxBool isLoading);

protected:
//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------
	enum MouseAction
	{
		MOUSE_NONE,
		MOUSE_PAN,
		MOUSE_ZOOM,
		MOUSE_BOXSELECT,
		MOUSE_MOVENODE,
		MOUSE_STRETCHLINK
	};

	enum ZoomDirection
	{
		ZOOM_NONE,
		ZOOM_IN,
		ZOOM_OUT
	};

	enum HitTestResult
	{
		HIT_NOTHING,
		HIT_NODE,
		HIT_LINKEND,
		HIT_EMPTYLINKEND
	};

//------------------------------------------------------------------------------
// Event handlers
//------------------------------------------------------------------------------
	// Paint handler.
	void OnPaint( wxPaintEvent& event );
	// Resize handler.
	void OnSize( wxSizeEvent& event );

	// Left button down event handler
	void OnLeftDown( wxMouseEvent& event );
	// Mouse movement event handler.
	void OnMouseMove( wxMouseEvent& event );
	// Mouse wheel event handler.
	void OnMouseWheel( wxMouseEvent& event );
	// Left button up event handler.
	void OnLeftUp( wxMouseEvent& event );
	// Right button event handler.
	void OnRightButton( wxMouseEvent& event );
	// Middle button event handler.
	void OnMiddleButton( wxMouseEvent& event );

	// We gained the focus.
	void OnGetFocus( wxFocusEvent& event );
	// We lost the focus.
	void OnLoseFocus( wxFocusEvent& event );

	void OnHelp( wxHelpEvent& event );

	// Resets the view.
	void OnResetView( wxCommandEvent& event );
	// Displays the selected nodes' properties.
	void OnNodeProperties( wxCommandEvent& event );
	// Displays the link properties.
	void OnLinkProperties( wxCommandEvent& event );
	// Deletes the currently selected nodes.
	void OnDeleteNode( wxCommandEvent& event );
	// Deletes the current link.
	void OnDeleteLink( wxCommandEvent& event );
	// Adds a node.
	void OnAddNode( wxCommandEvent& event );
	// Toggles visualizes the values of the face graph nodes.
	void OnToggleValues( wxCommandEvent& event );
	// Toggles drawing the links as curves.
	void OnToggleCurves( wxCommandEvent& event );
	// Toggles animating the transitions between selected dag nodes.
	void OnToggleTransitions( wxCommandEvent& event );
	// Toggles automatically panning to selected nodes.
	void OnToggleAutopan( wxCommandEvent& event );
	// Toggles whether or not to draw links only between selected nodes.
	void OnToggleLinks( wxCommandEvent& event );
	// Re-layout the graph.
	void OnRelayout( wxCommandEvent& event );
	// Display the options dialog.
	void OnOptions( wxCommandEvent& event );
	// Receives timer messages.
	void OnTimer( wxTimerEvent& event );
	// Creates the "Speech Gestures" face graph fragment
	void OnCreateSpeechGesturesFragment( wxCommandEvent& event );

//------------------------------------------------------------------------------
// Drawing functions
//------------------------------------------------------------------------------
	// Draw a node on a DC.
	void DrawNode( wxDC* pDC, FxFaceGraphNode* node );
	// Draw a link on a DC.
	void DrawLink( wxDC* pDC, wxPoint fromPoint, wxPoint toPoint, 
		           FxBool isSelected = FxFalse, FxBool isCorrective = FxFalse,
				   wxColour linkColour = *wxBLACK, FxBool touchesSelected = FxFalse );
	// Draw a node's link end boxes.
	void DrawLinkEnds( wxDC* pDC, FxFaceGraphNode* node );

//------------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------------
	// One-time initialization of the control.
	void Initialize();

	// Gets the bounds of a link end box in world space.
	FxRect GetLinkEndBox( FxFaceGraphNode* node, FxBool isInput, FxSize index );

	// Client to world point transformation.
	wxPoint ClientToWorld( const wxPoint& clientPoint );
	// Client to world rect transformation.
	wxRect ClientToWorld( const wxRect& clientRect );

	// World to client point transformation.
	wxPoint WorldToClient( const wxPoint& worldPoint );
	// World to client rect transformation.
	wxRect WorldToClient( const wxRect& worldRect );
	// World to client size transformation.
	wxSize WorldToClient( const wxSize& worldSize );

	// Zoom the view.
	void Zoom( const wxPoint& zoomPoint, FxReal zoomFactor, ZoomDirection zoomDir );
	// Calculates a panning offset for the window, applies the transformation, 
	// and returns that offset.
	wxPoint Pan( const wxPoint& currentMousePoint, const wxRect& clientRect );

	// Removes a node from the faceGraph, by name.
	void RemoveNode( const FxName& name );
	// Removes a node from the faceGraph.
	void RemoveNode( FxFaceGraphNode* node );

	// Recalculates a node's size.
	void RecalculateNodeSize( FxFaceGraphNode* node );
	// Recalculates the node sizes.
	void RecalculateNodeSizes();
	// Recalculates the node's link end bounding boxes.
	void RecalculateLinkEnds( FxFaceGraphNode* node );
	// Reacalculates all nodes' link end bounding boxes.
	void RecalculateAllLinkEnds();
	// Ensures that a node has all its necessary link end boxes.
	void ProofLinkEnds( FxFaceGraphNode* node );
	// Ensures that all nodes have their necessary link end boxes and positions
	// them correctly.
	void ProofAllLinkEnds();
	// Deletes the link ends and resets the arrays for a node.
	void ResetLinkEnds( FxFaceGraphNode* node );
	// Rebuilds the links.
	void RebuildLinks();

	// Removes the user data from a single node.
	void RemoveUserData( FxFaceGraphNode* node );

	// Arranges the nodes using the Graphviz dot algorithm.
	void DoDefaultLayout();

	// Does the hit testing.
	HitTestResult PerformHitTest( const wxPoint& position );

public:
	void ZoomToSelection();

protected:

//------------------------------------------------------------------------------
// User data access functions
//------------------------------------------------------------------------------
	// Returns true if the node is selected.
	FxBool IsNodeSelected( FxFaceGraphNode* node );
	// Sets if the node is selected.
	void SetNodeSelected( FxFaceGraphNode* node, FxBool selected );
	// Clears all selections.
	void ClearSelected();

	// Returns true if the link is selected.
	FxBool IsLinkSelected( const FxName& firstNode, const FxName& secondNode );
	// Sets the link between the two nodes as selected.
	void SetLinkSelected( const FxName& inputNode, const FxName& outputNode );

	// Returns true if the link is corrective
	FxBool IsLinkCorrective( const FxName& firstNode, const FxName& secondNode );

//------------------------------------------------------------------------------
// Selection-wide functions
//------------------------------------------------------------------------------
	// Applies a selection rectangle to the nodes.
	void ApplySelectionRect( wxRect& sel, FxBool toggle = FxFalse );
	// Offsets the selected nodes by a delta.
	void MoveSelectedNodes( const wxPoint& delta );

private:
//------------------------------------------------------------------------------
// Member variables
//------------------------------------------------------------------------------
	// A pointer to the face graph.
	FxFaceGraph* _pGraph;
	// A pointer to the animation controller.
	FxAnimController* _pAnimController;

	// The list of selected nodes.
	FxArray<FxName> _selectedNodes;

	// The current view rect.
	wxRect  _viewRect;
	// The zoom level.
	FxReal _zoomLevel;
	// The inverse of the zoom level.
	FxReal _invZoomLevel;
	// The current mouse action.
	MouseAction _mouseAction;
	// The last mouse point.
	wxPoint _lastPos;
	// The selection rectangle.
	wxRect _selection;

	// The current faceGraph node.
	FxFaceGraphNode* _currentNode;
	// The current link end.
	FxLinkEnd* _currentLinkEnd;

	// The start link end.
	FxLinkEnd* _linkStart;
	FxFaceGraphNode* _linkStartNode;
	// The end link end.
	FxLinkEnd* _linkEnd;
	FxFaceGraphNode* _linkEndNode;

	// The selected link.
	FxName		_inputNode;
	FxName		_outputNode;

	// Node options - node minimum.
	FxReal _nodeOptionsMinimum;
	// Node options - node maximum.
	FxReal _nodeOptionsMaximum;
	// Node options - node name.
	FxString _nodeOptionsName;
	// The nodes we're operating on.
	FxArray<FxFaceGraphNode*> _optionNodes;

	// The background of a tool tip.
	wxColour _tooltipBackground;
	// The foreground of a tool tip.
	wxColour _tooltipForeground;

	// Whether or not to draw links as curves.
	FxBool _drawCurves;
	// Whether or not to draw nodes shaded according to their value
	FxBool _drawValues;
	// Whether or not to animate transitions between selected dag nodes
	FxBool _animateTransitions;

	// The background image.
	wxBitmap _bmpBackground;
	// The timer for panning.
	wxTimer  _panTimer;
	// The timer for auto-panning.
	wxTimer _autopanTimer;
	// The stopwatch for measuring elapsed time during automatic actions
	wxStopWatch _stopwatch;

	// The current time in the application.
	FxReal _currentTime;

	// Three animation curves for animated transitions.
	FxAnimCurve _xCurve;
	FxAnimCurve _yCurve;
	FxAnimCurve _zoomCurve;
	FxReal      _t;

	// Flagged when the user data has been destroyed and the graph must not be drawn.
	FxBool _userDataDestroyed;

	// The list of FxFaceGraphNode derived node types that can be "placed" into
	// (added to) the face graph by the user.
	FxArray<const FxClass*> _placeableNodeTypes;

	// The flyout
	wxBitmap _flyout;
	// The flyout position
	wxPoint _flyoutPoint;
	// If the flyout is enabled.
	FxBool _flyoutEnabled;
	// Whether or not to transition to the new node
	FxBool _zoomToNode;

	// The list of icons for a given class.
	ClassIconList _classIconList;

	// True if we're doing a 1-d pan
	FxBool _oneDeePan;
	enum PanDirection
	{
		PAN_UNCERTAIN,
		PAN_X,
		PAN_Y
	};
	PanDirection _oneDeePanDirection;

	// Only draw links for selected nodes
	FxBool _drawSelectedNodeLinksOnly;
	// Do we have the focus
	FxBool _hasFocus;
};

//------------------------------------------------------------------------------
// Face Graph Widget Options Dialog
//------------------------------------------------------------------------------
class FxFaceGraphWidgetOptionsDlg : public wxDialog
{
	WX_DECLARE_DYNAMIC_CLASS(FxFaceGraphWidgetOptionsDlg)
	DECLARE_EVENT_TABLE()

public:
	// Constructors.
	FxFaceGraphWidgetOptionsDlg();
	FxFaceGraphWidgetOptionsDlg( wxWindow* parent, wxWindowID id = 10006, 
		const wxString& caption = _("Face Graph Options"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(400,300), 
		long style = wxDEFAULT_DIALOG_STYLE );

	// Destructor
	~FxFaceGraphWidgetOptionsDlg();

	// Creation.
	bool Create( wxWindow* parent, wxWindowID id = 10006, 
		const wxString& caption = _("Face Graph Options"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(400,300), 
		long style = wxDEFAULT_DIALOG_STYLE );

	// Creates the controls and sizers.
	void CreateControls();

	// Should we show tooltips?
	static FxBool ShowToolTips();

protected:
	// Called when the color list box changes
	void OnColourListChange( wxCommandEvent& event );
	// Called when the user clicks the "Change..." button to change a colour
	void OnColourChange( wxCommandEvent& event );
	// Called when the user clicks the Apply button
	void OnApply( wxCommandEvent& event );
	// Called when the user clicks the OK button
	void OnOK( wxCommandEvent& event );
	// Called when the user changes a checkbox.
	void OnCheck( wxCommandEvent& event );

	// Gets the options from the parent.
	void GetOptionsFromParent();
	// Sets the options in the parent.
	void SetOptionsInParent();

	FxFaceGraphWidget*	_facegraphWidget;
	wxColour*			_linkFnColours;
	wxButton*			_applyButton;
	wxListBox*			_colourList;
	wxStaticBitmap*		_colourPreview;

	wxCheckBox*			_valueVisualizationMode;
	wxCheckBox*			_bezierLinks;
	wxCheckBox*			_animatedTransitions;
	wxCheckBox*			_drawSelectedLinksOnly;
};

class FxFaceGraphWidgetContainer : public wxWindow
{
	typedef wxWindow Super;
	DECLARE_EVENT_TABLE();

public:
	FxFaceGraphWidgetContainer(wxWindow* parent = NULL, FxWidgetMediator* mediator = NULL);
	~FxFaceGraphWidgetContainer();

	void UpdateControlStates(FxBool state);

	FxFaceGraphWidget* GetFaceGraphWidget() { return _faceGraphWidget; }

protected:
	void OnPaint(wxPaintEvent& event);
	void DrawToolbar(wxDC* pDC);
	void RefreshChildren();

	void OnSize(wxSizeEvent& event);
	void DoResize();

	void CreateToolbarControls();

	void OnToolbarAddNodeButton(wxCommandEvent& event);
	void OnToolbarAddNodeDrop(wxCommandEvent& event);
	void OnAddNodeDropSelection(wxCommandEvent& event);
	void OnToolbarNodeValueVizButton(wxCommandEvent& event);
	void OnToolbarRelayoutButton(wxCommandEvent& event);
	void OnToolbarResetViewButton(wxCommandEvent& event);
	void OnToolbarFindSelectionButton(wxCommandEvent& event);
	void OnToolbarMakeSGSetupButton(wxCommandEvent& event);

	void OnUpdateUI(wxUpdateUIEvent& event);

	void LoadIcons();

	FxFaceGraphWidget*	_faceGraphWidget;
	wxRect				_toolbarRect;
	wxRect				_widgetRect;
	FxInt32				_selectedNodeType;

	FxDropButton*		_toolbarAddNodeButton;
	FxButton*			_toolbarNodeValueVizButton;
	FxButton*			_toolbarRelayoutButton;
	FxButton*			_toolbarResetViewButton;
	FxButton*			_toolbarFindSelectionButton;
	FxButton*			_toolbarMakeSGSetupButton;
};


} // namespace Face

} // namespace OC3Ent

#endif
