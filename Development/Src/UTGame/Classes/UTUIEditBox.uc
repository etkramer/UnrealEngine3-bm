/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Extended version of editbox for UT3.
 */
class UTUIEditBox extends UIEditBox
	native(UI);

cpptext
{
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
	 * Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
	 *
	 * @return	TRUE if this subscriber successfully resolved and applied the updated value.
	 */
	virtual UBOOL RefreshSubscriberValue(INT BindingIndex=INDEX_NONE);

	/** Saves the value for this subscriber. */
	virtual UBOOL SaveSubscriberValue(TArray<class UUIDataStore*>& out_BoundDataStores,INT BindingIndex=INDEX_NONE);
}

