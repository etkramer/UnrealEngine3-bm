/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GameplayCam_DBNO extends GearGameplayCameraMode
	config(Camera);

var() protected const int	MinPitch;
var() protected const int	MaxPitch;

/** Overridden to clamp pitch values. */
simulated function ProcessViewRotation( float DeltaTime, Actor ViewTarget, out Rotator out_ViewRotation, out Rotator out_DeltaRot )
{
	// Clamp Pitch
	SClampRotAxis( DeltaTime, out_ViewRotation.Pitch-ViewTarget.Rotation.Pitch, out_DeltaRot.Pitch, MaxPitch, MinPitch, 4.f );

	super.ProcessViewRotation(DeltaTime, ViewTarget, out_ViewRotation, out_DeltaRot);
}

defaultproperties
{
	ViewOffset={(
		OffsetHigh=(X=-210,Y=0,Z=-40),
		OffsetLow=(X=-210,Y=0,Z=-40),
		OffsetMid=(X=-210,Y=0,Z=-40),
		)}

	BlendTime=0.5f

	bDoPredictiveAvoidance=TRUE
	bValidateWorstLoc=FALSE

	WorstLocOffset=(X=0,Y=0,Z=64)

	TargetRelativeCameraOriginOffset=(X=64,Y=0,Z=0)

	// GameplayCam_DBNO vars
	MinPitch=-8000
	MaxPitch=-1000
}
