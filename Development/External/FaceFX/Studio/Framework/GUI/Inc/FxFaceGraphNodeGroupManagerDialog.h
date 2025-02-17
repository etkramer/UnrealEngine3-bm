//------------------------------------------------------------------------------
// Face Graph node group manager dialog.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFaceGraphNodeGroupManagerDialog_H__
#define FxFaceGraphNodeGroupManagerDialog_H__

#include "FxActor.h"
#include "FxWidget.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/listctrl.h"
#endif

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

// The face graph node group manager dialog.
class FxFaceGraphNodeGroupManagerDialog : public wxDialog
{
	WX_DECLARE_DYNAMIC_CLASS(FxFaceGraphNodeGroupManagerDialog)
	DECLARE_EVENT_TABLE()
public:
	// Constructors.
	FxFaceGraphNodeGroupManagerDialog();
	FxFaceGraphNodeGroupManagerDialog( wxWindow* parent, wxWindowID id = 10005, 
		const wxString& caption = _("Node Group Manager"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(400,300), 
		long style = wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX );

	// Creation.
	FxBool Create( wxWindow* parent, wxWindowID id = 10005, 
		const wxString& caption = _("Node Group Manager"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(400,300), 
		long style = wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX );

	// Initialization.  Must be called after Create() but before 
	// ShowModal().
	void Initialize( FxActor* actor, FxWidgetMediator* mediator );
	
	// Creates the controls and sizers.
	void CreateControls( void );

	// Should we show tooltips?
	static FxBool ShowToolTips( void );

protected:
	// The actor.
	FxActor* _actor;
	// The widget mediator.
	FxWidgetMediator* _mediator;
	// The face graph node filter drop down.
	wxChoice* _filterDropdown;
	// The face graph nodes list control.
	wxListCtrl* _faceGraphNodesListCtrl;
	// The add to face graph node group button.
	wxButton* _addToGroupButton;
	// The face graph node group drop down.
	wxChoice* _groupDropdown;
	// The create new face graph node group button.
	wxButton* _newGroupButton;
	// The delete face graph node group button.
	wxButton* _deleteGroupButton;
	// The list control containing the face graph nodes in the current group.
	wxListCtrl* _nodesInGroupListCtrl;
	// The delete face graph node group button.
	wxButton* _removeFromGroupButton;
	// The OK button.
	wxButton* _okButton;

	// Handle a selection event in the node filter drop down.
	void OnFaceGraphNodeFilterSelChanged( wxCommandEvent& event );
	// Handles column clicks in the face graph node list control.
	void OnFaceGraphNodeColumnClick( wxListEvent& event );
	// Handles add to group button presses.
	void OnAddToGroup( wxCommandEvent& event );

	// Handle a selection event in the group drop down.
	void OnGroupSelChanged( wxCommandEvent& event );
	// Handle new group button presses.
	void OnNewGroup( wxCommandEvent& event );
	// Handle delete group button presses.
	void OnDeleteGroup( wxCommandEvent& event );
	// Handles column clicks in the group face graph node list control.
	void OnGroupFaceGraphNodeColumnClick( wxListEvent& event );
	// Handles remove from group button presses.
	void OnRemoveFromGroup( wxCommandEvent& event );
	// Handles OK button presses.
	void OnOK( wxCommandEvent& event );
	// Handles closing the dialog.
	void OnClose( wxCloseEvent& event );

	// Handles a selection change in any list
	void OnSelectionChanged( wxListEvent& event );
	// Handles intelligently enabling/disabling the controls.
	void CheckControlState( void );
};

} // namespace Face

} // namespace OC3Ent

#endif
