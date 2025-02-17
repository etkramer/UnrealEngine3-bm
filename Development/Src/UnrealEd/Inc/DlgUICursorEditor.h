/*=============================================================================
	DlgUICursorEditor.h : This class lets the user edit cursors for the UI system, it should only be spawned by the UI Skin Editor.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DLGUICURSOREDITOR_H__
#define __DLGUICURSOREDITOR_H__


/**
 * This class lets the user edit cursors for the UI system, it should only be spawned by the UI Skin Editor.
 */
class WxDlgUICursorEditor : public wxDialog
{
public:
	WxDlgUICursorEditor(class WxDlgUISkinEditor* InParent, class UUISkin* InParentSkin, const TCHAR* InCursorName, struct FUIMouseCursor* InCursor);


protected:

	/** Callback for when the user clicks the OK button, applies the new settings to the cursor resource. */
	void OnOK(wxCommandEvent& event);

	/** Callback for when the user clicks the show browser button, shows the generic browser. */
	void OnShowBrowser(wxCommandEvent& event);

	/** Callback for when the user clicks the use selected button, uses the currently selected texture from the generic browser if one exists. */
	void OnUseSelected(wxCommandEvent& event);

	/** The texture to use for the cursor. */
	wxTextCtrl* CursorTexture;

	/** Combo with all of the possible styles for the cursor. */
	wxComboBox* CursorStyle;

	/** Parent skin editor. */
	class WxDlgUISkinEditor* SkinEditor;

	/** Parent skin of the cursor we are modifying. */
	class UUISkin* ParentSkin;

	/** Cursor we are modifying. */
	struct FUIMouseCursor* Cursor;

	DECLARE_EVENT_TABLE()
};

#endif

