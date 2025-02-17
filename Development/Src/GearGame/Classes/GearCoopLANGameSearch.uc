/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Used to find coop campaigns that are publicly joinable
 */
class GearCoopLANGameSearch extends GearCoopGameSearch
	native(Online);

/** Filters out any mp returned items */
native event SortSearchResults();

defaultproperties
{
	bIsLanQuery=true
}
