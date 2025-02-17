//------------------------------------------------------------------------------
// A wizard for creating new workspaces with given layouts.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxNewWorkspaceWizard_H__
#define FxNewWorkspaceWizard_H__

#include "FxPlatform.h"
#include "FxArray.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/wizard.h"
	#include "wx/listctrl.h"
#endif

namespace OC3Ent
{

namespace Face
{

class FxWizardPageSelectNodes : public wxWizardPageSimple
{
public:
	FxWizardPageSelectNodes(wxWizard* parent);

	// Retrieves the selected slider names.
	void GetSliderNames(FxArray<FxString>& sliderNames);

protected:
	DECLARE_EVENT_TABLE()

	// Event handlers
	void OnNodeGroupChoiceChange(wxCommandEvent& event);
	void OnNodeNameListDragStart(wxListEvent& event);
	void OnAddButton(wxCommandEvent& event);
	void OnRemoveButton(wxCommandEvent& event);
	void OnSliderNameListDragStart(wxListEvent& event);
	void OnWizardPageChanging(wxWizardEvent& event);

	// Helper functions
	void CreateControls();
	void FillNodeGroupChoice();
	void SetNodeGroupSelection();
	void FillNodeNameList();

	wxChoice*	_nodeGroupChoice;
	wxListCtrl*	_nodeNameList;
	wxListCtrl*	_sliderNameList;

	wxButton*	_addButton;
	wxButton*	_removeButton;

	FxBool		_reorderingSliders;
};

class FxWizardPageSelectLayout : public wxWizardPageSimple
{
public:
	FxWizardPageSelectLayout(wxWizard* parent);

	// Returns the selected slider orientation (0 is horizontal).
	FxInt32 GetSliderOrientation() const { return _sliderOrientation; }
	// Returns the selected number of columns/rows.
	FxInt32 GetNumCols() const { return _numCols; }
	// Returns the column spacing.
	FxInt32 GetColSpacing() const { return _colSpacing; }

protected:
	DECLARE_EVENT_TABLE()

	// Event handlers.
	void OnPresetPress(wxCommandEvent& event);
	void OnWizardPageChanging(wxWizardEvent& event);

	// Helper functions.
	void CreateControls();

	// Variables.
	FxInt32 _sliderOrientation;
	FxInt32 _numCols;
	FxInt32 _colSpacing;
};

class FxWizardPageWorkspaceOptions : public wxWizardPageSimple
{
public:
	FxWizardPageWorkspaceOptions(wxWizard* parent);

	const wxString& GetWorkspaceName() const { return _workspaceName; }

protected:
	DECLARE_EVENT_TABLE()

	// Event handlers.
	void OnWizardPageChanging(wxWizardEvent& event);

	// Helper functions.
	void CreateControls();

	// Variables.
	wxString _workspaceName;
};

} // namespace Face

} // namespace OC3Ent

#endif
