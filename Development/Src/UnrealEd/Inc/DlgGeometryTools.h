/*=============================================================================
	DlgGeometryTools.h: UnrealEd dialog for displaying map build progress and cancelling builds.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DLGGEOMETRYTOOLS_H
#define __DLGGEOMETRYTOOLS_H

/**
  * UnrealEd dialog with various geometry mode related tools on it.
  */
class WxDlgGeometryTools : public wxDialog
{
public:
	WxDlgGeometryTools( wxWindow* InParent );
	virtual ~WxDlgGeometryTools();

private:

	/**
	 * Toggles the geometry mode modifers window.
	 *
	 * @param In	wxCommandEvent object with information about the event.
	 */
	void OnToggleWindow( wxCommandEvent& In );

	/**
	* Select Toolbar - Sets selection mode to objects.
	*
	* @param In	wxCommandEvent object with information about the event.
	*/
	void OnSelObject( wxCommandEvent& In );

	/**
	* Select Toolbar - Sets selection mode to polygons.
	*
	* @param In	wxCommandEvent object with information about the event.
	*/
	void OnSelPoly( wxCommandEvent& In );

	/**
	* Select Toolbar - Sets selection mode to edge.
	*
	* @param In	wxCommandEvent object with information about the event.
	*/
	void OnSelEdge( wxCommandEvent& In );

	/**
	* Select Toolbar - Sets selection mode to vertex.
	*
	* @param In	wxCommandEvent object with information about the event.
	*/
	void OnSelVertex( wxCommandEvent& In );

	/**
	* Updates the wxWidgets ToggleWindow check button object.
	*
	* @param In	wxUpdateUIEvent object with information about the event.
	*/
	void UI_ToggleWindow( wxUpdateUIEvent& In );

	/**
	* Updates the wxWidgets SelObject radio button object.
	*
	* @param In	wxUpdateUIEvent object with information about the event.
	*/
	void UI_SelObject( wxUpdateUIEvent& In );

	/**
	* Updates the wxWidgets SelPoly radio button object.
	*
	* @param In	wxUpdateUIEvent object with information about the event.
	*/
	void UI_SelPoly( wxUpdateUIEvent& In );

	/**
	* Updates the wxWidgets SelEdge radio button object.
	*
	* @param In	wxUpdateUIEvent object with information about the event.
	*/
	void UI_SelEdge( wxUpdateUIEvent& In );

	/**
	* Updates the wxWidgets SelVertex radio button object.
	*
	* @param In	wxUpdateUIEvent object with information about the event.
	*/
	void UI_SelVertex( wxUpdateUIEvent& In );

	/**
	* Toggles the displaying of normals.
	*
	* @param In	wxCommandEvent object with information about the event.
	*/
	void OnShowNormals( wxCommandEvent& In );

	/**
	* Toggles whether or not to only affect widgets.
	*
	* @param In	wxCommandEvent object with information about the event.
	*/
	void OnAffectWidgetOnly( wxCommandEvent& In );

	/**
	* Toggles whether or not soft selection mode is enabled.
	*
	* @param In	wxCommandEvent object with information about the event.
	*/
	void OnUseSoftSelection( wxCommandEvent& In );

	/**
	* Called when the radius text box changes, updates the soft selection radius.
	*
	* @param In	wxCommandEvent object with information about the event.
	*/
	void OnSoftSelectionRadius( wxCommandEvent& In );

	/**
	* Updates the wxWidgets ShowNormals check box object.
	*
	* @param In	wxUpdateUIEvent object with information about the event.
	*/
	void UI_ShowNormals( wxUpdateUIEvent& In );

	/**
	* Updates the wxWidgets AffectWidgetOnly check box object.
	*
	* @param In	wxUpdateUIEvent object with information about the event.
	*/
	void UI_AffectWidgetOnly( wxUpdateUIEvent& In );

	/**
	* Updates the wxWidgets UseSoftSelection check box object.
	*
	* @param In	wxUpdateUIEvent object with information about the event.
	*/
	void UI_UseSoftSelection( wxUpdateUIEvent& In );

	/**
	* Updates the wxWidgets SoftSelectionRadius TextCtrl object.
	*
	* @param In	wxUpdateUIEvent object with information about the event.
	*/
	void UI_SoftSelectionRadius( wxUpdateUIEvent& In );

	/**
	* Updates the wxWidgets SoftSelectionLabel label object.
	*
	* @param In	wxUpdateUIEvent object with information about the event.
	*/
	void UI_SoftSelectionLabel( wxUpdateUIEvent& In );

	/** wxWidgets Object Pointers */
	wxCheckBox *ShowNormalsCheck, *AffectWidgetOnlyCheck, *UseSoftSelectionCheck;
	wxTextCtrl *SoftSelectionRadiusText;
	wxStaticText *SoftSelectionLabel;
	wxToolBar* ToolBar;

	DECLARE_EVENT_TABLE()
};



#endif

