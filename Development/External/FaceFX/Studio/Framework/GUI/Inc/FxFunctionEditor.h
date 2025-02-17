//------------------------------------------------------------------------------
// A little window to edit functions
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFunctionEditor_H__
#define FxFunctionEditor_H__

#include "FxPlatform.h"
#include "FxArray.h"
#include "FxLinkFn.h"

namespace OC3Ent
{

namespace Face
{

class FxFunctionEditor : public wxWindow
{
	typedef wxWindow Super;
	WX_DECLARE_DYNAMIC_CLASS( FxFunctionEditor )
	DECLARE_EVENT_TABLE()

public:
	FxFunctionEditor( wxWindow* parent = NULL, wxWindowID id = -1, wxPoint position = wxDefaultPosition, wxSize size = wxDefaultSize );
	~FxFunctionEditor();

	// Sets the link function to work on.
	void SetLinkFunction( const FxLinkFn* function, const FxLinkFnParameters& parameters );
	// Sets the domain and range to plot over
	void SetDomainRange( FxReal xMin, FxReal xMax, FxReal yMin, FxReal yMax );
	// Sets the input and output nodes
	void SetNodeNames( FxName& inputNode, FxName& outputNode );

protected:
	// Called to draw the control
	void OnPaint( wxPaintEvent& event );

	// Called when the window resizes
	void OnSize( wxSizeEvent& event );

	// The cached client rectangle
	wxRect _clientRect;
	// The rect of the drawn plot
	wxRect _plotRect;

	// The link function
	const FxLinkFn* _linkFunction;
	FxLinkFnType _linkFunctionType;
	FxLinkFnParameters _linkFnParams;

	enum MouseAction
	{
		MOUSE_NONE,
		MOUSE_ADDKEY,
		MOUSE_MOVEKEY,
		MOUSE_MOVEINCHANDLE,
		MOUSE_MOVEOUTHANDLE,
		MOUSE_DRAGSELECTION
	};
	MouseAction _mouseAction;

	// The domain and range
	FxReal _xMin;
	FxReal _xMax;
	FxReal _yMin;
	FxReal _yMax;

	// The node names
	FxName _inputNode;
	FxName _outputNode;
};

} // namespace Face

} // namespace OC3Ent

#endif

