/**
 * SaveGameSummary
 * Helper object embedded in save games containing information about saved map.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SaveGameSummary extends Object
	native;

/**
 * Name of level this savegame is saved against. The level must be already in memory
 * before the savegame can be applied.
 */
var	name		BaseLevel;

/** Human readable description */
var	string		Description;
