`include(Engine/Classes/UIDev.uci)

/**
 * Specialized list class for representing a single game setting along with its avaiable values.  The list has two states:
 * collapsed and expanded.  In the collapsed state, the list displays only the caption and current value for its associated
 * game setting.  In the expanded state, the list displays the game setting's available values as a single-column list
 * below the caption/current value.  While the list is in the expanded state, all navigation operations are suspended and
 * the user may only choose an option or return to the collapsed state.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUICollapsingSelectionList extends UIList
	native(UIPrivate)
	placeable;

const COLLAPSESELECTION_LIST_DATABINDING_INDEX		=0;
const COLLAPSESELECTION_CAPTION_DATABINDING_INDEX	=1;

/**
 * Renders the game setting caption text.
 */
var(Components)	editinline		UIComp_DrawString		CaptionComponent;

/**
 * Renders the current value of the game setting.
 */
var(Components)	editinline		UIComp_DrawString		ValueComponent;

/** this is the image component which renders a down arrow on the right side of the caption area */
var(Components)					UIComp_DrawImage		FocusIndicator;

/** Data source for the caption text */
var(Data)	private				UIDataStoreBinding		CaptionDataSource;

/**
 * Determines the width of the caption.  Disable by setting its value to 0 (to allow the component to autosize itself, for example)
 */
var(Appearance)					UIScreenValue_Extent	CaptionSize;

/**
 * Determines the width of the value string.  Disable by setting its value to 0.
 */
var(Appearance)					UIScreenValue_Extent	ValueSize;

/**
 * Determines how far in from the left edge the list portion will be rendered.
 */
var(Appearance)					UIScreenValue_Extent	ListIndent;

/**
 * Controls the size of the focus indicator.
 */
var(Appearance)					UIScreenValue_Extent	FocusIndicatorSize;

/**
 * Controls whether the value label changes to when the list's index is changed.  FALSE indicates that the value label should only be
 * changed when the user selects an option.
 */
var(Appearance)					bool					bIndexChangeUpdatesValue;

/**
 * Used to control when the selection hint is repositioned during a transition animation.
 */
var		transient				bool					bFadingSelectionHintOut;

/**
 * A padding value to apply to the background image.
 */
var(Appearance)					UIScreenValue_Extent	VertBackgroundPadding[2];
var(Appearance)					UIScreenValue_Extent	HorzBackgroundPadding[2];

/**
 * The different expansion modes the list can exist in.
 */
enum EListExpansionState
{
	/** list of fully collapsed and only displaying the current value */
	LES_Collapsed,

	/** list is fully expanded and displaying all available values */
	LES_Expanded,

	/** list is in the process of collapsing */
	LES_Collapsing,

	/** list is in the process of expanding */
	LES_Expanding,
};

/**
 * Current expansion state of the list.  Updated when the list changes modes.
 */
var(Interaction)	/*editconst*/	transient		EListExpansionState		ListExpansionState;

/**
 * The animation sequence template for this list's collapse animation.
 */
var(Animation)		editinline	transient	const	UIAnimationSeq			CollapseAnimation;
/**
 * The animation sequence template for this list's expand animation.
 */
var(Animation)		editinline	transient	const	UIAnimationSeq			ExpandAnimation;

var(Animation)		editinline	transient	const	UIAnimationSeq			ExpandFocusHintAnimation;
var(Animation)		editinline	transient	const	UIAnimationSeq			CollapseFocusHintAnimation;

/**
 * Tracks the value of Index the last time NotifySubmitSelection was called.  Used to ensure that the NotifySubmitSelection
 * is called if the list loses focus without the user actually clicking on the selected item.
 */
var		transient			int						SubmittedIndex;

// ===============================================
// Sounds
// ===============================================
/** this sound is played when the list is expanded */
var(Sound)				name						ExpandCue;

/** the sound is played when the list is collapsed, except when collapse is due to loss of focus */
var(Sound)				name						CollapseAcceptCue;

/** the sound is played when the list is collapsed due to loss of focus or cancellation*/
var(Sound)				name						CollapseCancelCue;

cpptext
{
	/* === UGearUICollapsingSelectionList interface === */
	/** Wrappers for checking the state of the list */
	UBOOL IsExpanded() const
	{
		return ListExpansionState == LES_Expanded;
	}
	UBOOL IsExpanding() const
	{
		return ListExpansionState == LES_Expanding;
	}
	UBOOL IsCollapsed() const
	{
		return ListExpansionState == LES_Collapsed;
	}
	UBOOL IsCollapsing() const
	{
		return ListExpansionState == LES_Collapsing;
	}
	UBOOL IsTransitioning() const
	{
		return (IsCollapsing() || IsExpanding());
	}

	/* === UUIList interface === */
	/**
	 * Sets the list's index to the value specified and activates the appropriate notification events.
	 *
	 * @param	NewIndex			An index into the Items array that should become the new Index for the list.
	 * @param	bClampValue			if TRUE, NewIndex will be clamped to a valid value in the range of 0 -> ItemCount - 1
	 * @param	bSkipNotification	if TRUE, no events are generated as a result of updating the list's index.
	 *
	 * @return	TRUE if the list's Index was successfully changed.
	 */
	virtual UBOOL SetIndex( INT NewIndex, UBOOL bClampValue=TRUE, UBOOL bSkipNotification=FALSE );

	/**
	 * Called when the list's index has changed.
	 *
	 * @param	PreviousIndex	the list's Index before it was changed
	 * @param	PlayerIndex		the index of the player associated with this index change.
	 */
	virtual void NotifyIndexChanged( INT PreviousIndex, INT PlayerIndex );

	/**
	 * Called whenever the user chooses an item while this list is focused.  Activates the SubmitSelection kismet event and calls
	 * the OnSubmitSelection delegate.
	 */
	virtual void NotifySubmitSelection( INT PlayerIndex );

	/* === UUIObject interface === */
	/**
	 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
	 *
	 * This version adds the LabelBackground (if non-NULL) to the StyleSubscribers array.
	 */
	virtual void InitializeStyleSubscribers();

	/**
	 * Wrapper for calculating the amount of additional room the list needs at the top to render headers or other things.
	 */
	virtual FLOAT GetHeaderSize() const;

	/**
	 * Render this list.
	 *
	 * @param	Canvas	the canvas to use for rendering this widget
	 */
	virtual void Render_Widget( FCanvas* Canvas );

	/**
	 * Renders the list's background image, if assigned.
	 */
	virtual void RenderBackgroundImage( FCanvas* Canvas, const FRenderParameters& Parameters );

	/**
	 * Called immediately before and after the scene perform an update.  Only called if bEnableSceneUpdateNotifications
	 * is set to TRUE on this widget.
	 */
	virtual void PreSceneUpdate();
	virtual void PostSceneUpdate();

	/**
	 * Evalutes the Position value for the specified face into an actual pixel value.  Should only be
	 * called from UIScene::ResolvePositions.  Any special-case positioning should be done in this function.
	 *
	 * @param	Face	the face that should be resolved
	 */
	virtual void ResolveFacePosition( EUIWidgetFace Face );

	/** === IUIDataStorePublisher interface === */
	/**
	 * Sets the data store binding for this object to the text specified.
	 *
	 * @param	MarkupText			a markup string which resolves to data exposed by a data store.  The expected format is:
	 *								<DataStoreTag:DataFieldTag>
	 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
	 *								objects which have multiple data store bindings.  How this parameter is used is up to the
	 *								class which implements this interface, but typically the "primary" data store will be index 0.
	 */
	virtual void SetDataStoreBinding(const FString& MarkupText,INT BindingIndex=INDEX_NONE);

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
	virtual FString GetDataStoreBinding(INT BindingIndex=INDEX_NONE) const;

	/**
	 * Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
	 *
	 * @return	TRUE if this subscriber successfully resolved and applied the updated value.
	 */
	virtual UBOOL RefreshSubscriberValue(INT BindingIndex=INDEX_NONE);

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
	virtual void NotifyDataStoreValueUpdated(class UUIDataStore* SourceDataStore,UBOOL bValuesInvalidated,FName PropertyTag,class UUIDataProvider* SourceProvider,INT ArrayIndex);

	/**
	 * Retrieves the list of data stores bound by this subscriber.
	 *
	 * @param	out_BoundDataStores		receives the array of data stores that subscriber is bound to.
	 */
	virtual void GetBoundDataStores(TArray<class UUIDataStore*>& out_BoundDataStores);

	/* === UUIScreenObject interface === */
	/**
	 * Perform all initialization for this widget. Called on all widgets when a scene is opened,
	 * once the scene has been completely initialized.
	 * For widgets added at runtime, called after the widget has been inserted into its parent's
	 * list of children.
	 *
	 * Initializes the string components.
	 *
	 * @param	inOwnerScene	the scene to add this widget to.
	 * @param	inOwner			the container widget that will contain this widget.  Will be NULL if the widget
	 *							is being added to the scene's list of children.
	 */
	virtual void Initialize( class UUIScene* inOwnerScene, class UUIObject* inOwner=NULL );

	/**
	 * Called when a property is modified that could potentially affect the widget's position onscreen.
	 */
	virtual void RefreshPosition();

	/**
	 * Called to globally update the formatting of all UIStrings.
	 */
	virtual void RefreshFormatting( UBOOL bRequestSceneUpdate=TRUE );

	/**
	 * Marks the Position for any faces dependent on the specified face, in this widget or its children,
	 * as out of sync with the corresponding RenderBounds.
	 *
	 * @param	Face	the face to modify; value must be one of the EUIWidgetFace values.
	 */
	virtual void InvalidatePositionDependencies( BYTE Face );

	/**
	 * Retrieves the current value for some data currently being interpolated by this widget.
	 *
	 * @param	AnimationType		the type of animation data to retrieve
	 * @param	out_CurrentValue	receives the current data value; animation type determines which of the fields holds the actual data value.
	 *
	 * @return	TRUE if the widget supports the animation type specified.
	 */
	virtual UBOOL Anim_GetValue( BYTE AnimationType, FUIAnimationRawData& out_CurrentValue ) const;
	/**
	 * Updates the current value for some data currently being interpolated by this widget.
	 *
	 * @param	AnimationType		the type of animation data to set
	 * @param	out_CurrentValue	contains the updated data value; animation type determines which of the fields holds the actual data value.
	 *
	 * @return	TRUE if the widget supports the animation type specified.
	 */
	virtual UBOOL Anim_SetValue( BYTE AnimationType, const FUIAnimationRawData& NewValue );

protected:
	/**
	 * Deactivates the UIState_Focused menu state and updates the pertinent members of FocusControls.
	 *
	 * @param	FocusedChild	the child of this widget that is currently "focused" control for this widget.
	 *							A value of NULL indicates that there is no focused child.
	 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated the focus change.
	 */
	virtual UBOOL LoseFocus( UUIObject* FocusedChild, INT PlayerIndex );

	/**
	 * Handles input events for this list.
	 *
	 * @param	EventParms		the parameters for the input event
	 *
	 * @return	TRUE to consume the key event, FALSE to pass it on.
	 */
	virtual UBOOL ProcessInputKey( const FSubscribedInputEventParameters& EventParms );

public:
	/* === UObject interface === */
	/**
	 * Called when a property value from a member struct or array has been changed in the editor, but before the value has actually been modified.
	 */
	virtual void PreEditChange( FEditPropertyChain& PropertyThatChanged );

	/**
	 * Called when a property value from a member struct or array has been changed in the editor.
	 */
	virtual void PostEditChange( FEditPropertyChain& PropertyThatChanged );
}

/**
 * Called when the list is about to begin collapsing.
 *
 * @param	Sender	the list that is about the begin collapsing
 *
 * @return	return FALSE to prevent the list from collapsing.
 */
delegate bool IsCollapseAllowed( GearUICollapsingSelectionList Sender );

/**
 * Called when the list is about to begin expanding.
 *
 * @param	Sender	the list that is about the begin expanding
 *
 * @return	return FALSE to prevent the list from expanding.
 */
delegate bool IsExpandAllowed( GearUICollapsingSelectionList Sender );

/**
 * Notification that the list has finished collapsing.
 *
 * @param	Sender	the list that is about the begin collapsing
 */
delegate OnListCollapsed( GearUICollapsingSelectionList Sender );

/**
 * Notification that the list has finished expanding.
 *
 * @param	Sender	the list that is about the begin expanding
 */
delegate OnListExpanded( GearUICollapsingSelectionList Sender );

/**
 * Called just before the list sets SubmittedIndex to Index and collapses.
 *
 * @param	Sender	the list that is submitting the value.
 *
 * @return	return FALSE to prevent the list from submitting its value.
 */
delegate bool CanSubmitValue( GearUICollapsingSelectionList Sender, int PlayerIndex );

/* == Delegates == */

/* == Natives == */
/**
 * Returns the height (in pixels) of the caption or value string (whichever is larger).
 */
native final function float GetMaxLabelHeight() const;

/**
 * Returns the height (in pixels) of the caption string.
 */
native final function float GetCaptionLabelHeight() const;

/**
 * Returns the height (in pixels) of the value string.
 */
native final function float GetValueLabelHeight() const;

/**
 * Expands the list to display the available choices for the bound game setting.
 *
 * @return	TRUE if the list was able to begin expanding.  FALSE if the list is not in the collapsed state or wasn't allowed to expand.
 */
native final function bool Expand();

/**
 * Returns the list to the collapsed state so that it only displays the setting caption and current value.
 *
 * @return	TRUE if the list was able to begin collapsing.  FALSE if the list is not in the expanded state or wasn't allowed to collapse.
 */
native final function bool Collapse();

/**
 * Called by the native code that starts collapsing the list.  Allows script-only child classes to do additional work here.
 */
protected native event ListCollapseBegin();

/**
 * Called by the native code when the list has finished collapsing.
 */
protected native event ListCollapseComplete();

/**
 * Called by the native code that starts expanding the list.  Allows script-only child classes to do additional work here.
 */
protected native event ListExpansionBegin();

/**
 * Called by the native code when the list has finished expanding.
 */
protected native event ListExpansionComplete();

/**
 * Sets the currently selected item according to the string associated with it.
 *
 * @param	StringToSelect	the string associated with the item that should become the selected item
 *
 * @return	TRUE if the value was changed successfully; FALSE if it wasn't found or was already selected.
 */
native final function bool SetStringValue( string StringToSelect );

/**
 * Sets the list's index to the value specified and activates the appropriate notification events.
 *
 * @param	NewIndex			An index into the Items array that should become the new Index for the list.
 * @param	PlayerIndex			PlayerIndex of the player responsible for setting the index
 * @param	bClampValue			if TRUE, NewIndex will be clamped to a valid value in the range of 0 -> ItemCount - 1
 * @param	bSkipNotification	if TRUE, no events are generated as a result of updating the list's index.
 *
 * @return	TRUE if the list's Index was successfully changed.
 */
native final virtual function bool SetIndexValue( int NewIndex, int PlayerIndex, optional bool bClampValue=true, optional bool bSkipNotification=false );

/**
 * Changes the data binding for the specified cell index.
 *
 * @param	CellDataBinding		a name corresponding to a tag from the UIListElementProvider currently bound to this list.
 * @param	ColumnHeader		the string that should be displayed in the column header for this cell.
 * @param	BindingIndex		the column or row to bind this data field to.  If BindingIndex is greater than the number
 *								schema cells, empty schema cells will be added to meet the number required to place the data
 *								at BindingIndex.
 *								If a value of INDEX_NONE is specified, the cell binding will only occur if there are no other
 *								schema cells bound to that data field.  In this case, a new schema cell will be appended and
 *								it will be bound to the data field specified.
 */
native final noexportheader function bool SetCellBinding( name CellDataBinding, out const string ColumnHeader, int BindingIndex );

/* == Events == */
/**
 * Callback for starting the list's collapse animation.
 */
protected event OnListCollapsing()
{
	local UIScene OwningScene;

	if ( CollapseAnimation != None )
	{
		if ( ExpandAnimation != None && ExpandAnimation.SeqName != '' && IsAnimating(ExpandAnimation.SeqName) )
		{
			StopUIAnimation(ExpandAnimation.SeqName, ExpandAnimation, true);
		}

		PlayUIAnimation('', CollapseAnimation,,,,false);
		PlayFocusHintAnimation(CollapseFocusHintAnimation);
	}
	else
	{
		OwningScene = GetScene();
		FocusIndicator.NotifyResolveStyle(OwningScene.SceneClient.ActiveSkin, true, GetCurrentState());

		ListCollapseComplete();
	}
}

/**
 * Callback for starting the list's expand animation.
 */
protected event OnListExpanding()
{
	local UIScene OwningScene;

	if ( ExpandAnimation != None )
	{
		if ( CollapseAnimation != None && CollapseAnimation.SeqName != '' && IsAnimating(CollapseAnimation.SeqName) )
		{
			StopUIAnimation(CollapseAnimation.SeqName, CollapseAnimation, true);
		}

		PlayUIAnimation('', ExpandAnimation,,,,false);
		PlayFocusHintAnimation(ExpandFocusHintAnimation, true);
	}
	else
	{
		ListExpansionComplete();
	}

	OwningScene = GetScene();
	FocusIndicator.NotifyResolveStyle(OwningScene.SceneClient.ActiveSkin, true, FindActiveState());
}

/* == UnrealScript == */

/** Wrappers for checking the state of the list */
final function bool IsExpanded()
{
	return ListExpansionState == LES_Expanded;
}
final function bool IsExpanding()
{
	return ListExpansionState == LES_Expanding;
}
final function bool IsCollapsed()
{
	return ListExpansionState == LES_Expanded;
}
final function bool IsCollapsing()
{
	return ListExpansionState == LES_Collapsing;
}

/**
 * Wrapper for animating the focus hint label when the list transitions between collapsed/expanded states.
 */
protected function PlayFocusHintAnimation( UIAnimationSeq AnimSeq, optional bool bDelayRepositionUntilFadeOut )
{
	local UIScene OwningScene;
	local UIObject FocusHintObject;

	if ( AnimSeq != None )
	{
		OwningScene = GetScene();
		if ( OwningScene != None )
		{
			FocusHintObject = OwningScene.GetFocusHint(true);
			if ( FocusHintObject != None )
			{
				if ( bDelayRepositionUntilFadeOut )
				{
					bFadingSelectionHintOut = true;
					FocusHintObject.Add_UIAnimKeyFrameCompletedHandler(OnFocusHintFadeOutComplete);
				}
				if ( AnimSeq.SeqName == 'FocusHintExpandAnim' )
				{
					FocusHintObject.StopUIAnimation('FocusHintCollapseAnim');
				}
				else if ( AnimSeq.SeqName == 'FocusHintCollapseAnim' )
				{
					FocusHintObject.StopUIAnimation('FocusHintExpandAnim');
				}

				FocusHintObject.PlayUIAnimation('', AnimSeq,,,,false);
			}
		}
	}
}

/**
 * Wrapper for getting a reference to this list's UIState_Active instance.
 */
function UIState FindActiveState()
{
	local int StateIndex;
	local UIState_Active ActiveState;

	for ( StateIndex = 0; StateIndex < InactiveStates.Length; StateIndex++ )
	{
		ActiveState = UIState_Active(InactiveStates[StateIndex]);
		if ( ActiveState != None )
		{
			break;
		}
	}

	return ActiveState;
}

/* == SequenceAction handlers == */

/* == Delegate handlers == */
/**
 * Handler for the focus hint's OnUIAnim_KeyFrameCompleted delegate.  Disables the bool preventing the selection hint from being
 * repositioned next to the expanding list.
 *
 * @param	Sender		the widget that owns the animation sequence
 * @param	AnimName	the name of the sequence containing the key-frame that finished.
 * @param	TrackType	the identifier for the track containing the key-frame that completed.
 */
protected function OnFocusHintFadeOutComplete( UIScreenObject Sender, name AnimName, EUIAnimType TrackType )
{
	if ( AnimName == 'FocusHintExpandAnim' && TrackType == EAT_Opacity )
	{
		Sender.Remove_UIAnimKeyFrameCompletedHandler(OnFocusHintFadeOutComplete);
		bFadingSelectionHintOut = false;
	}
}

/**
 * Handler for 'animation completed' delegate.
 *
 * @param	Sender			the widget executing the animation containing the track that just completed
 * @param	AnimName		the name of the animation sequence containing the track that completed.
 * @param	TrackTypeMask	a bitmask of EUIAnimType values indicating which animation tracks completed.  The value is generated by
 *							left shifting 1 by the value of the track type.  A value of 0 indicates that all tracks have completed (in
 *							other words, that the entire animation sequence is completed).
 */
protected function ListAnimationComplete( UIScreenObject Sender, name AnimName, int TrackTypeMask )
{
	local UIScene OwningScene;

	if ( Sender == Self && TrackTypeMask == 0 )
	{
		if ( AnimName == 'ExpandAnim' )
		{
			ListExpansionComplete();
		}
		else if ( AnimName == 'CollapseAnim' )
		{
			if ( FocusIndicator != None )
			{
				OwningScene = GetScene();
				FocusIndicator.NotifyResolveStyle(OwningScene.SceneClient.ActiveSkin, true, GetCurrentState());
			}

			ListCollapseComplete();
		}
	}
}

/* == UIScreenObject interface == */

/**
 * Called once this screen object has been completely initialized, before it has activated its InitialState or called
 * Initialize on its children.  This event is only called the first time a widget is initialized.  If reparented, for
 * example, the widget would already be initialized so the Initialized event would not be called.
 */
event Initialized()
{
	Super.Initialized();

	NotifyActiveStateChanged = None;
}

/**
 * Called after this screen object's children have been initialized.  While the Initialized event is only called when
 * a widget is initialized for the first time, PostInitialize() will be called every time this widget receives a call
 * to Initialize(), even if the widget was already initialized.  Examples would be reparenting a widget.
 */
event PostInitialize()
{
	Super.PostInitialize();

	Add_UIAnimTrackCompletedHandler(ListAnimationComplete);
}

DefaultProperties
{
	ExpandCue=G2UI_DropdownOpenCue
	CollapseAcceptCue=G2UI_DropDownAcceptCue
	CollapseCancelCue=G2UI_DropDownCancelCue
	//CollapseCue=CollapseSelectionList

	ListExpansionState=LES_Collapsed
	CaptionDataSource=(MarkupString="Caption",RequiredFieldType=DATATYPE_Property,BindingIndex=COLLAPSESELECTION_CAPTION_DATABINDING_INDEX)
	SubmittedIndex=INDEX_NONE
	CaptionSize=(Value=0.45,ScaleType=UIEXTENTEVAL_PercentSelf,Orientation=UIORIENT_Horizontal)
	ValueSize=(Value=0.55,ScaleType=UIEXTENTEVAL_PercentSelf,Orientation=UIORIENT_Horizontal)
	ListIndent=(Value=20,ScaleType=UIEXTENTEVAL_Pixels,Orientation=UIORIENT_Horizontal)

	HorzBackgroundPadding(0)=(Value=-2,ScaleType=UIEXTENTEVAL_Pixels,Orientation=UIORIENT_Horizontal)
	HorzBackgroundPadding(1)=(Value=2,ScaleType=UIEXTENTEVAL_Pixels,Orientation=UIORIENT_Horizontal)
	VertBackgroundPadding(0)=(Value=-2,ScaleType=UIEXTENTEVAL_Pixels,Orientation=UIORIENT_Vertical)
	VertBackgroundPadding(1)=(Value=7,ScaleType=UIEXTENTEVAL_Pixels,Orientation=UIORIENT_Vertical)

	bEnableSceneUpdateNotifications=true
	RowAutoSizeMode=CELLAUTOSIZE_AdjustList

	GlobalCellStyle(ELEMENT_Normal)=(DefaultStyleTag="cmb_OptionLists",RequiredStyleClass=class'Engine.UIStyle_Combo')
	GlobalCellStyle(ELEMENT_Active)=(DefaultStyleTag="DefaultGearListStyleActive",RequiredStyleClass=class'Engine.UIStyle_Combo')
	GlobalCellStyle(ELEMENT_Selected)=(DefaultStyleTag="cmb_OptionListsSelected",RequiredStyleClass=class'Engine.UIStyle_Combo')
	GlobalCellStyle(ELEMENT_UnderCursor)=(DefaultStyleTag="DefaultGearListStyleHover",RequiredStyleClass=class'Engine.UIStyle_Combo')

//	ItemOverlayStyle(ELEMENT_Normal)=(DefaultStyleTag="img_OptionListBG",RequiredStyleClass=class'Engine.UIStyle_Image')
	ItemOverlayStyle(ELEMENT_Normal)=(DefaultStyleTag="imgBlank",RequiredStyleClass=class'Engine.UIStyle_Image')
	ItemOverlayStyle(ELEMENT_Selected)=(DefaultStyleTag="img_OptionListBGSelected",RequiredStyleClass=class'Engine.UIStyle_Image')

	Begin Object Class=UIComp_GearCaption Name=CaptionCompTemplate
		StringStyle=(DefaultStyleTag="cmb_ButtonText_ListOptions",RequiredStyleClass=class'Engine.UIStyle_Combo')
		StyleResolverTag="Caption Style"
		AutoSizeParameters[UIORIENT_Vertical]=(bAutoSizeEnabled=true)
		ClampRegion[UIORIENT_Horizontal]=(bSubregionEnabled=true,ClampRegionSize=(Value=0.45,ScaleType=UIEXTENTEVAL_PercentOwner),ClampRegionAlignment=UIALIGN_Left)
		ClampRegion[UIORIENT_Vertical]=(ClampRegionSize=(ScaleType=UIEXTENTEVAL_PercentOwner))
	End Object
	CaptionComponent=CaptionCompTemplate

	Begin Object Class=UIComp_GearCaption Name=ValueCompTemplate
		StringStyle=(DefaultStyleTag="cmb_BodyText",RequiredStyleClass=class'Engine.UIStyle_Combo')
		StyleResolverTag="Value Style"
//		AutoSizeParameters[UIORIENT_Vertical]=(bAutoSizeEnabled=true)
		ClampRegion[UIORIENT_Horizontal]=(bSubregionEnabled=true,ClampRegionOffset=(Value=0.45,ScaleType=UIEXTENTEVAL_PercentOwner),ClampRegionSize=(Value=0.55,ScaleType=UIEXTENTEVAL_PercentOwner),ClampRegionAlignment=UIALIGN_Right)
		ClampRegion[UIORIENT_Vertical]=(ClampRegionSize=(ScaleType=UIEXTENTEVAL_PercentOwner))
	End Object
	ValueComponent=ValueCompTemplate
/*(RemainingTime=0.1,Data=(DestAsColor=(A=0.3))),*/
	Begin Object Class=UIAnimationSeq Name=ExpandAnim
		SeqName=ExpandAnim
		Tracks(0)=(TrackType=EAT_PositionOffset,KeyFrames=((RemainingTime=0.125,Data=(DestAsVector=(Y=0.0)),InterpMode=UIANIMMODE_EaseOut)))
		Tracks(1)=(TrackType=EAT_Color,KeyFrames=((RemainingTime=0.125,Data=(DestAsColor=(A=1.0)),InterpMode=UIANIMMODE_EaseInOut,InterpExponent=1.5)))
	End Object
	Begin Object Class=UIAnimationSeq Name=ExpandFocusHint
		SeqName=FocusHintExpandAnim
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.125,Data=(DestAsFloat=0.0),InterpMode=UIANIMMODE_EaseOut,InterpExponent=1.5),(RemainingTime=0.05,Data=(DestAsFloat=1.0))))
	End Object
	ExpandAnimation=ExpandAnim
	ExpandFocusHintAnimation=ExpandFocusHint

	Begin Object Class=UIAnimationSeq Name=CollapseAnim
		SeqName=CollapseAnim
		Tracks(0)=(TrackType=EAT_PositionOffset,KeyFrames=((RemainingTime=0.125,Data=(DestAsVector=(Y=0.0))),(RemainingTime=0.0,Data=(DestAsVector=(Y=-1.0)))))
		Tracks(1)=(TrackType=EAT_Color,KeyFrames=((RemainingTime=0.125,Data=(DestAsColor=(A=0.0)),InterpMode=UIANIMMODE_EaseIn,InterpExponent=1.2)))
	End Object
	Begin Object Class=UIAnimationSeq Name=CollapseFocusHint
		SeqName=FocusHintCollapseAnim
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.125,Data=(DestAsFloat=0.0),InterpMode=UIANIMMODE_EaseIn,InterpExponent=2),(RemainingTime=0.05,Data=(DestAsFloat=1.0))))
	End Object
	CollapseAnimation=CollapseAnim
	CollapseFocusHintAnimation=CollapseFocusHint

	/* == UIList defaults == */
	bSingleClickSubmission=true
	DataSource=(RequiredFieldType=DATATYPE_Collection,BindingIndex=COLLAPSESELECTION_LIST_DATABINDING_INDEX)

	Begin Object Class=UIComp_CollapsingListPresenter Name=CollapsingListPresenter
		SelectionHintPadding=(Value=26,ScaleType=UIEXTENTEVAL_Pixels,Orientation=UIORIENT_Horizontal)
	End Object
	CellDataComponent=CollapsingListPresenter

	Begin Object Class=UIComp_DrawImage Name=BackgroundImageTemplate
		StyleResolverTag="Background Image Style"
		ImageStyle=(DefaultStyleTag="imgDropDownBG",RequiredStyleClass=class'Engine.UIStyle_Image')
	End Object
	BackgroundImageComponent=BackgroundImageTemplate

	Begin Object Class=UIComp_DrawImage Name=FocusIndicatorTemplate
		ImageStyle=(DefaultStyleTag="imgDropDownArrow",RequiredStyleClass=class'Engine.UIStyle_Image')
		StyleResolverTag="Focus Indicator"
	End Object
	FocusIndicator=FocusIndicatorTemplate
	FocusIndicatorSize=(Value=16,ScaleType=UIEXTENTEVAL_Pixels,Orientation=UIORIENT_Horizontal)


	/* == UIObject interface == */

	/* == UIScreenObject interface == */

	// let's give it a default position so that it doesn't fill up the entire parent widget when first placed
	Position={(	Value[UIFACE_Right]=300,	ScaleType[UIFACE_Right]=EVALPOS_PixelOwner,
				Value[UIFACE_Bottom]=100,	ScaleType[UIFACE_Bottom]=EVALPOS_PixelOwner	)}

}
