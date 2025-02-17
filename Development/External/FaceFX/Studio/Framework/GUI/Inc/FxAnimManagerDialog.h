//------------------------------------------------------------------------------
// Animation manager dialog.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAnimManagerDialog_H__
#define FxAnimManagerDialog_H__

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

// The animation manager dialog.
class FxAnimManagerDialog : public wxDialog
{
	WX_DECLARE_DYNAMIC_CLASS(FxAnimManagerDialog)
	DECLARE_EVENT_TABLE()
public:
	// Constructors.
	FxAnimManagerDialog();
	FxAnimManagerDialog( wxWindow* parent, wxWindowID id = 10012, 
		const wxString& caption = _("Animation Manager"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(400,300), 
		long style = wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX );

	// Creation.
	FxBool Create( wxWindow* parent, wxWindowID id = 10012, 
		const wxString& caption = _("Animation Manager"), 
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
	FxActor* _pActor;
	// The widget mediator.
	FxWidgetMediator* _pMediator;
	// The primary animation group drop down.
	wxChoice* _primaryAnimGroupDropdown;
	// The new animation group button.
	wxButton* _newGroupButton;
	// The delete animation group button.
	wxButton* _deleteGroupButton;
	// The list box containing all animations in the primary animation group.
	wxListCtrl* _primaryAnimListCtrl;
	// The delete animation button.
	wxButton* _deleteAnimButton;
	// The create animation button.
	wxButton* _createAnimButton;
	// The curve manager button.
	wxButton* _curveManagerButton;
	// The move from secondary to primary group button.
	wxButton* _moveFromSecondaryToPrimaryButton;
	// The move from primary to secondary group button.
	wxButton* _moveFromPrimaryToSecondaryButton;
	// The copy from secondary to primary group button.
	wxButton* _copyFromSecondaryToPrimaryButton;
	// The copy from primary to secondary group button.
	wxButton* _copyFromPrimaryToSecondaryButton;
	// The secondary animation group drop down.
	wxChoice* _secondaryAnimGroupDropdown;
	// The list box containing all animations in the secondary animation group.
	wxListCtrl* _secondaryAnimListCtrl;
	// The OK button.
	wxButton* _okButton;

	// The currently selected primary animation group.
	wxString _primaryGroup;
	// The currently selected secondary animation group.
	wxString _secondaryGroup;
	// FxTrue if there are primary animations selected.
	FxBool _primaryAnimsSelected;
	// FxTrue if there are secondary animations selected.
	FxBool _secondaryAnimsSelected;

	// Event handlers.
	void OnPrimaryAnimGroupChanged( wxCommandEvent& event );
	void OnSecondaryAnimGroupChanged( wxCommandEvent& event );
	void OnPrimaryAnimListCtrlColumnClick( wxListEvent& event );
	void OnSecondaryAnimListCtrlColumnClick( wxListEvent& event );
	void OnPrimaryAnimListCtrlSelChanged( wxListEvent& event );
	void OnSecondaryAnimListCtrlSelChanged( wxListEvent& event );
	void OnNewGroup( wxCommandEvent& event );
	void OnDeleteGroup( wxCommandEvent& event );
	void OnDeleteAnim( wxCommandEvent& event );
	void OnCreateAnim( wxCommandEvent& event );
	void OnCurveManager( wxCommandEvent& event );
	void OnMoveFromSecondaryToPrimary( wxCommandEvent& event );
	void OnMoveFromPrimaryToSecondary( wxCommandEvent& event );
	void OnCopyFromSecondaryToPrimary( wxCommandEvent& event );
	void OnCopyFromPrimaryToSecondary( wxCommandEvent& event );
	void OnOK( wxCommandEvent& event );
	void OnClose( wxCloseEvent& event );

	// Helper functions.
	void _fillGroupDropdowns( void );
	void _fillPrimaryAnimListCtrl( void );
	void _fillSecondaryAnimListCtrl( void );
	void _enableTransferButtons( void );
};

} // namespace Face

} // namespace OC3Ent

#endif
