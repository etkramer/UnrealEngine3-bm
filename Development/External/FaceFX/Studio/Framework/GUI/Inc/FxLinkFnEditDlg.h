//------------------------------------------------------------------------------
// The link function editor dialog.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxLinkFnEditDialog_H__
#define FxLinkFnEditDialog_H__

#include "FxPlatform.h"
#include "FxArray.h"
#include "FxString.h"
#include "FxFunctionEditor.h"
#include "FxWidget.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/combobox.h"
	#include "wx/grid.h"
#endif

namespace OC3Ent
{

namespace Face
{

// The link function editor widget.
class FxLinkFnEditDlg : public wxDialog
{
	typedef wxDialog Super;
	WX_DECLARE_DYNAMIC_CLASS(FxLinkFnEditDlg)
	DECLARE_EVENT_TABLE()

public:
	// Default constructor.
	FxLinkFnEditDlg();
	// Constructor.
	FxLinkFnEditDlg( wxWindow* parent, FxWidgetMediator* mediator, FxFaceGraph* pFaceGraph, 
					const FxName& inputNode, const FxName& outputNode);
	// Destructor.
	~FxLinkFnEditDlg();

//------------------------------------------------------------------------------
// Widget functions.
//------------------------------------------------------------------------------
	// Called when the selected link changes.
	virtual void OnLinkSelChanged( FxWidget* sender, const FxName& inputNode, 
		const FxName& outputNode );

protected:
//------------------------------------------------------------------------------
// Message handlers.
//------------------------------------------------------------------------------
	// Called when the selection in the link function combo changes.
	void OnLinkFunctionChanged( wxCommandEvent& event );
	// Called when a cell in the parameter grid changes.
	void OnCellChanged( wxGridEvent& event );

	// Called when the user presses apply.
	void OnButtonApply( wxCommandEvent& event );
	// Called when the user presses OK
	void OnButtonOK( wxCommandEvent& event );
	// Called when the user presses OK
	void OnButtonCancel( wxCommandEvent& event );

	// Called when the user presses a navigation key.
	void OnNavKey( wxNavigationKeyEvent& event );

//------------------------------------------------------------------------------
// Utility functions.
//------------------------------------------------------------------------------
	// Creates the controls.
	void CreateControls( void );
	// Populates the combo box with the names of the link functions.
	void PopulateLinkFunctionCombo( void );
	// Populates the grid with the names of the parameters for a given link function.
	void PopulateParameterGrid( const FxString& linkFunctionName );
	// Updates the function editor.
	void UpdateFunctionEditor( void );
	// Updates the sizes.
	void UpdateSizers( void );
	// Updates the label.
	void UpdateLabel( void );

	// Sets whether or not the controls are active.
	void SetActive( FxBool isActive );

	wxStaticText*		_linkFunctionLabel;
	FxFunctionEditor*   _linkFunctionEditor;
	wxComboBox*			_linkFunctionCombo;
	wxGrid*				_parameterGrid;

	FxWidgetMediator*   _mediator;
	FxFaceGraph*		_pGraph;
	FxName				_inputNode;
	FxName				_outputNode;

	FxBool				_active;
	FxFaceGraphNodeLink _link;

};

} // namespace Face

} // namespace OC3Ent

#endif
