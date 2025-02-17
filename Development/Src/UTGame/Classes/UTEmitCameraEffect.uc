/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTEmitCameraEffect extends Emitter
	abstract
	native;

/** How far in front of the camera this emitter should live. */
var() protected float DistFromCamera;

/** Camera this emitter is attached to, will be notified when emitter is destroyed */
var protected UTPlayerController Cam;


simulated event PostBeginPlay()
{
	ParticleSystemComponent.SetDepthPriorityGroup(SDPG_Foreground);
	super.PostBeginPlay();
}

function Destroyed()
{
	Cam.RemoveCameraEffect(self);
}

/** Tell the emitter what camera it is attached to. */
function RegisterCamera( UTPlayerController inCam )
{
	Cam = inCam;
}

/** Given updated camera information, adjust this effect to display appropriately. */
native function UpdateLocation( const out vector CamLoc, const out rotator CamRot, float CamFOVDeg );

defaultproperties
{
	Begin Object Name=ParticleSystemComponent0
	End Object

	// makes sure I tick after the camera
	TickGroup=TG_DuringAsyncWork

	DistFromCamera=90

	LifeSpan=10.0f

	bDestroyOnSystemFinish=true
	bNetInitialRotation=true
	bNoDelete=false

}



