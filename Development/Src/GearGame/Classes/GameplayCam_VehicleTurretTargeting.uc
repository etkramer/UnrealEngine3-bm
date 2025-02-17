/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * Camera mode for when player is in a vehicle.
 */
class GameplayCam_VehicleTurretTargeting extends GameplayCam_VehicleTurret
	native(Camera)
	config(Camera);

/** Returns View relative offsets */
simulated native function GetBaseViewOffsets(Pawn ViewedPawn, EGearCam_ViewportTypes ViewportConfig, float DeltaTime, out Vector out_Low, out Vector out_Mid, out Vector out_High);

defaultproperties
{
	BlendTime=0.1f

	ViewOffset={(
		OffsetHigh=(X=-50,Y=0,Z=55),
		OffsetLow=(X=-50,Y=0,Z=55),
		OffsetMid=(X=-50,Y=0,Z=55),
	)}

	ViewOffset_ViewportAdjustments(CVT_16to9_HorizSplit)={(
		OffsetHigh=(X=0,Y=0,Z=-8),
		OffsetLow=(X=0,Y=0,Z=-8),
		OffsetMid=(X=0,Y=0,Z=-8),
		)}
	//ViewOffset_ViewportAdjustments(CVT_16to9_VertSplit)={(
	//	OffsetHigh=(X=0,Y=-20,Z=0),
	//	OffsetLow=(X=0,Y=-20,Z=0),
	//	OffsetMid=(X=0,Y=-20,Z=0),
	//	)}
	ViewOffset_ViewportAdjustments(CVT_4to3_Full)={(
		OffsetHigh=(X=0,Y=0,Z=10),
		OffsetLow=(X=0,Y=0,Z=10),
		OffsetMid=(X=0,Y=0,Z=10),
		)}
	ViewOffset_ViewportAdjustments(CVT_4to3_HorizSplit)={(
		OffsetHigh=(X=0,Y=0,Z=-8),
		OffsetLow=(X=0,Y=0,Z=-8),
		OffsetMid=(X=0,Y=0,Z=-8),
		)}
	//ViewOffset_ViewportAdjustments(CVT_4to3_VertSplit)={(
	//	OffsetHigh=(X=0,Y=0,Z=0),
	//	OffsetLow=(X=0,Y=0,Z=0),
	//	OffsetMid=(X=0,Y=0,Z=0),
	//	)}

	bAdjustDOF=TRUE
}
