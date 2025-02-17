/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Used to find parties that are publicly joinable
 */
class GearLANPartySearch extends GearPartyGameSearch
	native(Online);

/** Filters out any coop returned items */
native event SortSearchResults();

defaultproperties
{
	bIsLanQuery=true
}
