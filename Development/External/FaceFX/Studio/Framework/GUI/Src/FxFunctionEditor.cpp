//------------------------------------------------------------------------------
// A little window to edit functions
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxFunctionEditor.h"
#include "FxFunctionPlotter.h"
#include "FxVec2.h"

namespace OC3Ent
{

namespace Face
{

WX_IMPLEMENT_DYNAMIC_CLASS( FxFunctionEditor, wxWindow )

BEGIN_EVENT_TABLE( FxFunctionEditor, wxWindow )
	EVT_PAINT( FxFunctionEditor::OnPaint )
	EVT_SIZE( FxFunctionEditor::OnSize )
END_EVENT_TABLE()

FxFunctionEditor::FxFunctionEditor( wxWindow* parent, wxWindowID id, wxPoint position, wxSize size )
	: Super( parent, id, position, size, wxSTATIC_BORDER|wxFULL_REPAINT_ON_RESIZE )
	, _linkFunction( NULL )
	, _linkFunctionType( LFT_Invalid )
	, _xMin( -1 )
	, _xMax( 1 )
	, _yMin( -1 )
	, _yMax( 1 )
	, _mouseAction( MOUSE_NONE )
{
	static const int numEntries = 2;
	wxAcceleratorEntry entries[numEntries];
	entries[0].Set( wxACCEL_CTRL, (int)'R', MenuID_FunctionEditorResetView );
	entries[1].Set( wxACCEL_CTRL, (int)'A', MenuID_FunctionEditorSelectAll );
	// Note to self: when adding an accelerator entry, update numEntries. It works better.
	wxAcceleratorTable accelerator( numEntries, entries );
	SetAcceleratorTable( accelerator );
}

FxFunctionEditor::~FxFunctionEditor()
{
}

// Sets the link function to work on.
void FxFunctionEditor::SetLinkFunction( const FxLinkFn* function, const FxLinkFnParameters& parameters )
{
	_linkFunction = function;
	_linkFunctionType = LFT_Invalid;
	if( function )
	{
		_linkFunctionType = FxLinkFn::FindLinkFunctionType(function->GetName());
	}
	_linkFnParams = parameters;
}

// Sets the domain and range to plot over
void FxFunctionEditor::SetDomainRange( FxReal xMin, FxReal xMax, 
									    FxReal yMin, FxReal yMax )
{
	_xMin = xMin;
	_xMax = xMax;
	_yMin = yMin;
	_yMax = yMax;
}

// Sets the input and output nodes
void FxFunctionEditor::SetNodeNames( FxName& inputNode, FxName& outputNode )
{
	_inputNode = inputNode;
	_outputNode = outputNode;
}

// Called to draw the control
void FxFunctionEditor::OnPaint( wxPaintEvent& FxUnused(event) )
{
	wxPaintDC front( this );

	wxBrush backgroundBrush( wxColour(255, 255, 255), wxSOLID );

	// Set up the backbuffer
	wxMemoryDC dc;
	wxBitmap backbuffer( front.GetSize().GetWidth(), front.GetSize().GetHeight() );
	dc.SelectObject( backbuffer );

	if( _linkFunction )
	{
		wxString input( _inputNode.GetAsString().GetData(), wxConvLibc );
		wxString output(_outputNode.GetAsString().GetData(), wxConvLibc );
		wxBitmap plot = FxFunctionPlotter::Plot( _linkFunctionType, _linkFnParams, 
			_clientRect.GetSize(), _xMin, _xMax, wxColour(192,192,192), _yMin, _yMax, 
			FxTrue, wxColour(170,170,170), *wxBLACK, *wxWHITE, output, input, &_plotRect );
		wxMemoryDC bitmapDC;
		bitmapDC.SelectObject( plot );
		dc.Blit( dc.GetLogicalOrigin(), bitmapDC.GetSize(), &bitmapDC, bitmapDC.GetLogicalOrigin() );
	}
	else
	{
		dc.SetBrush( wxBrush( wxColour(192, 192, 192), wxSOLID ) );
		dc.DrawRectangle( _clientRect );
	}

	// Swap to the front buffer
	front.Blit( _clientRect.GetPosition(), _clientRect.GetSize(), &dc, dc.GetLogicalOrigin(), wxCOPY );
}

// Called when the window resizes
void FxFunctionEditor::OnSize( wxSizeEvent& event )
{
	_clientRect = GetClientRect();
	event.Skip();
}

} // namespace Face

} // namespace OC3Ent
