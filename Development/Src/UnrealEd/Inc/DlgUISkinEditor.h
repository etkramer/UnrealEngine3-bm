/*=============================================================================
	DlgUISkinEditor.h : Specialized dialog for editing UI Skins.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DLGUISKINEDITOR_H__
#define __DLGUISKINEDITOR_H__

/**
  * Specialized dialog for editing UI Skins.
  */
class WxDlgUISkinEditor : public wxDialog, public FCallbackEventDevice
{
	DECLARE_DYNAMIC_CLASS(WxDlgUISkinEditor)

public:
	/** This is the client data type used by the list views, it stores the data needed to sort the lists. */
	struct FStyleGroupNameSortData
	{
		const FString StyleGroupName;
		UBOOL bInherited;

		FStyleGroupNameSortData( const FString& InGroupName, UBOOL bIsInherited )
		: StyleGroupName(InGroupName), bInherited(bIsInherited)
		{
		}
	};

	/** Options for the sorting process, tells whether to sort ascending or descending and which column to sort by. */
	struct FSortOptions
	{
		UBOOL bSortAscending;
		INT Column;

		FSortOptions()
		: bSortAscending(TRUE), Column(0)
		{}
	};


	/**
	 * Constructor
	 */
    WxDlgUISkinEditor();

	/**
	 * Destructor
	 * Saves the window position and size to the ini
	 */
	virtual ~WxDlgUISkinEditor();

	/**
	 * Initialize this dialog.  Must be the first function called after creating the dialog.
	 *
	 * @param	InParent	the window that opened this dialog
	 * @param	InSkin		the skin to edit
	 */
	void Create( wxWindow* InParent, UUISkin* InSkin );

	/* === FCallbackEventDevice interface === */
	virtual void Send( ECallbackEventType EventType );

	/** Updates the widgets that represent skin properties to reflect the properties of the currently selected skin. */
	void UpdateSkinProperties();

protected:

	/** Used to map styles to the tree id that contains them. */
	typedef TMap<UUIStyle*,wxTreeItemId> TStyleTreeMap;

	/** Used to map styles to the tree id that contains them. */
	typedef TMap<UUISkin*,wxTreeItemId> TSkinTreeMap;

	/** Initializes values for the dialog based on the selected skin. */
	void InitializeValues(UUISkin* InSelectedSkin);

	/** Rebuilds the skin tree with all of the currently available skins. */
	void RebuildSkinTree();

	/** Rebuilds the tree control with all of the styles that this skin supports. */
	void RebuildStyleTree();

public:
	/** Rebuilds the list control with all of the style groups that this skin supports. */
	void RebuildStyleGroups();

protected:
	/** Rebuilds the cursor list with all of the cursors that this skin supports. */
	void RebuildCursorList();

	/** Rebuilds the sound cue list with all of the UISoundCues that this skin supports */
	void RebuildSoundCueList();

	/**
	 * Cleans up all memory allocated for sorting data which was stored in the list directly.
	 *
	 * @param	ItemIndex	the index of the item to cleanup; if INDEX_NONE, cleans up the memory for all item data
	 */
	void CleanupSortData( INT ItemIndex=INDEX_NONE );

	/** 
	 * Adds a skin and all of its children to the tree. 
	 * @param Skin		Skin to add to the tree.
	 * @return			The TreeItemId of the item that was added to the tree.
	 */
	wxTreeItemId AddSkin(UUISkin* Skin);

	/** 
	 * Adds a style and all of its children to the tree. 
	 * @param Style		Style to add to the tree.
	 * @return			The TreeItemId of the item that was added to the tree.
	 */
	wxTreeItemId AddStyle(UUIStyle* Style);

	/** 
	* Adds a style group to the group list.
	*
	* @param	GroupName			Name of the group being added.
	* @param	bAutosizeColumns	specify TRUE to force the column widths to be resized to fit the width of the text
	*/
	void AddGroup( const FString& GroupName, UBOOL bInherited, UBOOL bAutosizeColumns=FALSE );

	/** 
	 * Adds a cursor to the cursor list.
	 *
	 * @param	CursorName			Name of the cursor being added.
	 * @param	CursorInfo			Info about the cursor being added, including its texture and style.
	 * @param	bAutosizeColumns	specify TRUE to force the column widths to be resized to fit the width of the text
	 */
	void AddCursor(const FName& CursorName, const FUIMouseCursor* CursorInfo, UBOOL bAutosizeColumns=FALSE);

public:
	/** 
	 * Adds a new UISoundCue to the list of sound cues
	 *
	 * @param	SoundCueName		the alias used for referencing the sound cue specified
	 * @param	SoundToPlay			the actual sound cue associated with this alias
	 * @param	bAutosizeColumns	specify TRUE to force the column widths to be resized to fit the width of the text
	 */
	void AddSoundCue(const FName& SoundCueName, class USoundCue* SoundToPlay, UBOOL bAutosizeColumns=FALSE );

protected:
	/**
	 * @return Returns the currently selected style.
	 */
	UUIStyle* GetSelectedStyle() const;

	/**
	 * @return Returns the currently selected skin.
	 */
	UUISkin* GetSelectedSkin() const;

	/** @return Returns whether or not the selected style is implemented in the skin that is selected. */
	UBOOL WxDlgUISkinEditor::IsImplementedStyleSelected() const;

	/**
	 * Displays the "edit style" dialog to allow the user to modify the specified style.
	 * @param StyleToEdit	Style to edit in the dialog we spawn.
	 */
	void DisplayEditStyleDialog( UUIStyle* StyleToEdit );

	/**
	 * Displays the "edit cursor" dialog to allow the user to modify the specified cursor.
	 * @param CursorName	Name of the cursor to modify.
	 */
	void DisplayEditCursorDialog( FName& CursorName );

	/**
	 * Displays the "edit sound cue" dialog to allow the user to modify the selected UISoundCue
	 *
	 * @param	SelectedSoundCue	the index of the UISoundCue that the user wants to edit
	 */
	void DisplayEditSoundCueDialog( INT SelectedSoundCue=INDEX_NONE );

	/** Reinitializes the style pointers for all currently active widgets. */
	void ReinitializeAllWidgetStyles() const;

	/** Updates the skin options when the user changes the selected skin. */
	void OnSkinSelected( wxTreeEvent &Event );

	/** Shows a skin tree control context menu. */
	void OnSkinRightClick( wxTreeEvent &Event );

	/** Displays the edit style dialog if the style is implemented in the selected skin. */
	void OnStyleActivated ( wxTreeEvent &Event );

	/** Shows a skin tree control context menu. */
	void OnStyleRightClick( wxTreeEvent &Event );

	/** Called when the user clicks a column in the Style Groups list */
	void OnClickStyleGroupListColumn( wxListEvent& Event );

	/** Called when the user selects the "Create Style" context menu option */
	void OnContext_CreateStyle( wxCommandEvent& Event );

	/** Called when the user selects the "Edit Style" context menu option */
	void OnContext_EditStyle( wxCommandEvent& Event );

	/** Called when the user selects the "Replace Style" context menu option */
	void OnContext_ReplaceStyle( wxCommandEvent& Event );

	/** Called when the user selects the "Delete Style" context menu option */
	void OnContext_DeleteStyle( wxCommandEvent& Event );

	/** Displays the edit cursor dialog. */
	void OnCursorActivated( wxListEvent &Event );

	/** Displays a cursor specific context menu. */
	void OnCursorRightClick( wxListEvent &Event );

	/** Called when the user selects the "Add Cursor" context menu option */
	void OnContext_AddCursor( wxCommandEvent& Event );

	/** Called when the user selects the "Edit Cursor" context menu option */
	void OnContext_EditCursor( wxCommandEvent& Event );

	/** Called when the user selects the "Rename Cursor" context menu option */
	void OnContext_RenameCursor( wxCommandEvent& Event );

	/** Called when the user selects the "Delete Cursor" context menu option */
	void OnContext_DeleteCursor( wxCommandEvent& Event );

	/** Displays the edit sound cue dialog. */
	void OnSoundCueActivated( wxListEvent &Event );

	/** Displays a sound cue specific context menu. */
	void OnSoundCueRightClick( wxListEvent &Event );

	/** Called when the user selects the "Add Sound Cue" context menu option */
	void OnContext_AddSoundCue( wxCommandEvent& Event );

	/** Called when the user selects the "Edit Sound Cue" context menu option */
	void OnContext_EditSoundCue( wxCommandEvent& Event );

	/** Called when the user selects the "Delete Sound Cue" context menu option */
	void OnContext_DeleteSoundCue( wxCommandEvent& Event );

	/** Called when the user selects the "Add New Group" context menu option */
	void OnContext_AddGroup( wxCommandEvent& Event );
	/** Called when the user selects the "Rename Group" context menu option */
	void OnContext_RenameGroup( wxCommandEvent& Event );
	/** Called when the user selects the "Delete Group" context menu option */
	void OnContext_DeleteGroup( wxCommandEvent& Event );

	/** Skin tree control. */
	WxTreeCtrl* SkinTree;

	/** Style tree control. */
	WxTreeCtrl* StyleTree;

	/** List of all of the cursors in the currently selected skin. */
	WxListView* CursorList;

	/** List of all of the style groups in the currently selected skin. */
	WxListView *GroupsList;

	/** List of all of the sound cues in the currently selected skin. */
	WxListView* SoundCueList;

	/** Splitter window for the skin and style trees. */
	wxSplitterWindow* SplitterWnd;

	/** Pointer to the skin we are modifying. */
	UUISkin* SelectedSkin;

	/** Maps skins to their corresponding tree id. */
	TSkinTreeMap SkinIDMap;

	/** Maps styles to their corresponding tree id. */
	TStyleTreeMap StyleIDMap;

	FSortOptions StyleGroupSortOptions;

	DECLARE_EVENT_TABLE()
};


#endif

