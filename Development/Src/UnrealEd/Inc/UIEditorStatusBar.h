/*=============================================================================
	UIEditorStatusBar.h: Status Bar Class for the UI Editor Frame
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UIEDITORSTATUSBAR_H__
#define __UIEDITORSTATUSBAR_H__

/**
 * Status Bar Class for the UI Editor Frame
 */
class WxUIEditorStatusBar : public wxStatusBar
{
	DECLARE_DYNAMIC_CLASS(WxUIEditorStatusBar)

public:
	WxUIEditorStatusBar();
	virtual ~WxUIEditorStatusBar();

	/** 2nd part of the 2 step widget creation process. */
	virtual void Create( class WxUIEditorBase* InParent, wxWindowID InID, const FString& Name = TEXT("StatusBar") );

	/** Called when the UI editor frame wants the status bar to update. */
	virtual void UpdateUI();

	/**
	 * Updates the status bar to reflect which widgets are currently selected.
	 *
	 * @param SelectedWidgets		A array of the currently selected widgets.
	 */
	void UpdateSelectedWidgets(const TArray<UUIObject*> &SelectedWidgets);

	/**
	 * Updates the mouse delta tracker status field with new text.
	 *
 	 * @param InText	New text for the mouse delta tracker field.
	 */
	void UpdateMouseDelta(const TCHAR* InText);

	/**
	 * Updates the mouse tracker status field with new text.
	 *
	 * @param	ViewportClient	the viewport client containing the viewport that the mouse is moving across
	 * @param	MouseX/MouseY	the position of the mouse cursor, relative to the viewport's upper-left corner
	 */
	void UpdateMousePosition( FEditorObjectViewportClient* ViewportClient, INT MouseX, INT MouseY );

private:

	/** Event handler for when the status bar is resized. */ 
	void OnSize(wxSizeEvent& event);

	/** Event handler for when the user clicks on the drag grid toggle check box. */
	void OnDragGridToggleClick(wxCommandEvent& event);

	/** Pointer to the parent UI editor frame. */
	WxUIEditorBase* Editor;

	/** Image for the Grid field. */
	wxBitmap DragGridB;

	/** Checkbox to toggle snapping to grid. */
	wxCheckBox *DragGridCB;

	/** Image widget to represent the grid in the status bar. */
	wxStaticBitmap *DragGridSB;

	/** Text that displays what the current grid size is. */
	wxStaticText *DragGridST;

	/** Menu button that lets the user choose the grid size. */
	WxMenuButton *DragGridMB;

	/** Stores the width in pixels of the drag grid status text widget. */
	INT DragTextWidth;



	DECLARE_EVENT_TABLE()
};



#endif

