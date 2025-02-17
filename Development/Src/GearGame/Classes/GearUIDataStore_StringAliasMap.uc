/**
 * This datastore allows games to map aliases to strings that may change based on the current platform or language setting.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 */
class GearUIDataStore_StringAliasMap extends UIDataStore_StringAliasMap
	native(UIPrivate)
	Config(UI);

/**
 * Set MappedString to be the localized string using the FieldName as a key
 * Returns the index into the mapped string array of where it was found.
 */
native virtual function int GetStringWithFieldName( String FieldName, out String MappedString );

