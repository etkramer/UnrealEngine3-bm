#ifndef __TAGBROWSER_H__
#define __TAGBROWSER_H__

class WxTagBrowser : public WxBrowser, public FViewportClient, public FSerializableObject
{
	DECLARE_DYNAMIC_CLASS(WxTagBrowser);

public:
	WxTagBrowser();
	virtual ~WxTagBrowser();

	/**
	* Forwards the call to our base class to create the window relationship.
	* Creates any internally used windows after that
	*
	* @param DockID the unique id to associate with this dockable window
	* @param FriendlyName the friendly name to assign to this window
	* @param Parent the parent of this window (should be a Notebook)
	*/
	virtual void Create(INT DockID,const TCHAR* FriendlyName,wxWindow* Parent);

	/** Returns the key to use when looking up values. */
	virtual const TCHAR* GetLocalizationKey() const;

	virtual void Update();
	virtual void InitialUpdate()
	{
		Update();
	}
	virtual void Send(ECallbackEventType Event);

	/**
	* Repaints the viewport with the thumbnail data
	*
	* @param Viewport the viewport to draw in
	* @param RI the render interface to draw with
	*/
	virtual void Draw(FViewport* Viewport,FCanvas* Canvas);

	virtual UBOOL InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed = 1.f,UBOOL bGamepad=FALSE);

	USelection* GetSelection();

	// FSerializableObject interface
	void Serialize(FArchive& Ar);

protected:
	/** List of known asset types with tags */
	wxComboBox *AssetTypes, *SearchType;
	/** List of required tags for filtering content */
	wxCheckListBox *RequiredTagList;
	/** List of optional tags for filtering content */
	wxCheckListBox *OptionalTagList;
	/** Main split between tag list and viewport */
	wxSplitterWindow *SplitterWindow;
	/** Window for drawing the thumbnails */
	class WxViewportHolder *ViewportHolder;
	/** The viewport that the holder will draw into	*/
	FViewport *Viewport;
	/** Tag matching results label */
	wxStaticText *TagResultsText;
	/** Auto-load assets checkbox */
	wxCheckBox *AutoLoad;
	/** Manual load assets button */
	wxButton *LoadButton;

	class UContentTagIndex *TagIndexObject;

	USelection *Selection;

	/** List of referenced assets */
	struct FTagAssetReference
	{
		FString AssetName;
		UObject* Asset;

		friend FArchive& operator<<(FArchive &Ar, FTagAssetReference &Ref)
		{
			Ar << Ref.Asset;
			return Ar;
		}
	};
	TArray<FTagAssetReference> Assets;

	/** Currently selected asset used to prevent rebuilding the list if no selection change */
	FString CurrentAssetTypeString;

	/** Matching contents of the RequiredTagList checklist control */
	TArray<FName> RequiredTagListNames;

	/** Matching contents of the OptionalTagList checklist control */
	TArray<FName> OptionalTagListNames;


private:
	DECLARE_EVENT_TABLE();

	void OnSize(wxSizeEvent& In);
	void OnRefresh( wxCommandEvent& In );
	void OnAssetTypeChange(wxCommandEvent &In);
	void OnTagListChange(wxCommandEvent &In);
	void OnFilterListChange(wxCommandEvent &In);
	void OnScroll(wxScrollEvent& InEvent);
	void OnLoadButtonClick(wxCommandEvent &In);
	void OnAutoLoadChange(wxCommandEvent &In);

	void OnAddTag(wxCommandEvent &In);
	void OnSyncGB(wxCommandEvent &In);

	void BuildAndLoadAssetList(UContentTagIndex::AssetMapType &AssetTypeMap);
	void GetCurrentAssetAndTagSelections(UClass **OutAssetType, TArray<FName> &OutReqTagListNames, TArray<FName> &OutOptionalTagListNames);
};


#endif // __TAGBROWSER_H__
