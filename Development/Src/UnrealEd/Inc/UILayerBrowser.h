/*=============================================================================
	UILayerBrowser.h: UI Layer Browser window and control declarations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UILAYERBROWSER_H__
#define __UILAYERBROWSER_H__

typedef TMap<struct FUILayerNode*, wxTreeItemId> LayerTreeMap;

/* ==========================================================================================================
	WxUILayerTreeObjectWrapper
========================================================================================================== */

class WxUILayerTreeObjectWrapper : public WxTreeObjectWrapper
{
public:
	WxUILayerTreeObjectWrapper(class UObject* LayerObject, class UUILayer* ParentLayer);

	UBOOL IsUILayerRoot();
	UBOOL IsUILayer();
	UBOOL IsUIObject();

	struct FUILayerNode* GetObject();

	class UUILayer* GetLayerParent() const { return LayerParent; }

	/**
	 * Templatized accessor that returns the object of the template
	 * type if it could be cast and NULL otherwise.
	 *
	 * @return	current Object if it could be cast to type T, NULL otherwise
	 */
	template< class T > T* GetLayerObject()
	{
		return WxTreeObjectWrapper::GetObject<T>();
	}

	FString GetObjectName();

private:
	WxUILayerTreeObjectWrapper();

private:
	class UUILayer* LayerParent;
};

/* ==========================================================================================================
	WxUILayerTreeCtrl
========================================================================================================== */
/**
 * This is the tree control for all layers within a scene
 */ 
class WxUILayerTreeCtrl : public WxTreeCtrl, public FCallbackEventDevice, public FSelectionInterface
{
	DECLARE_DYNAMIC_CLASS(WxUILayerTreeCtrl)
	DECLARE_EVENT_TABLE()

public:
	enum UI_LAYER_ICON_ENUM
	{
		UI_LAYER_ICON_FOLDER_OPEN,
		UI_LAYER_ICON_FOLDER_CLOSED,
		UI_LAYER_ICON_WIDGET
	};

	// Default constructor for use in two-stage dynamic window creation
	WxUILayerTreeCtrl();
	~WxUILayerTreeCtrl();

	/**
	 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	 *
	 * @param	InEditor	pointer to the editor window that contains this control
	 * @param	InParent	the window that opened this dialog
	 * @param	InID		the ID to use for this dialog
	 */
	void Create(class WxUIEditorBase* InEditor, wxWindow* InParent, wxWindowID InID);

	/**
	 * Constructs a new UILayer object
	 *
	 * @param	LayerClass	the class to create an instance of; should be UILayer or a child class
	 * @param	LayerOuter	the object to use as the Outer for the new layer
	 * @param	LayerName	optional name to give the layer; usually only specified when creating a layer root.
	 * @param	LayerFlags	the object flags to use when creating the new layer;  RF_NotForClient|RF_NotForServer will always be OR'd into the value specified
	 *
	 * @return	a pointer to a new UILayer instance of the specified class.
	 */
	UUILayer* ConstructLayer( UClass* LayerClass, UObject* LayerOuter, FName LayerName=NAME_None, QWORD LayerFlags=0 ) const; 

	/**
	 * Get the associated owner scene.
	 *
	 * @return	NULL if an owner scene could not be found
	 */
	class UUIScene* GetOwnerScene() const;

	/**
	 * Get the associated root layer from the tree control.
	 *
	 * @return	NULL if a root layer could not be found
	 */
	class UUILayerRoot* GetLayerRootFromTree() const;

	/**
	 * Returns the UILayerRoot associated with the owner scene.
	 *
	 * @return	NULL if a root layer could not be found
	 */
	class UUILayerRoot* GetLayerRootFromScene() const;

	/**
	 * Create the root layer.
	 *
	 * @return	NULL if a root layer could not be created
	 */
	class UUILayerRoot* CreateLayerRoot() const;

	/**
	 * Audit the root layer object contents.
	 */
	void AuditLayerRoot(class UUILayerRoot* LayerRoot) const;

	/**
	 * Repopulates the tree control with the layers in the scene
	 */
	void RefreshTree();

	/**
	 * Recursively adds all children of the specified parent to the tree control.
	 *
	 * @param	ParentLayer	the layer containing the children that should be added
	 * @param	ParentId	the tree id for the specified parent
	 */
	void AddChildren(class UUILayer* ParentLayer, wxTreeItemId ParentId);

	/**
	 * Method for retrieving the selected items, with the option of ignoring children of selected items.
	 *
	 * @param	Selections					array of wxTreeItemIds corresponding to the selected items
	 * @param	bIgnoreChildrenOfSelected	if TRUE, selected items are only added to the array if the parent item is not selected
	 */
	void GetSelections(TArray<wxTreeItemId>& Selections, UBOOL bIgnoreChildrenOfSelected=FALSE);

	/**
	 * Retrieves the layer nodes associated with the currently selected items
	 */
	void GetSelectedLayers(TArray<struct FUILayerNode*>& SelectedLayers);

	/**
	 * Retrieves the layer associated with the currently selected items
	 */
	void GetSelectedLayers(TArray<class UUILayer*>& SelectedLayers);

	/**
	 * Adds a new layer sibling to the currently selected layer within the tree control.
	 */
	void InsertLayer();

	/**
	 * Adds a new child layer to the currently selected layer within the tree control.
	 */
	void InsertChildLayer();

	/**
	 * Selects all widgets within the currently selected layer within the tree control.
	 */
	void SelectAllWidgetsInLayer();

	/**
	 * Removes the currently selected layer from the tree control and re-parents any
	 * children of the selected layer to the parent of the selected layer.
	 */
	void RemoveSelectedLayer();

	/**
	 * Displays the rename layer dialog for the currently selected layers.
	 */
	void RenameSelectedLayers();

	/**
	 * Tries to rename a UILayer, if unsuccessful, will return FALSE and display a error messagebox.
	 *
	 * @param Layer		Layer to rename.
	 * @param NewName	New name for the object.
	 *
	 * @return	Returns whether or not the object was renamed successfully.
	 */
	UBOOL RenameLayer(class UUILayer* Layer, const TCHAR* NewName);

	/**
	 * Displays the rename dialog for each of the layers specified.
	 *
	 * @param	Layers		the list of layers to rename
	 *
	 * @return	TRUE if at least one of the specified layers was renamed successfully
	 */
	UBOOL DisplayRenameDialog(TArray<class UUILayer*> Layers);

	/**
	 * Retrieves the current total number of layers.
	 */
	INT GetLayerCount() const;

	/**
	 * Returns a string to use as the default LayerName for inserted layers.
	 * This method will also increment the num layers created count.
	 */
	FString CreateDefaultLayerName();

	/**
	 * Synchronizes the selected widgets across all windows.
	 *
	 * If source is the editor window, iterates through the tree control, making sure that the selection state of each tree
	 * item matches the selection state of the corresponding widget in the editor window.
	 *
	 * @param	Source	the object that requested the synchronization.  The selection set of this object will be
	 *					used as the authoritative selection set.
	 */
	virtual void SynchronizeSelectedWidgets(FSelectionInterface* Source);

	/**
	 * Changes the selection set to the widgets specified.
	 *
	 * @return	TRUE if the selection set was accepted
	 */
	virtual UBOOL SetSelectedWidgets(TArray<UUIObject*> SelectionSet);

	/**
	 * Retrieves the widgets corresonding to the selected items in the tree control
	 *
	 * @param	OutWidgets					Storage array for the currently selected widgets.
	 * @param	bIgnoreChildrenOfSelected	if TRUE, widgets will not be added to the list if their parent is in the list
	 */
	virtual void GetSelectedWidgets(TArray<UUIObject*> &OutWidgets, UBOOL bIgnoreChildrenOfSelected = FALSE);

	/** 
	 * Loops through all of the elements of the tree and adds selected items to the selection set,
	 * and expanded items to the expanded set.
	 */
	void SaveSelectionExpansionState();

	/** 
	 * Loops through all of the elements of the tree and sees if the client data of the item is in the 
	 * selection or expansion set, and modifies the item accordingly.
	 */
	void RestoreSelectionExpansionState();

	/** 
	 * Recursion function that loops through all of the elements of the tree item provided and saves their select/expand state. 
	 * 
	 * @param Item Item to use for the root of this recursion.
	 */
	void SaveSelectionExpansionStateRecurse(wxTreeItemId& Item);

	/** 
	 * Recursion function that loops through all of the elements of the tree item provided and restores their select/expand state. 
	 * 
	 * @param Item Item to use for the root of this recursion.
	 */
	void RestoreSelectionExpansionStateRecurse(wxTreeItemId& Item);

public:
	class LayerTreeIterator
	{
	public:
		LayerTreeIterator(WxUILayerTreeCtrl* InTreeControl)
		:	TreeCtrl(InTreeControl)
		,	InternalIterator(NULL)
		,	bSkip(FALSE)
		,	ItemCookie(NULL)
		{
			checkSlow(TreeCtrl);
			ItemId = TreeCtrl->GetRootItem();
			if(ItemId.IsOk())
			{
				ItemId = TreeCtrl->GetFirstChild(ItemId, ItemCookie);
			}
		}

		LayerTreeIterator(WxUILayerTreeCtrl* InTreeControl, wxTreeItemId rootId)
		:	TreeCtrl(InTreeControl)
		,	ItemId(rootId)
		,	InternalIterator(NULL)
		,	bSkip(FALSE)
		,	ItemCookie(NULL)
		{
			checkSlow(TreeCtrl);
			if(ItemId.IsOk())
			{
				ItemId = TreeCtrl->GetFirstChild(ItemId, ItemCookie);
			}
		}

		void operator++()
		{
			checkSlow(ItemId.IsOk());

			if(InternalIterator == NULL)
			{
				if(!bSkip && TreeCtrl->ItemHasChildren(ItemId))
				{
					InternalIterator = new LayerTreeIterator(TreeCtrl, ItemId);
				}
				else
				{
					ItemId = TreeCtrl->GetNextChild(ItemId, ItemCookie);
					bSkip = FALSE;
				}
			}
			else
			{
				++(*InternalIterator);
				if(!(*InternalIterator))
				{
					delete InternalIterator;
					InternalIterator = NULL;
					ItemId = TreeCtrl->GetNextChild(ItemId, ItemCookie);
				}
			}
		}

		struct FUILayerNode* operator*()
		{
			return GetObject();
		}

		struct FUILayerNode* operator->()
		{
			return GetObject();
		}

		operator UBOOL() const
		{
			if(InternalIterator)
			{
				return *InternalIterator;
			}
			return ItemId.IsOk();
		}

		struct FUILayerNode* GetObject() const
		{
			struct FUILayerNode* LayerObject = NULL;
			if(InternalIterator)
			{
				return InternalIterator->GetObject();
			}

			checkSlow(ItemId.IsOk());
			WxUILayerTreeObjectWrapper* ItemData = (WxUILayerTreeObjectWrapper*)TreeCtrl->GetItemData(ItemId);
			if(ItemData)
			{
				LayerObject = ItemData->GetObject();
			}
			return LayerObject;
		}

		template< class T > T* GetLayerObject()
		{
			T* LayerObject = NULL;
			if(InternalIterator)
			{
				return InternalIterator->GetLayerObject<T>();
			}

			checkSlow(ItemId.IsOk());
			WxUILayerTreeObjectWrapper* ItemData = (WxUILayerTreeObjectWrapper*)TreeCtrl->GetItemData(ItemId);
			if(ItemData)
			{
				LayerObject = ItemData->GetLayerObject<T>();
			}
			return LayerObject;
		}

		wxTreeItemId GetId() const
		{
			if(InternalIterator)
			{
				return InternalIterator->GetId();
			}

			return ItemId;
		}

		void Skip()
		{
			if(InternalIterator)
			{
				InternalIterator->Skip();
			}
			else
			{
				bSkip = TRUE;
			}
		}

	private:
		WxUILayerTreeCtrl*  TreeCtrl;
		wxTreeItemId        ItemId;
		wxTreeItemIdValue   ItemCookie;
		LayerTreeIterator*  InternalIterator;
		UBOOL               bSkip;
	};

	LayerTreeIterator TreeIterator() { return LayerTreeIterator(this); }

protected:
	/**
	 * Callback for wxWindows RMB events. Default behaviour is to show the popup menu if a WxUILayerTreeObjectWrapper
	 * is attached to the tree item that was clicked.
	 */
	virtual void OnRightButtonDown(wxMouseEvent& In);

	/**
	 * Called when the user begins editing a label.
	 */
	void OnBeginLabelEdit(wxTreeEvent &Event);

	/**
	 * Called when the user finishes editing a label.
	 */
	void OnEndLabelEdit(wxTreeEvent &Event);

	/**
	 * Called when an item is dragged.
	 */
	void OnBeginDrag(wxTreeEvent& Event);

	/**
	 * Called when an item is done being dragged.
	 */
	void OnEndDrag(wxTreeEvent& Event);

private:
	/** pointer to the editor window that owns this control */
	class WxUIEditorBase* UIEditor;

	/** current count of layers created  */
	INT NumLayersCreated;

	/** Maps layer nodes to their corresponding tree id */
	LayerTreeMap LayerIDMap;

	/** Maps widget classes to their image map id. */
	TMap<FString, INT> WidgetImageMap;

	/** Whether or not we are in an edit label event. */
	UBOOL bEditingLabel;

	/** Whether or not we are in a drag and drop event. */
	UBOOL bInDrag;

	/** Item that is being dragged. */
	wxTreeItemId DragItemId;

	/** Saved state of client data objects that were selected. THESE VOID* POINTERS SHOULD ~NOT~ BE DEREFERENCED! */
	TMap<struct FUILayerNode*, struct FUILayerNode*> SavedSelections;

	/** Saved state of client data objects that were expanded. THESE VOID* POINTERS SHOULD ~NOT~ BE DEREFERENCED! */
	TMap<struct FUILayerNode*, struct FUILayerNode*> SavedExpansions;
};

/* ==========================================================================================================
	WxUILayerBrowser
========================================================================================================== */

class WxUILayerBrowser : public wxPanel
{    
	DECLARE_DYNAMIC_CLASS(WxUILayerBrowser)
	DECLARE_EVENT_TABLE()

public:
	/** Constructor */
	WxUILayerBrowser();

	/**
	 * Initialize this panel and its children.  Must be the first function called after creating the panel.
	 *
	 * @param	InEditor	the ui editor that contains this panel
	 * @param	InParent	the window that owns this panel (may be different from InEditor)
	 * @param	InID		the ID to use for this panel
	 */
	void Create(class WxUIEditorBase* InEditor, wxWindow* InParent, wxWindowID InID);

	/**
	 * Refresh the referenced browser resources.
	 */
	void Refresh();

	/**
	 * Synchronizes the selected widgets across all windows.
	 *
	 * If source is the editor window, iterates through the tree control, making sure that the selection state of each tree
	 * item matches the selection state of the corresponding widget in the editor window.
	 *
	 * @param	Source	the object that requested the synchronization.  The selection set of this object will be
	 *					used as the authoritative selection set.
	 */
	void SynchronizeSelectedWidgets(FSelectionInterface* Source);

	WxUILayerTreeCtrl* GetLayerTree() const { return LayerTree; }

private:
	/**
     * Creates the controls and sizers for this dialog.
     */
	void CreateControls();

	/**
	 * Populates the LayerTree
	 */
	void InitializeLayers();

private:
	/** pointer to the editor window that owns this control */
	class WxUIEditorBase* UIEditor;

	/** this is the tree control that displays the layer contents */
	WxUILayerTreeCtrl* LayerTree;

private:
	/** Painting event handler. */
	void OnPaint(wxPaintEvent& Event);

	/** Sizing event handler. */
	void OnSize(wxSizeEvent& Event);

	/** Called when the user selects the "Insert Layer" context menu option */
	void OnContext_InsertLayer(wxCommandEvent& Event);

	/** Called when the user selects the "Insert Child Layer" context menu option */
	void OnContext_InsertChildLayer(wxCommandEvent& Event);

	/** Called when the user selects the "Rename Layer" context menu option */
	void OnContext_RenameLayer(wxCommandEvent& Event);

	/** Called when the user selects the "Select All Widgets In Layer" context menu option */
	void OnContext_SelectAllWidgetsInLayer(wxCommandEvent& Event);

	/** Called when the user selects the "Delete Layer" context menu option */
	void OnContext_DeleteLayer(wxCommandEvent& Event);
};

#endif // __UILAYERBROWSER_H__


