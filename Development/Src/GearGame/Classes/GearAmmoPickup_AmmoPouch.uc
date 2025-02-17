
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAmmoPickup_AmmoPouch extends GearAmmoPickup_AmmoCase;

simulated event string GetDebugAbbrev()
{
	return "APPouch";
}

defaultproperties
{
	NumMagsDefault=(X=0.75f,Y=1.5f)

	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'AmmoPickups.Ammopickup_Case'
		Scale=1.0
		Translation=(Z=-20)
	End Object
}
