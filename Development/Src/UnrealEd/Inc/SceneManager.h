/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

// PCF Begin
class WxDelGrid;
// PCF End

class WxSceneManager : public WxBrowser, public FSerializableObject
{
	DECLARE_DYNAMIC_CLASS(WxSceneManager);

public:
	WxSceneManager();

	/**
	 * Returns the key to use when looking up values
	 */
	virtual const TCHAR* GetLocalizationKey() const
	{
		return TEXT("SceneManager");
	}

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
	 * Adds entries to the browser's accelerator key table.  Derived classes should call up to their parents.
	 */
	virtual void AddAcceleratorTableEntries(TArray<wxAcceleratorEntry>& Entries);

	////////////////////////////////
	// FCallbackEventDevice interface

	virtual void Send(ECallbackEventType Event);

	void CreateControls();
	void PopulateCombo(const TArray<ULevel*>& InLevels);
	void PopulateGrid(const TArray<ULevel*>& InLevels);
	void PopulatePropPanel();
	void PopulateLevelList();	
	
	/** 
	* Build levels array based on names of selected levels from list.
	 *
	 * @param	InLevels			[out] The set of selected levels.
	*/
	void GetSelectedLevelsFromList(TArray<ULevel*>& InLevels);

	/**
	 * delete selected actors
	 */
	void DeleteSelectedActors();
	
	/**
	 * @param OutSelectedActors		[out] The set of selected actors.
	 */
	void GetSelectedActors(TArray<UObject*>& InSelectedActors);

	void FocusOnSelected();

	/**
	 * Sets the set of levels to visualize the scene manager.
	 */
	void SetActiveLevels(const TArray<ULevel*>& InActiveLevels);

	/**
	 * Since this class holds onto an object reference, it needs to be serialized
	 * so that the objects aren't GCed out from underneath us.
	 *
	 * @param	Ar			The archive to serialize with.
	 */
	virtual void Serialize(FArchive& Ar);

protected:
	/** TRUE once the scene manager has been initialized. */
	UBOOL					bAreWindowsInitialized;

	/** Displays properties of currently selected object(s). */
	WxPropertyWindow*		PropertyWindow;

    // PCF Begin
	// Deprecated
	/** Splitter bar separates the grid and the property panel. */
	// wxSplitterWindow*		SplitterWindow;
	// PCF End

	/** Pulldown to filter the list by actor type. */
	wxComboBox*				TypeFilter_Combo;
	/** Currently selected type filter. */
	INT						TypeFilter_Selection;
	/** Grid widget displaying all actors in the level. */

	// PCF Begin
	/** Level selector list */
	wxListBox*				Level_List;

	// change from wxGrid to WxDelGrid
	WxDelGrid*				Grid;
	// PCF End

	/** List of actors currently displayed in the grid. */
	TArray<AActor*>			GridActors;

	/** If TRUE, show brushes in the grid. */
	wxCheckBox *			ShowBrushes_Check;
	/** Grid is sorted by this column. */
	INT						SortColumn;
	/** Filter by name. */
	FString					NameFilter;
	/** Main views are focused on selected grid object. */
	UBOOL					bAutoFocus;
    
	/**
	 * TRUE if an update was requested while the browser tab wasn't active.
	 * The browser will Update() the next time this browser tab is Activated().
	 */
	UBOOL					bUpdateOnActivated;

	/** The set of actors selected in the grid display. */
	TArray<UObject*>		SelectedActors;
	/** The set of levels whose actors are displayed in the scene manager. */
	TArray<ULevel*>			ActiveLevels;

private:
	void OnFileOpen( wxCommandEvent& In );
	void OnAutoFocus( wxCommandEvent& In );
	void OnFocus( wxCommandEvent& In );
	void OnDelete( wxCommandEvent& In );
    /** Handler for IDM_RefreshBrowser events; updates the browser contents. */
	void OnRefresh( wxCommandEvent& In );
	void OnGridSelectCell( wxGridEvent& event );
	void OnGridRangeSelect( wxGridRangeSelectEvent& event );
	void OnLabelLeftClick( wxGridEvent& event );
	void OnCellChange( wxGridEvent& In );
	void OnTypeFilterSelected( wxCommandEvent& event );
	void OnShowBrushes( wxCommandEvent& event );
	void OnNameFilterChanged( wxCommandEvent& In );
	// PCF Begin
	void OnLevelSelected( wxCommandEvent& event );
	// PCF End

	DECLARE_EVENT_TABLE()
};

// PCF Begin
/*-----------------------------------------------------------------------------
 WxDelGrid
-----------------------------------------------------------------------------*/

/**
 * This is a overloaded wxGrid class with additional functionality
 */
class WxDelGrid: public wxGrid
{
public:
	WxDelGrid(wxWindow *parent, wxWindowID id, const wxPoint& pos,
                 const wxSize& size, long style);

	/** Parent for callbacks */
	WxSceneManager* Parent;
	void SaveGridPosition();
	void RestoreGridPosition();

protected:
	// grid position for saving
	INT PrevSX,PrevSY,PrevCX,PrevCY;
	/** Keypress event handler */
	void OnChar(wxKeyEvent &Event);

	DECLARE_EVENT_TABLE()
};
// PCF End
#endif __SCENE_MANAGER_H__
