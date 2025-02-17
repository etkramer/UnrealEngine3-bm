/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Emit_Camera_WaterDroplets extends Emit_CameraLensEffectBase
	config(Weapon);


defaultproperties
{

//	I've got a new effect to put water droplets on the screen. It's subtle and spawns, all at once, between 3 and 8 droplets on the camera which fade over the same duration as the screen blood. I was hoping we could get them hooked up to various events that happen in the game. Some possible uses:

//Roll through water (spawn at middle-end of roll)
//when roadie running in water (every couple seconds of roadie running, tick to see if we're in water and spawn one of these? might have to make a version that has fewer drops for this one)
//							  If not when we are roadie running, maybe if we're behind someone that is?
//							  If an explosive weapon goes off near you in the water but doesn't necessarily kill you, spawn one of these

	PS_CameraEffect=ParticleSystem'CameraBlood.FX.P_Water_Droplets_Few'
	PS_CameraEffectNonExtremeContent=ParticleSystem'CameraBlood.FX.P_Water_Droplets_Few'
	bAllowMultipleInstances=TRUE
}



