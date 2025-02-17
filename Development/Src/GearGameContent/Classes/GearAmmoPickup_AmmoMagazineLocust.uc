/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAmmoPickup_AmmoMagazineLocust extends GearAmmoPickup_AmmoMagazine;

simulated event string GetDebugAbbrev()
{
	return "APMagLocust";
}

defaultproperties
{
	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'AmmoPickups.Mesh.Magazine'
	End Object
}
