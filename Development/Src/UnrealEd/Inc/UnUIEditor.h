/*=============================================================================
	UnUIEditor.h: UI editor class declarations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UIEDITOR_H__
#define __UIEDITOR_H__

// Forward declarations
class WxUIEditorBase;
class WxUIKismetEditor;
class WxObjectViewportHolder;
class WxUISceneTreeContextMenu;
class WxUILayersTreeContextMenu;
class WxUIStyleTreeContextMenu;
class WxUIObjectNewContextMenu;
class WxUIObjectOptionsContextMenu;

#include "DockingParent.h"
#include "Kismet.h"
#include "UnLinkedObjDrawUtils.h"
#include "TrackableWindow.h"

/**
 * Loads the specified key from the UIEditor section of the UnrealEd .int file
 * 
 * @param	Key			the key to load
 * @param	bOptional	TRUE to ignore missing localization values
 *
 * @return	the value corresponding to the key specified.  For keys that can't be found, if
 * bOptional is FALSE, the result will be a localization error string indicating which key was missing.
 * If bOptional is TRUE, the result will be an empty string.
 */
FString LocalizeUI( const TCHAR* Key, UBOOL bOptional=FALSE );


/**
 * Loads the specified key from the UIEditor section of the UnrealEd .int file
 * 
 * @param	Key			the key to load
 * @param	bOptional	TRUE to ignore missing localization values
 *
 * @return	the value corresponding to the key specified.  For keys that can't be found, if
 * bOptional is FALSE, the result will be a localization error string indicating which key was missing.
 * If bOptional is TRUE, the result will be an empty string.
 */
FString LocalizeUI( const ANSICHAR* Key, UBOOL bOptional=FALSE );

IMPLEMENT_COMPARE_POINTER(UUIScreenObject,UnUIEditor_AlphaSort,{ return appStricmp(*A->GetName(),*B->GetName()); });

/**
 * This is an interface for any controls which are capable of controlling widget selection.  It provides
 * methods for retrieving the selection set.
 */
class FSelectionInterface
{
	/**
	* Retrieves the widgets corresonding to the selected items in the tree control
	*
	* @param	OutWidgets					Storage array for the currently selected widgets.
	* @param	bIgnoreChildrenOfSelected	if TRUE, widgets will not be added to the list if their parent is in the list
	*/
	virtual void GetSelectedWidgets( TArray<UUIObject*> &OutWidgets, UBOOL bIgnoreChildrenOfSelected = FALSE )=0;

	/**
	 * Changes the selection set for this FSelectionInterface to the widgets specified.
	 *
	 * @return	TRUE if the selection set was accepted
	 */
	virtual UBOOL SetSelectedWidgets( TArray<class UUIObject*> SelectionSet )=0;

	/**
	 * Synchronizes the selected widgets across all windows
	 *
	 * @param	Source	the object that requested the synchronization.  The selection set of this object will be
	 *					used as the authoritative selection set.
	 */
	virtual void SynchronizeSelectedWidgets( class FSelectionInterface* Source )=0;
};

/**
 * This class builds a list of widgets that meet arbitrary requirements.  To use this class, first apply any filters
 * that should be used when evaluating whether a widget should be included in the list using the ApplyFilter methods.
 * When all filters are applied, call Process() to build the list of widgets that match the criteria specified.
 */
class FWidgetCollector : public TArray<UUIObject*>
{
public:

	/** Constructors */
	FWidgetCollector();

	/**
	 * Adds the specified flag/s to the collection filter.  Only widgets with the specified ObjectFlags will be included in the results.
	 */
	void ApplyFilter( EObjectFlags RequiredFlags );

	/**
	 * Adds the specified style to the collection filter.   Only widgets with this style (or a style based on this one) will be included in the results.
	 *
	 * @param	RequiredStyle	the style to filter for
	 * @param	bIncludeDerived	if TRUE, styles which are based on RequiredStyle will also be included
	 */
	void ApplyFilter( UUIStyle* RequiredStyle, UBOOL bIncludeDerived=TRUE );

	/**
	 * Builds the list of widgets which match the filters for this collector.
	 *
	 * @param	Container	the screen object to use to start the recursion.  Container will NOT be included in the results.
	 */
	void Process( UUIScreenObject* Container );

protected:
	/**
	 * Adds the widget specified widget to the list if it passes the current filters, then recurses through all children of the widget.
	 */
	void Integrate( UUIObject* Widget );

private:
	/**
	 * Restricts the list of widgets to those that match this flag mask.  A value of 0 indicates that widgets are not filtered by ObjectFlags.
	 */
	EObjectFlags			FilterFlags;

	/**
	 * Restricts the list of widget to those which have the style (or derivation) specified
	 */
	TArray<UUIStyle*>		FilterStyles;
};

// Includes
#include "UnUIEditorMenus.h"
#include "UnUIEditorControls.h"

/** Hit proxy for dock handles. */
struct HUIDockHandleHitProxy : HObject
{
	DECLARE_HIT_PROXY(HUIDockHandleHitProxy, HObject);

	UUIObject*			UIObject;
	EUIWidgetFace		DockFace;

	HUIDockHandleHitProxy(UUIObject* InObject, EUIWidgetFace InDockFace): HObject(InObject), UIObject(InObject), DockFace(InDockFace)  {}

};

/** Hit proxy for list cell boundaries */
struct HUICellBoundaryHitProxy : public HObject
{
	DECLARE_HIT_PROXY(HUICellBoundaryHitProxy,HObject);

	UUIList* SelectedList;
	INT ResizeCell;

	/** constructor */
	HUICellBoundaryHitProxy( UUIList* InSelectedList, INT InResizeCell );

	/**
	 * Changes the cursor to the resize cursor
	 */
	virtual EMouseCursor GetMouseCursor();
};

/**
 * This struct contains all information required to place a new widget in the scene.
 */
struct FUIWidgetPlacementParameters
{
	/** the dimensions to use for the new widget */
	FLOAT PosX, PosY, SizeX, SizeY;

	/** the archetype to use for the new widget; this also determines the widget's class */
	UUIObject* Archetype;

	/** the widget or scene that should become the parent for the new widget  */
	UUIScreenObject* Parent;

	/**
	 * whether the user should be prompted to enter a name for this new widget
	 * if false, the widget will be given a name using the MakeUniqueObjectName method
	 */
	UBOOL bPromptForWidgetName;

	/**
	 * whether or not to use widget class default position/size when placing widgets.
	 */
	UBOOL bUseWidgetDefaults;

	/** Constructors */
	// no default constructor to make sure code doesn't break if we add new members to this struct
// 	FUIWidgetPlacementParameters()
// 	: PosX(0.f), PosY(0.f), SizeX(0.f), SizeY(0.f)
// 	, Archetype(NULL), Parent(NULL), bPromptForWidgetName(FALSE)
// 	{}

	FUIWidgetPlacementParameters( FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, UUIObject* InArchetype, UUIScreenObject* InParent=NULL, UBOOL bDisplayNamePrompt=TRUE, UBOOL bInUseWidgetDefaults=FALSE )
	: PosX(X), PosY(Y), SizeX(XL), SizeY(YL)
	, Archetype(InArchetype), Parent(InParent), bPromptForWidgetName(bDisplayNamePrompt), bUseWidgetDefaults(bInUseWidgetDefaults)
	{}
};

/** Hit proxy for focus chain handles. */
struct HUIFocusChainHandleHitProxy : HObject
{
	DECLARE_HIT_PROXY(HUIFocusChainHandleHitProxy, HObject);

	UUIObject*			UIObject;
	EUIWidgetFace		DockFace;

	HUIFocusChainHandleHitProxy(UUIObject* InObject, EUIWidgetFace InDockFace): HObject(InObject), UIObject(InObject), DockFace(InDockFace)  {}

};

/**
 * This tool renders the selection outline and handles for the currently selected widget, and
 * processes widget movement and resizing operations.
 */
struct FUISelectionTool : public FObjectSelectionTool
{
	/**
	 * Returns whether the user is allowed to move the anchor handle.
	 */
	virtual UBOOL IsAnchorTransformable( const TArray<UUIObject*>& SelectedObjects ) const;
};

/**
 * Base class for all UI Editor render modifiers
 */
class FUIEditor_RenderModifier : public FObjectViewportRenderModifier
{
public:
	/**
	 * Constructor
	 *
	 * @param	InEditor	the editor window that holds the container to render the outline for
	 */
	FUIEditor_RenderModifier( class WxUIEditorBase* InEditor ) : UIEditor(InEditor)
	{}

protected:

	/** the editor window that that owns this render modifier */
	class WxUIEditorBase*	UIEditor;
};

/**
 * This class draws a tooltip telling the user which dock widget they are currently moused over.
 */
class FRenderModifier_TargetTooltip : public FUIEditor_RenderModifier
{
public:
	/**
	 * Constructor
	 *
	 * @param	InEditor	the editor window that holds the container to render the outline for
	 */
	FRenderModifier_TargetTooltip( class WxUIEditorBase* InEditor );

	/**
	 * Renders a tooltip box with each of the strings in our TooltipStrings list displayed in order from top to bottom.
	 */
	virtual void Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas );


	/**
	 * This function clears the current array of strings, which causes the tooltip to not be drawn.
	 */
	void ClearStringList()
	{
		TooltipStrings.Empty();
	}

	/** 
	 * This function takes a array of strings which will be drawn in order, from top to bottom,
	 * in a tooltip type box.
	 * @param InStrings The array of strings to use for drawing our tooltip box, they will not be reordered.
	 */
	void SetStringList(TArray<FString> &InStrings)
	{
		TooltipStrings = InStrings;
	}
private:

	/** This variable stores the list of handles we will use to generate our tooltip. */
	TArray<FString>	TooltipStrings;
	
};


/**
 * This class renders an outline for a UIScreenObject
 */
class FRenderModifier_ContainerOutline : public FUIEditor_RenderModifier
{
public:
	/**
	 * Constructor
	 *
	 * @param	InEditor	the editor window that holds the container to render the outline for
	 */
	FRenderModifier_ContainerOutline( class WxUIEditorBase* InEditor );

	/**
	 * Renders the outline for the top-level object of a UI editor window.
	 */
	virtual void Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas );

	FColor	OutlineColor;
};


/**
 * This class renders an outline for the viewport
 */
class FRenderModifier_ViewportOutline : public FUIEditor_RenderModifier
{
public:
	/**
	 * Constructor
	 *
	 * @param	InEditor	the editor window that holds the container to render the outline for
	 */
	FRenderModifier_ViewportOutline( class WxUIEditorBase* InEditor );

	/**
	 * Renders an outline indicating the size of the viewport as it is reported to the scene being edited.
	 */
	virtual void Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas );

	FColor	OutlineColor;
};

class FRenderModifier_CustomWidget : public FUIEditor_RenderModifier
{
public:
	FRenderModifier_CustomWidget( class WxUIEditorBase* InEditor, class UUIObject* InWidget )
	: FUIEditor_RenderModifier(InEditor), SelectedWidget(InWidget)
	{
	}

	class UUIObject* SelectedWidget;
};

/**
 * This class renders small lines at the boundaries of UIList column headers
 */
class FRenderModifier_UIListCellBoundary : public FRenderModifier_CustomWidget
{
public:
	/**
	 * Constructor
	 *
	 * @param	InEditor	the editor window that holds the list we want to render column header boundaries for
	 */
	FRenderModifier_UIListCellBoundary( class WxUIEditorBase* InEditor, class UUIList* InSelectedList );

	/**
	 * Renders lines indicating where a column header boundary is.
	 */
	virtual void Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas );

	/** the list associated with this render modifier */
	UUIList* SelectedList;
};

class FEditorUIObjectViewportClient : public FEditorObjectViewportClient
{
public:
	/** Constructor */
	FEditorUIObjectViewportClient( class WxUIEditorBase* InUIEditor );

	/** Destructor */
	virtual ~FEditorUIObjectViewportClient();

	/** Initialize this client's workers */
	virtual void InitializeClient();

	/**
	 * @return	TRUE if the UIScene associated with this viewport is open and initialized
	 */
	UBOOL HasValidUIScene() const;

	/**
	 * Retrieve the vectors marking the region within the viewport available for the object editor to render objects. 
	 *
	 * @param	StartPos					receives the value of the origin of the rendering region
	 * @param	EndPos						receives the value for the end point of the rendering region
	 * @param	bIncludeVirtualAdjustments	specify FALSE to ignore any gutter or offsets which may be in use by the viewport
	 */
	virtual void GetClientRenderRegion( FVector2D& StartPos, FVector2D& EndPos, UBOOL bIncludeVirtualAdjustments=TRUE );

	/**
	 * Determine whether the required buttons are pressed to begin a box selection.
	 */
	virtual UBOOL ShouldBeginBoxSelect() const;

	virtual UBOOL InputKeyUser (FName Key, EInputEvent Event);

	/**
	 * Check a key event received by the viewport.
	 * If the viewport client uses the event, it should return true to consume it.
	 * @param	Viewport - The viewport which the key event is from.
	 * @param	ControllerId - The controller which the key event is from.
	 * @param	Key - The name of the key which an event occured for.
	 * @param	Event - The type of event which occured.
	 * @param	AmountDepressed - For analog keys, the depression percent.
	 * @return	True to consume the key event, false to pass it on.
	 */
	virtual UBOOL InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed,UBOOL bGamepad=FALSE);

	/**
	 * Called when the user clicks anywhere in the viewport.
	 *
	 * @param	HitProxy	the hitproxy that was clicked on (may be null if no hit proxies were clicked on)
	 * @param	Click		contains data about this mouse click
	 */
	virtual void ProcessClick (HHitProxy* HitProxy, const FObjectViewportClick& Click);

	/**
	 * Called when the user moves the mouse while this viewport is capturing mouse input (i.e. dragging)
	 */
	virtual UBOOL InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad=FALSE);

	/**
	 * Called when the user moves the mouse while this viewport is not capturing mouse input (i.e. not dragging).
	 */
	virtual void MouseMove(FViewport* Viewport, INT X, INT Y);

	/**
	 * Create the appropiate drag tool.  Called when the user has dragged the mouse at least the minimum number of pixels.
	 *
	 * @return	a pointer to the appropriate drag tool given the currently pressed buttons, or NULL if no drag tool should be active
	 */
	virtual FObjectDragTool* CreateCustomDragTool();

	/**
	 * Creates or cleans up custom widget render modifiers based on the currently selected widgets.
	 */
	void UpdateWidgetRenderModifiers();

	/**
	 * Returns the index of the custom widget render modifier associated with the specified widget, or INDEX_NONE if there are none.
	 */
	INT FindCustomWidgetRenderModifierIndex( class UUIObject* SearchWidget );

	/**
	 * Renders 3D primitives into the viewport.
	 *
	 * @param Viewport - current editor viewport to render to
	 * @param Canvas - current canvas with render target to use
	 * @return TRUE if anything was rendered
	 * 
	 */
	virtual UBOOL DrawPrimitives( FViewport* Viewport, FCanvas* Canvas );

	/**
	* Renders the post process pass for the UI editor and copies the results to the Viewport render target
	*/
	virtual void DrawPostProcess( FViewport* Viewport, FCanvas* Canvas, UPostProcessChain* UIPostProcessChain );

	/** 
	 *	Rendering loop call for this viewport.
	 */
	virtual void Draw(FViewport* Viewport,FCanvas* Canvas);

	/**
	 * Returns the scene to use for rendering primitives in this viewport.
	 */
	virtual FSceneInterface* GetScene();

	/**
	 * @return Returns a mouse cursor depending on where the mouse is in the viewport and if a mouse tool is active.
	 */
	virtual EMouseCursor GetCursor(FViewport* Viewport,INT X,INT Y);

protected:
	/** Creates an object selection tool */
	virtual void CreateSelectionTool();

	/**
	 *	Creates a mouse tool depending on the current state of the editor and what key modifiers are held down.
	 *  @param HitProxy	Pointer to the hit proxy currently under the mouse cursor.
	 *  @return Returns a pointer to the newly created FMouseTool.
	 */
	virtual FMouseTool* CreateMouseTool(const HHitProxy* HitProxy);

public:
	/** the UIEditor that owns this viewport client */
	class WxUIEditorBase*	UIEditor;

	/** displays the outline which indicates the boundries of the viewport's region */
	class FRenderModifier_ViewportOutline*		ViewportOutline;

	/** array of render modifiers which are only active for specific widgets */
	TArray<class FRenderModifier_CustomWidget*>		WidgetRenderModifiers;
};


/**
 * This wrapper class for FEditorUIObjectViewportClient allows the viewport client to be used like a wxWindow.
 */
class WxObjectViewportHolder : public WxViewportHolder, public wxScrollHelper
{
public:
	WxObjectViewportHolder(wxWindow* InParent, wxWindowID InID, class WxUIEditorBase* InEditor);
	virtual ~WxObjectViewportHolder();

	/** Handler for when the pane is resized, updates the viewport. */
	void OnSize(wxSizeEvent &Event);

	class FEditorUIObjectViewportClient*	ObjectVC;

	DECLARE_EVENT_TABLE()
};


/**
 * Simple data structure which holds a reference to a FUIDataStoreBinding for a specific widget.
 */
struct FWidgetDataBindingReference
{
	/**
	 * The widget that contains the style references
	 */
	class UUIObject*	DataBindingOwner;

	/**
	 * A list of data store binding values contained by DataBindingOwner.  Normally these will correspond to the values for a single FUIDataStoreBinding property
	 * in the widget, so the array will usually have only one element unless the property associated with these data bindings is also an array.
	 */
	TArray<struct FUIDataStoreBinding*> Bindings;

	/** Constructor */
	FWidgetDataBindingReference() : DataBindingOwner(NULL)
	{}
	FWidgetDataBindingReference( class UUIObject* InOwner )
	: DataBindingOwner(InOwner)
	{}

	FORCEINLINE UBOOL operator==( const FWidgetDataBindingReference& Other ) const
	{
		return DataBindingOwner == Other.DataBindingOwner && Bindings == Other.Bindings;
	}
	FORCEINLINE UBOOL operator!=( const FWidgetDataBindingReference& Other ) const
	{
		return DataBindingOwner != Other.DataBindingOwner || Bindings != Other.Bindings;
	}
	friend FArchive& operator<<( FArchive& Ar, FWidgetDataBindingReference& Reference )
	{
		return Ar << Reference.DataBindingOwner << Reference.Bindings;
	}
};
typedef TMultiMap<UProperty*,FWidgetDataBindingReference>	FDataBindingPropertyValueMap;

/** Stores a property name in a widget and a possible style to assign to that widget. */
struct FUIDataBindingMenuItem
{
	/** The data binding property to apply the change to */
	FDataBindingPropertyValueMap DataBindingProperties;

	/** the array element to assign the data binding to; only relevant for array properties, obviously */
	INT ArrayElement;

	/** extra field available for use by clients of this struct */
	INT ExtraData;

	FUIDataBindingMenuItem()
	: ArrayElement(INDEX_NONE), ExtraData(INDEX_NONE)
	{
	}
	FUIDataBindingMenuItem( FDataBindingPropertyValueMap InDataBindingProperties, INT ArrayIndex=INDEX_NONE, INT InExtraData=INDEX_NONE )
	: DataBindingProperties(InDataBindingProperties), ArrayElement(ArrayIndex), ExtraData(InExtraData)
	{
	}

	FORCEINLINE UBOOL operator==( const FUIDataBindingMenuItem& Other ) const
	{
		return LegacyCompareEqual(DataBindingProperties,Other.DataBindingProperties) && ArrayElement == Other.ArrayElement && ExtraData == Other.ExtraData;
	}
	FORCEINLINE UBOOL operator!=( const FUIDataBindingMenuItem& Other ) const
	{
		return LegacyCompareNotEqual(DataBindingProperties,Other.DataBindingProperties) || ArrayElement != Other.ArrayElement || ExtraData != Other.ExtraData;
	}

	friend FArchive& operator<<( FArchive& Ar, FUIDataBindingMenuItem& Item )
	{
		return Ar << Item.DataBindingProperties;
	}
};

/**
 * Base class for windows for editing widgets.
 */
class WxUIEditorBase : public WxTrackableFrame, public FNotifyHook, public FObjectEditorInterface, public FSerializableObject, public FCallbackEventDevice, public FSelectionInterface, public FDockingParent
{
public:
	/** Enum that corresponds to a widget reordering action. */
	enum EReorderAction
	{
		RA_MoveUp,
		RA_MoveDown,
		RA_MoveToTop,
		RA_MoveToBottom
	};
	
	/** Stores a property name in a widget and a possible style to assign to that widget. */
	struct FUIStyleMenuPair
	{
		/** Name of the property to assign the style to. */
		struct FStyleReferenceId StylePropertyId;

		/** the array element to assign the style to, for style properties which are static arrays */
		INT ArrayElement;

		/** Style to assign to the property. */
		UUIStyle* Style;

		/** extra field available for use by clients of this struct */
		INT ExtraData;

		FUIStyleMenuPair()
		: ArrayElement(INDEX_NONE), Style(NULL), ExtraData(INDEX_NONE)
		{}
		FUIStyleMenuPair( const FStyleReferenceId& PropertyId, INT ArrayIndex, UUIStyle* InStyle, INT InExtraData=INDEX_NONE )
		: StylePropertyId(PropertyId), ArrayElement(ArrayIndex), Style(InStyle), ExtraData(InExtraData)
		{
		}

		FORCEINLINE UBOOL operator==( const FUIStyleMenuPair& Other )
		{
			return StylePropertyId == Other.StylePropertyId && ArrayElement == Other.ArrayElement && Style == Other.Style && ExtraData == Other.ExtraData;
		}

		friend FArchive& operator<<( FArchive& Ar, FUIStyleMenuPair& Pair )
		{
			return Ar << Pair.StylePropertyId.StyleProperty << Pair.Style;
		}
	};

	/** Constructor */
	WxUIEditorBase( wxWindow* InParent, wxWindowID InID, class UUISceneManager* inSceneManager, UUIScene* InScene );

	/** Destructor */
	virtual ~WxUIEditorBase();

	/**
	 * This function is called when the window has been selected from within the ctrl + tab dialog.
	 */
	virtual void OnSelected();

	/** the editor viewport that this window renders to */
	class FEditorUIObjectViewportClient*	ObjectVC;

	/** container for the viewport client */
	class WxObjectViewportHolder*			ViewportHolder;

	/** a pointer to the UISceneManager singleton */
	class UUISceneManager*					SceneManager;

	/** the widget that is being edited by this window */
	class UUIScene*							OwnerScene;

	/** stores the settings and options for this editor window */
	class UUIEditorOptions*					EditorOptions;

	/** Panel that lets the user modify position settings for the currently selected widget. */
	WxUIPositionPanel*						PositionPanel;

	/** Panel that lets the user modify docking settings for the currently selected widget. */
	class WxUIDockingPanel*					DockingPanel;

	/** displays the menu options for the main editor window */
	WxUIEditorMenuBar*						MenuBar;

	/** displays the tools for the ui editor */
	WxUIEditorMainToolBar*					MainToolBar;

	/** displays the tools for manipulating and placing widgets */
	WxUIEditorWidgetToolBar*				WidgetToolBar;

	/** The frame's status bar */
	class WxUIEditorStatusBar*					StatusBar;

	/** Main docking client pane for the window. */
	class WxUIPreviewPane*						PreviewPane;

	/** dispays the properties for the currentlys selected widgets */
	WxPropertyWindow*						PropertyWindow;

	/** contains all extra scene information windows, such as the scene tree and style browser */
	wxNotebook*								SceneToolsNotebook;

	/** displays the hierarchy of the widgets in the current scene */
	WxSceneTreeCtrl*						SceneTree;

	/** displays the hierarchy of layers in the current scene */
	class WxUILayerBrowser*					LayerBrowser;

	/** displays the available skins and styles */
	WxStyleBrowser*							StyleBrowser;

	/** displays the outline which indicates the boundries of the OwnerScene */
	FRenderModifier_ContainerOutline*		ContainerOutline;

	/** Displays a tooltip box when the user mouses over various UI elements. */
	FRenderModifier_TargetTooltip*			TargetTooltip;

	/** The currently active widget tool. */
	class FUIWidgetTool*					WidgetTool;

	class WxUIDragGridMenu*					DragGridMenu;

	/** A map of wx menu IDs to a struct of a property name and a style class. */
	TMap<INT, FUIStyleMenuPair>				SelectStyleMenuMap;

	/** mapping of wx menu Ids to a struct containing information about a style reference */
	TMap<INT,FUIStyleMenuPair>				EditStyleMenuMap;

	/** A map of wxMenu IDs to a struct which contains data about the property to bind */
	TMap<INT,FUIDataBindingMenuItem>		SetDataStoreBindingMap;

	/** A map of wxMenu IDs to a struct which contains data about the property to clear */
	TMap<INT,FUIDataBindingMenuItem>		ClearDataStoreBindingMap;

	/** Flag that lets us know when the window has been fully initialized. */
	UBOOL bWindowInitialized;

	/** Indicates that we received a selection synchronization request from someone else */
	UBOOL bSynchronizingSelection;

	UTexture2D*	BackgroundTexture;
	FString WinNameString;

public:
	/* =======================================
		Input methods and notifications
	   =====================================*/

	/**
	 * Called when the user right-clicks on an empty region of the viewport
	 */
	virtual void ShowNewObjectMenu();

	/**
	 * Called when the user right-clicks on an object in the viewport
	 *
	 * @param	ClickedObject	the object that was right-clicked on
	 */
	virtual void ShowObjectContextMenu( UObject* ClickedObject );


	/**
	 * Called when the user double-clicks an object in the viewport
	 *
	 * @param	Obj		the object that was double-clicked on
	 */
	virtual void DoubleClickedObject(UObject* Obj);

	/**
	 * Called when the user left-clicks on an empty region of the viewport
	 *
	 * @return	TRUE to deselect all selected objects
	 */
	virtual UBOOL ClickOnBackground();


	/**
	 * Called when the user releases the mouse after dragging the selection tool's rotation handle.
	 *
	 * @param	Anchor	The anchor to rotate the selected objects around.
	 * @param	DeltaRotation	the amount of rotation to apply to objects
	 */
	virtual void RotateSelectedObjects( const FVector& Anchor, const FRotator& DeltaRotation );

	/**
	 * Called when the user releases the mouse after dragging the selection tool's outline handle.
	 *
	 * @param	Delta	the number of pixels to move the objects
	 */
	virtual void MoveSelectedObjects( const FVector& Delta );

	/**
	 * Called when the user completes a drag operation while objects are selected.
	 *
	 * @param	Handle	the handle that was used to move the objects; determines whether the objects should be resized or moved.
	 * @param	Delta	the number of pixels to move the objects
	 * @param   bPreserveProportion		Whether or not we should resize objects while keeping their original proportions.
	 */
	virtual void MoveSelectedObjects( ESelectionHandle Handle, const FVector& Delta, UBOOL bPreserveProportion=FALSE  );

	/**
	 * Moves the anchor for the selected objects.
	 *
	 * @param	Delta	Change in the position of the anchor.
	 *
	 */
	void MoveSelectedObjectsAnchor(const FVector2D& Delta);

	/**
	 * Called when the user drags one of the resize handles of the selection tools.
	 *
	 * @param	Handle						the handle that was dragged
	 * @param	Delta					the number of pixels to move the objects
	 * @param   bPreserveProportion		Whether or not we should resize objects while keeping their original proportions.
	 */
	void ResizeSelectedObjects( ESelectionHandle Handle, const FVector& Delta, UBOOL bPreserveProportion=FALSE );

	/**
	 * Calculates start and end offsets for widget faces when resizing a widget.
	 *
	 * @param	Handle					the handle that was dragged
	 * @param   bPreserveProportion		Whether or not we should resize objects while keeping their original proportions.
	 * @param   AspectRatio				The aspect ratio to resize by if preserve proportion is TRUE, should most likely be (Widget Width / Widget Height).
	 * @param   OutStart				Offset for the start of the widget box (top left)
	 * @param   OutEnd					Offset for the end of the widget box (bottom right)
	 * @param	Widget					The widget that the handles modify. Primarily used to access the widget's rotation when calculating movement.
	 * @param	bIncludeTransform		specify TRUE to have Delta tranformed using the widget's rotation matrix. Normally, this will be TRUE when getting offsets
	 *									for rendering, and FALSE when getting offsets for applying to widgets' positions
	 *
	 */
	void GetResizeOffsets(ESelectionHandle Handle, UBOOL bPreserveProportion, FLOAT AspectRatio, FVector &OutStart, FVector &OutEnd, UUIObject* Widget=NULL, UBOOL bIncludeTranform=TRUE );

	/**
	 * Perform any perform special logic for key presses and override the viewport client's default behavior.
	 *
	 * @param	Viewport	the viewport processing the input
	 * @param	Key			the key that triggered the input event (KEY_LeftMouseButton, etc.)
	 * @param	Event		the type of event (press, release, etc.)
	 *
	 * @return	TRUE to override the viewport client's default handling of this event.
	 */
	virtual UBOOL HandleKeyInput(FViewport* Viewport, FName Key, EInputEvent Event);

	/**
	 * Called once when the user mouses over an new object.  Child classes should implement
	 * this function if they need to perform any special logic when an object is moused over
	 *
	 * @param	Obj		the object that is currently under the mouse cursor
	 */
//	virtual void OnMouseOver(UObject* Obj);

	/* =======================================
		General methods and notifications
	   =====================================*/
	/**
	* Generates a printable string using the widget name and the current dock face.
	*
	* @param InWidget		Widget that the dock handle belongs to.
	* @param DockFace		The dock handle we are generating a string for.
	* @param OutString		Container for the finalized string.
	*/
	void GetDockHandleString(const UUIScreenObject* InWidget, EUIWidgetFace DockFace, FString& OutString);

	/** @return Returns the viewport size in pixels for the current scene. */
	FVector2D GetViewportSize() const;

	/** @return Returns the current grid box size setting in pixels (1,2,4,8, etc.) for the editor window. */
	INT GetGridSize() const;

	/** @return Returns the top left origin of the grid in pixels. */
	FVector GetGridOrigin() const;

	/** @return Returns whether or not snap to grid is enabled. */
	UBOOL GetUseSnapping() const;

	/** Returns the pointer to the drag grid menu. */
	WxUIDragGridMenu* GetDragGridMenu()
	{
		return DragGridMenu;
	}

	/**
	 * Get the package that owns the objects that are being modified in this viewport
	 */
	virtual UObject* GetPackage() const;

	virtual void DrawBackground (FViewport* viewport, FCanvas* Canvas);
	virtual void DrawGrid (FViewport* viewport, FCanvas* Canvas);

	/**
	 * Child classes should implement this function to render the objects that are currently visible.
	 */
	virtual void DrawObjects(FViewport* Viewport, FCanvas* Canvas);

	/**
	 * Render outlines for selected objects which are being moved / resized.
	 */
	virtual void DrawDragSelection( FCanvas* Canvas, FMouseTool_DragSelection* DragTool );

	/**
	 * Called when the viewable region is changed, such as when the user pans or zooms the workspace.
	 */
	virtual void ViewPosChanged();

	/**
	 * Called when something happens in the linked object drawing that invalidates the property window's
	 * current values, such as selecting a new object.
	 * Child classes should implement this function to update any property windows which are being displayed.
	 */
	virtual void UpdatePropertyWindow();

	/**
	 * Called when items within the Main Tool Bar need to be refreshed as a result
	 * of the referenced data being changed.
	 */
	virtual void UpdateMainToolBar();

	/**
	 *	Refreshes only the values of the positioning panel, should NOT be used if the selection set has changed (Use UpdatePropertyWindow instead).
	 */
	void RefreshPositionPanelValues();

	/**
	 *	Refreshes only the values of the docking panel, should NOT be used if the selection set has changed (Use UpdatePropertyWindow instead).
	 */
	void RefreshDockingPanelValues();

	/**
	 * Displays the skin editor dialog.
	 */
	void ShowSkinEditor();

	/**
	 * Begins an undo transaction.
	 *
	 * @param	TransactionLabel	the label to place in the Undo menu
	 *
	 * @return	the value of UTransactor::Begin()
	 */
	virtual INT BeginTransaction( FString TransactionLabel );

	/**
	 * Cancels an active transaction
	 *
	 * @param	StartIndex	the index of the transaction to cancel. All transaction in the buffers after this index will be removed and
	 *						object states will be returned to this point.
	 */
	virtual void CancelTransaction( INT StartIndex );

	/**
	 * Ends an undo transaction.  Should be called when you are done modifying the objects.
	 */
	virtual void EndTransaction();

	/**
	 * Called immediately after the window is created, prior to the first call to Show()
	 */
	virtual void InitEditor();

	/**
	 * Register any callbacks this editor would like to receive
	 */
	virtual void RegisterCallbacks();

	/**
	 * Create any viewport modifiers for this editor window.  Called when the scene is opened.
	 */
	virtual void CreateViewportModifiers();

	/**
	 * Remove any viewport modifiers that were added by this editor.
	 */
	virtual void RemoveViewportModifiers();

	/* =======================================
		Selection methods and notifications
	   =====================================*/

	/**
	 * Retrieves the widgets corresonding to the selected items in the tree control
	 *
	 * @param	OutWidgets					Storage array for the currently selected widgets.
	 * @param	bIgnoreChildrenOfSelected	if TRUE, widgets will not be added to the list if their parent is in the list
	 */
	virtual void GetSelectedWidgets( TArray<UUIObject*> &OutWidgets, UBOOL bIgnoreChildrenOfSelected = FALSE );

	/**
	 * Changes the selection set to the widgets specified.
	 *
	 * @return	TRUE if the selection set was accepted
	 */
	virtual UBOOL SetSelectedWidgets( TArray<UUIObject*> SelectionSet );

	/**
	 * Synchronizes the selected widgets across all windows
	 *
	 * @param	Source	the object that requested the synchronization.  The selection set of this object will be
	 *					used as the authoritative selection set.
	 */
	virtual void SynchronizeSelectedWidgets( FSelectionInterface* Source );
	void SynchronizeSelectedWidgets( const wxCommandEvent& Event );

	/**
	 * Selects all objects within the specified region and notifies the viewport client so that it can
	 * reposition the selection overlay.
	 *
	 * @param	NewSelectionRegion	the box to use for choosing which objects to select.  Generally, all objects that fall within the bounds
	 *								of this box should be selected
	 *
	 * @return	TRUE if the selected objects were changed
	 */
	virtual UBOOL BoxSelect( FBox NewSelectionRegion );

	/**
	 * Called when the selected widgets are changed.
	 */
	virtual void SelectionChangeNotify();

	/**
	 * Calculates and updates the size of the selection handle to contain all selected objects.
	 *
	 * @param	bClearAnchorOffset		Whether or not to clear the current anchor offset(should be TRUE when the selection set changes).
	 */
	virtual void UpdateSelectionRegion(UBOOL bClearAnchorOffset = FALSE);

	/**
	 * Returns the number of selected objects
	 */
	virtual INT GetNumSelected() const;

	/**
	 * Called once the mouse is released after moving objects.
	 * Snaps the selected objects to the grid.
	 */
//	virtual void PositionSelectedObjects();

	// FNotifyHook interface

//	virtual void NotifyDestroy( void* Src );
//	virtual void NotifyPreChange( void* Src, FStructPropertyChain* PropertyAboutToChange );
	virtual void NotifyPostChange( void* Src, class FEditPropertyChain* PropertyThatChanged );
//	virtual void NotifyExec( void* Src, const TCHAR* Cmd );

	// FSeralizableObject interface.

	/**
	 * Method for serializing UObject references that must be kept around through GC's.
	 *
	 * @param Ar The archive to serialize with
	 */
	virtual void Serialize(FArchive& Ar);

	/* =======================================
	   FDockingWindow interface
	   =====================================*/

	/**
	 *	This function returns the name of the docking parent.  This name is used for saving and loading the layout files.
	 *  @return A string representing a name to use for this docking parent.
	 */
	virtual const TCHAR* GetDockingParentName() const;

	/**
	 * @return The current version of the docking parent, this value needs to be increased every time new docking windows are added or removed.
	 */
	virtual const INT GetDockingParentVersion() const;

	/* =======================================
		WxUIEditorBase interface.
	   =====================================*/
	/**
	 * Updates the editor window's titlebar with the tag of the scene we are currently editing.
	 */
	void UpdateTitleBar();

	/**
	 * Repopulates the layer browser with the widgets in the scene.
	 */
	virtual void RefreshLayerBrowser();

	/**
	 * Updates any UIPrefabInstances contained by this editor's scene.  Called when the scene is opened for edit.
	 */
	void UpdatePrefabInstances();

    /**
     * Creates the controls for this window
     */
	void CreateControls();

	/**
	 * Returns the size of the toolbar, if one is attached.  If the toolbar is configured for vertical layout, only the height
	 * member will have a value (and vice-versa).
	 */
	wxSize GetToolBarDisplacement() const;

	/**
	 * Determines whether the viewport boundary indicator should be rendered
	 * 
	 * @return	TRUE if the viewport boundary indicator should be rendered
	 */
	UBOOL ShouldRenderViewportOutline() const;

	/**
	 * Determines whether the container boundary indicator should be rendered
	 * 
	 * @return	TRUE if the container boundary indicator should be rendered
	 */
	UBOOL ShouldRenderContainerOutline() const;

	/**
	 * Determines whether handles for manipulating the selected objects should be rendered
	 * 
	 * @return	TRUE if the selection indicator should be rendered
	 */
	virtual UBOOL ShouldRenderSelectionHandles() const;

	/**
	 * Gets the window that generated this wxCommandEvent
	 */
	wxWindow* GetEventGenerator( const wxCommandEvent& Event ) const;

	/**
	 * Find the widget that is the lowest in the hierarchy which is the parent of all widgets in the set.
	 *
	 * @param	DefaultParent	the widget that is the top-most parent in the scene (usually the scene itself, unless editing a UIPrefab)
	 * @param	SourceWidgets	the set of widgets to find a common parent for
	 */
	static UUIScreenObject* FindCommonParent( UUIScreenObject* DefaultParent, const TArray<UUIObject*>& SourceWidgets );

	/**
	 * Displays a dialog for selecting a single widget
	 *
	 * @param	WidgetList	the list of widgets that should be available in the selection dialog.
	 */
	UUIScreenObject* DisplayWidgetSelectionDialog( TArray<UUIScreenObject*> WidgetList );

	/**
	 * Displays the rename dialog for each of the widgets specified.
	 *
	 * @param	Widgets		the list of widgets to rename
	 *
	 * @return	TRUE if at least one of the specified widgets was renamed successfully
	 */
	UBOOL DisplayRenameDialog( TArray<UUIObject*> Widgets );

	/**
	 * Displays a dialog requesting the user to input the package name, group name, and name for a new UI archetype.
	 *
	 * @param	out_ArchetypePackage	will receive a pointer to the package that should contain the UI archetype
	 * @param	out_ArchetypeGroup		will receive a pointer to the package that should be the group that will contain the UI archetype;
	 *									NULL if the user doesn't specify a group name in the dialog.
	 * @param	out_ArchetypeName		will receive the value the user entered as the name to use for the new UI archetype
	 *
	 *
	 * @return	TRUE if the user chose OK and all parameters were valid (i.e. a package was successfully created and the name chosen is valid); FALSE otherwise.
	 */
	UBOOL DisplayCreateArchetypeDialog( UPackage*& out_ArchetypePackage, UPackage*& out_ArchetypeGroup, FString& out_ArchetypeName );

	/**
	 * Displays the rename dialog for the scene specified.
 	 *
	 * @param	Scene		The scene to rename
	 *
	 * @return	TRUE if the scene was renamed successfully.
	 */
	UBOOL DisplayRenameDialog( UUIScene* Scene );


	/**
	 * Tries to rename a UIScene, if unsuccessful, will return FALSE and display a error messagebox.
	 *
	 * @param Scene		Scene to rename.
	 * @param NewName	New name for the object.
	 *
	 * @return	Returns whether or not the object was renamed successfully.
	 */
	UBOOL RenameObject( UUIScene* Scene, const TCHAR* NewName );

	/**
	 * Tries to rename a UUIObject, if unsuccessful, will return FALSE and display a error messagebox.
	 *
	 * @param Widget		Widget to rename.
	 * @param NewName	New name for the object.
	 *
	 * @return	Returns whether or not the object was renamed successfully.
	 */
	UBOOL RenameObject( UUIObject* Widget, const TCHAR* NewName );

	/**
	 * Change the parent of the specified widgets.
	 * 
	 * @param	Widgets		the widgets to change the parent for
	 * @param	NewParent	the object that should be the widget's new parent
	 *
	 * @return	TRUE if the widget's parent was successfully changed.
	 */
	UBOOL ReparentWidgets( TArray<UUIObject*> Widgets, UUIScreenObject* NewParent );

	/**
	 * Change the parent of the specified widget.
	 * 
	 * @param	Widget		the widgets to change the parent for
	 * @param	NewParent	the object that should be the widget's new parent
	 *
	 * @return	TRUE if the widget's parent was successfully changed.
	 */
	UBOOL ReparentWidget( UUIObject* Widget, UUIScreenObject* NewParent, TArray<UUIObject*>* ExclusionSet=NULL );

	/**
 	 * Centers on the currently selected widgets.
	 */
	void CenterOnSelectedWidgets();

	/**
	 * Copy the current selected widgets.
	 */
	void CopySelectedWidgets();

	/**
	 * Cuts the current selected widgets.
	 */
	void CutSelectedWidgets();

	/**
	 * Deletes the currently selected widgets.
	 */
	void DeleteSelectedWidgets();


	/**
 	 * Changes the position of a widget in the widget parent's Children array, which affects the render order of the scene
	 * Called when the user selects a "Move Up/Move ..." command in the context menu
	 *
	 * @param Action - The type of reordering action to perform on each widget that is selected.
	 */
	void ReorderSelectedWidgets( EReorderAction Action );

	/**
	 * Repositions the specified widget within its parent's Children array.
	 *
	 * @param	Widget					the widget to move
	 * @param	NewPosition				the new position of the widget. the value will be clamped to the size of the parent's Children array
	 * @param	bRefreshSceneBrowser	if TRUE, calls RefreshLayerBrowser to update the scene browser tree control
	 */
	UBOOL ReorderWidget( UUIObject* Widget, INT NewPosition, UBOOL bRefreshSceneBrower=TRUE );

	// Notifications
	/**
	 * Notification that the specified style was modified.  Calls ApplyStyle on all widgets in the scene which use this style.
	 *
	 * @param	ModifiedStyle	the style that was modified
	 */
	void NotifyStyleModified( UUIStyle* ModifiedStyle );

	/**
	 * Places a new instance of a widget in the current scene.
	 *
	 * @param	PlacementParameters		parameters for placing the widget in the scene
	*
	 * @return	a pointer to the widget that was created, or NULL if the widget couldn't be placed.
	*/
	UUIObject* CreateNewWidget( const FUIWidgetPlacementParameters& PlacementParameters );

	/**
	 * Creates a new instance of a UIPrefab and inserts into the current scene.
	 *
	 * @param	PlacementParameters		parameters for placing the UIPrefabInstance in the scene
	 *
	 * @return	a pointer to the UIPrefabInstance that was created, or NULL if the widget couldn't be placed.
	 */
	class UUIPrefabInstance* InstanceUIPrefab( const FUIWidgetPlacementParameters& PlacementParameters );

	/**
	 * Constructs a UI Widget of the specified type at the specified location with the specified size.
	 *
	 * @param TypeNum	Type index of the widget to create.
	 * @param PosX		X Position of the new widget.
	 * @param PosY		Y Position of the new widget.
	 * @param SizeX		Width of the new widget.
	 * @param SizeY		Height of the new widget.
	 * @return			Pointer to the new widget, returns NULL if the operation failed.
	 */
	UUIObject* CreateWidget(INT TypeNum, INT PosX, INT PosY, INT SizeX, INT SizeY);

	/**
	 * Uses UUISceneFactoryText to create new widgets based on the contents of the windows clipboard.
	 * @param InParent		The parent to assign to the widgets we are pasting.
	 * @param bClickPaste	Whether or not the user right clicked to paste the widgets.  If they did, then paste the widgets at the position of the mouse click.
	 */
	void PasteWidgets( class UUIScreenObject* InParent=NULL, UBOOL bClickPaste=FALSE );

	/**
	 * Replaces the specified widget with a new widget of the specified class.  The new widget
	 * will be selected if the widget being replaced was selected.
	 * 
	 * @param	WidgetToReplace			the widget to replace; must be a valid widget contained within this editor window
	 * @param	ReplacementArchetype	the archetype to use for the replacement widget
	 *
	 * @return	a pointer to the newly created widget, or NULL if the widget couldn't be replaced
	 */
	UUIObject* ReplaceWidget( UUIObject* WidgetToReplace, UUIObject* ReplacementArchetype );

	/**
	 * Creates archetypes for the specified widgets and groups them together into a resource that can be placed into a UIScene 
	 * or another UIArchetype as a unit.
	 *
	 * @param	SourceWidgets	the widgets to create archetypes of
	 *
	 * @return	TRUE if the archetype was successfully created using the specified widgets.
	 */
	UBOOL CreateUIArchetype( TArray<UUIObject*>& SourceWidget );

	/**
	 * Creates a new UISkin in the specified package
	 *
	 * @param	PackageName		the name of the package to create the skin in.  This can be the name of an existing package or a completely new package
	 * @param	SkinName		the name to use for the new skin
	 * @param	SkinArchetype	the skin to use as the archetype for the new skin
	 *
	 * @return	a pointer to the newly created UISkin, or NULL if it couldn't be created.
	 */
	class UUISkin* CreateUISkin( const FString& PackageName, const FString& SkinName, UUISkin* SkinArchetype );

	/** Updates various wxWidgets elements. */
	void UpdateUI();

	/** Shows the docking editor for the currently selected widgets. */
	void ShowDockingEditor();

	/** Shows the event editor for the currently selected widgets. */
	void ShowEventEditor();

	/**
	 * Changes the platform that the UI editor should simulate.
	 *
	 * @param	DesiredPreviewPlatform	the name of the platform to set; should match the string returned by calling FConsoleSupport::GetConsoleName() (FConsoleSupportIterator)
	 *
	 * @return	TRUE if the platform was changed successfully.
	 */
	UBOOL SetPreviewPlatform( const FString& DesiredPreviewPlatform );

	/**
	 * Determines whether the currently selected data store tag is valid for the widgets specified.
	 *
	 * @param	WidgetsToBind	the list of widgets that we want to bind to the currently selected data store tag
	 *
	 * @return	TRUE if all of the widgets specified support the data store field type of the currently selected data store tag
	 */
	UBOOL IsSelectedDataStoreTagValid( TArray<class UUIObject*>& WidgetsToBind );

	/* =======================================
		wxFrame interface
	   =====================================*/

	/**
	 * Called when a toolbar is requested by wxFrame::CreateToolBar
	 */
	virtual wxToolBar* OnCreateToolBar(long style, wxWindowID id, const wxString& name);

	/* =======================================
		Windows event handlers.
	   =====================================*/
	void OnUpdateUI( wxUpdateUIEvent& Event );

	/** Updates the toolbar buttons corresponding to the various UI tool modes */
	void OnUpdateUIToolMode( wxUpdateUIEvent& Event );

	void SetToolMode (INT mode);


private:
	/* =======================================
		Toolbar/Menubar event handlers.
	   =====================================*/
	void OnToolbar_SelectViewportSize( wxCommandEvent& Event );
	void OnToolbar_SelectViewportGutterSize( wxSpinEvent& Event );
	void OnToolbar_EnterViewportGutterSize( wxCommandEvent& Event );
	void OnToolbar_SelectWidgetPreviewState( wxCommandEvent& Event );
	void OnToolbar_SelectPreviewPlatform( wxCommandEvent& Event );
	void OnToolbar_SelectSSPreviewMode( wxCommandEvent& Event );

	void OnMenu_View (wxCommandEvent& Event);
	void OnToggleOutline( wxCommandEvent& Event );
	void OnToolbar_SelectMode (wxCommandEvent& Event);
	void OnMenuDragGrid_UpdateUI( wxUpdateUIEvent& Event );
	void OnMenuDragGrid( wxCommandEvent& In );
	void OnMenuAlignToViewport( wxCommandEvent& In );
	void OnMenuAlignText( wxCommandEvent& In );
	void OnMenuCenterOnSelection( wxCommandEvent& In );

	/* =======================================
	    File menu handlers.
	=====================================*/
	void OnCloseEditorWindow(wxCommandEvent &In);

	/* =======================================
		Edit menu handlers.
	   =====================================*/
	void OnUndo( wxCommandEvent& Event );
	void OnRedo( wxCommandEvent& Event );
	void OnCopy( wxCommandEvent& Event );
	void OnCut( wxCommandEvent& Event );
	void OnPaste( wxCommandEvent& Event );
	void OnPasteAsChild( wxCommandEvent& Event );
	void OnBindUIEventKeys( wxCommandEvent& Event );
	void OnShowDataStoreBrowser( wxCommandEvent& Event );

	/*  Called when the user right clicks and pastes. */
	void OnContext_Paste( wxCommandEvent& Event );

	/*  Called when the user right clicks and pastes. */
	void OnContext_PasteAsChild( wxCommandEvent& Event );

	void OnEditMenu_UpdateUI( wxUpdateUIEvent& Event );

	/* =======================================
		Skin menu handlers.
	   =====================================*/

	/** Called when the user selects the "Load Existing Skin" option in the load skin context menu */
	void OnLoadSkin( wxCommandEvent& Event );

	/** Called when the user selects the "Create New Skin" option in the load skin context menu */
	void OnCreateSkin( wxCommandEvent& Event );

	void OnEditSkin( wxCommandEvent& Event );
	void OnSaveSkin( wxCommandEvent& Event );
	void OnSkinMenu_UpdateUI( wxUpdateUIEvent& Event );

	/* =======================================
		Scene menu handlers.
	   =====================================*/
	void OnForceSceneUpdate( wxCommandEvent& Event );

	/* =======================================
		Context menu handlers.
	   =====================================*/
	void OnContext_CreateNew( wxCommandEvent& Event );
	void OnContext_CreateNewFromArchetype( wxCommandEvent& Event );
	void OnContext_Rename( wxCommandEvent& Event );
	void OnContext_Delete( wxCommandEvent& Event );
	void OnContext_SelectStyle( wxCommandEvent& Event );
	void OnContext_EditStyle( wxCommandEvent& Event );
	void OnContext_EditDockingSet( wxCommandEvent& Event );
	void OnContext_ShowKismet( wxCommandEvent& Event );
	void OnContext_ChangeParent( wxCommandEvent& Event );
	void OnContext_ConvertPosition( wxCommandEvent& Event );
	void OnContext_ModifyRenderOrder( wxCommandEvent& Event );
	void OnContext_DumpProperties( wxCommandEvent& Event );
	void OnContext_SaveAsResource( wxCommandEvent& Event );
	void OnContext_BindWidgetsToDataStore( wxCommandEvent& Event );
	void OnContext_ClearWidgetsDataStore( wxCommandEvent& Event );

	/**
	 * Shows the edit events dialog for any selected widgets.  If no widgets are selected, it shows it for the scene.
	 */
	void OnContext_EditEvents( wxCommandEvent& Event );

	/* =======================================
		UI List Context menu handlers.
	=====================================*/
	void OnContext_BindListElement( wxCommandEvent& Event );

	/* =======================================
		UITabControl custom context menu item handlers.
	=====================================*/
	void OnContext_InsertTabPage( wxCommandEvent& Event );
	void OnContext_RemoveTabPage( wxCommandEvent& Event );

	/* =======================================
		UUICalloutPanel custom context menu item handlers.
	=====================================*/
	void OnContext_InsertCalloutButton( wxCommandEvent& Event );
	void OnContext_RemoveCalloutButton( wxCommandEvent& Event );
	void OnContext_ChangeCalloutButtonAlias( wxCommandEvent& Event );

	void OnSize( wxSizeEvent& Event );
	void OnClose( wxCloseEvent& Event );
	DECLARE_EVENT_TABLE()


public:

	/** FCallbackEventDevice interface */
	virtual void Send( ECallbackEventType InType );
	virtual void Send( ECallbackEventType InType, UObject* InObject );
	virtual void Send( FCallbackEventParameters& Parms );
};

class WxUIKismetUpdate : public WxKismetUpdate
{
public:
	WxUIKismetUpdate( WxKismet* InKismet, wxWindowID InID )
	: WxKismetUpdate(InKismet, InID)
	{}

	/**
	 * Generates a list of sequence objects that are out of date and populates the list with their names
	 */
	virtual void BuildUpdateList();

	void OnClose( wxCloseEvent& In );

	DECLARE_EVENT_TABLE()
};

/**
 * This class allows the designer to edit the kismet sequences, events and actions for a widget.
 * @fixme - this is a temporary implementation...not very well designed
 */
class WxUIKismetEditor : public WxKismet
{
	typedef WxKismet Super;

//	DECLARE_DYNAMIC_CLASS(WxUIKismetEditor)

public:
	/** Constructor */
	WxUIKismetEditor( WxUIEditorBase* InParent, wxWindowID InID, class UUIScreenObject* InSequenceOwner, FString Title=TEXT("Kismet") );
	~WxUIKismetEditor();

	/**
	 * Display the kismet editor for the specified sequence.  If there is already a kismet editor
	 * open for the sequence, brings it to the front and gives focus.  Otherwise, creates a new kismet
	 * editor for the sequence.
	 *
	 * @param	InSeq			the sequence to edit; the global sequence for this sequence's owner widget will be used as
	 *							the root node of the kismet editor
	 * @param	ParentWindow	the window which contains the sequence's owner widget
	 *
	 * @return	reference to the kismet editor window that was opened
	 */
	static WxUIKismetEditor* OpenUISequenceInKismet(UUISequence* InSeq, WxUIEditorBase* ParentWindow);

	/**
	 * Creates the tree control for this linked object editor.  Only called if TRUE is specified for bTreeControl
	 * in the constructor.
	 *
	 * @param	TreeParent	the window that should be the parent for the tree control
	 */
	virtual void CreateTreeControl( wxWindow* TreeParent );

	/**
	 * One or more objects changed, so update the tree control
	 */
	virtual void NotifyObjectsChanged();

	/**
	 * Updates the property window with the properties for the currently selected sequence objects, or if no sequence objects are selected
	 * and a UIState is selected in the tree control, fills the property window with the properties of the selected state.
	 */
	virtual void UpdatePropertyWindow();

	/** 
	 * Opens the context menu to let new users create new kismet objects. 
	 *
	 * This version opens up a UI Kismet specific menu.
	 */
	virtual void OpenNewObjectMenu();

	/**
	 * Displays a context menu when the user right-clicks an object in the kismet editor.
	 * This version adds additional options specific to the UI when certain types of objects are selected.
	 */
	virtual void OpenObjectOptionsMenu();
	/**
	 * Displays a context menu when the user right-clicks the connector node for a link on an object in the kismet editor.
	 * This version adds additional options specific to the UI when certain types of objects are selected.
	 */
	virtual void OpenConnectorOptionsMenu();

	/**
	 * Changes the active sequence for the kismet editor.
	 *
	 * This version changes the titlebar of the kismet editor to a path based on the widget that the sequenence belongs to.
	 */
	virtual void ChangeActiveSequence(USequence *NewSeq, UBOOL bNotifyTree=true);

	/**
	 * @return Returns a friendly/printable name for the sequence provided.
	 */
	FString GetFriendlySequenceName(UUISequence *Sequence);

	/** the widget that owns the top-level sequence that is being edited by this window */
	UUIScreenObject* SequenceOwner;

	/** The owner UI editor. */
	WxUIEditorBase* UIEditor;

	/** dialog for updating multiple sequence objects at once */
	WxUIKismetUpdate* SequenceUpdateDialog;

protected:
	/** 
	 *	Create a new Object Variable and fill it in with the currently selected Widgets.
	 *	If bAttachVarsToConnector is TRUE, it will also connect these new variables to the selected variable connector.
	 *
	 *  This version uses selected UI widgets instead of Actors.
	 */
	virtual void OnContextNewScriptingObjVarContext( wxCommandEvent& In );

	/**
	 * Recursively loops through all of the widgets in the scene and creates input event meta objects for their state sequences.
	 *
	 * @param InSequenceOwner	The widget parent of the sequences we will be generating new meta objects for.
	 */
	void CreateStateSequenceMetaObject(class UUIScreenObject* InSequenceOwner);

	/**
	 * Returns whether the specified class can be displayed in the list of ops which are available to be placed.
	 *
	 * @param	SequenceClass	a child class of USequenceObject
	 *
	 * @return	TRUE if the specified class should be available in the context menus for adding sequence ops
	 */
	virtual UBOOL IsValidSequenceClass( UClass* SequenceClass ) const;

private:
	/** Called when the user selects the "Add Callout Button" context menu option */
	void OnContext_AddCalloutButton( wxCommandEvent& Event );
	/** Called when the user selects the "Remove Callout Button" context menu option */
	void OnContext_RemoveCalloutButton( wxCommandEvent& Event );
	/** Called when the user selects the "Change Callout Alias" context menu option */
	void OnContext_ChangeCalloutButtonAlias( wxCommandEvent& Event );

	/** Forces the kismet editor tree control to be refreshed. */
	virtual void OnRefreshTree( wxCommandEvent& Event );

	/** Called when a new item has been selected in the tree control */
	virtual void OnTreeItemSelected( wxTreeEvent& Event );

	/**
	 * Node was added to selection in view so select in tree
	 *
	 * @param	Obj		Object to select
	 */
	virtual void AddToSelection( UObject* Obj );

	/**
	 * Selection set was emptied, so unselect everything in the tree.
	 */
	virtual void EmptySelection();
	
	/**
	 * Node was removed from selection in view so deselect in tree.
	 *
	 * @param	Obj		Object to deselect
	 */
	virtual void RemoveFromSelection( UObject* Obj );

	/** === WxKismetEditor interface === */

	/** Called when the user clicks the "Update List" button. */
	void OnOpenUpdate( wxCommandEvent &In );

	/** Keeps the tree control changed event from doing anything when we know that we will be making a large number of changes to the selection set. */
	INT UpdatesLocked;

	DECLARE_EVENT_TABLE()
};

/**
 * This struct contains the class, names, and currently selected item for a single style type.
 */
struct FStyleSelectionData : public FSerializableObject
{
	/** the UIStyle class and name represented by this data structure */
	const FUIStyleResourceInfo&	ResourceInfo;

	/** Constructor */
	FStyleSelectionData( const FUIStyleResourceInfo& InResourceInfo );

	/** Removes all templates from this style selection */
	void ClearTemplates()
	{
		StyleTemplates.Empty();
		SetCurrentTemplate(NULL);
	}

	/**
	 * Adds the specified style to this selection's list of available templates
	 */
	void AddStyleTemplate( UUIStyle* InTemplate );

	/**
	 * Retrieves the list of templates for this style type
	 *
	 * @param	out_Templates	[out] filled with the templates associated with this style type
	 */
	void GetStyleTemplates( TArray<UUIStyle*>& out_Templates )
	{
		out_Templates = StyleTemplates;
	}

	/**
	 * Sets the index for the currently selected template
	 */
	void SetCurrentTemplate( UUIStyle* SelectedTemplate )
	{
		CurrentTemplate = SelectedTemplate != NULL
			? StyleTemplates.FindItemIndex(SelectedTemplate)
			: INDEX_NONE;
	}

	/**
	 * Retrieves the currently selected template for this style type
	 */
	INT GetCurrentTemplate() const
	{
		return StyleTemplates.IsValidIndex(CurrentTemplate) ? CurrentTemplate : INDEX_NONE;
	}

	//====================================
	// FSerializableObject interface
	//====================================
	/**
	 * Serializes the references to the style templates for this selection
	 */
	virtual void Serialize( FArchive& Ar );

private:

	/** the UIStyle objects which can be used as a template for this style type */
	TArray<UUIStyle*>			StyleTemplates;

	/**
	 * the index (into StyleTemplates) for the currently selected style template for this type
	 * -1 if there is no template or the template index is invalid
	 */
	INT							CurrentTemplate;

};

/*
 * This dialog is shown when the user chooses to create a new UI style.  It allows the user to configure the unique tag, friendly name, and archetype for the new style.
 */
class WxDlgCreateUIStyle: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( WxDlgCreateUIStyle )
	DECLARE_EVENT_TABLE()

public:
	/**
	 * Constructor
	 * Initializes the available list of style type selections.
	 */
    WxDlgCreateUIStyle();

	/**
	 * Destructor
	 * Saves the window position and size to the ini
	 */

	~WxDlgCreateUIStyle();

	/**
	 * Initialize this dialog.  Must be the first function called after creating the dialog.
	 *
	 * @param	InParent	the window that opened this dialog
	 * @param	InID		the ID to use for this dialog
	 */
    void Create( wxWindow* InParent, wxWindowID InID=ID_UI_NEWSTYLE_DLG );

	/**
	 * Displays this dialog.
	 *
	 * @param	InStyleTag		the default value for the "Unique Name" field
	 * @param	InFriendlyName	the default value for the "Friendly Name" field
	 * @param	InTemplate		the style template to use for the new style
	 */
	int ShowModal( const FString& InStyleTag, const FString& InFriendlyName, UUIStyle* InTemplate=NULL );

	/**
	 * Initializes the list of available styles, along with the templates available to each one.
	 */
	void InitializeStyleCollection();

	/**
	 * Retrieve the style class that was selected by the user
	 */
	UClass* GetStyleClass() const;

	/**
	 * Retrieve the style tag set by the user
	 */
	FString GetStyleTag() const
	{
		return StyleTagString.c_str();
	}

	/**
	 * Retrieve the friendly name set by the user
	 */
	FString GetFriendlyName() const
	{
		return FriendlyNameEditbox != NULL ? FriendlyNameEditbox->GetValue().c_str() : TEXT("");
	}

	/**
	 * Retrieve the style template that was selected by the user
	 */
	UUIStyle* GetStyleTemplate() const;

	/** this control displays the available UIStyle classes */
    wxRadioBox* StyleClassSelect;

	/** this editbox allows the user the enter the unique StyleTag for the new style */
    wxTextCtrl* TagEditbox;

	/** this editbox allows the user the enter a friendly name for the new style */
    wxTextCtrl* FriendlyNameEditbox;

	/** this dropdown allows the user to choose an archetype for the new UIStyle */
    wxChoice* TemplateCombo;

protected:
	
    /**
     * Creates the controls and sizers for this dialog.
     */
    void CreateControls();

	/**
	 * Retrieves the style selection data associated with the specified style object's class
	 *
	 * @param	StyleObject			the style object to find selection data for
	 * @param	out_SelectionData	will be filled with the index corresponding to the
	 *								selection data for the specified style object's class
	 *
	 * @return	TRUE if selection data was successfully found for the specified style class
	 */
	UBOOL GetSelectionData( UUIStyle* StyleObject, INT& out_SelectionIndex );

	/**
	 * Populates the style template combo with the list of UIStyle objects which can be used as an archetype for the new style.
	 * Only displays the styles that correspond to the selected style type.
	 *
	 * @param	SelectedStyleIndex	the index of the selected style; corresponds to an element in the StyleChoices array
	 */
	void PopulateStyleTemplateCombo( INT SelectedStyleIndex=INDEX_NONE );

private:
	/** local storage for the style tag - necessary for the wxTextValidator */
	wxString			StyleTagString;

	/** pointer to the scene manager singleton */
	UUISceneManager*	SceneManager;

	/** data about each of the available style types */
	TIndirectArray<FStyleSelectionData>	StyleChoices;

	/* =======================================
		Windows event handlers.
	   =====================================*/

	/**
	 * This handler is called when the user selects an item in the radiobox.
	 */
	void OnStyleTypeSelected( wxCommandEvent& Event );

	/**
	 * This handler is called when the user selects an item in the choice control
	 */
	void OnStyleTemplateSelected( wxCommandEvent& Event );
};

/**
 * This dialog allows the user to create a new UISkin.  It displays text controls for entering the package and skin name, and a combo
 * (currently not editable) for choosing the template.
 */
class WxDlgCreateUISkin : public wxDialog
{
	DECLARE_DYNAMIC_CLASS(WxDlgCreateUISkin);
	DECLARE_EVENT_TABLE()

public:
	/** Default constructor for use in two-stage dynamic window creation */
	WxDlgCreateUISkin() : PackageNameEdit(NULL), SkinNameEdit(NULL), SkinTemplateCombo(NULL), OKButton(NULL)
	{}

	/**
	 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	 *
	 * @param	InParent				the window that opened this dialog
	 * @param	InID					the ID to use for this dialog
	 */
	void Create( wxWindow* InParent, wxWindowID InID=ID_UI_NEWSKIN_DLG ); 

	/**
     * Creates the controls for this window
     */
	void CreateControls();

	/**
	 * Displays this dialog.
	 *
	 * @param	InPackageName	the default value for the "Package Name" field
	 * @param	InSkinName		the default value for the "Skin Name" field
	 * @param	InTemplate		the template to use for the new skin
	 */
	INT ShowModal( const FString& InPackageName, const FString& InSkinName, UUISkin* InTemplate );

	/** Called when the user clicks the OK button - verifies that the names entered are unique */
	virtual bool Validate();

	/**
	 * Returns the UUISkin corresponding to the selected item in the template combobox
	 */
	UUISkin* GetSkinTemplate();

	WxTextCtrl*	PackageNameEdit;
	WxTextCtrl*	SkinNameEdit;
	wxChoice*	SkinTemplateCombo;

	/** the OK button */
	wxButton*			OKButton;

	/** local storage for the package and skin names - necessary for the wxTextValidator */
	wxString	PackageNameString;
	wxString	SkinNameString;

private:
	void OnTextEntered( wxCommandEvent& Event );
};

/**
 * This modal dialog allows the user to edit the docking set for a widget in the scene.  All editing is handled by an internal WxUIDockingSetEditorPanel object; the dialog
 * is responsible for propagating changes made to the docking parameters back to the selected widget.
 */
class WxDlgDockingEditor : public wxDialog
{
	DECLARE_DYNAMIC_CLASS(WxDlgDockingEditor)

public:

	/** Default constructor required by two-stage dynamic window creation */
	WxDlgDockingEditor()
	{}

	/**
	 * Destructor
	 * Saves the window position
	 */
	virtual ~WxDlgDockingEditor();

	/**
	 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	 *
	 * @param	InParent				the window that opened this dialog
	 * @param	InID					the ID to use for this dialog
	 * @param	AdditionalButtonIds		additional buttons that should be added to the dialog....currently only ID_CANCEL_ALL is supported
	 */
	void Create( wxWindow* InParent, wxWindowID InID=ID_UI_DOCKINGEDITOR_DLG, long AdditionalButtonIds=0 ); 

	/**
	 * Displays this dialog.
	 *
	 * @param	Container	the object which contains InWidget
	 * @param	InWidget	the widget to edit the docking sets for
	 */
	INT ShowModal( UUIScreenObject* Container, UUIObject* InWidget );

	/**
	 * Applies changes made in the dialog to the widget being edited.  Called from the owner window when ShowModal()
	 * returns the OK code.
	 *
	 * @return	TRUE if all changes were successfully applied
	 */
	virtual UBOOL SaveChanges();

	/** the panel which contains all of the controls used for editing docking links */
	class WxUIDockingSetEditorPanel* EditorPanel;

	/** the OK button */
	wxButton*		btn_OK;

protected:

	/**
     * Creates the controls for this window
	 *
	 * @param	AdditionalButtonIds		additional buttons that should be added to the dialog....currently only ID_CANCEL_ALL is supported
     */
	void CreateControls( long AdditionalButtonIds=0 );
};

enum EStyleIcon
{
	SI_Text,
	SI_Image,
	SI_Combo,
	SI_MAX
};

#endif	// __UIEDITOR_H__
