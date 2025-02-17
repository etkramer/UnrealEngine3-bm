/**
 * Base class for data providers which provide the UI with with read/write access to settings which are configurable
 * per-player, such as input, display, audio, and general preferences.  Each local player in the game receives its own
 * set of unique PlayerSettingsProviders.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class PlayerSettingsProvider extends UISettingsProvider
	within UIDataStore_PlayerSettings
	native(inherit)
	deprecated
	abstract;

cpptext
{
	/**
	 * Provides the data provider with the player they were just bound to
	 *
	 * @param Player the local player associated with this player settings provider
	 */
	virtual void OnRegister(ULocalPlayer* Player)
	{
		eventOnRegister(Player);
	}

	/**
	 * Tells the provider that the player is no longer valid
	 */
	virtual void OnUnregister(void)
	{
		eventOnUnregister();
	}
}

/**
 * Binds the player to this provider
 *
 * @param InPlayer the player that is being bound to the provider
 */
event OnRegister(LocalPlayer InPlayer);

/**
 * Tells the provider that the player is no longer valid
 */
event OnUnregister();

DefaultProperties
{
	ProviderTag=PlayerSettingsProvider
}
