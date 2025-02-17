/*=============================================================================
DlgUIListEditor.h : This class lets the user edit settings for a list object in the UI system.
Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DLGUILISTEDITOR_H__
#define __DLGUILISTEDITOR_H__


/**
 * This class lets the user edit settings for a list object in the UI system.
 */
class WxDlgUIListEditor : public wxDialog, public FSerializableObject
{
public:
	WxDlgUIListEditor(WxUIEditorBase* InEditor, UUIList* InList);
	~WxDlgUIListEditor();

	/**
	 * Since this class holds onto an object reference, it needs to be serialized
	 * so that the objects aren't GCed out from underneath us.
	 *
	 * @param	Ar			The archive to serialize with.
	 */
	void Serialize(FArchive& Ar);


protected:

	/** Initializes the grid using the values stored in the list widget. */
	void RefreshGrid();

	/** Called when the user cancels operation on the list. */
	void OnCancel(wxCommandEvent& Event);

	/** Called when the user right clicks on a grid label, displays a context menu. */
	void OnLabelRightClick(wxGridEvent& Event);

	/** The UI editor that spawned this dialog. */
	WxUIEditorBase* UIEditor;

	/** Current list object we are editing. */
	UUIList* ListWidget;

	/** Simple representation of the wx grid widget. */
	wxGrid* ListGrid;

	DECLARE_EVENT_TABLE()
};

#endif


