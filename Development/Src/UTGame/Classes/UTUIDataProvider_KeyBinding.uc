/**
 * Provides a possible key binding for the game.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTUIDataProvider_KeyBinding extends UTUIResourceDataProvider
	native(UI)
	PerObjectConfig;

/** Friendly displayable name to the player. */
var config localized string FriendlyName;

/** Command to bind the key to. */
var config string Command;

/** Whether this bind must be bound to leave the screen or not. */
var config bool bIsCrucialBind;
