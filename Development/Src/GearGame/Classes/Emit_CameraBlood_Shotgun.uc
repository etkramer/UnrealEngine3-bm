/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Emit_CameraBlood_Shotgun extends Emit_CameraLensEffectBase
	config(Weapon);



defaultproperties
{
	PS_CameraEffect=ParticleSystem'CameraBlood.FX.P_ShotgunBlood'
	PS_CameraEffectNonExtremeContent=none
	bAllowMultipleInstances=TRUE
}



