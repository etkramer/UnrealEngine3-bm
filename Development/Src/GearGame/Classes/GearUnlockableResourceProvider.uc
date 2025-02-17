/**
 * Provides the UI with data about a single unlockable item in Gears2.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUnlockableResourceProvider extends GearResourceDataProvider;

/** the id for this unlockable item */
var	config		EGearUnlockable			UnlockableId;

/** the friendly name for the item */
var	localized	string					UnlockableName;

/** a short description of what the item is */
var	localized	string					UnlockableDescription;

/** a preview image of the unlockable item */
var	config		string					UnlockableImage;

DefaultProperties
{

}
