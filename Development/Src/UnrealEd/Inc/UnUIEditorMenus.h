/*=============================================================================
	UnUIEditorMenus.h: UI editor menu class declarations.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNUIEDITORMENUS_H__
#define __UNUIEDITORMENUS_H__

class wxWindow;

/**
 * Simple data structure for grouping a widget with an arbitrary collection of FUIStyleReferences contained in that widget.
 */
struct FWidgetStyleReference
{
	/**
	 * The widget that contains the style references
	 */
	class UUIObject*	StyleReferenceOwner;

	/**
	 * A list of style references contained by StyleReferenceOwner.  Normally these will correspond to the values for a single FUIStyleReference property
	 * in the widget, so the array will usually have only one element unless the property associated with these style references is also an array.
	 */
	TArray<struct FUIStyleReference*> References;

	/** Constructor */
	FWidgetStyleReference( class UUIObject* InOwner )
	: StyleReferenceOwner(InOwner)
	{}
};

/**
 * Data structure for grouping the values for a particular style reference property, for one or more widgets.
 */
struct FStylePropertyReferenceGroup
{
	/**
	 * The name of the style reference property that this group contains.
	 */
	struct FStyleReferenceId StylePropertyRef;

	/**
	 * A list of widgets and the style reference values for the property with the name - StylePropertyName
	 */
	TArray<FWidgetStyleReference> WidgetStyleReferences;

	/** Constructor */
	FStylePropertyReferenceGroup( const struct FStyleReferenceId& InStylePropertyRef )
	: StylePropertyRef(InStylePropertyRef)
	{}
};

/**
 * Base class for all UI editor context and popup menus.
 */
class WxUIMenu : public wxMenu
{
	DECLARE_DYNAMIC_CLASS(WxUIMenu)

public:

	/**
	 * You must pass in the window used to create this menu if your menu needs to call GetUIEditor() prior to Show()
	 */
	WxUIMenu( wxWindow* InOwner=NULL );

	/**
	 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	 */
	virtual void Create();

	/**
	 * Appends menu options that are used for manipulating widgets
	 */
	virtual void AppendWidgetOptions();

	/**
	 * Appends menu items that let the user align selected objects to the viewport.
	 */
	virtual void AppendAlignMenu();

	/**
	 * Appends menu items that relate to datastores.
	 */
	virtual void AppendDataStoreMenu();

	/**
	 * Appends menu options which are applicable to both UIScenes and UIObjects
	 */
	virtual void AppendScreenObjectOptions();

	/**
	 * Appends a submenu containing context-sensitive menu options based on the currently selected widget
	 */
	virtual void AppendCustomWidgetMenu();

	/**
	 * Appends a menu for converting this widget's position into a different scale type.
	 */
	virtual void AppendPositionConversionMenu();

	/**
	 * Appends menu options for changing or editing styles associated with the selected widgets.
	 */
	virtual void AppendStyleOptions();

	/**
	 * Appends options which are normally associated with "Edit" menus
	 * @param bRightClickMenu Whether or not this is a right click menu.
	 */
	virtual void AppendEditOptions(UBOOL bRightClickMenu);

	/**
	 * Appends all options which are used for manipulating the Z-order placement of the scene's widgets
	 */
	virtual void AppendLayerOptions();

	/**
	 * Appends items for placing a widget into the scene
	 */
	virtual void AppendPlacementOptions();

	/**
	 * Appends all options which are common to all UObjects, such as rename, delete, etc.
	 */
	virtual void AppendObjectOptions();

	/** Retrieves the WxUIEditorBase window that contains this menu */
	WxUIEditorBase* GetUIEditor();

	/**
	 * Retrieves the top-level context menu
	 */
	class WxUIContextMenu* GetMainContextMenu();

	/**
	 * Retrieves the top-level context menu, asserting if it is not a WxUIContextMenu.
	 */
	class WxUIContextMenu* GetMainContextMenuChecked();

	/**
	 * Returns the next ID available for use in style selection menus from the owner WxUIContextMenu.
	 *
	 * @return	the next menu id available for context menu items that contain options for changing the style assigned
	 *			to a widget's style property; value will be between IDM_UI_STYLECLASS_BEGIN and IDM_UI_STYLECLASS_END if there
	 *			are menu ids available for use, or INDEX_NONE if no more menu ids are available or if this menu's top-level menu
	 *			is not a WxUIContextMenu
	 */
	INT GetNextStyleSelectionMenuID();

	/**
	 * Returns the next ID available for use in edit style menus from the owner WxUIContextMenu.
	 *
	 * @return	the next menu id available for context menu items that contain options for editing the style assigned
	 *			to a widget's style property; value will be between IDM_UI_STYLEREF_BEGIN and IDM_UI_STYLEREF_END if there
	 *			are menu ids available for use, or INDEX_NONE if no more menu ids are available or if this menu's top-level menu
	 *			is not a WxUIContextMenu.
	 */
	INT GetNextStyleEditMenuID();

	/**
	 * Returns the next ID available for use in set data binding menus from the owner WxUIContextMenu.
	 *
	 * @return	the next menu id available for context menu items that correspond to data store binding properties.
	 *			value will be between IDM_UI_SET_DATASTOREBINDING_START and IDM_UI_SET_DATASTOREBINDING_END if there
	 *			are menu ids available for use, or INDEX_NONE if no more menu ids are available or if this menu's top-level menu
	 *			is not a WxUIContextMenu
	 */
	INT GetNextSetDataBindingMenuId();

	/**
	 * Returns the next ID available for use in clear data binding menus from the owner WxUIContextMenu.
	 *
	 * @return	the next menu id available for context menu items that correspond to data store binding properties.
	 *			value will be between IDM_UI_SET_DATASTOREBINDING_START and IDM_UI_SET_DATASTOREBINDING_END if there
	 *			are menu ids available for use, or INDEX_NONE if no more menu ids are available or if this menu's top-level menu
	 *			is not a WxUIContextMenu
	 */
	INT GetNextClearDataBindingMenuId();

	/** the widgets that were selected when this menu was invoked */
	TArray<UUIObject*>	SelectedWidgets;

	/** the selection set with children of selected widgets removed */
	TArray<UUIObject*>	SelectedParents;

	/** Pointer to the UI Editor that owns this menu. */
	WxUIEditorBase* UIEditor;

protected:
	
	/**
	 * Appends a submenu containing a list of style properties for the selected widgets, along with the styles which are valid for those properties.
	 * Used for allowing the designer to change the style assigned to a widget's style property.
	 *
	 * @return	TRUE if at least one menu item was appended
	 */
	virtual UBOOL AppendStyleSelectionMenu();

	/**
	 * Appends a submenu containing a list of style properties for the selected widgets, along with the styles which are valid for those properties.
	 *
	 * @return	TRUE if at least one menu item was appended
	 */
	virtual UBOOL AppendEditStyleMenu();

	/**
	 * Retrieves a list of styles which are valid to be assigned to the specified style references.
	 *
	 * @param	StyleReferences		the list of style references to find valid styles for
	 * @param	out_ValidStyles		receives the list of styles that can be assigned to all of the style references specified.
	 */
	void GetValidStyles( const TArray<FUIStyleReference*>& StyleReferences, TArray<UUIStyle*>& out_ValidStyles );

	/**
	 * Generates a style selection submenu for each of the style references specified and appends them to the ParentMenu specified.
	 *
	 * @param	ParentMenu			the menu to append the submenus to
	 * @param	StyleReferences		the list of style properties to build submenus for
	 * @param	ExtraData			optional additional data to associate with the menu items generated; this value will be set as the ExtraData
	 *								member in all FUIStyleMenuPairs that are created
	 *
	 * @return	TRUE if any menu items were appended
	 */
	UBOOL BuildStyleSelectionSubmenus( wxMenu* ParentMenu, const FStylePropertyReferenceGroup& StyleReferences, INT ExtraData=INDEX_NONE );

	/**
	 * Holds the value of next available menu id to use for building style selection submenus.
	 */
	INT NextStyleMenuID;
	
	/**
	 * Holds the value of the next available menu id to use for building edit style selection submenus.
	 */
	INT NextEditStyleMenuID;


	/**
	 * Holds the value of next available menu id to use for building "set data binding" submenus.
	 */
	INT NextSetDataStoreBindingMenuID;

	/**
	 * Holds the value of next available menu id to use for building "clear data binding" submenus.
	 */
	INT NextClearDataStoreBindingMenuID;
};

/**
 * Base class for all UI editor context menus.
 */
class WxUIContextMenu : public WxUIMenu
{
	DECLARE_DYNAMIC_CLASS(WxUIContextMenu)

public:

	/**
	 * You must pass in the window used to create this menu if your menu needs to call GetUIEditor() prior to Show()
	 */
	WxUIContextMenu( wxWindow* InOwner=NULL ) : WxUIMenu(InOwner), ActiveWidget(NULL) {}

	virtual void Create( UUIObject* InActiveWidget )
	{
		WxUIMenu::Create();
		ActiveWidget = InActiveWidget;
	}

	/**
	 * This is the widget that was clicked on when this context menu was invoked
	 * It must be part of the selection set, though it may not be the only widget that
	 * actions should be applied to
	 */
	UUIObject*			ActiveWidget;

private:
	// hide this implementation
	virtual void Create() {}
};

/**
 * Context menu that is displayed when the user right-clicks on an item in the scene tree
 */
class WxUISceneTreeContextMenu : public WxUIContextMenu
{
	DECLARE_DYNAMIC_CLASS(WxUISceneTreeContextMenu)

public:
	/**
	 * You must pass in the window used to create this menu if your menu needs to call GetUIEditor() prior to Show()
	 */
	WxUISceneTreeContextMenu( wxWindow* InOwner=NULL ) : WxUIContextMenu(InOwner) {}

	/**
	 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	 * Appends the menu options
	 *
	 * @param	InItemData	the tree item that was clicked on to invoke this menu
	 */
	virtual void Create( class WxTreeObjectWrapper* InItemData );
};

/**
 * Context menu that is displayed when the user right-clicks on an item in the layer tree
 */
class WxUILayerTreeContextMenu : public WxUIContextMenu
{
	DECLARE_DYNAMIC_CLASS(WxUILayerTreeContextMenu)

public:
	/**
	 * Constructor
	 * Appends the menu options
	 */
	WxUILayerTreeContextMenu( wxWindow* InOwner=NULL );
};

/**
 * Context menu that is displayed when the user right-clicks on an item in the style browser
 */
class WxUIStyleTreeContextMenu : public WxUIContextMenu
{
	DECLARE_DYNAMIC_CLASS(WxUIStyleTreeContextMenu)

public:
	/**
	 * Constructor
	 * Appends the menu options
	 */
	WxUIStyleTreeContextMenu( wxWindow* InOwner=NULL );
};

/**
 * Context menu that is displayed when the user clicks the "load skin" button in the style browser
 */
class WxUILoadSkinContextMenu : public WxUIContextMenu
{
	DECLARE_DYNAMIC_CLASS(WxUILoadSkinContextMenu)

public:
	/**
	 * Constructor
	 * Appends the menu options
	 */
	WxUILoadSkinContextMenu( wxWindow* InOwner=NULL );
};

/**
 * Context menu that is displayed when the user right-clicks an empty area of
 * a UIEditor window.
 */
class WxUIObjectNewContextMenu : public WxUIContextMenu
{
	DECLARE_DYNAMIC_CLASS(WxUIObjectNewContextMenu)

public:
	/**
	 * You must pass in the window used to create this menu if your menu needs to call GetUIEditor() prior to Show()
	 */
	WxUIObjectNewContextMenu( wxWindow* InOwner=NULL ) : WxUIContextMenu(InOwner) {}

	/**
	 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	 * Appends the menu options.
	 *
	 * @param	InActiveWidget	the widget that was clicked on to invoke this context menu
	 */
	virtual void Create( UUIObject* InActiveWidget );
};

/**
* Context menu that is displayed when the user right-clicks an object in
* a UIEditor window.
*/
class WxUIObjectOptionsContextMenu : public WxUIContextMenu
{
	DECLARE_DYNAMIC_CLASS(WxUIObjectOptionsContextMenu)

public:
	/**
	 * You must pass in the window used to create this menu if your menu needs to call GetUIEditor() prior to Show()
	 */
	WxUIObjectOptionsContextMenu( wxWindow* InOwner=NULL )
	: WxUIContextMenu(InOwner)
	{}

	/**
	 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	 *
	 * @param	InActiveWidget	the widget that was clicked on to invoke this context menu
	 */
	virtual void Create( UUIObject* InActiveWidget );
};

/*-----------------------------------------------------------------------------
	WxUIListBindMenu
-----------------------------------------------------------------------------*/
class WxUIListBindMenu : public WxUIContextMenu
{
public:
	/**
	 * You must pass in the window used to create this menu if your menu needs to call GetUIEditor() prior to Show()
	 */
	WxUIListBindMenu( wxWindow* InOwner=NULL ) : WxUIContextMenu(InOwner), NextStyleMenuID(IDM_UI_STYLECLASS_BEGIN) {}

	/**
	 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	 * Appends the menu options.
	 *
	 * @param	InActiveWidget	the widget that was clicked on to invoke this context menu
	 */
	virtual void Create( UUIObject* InActiveWidget, INT ColumnIdx );

private:

	/**
	 * Generates a menu for the column index specified using all of the available data tags as options.
	 *
	 * @param InList			List to use to generate the menu.
	 * @param ElementNames		Array of available data tags.
	 * @param ColumnMenu		Menu to append items to.
	 * @param ColumnIdx			Column that we will be generating a menu for.
	 */
	wxMenu* CreateColumnMenu(class UUIList* InList, const TArray<FName> &ElementNames, wxMenu* ColumnMenu, INT ColumnIdx);

protected:
	/**
	 * Holds the value of next available menu id to use for building style selection submenus.
	 */
	INT NextStyleMenuID;
};

/*-----------------------------------------------------------------------------
	WxUICalloutButtonPanelMenu
-----------------------------------------------------------------------------*/
class WxUICalloutButtonPanelMenu : public WxUIContextMenu
{
public:
	/**
	 * You must pass in the window used to create this menu if your menu needs to call GetUIEditor() prior to Show()
	 */
	WxUICalloutButtonPanelMenu( wxWindow* InOwner=NULL ) : WxUIContextMenu(InOwner) {}

	/**
	 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	 * Appends the menu options.
	 *
	 * @param	InActivePanel	the panel that was clicked on to invoke this context menu
	 * @param	ActiveButton	the button the user clicked on; NULL if the user clicked on the panel itself
	 */
	virtual void Create( UUICalloutButtonPanel* InActiveWidget, UUICalloutButton* ActiveButton=NULL );
};

/**
* Context menu that is displayed when the user right-clicks an handle in
* a UIEditor window.
*/
class WxUIHandleContextMenu : public WxUIContextMenu
{
	DECLARE_DYNAMIC_CLASS(WxUIHandleContextMenu)

public:
	/**
	* You must pass in the window used to create this menu if your menu needs to call GetUIEditor() prior to Show()
	*/
	WxUIHandleContextMenu( wxWindow* InOwner=NULL ) : WxUIContextMenu(InOwner) {}

	/**
	* Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	*
	* @param	InActiveWidget	the widget that was clicked on to invoke this context menu
	* @param	InActiveFace	the dock handle that was clicked on to invoke this context menu
	*/
	virtual void Create( UUIObject* InActiveWidget, EUIWidgetFace InActiveFace );

	/** The dock handle that was clicked on. */
	EUIWidgetFace ActiveFace;
};

/**
 * Context menu that is displayed when the user needs to choose among a list of possible dock handle targets after trying to create a dock link.
 */
class WxUIDockTargetContextMenu : public WxUIContextMenu
{
	DECLARE_DYNAMIC_CLASS(WxUIDockTargetContextMenu)

public:
	/**
	* You must pass in the window used to create this menu if your menu needs to call GetUIEditor() prior to Show()
	*/
	WxUIDockTargetContextMenu( wxWindow* InOwner=NULL ) : WxUIContextMenu(InOwner) {}

	/**
	* Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	*/
	virtual void Create( INT InNumOptions );

	/** 
	 * Sets the label for a menu item.
	 *
	 * @param OptionIdx	Which menu item to set the label for (Must be between 0 and the number of options - 1).
	 * @param Label		New label for the item.
	 */
	void SetMenuItemLabel(INT OptionIdx, const TCHAR* Label );

private:
	INT NumOptions;
};


/* ==========================================================================================================
	WxUIEditorMenuBar
========================================================================================================== */

class WxUIEditorMenuBar : public wxMenuBar
{
	DECLARE_DYNAMIC_CLASS(WxUIEditorMenuBar)

public:
	wxMenu	*FileMenu, *ViewMenu;
	WxUIMenu *EditMenu, *SceneMenu;

	/** Default constructor for use in two-stage dynamic window creation */
	WxUIEditorMenuBar() : FileMenu(NULL), ViewMenu(NULL), EditMenu(NULL), SceneMenu(NULL)
	{}

	~WxUIEditorMenuBar();

	/**
	 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	 *
	 * @param	InEditor	pointer to the editor window that contains this control
	 */
	void Create( WxUIEditorBase* InEditor );
};

/* ==========================================================================================================
	WxUIDragGridMenu
========================================================================================================== */

/**
 * Menu for controlling options related to the drag grid.
 */
class WxUIDragGridMenu : public wxMenu
{
public:
	WxUIDragGridMenu();
};


/* ==========================================================================================================
	WxUIKismetNewObject
========================================================================================================== */
class WxUIKismetNewObject : public wxMenu
{
public:
	WxUIKismetNewObject(WxUIKismetEditor* SeqEditor);

protected:

	/**
	 * Looks for a category in the op's name, returns TRUE if one is found.
	 */
	UBOOL GetSequenceObjectCategory(USequenceObject *Op, FString &CategoryName, FString &OpName);


	/** Submenu pointers. */
	wxMenu *ActionMenu, *VariableMenu, *ConditionMenu, *EventMenu, *ContextEventMenu;
};

/* ==========================================================================================================
	WxUIKismet_CalloutProxyOptions
========================================================================================================== */
/**
 * This context menu is displayed when the user right-clicks on a UUIEvent_CalloutInputProxy
 * object.  It simply adds a new item to the top of the menu which allows the user to add
 * a new callout alias to the event object.
 */
class WxUIKismet_CalloutProxyOptions : public WxMBKismetObjectOptions
{
public:
	/** Constructor */
	WxUIKismet_CalloutProxyOptions(class WxUIKismetEditor* KismetEditor);
};

/* ==========================================================================================================
	WxUIKismet_CalloutProxyConnectorOptions
========================================================================================================== */
/**
 * This context menu is displayed when the user right-clicks on a UUIEvent_CalloutInputProxy
 * object.  It simply adds a new item to the top of the menu which allows the user to add
 * a new callout alias to the event object.
 */
class WxUIKismet_CalloutProxyConnectorOptions : public WxMBKismetConnectorOptions
{
public:
	/** Constructor */
	WxUIKismet_CalloutProxyConnectorOptions(class WxUIKismetEditor* KismetEditor);
};

#endif

