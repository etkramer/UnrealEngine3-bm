/**
 * A button that has two (and possibly three, in the future) state - checked and unchecked.
 *
 * @todo - implement support for 3 check states (checked, unchecked, partially checked)
 * @todo - implement data store binding
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UICheckbox extends UIButton
	native(inherit)
	DontAutoCollapseCategories(Data)
	implements(UIDataStorePublisher);

/** this sound is played when this widget is checked */
var(Sound)				name						CheckedCue;

/** this sound is played when this widget is unchecked */
var(Sound)				name						UncheckedCue;

/** the data store that this checkbox retrieves its value from */
var(Data)	private							UIDataStoreBinding		ValueDataSource;

/** Component for rendering the button background image */
var(Components)	editinline	const	noclear	UIComp_DrawImage		CheckedImageComponent;

/**
 * Controls whether this button is considered checked.  When bIsChecked is TRUE, CheckedImage will be rendered over
 * the button background image, using the current style.
 */
var(Value)	private							bool					bIsChecked;

cpptext
{
	/* === UUIScreenObject interface === */
	/**
	 * Render this checkbox.
	 *
	 * @param	Canvas	the FCanvas to use for rendering this widget
	 */
	virtual void Render_Widget( FCanvas* Canvas );

	/**
	 * Changes the checked image for this checkbox, creating the wrapper UITexture if necessary.
	 *
	 * @param	NewImage		the new surface to use for this UIImage
	 */
	virtual void SetCheckImage( class USurface* NewImage );

	/**
	 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
	 *
	 * This version adds the CheckedImageComponent (if non-NULL) to the StyleSubscribers array.
	 */
	virtual void InitializeStyleSubscribers();

protected:

	/**
	 * Handles input events for this checkbox.
	 *
	 * @param	EventParms		the parameters for the input event
	 *
	 * @return	TRUE to consume the key event, FALSE to pass it on.
	 */
	virtual UBOOL ProcessInputKey( const struct FSubscribedInputEventParameters& EventParms );

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

	/**
	 * Called after this object has been completely de-serialized.  This version migrates values for the deprecated CheckedImage,
	 * CheckCoordinates, and CheckedStyle properties over to the CheckedImageComponent.
	 */
	virtual void PostLoad();
}

/* === Unrealscript === */
/**
 * Changes the checked image for this checkbox, creating the wrapper UITexture if necessary.
 *
 * @param	NewImage		the new surface to use for this UIImage
 */
final function SetCheckImage( Surface NewImage )
{
	if ( CheckedImageComponent != None )
	{
		CheckedImageComponent.SetImage(NewImage);
	}
}

/**
 * Returns TRUE if this button is in the checked state, FALSE if in the
 */
final function bool IsChecked()
{
	return bIsChecked;
}

/** UIDataSourceSubscriber interface */
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
native final virtual function NotifyDataStoreValueUpdated( UIDataStore SourceDataStore, bool bValuesInvalidated, name PropertyTag, UIDataProvider SourceProvider, int ArrayIndex );

/**
 * Retrieves the list of data stores bound by this subscriber.
 *
 * @param	out_BoundDataStores		receives the array of data stores that subscriber is bound to.
 */
native final virtual function GetBoundDataStores( out array<UIDataStore> out_BoundDataStores );

/**
 * Notifies this subscriber to unbind itself from all bound data stores
 */
native final function ClearBoundDataStores();

/**
 * Resolves this subscriber's data store binding and publishes this subscriber's value to the appropriate data store.
 *
 * @param	out_BoundDataStores	contains the array of data stores that widgets have saved values to.  Each widget that
 *								implements this method should add its resolved data store to this array after data values have been
 *								published.  Once SaveSubscriberValue has been called on all widgets in a scene, OnCommit will be called
 *								on all data stores in this array.
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 *
 * @return	TRUE if the value was successfully published to the data store.
 */
native virtual function bool SaveSubscriberValue( out array<UIDataStore> out_BoundDataStores, optional int BindingIndex=INDEX_NONE );

/**
 * Changed the checked state of this checkbox and activates a checked event.
 *
 * @param	bShouldBeChecked	TRUE to turn the checkbox on, FALSE to turn it off
 * @param	PlayerIndex			the index of the player that generated the call to SetValue; used as the PlayerIndex when activating
 *								UIEvents; if not specified, the value of GetBestPlayerIndex() is used instead.
 */
native function SetValue( bool bShouldBeChecked, optional int PlayerIndex=INDEX_NONE );


/* === Kismet action handlers === */
final function OnSetBoolValue( UIAction_SetBoolValue Action )
{
	SetValue(Action.bNewValue);
}

DefaultProperties
{
	ValueDataSource=(RequiredFieldType=DATATYPE_Property)

	Begin Object class=UIComp_DrawImage Name=CheckedImageTemplate
		ImageStyle=(DefaultStyleTag="DefaultCheckbox",RequiredStyleClass=class'Engine.UIStyle_Image')
		StyleResolverTag="Check Style"
	End Object
	CheckedImageComponent=CheckedImageTemplate

	ClickedCue=""
	CheckedCue="CheckboxChecked"
	UncheckedCue="CheckboxUnchecked"
}
