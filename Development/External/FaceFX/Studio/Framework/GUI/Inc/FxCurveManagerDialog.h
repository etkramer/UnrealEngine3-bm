//------------------------------------------------------------------------------
// Curve manager dialog.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCurveManagerDialog_H__
#define FxCurveManagerDialog_H__

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

class FxCurveManagerDialog : public wxDialog
{
	WX_DECLARE_DYNAMIC_CLASS(FxCurveManagerDialog)
	DECLARE_EVENT_TABLE()
public:
	// Constructors.
	FxCurveManagerDialog();
	FxCurveManagerDialog( wxWindow* parent, wxWindowID id = 10013, 
		const wxString& caption = _("Curve Manager"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(400,300), 
		long style = wxDEFAULT_DIALOG_STYLE);

	// Creation.
	FxBool Create( wxWindow* parent, wxWindowID id = 10013, 
		const wxString& caption = _("Curve Manager"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(400,300), 
		long style = wxDEFAULT_DIALOG_STYLE );

	// Initialization.  Must be called after Create() but before 
	// ShowModal().
	void Initialize( FxActor* actor, FxWidgetMediator* mediator,
		FxName animGroup = FxName::NullName, FxName animation = FxName::NullName );

	// Creates the controls and sizers.
	void CreateControls( void );

protected:
	// Handles a change in the animation group.
	void OnChoiceAnimGroupChanged( wxCommandEvent& event );
	// Handles a change in the animation.
	void OnChoiceAnimationChanged( wxCommandEvent& event );
	// Handles a change in the top filter.
	void OnChoiceTopFilterChanged( wxCommandEvent& event );
	// Handles a change in the second filter.
	void OnChoiceBottomFilterChanged( wxCommandEvent& event );
	// Handles a selection change in either list
	void OnListSelectionChanged( wxListEvent& event );
	// Handles a request to remove curves from the target animation.
	void OnButtonRemoveCurves( wxCommandEvent& event );
	// Handles a request to add a curve to the current target animation.
	void OnButtonAddCurrentAnim( wxCommandEvent& event );
	// Handles a request to add a curve to the current animation group.
	void OnButtonAddCurrentGroup( wxCommandEvent& event );
	// Handles a request to add a curve to all animation groups.
	void OnButtonAddAllGroups( wxCommandEvent& event );
	// Handles the OK button press.
	void OnButtonOK( wxCommandEvent& event );
	// Handles a column click in the target names list
	void OnListTargetNamesColumnClick( wxListEvent& event );
	// Handles a column click in the source names list
	void OnListSourceNamesColumnClick( wxListEvent& event );

	// Fills out the animation group choice
	void _FillChoiceAnimGroup( void );
	// Fills out the animation choice
	void _FillChoiceAnimation( void );
	// Fills out the top level filter choice
	void _FillChoiceTopFilter( void );
	// Fills out the second level filter choice
	void _FillChoiceBottomFilter( void );
	// Fills out the target name list
	void _FillListTarget( void );
	// Fills out the source name list
	void _FillListSource( void );
	// Properly enables/disables the controls on the dialog.
	void _CheckControlState();

	// Returns an array of the currently selected strings in a list control.
	FxArray<wxString> _GetSelectedItems( wxListCtrl* pList );
	// Returns an array of the currently selected string's owners (only for source)
	FxArray<FxBool> _GetSelectedItemsOwners();
	// Adds a curve to a specified animation.
	void _AddCurveToAnim( const FxName& animGroup, const FxName& animation, const FxName& curve, FxBool ownedByAnalysis = FxFalse );
	// Adds a curve to a specified group.
	void _AddCurveToGroup( const FxName& animGroup, const FxName& curve, FxBool ownedByAnalysis = FxFalse);
	// Adds a curve to each group.
	void _AddCurveToAllGroups( const FxName& curve, FxBool ownedByAnalysis = FxFalse );

	// The actor.
	FxActor* _pActor;
	// The widget mediator.
	FxWidgetMediator* _pMediator;
	// The current animation group
	FxName _animGroup;
	// The current animation
	FxName _animation;
	// The anim group choice
	wxChoice* _choiceAnimGroup;
	// The animation choice
	wxChoice* _choiceAnimation;
	// The top level filter choice
	wxChoice* _choiceTopFilter;
	// The bottom level filter choice
	wxChoice* _choiceBottomFilter;
	// The target curves names
	wxListCtrl* _listTargetCurves;
	// The source curve names
	wxListCtrl* _listSourceCurves;
	// The remove curves button
	wxButton* _buttonRemoveCurves;
	// The add curves to current anim button
	wxButton* _buttonAddCurrentAnim;
	// The add curves to current group button
	wxButton* _buttonAddCurrentGroup;
	// The add curves to all groups button
	wxButton* _buttonAddAllGroups;
};

} // namespace Face

} // namespace OC3Ent

#endif
