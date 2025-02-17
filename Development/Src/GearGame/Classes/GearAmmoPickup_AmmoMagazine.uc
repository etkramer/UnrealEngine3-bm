
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAmmoPickup_AmmoMagazine extends GearAmmoPickup_AmmoCase;

simulated event string GetDebugAbbrev()
{
	return "APMag";
}

defaultproperties
{
	NumMagsDefault=(X=0.5f,Y=1.0f)

	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'AmmoPickups.Ammopickup_Magazine'
		Scale=1.0
		Translation=(Z=-20)
	End Object
}
