/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * Camera mode for when player is in a vehicle.
 */
class GameplayCam_MountedHeavyWeapon extends GearGameplayCameraMode
	native(Camera)
	config(Camera);


/** Returns View relative offsets */
simulated native function GetBaseViewOffsets(Pawn ViewedPawn, EGearCam_ViewportTypes ViewportConfig, float DeltaTime, out Vector out_Low, out Vector out_Mid, out Vector out_High);

/** returns camera mode desired FOV */
function float GetDesiredFOV( Pawn ViewedPawn )
{
	local GearWeap_HeavyBase HeavyWeap;

	HeavyWeap = GearWeap_HeavyBase(ViewedPawn.Weapon);

	// give the heavy weapon a crack at setting the fov
	if( (HeavyWeap != None) && (HeavyWeap.MountedCameraFOV > 0.f) )
	{
		return HeavyWeap.MountedCameraFOV;
	}
	else
	{
		return super.GetDesiredFOV(ViewedPawn);
	}
}

defaultproperties
{
	BlendTime=0.13f						// tighter feel
	//bDoPredictiveAvoidance=FALSE		// turrets should be placed in the clear by LDs
	bValidateWorstLoc=FALSE
	InterpLocSpeed=500

	// defaults, in case the turret's data doesn't work out
	ViewOffset={(
		OffsetHigh=(X=-100,Y=0,Z=15),
		OffsetLow=(X=-100,Y=0,Z=80),
		OffsetMid=(X=-100,Y=0,Z=30),
		)}
}
