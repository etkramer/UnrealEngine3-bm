/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GameplayCam_DeployedShield extends GearGameplayCameraMode
	native(Camera)
	config(Camera);


/** Returns View relative offsets */
//simulated native function GetBaseViewOffsets(Pawn ViewedPawn, EGearCam_ViewportTypes ViewportConfig, float DeltaTime, out Vector out_Low, out Vector out_Mid, out Vector out_High);

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

defaultproperties
{
	BlendTime=0.13f						// tighter feel
	bDoPredictiveAvoidance=TRUE			// turrets should be placed in the clear by LDs
	bValidateWorstLoc=FALSE
	InterpLocSpeed=500

	// defaults, in case the turret's data doesn't work out
	ViewOffset={(
		OffsetHigh=(X=-20,Y=55,Z=25),
		OffsetLow=(X=-60,Y=55,Z=-15),
		OffsetMid=(X=-50,Y=55,Z=-15),
	)}
}
