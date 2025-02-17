/*=============================================================================
	StatusBars : All the status bars the editor uses

	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

enum EStatusBar
{
	SB_Standard,
	SB_Max,
};

///////////////////////////////////////////////////////////////////////////////

/**
 * The standard editor status bar.  Displayed during normal editing.
 */
class WxStatusBarStandard : public WxStatusBar
{
public:
	WxStatusBarStandard();

	virtual void SetUp();
	virtual void UpdateUI();

	/** Update the position and size of the status bar */
	virtual void RefreshPositionAndSize();

	/** Adds the specified string to the combo box list if it's not already there. */
	void AddToExecCombo( const FString& InExec );


	/**
	* Sets the mouse worldspace position text field to the text passed in.
	*
	* @param StatusText	 String to use as the new text for the worldspace position field.
	*/
	virtual void SetMouseWorldspacePositionText( const TCHAR* StatusText );

protected:

	wxComboBox *ExecCombo;
	wxBitmap DragGridB, RotationGridB;
	wxCheckBox *DragGridCB, *RotationGridCB, *ScaleGridCB, *AutoSaveCB;
	wxStaticBitmap *DragGridSB, *RotationGridSB;
	wxStaticText *DragGridST, *RotationGridST, *ScaleGridST;
	wxStaticText *ActorNameST;
	WxMenuButton *DragGridMB, *RotationGridMB, *ScaleGridMB, *AutoSaveMB;

	class WxScaleTextCtrl *DrawScaleTextCtrl;
	class WxScaleTextCtrl *DrawScaleXTextCtrl;
	class WxScaleTextCtrl *DrawScaleYTextCtrl;
	class WxScaleTextCtrl *DrawScaleZTextCtrl;

	INT DragTextWidth, RotationTextWidth, ScaleTextWidth;

	/** Bitmaps that indicate the current status of autosave. */
	WxBitmap AutosaveEnabled, AutosaveDisabled, AutosaveSoon;
	wxStaticBitmap *AutoSaveSB;

private:

	/**
	 * Overload for the default SetStatusText function.  This overload maps
	 * any text set to field 0, to field 1 because the command combo box takes up
	 * field 0.
	 */
	virtual void SetStatusText( const wxString& InText, INT InFieldIdx );

	void OnSize( wxSizeEvent& InEvent );
	void OnDragGridToggleClick( wxCommandEvent& InEvent );
	void OnRotationGridToggleClick( wxCommandEvent& InEvent );
	void OnScaleGridToggleClick( wxCommandEvent& InEvent );
	void OnAutosaveToggleClick( wxCommandEvent& InEvent );
	void OnDrawScale( wxCommandEvent& In );
	void OnDrawScaleX( wxCommandEvent& In );
	void OnDrawScaleY( wxCommandEvent& In );
	void OnDrawScaleZ( wxCommandEvent& In );
	void OnExecComboEnter( wxCommandEvent& In );
	void OnExecComboSelChange( wxCommandEvent& In );
	void OnUpdateUI(wxUpdateUIEvent &Event );

	DECLARE_EVENT_TABLE()
};

