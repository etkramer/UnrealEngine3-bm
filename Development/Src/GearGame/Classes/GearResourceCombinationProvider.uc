/**
 * Base class for Gears2 resource combination providers.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearResourceCombinationProvider extends UIResourceCombinationProvider
	abstract;

/**
 * Accessor for getting a reference to the GearProfileSettings object associated with this provider.
 */
function GearProfileSettings GetGearProfile()
{
	local GearProfileSettings Result;

	if ( ProfileProvider != None )
	{
		Result = GearProfileSettings(ProfileProvider.Profile);
	}

	return Result;
}

DefaultProperties
{

}
