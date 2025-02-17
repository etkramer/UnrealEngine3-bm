/*=============================================================================
	UIDataStorebrowser.h: Panel with a treeview and listview that lets the user view and manipulate UI datastores.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UIDATASTOREBROWSER_H__
#define __UIDATASTOREBROWSER_H__

typedef TMap<UUIDataProvider*,wxTreeItemId>		ProviderTreeMap;

/**
 * Dialog that holds the datastore panels.
 */
class WxDlgUIDataStoreBrowser : public wxFrame
{
public:
	WxDlgUIDataStoreBrowser(wxWindow* InParent, UUISceneManager* InSceneManager);
	virtual ~WxDlgUIDataStoreBrowser();

	/** Shows the dialog. */
	virtual bool Show(bool show=true);

	/**
	 * @return	TRUE if there is a valid tag selected.
	 */
	UBOOL IsTagSelected() const;

	/** 
	 * Returns the currently selected tag in OutTagName. 
	 * @return TRUE if there is a tag selected, FALSE otherwise.
	 */
	UBOOL GetSelectedTag(FName &OutTagName);

	/** 
	 * @return the path to the current tag if one is selected, or a blank string if one is not selected.
	 */
	FString GetSelectedTagPath() const;

	/**
	 * @return	the provider field type for the currently selected tag, or DATATYPE_MAX if no tags are selected
	 */
	EUIDataProviderFieldType GetSelectedTagType() const;

	/** 
	 * @return Returns the selected dataprovider for the selected tag if there is one, otherwise NULL.
	 */
	UUIDataProvider* GetSelectedTagProvider();

	/** 
	 * @return Returns the selected dataprovider using the tree control if there is one, otherwise NULL.
	 */
	UUIDataProvider* GetSelectedTreeProvider();

	/**
	 * @param	DataProvider		Provider to find the owning datastore for.
	 * @return	Returns the data store that contains the specified data provider.
	 */
	UUIDataStore* GetOwningDataStore(UUIDataProvider* DataProvider);

	/**
	 * Copies the markup string for the selected tag to the clipboard 
	 */
	void CopySelectedMarkupToClipboard();

private:


	/** Save window position and other settings for the dialog. */
	void SaveSettings() const;

	/** Load window position and other settings for the dialog. */
	void LoadSettings();

	/** OnClose Event, Saves Settings for the dialog. */
	void OnClose(wxCloseEvent &Event);

	/** Adds a new datastore tag to the selected provider. */
	void OnAddDatastoreTag( wxCommandEvent& Event );

	/** Event that calls CopySelectedMarkupToClipboard */
	void OnCopyDataStoreMarkup( wxCommandEvent& Event );

	wxNotebook* DataStoreNotebook;

	/** Panel that lets users edit non-string datastores. */
	class WxUIDataStorePanel* OthersPanel;

	/** Panel that lets users edit string datastores. */
	class WxUIStringDataStorePanel* StringsPanel;


	DECLARE_EVENT_TABLE()
};

/** Base class for datastore panels */
class WxUIDataStorePanel : public wxPanel
{
public:
	WxUIDataStorePanel(wxWindow* InParent,UBOOL bInDisplayNestedTags=FALSE);
	WxUIDataStorePanel(wxWindow* InParent, UUISceneManager* InSceneManager,UBOOL bInDisplayNestedTags=FALSE);
	virtual ~WxUIDataStorePanel() {}

	/** Loads settings for this panel. */
	virtual void LoadSettings();
	
	/** Saves settings for this panel. */
	virtual void SaveSettings();

	/** Adds a tag to the datastore. */
	virtual void AddTag() 
	{
		// Intentionally empty.
	}

	/**
	 * Iterates over all of the available datastores and adds them to the tree.
	 */
	void RefreshTree();

	/**
	 * Resets focus to the filter textbox for quick searching.
	 */
	void ResetFocus()
	{
		FilterText->SetFocus();
	}

	
	/**
	 * @return	TRUE if there is a valid tag selected.
	 */
	UBOOL IsTagSelected() const;

	/** 
	 * Returns the currently selected tag in OutTagName. 
	 * @return TRUE if there is a tag selected, FALSE otherwise.
	 */
	UBOOL GetSelectedTag(FName &OutTagName);

	/** 
	 * @return the path to the current tag if one is selected, or a blank string if one is not selected.
	 */
	FString GetSelectedTagPath() const;

	/**
	* @return	the provider field type for the currently selected tag, or DATATYPE_MAX if no tags are selected
	 */
	EUIDataProviderFieldType GetSelectedTagType() const;

	/** 
 	 * @return Returns the dataprovider for the selected tag if there is one, otherwise NULL.
	 */
	UUIDataProvider* GetSelectedTagProvider();

	/** 
 	 * @return Returns the selected dataprovider using the tree control if there is one, otherwise NULL.
	 */
	UUIDataProvider* GetSelectedTreeProvider();

	/**
	 * @param	DataProvider		Provider to find the owning datastore for.
	 * @return	Returns the data store that contains the specified data provider.
	 */
	UUIDataStore* GetOwningDataStore( UUIDataProvider* DataProvider );

protected:



	/** @return Returns whether or not this panel supports a datastore and should display it in its datastore tree. */
	virtual UBOOL DataStoreSupported(const UUIDataProvider* DataStore) const;

	/**
	 * Generates a list of tags using the current tag list root.
	 */
	void RefreshList();

	/**
 	 * Wrapper for getting the data provider associated with the specified tree id.
	 */
	UUIDataProvider* GetProviderFromTreeId( const wxTreeItemId& ProviderId );

	/** 
	 * Adds the data provider and all of its children to the tree.
	 *
	 * @param ParentId		Parent of the provider we are adding.
	 * @param ProviderData	Provider to add to the tree ctrl.
	 * @param CurrentPath	String path to the current dataprovider.
	 */
	void AddDataProviderToTree(wxTreeItemId &ParentID, FUIDataProviderField& ProviderData, const FString &CurrentPath);

	/**
	 * Generates a list of tags using the provided data store. Adds the tags to the list and then recursively adds the tags for the children of this data provider to the list.
	 *
	 * @param	ProviderId		DataProvider to use as a source for generating properties.
	 * @param	TotalProviders	the total number of providers that we need to add tags for; used to update the status bar
	 * @param	ProgressCount	the number of providers that have been added to the list so far; used to update the status bar
	 */
	void AddDataProviderTagsToList( const wxTreeItemId& ProviderId, const INT TotalProviders, INT& ProgressCount );

	/**
	 * Generates tags for a dataprovider and stores them in a lookup map.
	 *
	 * @param DataProvider		DataProvider to generate tags for.
	 * @param CurrentPath	String path to the current dataprovider.
	 */
	void GenerateTags(UUIDataProvider* DataProvider, const FString &CurrentPath);

	/** Updates the tags list. */
	void OnTreeSelectionChanged(wxTreeEvent& Event);

	/** Reapplies a filter to the set of visible tags. */
	void OnFilterTextChanged(wxCommandEvent& Event);

	/** Struct that holds info for a tag. */
	struct FTagInfo
	{
		FTagInfo( const FName &InTagName, const FString &InTagPath, EUIDataProviderFieldType InTagType, const FString& InTagMarkup )
		: TagName(InTagName)
		, TagMarkup(InTagMarkup)
		, TagPath(InTagPath)
		, TagType(InTagType)
		{
			TagTypeString = UUIRoot::GetDataProviderFieldTypeText(TagType);
			if ( TagTypeString.InStr(TEXT("DATATYPE_")) != INDEX_NONE )
			{
				// remove the DATATYPE_ prefix from the field type text
				TagTypeString = TagTypeString.Mid(9);
			}
		}

		FName TagName;
		FString TagMarkup;
		FString TagPath;
		FString TagTypeString;
		EUIDataProviderFieldType TagType;
	};

	TMultiMap<UUIDataProvider*, FTagInfo> TagMap;

	/** Tree control that shows the user all of the currently available data stores. */
	WxTreeCtrl* DataStoreTree;

	/** Splitter window to hold the tree and list controls. */
	wxSplitterWindow* SplitterWnd;

	/** Pointer to the currently selected data provider. */
	UUIDataProvider* CurrentDataProvider;

	/** maps data providers to their corresponding tree id */
	ProviderTreeMap	ProviderIDMap;

	/** List that displays all of the properties of the currently selected data store. */
	WxListView* TagsList;

	/** Filter for currently visible tags. */
	wxTextCtrl* FilterText;

	/** Pointer to the scene manager. */
	UUISceneManager* SceneManager;

	/** Current filter to use to filter the tags by. */
	FString CurrentFilter;

	/** Root item to use for the tag list. */
	wxTreeItemId TagListRoot;

	/** Determines whether tags for nested providers are also displayed in this panel when displaying the tags for a provider which contains other providers */
	UBOOL bDisplayNestedTags;

	DECLARE_EVENT_TABLE()
};

/** Panel that is used for string datastores, it contains a Value text control that lets users edit the value of a tag */
class WxUIStringDataStorePanel : public WxUIDataStorePanel
{
public:
	WxUIStringDataStorePanel(wxWindow* InParent,UBOOL bInDisplayNestedTags=TRUE);
	WxUIStringDataStorePanel(wxWindow* InParent, UUISceneManager* InSceneManager,UBOOL bInDisplayNestedTags=TRUE);
	virtual ~WxUIStringDataStorePanel() {}

	/** Loads settings for this panel. */
	virtual void LoadSettings();

	/** Saves settings for this panel. */
	virtual void SaveSettings();

	/** Adds a tag to the datastore. */
	virtual void AddTag();
private:

	/** Saves a config tag, checks out the loc file if necessary. */
	UBOOL SaveTag(const TCHAR *InSection, const TCHAR *InTag, const TCHAR *InValue, UUIConfigFileProvider* ConfigProvider);

	/** Resets the value of the value text box using the currently selected tag. */
	void ResetValue();

	/** Called when the user clicks on a tag in the list. */
	virtual void OnListSelectionChanged(wxListEvent &Event);

	/** Called when the user clicks on the accept change button. */
	void OnApplyChanges(wxCommandEvent &Event);

	/** Cancels changes that were made to the value for a tag. */
	void OnCancelChanges(wxCommandEvent& Event);

	/** Update UI callback for this panel. */
	void OnUpdateUI(wxUpdateUIEvent &Event);

	/** Called when the user modifies the value for a tag. */
	void OnValueTextChanged(wxCommandEvent& Event);

	/** @return Returns whether or not this panel supports a datastore and should display it in its datastore tree. */
	virtual UBOOL DataStoreSupported(const UUIDataProvider* DataStore) const;

	/** Filter the value of the currently selected tag. */
	wxTextCtrl* Value;

	/** Splits the tags list and value portions of the panel. */
	wxSplitterWindow* SplitterValue;

	/** Whether or not the user has made a change to the value of a tag. */
	UBOOL bMadeChange;

	/** Currently selected config section. */
	FString CurrentSection;

	DECLARE_EVENT_TABLE()
};

#endif

