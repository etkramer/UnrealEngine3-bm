/*=============================================================================
	UIDockingPanel.h: Wx Panel that lets the user change docking settings for the currently selected widget. 
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UIDOCKINGPANEL_H__
#define __UIDOCKINGPANEL_H__

/**
 * This panel has controls for displaying and editing the docking links for a single widget.  This panel doesn't actually propagate changes
 * to the associated widget.  Therefore it isn't intended to be used by itself; it should be used as a component of another control which handles
 * the data flow between this control and the actual widget's docking link values.
 */
class WxUIDockingSetEditorPanel : public wxPanel, public FSerializableObject
{
	DECLARE_EVENT_TABLE()

public:
	/* === FSerializableObject interface === */
	virtual void Serialize( FArchive& Ar );

	/* === WxUIDockingSetEditorPanel interface === */
	WxUIDockingSetEditorPanel( wxWindow* InParent );

	/**
	 * Sets the widget that this panel will edit docking sets for, and refreshes all control values with the specified widget's docking set values.
	 */
	void RefreshEditorPanel( UUIObject* InWidget );

	/**
	 * Refreshes and validates the list of faces which can be used as the dock target face for the specified face.  Removes any entries that
	 * are invalid for this face (generally caused by restrictions on docking relationships necessary to prevent infinite recursion)
	 *
	 * @param	SourceFace	indicates which of the combo boxes should be validated & updated
	 */
	void UpdateAvailableDockFaces( EUIWidgetFace SourceFace );

	/**
	 * Returns the widget corresponding to the selected item in the "target widget" combo box for the specified face
	 *
	 * @param	SourceFace	indicates which of the "target widget" combo boxes to retrieve the value from
	 */
	UUIScreenObject* GetSelectedDockTarget( EUIWidgetFace SourceFace );

	/**
	 * Returns the UIWidgetFace corresponding to the selected item in the "target face" combo box for the specified face
	 *
	 * @param	SourceFace	indicates which of the "target face" combo boxes to retrieve the value from
	 */
	EUIWidgetFace GetSelectedDockFace( EUIWidgetFace SourceFace );

	/**
	 * Gets the padding value from the "padding" spin control of the specified face
	 *
	 * @param	SourceFace	indicates which of the "padding" spin controls to retrieve the value from
	 */
	FLOAT GetSelectedDockPadding( EUIWidgetFace SourceFace );

	/**
	 * Gets the evaluation type for the padding of the specified face
	 *
	 * @param	SourceFace	indicates which face to retrieve
	 */
	EUIDockPaddingEvalType GetSelectedDockPaddingType( EUIWidgetFace SourceFace );

	/** the widget we're editing */
	UUIObject*			CurrentWidget;

protected:
	/** the combo for choosing the docking target widget, per face */
	wxComboBox*		cmb_DockTarget[UIFACE_MAX];

	/** the combo for choosing the docking target face, per face */
	wxComboBox*		cmb_DockFace[UIFACE_MAX];

	/** the combo for choosing how the padding value should be evaluated, per face */
	wxComboBox*		cmb_PaddingEvalType[UIFACE_MAX];

	/** the numeric control for choosing the dock padding for each docking set */
	WxSpinCtrlReal*	spin_DockPadding[UIFACE_MAX];


	/** store the list of widgets contained by Container for quick lookup */
	TArray<UUIObject*>	ValidDockTargets;

	/** the localized names for each dock face */
	TArray<FString>		DockFaceStrings;

	/** the localized names for the different position eval types */
	TArray<FString>		EvalTypeStrings;

	/**
	 * Tracks the padding type for each face (needed when the padding type is changed)
	 */
	EUIDockPaddingEvalType	PreviousPaddingType[UIFACE_MAX];

	/**
     * Creates the controls for this window
     */
	void CreateControls();

	/**
	 * Fills the "dock target" combos with the names of the widgets within the owner container that are valid dock targets for CurrentWidget.
	 */
	void InitializeTargetCombos();

private:

	/**
	 * Called when the user changes the value of a "target widget" combo box.  Refreshes and validates the 
	 * choices available in the "target face" combo for that source face.
	 */
	void OnChangeDockingTarget( wxCommandEvent& Event );

	/**
	 * Called when the user changes the value of a "target face" combo box.  Refreshes and validates the
	 * choices available in the "target face" combo for all OTHER source faces.
	 */
	void OnChangeDockingFace( wxCommandEvent& Event );

	/**
	 * Called when the user changes the scale type for a dock padding.  Converts the existing dock padding value into
	 * the new scale type and updates the min/max values for the dock padding value spin control.
	 */
	void OnChangeDockingEvalType( wxCommandEvent& Event );
};

/**
 * Panel that lets the user modify dock links for the currently selected widget.  This panel is normally displayed in the main UI editor window.
 * All editing is handled by an internal WxUIDockingSetEditorPanel object; this panel is responsible for notifying the internal panel that the
 * selected widget has been changed, and propagating changes made to the docking parameters back to the selected widget.
 */
class WxUIDockingPanel : public wxScrolledWindow
{
public:
	WxUIDockingPanel(class WxUIEditorBase* InEditor);

	/** 
	 * Sets which widgets are currently selected and updates the panel accordingly.
	 *
	 * @param SelectedWidgets	The array of currently selected widgets.
	 */
	void SetSelectedWidgets(TArray<UUIObject*> &SelectedWidgets);

private:
	/**
	* Called when the user changes the value of a "target widget" combo box.  Refreshes and validates the 
	* choices available in the "target face" combo for that source face.
	*/
	void OnChangeDockingTarget( wxCommandEvent& Event );

	/**
	 * Called when the user changes the value of a "target face" combo box.  Refreshes and validates the
	 * choices available in the "target face" combo for all OTHER source faces.
	 */
	void OnChangeDockingFace( wxCommandEvent& Event );

	/**
	 * Called when the user changes the scale type for a dock padding.  Applies the updated scale type to the selected widgets
	 */
	void OnChangeDockingEvalType( wxCommandEvent& Event );

	/**
	 * Called when the user changes the padding for a dock face. 
	 */
	void OnChangeDockingPadding( wxSpinEvent& Event );

	/**
	* Called when the user changes the value of any docking link value which affects the widget's screen location and/or appearance.  Propagates
	* the changes to the widget.
	*/
	void PropagateDockingChanges( EUIWidgetFace SourceFace );

	/** the panel that contains all of the controls for editing */
	WxUIDockingSetEditorPanel* EditorPanel;

	DECLARE_EVENT_TABLE()
};

#endif

