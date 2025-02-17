//------------------------------------------------------------------------------
// A widget for editing the properties of a node in the face graph.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFaceGraphNodePropertiesWidget_H__
#define FxFaceGraphNodePropertiesWidget_H__

#include "FxPlatform.h"
#include "FxWidget.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/grid.h"
#endif

namespace OC3Ent
{

namespace Face
{

class FxFaceGraphWidget;

class FxNodePropertiesWidget : public wxGrid, public FxWidget
{
	WX_DECLARE_DYNAMIC_CLASS( FxNodePropertiesWidget );
	DECLARE_EVENT_TABLE();
public:
	// Constructor.
	FxNodePropertiesWidget( wxWindow* parent = NULL, FxWidgetMediator* mediator = NULL, 
		wxWindowID id = -1, 
		wxPoint position = wxDefaultPosition, wxSize size = wxDefaultSize, 
		FxInt32 style = wxWANTS_CHARS );

	// Called when the actor is changed.
	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	// Called when the selected faceGraph node(s) change(s).
	virtual void OnFaceGraphNodeSelChanged( FxWidget* sender, 
		const FxArray<FxName>& selFaceGraphNodeNames, FxBool zoomToSelection = FxTrue );
	// Called when the actor's internal data changes.
	virtual void OnActorInternalDataChanged( FxWidget* sender, FxActorDataChangedFlag flags );
	// Called to refresh the widget.
	virtual void OnRefresh( FxWidget* sender );
	
	// Binds the properties widget to a face graph.
	void BindToFaceGraphWidget( FxFaceGraphWidget* pFaceGraphWidget );

protected:
	enum ValueToGet
	{
		MinValue,
		MaxValue,
		InputOperation
	};
	// Fills out the grid.
	void FillGrid();

	// Called when a cell is right-clicked.
	void OnCellRightClick( wxGridEvent& event );
	// Called when a cell changes.
	void OnCellChanged( wxGridEvent& event );
	// Called when a cell is double-clicked.
	void OnCellLeftDClick( wxGridEvent& event );

	// Sets the correct cells to read only.
	void SetCorrectReadOnly();
	// Converts an input node op to a string.
	wxString NodeOpToString( FxInputOp operation );
	// Converts a string to an input node op.
	FxInputOp StringToNodeOp( const wxString& str );
	// Gets the min/max value for a range of nodes.
	wxString GetRange( ValueToGet valToGet );
	// Dispatches a node selection change.
	void DispatchNodeSelChange( const FxArray<FxName>& selNodes );

	// Called when a column is resized.
	void OnColSize( wxGridSizeEvent& event );
	// Called when the grid is resized.
	void OnSize( wxSizeEvent& event );
	
	// Called to copy the current node's properties.
	void OnCopy( wxCommandEvent& event );
	// Called to paste the buffer into the current node's properties.
	void OnPaste( wxCommandEvent& event );

	enum FxNodePropertiesWidgetRows
	{
		rowID_Name = 0,
		rowID_MinVal,
		rowID_MaxVal,
		rowID_NodeOp,
		rowID_UserPropStart
	};
	enum FxNodePropertiesWidgetCols
	{
		colID_Name = 0,
		colID_Value
	};

	// The combiner graph.
	FxFaceGraph* _pGraph;
	// The list of selected nodes.
	FxArray<FxName> _selectedNodes;

	// Copy/Paste buffer
	static FxReal    _copyMin;
	static FxReal	 _copyMax;
	static FxInputOp _copyInputOp;

	// Pointer to the face graph widget
	FxFaceGraphWidget* _pFaceGraphWidget;

	// Flag indicating if the widget is currently deleting rows from the grid.
	FxBool _deletingRows;
	// Flag indicating if the widget is currently editing a user property.
	FxBool _editingUserProp;
};

class FxNodeLinkPropertiesWidget : public wxGrid, public FxWidget
{
	WX_DECLARE_DYNAMIC_CLASS(FxNodeLinkPropertiesWidget);
	DECLARE_EVENT_TABLE();
public:
	enum FxNodeSide
	{
		NS_InputLinks,
		NS_OutputLinks
	};

	// Constructor.
	FxNodeLinkPropertiesWidget( wxWindow* parent = NULL, FxNodeSide side = NS_InputLinks, 
		FxWidgetMediator* mediator = NULL, wxWindowID id = -1, 
		wxPoint position = wxDefaultPosition, wxSize size = wxDefaultSize, 
		FxInt32 style = wxWANTS_CHARS );

	// Called when the actor is changed.
	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	// Called when the selected faceGraph node(s) change(s).
	virtual void OnFaceGraphNodeSelChanged( FxWidget* sender, 
		const FxArray<FxName>& selFaceGraphNodeNames, FxBool zoomToSelection = FxTrue );
	// Called when the link selection changed.
	virtual void OnLinkSelChanged( FxWidget* sender, const FxName& inputNode, 
		const FxName& outputNode );
	// Called when the actor's internal data changes.
	virtual void OnActorInternalDataChanged( FxWidget* sender, FxActorDataChangedFlag flags );
	// Called to refresh the widget.
	virtual void OnRefresh( FxWidget* sender );

protected:
	// Lays out the grid.
	void LayoutGrid();
	// Fills out the grid.
	void FillGrid();
	// Called when a cell is left-clicked.
	void OnCellLeftClick( wxGridEvent& event );
	// Called when a cell is double-clicked.
	void OnCellDoubleClick( wxGridEvent& event );
	// Called when a cell is right-clicked.
	void OnCellRightClick( wxGridEvent& event );

	// Called when a column is resized.
	void OnColSize( wxGridSizeEvent& event );
	// Called when the grid is resized.
	void OnSize( wxSizeEvent& event );

	// Called to copy the current selection into the buffer.
	void OnCopy( wxCommandEvent& event );
	// Called to paste the buffer into the current selection.
	void OnPaste( wxCommandEvent& event );
	// Called to delete the current selection.
	void OnDelete( wxCommandEvent& event );

	// Whether to display input or output links.
	FxNodeSide _side;
	// The number of rows.
	FxSize _numRows;

	enum FxNodeLinkPropertiesWidgetCols
	{
		colID_LinkedNode = 0,
		colID_LinkFunction
	};

	// The combiner graph.
	FxFaceGraph* _pGraph;
	// The list of selected nodes.
	FxArray<FxName> _selectedNodes;
	// The selected link.
	FxName _linkInputNode;
	FxName _linkOutputNode;

	// Copy/Paste buffer
	static FxLinkFnType	      _copyLinkFnType;
	static FxLinkFnParameters _copyLinkFnParams;
};

} // namespace Face

} // namespace OC3Ent

#endif
