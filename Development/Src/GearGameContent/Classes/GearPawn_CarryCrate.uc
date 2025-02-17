/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_CarryCrate extends GearPawn_CarryCrate_Base
	placeable;


defaultproperties
{
	Begin Object Name=CrateComp
		StaticMesh=StaticMesh'Cine_Props01.Mesh.GOW_CineProps_BombCrate'
		Rotation=(Yaw=-16384)
		Translation=(Z=-20)
	End Object

	Begin Object Name=CollisionCylinder
		CollisionRadius=120.0
		CollisionHeight=50.0
	End Object

	PelvisBoneName=""

	Physics=PHYS_Walking
	ControllerClass=None
	bRunPhysicsWithNoController=TRUE

	CarrierRelPos=(Y=95.0,Z=12.0)
	HandleRelPos=(X=-62.0,Z=60.0)


	WalkAnimVelScale=0.011
	CrateWalkRotFactor=250.0
	CrateRollXTransFactor=-100.0

	CrateXSpringStiffness=0.15
	CrateXSpringDamping=0.03
	CrateXSpringAccelFactor=-2.0
	CrateXSpringMaxDisplacement=35.0

	CrateAmbientTransFreq=2.0
	CrateAmbientTransMag=0.022
}
