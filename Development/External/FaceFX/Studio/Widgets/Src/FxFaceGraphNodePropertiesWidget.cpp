//------------------------------------------------------------------------------
// A widget for editing the properties of a node in the face graph.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxFaceGraphNodePropertiesWidget.h"
#include "FxMath.h"
#include "FxFaceGraphNodeUserData.h"
#include "FxFunctionPlotter.h"
#include "FxLinkFnEditDlg.h"
#include "FxFaceGraphWidget.h"
#include "FxVM.h"
#include "FxTearoffWindow.h"

namespace OC3Ent
{

namespace Face
{

static const wxChar FxNodeDelimiter = wxT('?');

class FxLinkFnRenderer : public wxGridCellRenderer
{
public:
	FxLinkFnRenderer( FxFaceGraph* pGraph = NULL )
		: _pGraph( pGraph )
	{
	}

	virtual void Draw( wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rectCell, int row, int col, bool isSelected )
	{
		wxRect clientRect = rectCell;
		clientRect.Inflate(-1);

		// erase only this cells background, overflow cells should have been erased
		wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

		wxString value = grid.GetTable()->GetValue( row, col );
		FxString firstNode( value.BeforeFirst( FxNodeDelimiter ).mb_str( wxConvLibc ) );
		FxString secondNode( value.AfterFirst( FxNodeDelimiter ).mb_str( wxConvLibc ) );

		FxFaceGraphNodeLink linkInfo;
		if( _pGraph && _pGraph->GetLink( firstNode.GetData(), secondNode.GetData(), linkInfo ) )
		{
			FxFaceGraphNode* inputNode  = _pGraph->FindNode( firstNode.GetData() );
			FxFaceGraphNode* outputNode = _pGraph->FindNode( secondNode.GetData() );
			if( inputNode && outputNode )
			{
				wxColour background = isSelected ? grid.GetSelectionBackground() : grid.GetDefaultCellBackgroundColour();
				wxBitmap functionPlot = FxFunctionPlotter::Plot( 
					linkInfo.GetLinkFnType(), 
					linkInfo.GetLinkFnParams(), clientRect.GetSize(), 
					outputNode->GetMin(), outputNode->GetMax(), background,
					inputNode->GetMin(), inputNode->GetMax() );
				functionPlot.SetMask( new wxMask( functionPlot, *wxWHITE ) );

				wxMemoryDC bitmapDC;
				bitmapDC.SelectObject( functionPlot );

				dc.Blit( clientRect.GetPosition(), clientRect.GetSize(), &bitmapDC, 
						 bitmapDC.GetLogicalOrigin(), wxCOPY, TRUE );
			}
		}
	}

	virtual wxSize GetBestSize( wxGrid& FxUnused(grid), wxGridCellAttr& FxUnused(attr), wxDC& FxUnused(dc), int FxUnused(row), int FxUnused(col) )
	{
		return wxSize( 100, 100 );
	}

	virtual FxLinkFnRenderer* Clone( void ) const
	{
		return new FxLinkFnRenderer( _pGraph );
	}

protected:
	FxFaceGraph* _pGraph;
};

// Overload the starting key of the float editor to correctly handle '.'.
class FxGridCellRealEditor : public wxGridCellFloatEditor
{
public:
	FxGridCellRealEditor( int width = -1, int precision = -1 )
		: wxGridCellFloatEditor(width, precision)
		, _width(width)
		, _precision(precision)
	{
	}

	virtual void StartingKey(wxKeyEvent& event)
	{
		int keycode = (int)event.GetKeyCode();
		if ( isdigit(keycode) || keycode == '+' || keycode == '-' || keycode == '.'
			|| keycode ==  WXK_NUMPAD0
			|| keycode ==  WXK_NUMPAD1
			|| keycode ==  WXK_NUMPAD2
			|| keycode ==  WXK_NUMPAD3
			|| keycode ==  WXK_NUMPAD4
			|| keycode ==  WXK_NUMPAD5
			|| keycode ==  WXK_NUMPAD6
			|| keycode ==  WXK_NUMPAD7
			|| keycode ==  WXK_NUMPAD8
			|| keycode ==  WXK_NUMPAD9
			|| keycode ==  WXK_ADD
			|| keycode ==  WXK_NUMPAD_ADD
			|| keycode ==  WXK_SUBTRACT
			|| keycode ==  WXK_NUMPAD_SUBTRACT
			|| keycode ==  WXK_NUMPAD_DECIMAL )
		{
			wxGridCellTextEditor::StartingKey(event);

			// Skip Skip() below.
			return;
		}

		event.Skip();
	}

	virtual wxGridCellEditor* Clone() const
	{
		return new FxGridCellRealEditor(_width, _precision);
	}

protected:
	FxInt32 _width;
	FxInt32 _precision;
};

//------------------------------------------------------------------------------
// FxNodePropertiesWidget
//------------------------------------------------------------------------------
FxReal    FxNodePropertiesWidget::_copyMin = 0.0f;
FxReal	  FxNodePropertiesWidget::_copyMax = 1.0f;
FxInputOp FxNodePropertiesWidget::_copyInputOp = IO_Sum;

WX_IMPLEMENT_DYNAMIC_CLASS(FxNodePropertiesWidget, wxGrid)
BEGIN_EVENT_TABLE(FxNodePropertiesWidget, wxGrid)
	EVT_GRID_CELL_RIGHT_CLICK(FxNodePropertiesWidget::OnCellRightClick)
	EVT_GRID_CELL_CHANGE(FxNodePropertiesWidget::OnCellChanged)
	//EVT_GRID_CELL_LEFT_DCLICK(FxNodePropertiesWidget::OnCellLeftDClick)
	//EVT_GRID_COL_SIZE(FxNodePropertiesWidget::OnColSize)
	//EVT_SIZE(FxNodePropertiesWidget::OnSize)

	EVT_MENU(MenuID_FaceGraphNodePropertiesWidgetCopy, FxNodePropertiesWidget::OnCopy)
	EVT_MENU(MenuID_FaceGraphNodePropertiesWidgetPaste, FxNodePropertiesWidget::OnPaste)
END_EVENT_TABLE()

// Constructor
FxNodePropertiesWidget::
FxNodePropertiesWidget( wxWindow* parent, FxWidgetMediator* mediator,
						wxWindowID id, wxPoint position, wxSize size, FxInt32 style )
	: wxGrid( parent, id, position, size, style|wxSIMPLE_BORDER|wxFULL_REPAINT_ON_RESIZE )
	, FxWidget( mediator )
	, _pFaceGraphWidget( NULL )
	, _deletingRows( FxFalse )
	, _editingUserProp( FxFalse )
{
	static const int numEntries = 2;
	wxAcceleratorEntry entries[numEntries];
	entries[0].Set( wxACCEL_CTRL, (FxInt32)'C', MenuID_FaceGraphNodePropertiesWidgetCopy );
	entries[1].Set( wxACCEL_CTRL, (FxInt32)'V', MenuID_FaceGraphNodePropertiesWidgetPaste );
	// Note to self: when adding an accelerator entry, update numEntries. It works better.
	wxAcceleratorTable accelerator( numEntries, entries );
	SetAcceleratorTable( accelerator );

	// Set up the grid.
	CreateGrid( 4, 2, wxGrid::wxGridSelectRows );
	SetRowLabelSize(0);
	SetColLabelSize(20);
	SetColSize(1, 170);

	// Create the column labels.
	SetColLabelValue( colID_Name, _("Name") );
	SetColLabelValue( colID_Value, _("Value") );

	// Fill out the row labels.
	SetCellValue( _("Node Name"), rowID_Name, colID_Name );
	SetCellValue( _("Minimum Value"), rowID_MinVal, colID_Name );
	SetCellValue( _("Maximum Value"), rowID_MaxVal, colID_Name );
	SetCellValue( _("Input Operation"), rowID_NodeOp, colID_Name );

	// Set the editors and renderers.
	SetCellRenderer( rowID_Name, colID_Value, new wxGridCellStringRenderer );
	SetCellEditor( rowID_Name, colID_Value, new wxGridCellTextEditor );

	SetCellRenderer( rowID_MinVal, colID_Value, new wxGridCellFloatRenderer(-1,2) );
	SetCellEditor( rowID_MinVal, colID_Value, new FxGridCellRealEditor(-1,2) );

	SetCellRenderer( rowID_MaxVal, colID_Value, new wxGridCellFloatRenderer(-1,2) );
	SetCellEditor( rowID_MaxVal, colID_Value, new FxGridCellRealEditor(-1,2) );

	const wxString choices[] = {_("Sum Inputs"), _("Multiply Inputs"), _("Select Max Input"), _("Select Min Input")};
	SetCellEditor( rowID_NodeOp, colID_Value, new wxGridCellChoiceEditor(WXSIZEOF(choices), choices) );

	// Set the correct cells to read only.
	SetCorrectReadOnly();
}

// Called when the actor is changed.
void FxNodePropertiesWidget::OnActorChanged( FxWidget* FxUnused(sender), FxActor* actor )
{
	if( actor )
	{
		_pGraph = &actor->GetDecompiledFaceGraph();
	}
}

// Called when the selected faceGraph node(s) change(s).
void FxNodePropertiesWidget::
OnFaceGraphNodeSelChanged( FxWidget* FxUnused(sender), const FxArray<FxName>& selFaceGraphNodeNames, FxBool FxUnused(zoomToSelection) )
{
	_selectedNodes = selFaceGraphNodeNames;
	FillGrid();
	ForceRefresh();
}

// Called when the actor's internal data changes.
void FxNodePropertiesWidget::OnActorInternalDataChanged( FxWidget* FxUnused(sender), FxActorDataChangedFlag flags )
{
	if( flags & ADCF_FaceGraphChanged && !_editingUserProp )
	{
		FillGrid();
		ForceRefresh();
	}
}

// Called to refresh the widget
void FxNodePropertiesWidget::OnRefresh( FxWidget* FxUnused(sender) )
{
}

// Binds the properties widget to a face graph.
void FxNodePropertiesWidget::BindToFaceGraphWidget( FxFaceGraphWidget* pFaceGraphWidget )
{
	_pFaceGraphWidget = pFaceGraphWidget;
}

// Fills out the grid
void FxNodePropertiesWidget::FillGrid()
{
	BeginBatch();

	// Clear any old user property editors.
	if( GetNumberRows() > 4  )
	{
		_deletingRows = FxTrue;
		DeleteRows(rowID_UserPropStart, GetNumberRows() - rowID_UserPropStart, FxFalse);
		_deletingRows = FxFalse;
	}

	wxFont font = GetCellFont(0,0);
	if( _pGraph && _selectedNodes.Length() == 1 )
	{
		FxFaceGraphNode* node = _pGraph->FindNode(_selectedNodes.Back());
		if( node )
		{
			SetCellFont( rowID_Name, colID_Value, font );
			SetCellValue( rowID_Name, colID_Value, wxString(node->GetNameAsCstr(), wxConvLibc) );
			SetCellValue( rowID_MinVal, colID_Value, wxString::Format(wxT("%.2f"), node->GetMin()) );
			SetCellValue( rowID_MaxVal, colID_Value, wxString::Format(wxT("%.2f"), node->GetMax()) );
			SetCellValue( rowID_NodeOp, colID_Value, NodeOpToString(node->GetNodeOperation()) );

			// Set up any required user property editors.
			AppendRows(node->GetNumUserProperties());
			for( FxSize i = 0; i < node->GetNumUserProperties(); ++i )
			{
				FxInt32 rowID = rowID_UserPropStart + i;
				const FxFaceGraphNodeUserProperty& userProperty = node->GetUserProperty(i);
				SetCellValue(wxString(userProperty.GetNameAsCstr(), wxConvLibc), rowID, colID_Name);
				SetReadOnly(rowID, colID_Name);

				if( UPT_Integer == userProperty.GetPropertyType() )
				{
					SetCellRenderer(rowID, colID_Value, new wxGridCellNumberRenderer());
					SetCellEditor(rowID, colID_Value, new wxGridCellNumberEditor());
					SetCellValue(rowID, colID_Value, wxString::Format(wxT("%d"), userProperty.GetIntegerProperty()));
				}
				else if( UPT_Bool == userProperty.GetPropertyType() )
				{
					const wxString choices[] = {_("False"), _("True")};
					SetCellEditor(rowID, colID_Value, new wxGridCellChoiceEditor(WXSIZEOF(choices), choices));
					if( FxFalse == userProperty.GetBoolProperty() )
					{
						SetCellValue(rowID, colID_Value, wxT("False"));
					}
					else
					{
						SetCellValue(rowID, colID_Value, wxT("True"));
					}
				}
				else if( UPT_Real == userProperty.GetPropertyType() )
				{
					SetCellRenderer(rowID, colID_Value, new wxGridCellFloatRenderer(-1,2));
					SetCellEditor(rowID, colID_Value, new FxGridCellRealEditor(-1,2));
					SetCellValue(rowID, colID_Value, wxString::Format(wxT("%.2f"), userProperty.GetRealProperty()));
				}
				else if( UPT_String == userProperty.GetPropertyType() )
				{
					SetCellFont(rowID, colID_Value, font);
					SetCellRenderer(rowID, colID_Value, new wxGridCellStringRenderer());
					SetCellEditor(rowID, colID_Value, new wxGridCellTextEditor());
					SetCellValue(rowID, colID_Value, wxString(userProperty.GetStringProperty().GetData(), wxConvLibc));
				}
				else if( UPT_Choice == userProperty.GetPropertyType() )
				{
					wxString* choices = new wxString[userProperty.GetNumChoices()];
					for( FxSize j = 0; j < userProperty.GetNumChoices(); ++j )
					{
						choices[j] = wxString(userProperty.GetChoice(j).GetData(), wxConvLibc);
					}
					SetCellEditor(rowID, colID_Value, new wxGridCellChoiceEditor(userProperty.GetNumChoices(), choices));
					delete [] choices;
					SetCellValue(rowID, colID_Value, wxString(userProperty.GetChoiceProperty().GetData(), wxConvLibc));
				}
			}
		}
	}
	else if( _pGraph && _selectedNodes.Length() > 1 )
	{
		if( IsCellEditControlShown() )
		{
			HideCellEditControl();
		}
		font.SetStyle( wxITALIC );
		SetCellFont( rowID_Name, colID_Value, font );
		SetCellValue( rowID_Name, colID_Value, _("Multiple Nodes") );
		SetCellValue( rowID_MinVal, colID_Value, GetRange(MinValue) );
		SetCellValue( rowID_MaxVal, colID_Value, GetRange(MaxValue) );
		SetCellValue( rowID_NodeOp, colID_Value, GetRange(InputOperation) );
	}
	else
	{
		for( FxSize i = 0; i < 4; ++i )
		{
			SetCellValue( i, colID_Value, wxT("") );
		}
	}

	// Set the correct cells to read only.
	SetEditable( _selectedNodes.Length() != 0 );
	SetCorrectReadOnly();
	EndBatch();
}

// Called when a cell is right-clicked.
void FxNodePropertiesWidget::OnCellRightClick( wxGridEvent& event )
{
	wxMenu popup;
	popup.Append( MenuID_FaceGraphNodePropertiesWidgetCopy, _("&Copy\tCtrl+C") );
	popup.Append( MenuID_FaceGraphNodePropertiesWidgetPaste, _("&Paste\tCtrl+V") );
	popup.Enable( MenuID_FaceGraphNodePropertiesWidgetPaste, _copyMin != FxInvalidValue );
	PopupMenu( &popup, event.GetPosition() );
}

// Called when a cell changes
void FxNodePropertiesWidget::OnCellChanged( wxGridEvent& event )
{
	if( _pGraph && !_deletingRows )
	{
		wxString cellValue = GetCellValue( event.GetRow(), event.GetCol() );
		if( event.GetRow() == rowID_Name ) // name changed
		{
			FxString newName(cellValue.mb_str(wxConvLibc));
			if( _pGraph->FindNode(newName.GetData()) )
			{
				SetCellValue(event.GetRow(), event.GetCol(), wxString::FromAscii(_selectedNodes[0].GetAsCstr()));
				FxMessageBox(_("Error: A node with that name already exists in the Face Graph."), 
					_("Duplicate node name"), wxOK|wxCENTRE);
				return;
			}

			for( FxSize i = 0; i < _selectedNodes.Length(); ++i )
			{
				
				FxFaceGraphNode* node = _pGraph->FindNode(_selectedNodes[i]);
				if( node )
				{
					wxString command = wxString::Format(wxT("setName -facegraphnode -old \"%s\" -new \"%s\""),
						wxString(node->GetNameAsCstr(), wxConvLibc), wxString(newName.GetData(), wxConvLibc));
					FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
				}
			}
		}
		else if( event.GetRow() == rowID_MinVal ) // min value changed
		{
			FxReal minValue = FxAtof(cellValue.mb_str(wxConvLibc));
			if( _selectedNodes.Length() > 1 )
			{
				FxVM::ExecuteCommand("batch");
				for( FxSize i = 0; i < _selectedNodes.Length(); ++i )
				{
					FxFaceGraphNode* node = _pGraph->FindNode(_selectedNodes[i]);
					if( node )
					{
						wxString command = wxString::Format(wxT("graph -editnode -name \"%s\" -min %f"),
							wxString(node->GetNameAsCstr(), wxConvLibc), minValue);
						FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
					}
				}
				FxVM::ExecuteCommand("execBatch -editednodes");
			}
			else if( _selectedNodes.Length() == 1 )
			{
				FxFaceGraphNode* node = _pGraph->FindNode(_selectedNodes.Back());
				if( node )
				{
					wxString command = wxString::Format(wxT("graph -editnode -name \"%s\" -min %f"),
						wxString(node->GetNameAsCstr(), wxConvLibc), minValue);
					FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
				}
			}
		}
		else if( event.GetRow() == rowID_MaxVal ) // max value changed
		{
			FxReal maxValue = FxAtof(cellValue.mb_str(wxConvLibc));
			if( _selectedNodes.Length() > 1 )
			{
				FxVM::ExecuteCommand("batch");
				for( FxSize i = 0; i < _selectedNodes.Length(); ++i )
				{
					FxFaceGraphNode* node = _pGraph->FindNode(_selectedNodes[i]);
					if( node )
					{
						wxString command = wxString::Format(wxT("graph -editnode -name \"%s\" -max %f"),
							wxString(node->GetNameAsCstr(), wxConvLibc), maxValue);
						FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
					}
				}
				FxVM::ExecuteCommand("execBatch -editednodes");
			}
			else if( _selectedNodes.Length() == 1 )
			{
				FxFaceGraphNode* node = _pGraph->FindNode(_selectedNodes.Back());
				if( node )
				{
					wxString command = wxString::Format(wxT("graph -editnode -name \"%s\" -max %f"),
						wxString(node->GetNameAsCstr(), wxConvLibc), maxValue);
					FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
				}
			}
		}
		else if( event.GetRow() == rowID_NodeOp ) // input operation changed
		{
			if( _selectedNodes.Length() > 1 )
			{
				FxVM::ExecuteCommand("batch");
				for( FxSize i = 0; i < _selectedNodes.Length(); ++i )
				{
					FxFaceGraphNode* node = _pGraph->FindNode(_selectedNodes[i]);
					if( node )
					{
						wxString command = wxString::Format(wxT("graph -editnode -name \"%s\" -inputop \"%s\""),
							wxString(node->GetNameAsCstr(), wxConvLibc), cellValue);
						FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
					}
				}
				FxVM::ExecuteCommand("execBatch -editednodes");
			}
			else if( _selectedNodes.Length() == 1 )
			{
				FxFaceGraphNode* node = _pGraph->FindNode(_selectedNodes.Back());
				if( node )
				{
					wxString command = wxString::Format(wxT("graph -editnode -name \"%s\" -inputop \"%s\""),
						wxString(node->GetNameAsCstr(), wxConvLibc), cellValue);
					FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
				}
			}
		}
		else
		{
			// User properties (only valid with single selection).
			if( _selectedNodes.Length() == 1 )
			{
				FxFaceGraphNode* node = _pGraph->FindNode(_selectedNodes.Back());
				if( node )
				{
					wxString userPropName = GetCellValue(event.GetRow(), colID_Name);
					wxString command = wxString::Format(wxT("graph -editnode -name \"%s\" -userprops \"%s|%s\""),
						wxString(node->GetNameAsCstr(), wxConvLibc), userPropName, cellValue);
					_editingUserProp = FxTrue;
					FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
					_editingUserProp = FxFalse;
				}
			}
		}
	}
}

// Sets the correct cells to read only
void FxNodePropertiesWidget::SetCorrectReadOnly()
{
	FxBool hasSelection    = _selectedNodes.Length() != 0;
	FxBool singleSelection = _selectedNodes.Length() == 1;
	if( hasSelection )
	{
		for( FxSize i = 0; i < 4; ++i )
		{
			SetReadOnly( i, colID_Name );

			if( i == 0 )
			{
				SetReadOnly( i, colID_Value, !singleSelection );
			}
			else
			{
				SetReadOnly( i, colID_Value, FALSE );
			}
		}
	}
	else
	{
		for( FxSize i = 0; i < 4; ++i )
		{
			SetReadOnly( i, colID_Name );
			SetReadOnly( i, colID_Value );
		}
	}
}

// Converts an input node op to a string
wxString FxNodePropertiesWidget::NodeOpToString( FxInputOp operation )
{
	switch( operation )
	{
	case IO_Multiply:
		return _("Multiply Inputs");
	case IO_Max:
		return _("Select Max Input");
	case IO_Min:
		return _("Select Min Input");
	default:
	case IO_Sum:
		return _("Sum Inputs");
	};
}

// Converts a string to an input node op
FxInputOp FxNodePropertiesWidget::StringToNodeOp( const wxString& str )
{
	if( str == wxT("Multiply Inputs") )
	{
		return IO_Multiply;
	}
	else if( str == wxT("Select Max Input") )
	{
		return IO_Max;
	}
	else if( str == wxT("Select Min Input") )
	{
		return IO_Min;
	}
	return IO_Sum;
}

// Gets the min/max value for a range of nodes
wxString FxNodePropertiesWidget::GetRange( ValueToGet valToGet )
{
	// Find the if the minimum value should be displayed.
	if( _pGraph )
	{
		FxFaceGraphNode* node = _pGraph->FindNode( _selectedNodes[0] );
		if( node )
		{
			if( valToGet != InputOperation )
			{
				FxReal value = valToGet == MinValue ? node->GetMin() : node->GetMax();
				FxBool valuesConsistent = FxTrue;
				for( FxSize i = 0; i < _selectedNodes.Length(); ++i )
				{
					FxFaceGraphNode* currNode = _pGraph->FindNode( _selectedNodes[i] );
					if( currNode )
					{
						valuesConsistent &= FxRealEquality( value, 
							valToGet == MinValue ? currNode->GetMin() : currNode->GetMax() );
					}
				}
				if( valuesConsistent )
				{
					return wxString::Format( wxT("%f"), value );
				}
			}
			else
			{
				FxInputOp value = node->GetNodeOperation();
				FxBool valuesConsistent = FxTrue;
				for( FxSize i = 0; i < _selectedNodes.Length(); ++i )
				{
					FxFaceGraphNode* currNode = _pGraph->FindNode( _selectedNodes[i] );
					if( currNode )
					{
						valuesConsistent &= value == currNode->GetNodeOperation();
					}
				}
				if( valuesConsistent )
				{
					return NodeOpToString(value);
				}
			}
		}
	}
	return wxEmptyString;
}

// Dispatches a node selection change.
void FxNodePropertiesWidget::DispatchNodeSelChange( const FxArray<FxName>& selNodes )
{
	if( _mediator )
	{
		_mediator->OnFaceGraphNodeSelChanged( this, selNodes );
	}
}

// Called when a column is resized
void FxNodePropertiesWidget::OnColSize( wxGridSizeEvent& FxUnused(event) )
{
}

// Called when the grid is resized
void FxNodePropertiesWidget::OnSize( wxSizeEvent& event )
{
	event.Skip();
}

// Called to copy the current node's properties.
void FxNodePropertiesWidget::OnCopy( wxCommandEvent& FxUnused(event) )
{
	if( _selectedNodes.Length() == 1 )
	{
		FxFaceGraphNode* node = _pGraph->FindNode( _selectedNodes.Back() );
		if( node )
		{
			_copyMin = node->GetMin();
			_copyMax = node->GetMax();
			_copyInputOp = node->GetNodeOperation();
		}
	}
	else
	{
		_copyMin = FxAtof( GetCellValue(rowID_MinVal, colID_Value).mb_str(wxConvLibc) );
		_copyMax = FxAtof( GetCellValue(rowID_MaxVal, colID_Value).mb_str(wxConvLibc) );
		_copyInputOp = static_cast<FxInputOp>(-1);
	}
}

// Called to paste the buffer into the current node's properties
void FxNodePropertiesWidget::OnPaste( wxCommandEvent& FxUnused(event) )
{
	if( _selectedNodes.Length() == 1 )
	{
		FxFaceGraphNode* node = _pGraph->FindNode( _selectedNodes.Back() );
		node->SetMin( _copyMin );
		node->SetMax( _copyMax );
		if( static_cast<FxInt32>(_copyInputOp) != -1 )
		{
			node->SetNodeOperation( _copyInputOp );
		}
	}
	else
	{
		for( FxSize i = 0; i < _selectedNodes.Length(); ++i )
		{
			FxFaceGraphNode* node = _pGraph->FindNode( _selectedNodes[i] );
			if( node )
			{
				node->SetMin( _copyMin );
				node->SetMax( _copyMax );
			}
		}
	}

	FillGrid();
	ForceRefresh();
}

//------------------------------------------------------------------------------
// FxNodeLinkPropertiesWidget
//------------------------------------------------------------------------------

FxLinkFnType FxNodeLinkPropertiesWidget::_copyLinkFnType = LFT_Invalid;
FxLinkFnParameters FxNodeLinkPropertiesWidget::_copyLinkFnParams;

WX_IMPLEMENT_DYNAMIC_CLASS(FxNodeLinkPropertiesWidget,wxGrid)
BEGIN_EVENT_TABLE(FxNodeLinkPropertiesWidget,wxGrid)
	EVT_GRID_CELL_LEFT_CLICK(FxNodeLinkPropertiesWidget::OnCellLeftClick)
	EVT_GRID_CELL_LEFT_DCLICK(FxNodeLinkPropertiesWidget::OnCellDoubleClick)
	EVT_GRID_CELL_RIGHT_CLICK(FxNodeLinkPropertiesWidget::OnCellRightClick)
	//EVT_GRID_COL_SIZE(FxNodeLinkPropertiesWidget::OnColSize)
	//EVT_SIZE(FxNodeLinkPropertiesWidget::OnSize)

	EVT_MENU(MenuID_FaceGraphNodePropertiesWidgetDelete, FxNodeLinkPropertiesWidget::OnDelete)
	EVT_MENU(MenuID_FaceGraphNodePropertiesWidgetCopy, FxNodeLinkPropertiesWidget::OnCopy)
	EVT_MENU(MenuID_FaceGraphNodePropertiesWidgetPaste, FxNodeLinkPropertiesWidget::OnPaste)
END_EVENT_TABLE()
// Constructor.
FxNodeLinkPropertiesWidget::
FxNodeLinkPropertiesWidget( wxWindow* parent, FxNodeSide side,
						   FxWidgetMediator* mediator, wxWindowID id, 
						   wxPoint position, wxSize size, FxInt32 style )
	: wxGrid( parent, id, position, size, style|wxSIMPLE_BORDER|wxFULL_REPAINT_ON_RESIZE )
	, FxWidget( mediator )
	, _side( side )
	, _numRows(0)
{
	static const int numEntries = 3;
	wxAcceleratorEntry entries[numEntries];
	entries[0].Set( wxACCEL_NORMAL, WXK_DELETE, MenuID_FaceGraphNodePropertiesWidgetDelete );
	entries[1].Set( wxACCEL_CTRL, (FxInt32)'C', MenuID_FaceGraphNodePropertiesWidgetCopy );
	entries[2].Set( wxACCEL_CTRL, (FxInt32)'V', MenuID_FaceGraphNodePropertiesWidgetPaste );
	// Note to self: when adding an accelerator entry, update numEntries. It works better.
	wxAcceleratorTable accelerator( numEntries, entries );
	SetAcceleratorTable( accelerator );

	// Create the grid.
	CreateGrid( _numRows, 2, wxGrid::wxGridSelectRows );
	SetRowLabelSize(0);
	SetColLabelSize(20);

	// Create the column labels.
	wxString columnLabel = (_side == NS_InputLinks) ? _("Input From") : _("Output To");
	SetColLabelValue( colID_LinkedNode, columnLabel );
	SetColLabelValue( colID_LinkFunction, _("Link Function") );

	// Disable any edits
	SetEditable( FALSE );
}

// Called when the actor is changed.
void FxNodeLinkPropertiesWidget::OnActorChanged( FxWidget* FxUnused(sender), FxActor* actor )
{
	if( actor )
	{
		_pGraph = &actor->GetDecompiledFaceGraph();
	}
}

// Called when the selected faceGraph node(s) change(s).
void FxNodeLinkPropertiesWidget::
OnFaceGraphNodeSelChanged( FxWidget* FxUnused(sender), const FxArray<FxName>& selFaceGraphNodeNames, FxBool FxUnused(zoomToSelection) )
{
	_selectedNodes = selFaceGraphNodeNames;
	LayoutGrid();
	FillGrid();
}

// Called when the link selection changed.
void FxNodeLinkPropertiesWidget::OnLinkSelChanged( FxWidget* FxUnused(sender), 
											  const FxName& inputNode, const FxName& outputNode )
{
	_linkInputNode = inputNode;
	_linkOutputNode = outputNode;

	ClearSelection();

	wxString searchString;
	searchString.Append( wxString::FromAscii( _linkInputNode.GetAsCstr() ) );
	searchString.Append( FxNodeDelimiter );
	searchString.Append( wxString::FromAscii( _linkOutputNode.GetAsCstr() ) );

	for( FxSize i = 0; i < _numRows; ++i )
	{
		if( searchString == GetCellValue( i, colID_LinkFunction ) )
		{
			SetGridCursor( i, colID_LinkFunction );
			SelectRow( i );
		}
	}
}

// Called when the actor's internal data changes.
void FxNodeLinkPropertiesWidget::OnActorInternalDataChanged( FxWidget* FxUnused(sender), FxActorDataChangedFlag flags )
{
	if( flags & ADCF_FaceGraphChanged )
	{
		LayoutGrid();
		FillGrid();
		ForceRefresh();
	}
}

// Called to refresh the widget
void FxNodeLinkPropertiesWidget::OnRefresh( FxWidget* FxUnused(sender) )
{
}

// Lays out the grid
void FxNodeLinkPropertiesWidget::LayoutGrid()
{
	BeginBatch();
	if( _selectedNodes.Length() == 1 )
	{
		if( _pGraph )
		{
			FxSize currRows = _numRows;
			FxFaceGraphNode* node = _pGraph->FindNode( _selectedNodes.Back() );
			if( node )
			{
				if( _side == NS_InputLinks )
				{
					_numRows = GetNodeUserData( node )->inputLinkEnds.Length() - 1;
				}
				else if( _side == NS_OutputLinks )
				{
					_numRows = GetNodeUserData( node )->outputLinkEnds.Length() - 1;
				}
			}

			_numRows = FxMax<FxSize>(0, _numRows);

			if( currRows > 0 )
			{
				DeleteRows( 0, currRows );
			}
			InsertRows( 0, _numRows );

			for( FxSize i = 0; i < _numRows; ++i )
			{
				SetRowSize( i, 30 );
				SetCellRenderer( i, colID_LinkFunction, new FxLinkFnRenderer(_pGraph) );
			}
		}
	}
	else
	{
		if( _numRows > 0 )
		{
			DeleteRows( 0, _numRows );
			_numRows = 0;
		}
	}
	EndBatch();
}

void FxNodeLinkPropertiesWidget::FillGrid()
{
	BeginBatch();
	if( _pGraph && _selectedNodes.Length() )
	{
		FxFaceGraphNode* node = _pGraph->FindNode( _selectedNodes.Back() );
		if( node )
		{
			FxString currentNodeName = node->GetNameAsString();
			FxArray<FxLinkEnd*>& linkEnds = (_side == NS_InputLinks) ? 
				GetNodeUserData(node)->inputLinkEnds : GetNodeUserData(node)->outputLinkEnds;

			for( FxSize i = 0; i < _numRows; ++i )
			{
				wxString linkedNodeNameStr(linkEnds[i]->otherNode.GetAsCstr(), wxConvLibc);
				SetCellValue( i, colID_LinkedNode, linkedNodeNameStr );

				FxFaceGraphNode* inputNode = NULL;
				FxFaceGraphNode* outputNode = NULL;
				_pGraph->DetermineLinkDirection( currentNodeName.GetData(), FxString(linkedNodeNameStr.mb_str(wxConvLibc)).GetData(), inputNode, outputNode );

				wxString linkIdentifier = wxEmptyString;
				if( inputNode && outputNode )
				{
					linkIdentifier = wxString::FromAscii( inputNode->GetNameAsCstr() ) + 
						FxNodeDelimiter + 
						wxString::FromAscii( outputNode->GetNameAsCstr() );
				}
				SetCellValue( i, colID_LinkFunction, linkIdentifier );
			}
		}
	}
	EndBatch();
}

// Called when a cell is left-clicked
void FxNodeLinkPropertiesWidget::OnCellLeftClick( wxGridEvent& event )
{
	wxString value = GetCellValue( event.GetRow(), colID_LinkFunction );
	wxString firstNode( value.BeforeFirst( FxNodeDelimiter ) );
	wxString secondNode( value.AfterFirst( FxNodeDelimiter ) );
	FxName firstName( FxString(firstNode.mb_str(wxConvLibc)).GetData() );
	FxName secondName( FxString(secondNode.mb_str(wxConvLibc)).GetData() );
	if( !(_linkInputNode == firstName && _linkOutputNode == secondName) &&
		!(_linkInputNode == secondName && _linkOutputNode == firstName) )
	{
		wxString command;
		if( FxString(firstNode.mb_str(wxConvLibc)) == _selectedNodes[0].GetAsString() )
		{
			command = wxString::Format(wxT("select -type \"link\" -node1 \"%s\" -node2 \"%s\" -nozoom"),
				firstNode, secondNode );
		}
		else
		{
			command = wxString::Format(wxT("select -type \"link\" -node1 \"%s\" -node2 \"%s\" -nozoom"),
				secondNode, firstNode);		
		}
		FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
	}
	event.Skip();
}

// Called when a cell is double-clicked
void FxNodeLinkPropertiesWidget::OnCellDoubleClick( wxGridEvent& event )
{
	if( event.GetCol() == colID_LinkedNode )
	{
		wxString value = GetCellValue( event.GetRow(), event.GetCol() );
		wxString command = wxString::Format(wxT("select -type \"node\" -names \"%s\""), value);
		FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
	}
	else if( event.GetCol() == colID_LinkFunction )
	{
		OnCellLeftClick(event);
		wxString value = GetCellValue( event.GetRow(), colID_LinkFunction );
		wxString firstNode( value.BeforeFirst( FxNodeDelimiter ) );
		wxString secondNode( value.AfterFirst( FxNodeDelimiter ) );
		FxName firstName( FxString(firstNode.mb_str(wxConvLibc)).GetData() );
		FxName secondName( FxString(secondNode.mb_str(wxConvLibc)).GetData() );

		FxLinkFnEditDlg linkFnEditDlg(this, _mediator, _pGraph, firstName, secondName);
		FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
		linkFnEditDlg.ShowModal();
		FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
	}
}

// Called when a cell is right-clicked
void FxNodeLinkPropertiesWidget::OnCellRightClick( wxGridEvent& event )
{
	wxMenu popup;
	popup.Append( MenuID_FaceGraphNodePropertiesWidgetCopy, _("&Copy\tCtrl+C") );
	popup.Append( MenuID_FaceGraphNodePropertiesWidgetPaste, _("&Paste\tCtrl+V") );
	popup.Append( MenuID_FaceGraphNodePropertiesWidgetDelete, _("&Remove link\tDel") );
	popup.Enable( MenuID_FaceGraphNodePropertiesWidgetPaste, _copyLinkFnType != LFT_Invalid );
	PopupMenu( &popup, event.GetPosition() );
}

// Called when a column is resized
void FxNodeLinkPropertiesWidget::OnColSize( wxGridSizeEvent& FxUnused(event) )
{
}

// Called when the grid is resized
void FxNodeLinkPropertiesWidget::OnSize( wxSizeEvent& event )
{
	event.Skip();
}

// Called to delete the current selection
void FxNodeLinkPropertiesWidget::OnDelete( wxCommandEvent& FxUnused(event) )
{
	wxString value = GetCellValue(GetGridCursorRow(), colID_LinkFunction);
	if( value != wxEmptyString )
	{
		FxString firstNode( value.BeforeFirst( FxNodeDelimiter ).mb_str( wxConvLibc ) );
		FxString secondNode( value.AfterFirst( FxNodeDelimiter ).mb_str( wxConvLibc ) );
		FxFaceGraphNode* inputNode = NULL;
		FxFaceGraphNode* outputNode = NULL;
		_pGraph->DetermineLinkDirection( firstNode.GetData(), secondNode.GetData(), inputNode, outputNode );
		if( inputNode && outputNode )
		{
			wxString command = wxString::Format(wxT("graph -unlink -from \"%s\" -to \"%s\""), 
				wxString(outputNode->GetNameAsCstr(), wxConvLibc), 
				wxString(inputNode->GetNameAsCstr(), wxConvLibc) );
			FxVM::ExecuteCommand( FxString(command.mb_str(wxConvLibc)) );
		}
	}
}

// Called to copy the current selection
void FxNodeLinkPropertiesWidget::OnCopy( wxCommandEvent& FxUnused(event) )
{
	FxInt32 row = GetGridCursorRow();
	if( _pGraph && _selectedNodes.Length() == 1 )
	{
		FxFaceGraphNode* node = _pGraph->FindNode( _selectedNodes.Back() );
		FxString currentNodeName;
		FxString linkedNodeName;
		if( node )
		{
			currentNodeName = node->GetNameAsString();
			if( _side == NS_InputLinks )
			{
				linkedNodeName = GetNodeUserData(node)->inputLinkEnds[row]->otherNode.GetAsString();
			}
			else if( _side == NS_OutputLinks )
			{
				linkedNodeName = GetNodeUserData(node)->outputLinkEnds[row]->otherNode.GetAsString();
			}
		}

		FxFaceGraphNode* inputNode = NULL;
		FxFaceGraphNode* outputNode = NULL;
		_pGraph->DetermineLinkDirection( currentNodeName.GetData(), linkedNodeName.GetData(), inputNode, outputNode );

		if( inputNode && outputNode )
		{
			FxFaceGraphNodeLink linkInfo;
			_pGraph->GetLink( inputNode->GetName(), outputNode->GetName(), linkInfo );
			_copyLinkFnType = linkInfo.GetLinkFnType();
			_copyLinkFnParams = linkInfo.GetLinkFnParams();
		}
	}
}

// Called to paste the current selection
void FxNodeLinkPropertiesWidget::OnPaste( wxCommandEvent& FxUnused(event) )
{
	FxInt32 row = GetGridCursorRow();
	if( _pGraph && _selectedNodes.Length() == 1 )
	{
		FxFaceGraphNode* node = _pGraph->FindNode( _selectedNodes.Back() );
		FxString currentNodeName;
		FxString linkedNodeName;
		if( node )
		{
			currentNodeName = node->GetNameAsString();
			if( _side == NS_InputLinks )
			{
				linkedNodeName = GetNodeUserData(node)->inputLinkEnds[row]->otherNode.GetAsString();
			}
			else if( _side == NS_OutputLinks )
			{
				linkedNodeName = GetNodeUserData(node)->outputLinkEnds[row]->otherNode.GetAsString();
			}
		}

		FxFaceGraphNode* inputNode = NULL;
		FxFaceGraphNode* outputNode = NULL;
		_pGraph->DetermineLinkDirection( currentNodeName.GetData(), linkedNodeName.GetData(), inputNode, outputNode );

		if( inputNode && outputNode )
		{
			FxFaceGraphNodeLink linkInfo;
			linkInfo.SetLinkFnParams( FxLinkFnParameters(_copyLinkFnParams) );
			const FxLinkFn* func = FxLinkFn::FindLinkFunctionByType(_copyLinkFnType);
			linkInfo.SetLinkFn( func );
			linkInfo.SetNode( outputNode );
			if( func )
			{
				linkInfo.SetLinkFnName( func->GetName() );
			}
			_pGraph->SetLink( inputNode->GetName(), outputNode->GetName(), linkInfo );
			ForceRefresh();
		}
	}
}

} // namespace Face

} // namespace OC3Ent
