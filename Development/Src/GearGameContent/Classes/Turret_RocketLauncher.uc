
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Turret_RocketLauncher extends Turret_TroikaCabal
	config(Pawn);


/** Overridden to get xhair when targeting. */
simulated function bool WantsCrosshair(PlayerController PC)
{
	return TRUE;
}
simulated function bool ShouldTargetingModeZoomCamera()
{
	return FALSE;
}

defaultproperties
{
	DefaultInventory(0)=class'GearWeap_RocketLauncherTurret'

	ViewPitchMin=-5000
	ViewPitchMax=6500		// Pawn arm IK breaks down if more than this

	TurretTurnRateScale=0.66

	CameraViewOffsetHigh=(X=-300,Y=60,Z=25)
	CameraViewOffsetLow=(X=-250,Y=60,Z=80)
	CameraViewOffsetMid=(X=-300,Y=60,Z=60)
	CameraTargetingViewOffsetHigh=(X=-185,Y=0,Z=40)
	CameraTargetingViewOffsetLow=(X=-185,Y=0,Z=40)
	CameraTargetingViewOffsetMid=(X=-185,Y=0,Z=40)

	bEnforceHardAttach=FALSE
	bBlockActors=FALSE
	bCollideActors=TRUE
	bCollideWorld=FALSE
	bNoEncroachCheck=TRUE

	bAllowTargetingCamera=FALSE

	// use the cylinder for collision
	CollisionComponent=CollisionCylinder

	Begin Object Name=SkelMeshComponent0
		CollideActors=FALSE
		BlockActors=FALSE
	End Object
}
