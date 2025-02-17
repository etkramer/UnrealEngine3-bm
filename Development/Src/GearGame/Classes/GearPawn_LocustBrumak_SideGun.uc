/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_LocustBrumak_SideGun extends GearPawn_LocustBrumakHelper_Base;

var bool	bIsLeftGun;

function AddDefaultInventory();
simulated function UpdateAnimSetList();

simulated event Vector GetAimOffsetOrigin()
{
	return Weapon.GetPhysicalFireStartLoc();
}

defaultproperties
{
	ControllerClass=class'GearAI_Brumak_SideGun'
	RemoteRole=ROLE_None
	bNeverAValidEnemy=TRUE

	Mesh=None
	Components.Remove(GearPawnMesh)
	bCollideActors=FALSE
	bCollideWorld=FALSE
	bBlockActors=FALSE
	bProjTarget=FALSE

	SightRadius=0
}

