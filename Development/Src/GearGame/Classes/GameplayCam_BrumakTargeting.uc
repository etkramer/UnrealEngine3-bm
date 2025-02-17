/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GameplayCam_BrumakTargeting extends GameplayCam_BrumakDriver
	native(Camera)
	config(Camera);

/** returns camera mode desired FOV */
function float GetDesiredFOV( Pawn ViewedPawn )
{
	local GearWeapon			Wpn;

	Wpn = GearWeapon(GetGearPC().Pawn.Weapon);

	if (Wpn != None)
	{
		return Wpn.GetTargetingFOV(FOVAngle);
	}
	else
	{
		return FOVAngle;
	}
}

defaultproperties
{
	BlendTime=0.2f

	// defaults, in case the vehicle's data doesn't work out
	ViewOffset={(
		OffsetHigh=(X=-450,Y=300,Z=500),
		OffsetLow=(X=50,Y=300,Z=400),
		OffsetMid=(X=-200,Y=300,Z=150),
	)}
	WorstLocOffset=(X=0,Y=0,Z=150)

	bDirectLook=FALSE
	bLockedToViewTarget=TRUE
	bFollowTarget=FALSE
	FollowingInterpSpeed_Pitch=5.f
	FollowingInterpSpeed_Yaw=1.5f
	FollowingInterpSpeed_Roll=5.f
	FollowingCameraVelThreshold=1000.f

	bAdjustDOF=TRUE
}

