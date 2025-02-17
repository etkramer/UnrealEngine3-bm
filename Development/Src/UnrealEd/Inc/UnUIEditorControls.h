/*=============================================================================
	UnUIEditorControls.h: UI editor custom control class declarations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNUIEDITORCONTROLS_H__
#define __UNUIEDITORCONTROLS_H__

#include "UnLinkedObjEditor.h"
#include "Kismet.h"

typedef TMap<UUIScreenObject*,wxTreeItemId>			WidgetTreeMap;
typedef TMap<UUIStyle*,wxTreeItemId>				StyleTreeMap;
typedef TMap<IUIEventContainer*,wxTreeItemId>		EventContainerMap;

/**
 * This tree control displays the widget hierarchy for the scene
 */ 
class WxSceneTreeCtrl : public WxTreeCtrl, public FCallbackEventDevice, public FSelectionInterface
{
	DECLARE_DYNAMIC_CLASS(WxSceneTreeCtrl)
	DECLARE_EVENT_TABLE();

public:
	/** Default constructor for use in two-stage dynamic window creation */
	WxSceneTreeCtrl();
	~WxSceneTreeCtrl();

	/**
	 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	 *
	 * @param	InParent	the window that opened this dialog
	 * @param	InID		the ID to use for this dialog
	 * @param	InEditor	pointer to the editor window that contains this control
	 */
	void Create( wxWindow* InParent, wxWindowID InID, WxUIEditorBase* InEditor );

	/**
	 * Repopulates the tree control with the widgets in the current scene
	 */
	virtual void RefreshTree();

	/**
	 * Recursively adds all children of the specified parent to the tree control.
	 */
	virtual void AddChildren( UUIScreenObject* Parent, wxTreeItemId ParentId );

	/**
	 * Expands the tree items corresponding to the specified widgets.
	 *
	 * @param	WidgetsToExpand		the widgets that should be expanded
	 */
	virtual void SetExpandedWidgets( TArray<UUIObject*> WidgetsToExpand );

	/**
	 * Get the list of widgets corresponding to the expanded items in the tree control
	 */
	virtual TArray<UUIObject*> GetExpandedWidgets();


	/**
	 * Synchronizes the selected widgets across all windows.
	 * If source is the editor window, iterates through the tree control, making sure that the selection state of each tree
	 * item matches the selection state of the corresponding widget in the editor window.
	 *
	 * @param	Source	the object that requested the synchronization.  The selection set of this object will be
	 *					used as the authoritative selection set.
	 */
	virtual void SynchronizeSelectedWidgets( FSelectionInterface* Source );

	/**
	 * Changes the selection set to the widgets specified.
	 *
	 * @return	TRUE if the selection set was accepted
	 */
	virtual UBOOL SetSelectedWidgets( TArray<UUIObject*> SelectionSet );

	/**
	* Retrieves the widgets corresonding to the selected items in the tree control
	*
	* @param	OutWidgets					Storage array for the currently selected widgets.
	* @param	bIgnoreChildrenOfSelected	if TRUE, widgets will not be added to the list if their parent is in the list
	*/
	virtual void GetSelectedWidgets( TArray<UUIObject*> &OutWidgets, UBOOL bIgnoreChildrenOfSelected = FALSE );

protected:

	/**
	 * Overloaded method for retrieving the selected items, with the option of ignoring children of selected items.
	 *
	 * @param	Selections					array of wxTreeItemIds corresponding to the selected items
	 * @param	bIgnoreChildrenOfSelected	if TRUE, selected items are only added to the array if the parent item is not selected
	 */
	virtual void GetSelections( TArray<wxTreeItemId>& Selections, UBOOL bIgnoreChildrenOfSelected=FALSE );

	/**
	 * Called when a widget is selected/de-selected in the editor.  Synchronizes the "selected" state of the tree item corresponding to the specified widget.
	 */
	virtual void OnSelectWidget( UUIObject* Widget, UBOOL bSelected );

	/**
	 * Activates the context menu associated with this tree control.
	 *
	 * @param	ItemData	the data associated with the item that the user right-clicked on
	 */
	virtual void ShowPopupMenu( class WxTreeObjectWrapper* ItemData );

private:
	/** Hide this constructor */
	WxSceneTreeCtrl( wxWindow* parent, wxWindowID id, wxMenu* InMenu ) {}

	/** the object editor window that contains this tree control */
	class WxUIEditorBase*			UIEditor;

	/** maps widgets to their corresponding tree id */
	WidgetTreeMap					WidgetIDMap;

	/** used to ignore selection notifications from the event callback system that were the result of a selection event in the tree control */
	UBOOL							bProcessingTreeSelection;

	/** Whether or not we are in a edit label event. */
	UBOOL							bEditingLabel;

	/** Whether or not we are in a drag and drop event. */
	UBOOL							bInDrag;

	/** Indicates that we received a selection synchronization request from someone else */
	UBOOL							bSynchronizingSelection;

	/** Item that is being dragged. */
	wxTreeItemId					DragItem;

	/** Maps widget classes to their image map id. */
	TMap<FString, INT>				WidgetImageMap;

	/**
	 * Called when an item has been activated, either by double-clicking or keyboard. Synchronizes the selected widgets
	 * between the editor window and the scene tree.
	 */
	void OnItemActivated( wxTreeEvent& Event );

	/**
	 * Called when the selected item changes
	 */
	void OnSelectionChanged( wxTreeEvent& Event );

	/**
	 * Called when the selected item changes
	 */
	void OnSelectionChanging( wxTreeEvent& Event );

	/**
	 * Called when an item is dragged.
	 */
	void OnBeginDrag( wxTreeEvent& Event );

	/**
	 * Called when an item is done being dragged.
	 */
	void OnEndDrag( wxTreeEvent& Event );

	/** 
	 * Called when the user presses a key while the tree control has focus.
	 */
	void OnKeyDown( wxTreeEvent& Event );

	/**
	 * Called when the user begins editing a label.
	 */
	void OnBeginLabelEdit( wxTreeEvent &Event );

	/**
	 * Called when the user finishes editing a label.
	 */
	void OnEndLabelEdit( wxTreeEvent &Event );

public:
	class SceneTreeIterator
	{
	public:
		SceneTreeIterator( class WxSceneTreeCtrl* InTreeControl ) : TreeCtrl(InTreeControl), InternalIterator(NULL), bSkip(FALSE), ItemCookie(NULL)
		{
			checkSlow(TreeCtrl);
			ItemId = TreeCtrl->GetRootItem();
			if ( ItemId.IsOk() )
			{
				ItemId = TreeCtrl->GetFirstChild(ItemId, ItemCookie);
			}
		}

		SceneTreeIterator( class WxSceneTreeCtrl* InTreeControl, wxTreeItemId rootId ) : TreeCtrl(InTreeControl), ItemId(rootId), InternalIterator(NULL), bSkip(FALSE), ItemCookie(NULL)
		{
			checkSlow(TreeCtrl);
			if ( ItemId.IsOk() )
			{
				ItemId = TreeCtrl->GetFirstChild(ItemId, ItemCookie);
			}

		}
		inline void operator++()
		{
			checkSlow(ItemId.IsOk());

			if ( InternalIterator == NULL )
			{
				if ( !bSkip && TreeCtrl->ItemHasChildren(ItemId) )
				{
					InternalIterator = new SceneTreeIterator(TreeCtrl, ItemId);
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
				if ( !(*InternalIterator) )
				{
					delete InternalIterator;
					InternalIterator = NULL;

					ItemId = TreeCtrl->GetNextChild(ItemId, ItemCookie);
				}
			}
			
		}
		inline UUIScreenObject* operator*()
		{
			return GetObject();
		}
		inline UUIScreenObject* operator->()
		{
			return GetObject();
		}
		inline operator UBOOL() const
		{
			if ( InternalIterator != NULL )
			{
				return *InternalIterator;
			}
			return ItemId.IsOk();
		}
		UUIScreenObject* GetObject() const
		{
			if ( InternalIterator != NULL )
			{
				return InternalIterator->GetObject();
			}

			checkSlow(ItemId.IsOk());
			WxTreeObjectWrapper* ItemData = (WxTreeObjectWrapper*)TreeCtrl->GetItemData(ItemId);
			return ItemData->GetObjectChecked<UUIScreenObject>();
		}
		wxTreeItemId GetId() const
		{
			if ( InternalIterator != NULL )
			{
				return InternalIterator->GetId();
			}

			return ItemId;
		}
		void Skip()
		{
			if ( InternalIterator != NULL )
			{
				InternalIterator->Skip();
			}
			else
			{
				bSkip = TRUE;
			}
		}

	private:
		WxSceneTreeCtrl*	TreeCtrl;
		wxTreeItemId		ItemId;
		wxTreeItemIdValue	ItemCookie;

		SceneTreeIterator*	InternalIterator;
		UBOOL				bSkip;
	};

	SceneTreeIterator	TreeIterator() { return SceneTreeIterator(this); }

	/** FCallbackEventDevice interface */
	virtual void Send( ECallbackEventType InType );
	virtual void Send( ECallbackEventType InType, UObject* Obj );
};

/* ==========================================================================================================
	WxUIStyleTreeCtrl
========================================================================================================== */
/**
 * This tree control displays the styles contained in a UISkin
 */ 
class WxUIStyleTreeCtrl : public WxTreeCtrl, public FCallbackEventDevice
{
	DECLARE_DYNAMIC_CLASS(WxUIStyleTreeCtrl)

public:
	/** Default constructor for use in two-stage dynamic window creation */
	WxUIStyleTreeCtrl();
	~WxUIStyleTreeCtrl();

	/**
	 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	 *
	 * @param	InParent	the window that opened this dialog
	 * @param	InID		the ID to use for this dialog
	 * @param	InEditor	pointer to the editor window that contains this control
	 */
	void Create( wxWindow* InParent, wxWindowID InID, WxUIEditorBase* InEditor );

	/**
	 * Repopulates the tree control with the styles in the specified skin
	 *
	 * @param	CurrentlyActiveSkin	the skin to display the styles for
	 */
	virtual void RefreshTree( UUISkin* CurrentlyActiveSkin );

	/**
	 * Retrieves the UISkin that is currently set as the root of this tree control
	 */
	UUISkin* GetCurrentSkin();

	/**
	 * Retrieves the UIStyle associated with the currently selected item
	 */
	UUIStyle* GetSelectedStyle();

	/** the object editor window that contains this tree control */
	class WxUIEditorBase*			UIEditor;

	/** maps styles to their corresponding tree id */
	StyleTreeMap					StyleIDMap;

	class StyleTreeIterator
	{
	public:
		StyleTreeIterator( class WxUIStyleTreeCtrl* InTreeControl ) : TreeCtrl(InTreeControl)
		{
			checkSlow(TreeCtrl);
			ItemId = TreeCtrl->GetFirstChild(TreeCtrl->GetRootItem(), ItemCookie);

			// advance past the skin item
			++(*this);

		}
		inline void operator++()
		{
			ItemId = TreeCtrl->GetNextChild(ItemId, ItemCookie);
		}
		inline UUIStyle* operator*()
		{
			return GetObject();
		}
		inline UUIStyle* operator->()
		{
			return GetObject();
		}
		inline operator UBOOL() const
		{
			return ItemId;
		}

		UUIStyle* GetObject() const
		{
			WxTreeObjectWrapper* ItemData = (WxTreeObjectWrapper*)TreeCtrl->GetItemData(ItemId);
			return ItemData->GetObjectChecked<UUIStyle>();
		}
		wxTreeItemId GetId() const
		{
			return ItemId;
		}

	private:
		WxUIStyleTreeCtrl*	TreeCtrl;
		wxTreeItemId		ItemId;
		wxTreeItemIdValue	ItemCookie;
	};

	StyleTreeIterator	StyleIterator() { return StyleTreeIterator(this); }

protected:

	/**
	 * Appends a tree item for the specified style.  If the style is based on another style, ensures that
	 * the style's template has already been added (if the template exists in the current skin), and adds
	 * the style as a child of its template.  Otherwise, the style is added as a child of the root item.
	 *
	 * @param	Style	the style to add to the tree control
	 *
	 * @return	the tree id for the style
	 */
	wxTreeItemId AddStyle( UUIStyle* Style );


	/**
	 * Callback for wxWindows RMB events. Default behaviour is to show the popup menu if a WxTreeObjectWrapper
	 * is attached to the tree item that was clicked.
	 */
	void OnRightButtonDown( wxMouseEvent& In );


public:

	/** FCallbackEventDevice interface */
	virtual void Send( ECallbackEventType InType );
	virtual void Send( ECallbackEventType InType, UObject* InObj );
};

#if 0
/* ==========================================================================================================
	WxUISequenceTreeCtrl
========================================================================================================== */
/**
 * This class displays the sequence objects for the currently selected widgets.  Depending on the filtering
 * flags, it may display all sequence objects, or only those sequence objects contained by the currently selected UIState.
 */
class WxUISequenceTreeCtrl : public WxTreeCtrl, public FCallbackEventDevice, public FSelectionInterface
{
	DECLARE_DYNAMIC_CLASS(WxUISequenceTreeCtrl)

public:
	/** Default constructor for use in two-stage dynamic window creation */
	WxUISequenceTreeCtrl();
	~WxUISequenceTreeCtrl();

	/**
	 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	 *
	 * @param	InParent	the window that opened this dialog
	 * @param	InID		the ID to use for this dialog
	 * @param	InEditor	pointer to the editor window that contains this control
	 */
	void Create( wxWindow* InParent, wxWindowID InID, WxUIEditorBase* InEditor );

	/**
	 * Repopulates the tree control with the sequence objects for the selected widgets
	 *
	 * @param	Filter	if specified, only the sequence objects contained by the
	 */
	virtual void RefreshTree( IUIEventContainer* Filter=NULL );

	virtual void AddEvents( wxTreeItemId ItemId, IUIEventContainer* EventContainer );

	virtual void AddInputKeys( wxTreeItemId ItemId, UUIState* State );

	virtual void AddActions( wxTreeItemId ItemId, IUIEventContainer* EventContainer );

	/** FCallbackEventDevice interface */
	virtual void Send( ECallbackEventType InType );
	virtual void Send( ECallbackEventType InType, UObject* Obj );

	/** the object editor window that contains this tree control */
	class WxUIEditorBase*			UIEditor;

	/** maps widgets to their corresponding tree id */
	WidgetTreeMap					WidgetIDMap;
};
#endif

/* ==========================================================================================================
	WxStyleBrowser
========================================================================================================== */
/**
 * This class displays a combobox for changing the currently active skin and a tree control containing the
 * styles contained by the currently active skin.
 */
class WxStyleBrowser: public wxPanel
{    
	DECLARE_DYNAMIC_CLASS(WxStyleBrowser)
	DECLARE_EVENT_TABLE()

public:
	/// Constructors
	WxStyleBrowser();

	/**
	 * Initialize this panel and its children.  Must be the first function called after creating the panel.
	 *
	 * @param	InEditor	the ui editor that contains this panel
	 * @param	InParent	the window that owns this panel (may be different from InEditor)
	 * @param	InID		the ID to use for this panel
	 */
	void Create( WxUIEditorBase* InEditor, wxWindow* parent, wxWindowID id=ID_UI_PANEL_SKINBROWSER );

	/**
	 * Rebuilds the combo list and sets the appropriate skin as the selected item.
	 *
	 * @param	SkinToSelect	the skin to make the selected item in the combo after it's refreshed.  If not specified, attempts to
	 *							preserve the currently selected item, or if there is none, uses the currently active skin.
	 */
	void RefreshAvailableSkinCombo( UUISkin* SkinToSelect=NULL );

	/**
	 * Populates the skin selection combobox with the loaded UISkins
	 */
	void PopulateSkinCombo();

	/**
	 * Called when the user has changed the active skin.  Notifies all other editor windows about the change.
	 *
	 * @param	NewActiveSkin	the skin that will now be the active skin
	 * @param	PreviouslyActiveSkin	the skin that was previously the active skin
	 */
	void NotifyActiveSkinChanged( UUISkin* NewActiveSkin, UUISkin* PreviouslyActiveSkin );

	/**
	 * Displays the "edit style" dialog to allow the user to modify the specified style.
	 */
	void DisplayEditStyleDialog( UUIStyle* StyleToEdit );

	/** this combo displays the list of available skins */
	wxChoice*					ActiveSkinCombo;

	/** this button displays the "load skin" dialog */
	wxBitmapButton*				LoadSkinButton;

	/** this is the tree control that displays the styles contains by the active skin */
	class WxUIStyleTreeCtrl*	StyleTree;

protected:
	/**
     * Creates the controls and sizers for this dialog.
     */
	void CreateControls();

	/** pointer to the editor window that owns this control */
	WxUIEditorBase*			UIEditor;

private:
	/* =======================================
		Windows event handlers.
	   =====================================*/
	void OnSize( wxSizeEvent& Event );

	/** Called when the user clicks the load skin button */
	void OnButton_LoadSkin( wxCommandEvent& Event );

	/** called when the user double-clicks a style in the style tree */
	void OnDoubleClickStyle( wxTreeEvent& Event );

	/** Called when the user changes the value of the skin combo */
	void OnChangeActiveSkin( wxCommandEvent& Event );

	/** Called when the user selects the "Create Style" context menu option */
	void OnContext_CreateStyle( wxCommandEvent& Event );

	/** Called when the user selects the "Edit Style" context menu option */
	void OnContext_EditStyle( wxCommandEvent& Event );

	/** Called when the user selects the "Delete Style" context menu option */
	void OnContext_DeleteStyle( wxCommandEvent& Event );

	/** Called when wx wants to update the UI for a widget. */
	void OnUpdateUI( wxUpdateUIEvent &Event );

};

#if 0
/* ==========================================================================================================
	WxUIEventBrowser
========================================================================================================== */
/**
 * This class displays a drop-down containing all event containers for a particular widget (i.e. the widget's
 * sequence along with the widget's UIStates).  When an event container is chosen, the event browser refills
 * the tree control with the sequence objects contained by that container.
 */
class WxUIEventBrowser : public wxPanel
{
	DECLARE_DYNAMIC_CLASS(WxUIEventBrowser)
	DECLARE_EVENT_TABLE()

public:
	/** Default constructor for use in two-stage dynamic window creation */
	WxStyleBrowser();

	/**
	 * Initialize this panel and its children.  Must be the first function called after creating the panel.
	 *
	 * @param	InEditor	the ui editor that contains this panel
	 * @param	InParent	the window that owns this panel (may be different from InEditor)
	 * @param	InID		the ID to use for this panel
	 */
	void Create( WxUIEditorBase* InEditor, wxWindow* parent, wxWindowID id=ID_UI_PANEL_EVENTBROWSER );

	/** this combo displays the list of available event containers */
	wxChoice*					cmb_EventContainer;
};
#endif

/* ==========================================================================================================
	WxUIEditorToolBar
========================================================================================================== */

class WxUIEditorToolBar : public wxToolBar
{
	DECLARE_DYNAMIC_CLASS(WxUIEditorToolBar)

public:
	/** Default constructor for use in two-stage dynamic window creation */
	WxUIEditorToolBar() : UIEditor(NULL) {}
	virtual ~WxUIEditorToolBar();

	/**
	 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
	 */
	void Create( wxWindow* InParent, wxWindowID InID, long ToolbarStyle, WxUIEditorBase* InEditorWindow );

protected:

	/** pointer to the editor window containing this toolbar */
	WxUIEditorBase*		UIEditor;

//	WxMaskedBitmap	WireframeB;
//	WxMaskedBitmap	PlayB, PauseB;
//	WxMaskedBitmap	Speed1B, Speed10B, Speed25B, Speed50B, Speed100B;
//	WxMaskedBitmap	LoopSystemB;
//	WxMaskedBitmap	RealtimeB;
//	UBOOL			bRealtime;
//	WxMaskedBitmap	BackgroundColorB;
//	WxMaskedBitmap	RestartInLevelB;
//	WxMaskedBitmap	UndoB;
//	WxMaskedBitmap	RedoB;
//	WxMaskedBitmap	PerformanceMeterB;
//
//	wxSlider*		LODSlider;
//	wxTextCtrl*		LODSetting;
//	WxMaskedBitmap	LODLow;
//	WxMaskedBitmap	LODLower;
//	WxMaskedBitmap	LODAdd;
//	WxMaskedBitmap	LODHigher;
//	WxMaskedBitmap	LODHigh;
//	WxMaskedBitmap	LODDelete;
//	wxComboBox*		LODCombo;
//    
//	DECLARE_EVENT_TABLE()
};

/* ==========================================================================================================
	WxUIEditorMainToolBar
========================================================================================================== */

/**
 * This toolbar contains controls for manipulating the scene and viewport window, along with controls for easily setting
 * common properties of widgets.
 */
class WxUIEditorMainToolBar : public WxUIEditorToolBar
{
	DECLARE_DYNAMIC_CLASS(WxUIEditorMainToolBar)

public:
	/** Default constructor for use in two-stage dynamic window creation */
	WxUIEditorMainToolBar();
	~WxUIEditorMainToolBar();

	/**
	 * Initializes the values of this window's members.
	 */
	void Init();

	/**
	 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
	 */
	void Create( wxWindow* InParent, wxWindowID InID, WxUIEditorBase* InEditorWindow );

	/**
	 * Gets the configured viewport size.
	 *
	 * @param	out_ViewportSizeX	the X size selected by the user
	 * @param	out_ViewportSizeY	the Y size selected by the user
	 *
	 * @return	TRUE if the user selected an explicit size, FALSE if the viewport size should be the same size of the editor window.
	 */
	UBOOL GetViewportSize( INT& out_ViewportSizeX, INT& out_ViewportSizeY ) const;

	/**
	 * Gets the configured gutter size
	 *
	 * @return the value of the Gutter Size spin control
	 */
	INT	GetGutterSize() const;

	/**
	 * Changes the value of the gutter size spin control.  If the input value is out of range, it will be clamped.
	 *
	 * @param	NewGutterSize	a string representing an integer value between 0 and 100
	 */
	void SetGutterSizeValue( const FString& NewGutterSize );

	/**
	 * Finds out which WxWidgets ToolBar button ID corresponds to the Widget with the ID passed in.
	 *
	 * @param InToolID		WxWidget's ID, we will map this to a widget id
	 * @param OutWidgetID	Stores the Widget ID, if we were able to find one.
	 * @return				Whether or not we could find the Widget's ID
	 */
	UBOOL GetWidgetIDFromToolID(INT InToolId, INT& OutWidgetID);

	/**
	 * Re-populates the Widget Preview State combo with a common subset
	 * of states valid for all currently selected widgets within the scene.
	 */
	void UpdateWidgetPreviewStates();

	/**
	 * Handles the selected widget preview state activation.
	 */
	void SetWidgetPreviewStateSelection(INT SelectionIndex);

	/**
	 * Changes the currently selected item in the preview platform combobox.
	 */
	UBOOL SetPreviewPlatformSelection( const FString& NewPlatformName );

	/**
	 * Returns the preview platform combo's currently selected item.
	 */
	FString GetCurrentPreviewPlatform() const;

protected:
	/**
	 * Creates the controls and sizers for this dialog.
	 */
	void CreateControls();

	/**
	* Creates the radio buttons for all of the various UI tool modes.
	*/
	void CreateToolModeButtons();

	/**
	 * Loads the bmp/tga files and assigns them to the WxMaskedBitmaps that are used by this toolbar.
	 */
	void LoadButtonResources();

protected:

	WxMaskedBitmap	ViewportOutlineB;
	WxMaskedBitmap	ContainerOutlineB;
	WxMaskedBitmap	SelectionOutlineB;
	WxMaskedBitmap	SelectionHandleB;
	WxMaskedBitmap	MultiSelectionOutlineB;
	WxMaskedBitmap	ShowDockHandlesB;

	WxComboBox*		cmb_ViewportSize;
	WxComboBox*		cmb_SSPreviewState;
	WxComboBox*		cmb_WidgetPreviewState;
	WxComboBox*		cmb_PreviewPlatform;
	wxSpinCtrl*		spin_ViewportGutter;

	/** A simple map to figure out which tool modes each toolbar widget button corresponds to. */
	TArray<INT>		WidgetButtonResourceIDs;
};

/* ==========================================================================================================
	WxUIEditorWidgetToolBar
========================================================================================================== */

/**
 * This toolbar displays buttons for placing and manipulating widgets in the scene.
 */
class WxUIEditorWidgetToolBar : public WxUIEditorToolBar
{
	DECLARE_DYNAMIC_CLASS(WxUIEditorWidgetToolBar)

public:
	/** Default constructor for use in two-stage dynamic window creation */
	~WxUIEditorWidgetToolBar();

	/**
	 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
	 */
	void Create( wxWindow* InParent, wxWindowID InID, WxUIEditorBase* InEditorWindow );

protected:
	/**
	 * Creates the controls and sizers for this dialog.
	 */
	void CreateControls();
};

/* ==========================================================================================================
	WxUIPositionPanel
========================================================================================================== */
class WxUIPositionPanel : public wxScrolledWindow, FSerializableObject
{
public:
	WxUIPositionPanel(WxUIEditorBase* InParent);

	/** 
	 * Seralizes the array of objects that we are storing. 
	 *
	 * @param Ar	Archive to serialize to.
	 */
	virtual void Serialize( FArchive& Ar );

	/** 
	 * Sets a object to modify, replaces the existing objects we were modifying.
	 *
	 * @param	Object	New object to start modifying.
	 */
	void SetObject(UUIScreenObject* Object);

	/** 
	 * Sets a array of objects to modify, replaces the existing objects we were modifying.
	 *
	 * @param	Objects		New objects to start modifying.
	 */
	void SetObjectArray(const TArray<UUIObject*> &ObjectsArray);

	/** Refreshes the panel's values */
	void RefreshControls();
private:
	/** Updates the width and height fields using the values of the faces. */
	void RefreshWidthAndHeight();

	/** Applies the values of the panel to selected widgets. */
	void ApplyValuesToSelectedWidgets() const;

	/** Called when the user changes the scale type combo box. */
	void OnComboScaleChanged(wxCommandEvent& Event);	

	/** Called when the user presses the enter key on one of the textboxes. */
	void OnTextEntered(wxCommandEvent& Event);	

	/** Array of objects that we are modifying. */
	TArray<UUIScreenObject*> Objects;

	/** Pointer to the parent frame for this panel. */
	WxUIEditorBase* Parent;

	/** Text that displays the names for the currently selected widgets. */
	wxStaticText* SelectedNames;

	/** Value for each of the faces. */
	wxTextCtrl* TextValue[UIFACE_MAX];

	/** Text box that represents the widget's width */
	wxTextCtrl* TextWidth;

	/** Text box that represents the widget's height */
	wxTextCtrl* TextHeight;

	/** Scale type for each of the faces. */
	wxComboBox* ComboScale[UIFACE_MAX];

	DECLARE_EVENT_TABLE()
};

/* ==========================================================================================================
	WxUIKismetTreeControl
========================================================================================================== */
/**
 * This tree control displays the sequences and state-specific events for the current widget
 */ 
class WxUIKismetTreeControl : public WxSequenceTreeCtrl
{
	DECLARE_DYNAMIC_CLASS(WxUIKismetTreeControl)
	DECLARE_EVENT_TABLE()

public:
	/** Default constructor for use in two-stage dynamic window creation */
	WxUIKismetTreeControl();
	virtual ~WxUIKismetTreeControl();

	/**
	 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	 *
	 * @param	InParent	the window that opened this dialog
	 * @param	InID		the ID to use for this dialog
	 * @param	InEditor	pointer to the editor window that contains this control
	 */
	void Create( wxWindow* InParent, wxWindowID InID, class WxUIKismetEditor* InEditor );

	/**
	 * One or more objects changed, so update the tree
	 */
	void NotifyObjectsChanged();

	/**
	 * Repopulates the tree control with the heirarchy of Sequences from the current level.
	 */
	virtual void RefreshTree();

	/**
	 * Adds the sequences contained by the specified widget to the tree control.
	 *
	 * @param	ParentId	the tree item id to use as the parent node for the new tree items
	 * @param	SequenceOwner	the widget to add the sequences for
	 */
	virtual void AddWidgetSequences( wxTreeItemId ParentId, UUIScreenObject* SequenceOwner );

	/**
	 * Adds the sequences for each of the states supported by the specified widget to the tree control.
	 *
	 * @param	ParentId		the tree item id to use as the parent node for all of the state sequences
	 * @param	SequenceOwner	the widget to add the sequences for
	 */
	virtual void AddStateSequences( wxTreeItemId ParentId, UUIScreenObject* SequenceOwner );

	/**
	 * Add the sequence op to the tree control and any children
	 *
	 * @param	ParentId		the tree item id to use as the parent node
	 * @param	Op				the sequence op we are adding
	 */
	void AddSequenceOp( wxTreeItemId ParentId, USequenceOp *Op );

	/**
	 * Recursively adds all subsequences of the specified sequence to the tree control.
	 */
	virtual void AddChildren( wxTreeItemId ParentId, UUIScreenObject* ParentObj );

	/**
	 * De/selects the tree item corresponding to the specified object
	 *
	 * @param	SeqObj		the sequence object to select in the tree control
	 * @param	bSelect		whether to select or deselect the sequence
	 *
	 * @return	True if the specified sequence object was found
	 */
	virtual UBOOL SelectObject( USequenceObject* SeqObj, UBOOL bSelect=TRUE );

	/**
	 * Notification that a tree node collapsed, this is implemented because the base class changes icons.
	 *
	 * @param	Event	the tree event
	 */
	void OnTreeCollapsed( wxTreeEvent& Event )
	{
		// Intentionally do nothing.
	}

	/**
	 * Notification that a tree node expanded, this is implemented because the base class changes icons.
	 *
	 * @param	Event	the tree event
	 */
	void OnTreeExpanding( wxTreeEvent& Event )
	{
		// Intentionally do nothing.
	}

	/**
	 * Called when an item has been activated, either by double-clicking or keyboard. Calls the OnDoubleClick in the sequence object's
	 * helper class.
	 */
	void OnTreeItemActivated( wxTreeEvent& Event );


	/**
	 * Get the tree object that goes with the tree id
	 *
	 * @param	TreeId	the tree id
	 * @return			the sequence object we found or NULL
	 */
	USequenceObject *GetTreeObject( wxTreeItemId TreeId );

	/** Spawn a context menu depending on the selected object type. */
	void OnTreeItemRightClick( wxTreeEvent& In );

protected:

	WxUIKismetEditor* UIKismetEditor;


private:

	enum TREE_ICON
	{
		TREE_ICON_FOLDER_CLOSED,
		TREE_ICON_FOLDER_OPEN,
		TREE_ICON_SEQUENCE,
		TREE_ICON_SEQUENCE_STATE,
		TREE_ICON_EVENT,
		TREE_ICON_EVENT_KEY,
		TREE_ICON_ACTION,
		TREE_ICON_CONDITIONAL,
		TREE_ICON_VARIABLE,
	};
};

#endif

/* ==========================================================================================================
	UIEditorMultiValuesItemControl
========================================================================================================== */
/**
 * Controls which implement this interface can support a display of <multiple values> item
 */ 
class UIEditorMultiControl
{
public:
	
	UIEditorMultiControl()
	: MultipleValuesString(*LocalizeUnrealEd("MultiplePropertyValues"))
	{}

	/* Control unique handling of displaying multiple values */
	virtual void DisplayMultipleValues() =0;

	/* Clear the control changes done by the DisplayMultipleValues method */
	virtual void ClearMultipleValues() =0;

	/* String which will indicate multiple values */
	wxString MultipleValuesString;
};

/* ==========================================================================================================
	WxUIButton
========================================================================================================== */
/**
 * This UI Editor specific Button control supports a display of <multiple values> item
 */ 
class WxUIButton: public wxButton, public UIEditorMultiControl
{
public:

	/**
	 * Constructor to match wxButton 
	 */
	WxUIButton(wxWindow* parent,
			   wxWindowID id,
			   const wxString& label = wxEmptyString,
			   const wxPoint& pos = wxDefaultPosition,
			   const wxSize& size = wxDefaultSize,
			   long style = 0,
			   const wxValidator& validator = wxDefaultValidator,
			   const wxString& name = wxButtonNameStr)
			   : wxButton(parent, id, label, pos, size, style, validator, name){}

	/* Control unique handling of displaying multiple values */
	virtual void DisplayMultipleValues();

	/* Clear the control changes done by the DisplayMultipleValues method */
	virtual void ClearMultipleValues();
};

/* ==========================================================================================================
	WxUITextCtrl
========================================================================================================== */
/**
 * This UI Editor specific TextCtrl control supports a display of <multiple values> item
 */ 
class WxUITextCtrl: public WxTextCtrl, public UIEditorMultiControl
{
public:

	/**
	 * Constructor to match WxTextCtrl 
	 */
	WxUITextCtrl(wxWindow* parent,
				 wxWindowID id,
				 const wxString& value = wxEmptyString,
				 const wxPoint& pos = wxDefaultPosition,
				 const wxSize& size = wxDefaultSize,
				 long style = 0)
				 :WxTextCtrl(parent,id,value,pos,size,style){}

	/* Control unique handling of displaying multiple values */
	virtual void DisplayMultipleValues();

	/* Clear the control changes done by the DisplayMultipleValues method */
	virtual void ClearMultipleValues();
};


/* ==========================================================================================================
	WxUIChoice
========================================================================================================== */
/**
 * This UI Editor specific Choice control supports a display of <multiple values> item
 */ 
class WxUIChoice: public wxChoice, public UIEditorMultiControl
{
public:

	/* Constructors */
	/**
	 * Constructors to match wxChoice 
	 */
	WxUIChoice(wxWindow *parent,
			   wxWindowID id,
			   const wxPoint& pos,
			   const wxSize& size,
			   int n,
			   const wxString choices[],
			   long style = 0,
			   const wxValidator& validator = wxDefaultValidator,
			   const wxString& name = wxChoiceNameStr)
			   : wxChoice(parent,id,pos,size,n,choices,style,validator,name){}

	WxUIChoice(wxWindow *parent,
			   wxWindowID id,
			   const wxPoint& pos,
			   const wxSize& size,
			   const wxArrayString& choices,
			   long style = 0,
			   const wxValidator& validator = wxDefaultValidator,
			   const wxString& name = wxChoiceNameStr)
			   : wxChoice(parent,id,pos,size,choices,style,validator,name){}


	/* Control unique handling of displaying multiple values */
	virtual void DisplayMultipleValues();

	/* Clear the control changes done by the DisplayMultipleValues method */
	virtual void ClearMultipleValues();
};

/* ==========================================================================================================
	WxUISpinCtrl
========================================================================================================== */
/**
 * This UI Editor specific Spin control supports a display of <multiple values> item
 */ 
class WxUISpinCtrl : public WxSpinCtrlReal, public UIEditorMultiControl
{
public:
	/**
	 *	Constructor to match WxSpinCtrlReal
	 */
	WxUISpinCtrl(
		wxWindow* parent,
		wxWindowID InID=wxID_ANY,
		FLOAT InitialValue=0.0f,
		wxPoint Position=wxDefaultPosition,
		wxSize Size=wxDefaultSize,
		FLOAT InMinValue=-100000.0f,
		FLOAT InMaxValue=100000.0f
		) : WxSpinCtrlReal(
		parent, InID, InitialValue, Position, Size,
		InMinValue, InMaxValue)
	{}
	
	/* Control unique handling of displaying multiple values */
	virtual void DisplayMultipleValues();

	/* Clear the control changes done by the DisplayMultipleValues method */
	virtual void ClearMultipleValues();
};

// EOF














