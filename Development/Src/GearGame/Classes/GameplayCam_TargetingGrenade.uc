/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GameplayCam_TargetingGrenade extends GearGameplayCameraMode
	config(Camera)
	native(Camera);

/** Affects camera pitch, but not pitch of grenade arc.  Goal is to limit camera pitch so plane the player is on is always visible. */
var() protected vector2d GrenadeCamPitchLimit;

cpptext
{
	// GearGameplayCameraMode interface
	virtual void GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot);
};

function OnBecomeActive(Pawn TargetPawn, GearGameplayCameraMode PrevMode)
{
	bInterpRotation	= TRUE;
	Super.OnBecomeActive(TargetPawn, PrevMode);
}

/** returns camera mode desired FOV */
function float GetDesiredFOV( Pawn ViewedPawn )
{
	local GearWeapon			Wpn;

	Wpn = GearWeapon(ViewedPawn.Weapon);

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
	ViewOffset={(
		OffsetHigh=(X=-128,Y=70,Z=25),
		OffsetLow=(X=-160,Y=70,Z=48),
		OffsetMid=(X=-160,Y=70,Z=16),
		)}
	ViewOffset_ViewportAdjustments(CVT_16to9_HorizSplit)={(
		OffsetHigh=(X=0,Y=0,Z=-5),
		OffsetLow=(X=0,Y=0,Z=-5),
		OffsetMid=(X=0,Y=0,Z=-5),
		)}
	ViewOffset_ViewportAdjustments(CVT_16to9_VertSplit)={(
		OffsetHigh=(X=0,Y=0,Z=0),
		OffsetLow=(X=0,Y=0,Z=0),
		OffsetMid=(X=0,Y=0,Z=0),
		)}
	ViewOffset_ViewportAdjustments(CVT_4to3_Full)={(
		OffsetHigh=(X=0,Y=0,Z=3),
		OffsetLow=(X=0,Y=0,Z=3),
		OffsetMid=(X=0,Y=0,Z=3),
		)}
	ViewOffset_ViewportAdjustments(CVT_4to3_HorizSplit)={(
		OffsetHigh=(X=0,Y=0,Z=-5),
		OffsetLow=(X=0,Y=0,Z=-5),
		OffsetMid=(X=0,Y=0,Z=-5),
		)}
	ViewOffset_ViewportAdjustments(CVT_4to3_VertSplit)={(
		OffsetHigh=(X=0,Y=0,Z=0),
		OffsetLow=(X=0,Y=0,Z=0),
		OffsetMid=(X=0,Y=0,Z=0),
		)}

	bInterpRotation=TRUE
	InterpRotSpeed=8

	StrafeRightAdjustment=(X=0,Y=22,Z=10)
	StrafeLeftAdjustment=(X=0,Y=-12,Z=10)
	StrafeOffsetScalingThreshold=200

	RunFwdAdjustment=(X=20,Y=0,Z=10)
	RunBackAdjustment=(X=-35,Y=0,Z=10)
	RunOffsetScalingThreshold=200

	WorstLocOffset=(X=7,Y=30,Z=80)

	BlendTime=0.067f

	GrenadeCamPitchLimit=(X=-10000.f,Y=3500.f)
}

