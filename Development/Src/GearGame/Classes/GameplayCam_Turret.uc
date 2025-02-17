/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * Camera mode for when player is in a vehicle.
 */
class GameplayCam_Turret extends GearGameplayCameraMode
	native(Camera)
	config(Camera);


/** Returns View relative offsets */
simulated native function GetBaseViewOffsets(Pawn ViewedPawn, EGearCam_ViewportTypes ViewportConfig, float DeltaTime, out Vector out_Low, out Vector out_Mid, out Vector out_High);

/** returns camera mode desired FOV */
function float GetDesiredFOV( Pawn ViewedPawn )
{
	local GearTurret WT;

	WT = GearTurret(ViewedPawn);

	if ( (WT != None) && (WT.CameraFOV > 0.f) )
	{
		return WT.CameraFOV;
	}
	else
	{
		return super.GetDesiredFOV(ViewedPawn);
	}
}

defaultproperties
{
	BlendTime=0.08f						// tight feel
	bDoPredictiveAvoidance=FALSE		// turrets should be placed in the clear by LDs
	bValidateWorstLoc=FALSE
	InterpLocSpeed=500

	// defaults, in case the turret's data doesn't work out
	ViewOffset={(
		OffsetHigh=(X=-100,Y=0,Z=15),
		OffsetLow=(X=-100,Y=0,Z=80),
		OffsetMid=(X=-100,Y=0,Z=30),
		)}
	ViewOffset_ViewportAdjustments(CVT_16to9_HorizSplit)={(
		OffsetHigh=(X=-40,Y=0,Z=0),
		OffsetLow=(X=-45,Y=0,Z=-10),
		OffsetMid=(X=-40,Y=0,Z=0),
		)}
	ViewOffset_ViewportAdjustments(CVT_16to9_VertSplit)={(
		OffsetHigh=(X=0,Y=0,Z=0),
		OffsetLow=(X=0,Y=0,Z=0),
		OffsetMid=(X=0,Y=0,Z=0),
		)}
	ViewOffset_ViewportAdjustments(CVT_4to3_Full)={(
		OffsetHigh=(X=0,Y=0,Z=6.5),
		OffsetLow=(X=0,Y=0,Z=6.5),
		OffsetMid=(X=0,Y=0,Z=6.5),
		)}
	ViewOffset_ViewportAdjustments(CVT_4to3_HorizSplit)={(
		OffsetHigh=(X=-20,Y=0,Z=0),
		OffsetLow=(X=-30,Y=0,Z=-5),
		OffsetMid=(X=-20,Y=0,Z=0),
		)}
	ViewOffset_ViewportAdjustments(CVT_4to3_VertSplit)={(
		OffsetHigh=(X=0,Y=0,Z=6.5),
		OffsetLow=(X=0,Y=0,Z=6.5),
		OffsetMid=(X=0,Y=0,Z=6.5),
		)}

}
