/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * Camera mode for when player is in a vehicle.
 */
class GameplayCam_VehicleTurret extends GearGameplayCameraMode
	native(Camera)
	config(Camera);

/** Returns View relative offsets */
simulated native function GetBaseViewOffsets(Pawn ViewedPawn, EGearCam_ViewportTypes ViewportConfig, float DeltaTime, out Vector out_Low, out Vector out_Mid, out Vector out_High);

simulated function vector GetCameraWorstCaseLoc(Pawn TargetPawn)
{
	local GearWeaponPawn WWP;
	WWP = GearWeaponPawn(TargetPawn);

	if (WWP != None)
	{
		return WWP.GetCameraWorstCaseLoc();
	}
	else
	{
		return super.GetCameraWorstCaseLoc(TargetPawn);
	}
}

defaultproperties
{
	BlendTime=0.075f

	bDoPredictiveAvoidance=FALSE
	bValidateWorstLoc=FALSE

	InterpLocSpeed=10000.f

	// defaults, in case the vehicle's data doesn't work out
	ViewOffset={(
		OffsetHigh=(X=-128,Y=0,Z=24),
		OffsetLow=(X=-160,Y=0,Z=32),
		OffsetMid=(X=-160,Y=0,Z=0),
	)}
	ViewOffset_ViewportAdjustments(CVT_16to9_HorizSplit)={(
		OffsetHigh=(X=0,Y=0,Z=-14),
		OffsetLow=(X=0,Y=0,Z=-14),
		OffsetMid=(X=0,Y=0,Z=-14),
		)}
	//ViewOffset_ViewportAdjustments(CVT_16to9_VertSplit)={(
	//	OffsetHigh=(X=0,Y=-20,Z=0),
	//	OffsetLow=(X=0,Y=-20,Z=0),
	//	OffsetMid=(X=0,Y=-20,Z=0),
	//	)}
	ViewOffset_ViewportAdjustments(CVT_4to3_Full)={(
		OffsetHigh=(X=0,Y=0,Z=17),
		OffsetLow=(X=0,Y=0,Z=17),
		OffsetMid=(X=0,Y=0,Z=17),
		)}
	ViewOffset_ViewportAdjustments(CVT_4to3_HorizSplit)={(
		OffsetHigh=(X=0,Y=0,Z=-15),
		OffsetLow=(X=0,Y=0,Z=-15),
		OffsetMid=(X=0,Y=0,Z=-15),
		)}
	//ViewOffset_ViewportAdjustments(CVT_4to3_VertSplit)={(
	//	OffsetHigh=(X=0,Y=0,Z=0),
	//	OffsetLow=(X=0,Y=0,Z=0),
	//	OffsetMid=(X=0,Y=0,Z=0),
	//	)}
}
