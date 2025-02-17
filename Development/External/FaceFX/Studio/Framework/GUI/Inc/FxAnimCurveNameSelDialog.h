//------------------------------------------------------------------------------
// Allows a user to choose a name for a curve.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAnimCurveNameSelDialog_H__
#define FxAnimCurveNameSelDialog_H__

#include "FxPlatform.h"
#include "FxFaceGraph.h"

namespace OC3Ent
{

namespace Face
{

class FxAnimCurveNameSelectionDlg : public wxDialog
{
	DECLARE_EVENT_TABLE();
public:

	FxAnimCurveNameSelectionDlg( wxWindow* parent, FxFaceGraph* faceGraph );

	FxName selectedTargetName;

	virtual void InitDialog();

protected:
	// Called when the filter selection changes.
	void OnFilterSelChanged( wxCommandEvent& event );

	// Called when the user presses OK.
	void OnOK( wxCommandEvent& event );

	// Initialize the selection
	void SetSelection( void );

	wxChoice*       _filterChoice;
	wxComboBox*     _targetNameCombo;
	FxFaceGraph*   _faceGraph;
	const FxClass* _nodeFilter;
};

} // namespace Face

} // namespace OC3Ent

#endif
