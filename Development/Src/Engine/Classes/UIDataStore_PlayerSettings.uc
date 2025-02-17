/**
 * This class provides the UI with access to player settings providers.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIDataStore_PlayerSettings extends UIDataStore_Settings
	native(inherit)
	deprecated;

cpptext
{
	/* === UIDataStore interface === */
	/**
	 * Hook for performing any initialization required for this data store
	 */
	virtual void InitializeDataStore();

	/**
	 * Called when this data store is added to the data store manager's list of active data stores.
	 *
	 * @param	PlayerOwner		the player that will be associated with this DataStore.  Only relevant if this data store is
	 *							associated with a particular player; NULL if this is a global data store.
	 */
	virtual void OnRegister( class ULocalPlayer* PlayerOwner );

	/* === UUIDataProvider interface === */
	/**
	 * Gets the list of data fields exposed by this data provider.
	 *
	 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
	 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
	 */
	virtual void GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields );
}

/**
 * Array of PlayerSettingsProvider derived class names to load and initialize
 */
var	const	config		array<string>							PlayerSettingsProviderClassNames;

/**
 * Array of PlayerSettingsProvider derived classes loaded from PlayerSettingsProviderClassNames.  Filled in InitializeDataStore().
 */
var	const	transient	array<class<PlayerSettingsProvider> >	PlayerSettingsProviderClasses;

/**
 * The data provider for all player specific settings, such as input, display, and audio settings.  Each element of the array
 * represents the settings for the player associated with the corresponding element of the Engine.GamePlayers array.
 */
var			transient	array<PlayerSettingsProvider>			PlayerSettings;

/**
 * The index [into the Engine.GamePlayers array] for the player that this data store provides settings for.
 */
var	const	transient		int									PlayerIndex;

/**
 * Returns a reference to the ULocalPlayer that this PlayerSettingsProvdier provider settings data for
 */
native final function LocalPlayer GetPlayerOwner() const;

/**
 * Clears all data provider references.
 */
final function ClearDataProviders()
{
	local int i;

	for ( i = 0; i < PlayerSettings.Length; i++ )
	{
		//@todo - what to do if CleanupDataProvider return false?
		PlayerSettings[i].CleanupDataProvider();
	}

	PlayerSettings.Length = 0;
}

/**
 * Called when the current map is being unloaded.  Cleans up any references which would prevent garbage collection.
 *
 * @return	TRUE indicates that this data store should be automatically unregistered when this game session ends.
 */
function bool NotifyGameSessionEnded()
{
	ClearDataProviders();

	// this data store should be automatically unregistered when the game session ends
	return true;
}

DefaultProperties
{
	Tag=PlayerSettings
	PlayerIndex=INDEX_NONE
}
