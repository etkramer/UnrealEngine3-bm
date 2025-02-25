/**
 * This datastore allows games to map aliases to strings that may change based on the current platform or language setting.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 */
class GearUIDataStore_StringAliasBindingsMap extends UIDataStore_StringAliasMap
	native(UIPrivate)
	Config(UI);

const SABM_FIND_FIRST_BIND = -2;

/** Struct to map localized text from [GameMappedStrings] to [ButtonFont] for non-PC platforms */
struct native ControllerMap
{
	/** The key - which is a [GameMappedStrings] */
	var name KeyName;
	/** The XBox360 mapping */
	var String XBoxMapping;
};

struct native BindCacheElement
{
	var name KeyName;
	var String MappingString;
	var int FieldIndex;
};

/** The value of the input's bUsingGamepad value the last time this datastore was used - for clearing cache */
var transient bool bPreviousUsingGamepadValue;

/**
* Map of command names of the form GBA_ to the key bindings that should be drawn on screen
* since its heavy on string compares
*/
var	const	transient	native	Map_Mirror	CommandToBindNames{TMap<FName, FBindCacheElement>};

/** Array of mappings for localized text from [GameMappedStrings] to [ButtonFont] for non-PC platforms */
var config array<ControllerMap> ControllerMapArray;

/**
 * Set MappedString to be the localized string using the FieldName as a key
 * Returns the index into the mapped string array of where it was found.
 */
native virtual function int GetStringWithFieldName( String FieldName, out String MappedString );

/**
 * Called by GetStringWithFieldName() to retreive the string using the input binding system.
 */
native virtual function int GetBoundStringWithFieldName( String FieldName, out String MappedString, optional out int StartIndex, optional out String BindString );

/*
* Given an input command of the form GBA_ return the mapped keybinding string
* Returns TRUE if it exists, FALSE otherwise
*/
native protected final function bool FindMappingInBoundKeyCache(string Command, out string MappingStr, out int FieldIndex);

/** Given a input command of the form GBA_ and its mapping store that in a lookup for future use */
native protected final function AddMappingToBoundKeyCache(string Command, string MappingStr, int FieldIndex);

/** Clear the command to input keybinding cache	*/
native final function ClearBoundKeyCache();

DefaultProperties
{
	Tag=StringAliasBindings
}
