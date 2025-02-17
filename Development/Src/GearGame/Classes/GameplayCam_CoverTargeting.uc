/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GameplayCam_CoverTargeting extends GameplayCam_Cover
	native(Camera)
	config(Camera);

var() protected vector LeanOriginOffset;
var() protected vector LeanOriginOffsetLow;

var ECoverAction LastCoverAction;

/** Blend time for lean <-> 360 aiming transition */
var() float		LeanTo360AimingBlendTime;

cpptext
{
	/** Returns time to interpolate FOV changes. */
	virtual FLOAT GetBlendTime(class APawn* Pawn);
	virtual void GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot);
}

/**
 * When in targeting mode, set depthprioritygroup of pawn and weapon to foreground to prevent clipping
 */
function OnBecomeActive(Pawn CameraTarget, GearGameplayCameraMode PrevMode)
{
	local GearPawn GP;

	Super.OnBecomeActive(CameraTarget, PrevMode);

	LastCoverAction = CA_Default;

	GP = GearPawn(CameraTarget);
	if ( (GP != None) && (CameraTarget.Controller != None) &&  CameraTarget.Controller.IsLocalPlayerController() )
	{
		GP.SetDepthPriorityGroup(SDPG_Foreground);
	}
}

/**
 * When leave targeting mode, reset depthprioritygroup of pawn and weapon to world
 */
function OnBecomeInActive(Pawn CameraTarget, GearGameplayCameraMode NewMode)
{
	local GearPawn GP;

	Super.OnBecomeInActive(CameraTarget, NewMode);

	GP = GearPawn(CameraTarget);
	if ( (GP != None) && (CameraTarget.Controller != None) &&  CameraTarget.Controller.IsLocalPlayerController() )
	{
		GP.SetDepthPriorityGroup(SDPG_World);
	}
}

/**
 * When in targeting mode, make sure weapon changes update the weapon mesh depthprioritygroup properly
 */
function WeaponChanged(Controller C, Weapon OldWeapon, Weapon NewWeapon)
{
	if ( (C != None) && C.IsLocalPlayerController() )
	{
		if ( OldWeapon.Mesh != None )
		{
			OldWeapon.Mesh.SetViewOwnerDepthPriorityGroup(FALSE,SDPG_World);
		}
		if ( NewWeapon.Mesh != None )
		{
			NewWeapon.Mesh.SetViewOwnerDepthPriorityGroup(TRUE,SDPG_Foreground);
		}
	}
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
 

simulated protected function vector AdjustPROForStance(GearPawn WP, const out vector Offset)
{
	local vector AdjustedOffset;

	AdjustedOffset = Offset;
	if( WP.bIsMirrored )
	{
		AdjustedOffset.Y = -AdjustedOffset.Y;
	}

	//@fixme - slight hijack
	// we want to slow down the blend time when transition from popup->lean temporarily, and this is the simplest place to hook w/o adding a new function + event + etc
	if ((WP.CoverAction == CA_PopUp || WP.CoverAction == CA_LeanLeft || WP.CoverAction == CA_LeanRight) &&
		LastCoverAction != WP.CoverAction &&
		(LastCoverAction == CA_PopUp || LastCoverAction == CA_LeanLeft || LastCoverAction == CA_LeanRight))
	{
		InterpLocSpeed = default.InterpLocSpeed * 0.25f;
		WP.SetTimer( 0.8f,FALSE,nameof(self.RestoreInterpLocSpeedToDefault), self );
	}
	LastCoverAction = WP.CoverAction;

	return AdjustedOffset;
}

simulated function RestoreInterpLocSpeedToDefault()
{
	InterpLocSpeed = default.InterpLocSpeed;
}

/**
 * Overloaded from GearCameraMode.  Returns an offset, in pawn-local space, 
 * to be applied to the camera origin.
 */
simulated native function vector GetTargetRelativeOriginOffset(Pawn TargetPawn);

defaultproperties
{
	PawnRel_Posture_Standing=(X=-10,Y=40,Z=8)
	PawnRel_Posture_MidLevel=(X=-10,Y=8,Z=-82)

	PawnRel_Mod_MidLvlBlindUp=(X=0,Y=40,Z=8)

	PawnRel_Mod_PopUp_MidLevel=(X=15,Y=34,Z=86)
	PawnRel_Mod_360Aim_PopUp=(X=-7,Y=42,Z=35)
	PawnRel_Mod_Lean_Standing=(X=0,Y=10,Z=-4)
	PawnRel_Mod_Lean_MidLevel=(X=0,Y=40,Z=35)

	PawnRel_Mod_Lean_360Aim_Standing=(X=0,Y=0,Z=0)
	PawnRel_Mod_Lean_360Aim_MidLevel=(X=0,Y=34,Z=86)		// same as PawnRel_Mod_360Aim_PopUp

	PawnRel_Mod_CoverEdge=(X=0,Y=0,Z=0)

	// modify these to offset the camera when you are in cover and are targeting
	ViewOffset={(
		OffsetHigh=(X=-54,Y=0,Z=4),
		OffsetMid=(X=-80,Y=0,Z=0),
		OffsetLow=(X=-96,Y=0,Z=8),
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

	WorstLocOffset=(X=0,Y=-15,Z=85)
	WorstLocOffsetLow=(X=0,Y=-15,Z=45)

	BlendTime=0.067f
	LeanTo360AimingBlendTime=0.15f

	LeanOriginOffset=(Y=48)
	LeanOriginOffsetLow=(Y=48)

	StrafeLeftAdjustment=(X=0.f,Y=0.f,Z=0.f)
	StrafeRightAdjustment=(X=0.f,Y=0.f,Z=0.f)

	InterpLocSpeed=32.f

	bAdjustDOF=TRUE
}
