/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * Camera mode for when player is in a brumak.
 */
class GameplayCam_BrumakDriver extends GearGameplayCameraMode
	native(Camera)
	config(Camera);

cpptext
{
	virtual void GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot);
};

/** returns View relative offsets */
simulated native function GetBaseViewOffsets(Pawn ViewedPawn, EGearCam_ViewportTypes ViewportConfig, float DeltaTime, out Vector out_Low, out Vector out_Mid, out Vector out_High);

simulated function vector GetCameraWorstCaseLoc(Pawn TargetPawn)
{
	local GearPawn_LocustBrumakBase Brumak;
	local Vector  SocketLoc;
	local Rotator SocketRot;

	Brumak = GearPawn_LocustBrumakBase(TargetPawn);
	if( Brumak == None )
	{
		Brumak = GearPawn_LocustBrumakBase(TargetPawn.Base);
	}

	if( Brumak != None )
	{
		Brumak.Mesh.GetSocketWorldLocationAndRotation( Brumak.DriverSocket, SocketLoc, SocketRot );
		return SocketLoc + (WorstLocOffset >> Brumak.Rotation);
	}
	else
	{
		return super.GetCameraWorstCaseLoc( TargetPawn );
	}
}

/** Returns true if mode should be using direct-look mode, false otherwise */
simulated native function bool UseDirectLookMode(Pawn CameraTarget);

/** Returns true if mode should lock camera to view target, false otherwise */
simulated native function bool LockedToViewTarget(Pawn CameraTarget);

simulated native function bool ShouldFollowTarget(Pawn CameraTarget, out float PitchInterpSpeed, out float YawInterpSpeed, out float RollInterpSpeed);

defaultproperties
{
	BlendTime=0.2f

	// defaults, in case the vehicle's data doesn't work out
	ViewOffset={(
		OffsetHigh=(X=-400,Y=300,Z=500),
		OffsetLow=(X=-300,Y=350,Z=200),
		OffsetMid=(X=-100,Y=300,Z=250),
	)}
	WorstLocOffset=(X=0,Y=0,Z=150)

	bDirectLook=FALSE
	bLockedToViewTarget=TRUE
	bFollowTarget=FALSE
	FollowingInterpSpeed_Pitch=5.f
	FollowingInterpSpeed_Yaw=1.5f
	FollowingInterpSpeed_Roll=5.f
	FollowingCameraVelThreshold=1000.f
}
