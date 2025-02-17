//------------------------------------------------------------------------------
// The face graph editor widget.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxFaceGraphWidget.h"
#include "FxVec2.h"
#include "FxMath.h"
#include "FxValidators.h"
#include "FxLinkFnEditDlg.h"
#include "FxConsole.h"
#include "FxProgressDialog.h"
#include "FxAnimController.h"
#include "FxColourMapping.h"
#include "FxStudioOptions.h"
#include "FxFunctionPlotter.h"
#include "FxStudioApp.h"
#include "FxVM.h"
#include "FxTearoffWindow.h"
#include "FxSessionProxy.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/file.h"
	#include "wx/textdlg.h"
	#include "wx/colordlg.h"
	#include "wx/dynlib.h"
	#include "wx/dcbuffer.h"
	#include "wx/confbase.h"
	#include "wx/fileconf.h"
#endif

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// Event table
//------------------------------------------------------------------------------
enum TimerIDs
{
	timerID_PAN = 154,
	timerID_AUTOPAN
};

#define ADDNODEMENU_MODIFIER 9285

// If there are ever more than 128 types of "placeable" nodes, this number will
// need to be updated.
#define MAX_NODE_TYPES 128

WX_IMPLEMENT_DYNAMIC_CLASS( FxFaceGraphWidget, wxWindow )

BEGIN_EVENT_TABLE( FxFaceGraphWidget, wxWindow )
	EVT_PAINT( FxFaceGraphWidget::OnPaint )
	EVT_SIZE( FxFaceGraphWidget::OnSize )

	EVT_LEFT_DOWN( FxFaceGraphWidget::OnLeftDown )
	EVT_MOTION( FxFaceGraphWidget::OnMouseMove )
	EVT_MOUSEWHEEL( FxFaceGraphWidget::OnMouseWheel )
	EVT_LEFT_UP( FxFaceGraphWidget::OnLeftUp )

	EVT_RIGHT_DOWN( FxFaceGraphWidget::OnRightButton )
	EVT_RIGHT_UP( FxFaceGraphWidget::OnRightButton )
	EVT_MIDDLE_DOWN( FxFaceGraphWidget::OnMiddleButton )
	EVT_MIDDLE_UP( FxFaceGraphWidget::OnMiddleButton )

	EVT_SET_FOCUS( FxFaceGraphWidget::OnGetFocus )
	EVT_KILL_FOCUS( FxFaceGraphWidget::OnLoseFocus )

	EVT_HELP_RANGE( wxID_ANY, wxID_HIGHEST, FxFaceGraphWidget::OnHelp )

	EVT_MENU( MenuID_FaceGraphWidgetResetView, FxFaceGraphWidget::OnResetView )
	EVT_MENU( MenuID_FaceGraphWidgetLinkProperties, FxFaceGraphWidget::OnLinkProperties )
	EVT_MENU( MenuID_FaceGraphWidgetDeleteNode, FxFaceGraphWidget::OnDeleteNode )
	EVT_MENU( MenuID_FaceGraphWidgetDeleteLink, FxFaceGraphWidget::OnDeleteLink )
	EVT_MENU( MenuID_FaceGraphWidgetToggleValues, FxFaceGraphWidget::OnToggleValues )
	EVT_MENU( MenuID_FaceGraphWidgetToggleCurves, FxFaceGraphWidget::OnToggleCurves )
	EVT_MENU( MenuID_FaceGraphWidgetToggleTransitions, FxFaceGraphWidget::OnToggleTransitions )
	EVT_MENU( MenuID_FaceGraphWidgetToggleAutopan, FxFaceGraphWidget::OnToggleAutopan )
	EVT_MENU( MenuID_FaceGraphWidgetToggleLinks, FxFaceGraphWidget::OnToggleLinks )
	EVT_MENU( MenuID_FaceGraphWidgetRelayout, FxFaceGraphWidget::OnRelayout )
	EVT_MENU( MenuID_FaceGraphWidgetOptions, FxFaceGraphWidget::OnOptions )
	EVT_MENU( MenuID_FaceGraphWidgetCreateSpeechGesturesFragment, FxFaceGraphWidget::OnCreateSpeechGesturesFragment )
	
	EVT_MENU_RANGE( ADDNODEMENU_MODIFIER, ADDNODEMENU_MODIFIER + MAX_NODE_TYPES, FxFaceGraphWidget::OnAddNode )

	EVT_TIMER( timerID_PAN, FxFaceGraphWidget::OnTimer )
	EVT_TIMER( timerID_AUTOPAN, FxFaceGraphWidget::OnTimer )
END_EVENT_TABLE()

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
// The origin
static const wxPoint kOrigin( 0, 0 );
// The size of the link end boxes
static const wxSize kLinkEndBoxSize( 14, 18 );
// The size of the space between link end boxes
static const wxSize kLinkEndBoxSpacer( 0, 10 );
// The default size of a node
static const wxSize kNodeBox( 480, 0 );
// The height of the node header
static const wxSize kNodeHeaderBox( 0, 48 );
// The height of the node footer
static const wxSize kNodeFooterBox( 0, 24 );
// The "null" selection box
static const wxRect kNullSelection( 0, 0, 0, 0 );

#define FxPoint2wxPoint(pt) wxPoint((pt).x, (pt).y)
#define FxSize2D2wxSize(size) wxSize((size).x, (size).y)
#define FxRect2wxRect(rect) wxRect(wxPoint((rect).GetPosition().x,(rect).GetPosition().y), wxSize((rect).GetSize().x, (rect).GetSize().y))

//------------------------------------------------------------------------------
// Construction / Destruction
//------------------------------------------------------------------------------
// Constructor
FxFaceGraphWidget::FxFaceGraphWidget( wxWindow* parent, FxWidgetMediator* mediator )
	: Super( parent, -1, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE|wxCLIP_CHILDREN )
	, FxWidget( mediator )
	, _pGraph( NULL )
	, _pAnimController( NULL )
	, _viewRect( 0, 0, 640, 480 )
	, _zoomLevel( 1.0f )
	, _invZoomLevel( 1.0f )
	, _mouseAction( MOUSE_NONE )
	, _lastPos( kOrigin )
	, _selection( kNullSelection )
	, _currentNode( NULL )
	, _currentLinkEnd( NULL )
	, _linkStart( NULL )
	, _linkStartNode( NULL )
	, _linkEnd( NULL )
	, _linkEndNode( NULL )
	, _tooltipBackground( wxSystemSettings::GetColour( wxSYS_COLOUR_INFOBK ) )
	, _tooltipForeground( wxSystemSettings::GetColour( wxSYS_COLOUR_INFOTEXT ) )
	, _drawCurves( FxTrue )
	, _drawValues( FxFalse )
	, _animateTransitions( FxTrue )
	, _bmpBackground( wxNullBitmap )
	, _currentTime( 0.0f )
	, _userDataDestroyed( FxTrue )
	, _zoomToNode( FxTrue )
	, _flyoutEnabled( FxFalse )
	, _oneDeePan( FxFalse )
	, _oneDeePanDirection( PAN_UNCERTAIN )
	, _drawSelectedNodeLinksOnly( FxFalse )
	, _hasFocus( FxFalse )
{
	Initialize();

	static const int numEntries = 8;
	wxAcceleratorEntry entries[numEntries];
	entries[0].Set( wxACCEL_CTRL, (int)'R', MenuID_FaceGraphWidgetResetView );
	entries[1].Set( wxACCEL_NORMAL, WXK_DELETE, MenuID_FaceGraphWidgetDeleteNode );
	entries[2].Set( wxACCEL_CTRL, (int)'B', MenuID_FaceGraphWidgetToggleCurves );
	entries[3].Set( wxACCEL_CTRL, (int)'D', MenuID_FaceGraphWidgetToggleValues );
	entries[4].Set( wxACCEL_CTRL, (int)'G', MenuID_FaceGraphWidgetRelayout );
	entries[5].Set( wxACCEL_CTRL, (int)'L', MenuID_FaceGraphWidgetToggleLinks );
	entries[6].Set( wxACCEL_CTRL, (int)'P', MenuID_FaceGraphWidgetOptions );
	entries[7].Set( wxACCEL_CTRL, (int)'E', MenuID_FaceGraphWidgetLinkProperties );
	// Note to self: when adding an accelerator entry, update numEntries. It works better.
	wxAcceleratorTable accelerator( numEntries, entries );
	SetAcceleratorTable( accelerator );

	_panTimer.SetOwner( this, timerID_PAN );
	_autopanTimer.SetOwner( this, timerID_AUTOPAN );
}

// Destructor
FxFaceGraphWidget::~FxFaceGraphWidget()
{
	// Ensure that the timers have been stopped.
	_panTimer.Stop();
	_autopanTimer.Stop();
	_stopwatch.Pause();
}

//------------------------------------------------------------------------------
// Public interface
//------------------------------------------------------------------------------
// Sets the face graph pointer
void FxFaceGraphWidget::SetFaceGraphPointer( FxFaceGraph* pGraph )
{
	CleanupUserdata(_pGraph);
	_pGraph = pGraph;
	_currentNode = NULL;
	_currentLinkEnd = NULL;

	if( _pGraph )
	{
		// Ensure there is user data attached to each node.
		FxFaceGraph::Iterator curr = _pGraph->Begin(FxFaceGraphNode::StaticClass());
		FxFaceGraph::Iterator end  = _pGraph->End(FxFaceGraphNode::StaticClass());
		for( ; curr != end; ++curr )
		{
			if( curr.GetNode() && !GetNodeUserData(curr.GetNode() ))
			{
				curr.GetNode()->SetUserData( new FxNodeUserData() );
			}
		}

		RebuildLinks();
		ProofAllLinkEnds();
		RecalculateNodeSizes();
		RecalculateAllLinkEnds();
		//DoDefaultLayout();

		_userDataDestroyed = FxFalse;
	}
}

// Returns the face graph pointer
FxFaceGraph* FxFaceGraphWidget::GetFaceGraphPointer()
{
	return _pGraph;
}

// Sets the animation controller.
void FxFaceGraphWidget::SetAnimControllerPointer( FxAnimController* pAnimController )
{
	_pAnimController = pAnimController;
}

// Cleans up any user data attached to the face graph
void FxFaceGraphWidget::CleanupUserdata( FxFaceGraph* pGraph )
{
	if( pGraph )
	{
		FxFaceGraph::Iterator curr = pGraph->Begin(FxFaceGraphNode::StaticClass());
		FxFaceGraph::Iterator end  = pGraph->End(FxFaceGraphNode::StaticClass());
		for( ; curr != end; ++curr )
		{
			if( curr.GetNode() && GetNodeUserData(curr.GetNode()) )
			{
				ResetLinkEnds( curr.GetNode() );

				delete GetNodeUserData(curr.GetNode());
				curr.GetNode()->SetUserData( NULL );
			}
		}
		_userDataDestroyed = FxTrue;
	}
}

// Enables/disables auto-panning.
void FxFaceGraphWidget::SetAutoPanning( FxBool enabled )
{
	_zoomToNode = enabled;
}

wxMenu* FxFaceGraphWidget::GetAddNodePopup()
{
	wxMenu* popupMenu = new wxMenu;
	for( FxSize i = 0; i < _placeableNodeTypes.Length(); ++i )
	{
		popupMenu->AppendCheckItem( i + ADDNODEMENU_MODIFIER, wxString(_placeableNodeTypes[i]->GetName().GetAsCstr(), wxConvLibc));
	}
	return popupMenu;
}

//------------------------------------------------------------------------------
// Widget interface
//------------------------------------------------------------------------------
void FxFaceGraphWidget::OnAppStartup( FxWidget* FxUnused(sender) )
{
	// Generate the list of "placeable" face graph node types to support
	// proper "add node" functionality.
	for( FxSize i = 0; i < FxClass::GetNumClasses(); ++i )
	{
		const FxClass* pClass = FxClass::GetClass(i);
		if( pClass )
		{
			if( pClass->IsKindOf(FxFaceGraphNode::StaticClass()) &&
				!pClass->IsExactKindOf(FxFaceGraphNode::StaticClass()) )
			{
				FxObject* pObject = pClass->ConstructObject();
				if( pObject )
				{
					FxFaceGraphNode* pNode = FxCast<FxFaceGraphNode>(pObject);
					if( pNode )
					{
						if( pNode->IsPlaceable() )
						{
							_placeableNodeTypes.PushBack(pClass);
						}
					}
					delete pObject;
				}
			}
		}
	}
}

void FxFaceGraphWidget::OnAppShutdown( FxWidget* FxUnused(sender) )
{
	CleanupUserdata(_pGraph);
}

// Called when the actor is changed
void FxFaceGraphWidget::OnActorChanged( FxWidget* FxUnused(sender), FxActor* actor )
{
	if( actor )
	{
		SetFaceGraphPointer(&actor->GetDecompiledFaceGraph());
	}
	else
	{
		SetFaceGraphPointer(NULL);
	}
	//OnResetView(wxCommandEvent());
}

// Called when the actor's internal data changes
void FxFaceGraphWidget::OnActorInternalDataChanged( FxWidget* FxUnused(sender), FxActorDataChangedFlag flags )
{
	if( flags & ADCF_FaceGraphChanged )
	{
		// We may have deleted a link here.  Better rebuild everything just in case.
		RebuildLinks();
		ProofAllLinkEnds();
		RecalculateNodeSizes();
		RecalculateAllLinkEnds();
		Refresh(FALSE);
	}
}

// Called to layout the Face Graph.
void FxFaceGraphWidget::OnLayoutFaceGraph( FxWidget* FxUnused(sender) )
{
	DoDefaultLayout();
	wxCommandEvent temp;
	OnResetView(temp);
}

// Called when a node has been added to the Face Graph.  The node has already
// been added when this message is sent.
void FxFaceGraphWidget::OnAddFaceGraphNode( FxWidget* FxUnused(sender), FxFaceGraphNode* node )
{
	if( node )
	{
		if( !node->GetUserData() )
		{
			node->SetUserData(new FxNodeUserData());
		}

		RebuildLinks();
		ProofAllLinkEnds();
		RecalculateNodeSizes();
		RecalculateAllLinkEnds();
	}
}

// Called when a node is about to be removed from the Face Graph.  The node has
// not been removed yet when this message is sent.
void FxFaceGraphWidget::OnRemoveFaceGraphNode( FxWidget* FxUnused(sender), FxFaceGraphNode* node )
{
	if( node )
	{
		if( _currentNode == node )
		{
			_currentNode = NULL;
			_currentLinkEnd = NULL;
		}
		RemoveUserData(node);
	}
}

// Called when the faceGraph node selection changes
void FxFaceGraphWidget::
OnFaceGraphNodeSelChanged( FxWidget* FxUnused(sender), const FxArray<FxName>& selFaceGraphNodeNames, FxBool zoomToSelection )
{
	if( _pGraph )
	{
		_selectedNodes = selFaceGraphNodeNames;
		// Check if we should retain our link selection
		FxName oldInputNode = _inputNode;
		FxName oldOutputNode = _outputNode;
		ClearSelected();
		FxBool retainLink = FxFalse;
		for( FxSize i = 0; i < selFaceGraphNodeNames.Length(); ++i )
		{
			FxFaceGraphNode* node = _pGraph->FindNode(selFaceGraphNodeNames[i]);
			if( node )
			{
				if( (node->GetName() == oldInputNode || node->GetName() == oldOutputNode) && 
					selFaceGraphNodeNames.Length() == 1 )
				{
					retainLink = FxTrue;
				}
				SetNodeSelected( node, FxTrue );
			}
		}
		if( retainLink )
		{
			SetLinkSelected( oldInputNode, oldOutputNode );
		}

		if( zoomToSelection && _zoomToNode )
		{
			ZoomToSelection();
		}

	}
}

// Called when the link selection changes
void FxFaceGraphWidget::OnLinkSelChanged( FxWidget* FxUnused(sender), 
										   const FxName& inputNode, 
										   const FxName& outputNode )
{
	SetLinkSelected( inputNode, outputNode );
	Refresh( FALSE );
}

// Called when the widget needs to refresh itself
void FxFaceGraphWidget::OnRefresh( FxWidget* FxUnused(sender) )
{
	//RecalculateNodeSizes();
	Refresh( FALSE );
	Update();
}

// Called when the time changes
void FxFaceGraphWidget::OnTimeChanged( FxWidget* FxUnused(sender), FxReal newTime )
{
	_currentTime = newTime;
	if( _drawValues )
	{
		Refresh( FALSE );
		Update();
	}
}

// Called to get/set options in the face graph
void FxFaceGraphWidget::OnSerializeFaceGraphSettings( FxWidget* FxUnused(sender), FxBool isGetting, 
										  FxArray<FxNodePositionInfo>& nodeInfos, FxInt32& viewLeft, FxInt32& viewTop,
										  FxInt32& viewRight, FxInt32& viewBottom, FxReal& zoomLevel )
{
	if( isGetting )
	{
		nodeInfos.Clear();
		nodeInfos.Reserve(_pGraph->GetNumNodes());
		// Go through each node in the face graph and get their positions.
		FxFaceGraph::Iterator curr = _pGraph->Begin(FxFaceGraphNode::StaticClass());
		FxFaceGraph::Iterator end  = _pGraph->End(FxFaceGraphNode::StaticClass());
		for( ; curr != end; ++curr )
		{
			FxFaceGraphNode* node = curr.GetNode();
			if( node )
			{
				FxNodePositionInfo nodeInfo;
				nodeInfo.nodeName = node->GetNameAsString();
				nodeInfo.x = GetNodeUserData(node)->bounds.GetLeft();
				nodeInfo.y = GetNodeUserData(node)->bounds.GetTop();
				nodeInfos.PushBack(nodeInfo);
			}
		}
		viewLeft   = _viewRect.GetLeft();
		viewTop    = _viewRect.GetTop();
		viewRight  = _viewRect.GetRight();
		viewBottom = _viewRect.GetBottom();
		zoomLevel  = _zoomLevel;
	}
	else
	{
		for( FxSize i = 0; i < nodeInfos.Length(); ++i )
		{
			FxFaceGraphNode* node = _pGraph->FindNode(nodeInfos[i].nodeName.GetData());
			if( node )
			{
				GetNodeUserData(node)->bounds.SetLeft(nodeInfos[i].x);
				GetNodeUserData(node)->bounds.SetTop(nodeInfos[i].y);
			}
		}
		_viewRect = wxRect(wxPoint(viewLeft,viewTop), wxPoint(viewRight, viewBottom));
		_zoomLevel = zoomLevel;
		_invZoomLevel = 1/_zoomLevel;
		RebuildLinks();
		ProofAllLinkEnds();
		RecalculateNodeSizes();
		RecalculateAllLinkEnds();
		Refresh(FALSE);
	}
}

void FxFaceGraphWidget::OnSerializeOptions(FxWidget* FxUnused(sender), wxFileConfig* config, FxBool isLoading)
{
	if( isLoading )
	{
		config->Read(wxT("/FaceGraphWidget/DrawNodeValues"), &_drawValues, FxFalse);
		config->Read(wxT("/FaceGraphWidget/DrawCurveLinks"), &_drawCurves, FxTrue);
		config->Read(wxT("/FaceGraphWidget/AnimateTransitions"), &_animateTransitions, FxTrue);
		config->Read(wxT("/FaceGraphWidget/DrawOnlySelectedLinks"), &_drawSelectedNodeLinksOnly, FxFalse);
		config->Read(wxT("/FaceGraphWidget/AutoPanToSelectedNodes"), &_zoomToNode, FxTrue);

		Refresh(FALSE);
	}
	else
	{
		config->Write( wxT("/FaceGraphWidget/DrawNodeValues"), _drawValues );
		config->Write( wxT("/FaceGraphWidget/DrawCurveLinks"), _drawCurves );
		config->Write( wxT("/FaceGraphWidget/AnimateTransitions"), _animateTransitions );
		config->Write( wxT("/FaceGraphWidget/DrawOnlySelectedLinks"), _drawSelectedNodeLinksOnly );
		config->Write( wxT("/FaceGraphWidget/AutoPanToSelectedNodes"), _zoomToNode );
	}
}

//------------------------------------------------------------------------------
// Event handlers
//------------------------------------------------------------------------------
// Paint handler
void FxFaceGraphWidget::OnPaint( wxPaintEvent& FxUnused(event) )
{
	wxRect clientRect = GetClientRect();
	wxPaintDC front( this );

	wxBrush backgroundBrush( wxColour(192, 192, 192), wxSOLID );

	// Set up the backbuffer
	wxMemoryDC dc;
	wxBitmap backbuffer( front.GetSize().GetWidth(), front.GetSize().GetHeight() );
	dc.SelectObject( backbuffer );
	dc.SetBackground( backgroundBrush  );
	dc.Clear();

	// Draw the background
	wxMemoryDC bitmapDC;
	bitmapDC.SelectObject( _bmpBackground );
	dc.Blit( wxPoint( clientRect.GetRight() - _bmpBackground.GetWidth(), clientRect.GetBottom() - _bmpBackground.GetHeight() ),
		bitmapDC.GetSize(), &bitmapDC, bitmapDC.GetLogicalOrigin() );

	if( _pGraph && !_userDataDestroyed )
	{
		FxFaceGraph::Iterator curr = _pGraph->Begin(FxFaceGraphNode::StaticClass());
		FxFaceGraph::Iterator end  = _pGraph->End(FxFaceGraphNode::StaticClass());
		for( ; curr != end; ++curr )
		{
			FxFaceGraphNode* currentNode = curr.GetNode();
			if( currentNode )
			{
				for( FxSize i = 0; i < GetNodeUserData(currentNode)->inputLinkEnds.Length(); ++i )
				{
					FxLinkEnd* input  = GetNodeUserData(currentNode)->inputLinkEnds[i];
					FxLinkEnd* output = input->dest;
					if( output )
					{
						FxBool linkConnectsSelectedNode = IsNodeSelected(_pGraph->FindNode(output->otherNode)) ||
							IsNodeSelected(_pGraph->FindNode(input->otherNode));
						if( (_drawSelectedNodeLinksOnly && linkConnectsSelectedNode) ||
							 !_drawSelectedNodeLinksOnly)
						{
							FxFaceGraphNodeLink linkInfo;
							_pGraph->GetLink(input->otherNode, output->otherNode, linkInfo);
							wxPoint outputPoint( wxPoint(output->bounds.GetPosition().x, output->bounds.GetPosition().y) );
							outputPoint.y += kLinkEndBoxSize.GetHeight() / 2;
							outputPoint.x += kLinkEndBoxSize.GetWidth();
							wxPoint inputPoint( wxPoint(input->bounds.GetPosition().x, input->bounds.GetPosition().y) );
							inputPoint.y += kLinkEndBoxSize.GetHeight() / 2;

							FxBool selected = IsLinkSelected( input->otherNode, output->otherNode );
							FxBool corrective = linkInfo.IsCorrective();
							DrawLink( &dc, WorldToClient(outputPoint), WorldToClient(inputPoint), selected, corrective,
								FxColourMap::Get(linkInfo.GetLinkFnName()), linkConnectsSelectedNode );
						}
					}
				}
			}
		}

		curr = _pGraph->Begin(FxFaceGraphNode::StaticClass());
		for( ; curr != end; ++curr )
		{
			FxFaceGraphNode* currentNode = curr.GetNode();
			if( currentNode )
			{
				DrawLinkEnds( &dc, currentNode );
				DrawNode( &dc, currentNode );
			}
		}

		if( _mouseAction == MOUSE_BOXSELECT )
		{
			if( _selection != kNullSelection )
			{
				wxRect clientSelection = WorldToClient( _selection );
				FxInt32 function = dc.GetLogicalFunction();
				dc.SetLogicalFunction( wxXOR );
				dc.SetPen( wxPen( wxColour( *wxWHITE ), 1, wxDOT ) );
				dc.SetBrush( *wxTRANSPARENT_BRUSH );
				dc.DrawRectangle( clientSelection );
				dc.SetLogicalFunction( function );
			}
		}
		if( _mouseAction == MOUSE_STRETCHLINK && _linkStart )
		{
			wxPoint outputPoint;
			wxPoint inputPoint;
			if( _linkStart->isInput )
			{
				inputPoint.x = _linkStart->bounds.GetPosition().x;
				inputPoint.y = _linkStart->bounds.GetPosition().y + kLinkEndBoxSize.GetHeight() / 2;
				inputPoint = WorldToClient( inputPoint );

				if( !_linkEnd )
				{
					outputPoint = _lastPos;
				}
				else
				{
					outputPoint.x = _linkEnd->bounds.GetPosition().x + kLinkEndBoxSize.GetWidth();
					outputPoint.y = _linkEnd->bounds.GetPosition().y + kLinkEndBoxSize.GetHeight() / 2;
					outputPoint = WorldToClient( outputPoint );
				}
			}
			else
			{
				outputPoint.x = _linkStart->bounds.GetPosition().x + kLinkEndBoxSize.GetWidth();
				outputPoint.y = _linkStart->bounds.GetPosition().y + kLinkEndBoxSize.GetHeight() / 2;
				outputPoint = WorldToClient( outputPoint );

				if( !_linkEnd )
				{
					inputPoint = _lastPos;
				}
				else
				{
					inputPoint.x = _linkEnd->bounds.GetPosition().x;
					inputPoint.y = _linkEnd->bounds.GetPosition().y + kLinkEndBoxSize.GetHeight() / 2;
					inputPoint = WorldToClient( inputPoint );
				}
			}
			DrawLink( &dc, outputPoint, inputPoint, FxFalse, FxFalse, wxColour(100,100,100) );
		}
	}

	// Draw the node's "tooltip", if applicable
	if( _currentNode && !_flyoutEnabled && _zoomLevel > 1.0f )
	{
		wxRect tooltip( _lastPos, wxDefaultSize );
		wxFont font( 10.0f, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE, _("Lucida Sans Unicode") );
		dc.SetFont( font );
		dc.SetTextForeground( _tooltipForeground );
		dc.SetBrush( wxBrush( _tooltipBackground, wxSOLID ) );

		FxInt32 width, height;
		wxString nodeLabel = wxString::FromAscii( _currentNode->GetNameAsCstr() );
		dc.GetTextExtent( nodeLabel, &width, &height );
		width += 5;
		height += 5;

		tooltip.SetWidth( width );
		tooltip.SetHeight( height );
		tooltip.SetTop( tooltip.GetTop() - tooltip.GetHeight() );
		dc.DrawRectangle( tooltip );

		wxPoint textPos( tooltip.GetPosition() );
		textPos.x += 2;
		textPos.y += 2;
		dc.DrawText( nodeLabel, textPos );
	}

	if( _hasFocus )
	{
		dc.SetBrush( *wxTRANSPARENT_BRUSH );
		dc.SetPen( wxPen(FxStudioOptions::GetFocusedWidgetColour(), 1, wxSOLID) );
		dc.DrawRectangle( clientRect );
	}

	// Swap to the front buffer
	front.Blit( front.GetLogicalOrigin(), front.GetSize(), &dc, dc.GetLogicalOrigin(), wxCOPY );
}

// Resize handler
void FxFaceGraphWidget::OnSize( wxSizeEvent& event )
{
	// We need to keep the view rect's aspect ratio equal to the client rect
	wxSize clientSize = event.GetSize();
	_viewRect.SetSize( wxSize( clientSize.GetWidth()  * _zoomLevel,
					           clientSize.GetHeight() * _zoomLevel ) );
}

// Left button down event handler
void FxFaceGraphWidget::OnLeftDown( wxMouseEvent& event )
{
	SetFocus();
	if( _pGraph && !_userDataDestroyed )
	{
		// Cancel autopanning if the user interrupted it.
		if( _autopanTimer.IsRunning() )
		{
			_autopanTimer.Stop();
			_stopwatch.Pause();
		}

		if( !HasCapture() )
		{
			CaptureMouse();
			if( !_panTimer.IsRunning() )
			{
				_stopwatch.Start();
				_panTimer.Start( 1, wxTIMER_CONTINUOUS );
			}
		}

		if( !event.ControlDown() && !event.ShiftDown() )
		{
			HitTestResult res = PerformHitTest( event.GetPosition() );
			if( res == HIT_NODE )
			{
				// Select single node, start move.
				_mouseAction = MOUSE_MOVENODE;
				if( !IsNodeSelected(_currentNode) &&
					(_selectedNodes.Length() != 1 ||
					(_selectedNodes.Length() == 1 && _selectedNodes.Back() != _currentNode->GetName())) )
				{
					wxString command = wxString::Format(wxT("select -type \"node\" -names \"%s\" -nozoom"),
						wxString(_currentNode->GetNameAsCstr(), wxConvLibc));
					FxVM::ExecuteCommand( FxString(command.mb_str(wxConvLibc)) );
				}
			}
			else if( res == HIT_LINKEND )
			{
				FxFaceGraphNode* inputNode  = NULL;
				FxFaceGraphNode* outputNode = NULL;
				_pGraph->DetermineLinkDirection( _currentLinkEnd->otherNode, 
					_currentLinkEnd->dest->otherNode, inputNode, outputNode );
				if( !(_inputNode == inputNode->GetName() && _outputNode == outputNode->GetName()) &&
					!(_inputNode == outputNode->GetName() && _outputNode == inputNode->GetName()) )
				{
					wxString command = wxString::Format(wxT("select -type \"link\" -node1 \"%s\" -node2 \"%s\" -nozoom"),
						wxString(_currentLinkEnd->dest->otherNode.GetAsCstr(), wxConvLibc),
						wxString(_currentLinkEnd->otherNode.GetAsCstr(), wxConvLibc));
					FxVM::ExecuteCommand( FxString(command.mb_str(wxConvLibc)) );
				}
			}
			else if( res == HIT_EMPTYLINKEND )
			{
				if( _selectedNodes.Length() != 1 ||
					(_selectedNodes.Length() == 1 && _selectedNodes.Back() != _currentNode->GetName()) )
				{
					wxString command = wxString::Format(wxT("select -type \"node\" -names \"%s\" -nozoom"),
						wxString(_currentNode->GetNameAsCstr(), wxConvLibc));
					FxVM::ExecuteCommand( FxString(command.mb_str(wxConvLibc)) );
				}

				_mouseAction = MOUSE_STRETCHLINK;
				_linkStart = _currentLinkEnd;
				_linkStartNode = _currentNode;
			}
			else if( res == HIT_NOTHING )
			{
				// Clear selection, begin new selection.
				if( _selectedNodes.Length() > 0 )
				{
					FxVM::ExecuteCommand(FxString("select -type \"node\" -clear -nozoom"));
				}

				_mouseAction = MOUSE_BOXSELECT;
				wxPoint position = ClientToWorld( event.GetPosition() );
				_selection = wxRect( position, position );

				Refresh( FALSE );
			}
		}
		else if( event.ControlDown() && !event.ShiftDown() )
		{
			HitTestResult res = PerformHitTest( event.GetPosition() );
			if( res == HIT_NOTHING )
			{
				// Begin toggle box selection.
				_mouseAction = MOUSE_BOXSELECT;
				wxPoint position = ClientToWorld( event.GetPosition() );
				_selection = wxRect( position, position );

				Refresh( FALSE );
			}
			else if( res == HIT_NODE )
			{
				// Toggle single node selection.
				wxString command = wxString::Format(wxT("select -type \"node\" -names \"%s\" -nozoom"),
					wxString(_currentNode->GetNameAsCstr(), wxConvLibc));
				if( IsNodeSelected(_currentNode) )
				{
					command += wxT(" -remove");
				}
				else
				{
					command += wxT(" -add");
				}
				FxVM::ExecuteCommand( FxString(command.mb_str(wxConvLibc)) );
			}
		}
		else if( event.ShiftDown() && !event.ControlDown() )
		{
			_mouseAction = MOUSE_MOVENODE;
		}
		else if( event.ControlDown() && event.ShiftDown() )
		{
			HitTestResult res = PerformHitTest( event.GetPosition() );
			if( res == HIT_NODE )
			{
				// Begin "easy" link dragging.
				// Do my own hit test to see which side of the node we hit.
				wxRect bounds = FxRect2wxRect(GetNodeUserData(_currentNode)->bounds);
				wxRect leftHalf(bounds);
				leftHalf.SetWidth( bounds.GetSize().GetWidth() / 2 );
				wxRect rightHalf(leftHalf);
				rightHalf.SetLeft( leftHalf.GetLeft() + leftHalf.GetWidth() );

				if( WorldToClient(leftHalf).Contains( event.GetPosition() ) )
				{
					_mouseAction = MOUSE_STRETCHLINK;
					_linkStartNode = _currentNode;
					_linkStart = GetNodeUserData(_linkStartNode)->inputLinkEnds.Back();
				}
				else if ( WorldToClient(rightHalf).Contains( event.GetPosition() ) )
				{
					_mouseAction = MOUSE_STRETCHLINK;
					_linkStartNode = _currentNode;
					_linkStart = GetNodeUserData(_linkStartNode)->outputLinkEnds.Back();
				}
			}
			else if( res == HIT_NOTHING )
			{
				// Begin zoom to selection box.
				_mouseAction = MOUSE_BOXSELECT;
				wxPoint position = ClientToWorld( event.GetPosition() );
				_selection = wxRect( position, position );
			}
		}
	}

	_lastPos = event.GetPosition();
}

// Mouse movement event handler
void FxFaceGraphWidget::OnMouseMove( wxMouseEvent& event )
{
	if( _pGraph && !_userDataDestroyed )
	{
		wxPoint deltaPos = ClientToWorld(_lastPos) - ClientToWorld(event.GetPosition());
		if( _mouseAction == MOUSE_PAN )
		{
			// Cancel autopanning if the user interrupted it.
			if( _autopanTimer.IsRunning() )
			{
				_autopanTimer.Stop();
				_stopwatch.Pause();
			}

			if( _oneDeePan )
			{
				if( _oneDeePanDirection == PAN_UNCERTAIN )
				{
					if( FxAbs(deltaPos.x) > FxAbs(deltaPos.y) )
					{
						_oneDeePanDirection = PAN_X;
					}
					else if( FxAbs(deltaPos.y) > FxAbs(deltaPos.x) )
					{
						_oneDeePanDirection = PAN_Y;
					}
					else
					{
						// Not enough information to start the 1D pan.
						return;
					}
				}

				if( _oneDeePanDirection == PAN_X )
				{
					deltaPos.y = 0;
				}
				else if( _oneDeePanDirection == PAN_Y )
				{
					deltaPos.x = 0;
				}
			}

			_viewRect.Offset( deltaPos );

			Refresh( FALSE );
		}
		else if( _mouseAction == MOUSE_BOXSELECT )
		{
			Pan( event.GetPosition(), GetClientRect() );
			wxPoint position = ClientToWorld( event.GetPosition() );
			_selection.SetBottom( position.y );
			_selection.SetRight( position.x );
			Refresh( FALSE );
		}
		else if( _mouseAction == MOUSE_MOVENODE )
		{
			wxPoint panning = Pan( event.GetPosition(), GetClientRect() );

			wxPoint delta;
			delta.x = -deltaPos.x + panning.x;
			delta.y = -deltaPos.y + panning.y;

			MoveSelectedNodes( delta );
			Refresh( FALSE );
		}
		else if( _mouseAction == MOUSE_STRETCHLINK && _linkStart )
		{
			wxPoint panning = Pan( event.GetPosition(), GetClientRect() );

			HitTestResult res = PerformHitTest( event.GetPosition() );
			if( res == HIT_EMPTYLINKEND )
			{
				if( _currentLinkEnd->isInput != _linkStart->isInput )
				{
					_linkEnd = _currentLinkEnd;
					_linkEndNode = _currentNode;
				}
			}
			else if( res == HIT_NODE )
			{
				// Find our link end.  Note that if the link originates from an input
				// link end, we cannot link to a target node (ie, a node with no outputs)
				if( _linkStart->isInput && GetNodeUserData(_currentNode)->outputLinkEnds.Length() )
				{
					_linkEnd = GetNodeUserData(_currentNode)->outputLinkEnds.Back();
				}
				else if( !_linkStart->isInput )
				{
					_linkEnd = GetNodeUserData(_currentNode)->inputLinkEnds.Back();
				}
				_linkEndNode = _currentNode;
			}
			else
			{
				_linkEnd = NULL;
				_linkEndNode = NULL;
			}

			Refresh( FALSE );
		}
		else
		{
			HitTestResult res = PerformHitTest( event.GetPosition() );
			if( res == HIT_LINKEND )
			{
				if( !_flyoutEnabled )
				{
					FxString inputNodeName;
					FxString outputNodeName;
					if( _currentLinkEnd->isInput )
					{
						outputNodeName = _currentLinkEnd->otherNode.GetAsCstr();
						inputNodeName  = _currentLinkEnd->dest->otherNode.GetAsCstr();
					}
					else
					{
						inputNodeName  = _currentLinkEnd->otherNode.GetAsCstr();
						outputNodeName = _currentLinkEnd->dest->otherNode.GetAsCstr();
					}

					FxFaceGraphNode* inputNode = _pGraph->FindNode( inputNodeName.GetData() );
					FxFaceGraphNode* outputNode = _pGraph->FindNode( outputNodeName.GetData() );
					if( inputNode && outputNode )
					{
						FxFaceGraphNodeLink linkInfo;
						_pGraph->GetLink( inputNodeName.GetData(), outputNodeName.GetData(), linkInfo );
						wxSize plotSize = WorldToClient(wxSize(GetNodeUserData(_currentNode)->bounds.GetWidth(),
															   GetNodeUserData(_currentNode)->bounds.GetHeight() - 
																	kNodeFooterBox.GetHeight() - kNodeHeaderBox.GetHeight()) );
						_flyout = FxFunctionPlotter::Plot( linkInfo.GetLinkFnType(), 
							linkInfo.GetLinkFnParams(), plotSize, outputNode->GetMin(),
							outputNode->GetMax(), wxColour(170,170,175),
							inputNode->GetMin(), inputNode->GetMax(), FxTrue, wxColour(100,100,100), 
							*wxBLACK, *wxWHITE, wxString::FromAscii(outputNodeName.GetData()), 
							wxString::FromAscii(inputNodeName.GetData()) );

						wxPoint position = event.GetPosition();
						position.x += 10;
						position.y -= 10;
						_flyoutPoint = position;
						_flyoutEnabled = FxTrue;
						Refresh(FALSE);
					}
				}
			}
			else
			{
				if( _flyoutEnabled )
				{
					_flyoutEnabled = FxFalse;
					Refresh(FALSE);
				}
			}
		}
	}

	_lastPos = event.GetPosition();
}

// Mouse wheel event handler
void FxFaceGraphWidget::OnMouseWheel( wxMouseEvent& event )
{
	if( _pGraph && !_userDataDestroyed )
	{
		// Zoom
		if( _autopanTimer.IsRunning() )
		{
			_autopanTimer.Stop();
			_stopwatch.Pause();
		}

		ZoomDirection zoomDir = ZOOM_OUT;
		if( event.GetWheelRotation() < 0 )
		{
			zoomDir = ZOOM_IN;
		}

		wxRect clientRect = GetClientRect();
		wxPoint center( clientRect.GetX() + (clientRect.GetWidth() / 2 ),
						clientRect.GetY() + (clientRect.GetHeight() / 2 ) );
		Zoom( center, 1.15f, zoomDir );

		Refresh( FALSE );
	}
}

// Left button up event handler
void FxFaceGraphWidget::OnLeftUp( wxMouseEvent& event )
{
	if( _pGraph && !_userDataDestroyed )
	{
		if( HasCapture() )
		{
			ReleaseMouse();
			_panTimer.Stop();
			_stopwatch.Pause();
		}

		if( _mouseAction == MOUSE_BOXSELECT )
		{
			if( event.ControlDown() && event.ShiftDown() )
			{
				// Zoom to selection.
				wxPoint boundsMin;
				wxPoint boundsMax;
				boundsMin.x = FxMin( _selection.GetLeft(), _selection.GetRight() );
				boundsMax.x = FxMax( _selection.GetLeft(), _selection.GetRight() );
				boundsMin.y = FxMin( _selection.GetTop(), _selection.GetBottom() );
				boundsMax.y = FxMax( _selection.GetTop(), _selection.GetBottom() );
				wxRect clientRect = GetClientRect();
				FxReal halfClientWidth  = clientRect.GetWidth() * 0.5f;
				FxReal halfClientHeight = clientRect.GetHeight() * 0.5f;
				wxPoint selectionCenter( (boundsMax.x + boundsMin.x)/2, (boundsMax.y + boundsMin.y)/2 );

				FxReal dx = (boundsMax.x - boundsMin.x) * 0.5f;
				FxReal dy = (boundsMax.y - boundsMin.y) * 0.5f;
				_zoomLevel = FxMax( dx / halfClientWidth, dy / halfClientHeight ) + 0.01f;
				_invZoomLevel = 1.0f / _zoomLevel;

				wxSize viewSize(clientRect.GetWidth() * _zoomLevel, clientRect.GetHeight() * _zoomLevel);
				_viewRect.SetSize( viewSize );
				_viewRect.SetPosition( wxPoint(selectionCenter.x - (0.5f * viewSize.GetWidth()),
					selectionCenter.y - (0.5f * viewSize.GetHeight())) );

				_mouseAction = MOUSE_NONE;
				_lastPos = event.GetPosition();
				Refresh(FALSE);
			}
			else if( event.ControlDown() && !event.ShiftDown() )
			{
				// Toggle selection
				ApplySelectionRect( _selection, FxTrue );
			}
			else
			{
				// Set selection.
				ApplySelectionRect( _selection );
			}
			_selection = kNullSelection;
			Refresh( FALSE );
		}
		else if( _mouseAction == MOUSE_STRETCHLINK )
		{
			if( _linkStart && _linkEnd && _linkStartNode && _linkEndNode )
			{
				FxName input;
				FxName output;
				if( _linkStart->isInput )
				{
					input = _linkStartNode->GetName();
					output = _linkEndNode->GetName();
				}
				else
				{
					input = _linkEndNode->GetName();
					output = _linkStartNode->GetName();
				}

				// We need to reset these before issuing the graph command
				// so nothing dereferences pointers that have been invalidated
				// as a result of the command's execution.
				_linkStart = _linkEnd = NULL;
				_linkStartNode = _linkEndNode = NULL;
				
				wxString command = wxString::Format(wxT("graph -link -from \"%s\" -to \"%s\" -linkfn \"linear\""),
					wxString(output.GetAsCstr(), wxConvLibc),
					wxString(input.GetAsCstr(), wxConvLibc)	);
				if( FxVM::ExecuteCommand( FxString(command.mb_str(wxConvLibc)) ) == CE_Failure )
				{
					FxMessageBox( _("Link creation failed.  Please refer to the Console for details.") );
				}
			}

			// Ensure all the pointers are reset in the event of a 
			// non-completed linking operation.
			_linkStart = _linkEnd = NULL;
			_linkStartNode = _linkEndNode = NULL;
		}

		if( _mouseAction ) 
		{
			Refresh( FALSE );
		}
	}

	_mouseAction = MOUSE_NONE;
	_lastPos = event.GetPosition();
}

// Right button event handler
void FxFaceGraphWidget::OnRightButton( wxMouseEvent& event )
{
	SetFocus();
	if( _pGraph && !_userDataDestroyed )
	{
		// Cancel autopanning if the user interrupted it.
		if( _autopanTimer.IsRunning() )
		{
			_autopanTimer.Stop();
			_stopwatch.Pause();
		}

		if( event.RightUp() )
		{
			_lastPos = event.GetPosition();

			HitTestResult res = PerformHitTest( event.GetPosition() );
			if( HIT_NODE == res )
			{
				if( !IsNodeSelected(_currentNode) )
				{
					wxString command = wxString::Format(wxT("select -type \"node\" -names \"%s\" -nozoom"), 
						wxString::FromAscii(_currentNode->GetNameAsCstr()));
					FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
					Refresh( FALSE );
				}

				// Show the context menu
				wxMenu popupMenu;
				popupMenu.Append( MenuID_FaceGraphWidgetDeleteNode, _("Delete node\tDel"), _("Removes the node from the face graph") );
				PopupMenu( &popupMenu, _lastPos );
			}
			else if( HIT_LINKEND == res )
			{
				wxMenu popupMenu;
				popupMenu.Append( MenuID_FaceGraphWidgetLinkProperties, _("Edit link properties...\tCtrl+E"), _("Displays the link function editor") );
				popupMenu.Append( MenuID_FaceGraphWidgetDeleteLink, _("Delete link"), _("Deletes the current link") );
				PopupMenu( &popupMenu, _lastPos );
			}
			else if( HIT_NOTHING == res )
			{
				wxMenu popupMenu;
				wxMenu* addNodeMenu = new wxMenu();
				for( FxSize i = 0; i < _placeableNodeTypes.Length(); ++i )
				{
					addNodeMenu->Append( i + ADDNODEMENU_MODIFIER, wxString(_placeableNodeTypes[i]->GetName().GetAsCstr(), wxConvLibc));
				}
				// Magic numbers don't matter here.  Since we can't select the 
				// submenus themselves, we don't need to worry about processing 
				// a menu message with an unknown ID.
				popupMenu.Append( 4573, _("Add node"), addNodeMenu, _("Adds a node to the face graph") );
				popupMenu.AppendSeparator();
				popupMenu.Append( MenuID_FaceGraphWidgetToggleCurves, _("&Bezier links\tCtrl+B"), _("Switches drawing curves for the links on and off"), TRUE );
				popupMenu.Append( MenuID_FaceGraphWidgetToggleValues, _("&Node value visualization\tCtrl+D"), _("Shades the nodes according to their value"), TRUE );
				popupMenu.Append( MenuID_FaceGraphWidgetToggleLinks, _("Draw only selected node &links\tCtrl+L"), _("Only draws links that are connected to a selected face graph node"), TRUE );
				popupMenu.Append( MenuID_FaceGraphWidgetToggleTransitions, _("Animated transitions"), _("Toggles animating the transitions between selected face graph nodes"), TRUE );
				popupMenu.Append( MenuID_FaceGraphWidgetToggleAutopan, _("Auto-find selected nodes"), _("Toggles automatically finding selected nodes in the face graph."), TRUE );
				popupMenu.AppendSeparator();
				popupMenu.Append( MenuID_FaceGraphWidgetRelayout, _("Relayout graph\tCtrl+G"), _("Automatically generates a new layout for the graph.") );
				popupMenu.Append( MenuID_FaceGraphWidgetResetView, _("Reset view\tCtrl+R"), _("Bring the entire graph into view.") );
				popupMenu.AppendSeparator();
				popupMenu.Append( MenuID_FaceGraphWidgetCreateSpeechGesturesFragment, _("Create Speech Gestures setup"), _("Creates the fragment of the face graph needed to process speech gestures."));

				popupMenu.Check( MenuID_FaceGraphWidgetToggleCurves, _drawCurves );
				popupMenu.Check( MenuID_FaceGraphWidgetToggleValues, _drawValues );
				popupMenu.Check( MenuID_FaceGraphWidgetToggleTransitions, _animateTransitions );
				popupMenu.Check( MenuID_FaceGraphWidgetToggleLinks, _drawSelectedNodeLinksOnly );
				popupMenu.Check( MenuID_FaceGraphWidgetToggleAutopan, _zoomToNode );

				PopupMenu( &popupMenu, _lastPos );
			}
		}
	}
}

// Middle button event handler
void FxFaceGraphWidget::OnMiddleButton( wxMouseEvent& event )
{
	SetFocus();
	if( _pGraph && !_userDataDestroyed )
	{
		// Cancel autopanning if the user interrupted it.
		if( _autopanTimer.IsRunning() )
		{
			_autopanTimer.Stop();
			_stopwatch.Pause();
		}

		if( event.MiddleDown() )
		{
			if( !HasCapture() )
			{
				CaptureMouse();
			}

			if( !event.ShiftDown() )
			{
				// Free panning
				_mouseAction = MOUSE_PAN;
				_oneDeePan   = FxFalse;
			}
			else if( event.ShiftDown() && !event.ControlDown() && !event.AltDown() )
			{
				// 1D panning.
				_mouseAction = MOUSE_PAN;
				_oneDeePan   = FxTrue;
				_oneDeePanDirection = PAN_UNCERTAIN;
			}
		}
		else if( event.MiddleUp() )
		{
			if( HasCapture() )
			{
				ReleaseMouse();
			}
			_mouseAction = MOUSE_NONE;
			_oneDeePan   = FxFalse;
			_oneDeePanDirection = PAN_UNCERTAIN;
		}
	}
}

// We gained the focus.
void FxFaceGraphWidget::OnGetFocus( wxFocusEvent& FxUnused(event) )
{
	_hasFocus = FxTrue;
	Refresh(FALSE);
}

// We lost the focus.
void FxFaceGraphWidget::OnLoseFocus( wxFocusEvent& FxUnused(event) )
{
	_hasFocus = FxFalse;
	Refresh(FALSE);
}

void FxFaceGraphWidget::OnHelp(wxHelpEvent& FxUnused(event))
{
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().LoadFile();
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().DisplaySection(wxT("Face Graph Tab"));
}

// Resets the view
void FxFaceGraphWidget::OnResetView( wxCommandEvent& FxUnused(event) )
{
	wxRect clientRect = GetClientRect();
	if( _pGraph && clientRect.GetWidth() && clientRect.GetHeight()  && !_userDataDestroyed )
	{
		// Calculate the bounding box of the entire face graph
		wxPoint boundsMin( FX_INT32_MAX, FX_INT32_MAX );
		wxPoint boundsMax( FX_INT32_MIN, FX_INT32_MIN );

		FxFaceGraph::Iterator curr = _pGraph->Begin(FxFaceGraphNode::StaticClass());
		FxFaceGraph::Iterator end  = _pGraph->End(FxFaceGraphNode::StaticClass());
		for( ; curr != end; ++curr )
		{
			FxFaceGraphNode* node = curr.GetNode();
			if( node )
			{
				const FxRect& nodeBounds = GetNodeUserData(node)->bounds;
				boundsMin.x = FxMin( nodeBounds.GetLeft(),   boundsMin.x );
				boundsMin.y = FxMin( nodeBounds.GetTop(),    boundsMin.y );
				boundsMax.x = FxMax( nodeBounds.GetRight(),  boundsMax.x );
				boundsMax.y = FxMax( nodeBounds.GetBottom(), boundsMax.y );
			}
		}

		if( boundsMin.x != FX_INT32_MAX )
		{
			wxPoint graphCenter( (boundsMax.x + boundsMin.x)/2,
								(boundsMax.y + boundsMin.y)/2 );

			FxReal dx = (boundsMax.x - boundsMin.x) * 0.5f;
			FxReal dy = (boundsMax.y - boundsMin.y) * 0.5f;
			_zoomLevel = FxMax( dx / (clientRect.GetWidth()/2), dy / (clientRect.GetHeight()/2) );
			_invZoomLevel = 1.0f / _zoomLevel;

			wxSize viewSize(clientRect.GetWidth() * _zoomLevel, clientRect.GetHeight() * _zoomLevel);
			_viewRect.SetSize( viewSize );
			_viewRect.SetPosition( wxPoint(graphCenter.x - (0.5f * viewSize.GetWidth()),
										graphCenter.y - (0.5f * viewSize.GetHeight())) );
		}
		Refresh( FALSE );
	}
}

// Displays the selected nodes' properties
void FxFaceGraphWidget::OnNodeProperties( wxCommandEvent& FxUnused(event) )
{
}

// Displays the link properties
void FxFaceGraphWidget::OnLinkProperties( wxCommandEvent& FxUnused(event) )
{
	if( _pGraph && _inputNode != FxName::NullName && _outputNode != FxName::NullName )
	{
		FxFaceGraphNode* inputNode  = NULL;
		FxFaceGraphNode* outputNode = NULL;
		_pGraph->DetermineLinkDirection( _inputNode, _outputNode, inputNode, outputNode );

		FxLinkFnEditDlg linkFnEditDlg(this, _mediator, _pGraph, inputNode->GetName(), outputNode->GetName());
		FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
		linkFnEditDlg.ShowModal();
		FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();

		Refresh( FALSE );
	}
}

// Deletes the currently selected nodes
void FxFaceGraphWidget::OnDeleteNode( wxCommandEvent& FxUnused(event) )
{
	if( _pGraph && !_userDataDestroyed )
	{
		FxArray<wxString> toRemove;

		FxFaceGraph::Iterator curr = _pGraph->Begin(FxFaceGraphNode::StaticClass());
		FxFaceGraph::Iterator end  = _pGraph->End(FxFaceGraphNode::StaticClass());
		for( ; curr != end; ++curr )
		{
			if( curr.GetNode() && IsNodeSelected( curr.GetNode() ) )
			{
				toRemove.PushBack( wxString::FromAscii(curr.GetNode()->GetNameAsCstr()) );
			}
		}

		if( toRemove.Length() > 1 )
		{
			FxVM::ExecuteCommand("batch");
			for( FxSize i = 0; i < toRemove.Length(); ++i )
			{
				wxString command = wxString::Format(wxT("graph -removenode -name \"%s\""),
					toRemove[i] );
				FxVM::ExecuteCommand( FxString(command.mb_str(wxConvLibc)) );
			}
			FxVM::ExecuteCommand("execBatch -removednodes");
		}
		else if( toRemove.Length() == 1 )
		{
			wxString command = wxString::Format(wxT("graph -removenode -name \"%s\""),
				toRemove.Back() );
			FxVM::ExecuteCommand( FxString(command.mb_str(wxConvLibc)) );
		}
	}
}

// Deletes the current link
void FxFaceGraphWidget::OnDeleteLink( wxCommandEvent& FxUnused(event) )
{
	if( _currentLinkEnd && _pGraph  && !_userDataDestroyed )
	{
		// Figure out the names of the two nodes to unlink.
		FxName thisNodeName = _currentLinkEnd->dest->otherNode;
		FxName otherNodeName = _currentLinkEnd->otherNode;

		FxFaceGraphNode* inputNode = NULL;
		FxFaceGraphNode* outputNode = NULL;
		_pGraph->DetermineLinkDirection( thisNodeName, otherNodeName, inputNode, outputNode );
		if( inputNode && outputNode )
		{
			wxString command = wxString::Format(wxT("graph -unlink -from \"%s\" -to \"%s\""), 
				wxString(outputNode->GetNameAsCstr(), wxConvLibc), 
				wxString(inputNode->GetNameAsCstr(), wxConvLibc) );
			FxVM::ExecuteCommand( FxString(command.mb_str(wxConvLibc)) );
		}
	}
}

// Adds a node
void FxFaceGraphWidget::OnAddNode( wxCommandEvent& event )
{
	if( _pGraph && !_userDataDestroyed )
	{
		FxSize placeableNodeTypeIndex = event.GetId() - ADDNODEMENU_MODIFIER;
		FxAssert( placeableNodeTypeIndex < _placeableNodeTypes.Length() );
		FxObject* pObject = _placeableNodeTypes[placeableNodeTypeIndex]->ConstructObject();
		FxFaceGraphNode* nodeToAdd = FxCast<FxFaceGraphNode>(pObject);
		if( !nodeToAdd )
		{
			delete pObject;
		}

		wxString nodeName;
		wxPoint nodePos = wxPoint((_viewRect.GetLeft() + _viewRect.GetRight()) * 0.5, 
			(_viewRect.GetTop() + _viewRect.GetBottom()) * 0.5);
		wxTextEntryDialog textDialog(this, _("Enter new node's name:"), _("Name New Node") );
		FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
		int retval = textDialog.ShowModal();
		FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
		nodeName = textDialog.GetValue();
		if( nodeName != wxEmptyString && retval == wxID_OK )
		{
			// Make sure the requested name doesn't already exist in the graph.
			if( _pGraph->FindNode(FxString(nodeName.mb_str(wxConvLibc)).GetData()) != NULL )
			{
				wxString msg = wxString::Format(wxT("A node named %s already exists."), nodeName);
				FxMessageBox(msg, _("Invalid Node Name"), wxOK|wxCENTRE|wxICON_EXCLAMATION, this);
			}
			else
			{
				wxString command = wxString::Format( _("graph -addnode -nodetype \"%s\" -name \"%s\" -nodex %d -nodey %d"),
					wxString(nodeToAdd->GetClassDesc()->GetName().GetAsCstr(), wxConvLibc),
					nodeName, nodePos.x, nodePos.y );
				if( CE_Success == FxVM::ExecuteCommand( FxString(command.mb_str(wxConvLibc)) ) )
				{
					// If the node was added, set it as the sole selection.
					command = wxString::Format( _("select -type \"node\" -names \"%s\" -nozoom"),
						nodeName);
					FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
				}
			}
		}
		else if( retval == wxID_OK && nodeName == wxEmptyString )
		{
			FxMessageBox(_("Please enter a name for the new node."), _("Invalid Node Name"), wxOK|wxCENTRE|wxICON_EXCLAMATION, this);
		}
	}
}

// Toggles visualizes the values of the face graph nodes
void FxFaceGraphWidget::OnToggleValues( wxCommandEvent& FxUnused(event) )
{
	_drawValues = !_drawValues;
	Refresh( FALSE );
}

// Toggles drawing the links as curves
void FxFaceGraphWidget::OnToggleCurves( wxCommandEvent& FxUnused(event) )
{
	_drawCurves = !_drawCurves;
	Refresh( FALSE );
}

// Toggles animating the transitions between selected dag nodes
void FxFaceGraphWidget::OnToggleTransitions( wxCommandEvent& FxUnused(event) )
{
	_animateTransitions = !_animateTransitions;
}

void FxFaceGraphWidget::OnToggleAutopan( wxCommandEvent& FxUnused(event) )
{
	_zoomToNode = !_zoomToNode;
}

// Toggles whether or not to draw links only between selected nodes.
void FxFaceGraphWidget::OnToggleLinks( wxCommandEvent& FxUnused(event) )
{
	_drawSelectedNodeLinksOnly = !_drawSelectedNodeLinksOnly;
	Refresh(FALSE);
}

// Re-layout the graph.
void FxFaceGraphWidget::OnRelayout( wxCommandEvent& FxUnused(event) )
{
	FxVM::ExecuteCommand("graph -layout");
}

// Display the options dialog.
void FxFaceGraphWidget::OnOptions( wxCommandEvent& FxUnused(event) )
{
	FxFaceGraphWidgetOptionsDlg optionsDialog(this);
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	if( optionsDialog.ShowModal() == wxID_OK )
	{
	}
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
}

void FxFaceGraphWidget::OnCreateSpeechGesturesFragment( wxCommandEvent& FxUnused(event) )
{
	FxVM::ExecuteCommand("batch");
	FxVM::ExecuteCommand("graph -addnode -nodetype \"FxCombinerNode\" -name \"Orientation Head Pitch\" -min -10 -max 10;");
	FxVM::ExecuteCommand("graph -addnode -nodetype \"FxCombinerNode\" -name \"Orientation Head Roll\" -min -10 -max 10;");
	FxVM::ExecuteCommand("graph -addnode -nodetype \"FxCombinerNode\" -name \"Orientation Head Yaw\" -min -10 -max 10;");
	FxVM::ExecuteCommand("graph -addnode -nodetype \"FxCombinerNode\" -name \"Emphasis Head Pitch\" -min -10 -max 10;");
	FxVM::ExecuteCommand("graph -addnode -nodetype \"FxCombinerNode\" -name \"Emphasis Head Roll\" -min -10 -max 10;");
	FxVM::ExecuteCommand("graph -addnode -nodetype \"FxCombinerNode\" -name \"Emphasis Head Yaw\" -min -10 -max 10;");
	FxVM::ExecuteCommand("graph -addnode -nodetype \"FxCombinerNode\" -name \"Gaze Eye Pitch\" -min -10 -max 10;");
	FxVM::ExecuteCommand("graph -addnode -nodetype \"FxCombinerNode\" -name \"Gaze Eye Yaw\" -min -10 -max 10;");
	FxVM::ExecuteCommand("graph -addnode -nodetype \"FxCombinerNode\" -name \"Eyebrow Raise\";");
	FxVM::ExecuteCommand("graph -addnode -nodetype \"FxCombinerNode\" -name \"Blink\";");
	FxVM::ExecuteCommand("graph -addnode -nodetype \"FxCombinerNode\" -name \"Head Pitch\" -min -10 -max 10;");
	FxVM::ExecuteCommand("graph -addnode -nodetype \"FxCombinerNode\" -name \"Head Roll\" -min -10 -max 10;");
	FxVM::ExecuteCommand("graph -addnode -nodetype \"FxCombinerNode\" -name \"Head Yaw\" -min -10 -max 10;");
	FxVM::ExecuteCommand("graph -addnode -nodetype \"FxCombinerNode\" -name \"Eye Pitch\" -min -10 -max 10;");
	FxVM::ExecuteCommand("graph -addnode -nodetype \"FxCombinerNode\" -name \"Eye Yaw\" -min -10 -max 10;");
	FxVM::ExecuteCommand("graph -link -from \"Orientation Head Pitch\" -to \"Head Pitch\" -linkfn \"linear\";");
	FxVM::ExecuteCommand("graph -link -from \"Emphasis Head Pitch\" -to \"Head Pitch\" -linkfn \"linear\";");
	FxVM::ExecuteCommand("graph -link -from \"Orientation Head Roll\" -to \"Head Roll\" -linkfn \"linear\";");
	FxVM::ExecuteCommand("graph -link -from \"Emphasis Head Roll\" -to \"Head Roll\" -linkfn \"linear\";");
	FxVM::ExecuteCommand("graph -link -from \"Orientation Head Yaw\" -to \"Head Yaw\" -linkfn \"linear\";");
	FxVM::ExecuteCommand("graph -link -from \"Emphasis Head Yaw\" -to \"Head Yaw\" -linkfn \"linear\";");
	FxVM::ExecuteCommand("graph -link -from \"Gaze Eye Pitch\" -to \"Eye Pitch\" -linkfn \"linear\";");
	FxVM::ExecuteCommand("graph -link -from \"Gaze Eye Yaw\" -to \"Eye Yaw\" -linkfn \"linear\";");
	FxVM::ExecuteCommand("graph -link -from \"Head Yaw\" -to \"Eye Yaw\" -linkfn \"negate\";");
	FxVM::ExecuteCommand("graph -link -from \"Head Pitch\" -to \"Eye Pitch\" -linkfn \"negate\";");
	FxVM::ExecuteCommand("execBatch -editednodes");
}

// Receives timer messages.
void FxFaceGraphWidget::OnTimer( wxTimerEvent& event )
{
	// If we're doing an action which could potentially cause the window
	// to pan, we want to keep it moving while the mouse is in the warning
	// track.  Thus, we'll just dispatch a mouse move message every timer period
	// while the action is active.
	static FxInt32 lastTime = 0;
	FxInt32 thisTime = _stopwatch.Time();
	switch( event.GetId() )
	{
	case timerID_PAN:
		{
			if( _mouseAction == MOUSE_STRETCHLINK || 
				_mouseAction == MOUSE_MOVENODE ||
				_mouseAction == MOUSE_BOXSELECT )
			{
				wxMouseEvent motion( wxEVT_MOTION );
				motion.m_x = _lastPos.x;
				motion.m_y = _lastPos.y;
				wxPostEvent( this, motion );
			}
			break;
		}
	case timerID_AUTOPAN:
		{
			wxRect clientRect = GetClientRect();
			_zoomLevel = _zoomCurve.EvaluateAt(_t);
			_invZoomLevel = 1.0f / _zoomLevel;

			wxSize viewSize(clientRect.GetWidth() * _zoomLevel, clientRect.GetHeight() * _zoomLevel);
			_viewRect.SetSize( viewSize );
			_viewRect.SetPosition( wxPoint(_xCurve.EvaluateAt(_t) - (0.5f * viewSize.GetWidth()),
				_yCurve.EvaluateAt(_t) - (0.5f * viewSize.GetHeight())) );

			_t = static_cast<FxReal>(lastTime * 0.001f) * 0.33333f; // panning takes 3 seconds
			if( _t > 1.0f )
			{
				_autopanTimer.Stop();
				_stopwatch.Pause();
				lastTime = 0;
			}
			else
			{
				lastTime = thisTime;
			}

			Refresh( FALSE );
			break;
		}
	default:
		break;
	};
}

//------------------------------------------------------------------------------
// Drawing functions
//------------------------------------------------------------------------------

// Draw a node on a DC
void FxFaceGraphWidget::DrawNode( wxDC* pDC, FxFaceGraphNode* node )
{
	if( node )
	{
		// Cull the node if it is off the screen
		wxRect nodePosition = WorldToClient( FxRect2wxRect(GetNodeUserData(node)->bounds) );
		if( nodePosition.Intersects( GetClientRect() ) )
		{
			const wxColour headerColour(110,110,110);
			const wxColour bodyColour(170,170,175);
			wxColour footerColour(headerColour);
			wxSize headerSize = WorldToClient( kNodeHeaderBox );
			wxSize footerSize = WorldToClient( kNodeFooterBox );
			wxRect nodeHeader = wxRect(nodePosition.GetPosition(), wxSize(nodePosition.GetWidth(), headerSize.GetHeight()));
			wxRect nodeBody   = wxRect(wxPoint(nodePosition.GetLeft(), nodeHeader.GetBottom()),
				wxSize(nodePosition.GetWidth(), 
				nodePosition.GetHeight() - headerSize.GetHeight() - footerSize.GetHeight()));
			wxRect nodeFooter = wxRect(wxPoint(nodePosition.GetLeft(), nodeBody.GetBottom()),
				wxSize(nodePosition.GetWidth(), footerSize.GetHeight()));

			wxRect iconRect = WorldToClient(wxRect(wxPoint(GetNodeUserData(node)->bounds.GetLeft() + 4, GetNodeUserData(node)->bounds.GetTop() + 6), 
											       wxSize(32,32)));
			wxRect textBounds = nodeHeader;
			textBounds.SetWidth( textBounds.GetWidth() - iconRect.GetWidth() - 1 );
			textBounds.SetLeft( iconRect.GetLeft() + iconRect.GetWidth() );

			wxPen    outlinePen( *wxBLACK, 1, wxSOLID );
			wxColour curveColour = FxColourMap::Get(node->GetName());
			if( curveColour != *wxBLACK )
			{
				footerColour = curveColour;
			}

			// Select the node fill colour if we're in value visualization mode.
			if( _drawValues )
			{
				FxCompiledFaceGraph* pCG = NULL;
				FxSessionProxy::GetCompiledFaceGraph(&pCG);
				FxAssert(pCG);

				FxSize nodeIndex = pCG->FindNodeIndex(node->GetNameAsCstr());
				FxAssert(FxInvalidIndex != nodeIndex);

				FxReal nodeValue = 0.0f;
				FxSessionProxy::GetRegister(node->GetName(), nodeValue);
				wxColour minColour  = FxStudioOptions::GetMinValueColour();
				wxColour zeroColour = FxStudioOptions::GetZeroValueColour();
				wxColour maxColour  = FxStudioOptions::GetMaxValueColour();
				wxColour colour(zeroColour);
				if( nodeValue > 0 )
				{
					FxReal val = nodeValue/pCG->nodes[nodeIndex].nodeMax;
					colour = wxColour((maxColour.Red()-zeroColour.Red())*val+zeroColour.Red(),
						(maxColour.Green()-zeroColour.Green())*val+zeroColour.Green(),
						(maxColour.Blue()-zeroColour.Blue())*val+zeroColour.Blue());
				}
				else if( nodeValue < 0 )
				{
					FxReal val = -nodeValue/-pCG->nodes[nodeIndex].nodeMin;
					colour = wxColour((minColour.Red()-zeroColour.Red())*val+zeroColour.Red(),
						(minColour.Green()-zeroColour.Green())*val+zeroColour.Green(),
						(minColour.Blue()-zeroColour.Blue())*val+zeroColour.Blue());
				}
				
				if( IsNodeSelected(node) )
				{
					pDC->SetPen( wxPen(wxColour(255, 240, 0), 1, wxSOLID) );
				}
				else
				{
					pDC->SetPen( wxPen(colour, 1, wxSOLID) );
				}
				pDC->SetBrush( wxBrush( colour, wxSOLID ) );
			}
			else
			{
				if( IsNodeSelected( node ) )
				{
					pDC->SetPen( wxPen(wxColour(255, 240, 0), 1, wxSOLID) );
				}
				else
				{
					pDC->SetPen( outlinePen );
				}
				pDC->SetBrush( wxBrush(bodyColour, wxSOLID) );
			}

			// Draw the node's body
			if( _flyoutEnabled && node == _currentNode && !_drawValues )
			{
				pDC->DrawBitmap( _flyout, nodeBody.GetPosition() );
				pDC->SetBrush( *wxTRANSPARENT_BRUSH );
				pDC->DrawRectangle( nodeBody );
			}
			else
			{
				pDC->DrawRectangle(nodeBody);
			}

			// Draw the node's header.
			if( nodeHeader.GetHeight() > 0 )
			{
				if( IsNodeSelected( node ) )
				{
					pDC->SetPen( wxPen(wxColour(255, 240, 0), 1, wxSOLID) );
				}
				else
				{
					pDC->SetPen( *wxBLACK_PEN );
				}
				pDC->SetBrush(wxBrush(headerColour,wxSOLID));
				pDC->DrawRectangle(nodeHeader);
			}
			
			// Draw the node's footer
			if( nodeFooter.GetHeight() > 0 )
			{
				pDC->SetBrush(wxBrush(footerColour, wxSOLID));
				pDC->DrawRectangle(nodeFooter);
			}

			// Find the current node's icon and draw it.
			if( iconRect.GetWidth() >= 8 )
			{
				wxBitmap nodeIcon( wxNullBitmap );
				for( FxSize i = 0; i < _classIconList.Length(); ++i )
				{
					if( node->IsKindOf(_classIconList[i].pClass) )
					{
						nodeIcon = _classIconList[i].classIcon;
					}
				}
				if( nodeIcon.IsOk() )
				{
					wxImage iconImage = nodeIcon.ConvertToImage();
					iconImage.Rescale( iconRect.GetWidth(), iconRect.GetHeight() );
					nodeIcon = wxBitmap( iconImage );

					pDC->DrawBitmap( nodeIcon, iconRect.GetPosition(), true );
				}
			}

			// Draw the node's name.
			wxString nodeLabel = wxString::FromAscii( node->GetNameAsCstr() );
			FxReal fontSize = FxMin( (textBounds.GetWidth() * 1.3f) / nodeLabel.Length(), 32.0f );
			fontSize = FxMin( FxMin( (textBounds.GetHeight() * 0.40f), 32.0f ), fontSize );
			if( fontSize > 2.0f )
			{
				// Select the font
				wxFont font( fontSize, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE, _("Lucida Sans Unicode") );
				pDC->SetFont( font );
				FxInt32 width, height, decent;
				pDC->GetTextExtent(nodeLabel, &width, &height, &decent);
				wxPoint textPos( textBounds.GetLeft() + ((textBounds.GetRight() - textBounds.GetLeft())/2 - (width/2)),
					textBounds.GetTop() + ((textBounds.GetBottom() - textBounds.GetTop())/2 - ((decent+height)/2)) );
				pDC->SetTextForeground( *wxBLACK );
				pDC->DrawText( nodeLabel, wxPoint( textPos.x + 1, textPos.y + 1 ) );
				pDC->SetTextForeground( *wxWHITE );
				pDC->DrawText( nodeLabel, wxPoint( textPos.x, textPos.y ) );
			}
			if( _drawValues )
			{
				FxReal nodeValue = 0.0f;
				FxSessionProxy::GetRegister(node->GetName(), nodeValue);
				
				wxString nodeValueString = wxString::Format(wxT("%.2f"), nodeValue);
				fontSize = FxMin( (nodeBody.GetWidth() * 1.3f) / nodeValueString.Length(), 120.0f );
				fontSize = FxMin( FxMin( (nodeBody.GetHeight() * 0.40f), 120.0f ), fontSize );
				if( fontSize > 2.0f )
				{
					wxFont font( fontSize, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE, _("Lucida Sans Unicode") );
					pDC->SetFont( font );
					pDC->SetTextForeground(FxStudioOptions::GetOverlayTextColour());
					FxInt32 width, height;
					pDC->GetTextExtent(nodeValueString, &width, &height);
					wxPoint textPos( nodeBody.GetLeft() + ((nodeBody.GetRight() - nodeBody.GetLeft())/2 - (width/2)),
									 nodeBody.GetTop() + ((nodeBody.GetBottom() - nodeBody.GetTop())/2 - (height/2)) );
					pDC->DrawText(nodeValueString, textPos);
				}
			}
		}
	}
}

// Draw a link on a DC
void FxFaceGraphWidget::DrawLink( wxDC* pDC, wxPoint fromPoint, wxPoint toPoint, 
								   FxBool isSelected, FxBool isCorrective, wxColour linkColour,
								   FxBool touchesSelected )
{
	wxPoint p1( toPoint );
	wxPoint p2( toPoint );
	wxPoint mp2;
	wxPoint mp1;

	FxInt32 style = isCorrective ? wxDOT : wxSOLID;
	wxColour colour = isSelected ? wxColour(255, 240, 0) : linkColour;
	pDC->SetPen( wxPen(colour, touchesSelected ? 3 : (isCorrective ? 2 : 1), style) );

	const FxReal linearThreshold = 30.0f;
	if( _drawCurves && _zoomLevel < linearThreshold)
	{
		FxInt32 numXPixels = FxAbs(fromPoint.x - toPoint.x);

		p2.x -= 0.45 * numXPixels;
		p2.y += 0.2 * (fromPoint.y - toPoint.y );
		wxPoint p3( fromPoint );
		p3.x += 0.45 * numXPixels;
		p3.y -= 0.2 * (fromPoint.y - toPoint.y );
		wxPoint p4( fromPoint );

		static const FxInt32 resolution = 30;
		wxPoint pointArray[resolution];
		for( FxInt32 time = 0; time < resolution; ++time )
		{
			// Bézier interpolation
			FxReal t = static_cast<FxReal>(time) / static_cast<FxReal>(resolution-1);

			FxReal invt = 1 - t;
			FxReal invt3 = invt * invt * invt;
			FxReal t3 = t * t * t;

			FxInt32 x  = FxNearestInteger(invt3*p1.x + 3*t*invt*invt*p2.x + 3*t*t*invt*p3.x + t3*p4.x);
			FxInt32 y = FxNearestInteger(invt3*p1.y + 3*t*invt*invt*p2.y + 3*t*t*invt*p3.y + t3*p4.y);
			pointArray[time] = wxPoint( x, y );
		}

		// Draw the curve
		pDC->DrawLines( resolution, pointArray );
	}
	else if( _drawCurves && _zoomLevel >= linearThreshold )
	{
		p2 = fromPoint;
		pDC->DrawLine( p1, p2 );
	}
	else
	{
		p2 = fromPoint;
		if( p2.x < p1.x )
		{
			FxInt32 midX = (p1.x + p2.x) / 2;
			mp1 = wxPoint(midX, p1.y);
			mp2 = wxPoint(midX, p2.y);
			pDC->DrawLine( p1,  mp1 );
			pDC->DrawLine( mp1, mp2 );
			pDC->DrawLine( mp2, p2  );
		}
		else
		{
			p2 = fromPoint;
			FxInt32 midY = (p1.y + p2.y) / 2;
			FxInt32 midX1 = p1.x - (p2.x - p1.x) * 0.12;
			FxInt32 midX2 = p2.x + (p2.x - p1.x) * 0.12;
			mp1 = wxPoint(midX1, midY);
			mp2 = wxPoint(midX2, midY);
			wxPoint mmp1(midX1, p1.y);
			wxPoint mmp2(midX2, p2.y);
			pDC->DrawLine( p1,   mmp1 );
			pDC->DrawLine( mmp1, mp1  );
			pDC->DrawLine( mp1,  mp2  );
			pDC->DrawLine( mp2,  mmp2 );
			pDC->DrawLine( mmp2, p2   );
			mp1 = mmp1;
		}
	}

	// Arrowhead drawing
	wxPoint front = p1;
	wxPoint back  = _drawCurves ? p2 : mp1;
	if( back == front ) // prevent some strange polygon artifacts if p1 == p2
	{
		back.x += 10;
	}

	// Calculate the direction vector and the vector perpendicular to the 
	// direction vector/
	FxVec2 perpvec = FxVec2( -(back - front).y, (back - front).x );
	FxVec2 dirvec = FxVec2( back.x - front.x, back.y - front.y );
	perpvec.Normalize();
	dirvec.Normalize();
	// Scale the vectors according to the zoom level
	perpvec *= 10.0f * _invZoomLevel;
	dirvec  *= 30.0f * _invZoomLevel;
	wxPoint perp( perpvec.x, perpvec.y );
	wxPoint mid( front.x + dirvec.x, front.y + dirvec.y );
	wxPoint arrowhead[3];
	arrowhead[0] = front;
	arrowhead[1] = mid + perp;
	arrowhead[2] = mid - perp;

	if( isSelected )
	{
		pDC->SetPen( wxPen( wxColour( 255, 240, 0 ), 1, wxSOLID ) );
		pDC->SetBrush( wxBrush( wxColour( 255, 240, 0 ), wxSOLID ) );
	}
	else
	{
		pDC->SetPen( wxPen(colour, 1, wxSOLID) );
		pDC->SetBrush( wxBrush(colour, wxSOLID) );
	}

	pDC->DrawPolygon( 3, arrowhead, 0, 0, wxWINDING_RULE );
}

// Draw a node's link end boxes
void FxFaceGraphWidget::DrawLinkEnds( wxDC* pDC, FxFaceGraphNode* node )
{
	if( node )
	{
		// Make sure they'll be seen before we bother drawing them.
		wxRect largestBounds = WorldToClient( FxRect2wxRect(GetNodeUserData(node)->bounds) );
		wxSize currLinkEndBoxSize = WorldToClient( kLinkEndBoxSize );
		largestBounds.SetWidth( largestBounds.GetWidth() + 2 * currLinkEndBoxSize.GetWidth() );
		largestBounds.SetLeft( largestBounds.GetLeft() - currLinkEndBoxSize.GetWidth() );
		if( largestBounds.Intersects( GetClientRect() ) )
		{
			// Draw the input boxes
			FxArray<FxLinkEnd*>& inputs = GetNodeUserData(node)->inputLinkEnds;
			FxArray<FxLinkEnd*>& outputs = GetNodeUserData(node)->outputLinkEnds;

			wxBrush unselectedBrush( wxBrush(wxColour(100,100,100), wxSOLID) );
			wxBrush selectedBrush( wxColour( 255, 240, 0 ), wxSOLID );

			pDC->SetPen(*wxBLACK_PEN);

			FxSize input;
			for( input = 0; input < inputs.Length(); ++input )
			{
				if( inputs[input]->selected || _linkEnd == inputs[input] )
				{
					pDC->SetBrush( selectedBrush );
				}
				else
				{
					pDC->SetBrush( unselectedBrush );
				}
				pDC->DrawRectangle( WorldToClient(FxRect2wxRect(inputs[input]->bounds)) );
			}

			// Draw the output boxes
			FxSize output;
			for( output = 0; output < outputs.Length(); ++output )
			{
				if( outputs[output]->selected || _linkEnd == outputs[output] )
				{
					pDC->SetBrush( selectedBrush );
				}
				else
				{
					pDC->SetBrush( unselectedBrush );
				}
				pDC->DrawRectangle( WorldToClient( FxRect2wxRect(outputs[output]->bounds) ) );
			}
		}
	}
}

//------------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------------
// One-time initialization of the control
void FxFaceGraphWidget::Initialize()
{
	SetBackgroundColour( wxColour( 192, 192, 192 ) );
	_bmpBackground = wxBitmap( FxStudioApp::GetBitmapPath(wxT("FaceGraph_Background.bmp")), wxBITMAP_TYPE_BMP );

	// Set the corrective link fn to red at initialization time.
	FxColourMap::Set( "corrective", *wxRED );

	wxFileName iconPath = FxStudioApp::GetAppDirectory();
	iconPath.AppendDir(wxT("res"));
	iconPath.AppendDir(wxT("icons"));
	for( FxSize i = 0; i < FxClass::GetNumClasses(); ++i )
	{		
		const FxClass* pClass = FxClass::GetClass(i);
		if( pClass )
		{
			wxString filename( pClass->GetName().GetAsCstr(), wxConvLibc );
			filename += wxT(".bmp");
			iconPath.SetName(filename);
			wxString path = iconPath.GetFullPath();
			if( wxFile::Exists( path ) )
			{
				ClassIconInfo iconInfo;
				iconInfo.pClass = pClass;
				iconInfo.classIcon = wxBitmap( path, wxBITMAP_TYPE_BMP );
				_classIconList.PushBack( iconInfo );
			}
		}
	}
}

// Gets the bounds of a link end box in world space
FxRect FxFaceGraphWidget::GetLinkEndBox( FxFaceGraphNode* node, FxBool isInput, FxSize index )
{
	FxRect retval( FxPoint(0,0), FxSize2D(kLinkEndBoxSize.x, kLinkEndBoxSize.y) );
	FxSize2D nodeSize = GetNodeUserData(node)->bounds.GetSize();
	retval.Offset( GetNodeUserData(node)->bounds.GetPosition() );
	retval.SetTop( retval.GetTop() + kNodeHeaderBox.GetHeight() + (index * kLinkEndBoxSize.GetHeight() + (index + 1) * kLinkEndBoxSpacer.GetHeight()) );
	if( !isInput )
	{
		retval.SetLeft( retval.GetLeft() + nodeSize.GetWidth() - 1);
	}
	else
	{
		retval.SetLeft( retval.GetLeft() - kLinkEndBoxSize.GetWidth() + 1);
	}
	return retval;
}

// Client to world point transformation
wxPoint FxFaceGraphWidget::ClientToWorld( const wxPoint& clientPoint )
{
	wxPoint retval( clientPoint );
	retval.x *= _zoomLevel;
	retval.y *= _zoomLevel;
	retval.x += _viewRect.GetPosition().x;
	retval.y += _viewRect.GetPosition().y;
	return retval;
}

// Client to world rect transformation
wxRect FxFaceGraphWidget::ClientToWorld( const wxRect& clientRect )
{
	return wxRect( ClientToWorld( wxPoint( clientRect.GetLeft(),  clientRect.GetTop() ) ),
				   ClientToWorld( wxPoint( clientRect.GetRight(), clientRect.GetBottom() ) ) );
}

// World to client point transformation
wxPoint FxFaceGraphWidget::WorldToClient( const wxPoint& worldPoint )
{
	wxPoint retval( worldPoint - _viewRect.GetPosition() );
	retval.x *= _invZoomLevel;
	retval.y *= _invZoomLevel;
	return retval;
}

// World to client rect transformation
wxRect FxFaceGraphWidget::WorldToClient( const wxRect& worldRect )
{
	return wxRect( WorldToClient( wxPoint( worldRect.GetLeft(),  worldRect.GetTop() ) ),
				   WorldToClient( wxPoint( worldRect.GetRight(), worldRect.GetBottom() ) ) );
}

// World to client size transformation
wxSize FxFaceGraphWidget::WorldToClient( const wxSize& worldSize )
{
	return wxSize( worldSize.GetWidth() * _invZoomLevel, 
				   worldSize.GetHeight() * _invZoomLevel );
}

void FxFaceGraphWidget::Zoom( const wxPoint& zoomPoint, FxReal zoomFactor, 
							   ZoomDirection zoomDir )
{
	if( zoomDir != ZOOM_NONE )
	{
		FxReal lastZoomLevel = _zoomLevel;
		wxRect client = GetClientRect();
		FxReal pctAcrossX = static_cast<FxReal>(zoomPoint.x - client.GetPosition().x) / static_cast<FxReal>(client.GetWidth());
		FxReal pctAcrossY = static_cast<FxReal>(zoomPoint.y - client.GetPosition().y) / static_cast<FxReal>(client.GetHeight());

		wxPoint zoomWorldPoint = ClientToWorld( zoomPoint );

		if( zoomDir == ZOOM_IN )
		{
			_zoomLevel *= zoomFactor;
		}
		else
		{
			_zoomLevel /= zoomFactor;
		}
		_zoomLevel = FxClamp( 0.5f, _zoomLevel, 50.0f );
		_invZoomLevel = 1.0f / _zoomLevel;

		if( lastZoomLevel != _zoomLevel )
		{
			wxSize viewSize(0, 0);
			viewSize.SetWidth( client.GetWidth() * _zoomLevel );
			viewSize.SetHeight( client.GetHeight() * _zoomLevel );
			_viewRect.SetSize( viewSize );
			_viewRect.SetPosition( wxPoint( zoomWorldPoint.x - (pctAcrossX * viewSize.GetWidth() ),
											zoomWorldPoint.y - (pctAcrossY * viewSize.GetHeight() ) ) );
		}
	}
}

wxPoint FxFaceGraphWidget::Pan( const wxPoint& currentMousePos, const wxRect& clientRect )
{
	wxPoint retval( kOrigin );
	static const FxReal warningTrack = 5.0f;
	const FxReal fullSpeed = _zoomLevel * 3.33f;

	// Ensure the cursor stays in the window during pan movements
	FxInt32 mouseX = currentMousePos.x;
	FxInt32 mouseY = currentMousePos.y;
	if( !clientRect.Contains( currentMousePos ) )
	{
		if( currentMousePos.x <= clientRect.GetLeft() )
		{
			mouseX = clientRect.GetLeft();
		}
		else if( currentMousePos.x >= clientRect.GetRight() )
		{
			mouseX = clientRect.GetRight();
		}
		if( currentMousePos.y <= clientRect.GetTop() )
		{
			mouseY = clientRect.GetTop();
		}
		else if( currentMousePos.y >= clientRect.GetBottom() )
		{
			mouseY = clientRect.GetBottom();
		}
		WarpPointer( mouseX, mouseY );
	}

	if( mouseX <= (clientRect.GetLeft() + warningTrack) )
	{
		FxReal percent = 1.0f - ((mouseX - clientRect.GetLeft()) / warningTrack);
		retval.x = -(fullSpeed * percent);
	}
	else if( mouseX >= (clientRect.GetRight() - warningTrack) )
	{
		FxReal percent = 1.0f - ((clientRect.GetRight() - mouseX) / warningTrack);
		retval.x = fullSpeed * percent;
	}
	if( mouseY <= (clientRect.GetTop() + warningTrack) )
	{
		FxReal percent = 1.0f - ((mouseY - clientRect.GetTop()) / warningTrack);
		retval.y = -(fullSpeed * percent);
	}
	else if( mouseY >= (clientRect.GetBottom() - warningTrack) )
	{
		FxReal percent = 1.0f - ((clientRect.GetBottom() - mouseY) / warningTrack);
		retval.y = fullSpeed * percent;
	}

	_viewRect.Offset( retval );
	return retval;
}

// Removes a node from the faceGraph, by name
void FxFaceGraphWidget::RemoveNode( const FxName& name )
{
	if( _pGraph )
	{
		RemoveNode( _pGraph->FindNode( name ) );
	}
}

// Removes a node from the faceGraph
void FxFaceGraphWidget::RemoveNode( FxFaceGraphNode* node )
{
	if( node )
	{
		RemoveUserData(node);
		_pGraph->RemoveNode( node->GetName() );

		RebuildLinks();
		ProofAllLinkEnds();
		RecalculateAllLinkEnds();
	}
}

// Recalculates a node's size
void FxFaceGraphWidget::RecalculateNodeSize( FxFaceGraphNode* node )
{
	if( node )
	{
		FxSize2D nodeSize( kNodeBox.x, kNodeBox.y );
		FxSize numLinks = FxMax( GetNodeUserData(node)->inputLinkEnds.Length(), 
								GetNodeUserData(node)->outputLinkEnds.Length() );
		nodeSize.x = node->GetNameAsString().Length() * 11 + 60;
		nodeSize.y = kNodeHeaderBox.GetHeight() + numLinks * kLinkEndBoxSize.GetHeight() + (numLinks + 1) * kLinkEndBoxSpacer.GetHeight() + kNodeFooterBox.GetHeight();
		FxRect currBounds( GetNodeUserData(node)->bounds );
		currBounds.SetSize( nodeSize );
		GetNodeUserData(node)->bounds = currBounds;
	}
}

// Recalculates the node sizes
void FxFaceGraphWidget::RecalculateNodeSizes()
{
	if( _pGraph )
	{
		FxFaceGraph::Iterator curr = _pGraph->Begin(FxFaceGraphNode::StaticClass());
		FxFaceGraph::Iterator end  = _pGraph->End(FxFaceGraphNode::StaticClass());
		for( ; curr != end; ++curr )
		{
			RecalculateNodeSize( curr.GetNode() );
		}
	}
}

// Recalculates the node's link end bounding boxes
void FxFaceGraphWidget::RecalculateLinkEnds( FxFaceGraphNode* node )
{
	if( node )
	{
		FxArray<FxLinkEnd*>& inputs = GetNodeUserData(node)->inputLinkEnds;
		for( FxSize i = 0; i < inputs.Length(); ++i )
		{
			inputs[i]->bounds = GetLinkEndBox( node, TRUE, i );
		}

		FxArray<FxLinkEnd*>& outputs = GetNodeUserData(node)->outputLinkEnds;
		for( FxSize i = 0; i < outputs.Length(); ++i )
		{
			outputs[i]->bounds = GetLinkEndBox( node, FALSE, i );
		}
	}
}

// Recalculates all nodes' link end bounding boxes
void FxFaceGraphWidget::RecalculateAllLinkEnds()
{
	if( _pGraph )
	{
		FxFaceGraph::Iterator curr = _pGraph->Begin(FxFaceGraphNode::StaticClass());
		FxFaceGraph::Iterator end  = _pGraph->End(FxFaceGraphNode::StaticClass());
		for( ; curr != end; ++curr )
		{
			RecalculateLinkEnds( curr.GetNode() );
		}
	}
}

// Ensures that a node has all its necessary link end boxes
void FxFaceGraphWidget::ProofLinkEnds( FxFaceGraphNode* node )
{
	if( node )
	{
		FxArray<FxLinkEnd*>& inputs = GetNodeUserData(node)->inputLinkEnds;
		if( !inputs.Length() || inputs.Back()->dest )
		{
			inputs.PushBack( new FxLinkEnd( FxTrue ) );
		}

		FxArray<FxLinkEnd*>& outputs = GetNodeUserData(node)->outputLinkEnds;
		if( !outputs.Length() || outputs.Back()->dest )
		{
			outputs.PushBack( new FxLinkEnd( FxFalse ) );
		}
	}
}

// Ensures that all nodes have their necessary link end boxes and positions
// them correctly
void FxFaceGraphWidget::ProofAllLinkEnds()
{
	if( _pGraph )
	{
		FxFaceGraph::Iterator curr = _pGraph->Begin(FxFaceGraphNode::StaticClass());
		FxFaceGraph::Iterator end  = _pGraph->End(FxFaceGraphNode::StaticClass());
		for( ; curr != end; ++curr )
		{
			ProofLinkEnds( curr.GetNode() );
		}
	}
}

// Deletes the link ends and resets the arrays for a node
void FxFaceGraphWidget::ResetLinkEnds( FxFaceGraphNode* node )
{
	if( node )
	{
		FxArray<FxLinkEnd*>& inputLinks  = GetNodeUserData(node)->inputLinkEnds;
		for( FxSize i = 0; i < inputLinks.Length(); ++i )
		{
			delete inputLinks[i];
		}

		FxArray<FxLinkEnd*>& outputLinks = GetNodeUserData(node)->outputLinkEnds;
		for( FxSize i = 0; i < outputLinks.Length(); ++i )
		{
			delete outputLinks[i];
		}

		inputLinks.Clear();
		outputLinks.Clear();
	}
}

// Rebuilds the links
void FxFaceGraphWidget::RebuildLinks()
{
	if( _pGraph )
	{
		FxFaceGraph::Iterator curr = _pGraph->Begin(FxFaceGraphNode::StaticClass());
		FxFaceGraph::Iterator end  = _pGraph->End(FxFaceGraphNode::StaticClass());
		for( ; curr != end; ++curr )
		{
			ResetLinkEnds( curr.GetNode() );
		}

		FxLinkEnd* input;
		FxLinkEnd* output;

		curr = _pGraph->Begin(FxFaceGraphNode::StaticClass());
        for( ; curr != end; ++curr )
		{
			FxFaceGraphNode* currentNode = curr.GetNode();
			if( currentNode )
			{
				FxSize i;
				for( i = 0; i < currentNode->GetNumInputLinks(); ++i )
				{
					input  = new FxLinkEnd( FxTrue );
					output = new FxLinkEnd( FxFalse );

					input->dest = output;
					output->dest = input;
					input->otherNode = currentNode->GetInputLink( i ).GetNodeName();
					output->otherNode = currentNode->GetName();
					input->bounds  = GetLinkEndBox( currentNode, TRUE, i );
					output->bounds = GetLinkEndBox( const_cast<FxFaceGraphNode*>(currentNode->GetInputLink( i ).GetNode()), FALSE, 
						GetNodeUserData( const_cast<FxFaceGraphNode*>(currentNode->GetInputLink( i ).GetNode()) )->outputLinkEnds.Length() );

					GetNodeUserData(currentNode)->inputLinkEnds.PushBack( input );
					GetNodeUserData(const_cast<FxFaceGraphNode*>(currentNode->GetInputLink( i ).GetNode()))->outputLinkEnds.PushBack( output );
				}
			}
		}
	}
}

// Helper function to remove references to a specific node.
void RemoveReferencesTo( const FxName& node, FxArray<FxLinkEnd*> linkEnds )
{
	for( FxSize i = 0; i < linkEnds.Length(); ++i )
	{
		if( node == linkEnds[i]->otherNode )
		{
			delete linkEnds[i];
			linkEnds.Remove(i--);
		}
	}
}

// Removes the user data from a single node
void FxFaceGraphWidget::RemoveUserData( FxFaceGraphNode* node )
{
	if( node )
	{
		// Remove all references to this node.
		FxFaceGraph::Iterator iter = _pGraph->Begin(FxFaceGraphNode::StaticClass());
		for( ; iter != _pGraph->End(FxFaceGraphNode::StaticClass()); ++iter )
		{
			RemoveReferencesTo(node->GetName(), GetNodeUserData(node)->inputLinkEnds);
			RemoveReferencesTo(node->GetName(), GetNodeUserData(node)->outputLinkEnds);
		}

		ResetLinkEnds( node );
		delete GetNodeUserData(node);
		node->SetUserData( NULL );
	}
}

// Arranges the nodes using the Graphviz dot algorithm.
void FxFaceGraphWidget::DoDefaultLayout()
{
	if( _pGraph )
	{
		FxProgressDialog progressDialog( _("Calculating Face Graph layout..."), this );
		SetCursor( wxCursor(wxCURSOR_WAIT) );

		wxDynamicLibrary layoutLibrary(wxT("FxGraphLayout"));
		if( layoutLibrary.IsLoaded() )
		{
			typedef void* (*Layout_t)( FxFaceGraph* );
			wxDYNLIB_FUNCTION(Layout_t, DoLayout, layoutLibrary);
			if( pfnDoLayout )
			{
				(*pfnDoLayout)(_pGraph);
			}
		}

		progressDialog.Update( 1.0f );

		RecalculateAllLinkEnds();
		Refresh( FALSE );
		SetCursor( wxNullCursor );
	}
}

// Does the hit testing
FxFaceGraphWidget::HitTestResult FxFaceGraphWidget::PerformHitTest( const wxPoint& position )
{
	if( _pGraph )
	{
		// Used in determining whether or not to refresh the widget
		static HitTestResult lastResult = HIT_NOTHING;
		FxFaceGraphNode* temp = _currentNode;

		// Set up the 
		HitTestResult retval = HIT_NOTHING;
		_currentNode = NULL;

		wxSize currLinkEndBoxSize = WorldToClient( kLinkEndBoxSize );

		FxFaceGraph::Iterator curr = _pGraph->Begin(FxFaceGraphNode::StaticClass());
		FxFaceGraph::Iterator end  = _pGraph->End(FxFaceGraphNode::StaticClass());
		for( ; curr != end; ++curr )
		{
			FxFaceGraphNode* currNode = curr.GetNode();
			if( currNode )
			{
				wxRect largestBounds = WorldToClient( FxRect2wxRect(GetNodeUserData(currNode)->bounds) );
				largestBounds.SetWidth( largestBounds.GetWidth() + 2 * currLinkEndBoxSize.GetWidth() );
				largestBounds.SetLeft( largestBounds.GetLeft() - currLinkEndBoxSize.GetWidth() );
				// The first hit test checks whether we're within of the bounding
				// rectangle of the node, its input link ends and its output link ends.
				if( largestBounds.Contains( position ) )
				{
					// Now let's check the actual bounds of the node
					wxRect nodeBounds = WorldToClient(FxRect2wxRect(GetNodeUserData(currNode)->bounds));
					if( nodeBounds.Contains( position ) )
					{
						_currentNode = currNode;
						retval = HIT_NODE;
					}
					else
					{
						// If we're here, we're either over the input or output link
						// end areas - narrow it down further
						wxRect inputNodes( wxPoint( nodeBounds.GetLeft() - currLinkEndBoxSize.GetWidth(),
										nodeBounds.GetTop() ),
										wxSize(  currLinkEndBoxSize.GetWidth(), 
										nodeBounds.GetHeight() ) );

						wxRect outputNodes = inputNodes;
						outputNodes.Offset( currLinkEndBoxSize.GetWidth() + nodeBounds.GetWidth(), 0 );
						if( outputNodes.Contains( position ) )
						{
							// We're on the *output* link end area - now we'll do 
							// the hit tests on the actual link ends
							FxArray<FxLinkEnd*>& linkEnds = GetNodeUserData(currNode)->outputLinkEnds;
							for( FxSize j = 0; j < linkEnds.Length(); ++j )
							{
								linkEnds[j]->selected = FxFalse;
								wxRect linkRect = WorldToClient( FxRect2wxRect(linkEnds[j]->bounds) );
								if( linkRect.Contains( position ) )
								{
									_currentNode = currNode;
									_currentLinkEnd = linkEnds[j];
									linkEnds[j]->selected = FxTrue;
									retval = (j == linkEnds.Length() - 1) ? HIT_EMPTYLINKEND : HIT_LINKEND;
								}
							}
						}
						else if( inputNodes.Contains( position ) )
						{
							// We're on the *input*link end area - now we'll do the 
							// hit tests on the actual link ends
							FxArray<FxLinkEnd*>& linkEnds = GetNodeUserData(currNode)->inputLinkEnds;
							for( FxSize j = 0; j < linkEnds.Length(); ++j )
							{
								linkEnds[j]->selected = FxFalse;
								wxRect linkRect = WorldToClient( FxRect2wxRect(linkEnds[j]->bounds) );
								if( linkRect.Contains( position ) )
								{
									_currentNode = currNode;
									_currentLinkEnd = linkEnds[j];
									linkEnds[j]->selected = FxTrue;
									retval = (j == linkEnds.Length() - 1) ? HIT_EMPTYLINKEND : HIT_LINKEND;
								}
							}
						}
					}
				}
			}
		}

		if( temp && (temp != _currentNode || (lastResult != retval && retval != HIT_LINKEND) ) )
		{
			// here, we clear *all* selected flags from temp's end points
			FxArray<FxLinkEnd*>& inLinkEnds = GetNodeUserData(temp)->inputLinkEnds;
			for( FxSize i = 0; i < inLinkEnds.Length(); ++i )
			{
				inLinkEnds[i]->selected = FxFalse;
			}
			FxArray<FxLinkEnd*>& outLinkEnds = GetNodeUserData(temp)->outputLinkEnds;
			for( FxSize i = 0; i < outLinkEnds.Length(); ++i )
			{
				outLinkEnds[i]->selected = FxFalse;
			}
		}

		if( lastResult != retval || temp != _currentNode )
		{
			Refresh( FALSE );
		}

		lastResult = retval;
		return retval;
	}

	return HIT_NOTHING;
}

void FxFaceGraphWidget::ZoomToSelection()
{
	wxPoint boundsMin( FX_INT32_MAX, FX_INT32_MAX );
	wxPoint boundsMax( FX_INT32_MIN, FX_INT32_MIN );

	FxFaceGraph::Iterator curr = _pGraph->Begin(FxFaceGraphNode::StaticClass());
	FxFaceGraph::Iterator end  = _pGraph->End(FxFaceGraphNode::StaticClass());
	FxSize selNodeCount = 0;
	for( ; curr != end; ++curr )
	{
		if( curr.GetNode() && IsNodeSelected(curr.GetNode()) )
		{
			++selNodeCount;

			FxFaceGraphNode* node = curr.GetNode();
			const FxRect& nodeBounds = GetNodeUserData(node)->bounds;
			boundsMin.x = FxMin( nodeBounds.GetLeft(),   boundsMin.x );
			boundsMin.y = FxMin( nodeBounds.GetTop(),    boundsMin.y );
			boundsMax.x = FxMax( nodeBounds.GetRight(),  boundsMax.x );
			boundsMax.y = FxMax( nodeBounds.GetBottom(), boundsMax.y );
		}
	}

	if( selNodeCount > 0 )
	{
		wxPoint selectionCenter;
		selectionCenter.x = (boundsMax.x + boundsMin.x)/2;
		selectionCenter.y = (boundsMax.y + boundsMin.y)/2;

		wxRect selectionBounds( boundsMin, boundsMax );

		// Center on the selected node
		// Possibly smoothly animate the transition
		wxRect clientRect = GetClientRect();
		FxReal halfClientWidth  = clientRect.GetWidth() * 0.5f;
		FxReal halfClientHeight = clientRect.GetHeight() * 0.5f;

		if( _animateTransitions && IsShown() )
		{
			wxPoint currentCenter = ClientToWorld( wxPoint(clientRect.GetPosition().x + (clientRect.GetWidth()/2),
				clientRect.GetPosition().y + (clientRect.GetHeight()/2)) );

			// Clear the curves
			_xCurve.RemoveAllKeys();
			_yCurve.RemoveAllKeys();
			_zoomCurve.RemoveAllKeys();
			_t = 0.0f;

			_zoomCurve.InsertKey( FxAnimKey(0.0f, _zoomLevel, 0.0f, 0.0f ) );
			if( !(_viewRect.Contains( boundsMin ) && _viewRect.Contains( boundsMax )) )
			{
				// If the selection is not already entirely bounded by the current
				// view, calculate how far we need to zoom out to bring it into
				// view before starting the pan.
				FxInt32 dx = FxMax( FxAbs(boundsMax.x - currentCenter.x),
					FxAbs(boundsMin.x - currentCenter.x) );
				FxInt32 dy = FxMax( FxAbs(boundsMax.y - currentCenter.y),
					FxAbs(boundsMin.y - currentCenter.y) );
				FxReal targetZoom = FxMax( dx / halfClientWidth, dy / halfClientHeight );

				if( targetZoom > _zoomLevel )
				{
					// Pull back for the course of the movement
					_zoomCurve.InsertKey( FxAnimKey(0.2f, targetZoom, 0.0f, 0.0f) );
					_zoomCurve.InsertKey( FxAnimKey(0.8f, targetZoom, 0.0f, 0.0f) );
				}
			}

			// Now calculate the final zoom level.  If it's a single node, we'll 
			// assume we're going to zoom to the "normal" level
			if( selNodeCount == 1 )
			{
				_zoomCurve.InsertKey( FxAnimKey(1.0f, 1.0f, 0.0f, 0.0f) );
			}
			else
			{
				FxReal dx = (boundsMax.x - boundsMin.x) * 0.5f;
				FxReal dy = (boundsMax.y - boundsMin.y) * 0.5f;
				FxReal targetZoom = FxMax( dx / halfClientWidth, dy / halfClientHeight );
				_zoomCurve.InsertKey( FxAnimKey(1.0f, targetZoom+0.01f, 0.0f, 0.0f) );
			}

			FxReal startTime = 0.2f;
			FxReal endTime = 0.8f;
			if( _zoomCurve.GetNumKeys() == 2 && 
				FxRealEquality(_zoomCurve.GetKey(0).GetValue(),_zoomCurve.GetKey(1).GetValue()) )
			{
				startTime = 0.0f;
				endTime   = 1.0f;
			}
			_xCurve.InsertKey( FxAnimKey(startTime, currentCenter.x, 0.0f, 0.0f) );
			_xCurve.InsertKey( FxAnimKey(endTime, selectionCenter.x, 0.0f, 0.0f) );
			_yCurve.InsertKey( FxAnimKey(startTime, currentCenter.y, 0.0f, 0.0f) );
			_yCurve.InsertKey( FxAnimKey(endTime, selectionCenter.y, 0.0f, 0.0f) );

			if( !_autopanTimer.IsRunning() )
			{
				_stopwatch.Start();
				_autopanTimer.Start( 1, wxTIMER_CONTINUOUS );
			}
		}
		else
		{
			if( selNodeCount == 1 )
			{
				_zoomLevel = _invZoomLevel = 1.0f;
			}
			else
			{
				FxReal dx = (boundsMax.x - boundsMin.x) * 0.5f;
				FxReal dy = (boundsMax.y - boundsMin.y) * 0.5f;
				_zoomLevel = FxMax( dx / halfClientWidth, dy / halfClientHeight ) + 0.01f;
				_invZoomLevel = 1.0f / _zoomLevel;
			}

			wxSize viewSize(clientRect.GetWidth() * _zoomLevel, clientRect.GetHeight() * _zoomLevel);
			_viewRect.SetSize( viewSize );
			_viewRect.SetPosition( wxPoint(selectionCenter.x - (0.5f * viewSize.GetWidth()),
				selectionCenter.y - (0.5f * viewSize.GetHeight())) );
			Refresh( FALSE );
		}
	}
	else
	{
		Refresh(FALSE);
	}
}

//------------------------------------------------------------------------------
// User data access functions
//------------------------------------------------------------------------------
// Returns true if the node is selected
FxBool FxFaceGraphWidget::IsNodeSelected( FxFaceGraphNode* node )
{
	if( node )
	{
		return GetNodeUserData(node)->selected;
	}
	return FxFalse;
}

// Sets if the node is selected
void FxFaceGraphWidget::SetNodeSelected( FxFaceGraphNode* node, FxBool selected )
{
	if( node )
	{
		GetNodeUserData(node)->selected = selected;
	}
}

// Clears all selections
void FxFaceGraphWidget::ClearSelected()
{
	if( _pGraph )
	{
		FxFaceGraph::Iterator curr = _pGraph->Begin(FxFaceGraphNode::StaticClass());
		FxFaceGraph::Iterator end  = _pGraph->End(FxFaceGraphNode::StaticClass());
		for( ; curr != end; ++curr )
		{
			SetNodeSelected( curr.GetNode(), FxFalse );
		}

	}
	_inputNode  = FxName::NullName;
	_outputNode = FxName::NullName;
}

// Returns true if the link is selected
FxBool FxFaceGraphWidget::IsLinkSelected( const FxName& firstNode, const FxName& secondNode )
{
	return (firstNode == _inputNode && secondNode == _outputNode ) ||
		   (firstNode == _outputNode && secondNode == _inputNode );
}

// Sets the link between the two nodes as selected
void FxFaceGraphWidget::SetLinkSelected( const FxName& inputNode, const FxName& outputNode )
{
	_inputNode  = inputNode;
	_outputNode = outputNode;
}

// Returns true if the link is corrective
FxBool FxFaceGraphWidget::IsLinkCorrective( const FxName& firstNode, const FxName& secondNode )
{
	if( _pGraph )
	{
		FxFaceGraphNodeLink linkInfo;
		if( _pGraph->GetLink( firstNode, secondNode, linkInfo ) )
		{
			return linkInfo.IsCorrective();
		}
	}
	return FxFalse;
}

//------------------------------------------------------------------------------
// Selection-wide functions
//------------------------------------------------------------------------------
// Applies a selection rectangle to the nodes
void FxFaceGraphWidget::ApplySelectionRect( wxRect& sel, FxBool toggle )
{
	if( _pGraph )
	{
		// Align the selection rectangle
		FxInt32 minX = FxMin( sel.GetLeft(), sel.GetRight() );
		FxInt32 maxX = FxMax( sel.GetLeft(), sel.GetRight() );
		FxInt32 minY = FxMin( sel.GetTop(), sel.GetBottom() );
		FxInt32 maxY = FxMax( sel.GetTop(), sel.GetBottom() );

		sel.SetLeft( minX );
		sel.SetRight( maxX );
		sel.SetTop( minY );
		sel.SetBottom( maxY );

		wxString nodeList;
		FxFaceGraph::Iterator curr = _pGraph->Begin(FxFaceGraphNode::StaticClass());
		FxFaceGraph::Iterator end  = _pGraph->End(FxFaceGraphNode::StaticClass());
		for( ; curr != end; ++curr )
		{
			if( curr.GetNode() && 
				FxRect2wxRect(GetNodeUserData(curr.GetNode())->bounds).Intersects( sel ) )
			{
				if( toggle && !IsNodeSelected(curr.GetNode()) )
				{
					if( nodeList.Length() != 0 )
					{
						nodeList += wxT("|");
					}
					nodeList += wxString(curr.GetNode()->GetNameAsCstr(), wxConvLibc);
				}
				else if( !toggle )
				{
					if( nodeList.Length() != 0 )
					{
						nodeList += wxT("|");
					}
					nodeList += wxString(curr.GetNode()->GetNameAsCstr(), wxConvLibc);
				}
			}
			else if( curr.GetNode() && IsNodeSelected(curr.GetNode()) )
			{
				if( nodeList.Length() != 0 )
				{
					nodeList += wxT("|");
				}
				nodeList += wxString(curr.GetNode()->GetNameAsCstr(), wxConvLibc);
			}
		}
		if( nodeList != wxEmptyString )
		{
			wxString command = wxString::Format(wxT("select -type \"node\" -names \"%s\" -nozoom"), nodeList);
			FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
		}
		else if( _selectedNodes.Length() > 0 )
		{
			FxVM::ExecuteCommand("select -type \"node\" -clear -nozoom");
		}
	}
}

// Offsets the selected nodes by a delta
void FxFaceGraphWidget::MoveSelectedNodes( const wxPoint& delta )
{
	if( _pGraph )
	{
		FxFaceGraph::Iterator curr = _pGraph->Begin(FxFaceGraphNode::StaticClass());
		FxFaceGraph::Iterator end  = _pGraph->End(FxFaceGraphNode::StaticClass());
		for( ; curr != end; ++curr )
		{
			if( curr.GetNode() && IsNodeSelected( curr.GetNode() ) )
			{
				GetNodeUserData(curr.GetNode())->bounds.Offset( FxPoint(delta.x, delta.y) );
				RecalculateLinkEnds( curr.GetNode() );
			}
		}
	}
}

//------------------------------------------------------------------------------
// Face Graph Widget Options Dialog
//------------------------------------------------------------------------------

WX_IMPLEMENT_DYNAMIC_CLASS(FxFaceGraphWidgetOptionsDlg, wxDialog)
BEGIN_EVENT_TABLE(FxFaceGraphWidgetOptionsDlg, wxDialog)
	EVT_BUTTON( wxID_OK, FxFaceGraphWidgetOptionsDlg::OnOK )
	EVT_BUTTON( wxID_APPLY, FxFaceGraphWidgetOptionsDlg::OnApply )

	EVT_CHECKBOX(ControlID_FaceGraphWidgetOptionsDlgValueVisualizationCheck, FxFaceGraphWidgetOptionsDlg::OnCheck )
	EVT_CHECKBOX(ControlID_FaceGraphWidgetOptionsDlgBezierLinksCheck, FxFaceGraphWidgetOptionsDlg::OnCheck )
	EVT_CHECKBOX(ControlID_FaceGraphWidgetOptionsDlgAnimateTransitionsCheck, FxFaceGraphWidgetOptionsDlg::OnCheck )
	EVT_CHECKBOX(ControlID_FaceGraphWidgetOptionsDlgDrawOnlySelectedCheck, FxFaceGraphWidgetOptionsDlg::OnCheck )

	EVT_LISTBOX( ControlID_FaceGraphWidgetOptionsDlgColourListBox, FxFaceGraphWidgetOptionsDlg::OnColourListChange )
	EVT_BUTTON( ControlID_FaceGraphWidgetOptionsDlgColourChangeButton, FxFaceGraphWidgetOptionsDlg::OnColourChange )
END_EVENT_TABLE()
FxFaceGraphWidgetOptionsDlg::FxFaceGraphWidgetOptionsDlg()
	: _linkFnColours(NULL)
{
}

FxFaceGraphWidgetOptionsDlg::FxFaceGraphWidgetOptionsDlg( wxWindow* parent, 
					wxWindowID id, const wxString& caption,	const wxPoint& pos, 
					const wxSize& size, long style)
	: _linkFnColours(NULL)
{
	Create( parent, id, caption, pos, size, style );
}

// Destructor
FxFaceGraphWidgetOptionsDlg::~FxFaceGraphWidgetOptionsDlg()
{
	delete[] _linkFnColours;
}

// Creation.
bool FxFaceGraphWidgetOptionsDlg::Create( wxWindow* parent, wxWindowID id , 
					const wxString& caption, const wxPoint& pos, 
					const wxSize& size, long style )
{
	_facegraphWidget = static_cast<FxFaceGraphWidget*>(parent);

	SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
	SetExtraStyle(GetExtraStyle()|wxWS_EX_VALIDATE_RECURSIVELY);
	wxDialog::Create( parent, id, caption, pos, size, style );

	CreateControls();
	GetOptionsFromParent();
	wxCommandEvent temp;
	OnColourListChange( temp );

	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();
	return TRUE;
}

// Creates the controls and sizers.
void FxFaceGraphWidgetOptionsDlg::CreateControls()
{
	wxBoxSizer* boxSizerV = new wxBoxSizer( wxVERTICAL );
	this->SetSizer( boxSizerV );
	this->SetAutoLayout( TRUE );

	wxStaticText* label = new wxStaticText(this, wxID_DEFAULT, _("Link Function Colors"));
	boxSizerV->Add(label, 0, wxALIGN_LEFT|wxALL, 5);

	// Create the color selection component
	wxBoxSizer* boxSizerColours = new wxBoxSizer( wxHORIZONTAL );
	boxSizerV->Add( boxSizerColours, 0, wxALIGN_RIGHT|wxALL, 0 );

	FxInt32 numColours = FxLinkFn::GetNumLinkFunctions();
	wxString* colourChoices = new wxString[numColours];
	for( FxSize i = 0; i < static_cast<FxSize>(numColours); ++i )
	{
		colourChoices[i] = wxString::FromAscii(FxLinkFn::GetLinkFunction(i)->GetNameAsCstr());
	}
	_colourList = new wxListBox( this, ControlID_FaceGraphWidgetOptionsDlgColourListBox, wxDefaultPosition,
		wxDefaultSize, numColours, colourChoices );
	_colourList->SetSelection(0);
	boxSizerColours->Add( _colourList, 2, wxGROW|wxALIGN_LEFT|wxALL, 5 );

	wxBoxSizer* smallVert = new wxBoxSizer( wxVERTICAL );
	boxSizerColours->Add( smallVert, 1, wxALIGN_LEFT|wxALL, 5 );

	wxBitmap previewBmp( wxNullBitmap );
	_colourPreview = new wxStaticBitmap( this, ControlID_FaceGraphWidgetOptionsDlgColourPreviewStaticBmp,
		previewBmp, wxDefaultPosition, wxSize( 92, 20 ), wxBORDER_SIMPLE );
	smallVert->Add( _colourPreview, 0, wxALIGN_LEFT|wxALL, 5 );
	_colourPreview->SetBackgroundColour( *wxRED );
	wxButton* changeColourBtn = new wxButton( this, ControlID_FaceGraphWidgetOptionsDlgColourChangeButton,
		_("Change...") );
	smallVert->Add( changeColourBtn, 0, wxALIGN_LEFT|wxALL, 5 );

	_valueVisualizationMode = new wxCheckBox( this, ControlID_FaceGraphWidgetOptionsDlgValueVisualizationCheck, _("Visualize node values") );
	boxSizerV->Add( _valueVisualizationMode, 0, wxALIGN_LEFT|wxALL, 5 );
	_bezierLinks = new wxCheckBox( this, ControlID_FaceGraphWidgetOptionsDlgBezierLinksCheck, _("Draw Bezier links") );
	boxSizerV->Add( _bezierLinks, 0, wxALIGN_LEFT|wxALL, 5 );
	_animatedTransitions = new wxCheckBox( this, ControlID_FaceGraphWidgetOptionsDlgAnimateTransitionsCheck, _("Animate node transitions") );
	boxSizerV->Add( _animatedTransitions, 0, wxALIGN_LEFT|wxALL, 5 );
	_drawSelectedLinksOnly = new wxCheckBox( this, ControlID_FaceGraphWidgetOptionsDlgDrawOnlySelectedCheck, _("Draw only selected node links") );
	boxSizerV->Add( _drawSelectedLinksOnly, 0, wxALIGN_LEFT|wxALL, 5 );

	// Create the buttons at the bottom
	wxBoxSizer* boxSizerH = new wxBoxSizer( wxHORIZONTAL );
	boxSizerV->Add(boxSizerH, 1, wxGROW|wxALL, 5);

	wxButton* okButton = new wxButton( this, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
	okButton->SetDefault();
	boxSizerH->Add(okButton, 0, wxALIGN_LEFT|wxALL, 5);
	wxButton* cancelButton = new wxButton( this, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	boxSizerH->Add(cancelButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	_applyButton = new wxButton( this, wxID_APPLY, _("&Apply"), wxDefaultPosition, wxDefaultSize, 0 );
	_applyButton->Enable(false);
	boxSizerH->Add(_applyButton, 0, wxALIGN_RIGHT|wxALL, 5);

	Refresh( FALSE );
}

// Should we show tooltips?
FxBool FxFaceGraphWidgetOptionsDlg::ShowToolTips()
{
	return FxTrue;
}

// Called when the color list box changes
void FxFaceGraphWidgetOptionsDlg::OnColourListChange( wxCommandEvent& FxUnused(event) )
{
	FxSize index = _colourList->GetSelection();
	_colourPreview->SetBackgroundColour( _linkFnColours[index] );
	_colourPreview->Refresh();
}

// Called when the user clicks the "Change..." button to change a colour
void FxFaceGraphWidgetOptionsDlg::OnColourChange( wxCommandEvent& FxUnused(event) )
{
	FxSize index = _colourList->GetSelection();
	wxColourDialog colorPickerDialog( this );
	colorPickerDialog.GetColourData().SetChooseFull(true);
	colorPickerDialog.GetColourData().SetColour( _linkFnColours[index] );
	// Remember the user's custom colors.
	for( FxSize i = 0; i < FxColourMap::GetNumCustomColours(); ++i )
	{
		colorPickerDialog.GetColourData().SetCustomColour(i, FxColourMap::GetCustomColours()[i]);
	}
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	if( colorPickerDialog.ShowModal() == wxID_OK )
	{
		_linkFnColours[index] = colorPickerDialog.GetColourData().GetColour();
	}
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
	for( FxSize i = 0; i < FxColourMap::GetNumCustomColours(); ++i )
	{
		FxColourMap::GetCustomColours()[i] = colorPickerDialog.GetColourData().GetCustomColour(i);
	}
	_colourPreview->SetBackgroundColour( _linkFnColours[index] );
	_applyButton->Enable( TRUE );
	Refresh( FALSE );
}

// Called when the user clicks the Apply button
void FxFaceGraphWidgetOptionsDlg::OnApply( wxCommandEvent& FxUnused(event) )
{
	SetOptionsInParent();
	if( _facegraphWidget )
	{
		_facegraphWidget->Refresh(FALSE);
	}
	_applyButton->Enable( FALSE );
	Refresh(TRUE);
}

// Called when the user clicks the OK button
void FxFaceGraphWidgetOptionsDlg::OnOK( wxCommandEvent& FxUnused(event) )
{
	SetOptionsInParent();
	if( _facegraphWidget )
	{
		_facegraphWidget->Refresh(FALSE);
	}
	EndDialog(wxID_OK);
}

// Called when the user changes a checkbox.
void FxFaceGraphWidgetOptionsDlg::OnCheck( wxCommandEvent& FxUnused(event) )
{
	_applyButton->Enable(TRUE);
}

// Gets the options from the parent.
void FxFaceGraphWidgetOptionsDlg::GetOptionsFromParent()
{
	FxSize numLinkFns = FxLinkFn::GetNumLinkFunctions();
	_linkFnColours = new wxColour[numLinkFns];
	for( FxSize i = 0; i < numLinkFns; ++i )
	{
		_linkFnColours[i] = FxColourMap::Get(FxLinkFn::GetLinkFunction(i)->GetName());
	}

	if( _facegraphWidget )
	{
		_valueVisualizationMode->SetValue( _facegraphWidget->_drawValues );
		_bezierLinks->SetValue( _facegraphWidget->_drawCurves );
		_animatedTransitions->SetValue( _facegraphWidget->_animateTransitions );
		_drawSelectedLinksOnly->SetValue( _facegraphWidget->_drawSelectedNodeLinksOnly );
	}
}

// Sets the options in the parent.
void FxFaceGraphWidgetOptionsDlg::SetOptionsInParent()
{
	if( _linkFnColours )
	{
		FxSize numLinkFns = FxLinkFn::GetNumLinkFunctions();
		for( FxSize i = 0; i < numLinkFns; ++i )
		{
			FxColourMap::Set(FxLinkFn::GetLinkFunction(i)->GetName(), _linkFnColours[i]);
		}
	}

	if( _facegraphWidget )
	{
		_facegraphWidget->_drawValues = _valueVisualizationMode->GetValue();
		_facegraphWidget->_drawCurves = _bezierLinks->GetValue();
		_facegraphWidget->_animateTransitions = _animatedTransitions->GetValue();
		_facegraphWidget->_drawSelectedNodeLinksOnly = _drawSelectedLinksOnly->GetValue();
	}
}

//------------------------------------------------------------------------------
// FxFaceGraphWidgetContainer
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(FxFaceGraphWidgetContainer, wxWindow)
	EVT_PAINT(FxFaceGraphWidgetContainer::OnPaint)
	EVT_SIZE(FxFaceGraphWidgetContainer::OnSize)

	EVT_BUTTON(ControlID_FaceGraphWidgetToolbarAddNodeButton, FxFaceGraphWidgetContainer::OnToolbarAddNodeButton)
	EVT_COMMAND(ControlID_FaceGraphWidgetToolbarAddNodeButton, wxEVT_COMMAND_BUTTON_DROP_CLICKED, FxFaceGraphWidgetContainer::OnToolbarAddNodeDrop)
	EVT_MENU_RANGE(ADDNODEMENU_MODIFIER, ADDNODEMENU_MODIFIER + MAX_NODE_TYPES, FxFaceGraphWidgetContainer::OnAddNodeDropSelection)
	EVT_BUTTON(ControlID_FaceGraphWidgetToolbarNodeValueVizButton, FxFaceGraphWidgetContainer::OnToolbarNodeValueVizButton)
	EVT_BUTTON(ControlID_FaceGraphWidgetToolbarRelayoutButton, FxFaceGraphWidgetContainer::OnToolbarRelayoutButton)
	EVT_BUTTON(ControlID_FaceGraphWidgetToolbarResetViewButton, FxFaceGraphWidgetContainer::OnToolbarResetViewButton)
	EVT_BUTTON(ControlID_FaceGraphWidgetToolbarFindSelectionButton, FxFaceGraphWidgetContainer::OnToolbarFindSelectionButton)
	EVT_BUTTON(ControlID_FaceGraphWidgetToolbarMakeSGSetupButton, FxFaceGraphWidgetContainer::OnToolbarMakeSGSetupButton)

	EVT_UPDATE_UI_RANGE(ControlID_FaceGraphWidgetToolbarAddNodeButton, ControlID_FaceGraphWidgetToolbarAddNodeButton, FxFaceGraphWidgetContainer::OnUpdateUI)
END_EVENT_TABLE()

FxFaceGraphWidgetContainer::FxFaceGraphWidgetContainer(wxWindow* parent, FxWidgetMediator* mediator)
	: Super(parent, -1, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE|wxCLIP_CHILDREN|wxTAB_TRAVERSAL)
	, _selectedNodeType(ADDNODEMENU_MODIFIER)
{
	_faceGraphWidget = new FxFaceGraphWidget(this, mediator);
	CreateToolbarControls();
}

FxFaceGraphWidgetContainer::~FxFaceGraphWidgetContainer()
{
}

void FxFaceGraphWidgetContainer::UpdateControlStates(FxBool state)
{
	if( _toolbarAddNodeButton )
	{
		_toolbarAddNodeButton->Enable(state);
		_toolbarNodeValueVizButton->Enable(state);
		_toolbarNodeValueVizButton->SetValue(_faceGraphWidget->_drawValues);
		_toolbarRelayoutButton->Enable(state);
		_toolbarResetViewButton->Enable(state);
		_toolbarFindSelectionButton->Enable(state);
		_toolbarMakeSGSetupButton->Enable(state);
	}
}

void FxFaceGraphWidgetContainer::OnPaint(wxPaintEvent& FxUnused(event))
{
	wxBufferedPaintDC dc(this);
	DrawToolbar(&dc);
}

void FxFaceGraphWidgetContainer::DrawToolbar(wxDC* pDC)
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

void FxFaceGraphWidgetContainer::RefreshChildren()
{
	wxWindowList& children = GetChildren();
	wxWindowListNode* node = children.GetFirst();
	while( node )
	{
		node->GetData()->Refresh();
		node = node->GetNext();
	}
}

void FxFaceGraphWidgetContainer::OnSize(wxSizeEvent& FxUnused(event))
{
	DoResize();
	RefreshChildren();
}

void FxFaceGraphWidgetContainer::DoResize()
{
	wxRect clientRect(GetClientRect());
	_toolbarRect = clientRect;
	_toolbarRect.SetHeight(FxToolbarHeight);
	_widgetRect = clientRect;
	_widgetRect.SetTop(FxToolbarHeight);
	_widgetRect.SetHeight(_widgetRect.GetHeight() - FxToolbarHeight);

	if( _faceGraphWidget )
	{
		_faceGraphWidget->SetSize(_widgetRect);
		_faceGraphWidget->Refresh(FALSE);
	}
}

void FxFaceGraphWidgetContainer::CreateToolbarControls()
{
	SetSizer(new wxBoxSizer(wxHORIZONTAL));

	GetSizer()->AddSpacer(5);

	_toolbarAddNodeButton = new FxDropButton(this,
		ControlID_FaceGraphWidgetToolbarAddNodeButton, _("AN"), wxDefaultPosition, wxSize(30,20));
	_toolbarAddNodeButton->MakeToolbar();
	_toolbarAddNodeButton->SetToolTip(_("Add Node"));
	GetSizer()->Add(_toolbarAddNodeButton, 0, wxTOP, 2);

	_toolbarNodeValueVizButton = new FxButton(this, 
		ControlID_FaceGraphWidgetToolbarNodeValueVizButton, _("VV"), wxDefaultPosition, wxSize(20,20));
	_toolbarNodeValueVizButton->MakeToolbar();
	_toolbarNodeValueVizButton->MakeToggle();
	_toolbarNodeValueVizButton->SetToolTip(_("Toggle Node Value Visualization"));
	GetSizer()->Add(_toolbarNodeValueVizButton, 0, wxTOP|wxLEFT, 2);

	_toolbarRelayoutButton = new FxButton(this, ControlID_FaceGraphWidgetToolbarRelayoutButton,
		_("L"), wxDefaultPosition, wxSize(20,20));
	_toolbarRelayoutButton->MakeToolbar();
	_toolbarRelayoutButton->SetToolTip(_("Layout Graph"));
	GetSizer()->Add(_toolbarRelayoutButton, 0, wxTOP, 2);

	_toolbarResetViewButton = new FxButton(this, ControlID_FaceGraphWidgetToolbarResetViewButton,
		_("RV"), wxDefaultPosition, wxSize(20,20));
	_toolbarResetViewButton->MakeToolbar();
	_toolbarResetViewButton->SetToolTip(_("Reset View"));
	GetSizer()->Add(_toolbarResetViewButton, 0, wxTOP, 2);

	_toolbarFindSelectionButton = new FxButton(this, ControlID_FaceGraphWidgetToolbarFindSelectionButton,
		_("FS"), wxDefaultPosition, wxSize(20,20));
	_toolbarFindSelectionButton->MakeToolbar();
	_toolbarFindSelectionButton->SetToolTip(_("Find Selected Node(s)"));
	GetSizer()->Add(_toolbarFindSelectionButton, 0, wxTOP, 2);

	_toolbarMakeSGSetupButton = new FxButton(this,
		ControlID_FaceGraphWidgetToolbarMakeSGSetupButton, _("SG"), wxDefaultPosition, wxSize(20,20));
	_toolbarMakeSGSetupButton->MakeToolbar();
	_toolbarMakeSGSetupButton->SetToolTip(_("Create Speech Gestures Setup"));
	GetSizer()->Add(_toolbarMakeSGSetupButton, 0, wxTOP|wxLEFT, 2);

	SetAutoLayout(TRUE);
	GetSizer()->Layout();

	LoadIcons();
}

void FxFaceGraphWidgetContainer::OnToolbarAddNodeButton(wxCommandEvent& FxUnused(event))
{
	wxCommandEvent toDispatch(wxEVT_COMMAND_MENU_SELECTED, _selectedNodeType);
	_faceGraphWidget->ProcessEvent(toDispatch);
}

void FxFaceGraphWidgetContainer::OnToolbarAddNodeDrop(wxCommandEvent& FxUnused(event))
{
	wxMenu* popup = _faceGraphWidget->GetAddNodePopup();
	wxMenuItem* selected = popup->FindItem(_selectedNodeType);
	if( selected )
	{
		selected->Check(TRUE);
	}
	PopupMenu( popup, wxPoint(_toolbarAddNodeButton->GetRect().GetRight()+3, _toolbarAddNodeButton->GetRect().GetBottom()) );
	delete popup; popup = NULL;
}

void FxFaceGraphWidgetContainer::OnAddNodeDropSelection(wxCommandEvent& event)
{
	_selectedNodeType = event.GetId();
}

void FxFaceGraphWidgetContainer::OnToolbarNodeValueVizButton(wxCommandEvent& FxUnused(event))
{
	wxCommandEvent toDispatch(wxEVT_COMMAND_MENU_SELECTED, MenuID_FaceGraphWidgetToggleValues);
	_faceGraphWidget->ProcessEvent(toDispatch);
}

void FxFaceGraphWidgetContainer::OnToolbarRelayoutButton(wxCommandEvent& FxUnused(event))
{
	wxCommandEvent toDispatch(wxEVT_COMMAND_MENU_SELECTED, MenuID_FaceGraphWidgetRelayout);
	_faceGraphWidget->ProcessEvent(toDispatch);
}

void FxFaceGraphWidgetContainer::OnToolbarResetViewButton(wxCommandEvent& FxUnused(event))
{
	wxCommandEvent toDispatch(wxEVT_COMMAND_MENU_SELECTED, MenuID_FaceGraphWidgetResetView);
	_faceGraphWidget->ProcessEvent(toDispatch);
}

void FxFaceGraphWidgetContainer::OnToolbarFindSelectionButton(wxCommandEvent& FxUnused(event))
{
	_faceGraphWidget->ZoomToSelection();
}

void FxFaceGraphWidgetContainer::OnToolbarMakeSGSetupButton(wxCommandEvent& FxUnused(event))
{
	wxCommandEvent toDispatch(wxEVT_COMMAND_MENU_SELECTED, MenuID_FaceGraphWidgetCreateSpeechGesturesFragment);
	_faceGraphWidget->ProcessEvent(toDispatch);
}

void FxFaceGraphWidgetContainer::OnUpdateUI(wxUpdateUIEvent& FxUnused(event))
{
	FxActor* pActor = NULL;
	FxSessionProxy::GetActor(&pActor);
	UpdateControlStates(pActor != NULL);
}

void FxFaceGraphWidgetContainer::LoadIcons()
{
	if( _toolbarAddNodeButton )
	{
		wxString addnodePath = FxStudioApp::GetIconPath(wxT("addnode.ico"));
		wxString addnodeDisabledPath = FxStudioApp::GetIconPath(wxT("addnode_disabled.ico"));
		if( FxFileExists(addnodePath) && FxFileExists(addnodeDisabledPath) )
		{
			wxIcon addnodeIcon = wxIcon(wxIconLocation(addnodePath));
			addnodeIcon.SetHeight(16);
			addnodeIcon.SetWidth(16);
			wxIcon addnodeDisabledIcon = wxIcon(wxIconLocation(addnodeDisabledPath));
			addnodeDisabledIcon.SetHeight(16);
			addnodeDisabledIcon.SetWidth(16);
			_toolbarAddNodeButton->SetEnabledBitmap(addnodeIcon);
			_toolbarAddNodeButton->SetDisabledBitmap(addnodeDisabledIcon);
		}

		wxString nodevaluevizPath = FxStudioApp::GetIconPath(wxT("nodevaluevisualization.ico"));
		wxString nodevaluevizDisabledPath = FxStudioApp::GetIconPath(wxT("nodevaluevisualization_disabled.ico"));
		if( FxFileExists(nodevaluevizPath) && FxFileExists(nodevaluevizDisabledPath) )
		{
			wxIcon nodevaluevizIcon = wxIcon(wxIconLocation(nodevaluevizPath));
			nodevaluevizIcon.SetHeight(16);
			nodevaluevizIcon.SetWidth(16);
			wxIcon nodevaluevizDisabledIcon = wxIcon(wxIconLocation(nodevaluevizDisabledPath));
			nodevaluevizDisabledIcon.SetHeight(16);
			nodevaluevizDisabledIcon.SetWidth(16);
			_toolbarNodeValueVizButton->SetEnabledBitmap(nodevaluevizIcon);
			_toolbarNodeValueVizButton->SetDisabledBitmap(nodevaluevizDisabledIcon);
		}

		wxString relayoutPath = FxStudioApp::GetIconPath(wxT("relayout.ico"));
		wxString relayoutDisabledPath = FxStudioApp::GetIconPath(wxT("relayout_disabled.ico"));
		if( FxFileExists(relayoutPath) && FxFileExists(relayoutDisabledPath) )
		{
			wxIcon relayoutIcon = wxIcon(wxIconLocation(relayoutPath));
			relayoutIcon.SetHeight(16);
			relayoutIcon.SetWidth(16);
			wxIcon relayoutDisabledIcon = wxIcon(wxIconLocation(relayoutDisabledPath));
			relayoutDisabledIcon.SetHeight(16);
			relayoutDisabledIcon.SetWidth(16);
			_toolbarRelayoutButton->SetEnabledBitmap(relayoutIcon);
			_toolbarRelayoutButton->SetDisabledBitmap(relayoutDisabledIcon);
		}

		wxString resetviewPath = FxStudioApp::GetIconPath(wxT("resetview.ico"));
		wxString resetviewDisabledPath = FxStudioApp::GetIconPath(wxT("resetview_disabled.ico"));
		if( FxFileExists(resetviewPath) && FxFileExists(resetviewDisabledPath) )
		{
			wxIcon resetviewIcon = wxIcon(wxIconLocation(resetviewPath));
			resetviewIcon.SetHeight(16);
			resetviewIcon.SetWidth(16);
			wxIcon resetviewDisabledIcon = wxIcon(wxIconLocation(resetviewDisabledPath));
			resetviewDisabledIcon.SetHeight(16);
			resetviewDisabledIcon.SetWidth(16);
			_toolbarResetViewButton->SetEnabledBitmap(resetviewIcon);
			_toolbarResetViewButton->SetDisabledBitmap(resetviewDisabledIcon);
		}

		wxString findselectedPath = FxStudioApp::GetIconPath(wxT("findselected.ico"));
		wxString findselectedDisabledPath = FxStudioApp::GetIconPath(wxT("findselected_disabled.ico"));
		if( FxFileExists(findselectedPath) && FxFileExists(findselectedDisabledPath) )
		{
			wxIcon findselectedIcon = wxIcon(wxIconLocation(findselectedPath));
			findselectedIcon.SetHeight(16);
			findselectedIcon.SetWidth(16);
			wxIcon findselectedDisabledIcon = wxIcon(wxIconLocation(findselectedDisabledPath));
			findselectedDisabledIcon.SetHeight(16);
			findselectedDisabledIcon.SetWidth(16);
			_toolbarFindSelectionButton->SetEnabledBitmap(findselectedIcon);
			_toolbarFindSelectionButton->SetDisabledBitmap(findselectedDisabledIcon);
		}

		wxString speechgesturesetupPath = FxStudioApp::GetIconPath(wxT("speechgesturesetup.ico"));
		wxString speechgesturesetupDisabledPath = FxStudioApp::GetIconPath(wxT("speechgesturesetup_disabled.ico"));
		if( FxFileExists(speechgesturesetupPath) && FxFileExists(speechgesturesetupDisabledPath) )
		{
			wxIcon speechgesturesetupIcon = wxIcon(wxIconLocation(speechgesturesetupPath));
			speechgesturesetupIcon.SetHeight(16);
			speechgesturesetupIcon.SetWidth(16);
			wxIcon speechgesturesetupDisabledIcon = wxIcon(wxIconLocation(speechgesturesetupDisabledPath));
			speechgesturesetupDisabledIcon.SetHeight(16);
			speechgesturesetupDisabledIcon.SetWidth(16);
			_toolbarMakeSGSetupButton->SetEnabledBitmap(speechgesturesetupIcon);
			_toolbarMakeSGSetupButton->SetDisabledBitmap(speechgesturesetupDisabledIcon);
		}
	}
}


} // namespace Face

} // namespace OC3Ent
