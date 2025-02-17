/**
 * "Fixed" camera mode.  Views through a CameraActor in the level.
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearFixedCamera extends GearCameraBase
	config(Camera);

/** FOV to fall back to if we can't get one from somewhere else. */
var const protected float DefaultFOV;

/** Autoframing fixed camera. */
var protected transient FixedCam_AutoFraming	AutoFramingCam;


simulated function Init()
{
	super.Init();

	// Setup camera modes
	if ( AutoFramingCam == None )
	{
		AutoFramingCam = new(Outer) class'FixedCam_AutoFraming';
		AutoFramingCam.FixedCam = self;
		//AutoFramingCam.Init();
	}
}

simulated function UpdateCamera(Pawn P, float DeltaTime, out TViewTarget OutVT)
{
	local CameraActor CamActor;
	local GearPawn GP;
	local bool bAutoFraming;

	// are we looking at a camera actor?
	CamActor = CameraActor(OutVT.Target);

	if (CamActor == None)
	{
		// see if we're looking at a pawn who is in a camera volume, 
		// and use that actor if so
		GP = GearPawn(P);
		if ( (GP != None) && (GP.CameraVolumes.length > 0) )
		{
			CamActor = GP.CameraVolumes[0].CameraActor;
			bAutoFraming = GP.CameraVolumes[0].bAutomaticFraming;
		}
	}

	if (bAutoFraming)
	{
		AutoFramingCam.UpdateCamera2(CamActor, P, DeltaTime, OutVT);
	}
	else
	{
		if (CamActor != None)
		{
			// we're attached to a camactor, use it's FOV
			OutVT.POV.FOV = CamActor.FOVAngle;
		}
		else
		{
			OutVT.POV.FOV = DefaultFOV;
		}

		// copy loc/rot from actor we're attached to
		if (OutVT.Target != None)
		{
			OutVT.POV.Location = CamActor.Location;
			OutVT.POV.Rotation = CamActor.Rotation;
		}
	}

	// cameraanims, etc
	PlayerCamera.ApplyCameraModifiers(DeltaTime, OutVT.POV);

	// if we had to reset camera interpolation, then turn off flag once it's been processed.
	bResetCameraInterpolation = FALSE;
}

/** Called when Camera mode becomes active */
function OnBecomeActive()
{
	bResetCameraInterpolation = TRUE;
	super.OnBecomeActive();
}


defaultproperties
{
	DefaultFOV=80
}