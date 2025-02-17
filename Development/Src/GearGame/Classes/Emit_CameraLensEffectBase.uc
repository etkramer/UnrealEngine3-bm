/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Emit_CameraLensEffectBase extends EmitterCameraLensEffectBase
	abstract
	native
	config(Weapon);

/** How far in front of the camera this emitter should live, at an FOV of 80 degrees. */
var() protected const float				DistFromCamera;

/** Camera this emitter is attached to, will be notified when emitter is destroyed */
var private transient GearPlayerCamera	Cam;

/** TRUE if multiple instances of this emitter can exist simultaneously, FALSE otherwise.  */
var const bool							bAllowMultipleInstances;


simulated function Destroyed()
{
	Cam.RemoveCameraLensEffect(self);
}

/** Tell the emitter what camera it is attached to. */
simulated function RegisterCamera(GearPlayerCamera inCam)
{
	Cam = inCam;
}

/** Given updated camera information, adjust this effect to display appropriately. */
simulated native function UpdateLocation(const out vector CamLoc, const out rotator CamRot, float CamFOVDeg);

/** Called when this emitter is re-triggered, for bAllowMultipleInstances=FALSE emitters. */
function NotifyRetriggered();

defaultproperties
{
	// makes sure I tick after the camera
	TickGroup=TG_PostAsyncWork

	DistFromCamera=90

	LifeSpan=10.0f

	bOnlyOwnerSee=TRUE

}



