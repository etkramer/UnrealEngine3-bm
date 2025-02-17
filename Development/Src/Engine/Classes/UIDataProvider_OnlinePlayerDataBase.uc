/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This class is responsible for retrieving the friends list from the online
 * subsystem and populating the UI with that data.
 */
class UIDataProvider_OnlinePlayerDataBase extends UIDataProvider
	native(inherit)
	transient
	abstract;

/** Holds the player that this provider is getting friends for */
var LocalPlayer Player;

cpptext
{
	/**
	 * Provides the data provider with the player they were just bound to
	 *
	 * @param Player the local player associated with this player settings provider
	 */
	virtual void OnRegister(ULocalPlayer* InPlayer)
	{
		eventOnRegister(InPlayer);
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
event OnRegister(LocalPlayer InPlayer)
{
	Player = InPlayer;
}

/**
 * Tells the provider that the player is no longer valid
 */
event OnUnregister()
{
	Player = None;
}
