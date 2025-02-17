/*=============================================================================
	GenericBrowser.h: UnrealEd's generic browser.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __GENERICBROWSER_H__
#define __GENERICBROWSER_H__

// Forward declarations.
class WxDlgGenericBrowserSearch;

/** Data specific to each browser. */
enum EGenericBrowser_TreeCtrlImages
{
	GBTCI_None = -1,
	GBTCI_FolderClosed,
	GBTCI_FolderOpen,
	GBTCI_Resource,
	GBTCI_SCC_Doc,
	GBTCI_SCC_ReadOnly,
	GBTCI_SCC_CheckedOut,
	GBTCI_SCC_CheckedOutOther,
	GBTCI_SCC_NotCurrent,
	GBTCI_SCC_NotInDepot,
};

/**
 * Contains preferences which control how items in the browser are displayed.
 */
class WxBrowserData
{
public:
	WxBrowserData( EGenericBrowser_TreeCtrlImages InTreeCtrlImg,
		const TCHAR* KeyPrefix );
	/**
	 * Saves the settings for this instance
	 */
	~WxBrowserData(void);

	/**
	 * Scaling factor for item drawing.  Ignored if FixedSz is non-zero.
	 */
	void SetZoomFactor(FLOAT InZoomFactor)				{ ZoomFactor = InZoomFactor; }
	FLOAT GetZoomFactor() const							{ return ZoomFactor; }

	/**
	 * If FizedSz is non-zero, zoom factor is ignored and FixedSz is use to hard-set size.
	 */
	void SetFixedSz(INT InFixedSz)						{ FixedSz = InFixedSz; }
	INT GetFixedSz() const								{ return FixedSz; }

	/**
	 * Returns the current primitive preference for this type
	 */
	EThumbnailPrimType GetPrimitive(void) const
	{
		return PrimitiveType;
	}

	/**
	 * Sets the current primitive preference for this type
	 */
	void SetPrimitive(EThumbnailPrimType InType)
	{
		PrimitiveType = InType;
	}

	/**
	 * Sets the default primitive preference
	 */
	static void SetDefaultPrimitive(EThumbnailPrimType InType)
	{
		DefaultPrimitiveType = InType;
	}

	/**
	 * Sets the default zoom factor preference
	 */
	static void SetDefaultZoomFactor(FLOAT InZoom)
	{
		DefaultZoomFactor = InZoom;
	}

	/**
	 * Sets the default fixed size preference
	 */
	static void SetDefaultFixedSize(INT InSize)
	{
		DefaultFixedSize = InSize;
	}

	/**
	 * Sets the default particle system real-time preference
	 */
	static void SetDefaultPSysRealTime(UBOOL InPSysRealTime)
	{
		bDefaultsPSysRealTime = InPSysRealTime;
	}

	/**
	 * This function loads the default values to use from the editor INI
	 */
	static void LoadDefaults(void);

	/**
	 * This function saves the default values to the editor INI
	 */
	static void SaveDefaults(void);

	/**
	 * Loads the per instance values from the editor INI. Uses defaults if they
	 * are missing
	 *
	 * @param KeyPrefix the prefix to apply to all keys that are loaded
	 */
	void LoadConfig(void);

	/**
	 * Saves the current values to the editor INI
	 *
	 * @param KeyPrefix the prefix to apply to all keys that are saved
	 */
	void SaveConfig(void);

private:
	INT ScrollMax, ScrollPos;
	FLOAT ZoomFactor;
	INT FixedSz;
	EGenericBrowser_TreeCtrlImages TreeCtrlImg;
	/**
	 * The primitive type that was last associated with this type
	 */
	EThumbnailPrimType PrimitiveType;
	/**
	 * The prefix to use when loading/saving settings from/to the INI file
	 */
	FString KeyPrefix;

// Statics used for saving the default values
	static FLOAT DefaultZoomFactor;
	static INT DefaultFixedSize;
	static EThumbnailPrimType DefaultPrimitiveType;
	static UBOOL bDefaultsAreInited;
	static UBOOL bDefaultsPSysRealTime;

	// Protect default ctor.
	WxBrowserData();
};


enum EUsageFilterContainerTypes
{
	USAGEFILTER_Content,
	USAGEFILTER_AllLevels,
	USAGEFILTER_CurrentLevel,
	USAGEFILTER_VisibleLevels,
	USAGEFILTER_MAX,
};

class WxGBLeftContainer : public wxPanel, public FSerializableObject
{
public:
	WxGBLeftContainer( wxWindow* InParent );
	virtual ~WxGBLeftContainer();

	class WxGenericBrowser* GenericBrowser;

	TMap<FString, wxTreeItemId> FolderMap;
	TMap<UPackage*,wxTreeItemId> PackageMap;


	/**
	 * Splitter between the list of resource types and the list of packages
	 */
	wxSplitterWindow* PackageSplitter;

	/**
	 * Splitter between the list of packages and the list of containers to use when the "in-use" filter is enabled.
	 */
	wxSplitterWindow* UsageFilterHSplitter;

	wxCheckBox* ShowAllCheckBox;
	wxCheckListBox* ResourceFilterList;
	WxTreeCtrl* TreeCtrl;

	/**
	 * list of available "containers" for generating a root set to be used by the "in-use" filtering
	 */
	wxCheckListBox* UsageFilterContainerList;

	TArray<UGenericBrowserType*> ResourceTypes;
	UGenericBrowserType* CurrentResourceType;

	/** Stores support information on all possible resource types. */
	UGenericBrowserType* ResourceAllType;

	/** Stores support information on all of the currently checked resource types. */
	UGenericBrowserType* ResourceFilterType;

	/** Flag for whether or not we are filtering nothing (a show all flag for resources). */
	UBOOL bShowAllTypes;

	/** Flag for when the RightContainer needs to be updated because the tree view changed. */
	UBOOL bTreeViewChanged;

	/**
	 * Gets a list of packages that are editor only and should not be displayed.
	 *
	 * @param	PackageList		Receives the list of editor only packages.
	 */
	static void GetEditorOnlyPackages(TArray<const UPackage*> &PackageList);

	/**
	 * Filters the global set of packages.
	 *
	 * @param	OutFilteredPackageMap		The map that will contain the filtered set of packages.
	 * @param	OutFilteredLevelPackages	The map that will contain the filtered set of level packages.
	 */
	void GetFilteredPackageList(TMap<const UPackage*, const UPackage*> &OutFilteredPackageMap, TMap<const UPackage*, const UPackage*> &OutFilteredLevelPackages, TArray<UPackage*> &OutPackageList);

	/**
	 * Gets a filtered list of group packages.
	 *
	 * @param	InFilteredLevelPackages		The filtered list of level packages
	 * @param	OutGroupPackages			The map that receives the filtered list of group packages.
	 */
	void GetFilteredGroupList(TMap<const UPackage*, const UPackage*> &InFilteredLevelPackages, TMap<UPackage*,UPackage*> &OutGroupPackages);

	/**
	 * Selects the package corresponding to the specified resource.
	 *
	 * @param	InObject	The resource to sync to.  Must be a valid pointer.
	 */
	void SyncToObject(UObject* InObject);

	/**
	  * Loads the window settings specific to this container.
	  */
	void LoadSettings();

	/**
	 * Recursively removes packages from the package map.
	 *
	 * @param	Id		The parent id to remove.
	 */
	void RecursivelyRemovePackages(const wxTreeItemId &Id);

	/**
	  * Saves the window settings specific to this container.
	  */
	void SaveSettings();

	/**
	 *  Sets the show all types flag, and enables and disables the check list as necessary.
	 */
	void SetShowAllTypes( UBOOL bIn );

	/**
	 * Toggles the visibility of the level list
	 *
	 * @param	bUsageFilterEnabled		specify TRUE to display the list of containers that can be enabled for the "in-use" filter
	 * @param	bInitialUpdate			specify TRUE when calling SetUsageFilter during initialization of this panel
	 */
	void SetUsageFilter( UBOOL bUsageFilterEnabled, UBOOL bInitialUpdate=FALSE );

	/**
	 * Updates the tree control that contains the list of loaded packages.
	 */
	void UpdateTree();

	void Update();

	void GetSelectedPackages(TArray<UPackage*>* InPackages) const;
	void GetSelectedTopLevelPackages(TArray<UPackage*>* InPackages) const;

	/**
	 * Returns the list of container objects that will be used to filter which resources are displayed when "Show Referenced Only" is enabled.
	 * Normally these will correspond to ULevels (i.e. the currently loaded levels), but may contain other packages such as script or UI packages.
	 *
	 * @param	out_ReferencerContainers	receives the list of container objects that will be used for filtering which objects are displayed in the GB
	 *										when "In Use Only" is enabled
	 */
	void GetReferencerContainers( TArray<UObject*>& out_ReferencerContainers );

	/**
	 * Requests that source control state be updated.
	 */
	void RequestSCCUpdate();

	void Serialize(FArchive& Ar);

private:
	/** TRUE if an SCC update request is pending. */
	UBOOL bIsSCCStateDirty;

	/**
	 * Determines which types of referencers are listed in the "referencers" listbox when the usage filter is enabled.
	 */
	UBOOL bUsageFilterContainers[USAGEFILTER_MAX];

	/**
	 * The position of the sash for the splitter between the package list and the usage filter container list
	 */
	INT UsageFilterSashPos;

	/**
	* Rebuilds the list of supported objects using the CurrentResourceType and then
	* calls the generic browser update function to update all of the UI elements.
	*/
	void ResourceTypeFilterChanged();

	/**
	 * Rebuilds the list of classes ResourceFilterType supports using the 
	 * currently checked items of the ResourceFilterList->
	 */
	void RegenerateCustomResourceFilterType();

	/**
	 * Clusters sounds into a cue through a random node, based on wave name, and optionally an attenuation node.
	 */
	void ClusterSounds(UBOOL bIncludeAttenuationNode);

	////////////////////
	// Wx events.

	void OnTreeSelChanged( wxTreeEvent& In );
	void OnTreeSelChanging( wxTreeEvent& In );

	/**
	* Event handler for when the user double clicks on a item in the check list.
	*/
	void OnResourceFilterListDoubleClick( wxCommandEvent& In );

	/**
	 * Event handler for when the user changes a checkbox in the resource filter list.
	 */
	void OnResourceFilterListChecked( wxCommandEvent& In );

	/**
	 * Called when the user double-clicks an item in the usage filter containers list.
	 */
	void OnUsageFilterContainerListDoubleClick( wxCommandEvent& Event );

	/**
	 * Called when the user checks/unchecks an item in the usage filter container list.
	 */
	void OnUsageFilterContainerListChecked( wxCommandEvent& Event );

	/** 
	 * Event handler for when the show all types checked box is changed. 
	 */
	void OnShowAllTypesChanged( wxCommandEvent &In );


	/**
	 * Event handler for when the UI system wants to update the ShowAllTypes check box. 
	 * This function forces the checkbox to resemble the state of the bShowAllTypes flag.
	 */

	void UI_ShowAllTypes( wxUpdateUIEvent &In );

	/**
	 * Event handler for the BatchProcess command.
	 */
	void OnBatchProcess( wxCommandEvent& In );

	/**
	 * Clusters sounds into a cue through a random node, based on wave name.
	 */
	void OnClusterSounds(wxCommandEvent& InEvent);

	/**
	 * Clusters sounds into a cue through a random node, based on wave name, and an attenuation node.
	 */
	void OnClusterSoundsWithAttenuation(wxCommandEvent& InEvent);

	/**
	 * Event handler for package error checking via the package tree view.
	 */
	void OnCheckPackageForErrors( wxCommandEvent& In );

	/**
	 * Event handler for "Set Folder..." context menu option
	 */
	void OnSetFolder( wxCommandEvent& InEvent );

	/**
	* Event handler for "Remove From Folder..." context menu option
	*/
	void OnRemoveFromFolder( wxCommandEvent& InEvent );

	/*
	 * Recursively updates the package list within the tree view.
	 *
	 * @param	ParentId				The current parent node.
	 * @param	InFilteredPackages		The filtered list of packages.
	 * @param	InGroupPackages			The list of groups.
	 * @param	TreeItemsToDeleteBuffer	A buffer containing package entries that are no longer valid and require deleting.
	 */
	void RecursivelyUpdatePackageList(const wxTreeItemId &ParentId, TMap<const UPackage*, const UPackage*> &InFilteredPackages, TMap<UPackage*,UPackage*> InGroupPackages, TArray<wxTreeItemId> &TreeItemsToDeleteBuffer);
	
	/**
	 * Updates the appearance of a package entry within the package tree view
	 *
	 * @param	Item	The package entry that needs updating.
	 */
	void UpdateTreeViewPackageItem(const wxTreeItemId &Item);

	/*
	 * Returns whether or not a package entry should be removed from the tree view.
	 *
	 * @param	Item					The item to be checked for removal.
	 * @param	InFilteredPackages		The filtered list of packages.
	 * @param	InGroupPackages			The list of groups.
	 */
	UBOOL RemoveItemFromPackageTree(const wxTreeItemId &Item, TMap<const UPackage*, const UPackage*> &InFilteredPackages, TMap<UPackage*,UPackage*> &InGroupPackages);

	/**
	 * Recursively copies a branch in the tree view to the provided parent node.
	 *
	 * @param	DestParent	The destination node that will become the parent of the source branch.
	 * @param	Src			The source branch to be copied.
	 */
	void CopyPackageTreeBranch(const wxTreeItemId& DestParent, const wxTreeItemId& Src);

	DECLARE_EVENT_TABLE();
};

enum ERightViewMode
{
	RVM_List		= 0,
	RVM_Preview		= 1,
	RVM_Thumbnail	= 2,
};

class WxGBRightContainer : public wxPanel, FViewportClient
{
private:
	friend class WxGenericBrowser;

	WxGBRightContainer( wxWindow* InParent );
	~WxGBRightContainer();

	class WxGenericBrowser* GenericBrowser;

	/** Controls how the resources are displayed */
	ERightViewMode ViewMode;
	
	/** If TRUE, objects shown in the browser viewport are sorted by name AND grouped by their class names. */
	UBOOL bGroupByClass;

	/** If TRUE, the browser viewport is being scrolled. */
	UBOOL bIsScrolling;

public:
	/** If TRUE, ignore Draw(...) requests. */
	UBOOL bSuppressDraw;

private:
	wxSplitterWindow *SplitterWindowV, *SplitterWindowH;
	class WxGBViewToolBar* ToolBar;
	wxListView* ListView;

	/** Displays the list of objects referencing the currently selected asset in the generic browser */
	wxListView* ReferencersList;

	class WxViewportHolder* ViewportHolder;
	wxStaticText* SelectionLabel;
	FViewport* Viewport;
	EThumbnailPrimType ThumbnailPrimType;
	WxDlgGenericBrowserSearch* SearchDialog;
	INT SplitterPosV, SplitterPosH;
	INT ReferencerNameColumnWidth, ReferencerCountColumnWidth;
	UBOOL bNeedsUpdate;
	FString NameFilter;

	/** This flag is set when the user has clicked and held down on a object with the left or middle mouse button */
	UBOOL bDraggingObject;

	/** if greater than 0, ignore calls to Update */
	INT SuppressUpdateMutex;

	/**
	 * This list of objects to add to the generic browser thumbnail view
	 */
	TArray<UObject*> VisibleObjects;
	/**
	 * This is the list of objects to draw when being filtered by list view
	 * selection
	 */
	TArray<UObject*> FilteredObjects;

public:
	/** @name FViewportClient interface */
	//@{
	/**
	 * Handles left and right mouse button input for the main generic browser window.
	 *
	 * @param	Key					Name of the key that was pressed.
	 * @param	Event				Event describing the key action.
	 * @param	AmountDepressed		Amount of key depression.
	 */
	virtual UBOOL InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed = 1.f,UBOOL bGamepad=FALSE);

	virtual UBOOL InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad=FALSE);
	/**
	 * Renders objects to the selected interface 
	 */
	virtual void Draw(FViewport* Viewport,FCanvas* Canvas);
	//@}

	enum EReferencerListColumnHeaders
	{
		COLHEADER_Name,
		COLHEADER_Count,
		COLHEADER_Class,
	};

	enum EListViewColumns
	{
		GB_LV_COLUMN_Object,
		GB_LV_COLUMN_Group,
		GB_LV_COLUMN_ResourceSize,
		GB_LV_COLUMN_Info0,
		GB_LV_COLUMN_Info1,
		GB_LV_COLUMN_Info2,
		GB_LV_COLUMN_Info3,
		GB_LV_COLUMN_Info4,
		GB_LV_COLUMN_Info5,
		GB_LV_COLUMN_Info6,
		GB_LV_COLUMN_Info7,
		GB_LV_COLUMN_Count
	};

	FListViewSortOptions SearchOptions;
	FListViewSortOptions ReferencerOptions;

	void LoadSettings();
	void SaveSettings();

	/**
	 * Fills the list view with items relating to the selected packages on the left side.
	 */
	void UpdateList();

	/**
	 * This updates the ReferencersList with the list of objects referencing the resources currently selected in the GB viewport or list.
	 */
	void UpdateReferencers( UObject* SelectedObject );
	void UpdateUI();
	void Update();

	/**
	 * Called by WxGenericBrowser::Update() when the browser is not shown to clear out unwanted state.
	 */
	void UpdateEmpty();

	void SetViewMode( ERightViewMode InViewMode );

	/**
	 * Toggles the visibility of the "referencers" list
	 */
	void SetUsageFilter( UBOOL bUsageFilterEnabled );

	/**
	 * Builds the list of objects to display with
	 */
	const TArray<UObject*>& GetVisibleObjects(void);

	/**
	 * Fills the OutObjects list with all valid objects that are supported by the current
	 * browser settings and that reside withing the set of specified packages.
	 *
	 * @param	InPackages			Filters objects based on package.
	 * @param	OutObjects			[out] Receives the list of objects
	 * @param	bIgnoreUsageFilter	specify TRUE to prevent dependency checking serialization; useful for retrieving objects from packages in situations the returned object
	 *								set isn't used for displaying stuff in the GB.
	 */
	void GetObjectsInPackages( const TArray<UPackage*> &Packages, TArray<UObject*> &OutObjects, UBOOL bIgnoreUsageFilter=FALSE );

	/**
	 * Display a popup menu for the selected objects.  If the objects vary in type it will show a generic pop up menu instead of a object specific one.
	 */
	void ShowContextMenu();

	/**
	 * Brings up a context menu that allows creation of new objects using Factories.
	 * Serves as an alternative to choosing 'New' from the File menu.
	 */
	void ShowNewObjectMenu();

	/**
	 * Selects the specified resource and scrolls the viewport so that resource is in view.
	 *
	 * @param	InObject		The resource to sync to.  Must be a valid pointer.
	 * @param	bDeselectAll	If TRUE (the default), deselect all objects the specified resource's class.
	 */
	void SyncToObject(UObject* InObject, UBOOL bDeselectAll=TRUE);

	/** If this is non-NULL, the viewport will be scrolled to make sure this object is in view when it draws next. */
	UObject* SyncToNextDraw;

	/**
	 * Empties the arrays that hold object pointers since they aren't
	 * serialized. Marks the class as needing refreshed too
	 */
	void ClearObjectReferences(void);

	/**
	 * Removes objects from the selection set that aren't currently visible to the user.
	 */
	void RemoveInvisibleObjects();

	/**
	 * Increases the mutex which controls whether Update() will be executed.  Should be called just before beginning an operation which might result in a
	 * call to Update, if the calling code must manually call Update immediately afterwards.
	 */
	void DisableUpdate()
	{
		SuppressUpdateMutex++;
	}

	/**
	 * Decreases the mutex which controls whether Update() will be executed.
	 */
	void EnableUpdate()
	{
		SuppressUpdateMutex--;
		check(SuppressUpdateMutex>=0);
	}

private:

	////////////////////
	// Wx events.

	void OnSize( wxSizeEvent& In );
	void OnViewMode( wxCommandEvent& In );
	void OnSearch( wxCommandEvent& In );
	void OnGroupByClass( wxCommandEvent& In );
	void OnPSysRealTime( wxCommandEvent& In );
	void UI_ViewMode( wxUpdateUIEvent& In );
	void UI_UsageFilter( wxUpdateUIEvent& In );
	void OnPrimType( wxCommandEvent& In );
	void UI_PrimType( wxUpdateUIEvent& In );
	void UI_GroupByClass( wxUpdateUIEvent& In );
	void UI_PSysRealTime( wxUpdateUIEvent& In );
	void UI_SelectionInfo( wxUpdateUIEvent& In );
	void OnSelectionChanged_Select( wxListEvent& In );
	void OnSelectionChanged_Deselect( wxListEvent& In );
	void OnListViewRightClick( wxListEvent& In );
	void OnListItemActivated( wxListEvent& In );
	void OnListColumnClicked( wxListEvent& In );

	/** called when the user double-clicks an entry in the "referencer" list */
	void OnReferencerListViewActivated( wxListEvent& Event );

	/** called when the user clicks a column header in the "referencer" list */
	void OnReferencerListColumnClicked( wxListEvent& Event );

	void OnZoomFactorSelChange( wxCommandEvent& In );
	void OnNameFilterChanged( wxCommandEvent& In );
	void OnScroll( wxScrollEvent& InEvent );
	/** Resource input command event */
	void OnAtomicResourceReimport( wxCommandEvent& In );

	/**
	 * Called when the user toggles the "Display only referenced assets" button.  Updates the value of bShowReferencedOnly
	 */
	void OnUsageFilter( wxCommandEvent& Event );

	DECLARE_EVENT_TABLE();
};

class WxGenericBrowser :
	public WxBrowser,
	// This class holds an object reference so it needs to be serialized
	public FSerializableObject
{
	DECLARE_DYNAMIC_CLASS(WxGenericBrowser);

public:
	WxGenericBrowser(void);
	virtual ~WxGenericBrowser(void);

	/**
	 * Forwards the call to our base class to create the window relationship.
	 * Creates any internally used windows after that
	 *
	 * @param DockID the unique id to associate with this dockable window
	 * @param FriendlyName the friendly name to assign to this window
	 * @param Parent the parent of this window (should be a Notebook)
	 */
	virtual void Create(INT DockID,const TCHAR* FriendlyName,wxWindow* Parent);

	TMap<UClass*,WxBrowserData*> BrowserData;				// Individual data for each type of browser

	WxGBLeftContainer* LeftContainer;
	WxGBRightContainer* RightContainer;
	wxSplitterWindow* SplitterWindow;
	wxImageList *ImageListTree;
	wxImageList *ImageListView;
	FString LastExportPath;
	FString LastImportPath;
	FString LastOpenPath;
	FString LastSavePath;

	/** True if memory stats should be shown in generic browser thumbnails */
	UBOOL bShowMemoryStats;

	/** True if we should draw Content Tag labels in generic browser thumbnails */
	UBOOL bShowContentTags;

	INT NumSelectedObjects;
	/** Keeps track of the filter index for the last filter used in the import files dialog box. */
	INT LastImportFilter;

	/**
	 * Controls whether the generic browser displays all loaded assets or only those assets referenced by the selected levels.
	 */
	UBOOL bShowReferencedOnly;

	/**
	 * TRUE if an update was requested while the browser tab wasn't active.
	 * The browser will Update() the next time this browser tab is Activated().
	 */
	UBOOL bUpdateOnActivated;

	// Array of UFactory classes which can create new objects.
	TArray<UClass*>	FactoryNewClasses;

	// Always returns a valid pointer.
	WxBrowserData* FindBrowserData( UClass* InClass );

	/**
	 *	Traverses the tree recursively in a attempt to locate a package.
	 *
	 *	@param	TreeId		The Tree item ID we are searching recursively.
	 *  @param	Cookie		Cookie used to find a child in a tree control.
	 *	@param	InPackage	Package to search for.
	 *
	 *	@return	TRUE if we found the package, FALSE otherwise.
	 */
	bool RecurseFindPackage( wxTreeItemId TreeId, wxTreeItemIdValue Cookie, UPackage *InPackage );

	/**
	 *	Finds and selects InPackage in the tree control
	 *
	 *	@param	InPackage		Package to search for.
	 */
	void SelectPackage( UPackage* InPackage );

	virtual void InitialUpdate();
	virtual void Update();
	virtual void Activated();
	UPackage* GetTopPackage();
	UPackage* GetGroup();

	/**
	 * Requests that source control state be updated.
	 */
	void RequestSCCUpdate();

	/**
	* Loads a package and handles housekeeping like the MRU list.
	*
	* @param	InFilename		The name of the package to load.
	*
	* @return	A pointer to the package that was loaded.  NULL if loading failed.
	*/
	UPackage* LoadPackage( FFilename InFilename );

	/**
	 * Imports all of the files in the string array passed in.
	 *
	 * @param InFiles					Files to import
	 * @param InFactories				Array of UFactory classes to be used for importing.
	 * @return							Returns TRUE if ImportFiles() imported at least 1 file.
	 */
	UBOOL ImportFiles( const wxArrayString& InFiles, const TArray<UFactory*> &InFactories );

	/**
	 * Selects the resources in the specified array and scrolls the viewport so that at least one selected resource is in view.
	 *
	 * @param	Objects		The array of resources to sync to.  All object pointers must be valid.
	 */
	void SyncToObjects(TArray<UObject*> Objects);

	/**
	 * @return Returns the number of objects currently selected.
	 */
	INT GetNumSelectedObjects() const
	{
		return NumSelectedObjects;
	}

	/**
	 * Returns the key to use when looking up values
	 */
	virtual const TCHAR* GetLocalizationKey(void) const
	{
		return TEXT("GenericBrowser");
	}

	/**
	 * Saves the WxBrowserData state in addition to base window state
	 */
	virtual void SaveWindowState(void);

	/**
	 * Determines if the selected resource is the default one
	 */
	UBOOL IsDefaultResourceSelected(void)
	{
		return (LeftContainer->CurrentResourceType ==
			LeftContainer->ResourceAllType);
	}

	UBOOL IsUsageFilterEnabled() const
	{
		return bShowReferencedOnly;
	}

	/**
	 * Returns the label renderer to use for rendering
	 */
	FORCEINLINE UMemCountThumbnailLabelRenderer* GetMemCountLabelRenderer(void)
	{
		// Construct the object if we don't already have one
		if (LabelRenderer == NULL)
		{
			LabelRenderer = ConstructObject<UMemCountThumbnailLabelRenderer>(UMemCountThumbnailLabelRenderer::StaticClass());
		}
		check(LabelRenderer);
		return bShowMemoryStats == TRUE ? LabelRenderer : NULL;
	}

	/**
	 * Copies references for selected generic browser objects to the clipboard.
	 */
	void CopyReferences() const;

	/**
	 * Returns the list of container objects that will be used to filter which resources are displayed when "Show Referenced Only" is enabled.
	 * Normally these will correspond to ULevels (i.e. the currently loaded levels), but may contain other packages such as script or UI packages.
	 *
	 * @param	out_ReferencerContainers	receives the list of container objects that will be used for filtering which objects are displayed in the GB
	 *										when "In Use Only" is enabled
	 */
	void GetReferencerContainers( TArray<UObject*>& out_ReferencerContainers );

	/**
	 * Builds a list of source objects which should be used for determining which assets to display in the GB when "In Use Only" is displayed.  Only assets referenced
	 * by this set will be displayed in the GB.
	 *
	 * @param	out_RootSet		receives the list of objects that will be searched for referenced assets.
	 * @param	RootContainers	if specified, the list of container objects to use for generating the dependency root set.  If not specified, uses the result of GetReferencerContainers.
	 */
	void BuildDependencyRootSet( TArray<UObject*>& out_RootSet, TArray<UObject*>* RootContainers=NULL );

	/**
	 * Marks all objects which are referenced by objects contained in the current filter set with RF_TagImp
	 *
	 * @param	out_RootSet		if specified, receives the array of objects which were used as the root set when performing the reference checking
	 */
	void MarkReferencedObjects( TArray<UObject*>* out_RootSet=NULL );

	// FSerializableObject interface

	/**
	 * Since this class holds onto an object reference, we need to serialize it
	 * so that we don't have it GCed underneath us
	 *
	 * @param Ar The archive to serialize with
	 */
	void Serialize(FArchive& Ar);

	/**
	 * Returns an USelection for the set of objects selected in the generic browser.
	 */
	static class USelection* GetSelection();

	/*
	 * Called by UEditorEngine::Init() to initialize shared generic browser selection structures.
	 */
	static void InitSelection();

	/*
	 * Called by UEditorEngine::Destroy() to tear down shared generic browser selection structures.
	 */
	static void DestroySelection();

	/**
	 * Exports the specified objects to file.
	 *
	 * @param	ObjectsToExport		The set of objects to export.
	 * @param	bPromptFilenames	If TRUE, prompt individually for filenames.  If FALSE, bulk export to a single directory.
	 */
	void ExportObjects(const TArray<UObject*>& ObjectsToExport, UBOOL bPromptFilenames);

	/**
	 *	Exports the given packages to files.
	 *
	 *	@param	PackagesToExport	The set of packages to export.
	 */
	void ExportPackages(const TArray<UPackage*>& PackagesToExport);

	/**
	 * Handles fully loading passed in packages.
	 *
	 * @param	TopLevelPackages	Packages to be fully loaded.
	 * @param	OperationString		Localization key for a string describing the operation; appears in the warning string presented to the user.
	 * 
	 * @return TRUE if all packages where fully loaded, FALSE otherwise
	 */
	UBOOL HandleFullyLoadingPackages( const TArray<UObject*>& Objects, const TCHAR* OperationString );

	/**
	 * Handles fully loading passed in packages.
	 *
	 * @param	TopLevelPackages	Packages to be fully loaded.
	 * @param	OperationString		Localization key for a string describing the operation; appears in the warning string presented to the user.
	 * 
	 * @return TRUE if all packages where fully loaded, FALSE otherwise
	 */
	UBOOL HandleFullyLoadingPackages( const TArray<UPackage*>& TopLevelPackages, const TCHAR* OperationString );

	/**
	 * Attempts to save all selected packages.
	 *
	 * @return		TRUE if all selected packages were successfully saved, FALSE otherwise.
	 */
	UBOOL SaveSelectedPackages();

	/**
	 * Attempts a 'save as' operation on all selected packages.
	 *
	 * @return		TRUE if all selected packages were successfully saved, FALSE otherwise.
	 */
	UBOOL SaveAsSelectedPackages();

	/**
	 * Returns whether saving the specified package is allowed
	 */
	UBOOL AllowPackageSave( UPackage* PackageToSave );

	/**
	 * Deletes the list of objects
	 *
	 * @param ObjectsToDelete The list of objects to delete
	 *
	 * @return The number of objects successfully deleted
	 */
	INT DeleteObjects( const TArray< UObject* >& ObjectsToDelete );

	/**
	* If any cooked packages are found throw up an error and return TRUE.
	*
	* @param	Packages	The list of packages to check for cooked status.
	*
	* @return				TRUE if cooked packages were found; false otherwise.
	*/
	static UBOOL DisallowOperationOnCookedPackages(const TArray<UPackage*>& Packages);


private:
	void LoadSettings();
	void SaveSettings();

	/**
	 * Used to add the memory usage for the object to the labels
	 */
	UMemCountThumbnailLabelRenderer* LabelRenderer;

	// FCallbackEventDevice interface

	/**
	 * Called when there is a Callback issued.
	 * @param InType	The type of callback that was issued.
	 */
	virtual void Send( ECallbackEventType InType );

	/**
	 * Called when there is a Callback issued.
	 * @param InType	The type of callback that was issued.
	 * @param InObject	Object that was modified.
	 */
	virtual void Send( ECallbackEventType InType, UObject* InObject );

	/**
	 * Callback for when the Generic Browser selection set changes.
	 */
	void SelectionChangeNotify();

	/**
	 * Attempts to save the specified packages; helper function for by e.g. SaveSelectedPackages().
	 *
	 * @param		InPackages					The content packages to save.
	 * @param		bUnloadPackagesAfterSave	If TRUE, unload each package if it was saved successfully.
	 * @return									TRUE if all packages were saved successfully, FALSE otherwise.
	 */
	UBOOL SavePackages(const TArray<UPackage*>& InPackages, UBOOL bUnloadPackagesAfterSave);

	/** Helper function that attempts to check out the specified top-level packages. */
	void CheckOutTopLevelPackages(const TArray<UPackage*> Packages) const;

	/** Helper function that attempts to unlaod the specified top-level packages. */
	void UnloadTopLevelPackages(const TArray<UPackage*> TopLevelPackages);

	/** Localisation helpers */
	void RenameObjectsWithRefs( TArray<UObject*>& Objects, UBOOL bLocPackages );
	void AddLanguageVariants( TArray<UObject*>& Objects, USoundNodeWave* Object );

	////////////////////
	// Wx events.

	void OnSize( wxSizeEvent& In );
	/**
	 * Handler for IDM_RefreshBrowser events; updates the browser contents.
	 */
	void OnRefresh( wxCommandEvent& In );

	/**
	 * Event handler for when the user selects an item from the Most Recently Used menu.
	 */
	void MenuFileMRU( wxCommandEvent& In );

	void OnFileOpen( wxCommandEvent& In );
	void OnFileOpenPackages( wxCommandEvent& In );
	void OnFileSave( wxCommandEvent& In );
	void OnFileSaveAs( wxCommandEvent& In );
	void OnFileFullyLoad( wxCommandEvent& In );
	void OnFileNew( wxCommandEvent& In );
	void OnFileImport( wxCommandEvent& In );
	void OnSCCRefresh( wxCommandEvent& In );
	void OnSCCHistory( wxCommandEvent& In );
	void OnSCCMoveToTrash( wxCommandEvent& In );
	void OnSCCAdd( wxCommandEvent& In );
	void OnSCCCheckOut( wxCommandEvent& In );
	void OnSCCCheckIn( wxCommandEvent& In );
	void OnSCCRevert( wxCommandEvent& In );
	void UI_SCCAdd( wxUpdateUIEvent& In );
	void UI_SCCCheckOut( wxUpdateUIEvent& In );
	void UI_SCCCheckIn( wxUpdateUIEvent& In );
	void UI_SCCRevert( wxUpdateUIEvent& In );

	void OnUnloadPackage(wxCommandEvent& In);

	void OnCopyReference(wxCommandEvent& In);
	void OnObjectProperties(wxCommandEvent& In);
	void OnObjectDelete(wxCommandEvent& In);
	void OnObjectShowReferencers(wxCommandEvent& In);
	void OnObjectExport(wxCommandEvent& In);
	void OnBulkExport(wxCommandEvent& In);
	void OnFullLocExport(wxCommandEvent& In);
	void OnResliceFracturedMeshes(wxCommandEvent& In);

	void OnContextObjectNew(wxCommandEvent& In);
	void OnObjectEditor(wxCommandEvent& In);
	void OnObjectEditNodes(wxCommandEvent& In);
	void OnObjectCustomCommand( wxCommandEvent& In );
	void OnObjectResliceFracturedMeshes( wxCommandEvent& In );

	void OnObjectDeleteRedirects( wxCommandEvent& In );
	void OnObjectDuplicateWithRefs( wxCommandEvent& In );
	void OnObjectRenameWithRefs( wxCommandEvent& In );
	void OnObjectRenameLocWithRefs( wxCommandEvent& In );
	void OnObjectDeleteWithRefs( wxCommandEvent& In );
	void OnObjectShowReferencedObjs( wxCommandEvent& In );

	void OnAddTag( wxCommandEvent& In );

	/**
	 * Toggles the internal variable that manages whether memory counting is
	 * on or not.
	 *
	 * @param In the event to process
	 */
	void OnShowMemStats(wxCommandEvent& In);
	/**
	 * Updates the menu's check state based upon whether memory counting is
	 * on or not
	 *
	 * @param In the event to set the checked or unchecked state on
	 */
	void UI_ShowMemStats(wxUpdateUIEvent& In);

	/**
	 * Toggles whether content tags are visible in thumbnails.  Called from a Wx menu event.  
	 *
	 * @param In the event to process
	 */
	void OnShowContentTags( wxCommandEvent& In );

	/**
	 * Updates the menu's check state based upon whether content tags are visible
	 *
	 * @param In the event to set the checked or unchecked state on
	 */
	void UI_ShowContentTags( wxUpdateUIEvent& In );

	/**
	 * Sets the background color for the material preview thumbnails.
	 * 
	 * @param	In	The event being processed
	 */
	void OnSetMaterialPreviewBackgroundColor( wxCommandEvent& In );

	/**
	 * Sets the background color for the translucent material preview thumbnails.
	 * 
	 * @param	In	The event being processed
	 */
	void OnSetTranslucentMaterialPreviewBackgroundColor( wxCommandEvent& In );

	/**
	 * Toggles the internal variable that manages whether StreamingBounds are being displayed or not.
	 *
	 * @param In the event to process
	 */
	void OnShowStreamingBounds(wxCommandEvent& In);

	/**
	 * Updates the menu's check state based upon whether StreamingBounds are being displayed or not
	 *
	 * @param In the event to set the checked or unchecked state on
	 */
	void UI_ShowStreamingBounds(wxUpdateUIEvent& In);

	/**
	 * Called when the user toggles the "Display only referenced assets" button.  Updates the value of bShowReferencedOnly
	 */
	void OnUsageFilter( wxCommandEvent& Event );

	/** Simplifies (or resimplifies) the selected static mesh for the current level */
	void OnSimplifyStaticMesh( wxCommandEvent& In );

	/**
	 * Brings up a color picker dialog for InOutColor; the custom color selector will be expanded.
	 * 
	 * @param InOutColor	An FColor& that is being changed by the color picker.
	 */
	void PickColorFor(FColor& InOutColor);

	DECLARE_EVENT_TABLE();
};

/** Dialog for adding content tags to assets. */
class WxDlgAddTag : public wxDialog
{
	wxCheckListBox *TagList;
	FTagInfo *TagInfo;
	TArray<FName> SortedTags;

public:
	WxDlgAddTag(FTagInfo* InTagInfo, TArray<FName> &SelectedTags);
	~WxDlgAddTag();

	void GetSelectedTags(TArray<FName> &OutTags);
};

#endif // __GENERICBROWSER_H__
