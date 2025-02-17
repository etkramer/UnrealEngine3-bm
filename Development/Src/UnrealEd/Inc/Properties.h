/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__

#include "Bitmaps.h"

#define PROP_CategoryHeight		24
#define PROP_CategorySpacer		8
#define PROP_GenerateHeight		-1
#define	PROP_DefaultItemHeight	20
#define PROP_Indent				16


class WxPropertyWindow;
class WxPropertyWindowFrame;
class UPropertyDrawProxy;
class UPropertyInputProxy;
class WxPropertyWindow_Objects;

///////////////////////////////////////////////////////////////////////////////
//
// Property window toolbars.
//
///////////////////////////////////////////////////////////////////////////////

class WxPropertyWindowToolBarBase : public wxToolBar
{
public:
	WxPropertyWindowToolBarBase( wxWindow* InParent, wxWindowID InID );

	/**
	 * Assembles the toolbar and loads bitmaps.  Not reentrant.
	 */
	UBOOL Init();

protected:

	/**
	 * Callback to allow derived classes to add widgets to the toolbar.
	 * Derived classes should always call up to their parent implementation.
	 */
	virtual UBOOL OnInit();
};

/**
 * A toolbar of buttons that can be added to the top of any WxPropertyWindowFrame.
 * Derived classes can add buttons by implementing 
 */
class WxPropertyWindowToolBarCopyLock : public WxPropertyWindowToolBarBase
{
public:
	WxPropertyWindowToolBarCopyLock( wxWindow* InParent, wxWindowID InID );

protected:
	virtual UBOOL OnInit();

private:
	/** The bitmaps used by the copy buttons. */
	WxMaskedBitmap CopyB;
	WxMaskedBitmap CopyCompleteB;
	WxMaskedBitmap ExpandAllB;
	WxMaskedBitmap CollapseAllB;

	/** The bitmap used by the lock button. */
	WxMaskedBitmap LockB;
};

///////////////////////////////////////////////////////////////////////////////
//
// Property windows.
//
///////////////////////////////////////////////////////////////////////////////

class FDeferredInitializationWindow
{
	/** whether Create has been called on this window yet */
	UBOOL	bCreated;

public:
	FDeferredInitializationWindow()
		:	bCreated( FALSE )
	{}

	virtual ~FDeferredInitializationWindow() {}

	/** returns whether Create has been called on this window yet */
	UBOOL IsCreated() const
	{
		return bCreated;
	}

	/** Changes the value of bCreated to true...called when this window is created */
	void RegisterCreation()
	{
		bCreated = TRUE;
	}
};

/*-----------------------------------------------------------------------------
	WxPropertyWindow_Base
-----------------------------------------------------------------------------*/

/**
 * Base class for all window types that appear in the property window.
 */
class WxPropertyWindow_Base : public wxWindow, public FDeferredInitializationWindow
{
public:
	DECLARE_DYNAMIC_CLASS(WxPropertyWindow_Base);

	/** Destructor */
	virtual ~WxPropertyWindow_Base();

	/**
	 * Initialize this property window.  Must be the first function called after creating the window.
	 */
	virtual void Create(	wxWindow* InParent,
							WxPropertyWindow_Base* InParentItem,
							WxPropertyWindow* InTopPropertyWindow,
							UProperty* InProperty,
							INT InPropertyOffset,
							INT	 InArrayIdx,
							UBOOL bInSupportsCustomControls=FALSE );

	/** How far the text label for this item is indented when drawn. */
	INT IndentX;

	/** The items parented to this one. */
	TArray<WxPropertyWindow_Base*> ChildItems;

	/** Is this item expanded? */
	UBOOL					bExpanded;
	/** Does this item allow expansion? */
	UBOOL					bCanBeExpanded;
	/** Parent item/window. */
	WxPropertyWindow_Base*	ParentItem;

protected:
	/** The top level property window.  Always valid. */
	WxPropertyWindow*		TopPropertyWindow;

public:
	/** LL next. */
	WxPropertyWindow_Base*	Next;

	/** LL prev. */
	WxPropertyWindow_Base*	Prev;

	/** The property being displayed/edited. */
	UProperty*				Property;

	/** The proxy used to draw this windows property. */
	UPropertyDrawProxy*		DrawProxy;

	/** The proxy used to handle user input for this property. */
	UPropertyInputProxy*	InputProxy;

	/** Offset to the properties data. */
	INT						PropertyOffset;
	INT						ArrayIndex;

	/** Used when the property window is positioning/sizing child items. */
	INT						ChildHeight;
	/** Used when the property window is positioning/sizing child items. */
	INT						ChildSpacer;

	/** The last x position the splitter was at during a drag operation. */
	INT						LastX;

protected:
	/** Should the child items of this window be sorted? */
	UBOOL					bSorted;

public:
	/** Map of menu identifiers to classes - for editinline popup menus. */
	TMap<INT,UClass*>		ClassMap;

	/** The base class for creating a list of classes applicable to editinline items. */
	UClass*					PropertyClass;

	/** TRUE if the property can be expanded into the property window. */
	UBOOL bEditInline;

	/** TRUE if the property is EditInline with a use button. */
	UBOOL bEditInlineUse;

	/** Flag indicating whether or not only a single object is selected. */
	UBOOL bSingleSelectOnly;

	/** Flag indicating whether this property window supports custom controls */
	UBOOL bSupportsCustomControls;

	/**
	 * Returns the property window this node belongs to.  The returned pointer is always valid.
	 */
	WxPropertyWindow* GetPropertyWindow();

	/**
	 *  @return	The height of the property item, as determined by the input and draw proxies.
	 */
	INT GetPropertyHeight();

	/**
	 * Returns a string representation of the contents of the property.
	 */
	FString GetPropertyText();

	/**
	 * Follows the chain of items upwards until it finds the object window that houses this item.
	 */
	WxPropertyWindow_Objects* FindObjectItemParent();

	// The bArrayPropertiesCanDifferInSize flag is an override for array properties which want to display
	// e.g. the "Clear" and "Empty" buttons, even though the array properties may differ in the number of elements.
	virtual UBOOL GetReadAddress(WxPropertyWindow_Base* InItem,
								 UBOOL InRequiresSingleSelection,
								 TArray<BYTE*>& OutAddresses,
								 UBOOL bComparePropertyContents = TRUE,
								 UBOOL bObjectForceCompare = FALSE,
								 UBOOL bArrayPropertiesCanDifferInSize = FALSE);

	/**
	 * fills in the OutAddresses array with the addresses of all of the available objects.
	 * @param InItem		The property to get objects from.
	 * @param OutAddresses	Storage array for all of the objects' addresses.
	 */
	virtual UBOOL GetReadAddress(WxPropertyWindow_Base* InItem,
								 TArray<BYTE*>& OutAddresses);

	virtual BYTE* GetBase( BYTE* Base );
	virtual BYTE* GetContents( BYTE* Base );

	/**
	 * Creates a path to the current item.
	 */
	virtual FString GetPath() const;

	/**
	 * Appends my path, including an array index (where appropriate)
	 */
	virtual void AppendPathWithArrayIndex( FString& PathPlusIndex ) const;

	/**
	 * Checks to see if an X position is on top of the property windows splitter bar.
	 */
	UBOOL IsOnSplitter( INT InX );

	/**
	 * Calls through to the enclosing property window's NotifyPreChange.
	 */
	void NotifyPreChange(UProperty* PropertyAboutToChange, UObject* Object=NULL);

	/**
	 * Calls through to the enclosing property window's NotifyPreChange.
	 */
	void NotifyPostChange(UProperty* PropertyThatChanged, UObject* Object=NULL);

	///////////////////////////////////////////////////////////////////////////
	// Item expansion

	/**
	 * Expands child items.  Does nothing if the window is already expanded.
	 */
	virtual void Expand();

	/**
	 * Collapses child items.  Does nothing if the window is already collapsed.
	 */
	void Collapse();

	void RememberExpandedItems(TArray<FString>& InExpandedItems);
	void RestoreExpandedItems(const TArray<FString>& InExpandedItems);

	/**
	 * Recursively searches through children for a property named PropertyName and expands it.
	 * If it's a UArrayProperty, the propery's ArrayIndex'th item is also expanded.
	 */
	virtual void ExpandItem( const FString& PropertyName, INT ArrayIndex );

	/**
	 * Expands all items rooted at the property window node.
	 */
	virtual void ExpandAllItems();

	/**
	 * Expands all category items belonging to this window.
	 */
	virtual void ExpandCategories();

	/**
	 * Recursively searches through children for a property named PropertyName and collapses it.
	 * If it's a UArrayProperty, the propery's InArrayIndex'th item is also collapsed.
	 */
	virtual void CollapseItem( const FString& PropertyName, INT InArrayIndex );

	///////////////////////////////////////////////////////////////////////////
	// Event handling

	void OnLeftClick( wxMouseEvent& In );
	void OnLeftUnclick( wxMouseEvent& In );
	void OnLeftDoubleClick( wxMouseEvent& In );
	void OnRightClick( wxMouseEvent& In );
	void OnSize( wxSizeEvent& In );
	void OnSetFocus( wxFocusEvent& In );
	void OnPropItemButton( wxCommandEvent& In );
	void OnEditInlineNewClass( wxCommandEvent& In );
	void OnMouseMove( wxMouseEvent& In );
	void OnChar( wxKeyEvent& In );
	void SortChildren();

	//////////////////////////////////////////////////////////////////////////
	// Sorting
	UBOOL IsSorted() const				{ return bSorted; }
	void SetSorted(UBOOL bInSorted)		{ bSorted = bInSorted; }

	//////////////////////////////////////////////////////////////////////////
	// Custom property editor support
	UBOOL SupportsCustomControls() const { return bSupportsCustomControls; }
	void SetCustomControlSupport(UBOOL bShouldSupportCustomControls);

	/**
	 * Returns the draw proxy class for the property contained by this item.
	 */
	UClass* GetDrawProxyClass() const;

	/**
	 * Get the input proxy class for the property contained by this item.
	 */
	UClass* GetInputProxyClass() const;

	/**
	 * Serializes the input and draw proxies, as well as all child windows.
	 *
	 * @param		Ar		The archive to read/write.
	 */
	virtual void Serialize( FArchive& Ar );

	DECLARE_EVENT_TABLE();

protected:
	/**
	 * Called when an property window item receives a left-mouse-button press which wasn't handled by the input proxy.  Typical response is to gain focus
	 * and (if the property window item is expandable) to toggle expansion state.
	 *
	 * @param	Event	the mouse click input that generated the event
	 *
	 * @return	TRUE if this property window item should gain focus as a result of this mouse input event.
	 */
	virtual UBOOL ClickedPropertyItem( wxMouseEvent& In );

	/**
	 * Creates the appropriate editing control for the property specified.
	 *
	 * @param	InProperty	the property that will use the new property item
	 * @param	ArrayIndex	specifies which element of an array property that this property window will represent.  Only valid
	 *						when creating property window items for individual elements of an array.
	 * @param	ParentItem	specified the property window item that will contain this new property window item.  Only
	 *						valid when creating property window items for individual array elements or struct member properties
	 */
	class WxPropertyWindow_Item* CreatePropertyItem( class UProperty* InProperty, INT ArrayIndex=INDEX_NONE, class WxPropertyWindow_Item* ParentItem=NULL );

	/**
	 * Called by Expand(), creates any child items for any properties within this item.
	 */
	virtual void CreateChildItems();

	/**
	 * Called by Collapse(), deletes any child items for any properties within this item.
	 */
	void DeleteChildItems();

	/**
	 * Finalizes draw and input proxies.  Must be called by derived constructors of
	 * WxPropertyWindow_Base-derived classes.  Child class implementations of FinishSetup
	 * should call up to their parent.
	 */
	virtual void FinishSetup( UProperty* InProperty );

	/**
	 * Sets the target property value for property-based coloration to the value of
	 * this property window node's value.
	 */
	void SetPropertyColorationTarget();

private:
	/**
	 * Implements OnLeftDoubleClick().
	 */
	void PrivateLeftDoubleClick();
};

/*-----------------------------------------------------------------------------
	WxPropertyWindow_Objects
-----------------------------------------------------------------------------*/

/**
 * This holds all the child controls and anything related to
 * editing the properties of a collection of UObjects.
 */
class WxPropertyWindow_Objects : public WxPropertyWindow_Base
{
public:
	DECLARE_DYNAMIC_CLASS(WxPropertyWindow_Objects);

	//////////////////////////////////////////////////////////////////////////
	// Constructors

	/**
	 * Initialize this property window.  Must be the first function called after creating the window.
	 */
	virtual void Create(	wxWindow* InParent,
							WxPropertyWindow_Base* InParentItem,
							WxPropertyWindow* InTopPropertyWindow,
							UProperty* InProperty,
							INT InPropertyOffset,
							INT InArrayIdx,
							UBOOL bInSupportsCustomControls=FALSE);

	//////////////////////////////////////////////////////////////////////////
	// Object binding

	/**
	 * Adds a new object to the list.
	 */
	void AddObject( UObject* InObject );

	/**
	 * Removes an object from the list.
	 */
	void RemoveObject(UObject* InObject);

	/**
	 * Returns TRUE if the specified object is in this node's object list.
	 */
	UBOOL HandlesObject(UObject* InObject) const;

	/**
	 * Removes all objects from the list.
	 */
	void RemoveAllObjects();

	/**
	 * Called when the object list is finalized, Finalize() finishes the property window setup.
	 */
	void Finalize();

	//////////////////////////////////////////////////////////////////////////
	// Misc accessors

	// The bArrayPropertiesCanDifferInSize flag is an override for array properties which want to display
	// e.g. the "Clear" and "Empty" buttons, even though the array properties may differ in the number of elements.
	virtual UBOOL GetReadAddress(WxPropertyWindow_Base* InItem,
								 UBOOL InRequiresSingleSelection,
								 TArray<BYTE*>& OutAddresses,
								 UBOOL bComparePropertyContents = TRUE,
								 UBOOL bObjectForceCompare = FALSE,
								 UBOOL bArrayPropertiesCanDifferInSize = FALSE);

	/**
	 * fills in the OutAddresses array with the addresses of all of the available objects.
	 * @param InItem		The property to get objects from.
	 * @param OutAddresses	Storage array for all of the objects' addresses.
	 */
	virtual UBOOL GetReadAddress(WxPropertyWindow_Base* InItem,
		TArray<BYTE*>& OutAddresses);


	virtual BYTE* GetBase( BYTE* Base );
	virtual FString GetPath() const;

	/**
	 * Appends my path, including an array index (where appropriate)
	 */
	virtual void AppendPathWithArrayIndex( FString& PathPlusIndex ) const;

	/**
	 * Handles paint events.
	 */
	void OnPaint( wxPaintEvent& In );

	/** Does nothing to avoid flicker when we redraw the screen. */
	void OnEraseBackground( wxEraseEvent& Event );

	//////////////////////////////////////////////////////////////////////////
	// Object iteration

	typedef TArray<UObject*>::TIterator TObjectIterator;
	typedef TArray<UObject*>::TConstIterator TObjectConstIterator;

	TObjectIterator			ObjectIterator()			{ return TObjectIterator( Objects ); }
	TObjectConstIterator	ObjectConstIterator() const	{ return TObjectConstIterator( Objects ); }

	/** Returns the number of objects for which properties are currently being edited. */
	INT GetNumObjects() const	{ return Objects.Num(); }

	// @todo DB: remove this method which is just a hack for UUnrealEdEngine::UpdatePropertyWindows.
	void RemoveActor(AActor* Actor);

	//////////////////////////////////////////////////////////////////////////
	// Base class

	/** @return		The base-est baseclass for objects in this list. */
	UClass*			GetBaseClass()			{ return BaseClass; }
	
	/** @return		The base-est baseclass for objects in this list. */
	const UClass*	GetBaseClass() const	{ return BaseClass; }

	/** @return		The property stored at this node, to be passed to Pre/PostEditChange. */
	UProperty*		GetStoredProperty()		{ return StoredProperty; }

protected:
	/** The list of objects we are editing properties for. */
	TArray<UObject*>		Objects;

	/** The lowest level base class for objects in this list. */
	UClass*					BaseClass;

	/**
	 * The property passed to Pre/PostEditChange calls.  Separated from WxPropertyWindow_Base::Property
	 * because WxPropertyWindow_Objects is structural only, and Property should be NULL so this node
	 * doesn't eg return values for GetPropertyText(), etc.
	 */
	UProperty*				StoredProperty;

	virtual void CreateChildItems();

private:
	/**
	 * Looks at the Objects array and creates the best base class.  Called by
	 * Finalize(); that is, when the list of selected objects is being finalized.
	 */
	void SetBestBaseClass();

	DECLARE_EVENT_TABLE();
};

/*-----------------------------------------------------------------------------
	WxPropertyWindow_Category
-----------------------------------------------------------------------------*/

/**
 * The header item for a category of items.
 */
class WxPropertyWindow_Category : public WxPropertyWindow_Base
{
public:
	DECLARE_DYNAMIC_CLASS(WxPropertyWindow_Category);

	/**
	 * Initialize this property window.  Must be the first function called after creating the window.
	 */
	virtual void Create(	FName InCategoryName,
							wxWindow* InParent,
							WxPropertyWindow_Base* InParentItem,
							WxPropertyWindow* InTopPropertyWindow,
							UProperty* InProperty,
							INT InPropertyOffset,
							INT InArrayIdx,
							UBOOL bInSupportsCustomControls=FALSE);

	/** Does nothing to avoid flicker when we redraw the screen. */
	void OnEraseBackground( wxEraseEvent& Event )
	{
		// Intentionally do nothing.
	}

	void OnPaint( wxPaintEvent& In );
	void OnLeftClick( wxMouseEvent& In );
	void OnRightClick( wxMouseEvent& In );
	void OnMouseMove( wxMouseEvent& In );

	virtual FString GetPath() const;

	/**
	 * Appends my path, including an array index (where appropriate)
	 */
	virtual void AppendPathWithArrayIndex( FString& PathPlusIndex ) const;

	const FName& GetCategoryName() const { return CategoryName; }

	DECLARE_EVENT_TABLE();

protected:
	/** The category name. */
	FName	CategoryName;

	virtual void CreateChildItems();
};

/*-----------------------------------------------------------------------------
	WxPropertyWindow_Item
-----------------------------------------------------------------------------*/

class WxPropertyWindow_Item : public WxPropertyWindow_Base
{
public:
	DECLARE_DYNAMIC_CLASS(WxPropertyWindow_Item);

	/**
	 * Initialize this property window.  Must be the first function called after creating the window.
	 */
	virtual void Create(	wxWindow* InParent,
							WxPropertyWindow_Base* InParentItem,
							WxPropertyWindow* InTopPropertyWindow,
							UProperty* InProperty,
							INT InPropertyOffset,
							INT InArrayIdx,
							UBOOL bInSupportsCustomControls=FALSE);

	virtual BYTE* GetBase( BYTE* Base );
	virtual BYTE* GetContents( BYTE* Base );

	/** Does nothing to avoid flicker when we redraw the screen. */
	void OnEraseBackground( wxEraseEvent& Event )
	{
		// Intentionally do nothing.
	}

	void OnPaint( wxPaintEvent& In );
	void OnInputTextEnter( wxCommandEvent& In );
	void OnInputTextChanged( wxCommandEvent& In );
	void OnInputComboBox( wxCommandEvent& In );

	/**
	 * Renders the left side of the property window item.
	 *
	 * @param	RenderDeviceContext		the device context to use for rendering the item name
	 * @param	ClientRect				the bounding region of the property window item
	 */
	virtual void RenderItemName( wxBufferedPaintDC& RenderDeviceContext, const wxRect& ClientRect );

	DECLARE_EVENT_TABLE();

protected:
	virtual void CreateChildItems();

private:
	UBOOL DiffersFromDefault(UObject* InObject, UProperty* InProperty);
	UBOOL ComesFromArray() const;
};

///////////////////////////////////////////////////////////////////////////////
//
// Property window.
//
///////////////////////////////////////////////////////////////////////////////

/**
 * Top level window for any property window (other than the optional WxPropertyWindowFrame).
 * WxPropertyWindow objects automatically register/unregister themselves with GPropertyWindowManager
 * on construction/destruction.
 */
class WxPropertyWindow : public wxWindow, public FDeferredInitializationWindow
{
public:
	DECLARE_DYNAMIC_CLASS(WxPropertyWindow);

	/** Destructor */
	virtual ~WxPropertyWindow();

	/**
	 * Initialize this window class.  Must be the first function called after creation.
	 *
	 * @param	parent			The parent window.
	 * @param	InNotifyHook	An optional callback that receives property PreChange and PostChange notifies.
	 */
	virtual void Create( wxWindow* InParent, FNotifyHook* InNotifyHook );

	/**
	 * Sets an object in the property window.  T must be derived from UObject; will not
	 * compile if this is not the case.
	 *
	 * @param	InObject				The object to bind to the property window.  Can be NULL.
	 * @param	InExpandCategories		If TRUE, expand property categories.
	 * @param	InSorted				If TRUE, properties in this window are sorted alphabetically.
	 * @param	InShowCategories		If TRUE, show categories in this window.
	 * @param	InShowNonEditable		If TRUE, show properties which aren't editable in the None group.
	 */
	void SetObject( UObject* InObject,
					UBOOL InExpandCategories,
					UBOOL InSorted,
					UBOOL InShowCategories,
					UBOOL InShowNonEditable = 0 );

	/**
	 * Sets multiple objects in the property window.  T must be derived from UObject; will not
	 * compile if this is not the case.
	 *
	 * @param	InObjects				An array of objects to bind to the property window.
	 * @param	InExpandCategories		If TRUE, expand property categories.
	 * @param	InSored					If TRUE, properties in this window are sorted alphabetically.
	 * @param	InShowCategories		If TRUE, show categories in this window.
	 * @param	InShowNonEditable		If TRUE, show properties which aren't editable in the None group.
	 */
	template< class T >
	void SetObjectArray(const TArray<T*>& InObjects,
						UBOOL InExpandCategories,
						UBOOL InSorted,
						UBOOL InShowCategories,
						UBOOL InShowNonEditable = 0 )
	{
		Freeze();
		PreSetObject();
		for( INT ObjectIndex = 0 ; ObjectIndex < InObjects.Num() ; ++ObjectIndex )
		{
			if ( InObjects(ObjectIndex) && (GWorld == NULL || ( GWorld->HasBegunPlay() || InObjects(ObjectIndex) != static_cast<UObject*>(GWorld->GetBrush()) ) ) )
			{
				Root->AddObject( static_cast<UObject*>( InObjects(ObjectIndex) ) );
			}
		}
		PostSetObject( InExpandCategories, InSorted, InShowCategories, InShowNonEditable );
		Thaw();
	}

	//////////////////////////////////////////////////////////////////////////
	// Object access

	typedef WxPropertyWindow_Objects::TObjectIterator		TObjectIterator;
	typedef WxPropertyWindow_Objects::TObjectConstIterator	TObjectConstIterator;

	TObjectIterator			ObjectIterator()			{ return Root->ObjectIterator(); }
	TObjectConstIterator	ObjectConstIterator() const	{ return Root->ObjectConstIterator(); }

	/** Utility for finding if a particular object is being edited by this property window. */
	UBOOL					IsEditingObject(const UObject* InObject) const;

	//////////////////////////////////////////////////////////////////////////
	// Moving between items

	/**
	 * Moves focus to the item/category that follows InItem.
	 */
	void MoveFocusToNextItem( WxPropertyWindow_Base* InItem );

	/**
	 * Moves focus to the item/category that preceeds InItem.
	 */
	void MoveFocusToPrevItem( WxPropertyWindow_Base* InItem );

	//////////////////////////////////////////////////////////////////////////
	// Item expansion

	TMultiMap<FName,FString> ExpandedItems;

	/**
	 * Recursively searches through children for a property named PropertyName and expands it.
	 * If it's a UArrayProperty, the propery's ArrayIndex'th item is also expanded.
	 */
	void ExpandItem( const FString& PropertyName, INT ArrayIndex=INDEX_NONE);

	/**
	 * Expands all items in this property window.
	 */
	void ExpandAllItems();

	/**
	 * Recursively searches through children for a property named PropertyName and collapses it.
	 * If it's a UArrayProperty, the propery's ArrayIndex'th item is also collapsed.
	 */
	void CollapseItem( const FString& PropertyName, INT ArrayIndex=INDEX_NONE);

	/**
	 * Collapses all items in this property window.
	 */
	void CollapseAllItems();

	/**
	 * Rebuild all the properties and categories, with the same actors 
	 *
	 * @param IfContainingObject Only rebuild this property window if it contains the given object in the object hierarchy
	 */
	void Rebuild(UObject* IfContainingObject = NULL);

	/**
	* Queues a message in the property window message queue to Rebuild all the properties and categories, with the same actors,  
	*
	* @param IfContainingObject Only rebuild this property window if it contains the given object in the object hierarchy
	*/
	void PostRebuild(UObject* IfContainingObject = NULL);

	/**
	 * Freezes rebuild requests, so we do not process them.  
	 */
	void FreezeRebuild();

	/**
	 * Resumes rebuild request processing.
	 *
	 * @param bRebuildNow	Whether or not to rebuild the window right now if rebuilding is unlocked.
	 */
	void ThawRebuild(UBOOL bRebuildNow=TRUE);

	//////////////////////////////////////////////////////////////////////////
	// Change notification

	/**
	 * Calls PreEditChange on Root's objects, and passes PropertyAboutToChange to NotifyHook's NotifyPreChange.
	 */
	void NotifyPreChange(WxPropertyWindow_Base* InItem, UProperty* PropertyAboutToChange, UObject* Object);

	/**
	 * Calls PostEditChange on Root's objects, and passes PropertyThatChanged to NotifyHook's NotifyPostChange.
	 */
	void NotifyPostChange(WxPropertyWindow_Base* InItem, UProperty* PropertyThatChanged, UObject* Object);

	/**
	 * Passes a pointer to this property window to NotifyHook's NotifyDestroy.
	 */
	void NotifyDestroy();

	//////////////////////////////////////////////////////////////////////////
	// Splitter

	/**
	 * @return		The current splitter position.
	 */
	INT GetSplitterPos() const			{ return SplitterPos; }

	/**
	 * Moves the splitter position by the specified amount.
	 */
	void MoveSplitterPos(INT Delta)		{ SplitterPos += Delta; }

	/**
	 * @return		TRUE if the splitter is currently being dragged.
	 */
	UBOOL IsDraggingSplitter() const		{ return bDraggingSplitter; }

	void StartDraggingSplitter()			{ bDraggingSplitter = 1; }
	void StopDraggingSplitter()				{ bDraggingSplitter = 0; }

	//////////////////////////////////////////////////////////////////////////
	// Last focused

	/** Returns the last property that had focus. */
	WxPropertyWindow_Base* GetLastFocused()						
	{ 
		return LastFocused; 
	}

	/** Returns the last property that had focus. */
	const WxPropertyWindow_Base* GetLastFocused() const			
	{ 
		return LastFocused; 
	}


	/** Sets the last property that had focus. */
	void SetLastFocused(WxPropertyWindow_Base* InLastFocused)	
	{ 
		LastFocused = InLastFocused; 

		// Anytime LastFocused changes, we need to clear the bColorPickModeEnabled flag.
		bColorPickModeEnabled = FALSE;
	}

	/** Clears reference to the last property that had focus. */
	void ClearLastFocused()										
	{ 
		LastFocused = NULL; 

		// Anytime LastFocused changes, we need to clear the bColorPickModeEnabled flag.
		bColorPickModeEnabled = FALSE;
	}

	/**
	 * Flushes the last focused item to the input proxy.
	 */
	void FlushLastFocused();

	/**
	 * Relinquishes focus to LastFocused.
	 */
	void FinalizeValues();

	/**
	 * Positions existing child items so they are in the proper positions, visible/hidden, etc.
	 */
	void PositionChildren();

	/**
	 * Looks up the expansion tree to determine the hide/show status of the specified item.
	 *
	 * @param		InItem		The item for which to determine visibility status.
	 * @return					TRUE if InItem is showing, FALSE otherwise.
	 */
	UBOOL IsItemShown( WxPropertyWindow_Base* InItem );

	/**
	 * Updates the scrollbar values.
	 */
	void UpdateScrollBar();

	//////////////////////////////////////////////////////////////////////////
	// Color Picking

	/** 
	 * Used for color picking callbacks.  If the color picking flag is set then this function will update the selected property with the color.
	 *
	 * @param InColor	Color that was picked from scene.
	 */
	void OnColorPick( const FColor& InColor );

	/** Sets whether or not color pick mode is enabled. */
	void SetColorPickModeEnabled ( UBOOL bEnableMode )
	{
		bColorPickModeEnabled = bEnableMode;
	}

	/** Returns whether or not color pick mode is enabled. */
	UBOOL GetColorPickModeEnabled() const
	{
		return bColorPickModeEnabled;
	}

	//////////////////////////////////////////////////////////////////////////
	// Event Handling

	void OnRefresh( wxCommandEvent& In );
	void OnScroll( wxScrollEvent& In );
	void OnMouseWheel( wxMouseEvent& In );
	void OnSize( wxSizeEvent& In );
	void OnKeyDown(wxKeyEvent &In);
	WxPropertyWindow_Objects* GetRoot() const { return Root; }

	//////////////////////////////////////////////////////////////////////////
	// Sorting

	UBOOL IsSorted() const				{ return bSorted; }
	void SetSorted(UBOOL bInSorted)		{ bSorted = bInSorted; }


	/** Returns true if we want to allow the ENTER key to be intercepted by text property fields and used to
	    apply the text input.  This is usually what you want.  Otherwise, the ENTER key will be ignored
		and the parent dialog window will have a chance to handle the event */
	UBOOL AllowEnterKeyToApplyChanges() const
	{
		return bAllowEnterKeyToApplyChanges;
	}

	/** Sets whether the ENTER key will apply changes to text fields.  Must be set before controls are created. */
	void SetAllowEnterKeyToApplyChanges( UBOOL bIn )
	{
		bAllowEnterKeyToApplyChanges = bIn;
	}


	//////////////////////////////////////////////////////////////////////////
	// Custom property editor support
	UBOOL SupportsCustomControls() const { return bSupportsCustomControls; }
	void SetCustomControlSupport(UBOOL bShouldSupportCustomControls)
	{
		bSupportsCustomControls = bShouldSupportCustomControls;

		// propagate the call to the root
		if ( Root != NULL )
		{
			Root->SetCustomControlSupport(bSupportsCustomControls);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Show categories

	UBOOL ShouldShowCategories() const					{ return bShowCategories; }
	void SetShowCategories(UBOOL bInShowCategories)		{ bShowCategories = bInShowCategories; }

	//////////////////////////////////////////////////////////////////////////
	// Non-editable properties

	UBOOL ShouldShowNonEditable() const					{ return bShowNonEditable; }
	void SetShowNonEditable(UBOOL bInShowNonEditable)	{ bShowNonEditable = bInShowNonEditable; }

	//////////////////////////////////////////////////////////////////////////
	// Read-only properties
	UBOOL IsReadOnly() const				{ return bReadOnly;			}
	void SetReadOnly(UBOOL bInReadOnly)		{ bReadOnly = bInReadOnly; }

	/**
	* Sets the bCanBeHiddenByPropertyWindowManager flag.
	*
	* @param		bInCanBeHiddenByPropertyWindowManager 
	*/
	void SetCanBeHiddenByPropertyWindowManager(UBOOL bInCanBeHiddenByPropertyWindowManager)	
	{
		bCanBeHiddenByPropertyWindowManager = bInCanBeHiddenByPropertyWindowManager;
	}

	/**
	* Gets the bCanBeHiddenByPropertyWindowManager flag.\
	*/
	UBOOL GetCanBeHiddenByPropertyWindowManager() const
	{
		return bCanBeHiddenByPropertyWindowManager;
	}


	/**
	 * Serializes the root WxPropertyWindow_Objects object contained in the window.
	 *
	 * @param		Ar		The archive to read/write.
	 */
	void Serialize( FArchive& Ar );

	/** Mutator for bExecDeselectOnEscape */
	void SetExecDeselectOnEscape(UBOOL bValue)
	{
		bExecDeselectOnEscape = bValue;
	}

	/** Accessor for bExecDeselectOnEscape */
	UBOOL GetExecDeslectOnEscape() const
	{
		return bExecDeselectOnEscape;
	}

	/**
	 * Recursively shows the buttons for every visible property item.
	 */
	void ShowAllItemButtons(WxPropertyWindow_Base *Item);

	/**
	 * Recursively hides the buttons for every visible property item.
	 */
	void HideAllItemButtons(WxPropertyWindow_Base *Item);

	/**
	 * Event handler for showing/hiding all of the property item buttons.
	 */
	void OnShowAllItemButtons(wxCommandEvent &Event);

	DECLARE_EVENT_TABLE();

protected:
	/** The path names to every expanded item in the tree. */
	//TArray<FString> ExpandedItems;

	/** Flag for whether or not the ACTOR DESELECT command is executed when the Escape key is pressed. */
	UBOOL bExecDeselectOnEscape;

	/** The position of the break between the variable names and their values/editing areas. */
	INT SplitterPos;

	/** The item window that last had focus. */
	WxPropertyWindow_Base* LastFocused;

	/** If non-zero, we do not process rebuild requests, this should be set before operations that may update the property window multiple times and cleared afterwards. */
	INT RebuildLocked;

	/** If 1, the user is dragging the splitter bar. */
	UBOOL bDraggingSplitter;

	/** Should the child items of this window be sorted? */
	UBOOL bSorted;

	/** If TRUE, show property categories in this window. */
	UBOOL bShowCategories;

	/** If TRUE, show non-editable properties in this window. */
	UBOOL bShowNonEditable;

	/** The property is read-only		*/
	UBOOL bReadOnly;

	/** Flag indicating whether this property window supports custom controls */
	UBOOL bSupportsCustomControls;

	/** Flag indicating whether this property window can be hidden by the PropertyWindowManager. */
	UBOOL bCanBeHiddenByPropertyWindowManager;

	/** Flag indicating whether the property window should react to color pick events. */
	UBOOL bColorPickModeEnabled;

	/** True if we want to allow the ENTER key to be intercepted by text property fields and used to
	    apply the text input.  This is usually what you want.  Otherwise, the ENTER key will be ignored
		and the parent dialog window will have a chance to handle the event */
	UBOOL bAllowEnterKeyToApplyChanges;

	wxScrollBar* ScrollBar;
	INT ThumbPos;
	INT MaxH;

	FNotifyHook* NotifyHook;

	/** 
	* Command handler for the rebuild event.
	*
	* @param	Event	Information about the event.
	*/
	virtual void OnRebuild(wxCommandEvent &Event);

private:
	/**
	 * Performs necessary tasks (remember expanded items, etc.) performed before setting an object.
	 */
	void PreSetObject();
	
	/**
	 * Performs necessary tasks (restoring expanded items, etc.) performed after setting an object.
	 */
	void PostSetObject(UBOOL InExpandCategories, UBOOL InSorted, UBOOL InShowCategories,UBOOL InShowNonEditable);

	/**
	 * Links up the Next/Prev pointers the children.  Needed for things like keyboard navigation.
	 */
	void LinkChildren();

	/**
	 * Recursive minion of LinkChildren.
	 */
	WxPropertyWindow_Base* LinkChildrenForItem( WxPropertyWindow_Base* InItem );

	/**
	 * Recursive minion of PositionChildren.
	 *
	 * @param	InX		The horizontal position of the child item.
	 * @param	InY		The vertical position of the child item.
	 * @return			An offset to the current Y for the next position.
	 */
	INT PositionChild( WxPropertyWindow_Base* InItem, INT InX, INT InY );

	/** The first item window of this property window. */
	WxPropertyWindow_Objects* Root;

	friend class WxPropertyWindowFrame;
	friend class WxPropertyWindow_Objects;
	friend class WxPropertyWindow_Category;
};


///////////////////////////////////////////////////////////////////////////////
//
// Property window frame.
//
///////////////////////////////////////////////////////////////////////////////

/**
 * Property windows which are meant to be stand-alone windows must sit inside
 * of a WxPropertyWindowFrame.  Floating property windows can contain an optional
 * toolbar, and can be locked to paticular properties.  WxPropertyWindowFrame
 * also manages whether or not a property window can be closed.
 */
class WxPropertyWindowFrame : public wxFrame, public FDeferredInitializationWindow
{
public:
	DECLARE_DYNAMIC_CLASS(WxPropertyWindowFrame);

	virtual ~WxPropertyWindowFrame();

	/**
	 *	Initialize this property window.  Must be the first function called after creation.
	 *
	 * @param	parent			The parent window.
	 * @param	id				The ID for this control.
	 * @param	bShowToolBar	If TRUE, create a toolbar along the top.
	 * @param	InNotifyHook	An optional callback that receives property PreChange and PostChange notifies.
	 */
	virtual void Create( wxWindow* parent, wxWindowID id, UBOOL bShowToolBar, FNotifyHook* InNotifyHook = NULL );

	const WxPropertyWindow* GetPropertyWindow() const { return PropertyWindow; }
	WxPropertyWindow* GetPropertyWindow() { return PropertyWindow; }

	/**
	 * Responds to close events.  Rejects the event if closing is disallowed.
	 */
	void OnClose( wxCloseEvent& In );

	/**
	 * Responds to size events.  Resizes the handled property window.
	 */
	void OnSize( wxSizeEvent& In );

	void OnCopy( wxCommandEvent& In );
	void OnCopyComplete( wxCommandEvent& In );

	/**
	 * Expands all categories for the selected objects.
	 */
	void OnExpandAll( wxCommandEvent& In );

	/**
	 * Collapses all categories for the selected objects.
	 */
	void OnCollapseAll( wxCommandEvent& In );

	/**
	 * Locks or unlocks the property window, based on the 'checked' status of the event.
	 */
	void OnLock( wxCommandEvent& In );

	/**
	 * Updates the UI with the locked status of the contained property window.
	 */
	void UI_Lock( wxUpdateUIEvent& In );

	/**
	 * Returns TRUE if this property window is locked, or FALSE if it is unlocked.
	 */
	UBOOL IsLocked() const			{ return bLocked; }

	UBOOL IsCloseAllowed() const	{ return bAllowClose; }
	void AllowClose()				{ bAllowClose = TRUE; }
	void DisallowClose()			{ bAllowClose = FALSE; }

	/**
	 * Updates the caption of the floating frame based on the objects being edited.
	 */
	void UpdateTitle();

	/**
	 * Clears out the property window's last item of focus.
	 */
	void ClearLastFocused()
	{
		PropertyWindow->ClearLastFocused();
	}

	/**
	 * Rebuild all the properties and categories, with the same actors 
	 *
	 * @param IfContainingObject Only rebuild this property window if it contains the given object in the object hierarchy
	 */
	void Rebuild(UObject* IfContainingObject=NULL)
	{
		PropertyWindow->Rebuild(IfContainingObject);
	}

	//////////////////////////////////////////////////////////////////////////
	// Attaching objects.  Simply passes down to the property window.

	/**
	 * Sets an object in the property window.  T must be derived from UObject; will not
	 * compile if this is not the case.
	 *
	 * @param	InObject				The object to bind to the property window.  Can be NULL.
	 * @param	InExpandCategories		If TRUE, expand property categories.
	 * @param	InSorted				If TRUE, properties in this window are sorted alphabetically.
	 * @param	InShowCategories		If TRUE, show categories in this window.
	 * @param	InShowNonEditable		If TRUE, show properties which aren't editable in the None group.
	 */
	void SetObject( UObject* InObject,
					UBOOL InExpandCategories,
					UBOOL InSorted,
					UBOOL InShowCategories,
					UBOOL InShowNonEditable = 0)
	{
		PropertyWindow->SetObject( InObject, InExpandCategories, InSorted, InShowCategories, InShowNonEditable );
	}

	/**
	 * Sets multiple objects in the property window.  T must be derived from UObject; will not
	 * compile if this is not the case.
	 *
	 * @param	InObjects				An array of objects to bind to the property window.
	 * @param	InExpandCategories		If TRUE, expand property categories.
	 * @param	InSored					If TRUE, properties in this window are sorted alphabetically.
	 * @param	InShowCategories		If TRUE, show categories in this window.
	 */
	template< class T >
	void SetObjectArray(const TArray<T*>& InObjects,
						UBOOL InExpandCategories,
						UBOOL InSorted,
						UBOOL InShowCategories )
	{
		PropertyWindow->SetObjectArray( InObjects, InExpandCategories, InSorted, InShowCategories );
	}

	// @todo DB: remove this method which is just a hack for UUnrealEdEngine::UpdatePropertyWindows.
	void RemoveActor(AActor* Actor);

	//////////////////////////////////////////////////////////////////////////
	// Object iteration.  Simply passes down to the property window.

	typedef WxPropertyWindow::TObjectIterator		TObjectIterator;
	typedef WxPropertyWindow::TObjectConstIterator	TObjectConstIterator;

	TObjectIterator			ObjectIterator()			{ return PropertyWindow->ObjectIterator(); }
	TObjectConstIterator	ObjectConstIterator() const	{ return PropertyWindow->ObjectConstIterator(); }

	DECLARE_EVENT_TABLE();

protected:

	/** The property window embedded inside this floating frame. */
	WxPropertyWindow* PropertyWindow;

	/** If TRUE, this property frame will destroy itself if the user clicks the "X" button.  Otherwise, it just hides. */
	UBOOL bAllowClose;

	/** If TRUE, this property window is locked and won't react to selection changes. */
	UBOOL bLocked;

	/** The optional toolbar at the top of this frame.  Allocated in the ctor if the bShowToolBar ctor arg is TRUE.*/
	WxPropertyWindowToolBarBase* ToolBar;
};

#endif // __PROPERTIES_H__
