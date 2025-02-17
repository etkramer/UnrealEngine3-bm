/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __LEVELBROWSER_H__
#define __LEVELBROWSER_H__

// Forward declarations.
class ULevel;
class WxGroupPane;
class WxLevelPane;
class WxPropertyWindowFrame;

class WxLevelBrowser : public WxBrowser, public FNotifyHook, public FSerializableObject
{
	DECLARE_DYNAMIC_CLASS( WxLevelBrowser );

public:
	WxLevelBrowser();

	void AddLevel(ULevel* Level, const TCHAR* Description);

	void UpdateLevelList();
	void UpdateLevelPane();
	void UpdateGroupPane();

	////////////////////////////////
	// Level selection

	/**
	 * Clears the level selection, then sets the specified level.  Refreshes.
	 */
	void SelectSingleLevel(ULevel* InLevel);

	/**
	 * Adds the specified level to the selection set.  Refreshes.
	 */
	void SelectLevel(ULevel* InLevel);

	/**
	 * Selects all selected levels.  Refreshes.
	 */
	void SelectAllLevels();

	/**
	 * Deselects the specified level.  Refreshes.
	 */
	void DeselectLevel(const ULevel* InLevel);

	/**
	 * Deselects all selected levels.  Refreshes.
	 */
	void DeselectAllLevels();

	/**
	 * Inverts the level selection.  Refreshes.
	 */
	void InvertLevelSelection();

	/**
	 * @return		TRUE if the specified level is selected in the level browser.
	 */
	UBOOL IsSelected(ULevel* InLevel) const;

	/**
	 * Returns the head of the selection list, or NULL if nothing is selected.
	 */
	ULevel* GetSelectedLevel();
	const ULevel* GetSelectedLevel() const;

	/**
	 * Returns NULL if the number of selected level is zero or more than one.  Otherwise,
	 * returns the singly selected level.
	 */
	ULevel* GetSingleSelectedLevel();

	////////////////////////////////
	// Selected level iteration

	typedef TArray<ULevel*>::TIterator TSelectedLevelIterator;
	typedef TArray<ULevel*>::TConstIterator TSelectedLevelConstIterator;

	TSelectedLevelIterator		SelectedLevelIterator();
	TSelectedLevelConstIterator	SelectedLevelConstIterator() const;

	/**
	 * Returns the number of selected levels.
	 */
	INT GetNumSelectedLevels() const;

	void RequestUpdate(UBOOL bFullUpdate=FALSE);

	/**
 	 * Makes the specified level the current level.  Refreshes.
	 */
	void MakeLevelCurrent(ULevel* InLevel);

	/**
 	 * Merges all visible levels into the persistent level at the top.  All included levels are then removed.
	 */
	void MergeVisibleLevels();

	/**
	 * Selects all actors in the selected levels.
	 */
	void SelectActorsOfSelectedLevels();

	////////////////////////////////
	// WxBrowser interface

	/**
	* Forwards the call to our base class to create the window relationship.
	* Creates any internally used windows after that
	*
	* @param DockID the unique id to associate with this dockable window
	* @param FriendlyName the friendly name to assign to this window
	* @param Parent the parent of this window (should be a Notebook)
	*/
	virtual void Create(INT DockID,const TCHAR* FriendlyName,wxWindow* Parent);

	virtual void Update();
	virtual void Activated();

	/**
	* Returns the key to use when looking up values
	*/
	virtual const TCHAR* GetLocalizationKey() const
	{
		return TEXT("LevelBrowser");
	}


	/**
 	* Adds entries to the browser's accelerator key table.  Derived classes should call up to their parents.
	*/
	virtual void AddAcceleratorTableEntries(TArray<wxAcceleratorEntry>& Entries);

	////////////////////////////////
	// FCallbackEventDevice interface

	virtual void Send(ECallbackEventType Event);

	////////////////////////////////
	// FNotifyHook interface

	/**
	 * Refreshes the level browser if any level properties were modified.  Refreshes indirectly.
	 */
	virtual void NotifyPostChange(void* Src, UProperty* PropertyThatChanged);

	/**
	 * Used to track when the level browser's level property window is destroyed.
	 */
	virtual void NotifyDestroy(void* Src);

	/**
	 * Since this class holds onto an object reference, it needs to be serialized
	 * so that the objects aren't GCed out from underneath us.
	 *
	 * @param	Ar			The archive to serialize with.
	 */
	virtual void Serialize(FArchive& Ar);

	/**
	 * Updates the property window that contains level streaming objects, if it exists.
	 */
	void UpdateLevelPropertyWindow();


	/**
	 * Updates streaming level volume visibility status.  A streaming level volume is visible only if a level
	 * is both visible and selected in the level browser.
	 *
	 * @param	bNoteSelectionChange		[opt] If TRUE (the default), call GEditor->NoteSelectionChange() if streaming volume selection changed.
	 */
	void UpdateStreamingLevelVolumeVisibility(UBOOL bNoteSelectionChange=TRUE);

	/**
	 * Increases the mutex which controls whether Update() will be executed.  Should be called just before beginning an operation which might result in a
	 * call to Update, if the calling code must manually call Update immediately afterwards.
	 */
	void DisableUpdate();

	/**
	 * Decreases the mutex which controls whether Update() will be executed.
	 */
	void EnableUpdate();

protected:
	TArray<ULevel*>			SelectedLevels;

	WxLevelPane*			LevelPane;
	WxGroupPane*			GroupPane;
	wxSplitterWindow*		SplitterWnd;

	WxPropertyWindowFrame*	LevelPropertyWindow;

	/** TRUE if a full update was requested. */
	UBOOL					bDoFullUpdate;

	/**
	 * TRUE if an update was requested while the browser tab wasn't active.
	 * The browser will Update() the next time this browser tab is Activated().
	 */
	UBOOL					bUpdateOnActivated;

	/**
	 * Displays a property window for the selected levels.
	 */
	void ShowSelectedLevelPropertyWindow();

	/**
	 * Disassociates all objects from the level property window, then hides it.
	 */
	void ClearPropertyWindow();

	/**
	 * Creates a new streaming level.
	 *
	 * @param	bMoveSelectedActorsIntoNewLevel		If TRUE, move any selected actors into the new level.
	 */
	void CreateNewLevel(UBOOL bMoveSelectedActorsIntoNewLevel);

	/**
	 * Presents an "Open File" dialog to the user and adds the selected level(s) to the world.  Refreshes.
	 */
	void ImportLevelsFromFile();

	/**
	 * Adds the named level package to the world.  Does nothing if the level already exists in the world.
	 *
	 * @param	LevelPackageBaseFilename	The base filename of the level package to add.
	 * @return								The new level, or NULL if the level couldn't added.
	 */
	ULevel* AddLevelToWorld(const TCHAR* LevelPackageBaseFilename);

	/**
	 * Removes the specified level from the world.  Refreshes.
	 *
	 * @return	TRUE	If a level was removed.
	 */
	UBOOL RemoveLevelFromWorld(ULevel* InLevel);

	/**
	 * Sync's window elements to the specified level.  Refreshes indirectly
	 */
	void UpdateUIForLevel(ULevel* InLevel);

	/**
	 * Sets visibility for the selected levels.
	 */
	void SetSelectedLevelVisibility(UBOOL bVisible);

	/**
	 * @param bShowSelected		If TRUE, show only selected levels; if FALSE show only unselected levels.
	 */
	void ShowOnlySelectedLevels(UBOOL bShowSelected);

private:
	/** If greater than 0, ignore calls to Update. */
	INT SuppressUpdateMutex;

	/**
	 * Helper function for shifting level ordering; if a single level is selected, the
	 * level is shifted up or down one position in the WorldInfo streaming level ordering,
	 * depending on the value of the bUp argument.
	 *
	 * @param	bUp		TRUE to shift level up one position, FALSe to shife level down one position.
	 */
	void ShiftSingleSelectedLevel(UBOOL bUp);

	///////////////////
	// Wx events.

	void OnSize(wxSizeEvent& In);

	/**
	 * Handler for IDM_RefreshBrowser events; updates the browser contents.
	 */
	void OnRefresh(wxCommandEvent& In);

	/**
	 * Called when the user selects the "New Level" option from the level browser's file menu.
	 */
	void OnNewLevel(wxCommandEvent& In);

	/**
	 * Called when the user selects the "New Level From Selected Actors" option from the level browser's file menu.
	 */
	void OnNewLevelFromSelectedActors(wxCommandEvent& In);

	/**
	 * Called when the user selects the "Import Level" option from the level browser's file menu.
	 */
	void OnImportLevel(wxCommandEvent& In);
	void OnRemoveLevelFromWorld(wxCommandEvent& In);

	void OnMakeLevelCurrent(wxCommandEvent& In);
	void OnMergeVisibleLevels(wxCommandEvent& In);
	void OnSaveSelectedLevels(wxCommandEvent& In);
	void ShowSelectedLevelsInSceneManager(wxCommandEvent& In);

	/**
	 * Selects all actors in the selected levels.
	 */
	void OnSelectAllActors(wxCommandEvent& In);

	void ShiftLevelUp(wxCommandEvent& In);
	void ShiftLevelDown(wxCommandEvent& In);

	/**
	 * Shows the selected level in a property window.
	 */
	void OnLevelProperties(wxCommandEvent& In);

	/**
	 * Shows the selected level in a property window.
	 */
	void OnToggleSelfContainedLighting(wxCommandEvent& In);

	void OnShowSelectedLevels(wxCommandEvent& In);
	void OnHideSelectedLevels(wxCommandEvent& In);
	void OnShowAllLevels(wxCommandEvent& In);
	void OnHideAllLevels(wxCommandEvent& In);

	void OnSelectAllLevels(wxCommandEvent& In);
	void OnDeselectAllLevels(wxCommandEvent& In);
	void OnInvertSelection(wxCommandEvent& In);

	void OnMoveSelectedActorsToCurrentLevel(wxCommandEvent& In);

	void SetStreamingLevelVolumes(const TArray<ALevelStreamingVolume*>& LevelStreamingVolumes);
	void AddStreamingLevelVolumes(const TArray<ALevelStreamingVolume*>& LevelStreamingVolumes);
	void ClearStreamingLevelVolumes();
	void SelectStreamingLevelVolumes();

	void OnSetStreamingLevelVolumes(wxCommandEvent& In);
	void OnAddStreamingLevelVolumes(wxCommandEvent& In);
	void OnClearStreamingLevelVolumes(wxCommandEvent& In);
	void OnSelectStreamingLevelVolumes(wxCommandEvent& In);

	/*
	void OnAddSelectedLevelsToLevelStreamingVolumes(wxCommandEvent& In);
	void OnClearLevelStreamingVolumeAssignments(wxCommandEvent& In);
	void OnSelectAssociatedLevelStreamingVolumes(wxCommandEvent& In);
	*/

	void OnShowOnlySelectedLevels(wxCommandEvent& In);
	void OnShowOnlyUnselectedLevels(wxCommandEvent& In);

	DECLARE_EVENT_TABLE();
};

#endif // __LEVELBROWSER_H__
