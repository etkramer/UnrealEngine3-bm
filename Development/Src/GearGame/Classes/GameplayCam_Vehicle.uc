/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * Camera mode for when player is in a vehicle.
 */
class GameplayCam_Vehicle extends GearGameplayCameraMode
	native(Camera)
	config(Camera);

cpptext
{
	/** Returns View relative offsets */
	virtual void GetBaseViewOffsets(class APawn* ViewedPawn, BYTE ViewportConfig, FLOAT DeltaTime, FVector& out_Low, FVector& out_Mid, FVector& out_High);

	/**
	 * Returns location and rotation, in world space, of the camera's basis point.  The camera will rotate
	 * around this point, offsets are applied from here, etc.
	 */
	virtual void GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot);
}

simulated function vector GetCameraWorstCaseLoc(Pawn TargetPawn)
{
	local GearVehicle WV;
	local GearWeaponPawn WP;
	local int SeatIndex;

	WV = GearVehicle(TargetPawn);
	if (WV == None)
	{
		WP = GearWeaponPawn(TargetPawn);
		if (WP != None)
		{
			WV = WP.MyVehicle;
			SeatIndex = WP.MySeatIndex;
		}
	}

	if (WV != None)
	{
		return WV.GetCameraWorstCaseLoc(SeatIndex);
	}
	else
	{
		return super.GetCameraWorstCaseLoc(TargetPawn);
	}
}

simulated function float GetDesiredFOV( Pawn ViewedPawn )
{
	local GearVehicle WV;
	local GearWeaponPawn WP;
	local int SeatIndex;

	WV = GearVehicle(ViewedPawn);
	if (WV == None)
	{
		WP = GearWeaponPawn(ViewedPawn);
		if (WP != None)
		{
			WV = WP.MyVehicle;
			SeatIndex = WP.MySeatIndex;
		}
	}
	if (WV != None)
	{
		return WV.GetCameraFOV(SeatIndex);
	}
	else
	{
		return Super.GetDesiredFOV(ViewedPawn);
	}
}


/** Returns true if mode should be using direct-look mode, false otherwise */
simulated native function bool UseDirectLookMode(Pawn CameraTarget);

/** Returns true if mode should lock camera to view target, false otherwise */
simulated native function bool LockedToViewTarget(Pawn CameraTarget);

simulated native function bool ShouldFollowTarget(Pawn CameraTarget, out float PitchInterpSpeed, out float YawInterpSpeed, out float RollInterpSpeed);

defaultproperties
{
	BlendTime=0.13f

	// defaults, in case the vehicle's data doesn't work out
	ViewOffset={(
		OffsetHigh=(X=-128,Y=0,Z=24),
		OffsetLow=(X=-160,Y=0,Z=32),
		OffsetMid=(X=-160,Y=0,Z=0),
	)}

	bDirectLook=FALSE
	bLockedToViewTarget=TRUE
	bFollowTarget=FALSE
	FollowingInterpSpeed_Pitch=5.f
	FollowingInterpSpeed_Yaw=1.5f
	FollowingInterpSpeed_Roll=5.f
	FollowingCameraVelThreshold=1000.f
	bInterpLocation=FALSE
	bInterpRotation=FALSE

	bDoPredictiveAvoidance=FALSE
	bValidateWorstLoc=FALSE
}
