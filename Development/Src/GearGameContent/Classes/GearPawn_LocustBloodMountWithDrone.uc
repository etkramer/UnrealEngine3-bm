
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustBloodMountWithDrone extends GearPawn_LocustBloodMountRiderless;

simulated function DoDriverAttachment(GearPawn InDriver)
{
	local int i;

	Super.DoDriverAttachment(InDriver);

	// GSM_DeathAnimFallFromBeast required animations
	for (i = 0; i < ArrayCount(InDriver.KismetAnimSets); i++)
	{
		if (InDriver.KismetAnimSets[i] == None)
		{
			InDriver.KismetAnimSets[i] = AnimSet'Locust_Grunt.Locust_Grunt_OnReaver';
			break;
		}
	}
	InDriver.UpdateAnimSetList();
}

defaultproperties
{
	DriverClass=class'GearPawn_LocustBeastRider'
	DriverAnimTree=AnimTree'Locust_Bloodmount.Animations.AT_BloodMountRider'
}
