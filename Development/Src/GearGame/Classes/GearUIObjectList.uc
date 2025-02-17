/**
 * Copy of UT's UTUIOptionList - temporary implementation (for press preview - early Feb 2008); will be replaced by UIObjectList.
 *
 * Complete list of temporary classes which need to be re-implemented:
 * - GearUIObjectList
 * - GearUIDataProvider_MenuItem (and children)
 * - GearUIDataStore_MenuItems
 *
 * @todo when promoted to Engine
 * - override GetChildren/FindChild so that code using this class can't get a reference to the widgets in this list (bad because they will be removed
 *		and recreated everytime the list is regenerated)
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUIObjectList extends UIObject
	placeable
	native(UIPrivate)
	DontAutoCollapseCategories(Data)
	implements(UIDataStoreSubscriber,UITickableObject);


/* == Stuff from UTUIOptionList == */
/** Scrollbar to let PC users scroll up and down the list freely. */
var	transient UIScrollbar					VerticalScrollbar;

/** Info about an option we have generated. */
struct native GeneratedObjectInfo
{
	var name			OptionProviderName;
	var UIObject		LabelObj;
	var UIObject		OptionObj;
	var UIDataProvider	OptionProvider;
	var float 			OptionY;
	var float 			OptionHeight;
	var float 			OptionX;
	var float 			OptionWidth;
};

/** Current option index. */
var transient int CurrentIndex;

/** Previously selected option index. */
var transient int PreviousIndex;

/** Start time for animating option switches. */
var transient float StartMovementTime;

/** Whether or not we are currently animating the background prefab. */
var transient bool	bAnimatingBGPrefab;

/** List of auto-generated objects, anything in this array will be removed from the children's array before presave. */
var transient array<GeneratedObjectInfo>	GeneratedObjects;

/** The data store that this list is bound to */
var(Data)						UIDataStoreBinding		DataSource;

/** the list element provider referenced by DataSource */
var	const	transient			UIListElementProvider	DataProvider;

/** Background prefab for the currently selected item. */
var(Controls) editinlineuse		UIPrefab				BGPrefab;

/**
 * The height for each item in the list
 */
var(Appearance)					UIScreenValue_Extent	OptionHeight;

/**
 * The amount of padding to apply to the top and bottom of the option.
 */
var(Appearance)					UIScreenValue_Extent	OptionPadding;

/**
 * The size of the interactive option for each item.
 */
var(Appearance)					UIScreenValue_Extent	OptionSize;

/** the value entered here will be propagated to any collapsing selection lists in this object list. */
var(Appearance)					UIScreenValue_Extent	CollapsingListCaptionSize;

/**
 * The name of the style to use for the option's labels.
 *@todo ronp - we should really be using a managed style here rather than style names
 */
var(Style)						name					LabelCaptionStyleName;
var(Style)						name					LabelBackgroundStyleName;

/**
 * If TRUE, when the user presses up on the first option, focus will be given to the last option (and vice versa).  If FALSE,
 * focus follows the normal focus chain rules.
 */
var(Interaction)				bool					bClosedListNavigation;

/** this is the image component which renders a selection outline over the currently focused item */
var(Components)					UIComp_DrawImage		SelectionOverlayImage;

/** the position (in pixels) to render the overlay image in */
var	transient					float					OverlayImageBounds[EUIWidgetFace.UIFACE_MAX];

/** Instance of the background prefab. */
var transient					UIPrefabInstance		BGPrefabInstance;

/** Maximum number of visible items. */
var transient					int	 MaxVisibleItems;

/** indicates that the widgets in this list should be re-positioned during the next tick */
var(ZDebug)	const	private{private}	transient			bool bRepositionOptions;

/** Flag to let the optionlist know that it should regenerate its options on next tick. */
var(ZDebug) const	private{private}	transient			bool bRegenOptions;

/** The class to use when creating items that are configured as MENUOT_ComboSpinner */
var(Controls)	protected		class<UINumericEditBox>			SpinnerClass;
/** The class to use when creating items that are configured as MENUOT_Slider */
var(Controls)	protected		class<UISlider>					SliderClass;
/** The class to use when creating items that are configured as MENUOT_EditBox */
var(Controls)	protected		class<UIEditBox>				EditboxClass;
/** The class to use when creating items that are configured as MENUOT_CheckBox */
var(Controls)	protected		class<UICheckbox>				CheckboxClass;
/** The class to use when creating items that are configured as MENUOT_ComboNumeric or MENUOT_ComboReadOnly */
var(Controls)	protected		class<UIComboBox>				ComboBoxClass;
/** The class to use when creating items on consoles */
var(Controls)	protected		class<UIOptionList>				ConsoleOptionClass;

cpptext
{
protected:
	/* === UGearUIObjectList interface === */
	/** Generates widgets for all of the options. */
	virtual void RegenerateOptions();

	/** Repositions all of the visible options. */
	virtual void RepositionOptions();

public:
	/* === UUIObject interface === */
	/**
	 * Repositions all option widgets.
	 */
	virtual void ResolveFacePosition( EUIWidgetFace Face );

	/**
	 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
	 *
	 * This version adds the LabelBackground (if non-NULL) to the StyleSubscribers array.
	 */
	virtual void InitializeStyleSubscribers();

	/* === UUIScreenObject interface === */
	/**
	 * Perform all initialization for this widget. Called on all widgets when a scene is opened,
	 * once the scene has been completely initialized.
	 * For widgets added at runtime, called after the widget has been inserted into its parent's
	 * list of children.
	 *
	 * @param	inOwnerScene	the scene to add this widget to.
	 * @param	inOwner			the container widget that will contain this widget.  Will be NULL if the widget
	 *							is being added to the scene's list of children.
	 */
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner=NULL );

	/**
	 * Generates a array of UI Action keys that this widget supports.
	 *
	 * @param	out_KeyNames	Storage for the list of supported keynames.
	 */
	virtual void GetSupportedUIActionKeyNames(TArray<FName> &out_KeyNames );

	/**
	 * Called when a property is modified that could potentially affect the widget's position onscreen.
	 */
	virtual void RefreshPosition();

	/**
	 * Routes rendering calls to children of this screen object.
	 *
	 * @param	Canvas	the canvas to use for rendering
	 * @param	UIPostProcessGroup	Group determines current pp pass that is being rendered
	 */
	virtual void Render_Children( FCanvas* Canvas, EUIPostProcessGroup UIPostProcessGroup );

	/* === IUITickableObject interface === */
	/**
	 * Called each frame to allow the object to perform work.  This version updates the positioning of the background prefab.
	 *
	 * @param	PreviousFrameSeconds	amount of time (in seconds) between the start of this frame and the start of the previous frame.
	 */
	virtual void Tick( FLOAT PreviousFrameSeconds );

	/* === UObject interface === */
	/**
	 * Called when a property value has been changed in the editor.
	 */
	virtual void PostEditChange( FEditPropertyChain& PropertyThatChanged );

	/**
	 * Presave function. Gets called once before an object gets serialized for saving. This function is necessary
	 * for save time computation as Serialize gets called three times per object from within UObject::SavePackage.
	 *
	 * @warning: Objects created from within PreSave will NOT have PreSave called on them!!!
	 *
	 * This version removes all items from the Children array and flags the widget to regenerate them.
	 */
	virtual void PreSave();
}

/* == Delegates == */
/** Delegate called when an option gains focus. */
delegate OnOptionFocused(UIScreenObject InObject, UIDataProvider OptionProvider);

/** Delegate for when the user changes one of the options in this option list. */
delegate OnOptionChanged(UIScreenObject InObject, name OptionName, int PlayerIndex);

/**
 * Router for the UIList's OnValueSubmitted delegate - called when the user selects an item in a list option
 */
delegate OnListOptionSubmitted( UIList InListObject, name OptionName, int PlayerIndex);

/** Accept button was pressed on the option list. */
delegate OnAcceptOptions(UIScreenObject InObject, int PlayerIndex);

/** Called after the options are regenerated but before the scene is updated */
delegate OnRegeneratedOptions(GearUIObjectList ObjectList);

/* == Natives == */

/**
 * Marks the list of options to be repositioned during the next tick.
 */
native final function RequestOptionListReformat();

/**
 * Marks the list of options to be updated next tick.
 */
native final function RequestOptionListRegeneration();

/** Sets the currently selected option index. */
native function SetSelectedOptionIndex(int OptionIdx);

/** Initializes combobox widgets. */
native function InitializeComboboxWidgets();

/** Initializes the scrollbar widget for the option list. */
native function InitializeScrollbars();

/**
 * Wrapper for verifying that a widget is contained in this object list.
 */
native final function bool IsValidListOption( UIObject CheckObj ) const;

/**
 * Finds the position of an object using the name of the data provider it's bound to.
 *
 * @param	ProviderName	the data provider tag to search for
 *
 * @return	the index [into the GeneratedObjects array] for a widget that is bound to a data provider with the specified tag, or
 *			INDEX_NONE if there are none.
 */
native final function int FindObjectIndexByName( name ProviderName ) const;

/**
 * Find the position of an object using a reference to the object.
 *
 * @param	SearchObj	the object to search for
 *
 * @return	the index [into the GeneratedObjects array] for a widget that is in the GeneratedObjects array, or
 *			INDEX_NONE if no matching objects were found.
 */
native final function int FindObjectIndexByRef( UIObject SearchObj ) const;

/**
 * Get a reference to the option widget that is currently selected.
 *
 * @return	a reference to the child widget that is currently selected for the specified player, or NULL if this list doesn't have a valid
 *			selection.
 */
native final function UIObject GetSelectedOption() const;

/* == Events == */

/** Sets up the option bindings. */
event SetupOptionBindings()
{
	local UIObject Obj;
	local int ObjectIdx;
	local UIList ListChild;

	// Go through all of the generated object and set the OnValueChanged delegate.
	for ( ObjectIdx=0; ObjectIdx < GeneratedObjects.length; ObjectIdx++ )
	{
		Obj = GeneratedObjects[ObjectIdx].OptionObj;
		Obj.OnValueChanged = OnValueChanged;
		Obj.NotifyActiveStateChanged = OnOption_NotifyActiveStateChanged;
		Obj.NotifyPositionChanged = OnOption_PositionChanged;

		ListChild = UIList(Obj);
		if ( ListChild != None )
		{
			ListChild.OnSubmitSelection = ListSelectionSubmitted;
		}
	}

	// Setup scroll callbacks
	if ( VerticalScrollbar != None )
	{
		VerticalScrollbar.OnScrollActivity = ScrollVertical;
		VerticalScrollbar.OnClickedScrollZone = ClickedScrollZone;
	}

}

/* == UnrealScript == */
function SaveAllOptions()
{
	local int OptionIdx;
	local UIDataStorePublisher Publisher;
	local array<UIDataStore> Unused;

	for ( OptionIdx = 0; OptionIdx < GeneratedObjects.Length; OptionIdx++ )
	{
		Publisher = UIDataStorePublisher(GeneratedObjects[OptionIdx].OptionObj);
		if ( Publisher != None )
		{
			Publisher.SaveSubscriberValue(Unused);
		}
	}
}

/** Refreshes the value of all of the options by having them pull their options from the datastore again. */
function RefreshAllOptions()
{
	local int OptionIdx;

	for(OptionIdx=0; OptionIdx<GeneratedObjects.length; OptionIdx++)
	{
		UIDataStoreSubscriber(GeneratedObjects[OptionIdx].OptionObj).RefreshSubscriberValue();
	}
}

/**
 * Handler for the vertical scrollbar's OnClickedScrollZone delegate.  Scrolls the list by a full page (MaxVisibleItems).
 *
 * @param	Sender			the scrollbar that was clicked.
 * @param	PositionPerc	a value from 0.0 - 1.0, representing the location of the click within the region between the increment
 *							and decrement buttons.  Values closer to 0.0 means that the user clicked near the decrement button; values closer
 *							to 1.0 are nearer the increment button.
 * @param	PlayerIndex		Player that performed the action that issued the event.
 */
function ClickedScrollZone( UIScrollbar Sender, float PositionPerc, int PlayerIndex )
{
	local int MouseX, MouseY;
	local float MarkerPosition;
	local bool bDecrement;

	local int NewTopItem;

	if ( GetCursorPosition(MouseX, MouseY) )
	{
		// this is the position of the marker's minor side (left or top)
		MarkerPosition = Sender.GetMarkerButtonPosition();

		// determine whether the user clicked in the region above or below the marker button.
		bDecrement = (Sender.ScrollbarOrientation == UIORIENT_Vertical)
			? MouseY < MarkerPosition
			: MouseX < MarkerPosition;

		NewTopItem = bDecrement ? (CurrentIndex - 1) : (CurrentIndex + 1);
		SelectItem(NewTopItem);
	}
}

/** @return Returns the object info struct index given a provider namename. */
function int GetObjectInfoIndexFromName(name ProviderName)
{
	return FindObjectIndexByName(ProviderName);
}

/** @return Returns the object info struct given a sender object. */
function int GetObjectInfoIndexFromObject(UIObject Sender)
{
	return FindObjectIndexByRef(Sender);
}

/** Returns the currently selected option object */
function UIObject GetCurrentlySelectedOption()
{
	return GetSelectedOption();
}

/**
 * Enables / disables an item in the list.  If the item is the currently selected item, selects the next item in the list, if possible.
 *
 * @param	OptionIdx		the index for the option that should be updated
 * @param	bShouldEnable	TRUE to enable the item; FALSE to disable.
 *
 * @return	TRUE if the item's state was successfully changed; FALSE if it couldn't be changed or OptionIdx was invalid.
 */
function bool EnableItem( int PlayerIndex, UIObject ChosenObj, bool bShouldEnable=true )
{
	return EnableItemAtIndex(PlayerIndex, GetObjectInfoIndexFromObject(ChosenObj), bShouldEnable);
}
function bool EnableItemAtIndex( int PlayerIndex, int OptionIdx, bool bShouldEnable=true )
{
	local bool bResult, bStateChangeAllowed;
	local int idx;
	local UIObject ChosenObj;

	if ( OptionIdx >= 0 && OptionIdx < GeneratedObjects.Length )
	{
		ChosenObj = GeneratedObjects[OptionIdx].OptionObj;
		if ( ChosenObj != None )
		{
			bStateChangeAllowed = true;
			if ( !bShouldEnable )
			{
				if ( OptionIdx == CurrentIndex )
				{
					bStateChangeAllowed = false;
					for ( idx = (OptionIdx + 1) % GeneratedObjects.Length; idx != CurrentIndex; idx = (idx + 1) % GeneratedObjects.Length )
					{
						if ( SelectItem(idx, PlayerIndex, false) )
						{
							bStateChangeAllowed = true;
							break;
						}
					}
				}
				else if ( ChosenObj == GetFocusedControl(false, PlayerIndex) )
				{
					bStateChangeAllowed = ChosenObj.KillFocus(None, PlayerIndex);
				}
			}

			if ( bStateChangeAllowed )
			{
				bResult = ChosenObj.SetEnabled(bShouldEnable, PlayerIndex);
			}
		}
	}

	return bResult;
}

/** Selects the specified option item. */
function bool SelectItem(int OptionIdx, optional int PlayerIndex=GetBestPlayerIndex(), optional bool bClampValue=true )
{
	local bool bResult;

	if ( bClampValue )
	{
		OptionIdx = Clamp(OptionIdx, 0, GeneratedObjects.length - 1);
	}

	if ( OptionIdx >= 0 && OptionIdx < GeneratedObjects.length
	&&	GeneratedObjects[OptionIdx].OptionObj.IsEnabled(GetBestPlayerIndex()))
	{
		if ( IsFocused(PlayerIndex) )
		{
			bResult = GeneratedObjects[OptionIdx].OptionObj.SetFocus(none);
		}
		else
		{
			OverrideLastFocusedControl(PlayerIndex, GeneratedObjects[OptionIdx].OptionObj);
			bResult = true;
		}
	}

	return bResult;
}

/** Selects the next item in the list. */
function bool SelectNextItem(optional bool bWrap=false, optional int PlayerIndex=GetBestPlayerIndex())
{
	local int TargetIndex;

	TargetIndex = CurrentIndex+1;

	if(bWrap)
	{
		TargetIndex = TargetIndex%(GeneratedObjects.length);
	}

	return SelectItem(TargetIndex, PlayerIndex);
}

/** Selects the previous item in the list. */
function bool SelectPreviousItem(optional bool bWrap=false, optional int PlayerIndex=GetBestPlayerIndex())
{
	local int TargetIndex;

	TargetIndex = CurrentIndex-1;

	if(bWrap && TargetIndex<0)
	{
		TargetIndex=GeneratedObjects.length-1;
	}

	return SelectItem(TargetIndex, PlayerIndex);
}

/**
 * @Returns the mouse position in widget space
 */
function Vector GetMousePosition()
{
	local int x,y;
	local vector2D MousePos;
	local vector AdjustedMousePos;
	class'UIRoot'.static.GetCursorPosition( X, Y );
	MousePos.X = X;
	MousePos.Y = Y;
	AdjustedMousePos = PixelToCanvas(MousePos);
	AdjustedMousePos.X -= GetPosition(UIFACE_Left,EVALPOS_PixelViewport);
	AdjustedMousePos.Y -= GetPosition(UIFACE_Top, EVALPOS_PixelViewport);
	return AdjustedMousePos;
}

/**
 * All are in pixels
 *
 * @Param X1		Left
 * @Param Y1		Top
 * @Param X2		Right
 * @Param Y2		Bottom
 *
 * @Returns true if the mouse is within the bounds given
 */
function bool CursorCheck(float X1, float Y1, float X2, float Y2)
{
	local vector MousePos;;

	MousePos = GetMousePosition();

	return ( (MousePos.X >= X1 && MousePos.X <= X2) && (MousePos.Y >= Y1 && MousePos.Y <= Y2) );
}

/* == SequenceAction handlers == */

/* == Delegate handlers == */
/**
 * Called when a new UIState becomes the widget's currently active state, after all activation logic has occurred.
 *
 * @param	Sender					the widget that changed states.
 * @param	PlayerIndex				the index [into the GamePlayers array] for the player that activated this state.
 * @param	NewlyActiveState		the state that is now active
 * @param	PreviouslyActiveState	the state that used the be the widget's currently active state.
 */
final function OnStateChanged( UIScreenObject Sender, int PlayerIndex, UIState NewlyActiveState, optional UIState PreviouslyActiveState )
{
	local int ObjIndex;

	if ( Sender == Self )
	{
		if ( UIState_Disabled(NewlyActiveState) != None )
		{
			KillFocus(None);
			for ( ObjIndex = 0; ObjIndex < GeneratedObjects.Length; ObjIndex++ )
			{
				if ( GeneratedObjects[ObjIndex].OptionObj != None )
				{
					GeneratedObjects[ObjIndex].OptionObj.DisableWidget(PlayerIndex);
				}
			}

			if ( VerticalScrollbar != None )
			{
				VerticalScrollbar.DisableWidget(PlayerIndex);
			}
		}
		else if ( UIState_Disabled(PreviouslyActiveState) != None && IsEnabled(PlayerIndex) )
		{
			for ( ObjIndex = 0; ObjIndex < GeneratedObjects.Length; ObjIndex++ )
			{
				if ( GeneratedObjects[ObjIndex].OptionObj != None )
				{
					GeneratedObjects[ObjIndex].OptionObj.EnableWidget(PlayerIndex);
				}
			}

			if ( VerticalScrollbar != None )
			{
				VerticalScrollbar.EnableWidget(PlayerIndex);
			}
		}
	}
}

/**
 * Handler for vertical scrolling activity
 * PositionChange should be a number of nudge values by which the slider was moved
 *
 * @param	Sender			the scrollbar that generated the event.
 * @param	PositionChange	indicates how many items to scroll the list by
 * @param	bPositionMaxed	indicates that the scrollbar's marker has reached its farthest available position,
 *                          unused in this function
 */
function bool ScrollVertical( UIScrollbar Sender, float PositionChange, optional bool bPositionMaxed=false )
{
	local int NewIndex;

	if ( bPositionMaxed )
	{
		NewIndex = Clamp(PositionChange > 0 ? GeneratedObjects.Length - 1 : 0, 0, GeneratedObjects.Length - 1);
	}
	else
	{
		NewIndex = CurrentIndex + Round(PositionChange);
	}

	SelectItem(NewIndex);
	return true;
}

/** Callback for all of the options we generated. */
function OnValueChanged( UIObject Sender, int PlayerIndex )
{
	local name OptionProviderName;
	local int ObjectIdx;

	OptionProviderName = '';

	// Reoslve the option name
	ObjectIdx = GetObjectInfoIndexFromObject(Sender);

	if(ObjectIdx != INDEX_NONE)
	{
		OptionProviderName = GeneratedObjects[ObjectIdx].OptionProviderName;
	}

	// Call the option changed delegate
	OnOptionChanged(Sender, OptionProviderName, PlayerIndex);
}

/**
 * Called when the user presses Enter (or any other action bound to UIKey_SubmitListSelection) while this list has focus.
 *
 * @param	Sender	the list that is submitting the selection
 */
function ListSelectionSubmitted( UIList Sender, optional int PlayerIndex=GetBestPlayerIndex() )
{
	local name OptionProviderName;
	local int ObjectIdx;

	// Reoslve the option name
	ObjectIdx = GetObjectInfoIndexFromObject(Sender);
	if ( ObjectIdx != INDEX_NONE )
	{
		OptionProviderName = GeneratedObjects[ObjectIdx].OptionProviderName;
	}

	OnListOptionSubmitted(Sender, OptionProviderName, PlayerIndex);
}

/** Callback for when the object's active state changes. */
function OnOption_NotifyActiveStateChanged( UIScreenObject Sender, int PlayerIndex, UIState NewlyActiveState, optional UIState PreviouslyActiveState )
{
	local int ObjectIndex;

	ObjectIndex = GetObjectInfoIndexFromObject(UIObject(Sender));

	if(ObjectIndex != INDEX_NONE)
	{
		if ( NewlyActiveState.Class == class'UIState_Focused'.default.Class )
		{
			SetSelectedOptionIndex(ObjectIndex);
			OnOptionFocused(Sender, GeneratedObjects[ObjectIndex].OptionProvider);
		}
	}
}

/**
 * Handler for the NotifyPositionChanged delegate of the list options.  Request the object list to reposition all options.
 */
function OnOption_PositionChanged( UIScreenObject Sender )
{
	local UIObject SenderObj;

	SenderObj = UIObject(Sender);
	if ( IsValidListOption(SenderObj) )
	{
		RequestOptionListReformat();
	}
}

/**
 * Provides a hook for unrealscript to respond to input using actual input key names (i.e. Left, Tab, etc.)
 *
 * Called when an input key event is received which this widget responds to and is in the correct state to process.  The
 * keys and states widgets receive input for is managed through the UI editor's key binding dialog (F8).
 *
 * This delegate is called BEFORE kismet is given a chance to process the input.
 *
 * @param	EventParms	information about the input event.
 *
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */

function bool ProcessInputKey( const out SubscribedInputEventParameters EventParms )
{
	local bool bResult;
	local int OptionIdx;

	if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_Repeat )
	{
		if ( EventParms.InputAliasName == 'SelectionUp' )
		{
			SelectPreviousItem(true);
			bResult = true;
		}
		else if ( EventParms.InputAliasName == 'SelectionDown' )
		{
			SelectNextItem(true);
			bResult = true;
		}
		else if ( EventParms.InputAliasName == 'SelectionHome' )
		{
			SelectItem(0);
			bResult = true;
		}
		else if ( EventParms.InputAliasName == 'SelectionEnd' )
		{
			SelectItem(GeneratedObjects.Length-1);
			bResult = true;
		}
		else if ( EventParms.InputAliasName == 'SelectionPgUp' )
		{
			SelectItem(CurrentIndex - MaxVisibleItems);
			bResult = true;
		}
		else if ( EventParms.InputAliasName == 'SelectionPgDn' )
		{
			SelectItem(CurrentIndex + MaxVisibleItems);
			bResult = true;
		}
		else if ( EventParms.InputAliasName == 'Click' )
		{
			bResult = true;
		}
	}
	else if ( EventParms.EventType == IE_Released )
	{
		if ( EventParms.InputAliasName == 'AcceptOptions' )
		{
			PlayUISound('ListSubmit');
			OnAcceptOptions(self, EventParms.PlayerIndex);

			bResult = true;
		}
		else if ( EventParms.InputAliasName == 'Click' )
		{
			for(OptionIdx=0; OptionIdx<GeneratedObjects.length; OptionIdx++)
			{
				if (CursorCheck(GeneratedObjects[OptionIdx].OptionX,
					GeneratedObjects[OptionIdx].OptionY,
					GeneratedObjects[OptionIdx].OptionX+GeneratedObjects[OptionIdx].OptionWidth,
					GeneratedObjects[OptionIdx].OptionY+GeneratedObjects[OptionIdx].OptionHeight))
				{
					if ( CurrentIndex != OptionIdx )
					{
						SelectItem(OptionIdx);
					}

					break;
				}
			}

			bResult = true;
		}
	}

	return bResult;
}

/** === UIDataSourceSubscriber interface === */
/**
 * Sets the data store binding for this object to the text specified.
 *
 * @param	MarkupText			a markup string which resolves to data exposed by a data store.  The expected format is:
 *								<DataStoreTag:DataFieldTag>
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 */
native final virtual function SetDataStoreBinding( string MarkupText, optional int BindingIndex=INDEX_NONE );

/**
 * Retrieves the markup string corresponding to the data store that this object is bound to.
 *
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 *
 * @return	a datastore markup string which resolves to the datastore field that this object is bound to, in the format:
 *			<DataStoreTag:DataFieldTag>
 */
native final virtual function string GetDataStoreBinding( optional int BindingIndex=INDEX_NONE ) const;

/**
 * Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
 *
 * @return	TRUE if this subscriber successfully resolved and applied the updated value.
 */
native final virtual function bool RefreshSubscriberValue( optional int BindingIndex=INDEX_NONE );

/**
 * Handler for the UIDataStore.OnDataStoreValueUpdated delegate.  Used by data stores to indicate that some data provided by the data
 * has changed.  Subscribers should use this function to refresh any data store values being displayed with the updated value.
 * notify subscribers when they should refresh their values from this data store.
 *
 * @param	SourceDataStore		the data store that generated the refresh notification; useful for subscribers with multiple data store
 *								bindings, to tell which data store sent the notification.
 * @param	PropertyTag			the tag associated with the data field that was updated; Subscribers can use this tag to determine whether
 *								there is any need to refresh their data values.
 * @param	SourceProvider		for data stores which contain nested providers, the provider that contains the data which changed.
 * @param	ArrayIndex			for collection fields, indicates which element was changed.  value of INDEX_NONE indicates not an array
 *								or that the entire array was updated.
 */
native function NotifyDataStoreValueUpdated( UIDataStore SourceDataStore, bool bValuesInvalidated, name PropertyTag, UIDataProvider SourceProvider, int ArrayIndex );

/**
 * Retrieves the list of data stores bound by this subscriber.
 *
 * @param	out_BoundDataStores		receives the array of data stores that subscriber is bound to.
 */
native final virtual function GetBoundDataStores( out array<UIDataStore> out_BoundDataStores );

/**
 * Notifies this subscriber to unbind itself from all bound data stores
 */
native final virtual function ClearBoundDataStores();

/* == UIScreenObject interface == */
/** Post initialize, binds callbacks for all of the generated options. */
event PostInitialize()
{
	Super.PostInitialize();

	// Setup handler for input keys
	OnProcessInputAxis = None;
}

event GetSupportedUIActionKeyNames(out array<Name> out_KeyNames )
{
	out_KeyNames[out_KeyNames.Length] = 'MouseMoveX';
	out_KeyNames[out_KeyNames.Length] = 'MouseMoveY';
	out_KeyNames[out_KeyNames.Length] = 'Click';
	out_KeyNames[out_KeyNames.Length] = 'SelectionUp';
	out_KeyNames[out_KeyNames.Length] = 'SelectionDown';
	out_KeyNames[out_KeyNames.Length] = 'SelectionHome';
	out_KeyNames[out_KeyNames.Length] = 'SelectionEnd';
	out_KeyNames[out_KeyNames.Length] = 'SelectionPgUp';
	out_KeyNames[out_KeyNames.Length] = 'SelectionPgDn';
	out_KeyNames[out_KeyNames.Length] = 'AcceptOptions';
}

/**
 * Notification that this widget's parent is about to remove this widget from its children array.  Allows the widget
 * to clean up any references to the old parent.
 *
 * @param	WidgetOwner		the screen object that this widget was removed from.
 */
event RemovedFromParent( UIScreenObject WidgetOwner )
{
	local UIScene SceneOwner;

	Super.RemovedFromParent(WidgetOwner);

	SceneOwner = GetScene();
	if ( SceneOwner != None )
	{
		SceneOwner.UnregisterTickableObject(Self);
	}
}


defaultproperties
{
	bRepositionOptions=true
	bRegenOptions=true
	NotifyActiveStateChanged=OnStateChanged
	OnProcessInputKey=ProcessInputKey

	DefaultStates.Add(class'Engine.UIState_Focused')
	DefaultStates.Add(class'Engine.UIState_Active')

	DataSource=(RequiredFieldType=DATATYPE_Collection)
//	BGPrefab=UIPrefab'UI_Scenes_FrontEnd.Prefabs.OptionBG'


//	use this value for default when promoted to engine
	OptionSize=(Value=0.6f,ScaleType=UIEXTENTEVAL_PercentSelf,Orientation=UIORIENT_Horizontal)
	OptionHeight=(Value=0.202022,ScaleType=UIEXTENTEVAL_PercentSelf,Orientation=UIORIENT_Vertical)
	OptionPadding=(Value=0.0191326,ScaleType=UIEXTENTEVAL_PercentSelf,Orientation=UIORIENT_Vertical)
	CollapsingListCaptionSize=(Value=0.45,ScaleType=UIEXTENTEVAL_PercentSelf,Orientation=UIORIENT_Horizontal)

	LabelCaptionStyleName=cmb_ButtonText_Indent
	LabelBackgroundStyleName=ButtonMainSelection

	Begin Object Class=UIComp_DrawImage Name=SelectionHighlightTemplate
		ImageStyle=(DefaultStyleTag="ButtonMainSelection",RequiredStyleClass=class'Engine.UIStyle_Image')
		StyleResolverTag="Selection Highlight"
	End Object
	SelectionOverlayImage=SelectionHighlightTemplate

	SliderClass=class'UISlider'
	EditboxClass=class'UIEditBox'
	SpinnerClass=class'UINumericEditBox'
	CheckboxClass=class'UICheckbox'
	ComboBoxClass=class'UIComboBox'
//	use this value for default when promoted to engine
//	ConsoleOptionClass=class'UIOptionList'
	ConsoleOptionClass=class'GearOptionList'
}

