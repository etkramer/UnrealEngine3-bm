/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * Camera mode for when player is in a vehicle.
 */
class GameplayCam_TurretTargeting extends GameplayCam_Turret
	native(Camera)
	config(Camera);


/** Returns View relative offsets */
simulated native function GetBaseViewOffsets(Pawn ViewedPawn, EGearCam_ViewportTypes ViewportConfig, float DeltaTime, out Vector out_Low, out Vector out_Mid, out Vector out_High);

/** returns camera mode desired FOV */
function float GetDesiredFOV( Pawn ViewedPawn )
{
	local GearTurret WT;

	WT = GearTurret(ViewedPawn);

	if ( (WT != None) && (WT.CameraTargetingFOV > 0.f) )
	{
		return WT.CameraTargetingFOV;
	}
	else
	{
		return super.GetDesiredFOV(ViewedPawn);
	}
}


defaultproperties
{
	ViewOffset_ViewportAdjustments(CVT_16to9_HorizSplit)={(
		OffsetHigh=(X=0,Y=0,Z=-2),
		OffsetLow=(X=0,Y=0,Z=-2),
		OffsetMid=(X=0,Y=0,Z=-2),
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
		OffsetHigh=(X=0,Y=0,Z=-2),
		OffsetLow=(X=0,Y=0,Z=-2),
		OffsetMid=(X=0,Y=0,Z=-2),
		)}
	ViewOffset_ViewportAdjustments(CVT_4to3_VertSplit)={(
		OffsetHigh=(X=0,Y=0,Z=6.5),
		OffsetLow=(X=0,Y=0,Z=6.5),
		OffsetMid=(X=0,Y=0,Z=6.5),
		)}

	bAdjustDOF=TRUE
}
