//------------------------------------------------------------------------------
// A face graph node creation dialog.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCreateFaceGraphNodeDialog_H__
#define FxCreateFaceGraphNodeDialog_H__

#include "FxPlatform.h"
#include "FxActor.h"
#include "FxFaceGraphNode.h"
#include "FxWidget.h"

namespace OC3Ent
{

namespace Face
{

#ifndef wxCLOSE_BOX
	#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
	#define wxFIXED_MINSIZE 0
#endif

// Forward declare the properties.
class FxNodePropertiesWidget;
class FxNodeLinkPropertiesWidget;

// A face graph node creation dialog.
class FxCreateFaceGraphNodeDialog : public wxDialog
{
	WX_DECLARE_DYNAMIC_CLASS(FxCreateFaceGraphNodeDialog)
	DECLARE_EVENT_TABLE()
	typedef wxDialog Super;

public:
	// Constructors.
	FxCreateFaceGraphNodeDialog();

	// Creation.
	FxBool Create( wxWindow* parent, wxWindowID id = 10005, 
		const wxString& caption = _("Create Face Graph Node"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxDefaultSize, 
		long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX|wxMINIMIZE_BOX );

	// Creates the controls and sizers.
	void CreateControls( void );

	// Initializes the internal actor, face graph node, and widget mediator 
	// pointers.  This should be called after the default constructor but 
	// before the call to Create.
	void Initialize( FxActor* pActor, FxFaceGraphNode* pNode, FxWidgetMediator* pMediator );

	// Should we show tooltips?
	static FxBool ShowToolTips( void );

protected:
	// The actor.
	FxActor* _actor;
	// The face graph node.
	FxFaceGraphNode* _node;
	// The widget mediator.
	FxWidgetMediator* _mediator;
	// The create button.
	wxButton* _createButton;
	// The cancel button.
	wxButton* _cancelButton;

	FxNodePropertiesWidget* _nodePropertiesWidget;
	FxNodeLinkPropertiesWidget* _nodeLinkPropertiesWidget;

	// Removes the newly created node.
	void _removeNode( void );

	// Event handlers.
	void OnCreateButtonPressed( wxCommandEvent& event );
	void OnCancelButtonPressed( wxCommandEvent& event );
	void OnClose( wxCloseEvent& event );
};

} // namespace Face

} // namespace OC3Ent

#endif
