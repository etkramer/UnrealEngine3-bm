/**
 * Online player data store.
 * This adds screenshot functionality to the base engine class.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUIDataStore_OnlinePlayerData extends UIDataStore_OnlinePlayerData
	native(UIPrivate);

/** Provides access to the player's list of screenshots on disk */
var GearUIDataProvider_Screenshots ScreenshotsProvider;

/** The name of the data provider class to use as the default for screenshots */
var config string ScreenshotsProviderClassName;

/** The class that should be created when a player is bound to this data store */
var class<GearUIDataProvider_Screenshots> ScreenshotsProviderClass;

cpptext
{
	/* === UIDataStore interface === */

	/**
	* Loads the game specific OnlineProfileSettings class
	*/
	virtual void LoadDependentClasses(void);

	/**
	* Creates the data providers exposed by this data store
	*/
	virtual void InitializeDataStore(void);

	/**
	* Forwards the calls to the data providers so they can do their start up
	*
	* @param Player the player that will be associated with this DataStore
	*/
	virtual void OnRegister(ULocalPlayer* Player);

	/**
	* Tells all of the child providers to clear their player data
	*
	* @param Player ignored
	*/
	virtual void OnUnregister(ULocalPlayer*);

	/**
	* Gets the list of data fields exposed by this data provider
	*
	* @param OutFields Filled in with the list of fields supported by its aggregated providers
	*/
	virtual void GetSupportedDataFields(TArray<FUIDataProviderField>& OutFields);

	/**
	* Retrieves the list elements associated with the data tag specified.
	*
	* @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
	*							from GetElementProviderTags.
	* @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
	*
	* @return	TRUE if this data store contains a list element data provider matching the tag specified.
	*/
	virtual UBOOL GetListElements( FName FieldName, TArray<INT>& out_Elements );

	/**
	* Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
	* Used by the UI editor to know which cells are available for binding to individual list cells.
	*
	* @param	FieldName		the tag of the list element data provider that we want the schema for.
	*
	* @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
	*			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
	*/
	virtual TScriptInterface<class IUIListElementCellProvider> GetElementCellSchemaProvider( FName FieldName );
}

/**
* Registers the delegates with the providers so we can know when async data changes
*/
function RegisterDelegates()
{
	Super.RegisterDelegates();
	ScreenshotsProvider.AddPropertyNotificationChangeRequest(OnSettingProviderChanged);
}

function ClearDelegates()
{
	ScreenshotsProvider.RemovePropertyNotificationChangeRequest(OnSettingProviderChanged);
	Super.ClearDelegates();
}


