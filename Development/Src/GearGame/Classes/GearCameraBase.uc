/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearCameraBase extends Object
	abstract
	native(Camera)
	config(Camera);

//•	Not an Actor, but has locally-stored Pos, Loc, FOV, etc.  
//•	Supports camera shakes, camera anims.
//•	Supports debug rendering with 3rd person camera mesh.

var transient GearPlayerCamera	PlayerCamera;

/** resets camera interpolation. Set on first frame and teleports to prevent long distance or wrong camera interpolations. */
var transient bool				bResetCameraInterpolation;


/** Called when the camera becomes active */
function OnBecomeActive();
/** Called when the camera becomes inactive */
function OnBecomeInActive();

/** Called to indicate that the next update should skip interpolation and snap to desired values. */
function ResetInterpolation()
{
	bResetCameraInterpolation = TRUE;
}



/** Expected to fill in OutVT with new camera pos/loc/fov. */
simulated function UpdateCamera(Pawn P, float DeltaTime, out TViewTarget OutVT);

simulated function ProcessViewRotation( float DeltaTime, Actor ViewTarget, out Rotator out_ViewRotation, out Rotator out_DeltaRot );

simulated function Init();

simulated event ModifyPostProcessSettings(out PostProcessSettings PP);

defaultproperties
{
}
