/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GameplayCam_CoverTargetingGrenade extends GameplayCam_Cover
	config(Camera)
	native(Camera);

/** Affects camera pitch, but not pitch of grenade arc.  Goal is to limit camera pitch so plane the player is on is always visible. */
var() protected vector2d GrenadeCamPitchLimit;

cpptext
{
	// GearGameplayCameraMode interface
	virtual void GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot);
};


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

simulated protected function vector AdjustPROForStance(GearPawn WP, const out vector Offset)
{
	local vector AdjustedOffset;

	AdjustedOffset = Offset;
	if( WP.bIsMirrored )
	{
		AdjustedOffset.Y = -AdjustedOffset.Y;
	}

	return AdjustedOffset;
}

defaultproperties
{
	////ViewMaxLimit=(Pitch=0,Yaw=13650,Roll=0)
	////ViewMinLimit=(Pitch=0,Yaw=-13650,Roll=0)
	////ViewMaxLimit_Lean=(Pitch=0,Yaw=16384,Roll=0)
	////ViewMinLimit_Lean=(Pitch=0,Yaw=-4550,Roll=0)
	////ViewMaxLimit_Circular=(Pitch=0,Yaw=2048,Roll=0)
	////ViewMinLimit_Circular=(Pitch=0,Yaw=-2048,Roll=0)

	//PawnRel_Posture_Standing=(X=0,Y=45,Z=40)
	PawnRel_Posture_MidLevel=(X=0,Y=40,Z=-20)

	////PawnRel_Mod_MidLvlBlindUp=(X=-16,Y=0,Z=20)

	PawnRel_Mod_PopUp_MidLevel=(X=0,Y=30,Z=32)
	//PawnRel_Mod_360Aim_PopUp=(X=0,Y=10,Z=10)

	PawnRel_Mod_Lean_Standing=(X=0,Y=83,Z=0)
	PawnRel_Mod_Lean_MidLevel=(X=0,Y=70,Z=0)
	////PawnRel_Mod_Lean_360Aim_Standing=(X=0,Y=0,Z=0)
	////PawnRel_Mod_Lean_360Aim_MidLevel=(X=0,Y=0,Z=0)

	PawnRel_Mod_CoverEdge=(X=0,Y=0,Z=0)

	ViewOffset={(
		OffsetHigh=(X=-128,Y=0,Z=24),
		OffsetLow=(X=-140,Y=0,Z=32),
		//OffsetMid=(X=-160,Y=0,Z=0),
		OffsetMid=(X=-160,Y=0,Z=35),
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

	WorstLocOffset=(X=0,Y=-15,Z=85)
	WorstLocOffsetLow=(X=0,Y=-15,Z=45)

	BlendTime=0.067f

	GrenadeCamPitchLimit=(X=-10000.f,Y=3500.f)
}

