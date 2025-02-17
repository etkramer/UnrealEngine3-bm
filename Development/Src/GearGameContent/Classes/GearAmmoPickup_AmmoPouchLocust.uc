/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAmmoPickup_AmmoPouchLocust extends GearAmmoPickup_AmmoPouch;

simulated event string GetDebugAbbrev()
{
	return "APPouchLocust";
}

defaultproperties
{
	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'AmmoPickups.Mesh.Ammo'
	End Object
}
