/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GameplayCam_Targeting extends GearGameplayCameraMode
	config(Camera);

var() vector CrouchedPawnRelativeOffset;

/**
 * When in targeting mode, set depthprioritygroup of pawn and weapon to foreground to prevent clipping
 */
function OnBecomeActive(Pawn CameraTarget, GearGameplayCameraMode PrevMode)
{
	local GearPawn GP;

	Super.OnBecomeActive(CameraTarget, PrevMode);

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

/** Get Pawn's relative offset (from location based on pawn's rotation */
function vector GetPawnRelativeOffset( Pawn P )
{
	local vector	FinalOffset;
	local GearPawn	WP;

	WP = GearPawn(P);

	if( P.bIsCrouched && VSize(P.Velocity) < 1 )
	{
		FinalOffset = CrouchedPawnRelativeOffset;
	}
	else
	{
		FinalOffset = PawnRelativeOffset;
	}

	// if pawn is facing left, then mirror Y axis.
	if( (WP != None) && WP.bIsMirrored )
	{
		FinalOffset.Y = -FinalOffset.Y;
	}

	return FinalOffset;
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
		OffsetHigh=(X=-40,Y=40,Z=24),
		OffsetLow=(X=-125,Y=45,Z=20),
		OffsetMid=(X=-70,Y=40,Z=6),
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


	CrouchedPawnRelativeOffset=(Z=-15)

	StrafeRightAdjustment=(X=0,Y=22,Z=10)
	StrafeLeftAdjustment=(X=0,Y=-12,Z=10)
	StrafeOffsetScalingThreshold=200

	RunFwdAdjustment=(X=20,Y=0,Z=10)
	RunBackAdjustment=(X=-35,Y=0,Z=10)
	RunOffsetScalingThreshold=200

	WorstLocOffset=(X=0,Y=25,Z=70)

	BlendTime=0.067f

	bAdjustDOF=TRUE
}

