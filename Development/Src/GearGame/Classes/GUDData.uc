/**
 * Class: GUDData
 * Base class data container for Gears Unscripted Dialog data.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GUDData extends Object
	// within GearPawn?
	dependson(GUDTypes)
	native(Sound);

/** This is an array, populated at runtime, of currently loaded GUDBanks for this data object. */
var transient array<GUDBank> LoadedGUDBanks;

defaultproperties
{
}

