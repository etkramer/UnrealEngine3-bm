/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_LocustBurstPistol_Skorge extends GearWeap_LocustBurstPistol_Kantus;

simulated function bool ShouldAutoReload()
{
	return FALSE;
}

function Name GetGrenadeSocketName( GearPawn P )
{
	return P.RightHandSocketName;
}

defaultproperties
{
	Begin Object Name=WeaponMesh
		Scale3D=(X=1,Y=1,Z=-1)
	End Object
}