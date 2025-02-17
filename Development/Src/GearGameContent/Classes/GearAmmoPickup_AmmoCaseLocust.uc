/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAmmoPickup_AmmoCaseLocust extends GearAmmoPickup_AmmoCase;

simulated event string GetDebugAbbrev()
{
	return "APCaseLocust";
}

defaultproperties
{
	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'AmmoPickups.Mesh.SM_LOC_AmmoCrate'
	End Object
}
