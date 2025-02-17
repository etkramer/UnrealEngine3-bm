/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Vehicle_Crane extends Vehicle_Crane_Base
	placeable;

defaultproperties
{
	// yaw
	MaxYawAngVel=1500.0
	YawTorque=200000.0
	YawDamping=2.0

	MinYaw=-10000
	MaxYaw=10000

	// raise
	MaxRaiseAngVel=1500.0
	RaiseTorque=100000.0
	RaiseDamping=2.0

	MinRaise=-8000
	MaxRaise=6000

	DisableSwingDist=500
	SwingFadeOutTime=1.0

	DrumSpinFactor=-50.0

	bVehicleSpaceCamera=TRUE
	bOnlyInheritVehicleSpaceYaw=TRUE
	bDoActorLocationToCamStartTrace=FALSE
	CameraLag=0.0

	Begin Object Name=SVehicleMesh
		SkeletalMesh=SkeletalMesh'COG_Crane.Mesh.Crane_Sinkhole'
		PhysicsAsset=PhysicsAsset'COG_Crane.Mesh.Crane_Sinkhole_Physics'
		AnimTreeTemplate=AnimTree'COG_Crane.AT_COG_Crane'
		//bUseSingleBodyPhysics=FALSE
		bCastDynamicShadow=TRUE
	End Object

	Seats(0)={(	//GunClass=class'GearGameContent.GearVWeap_ReaverCannon',
		//GunSocket=("RideCannon"),
		//GunPivotPoints=(b_Gun_Front),
		bSeatVisible=true,
		SeatRotation=(Yaw=16384),
		SeatOffset=(Z=500.0f),
		SeatBone="Root_Bone",
		CameraTag="CamSocket",
		CameraBaseOffset=(X=-400,Y=0,Z=-600),
		CameraViewOffsetHigh=(X=-2000,Y=0,Z=0),
		CameraViewOffsetMid=(X=-2000,Y=0,Z=-0),
		CameraViewOffsetLow=(X=-2000,Y=0,Z=0),
		WorstCameraLocOffset=(X=0,Y=0,Z=0),
		bDisableOffsetZAdjust=TRUE,
		DriverDamageMult=0.0
		)}

	CraneArmMovingLoop=SoundCue'Cinematic_MatineeCues.Sinkhole.Crane_Arm_Cue'
	CraneEngineIdleLoop=SoundCue'Cinematic_MatineeCues.Sinkhole.Crane_Engine_Cue'
	CraneEngineStartLoop=SoundCue'Cinematic_MatineeCues.Sinkhole.CraneStartCue'
	CraneEngineStopLoop=SoundCue'Cinematic_MatineeCues.Sinkhole.CraneEndCue'
	CraneArmAudioMaxYawAngVel=250.f
}
